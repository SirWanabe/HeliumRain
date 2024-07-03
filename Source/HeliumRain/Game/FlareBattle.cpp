

#include "FlareBattle.h"
#include "../Flare.h"
#include "FlareWorld.h"
#include "FlareGame.h"
#include "FlareGameTools.h"

#include "../Data/FlareSpacecraftComponentsCatalog.h"

#include "../Player/FlarePlayerController.h"

#define MAXIMUM_TURNS_METEORITES 25
#define MAXIMUM_TURNS_SHIPBATTLE 1000

struct BattleTargetPreferences
{
        float IsLarge;
        float IsSmall;
        float IsStation;
        float IsNotStation;
        float IsMilitary;
        float IsNotMilitary;
        float IsDangerous;
        float IsNotDangerous;
        float IsStranded;
        float IsNotStranded;
		float IsUncontrollableCivil;
		float IsUncontrollableSmallMilitary;
		float IsUncontrollableLargeMilitary;
		float IsNotUncontrollable;
        float IsHarpooned;
        float TargetStateWeight;
};

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareBattle::UFlareBattle(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareBattle::Load(UFlareSimulatedSector* BattleSector)
{
    Game = Cast<UFlareWorld>(GetOuter())->GetGame();
    Sector = BattleSector;
    PlayerCompany = Game->GetPC()->GetCompany();
	Catalog = Game->GetShipPartsCatalog();
	FightingCompanies.Empty();
	InitialSectorViableFightingShips.Empty();
	CurrentSectorViableFightingShips.Empty();
	LocalMeteorites.Empty();
	CurrentBattleTurn = 0;
	SwitchedToFightingMeteorites = false;
	PlayerAssetsFighting = false;
	FoundFightingCompanies = false;
	ShipDisabledPreviousTurn = false;
	MaximumTurns = MAXIMUM_TURNS_SHIPBATTLE;

	for (FFlareMeteoriteSave& Meteorite : Sector->GetData()->MeteoriteData)
	{
		if (Meteorite.DaysBeforeImpact == 0)
		{
			LocalMeteorites.Add(&Meteorite);
		}
	}

	FindFightingCompanies();
	for (int32 ShipIndex = 0; ShipIndex < Sector->GetSectorCombatCapableShips().Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = Sector->GetSectorCombatCapableShips()[ShipIndex];
		if (Ship->IsReserve())
		{
			// No in fight
			continue;
		}

		if (!Ship->IsMilitaryArmed() || Ship->GetDamageSystem()->IsDisarmed())
		{
			// No weapon
			continue;
		}

		if (!FightingCompanies.Contains(Ship->GetCompany()))
		{
			// Not in war
			continue;
		}

		InitialSectorViableFightingShips.Add(Ship);
		CurrentSectorViableFightingShips.Add(Ship);
	}
}

/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/


void UFlareBattle::Simulate()
{
    FLOGV("Simulate battle in %s", *Sector->GetSectorName().ToString());
	CombatLog::AutomaticBattleStarted(Sector);

	while (HasBattle())
    {
		CurrentBattleTurn++;
		if(!SimulateTurn())
        {
            FLOG("Nobody can fight, end battle");
            break;
        }
        if (CurrentBattleTurn > MaximumTurns)
        {
			FLOGV("ERROR: Battle too long, still not ended after %d turns", MaximumTurns);
            break;
        }
    }

	CombatLog::AutomaticBattleEnded(Sector);
    FLOGV("Battle in %s finish after %d turns", *Sector->GetSectorName().ToString(), CurrentBattleTurn);
}

bool UFlareBattle::HasBattle()
{
    // Check if battle
    bool HasBattle = false;

	FindFightingCompanies();
	if (FightingCompanies.Num() > 0)
	{
		return true;
	}
	return false;
}

void UFlareBattle::FindFightingCompanies()
{
	FightingCompanies.Empty();

	FoundFightingCompanies = false;
	for (int CompanyIndex = 0; CompanyIndex < Game->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
		UFlareCompany* Company = Game->GetGameWorld()->GetCompanies()[CompanyIndex];
		if (Company == PlayerCompany)
		{
/*
			if (Sector == GetGame()->GetPC()->GetPlayerShip()->GetCurrentSector())
			{
				// Local sector, don't check if the player want fight
				continue;
			}
*/
		}

		FFlareSectorBattleState BattleState;
		if (ShipDisabledPreviousTurn)
		{
			BattleState = Sector->UpdateSectorBattleState(Company);
		}
		else
		{
			BattleState = Sector->GetSectorBattleState(Company);
		}

		if (!BattleState.WantFight())
		{
			// Don't want to fight other companies
			if (!LocalMeteorites.Num())
			{
				continue;
			}
		}
		else
		{
			FoundFightingCompanies = true;
		}

		FightingCompanies.Add(Company);
	}

	ShipDisabledPreviousTurn = false;
}

bool UFlareBattle::SimulateTurn()
{
	bool HasFight = false;
	PlayerAssetsFighting = false;

	// List all fighting ships
    TArray<UFlareSimulatedSpacecraft*> ShipToSimulate;
	for (int32 ShipIndex = 0 ; ShipIndex < CurrentSectorViableFightingShips.Num(); ShipIndex++)
	{
        UFlareSimulatedSpacecraft* Ship = CurrentSectorViableFightingShips[ShipIndex];
		if(Ship->IsReserve() || Ship->GetDamageSystem()->IsDisarmed())
		{
			// Not actively participating in fight
			CurrentSectorViableFightingShips.Remove(Ship);
			ShipIndex--;
			ShipDisabledPreviousTurn = true;
			continue;
		}

		if (Ship->GetCompany() == PlayerCompany)
		{
			PlayerAssetsFighting = true;
		}
		
		ShipToSimulate.Add(Ship);
    }

    // Play fighting ship in random order
    while(ShipToSimulate.Num())
    {
        int32 Index = FMath::RandRange(0, ShipToSimulate.Num() - 1);
        if(SimulateShipTurn(ShipToSimulate[Index]))
        {
            HasFight = true;
        }
       ShipToSimulate.RemoveAtSwap(Index);
	}

	for (UFlareSimulatedSpacecraft* Ship : InitialSectorViableFightingShips)
	{
		Ship->GetDamageSystem()->NotifyDamage();
	}

    return HasFight;
}

bool UFlareBattle::SimulateShipTurn(UFlareSimulatedSpacecraft* Ship)
{
    if(Ship->GetSize() == EFlarePartSize::S)
    {
        return SimulateSmallShipTurn(Ship);
    }
    else if(Ship->GetSize() == EFlarePartSize::L)
    {
        return SimulateLargeShipTurn(Ship);
    }
	else
	{
		return SimulateLargeShipTurn(Ship);
	}

    return false;
}

bool UFlareBattle::SimulateSmallShipTurn(UFlareSimulatedSpacecraft* Ship)
{
    //  - Find a target
    //  - Find a weapon
    //  - Apply damage

	UFlareSimulatedSpacecraft* Target = NULL;
	FFlareMeteoriteSave* MeteoriteTarget = NULL;

    struct BattleTargetPreferences TargetPreferences;
    TargetPreferences.IsLarge = 1;
    TargetPreferences.IsSmall = 1;
    TargetPreferences.IsStation = 1;
    TargetPreferences.IsNotStation = 1;
    TargetPreferences.IsMilitary = 1;
    TargetPreferences.IsNotMilitary = 0.1;
    TargetPreferences.IsDangerous = 1;
    TargetPreferences.IsNotDangerous = 0.01;
    TargetPreferences.IsStranded = 1;
    TargetPreferences.IsNotStranded = 0.5;
	TargetPreferences.IsUncontrollableCivil = 0.0;
	TargetPreferences.IsUncontrollableSmallMilitary = 0.;
	TargetPreferences.IsUncontrollableLargeMilitary = 0.;
	TargetPreferences.IsNotUncontrollable = 1;
    TargetPreferences.IsHarpooned = 0;
    TargetPreferences.TargetStateWeight = 1;

	Ship->GetWeaponsSystem()->GetTargetPreference(&TargetPreferences.IsSmall, &TargetPreferences.IsLarge, &TargetPreferences.IsUncontrollableCivil, &TargetPreferences.IsUncontrollableSmallMilitary, &TargetPreferences.IsUncontrollableLargeMilitary, &TargetPreferences.IsNotUncontrollable, &TargetPreferences.IsStation, &TargetPreferences.IsHarpooned);
	
	float MinAmmoRatio = 1.f;

	for (int32 ComponentIndex = 0; ComponentIndex < Ship->GetData().Components.Num(); ComponentIndex++)
	{
		FFlareSpacecraftComponentSave* ComponentData = &Ship->GetData().Components[ComponentIndex];
		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

		if (ComponentDescription->Type == EFlarePartType::Weapon)
		{
			float AmmoRatio = float(ComponentDescription->WeaponCharacteristics.AmmoCapacity - ComponentData->Weapon.FiredAmmo) /  ComponentDescription->WeaponCharacteristics.AmmoCapacity;
			if(AmmoRatio < MinAmmoRatio)
			{
				MinAmmoRatio = AmmoRatio;
			}
		}
	}
	//FLOGV("%s MinAmmoRatio=%f", *Ship->GetImmatriculation().ToString(), MinAmmoRatio);

	if(MinAmmoRatio <0.9)
	{
		TargetPreferences.IsUncontrollableSmallMilitary = 0.0;
	}

	if(MinAmmoRatio < 0.5)
	{
		TargetPreferences.IsNotMilitary = 0.0;
	}

	if (FoundFightingCompanies)
	{
		Target = GetBestTarget(Ship, TargetPreferences);
	}

	if (!Target)
    {
		MeteoriteTarget = GetMeteoriteTarget();
	}

	if (Target || MeteoriteTarget)
	{
		// Find best weapon
		int32 WeaponGroupIndex = Ship->GetWeaponsSystem()->FindBestWeaponGroup(Target,MeteoriteTarget);

		if (WeaponGroupIndex == -1)
		{
			return false;
		}
/*
		FLOGV("%s want to attack %s with %s",
			*Ship->GetImmatriculation().ToString(),
			*Target->GetImmatriculation().ToString(),
			*Ship->GetWeaponsSystem()->GetWeaponGroup(WeaponGroupIndex)->Description->Identifier.ToString())
*/
		return SimulateShipAttack(Ship, WeaponGroupIndex, Target, MeteoriteTarget);
	}
	return false;
}

bool UFlareBattle::SimulateLargeShipTurn(UFlareSimulatedSpacecraft* Ship)
{
	bool HasAttacked = false;

	// Fire each turret individualy
	for (int32 ComponentIndex = 0; ComponentIndex < Ship->GetData().Components.Num(); ComponentIndex++)
	{
		FFlareSpacecraftComponentSave* ComponentData = &Ship->GetData().Components[ComponentIndex];
		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

		if(ComponentDescription->Type != EFlarePartType::Weapon || !ComponentDescription->WeaponCharacteristics.TurretCharacteristics.IsTurret)
		{
			// Ignore if not a turret
			continue;
		}

		if(Ship->GetDamageSystem()->GetUsableRatio(ComponentDescription, ComponentData) <= 0)
		{
			// Not usable
			continue;
		}


		UFlareSimulatedSpacecraft* ShipTarget = NULL;
		FFlareMeteoriteSave* MeteoriteTarget = NULL;

		struct BattleTargetPreferences TargetPreferences;
		TargetPreferences.IsLarge = 1;
		TargetPreferences.IsSmall = 1;
		TargetPreferences.IsStation = 1;
		TargetPreferences.IsNotStation = 1;
		TargetPreferences.IsMilitary = 1;
		TargetPreferences.IsNotMilitary = 0.1;
		TargetPreferences.IsDangerous = 1;
		TargetPreferences.IsNotDangerous = 0.01;
		TargetPreferences.IsStranded = 1;
		TargetPreferences.IsNotStranded = 0.5;
		TargetPreferences.IsUncontrollableCivil = 0.0;
		TargetPreferences.IsUncontrollableSmallMilitary = 0.0;
		TargetPreferences.IsUncontrollableLargeMilitary = 0.0;
		TargetPreferences.IsNotUncontrollable = 1;
		TargetPreferences.IsHarpooned = 0;
		TargetPreferences.TargetStateWeight = 1;

		TargetPreferences.IsLarge = ComponentDescription->WeaponCharacteristics.AntiLargeShipValue;
		TargetPreferences.IsSmall = ComponentDescription->WeaponCharacteristics.AntiSmallShipValue;
		TargetPreferences.IsStation = ComponentDescription->WeaponCharacteristics.AntiStationValue;



		float AmmoRatio = float(ComponentDescription->WeaponCharacteristics.AmmoCapacity - ComponentData->Weapon.FiredAmmo) /  ComponentDescription->WeaponCharacteristics.AmmoCapacity;

		if(AmmoRatio < 0.9)
		{
			TargetPreferences.IsUncontrollableSmallMilitary = 0.0;
		}

		if(AmmoRatio < 0.5)
		{
			TargetPreferences.IsNotMilitary = 0.0;
		}

		if (FoundFightingCompanies)
		{
			ShipTarget = GetBestTarget(Ship, TargetPreferences);
		}

		if (!ShipTarget)
		{
			MeteoriteTarget = GetMeteoriteTarget();
		}
/*
		else
		{
			FLOGV("%s want to attack %s with %s",
			*Ship->GetImmatriculation().ToString(),
			*ShipTarget->GetImmatriculation().ToString(),
			*ComponentData->ShipSlotIdentifier.ToString())
		}
*/
		if (ShipTarget || MeteoriteTarget)
		{
			if (SimulateShipWeaponAttack(Ship, ComponentDescription, ComponentData, ShipTarget, MeteoriteTarget))
			{
				HasAttacked = true;
			}
		}
	}
	return HasAttacked;
}

FFlareMeteoriteSave* UFlareBattle::GetMeteoriteTarget()
{
	if (LocalMeteorites.Num() > 0)
	{
		if (!SwitchedToFightingMeteorites && !FoundFightingCompanies)
		{
			CurrentBattleTurn = 1;
			MaximumTurns = MAXIMUM_TURNS_METEORITES;
			SwitchedToFightingMeteorites = true;
			FLOG("Battle switched to meteorites only");
		}

		for (FFlareMeteoriteSave* Meteorite : LocalMeteorites)
		{
			if (Meteorite->Damage < Meteorite->BrokenDamage)
			{
				return Meteorite;
			}
		}
	}
	return nullptr;
}

UFlareSimulatedSpacecraft* UFlareBattle::GetBestTarget(UFlareSimulatedSpacecraft* Ship, struct BattleTargetPreferences Preferences)
{
	UFlareSimulatedSpacecraft* BestTarget = NULL;
	float BestScore = 0;

//	FLOGV("GetBestTarget for %s", *Ship->GetImmatriculation().ToString());

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* ShipCandidate = Sector->GetSectorSpacecrafts()[SpacecraftIndex];

		if(ShipCandidate->IsReserve())
		{
			// No in fight
			continue;
		}

		if (!ShipCandidate->IsHostile(Ship->GetCompany()))
		{
			// Ignore not hostile ships
			continue;
		}

		if (!ShipCandidate->GetDamageSystem()->IsAlive())
		{
			// Ignore destroyed ships
			continue;
		}

		if (ShipCandidate->IsStation() && ShipCandidate->GetStationEfficiency() <= 0)
		{
			// Ignore damaged stations
			continue;
		}

		if (ShipCandidate->IsStation() && (!ShipCandidate->GetCompany()->IsPlayerCompany() || ShipCandidate->GetCompany()->GetRetaliation() <= 0))
		{
			// Ignore company without retaliation
			continue;
		}

		float Score;
		float StateScore;
		float DistanceScore;

		StateScore = Preferences.TargetStateWeight;

		if (ShipCandidate->GetSize() == EFlarePartSize::L)
		{
			StateScore *= Preferences.IsLarge;
		}
		else if (ShipCandidate->GetSize() == EFlarePartSize::S)
		{
			StateScore *= Preferences.IsSmall;
		}

		if (ShipCandidate->IsStation())
		{
			StateScore *= Preferences.IsStation;
		}
		else
		{
			StateScore *= Preferences.IsNotStation;
		}

		if (ShipCandidate->IsMilitary())
		{
			StateScore *= Preferences.IsMilitary;
		}
		else
		{
			StateScore *= Preferences.IsNotMilitary;
		}

		if(ShipCandidate->IsMilitaryArmed()  && !ShipCandidate->GetDamageSystem()->IsDisarmed())
		{
			StateScore *= Preferences.IsDangerous;
		}
		else
		{
			StateScore *= Preferences.IsNotDangerous;
		}

		if (ShipCandidate->GetDamageSystem()->IsStranded())
		{
			StateScore *= Preferences.IsStranded;
		}
		else
		{
			StateScore *= Preferences.IsNotStranded;
		}

		if (ShipCandidate->GetDamageSystem()->IsUncontrollable() && ShipCandidate->GetDamageSystem()->IsDisarmed())
		{
			if(ShipCandidate->IsMilitary())
			{
				if (ShipCandidate->GetSize() == EFlarePartSize::S)
				{
					StateScore *= Preferences.IsUncontrollableSmallMilitary;
				}
				else
				{
					StateScore *= Preferences.IsUncontrollableLargeMilitary;
				}
			}
			else
			{
				StateScore *= Preferences.IsUncontrollableCivil;
			}
		}
		else
		{
			StateScore *= Preferences.IsNotUncontrollable;
		}

		if(ShipCandidate->IsHarpooned()) {
			if(ShipCandidate->GetDamageSystem()->IsUncontrollable())
			{
				// Never target harponned uncontrollable ships
				continue;
			}
			StateScore *=  Preferences.IsHarpooned;
		}

		DistanceScore = FMath::FRand();

		Score = StateScore * (DistanceScore);

		if (Score > 0)
		{
			if (BestTarget == NULL || Score > BestScore)
			{
				BestTarget = ShipCandidate;
				BestScore = Score;
			}
		}
	}

	return BestTarget;
}

