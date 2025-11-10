
#include "FlareSector.h"
#include "../Flare.h"

#include "FlareGame.h"
#include "FlarePlanetarium.h"
#include "FlareSimulatedSector.h"
#include "FlareCollider.h"

#include "../Player/FlarePlayerController.h"

#include "../Spacecrafts/FlareShell.h"
#include "../Spacecrafts/FlareSpacecraft.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

#define LOCTEXT_NAMESPACE "FlareSector"
//#define DEBUG_SECTORLOADTIME
//#define DEBUG_SPACESHIPPLACEMENT

#define SHIP_LARGE_EXPLOSION_CHANCE 0.05f
#define SHIP_SMALL_EXPLOSION_CHANCE 0.10f
#define SHIP_DRONE_EXPLOSION_CHANCE 0.50f

UFlareSector::UFlareSector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SectorRepartitionCache = false;
	IsDestroyingSector = false;
}

/*----------------------------------------------------
  Save
----------------------------------------------------*/

void UFlareSector::Load(UFlareSimulatedSector* Parent)
{
#ifdef DEBUG_SECTORLOADTIME
	double StartTs = FPlatformTime::Seconds();
#endif

	DestroySector();
	IsPaused = false;
	ParentSector = Parent;
	LocalTime = Parent->GetData()->LocalTime;
	CompanyShipsPerCompanyCache.Empty();
	CompanySpacecraftsPerCompanyCache.Empty();
	SectorRepartitionCache = false;

	// Load asteroids
	SectorAsteroids.Reserve(ParentSector->GetData()->AsteroidData.Num());
	for (int i = 0 ; i < ParentSector->GetData()->AsteroidData.Num(); i++)
	{
		LoadAsteroid(ParentSector->GetData()->AsteroidData[i]);
	}

	// Load meteorite
	for (FFlareMeteoriteSave& Meteorite : ParentSector->GetData()->MeteoriteData)
	{
		if (Meteorite.DaysBeforeImpact == 0 && Meteorite.Damage < Meteorite.BrokenDamage)
		{
			LoadMeteorite(Meteorite);
		}
	}

	// Load safe location spacecrafts
	SectorSpacecraftsCache.Reserve(ParentSector->GetSectorSpacecrafts().Num());
	SectorStations.Reserve(ParentSector->GetSectorStations().Num());
	SectorShips.Reserve(ParentSector->GetSectorShips().Num());

	for (int i = 0; i < ParentSector->GetSectorStations().Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = ParentSector->GetSectorStations()[i];
		LoadSpacecraft(Spacecraft,true);
	}

	for (int i = 0; i < ParentSector->GetSectorShips().Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = ParentSector->GetSectorShips()[i];
		if ((!Spacecraft->IsReserve() && Spacecraft->GetData().SpawnMode != EFlareSpawnMode::InternalDocked)
			|| Parent->GetGame()->GetPC()->GetPlayerShip() == Spacecraft)
		{
			LoadSpacecraft(Spacecraft, false);
		}
	}

// Load bombs
	SectorBombs.Reserve(ParentSector->GetData()->BombData.Num());
	for (int i = 0; i < ParentSector->GetData()->BombData.Num(); i++)
	{
		LoadBomb(ParentSector->GetData()->BombData[i]);
	}

	IsDestroyingSector = false;

	for (int i = 0; i < SectorSpacecrafts.Num(); i++)
	{
		FinishLoadSpacecraftReattachRedock(SectorSpacecrafts[i]);
	}

	for (int i = 0; i < SectorSpacecrafts.Num(); i++)
	{
		FinishLoadSpacecraft(SectorSpacecrafts[i], true);
	}

	for (UFlareCompany* Company : UniqueCompanies)
	{
		Company->NewSectorLoaded();
	}

#ifdef DEBUG_SECTORLOADTIME
	double EndTs = FPlatformTime::Seconds();
	FLOGV("** SectorLoadTime Done in %.6fs", EndTs - StartTs);
#endif
}

void UFlareSector::Tick(float DeltaSeconds)
{
	if (IsDestroyingSector || IsPaused)
	{
		return;
	}

	if (SignalLocalSectorUpdateSectorBattleStates)
	{
		UpdateSectorBattleStates();
		SignalLocalSectorUpdateSectorBattleStates = false;
	}
	
	for (int i = 0; i < SectorBombs.Num(); i++)
	{
		if (IsDestroyingSector)
		{
			break;
		}
		AFlareBomb* Bomb = SectorBombs[i];
		if (IsValid(Bomb))
		{
			if (Bomb->IsSafeDestroying())
			{
				continue;
			}
			Bomb->TickBomb(DeltaSeconds);
		}
	}

	for (int i = 0; i < SectorMeteorites.Num(); i++)
	{
		if (IsDestroyingSector)
		{
			break;
		}
		AFlareMeteorite* Meteorite = SectorMeteorites[i];
		if (IsValid(Meteorite))
		{
			Meteorite->TickMeteorite(DeltaSeconds);
		}
	}

	for (int i = 0; i < SectorSpacecrafts.Num(); i++)
	{
		if (IsDestroyingSector)
		{
			break;
		}

		AFlareSpacecraft* Spacecraft = SectorSpacecrafts[i];

		if (IsValid(Spacecraft))
		{
			if (Spacecraft->IsSafeDestroying())
			{
				continue;
			}
			Spacecraft->TickSpacecraft(DeltaSeconds);
		}
	}
}

