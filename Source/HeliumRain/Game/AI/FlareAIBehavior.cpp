
#include "FlareAIBehavior.h"
#include "../../Flare.h"

#include "../../Data/FlareResourceCatalog.h"

#include "../FlareGame.h"
#include "../FlareCompany.h"
#include "../FlareScenarioTools.h"

#include "../../Quests/FlareQuest.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Spacecrafts/FlareSimulatedSpacecraft.h"


//#define DEBUG_AI_NO_WAR

DECLARE_CYCLE_STAT(TEXT("FlareAIBehavior Load"), STAT_FlareAIBehavior_Load, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareAIBehavior Simulate"), STAT_FlareAIBehavior_Simulate, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareAIBehavior SimulateGeneralBehavior"), STAT_FlareAIBehavior_SimulateGeneralBehavior, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareAIBehavior SimulatePirateBehavior"), STAT_FlareAIBehavior_SimulatePirateBehavior, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareAIBehavior UpdateDiplomacy"), STAT_FlareAIBehavior_UpdateDiplomacy, STATGROUP_Flare);

#define LOCTEXT_NAMESPACE "UFlareAIBehavior"


/*----------------------------------------------------
	Public API
----------------------------------------------------*/

UFlareAIBehavior::UFlareAIBehavior(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareAIBehavior::Load(UFlareCompany* ParentCompany)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareAIBehavior_Load);

	if(!Company)
	{
		Company = ParentCompany;
		check(Company);
		CompanyDescription = Company->GetDescription();
		Game = Company->GetGame();
		ST = Game->GetScenarioTools();
		check(ST);
		GenerateAffilities();
		ProposeTributeToPlayer = false;
	}
}

void UFlareAIBehavior::Load(FFlareCompanyDescription* Description, AFlareGame* LoadingGame)
{
	CompanyDescription = Description;
	Game = LoadingGame;
	ST = Game->GetScenarioTools();
	GenerateAffilities(true);
}

void UFlareAIBehavior::Simulate()
{
	SCOPE_CYCLE_COUNTER(STAT_FlareAIBehavior_Simulate);

	// See how the player is doing

	TArray<UFlareCompany*> SortedCompanyValues = Game->GetGameWorld()->GetSortedCompanyValues();
	int64 TotalCompaniesMoney = Game->GetGameWorld()->GetTotalCompaniesMoney();

	//	TArray<UFlareCompany*> SortedCompany = Game->GetGameWorld()->GetCompanies();
//	SortedCompany.Sort(&CompanyValueComparator);

	int32 PlayerCompanyIndex = SortedCompanyValues.IndexOfByKey(GetGame()->GetPC()->GetCompany());
	int32 PlayerArmy = GetGame()->GetPC()->GetCompany()->GetCompanyValue().ArmyCurrentCombatPoints;
	int64 PlayerMoney = GetGame()->GetPC()->GetCompany()->GetMoney();

	int32 GameDifficulty = GameDifficulty = GetGame()->GetPC()->GetPlayerData()->DifficultyId;

	float PlayerExcessiveMoneyRepLoss = 0.0f;
	float PlayerExcessiveMoneyRatioRequirement = 0.0f;

	float PirateRepLoss = -0.5f;
	float RepLossDevisor = 30.f;

	switch (GameDifficulty)
	{
		case -1: // Easy
			PirateRepLoss = -0.25f;
			RepLossDevisor = 36.f;
			PlayerExcessiveMoneyRepLoss = -0.25f;
			PlayerExcessiveMoneyRatioRequirement = 0.95f;
			break;
		case 0: // Normal
			PirateRepLoss = -0.5f;
			RepLossDevisor = 30.f;
			PlayerExcessiveMoneyRepLoss = -0.50f;
			PlayerExcessiveMoneyRatioRequirement = 0.90f;
			break;
		case 1: // Hard
			PirateRepLoss = -0.75f;
			RepLossDevisor = 18.f;
			PlayerExcessiveMoneyRepLoss = -0.75f;
			PlayerExcessiveMoneyRatioRequirement = 0.80f;
			break;
		case 2: // Very Hard
			PirateRepLoss = -1.00f;
			RepLossDevisor = 11.f;
			PlayerExcessiveMoneyRepLoss = -1.0f;
			PlayerExcessiveMoneyRatioRequirement = 0.70f;
			break;
		case 3: // Expert
			PirateRepLoss = -1.25f;
			RepLossDevisor = 7.f;
			PlayerExcessiveMoneyRepLoss = -1.25f;
			PlayerExcessiveMoneyRatioRequirement = 0.60f;
			break;
		case 4: // Unfair
			PirateRepLoss = -1.50f;
			RepLossDevisor = 4.f;
			PlayerExcessiveMoneyRepLoss = -1.50f;
			PlayerExcessiveMoneyRatioRequirement = 0.50f;
			break;
	}

	float ReputationLoss = (PlayerCompanyIndex + 1) / RepLossDevisor;
	/*
	GetGame()->GetPC()->Notify(
		LOCTEXT("TestInfo", "Test Notification"),
		FText::Format(
			LOCTEXT("TestInfoFormat", "Rep loss {0}, PlayerCompanyIndex {1}"),
			ReputationLoss, PlayerCompanyIndex),
		"discover-sector",
		EFlareNotification::NT_Info,
		false);
	*/

	// Pirates hate you
	if (PlayerCompanyIndex > 0 && Company == ST->Pirates)
	{
		Company->GivePlayerReputation(PirateRepLoss);
	}

	// Competitors hate you more if you're doing well
	if (Company != ST->AxisSupplies)
	{
		if (PlayerArmy > 0 && Company->GetPlayerReputation() > ReputationLoss)
		{
			Company->GivePlayerReputation(-ReputationLoss);
		}
		if (PlayerMoney >= (TotalCompaniesMoney * PlayerExcessiveMoneyRatioRequirement))
		{
			Company->GivePlayerReputation(PlayerExcessiveMoneyRepLoss);
		}
	}

	// Simulate the day
	Company->GetAI()->AddIncomeToBudgets();
	if (Company == ST->Pirates)
	{
		SimulatePirateBehavior();
	}
	else
	{
		SimulateGeneralBehavior();
	}

}

void UFlareAIBehavior::SimulateGeneralBehavior()
{
	SCOPE_CYCLE_COUNTER(STAT_FlareAIBehavior_SimulateGeneralBehavior);

	// First make cargo evasion to avoid them to lock themselve trading
	Company->GetAI()->CargosEvasion();

	// Repair and refill ships and stations
	Company->GetAI()->RepairAndRefill();

	Company->GetAI()->ProcessBudget(Company->GetAI()->AllBudgets);

	Company->GetAI()->UpdateMilitaryMovement();
}

