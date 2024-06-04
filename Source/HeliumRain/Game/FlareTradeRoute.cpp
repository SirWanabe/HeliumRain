
#include "FlareTradeRoute.h"
#include "../Flare.h"

#include "../Data/FlareResourceCatalog.h"

#include "../Economy/FlareCargoBay.h"

#include "../Player/FlarePlayerController.h"

#include "../Quests/FlareQuestManager.h"

#include "FlareCompany.h"
#include "FlareSimulatedSector.h"
#include "FlareFleet.h"
#include "FlareGame.h"
#include "FlareSectorHelper.h"

#define LOCTEXT_NAMESPACE "FlareTradeRouteInfos"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareTradeRoute::UFlareTradeRoute(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareTradeRoute::Load(const FFlareTradeRouteSave& Data)
{
	TradeRouteCompany = Cast<UFlareCompany>(GetOuter());
	Game = TradeRouteCompany->GetGame();
	TradeRouteData = Data;
	IsFleetListLoaded = false;

	UpdateTargetSector();

    InitFleetList();
}

FFlareTradeRouteSave* UFlareTradeRoute::Save()
{
	return &TradeRouteData;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareTradeRoute::Simulate()
{
	FLOG("Trade route simulate");

	++TradeRouteData.StatsDays;

	if(TradeRouteData.IsPaused)
	{
		// Trade route paused. To nothing
		return;
	}

	if (TradeRouteData.Sectors.Num() == 0 || TradeRouteFleet == NULL)
    {
		FLOG("  -> no sector or assigned fleet");
        // Nothing to do
        return;
    }

	if (TradeRouteFleet->IsTraveling())
	{
		FLOG("  -> is travelling");
		return;
	}

	UFlareSimulatedSector* TargetSector = UpdateTargetSector();
	// Not travelling, check if the fleet is in a trade route sector
	UFlareSimulatedSector* CurrentSector = TradeRouteFleet->GetCurrentSector();
	ShouldRestartSimulation = false;

	if (TargetSector == CurrentSector)
	{
		// In the target sector
		FFlareTradeRouteSectorSave* SectorOrder = GetSectorOrders(CurrentSector);

		while (TradeRouteData.CurrentOperationIndex < SectorOrder->Operations.Num())
		{
			bool ProcessingResult = ProcessCurrentOperation(&SectorOrder->Operations[TradeRouteData.CurrentOperationIndex]);
			if (ShouldRestartSimulation)
			{
				Simulate();
				return;
			}

			if(ProcessingResult)
			{
				// Operation finish
				TradeRouteData.CurrentOperationDuration = 0;
				TradeRouteData.CurrentOperationProgress = 0;
				TradeRouteData.CurrentOperationIndex++;
			}
			else
			{
				TradeRouteData.CurrentOperationDuration++;
				break;
			}
		}

		if (TradeRouteData.CurrentOperationIndex >= SectorOrder->Operations.Num())
		{
			// Sector operations finished

			for(int i = 0; i < TradeRouteData.Sectors.Num(); i++)
			{
				TargetSector = GetNextTradeSector(TargetSector);
				if(IsUsefulSector(TargetSector))
				{
					break;
				}
			}

			SetTargetSector(TargetSector);
		}
	}


	if (TargetSector && TargetSector != CurrentSector)
	{
		if (! TradeRouteFleet->IsTrading())
		{
			if(TargetSector->GetSectorBattleState(TradeRouteCompany).HasDanger)
			{
				FFlareMenuParameterData Data;
				Data.Sector = TargetSector;

				Game->GetPC()->Notify(LOCTEXT("TradeRouteDanger", "Trade route destination is defended"),
					FText::Format(LOCTEXT("TradeRouteDangerFormat", "Your trade route {0} is stuck because its next destination is dangerous. Travel to {1} manually or pause the trade route."),
						GetTradeRouteName(),
						TargetSector->GetSectorName()),
					FName("trade-route-danger"),
					EFlareNotification::NT_Military,
					false,
					EFlareMenu::MENU_Sector,
					Data);

				FLOGV("  -> travel to %s abort because of danger", *TargetSector->GetSectorName().ToString());
			}
			else
			{

				FLOGV("  -> start travel to %s", *TargetSector->GetSectorName().ToString());
				// Travel to next sector
				Game->GetGameWorld()->StartTravel(TradeRouteFleet, TargetSector);
			}
		}
	}
	
	/*FLOGV("=== Stats for %s ===", *GetTradeRouteName().ToString());
	FLOGV("- day count: %d", TradeRouteData.StatsDays);
	FLOGV("- load quantity: %d", TradeRouteData.StatsLoadResources);
	FLOGV("- unload quantity: %d", TradeRouteData.StatsUnloadResources);
	FLOGV("- resource balance : %d", (TradeRouteData.StatsLoadResources - TradeRouteData.StatsUnloadResources));
	FLOGV("- buy amount: %lld credits", UFlareGameTools::DisplayMoney(TradeRouteData.StatsMoneyBuy));
	FLOGV("- sell amount : %lld credits", UFlareGameTools::DisplayMoney(TradeRouteData.StatsMoneySell));
	FLOGV("- balance : %lld credits", UFlareGameTools::DisplayMoney(TradeRouteData.StatsMoneySell - TradeRouteData.StatsMoneyBuy));
	FLOGV("- operation success: %d", TradeRouteData.StatsOperationSuccessCount);
	FLOGV("- operation fail: %d", TradeRouteData.StatsOperationFailCount);

	FLOGV("- gain per day: %f credits/day", 0.01 * float(TradeRouteData.StatsMoneySell - TradeRouteData.StatsMoneyBuy) / float(TradeRouteData.StatsDays));
	FLOGV("- sucess ratio: %f", float(TradeRouteData.StatsOperationSuccessCount) / (TradeRouteData.StatsOperationFailCount + TradeRouteData.StatsOperationSuccessCount));*/
}

UFlareSimulatedSector* UFlareTradeRoute::UpdateTargetSector()
{
	UFlareSimulatedSector* TargetSector = Game->GetGameWorld()->FindSector(TradeRouteData.TargetSectorIdentifier);
	if (!TargetSector || GetSectorOrders(TargetSector) == NULL)
	{
		TargetSector = GetNextTradeSector(NULL);
		if(TargetSector)
		{
			FLOGV("Has TargetSector %s", *TargetSector->GetIdentifier().ToString());
		}
		else
		{
			FLOG("Has no TargetSector");
		}
		SetTargetSector(TargetSector);
	}

	return TargetSector;
}

bool UFlareTradeRoute::ProcessCurrentOperation(FFlareTradeRouteSectorOperationSave* Operation)
{
	if(Operation->MaxWait == 0)
	{
		// Minimum wait is 1
		Operation->MaxWait = 1;
	}

	if (Operation->MaxWait != -1 && TradeRouteData.CurrentOperationDuration >= Operation->MaxWait)
	{
		FLOGV("Max wait duration reach (%d)", Operation->MaxWait);
		return true;
	}

	if (Operation->OperationConditions.Num() > 0)
	{
		for (FFlareTradeRouteOperationConditionSave& Condition : Operation->OperationConditions)
		{
			if (Condition.ConditionRequirement == EFlareTradeRouteOperationConditions::PercentOfTimes)
			{
				if (FMath::FRand() <= Condition.ConditionPercentage)
				{
					return Condition.SkipOnConditionFail;
				}
			}
			else if (Condition.ConditionRequirement == EFlareTradeRouteOperationConditions::LoadPercentage)
			{
				float PercentageCargo = (float)TradeRouteFleet->GetFleetUsedCargoSpace() / (float)TradeRouteFleet->GetFleetCapacity();
				if (PercentageCargo <= Condition.ConditionPercentage)
				{
					return Condition.SkipOnConditionFail;
				}
			}
			else if (Condition.ConditionRequirement == EFlareTradeRouteOperationConditions::RequiresMaintenance)
			{
				bool FleetNeedsRepair = TradeRouteFleet->FleetNeedsRepair();
				bool FleetNeedsRefill = TradeRouteFleet->FleetNeedsRefill();

				// Checking AND Condition
				if (Condition.BooleanTwo)
				{
					if (Condition.BooleanOne == FleetNeedsRepair && Condition.BooleanThree == FleetNeedsRefill)
					{
					}
					else
					{
						return Condition.SkipOnConditionFail;
					}
				}
				else if (Condition.BooleanOne == FleetNeedsRepair || Condition.BooleanThree == FleetNeedsRefill)
				{
				}
				else
				{
					return Condition.SkipOnConditionFail;
				}
			}
			else if (Condition.ConditionRequirement == EFlareTradeRouteOperationConditions::AtWar)
			{
				bool IsAtWar = TradeRouteFleet->GetFleetCompany()->AtWar();
				if (Condition.BooleanOne != IsAtWar)
				{
					return Condition.SkipOnConditionFail;
				}
			}
		}
	}

	// Return true if : limit reach or all ship full/empty or buy and no money or sell and nobody has money
	switch (Operation->Type)
	{
		case EFlareTradeRouteOperation::Load:
			return ProcessLoadOperation(Operation);
			break;
		case EFlareTradeRouteOperation::Unload:
			return ProcessUnloadOperation(Operation);
			break;
		case EFlareTradeRouteOperation::GotoOperation:
			return ProcessGotoOperation(Operation);
			break;
		case EFlareTradeRouteOperation::Maintenance:
			return ProcessMaintenanceOperation(Operation);
			break;

		default:
			FLOGV("ERROR: Unknown trade route operation (%d)", (Operation->Type + 0));
			break;
	}
	return true;
}


bool UFlareTradeRoute::ProcessMaintenanceOperation(FFlareTradeRouteSectorOperationSave* Operation)
{
	// Repair
	if (Operation->CanTradeWithStorages)
	{
		SectorHelper::RepairFleets(TradeRouteFleet->GetCurrentSector(), TradeRouteFleet->GetFleetCompany(), TradeRouteFleet);
	}
	// Rearm
	if (Operation->CanDonate)
	{
		SectorHelper::RefillFleets(TradeRouteFleet->GetCurrentSector(), TradeRouteFleet->GetFleetCompany(), TradeRouteFleet);
	}
	// Try once and move on
	return true;
}

bool UFlareTradeRoute::ProcessGotoOperation(FFlareTradeRouteSectorOperationSave* Operation)
{
	if (Operation->GotoSectorIndex != -1 && Operation->GotoOperationIndex != -1)
	{
		if (Operation->GotoSectorIndex < TradeRouteData.Sectors.Num())
		{
			FFlareTradeRouteSectorSave& SectorOrders = TradeRouteData.Sectors[Operation->GotoSectorIndex];
			if (Operation->GotoOperationIndex < SectorOrders.Operations.Num())
			{
				//Check useful enabled for goto order
				if (Operation->CanTradeWithStorages)
				{
					UFlareSimulatedSector* Sector = Game->GetGameWorld()->FindSector(SectorOrders.SectorIdentifier);
					if (Sector)
					{
						if (!IsUsefulSector(Sector, Operation->GotoOperationIndex))
						{
							//Sector isn't useful, skip
							return true;
						}
					}
				}

				UFlareSimulatedSector* SimulatedSector = Game->GetGameWorld()->FindSector(TradeRouteData.Sectors[Operation->GotoSectorIndex].SectorIdentifier);
				SetTargetSector(SimulatedSector);
				TradeRouteData.CurrentOperationIndex = Operation->GotoOperationIndex;
				ShouldRestartSimulation = true;
				return true;
			}
		}
	}
	return false;
}

bool UFlareTradeRoute::ProcessLoadOperation(FFlareTradeRouteSectorOperationSave* Operation)
{

	FFlareResourceDescription* Resource = Game->GetResourceCatalog()->Get(Operation->ResourceIdentifier);
	TArray<UFlareSimulatedSpacecraft*> UsefulShips;

	TArray<UFlareSimulatedSpacecraft*>&  RouteShips = TradeRouteFleet->GetShips();
	int32 FleetFreeSpace = 0;
	bool StatFail = false;

	auto UpdateOperationStats = [&]()
	{
		if(StatFail)
		{
			++TradeRouteData.StatsOperationFailCount;
		}
		else
		{
			++TradeRouteData.StatsOperationSuccessCount;
		}
	};

	//
	for (int ShipIndex = 0; ShipIndex < RouteShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = RouteShips[ShipIndex];

		// Keep trading ship as they will be useful later
		int32 FreeSpace = Ship->GetActiveCargoBay()->GetFreeSpaceForResource(Resource, Ship->GetCompany());
		if (FreeSpace > 0)
		{
			FleetFreeSpace += FreeSpace;
			UsefulShips.Add(Ship);
		}

		// TODO sort by most pertinent
	}

	if (FleetFreeSpace == 0)
	{
		// Fleet full: operation done
		StatFail = true;
		UpdateOperationStats();
		return true;
	}

	SectorHelper::FlareTradeRequest Request;
	Request.Resource = Resource;
	Request.Operation = Operation->Type;
	Request.CargoLimit = -1;
	Request.MaxQuantity = Operation->MaxQuantity;
	Request.LoadUnloadPriority = Operation->LoadUnloadPriority;
	Request.BuySellPriority = Operation->BuySellPriority;
	Request.IsDonation = Operation->CanDonate;
	Request.AllowStorage = Operation->CanTradeWithStorages;
	Request.AllowFullStock = true;

	for (UFlareSimulatedSpacecraft* Ship : UsefulShips)
	{
		if (Ship->IsTrading())
		{
			// Skip trading ships
			continue;
		}

		Request.Client = Ship;
		Request.MaxQuantity = Ship->GetActiveCargoBay()->GetFreeSpaceForResource(Resource, Ship->GetCompany());
		if (Operation->MaxQuantity !=-1)
		{
			Request.MaxQuantity = FMath::Min(Request.MaxQuantity, GetOperationRemainingQuantity(Operation));
		}

		if (Operation->InventoryLimit !=-1)
		{
			Request.MaxQuantity = FMath::Min(Request.MaxQuantity, GetOperationRemainingInventoryQuantity(Operation));
		}

		UFlareSimulatedSpacecraft* StationCandidate = SectorHelper::FindTradeStation(Request);

		if (StationCandidate)
		{
			int64 TransactionPrice;
			int32 Quantity = SectorHelper::Trade(StationCandidate, Ship, Resource, Request.MaxQuantity, &TransactionPrice, this);
			TradeRouteData.CurrentOperationProgress += Quantity;

			if (TradeRouteCompany == GetGame()->GetPC()->GetCompany())
			{
				Game->GetQuestManager()->OnEvent(FFlareBundle().PutTag("trade-route-transaction").PutInt32("money-variation", -TransactionPrice));
			}

			TradeRouteData.StatsMoneyBuy += TransactionPrice;
			TradeRouteData.StatsLoadResources += Quantity;

			if(Quantity == 0)
			{
				StatFail = true;
			}
		}
		else
		{
			StatFail = true;
		}

		if (IsOperationQuantityLimitReach(Operation) || IsOperationInventoryLimitReach(Operation))
		{
			// Operation limit reach : operation done
			UpdateOperationStats();
			return true;
		}
	}

	// Limit not reach and useful ship present. Operation not finished
	UpdateOperationStats();
	return false;
}

bool UFlareTradeRoute::ProcessUnloadOperation(FFlareTradeRouteSectorOperationSave* Operation)
{

	FFlareResourceDescription* Resource = Game->GetResourceCatalog()->Get(Operation->ResourceIdentifier);

	TArray<UFlareSimulatedSpacecraft*> UsefullShips;

	TArray<UFlareSimulatedSpacecraft*>&  RouteShips = TradeRouteFleet->GetShips();
	int32 FleetQuantity = 0;

	bool StatFail = false;

	auto UpdateOperationStats = [&]()
	{
		if(StatFail)
		{
			++TradeRouteData.StatsOperationFailCount;
		}
		else
		{
			++TradeRouteData.StatsOperationSuccessCount;
		}
	};


	for (int ShipIndex = 0; ShipIndex < RouteShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = RouteShips[ShipIndex];

		// Keep trading ship as they will be usefull later
		int32 Quantity = Ship->GetActiveCargoBay()->GetResourceQuantity(Resource, Ship->GetCompany());
		if (Quantity > 0)
		{
			FleetQuantity += Quantity;
			UsefullShips.Add(Ship);
		}

		// TODO sort by most pertinent
	}

	if (FleetQuantity == 0)
	{
		// Fleet empty: operation done
		StatFail = true;
		UpdateOperationStats();
		return true;
	}


	SectorHelper::FlareTradeRequest Request;
	Request.Resource = Resource;
	Request.Operation = Operation->Type;
	Request.CargoLimit = -1;
	Request.MaxQuantity = Operation->MaxQuantity;
	Request.AllowStorage = Operation->CanTradeWithStorages;
	Request.LoadUnloadPriority = Operation->LoadUnloadPriority;
	Request.BuySellPriority = Operation->BuySellPriority;
	Request.IsDonation = Operation->CanDonate;

	for (UFlareSimulatedSpacecraft* Ship : UsefullShips)
	{

		if (Ship->IsTrading())
		{
			// Skip trading ships
			continue;
		}
		
		Request.Client = Ship;
		Request.MaxQuantity = Ship->GetActiveCargoBay()->GetResourceQuantity(Resource, Ship->GetCompany());
		if (Operation->MaxQuantity !=-1)
		{
			Request.MaxQuantity = FMath::Min(Request.MaxQuantity, GetOperationRemainingQuantity(Operation));
		}

		if (Operation->InventoryLimit !=-1)
		{
			Request.MaxQuantity = FMath::Min(Request.MaxQuantity, GetOperationRemainingInventoryQuantity(Operation));
		}

		UFlareSimulatedSpacecraft* StationCandidate = SectorHelper::FindTradeStation(Request);

		if (StationCandidate)
		{
			int64 TransactionPrice;
			int32 Quantity = SectorHelper::Trade(Ship, StationCandidate, Resource, Request.MaxQuantity, &TransactionPrice, this, Operation->CanDonate);
			TradeRouteData.CurrentOperationProgress += Quantity;

			if(!Operation->CanDonate)
			{
				TradeRouteData.StatsMoneySell += TransactionPrice;
			}

			if (TradeRouteCompany == GetGame()->GetPC()->GetCompany())
			{
				Game->GetQuestManager()->OnEvent(FFlareBundle().PutTag("trade-route-transaction").PutInt32("money-variation", TransactionPrice));
			}

			TradeRouteData.StatsUnloadResources += Quantity;

			if(Quantity == 0)
			{
				StatFail = true;
			}
		}
		else
		{
			StatFail = true;
		}

		if (IsOperationQuantityLimitReach(Operation) || IsOperationInventoryLimitReach(Operation))
		{
			// Operation limit reach : operation done
			UpdateOperationStats();
			return true;
		}
	}

	// Limit not reach and useful ship present. Operation not finished
	UpdateOperationStats();
	return false;
}

int32 UFlareTradeRoute::GetOperationRemainingQuantity(FFlareTradeRouteSectorOperationSave* Operation)
{
	if (Operation->MaxQuantity == -1)
	{
		return MAX_int32;
	}

	return Operation->MaxQuantity - TradeRouteData.CurrentOperationProgress;
}

bool UFlareTradeRoute::IsOperationQuantityLimitReach(FFlareTradeRouteSectorOperationSave* Operation)
{
	if (Operation->MaxQuantity != -1 && TradeRouteData.CurrentOperationProgress >= Operation->MaxQuantity)
	{
		return true;
	}
	return false;
}

int32 UFlareTradeRoute::GetOperationRemainingInventoryQuantity(FFlareTradeRouteSectorOperationSave* Operation)
{
	check(Operation->InventoryLimit != -1);

	FFlareResourceDescription* Resource = Game->GetResourceCatalog()->Get(Operation->ResourceIdentifier);
	int32 ResourceQuantity = TradeRouteFleet->GetFleetResourceQuantity(Resource);

	if(IsLoadKindOperation(Operation->Type))
	{
		return Operation->InventoryLimit - ResourceQuantity;
	}
	else
	{
		return ResourceQuantity - Operation->InventoryLimit;
	}
}

bool UFlareTradeRoute::IsOperationInventoryLimitReach(FFlareTradeRouteSectorOperationSave* Operation)
{
	if(Operation->InventoryLimit == -1)
	{
		return false;
	}
	else
	{
		return GetOperationRemainingInventoryQuantity(Operation) <= 0;
	}
}

void UFlareTradeRoute::SetTargetSector(UFlareSimulatedSector* Sector)
{
//	FName PreviousTargetSectorIdentifier* = TradeRouteData.TargetSectorIdentifier;
	if (Sector)
	{
		TradeRouteData.TargetSectorIdentifier = Sector->GetIdentifier();
	}
	else
	{
		TradeRouteData.TargetSectorIdentifier = NAME_None;
	}

//	if (PreviousTargetSectorIdentifier != TradeRouteData.TargetSectorIdentifier)
	{
		TradeRouteData.CurrentOperationDuration = 0;
		TradeRouteData.CurrentOperationIndex = 0;
		TradeRouteData.CurrentOperationProgress = 0;
	}
}


void UFlareTradeRoute::AssignFleet(UFlareFleet* Fleet)
{
	UFlareTradeRoute* OldTradeRoute = Fleet->GetCurrentTradeRoute();
	if (OldTradeRoute)
	{
		OldTradeRoute->RemoveFleet(Fleet);
	}

	TradeRouteData.FleetIdentifier = Fleet->GetIdentifier();
	TradeRouteFleet = Fleet;
	Fleet->SetCurrentTradeRoute(this);

	Game->GetQuestManager()->OnEvent(FFlareBundle().PutTag("assign-fleet"));
}

void UFlareTradeRoute::RemoveFleet(UFlareFleet* Fleet)
{
	TradeRouteData.FleetIdentifier = NAME_None;
	TradeRouteFleet = NULL;
	Fleet->SetCurrentTradeRoute(NULL);
}

void UFlareTradeRoute::ChangeSector(UFlareSimulatedSector* OldSector, UFlareSimulatedSector* NewSector)
{
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		FFlareTradeRouteSectorSave& TradeRouteSector = TradeRouteData.Sectors[SectorIndex];

		if (TradeRouteSector.SectorIdentifier == OldSector->GetIdentifier())
		{
			TradeRouteSector.SectorIdentifier = NewSector->GetIdentifier();
			return;
		}
	}
}