void UFlareSector::UpdateSectorBattleStates()
{
	for (UFlareCompany* Company : UniqueCompanies)
	{
		if (Company && !GetGame()->GetGameWorld()->GetPrimarySimulate())
		{
			GetSimulatedSector()->UpdateSectorBattleState(Company);
		}
	}
	GetSimulatedSector()->CheckSkirmishEndCondition();
}

void UFlareSector::SimulateLocalCompanyAI()
{
	if (!IsDestroyingSector && !GetGame()->IsSkirmish())
	{
		for (UFlareCompany* Company : UniqueCompanies)
		{
			if (Company && !GetGame()->GetGameWorld()->GetPrimarySimulate())
			{
				Company->SimulateActiveAI();
			}
		}
	}
}

void UFlareSector::AddReinforcingShip(UFlareSimulatedSpacecraft* Ship)
{
	if (Ship)
	{
		if (Ship->GetData().SpawnMode != EFlareSpawnMode::Safe && Ship->GetData().SpawnMode != EFlareSpawnMode::InternalDocked)
		{
			if (LoadSpacecraft(Ship, true))
			{
				Ship->GetActive()->SetLoadedAndReady();
			}
		}
		else if (Ship->GetData().SpawnMode == EFlareSpawnMode::InternalDocked)
		{
			if (Ship->GetShipMaster() && Ship->GetShipMaster()->GetActive())
			{
//				Ship->GetShipMaster()->GetActive()->SetUndockedAllShips(false);
			}
			else
			{
				Ship->SetSpawnMode(EFlareSpawnMode::Exit);
				if (LoadSpacecraft(Ship, true))
				{
					Ship->GetActive()->SetLoadedAndReady();
				}
			}
		}
	}
}

void UFlareSector::Save()
{
	FFlareSectorSave* SectorData  = GetSimulatedSector()->GetData();

	SectorData->BombData.Empty();
	SectorData->AsteroidData.Empty();
	// Meteorites have references but save must be call

	for (int i = 0 ; i < SectorBombs.Num(); i++)
	{
		if (SectorBombs[i]->IsSafeDestroying())
		{
			continue;
		}

		SectorData->BombData.Add(*SectorBombs[i]->Save());
	}

	SectorData->AsteroidData.Reserve(SectorAsteroids.Num());
	for (int i = 0 ; i < SectorAsteroids.Num(); i++)
	{
		SectorData->AsteroidData.Add(*SectorAsteroids[i]->Save());
	}

	for (AFlareMeteorite* Meteorite : SectorMeteorites)
	{
		if (IsValid(Meteorite) && !Meteorite->IsBroken())
		{
			Meteorite->Save();
		}
	}

	SectorData->LocalTime = LocalTime + GetGame()->GetPlanetarium()->GetSmoothTime();
} 

void UFlareSector::AddDestroyedSpacecraft(AFlareSpacecraft* Spacecraft, bool ForceExplosion)
{
	if (Spacecraft)
	{
		bool RemoveSectorSpacecrafts = false;
		if (ParentSector->BattleBringInReserveShips(Spacecraft->GetParent()->GetCompany(), Spacecraft->GetParent()))
		{
			RemoveSectorSpacecrafts = true;
			SpaceCraftExplosionCheck(Spacecraft);
		}
		// lower chance in normal circumstances to cause an explosion chain-reaction
		else
		{
			if (ForceExplosion
				|| (Spacecraft->GetSize() == EFlarePartSize::L && FMath::FRand() <= SHIP_LARGE_EXPLOSION_CHANCE)
				|| (Spacecraft->GetSize() == EFlarePartSize::S && !Spacecraft->GetDescription()->IsDroneShip && FMath::FRand() <= SHIP_SMALL_EXPLOSION_CHANCE)
				|| (Spacecraft->GetDescription()->IsDroneShip && FMath::FRand() <= SHIP_DRONE_EXPLOSION_CHANCE))
			{
				RemoveSectorSpacecrafts = true;
				SpaceCraftExplosionCheck(Spacecraft);
			}
		}
		RemoveSpacecraft(Spacecraft, false);
	}
}

void UFlareSector::SpaceCraftExplosionCheck(AFlareSpacecraft* Spacecraft)
{
	//99% chance of ship violently exploding before death
	//alternatively skip to the instant final large explosion
	if (FMath::FRand() <= 0.99)
	{
		Spacecraft->BeginExplodingShip();
	}
	else
	{
		Spacecraft->SafeHide();
	}
}

void UFlareSector::RemoveSpacecraft(AFlareSpacecraft* Spacecraft, bool RemoveSectorSpacecrafts)
{
	if (Spacecraft)
	{
		if (Spacecraft->IsStation())
		{
			SectorStations.Remove(Spacecraft);
		}
		else
		{
			SectorShips.Remove(Spacecraft);
		}

		if(RemoveSectorSpacecrafts)
		{
			SectorSpacecrafts.Remove(Spacecraft);
		}

		FName SpacecraftImmatriculation = Spacecraft->GetImmatriculation();
		SectorSpacecraftsCache.Remove(SpacecraftImmatriculation);

		UFlareCompany* Company = Spacecraft->GetCompany();

		if (CompanyShipsPerCompanyCache.Contains(Company))
		{
			CompanyShipsPerCompanyCache[Company].Remove(Spacecraft);
		}
		if (CompanySpacecraftsPerCompanyCache.Contains(Company))
		{
			CompanySpacecraftsPerCompanyCache[Company].Remove(Spacecraft);
		}
	}
}