void UFlareAIBehavior::SimulatePirateBehavior()
{
	SCOPE_CYCLE_COUNTER(STAT_FlareAIBehavior_SimulatePirateBehavior);

	// Repair and refill ships and stations
	Company->GetAI()->RepairAndRefill();

	// First make cargo evasion to avoid them to lock themselve trading
	Company->GetAI()->CargosEvasion();

	Company->GetAI()->ProcessBudget(Company->GetAI()->AllBudgets);

	Company->GetAI()->UpdateMilitaryMovement();
}


void UFlareAIBehavior::UpdateDiplomacy(bool GlobalWar)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareAIBehavior_UpdateDiplomacy);

	// Not global war
	if (GlobalWar)
	{
		return;
	}

	TArray<UFlareCompany*> OtherCompanies = Company->GetOtherCompanies(true); // true for Shuffle
	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	if(Company->GetAI()->GetData()->Pacifism >= 100)
	{
		// Want peace
		for (UFlareCompany* OtherCompany : OtherCompanies)
		{
			Company->SetHostilityTo(OtherCompany, false);

			TArray<UFlareCompany*> Allies;

			if (Company->GetWarState(OtherCompany) == EFlareHostility::Hostile && Company->GetConfidenceLevel(OtherCompany, Allies) < PayTributeConfidence)
			{
				if (OtherCompany == PlayerCompany)
				{
					ProposeTributeToPlayer = true;
				}
				else
				{
					Company->PayTribute(OtherCompany, false);
				}
			}
		}
	}

	else if (Company->GetAI()->GetData()->Pacifism <= 0)

	{
		//Want war

		if (Company->GetWarCount(PlayerCompany) == 0 && !IsMercenaryCompany)
		{
			// Don't fight if already at war

			// Fill target company sorted by weight (only with weight higther than mine)
			TArray<TPair<UFlareCompany*, float>> TargetCompanies;

			for (UFlareCompany* OtherCompany : OtherCompanies)
			{
				if (Company->IsAtWar(OtherCompany))
				{
					continue;
				}


				//FLOGV("- %s Weight=%f", *OtherCompany->GetCompanyName().ToString(), Weight);
				//FLOGV("-    GetConfidenceLevel=%f", Company->GetConfidenceLevel(OtherCompany));

				if(Company->WantWarWith(OtherCompany))
				{
					//FLOGV("%s add %s as target", *Company->GetCompanyName().ToString(), *OtherCompany->GetCompanyName().ToString());

					TargetCompanies.Add(TPairInitializer<UFlareCompany*, float>(OtherCompany, OtherCompany->ComputeCompanyDiplomaticWeight()));
				}
			}

			TargetCompanies.Sort([&](const TPair<UFlareCompany*, float>& Lhs, const TPair<UFlareCompany*, float>& Rhs){

				if(Lhs.Key == PlayerCompany)
				{
					return true;
				}

				if(Rhs.Key == PlayerCompany)
				{
					return false;
				}

				return Lhs.Value > Rhs.Value;
			});


			auto DeclareWarConfidenceMean = [](TArray<UFlareCompany*> &Allies)
			{
				float DeclareWarConfidenceSum = 0;
				float DeclareWarConfidenceCount = 0;

				for (UFlareCompany* Ally : Allies)
				{
					DeclareWarConfidenceSum += Ally->GetAI()->GetBehavior()->DeclareWarConfidence;
					++DeclareWarConfidenceCount;
				}

				return DeclareWarConfidenceSum / DeclareWarConfidenceCount;
			};

			// For each target
			// find allies list
			// compute allies confidence level
			// compute mean of DeclareWarConfidence
			// if confidente, rand skip else declare war with all allies

			int i = 0;
			for(TPair<UFlareCompany*, float> Target : TargetCompanies)
			{
				if(i >= 3)
				{
					break;
				}
				++i;

				UFlareCompany* TargetCompany = Target.Key;
				float Skipchance = 0.2f;

				if (TargetCompany == PlayerCompany)
				{
					//if looking at player, increase the random chance factor to declare war
					int32 GameDifficulty = GameDifficulty = GetGame()->GetPC()->GetPlayerData()->DifficultyId;

					switch (GameDifficulty)
					{
					case -1: // Easy
						Skipchance = 0.25f;
						break;
					case 0: // Normal
						Skipchance = 0.2f;
						break;
					case 1: // Hard
						Skipchance = 0.18f;
						break;
					case 2: // Very Hard
						Skipchance = 0.16f;
						break;
					case 3: // Expert
						Skipchance = 0.12f;
						break;
					case 4: // Unfair
						Skipchance = 0.08f;
						break;
					}
				}
				else
				{
					//if looking at other companies, decrease the random factor chance for declaring war

					int32 GameDifficulty = GameDifficulty = GetGame()->GetPC()->GetPlayerData()->DifficultyId;

					switch (GameDifficulty)
					{
					case -1: // Easy
						Skipchance = 0.2f;
						break;
					case 0: // Normal
						Skipchance = 0.2f;
						break;
					case 1: // Hard
						Skipchance = 0.25f;
						break;
					case 2: // Very Hard
						Skipchance = 0.35f;
						break;
					case 3: // Expert
						Skipchance = 0.50f;
						break;
					case 4: // Unfair
						Skipchance = 0.60f;
						break;
					}

					//and because reputation is a player only mechanic, we chuck in a diplomacy modifier to the skip chance of the target
					float TechnologyBonus = TargetCompany->IsTechnologyUnlocked("diplomacy") ? 1.5f : 1.f;
					Skipchance *= TechnologyBonus;
				}

				//FLOGV("Target for %s: %s", *Company->GetCompanyName().ToString(), *TargetCompany->GetCompanyName().ToString());
				if(FMath::FRand() <= Skipchance)
				{
					continue;
				}

				TArray<UFlareCompany*> Allies;
				float Confidence = Company->GetConfidenceLevel(TargetCompany, Allies);
				if(Confidence > DeclareWarConfidenceMean(Allies))
				{
					FLOGV("Declare alliance agains %s with:", *TargetCompany->GetCompanyName().ToString());
					for (UFlareCompany* Ally : Allies)
					{
						FLOGV("- %s", *Ally->GetCompanyName().ToString());
						Ally->SetHostilityTo(TargetCompany, true);

					}
				}
			}
		}
	}
}

