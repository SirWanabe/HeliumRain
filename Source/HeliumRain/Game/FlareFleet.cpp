
#include "FlareFleet.h"
#include "../Flare.h"

#include "FlareCompany.h"
#include "FlareGame.h"
#include "FlareGameTools.h"
#include "FlareSimulatedSector.h"

#include "../Economy/FlareCargoBay.h"

#include "../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareFleet"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareFleet::UFlareFleet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareFleet::Load(const FFlareFleetSave& Data)
{
	FleetCompany = Cast<UFlareCompany>(GetOuter());
	Game = FleetCompany->GetGame();
	FleetData = Data;
	IsShipListLoaded = false;
}

FFlareFleetSave* UFlareFleet::Save()
{
	return &FleetData;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

FText UFlareFleet::GetName()
{
	if (GetShips().Num() > 0)
	{
		return UFlareGameTools::DisplaySpacecraftName(GetShips()[0]);// TODO Clean with GetFleetName
	}
	else
	{
		return GetFleetName();
	}
}

FText  UFlareFleet::GetFleetName() const
{
	return FleetData.Name;
}

bool UFlareFleet::IsTraveling() const
{
	return CurrentTravel != NULL;
}

bool UFlareFleet::IsTrading() const
{
	return GetTradingShipCount() > 0;
}

bool UFlareFleet::IsAlive()
{
	if (GetShips().Num() > 0)
	{
		for (UFlareSimulatedSpacecraft* Ship : FleetShips)
		{
			if (Ship->GetDamageSystem()->IsAlive())
			{
				return true;
			}
		}
	}
	return false;
}

bool UFlareFleet::CanTravel(UFlareSimulatedSector* TargetSector)
{
	if (IsTraveling())
	{
		if (!GetCurrentTravel()->CanChangeDestination())
		{
			return false;
		}
		if (TargetSector && TargetSector == GetCurrentTravel()->GetDestinationSector())
		{
			return false;
		}
	}

	if (GetImmobilizedShipCount() == FleetShips.Num())
	{
		// All ship are immobilized
		return false;
	}

	if(Game->GetPC()->GetPlayerFleet() == this && Game->GetPC()->GetPlayerShip()->GetDamageSystem()->IsStranded())
	{
		// The player ship is stranded
		return false;
	}

	return true;
}

bool UFlareFleet::CanTravel(FText& OutInfo, UFlareSimulatedSector* TargetSector)
{
	if (IsTraveling())
	{
		if (!GetCurrentTravel()->CanChangeDestination())
		{
			OutInfo = LOCTEXT("TravelingFormat", "Can't change destination");
			return false;
		}

		if(TargetSector && TargetSector == GetCurrentTravel()->GetDestinationSector())
		{
			int64 TravelDuration = UFlareTravel::ComputeTravelDuration(Game->GetGameWorld(), GetCurrentSector(), TargetSector, Game->GetPC()->GetCompany(),this);
			FText DayText;

			if (TravelDuration == 1)
			{
				OutInfo = LOCTEXT("ShortTravelFormat", "(1 day)");
			}
			else
			{
				DayText = FText::Format(LOCTEXT("TravelFormat", "({0} days)"), FText::AsNumber(TravelDuration));
			}

			OutInfo = FText::Format(LOCTEXT("TravelingFormatHere", "Already on the way {0}"), DayText);
			return false;
		}
	}

	if (GetImmobilizedShipCount() == FleetShips.Num())
	{
		OutInfo = LOCTEXT("Traveling", "Trading, stranded or intercepted");
		return false;
	}

	if(Game->GetPC()->GetPlayerFleet() == this && Game->GetPC()->GetPlayerShip()->GetDamageSystem()->IsStranded())
	{
		OutInfo = LOCTEXT("PlayerShipStranded", "The ship you are piloting is stranded");
		return false;
	}

	return true;
}

FText UFlareFleet::GetStatusInfo() const
{
	bool Intercepted = false;

	for (UFlareSimulatedSpacecraft* Ship : FleetShips)
	{
		if(Ship->IsIntercepted())
		{
			Intercepted = true;
			break;
		}
	}

	if (Intercepted)
	{
		return FText::Format(LOCTEXT("FleetIntercepted", "Intercepted in {0}"), GetCurrentSector()->GetSectorName());
	}
	else if (IsTraveling())
	{
		int64 RemainingDuration = CurrentTravel->GetRemainingTravelDuration();
		return FText::Format(LOCTEXT("TravelTextFormat", "Traveling to {0} ({1} left)"),
			CurrentTravel->GetDestinationSector()->GetSectorName(),
			UFlareGameTools::FormatDate(RemainingDuration, 1));
	}

	else if (IsTrading())
	{
		if (GetTradingShipCount() == GetShipCount())
		{
			return FText::Format(LOCTEXT("FleetTrading", "Trading in {0}"), GetCurrentSector()->GetSectorName());
		}
		else
		{
			return FText::Format(LOCTEXT("FleetPartialTrading", "{0} of {1} ships are trading in {2}"), FText::AsNumber(GetTradingShipCount()), FText::AsNumber(GetShipCount()), GetCurrentSector()->GetSectorName());
		}
	}
	else if (IsAutoTrading() && this != Game->GetPC()->GetPlayerFleet())
	{
		if (IsTraveling())
		{
			return LOCTEXT("FleetAutoTradingTravel", "Auto-trading ");
		}
		else
		{
			return FText::Format(LOCTEXT("FleetAutoTrading", "Auto-trading in {0}"), GetCurrentSector()->GetSectorName());
		}
	}
	else
	{
		if (GetCurrentTradeRoute() && !GetCurrentTradeRoute()->IsPaused() && this != Game->GetPC()->GetPlayerFleet())
		{
			return FText::Format(LOCTEXT("FleetStartTrade", "Starting trade in {0}"), GetCurrentSector()->GetSectorName());
		}
		else
		{
			return FText::Format(LOCTEXT("FleetIdle", "Idle in {0}"), GetCurrentSector()->GetSectorName());
		}
	}

	return FText();
}

uint32 UFlareFleet::RemoveImmobilizedShips()
{
	TArray<UFlareSimulatedSpacecraft*> ShipToRemove;

	uint32 ShipsRemoved = 0;
	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* RemovingShip = FleetShips[ShipIndex];
		if (RemovingShip
			&& RemovingShip != Game->GetPC()->GetPlayerShip() 
			&& !RemovingShip->GetDescription()->IsDroneShip 
			&& !RemovingShip->CanTravel() )
		{
			ShipToRemove.Add(FleetShips[ShipIndex]);
			ShipsRemoved++;
		}
	}

	if (ShipsRemoved < GetShipCount())
	{
		RemoveShips(ShipToRemove);
	}
	return ShipsRemoved;
}