bool UFlareBattle::SimulateShipAttack(UFlareSimulatedSpacecraft* Ship, int32 WeaponGroupIndex, UFlareSimulatedSpacecraft* ShipTarget, FFlareMeteoriteSave* MeteoriteTarget)
{
	FFlareSimulatedWeaponGroup* WeaponGroup = Ship->GetWeaponsSystem()->GetWeaponGroup(WeaponGroupIndex);

	bool HasAttacked = false;

	// TODO configure Fire probability
	float FireProbability = 0.8f;

	if(SwitchedToFightingMeteorites || FMath::FRand() < FireProbability)
	{
		// Fire with all weapon
		for (int32 WeaponIndex = 0; WeaponIndex <  WeaponGroup->Weapons.Num(); WeaponIndex++)
		{
			if(Ship->GetDamageSystem()->GetUsableRatio(WeaponGroup->Description, WeaponGroup->Weapons[WeaponIndex]) <= 0)
			{
				continue;
			}

			if (SimulateShipWeaponAttack(Ship, WeaponGroup->Description, WeaponGroup->Weapons[WeaponIndex], ShipTarget, MeteoriteTarget))
			{
				HasAttacked = true;
			}
		}
	}
	else
	{
		// Not really fire but not because its impossible
		HasAttacked = true;
	}

	return HasAttacked;
}