void UFlareAIBehavior::GenerateAffilities(bool Basic)
{
	// Reset resource affilities
	ResourceAffilities.Empty();
	ResearchOrder.Empty();

	// Default behavior
	SetResourceAffilities(1.f);

	// If above 5, will become exclusive
	if (!Basic)
	{
		SetSectorAffilities(1.f);
	}

	StationCapture = 1.f;
	TradingBuy = 1.f;
	TradingSell = 1.f;
	TradingBoth = 0.5f;

	DaysUntilTryGetStationLicense = 365;
	ShipyardAffility = 1.f;
	ConsumerAffility = 0.5f;
	MaintenanceAffility = 0.1f;

	// Budget
	BudgetTechnologyWeight = 0.05f;
	BudgetMilitaryWeight = 0.30f;
	BudgetStationWeight = 0.34f;
	BudgetTradeWeight = 0.31f;

	BudgetWarTechnologyWeight = 0.00f;
	BudgetWarMilitaryWeight = 0.75f;
	BudgetWarStationWeight = 0.125f;
	BudgetWarTradeWeight = 0.125f;

	WarDeclared_StationBudgetFactor = 1.05f;
	WarDeclared_TechnologyBudgetFactor = 0.85f;
	WarDeclared_TradeBudgetFactor = 1.25f;
	WarDeclared_TransferToMilitaryBudgetFactor = 1.10f;

	WarDeclared_TransferToMilitaryBudgetStationPercent = 0.05f;
	WarDeclared_TransferToMilitaryBudgetTechnologyPercent = 0.04f;
	WarDeclared_TransferToMilitaryBudgetTradePercent = 0.05f;

	MaxTradeShipsBuildingPeace = 1;
	MaxTradeShipsBuildingWar = 0;
	MaxMilitaryShipsBuildingPeace = 1;
	MaxMilitaryShipsBuildingWar = 100;

	ConfidenceTarget = -0.1f;
	DeclareWarConfidence = 0.2f;
	RequestPeaceConfidence = -0.5f;
	PayTributeConfidence = -0.8f;

	AttackThreshold = 0.99f;
	RetreatThreshold = 0.5f;
	DefeatAdaptation = 0.01f;

	DiplomaticReactivity = 1;

	PacifismAfterTribute = 75;
	PacifismIncrementRate = 0.8f;
	PacifismDecrementRate = 0.6f;

	DailyProductionCostSensitivityMilitary = 30;
	DailyProductionCostSensitivityEconomic = 7;
	CostSafetyMarginMilitaryShip = 1.1f;
	CostSafetyMarginTradeShip = 1.05f;
	CostSafetyMarginStation = 1.1f;

	BuildLTradeOnlyTreshhold = 50;
	BuildLMilitaryOnlyTreshhold = 50;

	BuildMilitaryDiversity = 1;
	BuildTradeDiversity = 1;

	// Pirate base
	if (!Basic)
	{
		SetSectorAffility(ST->Boneyard, 0.f);
	}

	UpgradeMilitarySalvagerSRatio = 0.10f;
	UpgradeMilitarySalvagerLRatio = 0.10f;

	BuildMilitaryDiversitySize = 5;
	BuildTradeDiversitySize = 5;
	BuildMilitaryDiversitySizeBase = 5;
	BuildTradeDiversitySizeBase = 10;
	
	BuildEfficientMilitaryChance = 0.05f;
	BuildEfficientMilitaryChanceSmall = 0.10f;

	BuildEfficientTradeChance = 0.10f;
	BuildEfficientTradeChanceSmall = 0.30f;

	BuildStationWorthMultiplier = 1.2f;
	Station_Shipyard_Maximum = 5;
	Scrap_Minimum_Ships = 60;
	Scrap_Min_S_Cargo = 10;
	Scrap_Min_S_Military = 10;
	BuildDroneCombatWorth = 3;

	if((Company && Company == ST->Pirates) || (CompanyDescription && CompanyDescription->ShortName==FName("PIR")))
	{
		// Pirates
		// -------
		//
		// Pirate don't trade, and are only interested in getting money by force
		// All resource affilities are set to 0
		// They never build stations
		// They never research science

		// They have 2 mode : agressive or moderate

		// They need a shipyard, if they have a shipyard they will steal resources,
		// or buy one to build ship, and fill and defend their base
		// They always want graveyard

		// They need some cargo to buy resources for their shipyard and arsenal

		SetResourceAffilities(0.1f);
		if (!Basic)
		{
			SetSectorAffilities(0.f);
			SetSectorAffility(ST->Boneyard, 10.f);
		}

		ShipyardAffility = 0.f;
		ConsumerAffility = 0.10f;
		MaintenanceAffility = 0.05f;

		// Only buy
		TradingSell = 0.f;
		TradingBoth = 0.f;

		// Doesn't capture station. Change in recovery
		StationCapture = 0.f;

		DeclareWarConfidence = -0.2f;
		RequestPeaceConfidence = -0.8f;
		PayTributeConfidence = -1.0f;

		AttackThreshold = 0.8f;
		RetreatThreshold = 0.2f;
		DefeatAdaptation = 0.001f;

		// Budget
		MaxMilitaryShipsBuildingPeace = 3;

		BudgetTechnologyWeight = 0.f;
		BudgetMilitaryWeight = 0.90f;
		BudgetStationWeight = 0.075f;
		BudgetTradeWeight = 0.025f;

		BudgetWarTechnologyWeight = 0.f;
		BudgetWarMilitaryWeight = 0.95f;
		BudgetWarStationWeight = 0.0375f;
		BudgetWarTradeWeight = 0.0125f;

		WarDeclared_TransferToMilitaryBudgetFactor = 0.50f;

		PacifismAfterTribute = 50;
		PacifismIncrementRate = 1.f;
		PacifismDecrementRate = 0.8f;

		DailyProductionCostSensitivityMilitary = 0;
		DailyProductionCostSensitivityEconomic = 0;

		BuildEfficientMilitaryChance = 0.0125f;
		BuildEfficientMilitaryChanceSmall = 0.025f;
		UpgradeMilitarySalvagerSRatio = 0.25f;
		UpgradeMilitarySalvagerLRatio = 0.25f;
		ResearchOrder.Reserve(4);
		ResearchOrder.Add("instruments");
		ResearchOrder.Add("flak");
		ResearchOrder.Add("fast-travel");
		ResearchOrder.Add("bombing");
	}
	else if((Company && Company == ST->GhostWorksShipyards) || (CompanyDescription && CompanyDescription->ShortName == FName("GWS")))
	{
		// Loves Hela and doesn't like Nema
		ShipyardAffility = 6.0;

		if (!Basic)
		{
			SetSectorAffilitiesByMoon(ST->Nema, 0.5f);
			SetSectorAffilitiesByMoon(ST->Hela, 6.f);
			SetResourceAffility(ST->Steel, 2.f);
			SetResourceAffility(ST->Plastics, 1.5f);
			SetResourceAffility(ST->Tools, 1.25f);
		}
		else
		{
			SetResourceAffility(Game->GetResourceCatalog()->Get("steel"), 2.f);
			SetResourceAffility(Game->GetResourceCatalog()->Get("plastics"), 1.5f);
			SetResourceAffility(Game->GetResourceCatalog()->Get("tools"), 1.25f);
		}

		// Budget
		MaxTradeShipsBuildingWar = 1;
		MaxMilitaryShipsBuildingPeace = 2;

		BuildMilitaryDiversity = 2;
		BuildTradeDiversity = 2;
		BuildEfficientTradeChance = 0.15f;
		ResearchOrder.Reserve(2);
		ResearchOrder.Add("science");
		ResearchOrder.Add("instruments");
	}
	else if((Company && Company == ST->MiningSyndicate) || (CompanyDescription && CompanyDescription->ShortName == FName("MSY")))
	{
		// Mining specialist.
		// Loves raw materials
		// Likes hard work

		if (!Basic)
		{
			SetResourceAffility(ST->Water, 10.f);
			SetResourceAffility(ST->Silica, 10.f);
			SetResourceAffility(ST->IronOxyde, 10.f);
			SetResourceAffility(ST->Hydrogen, 2.f);
		}
		else
		{
			SetResourceAffility(Game->GetResourceCatalog()->Get("h2o"), 10.f);
			SetResourceAffility(Game->GetResourceCatalog()->Get("sio2"), 10.f);
			SetResourceAffility(Game->GetResourceCatalog()->Get("feo"), 10.f);
			SetResourceAffility(Game->GetResourceCatalog()->Get("h2"), 2.f);
		}

		// Budget
		BudgetTechnologyWeight = 0.02f;
		BudgetMilitaryWeight = 0.10f;
		BudgetStationWeight = 0.50f;
		BudgetTradeWeight = 0.38f;

		BudgetWarTechnologyWeight = 0.f;
		BudgetWarMilitaryWeight = 0.50f;
		BudgetWarStationWeight = 0.25f;
		BudgetWarTradeWeight = 0.25f;

		WarDeclared_TransferToMilitaryBudgetTechnologyPercent = 0.01f;

		BuildEfficientMilitaryChance = 0.025f;
		BuildEfficientMilitaryChanceSmall = 0.5f;

		BuildEfficientTradeChance = 0.05f;
		BuildEfficientTradeChanceSmall = 0.15f;
		ResearchOrder.Reserve(5);
		ResearchOrder.Add("science");
		ResearchOrder.Add("instruments");
		ResearchOrder.Add("orbital-pumps");
		ResearchOrder.Add("metallurgy");
		ResearchOrder.Add("chemicals");
	}
	else if((Company && Company == ST->HelixFoundries) || (CompanyDescription && CompanyDescription->ShortName == FName("HFR")))
	{
		// Likes hard factory, and Anka.
		// Base at Outpost

		if (!Basic)
		{
			SetSectorAffilitiesByMoon(ST->Anka, 6.f);
			SetSectorAffility(ST->Outpost, 10.f);
			SetResourceAffility(ST->Steel, 15.f);
			SetResourceAffility(ST->Tools, 10.f);
			SetResourceAffility(ST->Tech, 5.f);
		}
		else
		{
			SetResourceAffility(Game->GetResourceCatalog()->Get("steel"), 15.f);
			SetResourceAffility(Game->GetResourceCatalog()->Get("tools"), 10.f);
			SetResourceAffility(Game->GetResourceCatalog()->Get("tech"), 5.f);
		}

		// Budget
		ResearchOrder.Reserve(2);
		ResearchOrder.Add("science");
		ResearchOrder.Add("instruments");
	}
	else if((Company && Company == ST->Sunwatch) || (CompanyDescription && CompanyDescription->ShortName == FName("SUN")))
	{
		// Likes hard factory, and Anka.
		// Base at Outpost

		if (!Basic)
		{
			SetResourceAffility(ST->Fuel, 10.f);
		}
		else
		{
			SetResourceAffility(Game->GetResourceCatalog()->Get("fuel"), 10.f);
		}

		// Budget
		//already starts with instruments
		ResearchOrder.Reserve(2);
		ResearchOrder.Add("science");
		ResearchOrder.Add("advanced-stations");
	}
	else if((Company && Company == ST->IonLane) || (CompanyDescription && CompanyDescription->ShortName == FName("ION")))
	{
		CostSafetyMarginMilitaryShip = 1.2f;
		CostSafetyMarginTradeShip = 1.0f;

		BudgetTechnologyWeight = 0.01f;
		BudgetMilitaryWeight = 0.10f;
		BudgetStationWeight = 0.30f;
		BudgetTradeWeight = 0.59f;

		BudgetWarTechnologyWeight = 0.f;
		BudgetWarMilitaryWeight = 0.50f;
		BudgetWarStationWeight = 0.15f;
		BudgetWarTradeWeight = 0.35f;

		WarDeclared_TransferToMilitaryBudgetTechnologyPercent = 0.01f;

		MaxTradeShipsBuildingPeace = 2;
		MaxTradeShipsBuildingWar = 1;

		DeclareWarConfidence = 0.1f;
		RequestPeaceConfidence = -0.4f;
		PayTributeConfidence = -0.85f;

		BuildLTradeOnlyTreshhold = 40;
		BuildTradeDiversity = 2;
		BuildTradeDiversitySize = 4;
		BuildEfficientTradeChance = 0.20f;
		ResearchOrder.Reserve(4);
		ResearchOrder.Add("science");
		ResearchOrder.Add("instruments");
		ResearchOrder.Add("fast-travel");
		ResearchOrder.Add("quick-repair");
	}
	else if((Company && Company == ST->UnitedFarmsChemicals) || (CompanyDescription && CompanyDescription->ShortName == FName("UFC")))
	{
		// Likes chemisty
		if (!Basic)
		{
			SetResourceAffility(ST->Food, 10.f);
			SetResourceAffility(ST->Carbon, 5.f);
			SetResourceAffility(ST->Methane, 5.f);
		}
		else
		{
			SetResourceAffility(Game->GetResourceCatalog()->Get("food"), 10.f);
			SetResourceAffility(Game->GetResourceCatalog()->Get("carbon"), 5.f);
			SetResourceAffility(Game->GetResourceCatalog()->Get("ch4"), 5.f);
		}
		// Budget
		BuildEfficientMilitaryChance = 0.025f;
		BuildEfficientMilitaryChanceSmall = 0.5f;

		BuildEfficientTradeChance = 0.05f;
		BuildEfficientTradeChanceSmall = 0.15f;
		ResearchOrder.Reserve(2);
		ResearchOrder.Add("science");
		ResearchOrder.Add("instruments");
	}
	else if((Company && Company == ST->NemaHeavyWorks) || (CompanyDescription && CompanyDescription->ShortName == FName("NHW")))
	{
		// Likes Nema and heavy work
		if (!Basic)
		{
			SetSectorAffilitiesByMoon(ST->Nema, 5.f);
			SetResourceAffility(ST->FleetSupply, 2.f);
			SetResourceAffility(ST->Steel, 7.f);
			SetResourceAffility(ST->Tools, 5.f);
			SetResourceAffility(ST->Tech, 5.f);
		}
		else
		{
			SetResourceAffility(Game->GetResourceCatalog()->Get("fleet-supply"), 2.f);
			SetResourceAffility(Game->GetResourceCatalog()->Get("steel"), 7.f);
			SetResourceAffility(Game->GetResourceCatalog()->Get("tools"), 5.f);
			SetResourceAffility(Game->GetResourceCatalog()->Get("tech"), 5.f);
		}
		ShipyardAffility = 3.f;

		// Budget
		BuildMilitaryDiversity = 2;
		BuildTradeDiversity = 2;

		MaxTradeShipsBuildingWar = 1;
		MaxMilitaryShipsBuildingPeace = 2;

		BuildEfficientTradeChance = 0.15f;
		ResearchOrder.Reserve(3);
		ResearchOrder.Add("science");
		ResearchOrder.Add("instruments");
		ResearchOrder.Add("shipyard-station");
	}
	else if((Company && Company == ST->AxisSupplies) || (CompanyDescription && CompanyDescription->ShortName == FName("AXS")))
	{
		// Assures fleet supply availability
		DaysUntilTryGetStationLicense = 60;
		PacifismDecrementRate = 0.5f;

		if (!Basic)
		{
			SetResourceAffility(ST->FleetSupply, 5.f);
			SetResourceAffility(ST->Food, 2.f);
		}
		else
		{
			SetResourceAffility(Game->GetResourceCatalog()->Get("fleet-supply"), 5.f);
			SetResourceAffility(Game->GetResourceCatalog()->Get("food"), 2.f);
		}

		ShipyardAffility = 0.f;
		MaintenanceAffility = 10.f;

		CostSafetyMarginMilitaryShip = 1.2f;
		CostSafetyMarginTradeShip = 1.0f;

		// Budget
		BudgetTechnologyWeight = 0.05f;
		BudgetMilitaryWeight = 0.05f;
		BudgetStationWeight = 0.30f;
		BudgetTradeWeight = 0.60f;

		BudgetWarTechnologyWeight = 0.f;
		BudgetWarMilitaryWeight = 0.50f;
		BudgetWarStationWeight = 0.15f;
		BudgetWarTradeWeight = 0.35f;

		DeclareWarConfidence = 1.f;
		RequestPeaceConfidence = 0.f;
		PayTributeConfidence = -0.1f;

		DiplomaticReactivity = 0.1f;
		BuildLTradeOnlyTreshhold = 40;
		BuildTradeDiversitySize = 4;

		BuildEfficientTradeChance = 0.15f;
		ResearchOrder.Reserve(3);
		ResearchOrder.Add("science");
		ResearchOrder.Add("instruments");
		ResearchOrder.Add("fast-travel");
	}
	else if ((Company && Company == ST->BrokenMoon) || (CompanyDescription && CompanyDescription->ShortName == FName("BRM")))
	{
		DailyProductionCostSensitivityMilitary = 7;
		DailyProductionCostSensitivityEconomic = 30;
		BuildLMilitaryOnlyTreshhold = 40;

		// budget
		BudgetTechnologyWeight = 0.01f;
		BudgetMilitaryWeight = 0.61f;
		BudgetStationWeight = 0.30f;
		BudgetTradeWeight = 0.08f;

		BudgetWarTechnologyWeight = 0.f;
		BudgetWarMilitaryWeight = 0.75f;
		BudgetWarStationWeight = 0.15f;
		BudgetWarTradeWeight = 0.10f;

		WarDeclared_TransferToMilitaryBudgetFactor = 0.75f;

		MaxMilitaryShipsBuildingPeace = 2;

		BuildMilitaryDiversity = 2;
		BuildMilitaryDiversitySize = 4;
		BuildMilitaryDiversitySizeBase = 0;
		PacifismAfterTribute = 50;
		BuildEfficientMilitaryChance = 0.10f;
		BuildEfficientMilitaryChanceSmall = 0.15f;
		ResearchOrder.Reserve(6);
		ResearchOrder.Add("instruments");
		ResearchOrder.Add("pirate-tech");
		ResearchOrder.Add("fast-travel");
		ResearchOrder.Add("bombing");
		ResearchOrder.Add("flak");
		ResearchOrder.Add("quick-repair");
	}
	else if ((Company && Company == ST->Quantalium) || (CompanyDescription && CompanyDescription->ShortName == FName("QNT")))
	{
		PacifismDecrementRate = 0.45;

		if (!Basic)
		{
			SetResourceAffility(ST->Tech, 10.f);
		}
		else
		{
			SetResourceAffility(Game->GetResourceCatalog()->Get("tech"), 10.f);
		}

		BudgetTechnologyWeight = 0.30f;
		BudgetMilitaryWeight = 0.10f;
		BudgetStationWeight = 0.30f;
		BudgetTradeWeight = 0.30f;

		BudgetWarTechnologyWeight = 0.05f;
		BudgetWarMilitaryWeight = 0.50f;
		BudgetWarStationWeight = 0.20f;
		BudgetWarTradeWeight = 0.25f;

		WarDeclared_TechnologyBudgetFactor = 0.40f;
		WarDeclared_TradeBudgetFactor = 1.50f;
		WarDeclared_TransferToMilitaryBudgetStationPercent = 0.075f;
		WarDeclared_TransferToMilitaryBudgetTechnologyPercent = 0.015f;
		WarDeclared_TransferToMilitaryBudgetTradePercent = 0.075f;

		MaxTradeShipsBuildingWar = 1;

		BuildEfficientMilitaryChance = 0.10f;
		BuildEfficientMilitaryChanceSmall = 0.15f;

		BuildEfficientTradeChance = 0.20f;
		BuildEfficientTradeChanceSmall = 0.30f;
		ResearchOrder.Reserve(3);
		ResearchOrder.Add("science");
		ResearchOrder.Add("instruments");
		ResearchOrder.Add("advanced-stations");
	}
	else if((Company && Company == ST->InfiniteOrbit) || (CompanyDescription && CompanyDescription->ShortName == FName("IFO")))
	{
		PacifismDecrementRate = 0.45f;
		MaxTradeShipsBuildingPeace = 2;
		MaxTradeShipsBuildingWar = 1;
		ResearchOrder.Reserve(3);
		ResearchOrder.Add("science");
		ResearchOrder.Add("instruments");
		ResearchOrder.Add("fast-travel");
	}

	if (!Basic)
	{
		if (CompanyDescription->AI_Behaviours.SectorAffilities.Num() > 0)
		{
			UFlareWorld* World = Game->GetGameWorld();
			TArray<FName> Keys;
			CompanyDescription->AI_Behaviours.SectorAffilities.GetKeys(Keys);
			for (int32 ROIndex = 0; ROIndex < Keys.Num(); ROIndex++)
			{
				FName CurrentSectorID = Keys[ROIndex];
				UFlareSimulatedSector* RealSector = World->FindSector(CurrentSectorID);

				if (RealSector != NULL)
				{
					float Value = CompanyDescription->AI_Behaviours.SectorAffilities[CurrentSectorID];
					SetSectorAffility(RealSector, Value);
				}
				else
				{
					FFlareCelestialBody* CelestialBody = World->GetPlanerarium()->FindCelestialBody(CurrentSectorID);
					if (CelestialBody)
					{
						float Value = CompanyDescription->AI_Behaviours.SectorAffilities[CurrentSectorID];
						SetSectorAffilitiesByMoon(CelestialBody, Value);
					}
				}
			}
		}
	}

	if (CompanyDescription->AI_Behaviours.ResourceAffilities.Num() > 0)
	{
		TArray<FName> Keys;
		CompanyDescription->AI_Behaviours.ResourceAffilities.GetKeys(Keys);

		for (int32 ROIndex = 0; ROIndex < Keys.Num(); ROIndex++)
		{
			FName CurrentResource = Keys[ROIndex];
			float Value = CompanyDescription->AI_Behaviours.ResourceAffilities[CurrentResource];
			SetResourceAffility(Game->GetResourceCatalog()->Get(CurrentResource), Value);
		}
	}

	if (CompanyDescription->AI_Behaviours.ResearchOrder.Num() > 0)
	{
		ResearchOrder.Empty();
		for (int32 ROIndex = 0; ROIndex < CompanyDescription->AI_Behaviours.ResearchOrder.Num(); ROIndex++)
		{
			FName CurrentResearch = CompanyDescription->AI_Behaviours.ResearchOrder[ROIndex];
			ResearchOrder.Add(CurrentResearch);
		}
	}
	else if(!ResearchOrder.Num())
	{
		ResearchOrder.Reserve(2);
		ResearchOrder.Add("science");
		ResearchOrder.Add("instruments");
	}

	if (CompanyDescription->AI_Behaviours.IsMercenaryCompany)
	{
		IsMercenaryCompany = CompanyDescription->AI_Behaviours.IsMercenaryCompany;
	}

	if (CompanyDescription->AI_Behaviours.DaysUntilTryGetStationLicense)
	{
		DaysUntilTryGetStationLicense = CompanyDescription->AI_Behaviours.DaysUntilTryGetStationLicense;
	}

	if (CompanyDescription->AI_Behaviours.AttackThreshold)
	{
		AttackThreshold = CompanyDescription->AI_Behaviours.AttackThreshold;
	}
	if (CompanyDescription->AI_Behaviours.TradingBuy)
	{
		TradingBuy = CompanyDescription->AI_Behaviours.TradingBuy;
	}
	if (CompanyDescription->AI_Behaviours.TradingSell)
	{
		TradingSell = CompanyDescription->AI_Behaviours.TradingSell;
	}

	if (CompanyDescription->AI_Behaviours.ShipyardAffility)
	{
		ShipyardAffility = CompanyDescription->AI_Behaviours.ShipyardAffility;
	}
	if (CompanyDescription->AI_Behaviours.ConsumerAffility)
	{
		ConsumerAffility = CompanyDescription->AI_Behaviours.ConsumerAffility;
	}
	if (CompanyDescription->AI_Behaviours.MaintenanceAffility)
	{
		MaintenanceAffility = CompanyDescription->AI_Behaviours.MaintenanceAffility;
	}
	if (CompanyDescription->AI_Behaviours.BudgetTechnologyWeight)
	{
		BudgetTechnologyWeight = CompanyDescription->AI_Behaviours.BudgetTechnologyWeight;
	}
	if (CompanyDescription->AI_Behaviours.BudgetMilitaryWeight)
	{
		BudgetMilitaryWeight = CompanyDescription->AI_Behaviours.BudgetMilitaryWeight;
	}
	if (CompanyDescription->AI_Behaviours.BudgetStationWeight)
	{
		BudgetStationWeight = CompanyDescription->AI_Behaviours.BudgetStationWeight;
	}

	if (CompanyDescription->AI_Behaviours.BudgetTradeWeight)
	{
		BudgetTradeWeight = CompanyDescription->AI_Behaviours.BudgetTradeWeight;
	}

	if (CompanyDescription->AI_Behaviours.BudgetWarTechnologyWeight)
	{
		BudgetWarTechnologyWeight = CompanyDescription->AI_Behaviours.BudgetWarTechnologyWeight;
	}
	if (CompanyDescription->AI_Behaviours.BudgetWarMilitaryWeight)
	{
		BudgetWarMilitaryWeight = CompanyDescription->AI_Behaviours.BudgetWarMilitaryWeight;
	}
	if (CompanyDescription->AI_Behaviours.BudgetWarStationWeight)
	{
		BudgetWarStationWeight = CompanyDescription->AI_Behaviours.BudgetWarStationWeight;
	}

	if (CompanyDescription->AI_Behaviours.WarDeclared_StationBudgetFactor)
	{
		WarDeclared_StationBudgetFactor = CompanyDescription->AI_Behaviours.WarDeclared_StationBudgetFactor;
	}
	if (CompanyDescription->AI_Behaviours.WarDeclared_TechnologyBudgetFactor)
	{
		WarDeclared_TechnologyBudgetFactor = CompanyDescription->AI_Behaviours.WarDeclared_TechnologyBudgetFactor;
	}
	if (CompanyDescription->AI_Behaviours.WarDeclared_TradeBudgetFactor)
	{
		WarDeclared_TradeBudgetFactor = CompanyDescription->AI_Behaviours.WarDeclared_TradeBudgetFactor;
	}
	if (CompanyDescription->AI_Behaviours.WarDeclared_TransferToMilitaryBudgetFactor)
	{
		WarDeclared_TransferToMilitaryBudgetFactor = CompanyDescription->AI_Behaviours.WarDeclared_TransferToMilitaryBudgetFactor;
	}

	if (CompanyDescription->AI_Behaviours.WarDeclared_TransferToMilitaryBudgetStationPercent)
	{
		WarDeclared_TransferToMilitaryBudgetStationPercent = CompanyDescription->AI_Behaviours.WarDeclared_TransferToMilitaryBudgetStationPercent;
	}
	if (CompanyDescription->AI_Behaviours.WarDeclared_TransferToMilitaryBudgetTechnologyPercent)
	{
		WarDeclared_TransferToMilitaryBudgetTechnologyPercent = CompanyDescription->AI_Behaviours.WarDeclared_TransferToMilitaryBudgetTechnologyPercent;
	}
	if (CompanyDescription->AI_Behaviours.WarDeclared_TransferToMilitaryBudgetTradePercent)
	{
		WarDeclared_TransferToMilitaryBudgetTradePercent = CompanyDescription->AI_Behaviours.WarDeclared_TransferToMilitaryBudgetTradePercent;
	}

	if (CompanyDescription->AI_Behaviours.BudgetWarTradeWeight)
	{
		BudgetWarTradeWeight = CompanyDescription->AI_Behaviours.BudgetWarTradeWeight;
	}

	if (CompanyDescription->AI_Behaviours.MaxTradeShipsBuildingPeace)
	{
		MaxTradeShipsBuildingPeace = CompanyDescription->AI_Behaviours.MaxTradeShipsBuildingPeace;
	}

	if (CompanyDescription->AI_Behaviours.MaxTradeShipsBuildingWar)
	{
		MaxTradeShipsBuildingWar = CompanyDescription->AI_Behaviours.MaxTradeShipsBuildingWar;
	}

	if (CompanyDescription->AI_Behaviours.MaxMilitaryShipsBuildingPeace)
	{
		MaxMilitaryShipsBuildingPeace = CompanyDescription->AI_Behaviours.MaxMilitaryShipsBuildingPeace;
	}

	if (CompanyDescription->AI_Behaviours.MaxMilitaryShipsBuildingWar)
	{
		MaxMilitaryShipsBuildingWar = CompanyDescription->AI_Behaviours.MaxMilitaryShipsBuildingWar;
	}

	if (CompanyDescription->AI_Behaviours.AttackThreshold)
	{
		AttackThreshold = CompanyDescription->AI_Behaviours.AttackThreshold;
	}
	if (CompanyDescription->AI_Behaviours.RetreatThreshold)
	{
		RetreatThreshold = CompanyDescription->AI_Behaviours.RetreatThreshold;
	}
	if (CompanyDescription->AI_Behaviours.DefeatAdaptation)
	{
		DefeatAdaptation = CompanyDescription->AI_Behaviours.DefeatAdaptation;
	}

	if (CompanyDescription->AI_Behaviours.ConfidenceTarget)
	{
		ConfidenceTarget = CompanyDescription->AI_Behaviours.ConfidenceTarget;
	}
	if (CompanyDescription->AI_Behaviours.DeclareWarConfidence)
	{
		DeclareWarConfidence = CompanyDescription->AI_Behaviours.DeclareWarConfidence;
	}
	if (CompanyDescription->AI_Behaviours.PayTributeConfidence)
	{
		PayTributeConfidence = CompanyDescription->AI_Behaviours.PayTributeConfidence;
	}

	if (CompanyDescription->AI_Behaviours.PacifismIncrementRate)
	{
		PacifismIncrementRate = CompanyDescription->AI_Behaviours.PacifismIncrementRate;
	}

	if (CompanyDescription->AI_Behaviours.PacifismAfterTribute)
	{
		PacifismAfterTribute = CompanyDescription->AI_Behaviours.PacifismAfterTribute;
	}	

	if (CompanyDescription->AI_Behaviours.PacifismDecrementRate)
	{
		PacifismDecrementRate = CompanyDescription->AI_Behaviours.PacifismDecrementRate;
	}

	if (CompanyDescription->AI_Behaviours.DailyProductionCostSensitivityMilitary)
	{
		DailyProductionCostSensitivityMilitary = CompanyDescription->AI_Behaviours.DailyProductionCostSensitivityMilitary;
	}
	if (CompanyDescription->AI_Behaviours.DailyProductionCostSensitivityEconomic)
	{
		DailyProductionCostSensitivityEconomic = CompanyDescription->AI_Behaviours.DailyProductionCostSensitivityEconomic;
	}

	if (CompanyDescription->AI_Behaviours.CostSafetyMarginMilitaryShip)
	{
		CostSafetyMarginMilitaryShip = CompanyDescription->AI_Behaviours.CostSafetyMarginMilitaryShip;
	}
	if (CompanyDescription->AI_Behaviours.CostSafetyMarginTradeShip)
	{
		CostSafetyMarginTradeShip = CompanyDescription->AI_Behaviours.CostSafetyMarginTradeShip;
	}
	if (CompanyDescription->AI_Behaviours.CostSafetyMarginStation)
	{
		CostSafetyMarginStation = CompanyDescription->AI_Behaviours.CostSafetyMarginStation;
	}

	if (CompanyDescription->AI_Behaviours.BuildLTradeOnlyTreshhold)
	{
		BuildLTradeOnlyTreshhold = CompanyDescription->AI_Behaviours.BuildLTradeOnlyTreshhold;
	}
	if (CompanyDescription->AI_Behaviours.BuildLMilitaryOnlyTreshhold)
	{
		BuildLMilitaryOnlyTreshhold = CompanyDescription->AI_Behaviours.BuildLMilitaryOnlyTreshhold;
	}

	if (CompanyDescription->AI_Behaviours.BuildMilitaryDiversity)
	{
		BuildMilitaryDiversity = CompanyDescription->AI_Behaviours.BuildMilitaryDiversity;
	}
	if (CompanyDescription->AI_Behaviours.BuildTradeDiversity)
	{
		BuildTradeDiversity = CompanyDescription->AI_Behaviours.BuildTradeDiversity;
	}

	if (CompanyDescription->AI_Behaviours.BuildMilitaryDiversitySize)
	{
		BuildMilitaryDiversitySize = CompanyDescription->AI_Behaviours.BuildMilitaryDiversitySize;
	}
	if (CompanyDescription->AI_Behaviours.BuildTradeDiversitySize)
	{
		BuildTradeDiversitySize = CompanyDescription->AI_Behaviours.BuildTradeDiversitySize;
	}
	if (CompanyDescription->AI_Behaviours.BuildMilitaryDiversitySizeBase)
	{
		BuildMilitaryDiversitySizeBase = CompanyDescription->AI_Behaviours.BuildMilitaryDiversitySizeBase;
	}
	if (CompanyDescription->AI_Behaviours.BuildTradeDiversitySizeBase)
	{
		BuildTradeDiversitySizeBase = CompanyDescription->AI_Behaviours.BuildTradeDiversitySizeBase;
	}

	if (CompanyDescription->AI_Behaviours.BuildEfficientMilitaryChance)
	{
		BuildEfficientMilitaryChance = CompanyDescription->AI_Behaviours.BuildEfficientMilitaryChance;
	}
	if (CompanyDescription->AI_Behaviours.BuildEfficientTradeChance)
	{
		BuildEfficientTradeChance = CompanyDescription->AI_Behaviours.BuildEfficientTradeChance;
	}
	if (CompanyDescription->AI_Behaviours.BuildEfficientMilitaryChanceSmall)
	{
		BuildEfficientMilitaryChanceSmall = CompanyDescription->AI_Behaviours.BuildEfficientMilitaryChanceSmall;
	}
	if (CompanyDescription->AI_Behaviours.BuildEfficientTradeChanceSmall)
	{
		BuildEfficientTradeChanceSmall = CompanyDescription->AI_Behaviours.BuildEfficientTradeChanceSmall;
	}

	if (CompanyDescription->AI_Behaviours.UpgradeMilitarySalvagerSRatio)
	{
		UpgradeMilitarySalvagerSRatio = CompanyDescription->AI_Behaviours.UpgradeMilitarySalvagerSRatio;
	}
	if (CompanyDescription->AI_Behaviours.UpgradeMilitarySalvagerLRatio)
	{
		UpgradeMilitarySalvagerLRatio = CompanyDescription->AI_Behaviours.UpgradeMilitarySalvagerLRatio;
	}

	if (CompanyDescription->AI_Behaviours.BuildStationWorthMultiplier)
	{
		BuildStationWorthMultiplier = CompanyDescription->AI_Behaviours.BuildStationWorthMultiplier;
	}
	if (CompanyDescription->AI_Behaviours.Station_Shipyard_Maximum)
	{
		Station_Shipyard_Maximum = CompanyDescription->AI_Behaviours.Station_Shipyard_Maximum;
	}
	if (CompanyDescription->AI_Behaviours.Scrap_Minimum_Ships)
	{
		Station_Shipyard_Maximum = CompanyDescription->AI_Behaviours.Scrap_Minimum_Ships;
	}
	if (CompanyDescription->AI_Behaviours.Scrap_Min_S_Cargo)
	{
		Station_Shipyard_Maximum = CompanyDescription->AI_Behaviours.Scrap_Min_S_Cargo;
	}
	if (CompanyDescription->AI_Behaviours.Scrap_Min_S_Military)
	{
		Station_Shipyard_Maximum = CompanyDescription->AI_Behaviours.Scrap_Min_S_Military;
	}
	if (CompanyDescription->AI_Behaviours.BuildDroneCombatWorth)
	{
		Station_Shipyard_Maximum = CompanyDescription->AI_Behaviours.BuildDroneCombatWorth;
	}
}

