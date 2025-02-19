
#include "FlareCompanyAI.h"
#include "../../Flare.h"
#include "FlareAIBehavior.h"
#include "FlareAITradeHelper.h"

#include "../FlareGame.h"
#include "../FlareGameTools.h"
#include "../FlareCompany.h"
#include "../FlareSectorHelper.h"
#include "../FlareScenarioTools.h"

#include "../../Data/FlareResourceCatalog.h"
#include "../../Data/FlareFactoryCatalogEntry.h"
#include "../../Data/FlareTechnologyCatalog.h"
#include "../../Data/FlareSpacecraftCatalog.h"
#include "../../Data/FlareSpacecraftComponentsCatalog.h"

#include "../../Economy/FlareFactory.h"
#include "../../Economy/FlareCargoBay.h"

#include "../../Player/FlarePlayerController.h"

#include "../../Quests/FlareQuestManager.h"
#include "../../Quests/FlareQuestGenerator.h"

#include "../../Spacecrafts/FlareSimulatedSpacecraft.h"

#define AI_MAX_SHIP_COUNT 200

// If one cargo out of X ships is wrecked, the fleet is unhealthy
#define AI_CARGO_HEALTHY_THRESHOLD 5
#define AI_TRADESHIP_UPGRADE_ENGINE_CHANCE 0.80
#define AI_TRADESHIP_UPGRADE_RCS_CHANCE 0.20

//#define AI_DEBUG_AUTOSCRAP
//#define DEBUG_AI_WAR_MILITARY_MOVEMENT
//#define DEBUG_AI_BATTLE_STATES
//#define DEBUG_AI_BUDGET
//#define DEBUG_AI_SHIP_ORDER

#define LOCTEXT_NAMESPACE "FlareCompanyAI"

DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI UpdateDiplomacy"), STAT_FlareCompanyAI_UpdateDiplomacy, STATGROUP_Flare);

DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI UpdateTrading"), STAT_FlareCompanyAI_UpdateTrading, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI UpdateTrading Ships"), STAT_FlareCompanyAI_UpdateTrading_Ships, STATGROUP_Flare);

DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI CargosEvasion"), STAT_FlareCompanyAI_CargosEvasion, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI RepairAndRefill"), STAT_FlareCompanyAI_RepairAndRefill, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI ProcessBudget"), STAT_FlareCompanyAI_ProcessBudget, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareCompanyAI UpdateMilitaryMovement"), STAT_FlareCompanyAI_UpdateMilitaryMovement, STATGROUP_Flare);


/*----------------------------------------------------
	Public API
----------------------------------------------------*/

UFlareCompanyAI::UFlareCompanyAI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AllBudgets.Reserve(4);
	AllBudgets.Add(EFlareBudget::Trade);
	AllBudgets.Add(EFlareBudget::Military);
	AllBudgets.Add(EFlareBudget::Station);
	AllBudgets.Add(EFlareBudget::Technology);
}

void UFlareCompanyAI::Load(UFlareCompany* ParentCompany, const FFlareCompanyAISave& Data)
{
// note: Load called from Company PostLoad() which is called either after the company has been created or after everything in the world has been fully initialized
	Company = ParentCompany;
	Game = Company->GetGame();
	AIData = Data;

	// Setup Behavior
	Behavior = NewObject<UFlareAIBehavior>(this, UFlareAIBehavior::StaticClass());

	UnderConstructionStations.Empty();
	UnderConstructionUpgradeStations.Empty();
	for (UFlareSimulatedSpacecraft* Station : Company->GetCompanyStations())
	{
		TrackStationConstructionStatus(Station);
	}

	if (Company->AtWar())
	{
		WasAtWar = true;
	}
}

void UFlareCompanyAI::TrackStationConstructionStatus(UFlareSimulatedSpacecraft* Station)
{
	if (Station->IsUnderConstruction())
	{
		if (Station->GetLevel() > 1)
		{
			UnderConstructionUpgradeStations.Add(Station);
		}
		else if (Station->IsComplex())
		{
			bool UnderConstructionUpgrade = false;
			for (UFlareSimulatedSpacecraft* Substation : Station->GetComplexChildren())
			{
				if (Substation->IsUnderConstruction(true))
				{
					if (Substation->GetLevel() > 1)
					{
						{
							UnderConstructionUpgradeStations.Add(Station);
							UnderConstructionUpgrade = true;
							break;
						}
					}
					else
					{
						UnderConstructionStations.Add(Station);
					}
				}
			}
			if (!UnderConstructionUpgrade)
			{
				UnderConstructionStations.Add(Station);
			}
		}
		else
		{
			UnderConstructionStations.Add(Station);
		}
	}
}

void UFlareCompanyAI::CapturedStation(UFlareSimulatedSpacecraft* CapturedStation)
{
	if (CapturedStation)
	{
		TrackStationConstructionStatus(CapturedStation);
	}
}

void UFlareCompanyAI::FinishedConstruction(UFlareSimulatedSpacecraft* FinishedStation)
{
	if (FinishedStation)
	{
		UnderConstructionStations.RemoveSwap(FinishedStation);
		UnderConstructionUpgradeStations.RemoveSwap(FinishedStation);
	}
}

FFlareCompanyAISave* UFlareCompanyAI::Save()
{
	return &AIData;
}

void UFlareCompanyAI::SetActiveAIDesireRepairs(bool NewValue)
{
	ActiveAIDesireRepairs = NewValue;
}

void UFlareCompanyAI::NewSectorLoaded()
{
	ActiveAIDesireRepairs = true;
}

//used for AI Company sending orders for their assets/ships within the actively simulated sector
void UFlareCompanyAI::SimulateActiveAI()
{
	if (!Game)
	{
		return;
	}

	if (Company != Game->GetPC()->GetCompany())
	{
		UFlareSector* ActiveSector = Game->GetActiveSector();
		if (ActiveSector&&!ActiveSector->GetIsDestroyingSector())
		{
			UFlareSimulatedSector* SimulatedSector = ActiveSector->GetSimulatedSector();
			if (SimulatedSector)
			{
				if (!SimulatedSector->IsInDangerousBattle(Company))
				{
					TArray<AFlareSpacecraft*> CompanyShips = ActiveSector->GetCompanyShips(Company);
					int32 GameDifficulty = Game->GetPC()->GetPlayerData()->DifficultyId;
					int ActivityAttempts = 1;
					switch (GameDifficulty)
					{
						case -1: // Easy
							ActivityAttempts = 1;
							break;
						case 0: // Normal
							ActivityAttempts = 1;
							break;
						case 1: // Hard
							ActivityAttempts = 2;
							break;
						case 2: // Very Hard
							ActivityAttempts = 3;
							break;
						case 3: // Expert
							ActivityAttempts = 4;
							break;
						case 4: // Unfair
							ActivityAttempts = 5;
							break;
					}

					for (AFlareSpacecraft* Ship : CompanyShips)
					{
						if (Ship)
						{
							if (!Ship->GetParent()->GetActiveCargoBay()->GetCapacity())
							{
								CompanyShips.RemoveSwap(Ship);
							}
/*
							else if(Ship->IsMilitary)
							{
								Addship to military array
								"do military stuff"?
							}
*/
						}
					}

					while (ActivityAttempts > 0 && CompanyShips.Num() > 0)
					{
						ActivityAttempts--;
						int32 ShipIndex = FMath::RandRange(0, CompanyShips.Num() - 1);
						AFlareSpacecraft* Ship = CompanyShips[ShipIndex];
						CompanyShips.RemoveSwap(Ship);

						if (Ship && (Ship->GetNavigationSystem()->IsAutoPilot() || Ship->GetParent()->GetDamageSystem()->IsUncontrollable()))
						{
							continue;
						}

						if (!Ship->GetParent()->GetDescription()->IsDroneCarrier)
						{
							bool TryTradeOrUpgrade = FMath::FRand() < 0.80;
							if (TryTradeOrUpgrade)
							{
								SimulateActiveTradeShip(Ship);
							}
							else
							{
								SimulateActiveShipAttemptUpgrades(Ship);
							}
							break;
						}
					}

					if (ActiveAIDesireRepairs)
					{
						SectorHelper::RepairFleets(SimulatedSector, Company);
						SectorHelper::RefillFleets(SimulatedSector, Company);
						ActiveAIDesireRepairs = false;
					}
				}
			}
		}
	}
}

//Select a single random ship, if it's viable try to find resources to buy/sell. Random searches rather than looking for good profits.
//It's probably not fun for players trying to do their own trades having too much competition for the local sector trade. hence all the possible fail conditions where a company may not send a ship to trade for that minute.
void UFlareCompanyAI::SimulateActiveTradeShip(AFlareSpacecraft* Ship)
{
	UFlareSector* ActiveSector = Game->GetActiveSector();
	if (!ActiveSector)
	{
		return;
	}
	UFlareSimulatedSector* SimulatedSector = ActiveSector->GetSimulatedSector();
	if (!SimulatedSector)
	{
		return;
	}

	bool BreakOutOfLoop = false;

	TArray<UFlareResourceCatalogEntry*> ResourceCatalog = Game->GetResourceCatalog()->Resources;
	bool FoundSomething = false;
	while (ResourceCatalog.Num())
	{
		if (ActiveSector)
		{
			if (ActiveSector->GetIsDestroyingSector())
			{
				break;
			}
		}
		else
		{
			break;
		}

		if (BreakOutOfLoop)
		{
			break;
		}

		int32 ResourceIndex = FMath::RandRange(0, ResourceCatalog.Num() - 1);
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		ResourceCatalog.RemoveAtSwap(ResourceIndex);
		if (!Resource)
		{
			continue;
		}

		int32 InitialQuantity = Ship->GetParent()->GetActiveCargoBay()->GetResourceQuantity(Resource, Ship->GetCompany());
		int32 FreeSpace = Ship->GetParent()->GetActiveCargoBay()->GetFreeSpaceForResource(Resource, Ship->GetCompany());
		TArray<AFlareSpacecraft*> LocalStations = ActiveSector->GetStations();
		FText Unused;

		while (LocalStations.Num())
		{
			if (ActiveSector)
			{
				if (ActiveSector->GetIsDestroyingSector())
				{
					BreakOutOfLoop = true;
					break;
				}
			}
			else
			{
				break;
			}

			int32 StationIndex = FMath::RandRange(0, LocalStations.Num() - 1);
			AFlareSpacecraft* Station = LocalStations[StationIndex];
			LocalStations.RemoveAtSwap(StationIndex);
			if (!Station)
			{
				continue;
			}

			//station wants to buy this resource, so we could potentially sell what the ship has
			if (Station->GetParent()->GetActiveCargoBay()->WantBuy(Resource, Ship->GetCompany()))
			{
				if (!Ship->GetParent()->CanTradeWith(Station->GetParent(), Unused, Resource))
				{
					continue;
				}

				//we already have cargo
				if (InitialQuantity > 0)
				{
					int32 StationFreeSpace = Station->GetParent()->GetActiveCargoBay()->GetFreeSpaceForResource(Resource, Ship->GetCompany());
					if (StationFreeSpace > 0)
					{
						int32 StationMaxAffordableQuantity = 0;

						if (Station->GetCompany() == Ship->GetCompany())
						{
							StationMaxAffordableQuantity = INT_MAX;
						}
						else
						{
							int32 ResourcePrice = Ship->GetParent()->GetCurrentSector()->GetTransfertResourcePrice(Station->GetParent(), Ship->GetParent(), Resource);
							StationMaxAffordableQuantity = FMath::Max(int64(0), Station->GetCompany()->GetMoney()) / ResourcePrice;
						}

						StationMaxAffordableQuantity = FMath::Min(StationMaxAffordableQuantity, InitialQuantity);
						StationMaxAffordableQuantity = FMath::Min(StationMaxAffordableQuantity, StationMaxAffordableQuantity);

						// found a station willing to buy, we'll sell!
						if (StationMaxAffordableQuantity > 0)
						{
							//chance has it we're already docked, sell immediately
							if (Station->GetDockingSystem()->IsDockedShip(Ship))
							{
								SectorHelper::Trade(Ship->GetParent(),
									Station->GetParent(),
									Resource,
									StationMaxAffordableQuantity);
								BreakOutOfLoop = true;
							}
							else
							{
								bool DockingConfirmed = Ship->GetNavigationSystem()->DockAtAndTrade(Station, Resource, StationMaxAffordableQuantity, Ship->GetParent(), Station->GetParent(), false);
								if (DockingConfirmed)
								{
									BreakOutOfLoop = true;
									FoundSomething = true;
								}
							}
							break;
						}
					}
				}
			}
			//station wants to sell this resource, so we could potentially buy what the station has
			if (FreeSpace > 0 && Station->GetParent()->GetActiveCargoBay()->WantSell(Resource, Ship->GetCompany()))
			{
				if (!Station->GetParent()->CanTradeWith(Ship->GetParent(), Unused, Resource))
				{
					continue;
				}

				int32 StationQuantity = Station->GetParent()->GetActiveCargoBay()->GetResourceQuantitySimple(Resource);
				if (StationQuantity > 0)
				{
					int32 ShipMaxAffordableQuantity = 0;

					if (Station->GetCompany() == Ship->GetCompany())
					{
						ShipMaxAffordableQuantity = INT_MAX;
					}
					else
					{
						int32 ResourcePrice = Ship->GetParent()->GetCurrentSector()->GetTransfertResourcePrice(Ship->GetParent(), Station->GetParent(), Resource);
						ShipMaxAffordableQuantity = FMath::Max(int64(0), Ship->GetCompany()->GetMoney()) / ResourcePrice;
					}

					ShipMaxAffordableQuantity = FMath::Min(ShipMaxAffordableQuantity, StationQuantity);
					ShipMaxAffordableQuantity = FMath::Min(ShipMaxAffordableQuantity, FreeSpace);

					// found a station willing to sell, we'll buy!
					if (ShipMaxAffordableQuantity > 0)
					{
						//chance has it we're already docked, sell immediately
						if (Station->GetDockingSystem()->IsDockedShip(Ship))
						{
							SectorHelper::Trade(Station->GetParent(),
							Ship->GetParent(),
							Resource,
							ShipMaxAffordableQuantity);
							BreakOutOfLoop = true;
						}
						else
						{
							bool DockingConfirmed = Ship->GetNavigationSystem()->DockAtAndTrade(Station, Resource, ShipMaxAffordableQuantity, Station->GetParent(), Ship->GetParent(), false);
							if (DockingConfirmed)
							{
								BreakOutOfLoop = true;
								FoundSomething = true;
							}
						}
						break;
					}
				}
			}
		}
	}
}
void UFlareCompanyAI::SimulateActiveShipAttemptUpgrades(AFlareSpacecraft* Ship)
{
	UFlareSector* ActiveSector = Game->GetActiveSector();
	if (!ActiveSector)
	{
		return;
	}
	UFlareSimulatedSector* SimulatedSector = ActiveSector->GetSimulatedSector();
	if (!SimulatedSector)
	{
		return;
	}

	FFlareSectorBattleState BattleState = SimulatedSector->GetSectorBattleState(Company);
	if (!BattleState.HasDanger)
	{
		UFlareSpacecraftComponentsCatalog* Catalog = Game->GetPC()->GetGame()->GetShipPartsCatalog();
		// Look for a station with upgrade capability
		for (int StationIndex = 0; StationIndex < SimulatedSector->GetSectorStations().Num(); StationIndex++)
		{
			UFlareSimulatedSpacecraft* StationInterface = SimulatedSector->GetSectorStations()[StationIndex];
			if (!StationInterface->IsHostile(Company)
				&& StationInterface->HasCapability(EFlareSpacecraftCapability::Upgrade))
			{
				FFlareSpacecraftComponentDescription* OldPart = nullptr;
				FFlareSpacecraftComponentDescription* NewPartDesc = nullptr;

				bool HasChance = FMath::FRand() < AI_TRADESHIP_UPGRADE_ENGINE_CHANCE;
				if (HasChance)
				{
					OldPart = Ship->GetParent()->GetCurrentPart(EFlarePartType::OrbitalEngine, 0);
					TArray< FFlareSpacecraftComponentDescription* > PartListData;
					Catalog->GetEngineList(PartListData, Ship->GetParent()->GetSize(), Ship->GetCompany(), Ship->GetParent());
					NewPartDesc = FindBestEngineOrRCS(PartListData, OldPart, EFlareBudget::Trade, false);
				}

				if (OldPart != NewPartDesc)
				{
					HasChance = FMath::FRand() < AI_TRADESHIP_UPGRADE_RCS_CHANCE;
					if (HasChance)
					{
						OldPart = Ship->GetParent()->GetCurrentPart(EFlarePartType::RCS, 0);
						TArray< FFlareSpacecraftComponentDescription* > PartListData;
						Catalog->GetRCSList(PartListData, Ship->GetParent()->GetSize(), Ship->GetCompany(), Ship->GetParent());
						NewPartDesc = FindBestEngineOrRCS(PartListData, OldPart, EFlareBudget::Trade, true);
					}
				}
		
				if (OldPart != NewPartDesc)
				{
					int64 TransactionCost = Ship->GetParent()->GetUpgradeCost(NewPartDesc, OldPart);
					if (CanSpendBudget(EFlareBudget::Trade, TransactionCost))
					{
						Ship->GetNavigationSystem()->DockAtAndUpgrade(NewPartDesc, 0, StationInterface->GetActive());
					}
				}
			}
		}
	}
}

bool UFlareCompanyAI::IsAboveMinimumMoney()
{
	if (Company->GetMoney() >= GetMinimumMoney())
	{
		return true;
	}
	return false;
}

void UFlareCompanyAI::CreateWorldResourceVariations()
{
	if (CreatedWorldResourceVariations)
	{
		return;
	}

	CreatedWorldResourceVariations = true;
	// Compute input and output ressource equation (ex: 100 + 10/ day)
// TODO
	WorldResourceVariation.Empty();
	WorldResourceVariation.Reserve(Company->GetKnownSectors().Num());
	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];
		SectorVariation Variation = AITradeHelper::ComputeSectorResourceVariation(Company, Sector, true);
		WorldResourceVariation.Add(Sector, Variation);
		//DumpSectorResourceVariation(Sector, &Variation);
	}
}

//Called from Company->SimulateAI()
void UFlareCompanyAI::Simulate(bool GlobalWar, int32 TotalReservedResources)
{
	if (Game && Company != Game->GetPC()->GetCompany())
	{
		CheckedBuildingShips = false;
		CreatedWorldResourceVariations = false;

		GlobalReservedResources = TotalReservedResources;
		Behavior->Load(Company);

		AutoScrap();
		CheckBattleResolution();
		UpdateDiplomacy(GlobalWar);

		WorldStats = WorldHelper::ComputeWorldResourceStats(Game, true);
		Shipyards = GetGame()->GetGameWorld()->GetShipyardsFor(Company);
		UndiscoveredSectors = Company->GetUndiscoveredSectors();
		CanUpgradeSectors.Empty();

		if (!AIData.CalculatedDefaultBudget)
		{
			AIData.CalculatedDefaultBudget = true;
			RecalculateFullBudgets();
		}

		if (!Behavior->FinishedResearch)
		{
			const FFlareCompanyDescription* CompanyDescription = Company->GetDescription();
			if (CompanyDescription->PassiveResearchGeneration)
			{
				Company->GiveResearch(CompanyDescription->PassiveResearchGeneration);
			}

			PurchaseResearch();
		}

		// Check if at war
		CompanyValue TotalValue = Company->GetCompanyValue();
		TArray<UFlareCompany*> SortedCompanyCombatValues = Game->GetGameWorld()->GetSortedCompanyCombatValues();
		int32 CompanyIndex = SortedCompanyCombatValues.IndexOfByKey(Company);
		int32 CompanyCombatCurrent = Company->GetCompanyValue().ArmyCurrentCombatPoints;
		int32 CompanyCombat = Company->GetCompanyValue().ArmyTotalCombatPoints;

		if(Company->AtWar())
		{
			AIData.Pacifism += Behavior->PacifismIncrementRate;
			if (!WasAtWar)
			{
				int64 WarBudget = GetBudget(EFlareBudget::Military);
				int64 StationBudget = GetBudget(EFlareBudget::Station);
				int64 TechnologyBudget = GetBudget(EFlareBudget::Technology);
				int64 TradeBudget = GetBudget(EFlareBudget::Trade);

//each time a new set of wars are entered check to see if we should relocate xx% of other funds towards military funding		
				if (WarBudget < ((TradeBudget * Behavior->WarDeclared_TradeBudgetFactor) * Behavior->WarDeclared_TransferToMilitaryBudgetFactor))
				{
					RedistributeBudgetTowards(EFlareBudget::Military, EFlareBudget::Trade, Behavior->WarDeclared_TransferToMilitaryBudgetTradePercent);
				}
				if (WarBudget < ((StationBudget * Behavior->WarDeclared_StationBudgetFactor) * Behavior->WarDeclared_TransferToMilitaryBudgetFactor))
				{
					RedistributeBudgetTowards(EFlareBudget::Military, EFlareBudget::Station, Behavior->WarDeclared_TransferToMilitaryBudgetStationPercent);
				}
				if (WarBudget < ((TechnologyBudget * Behavior->WarDeclared_TechnologyBudgetFactor) * Behavior->WarDeclared_TransferToMilitaryBudgetFactor))
				{
					RedistributeBudgetTowards(EFlareBudget::Military, EFlareBudget::Technology, Behavior->WarDeclared_TransferToMilitaryBudgetTechnologyPercent);
				}

				WasAtWar = true;
			}
		}
		else
		{
			WasAtWar = false;

			float Multiplier = 1.f;
			if (CompanyCombatCurrent > 0 && TotalValue.ArmyCurrentCombatPoints >= (TotalValue.ArmyTotalCombatPoints * 0.75))
			{
				bool IsStrongest = false;

				if ((CompanyIndex + 1) >= Game->GetGameWorld()->GetCompanies().Num())
				{
					Multiplier += 0.50;
					IsStrongest = true;
				}

				if (CompanyIndex > 0)
				{
					UFlareCompany* Lowercompany = SortedCompanyCombatValues[CompanyIndex - 1];
					if (Lowercompany)
					{
						int32 LowerCombat = Lowercompany->GetCompanyValue().ArmyTotalCombatPoints;
						if (LowerCombat > 0)// && CompanyCombat > 0)
						{
							float RatioRequirement = 0.30;
							float CombatRatio = LowerCombat / CompanyCombat;
							if (IsStrongest)
							{
								RatioRequirement = 0.50;
							}
							if (CombatRatio < RatioRequirement)
								//if next lowest has [RatioRequirement]% or lower of our military value, we're a bit more keen on war
							{
								Multiplier += 0.25;
							}
						}
					}
				}
			}
			AIData.Pacifism -= (Behavior->PacifismDecrementRate * Multiplier);
		}

		MinimumMoney = (TotalValue.TotalDailyProductionCost * (Behavior->DailyProductionCostSensitivityMilitary + Behavior->DailyProductionCostSensitivityEconomic))
			+ ((TotalValue.TotalTradeShipCargoCapacity * (Behavior->DailyProductionCostSensitivityEconomic * 0.75)) * 8)
			+ ((TotalValue.TotalStationCargoCapacity * (Behavior->DailyProductionCostSensitivityEconomic * 0.75)) * 2);

		if (!IsAboveMinimumMoney())
		{
			AIData.Pacifism += Behavior->PacifismIncrementRate * 0.33;
		}

		if(IdleCargoCapacity == 0)
		{
			AIData.Pacifism += Behavior->PacifismIncrementRate * 0.33;
		}

		for(UFlareSimulatedSpacecraft* Spacecraft : Company->GetCompanySpacecrafts())
		{
			if(Spacecraft->GetDamageSystem()->GetGlobalDamageRatio() < 0.99)
			{
				AIData.Pacifism += Behavior->PacifismIncrementRate * 0.5;
			}
		}

		AIData.Pacifism = FMath::Clamp(AIData.Pacifism, 0.f,100.f);
#if 0
		FLOGV("Pacifism for %s : %f (IdleCargoCapacity=%d)", *Company->GetCompanyName().ToString(), AIData.Pacifism, IdleCargoCapacity);
#endif
		Behavior->Simulate();
	}
}