void UFlareSector::DestroySector()
{
	FLOG("UFlareSector::DestroySector");
	IsDestroyingSector = true;
	IsPaused = false;

	SignalLocalSectorUpdateSectorBattleStates = false;
	AFlareSpacecraft* PlayerShip = nullptr;

	if (ParentSector != nullptr)
	{
		AFlarePlayerController* PC = GetGame()->GetPC();
		if (PC)
		{
			PlayerShip = PC->GetShipPawn();
		}
	}

	// Remove spacecrafts from world
	for (int SpacecraftIndex = 0 ; SpacecraftIndex < SectorShips.Num(); SpacecraftIndex++)
	{
		SectorShips[SpacecraftIndex]->FinishAutoPilots();
		SectorShips[SpacecraftIndex]->SafeDestroy();
	}

	for (int SpacecraftIndex = 0; SpacecraftIndex < SectorStations.Num(); SpacecraftIndex++)
	{
		SectorStations[SpacecraftIndex]->SafeDestroy();
	}

	for (int BombIndex = 0 ; BombIndex < SectorBombs.Num(); BombIndex++)
	{
		if (SectorBombs[BombIndex])
		{
			SectorBombs[BombIndex]->FinishSafeDestroy();
		}		
	}

	for (int AsteroidIndex = 0 ; AsteroidIndex < SectorAsteroids.Num(); AsteroidIndex++)
	{
		SectorAsteroids[AsteroidIndex]->SafeDestroy();
	}

	for (AFlareMeteorite* Meteorite : SectorMeteorites)
	{
		if (IsValid(Meteorite))
		{
			Meteorite->SafeDestroy();
		}
	}

	for (int ShellIndex = 0 ; ShellIndex < SectorShells.Num(); ShellIndex++)
	{
		if (SectorShells[ShellIndex])
		{
			SectorShells[ShellIndex]->SafeDestroy();
		}
	}

	UniqueCompanies.Empty();
	SectorSpacecrafts.Empty();
	SectorShips.Empty();
	SectorStations.Empty();
	SectorBombs.Empty();
	SectorAsteroids.Empty();
	SectorMeteorites.Empty();
	SectorShells.Empty();
	CompanyShipsPerCompanyCache.Empty();
	CompanySpacecraftsPerCompanyCache.Empty();
	SectorSpacecraftsCache.Empty();
}

/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

AFlareAsteroid* UFlareSector::LoadAsteroid(const FFlareAsteroidSave& AsteroidData)
{
	FActorSpawnParameters Params;
	Params.bNoFail = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AFlareAsteroid* Asteroid = GetGame()->GetCacheSystem()->RetrieveCachedAsteroid();
	if (IsValid(Asteroid))
	{
		//retrieved an asteroid
		Asteroid->UnSafeDestroy();
		Asteroid->SetActorLocationAndRotation(AsteroidData.Location, AsteroidData.Rotation);
	}
	else
	{
		Asteroid = GetGame()->GetWorld()->SpawnActor<AFlareAsteroid>(AFlareAsteroid::StaticClass(), AsteroidData.Location, AsteroidData.Rotation, Params);
	} 
	Asteroid->Load(AsteroidData);
	SectorAsteroids.AddUnique(Asteroid);
	return Asteroid;
}

AFlareMeteorite* UFlareSector::LoadMeteorite(FFlareMeteoriteSave& MeteoriteData)
{
	AFlareMeteorite* Meteorite = GetGame()->GetCacheSystem()->RetrieveCachedMeteorite();
	if (IsValid(Meteorite))
	{
		Meteorite->UnSafeDestroy();
		Meteorite->SetActorLocationAndRotation(MeteoriteData.Location, MeteoriteData.Rotation);
	}
	else
	{
		FActorSpawnParameters Params;
		Params.bNoFail = true;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		Meteorite = GetGame()->GetWorld()->SpawnActor<AFlareMeteorite>(AFlareMeteorite::StaticClass(), MeteoriteData.Location, MeteoriteData.Rotation, Params);
	}
	Meteorite->Load(&MeteoriteData, this);
	SectorMeteorites.AddUnique(Meteorite);
	return Meteorite;
}

void UFlareSector::RemoveSectorMeteorite(AFlareMeteorite* Meteorite)
{
	SectorMeteorites.RemoveSwap(Meteorite);
}