void UFlareAIBehavior::SetResourceAffilities(float Value)
{
	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		SetResourceAffility(Resource, Value);
	}
}

void UFlareAIBehavior::SetResourceAffility(FFlareResourceDescription* Resource, float Value)
{
	if(ResourceAffilities.Contains(Resource)){
		ResourceAffilities[Resource] = Value;
	}
	else
	{
		ResourceAffilities.Add(Resource, Value);
	}
}

void UFlareAIBehavior::SetSectorAffilities(float Value)
{
	for(int32 SectorIndex = 0; SectorIndex < Game->GetGameWorld()->GetSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Game->GetGameWorld()->GetSectors()[SectorIndex];
		SetSectorAffility(Sector, Value);
	}
}

void UFlareAIBehavior::SetSectorAffility(UFlareSimulatedSector* Sector, float Value)
{
	if(SectorAffilities.Contains(Sector)){
		SectorAffilities[Sector] = Value;
	}
	else
	{
		SectorAffilities.Add(Sector, Value);
	}
}

void UFlareAIBehavior::SetSectorAffilitiesByMoon(FFlareCelestialBody *CelestialBody, float Value)
{
	for(int32 SectorIndex = 0; SectorIndex < Game->GetGameWorld()->GetSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Game->GetGameWorld()->GetSectors()[SectorIndex];

		if(Sector->GetOrbitParameters()->CelestialBodyIdentifier == CelestialBody->Identifier)
		{
			SetSectorAffility(Sector, Value);
		}
	}
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

float UFlareAIBehavior::GetBudgetWeight(EFlareBudget::Type Budget)
{
	bool IsAtWar = Company->AtWar();
		switch (Budget) {
	case EFlareBudget::Military:
		return (IsAtWar ? BudgetWarMilitaryWeight : BudgetMilitaryWeight);
	break;
	case EFlareBudget::Trade:
		return (IsAtWar ? BudgetWarTradeWeight : BudgetTradeWeight);
	break;
	case EFlareBudget::Station:
		return (IsAtWar ? BudgetWarStationWeight : BudgetStationWeight);
	break;
	case EFlareBudget::Technology:
		return (IsAtWar ? BudgetWarTechnologyWeight : BudgetTechnologyWeight);
	break;
	default:
		break;
	}
	return 0;
}

float UFlareAIBehavior::GetSectorAffility(UFlareSimulatedSector* Sector)
{
	if(SectorAffilities.Contains(Sector))
	{
		return SectorAffilities[Sector];
	}
	return 1.f;
}

float UFlareAIBehavior::GetResourceAffility(FFlareResourceDescription* Resource)
{
	if(ResourceAffilities.Contains(Resource))
	{
		return ResourceAffilities[Resource];
	}
	return 1.f;
}

float UFlareAIBehavior::GetAttackThreshold()
{
	return AttackThreshold + Company->GetAI()->GetData()->Caution;
}

#undef LOCTEXT_NAMESPACE