void UFlareCompanyAI::AutoScrap()
{
	CompanyValue TotalValue = Company->GetCompanyValue();
	int32 TotalShipCount = TotalValue.TotalShipCount;
	int32 SCargoShipCount = TotalValue.TotalShipCountTradeS;
	int32 SMilitaryShipCount = TotalValue.TotalShipCountMilitaryS;

	while(true)
	{
		int32 ExtraCargo = SCargoShipCount - Behavior->Scrap_Min_S_Cargo;
		int32 ExtraMilitary = SMilitaryShipCount - Behavior->Scrap_Min_S_Military;

		if(TotalShipCount < Behavior->Scrap_Minimum_Ships || (ExtraCargo < 0 && ExtraMilitary < 0))
		{
			return;
		}

		bool ScrapMilitary = ExtraMilitary >= ExtraCargo;

#ifdef AI_DEBUG_AUTOSCRAP
		FLOGV("UFlareCompanyAI::AutoScrap %s need autoscrap : %d/%d ships. %d/%d cargo S ships, %d/%d military S ships. Scrap military ? %d ",
			  *Company->GetCompanyName().ToString(),

			  TotalShipCount,
			  Behavior->Scrap_Minimum_Ships,

			  SCargoShipCount,
			  Behavior->Scrap_Min_S_Cargo,
			
			  SMilitaryShipCount,
			  Behavior->Scrap_Min_S_Military,
			  
			  ScrapMilitary);
#endif

		// Find scrap candidate
		UFlareSimulatedSpacecraft* BestScrapCandidate = nullptr;
		int32 BestScrapCandidateResourceCount = 0;
		int32 BestScrapCandidateCapacity = 0;
		int32 BestScrapCandidateWeaponGroups = 0;


		for (UFlareSimulatedSpacecraft* ShipCandidate : Company->GetCompanyShips())
		{
			if (ShipCandidate->IsMilitary() != ScrapMilitary)
			{
				continue;
			}
			else if (ShipCandidate->GetSize() == EFlarePartSize::L)
			{
				continue;
			}
			else if (!CanUpgradeFromSector(ShipCandidate->GetCurrentSector()))
			{
				continue;
			}
			else if (ShipCandidate->GetDescription()->IsDroneShip)
			{
				continue;
			}
			else if (ShipCandidate->GetCurrentFleet() && ShipCandidate->GetCurrentFleet()->IsTraveling())
			{
				continue;
			}

			int32 ResourceCount = ShipCandidate->GetActiveCargoBay()->GetUsedCargoSpace();
			int32 Capacity = ShipCandidate->GetActiveCargoBay()->GetCapacity();
			if (ScrapMilitary)
			{
				FFlareSpacecraftDescription* Description = ShipCandidate->GetDescription();
				int32 WeaponGroups = Description->WeaponGroups.Num();

				if (BestScrapCandidate == nullptr || WeaponGroups < BestScrapCandidateWeaponGroups || ResourceCount < BestScrapCandidateResourceCount || Capacity < BestScrapCandidateCapacity)
				{
					BestScrapCandidate = ShipCandidate;
					BestScrapCandidateCapacity = Capacity;
					BestScrapCandidateResourceCount = ResourceCount;
					BestScrapCandidateWeaponGroups = WeaponGroups;
				}
			}
			else
			{
				if (BestScrapCandidate == nullptr || ResourceCount < BestScrapCandidateResourceCount || Capacity < BestScrapCandidateCapacity)
				{
					BestScrapCandidate = ShipCandidate;
					BestScrapCandidateCapacity = Capacity;
					BestScrapCandidateResourceCount = ResourceCount;
				}
			}
		}

		if (BestScrapCandidate == nullptr)
		{
			break;
		}

#ifdef AI_DEBUG_AUTOSCRAP
		FLOGV("UFlareCompanyAI::AutoScrap %s can be scraped",
			  *BestScrapCandidate->GetImmatriculation().ToString());
#endif

		UFlareSimulatedSpacecraft* TargetStation = NULL;
		TArray<UFlareSimulatedSpacecraft*> SectorStations = BestScrapCandidate->GetCompany()->GetCompanySectorStations(BestScrapCandidate->GetCurrentSector());
		// Find a suitable station
		for (int Index = 0; Index < SectorStations.Num(); Index++)
		{
			if (SectorStations[Index]->GetCompany() == BestScrapCandidate->GetCompany())
			{
				TargetStation = SectorStations[Index];
#ifdef AI_DEBUG_AUTOSCRAP
				FLOGV("UFlareCompanyAI::AutoScrap : found company station '%s'", *TargetStation->GetImmatriculation().ToString());
#endif
				break;
			}
			else if (TargetStation == NULL)
			{
				TargetStation = SectorStations[Index];
			}
		}

		// Scrap
		if (TargetStation)
		{
#ifdef AI_DEBUG_AUTOSCRAP
			FLOGV("UFlareCompanyAI::AutoScrap : scrapping at '%s'", *TargetStation->GetImmatriculation().ToString());
#endif
			TargetStation->GetGame()->Scrap(BestScrapCandidate->GetImmatriculation(), TargetStation->GetImmatriculation());
			TotalShipCount--;
			if(ScrapMilitary)
			{
				SMilitaryShipCount--;
			}
			else
			{
				SCargoShipCount--;
			}
			;
		}
		else
		{
#ifdef AI_DEBUG_AUTOSCRAP
			FLOGV("UFlareCompanyAI::AutoScrap : fail to find scrap station in '%s'", *BestScrapCandidate->GetCurrentSector()->GetSectorName().ToString());
#endif
			break;
		}
	}
}

bool UFlareCompanyAI::PurchaseSectorStationLicense(EFlareBudget::Type BudgetType)
{
	if (AIData.DesiredStationLicense == NAME_None)
	{
		TArray<UFlareSimulatedSector*> PrioritySectorCandidates;
		TArray<UFlareSimulatedSector*> SectorCandidates;
		int32 SectorsRemaining = 0;
		for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
		{
			UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];
			if (!(Company->IsSectorStationLicenseUnlocked(Sector->GetDescription()->Identifier)))
			{
				++SectorsRemaining;
				if (Behavior->GetSectorAffility(Sector) > 0 || Company->GetCompanySectorStationsCount(Sector,false) > 0)
				{
					PrioritySectorCandidates.Add(Sector);
				}
				else
				{
					SectorCandidates.Add(Sector);
				}
			}
		}

		if (SectorsRemaining <= 0)
		{
			Behavior->FinishedBuyingSectorStationLicenses = true;
		}
		else if (PrioritySectorCandidates.Num() > 0)
		{
			int32 PickIndex = FMath::RandRange(0, PrioritySectorCandidates.Num() - 1);
			AIData.DesiredStationLicense = PrioritySectorCandidates[PickIndex]->GetDescription()->Identifier;
		}
		else
		{
			int32 PickIndex = FMath::RandRange(0, SectorCandidates.Num() - 1);
			AIData.DesiredStationLicense = SectorCandidates[PickIndex]->GetDescription()->Identifier;
		}
	}

	if (Company->CanBuyStationLicense(AIData.DesiredStationLicense))
	{
		int64 Cost = Company->GetStationLicenseCost(AIData.DesiredStationLicense);
		if (CanSpendBudget(BudgetType,Cost) && Company->GetMoney() >= Cost)
		{
			SpendBudget(BudgetType, Cost);
			Company->BuyStationLicense(AIData.DesiredStationLicense);
			AIData.DateBoughtLicense = Game->GetGameWorld()->GetDate();
			AIData.DesiredStationLicense = NAME_None;
			return true;
		}
	}
	return false;
}

void UFlareCompanyAI::PurchaseResearch()
{
	FText Reason;
	if (AIData.ResearchProject == NAME_None || !Company->IsTechnologyAvailable(AIData.ResearchProject, Reason, true))
	{
		// Find a new research
		TArray<FFlareTechnologyDescription*> ResearchCandidates;
		int32 Researchtogo = 0;

		for (UFlareTechnologyCatalogEntry* Technology : GetGame()->GetTechnologyCatalog()->TechnologyCatalog)
		{
			FFlareTechnologyDescription* TechnologyData = &Technology->Data;

			if (TechnologyData->ResearchableCompany.Num() > 0)
			{
				if (!TechnologyData->ResearchableCompany.Contains(Company->GetDescription()->ShortName))
				{
					continue;
				}
			}

			if (TechnologyData->PlayerOnly)
			{
				continue;
			}

			if (!Company->IsTechnologyUnlocked(Technology->Data.Identifier))
			{
				++Researchtogo;
			}
			if (Company->IsTechnologyAvailable(Technology->Data.Identifier, Reason, true))
			{
				ResearchCandidates.Add(&Technology->Data);
			}
		}

		if (Researchtogo <= 0)
		{
			Behavior->FinishedResearch = true;
			RedistributeBudget(EFlareBudget::Technology, GetBudget(EFlareBudget::Technology));

//			int32 ScrappedStations = 0;
//			int32 TotalStations = 0;

			for (int StationIndex = 0; StationIndex < Company->GetCompanyStations().Num(); StationIndex++)
			{
				UFlareSimulatedSpacecraft* Station = Company->GetCompanyStations()[StationIndex];
				if (Station)
				{
//					++TotalStations;
					FFlareSpacecraftDescription* StationDescription = Station->GetDescription();
					if (StationDescription && StationDescription->IsResearch())
					{
						GetGame()->ScrapStation(Station);
//						++ScrappedStations;
						--StationIndex;
						continue;
					}
				}
			}
			// No research to research
			return;
		}

		bool FoundResearchOrder = false;
		for (int TechIndex = 0; TechIndex < Behavior->ResearchOrder.Num(); TechIndex++)
		{
			FName TechnologyName = Behavior->ResearchOrder[TechIndex];
			if (!Company->IsTechnologyUnlocked(TechnologyName))
			{
				if (Company->IsTechnologyAvailable(TechnologyName, Reason, true))
				{
				//just checking to make sure it actually exists?
					FFlareTechnologyDescription* RealTech = GetGame()->GetTechnologyCatalog()->Get(TechnologyName);
					if (RealTech)
					{
						AIData.ResearchProject = TechnologyName;
						FoundResearchOrder = true;
						Behavior->ResearchOrder.RemoveAt(TechIndex);
						break;
					}
				}
			}
			else
			{
				Behavior->ResearchOrder.RemoveAt(TechIndex);
				--TechIndex;
			}
		}

		if (!FoundResearchOrder&&ResearchCandidates.Num()>0)
		{
			int32 PickIndex = FMath::RandRange(0, ResearchCandidates.Num() - 1);
			AIData.ResearchProject = ResearchCandidates[PickIndex]->Identifier;
		}
	}

	// Try to buy
	if(Company->IsTechnologyAvailable(AIData.ResearchProject, Reason))
	{
		Company->UnlockTechnology(AIData.ResearchProject);
	}
}

void UFlareCompanyAI::DestroySpacecraft(UFlareSimulatedSpacecraft* Spacecraft)
{
}

void UFlareCompanyAI::UpdateIdleShipsStats(AITradeIdleShips& IdleShips)
{
	IdleCargoCapacity = 0;

	for(AIIdleShip* Ship : IdleShips.GetShips())
	{
		if(Ship->Ship->GetCompany() == Company)
		{
			IdleCargoCapacity += Ship->Capacity;
		}
	}
}

/*----------------------------------------------------
	Internal subsystems
----------------------------------------------------*/

void UFlareCompanyAI::UpdateDiplomacy(bool GlobalWar)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_UpdateDiplomacy);

	Behavior->Load(Company);
	Behavior->UpdateDiplomacy(GlobalWar);
}

void UFlareCompanyAI::RepairAndRefill()
{
	SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_RepairAndRefill);

	for (UFlareFleet* Fleet : Company->GetCompanyFleets())
	{
		if (Company->AtWar())
		{
			if (Fleet->GetCombatPoints(false) >= 1)
			{
				if (Fleet->FleetNeedsRepair())
				{
					Fleet->RepairFleet();
				}
				if (Fleet->FleetNeedsRefill())
				{
					Fleet->RefillFleet();
				}
			}
		}
		else  if (Fleet->GetCombatPoints(false) == 0)
		{
			if (Fleet->FleetNeedsRepair())
			{
				Fleet->RepairFleet();
			}
		}
	}

	bool RepairFirstChance = FMath::FRand() < 0.8;
	if (RepairFirstChance)
	{
		RepairFleets();
		RefillFleets();
	}
	else
	{
		RefillFleets();
		RepairFleets();
	}
}

void UFlareCompanyAI::RepairFleets()
{
	int64 MinimumMoneyRequired = GetMinimumMoney() * Behavior->BudgetMinimumRepairFactor;
	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];
		if (Company->GetMoney() >= MinimumMoneyRequired)
		{
			SectorHelper::RepairFleets(Sector, Company);
		}
		else
		{
//			FLOGV("%s not above minimum money of %d, attempting repair in %s", *Company->GetCompanyName().ToString(), MinimumMoneyRequired, *Sector->GetSectorName().ToString());
			SectorHelper::RepairFleets(Sector, Company, nullptr, true);
		}
	}
}

void UFlareCompanyAI::RefillFleets()
{
	int64 MinimumMoneyRequired = GetMinimumMoney() * Behavior->BudgetMinimumRepairFactor;
	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];
		if (Company->GetMoney() >= MinimumMoneyRequired)
		{
			SectorHelper::RefillFleets(Sector, Company);
		}
		else
		{
			SectorHelper::RefillFleets(Sector, Company, nullptr, true);
		}
	}
}

#define DEBUG_AI_CONSTRUCTION 0

void UFlareCompanyAI::UpdateBestScore(float Score,
									  UFlareSimulatedSector* Sector,
									  FFlareSpacecraftDescription* StationDescription,
									  UFlareSimulatedSpacecraft *Station,
									  float* BestScore,
									  FFlareSpacecraftDescription** BestStationDescription,
									  UFlareSimulatedSpacecraft** BestStation,
									  UFlareSimulatedSector** BestSector)
{
	//FLOGV("UpdateBestScore Score=%f BestScore=%f", Score, *BestScore);

	// Change best if we found better
	if (Score > 0.f && (!BestStationDescription || Score > *BestScore))
	{
		//FLOGV("New Best : Score=%f", Score);

		if (StationDescription->BuildableCompany.Num() > 0)
		{
			//buildable company has something, check if shipyard owning faction is allowed to build this
			if (!StationDescription->BuildableCompany.Contains(Company->GetDescription()->ShortName))
			{
				return;
			}
		}


		*BestScore = Score;
		*BestStationDescription = (Station ? Station->GetDescription() : StationDescription);
		*BestStation = Station;
		*BestSector = Sector;
	}
}

/*----------------------------------------------------
	Budget
----------------------------------------------------*/

bool UFlareCompanyAI::CanSpendBudget(EFlareBudget::Type Type, int64 Amount)
{
	int64 ExistingBudget = GetBudget(Type);
	if (ExistingBudget >= Amount)
	{
		return true;
	}
	return false;
}

int64 UFlareCompanyAI::GetTotalBudgets()
{
	int64 ReturnValue = 0;
	for (EFlareBudget::Type Budget : AllBudgets)
	{
		ReturnValue += GetBudget(Budget);
	}
	return ReturnValue;
}

void UFlareCompanyAI::AddIncomeToBudgets()
{
	int64 CompanyIncome = Company->GetDailyMoneyGain();
	if (CompanyIncome > 0)
	{
		for (EFlareBudget::Type Budget : AllBudgets)
		{
			int64 BudgetGain = CompanyIncome * Behavior->GetBudgetWeight(Budget);
			ModifyBudget(Budget, BudgetGain);
		}
	}
}

void UFlareCompanyAI::RedistributeBudgetTowards(EFlareBudget::Type Type, EFlareBudget::Type FromBudget, float Ratio)
{
	for (EFlareBudget::Type Budget : AllBudgets)
	{
		if (Budget == Type)
		{
			continue;
		}

		if (FromBudget != EFlareBudget::None && FromBudget != Budget)
		{
			continue;
		}

		int64 BudgetRedistAmount = GetBudget(Budget) * Ratio;
		SpendBudget(Budget, BudgetRedistAmount);
		ModifyBudget(Type, BudgetRedistAmount);
	}
}

void UFlareCompanyAI::RedistributeBudget(EFlareBudget::Type Type, int64 Amount)
{
	if (Amount > 0)
	{
		SpendBudget(Type, Amount);
		for (EFlareBudget::Type Budget : AllBudgets)
		{
			if (Budget == Type)
			{
				continue;
			}
			int64 BudgetRedistAmount = Amount * Behavior->GetBudgetWeight(Budget);
			ModifyBudget(Budget, BudgetRedistAmount);
		}
	}
}

void UFlareCompanyAI::SpendBudget(EFlareBudget::Type Type, int64 Amount)
{
	// A project spend money, dispatch available money for others projects

#ifdef DEBUG_AI_BUDGET
	FLOGV("%s spend %lld on %d", *Company->GetCompanyName().ToString(), Amount, (Type+0));
#endif
	ModifyBudget(Type, -Amount);
}

void UFlareCompanyAI::RecalculateFullBudgets()
{
	int64 CompanyMoney = Company->GetMoney();
	for (EFlareBudget::Type Budget : AllBudgets)
	{
		int64 StarterBudget = 0;
		StarterBudget = CompanyMoney * Behavior->GetBudgetWeight(Budget);
		SetBudget(Budget, StarterBudget);
	}
}

int64 UFlareCompanyAI::GetBudget(EFlareBudget::Type Type)
{
	switch(Type)
	{
		case EFlareBudget::Military:
			return AIData.BudgetMilitary;
		break;
		case EFlareBudget::Station:
			return AIData.BudgetStation;
		break;
		case EFlareBudget::Technology:
			return AIData.BudgetTechnology;
		break;
		case EFlareBudget::Trade:
			return AIData.BudgetTrade;
		break;
	}
#ifdef DEBUG_AI_BUDGET
	FLOGV("GetBudget: unknown budget type %d", Type);
#endif
	return 0;
}

void UFlareCompanyAI::SetBudget(EFlareBudget::Type Type, int64 Amount)
{
	switch (Type)
	{
		case EFlareBudget::Military:
			AIData.BudgetMilitary = Amount;
		break;
		case EFlareBudget::Station:
			AIData.BudgetStation = Amount;
		break;
		case EFlareBudget::Technology:
			AIData.BudgetTechnology = Amount;
		break;
		case EFlareBudget::Trade:
			AIData.BudgetTrade = Amount;
		break;
	}
}

void UFlareCompanyAI::ModifyBudget(EFlareBudget::Type Type, int64 Amount)
{
	switch(Type)
	{
		case EFlareBudget::Military:
			AIData.BudgetMilitary += Amount;
#ifdef DEBUG_AI_BUDGET
			FLOGV("New military budget %lld (%lld)", AIData.BudgetMilitary, Amount);
#endif
		break;
		case EFlareBudget::Station:
			AIData.BudgetStation += Amount;
#ifdef DEBUG_AI_BUDGET
			FLOGV("New station budget %lld (%lld)", AIData.BudgetStation, Amount);
#endif
		break;
		case EFlareBudget::Technology:
			AIData.BudgetTechnology += Amount;
#ifdef DEBUG_AI_BUDGET
			FLOGV("New technology budget %lld (%lld)", AIData.BudgetTechnology, Amount);
#endif
		break;
		case EFlareBudget::Trade:
			AIData.BudgetTrade += Amount;
#ifdef DEBUG_AI_BUDGET
			FLOGV("New trade budget %lld (%lld)", AIData.BudgetTrade, Amount);
#endif
		break;
#ifdef DEBUG_AI_BUDGET
	default:
			FLOGV("ModifyBudget: unknown budget type %d", Type);
#endif
	}
}

void UFlareCompanyAI::ProcessBudget(TArray<EFlareBudget::Type> BudgetToProcess)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_ProcessBudget);

	// Find
#ifdef DEBUG_AI_BUDGET
	FLOGV("Process budget for %s (%d projects)", *Company->GetCompanyName().ToString(), BudgetToProcess.Num());
#endif

	EFlareBudget::Type MaxBudgetType = EFlareBudget::None;
	int64 MaxBudgetAmount = 0;

	for (EFlareBudget::Type Type : BudgetToProcess)
	{
		int64 Budget = GetBudget(Type);
		if (MaxBudgetType == EFlareBudget::None || MaxBudgetAmount < Budget)
		{
			MaxBudgetType = Type;
			MaxBudgetAmount = Budget;
		}
	}

	if (MaxBudgetType == EFlareBudget::None)
	{
		// Nothing to do
		return;
	}
#ifdef DEBUG_AI_BUDGET
	FLOGV("max budget for %d with %lld", MaxBudgetType + 0, MaxBudgetAmount);
#endif

	bool Lock = false;
	bool Idle = true;

	if (Behavior->GetBudgetWeight(MaxBudgetType) > 0.f)
	{
		switch (MaxBudgetType)
		{
			case EFlareBudget::Military:
				ProcessBudgetMilitary(MaxBudgetAmount, Lock, Idle);
			break;
			case EFlareBudget::Station:
				ProcessBudgetStation(MaxBudgetAmount, false, Lock, Idle);
			break;
			case EFlareBudget::Technology:
				ProcessBudgetStation(MaxBudgetAmount, true, Lock, Idle);
			break;
			case EFlareBudget::Trade:
				ProcessBudgetTrade(MaxBudgetAmount, Lock, Idle);
			break;
		}
	}

	if (Lock)
	{
#ifdef DEBUG_AI_BUDGET
		FLOG("Lock");
#endif
		// Process no other projets
		return;
	}

	if (Idle)
	{
#ifdef DEBUG_AI_BUDGET
		FLOG("Idle");
#endif
		// Nothing to buy consume a part of its budget
//		SpendBudget(MaxBudgetType, MaxBudgetAmount / 100);
		RedistributeBudget(MaxBudgetType, MaxBudgetAmount / 100);
	}

	BudgetToProcess.Remove(MaxBudgetType);
	ProcessBudget(BudgetToProcess);
}

void UFlareCompanyAI::ProcessBudgetMilitary(int64 BudgetAmount, bool& Lock, bool& Idle)
{
	// Min confidence level
	float MinConfidenceLevel = 1;

	for (UFlareCompany* OtherCompany : Company->GetOtherCompanies())
	{
		TArray<UFlareCompany*> Allies;
		float ConfidenceLevel = Company->GetConfidenceLevel(OtherCompany, Allies);
		if (MinConfidenceLevel > ConfidenceLevel)
		{
			MinConfidenceLevel = ConfidenceLevel;
		}
	}

	if (!WasAtWar && (MinConfidenceLevel > Behavior->ConfidenceTarget || AIData.Pacifism > 90))
	{
		// Army size is ok
		Idle = true;
		Lock = false;
		return;
	}

	Idle = false;

	int64 ProjectCost = UpdateWarShipAcquisition();

	if (ProjectCost > 0 && ProjectCost < BudgetAmount / 2)
	{
		Lock = true;
	}
}