void UFlareFleet::SetFleetColor(FLinearColor Color)
{
	FleetData.FleetColor = Color;
}

FLinearColor UFlareFleet::GetFleetColor() const
{
	return FleetData.FleetColor;
}

void UFlareFleet::TrackIncomingBomb(AFlareBomb* Bomb)
{
	if (!Bomb)
	{
		return;
	}
	IncomingBombs.AddUnique(Bomb);
}

void UFlareFleet::UnTrackIncomingBomb(AFlareBomb* Bomb)
{
	if (!Bomb)
	{
		return;
	}
	IncomingBombs.RemoveSwap(Bomb);
}

TArray<AFlareBomb*> UFlareFleet::GetIncomingBombs()
{
	return IncomingBombs;
}

int32 UFlareFleet::GetRepairDuration() const
{
	int32 RepairDuration = 0;

	for (UFlareSimulatedSpacecraft* Ship : FleetShips)
	{
		int32 ShipRepairDuration = Ship->GetRepairDuration();

		if (ShipRepairDuration > RepairDuration)
		{
			RepairDuration = ShipRepairDuration;
		}
	}

	return RepairDuration;
}

int32 UFlareFleet::GetRefillDuration() const
{
	int32 RefillDuration = 0;

	for (UFlareSimulatedSpacecraft* Ship : FleetShips)
	{
		int32 ShipRefillDuration = Ship->GetRefillDuration();

		if (ShipRefillDuration > RefillDuration)
		{
			RefillDuration = ShipRefillDuration;
		}
	}

	return RefillDuration;
}

uint32 UFlareFleet::InterceptShips()
{
	// Intercept half of ships at maximum and min 1
	uint32 MaxInterseptedShipCount = FMath::Max(1,FleetShips.Num() / 2);
	uint32 InterseptedShipCount = 0;
	for (UFlareSimulatedSpacecraft* Ship : FleetShips)
	{
		if(InterseptedShipCount >= MaxInterseptedShipCount)
		{
			break;
		}

		if (Ship == Game->GetPC()->GetPlayerShip() || Ship->GetDescription()->IsDroneShip)
		{
			// Never intercept the player ship
			continue;
		}

		if(FMath::FRand() < 0.1)
		{
			Ship->SetIntercepted(true);
			InterseptedShipCount++;
		}
	}
	return InterseptedShipCount;
}

bool UFlareFleet::CanAddShip(UFlareSimulatedSpacecraft* Ship)
{
	if (IsTraveling())
	{
		return false;
	}

	if (GetCurrentSector() != Ship->GetCurrentSector())
	{
		return false;
	}

	if (!Ship->GetDescription()->IsDroneShip)
	{
		if (GetShipCount() + 1 > GetMaxShipCount())
		{
			return false;
		}
	}
	return true;
}