AFlareSpacecraft* UFlareSector::LoadSpacecraft(UFlareSimulatedSpacecraft* ParentSpacecraft,bool Reposition)
{
	FCHECK(ParentSpacecraft);
	FCHECK(ParentSpacecraft);
	FCHECK(ParentSpacecraft->GetDescription());
	FCHECK(ParentSpacecraft->GetDescription()->SpacecraftTemplate);

	/*FLOGV("UFlareSector::LoadSpacecraft : Start loading '%s' (template '%s' at %x)",
		*ParentSpacecraft->GetImmatriculation().ToString(),
		*ParentSpacecraft->GetDescription()->SpacecraftTemplate->GetName(),
		ParentSpacecraft->GetDescription()->SpacecraftTemplate);
	*/

	bool ReusedSpacecraft = false;
	// Create and configure the ship
	AFlareSpacecraft* Spacecraft = ParentSpacecraft->GetActive();
	if (!Spacecraft)
	{
		Spacecraft = GetGame()->GetCacheSystem()->RetrieveCachedSpacecraft(ParentSpacecraft->GetDescription()->SpacecraftTemplate);
		if (!Spacecraft)
		{
			// Spawn parameters
			FActorSpawnParameters Params;
			Params.bNoFail = true;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			Spacecraft = GetGame()->GetWorld()->SpawnActor<AFlareSpacecraft>(ParentSpacecraft->GetDescription()->SpacecraftTemplate,
			ParentSpacecraft->GetData().Location,
			ParentSpacecraft->GetData().Rotation,
			Params);
		}
		else
		{
			ReusedSpacecraft = true;
		}
	}
	else
	{
		ReusedSpacecraft = true;
	}

	if (Spacecraft)
	{
		Spacecraft->Load(ParentSpacecraft);

		if (Spacecraft->IsStation())
		{
			SectorStations.Add(Spacecraft);
		}
		else
		{
			SectorShips.Add(Spacecraft);
		}

		if (UniqueCompanies.AddUnique(Spacecraft->GetCompany()) != INDEX_NONE)
		{
//AddUnique returns -1, AKA Index_None if it was already in the array. Or the new index position if the element was added to the array
			GetSimulatedSector()->UpdateSectorBattleState(Spacecraft->GetCompany());
		}

		SectorSpacecrafts.Add(Spacecraft);
		SectorSpacecraftsCache.Add(Spacecraft->GetImmatriculation(), Spacecraft);

		if (CompanySpacecraftsPerCompanyCache.Contains(ParentSpacecraft->GetCompany()))
		{
			CompanySpacecraftsPerCompanyCache[ParentSpacecraft->GetCompany()].Add(Spacecraft);
		}
		if (CompanyShipsPerCompanyCache.Contains(ParentSpacecraft->GetCompany()))
		{
			CompanyShipsPerCompanyCache[ParentSpacecraft->GetCompany()].Add(Spacecraft);
		}

		if (Reposition)
		{
			SetSpacecraftSpawnPosition(ParentSpacecraft, Spacecraft);
		}

		Spacecraft->UnSafeDestroy();

	}
	else
	{
		FLOG("UFlareSector::LoadSpacecraft : failed to create AFlareSpacecraft");
	}

	if (ParentSpacecraft->IsComplex())
	{
		for(UFlareSimulatedSpacecraft* Child: ParentSpacecraft->GetComplexChildren())
		{
			LoadSpacecraft(Child,true);
		}
	}

	return Spacecraft;
}