bool UFlareBattle::SimulateShipWeaponAttack(UFlareSimulatedSpacecraft* Ship, FFlareSpacecraftComponentDescription* WeaponDescription, FFlareSpacecraftComponentSave* Weapon, UFlareSimulatedSpacecraft* ShipTarget, FFlareMeteoriteSave* MeteoriteTarget)
{
	float UsageRatio = Ship->GetDamageSystem()->GetUsableRatio(WeaponDescription, Weapon);
	int32 MaxAmmo = WeaponDescription->WeaponCharacteristics.AmmoCapacity;
	int32 CurrentAmmo = MaxAmmo - Weapon->Weapon.FiredAmmo;

	if(WeaponDescription->WeaponCharacteristics.GunCharacteristics.IsGun)
	{
		// Fire 5 s of ammo with a hit probability of 10% + precision * usage ratio
		float FiringPeriod = 1.f / (WeaponDescription->WeaponCharacteristics.GunCharacteristics.AmmoRate / 60.f);
		float DamageDelay = FMath::Square(1.f- UsageRatio) * 10 * FiringPeriod * FMath::FRandRange(0.f, 1.f);
		float Delay = DamageDelay + FiringPeriod;

		int32 AmmoToFire = FMath::Max(1, (int32) (5.f * (1.f/Delay)));

		AmmoToFire = FMath::Min(CurrentAmmo, AmmoToFire);

		float TargetCoef = 1.1;

		if (ShipTarget)
		{
			if (ShipTarget->GetSize() == EFlarePartSize::S)
			{
				TargetCoef *= 50;
			}

			if (ShipTarget->GetDamageSystem()->IsStranded())
			{
				TargetCoef /= 2;
			}

			if (ShipTarget->GetDamageSystem()->IsUncontrollable())
			{
				TargetCoef /= 10;
			}
		}
		else if (MeteoriteTarget)
		{
			TargetCoef = 0.50f;
		}

		if(WeaponDescription->WeaponCharacteristics.FuzeType == EFlareShellFuzeType::Proximity)
		{
			TargetCoef /= 100;
		}

		float Precision = UsageRatio * FMath::Max(0.01f, 1.f-(WeaponDescription->WeaponCharacteristics.GunCharacteristics.AmmoPrecision * TargetCoef));

//		FLOGV("Fire %d ammo with a hit probability of %f", AmmoToFire, Precision);
		for (int32 BulletIndex = 0; BulletIndex <  AmmoToFire; BulletIndex++)
		{
			if(FMath::FRand() < Precision)
			{
				// Apply bullet damage
				SimulateBulletDamage(WeaponDescription, ShipTarget, MeteoriteTarget, Ship);
			}
		}
		Weapon->Weapon.FiredAmmo += AmmoToFire;
		Ship->GetDamageSystem()->SetAmmoDirty();
	}
	else if(WeaponDescription->WeaponCharacteristics.BombCharacteristics.IsBomb && CurrentAmmo > 0)
	{
		// Drop one bomb with a hit probability of (1 + usable ratio + isUncontrollable)/3

		if (ShipTarget)
		{
			if (FMath::FRand() < (1 + UsageRatio + (ShipTarget->GetDamageSystem()->IsUncontrollable() ? 1.f : 0.f)))
			{
				// Apply bomb damage
				SimulateBombDamage(WeaponDescription, ShipTarget, Ship);
			}
		}
		else if (MeteoriteTarget)
		{
			ApplyMeteoriteDamage(MeteoriteTarget, WeaponDescription->WeaponCharacteristics.ExplosionPower, Ship);
		}

		Weapon->Weapon.FiredAmmo++;
		Ship->GetDamageSystem()->SetAmmoDirty();
	}
	else
	{
		FLOGV("Not supported weapon %s", *WeaponDescription->Identifier.ToString());
		return false;
	}
	return true;
}

