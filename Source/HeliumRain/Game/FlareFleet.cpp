
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
			int64 TravelDuration = UFlareTravel::ComputeTravelDuration(Game->GetGameWorld(), GetCurrentSector(), TargetSector, Game->GetPC()->GetCompany());
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
	bool Intersepted = false;

	for (UFlareSimulatedSpacecraft* Ship : FleetShips)
	{
		if(Ship->IsIntercepted())
		{
			Intersepted = true;
			break;
		}
	}

	if (Intersepted)
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
	else if (IsAutoTrading())
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
		if (GetCurrentTradeRoute() && !GetCurrentTradeRoute()->IsPaused())
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

void UFlareFleet::RemoveImmobilizedShips()
{
	TArray<UFlareSimulatedSpacecraft*> ShipToRemove;

	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* RemovingShip = FleetShips[ShipIndex];
		if (RemovingShip && !RemovingShip->CanTravel() && RemovingShip != Game->GetPC()->GetPlayerShip())
		{
			if (RemovingShip->GetShipMaster())
			{
				if (RemovingShip->GetDescription()->IsDroneShip)
				{
//				RemovingShip->TryMigrateDrones();
				continue;
				}
			}
			ShipToRemove.Add(FleetShips[ShipIndex]);
		}
	}

	RemoveShips(ShipToRemove);
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

int32 UFlareFleet::InterceptShips()
{
	// Intercept half of ships at maximum and min 1
	int32 MaxInterseptedShipCount = FMath::Max(1,FleetShips.Num() / 2);
	int32 InterseptedShipCount = 0;
	for (UFlareSimulatedSpacecraft* Ship : FleetShips)
	{
		if(InterseptedShipCount >=MaxInterseptedShipCount)
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

void UFlareFleet::AddShip(UFlareSimulatedSpacecraft* Ship)
{
	if (IsTraveling())
	{
		FLOGV("Fleet add fail: '%s' is travelling", *GetFleetName().ToString());
		return;
	}

	if (GetCurrentSector() != Ship->GetCurrentSector())
	{
		FLOGV("Fleet Merge fail: '%s' is the sector '%s' but '%s' is the sector '%s'",
			  *GetFleetName().ToString(),
			  *GetCurrentSector()->GetSectorName().ToString(),
			  *Ship->GetImmatriculation().ToString(),
			  *Ship->GetCurrentSector()->GetSectorName().ToString());
		return;
	}

	UFlareFleet* OldFleet = Ship->GetCurrentFleet();
	if (OldFleet)
	{
		OldFleet->RemoveShip(Ship, false, false);
	}

	FleetData.ShipImmatriculations.Add(Ship->GetImmatriculation());
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
			this->AddShip(OwnedShips);
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
	for (int32 Index = 0; Index < ShipsToRemove.Num(); Index++)
	{
		UFlareSimulatedSpacecraft* Ship = ShipsToRemove[Index];
		if (!Ship)
		{
			continue;
		}

		FleetData.ShipImmatriculations.Remove(Ship->GetImmatriculation());
		FleetShips.Remove(Ship);
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
}

void UFlareFleet::RemoveShip(UFlareSimulatedSpacecraft* Ship, bool destroyed, bool reformfleet)
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

	if (!destroyed)
	{
		if (reformfleet)
		{
			Ship->GetCompany()->CreateAutomaticFleet(Ship);
		}
	}

	if(FleetShips.Num() == 0)
	{
		Disband();
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

	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		FleetShips[ShipIndex]->SetCurrentFleet(NULL);
	}

	if (GetCurrentTradeRoute())
	{
		GetCurrentTradeRoute()->RemoveFleet(this);
	}
	GetCurrentSector()->DisbandFleet(this);
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
			if (!Ship->GetDescription()->IsDroneShip)
			{
				FleetCount++;
			}
		}
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

uint32 UFlareFleet::GetMilitaryShipCountBySize(EFlarePartSize::Type Size) const
{
	uint32 Count = 0;

	for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
	{
		if (FleetShips[ShipIndex]->GetDescription()->IsDroneShip)
		{
			continue;
		}

		if (FleetShips[ShipIndex]->GetDescription()->Size == Size && FleetShips[ShipIndex]->IsMilitary())
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

#undef LOCTEXT_NAMESPACE