void UFlareCompanyAI::ProcessBudgetTrade(int64 BudgetAmount, bool& Lock, bool& Idle)
{
	CompanyValue TotalValue = Company->GetCompanyValue();
	if (Company->GetMoney() > (TotalValue.TotalDailyProductionCost * Behavior->DailyProductionCostSensitivityEconomic))
	{
		UpgradeCargoShips();
	}

	int32 DamagedCargosCapacity = GetDamagedCargosCapacity();
	float IdleRatio = float(IdleCargoCapacity + DamagedCargosCapacity) / GetCargosCapacity();

	//FLOGV("%s DamagedCargosCapacity=%d", *Company->GetCompanyName().ToString(), DamagedCargosCapacity);
	//FLOGV("%s IdleCargoCapacity=%d", *Company->GetCompanyName().ToString(),IdleCargoCapacity);
	//FLOGV("%s CargosCapacity=%d", *Company->GetCompanyName().ToString(),GetCargosCapacity());


	if (IdleRatio > 0.1f)
	{
		//FLOG("fleet ok");
		// Trade fleet size is ok

		Idle = true;
		Lock = false;
		return;
	}

	Idle = false;

	int64 ProjectCost = UpdateCargoShipAcquisition();

	//FLOGV("ProjectCost %lld", ProjectCost);

	if (ProjectCost > 0 && ProjectCost < BudgetAmount / 2)
	{
		//FLOG("lock : BudgetAmount %lld");
		Lock = true;
	}
}

void UFlareCompanyAI::ProcessBudgetStation(int64 BudgetAmount, bool Technology, bool& Lock, bool& Idle)
{
	Idle = false;

	if (UnderConstructionStations.Num() > 0 && UnderConstructionUpgradeStations.Num() > 1)
	{
		ReservedResources = 75;
		return;
	}

	TArray<UFlareSimulatedSector*> KnownSectors;
	TMap<FName, UFlareSimulatedSector*> LicensedSectors = Company->GetLicenseStationSectors();
	TArray<FName> Keys;
	LicensedSectors.GetKeys(Keys);
	bool FoundPossibleSector = false;
	KnownSectors.Reserve(Keys.Num());
	for (int i = 0; i < Keys.Num(); i++)
	{
		FName Identifier = Keys[i];
		UFlareSimulatedSector* Sector = LicensedSectors[Identifier];
		if (Sector)
		{
//only build new stations or upgrade existing stations in sectors where the company owns a license
			KnownSectors.Add(Sector);

			if (!FoundPossibleSector)
			{
				int32 OwnedStationCount = Company->GetCompanySectorStationsCount(Sector);
				int32 MaxStationCount = Company->IsTechnologyUnlocked("dense-sectors") ? Sector->GetMaxStationsPerCompany() : Sector->GetMaxStationsPerCompany() / 2;
				if (OwnedStationCount < MaxStationCount)
				{
					FoundPossibleSector = true;
				}
			}
		}
	}

	//Check if a construction is in progress
	bool UnderConstruction = false;
	bool UnderConstructionUpgrade = false;

	if (!FoundPossibleSector)
	{
		UnderConstruction = true;
		if (!Behavior->FinishedBuyingSectorStationLicenses)
		{
			PurchaseSectorStationLicense(Technology ? EFlareBudget::Technology : EFlareBudget::Station);
		}
	}
	else
	{
		if (Game->GetGameWorld()->GetDate() >= (AIData.DateBoughtLicense + Behavior->DaysUntilTryGetStationLicense))
		{
			if (!Behavior->FinishedBuyingSectorStationLicenses)
			{
				PurchaseSectorStationLicense(Technology ? EFlareBudget::Technology : EFlareBudget::Station);
			}
		}
	}

	if (Technology && Behavior->FinishedResearch)
	{
		RedistributeBudget(EFlareBudget::Technology, GetBudget(EFlareBudget::Technology));
		Idle = true;
		return;
	}

	ReservedResources = 0;
	bool Resources_Upgrade = true;

	if (UnderConstructionStations.Num() > 0)
	{
		ReservedResources += 50;
		UnderConstruction = true;
	}

	if (UnderConstructionUpgradeStations.Num() > 1)
// AI Company will upgrade one station without caring about minimum available resource levels, and then start upgrading a second one simultaniously if resource levels are "ok"
	{
		ReservedResources += 25;
		UnderConstructionUpgrade = true;
	}
	else if (UnderConstructionUpgradeStations.Num() == 1)
	{
		if (!ComputeAvailableConstructionResourceAvailability(25 + (GlobalReservedResources - 25)))
		{
			Resources_Upgrade = false;
		}
	}

	// Prepare resources for station-building analysis
	float BestScore = 0;
	UFlareSimulatedSector* BestSector = NULL;
	FFlareSpacecraftDescription* BestStationDescription = NULL;
	UFlareSimulatedSpacecraft* BestStation = NULL;
	TArray<UFlareSpacecraftCatalogEntry*>& StationCatalog = Game->GetSpacecraftCatalog()->StationCatalog;

	CreateWorldResourceVariations();

	// Loop on sector list
	while (KnownSectors.Num())
	{
		int32 SectorIndex = FMath::RandRange(0, KnownSectors.Num() - 1);
		UFlareSimulatedSector* Sector = KnownSectors[SectorIndex];

		if (!UnderConstruction)
		{
			// Loop on catalog
			for (int32 StationIndex = 0; StationIndex < StationCatalog.Num(); StationIndex++)
			{
				FFlareSpacecraftDescription* StationDescription = &StationCatalog[StationIndex]->Data;

				if (StationDescription->IsSubstation)
				{
					// Never try to build substations
					continue;
				}

				if (!Company->IsTechnologyUnlockedStation(StationDescription))
				{
					continue;
				}

				// Check sector limitations
				TArray<FText> Reasons;
				if (!Sector->CanBuildStation(StationDescription, Company, Reasons, true))
				{
					continue;
				}

				int32 UpdatableStationCountForThisKind = 0;
				int32 StationCountForThisKind = 0;

				for (UFlareSimulatedSpacecraft* StationCandidate : Company->GetCompanySectorStations(Sector))
				{
					if (StationDescription == StationCandidate->GetDescription())
					{
						StationCountForThisKind++;

						if (StationCandidate->GetLevel() < StationDescription->MaxLevel)
						{
							UpdatableStationCountForThisKind++;
						}
					}
				}

				int MaximumUpdatable = 4;
				int Maximum = 10;

				if (StationDescription == Game->GetSpacecraftCatalog()->Get("station-habitation"))
				{
					Maximum = 1;
				}

				if (!UnderConstructionUpgrade)
				{
					MaximumUpdatable = 2;
				}
				
				if (UpdatableStationCountForThisKind >= MaximumUpdatable || StationCountForThisKind >= Maximum)
				{
					// Prefer update if possible
					continue;
				}

				if (StationDescription->Capabilities.Contains(EFlareSpacecraftCapability::Storage))
				{
					int StorageStationCount = 0;
					int StationCount = 0;
					// TODO

					for (UFlareSimulatedSpacecraft* Station : Sector->GetSectorStations())
					{
						if (Station->HasCapability(EFlareSpacecraftCapability::Storage))
						{
							StorageStationCount++;
						}
						StationCount++;
					}

					if (StorageStationCount < 1 && StationCount > AI_MAX_STATION_PER_SECTOR / 2)
					{
						UpdateBestScore(1e18f, Sector, StationDescription, NULL, &BestScore, &BestStationDescription, &BestStation, &BestSector);
						break;
					}
				}

				//FLOGV("> Analyse build %s in %s", *StationDescription->Name.ToString(), *Sector->GetSectorName().ToString());

				// Count factories for the company, compute rentability in each sector for each station
				for (int32 FactoryIndex = 0; FactoryIndex < StationDescription->Factories.Num(); FactoryIndex++)
				{
					FFlareFactoryDescription* FactoryDescription = &StationDescription->Factories[FactoryIndex]->Data;

					// Add weight if the company already have another station in this type
					float Score = ComputeConstructionScoreForStation(Sector, StationDescription, FactoryDescription, NULL, Technology);

					UpdateBestScore(Score, Sector, StationDescription, NULL, &BestScore, &BestStationDescription, &BestStation, &BestSector);
				}

				if (StationDescription->Factories.Num() == 0)
				{
					float Score = ComputeConstructionScoreForStation(Sector, StationDescription, NULL, NULL, Technology);
					UpdateBestScore(Score, Sector, StationDescription, NULL, &BestScore, &BestStationDescription, &BestStation, &BestSector);
				}
			}
		}

		if (!UnderConstructionUpgrade&&Resources_Upgrade)
		{
			
			for (int32 StationIndex = 0; StationIndex < Company->GetCompanySectorStations(Sector).Num(); StationIndex++)
			{
				UFlareSimulatedSpacecraft* Station = Company->GetCompanySectorStations(Sector)[StationIndex];
				if (GetGame()->GetQuestManager()->IsTradeQuestUseStation(Station, true))
				{
					// Do not update stations used by quests
					continue;
				}

				if (Station->GetLevel() >= Station->GetDescription()->MaxLevel || Station->IsUnderConstruction() || !Company->IsTechnologyUnlockedStation(Station->GetDescription()))
				{
					continue;
				}

				int32 StationCountForThisKind = 0;
				int32 StationWithLowerLevelInSectorForThisKind = 0;
				for (UFlareSimulatedSpacecraft* StationCandidate : Company->GetCompanyStations())
				{
					if (Station->GetDescription() == StationCandidate->GetDescription())
					{
						StationCountForThisKind++;

						if (StationCandidate->GetLevel() < Station->GetLevel())
						{
							StationWithLowerLevelInSectorForThisKind++;
						}
					}
				}

				if (StationWithLowerLevelInSectorForThisKind > 0)
				{
					continue;
				}

				if (StationCountForThisKind < 2)
				{
					// Don't upgrade the only station the company have to avoid deadlock
					continue;
				}

				//FLOGV("> Analyse upgrade %s in %s", *Station->GetImmatriculation().ToString(), *Sector->GetSectorName().ToString());

				if (Station->HasCapability(EFlareSpacecraftCapability::Storage))
				{
					int32 StationLevelSum = 0;
					int32 StationCount = 0;

					for (UFlareSimulatedSpacecraft* OtherStation : Sector->GetSectorStations())
					{
						if (OtherStation->GetCompany() != Company)
						{
							// Only AI company station
							continue;
						}

						if (OtherStation->HasCapability(EFlareSpacecraftCapability::Storage))
						{
							continue;
						}

						StationLevelSum = OtherStation->GetLevel();
						StationCount++;
					}


					if (StationCount > 0)
					{
						int32 StationLevelMean = StationLevelSum / StationCount;

						if (Station->GetLevel() < StationLevelMean)
						{
							UpdateBestScore(1e17f, Sector, Station->GetDescription(), Station, &BestScore, &BestStationDescription, &BestStation, &BestSector);
							break;
						}
					}
				}

				// Count factories for the company, compute rentability in each sector for each station
				for (int32 FactoryIndex = 0; FactoryIndex < Station->GetDescription()->Factories.Num(); FactoryIndex++)
				{
					FFlareFactoryDescription* FactoryDescription = &Station->GetDescription()->Factories[FactoryIndex]->Data;

					// Add weight if the company already have another station in this type
					float Score = ComputeConstructionScoreForStation(Sector, Station->GetDescription(), FactoryDescription, Station, Technology);
					UpdateBestScore(Score, Sector, Station->GetDescription(), Station, &BestScore, &BestStationDescription, &BestStation, &BestSector);
				}

				if (Station->GetDescription()->Factories.Num() == 0)
				{
					float Score = ComputeConstructionScoreForStation(Sector, Station->GetDescription(), NULL, Station, Technology);
					UpdateBestScore(Score, Sector, Station->GetDescription(), Station, &BestScore, &BestStationDescription, &BestStation, &BestSector);
				}
			}
		}
		KnownSectors.RemoveAt(SectorIndex);
	}

	if (BestSector && BestStationDescription)
	{
#ifdef DEBUG_AI_BUDGET
		FLOGV("UFlareCompanyAI::UpdateStationConstruction : %s >>> %s in %s (upgrade: %d) Score=%f",
			*Company->GetCompanyName().ToString(),
			*BestStationDescription->Name.ToString(),
			*BestSector->GetSectorName().ToString(),
			(BestStation != NULL),BestScore);
#endif
		float StationPrice = ComputeStationPrice(BestSector, BestStationDescription, BestStation);

		int64 CompanyMoney = Company->GetMoney();
		CompanyValue TotalValue = Company->GetCompanyValue();
		float CostSafetyMargin = Behavior->CostSafetyMarginStation;

		if (((StationPrice * CostSafetyMargin) + (TotalValue.TotalDailyProductionCost * Behavior->DailyProductionCostSensitivityEconomic) < CompanyMoney) && CanSpendBudget(Technology ? EFlareBudget::Technology : EFlareBudget::Station, StationPrice))
		{
			UFlareSimulatedSpacecraft* BuiltStation = NULL;
			TArray<FText> Reasons;
			if (BestStation && !UnderConstructionUpgrade)
				{
				if (BestSector->UpgradeStation(BestStation))
				{
					BuiltStation = BestStation;
					UnderConstructionUpgradeStations.Add(BuiltStation);
				}
			}
			else if (!UnderConstruction && BestSector->CanBuildStation(BestStationDescription, Company, Reasons))
			{
				BuiltStation = BestSector->BuildStation(BestStationDescription, Company);
				if (BuiltStation)
				{
					UnderConstructionStations.Add(BuiltStation);
				}
			}

			if (BuiltStation)
			{

#ifdef DEBUG_AI_BUDGET
				FLOG("Start construction");
#endif

				SpendBudget(Technology ? EFlareBudget::Technology : EFlareBudget::Station, StationPrice);
				GameLog::AIConstructionStart(Company, BestSector, BestStationDescription, BestStation);
			}

			if (StationPrice < BudgetAmount)
			{
				Lock = true;
			}
		}
		else
		{
			// keeping a reserve set of money before expanding their infrastructure
		}
	}
	else
	{
		Idle = true;
	}
}

int64 UFlareCompanyAI::UpdateCargoShipAcquisition()
{
	// For the transport pass, the best ship is choose. The best ship is the one with the small capacity, but
	// only if the is no more then the AI_CARGO_DIVERSITY_THERESOLD

	// Check if a ship is building
	GetBuildingShipNumber();
	if (WasAtWar)
	{
		if(BuildingTradeShips >= Behavior->MaxTradeShipsBuildingWar)
		{
			return 0;
		}
	}
	else
	{
		if (BuildingTradeShips >= Behavior->MaxTradeShipsBuildingPeace)
		{
			return 0;
		}
	}

	const FFlareSpacecraftDescription* ShipDescription = FindBestShipToBuild(false);
	if (ShipDescription == NULL)
	{
		//FLOG("Find no ship to build");
		return 0;
	}

	return OrderOneShip(ShipDescription);
}

int64 UFlareCompanyAI::UpdateWarShipAcquisition()
{
	// For the war pass there is 2 states : slow preventive ship buy. And war state.
	//
	// - In the first state, the company will limit his army to a percentage of his value.
	//   It will create only one ship at once
	// - In the second state, it is war, the company will limit itself to de double of the
	//   army value of all enemies and buy as many ship it can.

	// Check if a ship is building
	GetBuildingShipNumber();

	if (WasAtWar)
	{
		if (BuildingMilitaryShips >= Behavior->MaxMilitaryShipsBuildingWar)
		{
			return 0;
		}
	}
	else
	{
		if (BuildingMilitaryShips >= Behavior->MaxMilitaryShipsBuildingPeace)
		{
			return 0;
		}
	}

	const FFlareSpacecraftDescription* ShipDescription = FindBestShipToBuild(true);
	return OrderOneShip(ShipDescription);
}


/*----------------------------------------------------
	Military AI
----------------------------------------------------*/


void UFlareCompanyAI::UpdateMilitaryMovement()
{
	SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_UpdateMilitaryMovement);

	if (WasAtWar)
	{
		UpdateWarMilitaryMovement();
	}
	else
	{
		UpdatePeaceMilitaryMovement();
	}
}

void UFlareCompanyAI::CheckBattleResolution()
{
#ifdef DEBUG_AI_BATTLE_STATES
			FLOGV("CheckBattleResolution for %s : %d sector with battle",
				*Company->GetCompanyName().ToString(),
				SectorWithBattle.Num());
#endif

	for (UFlareSimulatedSector* Sector : SectorWithBattle)
	{
		bool BattleLost = false;
		bool BattleWin = false;
		FFlareSectorBattleState BattleState =  Sector->GetSectorBattleState(Company);
		if (BattleState.InBattle)
		{
			if (BattleState.InFight)
			{
				// Still no winner
			}
			else if (BattleState.BattleWon)
			{
				BattleWin = true;
			}
			else
			{
				BattleLost = true;
			}
		}
		else
		{
			if (BattleState.HasDanger)
			{
				BattleLost = true;
			}
			else
			{
				BattleWin = true;
			}
		}

		if (BattleLost)
		{
			AIData.Caution += Behavior->DefeatAdaptation;
#ifdef DEBUG_AI_BATTLE_STATES
			FLOGV("%s lost battle in %s, new caution is %f",
				*Company->GetCompanyName().ToString(), *Sector->GetSectorName().ToString(),
				AIData.Caution);
#endif

		}

		if (BattleWin)
		{
			AIData.Caution -= Behavior->DefeatAdaptation;
#ifdef DEBUG_AI_BATTLE_STATES
			FLOGV("%s win battle in %s, new caution is %f",
				*Company->GetCompanyName().ToString(), *Sector->GetSectorName().ToString(),
				AIData.Caution);
#endif
		}
	}
}


void UFlareCompanyAI::CheckBattleState()
{
	SectorWithBattle.Empty();

	for (UFlareSimulatedSector* Sector : Company->GetKnownSectors())
	{
		FFlareSectorBattleState BattleState =  Sector->GetSectorBattleState(Company);
		if (BattleState.InFight)
		{
			SectorWithBattle.Add(Sector);
		}
	}
}

bool UFlareCompanyAI::HasHealthyTradeFleet() const
{
	int32 IncapacitatedCargosCount = FindIncapacitatedCargos().Num();

	if (AI_CARGO_HEALTHY_THRESHOLD * IncapacitatedCargosCount > Company->GetCompanyShips().Num())
	{
		return false;
	}
	else
	{
		return true;
	}
}

TArray<WarTargetIncomingFleet> UFlareCompanyAI::GenerateWarTargetIncomingFleets(AIWarContext& WarContext, UFlareSimulatedSector* DestinationSector, bool SkipEnemiesOrAllies)
{
	TArray<WarTargetIncomingFleet> IncomingFleetList;

	for (UFlareTravel* Travel : Game->GetGameWorld()->GetTravels())
	{
		if (Travel->GetDestinationSector() != DestinationSector)
		{
			continue;
		}

		if (!SkipEnemiesOrAllies)
		{
			if (!WarContext.Allies.Contains(Travel->GetFleet()->GetFleetCompany()))
			{
				continue;
			}
		}
		else
		{
			if (!WarContext.Enemies.Contains(Travel->GetFleet()->GetFleetCompany()))
			{
				continue;
			}
		}

		int64 TravelDuration = Travel->GetRemainingTravelDuration();
		int32 ArmyCombatPoints = 0;

		int32 ArmySmallCombatPoints = 0;
		int32 ArmyLargeCombatPoints = 0;

		int32 ArmyAntiLCombatPoints = 0;
		int32 ArmyAntiSCombatPoints = 0;

		for (UFlareSimulatedSpacecraft* Ship : Travel->GetFleet()->GetShips())
		{
			int32 ShipCombatPoints = Ship->GetCombatPoints(true);
			ArmyCombatPoints += ShipCombatPoints;

			if (Ship->GetSize() == EFlarePartSize::L)
			{
				ArmyLargeCombatPoints += ShipCombatPoints;
			}
			else if (Ship->GetSize() == EFlarePartSize::L)
			{
				ArmySmallCombatPoints += ShipCombatPoints;
			}


			if (Ship->GetWeaponsSystem()->HasAntiLargeShipWeapon())
			{
				ArmyAntiLCombatPoints += ShipCombatPoints;
			}
			if (Ship->GetWeaponsSystem()->HasAntiSmallShipWeapon())
			{
				ArmyAntiSCombatPoints += ShipCombatPoints;
			}
		}

		// Add an entry or modify one
		bool ExistingTravelFound = false;
		for (WarTargetIncomingFleet& Fleet : IncomingFleetList)
		{
			if (Fleet.TravelDuration  == TravelDuration)
			{
				Fleet.ArmyCombatPoints += ArmyCombatPoints;
				Fleet.ArmySmallCombatPoints += ArmySmallCombatPoints;
				Fleet.ArmyLargeCombatPoints += ArmyLargeCombatPoints;
				Fleet.ArmyAntiLCombatPoints += ArmyAntiLCombatPoints;
				Fleet.ArmyAntiSCombatPoints += ArmyAntiSCombatPoints;
				ExistingTravelFound = true;
				break;
			}
		}

		if (!ExistingTravelFound)
		{
			WarTargetIncomingFleet Fleet;
			Fleet.TravelDuration = TravelDuration;
			Fleet.ArmyCombatPoints = ArmyCombatPoints;
			Fleet.ArmySmallCombatPoints = ArmySmallCombatPoints;
			Fleet.ArmyLargeCombatPoints = ArmyLargeCombatPoints;
			Fleet.ArmyAntiLCombatPoints = ArmyAntiLCombatPoints;
			Fleet.ArmyAntiSCombatPoints = ArmyAntiSCombatPoints;
			IncomingFleetList.Add(Fleet);
		}
	}
	return IncomingFleetList;
}

inline static bool WarTargetComparator(const WarTarget& ip1, const WarTarget& ip2)
{
	bool SELECT_TARGET1 = true;
	bool SELECT_TARGET2 = false;

	if (ip1.EnemyArmyCombatPoints > 0 && ip2.EnemyArmyCombatPoints == 0)
	{
		return SELECT_TARGET1;
	}

	if (ip2.EnemyArmyCombatPoints > 0 && ip1.EnemyArmyCombatPoints == 0)
	{
		return SELECT_TARGET2;
	}

	if (ip1.EnemyArmyCombatPoints > 0 && ip2.EnemyArmyCombatPoints > 0)
	{
		// Defend military
		if (ip1.OwnedMilitaryCount > ip2.OwnedMilitaryCount)
		{
			return SELECT_TARGET1;
		}

		if (ip2.OwnedMilitaryCount > ip1.OwnedMilitaryCount)
		{
			return SELECT_TARGET2;
		}

		// Defend station
		if (ip1.OwnedStationCount > ip2.OwnedStationCount)
		{
			return SELECT_TARGET1;
		}

		if (ip2.OwnedStationCount > ip1.OwnedStationCount)
		{
			return SELECT_TARGET2;
		}

		// Defend cargo
		if (ip1.OwnedCargoCount > ip2.OwnedCargoCount)
		{
			return SELECT_TARGET1;
		}

		if (ip2.OwnedCargoCount > ip1.OwnedCargoCount)
		{
			return SELECT_TARGET2;
		}

		return ip1.EnemyArmyCombatPoints > ip2.EnemyArmyCombatPoints;
	}


	// Cargo or station
	return FMath::RandBool();
}