void UFlareTradeRoute::AddSector(UFlareSimulatedSector* Sector)
{
    if (IsVisiting(Sector))
    {
        FLOG("Warning: try to add a sector already visited by the trade route");
        return;
    }

	FFlareTradeRouteSectorSave TradeRouteSector;
	TradeRouteSector.SectorIdentifier = Sector->GetIdentifier();

	TradeRouteData.Sectors.Add(TradeRouteSector);
	if(TradeRouteData.Sectors.Num() == 1)
	{
		SetTargetSector(Sector);
	}

	Game->GetQuestManager()->OnEvent(FFlareBundle().PutTag("trade-route-sector-add"));

}

void UFlareTradeRoute::RemoveSector(UFlareSimulatedSector* Sector)
{
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		if (TradeRouteData.Sectors[SectorIndex].SectorIdentifier == Sector->GetIdentifier())
		{
			TradeRouteData.Sectors.RemoveAt(SectorIndex);
			return;
		}
	}
}

void UFlareTradeRoute::ReplaceSector(UFlareSimulatedSector* Sector, UFlareSimulatedSector* NewSector)
{
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		if (TradeRouteData.Sectors[SectorIndex].SectorIdentifier == Sector->GetIdentifier())
		{
			TradeRouteData.Sectors[SectorIndex].SectorIdentifier = NewSector->GetIdentifier();
			return;
		}
	}
}