void UFlareFleet::AddShip(UFlareSimulatedSpacecraft* Ship, bool IgnoreSectorCheck)
{
	if (IsTraveling())
	{
		FLOGV("Fleet add fail: '%s' is travelling", *GetFleetName().ToString());
		return;
	}

	if (!IgnoreSectorCheck)
	{
		if (GetCurrentSector() != Ship->GetCurrentSector())
		{
			FLOGV("Fleet Merge fail: '%s' is the sector '%s' but '%s' is the sector '%s'",
				*GetFleetName().ToString(),
				*GetCurrentSector()->GetSectorName().ToString(),
				*Ship->GetImmatriculation().ToString(),
				*Ship->GetCurrentSector()->GetSectorName().ToString());
			return;
		}
	}

	UFlareFleet* OldFleet = Ship->GetCurrentFleet();
	if (OldFleet)
	{
		if (OldFleet == this)
		{
			return;
		}
		OldFleet->RemoveShip(Ship, false, false);
	}

	FleetData.ShipImmatriculations.AddUnique(Ship->GetImmatriculation());
	FleetShips.AddUnique(Ship);

	if (!Ship->GetDescription()->IsDroneShip)
	{
		FleetCount++;
	}

	Ship->SetCurrentFleet(this);

	if (FleetCompany == GetGame()->GetPC()->GetCompany() && GetGame()->GetQuestManager())
	{
		GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("add-ship-to-fleet"));
	}

	if (Ship->GetShipChildren().Num() > 0)
	{
		for (UFlareSimulatedSpacecraft* OwnedShips : Ship->GetShipChildren())
		{
			AddShip(OwnedShips,true);
		}
	}

	if (!Ship->GetDescription()->IsDroneShip)
	{
		if (!FleetSlowestShip)
		{
			if (GetShipCount() == 1)
			{
				SetSlowestShipPowerValue(Ship, Ship->GetEngineAccelerationPower());
			}
			else
			{
				RecalculateSlowestFleetShip();
			}
		}
		else if (FleetLowestEngineAccelerationPower > Ship->GetEngineAccelerationPower())
		{
			SetSlowestShipPowerValue(Ship, Ship->GetEngineAccelerationPower());
		}
	}
}

void UFlareFleet::RemoveShips(TArray<UFlareSimulatedSpacecraft*> ShipsToRemove)
{
	if (IsTraveling())
	{
		FLOGV("Fleet RemoveShips fail: '%s' is travelling", *GetFleetName().ToString());
		return;
	}

	UFlareFleet* NewFleet = nullptr;
	bool SlowestShipRemoved = false;
	for (int32 Index = 0; Index < ShipsToRemove.Num(); Index++)
	{
		UFlareSimulatedSpacecraft* Ship = ShipsToRemove[Index];
		if (!Ship)
		{
			continue;
		}

		FleetData.ShipImmatriculations.Remove(Ship->GetImmatriculation());
		FleetShips.Remove(Ship);

		if (FleetSlowestShip == Ship)
		{
			SlowestShipRemoved = true;
		}

		if (!Ship->GetDescription()->IsDroneShip)
		{
			FleetCount--;
		}

		Ship->SetCurrentFleet(NULL);

		if (NewFleet == nullptr)
		{
			NewFleet = Ship->GetCompany()->CreateAutomaticFleet(Ship);
		}
		else
		{
			NewFleet->AddShip(Ship);
		}
	}

	if (FleetShips.Num() == 0)
	{
		Disband();
	}
	else
	{
		if (SlowestShipRemoved)
		{
			RecalculateSlowestFleetShip();
		}
	}
}

void UFlareFleet::RemoveShip(UFlareSimulatedSpacecraft* Ship, bool Destroyed, bool ReformFleet)
{
	if (IsTraveling())
	{
		FLOGV("Fleet RemoveShip fail: '%s' is travelling", *GetFleetName().ToString());
		return;
	}

	FleetData.ShipImmatriculations.Remove(Ship->GetImmatriculation());
	FleetShips.Remove(Ship);
	Ship->SetCurrentFleet(NULL);

	if (!Ship->GetDescription()->IsDroneShip)
	{
		FleetCount--;
	}

	if (!Destroyed)
	{
		if (ReformFleet)
		{
			Ship->GetCompany()->CreateAutomaticFleet(Ship);
		}
	}

	if(FleetShips.Num() == 0)
	{
		Disband();
	}
	else if (FleetSlowestShip == Ship)
	{
		RecalculateSlowestFleetShip();
	}
}

/** Remove all ship from the fleet and delete it. Not possible during travel */
void UFlareFleet::Disband()
{
	if (IsTraveling())
	{
		FLOGV("Fleet Disband fail: '%s' is travelling", *GetFleetName().ToString());
		return;
	}

	FLOGV("Fleet Disband: '%s' for %s", 
	*GetFleetName().ToString(),
	*GetFleetCompany()->GetCompanyName().ToString());

	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		FleetShips[ShipIndex]->SetCurrentFleet(NULL);
	}

	if (GetCurrentTradeRoute())
	{
		GetCurrentTradeRoute()->RemoveFleet(this);
	}

	if (GetCurrentSector())
	{
		GetCurrentSector()->DisbandFleet(this);
	}

	FleetCompany->RemoveFleet(this);
}