void UFlareSector::SetSpacecraftSpawnPosition(UFlareSimulatedSpacecraft* ParentSpacecraft, AFlareSpacecraft* Spacecraft)
{
	if (!ParentSpacecraft || !Spacecraft)
	{
		return;
	}

	UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(Spacecraft->GetRootComponent());
	switch (ParentSpacecraft->GetData().SpawnMode)
	{
		// Already known to be correct
		case EFlareSpawnMode::Safe:
		{
#ifdef DEBUG_SPACESHIPPLACEMENT
			FLOGV("UFlareSector::SetSpacecraftSpawnPosition : Safe spawn '%s' at (%f,%f,%f)",
				*ParentSpacecraft->GetImmatriculation().ToString(),
				ParentSpacecraft->GetData().Location.X, ParentSpacecraft->GetData().Location.Y, ParentSpacecraft->GetData().Location.Z);
#endif
			if (!Spacecraft->IsStation() && !Spacecraft->GetNavigationSystem()->IsDocked())
			{
#ifdef DEBUG_SPACESHIPPLACEMENT
				FLOGV("UFlareSector::SetSpacecraftSpawnPosition : Safe spawn '%s' safety movement",
				*ParentSpacecraft->GetImmatriculation().ToString());
#endif
				PlaceSpacecraft(Spacecraft, ParentSpacecraft->GetData().Location, ParentSpacecraft->GetData().Rotation, 1500, true, 0,false);
			}
			else
			{
#ifdef DEBUG_SPACESHIPPLACEMENT
				FLOGV("UFlareSector::SetSpacecraftSpawnPosition : Safe spawn '%s' safety movement failure. Station %d, Docked %d",
					*ParentSpacecraft->GetImmatriculation().ToString(),
					Spacecraft->IsStation(),
					Spacecraft->GetNavigationSystem()->IsDocked());
#endif
			}

			RootComponent->SetPhysicsLinearVelocity(ParentSpacecraft->GetData().LinearVelocity, false);
			RootComponent->SetPhysicsAngularVelocityInDegrees(ParentSpacecraft->GetData().AngularVelocity, false);
			break;

			// First spawn
		}
		case EFlareSpawnMode::Spawn:
		{
			PlaceSpacecraft(Spacecraft, ParentSpacecraft->GetData().Location, ParentSpacecraft->GetData().Rotation);

#ifdef DEBUG_SPACESHIPPLACEMENT
			FVector NewLocation = Spacecraft->GetActorLocation();
			FLOGV("UFlareSector::SetSpacecraftSpawnPosition : Placing '%s' at (%f,%f,%f)",
				*ParentSpacecraft->GetImmatriculation().ToString(),
				NewLocation.X, NewLocation.Y, NewLocation.Z);
#endif

			RootComponent->SetPhysicsLinearVelocity(FVector::ZeroVector, false);
			RootComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false);
			break;
		}
		// Incoming in sector
		case EFlareSpawnMode::Travel:
		{

#ifdef DEBUG_SPACESHIPPLACEMENT
			FLOGV("UFlareSector::SetSpacecraftSpawnPosition : Travel '%s' at (%f, %f, %f)",
				*ParentSpacecraft->GetImmatriculation().ToString(),
				ParentSpacecraft->GetData().Location.X, ParentSpacecraft->GetData().Location.Y, ParentSpacecraft->GetData().Location.Z);
#endif
			FVector SpawnDirection;
			TArray<AFlareSpacecraft*> FriendlySpacecrafts = GetCompanyShips(Spacecraft->GetCompany());
			FVector FriendlyShipLocationSum = FVector::ZeroVector;
			int FriendlyShipCount = 0;

			for (int SpacecraftIndex = 0; SpacecraftIndex < FriendlySpacecrafts.Num(); SpacecraftIndex++)
			{
				AFlareSpacecraft *SpacecraftCandidate = FriendlySpacecrafts[SpacecraftIndex];
				if (SpacecraftCandidate != Spacecraft)
				{
					FriendlyShipLocationSum += SpacecraftCandidate->GetActorLocation();
					FriendlyShipCount++;
				}
			}

			if (FriendlyShipCount == 0)
			{
				FVector NotFriendlyShipLocationSum = FVector::ZeroVector;
				int NotFriendlyShipCount = 0;
				for (int SpacecraftIndex = 0; SpacecraftIndex < SectorShips.Num(); SpacecraftIndex++)
				{
					AFlareSpacecraft *SpacecraftCandidate = SectorShips[SpacecraftIndex];
					if (SpacecraftCandidate != Spacecraft && SpacecraftCandidate->GetCompany() != Spacecraft->GetCompany())
					{
						NotFriendlyShipLocationSum += SpacecraftCandidate->GetActorLocation();
						NotFriendlyShipCount++;
					}
				}

				if (NotFriendlyShipCount == 0)
				{
					SpawnDirection = FMath::VRand();
				}
				else
				{
					FVector	NotFriendlyShipLocationMean = NotFriendlyShipLocationSum / NotFriendlyShipCount;
					SpawnDirection = (GetSectorCenter() - NotFriendlyShipLocationMean).GetUnsafeNormal();
				}
			}
			else
			{
				FVector	FriendlyShipLocationMean = FriendlyShipLocationSum / FriendlyShipCount;
				SpawnDirection = (FriendlyShipLocationMean - GetSectorCenter()).GetUnsafeNormal();
			}

			float SpawnDistance = GetSectorRadius() + 1;

			if (GetSimulatedSector()->GetSectorBattleState(Spacecraft->GetCompany()).InBattle)
			{
				SpawnDistance += 500000; // 5 km
			}

			SpawnDistance = FMath::Min(SpawnDistance, GetSectorLimits());

			FVector Location = GetSectorCenter() + SpawnDirection * SpawnDistance;

			FVector CenterDirection = (GetSectorCenter() - Location).GetUnsafeNormal();
			PlaceSpacecraft(Spacecraft, Location, CenterDirection.Rotation());

			float SpawnVelocity = 0;

			if (GetSimulatedSector()->GetSectorBattleState(Spacecraft->GetCompany()).InBattle)
			{
				SpawnVelocity = 10000;
			}

			RootComponent->SetPhysicsLinearVelocity(CenterDirection * SpawnVelocity, false);
			RootComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false);
		}
		break;
		case EFlareSpawnMode::Exit:
		{
			float SpawnDistance = GetSectorLimits() * 0.85;
			float SpawnVelocity = ParentSpacecraft->GetData().LinearVelocity.Size() * 0.6;
			FVector SpawnDirection = ParentSpacecraft->GetData().Location.GetUnsafeNormal();
			FVector Location = SpawnDirection * SpawnDistance;
			FVector CenterDirection = (GetSectorCenter() - Location).GetUnsafeNormal();

#ifdef DEBUG_SPACESHIPPLACEMENT
			FLOGV("UFlareSector::SetSpacecraftSpawnPosition : Exit '%s' at (%f, %f, %f)",
				*ParentSpacecraft->GetImmatriculation().ToString(),
				Location.X, Location.Y, Location.Z);
#endif

			PlaceSpacecraft(Spacecraft, Location, CenterDirection.Rotation());

			RootComponent->SetPhysicsLinearVelocity(CenterDirection * SpawnVelocity, false);
			RootComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false);
		}
		break;
		case EFlareSpawnMode::InternalDocked:
		{
			if (ParentSpacecraft->GetShipMaster())
			{		
				int ShipPosition = 0;
				for (UFlareSimulatedSpacecraft* OwnedShips : ParentSpacecraft->GetShipMaster()->GetShipChildren())
				{
					ShipPosition++;
					if (OwnedShips == ParentSpacecraft)
					{
						break;
					}
				}

				bool OddEven;
				if (ShipPosition % 2)
				{
					OddEven = true;
				}
				else
				{
					OddEven = false;
				}		
			
				PlaceSpacecraftDrone(Spacecraft, ParentSpacecraft->GetShipMaster()->GetActive()->GetActorLocation(), ParentSpacecraft->GetShipMaster()->GetActive()->GetActorRotation(), 700, ParentSpacecraft->GetShipMaster()->GetActive()->GetMeshScale() + 200, OddEven);

				float SpawnVelocity = ParentSpacecraft->GetShipMaster()->GetActive()->GetLinearVelocity().Size() * 0.6;
				FVector CenterDirection = (GetSectorCenter() - ParentSpacecraft->GetShipMaster()->GetActive()->GetActorLocation()).GetUnsafeNormal();
				RootComponent->SetPhysicsLinearVelocity(CenterDirection * SpawnVelocity, false);
				RootComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false);
			}
			else
			{
				PlaceSpacecraft(Spacecraft, ParentSpacecraft->GetData().Location, ParentSpacecraft->GetData().Rotation);
			}
			break;
		}
	}

	ParentSpacecraft->SetInternalDockedTo(NULL);
	if (ParentSpacecraft->GetData().SpawnMode == EFlareSpawnMode::Travel && GetSimulatedSector()->IsTravelSector())
	{
		FLOG("UFlareSector::SetSpacecraftSpawnPosition : ship is still traveling");
	}
	else
	{
		ParentSpacecraft->SetSpawnMode(EFlareSpawnMode::Safe);
	}
}