void UFlareTradeRoute::MoveSectorUp(UFlareSimulatedSector* Sector)
{
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		if (TradeRouteData.Sectors[SectorIndex].SectorIdentifier == Sector->GetIdentifier())
		{
			if(SectorIndex > 0)
			{
				TArray<FFlareTradeRouteSectorOperationSave*> GatheredOperations = GatherAllOperations();
				TArray<FFlareTradeRouteSectorOperationSave*> FoundLinkedGotoOperations;
				for (FFlareTradeRouteSectorOperationSave* AllOperations : GatheredOperations)
				{
					TArray<FFlareTradeRouteSectorOperationSave*> FoundLinkedGotoOperationsCheck = FindLinkedGotoOperations(AllOperations);
					if (FoundLinkedGotoOperationsCheck.Num() > 0)
					{
						FoundLinkedGotoOperations += FoundLinkedGotoOperationsCheck;
					}
				}

				int32 SwapIndex = SectorIndex - 1;

				for (FFlareTradeRouteSectorOperationSave* CheckingOperation : FoundLinkedGotoOperations)
				{
					if (SectorIndex == CheckingOperation->GotoSectorIndex)
					{
						CheckingOperation->GotoSectorIndex = SwapIndex;
					}
					else if (SectorIndex == CheckingOperation->GotoSectorIndex + 1)
					{
						CheckingOperation->GotoSectorIndex = SectorIndex;
					}
				}

				FFlareTradeRouteSectorSave SwapTradeRouteSector = TradeRouteData.Sectors[SwapIndex];
				TradeRouteData.Sectors[SwapIndex] = TradeRouteData.Sectors[SectorIndex];
				TradeRouteData.Sectors[SectorIndex] = SwapTradeRouteSector;
			}
			return;
		}
	}
}