bool UFlareFleet::CanMerge(UFlareFleet* OtherFleet, FText& OutInfo)
{
	if (GetShipCount() + OtherFleet->GetShipCount() > GetMaxShipCount())
	{
		OutInfo = LOCTEXT("MergeFleetMaxShips", "Can't merge, max ships reached");
		return false;
	}

	if (IsTraveling())
	{
		OutInfo = LOCTEXT("MergeFleetTravel", "Can't merge during travel");
		return false;
	}

	if (OtherFleet == Game->GetPC()->GetPlayerFleet())
	{
		OutInfo = LOCTEXT("MergeFleetPlayer", "Can't merge the player fleet into another");
		return false;
	}

	if (OtherFleet->IsTraveling())
	{
		OutInfo = LOCTEXT("MergeOtherFleetTravel", "Can't merge traveling ships");
		return false;
	}

	if (GetCurrentSector() != OtherFleet->GetCurrentSector())
	{
		OutInfo = LOCTEXT("MergeFleetDifferenSector", "Can't merge from a different sector");
		return false;
	}

	return true;
}

void UFlareFleet::Merge(UFlareFleet* Fleet)
{
	FText Unused;
	if (!CanMerge(Fleet, Unused))
	{
		FLOGV("Fleet Merge fail: '%s' is not mergeable", *Fleet->GetFleetName().ToString());
		return;
	}

	TArray<UFlareSimulatedSpacecraft*> Ships = Fleet->GetShips();
	Fleet->Disband();
	for (int ShipIndex = 0; ShipIndex < Ships.Num(); ShipIndex++)
	{
		AddShip(Ships[ShipIndex]);
	}
}

void UFlareFleet::SetCurrentSector(UFlareSimulatedSector* Sector)
{
	if (!Sector->IsTravelSector())
	{
		CurrentTravel = NULL;
	}

	CurrentSector = Sector;
	InitShipList();

	if (FleetCompany == GetGame()->GetPC()->GetCompany() && IsVisibleForOrbitalFleetList())
	{
		GetGame()->GetPC()->UpdateOrbitMenuFleets(false);
	}
}

void UFlareFleet::SetCurrentTravel(UFlareTravel* Travel)
{
	CurrentSector = Travel->GetTravelSector();
	CurrentTravel = Travel;
	InitShipList();

	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* FleetShip = FleetShips[ShipIndex];
		if (FleetShip)
		{
			FleetShip->ForceUndock();

			if (FleetShip->GetShipMaster())
			{
				FleetShip->SetInternalDockedTo(FleetShip->GetShipMaster());
			}
			else
			{
				FleetShip->SetSpawnMode(EFlareSpawnMode::Travel);
			}
		}
	}

	if (FleetCompany == GetGame()->GetPC()->GetCompany() && IsVisibleForOrbitalFleetList())
	{
		GetGame()->GetPC()->UpdateOrbitMenuFleets(false);
	}
}

bool UFlareFleet::IsVisibleForOrbitalFleetList()
{
	if (!GetShips().Num())
	{
		return false;
	}

	if ((!IsAutoTrading() && !IsHiddenTravel()) || this == Game->GetPC()->GetPlayerFleet())
	{
		if (IsTraveling())
		{
			if (!GetCurrentTravel()->CanChangeDestination())
			{
				return false;
			}
		}

		if (this != Game->GetPC()->GetPlayerFleet())
		{
			if (GetCurrentTradeRoute() && !GetCurrentTradeRoute()->IsPaused())
			{
				return false;
			}
		}

		if (GetImmobilizedShipCount() == GetShipCount())
		{
			return false;
		}
		return true;
	}
	return false;
}

void UFlareFleet::FleetShipDied(UFlareSimulatedSpacecraft* Ship)
{
	if (FleetCompany == GetGame()->GetPC()->GetCompany() && IsVisibleForOrbitalFleetList())
	{
		GetGame()->GetPC()->UpdateOrbitMenuFleets(false);
	}
}
void UFlareFleet::FleetShipUncontrollable(UFlareSimulatedSpacecraft* Ship)
{
	if (FleetCompany == GetGame()->GetPC()->GetCompany() && IsVisibleForOrbitalFleetList())
	{
		GetGame()->GetPC()->UpdateOrbitMenuFleets(false);
	}
}

void UFlareFleet::InitShipList()
{
	if (!IsShipListLoaded)
	{
		IsShipListLoaded = true;
		FleetShips.Empty();
		for (int ShipIndex = 0; ShipIndex < FleetData.ShipImmatriculations.Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = FleetCompany->FindSpacecraft(FleetData.ShipImmatriculations[ShipIndex]);
			if (!Ship)
			{
				FLOGV("WARNING: Fail to find ship with id %s in company %s for fleet %s (%d ships)",
						*FleetData.ShipImmatriculations[ShipIndex].ToString(),
						*FleetCompany->GetCompanyName().ToString(),
						*GetFleetName().ToString(),
						FleetData.ShipImmatriculations.Num());
				continue;
			}
			Ship->SetCurrentFleet(this);

			FleetShips.Add(Ship);
			if (Ship->GetShipChildren().Num() > 0)
			{
				for (UFlareSimulatedSpacecraft* OwnedShips : Ship->GetShipChildren())
				{
					FleetShips.Add(OwnedShips);
				}
			}

			if (!Ship->GetDescription()->IsDroneShip)
			{
				FleetCount++;
			}
		}
		RecalculateSlowestFleetShip();
	}
}