TArray<WarTarget> UFlareCompanyAI::GenerateWarTargetList(AIWarContext& WarContext)
{
	TArray<WarTarget> WarTargetList;

	for (UFlareSimulatedSector* Sector : WarContext.KnownSectors)
	{
		bool IsTarget = false;

		if (Sector->GetSectorBattleState(Company).HasDanger)
		{
			IsTarget = true;
		}
		else
		{
			for (UFlareSimulatedSpacecraft* Spacecraft : Sector->GetSectorSpacecrafts())
			{
				if (!Spacecraft->IsStation() && !Spacecraft->IsMilitary() && Spacecraft->GetDamageSystem()->IsUncontrollable())
				{
					// Don't target uncontrollable ships
					continue;
				}

				if (WarContext.Enemies.Contains(Spacecraft->GetCompany()))
				{
					IsTarget = true;
					break;
				}
			}
		}

		if (!IsTarget)
		{
			continue;
		}

		WarTarget Target;
		Target.Sector = Sector;
		Target.EnemyArmyCombatPoints = 0;
		Target.EnemyArmyLCombatPoints = 0;
		Target.EnemyArmySCombatPoints = 0;
		Target.EnemyCargoCount = 0;
		Target.EnemyStationCount = 0;
		Target.OwnedArmyCombatPoints = 0;
		Target.OwnedArmyAntiSCombatPoints = 0;
		Target.OwnedArmyAntiLCombatPoints = 0;
		Target.OwnedCargoCount = 0;
		Target.OwnedStationCount = 0;
		Target.OwnedMilitaryCount = 0;
		Target.WarTargetIncomingFleets = GenerateWarTargetIncomingFleets(WarContext, Sector);

		for (UFlareSimulatedSpacecraft* Spacecraft : Sector->GetSectorSpacecrafts())
		{
			if (WarContext.Enemies.Contains(Spacecraft->GetCompany()))
			{
				if (Spacecraft->IsStation())
				{
					Target.EnemyStationCount++;
				}
				else if (Spacecraft->IsMilitary())
				{
					int32 ShipCombatPoints = Spacecraft->GetCombatPoints(true);
					Target.EnemyArmyCombatPoints += ShipCombatPoints;

					if (Spacecraft->GetSize() == EFlarePartSize::L)
					{
						Target.EnemyArmyLCombatPoints += ShipCombatPoints;
					}
					else
					{
						Target.EnemyArmySCombatPoints += ShipCombatPoints;
					}

					if(ShipCombatPoints > 0)
					{
						Target.ArmedDefenseCompanies.AddUnique(Spacecraft->GetCompany());
					}
				}
				else
				{
					Target.EnemyCargoCount++;
				}
			}
			else if (WarContext.Allies.Contains(Spacecraft->GetCompany()))
			{
				if (Spacecraft->IsStation())
				{
					Target.OwnedStationCount++;
				}
				else if (Spacecraft->IsMilitary())
				{
					int32 ShipCombatPoints = Spacecraft->GetCombatPoints(true);
					Target.OwnedArmyCombatPoints += ShipCombatPoints;
					Target.OwnedMilitaryCount++;

					if (Spacecraft->GetWeaponsSystem()->HasAntiLargeShipWeapon())
					{
						Target.OwnedArmyAntiLCombatPoints += ShipCombatPoints;
					}

					if (Spacecraft->GetWeaponsSystem()->HasAntiSmallShipWeapon())
					{
						Target.OwnedArmyAntiSCombatPoints += ShipCombatPoints;
					}
				}
				else
				{
					Target.OwnedCargoCount++;
				}
			}
		}

		if (Target.OwnedArmyAntiLCombatPoints <= Target.EnemyArmyLCombatPoints * Behavior->RetreatThreshold ||
				Target.OwnedArmyAntiSCombatPoints <= Target.EnemyArmySCombatPoints * Behavior->RetreatThreshold)
		{
			WarTargetList.Add(Target);

			// Retreat
			if (Target.EnemyArmyCombatPoints > 0)
			{
				TMap<UFlareCompany*, TArray<UFlareFleet*>> MovableFleets = GenerateWarFleetList(WarContext, Target.Sector);
				if (MovableFleets.Num() > 0)
				{
					TArray<UFlareCompany*> InvolvedCompanies;
					MovableFleets.GenerateKeyArray(InvolvedCompanies);
					TArray<UFlareFleet*> UsableFleets;
					for (UFlareCompany* InvolvedCompanies : InvolvedCompanies)
					{
						UsableFleets.Append(MovableFleets[InvolvedCompanies]);
					}

					for (UFlareFleet* AssignedFleet : UsableFleets)
					{
						// Find nearest sector without danger with available FS and travel there
						UFlareSimulatedSector* RetreatSector = FindNearestSectorWithFS(WarContext, Target.Sector, AssignedFleet);
						if (!RetreatSector)
						{
							RetreatSector = FindNearestSectorWithPeace(WarContext, Target.Sector, AssignedFleet);
						}

						if (RetreatSector)
						{
							FLOGV("UpdateWarMilitaryMovement %s : move %s from %s to %s for retreat",
								*Company->GetCompanyName().ToString(),
								*AssignedFleet->GetIdentifier().ToString(),
								*AssignedFleet->GetCurrentSector()->GetSectorName().ToString(),
								*RetreatSector->GetSectorName().ToString());
								Game->GetGameWorld()->StartTravel(AssignedFleet, RetreatSector);
						}
					}
				}
			}
		}
	}

	WarTargetList.Sort(&WarTargetComparator);

	return WarTargetList;
}

TArray<DefenseSector> UFlareCompanyAI::GenerateDefenseSectorList(AIWarContext& WarContext)
{
	TArray<DefenseSector> DefenseSectorList;

	for (UFlareSimulatedSector* Sector : WarContext.KnownSectors)
	{
		FFlareSectorBattleState BattleState = Sector->GetSectorBattleState(Company);

		DefenseSector Target;
		Target.Sector = Sector;
		Target.TotalCombatPoints = 0;
		Target.AlliedCombatPoints = 0;
		Target.OwnedCombatPoints = 0;

		Target.ArmyAntiSCombatPoints = 0;
		Target.ArmyAntiLCombatPoints = 0;
		Target.ArmyLargeShipCombatPoints = 0;
		Target.ArmySmallShipCombatPoints = 0;
		Target.LargeShipArmyCount = 0;
		Target.SmallShipArmyCount = 0;
		Target.PrisonersKeeper = NULL;

		if(BattleState.BattleWon)
		{
			// Keep prisoners
			int32 MinCombatPoints = MAX_int32;
			for (UFlareSimulatedSpacecraft* Ship : Sector->GetSectorShips())
			{
				if (Ship->GetDescription()->IsDroneShip)
				{
					continue;
				}

				if (!Ship->IsMilitary()
					|| !WarContext.Allies.Contains(Ship->GetCompany())
					|| Ship->CanTravel() == false)
				{
					continue;
				}

				int32 ShipCombatPoints = Ship->GetCombatPoints(true);
				if (ShipCombatPoints == 0)
				{
					continue;
				}

				if (Ship->GetCompany() == Company)
				{
					if (ShipCombatPoints < MinCombatPoints)
					{
						MinCombatPoints = ShipCombatPoints;
						Target.PrisonersKeeper = Ship;
					}
				}
			}

			if (Target.PrisonersKeeper && Target.PrisonersKeeper->GetCurrentFleet()->GetShipCount() > 1)
			{
				UFlareFleet* NewFleet = Target.PrisonersKeeper->GetCompany()->CreateAutomaticFleet(Target.PrisonersKeeper);
			}
		}

		for (UFlareSimulatedSpacecraft* Ship : Sector->GetSectorShips())
		{
			if (!Ship->IsMilitary()
				|| !WarContext.Allies.Contains(Ship->GetCompany())
				|| Ship->CanTravel() == false)
			{
				continue;
			}

			int32 ShipCombatPoints = Ship->GetCombatPoints(true);
			if (ShipCombatPoints == 0)
			{
				continue;
			}

			if (Ship->GetCompany() != Company)
			{
				Target.AlliedCombatPoints += ShipCombatPoints;
			}
			else
			{
				Target.OwnedCombatPoints += ShipCombatPoints;
			}
			
			Target.TotalCombatPoints += ShipCombatPoints;

			if (Ship->GetSize() == EFlarePartSize::L)
			{
				Target.ArmyLargeShipCombatPoints += ShipCombatPoints;
				Target.LargeShipArmyCount++;
			}
			else
			{
				Target.ArmySmallShipCombatPoints += ShipCombatPoints;
				Target.SmallShipArmyCount++;
			}

			if (Ship->GetWeaponsSystem()->HasAntiLargeShipWeapon())
			{
				Target.ArmyAntiLCombatPoints += ShipCombatPoints;
			}

			if (Ship->GetWeaponsSystem()->HasAntiSmallShipWeapon())
			{
				Target.ArmyAntiSCombatPoints += ShipCombatPoints;
			}
		}

		Target.CapturingStations.Empty();
		Target.CapturingShips.Empty();
		TArray<UFlareSimulatedSpacecraft*>& Stations =  Sector->GetSectorStations();
		TArray<UFlareSimulatedSpacecraft*> FilteredStations;

		for (UFlareSimulatedSpacecraft* Station : Stations)
		{
			// Capturing station
			if (WarContext.Enemies.Contains(Station->GetCompany()))
			{
				FilteredStations.Add(Station);
			}
		}

		for (UFlareCompany* AlliedCompany : WarContext.Allies)
		{
			if (AlliedCompany->GetCaptureOrderCountInSector(Sector) > 0)
			{
				Target.CapturingStations.Add(true);
			}
			else
			{
				bool BegunCapture = false;
				for (UFlareSimulatedSpacecraft* Station : FilteredStations)
				{
					// Capturing station
					if (AlliedCompany->CanStartCapture(Station))
					{
						if (!BegunCapture)
						{
							BegunCapture = true;
							Target.CapturingStations.Add(true);
						}
						continue;
					}
				}
				if (!BegunCapture)
				{
					Target.CapturingStations.Add(false);
				}
			}


			if (AlliedCompany->GetCaptureShipOrderCountInSector(Sector) > 0)
			{
				Target.CapturingShips.Add(true);
			}
			else
			{
				Target.CapturingShips.Add(false);
			}
		}

		if (Target.TotalCombatPoints > 0 || Target.PrisonersKeeper != nullptr)
		{
			DefenseSectorList.Add(Target);
		}
	}

	return DefenseSectorList;
}

TArray<UFlareSimulatedSpacecraft*> UFlareCompanyAI::GenerateWarShipList(AIWarContext& WarContext, UFlareSimulatedSector* Sector, UFlareSimulatedSpacecraft* ExcludeShip)
{
	TArray<UFlareSimulatedSpacecraft*> WarShips;
	for (UFlareSimulatedSpacecraft* Ship : Sector->GetSectorShips())
	{
		if (WarContext.Allies.Contains(Ship->GetCompany())
			&& Ship->CanTravel()
			&& !Ship->GetDamageSystem()->IsDisarmed()
			&& !Ship->GetDamageSystem()->IsStranded()
			&& Ship != ExcludeShip
			&& !Ship->GetDescription()->IsDroneShip)
		{
			WarShips.Add(Ship);
		}
	}

	return WarShips;
}

TMap<UFlareCompany*, TArray<UFlareFleet*>> UFlareCompanyAI::GenerateWarFleetList(AIWarContext& WarContext, UFlareSimulatedSector* Sector, UFlareSimulatedSpacecraft* ExcludeShip)
{
	TMap<UFlareCompany*, TArray<UFlareFleet*>> NewWarFleets;

	for (UFlareSimulatedSpacecraft* Ship : Sector->GetSectorShips())
	{
		if (WarContext.Allies.Contains(Ship->GetCompany())
		 &&  Ship->CanTravel()
		 && !Ship->GetDamageSystem()->IsDisarmed()
		 && !Ship->GetDamageSystem()->IsStranded()
		 && Ship != ExcludeShip
		 && !Ship->GetDescription()->IsDroneShip)
		{
			if (NewWarFleets.Contains(Ship->GetCompany()))
			{
				TArray<UFlareFleet*> &CompanyFleets = NewWarFleets[Ship->GetCompany()];

				UFlareFleet* SelectedFleet = nullptr;
				for (UFlareFleet* PossibleFleets : CompanyFleets)
				{
					if (PossibleFleets->GetShipCount() < PossibleFleets->GetMaxShipCount())
					{
						SelectedFleet = PossibleFleets;
						break;
					}
				}

				if (!SelectedFleet)
				{
					if (Ship->GetCurrentFleet()->GetShipCount() >= Ship->GetCurrentFleet()->GetMaxShipCount())
					{
						Ship->GetCompany()->CreateAutomaticFleet(Ship);
					}
					CompanyFleets.Emplace(Ship->GetCurrentFleet());
				}
				else
				{
					SelectedFleet->AddShip(Ship);
				}
			}
			else
			{
				TArray<UFlareFleet*> CompanyFleets;
				CompanyFleets.Emplace(Ship->GetCurrentFleet());
				NewWarFleets.Add(Ship->GetCompany(), CompanyFleets);
			}
		}
	}
	return NewWarFleets;
}

int64 UFlareCompanyAI::GetDefenseSectorTravelDuration(TArray<DefenseSector>& DefenseSectorList, const DefenseSector& OriginSector, UFlareFleet* Fleet)
{
	int64 MaxTravelDuration = 0;

	for (DefenseSector& Sector : DefenseSectorList)
	{
		int64 TravelDuration = UFlareTravel::ComputeTravelDuration(GetGame()->GetGameWorld(), OriginSector.Sector, Sector.Sector, Company, Fleet);
		if (TravelDuration > MaxTravelDuration)
		{
			MaxTravelDuration = TravelDuration;
		}
	}

	return MaxTravelDuration;
}

TArray<DefenseSector> UFlareCompanyAI::GetDefenseSectorListInRange(TArray<DefenseSector>& DefenseSectorList, const DefenseSector& OriginSector, int64 MaxTravelDuration, UFlareFleet* Fleet)
{
	TArray<DefenseSector> Sectors;

	for (DefenseSector& Sector : DefenseSectorList)
	{
		if (Sector.Sector == OriginSector.Sector)
		{
			continue;
		}

		int64 TravelDuration = UFlareTravel::ComputeTravelDuration(GetGame()->GetGameWorld(), OriginSector.Sector, Sector.Sector, Company,Fleet);
		if (TravelDuration <= MaxTravelDuration)
		{
			Sectors.Add(Sector);
		}
	}
	return Sectors;
}

inline static bool SectorDefenseDistanceComparator(const DefenseSector& ip1, const DefenseSector& ip2)
{
	int64 ip1TravelDuration = UFlareTravel::ComputeTravelDuration(ip1.Sector->GetGame()->GetGameWorld(), ip1.TempBaseSector, ip1.Sector, nullptr);
	int64 ip2TravelDuration = UFlareTravel::ComputeTravelDuration(ip1.Sector->GetGame()->GetGameWorld(), ip2.TempBaseSector, ip2.Sector, nullptr);

	return (ip1TravelDuration < ip2TravelDuration);
}

TArray<DefenseSector> UFlareCompanyAI::SortSectorsByDistance(UFlareSimulatedSector* BaseSector, TArray<DefenseSector> SectorsToSort)
{
	for (DefenseSector& Sector : SectorsToSort)
	{
		Sector.TempBaseSector = BaseSector;
	}

	SectorsToSort.Sort(&SectorDefenseDistanceComparator);
	return SectorsToSort;
}

void AIWarContext::Generate()
{
	float AttackThresholdSum = 0;
	float AttackThresholdCount = 0;

	for (UFlareCompany* Ally :  Allies)
	{
		for(UFlareSimulatedSector* Sector: Ally->GetKnownSectors())
		{
			KnownSectors.AddUnique(Sector);
		}

		Ally->GetAI()->GetBehavior()->Load(Ally);
		AttackThresholdSum += Ally->GetAI()->GetBehavior()->GetAttackThreshold();
		AttackThresholdCount++;
	}

	AttackThreshold = AttackThresholdSum/AttackThresholdCount;
}