void UFlareBattle::SimulateBulletDamage(FFlareSpacecraftComponentDescription* WeaponDescription, UFlareSimulatedSpacecraft* ShipTarget, FFlareMeteoriteSave* MeteoriteTarget, UFlareSimulatedSpacecraft* DamageSource)
{
	if(WeaponDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::ArmorPiercing)
	{
		if (ShipTarget)
		{
			ApplyDamage(ShipTarget, WeaponDescription->WeaponCharacteristics.GunCharacteristics.KineticEnergy, EFlareDamage::DAM_ArmorPiercing, DamageSource);
		}
		else if (MeteoriteTarget)
		{
			ApplyMeteoriteDamage(MeteoriteTarget, WeaponDescription->WeaponCharacteristics.GunCharacteristics.KineticEnergy, DamageSource);
		}
	}
	else if(WeaponDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::HEAT)
	{
		if (ShipTarget)
		{
			ApplyDamage(ShipTarget, WeaponDescription->WeaponCharacteristics.ExplosionPower, EFlareDamage::DAM_HEAT, DamageSource);
		}
		else if (MeteoriteTarget)
		{
			ApplyMeteoriteDamage(MeteoriteTarget, WeaponDescription->WeaponCharacteristics.ExplosionPower, DamageSource);
		}
	}
	else if(WeaponDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::HighExplosive)
	{
		// Generate fragments
		float FragmentHitRatio = FMath::FRandRange(0.01f, 0.1f);
		int32 FragmentCount = WeaponDescription->WeaponCharacteristics.AmmoFragmentCount * FragmentHitRatio;

		for(int FragmentIndex = 0; FragmentIndex < FragmentCount; FragmentIndex++)
		{
			float FragmentPowerEffect = FMath::FRandRange(0.f, 2.f);
			if (ShipTarget)
			{
				ApplyDamage(ShipTarget, FragmentPowerEffect * WeaponDescription->WeaponCharacteristics.ExplosionPower, EFlareDamage::DAM_HighExplosive, DamageSource);
			}
			else if (MeteoriteTarget)
			{
				ApplyMeteoriteDamage(MeteoriteTarget, FragmentPowerEffect * WeaponDescription->WeaponCharacteristics.ExplosionPower, DamageSource);
			}
		}
	}
}