void UFlareSector::FinishLoadSpacecraftReattachRedock(AFlareSpacecraft* Spacecraft)
{
	Spacecraft->ReattachAndRedock();
}

void UFlareSector::FinishLoadSpacecraft(AFlareSpacecraft* Spacecraft, bool Reposition)
{
	if (Reposition && !Spacecraft->GetNavigationSystem()->IsDocked() && !Spacecraft->IsStation())
	{
		SetSpacecraftSpawnPosition(Spacecraft->GetParent(), Spacecraft);
	}

	Spacecraft->FinishLoadandReady();
}

AFlareBomb* UFlareSector::LoadBomb(const FFlareBombSave& BombData)
{
	AFlareBomb* Bomb = NULL;
	FLOG("UFlareSector::LoadBomb");

	AFlareSpacecraft* ParentSpacecraft = NULL;

	for (int i = 0 ; i < SectorShips.Num(); i++)
	{
		AFlareSpacecraft* SpacecraftCandidate = SectorShips[i];
		if (SpacecraftCandidate->GetImmatriculation() == BombData.ParentSpacecraft)
		{
			ParentSpacecraft = SpacecraftCandidate;
			break;
		}
	}

	if (ParentSpacecraft)
	{
		UFlareWeapon* ParentWeapon = NULL;
		TArray<UActorComponent*> Components = ParentSpacecraft->GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
		for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
		{
			UFlareWeapon* WeaponCandidate = Cast<UFlareWeapon>(Components[ComponentIndex]);
			if (WeaponCandidate && WeaponCandidate->SlotIdentifier == BombData.WeaponSlotIdentifier)
			{

				ParentWeapon = WeaponCandidate;
				break;
			}
		}

		if (ParentWeapon)
		{
			// Create and configure the ship
			Bomb = GetGame()->GetCacheSystem()->RetrieveCachedBomb();
			if (IsValid(Bomb))
			{
				//retrieved a bomb
				Bomb->SetActorLocationAndRotation(BombData.Location, BombData.Rotation);
			}
			else
			{
				// Spawn parameters
				FActorSpawnParameters Params;
				Params.bNoFail = true;
				Bomb = GetGame()->GetWorld()->SpawnActor<AFlareBomb>(AFlareBomb::StaticClass(), BombData.Location, BombData.Rotation, Params);
			}
			
			if (Bomb)
			{
				Bomb->Initialize(&BombData, ParentWeapon);
				UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(Bomb->GetRootComponent());
				RootComponent->SetPhysicsLinearVelocity(BombData.LinearVelocity, false);
				RootComponent->SetPhysicsAngularVelocityInDegrees(BombData.AngularVelocity, false);
				SectorBombs.Add(Bomb);
			}
			else
			{
				FLOG("UFlareSector::LoadBomb fail to create AFlareBomb");
			}
		}
		else
		{
			FLOG("UFlareSector::LoadBomb failed (no parent weapon)");
		}
	}
	else
	{
		FLOG("UFlareSector::LoadBomb failed (no parent ship)");
	}

	return Bomb;
}

void UFlareSector::RegisterBomb(AFlareBomb* Bomb)
{
	SectorBombs.AddUnique(Bomb);
}

void UFlareSector::UnregisterBomb(AFlareBomb* Bomb)
{
	if (SectorBombs.RemoveSwap(Bomb))
	{
		//todo: let bomb know what's targetting it for smaller checks
		for (AFlareSpacecraft* Spacecraft : SectorSpacecrafts)
		{
			Spacecraft->ClearInvalidTarget(PilotHelper::PilotTarget(Bomb));
		}
	}
}

void UFlareSector::RegisterShell(AFlareShell* Shell)
{
	SectorShells.AddUnique(Shell);
}

void UFlareSector::UnregisterShell(AFlareShell* Shell)
{
	if (!IsDestroyingSector)
	{
		SectorShells.RemoveSwap(Shell);
	}
}

void UFlareSector::SetPause(bool Pause)
{
	for (int i = 0 ; i < SectorSpacecrafts.Num(); i++)
	{
		SectorSpacecrafts[i]->SetPause(Pause);
	}

	for (int i = 0 ; i < SectorBombs.Num(); i++)
	{
		SectorBombs[i]->SetPause(Pause);
	}
	
	for (int i = 0 ; i < SectorAsteroids.Num(); i++)
	{
		SectorAsteroids[i]->SetPause(Pause);
	}

	for (AFlareMeteorite* Meteorite : SectorMeteorites)
	{
		if (IsValid(Meteorite))
		{
			Meteorite->SetPause(Pause);
		}
	}

	for (int i = 0 ; i < SectorShells.Num(); i++)
	{
		SectorShells[i]->SetPause(Pause);
	}
}