void UFlareCompanyAI::UpdateWarMilitaryMovement()
{			
	auto GenerateAlliesCode = [&](UFlareCompany* iCompany)
	{
		int32 AlliesCode = 0x0;
		int32 CompanyMask = 0x1;

		for(UFlareCompany* OtherCompany : Game->GetGameWorld()->GetCompanies())
		{
			if(iCompany->IsAtWar(OtherCompany))
			{
				AlliesCode |= CompanyMask;
			}

			CompanyMask = CompanyMask<<1;
		}

		return AlliesCode;
	};

	auto GenerateAlliesList = [&]() {
		TArray<UFlareCompany*> Allies;
		Allies.Add(Company);
		int32 AlliesCode = GenerateAlliesCode(Company);

		for (UFlareCompany* OtherCompany: Company->GetOtherCompanies())
		{
			if(OtherCompany->IsPlayerCompany())
			{
				continue;
			}

			if(GenerateAlliesCode(OtherCompany) == AlliesCode)
			{
				Allies.Add(OtherCompany);
			}
		}

		return Allies;
	};

	auto GenerateEnemiesList = [&]() {
		TArray<UFlareCompany*> Enemies;

		for (UFlareCompany* OtherCompany: Company->GetOtherCompanies())
		{
			if(Company->IsAtWar(OtherCompany))
			{
				Enemies.Add(OtherCompany);
			}
		}

		return Enemies;
	};

	// Sort by fleet speeds
	struct FSortBySlowestFleet
	{
		UFlareSimulatedSector* OriginatingSector;
		UFlareSimulatedSector* TargetSector;

		FSortBySlowestFleet(UFlareSimulatedSector* OriginatingSector, UFlareSimulatedSector* TargetSector)
		: OriginatingSector(OriginatingSector), TargetSector(TargetSector)
		{
		}

		FORCEINLINE bool operator()(UFlareFleet& A, UFlareFleet& B) const
		{
			int64 TravelDurationFleetA = UFlareTravel::ComputeTravelDuration(A.GetFleetCompany()->GetGame()->GetGameWorld(), OriginatingSector, TargetSector, A.GetFleetCompany(), &A);
			int64 TravelDurationFleetB = UFlareTravel::ComputeTravelDuration(B.GetFleetCompany()->GetGame()->GetGameWorld(), OriginatingSector, TargetSector, B.GetFleetCompany(), &B);

			if (TravelDurationFleetA < TravelDurationFleetB)
			{
				return false;
			}
			else if (TravelDurationFleetA > TravelDurationFleetB)
			{
				return true;
			}
			return false;
		}
	};

	AIWarContext WarContext;

	WarContext.Allies = GenerateAlliesList();
	WarContext.Enemies = GenerateEnemiesList();
	WarContext.Generate();

	TArray<WarTarget> TargetList = GenerateWarTargetList(WarContext);
	TArray<DefenseSector> DefenseSectorList = GenerateDefenseSectorList(WarContext);

#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
	FLOGV("UpdateWarMilitaryMovement for %s : Target count %d, Defense sector count %d",
		*Company->GetCompanyName().ToString(),
		TargetList.Num(),
		DefenseSectorList.Num());
#endif

	// Manage attacking fleets
	for (WarTarget& Target : TargetList)
	{
		TArray<DefenseSector> SortedDefenseSectorList = SortSectorsByDistance(Target.Sector, DefenseSectorList);

		int64 IncomingFleetSlowestTravelDuration = -1;
		int64 IncomingFleetTotalCombatPoints = 0;
		int64 IncomingFleetArmyAntiLCombatPoints = 0;
		int64 IncomingFleetArmyAntiSCombatPoints = 0;

		for (WarTargetIncomingFleet& Fleet : Target.WarTargetIncomingFleets)
		{
			IncomingFleetTotalCombatPoints += Fleet.ArmyCombatPoints;
			IncomingFleetArmyAntiLCombatPoints += Fleet.ArmyAntiLCombatPoints;
			IncomingFleetArmyAntiSCombatPoints += Fleet.ArmyAntiSCombatPoints;

			if (IncomingFleetSlowestTravelDuration == -1 || Fleet.TravelDuration > IncomingFleetSlowestTravelDuration)
			{
				IncomingFleetSlowestTravelDuration = Fleet.TravelDuration;
			}
		}
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
		FLOGV("IncomingFleetSlowestTravelDuration set to %d",
		IncomingFleetSlowestTravelDuration);
#endif

		for (DefenseSector& Sector : SortedDefenseSectorList)
		{
			if (Target.Sector == Sector.Sector)
			{
				continue;
			}

			// Check if there is an allied incoming fleet
			int64 TotalCombatPoints = Sector.TotalCombatPoints + IncomingFleetTotalCombatPoints;

#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
			FLOGV("check attack from %s to %s : CombatPoints=%d (total = %d), EnemyArmyCombatPoints=%d",
				*Sector.Sector->GetSectorName().ToString(),
				*Target.Sector->GetSectorName().ToString(),
				Sector.TotalCombatPoints, TotalCombatPoints, Target.EnemyArmyCombatPoints);
#endif

			// Check if the army is strong enough
			if (TotalCombatPoints < Target.EnemyArmyCombatPoints * WarContext.AttackThreshold)
			{
				// Army too weak
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("army at %s too weak to attack %s : CombatPoints = %lld, EnemyArmyCombatPoints = %lld ",
					*Sector.Sector->GetSectorName().ToString(),
					*Target.Sector->GetSectorName().ToString(),
					TotalCombatPoints, Target.EnemyArmyCombatPoints);
#endif

				continue;
			}

			bool CapturingStation = Sector.CapturingStations[WarContext.Allies.Find(Company)];
			bool CapturingShip = Sector.CapturingShips[WarContext.Allies.Find(Company)];
			if (Target.EnemyArmyCombatPoints == 0 && (CapturingStation || CapturingShip))
			{
				// Capturing station or ship, don't consider moving from here just for enemy trade ships/stations
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("army at %s won't move to attack %s : no enemies at possible target and capturing something Station = %d Ship = %d",
					*Sector.Sector->GetSectorName().ToString(),
					*Target.Sector->GetSectorName().ToString(),
					CapturingStation, CapturingShip);
#endif

				continue;
			}

			if (CapturingStation || CapturingShip)
			{
				int32 DefenseIncomingEnemyArmyCombatPoints = 0;

				TArray<WarTargetIncomingFleet> WarTargetIncomingFleets = GenerateWarTargetIncomingFleets(WarContext, Sector.Sector, true);
				for (WarTargetIncomingFleet& Fleet : WarTargetIncomingFleets)
				{
					DefenseIncomingEnemyArmyCombatPoints += Fleet.ArmyCombatPoints;
				}

				if (WarTargetIncomingFleets.Num())
				{
					if (Sector.OwnedCombatPoints >= (DefenseIncomingEnemyArmyCombatPoints * Behavior->RetreatThreshold))
					{
						// Capturing station or ship, don't consider moving from here just for enemy trade ships/stations
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
						FLOGV("army at %s won't move to attack %s : incoming enemy force smaller than ours, defensive position. %d vs %d",
							*Sector.Sector->GetSectorName().ToString(),
							*Target.Sector->GetSectorName().ToString(),
							Sector.OwnedCombatPoints,
							DefenseIncomingEnemyArmyCombatPoints);
#endif
						continue;
					}
				}
			}

			// Should go defend ! Assemble a fleet
			TMap<UFlareCompany*, TArray<UFlareFleet*>> MovableFleets = GenerateWarFleetList(WarContext, Sector.Sector,Sector.PrisonersKeeper);

#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
			FLOGV("Initial MovableFleets = %d", MovableFleets.Num());
#endif
			if (MovableFleets.Num() > 0)
			{
				int64 TotalArmyAntiLCombatPoints = Sector.ArmyAntiLCombatPoints + IncomingFleetArmyAntiLCombatPoints;
				int64 TotalArmyAntiSCombatPoints = Sector.ArmyAntiSCombatPoints + IncomingFleetArmyAntiSCombatPoints;
				int32 SentAntiLFleetCombatPoints = IncomingFleetArmyAntiLCombatPoints;
				int32 SentAntiSFleetCombatPoints = IncomingFleetArmyAntiSCombatPoints;

				int32 AntiLFleetCombatPointsLimit = Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold * 1.5;
				int32 AntiSFleetCombatPointsLimit = Target.EnemyArmySCombatPoints * WarContext.AttackThreshold * 1.5;

				TArray<UFlareCompany*> InvolvedCompanies;
				MovableFleets.GenerateKeyArray(InvolvedCompanies);
				TArray<UFlareFleet*> UsableFleets;
				for (UFlareCompany* InvolvedCompanies : InvolvedCompanies)
				{
					UsableFleets.Append(MovableFleets[InvolvedCompanies]);
				}

				UsableFleets.Sort(FSortBySlowestFleet(Sector.Sector,Target.Sector));

				// Check if weapon are optimal
				// Check if the army is strong enough

				if (TotalArmyAntiLCombatPoints < Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold ||
					TotalArmyAntiSCombatPoints < Target.EnemyArmySCombatPoints * WarContext.AttackThreshold)
				{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
					FLOGV("army at %s want to attack %s after upgrade : ArmyAntiLCombatPoints=%d, EnemyArmyLCombatPoints=%d, ArmyAntiSCombatPoints=%d, EnemyArmySCombatPoints=%d",
						*Sector.Sector->GetSectorName().ToString(),
						*Target.Sector->GetSectorName().ToString(),
						Sector.ArmyAntiLCombatPoints, Target.EnemyArmyLCombatPoints, Sector.ArmyAntiSCombatPoints, Target.EnemyArmySCombatPoints);
#endif
					if (!UpgradeMilitaryFleet(WarContext, Target, Sector, UsableFleets))
					{
						// cannot upgrade, don't attack
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
						FLOGV("army at %s won't attack %s : upgrade failed",
							*Sector.Sector->GetSectorName().ToString(),
							*Target.Sector->GetSectorName().ToString());
#endif
						continue;
					}
				}
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("army from %s launching attack %s !",
					*Sector.Sector->GetSectorName().ToString(),
					*Target.Sector->GetSectorName().ToString());
#endif

				int32 SentCombatPoints = 0;

#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				int32 SentShips = 0;
				FLOGV("- UsableFleets= %d", MovableFleets.Num());
				FLOGV("- AntiLFleetCombatPointsLimit = %d", AntiLFleetCombatPointsLimit);
				FLOGV("- AntiSFleetCombatPointsLimit = %d", AntiSFleetCombatPointsLimit);
#endif
				int64 SlowestTravelDuration = 0;
				if (IncomingFleetSlowestTravelDuration > 0)
				{
					SlowestTravelDuration = IncomingFleetSlowestTravelDuration;
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
					FLOGV("SlowestTravelDuration set from incoming fleet, %d");
#endif
				}
				else
				{
					SlowestTravelDuration = UFlareTravel::ComputeTravelDuration(GetGame()->GetGameWorld(), Sector.Sector, Target.Sector, UsableFleets[0]->GetFleetCompany(), UsableFleets[0]);
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
					FLOGV("No incoming fleets, SlowestTravelDuration set from travel time");
#endif
				}

#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("SlowestTravelDuration = %d", SlowestTravelDuration);
#endif
				for (UFlareFleet* SelectedFleet : UsableFleets)
				{
					if (SelectedFleet->GetFleetCompany() != Company)
					{
						//allied fleet, extra checks before grabbing
						bool CapturingStation = Sector.CapturingStations[WarContext.Allies.Find(SelectedFleet->GetFleetCompany())];

						if (CapturingStation)
						{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
							FLOGV("Allied fleet %s is still capturing a station, they are unavailable for a joint attack",
								*SelectedFleet->GetFleetName().ToString());
#endif
							continue;
						}

						bool CapturingShip = Sector.CapturingShips[WarContext.Allies.Find(SelectedFleet->GetFleetCompany())];
						if (CapturingShip)
						{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
							FLOGV("Allied fleet %s is still capturing a ship, they are unavailable for a joint attack",
								*SelectedFleet->GetFleetName().ToString());
#endif
							continue;
						}
					}

					int64 FleetTravelDuration = UFlareTravel::ComputeTravelDuration(GetGame()->GetGameWorld(), Sector.Sector, Target.Sector, SelectedFleet->GetFleetCompany(), SelectedFleet);
					if (FleetTravelDuration != SlowestTravelDuration)
					{
						continue;
					}

					if ((SentAntiLFleetCombatPoints > AntiLFleetCombatPointsLimit && SentAntiSFleetCombatPoints > AntiSFleetCombatPointsLimit))
					{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
						FLOG("Sent enough, stop sending.");
						FLOGV("SentAntiLFleetCombatPoints %d/%d. AntiLFleetCombatPointsLimit",
						SentAntiLFleetCombatPoints, AntiLFleetCombatPointsLimit);
						FLOGV("SentAntiSFleetCombatPoints %d/%d. AntiSFleetCombatPointsLimit",
						SentAntiSFleetCombatPoints, AntiSFleetCombatPointsLimit);
#endif
						break;
					}

					for (UFlareSimulatedSpacecraft* SelectedShip : SelectedFleet->GetShips())
					{
						int32 ShipCombatPoints = SelectedShip->GetCombatPoints(true);
						if (SelectedShip->GetWeaponsSystem()->HasAntiLargeShipWeapon())
						{
							SentAntiLFleetCombatPoints += ShipCombatPoints;
							Sector.TotalCombatPoints -= ShipCombatPoints;
							Sector.ArmyAntiLCombatPoints -= ShipCombatPoints;

							if (SelectedFleet->GetFleetCompany() == Company)
							{
								Sector.OwnedCombatPoints -= ShipCombatPoints;
							}
							else
							{
								Sector.AlliedCombatPoints -= ShipCombatPoints;
							}

						}
						else if (SelectedShip->GetWeaponsSystem()->HasAntiSmallShipWeapon())
						{
							SentAntiSFleetCombatPoints += ShipCombatPoints;
							Sector.TotalCombatPoints -= ShipCombatPoints;
							Sector.ArmyAntiSCombatPoints -= ShipCombatPoints;

							if (SelectedFleet->GetFleetCompany() == Company)
							{
								Sector.OwnedCombatPoints -= ShipCombatPoints;
							}
							else
							{
								Sector.AlliedCombatPoints -= ShipCombatPoints;
							}
						}

#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
						FLOGV("UpdateWarMilitaryMovement %s : move %s from %s to %s for attack",
							*Company->GetCompanyName().ToString(),
							*SelectedShip->GetImmatriculation().ToString(),
							*SelectedShip->GetCurrentSector()->GetSectorName().ToString(),
							*Target.Sector->GetSectorName().ToString());
						FLOGV("- AntiLFleetCombatPoints= %d", SentAntiLFleetCombatPoints);
						FLOGV("- AntiSFleetCombatPoints= %d", SentAntiSFleetCombatPoints);
#endif
						SentCombatPoints += ShipCombatPoints;
					}

#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
					SentShips += SelectedFleet->GetShipCount();
#endif
					Game->GetGameWorld()->StartTravel(SelectedFleet, Target.Sector);

					if (FleetTravelDuration > 1)
					{
						Game->GetQuestManager()->GetQuestGenerator()->GenerateAttackQuests(SelectedFleet->GetFleetCompany(), SentCombatPoints, Target, FleetTravelDuration);
					}
				}

#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("- Attack Sent %d total ships", SentShips);
				FLOGV("- Sent Total Combat Points = %d", SentCombatPoints);
				FLOGV("- Sent AntiLFleetCombatPointsLimit = %d", SentAntiLFleetCombatPoints);
				FLOGV("- Sent AntiSFleetCombatPointsLimit = %d", SentAntiSFleetCombatPoints);
#endif
				if (Sector.OwnedCombatPoints == 0)
				{
					DefenseSectorList.Remove(Sector);
				}
			}
		}
	}

	// Manage remaining ships for defense
	for (DefenseSector& Sector : DefenseSectorList)
	{
		if (Sector.PrisonersKeeper)
		{
			//Prisoner ship is the only combat ship in the sector, check if we need to retreat
			int32 PrisonerShipCombatPoints = Sector.PrisonersKeeper->GetCombatPoints(true);
			if (Sector.OwnedCombatPoints == PrisonerShipCombatPoints)
			{
				bool IncomingEnemyFleet = false;
				bool IncomingEnemyFleetNeedRetreat = false;
				int32 IncomingEnemyFleetArmySmallCombatPoints = 0;
				int32 IncomingEnemyFleetArmyLargeCombatPoints = 0;

				TArray<WarTargetIncomingFleet> WarTargetIncomingFleets = GenerateWarTargetIncomingFleets(WarContext, Sector.Sector,true);
				for (WarTargetIncomingFleet& Fleet : WarTargetIncomingFleets)
				{
					if (Fleet.TravelDuration <= 2)
					{
						IncomingEnemyFleet = true;
						IncomingEnemyFleetArmySmallCombatPoints = Fleet.ArmySmallCombatPoints;
						IncomingEnemyFleetArmyLargeCombatPoints = Fleet.ArmyLargeCombatPoints;

						if (PrisonerShipCombatPoints <= Fleet.ArmyCombatPoints * Behavior->RetreatThreshold)
						{
							IncomingEnemyFleetNeedRetreat = true;
							break;
						}
					}
				}

				if (IncomingEnemyFleetNeedRetreat)
				{
					// Find nearest sector without danger with available FS and travel there
					UFlareSimulatedSector* RetreatSector = FindNearestSectorWithFS(WarContext, Sector.Sector, Sector.PrisonersKeeper->GetCurrentFleet());
					if (!RetreatSector)
					{
						RetreatSector = FindNearestSectorWithPeace(WarContext, Sector.Sector, Sector.PrisonersKeeper->GetCurrentFleet());
					}

					if (RetreatSector)
					{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
						FLOGV("UpdateWarMilitaryMovement %s : move prisoner keeper %s from %s to %s for retreat",
						*Sector.PrisonersKeeper->GetCompany()->GetCompanyName().ToString(),
						*Sector.PrisonersKeeper->GetCurrentFleet()->GetIdentifier().ToString(),
						*Sector.PrisonersKeeper->GetCurrentSector()->GetSectorName().ToString(),
						*RetreatSector->GetSectorName().ToString());
#endif
						Game->GetGameWorld()->StartTravel(Sector.PrisonersKeeper->GetCurrentFleet(), RetreatSector);
						Sector.PrisonersKeeper = nullptr;
					}
				}

				else if (IncomingEnemyFleet)
				{
					//make sure the prisoner ship attempts to actually arm itself even if its combat points is above the incoming threat
					Sector.PrisonersKeeper->GetCompany()->GetAI()->UpgradeShip(Sector.PrisonersKeeper, IncomingEnemyFleetArmySmallCombatPoints > IncomingEnemyFleetArmyLargeCombatPoints ? EFlarePartSize::S : EFlarePartSize::L, false, false);
				}
				else if (Sector.PrisonersKeeper->GetSize() == EFlarePartSize::S)
				{
					for (UFlareSimulatedSpacecraft* Ship : Sector.Sector->GetSectorShips())
					{
						if (WarContext.Enemies.Contains(Ship->GetCompany()))
						{
							Sector.PrisonersKeeper->GetCompany()->GetAI()->UpgradeShip(Sector.PrisonersKeeper, Ship->GetSize(), true, false);
							break;
						}
					}
				}
			}
		}

		bool CapturingStations = Sector.CapturingStations[WarContext.Allies.Find(Company)];

		// Capturing station, don't move
		if (CapturingStations)
		{
			// Start capture
			for (UFlareSimulatedSpacecraft* Station : Sector.Sector->GetSectorStations())
			{
				Company->StartCapture(Station);
			}
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
			FLOGV("army at %s won't move to defend: capturing station",
				*Sector.Sector->GetSectorName().ToString());
#endif
			continue;
		}

		bool CapturingShip = Sector.CapturingShips[WarContext.Allies.Find(Company)];

		if (CapturingShip)
		{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
			FLOGV("army at %s won't move to defend: capturing ship",
				*Sector.Sector->GetSectorName().ToString());
#endif
			continue;
		}

		// Check if need fleet supply
		int32 NeededFS;
		int32 TotalNeededFS;

		int32 CumulatedTotalNeededFS = 0;
		int64 MaxDuration;

		TArray<UFlareSimulatedSpacecraft*> MovableShips = GenerateWarShipList(WarContext, Sector.Sector, Sector.PrisonersKeeper);

		SectorHelper::GetRefillFleetSupplyNeeds(Sector.Sector, MovableShips, NeededFS, TotalNeededFS, MaxDuration, true);
		CumulatedTotalNeededFS += TotalNeededFS;

		SectorHelper::GetRepairFleetSupplyNeeds(Sector.Sector, MovableShips, NeededFS, TotalNeededFS, MaxDuration, true);
		CumulatedTotalNeededFS += TotalNeededFS;

		if (CumulatedTotalNeededFS > 0)
		{
			// FS need

			int32 AvailableFS;
			int32 OwnedFS;
			int32 AffordableFS;
			SectorHelper::GetAvailableFleetSupplyCount(Sector.Sector, Company, OwnedFS, AvailableFS, AffordableFS);

			if (AvailableFS == 0)
			{
				// Find nearest sector without danger with available FS and travel there
				bool FoundRepairSector = false;

				//Ships try to break off from their fleets so faster ships can possibly initiate resupplies earlier.
				for (UFlareSimulatedSpacecraft* Ship : MovableShips)
				{
					if (!Ship->GetDescription()->IsDroneShip && Ship->GetCurrentFleet()->GetShipCount() > 1)
					{
						Ship->GetCompany()->CreateAutomaticFleet(Ship);
					}

					UFlareSimulatedSector* RepairSector = FindNearestSectorWithFS(WarContext, Sector.Sector, Ship->GetCurrentFleet());
					if (RepairSector)
					{
						FLOGV("UpdateWarMilitaryMovement %s : move %s from %s to %s for repair/refill",
							*Company->GetCompanyName().ToString(),
							*Ship->GetImmatriculation().ToString(),
							*Ship->GetCurrentSector()->GetSectorName().ToString(),
							*RepairSector->GetSectorName().ToString());

						Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), RepairSector);
						FoundRepairSector = true;
					}
				}
				if (FoundRepairSector)
				{
					continue;
				}
			}
		}

		TMap<UFlareCompany*, TArray<UFlareFleet*>> MovableFleets = GenerateWarFleetList(WarContext, Sector.Sector, Sector.PrisonersKeeper);
		TArray<UFlareCompany*> InvolvedCompanies;
		MovableFleets.GenerateKeyArray(InvolvedCompanies);

		TArray<UFlareFleet*> UsableFleets;
		for (UFlareCompany* InvolvedCompanies : InvolvedCompanies)
		{
			UsableFleets.Append(MovableFleets[InvolvedCompanies]);
		}

		for (UFlareFleet* SelectedFleet : UsableFleets)
		{

			int64 MaxTravelDuration = GetDefenseSectorTravelDuration(DefenseSectorList, Sector, SelectedFleet);
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
			FLOGV("army at %s MaxTravelDuration=%lld",
				*Sector.Sector->GetSectorName().ToString(), MaxTravelDuration);
#endif

			for (int32 TravelDuration = 1; TravelDuration <= MaxTravelDuration; TravelDuration++)
			{
				TArray<DefenseSector> DefenseSectorListInRange = GetDefenseSectorListInRange(DefenseSectorList, Sector, TravelDuration,SelectedFleet);

#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("army at %s DefenseSectorListInRange(%d)=%d",
					*Sector.Sector->GetSectorName().ToString(), TravelDuration, DefenseSectorListInRange.Num());
#endif
				if (DefenseSectorListInRange.Num() == 0)
				{
					continue;
				}

				// Find bigger
				DefenseSector StrongestSector;
				StrongestSector.Sector = NULL;
				StrongestSector.TotalCombatPoints = 0;
				for (DefenseSector& DistantSector : DefenseSectorListInRange)
				{
					if (!StrongestSector.Sector || StrongestSector.TotalCombatPoints < DistantSector.TotalCombatPoints)
					{
						StrongestSector = DistantSector;
					}
				}

				if (StrongestSector.TotalCombatPoints > Sector.TotalCombatPoints)
				{
					// There is a stronger sector, travel here if no incoming army before
					bool IncomingFleet = false;
					TArray<WarTargetIncomingFleet> WarTargetIncomingFleets = GenerateWarTargetIncomingFleets(WarContext, StrongestSector.Sector);
					for (WarTargetIncomingFleet& Fleet : WarTargetIncomingFleets)
					{
						if (Fleet.TravelDuration <= TravelDuration)
						{
							IncomingFleet = true;
							break;
						}
					}

					// Wait incoming fleets
					if (IncomingFleet)
					{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
						FLOGV("army at %s won't move to defend: incoming fleet",
							*Sector.Sector->GetSectorName().ToString());
#endif
						break;
					}

					// Send ships
					TArray<UFlareSimulatedSpacecraft*> StillMovableShips = GenerateWarShipList(WarContext, Sector.Sector, Sector.PrisonersKeeper);
					for (UFlareSimulatedSpacecraft* Ship : SelectedFleet->GetShips())
					{
						FLOGV("UpdateWarMilitaryMovement%s : move %s from %s to %s for defense",
						*Company->GetCompanyName().ToString(),
						*Ship->GetImmatriculation().ToString(),
						*Ship->GetCurrentSector()->GetSectorName().ToString(),
						*StrongestSector.Sector->GetSectorName().ToString());

						int32 ShipCombatPoints = Ship->GetCombatPoints(true);
						Sector.TotalCombatPoints -= ShipCombatPoints;

						if (SelectedFleet->GetFleetCompany() == Company)
						{
							Sector.OwnedCombatPoints -= ShipCombatPoints;
						}
						else
						{
							Sector.AlliedCombatPoints -= ShipCombatPoints;
						}
					}

					Game->GetGameWorld()->StartTravel(SelectedFleet, StrongestSector.Sector);
				}
				else
				{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
					FLOGV("army at %s won't move to defend at %s: army is weaker (StrongestSector.CombatPoints=%d Sector.CombatPoints=%d)",
						*Sector.Sector->GetSectorName().ToString(),
						*StrongestSector.Sector->GetSectorName().ToString(),
						StrongestSector.TotalCombatPoints, Sector.TotalCombatPoints);
#endif
				}
				break;
			}
		}
	}
}

UFlareSimulatedSector* UFlareCompanyAI::FindNearestSectorWithPeace(AIWarContext& WarContext, UFlareSimulatedSector* OriginSector, UFlareFleet* Fleet)
{
	UFlareSimulatedSector* NearestSector = NULL;
	int64 NearestDuration = 0;

	for (UFlareSimulatedSector* Sector : WarContext.KnownSectors)
	{
		if (Sector == OriginSector)
		{
			continue;
		}

		FFlareSectorBattleState BattleState = Sector->GetSectorBattleState(Company);
		if (BattleState.HasDanger)
		{
			continue;
		}

		int64 Duration = UFlareTravel::ComputeTravelDuration(Sector->GetGame()->GetGameWorld(), OriginSector, Sector, Company, Fleet);

		if (!NearestSector || Duration < NearestDuration)
		{
			NearestSector = Sector;
			NearestDuration = Duration;
		}

	}
	return NearestSector;
}

UFlareSimulatedSector* UFlareCompanyAI::FindNearestSectorWithFS(AIWarContext& WarContext, UFlareSimulatedSector* OriginSector, UFlareFleet* Fleet)
{
	UFlareSimulatedSector* NearestSector = NULL;
	int64 NearestDuration = 0;

	for (UFlareSimulatedSector* Sector : WarContext.KnownSectors)
	{
		if (Sector == OriginSector)
		{
			continue;
		}

		FFlareSectorBattleState BattleState = Sector->GetSectorBattleState(Company);
		if (BattleState.HasDanger)
		{
			continue;
		}

		int32 AvailableFS;
		int32 OwnedFS;
		int32 AffordableFS;

		SectorHelper::GetAvailableFleetSupplyCount(Sector, Company, OwnedFS, AvailableFS, AffordableFS);

		if (AvailableFS == 0)
		{
			continue;
		}

		int64 Duration = UFlareTravel::ComputeTravelDuration(Sector->GetGame()->GetGameWorld(), OriginSector, Sector, Company, Fleet);

		if (!NearestSector || Duration < NearestDuration)
		{
			NearestSector = Sector;
			NearestDuration = Duration;
		}
	}
	return NearestSector;
}

UFlareSimulatedSector* UFlareCompanyAI::FindNearestSectorWithUpgradePossible(AIWarContext& WarContext, UFlareSimulatedSector* OriginSector,UFlareFleet* Fleet)
{
	UFlareSimulatedSector* NearestSector = NULL;
	int64 NearestDuration = 0;

	for (UFlareSimulatedSector* Sector : WarContext.KnownSectors)
	{
		if (!CanUpgradeFromSector(Sector))
		{
			continue;
		}

		int64 Duration = UFlareTravel::ComputeTravelDuration(Sector->GetGame()->GetGameWorld(), OriginSector, Sector, Company, Fleet);

		if (!NearestSector || Duration < NearestDuration)
		{
			NearestSector = Sector;
			NearestDuration = Duration;
		}

	}
	return NearestSector;
}

bool UFlareCompanyAI::UpgradeShip(UFlareSimulatedSpacecraft* Ship, EFlarePartSize::Type WeaponTargetSize, bool AllowSalvager, bool AllowEngineUpgrades)
{
	UFlareSpacecraftComponentsCatalog* Catalog = Game->GetPC()->GetGame()->GetShipPartsCatalog();

	// Upgrade weapon
	if (!Ship->CanUpgrade(EFlarePartType::Weapon))
	{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("failed to upgrade %s : cannot upgrade weapons",
					*Ship->GetImmatriculation().ToString());
#endif
		return false;
	}

	// iterate to find best weapon
	UpgradeShipWeapon(Ship, WeaponTargetSize,AllowSalvager);

	if (AllowEngineUpgrades)
	{
		CompanyValue TotalValue = Company->GetCompanyValue();
		if (Company->GetMoney() > (TotalValue.TotalDailyProductionCost * Behavior->DailyProductionCostSensitivityMilitary))
		{
			// Chance to upgrade rcs (optional)
			if (FMath::RandBool() && Ship->CanUpgrade(EFlarePartType::RCS)) // 50 % chance
			{
				UpgradeShipRCS(Ship, EFlareBudget::Military);
			}

			// Chance to upgrade pod (optional)
			if (FMath::RandBool() && Ship->CanUpgrade(EFlarePartType::OrbitalEngine)) // 50 % chance
			{
				UpgradeShipEngine(Ship, EFlareBudget::Military);
			}
		}
	}

	return true;
}