void UFlareTradeRoute::MoveSectorDown(UFlareSimulatedSector* Sector)
{
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		if (TradeRouteData.Sectors[SectorIndex].SectorIdentifier == Sector->GetIdentifier())
		{
			if(SectorIndex < TradeRouteData.Sectors.Num() - 1)
			{

				TArray<FFlareTradeRouteSectorOperationSave*> GatheredOperations = GatherAllOperations();
				TArray<FFlareTradeRouteSectorOperationSave*> FoundLinkedGotoOperations;
				for (FFlareTradeRouteSectorOperationSave* AllOperations : GatheredOperations)
				{
					TArray<FFlareTradeRouteSectorOperationSave*> FoundLinkedGotoOperationsCheck = FindLinkedGotoOperations(AllOperations);
					if (FoundLinkedGotoOperationsCheck.Num() > 0)
					{
						FoundLinkedGotoOperations += FoundLinkedGotoOperationsCheck;
					}
				}

				int32 SwapIndex = SectorIndex + 1;
				for (FFlareTradeRouteSectorOperationSave* CheckingOperation : FoundLinkedGotoOperations)
				{
					if (SectorIndex == CheckingOperation->GotoSectorIndex)
					{
						CheckingOperation->GotoSectorIndex = SwapIndex;
					}
					else if (SectorIndex == CheckingOperation->GotoSectorIndex - 1)
					{
						CheckingOperation->GotoSectorIndex = SectorIndex;
					}
				}

				FFlareTradeRouteSectorSave SwapTradeRouteSector = TradeRouteData.Sectors[SwapIndex];

				TradeRouteData.Sectors[SwapIndex] = TradeRouteData.Sectors[SectorIndex];
				TradeRouteData.Sectors[SectorIndex] = SwapTradeRouteSector;
			}
			return;
		}
	}
}