bool UFlareFleet::IsRepairing() const
{
	for (UFlareSimulatedSpacecraft* Ship : FleetShips)
	{
		if (Ship->IsRepairing())
		{
			return true;
		}
	}

	return false;
}

bool UFlareFleet::FleetNeedsRepair() const
{
	if (!IsRepairing())
	{
		for (UFlareSimulatedSpacecraft* Ship : FleetShips)
		{
			if (Ship->GetDamageRatio() < 1)
			{
				return true;
			}
		}
	}

	return false;
}

bool UFlareFleet::IsRefilling() const
{
	for (UFlareSimulatedSpacecraft* Ship : FleetShips)
	{
		if (Ship->IsRefilling())
		{
			return true;
		}
	}
	return false;
}

bool UFlareFleet::FleetNeedsRefill() const
{
	if (!IsRefilling())
	{
		for (UFlareSimulatedSpacecraft* Ship : FleetShips)
		{
			if (Ship->NeedRefill())
			{
				return true;
			}
		}
	}

	return false;
}

void UFlareFleet::RepairFleet()
{
	SectorHelper::RepairFleets(GetCurrentSector(), GetFleetCompany(), this);
}

void UFlareFleet::RefillFleet()
{
	SectorHelper::RefillFleets(GetCurrentSector(), GetFleetCompany(), this);
}



/*----------------------------------------------------
	Getters
----------------------------------------------------*/

uint32 UFlareFleet::GetImmobilizedShipCount()
{
	uint32 ImmobilizedShip = 0;

	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		if (!FleetShips[ShipIndex]->CanTravel() && FleetShips[ShipIndex]->GetDamageSystem()->IsAlive() && !FleetShips[ShipIndex]->GetDescription()->IsDroneShip)
		{
			ImmobilizedShip++;
		}
	}
	return ImmobilizedShip;
}

uint32 UFlareFleet::GetTradingShipCount() const
{
	uint32 TradingShip = 0;

	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		if (FleetShips[ShipIndex]->IsTrading())
		{
			TradingShip++;
		}
	}
	return TradingShip;
}

int32 UFlareFleet::GetFleetCapacity(bool SkipIfStranded) const
{
	int32 CargoCapacity = 0;
	for (UFlareSimulatedSpacecraft* Ship : FleetShips)
	{
		if (SkipIfStranded && Ship->GetDamageSystem()->IsStranded())
		{
			continue;
		}
		CargoCapacity += Ship->GetActiveCargoBay()->GetCapacity();
	}

	return CargoCapacity;
}

int32 UFlareFleet::GetFleetUsedCargoSpace() const
{
	int32 UsedCargoSpace = 0;

	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		UsedCargoSpace += FleetShips[ShipIndex]->GetActiveCargoBay()->GetUsedCargoSpace();
	}
	return UsedCargoSpace;
}

int32 UFlareFleet::GetFleetFreeCargoSpace() const
{
	int32 FreeCargoSpace = 0;

	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		FreeCargoSpace += FleetShips[ShipIndex]->GetActiveCargoBay()->GetFreeCargoSpace();
	}
	return FreeCargoSpace;
}

int32 UFlareFleet::GetFleetResourceQuantity(FFlareResourceDescription* Resource)
{
	int32 Quantity = 0;
	for (UFlareSimulatedSpacecraft* Ship : GetShips())
	{
		Quantity += Ship->GetActiveCargoBay()->GetResourceQuantity(Resource, Ship->GetCompany());
	}
	return Quantity;
}

int32 UFlareFleet::GetFleetFreeSpaceForResource(FFlareResourceDescription* Resource)
{
	int32 Quantity = 0;
	for (UFlareSimulatedSpacecraft* Ship : GetShips())
	{
		Quantity += Ship->GetActiveCargoBay()->GetFreeSpaceForResource(Resource, Ship->GetCompany());
	}
	return Quantity;
}

uint32 UFlareFleet::GetShipCount() const
{
	return FleetCount;
}

uint32 UFlareFleet::GetMilitaryShipCount() const
{
	uint32 Count = 0;
	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		if (FleetShips[ShipIndex]->GetDescription()->IsDroneShip)
		{
			continue;
		}

		if (FleetShips[ShipIndex]->IsMilitary())
		{
			Count++;
		}
	}
	return Count;
}

uint32 UFlareFleet::GetMilitaryShipCountBySize(EFlarePartSize::Type Size) const
{
	uint32 Count = 0;

	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		if (FleetShips[ShipIndex]->GetDescription()->IsDroneShip)
		{
			continue;
		}

		if (FleetShips[ShipIndex]->GetDescription()->Size == Size && FleetShips[ShipIndex]->IsMilitaryArmed())
		{
			Count++;
		}
	}

	return Count;
}