AActor* UFlareSector::GetNearestBody(FVector Location, float* NearestDistance, bool IncludeSize, AActor* ActorToIgnore)
{
	AActor* NearestCandidateActor = NULL;
	float NearestCandidateActorDistance = 0;

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < SectorSpacecrafts.Num(); SpacecraftIndex++)
	{
		AFlareSpacecraft* SpacecraftCandidate = GetSpacecrafts()[SpacecraftIndex];
		float Distance = FVector::Dist(SpacecraftCandidate->GetActorLocation(), Location) - SpacecraftCandidate->GetMeshScale();
		if (SpacecraftCandidate != ActorToIgnore && (!NearestCandidateActor || NearestCandidateActorDistance > Distance))
		{
			NearestCandidateActor = SpacecraftCandidate;
			NearestCandidateActorDistance = Distance;
		}
	}

	for (int32 AsteroidIndex = 0; AsteroidIndex < GetAsteroids().Num(); AsteroidIndex++)
	{
		AFlareAsteroid* AsteroidCandidate = GetAsteroids()[AsteroidIndex];

		FBox CandidateBox = AsteroidCandidate->GetComponentsBoundingBox();
		float CandidateSize = FMath::Max(CandidateBox.GetExtent().Size(), 1.0f);

		float Distance = FVector::Dist(AsteroidCandidate->GetActorLocation(), Location) - CandidateSize;
		if (AsteroidCandidate != ActorToIgnore && (!NearestCandidateActor || NearestCandidateActorDistance > Distance))
		{
			NearestCandidateActor = AsteroidCandidate;
			NearestCandidateActorDistance = Distance;
		}
	}

	TArray<AActor*> ColliderActorList;
	UGameplayStatics::GetAllActorsOfClass(GetGame()->GetWorld(), AFlareCollider::StaticClass(), ColliderActorList);
	for (int32 ColliderIndex = 0; ColliderIndex < ColliderActorList.Num(); ColliderIndex++)
	{
		AActor* ColliderCandidate = ColliderActorList[ColliderIndex];

		float CandidateSize = Cast<UStaticMeshComponent>(ColliderCandidate->GetRootComponent())->Bounds.SphereRadius;
		float Distance = FVector::Dist(ColliderCandidate->GetActorLocation(), Location) - CandidateSize;
		if (ColliderCandidate != ActorToIgnore && (!NearestCandidateActor || NearestCandidateActorDistance > Distance))
		{
			NearestCandidateActor = ColliderCandidate;
			NearestCandidateActorDistance = Distance;
		}
	}

	*NearestDistance = NearestCandidateActorDistance;
	return NearestCandidateActor;
}

void UFlareSector::PlaceSpacecraftDrone(AFlareSpacecraft* Spacecraft, FVector Location, FRotator Rotation, float RandomLocationRadiusIncrement, float InitialLocationRadius, bool PositiveOrNegative)
{
	float RandomLocationRadius = InitialLocationRadius;
	float EffectiveDistance = -1;
	float Size = Spacecraft->GetMeshScale() * 4;
	int Loops = 0;

	do
	{
		float HalfRadius = RandomLocationRadius / 2;
		if (PositiveOrNegative)
		{
			Location.Y += RandomLocationRadius;
		}
		else
		{
			Location.Y -= RandomLocationRadius;
		}

		if (FMath::RandBool())
		{
			Location.X -= HalfRadius;
		}
		else
		{
			Location.X += HalfRadius;
		}

		if (FMath::RandBool())
		{
			Location.Z -= HalfRadius;
		}
		else
		{
			Location.Z += HalfRadius;
		}

		float NearestDistance;
		// Check if location is secure
		if (GetNearestBody(Location, &NearestDistance, true, Spacecraft) == NULL)
		{
			// No other ship.
			break;
		}

		EffectiveDistance = NearestDistance - Size;
		if (Loops == 0)
		{
			RandomLocationRadius = RandomLocationRadiusIncrement;
		}
		Loops++;
	} while (EffectiveDistance <= 0 && RandomLocationRadius < RandomLocationRadiusIncrement * 1000);

	Spacecraft->SetActorLocationAndRotation(Location, Rotation);
}