FFlareTradeRouteSectorOperationSave* UFlareTradeRoute::AddSectorOperation(int32 SectorIndex, EFlareTradeRouteOperation::Type Type, FFlareResourceDescription* Resource)
{
	if (SectorIndex >= TradeRouteData.Sectors.Num())
	{
		FLOGV("Fail to configure trade route '%s', sector %d: only %d sector present.", *GetTradeRouteName().ToString(), SectorIndex, TradeRouteData.Sectors.Num());
		return NULL;
	}
// Operation Defaults
	FFlareTradeRouteSectorSave* Sector = &TradeRouteData.Sectors[SectorIndex];
	FFlareTradeRouteSectorOperationSave Operation;
	Operation.Type = Type;
	Operation.ResourceIdentifier = Resource->Identifier;

	Operation.GotoSectorIndex = -1;
	Operation.GotoOperationIndex = -1;
	Operation.MaxQuantity = -1;
	Operation.MaxWait = -1;
	Operation.InventoryLimit = -1;

	Operation.LoadUnloadPriority = 10.f;
	Operation.BuySellPriority = 1.f;

	if (Operation.Type == EFlareTradeRouteOperation::Maintenance)
	{
		//Maintenance TradeWith = Repair, Donate = Rearm
		Operation.CanTradeWithStorages = true;
		Operation.CanDonate = true;
	}
	else
	{
		Operation.CanTradeWithStorages = false;
		Operation.CanDonate = false;
	}

	Sector->Operations.Add(Operation);

	return &Sector->Operations.Last();
}

void UFlareTradeRoute::RemoveOperationCondition(FFlareTradeRouteSectorOperationSave* Operation, FFlareTradeRouteOperationConditionSave* Condition)
{
	if (Operation && Condition)
	{
		for (int32 ConditionsIndex = 0; ConditionsIndex < Operation->OperationConditions.Num(); ConditionsIndex++)
		{
			if (&Operation->OperationConditions[ConditionsIndex] == Condition)
			{
				Operation->OperationConditions.RemoveAt(ConditionsIndex);
				return;
			}
		}
	}
}

void UFlareTradeRoute::AddOperationCondition(FFlareTradeRouteSectorOperationSave* Operation, EFlareTradeRouteOperationConditions::Type ConditionType)
{
	if (Operation)
	{
		FFlareTradeRouteOperationConditionSave NewOperationCondition;
		NewOperationCondition.ConditionRequirement = ConditionType;
		NewOperationCondition.ConditionPercentage = 0;

		if (ConditionType == EFlareTradeRouteOperationConditions::PercentOfTimes)
		{
			NewOperationCondition.ConditionPercentage = 0.50f;
		}
		else if (ConditionType == EFlareTradeRouteOperationConditions::LoadPercentage)
		{
			NewOperationCondition.ConditionPercentage = 0.50f;
			NewOperationCondition.SkipOnConditionFail = true;
		}
		else if (ConditionType == EFlareTradeRouteOperationConditions::RequiresMaintenance)
		{
			NewOperationCondition.BooleanOne = true;
			NewOperationCondition.BooleanTwo = false;
			NewOperationCondition.BooleanThree = true;
			NewOperationCondition.SkipOnConditionFail = true;
		}
		else if (ConditionType == EFlareTradeRouteOperationConditions::AtWar)
		{
			NewOperationCondition.BooleanOne = true;
			NewOperationCondition.SkipOnConditionFail = true;
		}

		Operation->OperationConditions.Add(NewOperationCondition);
	}
}