uint32 UFlareFleet::GetMaxShipCount()
{
	return 20;
}

int32 UFlareFleet::GetCombatPoints(bool ReduceByDamage) const
{
	int32 CombatPoints = 0;

	for (UFlareSimulatedSpacecraft* Ship : FleetShips)
	{
		CombatPoints += Ship->GetCombatPoints(ReduceByDamage);
	}
	return CombatPoints;
}

TArray<UFlareSimulatedSpacecraft*>& UFlareFleet::GetShips()
{
	InitShipList();

	return FleetShips;
}

int32 UFlareFleet::GetUnableToTravelShips() const
{
	return UnableToTravelShips;
}

FText UFlareFleet::GetTravelConfirmText()
{
	bool Escape = GetCurrentSector()->GetSectorBattleState(GetFleetCompany()).HasDanger
		&& (this != Game->GetPC()->GetPlayerFleet() || GetShipCount() > 1);
	bool Abandon = GetImmobilizedShipCount() != 0;

	FText SingleShip = LOCTEXT("ShipIsSingle", "ship is");
	FText MultipleShips = LOCTEXT("ShipArePlural", "ships are");

	int32 TradingShips = 0;
	int32 InterceptedShips = 0;
	int32 StrandedShips = 0;
	UnableToTravelShips = 0;

	for (UFlareSimulatedSpacecraft* Ship : GetShips())
	{
		if (!Ship->GetDescription()->IsDroneShip && (Ship->IsTrading() || Ship->GetDamageSystem()->IsStranded() || Ship->IsIntercepted()))
		{
			UnableToTravelShips++;
		}

		if (Ship->IsTrading())
		{
			TradingShips++;
		}

		if (Ship->GetDamageSystem()->IsStranded())
		{
			StrandedShips++;
		}

		if (Ship->IsIntercepted())
		{
			InterceptedShips++;
		}
	}

	FText TooDamagedTravelText;
	FText TradingTravelText;
	FText InterceptedTravelText;

	bool useOr = false;

	if (TradingShips > 0)
	{
		TradingTravelText = LOCTEXT("TradingTravelText", "trading");
		useOr = true;
	}

	if (InterceptedShips > 0)
	{
		if (useOr)
		{
			InterceptedTravelText = UFlareGameTools::AddLeadingSpace(LOCTEXT("OrInterceptedTravelText", "or intercepted"));
		}
		else
		{
			InterceptedTravelText = LOCTEXT("InterceptedTravelText", "intercepted");
		}
		useOr = true;
	}

	if (StrandedShips > 0)
	{
		if (useOr)
		{
			TooDamagedTravelText = UFlareGameTools::AddLeadingSpace(LOCTEXT("OrTooDamagedToTravel", "or too damaged to travel"));
		}
		else
		{
			TooDamagedTravelText = LOCTEXT("TooDamagedToTravel", "too damaged to travel");
		}
	}

	FText ReasonNotTravelText = FText::Format(LOCTEXT("ReasonNotTravelText", "{0}{1}{2} and will be left behind"),
		TradingTravelText,
		InterceptedTravelText,
		TooDamagedTravelText);

	// We can escape
	if (Escape)
	{
		FText EscapeWarningText = LOCTEXT("ConfirmTravelEscapeWarningText", "Ships can be intercepted while escaping, are you sure ?");

		if (Abandon)
		{
			return FText::Format(LOCTEXT("ConfirmTravelEscapeFormat", "{0} {1} {2} {3}."),
				EscapeWarningText,
				FText::AsNumber(GetImmobilizedShipCount()),
				(GetImmobilizedShipCount() > 1) ? MultipleShips : SingleShip,
				ReasonNotTravelText);
		}
		else
		{
			return EscapeWarningText;
		}
	}

	// We have to abandon
	else
	{
		return FText::Format(LOCTEXT("ConfirmTravelAbandonFormat", "{0} {1} {2}."),
			FText::AsNumber(GetImmobilizedShipCount()),
			(GetImmobilizedShipCount() > 1) ? MultipleShips : SingleShip,
			ReasonNotTravelText);
	}
	return FText();
}
FText UFlareFleet::GetRepairText()
{
	UFlareSimulatedSector* TargetSector = GetCurrentSector();
	if (!TargetSector)
	{
		return FText();
	}

	int32 AvailableFS;
	int32 OwnedFS;
	int32 AffordableFS;
	int32 NeededFS;
	int32 TotalNeededFS;
	int64 MaxDuration;

	SectorHelper::GetRepairFleetSupplyNeeds(TargetSector, GetShips(), NeededFS, TotalNeededFS, MaxDuration, true);
	SectorHelper::GetAvailableFleetSupplyCount(TargetSector, GetFleetCompany(), OwnedFS, AvailableFS, AffordableFS, nullptr, this);

	if (TotalNeededFS > 0)
	{
		// Repair needed
		if (TargetSector->IsInDangerousBattle(GetFleetCompany()))
		{
			return LOCTEXT("CantRepairBattle", "Can't repair here : battle in progress!");
		}
		else if (AvailableFS == 0) {
			return LOCTEXT("CantRepairNoFS", "Can't repair here : no fleet supply available !");
		}
		else if (AffordableFS == 0)
		{
			return LOCTEXT("CantRepairNoMoney", "Can't repair here : not enough money !");
		}
		else
		{
			int32 UsedFs = FMath::Min(AffordableFS, TotalNeededFS);
			int32 UsedOwnedFs = FMath::Min(OwnedFS, UsedFs);
			int32 UsedNotOwnedFs = UsedFs - UsedOwnedFs;
			FFlareResourceDescription* FleetSupply = GetGame()->GetScenarioTools()->FleetSupply;

			int64 UsedNotOwnedFsCost = UsedNotOwnedFs * TargetSector->GetResourcePrice(FleetSupply, EFlareResourcePriceContext::MaintenanceConsumption);


			FText OwnedCostText;
			FText CostSeparatorText;
			FText NotOwnedCostText;

			if (UsedOwnedFs > 0)
			{
				OwnedCostText = FText::Format(LOCTEXT("RepairOwnedCostFormat", "{0} fleet supplies"), FText::AsNumber(UsedOwnedFs));
			}

			if (UsedNotOwnedFsCost > 0)
			{
				NotOwnedCostText = FText::Format(LOCTEXT("RepairNotOwnedCostFormat", "{0} credits"), FText::AsNumber(UFlareGameTools::DisplayMoney(UsedNotOwnedFsCost)));
			}

			if (UsedOwnedFs > 0 && UsedNotOwnedFsCost > 0)
			{
				CostSeparatorText = UFlareGameTools::AddLeadingSpace(LOCTEXT("CostSeparatorText", "+ "));
			}

			FText CostText = FText::Format(LOCTEXT("RepairCostFormat", "{0}{1}{2}"), OwnedCostText, CostSeparatorText, NotOwnedCostText);

			return FText::Format(LOCTEXT("RepairOkayFormat", "Repair all ships ({0}, {1} days)"),
				CostText,
				FText::AsNumber(MaxDuration));
		}
	}
	else if (SectorHelper::HasShipRepairing(GetShips(), GetFleetCompany()))
	{
		// Repair in progress
		return LOCTEXT("RepairInProgress", "Repair in progress...");
	}
	else
	{
		// No repair needed
		return LOCTEXT("NoShipToRepair", "No ship needs repairing");
	}
}
FText UFlareFleet::GetRefillText()