bool UFlareCompanyAI::UpgradeShipWeapon(UFlareSimulatedSpacecraft* Ship, EFlarePartSize::Type WeaponTargetSize, bool AllowSalvager)
{
	UFlareSpacecraftComponentsCatalog* Catalog = Game->GetPC()->GetGame()->GetShipPartsCatalog();

	int32 WeaponGroupCount = Ship->GetDescription()->WeaponGroups.Num();
	float CostSafetyMargin = Behavior->CostSafetyMarginMilitaryShip;
	CompanyValue TotalValue = Company->GetCompanyValue();

	if (AllowSalvager)
	{
		WeaponGroupCount = 1;
	}

	int32 SuccesfulUpgrades = 0;
	for (int32 WeaponGroupIndex = 0; WeaponGroupIndex < WeaponGroupCount; WeaponGroupIndex++)
	{
		TArray< FFlareSpacecraftComponentDescription* > PartListData;
		FFlareSpacecraftDescription* ConstDescription = const_cast<FFlareSpacecraftDescription*>(Ship->GetDescription());
		FFlareSpacecraftSlotGroupDescription* SlotGroupDescription = &ConstDescription->WeaponGroups[WeaponGroupIndex];

		Catalog->GetWeaponList(PartListData, Ship->GetSize(), Ship->GetCompany(), Ship, SlotGroupDescription);
		FFlareSpacecraftComponentDescription* BestWeapon = NULL;

		for (FFlareSpacecraftComponentDescription* Part : PartListData)
		{
			if (!Ship->GetCompany()->IsTechnologyUnlockedPart(Part))
			{
				continue;
			}

			if (Ship->GetCompany()->IsPartRestricted(Part, Ship))
			{
				continue;
			}

			if (!Part->WeaponCharacteristics.IsWeapon)
			{
				continue;
			}

			if (AllowSalvager)
			{
				if (WeaponTargetSize == EFlarePartSize::L && Part->WeaponCharacteristics.DamageType != EFlareShellDamageType::HeavySalvage)
				{
					continue;
				}

				if (WeaponTargetSize == EFlarePartSize::S && Part->WeaponCharacteristics.DamageType != EFlareShellDamageType::LightSalvage)
				{
					continue;
				}

				bool HasChance = FMath::FRand() < 0.7;
				if (!BestWeapon || HasChance)
				{
					BestWeapon = Part;
				}
				continue;
			}

			else if (WeaponTargetSize == EFlarePartSize::L && Part->WeaponCharacteristics.AntiLargeShipValue < 0.5)
			{
				continue;
			}

			else if (WeaponTargetSize == EFlarePartSize::S && Part->WeaponCharacteristics.AntiSmallShipValue < 0.5)
			{
				continue;
			}

			if (!AllowSalvager)
			{
				// Compatible target
				bool HasChance = FMath::FRand() < 0.7;
				if (!BestWeapon || (BestWeapon->Cost < Part->Cost && HasChance))
				{
					BestWeapon = Part;
				}
			}
		}

		if (BestWeapon)
		{
			FFlareSpacecraftComponentDescription* OldWeapon = Ship->GetCurrentPart(EFlarePartType::Weapon, WeaponGroupIndex);
			int64 TransactionCost = Ship->GetUpgradeCost(BestWeapon, OldWeapon);
			if (((TransactionCost * CostSafetyMargin) + (TotalValue.TotalDailyProductionCost * Behavior->DailyProductionCostSensitivityMilitary)) > Ship->GetCompany()->GetMoney() && CanSpendBudget(EFlareBudget::Military, TransactionCost))
			{
				// Cannot afford
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("failed to upgrade %s : cannot afford upgrade",
					*Ship->GetImmatriculation().ToString());
#endif
				return false;
			}

			if (Ship->UpgradePart(BestWeapon, WeaponGroupIndex))
			{
				++SuccesfulUpgrades;
				Ship->GetCompany()->GetAI()->SpendBudget(EFlareBudget::Military, TransactionCost);
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("%s upgrade its weapons to %s", *Ship->GetImmatriculation().ToString(), *BestWeapon->Identifier.ToString());
#endif
			}
			else
			{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("failed to upgrade %s : upgrade failed for unknown reseon",
					*Ship->GetImmatriculation().ToString());
#endif
				return false;
			}
		}
	}
	return false;
}

bool UFlareCompanyAI::UpgradeShipEngine(UFlareSimulatedSpacecraft* Ship, EFlareBudget::Type Budget)
{
	// iterate to find best par
	UFlareSpacecraftComponentsCatalog* Catalog = Game->GetPC()->GetGame()->GetShipPartsCatalog();
	FFlareSpacecraftComponentDescription* OldPart = Ship->GetCurrentPart(EFlarePartType::OrbitalEngine, 0);
	FFlareSpacecraftComponentDescription* BestPart = OldPart;
	int32 BestPartWeightValue = 0;

	TArray< FFlareSpacecraftComponentDescription* > PartListData;
	int64 TransactionCost = 0;
	Catalog->GetEngineList(PartListData, Ship->GetSize(), Ship->GetCompany(), Ship);

	BestPart = FindBestEngineOrRCS(PartListData, BestPart, Budget,false);

	if (BestPart != OldPart)
	{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
		FLOGV("%s upgrade its orbital engines to %s", *Ship->GetImmatriculation().ToString(), *BestPart->Identifier.ToString());
#endif
		TransactionCost = Ship->GetUpgradeCost(BestPart, OldPart);
		if (CanSpendBudget(Budget, TransactionCost))
		{
			if (Ship->UpgradePart(BestPart, 0))
			{
				SpendBudget(Budget, TransactionCost);
				return true;
			}
		}
	}
	return false;
}

bool UFlareCompanyAI::UpgradeShipRCS(UFlareSimulatedSpacecraft* Ship, EFlareBudget::Type Budget)
{
	UFlareSpacecraftComponentsCatalog* Catalog = Game->GetPC()->GetGame()->GetShipPartsCatalog();
	// iterate to find best par
	FFlareSpacecraftComponentDescription* OldPart = Ship->GetCurrentPart(EFlarePartType::RCS, 0);
	FFlareSpacecraftComponentDescription* BestPart = OldPart;
	

	TArray< FFlareSpacecraftComponentDescription* > PartListData;
	int64 TransactionCost = 0;
	Catalog->GetRCSList(PartListData, Ship->GetSize(), Ship->GetCompany(), Ship);
	BestPart = FindBestEngineOrRCS(PartListData, BestPart, Budget, true);

	if (BestPart != OldPart)
	{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
		FLOGV("%s upgrade its rcs to %s", *Ship->GetImmatriculation().ToString(), *BestPart->Identifier.ToString());
#endif
		TransactionCost = Ship->GetUpgradeCost(BestPart, OldPart);
		if (CanSpendBudget(Budget, TransactionCost))
		{
			if (Ship->UpgradePart(BestPart, 0))
			{
				SpendBudget(Budget, TransactionCost);
				return true;
			}
		}
	}
	return false;
}

FFlareSpacecraftComponentDescription* UFlareCompanyAI::FindBestEngineOrRCS(TArray< FFlareSpacecraftComponentDescription*> PartListData, FFlareSpacecraftComponentDescription* BestPart, EFlareBudget::Type Budget, bool EngineOrRCS)
{
	int32 BestPartWeightValue = 0;
	if (BestPart)
	{
		BestPartWeightValue = GetBestPartWeightValue(BestPart,Budget,EngineOrRCS);
	}

	for (FFlareSpacecraftComponentDescription* Part : PartListData)
	{
		bool HasChance = FMath::RandBool();
		if (!HasChance && BestPart)
		{
			continue;
		}

		int32 WeightFinalValue = GetBestPartWeightValue(Part,Budget,EngineOrRCS);

		if (!BestPart || (BestPartWeightValue < WeightFinalValue))
		{
			BestPart = Part;
			BestPartWeightValue = WeightFinalValue;
		}
	}
	return BestPart;
}

int32 UFlareCompanyAI::GetBestPartWeightValue(FFlareSpacecraftComponentDescription* Part, EFlareBudget::Type Budget, bool EngineOrRCS)
{
	int32 CostWeight = 1;
	int32 HitPointsWeight = 1;
	int32 EnginePowerWeight = 1;
	int32 AngularAccelerationWeight = 1;

	if (EngineOrRCS)
	{
		//RCS weights
		EnginePowerWeight = 2;
		AngularAccelerationWeight = 10;
	}
	else
	{
		//Engine weights
		EnginePowerWeight = 10;
		AngularAccelerationWeight = 1;
	}

	if (Budget == EFlareBudget::Military)
	{
		HitPointsWeight = 10;
	}

	return (Part->Cost * CostWeight) + (Part->HitPoints * HitPointsWeight) + (Part->EngineCharacteristics.EnginePower * EnginePowerWeight) + (Part->EngineCharacteristics.AngularAccelerationRate * AngularAccelerationWeight);
}

bool UFlareCompanyAI::UpgradeMilitaryFleet(AIWarContext& WarContext,  WarTarget Target, DefenseSector& Sector, TArray<UFlareFleet*> MovableFleets)
{
	// First check if upgrade is possible in sector
	// If not, find the closest sector where upgrade is possible and travel here

	if (!CanUpgradeFromSector(Sector.Sector))
	{
		UFlareSimulatedSector* UpgradeSector = FindNearestSectorWithUpgradePossible(WarContext, Sector.Sector);
		if (UpgradeSector)
		{
			for (UFlareFleet* AssignedFleet : MovableFleets)
			{

				FLOGV("UpdateWarMilitaryMovement %s : move %s from %s to %s for upgrade",
				*Company->GetCompanyName().ToString(),
				*AssignedFleet->GetIdentifier().ToString(),
				*AssignedFleet->GetCurrentSector()->GetSectorName().ToString(),
				*UpgradeSector->GetSectorName().ToString());
				Game->GetGameWorld()->StartTravel(AssignedFleet, UpgradeSector);
			}
		}
		else
		{
			#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
				FLOGV("failed to find upgrade sector from %s",
				*Sector.Sector->GetSectorName().ToString());
			#endif
		}
		return false;
	}

	// If upgrade possible
	bool UpgradeFailed = false;
	CompanyValue TotalValue = Company->GetCompanyValue();

	for (UFlareFleet* AssignedFleet : MovableFleets)
	{
		//for each L ship
		for (UFlareSimulatedSpacecraft* Ship : AssignedFleet->GetShips())
		{
			if (Ship->GetSize() != EFlarePartSize::L)
			{
				// L only
				continue;
			}

			bool EnoughAntiL = Sector.ArmyAntiLCombatPoints >= Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold;
			bool EnoughAntiS = Sector.ArmyAntiSCombatPoints >= Target.EnemyArmySCombatPoints * WarContext.AttackThreshold;
			int32 CombatPoints = Ship->GetCombatPoints(true);
			bool HasAntiLargeShipWeapon = Ship->GetWeaponsSystem()->HasAntiLargeShipWeapon();
			bool HasAntiSmallShipWeapon = Ship->GetWeaponsSystem()->HasAntiSmallShipWeapon();

			//if no enought anti L, upgrade to anti L
			if (!EnoughAntiL && !HasAntiLargeShipWeapon)
			{
				bool Upgraded = Ship->GetCompany()->GetAI()->UpgradeShip(Ship, EFlarePartSize::L);
				if (Upgraded)
				{
					Sector.ArmyAntiLCombatPoints += CombatPoints;
					if (HasAntiSmallShipWeapon)
					{
						Sector.ArmyAntiSCombatPoints -= CombatPoints;
					}
				}
				else
				{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
					FLOGV("failed to upgrade %s to anti L",
						*Ship->GetImmatriculation().ToString());
#endif
					UpgradeFailed = true;
				}
		}
			// if enought anti L and L ship value is not nesessary, upgrade to Anti S
			else if (!EnoughAntiS && HasAntiLargeShipWeapon)
			{
				bool EnoughAntiLWithoutShip = (Sector.ArmyAntiLCombatPoints - CombatPoints) >= Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold;
				if (EnoughAntiLWithoutShip)
				{
					bool Upgraded = Ship->GetCompany()->GetAI()->UpgradeShip(Ship, EFlarePartSize::S);
					if (Upgraded)
					{
						Sector.ArmyAntiSCombatPoints += CombatPoints;
						Sector.ArmyAntiLCombatPoints -= CombatPoints;
					}
					else
					{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
						FLOGV("failed to upgrade %s to anti S",
							*Ship->GetImmatriculation().ToString());
#endif
						UpgradeFailed = true;
					}
				}
	}
			else if (TotalValue.TotalShipCountMilitaryL > 0)
			{
				//good on firepower, now consider capture capability
				float EquippedRatio = TotalValue.TotalShipCountMilitaryLSalvager / TotalValue.TotalShipCountMilitaryL;
				if (EquippedRatio < Behavior->UpgradeMilitarySalvagerLRatio)
				{
					bool Upgraded = Ship->GetCompany()->GetAI()->UpgradeShip(Ship, FMath::RandBool() ? EFlarePartSize::S : EFlarePartSize::L, true);
					if (Upgraded)
					{
					}
				}
			}
		}

		//for each S ship
		for (UFlareSimulatedSpacecraft* Ship : AssignedFleet->GetShips())
		{
			if (Ship->GetSize() != EFlarePartSize::S)
			{
				// S only
				continue;
			}

			bool EnoughAntiL = Sector.ArmyAntiLCombatPoints >= Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold;
			bool EnoughAntiS = Sector.ArmyAntiSCombatPoints >= Target.EnemyArmySCombatPoints * WarContext.AttackThreshold;
			int32 CombatPoints = Ship->GetCombatPoints(true);
			bool HasAntiLargeShipWeapon = Ship->GetWeaponsSystem()->HasAntiLargeShipWeapon();
			bool HasAntiSmallShipWeapon = Ship->GetWeaponsSystem()->HasAntiSmallShipWeapon();

			//if no enought anti S, upgrade to anti S
			if (!EnoughAntiS && !HasAntiSmallShipWeapon)
			{
				bool Upgraded = Ship->GetCompany()->GetAI()->UpgradeShip(Ship, EFlarePartSize::S);
				if (Upgraded)
				{
					Sector.ArmyAntiSCombatPoints += CombatPoints;
					if (HasAntiLargeShipWeapon)
					{
						Sector.ArmyAntiLCombatPoints -= CombatPoints;
					}
				}
				else
				{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
					FLOGV("failed to upgrade %s to anti S",
						*Ship->GetImmatriculation().ToString());
#endif
					UpgradeFailed = true;
				}
			}
			// if enought anti S and S ship value is not nesessary, upgrade to Anti L
			else if (!EnoughAntiL && HasAntiSmallShipWeapon)
			{
				bool EnoughAntiSWithoutShip = (Sector.ArmyAntiSCombatPoints - CombatPoints) >= Target.EnemyArmySCombatPoints * WarContext.AttackThreshold;
				if (EnoughAntiSWithoutShip)
				{
					bool Upgraded = Ship->GetCompany()->GetAI()->UpgradeShip(Ship, EFlarePartSize::L);
					if (Upgraded)
					{
						Sector.ArmyAntiLCombatPoints += CombatPoints;
						Sector.ArmyAntiSCombatPoints -= CombatPoints;
					}
					else
					{
#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
						FLOGV("failed to upgrade %s to anti L",
							*Ship->GetImmatriculation().ToString());
#endif
						UpgradeFailed = true;
					}
				}
			}
			else if (TotalValue.TotalShipCountMilitaryS > 0)
			{
				//good on firepower, now consider capture capability
				float EquippedRatio = TotalValue.TotalShipCountMilitarySSalvager / TotalValue.TotalShipCountMilitaryS;
				if (EquippedRatio < Behavior->UpgradeMilitarySalvagerSRatio)
				{
					bool Upgraded = Ship->GetCompany()->GetAI()->UpgradeShip(Ship, FMath::RandBool() ? EFlarePartSize::S : EFlarePartSize::L, true);
					if (Upgraded)
					{
					}
				}
			}
		}
	}

	// if enough anti S and L, upgrade to min ratio
	if ((Sector.ArmyAntiLCombatPoints >= Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold) &&
		(Sector.ArmyAntiSCombatPoints >= Target.EnemyArmySCombatPoints * WarContext.AttackThreshold))
	{
		for (UFlareFleet* AssignedFleet : MovableFleets)
		{
			for (UFlareSimulatedSpacecraft* Ship : AssignedFleet->GetShips())
			{
				float AntiLRatio = (Target.EnemyArmyLCombatPoints > 0 ?
					Sector.ArmyAntiLCombatPoints / Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold :
					1000);
				float AntiSRatio = (Target.EnemyArmySCombatPoints > 0 ?
					Sector.ArmyAntiSCombatPoints / Target.EnemyArmySCombatPoints * WarContext.AttackThreshold :
					1000);

				if (AntiLRatio == AntiSRatio)
				{
					// Any change will change the balance
					break;
				}

				if (AntiLRatio > AntiSRatio)
				{
					if (Ship->GetWeaponsSystem()->HasAntiLargeShipWeapon())
					{
						int32 CombatPoints = Ship->GetCombatPoints(true);
						// See if upgrade to S improve this
						float FutureAntiLRatio = (Target.EnemyArmyLCombatPoints > 0 ?
							(Sector.ArmyAntiLCombatPoints - CombatPoints) / Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold :
							1000);
						float FutureAntiSRatio = (Target.EnemyArmySCombatPoints > 0 ?
							(Sector.ArmyAntiSCombatPoints + CombatPoints) / Target.EnemyArmySCombatPoints * WarContext.AttackThreshold :
							1000);
						if (FutureAntiLRatio > FutureAntiSRatio)
						{
							// Ratio of ratio still unchanged, upgrade to S
							bool Upgraded = Ship->GetCompany()->GetAI()->UpgradeShip(Ship, EFlarePartSize::S);
							if (Upgraded)
							{
								Sector.ArmyAntiSCombatPoints += CombatPoints;
								Sector.ArmyAntiLCombatPoints -= CombatPoints;
							}
							else
							{
								UpgradeFailed = true;
							}
						}
					}
				}
				else
				{
					if (Ship->GetWeaponsSystem()->HasAntiSmallShipWeapon())
					{
						int32 CombatPoints = Ship->GetCombatPoints(true);
						// See if upgrade to S improve this
						float FutureAntiLRatio = (Target.EnemyArmyLCombatPoints > 0 ?
							(Sector.ArmyAntiLCombatPoints + CombatPoints) / Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold :
							1000);
						float FutureAntiSRatio = (Target.EnemyArmySCombatPoints > 0 ?
							(Sector.ArmyAntiSCombatPoints - CombatPoints) / Target.EnemyArmySCombatPoints * WarContext.AttackThreshold :
							1000);
						if (FutureAntiSRatio > FutureAntiLRatio)
						{
							// Ratio of ratio still unchanged, upgrade to L
							bool Upgraded = Ship->GetCompany()->GetAI()->UpgradeShip(Ship, EFlarePartSize::L);
							if (Upgraded)
							{
								Sector.ArmyAntiLCombatPoints += CombatPoints;
								Sector.ArmyAntiSCombatPoints -= CombatPoints;
							}
							else
							{
								UpgradeFailed = true;
							}
						}
					}
				}
			}
		}
	}

	bool FinalEnoughAntiL = Sector.ArmyAntiLCombatPoints >= Target.EnemyArmyLCombatPoints * WarContext.AttackThreshold;
	bool FinalEnoughAntiS = Sector.ArmyAntiSCombatPoints >= Target.EnemyArmySCombatPoints * WarContext.AttackThreshold;

	#ifdef DEBUG_AI_WAR_MILITARY_MOVEMENT
					FLOGV("upgrade at %s failed FinalEnoughAntiL=%d, FinalEnoughAntiS=%d",
						*Sector.Sector->GetSectorName().ToString(),
						  FinalEnoughAntiL, FinalEnoughAntiS);
					FLOGV("ArmyAntiLCombatPoints=%d EnemyArmyLCombatPoints=%d ArmyAntiSCombatPoints=%d EnemyArmySCombatPoints=%d",
						Sector.ArmyAntiLCombatPoints, Target.EnemyArmyLCombatPoints,
						Sector.ArmyAntiSCombatPoints, Target.EnemyArmySCombatPoints);
	#endif

	return !UpgradeFailed || (FinalEnoughAntiL && FinalEnoughAntiS);
}

//#define DEBUG_AI_PEACE_MILITARY_MOVEMENT

void UFlareCompanyAI::UpdateNeedyCarrierMovement(UFlareSimulatedSpacecraft* Ship, TArray<FFlareResourceDescription*> InputResources)
{
	if (WasAtWar || !Ship->GetDamageSystem()->IsAlive() || Ship->IsTrading() || (Ship->GetCurrentFleet() && Ship->GetCurrentFleet()->IsTraveling()) || Ship->GetShipChildren().Num() >= Ship->GetDescription()->DroneMaximum || !Ship->CanTravel())
	{
		return;
	}

//Only runs during peacetime, carriers split off solo to find resources individually
	if (!Ship->GetDescription()->IsDroneShip && Ship->GetCurrentFleet()->GetShipCount() > 1)
	{
		Company->CreateAutomaticFleet(Ship);
	}

	TArray<UFlareSimulatedSector*> KnownSectorsLocal = Company->GetKnownSectors();
	while (KnownSectorsLocal.Num() > 0)
	{
		int32 Index = FMath::RandRange(0, KnownSectorsLocal.Num() - 1);
		UFlareSimulatedSector* Sector = KnownSectorsLocal[Index];

		bool FoundResources = false;
		FText Unused;

		for (int32 CargoIndex = 0; CargoIndex < InputResources.Num(); CargoIndex++)
		{
			FFlareResourceDescription* Resource = InputResources[CargoIndex];

			for (UFlareSimulatedSpacecraft* BuyingStation : Sector->GetSectorStations())
			{
				if (BuyingStation->IsUnderConstruction() || BuyingStation->IsHostile(Company))
				{
					continue;
				}

				if (!BuyingStation->CanTradeWhiteListTo(Ship, Unused, Resource) || !Ship->CanTradeWhiteListFrom(BuyingStation, Unused, Resource))
				{
					continue;
				}

				if (BuyingStation->GetActiveCargoBay()->WantSell(Resource, Company, true) && BuyingStation->GetActiveCargoBay()->GetResourceQuantity(Resource,Company) > 0 && Ship->GetActiveCargoBay()->GetFreeSpaceForResource(Resource, Company) > 0)
				{
					FoundResources = true;
					break;
				}
			}
		}

		if (FoundResources)
		{
			Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), Sector);
			break;
		}

		KnownSectorsLocal.RemoveSwap(Sector);
	}
}