void UFlareTradeRoute::RemoveSectorOperation(int32 SectorIndex, int32 OperationIndex)
{
	if (SectorIndex >= TradeRouteData.Sectors.Num())
	{
		FLOGV("Fail to configure trade route '%s', sector %d: only %d sector present.", *GetTradeRouteName().ToString(), SectorIndex, TradeRouteData.Sectors.Num());
		return;
	}

	FFlareTradeRouteSectorSave* Sector = &TradeRouteData.Sectors[SectorIndex];

	Sector->Operations.RemoveAt(OperationIndex);
}

void UFlareTradeRoute::DeleteOperation(FFlareTradeRouteSectorOperationSave* Operation)
{
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		FFlareTradeRouteSectorSave* Sector = &TradeRouteData.Sectors[SectorIndex];

		for (int32 OperationIndex = 0; OperationIndex < Sector->Operations.Num(); OperationIndex++)
		{
			if(&Sector->Operations[OperationIndex] == Operation)
			{
				TArray<FFlareTradeRouteSectorOperationSave*> FoundLinkedGotoOperations = FindLinkedGotoOperations(Operation);
				for (FFlareTradeRouteSectorOperationSave* CheckingOperation : FoundLinkedGotoOperations)
				{
					CheckingOperation->GotoSectorIndex = - 1;
					CheckingOperation->GotoOperationIndex = -1;
				}

				if(Operation == GetActiveOperation())
				{
					// Active operation, reset progress
					TradeRouteData.CurrentOperationDuration = 0;
					TradeRouteData.CurrentOperationProgress = 0;
				}
				else if(TradeRouteData.TargetSectorIdentifier == Sector->SectorIdentifier && TradeRouteData.CurrentOperationIndex > OperationIndex)
				{
					// Modify the current operation index
					TradeRouteData.CurrentOperationIndex--;
				}
				Sector->Operations.RemoveAt(OperationIndex);
				return;
			}
		}
	}
}

TArray<FFlareTradeRouteSectorOperationSave*> UFlareTradeRoute::FindLinkedGotoOperations(FFlareTradeRouteSectorOperationSave* FromOperation)
{
	TArray<FFlareTradeRouteSectorOperationSave*> FoundOperations;
	for (int SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		FFlareTradeRouteSectorSave* SectorOrders = &TradeRouteData.Sectors[SectorIndex];

		for (int OperationIndex = 0; OperationIndex < SectorOrders->Operations.Num(); OperationIndex++)
		{
			FFlareTradeRouteSectorOperationSave* CheckingOperation = &SectorOrders->Operations[OperationIndex];
			if (CheckingOperation->GotoSectorIndex != -1 && CheckingOperation->GotoOperationIndex != -1)
			{
				if (FromOperation)
				{
					if (CheckingOperation->GotoSectorIndex < TradeRouteData.Sectors.Num())
					{
						FFlareTradeRouteSectorSave* CheckingSector = &TradeRouteData.Sectors[CheckingOperation->GotoSectorIndex];
						if (CheckingOperation->GotoOperationIndex < CheckingSector->Operations.Num())
						{
							if (&CheckingSector->Operations[CheckingOperation->GotoOperationIndex] == FromOperation)
							{
								FoundOperations.Add(CheckingOperation);
							}
						}
					}
				}
				else
				{
					FoundOperations.Add(CheckingOperation);
				}
			}
		}
	}
	return FoundOperations;
}

bool UFlareTradeRoute::CanMoveOperationUp(FFlareTradeRouteSectorOperationSave* Operation)
{
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		FFlareTradeRouteSectorSave* Sector = &TradeRouteData.Sectors[SectorIndex];

		for (int32 OperationIndex = 0; OperationIndex < Sector->Operations.Num(); OperationIndex++)
		{
			if (&Sector->Operations[OperationIndex] == Operation)
			{
				if (OperationIndex == 0)
				{
					//Already on top
					return false;
				}
			}
		}
	}
	return true;
}

bool UFlareTradeRoute::CanMoveOperationDown(FFlareTradeRouteSectorOperationSave* Operation)
{
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		FFlareTradeRouteSectorSave* Sector = &TradeRouteData.Sectors[SectorIndex];

		for (int32 OperationIndex = 0; OperationIndex < Sector->Operations.Num(); OperationIndex++)
		{
			if (&Sector->Operations[OperationIndex] == Operation)
			{
				if (OperationIndex == Sector->Operations.Num() - 1)
				{
					//Already on bottom
					return false;
				}
			}
		}
	}
	return true;
}

int32 UFlareTradeRoute::MoveOperationUp(FFlareTradeRouteSectorOperationSave* Operation)
{
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		FFlareTradeRouteSectorSave* Sector = &TradeRouteData.Sectors[SectorIndex];

		for (int32 OperationIndex = 0; OperationIndex < Sector->Operations.Num(); OperationIndex++)
		{
			if(&Sector->Operations[OperationIndex] == Operation)
			{
				if(OperationIndex == 0)
				{
					//Already on top
					return OperationIndex;
				}

				TArray<FFlareTradeRouteSectorOperationSave*> FoundLinkedGotoOperations = FindLinkedGotoOperations(Operation);

				if(Operation == GetActiveOperation())
				{
					// Active operation, modify the current operation index
					TradeRouteData.CurrentOperationIndex--;
				}
				else if(TradeRouteData.TargetSectorIdentifier == Sector->SectorIdentifier && TradeRouteData.CurrentOperationIndex == OperationIndex-1)
				{
					TradeRouteData.CurrentOperationIndex++;
				}

				for (FFlareTradeRouteSectorOperationSave* CheckingOperation : FoundLinkedGotoOperations)
				{
					CheckingOperation->GotoOperationIndex--;
				}

				FFlareTradeRouteSectorOperationSave NewOperation = *Operation;
				Sector->Operations.RemoveAt(OperationIndex);
				Sector->Operations.Insert(NewOperation, OperationIndex-1);
				return OperationIndex-1;
			}
		}
	}

	return -1;
}