void UFlareSector::PlaceSpacecraft(AFlareSpacecraft* Spacecraft, FVector Location, FRotator Rotation, float RandomLocationRadiusIncrement, bool RandomLocRadiusBoost, float InitialLocationRadius, bool MultiplyLocationOrAdd, bool PositiveOrNegative)
{
	float RandomLocationRadius = InitialLocationRadius;
	float EffectiveDistance = -1;
	float Size = (Spacecraft->IsStation() ? 85000 : Spacecraft->GetMeshScale());
	int Loops = 0;

	do 
	{
		if (PositiveOrNegative)
		{
			if (MultiplyLocationOrAdd)
			{
				Location += FMath::VRand() * RandomLocationRadius;
			}
			else
			{
				Location += FMath::VRand() + RandomLocationRadius;
			}
		}
		else
		{
			if (MultiplyLocationOrAdd)
			{
				Location -= FMath::VRand() * RandomLocationRadius;
			}
			else
			{
				Location -= FMath::VRand() + RandomLocationRadius;
			}
		}

		float NearestDistance;
		// Check if location is secure
		if (GetNearestBody(Location, &NearestDistance, true, Spacecraft) == NULL)
		{
			// No other ship.
			break;
		}

		EffectiveDistance = NearestDistance - Size;
		if (Loops == 0)
		{
			RandomLocationRadius = RandomLocationRadiusIncrement;
		}
		else if (RandomLocRadiusBoost)
		{
			RandomLocationRadius += RandomLocationRadiusIncrement;
		}
		Loops++;
	}
	while (EffectiveDistance <= 0 && RandomLocationRadius < RandomLocationRadiusIncrement * 1000);

#if !UE_BUILD_SHIPPING
	{
		TArray<AActor*> ColliderActorList;
		UGameplayStatics::GetAllActorsOfClass(GetGame()->GetWorld(), AFlareCollider::StaticClass(), ColliderActorList);
		for (int32 ColliderIndex = 0; ColliderIndex < ColliderActorList.Num(); ColliderIndex++)
		{
			AActor* ColliderCandidate = ColliderActorList[ColliderIndex];

			float CandidateSize = Cast<UStaticMeshComponent>(ColliderCandidate->GetRootComponent())->Bounds.SphereRadius;
			float SpacecraftSize = Spacecraft->GetSimpleCollisionRadius();
			float Distance = FVector::Dist(ColliderCandidate->GetActorLocation(), Location);
			
			if (Distance < CandidateSize + SpacecraftSize)
			{
				FLOGV("UFlareSector::PlaceSpacecraft : %s was placed inside collider '%s'", *Spacecraft->GetImmatriculation().ToString(), *ColliderCandidate->GetName());
			}
		}
	}
#endif
	Spacecraft->SetActorLocationAndRotation(Location,Rotation);
}

/*----------------------------------------------------
	Getters
----------------------------------------------------*/

TArray<AFlareSpacecraft*> UFlareSector::GetCompanyShips(UFlareCompany* Company)
{
	TArray<AFlareSpacecraft*> CompanyShips;
	
	if (CompanyShipsPerCompanyCache.Contains(Company))
	{
		CompanyShips = CompanyShipsPerCompanyCache[Company];
	}

	else
	{
		for (int i = 0; i < SectorShips.Num(); i++)
		{
			if (SectorShips[i]->GetCompany() == Company)
			{
				CompanyShips.Add(SectorShips[i]);
			}
		}
		CompanyShipsPerCompanyCache.Add(Company, CompanyShips);
	}
	return CompanyShips;
}

bool UFlareSector::GetIsDestroyingSector()
{
	return IsDestroyingSector;
}

TArray<AFlareSpacecraft*> UFlareSector::GetCompanySpacecrafts(UFlareCompany* Company)
{
	TArray<AFlareSpacecraft*> CompanySpacecrafts;

	if (CompanySpacecraftsPerCompanyCache.Contains(Company))
	{
		CompanySpacecrafts = CompanySpacecraftsPerCompanyCache[Company];
	}

	else
	{
		for (int i = 0; i < SectorSpacecrafts.Num(); i++)
		{
			if (SectorSpacecrafts[i]->GetCompany() == Company)
			{
				CompanySpacecrafts.Add(SectorSpacecrafts[i]);
			}
		}
		CompanySpacecraftsPerCompanyCache.Add(Company, CompanySpacecrafts);
	}
	return CompanySpacecrafts;
}

AFlareSpacecraft* UFlareSector::FindSpacecraft(FName Immatriculation)
{
	if (SectorSpacecraftsCache.Contains(Immatriculation))
	{
		return SectorSpacecraftsCache[Immatriculation];
	}
	return NULL;
}


void UFlareSector::GenerateSectorRepartitionCache()
{
	if (!SectorRepartitionCache)
	{
		SectorRepartitionCache = true;
		SectorRadius = 0.0f;
		SectorCenter = FVector::ZeroVector;

		int SignificantObjectCount = 0;

		FVector SectorMin = FVector(INFINITY, INFINITY, INFINITY);
		FVector SectorMax = FVector(-INFINITY, -INFINITY, -INFINITY);

		for (int SpacecraftIndex = 0 ; SpacecraftIndex < SectorStations.Num(); SpacecraftIndex++)
		{
			AFlareSpacecraft *Spacecraft = SectorStations[SpacecraftIndex];
			SectorMin = SectorMin.ComponentMin(Spacecraft->GetActorLocation());
			SectorMax = SectorMax.ComponentMax(Spacecraft->GetActorLocation());
			SignificantObjectCount++;
		}

		if (SignificantObjectCount > 0)
		{
			// At least one station or asteroid in sector
			FVector BoxSize = SectorMax - SectorMin;
			SectorRadius = BoxSize.Size() / 2;
			SectorCenter = (SectorMax + SectorMin) / 2;
		}
	}
}

FVector UFlareSector::GetSectorCenter()
{
	GenerateSectorRepartitionCache();
	return SectorCenter;
}

float UFlareSector::GetSectorRadius()
{
	GenerateSectorRepartitionCache();
	return SectorRadius;
}

float UFlareSector::GetSectorLimits()
{
	if (ParentSector)
	{
		return ParentSector->GetSectorLimits();
	}
	return 2000000; // 20 km
}

#undef LOCTEXT_NAMESPACE