{
	UFlareSimulatedSector* TargetSector = GetCurrentSector();
	if (!TargetSector)
	{
		return FText();
	}

	int32 AvailableFS;
	int32 OwnedFS;
	int32 AffordableFS;
	int32 NeededFS;
	int32 TotalNeededFS;
	int64 MaxDuration;

	SectorHelper::GetRefillFleetSupplyNeeds(TargetSector, GetShips(), NeededFS, TotalNeededFS, MaxDuration, true);
	SectorHelper::GetAvailableFleetSupplyCount(TargetSector, GetFleetCompany(), OwnedFS, AvailableFS, AffordableFS, nullptr, this);

	if (TotalNeededFS > 0)
	{
		// Refill needed
		if (TargetSector->IsInDangerousBattle(GetFleetCompany()))
		{
			return LOCTEXT("CantRefillBattle", "Can't refill : battle in progress!");
		}
		else if (AvailableFS == 0) {
			return LOCTEXT("CantRefillNoFS", "Can't refill : no fleet supply available !");
		}
		else if (AffordableFS == 0)
		{
			return LOCTEXT("CantRefillNoMoney", "Can't refill : not enough money !");
		}
		else
		{
			int32 UsedFs = FMath::Min(AffordableFS, TotalNeededFS);
			int32 UsedOwnedFs = FMath::Min(OwnedFS, UsedFs);
			int32 UsedNotOwnedFs = UsedFs - UsedOwnedFs;
			FFlareResourceDescription* FleetSupply = TargetSector->GetGame()->GetScenarioTools()->FleetSupply;

			int64 UsedNotOwnedFsCost = UsedNotOwnedFs * TargetSector->GetResourcePrice(FleetSupply, EFlareResourcePriceContext::MaintenanceConsumption);

			FText OwnedCostText;
			FText CostSeparatorText;
			FText NotOwnedCostText;

			if (UsedOwnedFs > 0)
			{
				OwnedCostText = FText::Format(LOCTEXT("RefillOwnedCostFormat", "{0} fleet supplies"), FText::AsNumber(UsedOwnedFs));
			}

			if (UsedNotOwnedFsCost > 0)
			{
				NotOwnedCostText = FText::Format(LOCTEXT("RefillNotOwnedCostFormat", "{0} credits"), FText::AsNumber(UFlareGameTools::DisplayMoney(UsedNotOwnedFsCost)));
			}

			if (UsedOwnedFs > 0 && UsedNotOwnedFsCost > 0)
			{
				CostSeparatorText = UFlareGameTools::AddLeadingSpace(LOCTEXT("CostSeparatorText", "+ "));
			}

			FText CostText = FText::Format(LOCTEXT("RefillCostFormat", "{0}{1}{2}"), OwnedCostText, CostSeparatorText, NotOwnedCostText);

			return FText::Format(LOCTEXT("RefillOkayFormat", "Refill ({0}, {1} days)"),
			CostText,
			FText::AsNumber(MaxDuration));
		}
	}
	else if (SectorHelper::HasShipRefilling(GetShips(), GetFleetCompany()))
	{

		// Refill in progress
		return LOCTEXT("RefillInProgress", "Refill in progress...");
	}
	else
	{
		// No refill needed
		return LOCTEXT("NoFleetToRefill", "No ship needs refilling");
	}
}