int32 UFlareTradeRoute::MoveOperationDown(FFlareTradeRouteSectorOperationSave* Operation)
{
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		FFlareTradeRouteSectorSave* Sector = &TradeRouteData.Sectors[SectorIndex];

		for (int32 OperationIndex = 0; OperationIndex < Sector->Operations.Num(); OperationIndex++)
		{
			if(&Sector->Operations[OperationIndex] == Operation)
			{
				if(OperationIndex == Sector->Operations.Num()-1)
				{
					//Already on bottom
					return OperationIndex;
				}

				TArray<FFlareTradeRouteSectorOperationSave*> FoundLinkedGotoOperations = FindLinkedGotoOperations(Operation);

				if(Operation == GetActiveOperation())
				{
					// Active operation, modify the current operation index
					TradeRouteData.CurrentOperationIndex++;
				}
				else if(TradeRouteData.TargetSectorIdentifier == Sector->SectorIdentifier && TradeRouteData.CurrentOperationIndex == OperationIndex+1)
				{
					TradeRouteData.CurrentOperationIndex--;
				}

				for (FFlareTradeRouteSectorOperationSave* CheckingOperation : FoundLinkedGotoOperations)
				{
					CheckingOperation->GotoOperationIndex++;
				}

				FFlareTradeRouteSectorOperationSave NewOperation = *Operation;

				Sector->Operations.RemoveAt(OperationIndex);
				Sector->Operations.Insert(NewOperation, OperationIndex+1);
				return OperationIndex+1;
			}
		}
	}

	return -1;
}

/** Remove all ship from the trade route and delete it.*/
void UFlareTradeRoute::Dissolve()
{
	if (TradeRouteFleet)
	{
		TradeRouteFleet->SetCurrentTradeRoute(NULL);
	}

	TradeRouteCompany->RemoveTradeRoute(this);
}

void UFlareTradeRoute::InitFleetList()
{
	if (!IsFleetListLoaded)
	{
		IsFleetListLoaded = true;
		TradeRouteFleet = NULL;

		if (TradeRouteData.FleetIdentifier != NAME_None)
		{
			TradeRouteFleet = TradeRouteCompany->FindFleet(TradeRouteData.FleetIdentifier);
			if (TradeRouteFleet)
			{
				TradeRouteFleet->SetCurrentTradeRoute(this);
			}
			else
			{
				FLOGV("WARNING: Fail to find fleet '%s'. Save corrupted", *TradeRouteData.FleetIdentifier.ToString());
				TradeRouteData.FleetIdentifier = NAME_None;
			}
		}
	}
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

TArray<FFlareTradeRouteSectorOperationSave*> UFlareTradeRoute::GatherAllOperations()
{
	TArray<FFlareTradeRouteSectorOperationSave*> FoundOperations;
	for (int SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		FFlareTradeRouteSectorSave* SectorOrders = &TradeRouteData.Sectors[SectorIndex];

		for (int OperationIndex = 0; OperationIndex < SectorOrders->Operations.Num(); OperationIndex++)
		{
			FFlareTradeRouteSectorOperationSave* CheckingOperation = &SectorOrders->Operations[OperationIndex];
			FoundOperations.Add(CheckingOperation);
		}
	}
	return FoundOperations;
}

UFlareFleet* UFlareTradeRoute::GetFleet()
{
	InitFleetList();

	return TradeRouteFleet;
}

FFlareTradeRouteSectorSave* UFlareTradeRoute::GetSectorOrders(UFlareSimulatedSector* Sector)
{
	for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		if (TradeRouteData.Sectors[SectorIndex].SectorIdentifier == Sector->GetIdentifier())
		{
			return &TradeRouteData.Sectors[SectorIndex];
		}
	}

	return NULL;
}

UFlareSimulatedSector* UFlareTradeRoute::GetNextTradeSector(UFlareSimulatedSector* Sector)
{
	if(TradeRouteData.Sectors.Num() == 0)
	{
		return NULL;
	}

	int32 NextSectorId = -1;
	if(Sector)
	{
		for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
		{
			if(TradeRouteData.Sectors[SectorIndex].SectorIdentifier == Sector->GetIdentifier())
			{
				NextSectorId = SectorIndex + 1;
				break;
			}
		}
	}

	if (NextSectorId < 0)
	{
        NextSectorId = 0;
	}


	if (NextSectorId >= TradeRouteData.Sectors.Num())
	{
		NextSectorId = 0;
	}
	return Game->GetGameWorld()->FindSector(TradeRouteData.Sectors[NextSectorId].SectorIdentifier);
}