void UFlareCompanyAI::UpdatePeaceMilitaryMovement()
{
	CompanyValue TotalValue = Company->GetCompanyValue();
	int64 TotalLicenseValue = Company->GetTotalStationLicenseValue() / 2;
	int64 TotalDefendableValue = TotalValue.StationsValue + TotalValue.StockValue + TotalValue.ShipsValue + TotalLicenseValue - TotalValue.ArmyValue;
	float TotalDefenseRatio = (float) TotalValue.ArmyValue / (float) TotalDefendableValue;

#ifdef DEBUG_AI_PEACE_MILITARY_MOVEMENT
	FLOGV("UpdatePeaceMilitaryMovement for %s : TotalDefendableValue %lld, TotalDefenseRatio %f",
		*Company->GetCompanyName().ToString(),
		TotalDefendableValue,
		TotalDefenseRatio);
#endif
	
	TArray<UFlareSimulatedSpacecraft*> ShipsToMove;
	TArray<UFlareSimulatedSector*> LowDefenseSectors;

	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];
		CompanyValue SectorValue = Company->GetCompanyValue(Sector, true);
		int64 SectorLicenseValue = SectorValue.StationsValue + SectorValue.StockValue + SectorValue.ShipsValue + (Company->GetStationLicenseCost(Sector) / 2) - SectorValue.ArmyValue;

		int64 SectorDefendableValue = SectorValue.StationsValue + SectorValue.StockValue + SectorValue.ShipsValue - SectorValue.ArmyValue;
		int64 SectorArmyValue = SectorValue.ArmyValue;
		float SectorDefenseRatio = (float) SectorArmyValue / (float) SectorDefendableValue;

		if ((SectorDefendableValue == 0 && SectorArmyValue > 0) || SectorDefenseRatio > TotalDefenseRatio * 1.5)
		{

#ifdef DEBUG_AI_PEACE_MILITARY_MOVEMENT
			FLOGV("UpdatePeaceMilitaryMovement %s SectorDefendableValue %lld, SectorDefenseRatio %f",
				*Sector->GetSectorName().ToString(),
				SectorDefendableValue,
				SectorDefenseRatio);
#endif

			// Too much defense here, move a ship, pick a random ship and add to the ship to move list
			TArray<UFlareSimulatedSpacecraft*> ShipCandidates;
			TArray<UFlareSimulatedSpacecraft*>& SectorShips = Sector->GetSectorShips();
			for (UFlareSimulatedSpacecraft* ShipCandidate : SectorShips)
			{
				if (ShipCandidate->GetCompany() != Company || ShipCandidate->GetDescription()->IsDroneShip)
				{
					continue;
				}

				if (!ShipCandidate->IsMilitary()  || !ShipCandidate->CanTravel() || ShipCandidate->GetDamageSystem()->IsDisarmed())
				{
					continue;
				}

				if (ShipCandidate->GetActiveCargoBay()->GetCapacity() > 0)
				{
					if (!ShipCandidate->GetDescription()->IsDroneCarrier)
					{
						continue;
					}
					else if(ShipCandidate->GetShipChildren().Num() < ShipCandidate->GetDescription()->DroneMaximum)
					{
						continue;
					}
				}

				ShipCandidates.Add(ShipCandidate);
			}

			if (ShipCandidates.Num() > 1 || (SectorDefendableValue == 0 && ShipCandidates.Num() > 0))
			{
				UFlareSimulatedSpacecraft* SelectedShip = ShipCandidates[FMath::RandRange(0, ShipCandidates.Num()-1)];
				ShipsToMove.Add(SelectedShip);
				if (SelectedShip->GetCurrentFleet()->GetShipCount() > 1)
				{
					Company->CreateAutomaticFleet(SelectedShip);
				}

				#ifdef DEBUG_AI_PEACE_MILITARY_MOVEMENT
							FLOGV("UpdatePeaceMilitaryMovement - %s has high defense: pick %s", *Sector->GetSectorName().ToString(), *SelectedShip->GetImmatriculation().ToString());
				#endif
			}
			else
			{
#ifdef DEBUG_AI_PEACE_MILITARY_MOVEMENT
				FLOGV("UpdatePeaceMilitaryMovement - %s has high defense: no available ships", *Sector->GetSectorName().ToString());
#endif
			}
		}

		// Too few defense, add to the target sector list
		else if (SectorDefendableValue > 0 && SectorDefenseRatio < TotalDefenseRatio )
		{
#ifdef DEBUG_AI_PEACE_MILITARY_MOVEMENT
			FLOGV("UpdatePeaceMilitaryMovement - %s has low defense", *Sector->GetSectorName().ToString());
#endif
			LowDefenseSectors.Add(Sector);
		}
	}

	// Find destination sector
	for (UFlareSimulatedSpacecraft* Ship: ShipsToMove)
	{
		int64 MinDurationTravel = 0;
		UFlareSimulatedSector* BestSectorCandidate = NULL;

		for (UFlareSimulatedSector* SectorCandidate : LowDefenseSectors)
		{
			int64 TravelDuration = UFlareTravel::ComputeTravelDuration(Game->GetGameWorld(), Ship->GetCurrentSector(), SectorCandidate, Company, Ship->GetCurrentFleet());
			if (BestSectorCandidate == NULL || MinDurationTravel > TravelDuration)
			{
				MinDurationTravel = TravelDuration;
				BestSectorCandidate = SectorCandidate;
			}
		}

		// Low defense sector in reach
		if (BestSectorCandidate)
		{
			FLOGV("UpdatePeaceMilitaryMovement > move %s from %s to %s",
				*Ship->GetImmatriculation().ToString(),
				*Ship->GetCurrentSector()->GetSectorName().ToString(),
				*BestSectorCandidate->GetSectorName().ToString());

			Game->GetGameWorld()->StartTravel(Ship->GetCurrentFleet(), BestSectorCandidate);
		}
	}
}


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

int64 UFlareCompanyAI::OrderOneShip(const FFlareSpacecraftDescription* ShipDescription)
{
	if (ShipDescription == NULL)
	{
		return 0;
	}

	int64 CompanyMoney = Company->GetMoney();
	CompanyValue TotalValue = Company->GetCompanyValue();

	float CostSafetyMargin;
	float ProductionSensitivity;
	bool IsMilitary = false;

	if (ShipDescription->IsMilitary())
	{
		CostSafetyMargin = Behavior->CostSafetyMarginMilitaryShip;
		ProductionSensitivity = Behavior->DailyProductionCostSensitivityMilitary;
		IsMilitary = true;
	}
	else
	{
		CostSafetyMargin = Behavior->CostSafetyMarginTradeShip;
		ProductionSensitivity = Behavior->DailyProductionCostSensitivityEconomic;
}

	// FLOGV("OrderOneShip %s", *ShipDescription->Name.ToString());

	for (int32 ShipyardIndex = 0; ShipyardIndex < Shipyards.Num(); ShipyardIndex++)
	{
		UFlareSimulatedSpacecraft* Shipyard = Shipyards[ShipyardIndex];
/*
		FLOGV("UFlareCompanyAI::UpdateShipAcquisition : CanOrder, looking at: '%s' for %s",
			*Shipyard->GetImmatriculation().ToString(),
			*ShipDescription->Identifier.ToString());
*/
		if (Shipyard->CanOrder(ShipDescription, Company, true))
		{
			int64 ShipPrice;

			if (Shipyard->GetCompany() != Company)
			{
				ShipPrice = UFlareGameTools::ComputeSpacecraftPrice(ShipDescription->Identifier, Shipyard->GetCurrentSector(), true);
			}
			else
			{
				ShipPrice = ShipDescription->CycleCost.ProductionCost;
			}
/*
			FLOGV("UFlareCompanyAI::UpdateShipAcquisition : CanOrder, looking at: '%s', price %d", 
				*Shipyard->GetImmatriculation().ToString(),
				ShipPrice);
*/
			if (((ShipPrice * CostSafetyMargin) + (TotalValue.TotalDailyProductionCost * ProductionSensitivity) < CompanyMoney) && CanSpendBudget(IsMilitary ? EFlareBudget::Military : EFlareBudget::Trade, ShipPrice))

			{
				FName ShipClassToOrder = ShipDescription->Identifier;
				FLOGV("UFlareCompanyAI::UpdateShipAcquisition : Ordering spacecraft : '%s' from '%s'", 
					*ShipClassToOrder.ToString()
					,*Shipyard->GetImmatriculation().ToString());
				Shipyard->ShipyardOrderShip(Company, ShipClassToOrder);

				SpendBudget((IsMilitary ? EFlareBudget::Military : EFlareBudget::Trade), ShipPrice);

				if (IsMilitary)
				{
					++ BuildingMilitaryShips;
				}
				else
				{
					++ BuildingTradeShips;
				}
				return 0;
			}
			else
			{
				return ShipPrice;
			}
		}
	}

	return 0;
}

const FFlareSpacecraftDescription* UFlareCompanyAI::FindBestShipToBuild(bool Military)
{
	int32 TotalCompanyShipCount = Company->GetCompanyShips().Num();
	if(TotalCompanyShipCount > AI_MAX_SHIP_COUNT)
	{
		return nullptr;
	}

	//DEBUG
	int32 ShipSCount = 0;
	int32 ShipLCount = 0;

	// Count owned ships
	TMap<const FFlareSpacecraftDescription*, int32> OwnedShipSCount;
	TMap<const FFlareSpacecraftDescription*, int32> OwnedShipLCount;

	for (int32 ShipIndex = 0; ShipIndex < Company->GetCompanyShips().Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = Company->GetCompanyShips()[ShipIndex];
		if (Military != Ship->IsMilitary())
		{
			continue;
		}

		if (Ship->GetSize() ==EFlarePartSize::S)
		{
			ShipSCount++;
			if (OwnedShipSCount.Contains(Ship->GetDescription()))
			{
				OwnedShipSCount[Ship->GetDescription()]++;
			}
			else
			{
				OwnedShipSCount.Add(Ship->GetDescription(), 1);
			}
		}
		else if (Ship->GetSize() ==EFlarePartSize::L)
		{
			ShipLCount++;
			if (OwnedShipLCount.Contains(Ship->GetDescription()))
			{
				OwnedShipLCount[Ship->GetDescription()]++;
			}
			else
			{
				OwnedShipLCount.Add(Ship->GetDescription(), 1);
			}
		}
	}

	// Count ships in production
	for (int32 ShipyardIndex = 0; ShipyardIndex < Shipyards.Num(); ShipyardIndex++)
	{
		UFlareSimulatedSpacecraft* Shipyard =Shipyards[ShipyardIndex];
		TArray<UFlareFactory*>& Factories = Shipyard->GetFactories();

		for (int32 Index = 0; Index < Factories.Num(); Index++)
		{
			UFlareFactory* Factory = Factories[Index];
			if (Factory->GetTargetShipCompany() == Company->GetIdentifier())
			{
				const FFlareSpacecraftDescription* BuildingShip = Game->GetSpacecraftCatalog()->Get(Factory->GetTargetShipClass());
				if (Military != BuildingShip->IsMilitary())
				{
					continue;
				}

				if (BuildingShip->Size ==EFlarePartSize::S)
				{
					ShipSCount++;
					if (OwnedShipSCount.Contains(BuildingShip))
					{
						OwnedShipSCount[BuildingShip]++;
					}
					else
					{
						OwnedShipSCount.Add(BuildingShip, 1);
					}
				}
				else if (BuildingShip->Size ==EFlarePartSize::L)
				{
					ShipLCount++;
					if (OwnedShipLCount.Contains(BuildingShip))
					{
						OwnedShipLCount[BuildingShip]++;
					}
					else
					{
						OwnedShipLCount.Add(BuildingShip, 1);
					}
				}

			}
		}
	}

	// Choose which size to pick
	int32 Diversity = (Military ? Behavior->BuildMilitaryDiversity : Behavior->BuildTradeDiversity);
	int32 DiversityEfficiency = (Military ? Behavior->BuildMilitaryDiversity + 1 : Behavior->BuildTradeDiversity + 1);
	float EfficiencyChance = (Military ? Behavior->BuildEfficientMilitaryChance : Behavior->BuildEfficientTradeChance);

	int32 SizeDiversity = (Military ? Behavior->BuildMilitaryDiversitySize : Behavior->BuildTradeDiversitySize);
	int32 SizeDiversityBase = (Military ? Behavior->BuildMilitaryDiversitySizeBase : Behavior->BuildTradeDiversitySizeBase);
	int32 LSwitchThreshold = (Military ? Behavior->BuildLMilitaryOnlyTreshhold : Behavior->BuildLTradeOnlyTreshhold);
	bool PickLShip = true;

	if (TotalCompanyShipCount < LSwitchThreshold && SizeDiversityBase + (ShipLCount * SizeDiversity) > ShipSCount)
	{
		PickLShip = false;
		EfficiencyChance = (Military ? Behavior->BuildEfficientMilitaryChanceSmall : Behavior->BuildEfficientTradeChanceSmall);
	}

#ifdef DEBUG_AI_SHIP_ORDER
			FLOGV("FindBestShipToBuild for %s (military=%d) pick L=%d",
				  *Company->GetCompanyName().ToString(),
				  Military,
				  PickLShip);
			FLOGV("- ShipSCount %d", ShipSCount);
			FLOGV("- ShipLCount %d", ShipLCount);
			FLOGV("- SizeDiversity %d", SizeDiversity);
			FLOGV("- Diversity %d", Diversity);
			FLOGV("- Initial EfficiencyChance %f", EfficiencyChance);
#endif

	// List possible ship candidates
	TArray<const FFlareSpacecraftDescription*> CandidateShips;
	for (auto& CatalogEntry : Game->GetSpacecraftCatalog()->ShipCatalog)
	{
		const FFlareSpacecraftDescription* Description = &CatalogEntry->Data;
		if (Description->IsDroneShip)
		{
			continue;
		}
		if (Description->BuyableCompany.Num() > 0)
		{
			//buyable company has something, check if we're allowed to buy this in the first place
			if (!Description->BuyableCompany.Contains(Company->GetDescription()->ShortName))
			{
				continue;
			}
		}

		if (Military == Description->IsMilitary())
		{
			if ((PickLShip && Description->Size == EFlarePartSize::L) || (!PickLShip && Description->Size == EFlarePartSize::S))
			{
				CandidateShips.Add(Description);
			}
		}
		//not specifically looking for military ships, but a military ship might have cargo capacity
		else if (!Military && !Description->IsDroneCarrier && Description->CargoBayCapacity > 0 && Description->CargoBayCount > 0)
		{
			if ((PickLShip && Description->Size == EFlarePartSize::L) || (!PickLShip && Description->Size == EFlarePartSize::S))
			{
				CandidateShips.Add(Description);
			}
		}
	}
	
	// Sort by size
	struct FSortBySmallerShip
	{
		FSortBySmallerShip(float NewBuildDroneCombatWorth)
			: BuildDroneCombatWorth(NewBuildDroneCombatWorth)
		{
		}

		float BuildDroneCombatWorth;
		FORCEINLINE bool operator()(const FFlareSpacecraftDescription& A, const FFlareSpacecraftDescription& B) const
		{			
			if (A.Mass > B.Mass)
			{
				return false;
			}
			else if (A.Mass < B.Mass)
			{
				return true;
			}
			else if (A.IsMilitary())
			{
				if (!B.IsMilitary())
				{
					return false;
				}
				else
				{
					return A.CombatPoints + (A.DroneMaximum * BuildDroneCombatWorth) < B.CombatPoints + (B.DroneMaximum * BuildDroneCombatWorth);
				}
			}
			else
			{
				return true;
			}
		}
	};

	CandidateShips.Sort(FSortBySmallerShip(Behavior->BuildDroneCombatWorth));

	int32 SectorIndex = Company->GetKnownSectors().Num() - 1;
	SectorIndex = FMath::RandRange(0, SectorIndex);
	UFlareSimulatedSector* RandomSector = Company->GetKnownSectors()[SectorIndex];

	// Find the first ship that is diverse enough, from small to large

	const FFlareSpacecraftDescription* BestShipDescription = NULL;
	const FFlareSpacecraftDescription* BestShipDescriptionEfficiency = NULL;
	TMap<const FFlareSpacecraftDescription*, int32>* OwnedShipCount = (PickLShip ? &OwnedShipLCount : &OwnedShipSCount);

#ifdef DEBUG_AI_SHIP_ORDER
	FLOG("Beginning CanditateShipLoop");
#endif

	for (const FFlareSpacecraftDescription* Description : CandidateShips)
	{

#ifdef DEBUG_AI_SHIP_ORDER
		FLOGV("Looking at %s", *Description->Name.ToString());
#endif

		if (BestShipDescription == NULL)
		{
			BestShipDescription = Description;
			continue;
		}

		int32 CandidateCount = 0;
		if (OwnedShipCount->Contains(Description))
		{
			CandidateCount = (*OwnedShipCount)[Description];
		}

		int32 BestCount = 0;
		if (OwnedShipCount->Contains(BestShipDescription))
		{
			BestCount = (*OwnedShipCount)[BestShipDescription];
		}

		if (FMath::FRand() <= EfficiencyChance)
		{
			int64 ShipPriceA = UFlareGameTools::ComputeSpacecraftPrice(Description->Identifier, RandomSector, true);
			int64 ShipPriceB = UFlareGameTools::ComputeSpacecraftPrice(BestShipDescription->Identifier, RandomSector, true);
			float CostRatio = ShipPriceA / ShipPriceB;
			bool BestEfficiency;

#ifdef DEBUG_AI_SHIP_ORDER
			FLOGV("Passed Efficiency chance. CostRatio=%f", CostRatio);
#endif

			if (Military)
			{
				float Ratio1 = (Description->CombatPoints);
				float Ratio2 = (Description->DroneMaximum * Behavior->BuildDroneCombatWorth);

				if (BestShipDescription->CombatPoints)
				{
					Ratio1 /= (BestShipDescription->CombatPoints);
				}

				if (BestShipDescription->DroneMaximum > 0 && Behavior->BuildDroneCombatWorth > 0)
				{
					Ratio2 /= (BestShipDescription->DroneMaximum * Behavior->BuildDroneCombatWorth);
				}

				if (Ratio1 && Ratio2)
				{
					BestEfficiency = (Ratio1+Ratio2) > CostRatio;
				}
				else
				{
					BestEfficiency = Ratio1 > CostRatio;
				}

#ifdef DEBUG_AI_SHIP_ORDER
				FLOGV("- EfficiencyRatio 1 = %f", Ratio1);
				FLOGV("- EfficiencyRatio 2 = %f", Ratio2);
				FLOGV("- BestEfficiency %d", BestEfficiency);
#endif
			}
			else
			{
				float Ratio1 = Description->GetCapacity();
				if (BestShipDescription->GetCapacity() > 0)
				{
					Ratio1 /= BestShipDescription->GetCapacity();
				}

				BestEfficiency = Ratio1 > CostRatio;
			}

			if (BestEfficiency)
			{
				BestShipDescriptionEfficiency = Description;
			}
		}
		EfficiencyChance = EfficiencyChance / 2;

		bool BestBigger = BestShipDescription->Mass > Description->Mass;
		if (BestBigger && (BestCount + Diversity) > CandidateCount)
		{
			BestShipDescription = Description;
		}

		else if (!BestBigger && (CandidateCount + Diversity) < BestCount)
		{
			BestShipDescription = Description;
		}
	}

	if (BestShipDescription == NULL)
	{
		FLOG("ERROR: no ship to build");
		return NULL;
	}

	if (BestShipDescriptionEfficiency != NULL)
	{
		int32 BestCount = 0;
		if (OwnedShipCount->Contains(BestShipDescription))
		{
			BestCount = (*OwnedShipCount)[BestShipDescription];
		}

		int32 BestEfficiencyCount = 0;
		if (OwnedShipCount->Contains(BestShipDescriptionEfficiency))
		{
			BestEfficiencyCount = (*OwnedShipCount)[BestShipDescriptionEfficiency];
		}

		if (BestEfficiencyCount < BestCount || (BestEfficiencyCount == 0))
		{
#ifdef DEBUG_AI_SHIP_ORDER
			FLOGV("- Chose to build efficient ship, %s", *BestShipDescriptionEfficiency->Name.ToString());
#endif

			return BestShipDescriptionEfficiency;
		}
		else
		{
#ifdef DEBUG_AI_SHIP_ORDER
			FLOGV("- Chose to build standard diversity ship, %s", *BestShipDescription->Name.ToString());
#endif
			return BestShipDescription;
		}
		return (BestCount + Diversity) > (BestEfficiencyCount + DiversityEfficiency) ? BestShipDescription : BestShipDescriptionEfficiency;
	}
	else
	{
#ifdef DEBUG_AI_SHIP_ORDER
		FLOGV("- Chose to build standard diversity ship with no efficiency comparisons, %s", *BestShipDescription->Name.ToString());
#endif
		return BestShipDescription;
	}
}


void UFlareCompanyAI::GetCompany()
{
}

void UFlareCompanyAI::GetBuildingShipNumber()
{
	if (CheckedBuildingShips)
	{
		return;
	}

	BuildingMilitaryShips = 0;
	BuildingTradeShips = 0;
	for (int32 ShipyardIndex = 0; ShipyardIndex < Shipyards.Num(); ShipyardIndex++)
	{
		UFlareSimulatedSpacecraft* Shipyard = Shipyards[ShipyardIndex];
		TArray<UFlareFactory*>& Factories = Shipyard->GetFactories();

		for (int32 Index = 0; Index < Factories.Num(); Index++)
		{
			UFlareFactory* Factory = Factories[Index];
			if (Factory->GetTargetShipCompany() == Company->GetIdentifier())
			{
				FFlareSpacecraftDescription* BuildingShip = Game->GetSpacecraftCatalog()->Get(Factory->GetTargetShipClass());
				if (BuildingShip->IsMilitary())
				{
					++BuildingMilitaryShips;
				}
				else
				{
					++BuildingTradeShips;
				}
			}
		}
	}
	CheckedBuildingShips = true;
}

TArray<UFlareSimulatedSpacecraft*> UFlareCompanyAI::FindIncapacitatedCargos() const
{
	TArray<UFlareSimulatedSpacecraft*> IncapacitatedCargos;

	for (UFlareSimulatedSpacecraft* Ship : Company->GetCompanyShips())
	{
		if (Ship->GetActiveCargoBay()->GetCapacity() > 0 && (Ship->GetDamageSystem()->IsStranded() || Ship->GetDamageSystem()->IsUncontrollable()))
		{
			IncapacitatedCargos.Add(Ship);
		}
	}

	return IncapacitatedCargos;
}