void UFlareFleet::SelectWhiteListDefault(FName IdentifierSearch)
{
	UFlareCompanyWhiteList* FoundWhiteList = FleetCompany->GetWhiteList(IdentifierSearch);
	SelectWhiteListDefault(FoundWhiteList);
}

void UFlareFleet::SelectWhiteListDefault(UFlareCompanyWhiteList* NewWhiteList)
{
	if (NewWhiteList)
	{
		FleetData.DefaultWhiteListIdentifier = NewWhiteList->GetWhiteListIdentifier();

		if (FleetSelectedWhiteList && FleetSelectedWhiteList != NewWhiteList)
		{
			FleetSelectedWhiteList->RemoveFleetFromTracker(this);
		}

		FleetSelectedWhiteList = NewWhiteList;
		FleetSelectedWhiteList->AddFleetToTracker(this);
	}
	else
	{
		if (FleetSelectedWhiteList)
		{
			FleetSelectedWhiteList->RemoveFleetFromTracker(this);
		}

		FleetData.DefaultWhiteListIdentifier = FName();
		FleetSelectedWhiteList = nullptr;
	}
}

UFlareCompanyWhiteList* UFlareFleet::GetActiveWhitelist()
{
	if (GetSelectedWhiteList())
	{
		return GetSelectedWhiteList();
	}
	if (FleetCompany && FleetCompany->GetCompanySelectedWhiteList())
	{
		return FleetCompany->GetCompanySelectedWhiteList();
	}

	return nullptr;
}

bool UFlareFleet::CanTradeWhiteListTo(UFlareSimulatedSpacecraft* OtherSpacecraft, FFlareResourceDescription* Resource)
{
	if (!OtherSpacecraft)
	{
		return true;
	}
	UFlareCompanyWhiteList* ActiveWhiteList = GetActiveWhitelist();
	if (ActiveWhiteList)
	{
		FFlareWhiteListCompanyDataSave* CompanyData = ActiveWhiteList->GetCompanyDataFor(OtherSpacecraft->GetCompany());
		if (CompanyData)
		{
			FText Reason;
			if (!ActiveWhiteList->CanCompanyDataTradeTo(CompanyData, Resource, Reason))
			{
				return false;
			}
		}
	}
	return true;
}

bool UFlareFleet::CanTradeWhiteListFrom(UFlareSimulatedSpacecraft* OtherSpacecraft, FFlareResourceDescription* Resource)
{
	if (!OtherSpacecraft)
	{
		return true;
	}
	UFlareCompanyWhiteList* ActiveWhiteList = GetActiveWhitelist();
	if (ActiveWhiteList)
	{
		FFlareWhiteListCompanyDataSave* CompanyData = ActiveWhiteList->GetCompanyDataFor(OtherSpacecraft->GetCompany());
		if (CompanyData)
		{
			FText Reason;
			if (!ActiveWhiteList->CanCompanyDataTradeFrom(CompanyData, Resource, Reason))
			{
				return false;
			}
		}
	}
	return true;
}

void UFlareFleet::RecalculateSlowestFleetShip()
{
	FleetSlowestShip = nullptr;
	FleetLowestEngineAccelerationPower = 0.f;

	UFlareSpacecraftComponentsCatalog* Catalog = GetGame()->GetShipPartsCatalog();
	for (UFlareSimulatedSpacecraft* Ship : GetShips())
	{
		if (!Ship->GetDescription()->IsDroneShip)
		{
			float ShipEnginePower = Ship->GetEngineAccelerationPower();

			if (!FleetSlowestShip || ShipEnginePower < FleetLowestEngineAccelerationPower)
			{
				SetSlowestShipPowerValue(Ship, ShipEnginePower);
			}
		}
	}
}

void UFlareFleet::SetSlowestShipPowerValue(UFlareSimulatedSpacecraft* NewShip,float NewValue)
{
	FleetSlowestShip = NewShip;
	FleetLowestEngineAccelerationPower = NewValue;
}


#undef LOCTEXT_NAMESPACE