void UFlareBattle::SimulateBombDamage(FFlareSpacecraftComponentDescription* WeaponDescription, UFlareSimulatedSpacecraft* Target, UFlareSimulatedSpacecraft* DamageSource)
{
	// Apply damage
	ApplyDamage(Target, WeaponDescription->WeaponCharacteristics.ExplosionPower,
		SpacecraftHelper::GetWeaponDamageType(WeaponDescription->WeaponCharacteristics.DamageType),
	DamageSource);

	// Ship salvage
	if (!Target->IsStation() && !Target->GetDescription()->IsDroneShip &&
		((WeaponDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::LightSalvage && Target->GetDescription()->Size == EFlarePartSize::S)
	 || (WeaponDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::HeavySalvage && Target->GetDescription()->Size == EFlarePartSize::L)))
	{
		FLOGV("UFlareBattle::SimulateBombDamage : salvaging %s for %s", *Target->GetImmatriculation().ToString(), *DamageSource->GetCompany()->GetCompanyName().ToString());
		Target->SetHarpooned(DamageSource->GetCompany());
	}
}

void UFlareBattle::ApplyMeteoriteDamage(FFlareMeteoriteSave* MeteoriteTarget, float Energy, UFlareSimulatedSpacecraft* DamageSource)
{
	MeteoriteTarget->Damage += Energy;
	if (MeteoriteTarget->Damage >= MeteoriteTarget->BrokenDamage)
	{
		LocalMeteorites.RemoveSwap(MeteoriteTarget);
		if (PlayerAssetsFighting)
		{
			Game->GetQuestManager()->OnEvent(FFlareBundle().PutTag("meteorite-destroyed").PutName("sector", Sector->GetIdentifier()));
		}
	}
}