void UFlareCompanyAI::CargosEvasion()
{
	SCOPE_CYCLE_COUNTER(STAT_FlareCompanyAI_CargosEvasion);

	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];

		if (!Sector->GetSectorBattleState(Company).HasDanger)
		{
			continue;
		}

		// Use intermediate list as travel modify the sector list
		TArray<UFlareFleet*> CargoFleetsToTravel;
		for (int32 ShipIndex = 0; ShipIndex < Sector->GetSectorFleets().Num(); ShipIndex++)
		{
			UFlareFleet* Fleet = Sector->GetSectorFleets()[ShipIndex];
			if (Fleet->GetFleetCompany() != Company || !Fleet->CanTravel() || Fleet->GetMilitaryShipCount() > 0)
			{
				continue;
			}

			CargoFleetsToTravel.Add(Fleet);
		}

		if (CargoFleetsToTravel.Num() > 0)
		{
			for (UFlareFleet* Fleet : CargoFleetsToTravel)
			{
				// Find nearest safe sector
				// Or if no safe sector, go to the farest sector to maximise travel time
				UFlareSimulatedSector* SafeSector = NULL;
				UFlareSimulatedSector* DistantUnsafeSector = NULL;
				int64 MinDurationTravel = 0;
				int64 MaxDurationTravel = 0;

				for (int32 SectorIndex2 = 0; SectorIndex2 < Company->GetKnownSectors().Num(); SectorIndex2++)
				{
					UFlareSimulatedSector* SectorCandidate = Company->GetKnownSectors()[SectorIndex2];
					int64 TravelDuration = UFlareTravel::ComputeTravelDuration(Game->GetGameWorld(), Sector, SectorCandidate, Company,Fleet);

					if (DistantUnsafeSector == NULL || MaxDurationTravel < TravelDuration)
					{
						MaxDurationTravel = TravelDuration;
						DistantUnsafeSector = SectorCandidate;
					}

					if (SectorCandidate->GetSectorBattleState(Company).HasDanger)
					{
						// Dont go in a dangerous sector
						continue;
					}

					if (SafeSector == NULL || MinDurationTravel > TravelDuration)
					{
						MinDurationTravel = TravelDuration;
						SafeSector = SectorCandidate;
					}
				}

				if (SafeSector)
				{
					Game->GetGameWorld()->StartTravel(Fleet, SafeSector);
				}
				else if (DistantUnsafeSector)
				{
					Game->GetGameWorld()->StartTravel(Fleet, DistantUnsafeSector);
				}
			}
		}
	}
}

void UFlareCompanyAI::UpgradeCargoShips()
{
	for (int32 ShipIndex = 0; ShipIndex < Company->GetCompanyShips().Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = Company->GetCompanyShips()[ShipIndex];
		if ((Ship->GetCurrentFleet() && Ship->GetCurrentFleet()->IsTraveling()) || Ship->GetActiveCargoBay()->GetCapacity() == 0)
		{
			continue;
		}

		if (!Ship->IsMilitary())
		{
			UFlareSimulatedSector* CurrentSector = Ship->GetCurrentSector();
			if (CanUpgradeFromSector(CurrentSector))
			{
				bool UpgradedEngine = false;
				bool UpgradedRCS = false;

				bool HasChance = FMath::FRand() < AI_TRADESHIP_UPGRADE_ENGINE_CHANCE;
				if (HasChance)
				{
					UpgradedEngine = UpgradeShipEngine(Ship, EFlareBudget::Trade);
				}

				HasChance = FMath::FRand() < AI_TRADESHIP_UPGRADE_RCS_CHANCE;
				if (HasChance)
				{
					UpgradedRCS = UpgradeShipRCS(Ship, EFlareBudget::Trade);
				}

				if (UpgradedEngine || UpgradedRCS)
				{
					break;
				}
			}
		}
	}
}

bool UFlareCompanyAI::CanUpgradeFromSector(UFlareSimulatedSector* Sector)
{
	bool FoundKey = CanUpgradeSectors.Contains(Sector);
	bool KeyValue = false;
	if (FoundKey)
	{
		KeyValue = CanUpgradeSectors[Sector];
	}
	if ((FoundKey && KeyValue) || (!FoundKey && Sector->CanUpgrade(Company)))
	{
		if (!FoundKey)
		{
			CanUpgradeSectors.Add(Sector, true);
		}
		else
		{
			CanUpgradeSectors[Sector] = true;
		}

		return true;
	}
	else if (!FoundKey)
	{
		CanUpgradeSectors.Add(Sector, false);
	}

	return false;
}

int32 UFlareCompanyAI::GetDamagedCargosCapacity()
{
	int32 DamagedCapacity = 0;
	for (int32 ShipIndex = 0; ShipIndex < Company->GetCompanyShips().Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = Company->GetCompanyShips()[ShipIndex];
		if (Ship->IsTrading() || Ship->GetCurrentTradeRoute() != NULL || (Ship->GetCurrentFleet() && Ship->GetCurrentFleet()->IsTraveling()) || Ship->GetActiveCargoBay()->GetCapacity() == 0)
		{
			continue;
		}

		if (Ship->GetDamageSystem()->IsStranded())
		{
			DamagedCapacity += Ship->GetActiveCargoBay()->GetCapacity();
		}
	}
	return DamagedCapacity;
}

int32 UFlareCompanyAI::GetCargosCapacity()
{
	int32 Capacity = 0;
	for (UFlareSimulatedSpacecraft* Ship : Company->GetCompanyShips())
	{
		if (Ship->GetCompany() != Company || Ship->GetActiveCargoBay()->GetCapacity() == 0)
		{
			continue;
		}

		Capacity += Ship->GetActiveCargoBay()->GetCapacity();
	}
	return Capacity;
}

TArray<UFlareSimulatedSpacecraft*> UFlareCompanyAI::FindIdleMilitaryShips() const
{
	TArray<UFlareSimulatedSpacecraft*> IdleMilitaryShips;

	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];

		for (int32 ShipIndex = 0 ; ShipIndex < Sector->GetSectorShips().Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = Sector->GetSectorShips()[ShipIndex];
			if (Ship->GetCompany() != Company || !Ship->IsMilitary() || (Ship->GetCurrentFleet() && Ship->GetCurrentFleet()->IsTraveling()))
			{
				continue;
			}

			IdleMilitaryShips.Add(Ship);
		}
	}

	return IdleMilitaryShips;
}

float UFlareCompanyAI::GetShipyardUsageRatio() const
{
	float LargeSum = 0;
	float LargeCount = 0;
	float SmallSum = 0;
	float SmallCount = 0;

	for (UFlareSimulatedSpacecraft* Shipyard: Shipyards)
	{
		for (UFlareFactory* Factory : Shipyard->GetFactories())
		{
			if (Factory->IsLargeShipyard())
			{
				LargeCount++;
				if (Factory->GetTargetShipCompany() != NAME_None)
				{
					LargeSum++;
				}
			}
			else
			{
				SmallCount++;
				if (Factory->GetTargetShipCompany() != NAME_None)
				{
					SmallSum++;
				}
			}
		}
	}

	float LargeRatio = (LargeCount > 0 ? LargeSum / LargeCount : 1.f);
	float SmallRatio = (SmallCount > 0 ? SmallSum / SmallCount : 1.f);

	return FMath::Max(LargeRatio, SmallRatio);
}

bool UFlareCompanyAI::ComputeAvailableConstructionResourceAvailability(int32 MinimumQuantity) const
{
	UFlareScenarioTools* ST = Game->GetScenarioTools();
	bool OverMinimumQuantity = true;
	if (WorldStats[ST->Steel].Stock < MinimumQuantity || WorldStats[ST->Plastics].Stock < MinimumQuantity || WorldStats[ST->Tools].Stock < MinimumQuantity)
	{
		OverMinimumQuantity = false;
	}
	return OverMinimumQuantity;
}

float UFlareCompanyAI::ComputeConstructionScoreForStation(UFlareSimulatedSector* Sector, FFlareSpacecraftDescription* StationDescription, FFlareFactoryDescription* FactoryDescription, UFlareSimulatedSpacecraft* Station, bool Technology) const
{
	// The score is a number between 0 and infinity. A classical score is 1. If 0, the company don't want to build this station

	// Multiple parameter impact the score
	// Base score is 1
	// x sector affility
	// x resource affility if produce a resource
	// x customer, maintenance, shipyard affility if as the capability
	//
	// Then some world state multiplier occurs
	// - for factories : if the resource world flow of a input resourse is negative, multiply by 1 to 0 for 0 to x% (x is resource afficility) of negative ratio
	// - for factories : if the resource world flow of a output resourse is positive, multiply by 1 to 0 for 0 to x% (x is resource afficility) of positive ratio
	// - for customer, if customer affility > customer consumption in sector reduce the score
	// - maintenance, same with world FS consumption
	// - for shipyard, if a own shipyard is not used, don't do one
	//
	// - x 2 to 0, for the current price of input and output resource. If output resource price is min : 0, if max : 2. Inverse for input
	//
	// - Time to pay the construction price multiply from 1 for 1 day to 0 for infinity. 0.5 at 200 days

	/*if (StationDescription->Capabilities.Contains(EFlareSpacecraftCapability::Maintenance))
	{
		FLOGV(">>>>>Score for %s in %s", *StationDescription->Identifier.ToString(), *Sector->GetIdentifier().ToString());
	}*/

	if(Technology != StationDescription->IsResearch())
	{
		return 0;
	}

	float Score = 1.0f;

	//TODO customer, maintenance and shipyard limit
	Score *= Behavior->GetSectorAffility(Sector);
	//FLOGV(" after sector Affility: %f", Score);

	if (StationDescription->Capabilities.Contains(EFlareSpacecraftCapability::Consumer))
	{
		Score *= Behavior->ConsumerAffility;

		const SectorVariation* ThisSectorVariation = &WorldResourceVariation[Sector];

		float MaxScoreModifier = 0;

		for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->ConsumerResources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->ConsumerResources[ResourceIndex]->Data;
			const struct ResourceVariation* Variation = &ThisSectorVariation->ResourceVariations[Resource];


			float Consumption = Sector->GetPeople()->GetRessourceConsumption(Resource, false);
			//FLOGV("%s comsumption = %f", *Resource->Name.ToString(), Consumption);

			float ReserveStock =  Variation->ConsumerMaxStock / 10.f;
			//FLOGV("ReserveStock = %f", ReserveStock);
			if (Consumption < ReserveStock)
			{
				float ScoreModifier = 2.f * ((Consumption / ReserveStock) - 0.5);
				if (ScoreModifier > MaxScoreModifier)
				{
					MaxScoreModifier = ScoreModifier;
				}
			}
			else if (Consumption > 0)
			{
				MaxScoreModifier = 1;
				break;
			}
			// If superior, keep 1
		}
		Score *= MaxScoreModifier;
		float StationPrice = ComputeStationPrice(Sector, StationDescription, Station);
		Score *= 1.f + 1/StationPrice;
		//FLOGV("MaxScoreModifier = %f", MaxScoreModifier);
	}
	else if (StationDescription->Capabilities.Contains(EFlareSpacecraftCapability::Maintenance))
	{
		Score *= Behavior->MaintenanceAffility;

		const SectorVariation* ThisSectorVariation = &WorldResourceVariation[Sector];

		float MaxScoreModifier = 0;

		for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->MaintenanceResources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->MaintenanceResources[ResourceIndex]->Data;
			const struct ResourceVariation* Variation = &ThisSectorVariation->ResourceVariations[Resource];


			int32 Consumption = WorldStats[Resource].Consumption / Company->GetKnownSectors().Num();
			//FLOGV("%s comsumption = %d", *Resource->Name.ToString(), Consumption);

			float ReserveStock =  Variation->MaintenanceMaxStock;
			//FLOGV("ReserveStock = %f", ReserveStock);
			if (Consumption < ReserveStock)
			{
				float ScoreModifier = 2.f * ((Consumption / ReserveStock) - 0.5);

				if (ScoreModifier > MaxScoreModifier)
				{
					MaxScoreModifier = ScoreModifier;
				}
			}
			else if (Consumption > 0)
			{
				MaxScoreModifier = 1;
				break;
			}
			// If superior, keep 1
		}
		Score *= MaxScoreModifier;
		//FLOGV("MaxScoreModifier = %f", MaxScoreModifier);

		float StationPrice = ComputeStationPrice(Sector, StationDescription, Station);
		Score *= 1.f + 1/StationPrice;

	}
	else if (FactoryDescription && FactoryDescription->IsResearch())
	{
		// Underflow malus
		for (int32 ResourceIndex = 0; ResourceIndex < FactoryDescription->CycleCost.InputResources.Num(); ResourceIndex++)
		{
			const FFlareFactoryResource* Resource = &FactoryDescription->CycleCost.InputResources[ResourceIndex];

			float MaxVolume = FMath::Max(WorldStats[&Resource->Resource->Data].Production, WorldStats[&Resource->Resource->Data].Consumption);
			if (MaxVolume > 0)
			{
				float UnderflowRatio = WorldStats[&Resource->Resource->Data].Balance / MaxVolume;
				if (UnderflowRatio < 0)
				{
					float UnderflowMalus = FMath::Clamp((UnderflowRatio * 100)  / 20.f + 1.f, 0.f, 1.f);
					Score *= UnderflowMalus;
					//FLOGV("    MaxVolume %f", MaxVolume);
					//FLOGV("    UnderflowRatio %f", UnderflowRatio);
					//FLOGV("    UnderflowMalus %f", UnderflowMalus);
				}
			}
			else
			{
				if(Technology)
				{
					FLOG("No input production");
				}
				// No input production, ignore this station
				return 0;
			}
		}

		float StationPrice = ComputeStationPrice(Sector, StationDescription, Station);
		Score *= 1.f + 1/StationPrice;
		if (StationDescription->IsTelescope())
		{
			if (Company->GetCompanyTelescopes().Num() >= 1 || UndiscoveredSectors.Num() < 1)
			{
				Score = 0;
			}
		}
	}
	else if (FactoryDescription && FactoryDescription->IsShipyard())
	{
		int32 TotalOwnedShipyards = 0;
		for (int32 ShipyardIndex = 0; ShipyardIndex < Shipyards.Num(); ShipyardIndex++)
		{
			UFlareSimulatedSpacecraft* Shipyard = Shipyards[ShipyardIndex];
			if (Shipyard&&Shipyard->GetCompany() == Company)
			{
				TotalOwnedShipyards++;
			}
		}

		if (TotalOwnedShipyards >= Behavior->Station_Shipyard_Maximum)
		{
			return 0;
		}

		Score *= Behavior->ShipyardAffility;
		Score *= GetShipyardUsageRatio() * 0.5;

		// Underflow malus
		for (int32 ResourceIndex = 0; ResourceIndex < FactoryDescription->CycleCost.InputResources.Num(); ResourceIndex++)
		{
			const FFlareFactoryResource* Resource = &FactoryDescription->CycleCost.InputResources[ResourceIndex];

			float MaxVolume = FMath::Max(WorldStats[&Resource->Resource->Data].Production, WorldStats[&Resource->Resource->Data].Consumption);
			if (MaxVolume > 0)
			{
				float UnderflowRatio = WorldStats[&Resource->Resource->Data].Balance / MaxVolume;
				if (UnderflowRatio < 0)
				{
					float UnderflowMalus = FMath::Clamp((UnderflowRatio * 100)  / 20.f + 1.f, 0.f, 1.f);
					Score *= UnderflowMalus;
					//FLOGV("    MaxVolume %f", MaxVolume);
					//FLOGV("    UnderflowRatio %f", UnderflowRatio);
					//FLOGV("    UnderflowMalus %f", UnderflowMalus);
				}
			}
			else
			{
				// No input production, ignore this station
				return 0;
			}
		}

		float StationPrice = ComputeStationPrice(Sector, StationDescription, Station);
		Score *= 1.f + 1/StationPrice;
		

		//FLOGV("Score=%f for %s in %s", Score, *StationDescription->Identifier.ToString(), *Sector->GetIdentifier().ToString());

	}
	else if (FactoryDescription)
	{
		float GainPerCycle = 0;

		GainPerCycle -= FactoryDescription->CycleCost.ProductionCost;

		// Factory
		for (int32 ResourceIndex = 0; ResourceIndex < FactoryDescription->CycleCost.InputResources.Num(); ResourceIndex++)
		{
			const FFlareFactoryResource* Resource = &FactoryDescription->CycleCost.InputResources[ResourceIndex];
			GainPerCycle -= Sector->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::FactoryInput) * Resource->Quantity;

			float MaxVolume = FMath::Max(WorldStats[&Resource->Resource->Data].Production, WorldStats[&Resource->Resource->Data].Consumption);
			if (MaxVolume > 0)
			{
				float UnderflowRatio = WorldStats[&Resource->Resource->Data].Balance / MaxVolume;
				if (UnderflowRatio < 0)
				{
					float UnderflowMalus = FMath::Clamp((UnderflowRatio * 100)  / 20.f + 1.f, 0.f, 1.f);
					Score *= UnderflowMalus;
					//FLOGV("    MaxVolume %f", MaxVolume);
					//FLOGV("    UnderflowRatio %f", UnderflowRatio);
					//FLOGV("    UnderflowMalus %f", UnderflowMalus);
				}
			}
			else
			{
				// No input production, ignore this station
				return 0;
			}

			float ResourcePrice = Sector->GetPreciseResourcePrice(&Resource->Resource->Data);
			float PriceRatio = (ResourcePrice - (float) Resource->Resource->Data.MinPrice) / (float) (Resource->Resource->Data.MaxPrice - Resource->Resource->Data.MinPrice);

			Score *= (1 - PriceRatio) * 2;
		}

		//FLOGV(" after input: %f", Score);

		if (Score == 0)
		{
			return 0;
		}

		for (int32 ResourceIndex = 0; ResourceIndex < FactoryDescription->CycleCost.OutputResources.Num(); ResourceIndex++)
		{
			const FFlareFactoryResource* Resource = &FactoryDescription->CycleCost.OutputResources[ResourceIndex];
			GainPerCycle += Sector->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::FactoryOutput) * Resource->Quantity;

			float ResourceAffility = Behavior->GetResourceAffility(&Resource->Resource->Data);
			Score *= ResourceAffility;

			//FLOGV(" ResourceAffility for %s: %f", *Resource->Resource->Data.Identifier.ToString(), ResourceAffility);

			float MaxVolume = FMath::Max(WorldStats[&Resource->Resource->Data].Production, WorldStats[&Resource->Resource->Data].Consumption);
			if (MaxVolume > 0)
			{
				float OverflowRatio = WorldStats[&Resource->Resource->Data].Balance / MaxVolume;
//				float StockCapacityRatio = WorldStats[&Resource->Resource->Data].Stock / WorldStats[&Resource->Resource->Data].Capacity;

//				FLOGV("    %s", *Resource->Resource->Data.Name.ToString());
				if (OverflowRatio > 0)
				{
					float OverflowMalus = FMath::Clamp(1.f - ((OverflowRatio - 0.1f) * 100) / ResourceAffility, 0.f, 1.f);
					Score *= OverflowMalus;
//					FLOGV("    MaxVolume %f", MaxVolume);
//					FLOGV("    OverflowRatio %f", OverflowRatio);
//					FLOGV("    OverflowMalus %f", OverflowMalus);
				}
/*
				if (StockCapacityRatio > 0)
				{
					float StockCapacityMalus = FMath::Clamp(1.f - ((StockCapacityRatio - 0.1f) * 100) / ResourceAffility, 0.f, 1.f);
					Score *= StockCapacityRatio;
//					FLOGV("    StockCapacityRatio %f", StockCapacityRatio);
//					FLOGV("    StockCapacityMalus %f", StockCapacityMalus);
				}
				*/
			}
			else
			{
				Score *= 1000;
			}

			float ResourcePrice = Sector->GetPreciseResourcePrice(&Resource->Resource->Data);
			float PriceRatio = (ResourcePrice - (float) Resource->Resource->Data.MinPrice) / (float) (Resource->Resource->Data.MaxPrice - Resource->Resource->Data.MinPrice);


			//FLOGV("    PriceRatio %f", PriceRatio);


			Score *= PriceRatio * 2;
		}

		//FLOGV(" after output: %f", Score);

		if(FactoryDescription->CycleCost.ProductionTime <=0)
		{
			// Contruction cycle
			return 0;
		}

		float GainPerDay = GainPerCycle / FactoryDescription->CycleCost.ProductionTime;
		if (GainPerDay < 0)
		{
			return 0;
		}

		float StationPrice = ComputeStationPrice(Sector, StationDescription, Station);
		float DayToPayPrice = StationPrice / GainPerDay;

		float HalfRatioDelay = 1500;

		float PaybackMalus = (HalfRatioDelay -1.f)/(DayToPayPrice+(HalfRatioDelay -2.f)); // 1for 1 day, 0.5 for 1500 days
		Score *= PaybackMalus;
	}
	else
	{
		return 0;
	}

	//FLOGV(" GainPerCycle: %f", GainPerCycle);
	//FLOGV(" GainPerDay: %f", GainPerDay);
	//FLOGV(" StationPrice: %f", StationPrice);
	//FLOGV(" DayToPayPrice: %f", DayToPayPrice);
	//FLOGV(" PaybackMalus: %f", PaybackMalus);

	/*if (StationDescription->Capabilities.Contains(EFlareSpacecraftCapability::Consumer) ||
			StationDescription->Capabilities.Contains(EFlareSpacecraftCapability::Maintenance) ||
			StationDescription->Capabilities.Contains(EFlareSpacecraftCapability::Storage)
			)
	{
	FLOGV("Score=%f for %s in %s", Score, *StationDescription->Identifier.ToString(), *Sector->GetIdentifier().ToString());
	}*/

	return Score;
}

float UFlareCompanyAI::ComputeStationPrice(UFlareSimulatedSector* Sector, FFlareSpacecraftDescription* StationDescription, UFlareSimulatedSpacecraft* Station) const
{
	float StationPrice;

	if (Station)
	{
		// Upgrade
		StationPrice = Behavior->BuildStationWorthMultiplier * (Station->GetStationUpgradeFee() +  UFlareGameTools::ComputeSpacecraftPrice(StationDescription->Identifier, Sector, true, false, false)) - 1;
	}
	else
	{
		// Construction
		StationPrice = Behavior->BuildStationWorthMultiplier * UFlareGameTools::ComputeSpacecraftPrice(StationDescription->Identifier, Sector, true, true, false, Company);
	}
	return StationPrice;
}




void UFlareCompanyAI::DumpSectorResourceVariation(UFlareSimulatedSector* Sector, TMap<FFlareResourceDescription*, struct ResourceVariation>* SectorVariation) const
{
	FLOGV("DumpSectorResourceVariation : sector %s resource variation: ", *Sector->GetSectorName().ToString());
	for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		struct ResourceVariation* Variation = &(*SectorVariation)[Resource];
		if (Variation->OwnedFlow ||
				Variation->FactoryFlow ||
				Variation->OwnedStock ||
				Variation->FactoryStock ||
				Variation->StorageStock ||
				Variation->OwnedCapacity ||
				Variation->FactoryCapacity ||
				Variation->StorageCapacity ||
				Variation->MaintenanceCapacity
				)
		{
			FLOGV(" - Resource %s", *Resource->Name.ToString());
			if (Variation->OwnedFlow)
				FLOGV("   owned flow %d / day", Variation->OwnedFlow);
			if (Variation->FactoryFlow)
				FLOGV("   factory flow %d / day", Variation->FactoryFlow);
			if (Variation->OwnedStock)
				FLOGV("   owned stock %d", Variation->OwnedStock);
			if (Variation->FactoryStock)
				FLOGV("   factory stock %d", Variation->FactoryStock);
			if (Variation->StorageStock)
				FLOGV("   storage stock %d", Variation->StorageStock);
			if (Variation->OwnedCapacity)
				FLOGV("   owned capacity %d", Variation->OwnedCapacity);
			if (Variation->FactoryCapacity)
				FLOGV("   factory capacity %d", Variation->FactoryCapacity);
			if (Variation->StorageCapacity)
				FLOGV("   storage capacity %d", Variation->StorageCapacity);
			if (Variation->MaintenanceCapacity)
				FLOGV("   maintenance capacity %d", Variation->MaintenanceCapacity);
		}

	}
}



#undef LOCTEXT_NAMESPACE