bool UFlareTradeRoute::IsUsefulSector(UFlareSimulatedSector* Sector, int StartingOperationIndex)
{
	FFlareTradeRouteSectorSave* SectorOrder = GetSectorOrders(Sector);

	auto IsUseful = [](FFlareResourceUsage Usage, EFlareTradeRouteOperation::Type OperationType, bool Owned, float LoadUnloadPriority, float BuySellPriority)
	{
		if (Owned && (LoadUnloadPriority < 1.f))
		{
			return false;
		}

		if (!Owned && (BuySellPriority < 1.f))
		{
			return false;
		}

		bool LoadOperation = IsLoadKindOperation(OperationType);

		if(LoadOperation && (Usage.HasUsage(EFlareResourcePriceContext::FactoryOutput) || Usage.HasUsage(EFlareResourcePriceContext::HubOutput)))
		{
			return true;
		}

		bool UnloadOperation = IsUnloadKindOperation(OperationType);

		if(UnloadOperation && (Usage.HasUsage(EFlareResourcePriceContext::FactoryInput)
							   || Usage.HasUsage(EFlareResourcePriceContext::HubInput)
							   || Usage.HasUsage(EFlareResourcePriceContext::MaintenanceConsumption)
							   || Usage.HasUsage(EFlareResourcePriceContext::ConsumerConsumption)))
		{
			return true;
		}

		return false;
	};

	for (int OperationIndex = StartingOperationIndex; OperationIndex < SectorOrder->Operations.Num(); OperationIndex++)
	{
		FFlareTradeRouteSectorOperationSave* Operation = &SectorOrder->Operations[OperationIndex];
		if (Operation->Type == EFlareTradeRouteOperation::GotoOperation)
		{
			if (Operation->GotoSectorIndex != -1 && Operation->GotoOperationIndex != -1)
			{
				return false;
			}
			continue;
		}
		else if (Operation->Type == EFlareTradeRouteOperation::Maintenance)
		{
			int32 AvailableFS;
			int32 OwnedFS;
			int32 AffordableFS;
			bool CheckedAvailableFleetSupplyCount = false;

			// Repair
			if (Operation->CanTradeWithStorages)
			{
				if (TradeRouteFleet->FleetNeedsRepair())
				{
					if (!CheckedAvailableFleetSupplyCount)
					{
						SectorHelper::GetAvailableFleetSupplyCount(Sector, TradeRouteFleet->GetFleetCompany(), OwnedFS, AvailableFS, AffordableFS, nullptr, TradeRouteFleet);
						CheckedAvailableFleetSupplyCount = true;
					}
					if (AffordableFS > 0)
					{
						return true;
					}
				}
			}
			// Rearm
			if (Operation->CanDonate)
			{
				if (TradeRouteFleet->FleetNeedsRefill())
				{
					if (!CheckedAvailableFleetSupplyCount)
					{
						SectorHelper::GetAvailableFleetSupplyCount(Sector, TradeRouteFleet->GetFleetCompany(), OwnedFS, AvailableFS, AffordableFS, nullptr, TradeRouteFleet);
						CheckedAvailableFleetSupplyCount = true;
					}
					if (AffordableFS > 0)
					{
						return true;
					}
				}
			}
			continue;
		}

		//LOAD/UNLOAD Operation checks below
		FFlareResourceDescription* Resource = Game->GetResourceCatalog()->Get(Operation->ResourceIdentifier);
		bool UnloadOperation = (Operation->Type == EFlareTradeRouteOperation::Unload);
		bool LoadOperation = (Operation->Type == EFlareTradeRouteOperation::Load);

		if(UnloadOperation)
		{
			int32 ResourceCount = TradeRouteFleet->GetFleetResourceQuantity(Resource);
			if (ResourceCount == 0)
			{
				// Cannot be useful because nothing to exchange
				continue;
			}
		}
		if (LoadOperation)
		{
			int32 FleetFreeSpaceForResource = TradeRouteFleet->GetFleetFreeSpaceForResource(Resource);
			if (FleetFreeSpaceForResource == 0)
			{
				// Cannot be useful because no more free space
				continue;
			}
		}

		// Find if there is a station to exchange
		for(UFlareSimulatedSpacecraft* Station : Sector->GetSectorStations())
		{
			if (Station->IsHostile(TradeRouteCompany))
			{
				continue;
			}

			FFlareResourceUsage Usage = Station->GetResourceUseType(Resource);
			if(IsUseful(Usage, Operation->Type, TradeRouteCompany == Station->GetCompany(), Operation->LoadUnloadPriority,Operation->BuySellPriority))
			{
				return true;
			}
		}
	}

	return false;
}

bool UFlareTradeRoute::IsVisiting(UFlareSimulatedSector *Sector)
{
    for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
    {
        if (TradeRouteData.Sectors[SectorIndex].SectorIdentifier == Sector->GetIdentifier())
        {
            return true;
		}
    }
    return false;
}

int32 UFlareTradeRoute::GetSectorIndex(UFlareSimulatedSector *Sector)
{
    for (int32 SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
    {
        if (TradeRouteData.Sectors[SectorIndex].SectorIdentifier == Sector->GetIdentifier())
        {
            return SectorIndex;
        }
    }
    return -1;
}

int32 UFlareTradeRoute::GetOperationIndex(FFlareTradeRouteSectorOperationSave* Operation,bool ReturnOperationPosition)
{
	TArray<FFlareTradeRouteSectorOperationSave*> FoundOperations;
	for (int SectorIndex = 0; SectorIndex < TradeRouteData.Sectors.Num(); SectorIndex++)
	{
		FFlareTradeRouteSectorSave* SectorOrders = &TradeRouteData.Sectors[SectorIndex];

		for (int OperationIndex = 0; OperationIndex < SectorOrders->Operations.Num(); OperationIndex++)
		{
			FFlareTradeRouteSectorOperationSave* CheckingOperation = &SectorOrders->Operations[OperationIndex];
			if (CheckingOperation == Operation)
			{
				if (ReturnOperationPosition)
				{
					return OperationIndex;
				}
				else
				{
					return SectorIndex;
				}
			}
		}
	}
	return -1;
}

UFlareSimulatedSector* UFlareTradeRoute::GetTargetSector() const
{
	return Game->GetGameWorld()->FindSector(TradeRouteData.TargetSectorIdentifier);
}

FFlareTradeRouteSectorOperationSave* UFlareTradeRoute::GetActiveOperation()
{
	UFlareSimulatedSector* TargetSector = Game->GetGameWorld()->FindSector(TradeRouteData.TargetSectorIdentifier);

	if(TargetSector == NULL)
	{
		return NULL;
	}

	FFlareTradeRouteSectorSave* SectorOrder = GetSectorOrders(TargetSector);

	if(SectorOrder == NULL)
	{
		return NULL;
	}

	if(SectorOrder->Operations.Num() == 0)
	{
		return NULL;
	}

	if(TradeRouteData.CurrentOperationIndex >= SectorOrder->Operations.Num())
	{
		return NULL;
	}

	return &SectorOrder->Operations[TradeRouteData.CurrentOperationIndex];
}

void UFlareTradeRoute::SkipCurrentOperation()
{
	UFlareSimulatedSector* TargetSector = UpdateTargetSector();
	TradeRouteData.CurrentOperationDuration = 0;
	TradeRouteData.CurrentOperationProgress = 0;
	TradeRouteData.CurrentOperationIndex++;

	FFlareTradeRouteSectorSave* SectorOrder = GetSectorOrders(TargetSector);

	if (SectorOrder && TradeRouteData.CurrentOperationIndex >= SectorOrder->Operations.Num())
	{
		TargetSector = GetNextTradeSector(TargetSector);
		SetTargetSector(TargetSector);
	}
}

void UFlareTradeRoute::ResetStats()
{
	TradeRouteData.StatsDays = 0;
	TradeRouteData.StatsLoadResources = 0;
	TradeRouteData.StatsUnloadResources = 0;
	TradeRouteData.StatsMoneySell = 0;
	TradeRouteData.StatsMoneyBuy = 0;
	TradeRouteData.StatsOperationSuccessCount = 0;
	TradeRouteData.StatsOperationFailCount = 0;
}
#undef LOCTEXT_NAMESPACE