void UFlareBattle::ApplyDamage(UFlareSimulatedSpacecraft* Target, float Energy, EFlareDamage::Type DamageType, UFlareSimulatedSpacecraft* DamageSource)
{
	// Find a component and apply damages

	int32 ComponentIndex;
	if(DamageType == EFlareDamage::DAM_HighExplosive)
	{
		ComponentIndex = FMath::RandRange(0,  Target->GetData().Components.Num()-1);
	}
	else
	{
		ComponentIndex = GetBestTargetComponent(Target);
	}

	FFlareSpacecraftComponentSave* TargetComponent = &Target->GetData().Components[ComponentIndex];
	FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(TargetComponent->ComponentIdentifier);
	CombatLog::SpacecraftDamaged(Target, Energy, 0, FVector::ZeroVector, DamageType, DamageSource->GetCompany(), "SimulatedBattle");
	Target->GetDamageSystem()->ApplyDamage(ComponentDescription, TargetComponent, Energy, DamageType, DamageSource);
}

int32 UFlareBattle::GetBestTargetComponent(UFlareSimulatedSpacecraft* TargetSpacecraft)
{
	// Is armed, target the gun
	// Else if not stranger target the orbital
	// else target the rsc

	float WeaponWeight = 1;
	float PodWeight = 1;
	float RCSWeight = 1;
	float InternalWeight = 1;

	if (!TargetSpacecraft->GetDamageSystem()->IsDisarmed())
	{
		WeaponWeight = 20;
		PodWeight = 8;
		RCSWeight = 1;
		InternalWeight = 1;
	}
	else if (!TargetSpacecraft->GetDamageSystem()->IsStranded())
	{
		PodWeight = 8;
		RCSWeight = 1;
		InternalWeight = 1;
	}
	else
	{
		RCSWeight = 1;
		InternalWeight = 1;
	}

	TArray<int32> ComponentSelection;
	for (int32 ComponentIndex = 0; ComponentIndex < TargetSpacecraft->GetData().Components.Num(); ComponentIndex++)
	{
		FFlareSpacecraftComponentSave* TargetComponent = &TargetSpacecraft->GetData().Components[ComponentIndex];

		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(TargetComponent->ComponentIdentifier);

		float UsageRatio = TargetSpacecraft->GetDamageSystem()->GetUsableRatio(ComponentDescription, TargetComponent);
		float DamageRatio = TargetSpacecraft->GetDamageSystem()->GetDamageRatio(ComponentDescription, TargetComponent);

		if (ComponentDescription)
		{
			if (UsageRatio > 0)
			{
				if (ComponentDescription->Type == EFlarePartType::RCS)
				{
					ComponentSelection.Reserve(ComponentSelection.Num() + RCSWeight);
					for (int32 i = 0; i < RCSWeight; i++)
					{
						ComponentSelection.Add(ComponentIndex);
					}
				}

				if (ComponentDescription->Type == EFlarePartType::OrbitalEngine)
				{
					ComponentSelection.Reserve(ComponentSelection.Num() + PodWeight);
					for (int32 i = 0; i < PodWeight; i++)
					{
						ComponentSelection.Add(ComponentIndex);
					}
				}

				if (ComponentDescription->Type == EFlarePartType::Weapon)
				{
					ComponentSelection.Reserve(ComponentSelection.Num() + WeaponWeight);
					for (int32 i = 0; i < WeaponWeight; i++)
					{
						ComponentSelection.Add(ComponentIndex);
					}
				}

				if (ComponentDescription->Type == EFlarePartType::InternalComponent)
				{
					ComponentSelection.Reserve(ComponentSelection.Num() + InternalWeight);
					for (int32 i = 0; i < InternalWeight; i++)
					{
						ComponentSelection.Add(ComponentIndex);
					}
				}
			}
			else if (DamageRatio > 0)
			{
				ComponentSelection.Add(ComponentIndex);
			}
		}
	}

	if(ComponentSelection.Num() == 0)
	{
		return 0;
	}

	int32 ComponentIndex = FMath::RandRange(0, ComponentSelection.Num() - 1);
	return ComponentSelection[ComponentIndex];
}


#undef LOCTEXT_NAMESPACE