
#include "FlareWorld.h"
#include "../Flare.h"

#include "../Data/FlareSpacecraftCatalog.h"
#include "../Data/FlareSectorCatalogEntry.h"

#include "../Economy/FlareFactory.h"

#include "FlareGame.h"
#include "FlareGameTools.h"
#include "FlareScenarioTools.h"
#include "FlareSector.h"
#include "FlareTravel.h"
#include "FlareFleet.h"
#include "FlareBattle.h"
#include "AI/FlareAITradeHelper.h"

#include "../Quests/FlareQuest.h"
#include "../Quests/FlareQuestCondition.h"

#include "../Player/FlarePlayerController.h"
#include "../Player/FlareMenuManager.h"

#define LOCTEXT_NAMESPACE "FlareWorld"

/*----------------------------------------------------
    Constructor
----------------------------------------------------*/

UFlareWorld::UFlareWorld(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareWorld::Load(const FFlareWorldSave& Data)
{
	FLOG("UFlareWorld::Load");
	Game = Cast<AFlareGame>(GetOuter());
	WorldData = Data;

	// Init planetarium
	Planetarium = NewObject<UFlareSimulatedPlanetarium>(this, UFlareSimulatedPlanetarium::StaticClass());
	Planetarium->Load();

	// Load all companies
    for (int32 i = 0; i < WorldData.CompanyData.Num(); i++)
    {
		LoadCompany(WorldData.CompanyData[i]);
    }

	// Load sectors
	TArray<UFlareSectorCatalogEntry*> SectorList = Game->GetSectorCatalog();
	for (int32 SectorIndex = 0; SectorIndex < SectorList.Num(); SectorIndex++)
	{
		const FFlareSectorDescription* SectorDescription = &SectorList[SectorIndex]->Data;

		// Find save if exist
		FFlareSectorSave* SectorSave = NULL;
		for (int32 i = 0; i < WorldData.SectorData.Num(); i++)
		{
			if (WorldData.SectorData[i].Identifier == SectorDescription->Identifier)
			{
				// Old save found
				SectorSave = &WorldData.SectorData[i];
				break;
			}
		}

		FFlareSectorSave NewSectorData;
		if (!SectorSave)
		{
			// No save, init new sector
			NewSectorData.GivenName = FText();
			NewSectorData.Identifier = SectorDescription->Identifier;
			NewSectorData.LocalTime = 0;
			NewSectorData.IsTravelSector = false;

			// Init population
			NewSectorData.PeopleData.Population = 0;
			NewSectorData.PeopleData.BirthPoint = 0;
			NewSectorData.PeopleData.DeathPoint = 0;
			NewSectorData.PeopleData.FoodStock = 0;
			NewSectorData.PeopleData.FuelStock = 0;
			NewSectorData.PeopleData.ToolStock = 0;
			NewSectorData.PeopleData.TechStock = 0;
			NewSectorData.PeopleData.FoodConsumption = 0;
			NewSectorData.PeopleData.FuelConsumption = 0;
			NewSectorData.PeopleData.ToolConsumption = 0;
			NewSectorData.PeopleData.TechConsumption = 0;
			NewSectorData.PeopleData.HappinessPoint = 0;
			NewSectorData.PeopleData.HungerPoint = 0;
			NewSectorData.PeopleData.Money = 0;
			NewSectorData.PeopleData.Dept = 0;


			SectorSave = &NewSectorData;
		}

		FFlareSectorOrbitParameters OrbitParameters;
		OrbitParameters.CelestialBodyIdentifier = SectorDescription->CelestialBodyIdentifier;
		OrbitParameters.Altitude = SectorDescription->Altitude;
		OrbitParameters.Phase = SectorDescription->Phase;

		LoadSector(SectorDescription, *SectorSave, OrbitParameters);
	}

	UpdateStorageLocks();

	// Load all travels
	for (int32 i = 0; i < WorldData.TravelData.Num(); i++)
	{
		LoadTravel(WorldData.TravelData[i],NULL);
	}

	WorldMoneyReferenceInit = false;
}

FFlareWorldGameEventSave* UFlareWorld::GetGlobalEvent(FName EventSearch)
{
	for (int EventIndex = 0; EventIndex < WorldData.GlobalEvents.Num(); EventIndex++)
	{
		FFlareWorldGameEventSave* CheckingEvent = &WorldData.GlobalEvents[EventIndex];
		if (CheckingEvent->EventName == EventSearch)
		{
			return CheckingEvent;
		}
	}

	FFlareWorldGameEventSave NewEvent;
	NewEvent.EventName = EventSearch;
	NewEvent.EventDate = WorldData.Date - 1;
	NewEvent.EventDateEnd = WorldData.Date - 1;
	WorldData.GlobalEvents.Add(NewEvent);
	return &WorldData.GlobalEvents.Last();
}

void UFlareWorld::PostLoad()
{
	for (int i = 0; i < Companies.Num(); i++)
	{
		Companies[i]->PostLoad();
	}
}

UFlareCompany* UFlareWorld::LoadCompany(const FFlareCompanySave& CompanyData)
{
    UFlareCompany* Company = NULL;

    // Create the new company
	Company = NewObject<UFlareCompany>(this, UFlareCompany::StaticClass(), CompanyData.Identifier);
    Company->Load(CompanyData);
    Companies.AddUnique(Company);

	//FLOGV("UFlareWorld::LoadCompany : loaded '%s'", *Company->GetCompanyName().ToString());

    return Company;
}


UFlareSimulatedSector* UFlareWorld::LoadSector(const FFlareSectorDescription* Description, const FFlareSectorSave& SectorData, const FFlareSectorOrbitParameters& OrbitParameters)
{
	UFlareSimulatedSector* Sector = NULL;

	// Create the new sector
	Sector = NewObject<UFlareSimulatedSector>(this, UFlareSimulatedSector::StaticClass(), SectorData.Identifier);
	Sector->Load(Description, SectorData, OrbitParameters);
	Sectors.AddUnique(Sector);

	//FLOGV("UFlareWorld::LoadSector : loaded '%s'", *Sector->GetSectorName().ToString());

	return Sector;
}


UFlareTravel* UFlareWorld::LoadTravel(const FFlareTravelSave& TravelData, UFlareFleet* Fleet)
{
	UFlareTravel* Travel = NULL;

	// Create the new travel
	Travel = NewObject<UFlareTravel>(this, UFlareTravel::StaticClass());
	if (Travel->Load(TravelData, Fleet))
	{
		Travels.AddUnique(Travel);
	}

	//FLOGV("UFlareWorld::LoadTravel : loaded travel for fleet '%s'", *Travel->GetFleet()->GetFleetName().ToString());

	return Travel;
}


FFlareWorldSave* UFlareWorld::Save()
{
	WorldData.CompanyData.Empty();
	WorldData.SectorData.Empty();
	WorldData.TravelData.Empty();

	WorldData.CompanyData.Reserve(Companies.Num());
	WorldData.SectorData.Reserve(Sectors.Num());
	WorldData.TravelData.Reserve(Travels.Num());

	// Companies
	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];

		//FLOGV("UFlareWorld::Save : saving company ('%s')", *Company->GetName());
		FFlareCompanySave* TempData = Company->Save();
		WorldData.CompanyData.Add(*TempData);
	}

	// Sectors
	for (int i = 0; i < Sectors.Num(); i++)
	{
		UFlareSimulatedSector* Sector = Sectors[i];
		//FLOGV("UFlareWorld::Save : saving sector ('%s')", *Sector->GetName());

		WorldData.SectorData.Add(*Sector->Save());
	}

	// Travels
	for (int i = 0; i < Travels.Num(); i++)
	{
		UFlareTravel* Travel = Travels[i];

		//FLOGV("UFlareWorld::Save : saving travel for ('%s')", *Travel->GetFleet()->GetFleetName().ToString());
		FFlareTravelSave* TempData = Travel->Save();
		WorldData.TravelData.Add(*TempData);
	}

	return &WorldData;
}


void UFlareWorld::CompanyMutualAssistance()
{
	UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();

	// Base revenue between company. 1 per 100000 is share between all companies
	uint32 SharingCompanyCount = 0;
	int64 SharedPool = 0;

	for (int CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
	{
		UFlareCompany* Company = Companies[CompanyIndex];
		if (Company != PlayerCompany)
		{
			if (Company->GetMoney() > 0)
			{
				// 0.02 % given to others
				int64 MoneyToTake;
				if (Company == GetGame()->GetScenarioTools()->AxisSupplies)
				{
					MoneyToTake = Company->GetMoney() / 350;
				}
				else
				{
					MoneyToTake = Company->GetMoney() / 500;
				}

				if (MoneyToTake > 0)
				{
					if (Company->TakeMoney(MoneyToTake, false, FFlareTransactionLogEntry::LogMutualAssistance()))
					{
						SharedPool += MoneyToTake;
					}
				}
			}
			SharingCompanyCount++;
		}
	}

	// Share poll
	int64 PoolPart = SharedPool / SharingCompanyCount;
	int64 PoolBonus = SharedPool % SharingCompanyCount; // The bonus is given to a random company

	int32 BonusIndex;
	while (true)
	{
		BonusIndex = FMath::RandRange(0, SharingCompanyCount - 1);
		UFlareCompany* Company = Companies[BonusIndex];
		if (!Company)
		{
			break;
		}
		if (Company != GetGame()->GetScenarioTools()->AxisSupplies && Company != PlayerCompany)
		{
			Company->GiveMoney(PoolBonus, FFlareTransactionLogEntry::LogMutualAssistance());
			break;
		}
	}

	FLOGV("Share part amount is : %d", PoolPart/100);
	int32 SharingCompanyIndex = 0;
	for (int CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
	{
		UFlareCompany* Company = Companies[CompanyIndex];
		if (Company != PlayerCompany)
		{
			Company->GiveMoney(PoolPart, FFlareTransactionLogEntry::LogMutualAssistance());
			SharingCompanyIndex++;
		}
	}
}

bool UFlareWorld::CheckIntegrity()
{
	bool Integrity = true;
	for (int i = 0; i < Sectors.Num(); i++)
	{
		UFlareSimulatedSector* Sector = Sectors[i];

		for (int32 StationIndex = 0 ; StationIndex < Sector->GetSectorStations().Num(); StationIndex++)
		{
			UFlareSimulatedSpacecraft* Station = Sector->GetSectorStations()[StationIndex];
			if (!Station->IsStation())
			{
				FLOGV("WARNING : World integrity failure : station %s in %s is not a station", *Station->GetImmatriculation().ToString(), *Sector->GetSectorName().ToString());
				Integrity = false;
			}
		}
	}

	// Check money integrity
	if (! WorldMoneyReferenceInit)
	{
		WorldMoneyReference = GetWorldMoney();
		WorldMoneyReferenceInit = true;
	}
	else
	{
		int64 WorldMoney = GetWorldMoney();

		if (WorldMoneyReference != WorldMoney)
		{
			FLOGV("WARNING : World integrity failure : world contain %lld credits but reference is %lld", WorldMoney, WorldMoneyReference)
			Integrity = false;
		}
	}

	//  Check companyintegrity
	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];
		if (Company->GetCompanySpacecrafts().Num() != Company->GetCompanyShips().Num() + Company->GetCompanyStations().Num())
		{
			FLOGV("WARNING : World integrity failure : %s have %d spacecraft but %d ships and %s stations", *Company->GetCompanyName().ToString(),
				  Company->GetCompanySpacecrafts().Num(),
				  Company->GetCompanyShips().Num(),
				  Company->GetCompanyStations().Num());
			Integrity = false;
		}

		// Ships
		for (int32 ShipIndex = 0 ; ShipIndex < Company->GetCompanyShips().Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = Company->GetCompanyShips()[ShipIndex];

			UFlareSimulatedSector* ShipSector = Ship->GetCurrentSector();
			if(IsValid(ShipSector) && ShipSector->IsValidLowLevel())
			{
				if (Ship->GetCurrentFleet() == NULL)
				{
					FLOGV("WARNING : World integrity failure : %s in %s is in no fleet",
						*Ship->GetImmatriculation().ToString(),
						*ShipSector->GetSectorName().ToString());
					Integrity = false;
				}

				if(!ShipSector->GetSectorShips().Contains(Ship))
				{
					FLOGV("WARNING : World integrity failure : %s in %s but not in sector ship list",
						  *Ship->GetImmatriculation().ToString(),
						  *ShipSector->GetSectorName().ToString());
					Integrity = false;
				}
			}
			else
			{
				if (Ship->GetCurrentFleet() == NULL)
				{
					FLOGV("WARNING : World integrity failure : %s not in sector but in no fleet",
						  *Ship->GetImmatriculation().ToString());
					Integrity = false;

				}
				else if(Ship->GetCurrentFleet()->GetCurrentTravel() == NULL)
				{
					FLOGV("WARNING : World integrity failure : %s in fleet %s but not in sector and not in travel",
						  *Ship->GetImmatriculation().ToString(),
						  *Ship->GetCurrentFleet()->GetFleetName().ToString());
					if(Ship->GetCurrentFleet()->GetCurrentSector() != NULL)
					{
						FLOGV("  - %s in %s",
							  *Ship->GetCurrentFleet()->GetFleetName().ToString(),
							  *Ship->GetCurrentFleet()->GetCurrentSector()->GetSectorName().ToString());
						if (Ship->GetCurrentFleet()->GetCurrentSector()->GetSectorSpacecrafts().Contains(Ship))
						{
							FLOGV("  - %s contains the ship in its list",
								  *Ship->GetCurrentFleet()->GetCurrentSector()->GetSectorName().ToString());
						}
						else
						{
							FLOGV("  - %s don't contains the ship in its list",
								  *Ship->GetCurrentFleet()->GetCurrentSector()->GetSectorName().ToString());
						}

						Ship->GetCurrentFleet()->GetCurrentSector()->AddFleet(Ship->GetCurrentFleet());
						FLOGV("Fix integrity : set %s to %s",
							   *Ship->GetImmatriculation().ToString(),
							  *Ship->GetCurrentFleet()->GetCurrentSector()->GetSectorName().ToString());
					}
					else
					{
						FLOGV("  - %s in no sector", *Ship->GetCurrentFleet()->GetFleetName().ToString());
						if (Ship->GetCompany()->GetKnownSectors().Num() > 0)
						{
							Ship->GetCompany()->GetKnownSectors()[0]->AddFleet(Ship->GetCurrentFleet());
							FLOGV("Fix integrity : set %s to %s",
							   *Ship->GetImmatriculation().ToString(),
							  *Ship->GetCurrentSector()->GetSectorName().ToString());
						}
					}
					Integrity = false;
				}
			}
		}

		// Fleets
		for (int32 FleetIndex = 0 ; FleetIndex < Company->GetCompanyFleets().Num(); FleetIndex++)
		{
			UFlareFleet* Fleet = Company->GetCompanyFleets()[FleetIndex];
			bool MarkedForDisband = false;
			if(Fleet->GetShipCount() == 0)
			{
				FLOGV("WARNING : World integrity failure : %s fleet %s is empty",
					  *Company->GetCompanyName().ToString(),
					  *Fleet->GetFleetName().ToString());
				Integrity = false;
				MarkedForDisband = true;
			}

			if(Fleet->GetCurrentSector() == NULL )
			{
				FLOGV("WARNING : World integrity failure : %s fleet %s is not in a sector",
					  *Company->GetCompanyName().ToString(),
					  *Fleet->GetFleetName().ToString());
				Integrity = false;
			}
			else if(Fleet->GetCurrentSector()->IsTravelSector() && Fleet->GetCurrentTravel() == NULL)
			{
				FLOGV("WARNING : World integrity failure : %s fleet %s is in a travel sector and not in travel",
					  *Company->GetCompanyName().ToString(),
					  *Fleet->GetFleetName().ToString());
				Integrity = false;
			}
			if (MarkedForDisband)
			{
				Fleet->Disband();
			}
		}
	}
	return Integrity;
}

inline static bool CompanyValueComparator(const UFlareCompany& ip1, const UFlareCompany& ip2)
{
	return ip1.GetCompanyValue().TotalValue < ip2.GetCompanyValue().TotalValue;
}

inline static bool CompanyTotalCombatValueComparator(const UFlareCompany& ip1, const UFlareCompany& ip2)
{
	return ip1.GetCompanyValue().ArmyTotalCombatPoints < ip2.GetCompanyValue().ArmyTotalCombatPoints;
}

//PrimarySimulate()
void UFlareWorld::Simulate()
{
	RunningPrimarySimulate = true;
	double StartTs = FPlatformTime::Seconds();
	UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();

	/**
	 *  End previous day
	 */
	FLOGV("** Simulate day %d", WorldData.Date);

	FLOG("* Simulate > Player autotrade");
	AITradeHelper::CompanyAutoTrade(PlayerCompany);

	FLOG("* Simulate > Battles");
	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Sectors[SectorIndex];
		Sector->Cleartemporarycaches();

		// Check if battle
		bool HasBattle = false;
		for (FFlareMeteoriteSave& Meteorite : Sector->GetData()->MeteoriteData)
		{
			if (Meteorite.DaysBeforeImpact == 0)
			{
				HasBattle = true;
				break;
			}
		}

		if (!HasBattle)
		{
			for (int CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
			{
				UFlareCompany* Company = Companies[CompanyIndex];

				FFlareSectorBattleState BattleState = Sector->UpdateSectorBattleState(Company);

				if (!BattleState.WantFight())
				{
					// Don't want fight
					continue;
				}

				FLOGV("%s want fight in %s", *Company->GetCompanyName().ToString(),
					*Sector->GetSectorName().ToString());

				HasBattle = true;
			}
		}
		else
		{
			for (int CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
			{
				UFlareCompany* Company = Companies[CompanyIndex];
				FFlareSectorBattleState BattleState = Sector->UpdateSectorBattleState(Company);
			}
		}

		if (HasBattle)
		{
			if (!BattleSimulation || BattleSimulation == nullptr)
			{
				BattleSimulation = NewObject<UFlareBattle>(this, UFlareBattle::StaticClass());
			}
			BattleSimulation->Load(Sector);
			BattleSimulation->Simulate();
		}

		// Forcibly remove destroyed "active" spacecraft
		for (int32 SpacecraftIndex = 0; SpacecraftIndex < PendingActiveSpacecraftDeletions.Num(); SpacecraftIndex++)
		{
			AFlareSpacecraft* Spacecraft = PendingActiveSpacecraftDeletions[SpacecraftIndex];
			if (IsValid(Spacecraft))
			{
				Spacecraft->Destroy();
			}
		}

		PendingActiveSpacecraftDeletions.Empty();

		for (int32 SpacecraftIndex = 0 ; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorSpacecrafts()[SpacecraftIndex];

			if(!Spacecraft->GetDamageSystem()->IsAlive())
			{
				if (Spacecraft->GetActive())
				{
					PendingActiveSpacecraftDeletions.Add(Spacecraft->GetActive());
				}

				Spacecraft->GetCompany()->DestroySpacecraft(Spacecraft);
			}
			else if(Spacecraft->GetActive())
			{
				Spacecraft->GetActive()->SimulateDayRun();
			}
		}
	}

	FLOG("* Simulate > AI");

	HasTotalWorldCombatPointCache = false;

	// AI. Merged trading
	AITradeNeeds Needs;
	AITradeNeeds MaintenanceNeeds;
	AITradeNeeds StorageNeeds;
	AITradeSources Sources(this);
	AITradeSources MaintenanceSources(this);
	AITradeIdleShips IdleShips(this);

	AITradeHelper::GenerateTradingNeeds(Needs, MaintenanceNeeds, StorageNeeds, this);
	AITradeHelper::GenerateTradingSources(Sources, MaintenanceSources, this);
	AITradeHelper::GenerateIdleShips(IdleShips, this);

	int32 MaxCombatPoint = 0;
	UFlareCompany* MaxCombatPointCompany = NULL;

	TotalCompaniesMoney = 0;

	AICompaniesMoney CompaniesMoney;
	CompaniesMoney.CompaniesMoney.Reserve(GetCompanies().Num());
	for(UFlareCompany* Company: GetCompanies())
	{
		CompaniesMoney.CompaniesMoney.Add(Company, Company->GetMoney())	;
		TotalCompaniesMoney += Company->GetMoney();

		struct CompanyValue Value = Company->GetCompanyValue();
		if (MaxCombatPoint < Value.ArmyCurrentCombatPoints)
		{
			MaxCombatPoint = Value.ArmyCurrentCombatPoints;
			MaxCombatPointCompany = Company;
		}
	}

#if DEBUG_NEW_AI_TRADING
	FLOG("Initial trading stat");
	Needs.Print();
	MaintenanceNeeds.Print();
	StorageNeeds.Print();
	Sources.Print();
	MaintenanceSources.Print();
	IdleShips.Print();

	for(auto& CompanyMoney : CompaniesMoney.CompaniesMoney)
	{
		FLOGV("- %s start with %lld", *CompanyMoney.Key->GetCompanyName().ToString(), CompanyMoney.Value);
	}

#endif

	AITradeHelper::ComputeGlobalTrading(this, MaintenanceNeeds, Sources, MaintenanceSources, IdleShips, CompaniesMoney);
	AITradeHelper::ComputeGlobalTrading(this, Needs, Sources, MaintenanceSources, IdleShips, CompaniesMoney);

	int32 TotalReservedResources = 0;
	for(UFlareCompany* Company: Companies)
	{
		UFlareCompanyAI* AI = Company->GetAI();
		if (AI)
		{
			AI->UpdateIdleShipsStats(IdleShips);
			TotalReservedResources += AI->GetReservedResources();
		}
	}

	AITradeHelper::ComputeGlobalTrading(this, StorageNeeds, Sources, MaintenanceSources, IdleShips, CompaniesMoney);

#if DEBUG_NEW_AI_TRADING
	FLOG("Final trading stat");
	Needs.Print();
	Sources.Print();
	MaintenanceSources.Print();
	IdleShips.Print();
#endif

	// AI. Play them in random order

	FLOG("* Simulate > Global Events");
	bool GlobalWar = false;
	float MaxCombatPointRatio = float(MaxCombatPoint) / GetGame()->GetGameWorld()->GetTotalWorldCombatPoint();

	//FLOGV("MaxCombatPointRatio %f", MaxCombatPointRatio);
	//FLOGV("MaxCombatPoint %d", MaxCombatPoint);
	//FLOGV("TotalWorldCombatPoint %d", TotalWorldCombatPoint);

	// If a company is not far from half world power, a global alliance is formed
	int32 GameDifficulty = -1;
	GameDifficulty = Game->GetPC()->GetPlayerData()->DifficultyId;

	if (MaxCombatPointRatio > 0.3 && MaxCombatPoint > 500)
	{
		float Chance = 1.00f;
		int32 MinimumDays = 0;

		if (MaxCombatPointCompany == PlayerCompany)
		{
			//Chance every day to initiate a global war against the player
			switch (GameDifficulty)
			{
			case -1: // Easy
				Chance = 0.00f;
				MinimumDays = 365;
				break;
			case 0: // Normal
				Chance = 0.01f;
				MinimumDays = 335;
				break;
			case 1: // Hard
				Chance = 0.01f;
				MinimumDays = 305;
				break;
			case 2: // Very Hard
				Chance = 0.02f;
				MinimumDays = 245;
				break;
			case 3: // Expert
				Chance = 0.05f;
				MinimumDays = 185;
				break;
			case 4: // Unfair
				Chance = 0.10f;
				MinimumDays = 95;
				break;
			}
		}
		else
		{
			//Chance every day to initiate a global war against an AI player
			switch (GameDifficulty)
			{
			case -1: // Easy
				Chance = 0.50f;
				MinimumDays = 60;
				break;
			case 0: // Normal
				Chance = 0.25f;
				MinimumDays = 120;
				break;
			case 1: // Hard
				Chance = 0.125f;
				MinimumDays = 180;
				break;
			case 2: // Very Hard
				Chance = 0.0625f;
				MinimumDays = 240;
				break;
			case 3: // Expert
				Chance = 0.03125f;
				MinimumDays = 300;
				break;
			case 4: // Unfair
				Chance = 0.015625f;
				MinimumDays = 360;
				break;
			}
		}

		FFlareWorldGameEventSave* GlobalWarEvent = GetGlobalEvent("Global War");

		if(WorldData.Date > GlobalWarEvent->EventDateEnd+MinimumDays)
		{
			bool HasChance = FMath::FRand() < Chance;
			if (HasChance)
			{
				for (UFlareCompany* OtherCompany : GetCompanies())
				{
					if (OtherCompany == GetGame()->GetPC()->GetCompany())
					{
						continue;
					}

					if (MaxCombatPointCompany != OtherCompany && OtherCompany->GetCompanyValue().ArmyCurrentCombatPoints > 0)
					{
						if (OtherCompany->GetWarState(MaxCombatPointCompany) != EFlareHostility::Hostile)
						{
							GlobalWar = true;
						}

						OtherCompany->GetAI()->GetData()->Pacifism = 0;
						OtherCompany->SetHostilityTo(MaxCombatPointCompany, true, true);
					}
					else
					{
						OtherCompany->SetHostilityTo(MaxCombatPointCompany, false, true);
					}
				}

				// Notify the player that this is happening
				if (GlobalWar)
				{
					GlobalWarEvent->EventDate = WorldData.Date;
					GlobalWarEvent->EventDateEnd = WorldData.Date;
					GetGame()->GetPC()->Notify(LOCTEXT("GlobalWar", "Global War"),
						FText::Format(LOCTEXT("AIStartGlobalWarInfo", "All companies are now allied to stop the militarization of {0}."),
						MaxCombatPointCompany->GetCompanyName()),
						FName("global-war"),
						EFlareNotification::NT_Military,
						false,
						EFlareMenu::MENU_Leaderboard);
				}
			}
		}
	}

	FFlareWorldGameEventSave* GlobalMeteorStormEvent = GetGlobalEvent("Meteor Storm");
	bool MeteorStormInProgress = (WorldData.Date >= GlobalMeteorStormEvent->EventDate && WorldData.Date < GlobalMeteorStormEvent->EventDateEnd);
	if (!MeteorStormInProgress)
	{
		if (WorldData.Date > GlobalMeteorStormEvent->EventDateEnd + 365)
		{
			float MeteorStormProbability = 0.001;
			bool HasChance = FMath::FRand() < MeteorStormProbability;
			if (HasChance)
			{
				GlobalMeteorStormEvent->EventDate = WorldData.Date;
				GlobalMeteorStormEvent->EventDateEnd = WorldData.Date + FMath::RandRange(7, 21);

				GetGame()->GetPC()->Notify(LOCTEXT("MeteorStorm", "Meteoroid Activity"),
				FText::Format(LOCTEXT("MeteorStormInfo", "Warning: Nema is entering a period of heightened meteoroid activity. This period of activity is expected to pass in {0} days."),
				(GlobalMeteorStormEvent->EventDateEnd - GlobalMeteorStormEvent->EventDate) - 1),
				FName("MeteorStorm"),
				EFlareNotification::NT_Info,
				false);
				MeteorStormInProgress = true;
			}
		}
	}

	SortedCompanyValues = GetCompanies();
	SortedCompanyCombatValues = GetCompanies();

	SortedCompanyValues.Sort(&CompanyValueComparator);
	SortedCompanyCombatValues.Sort(&CompanyTotalCombatValueComparator);

	TArray<UFlareCompany*> CompaniesToSimulateAI = Companies;
	while(CompaniesToSimulateAI.Num())
	{
		int32 Index = FMath::RandRange(0, CompaniesToSimulateAI.Num() - 1);
		CompaniesToSimulateAI[Index]->SimulateAI(GlobalWar, TotalReservedResources);
		CompaniesToSimulateAI.RemoveAtSwap(Index);
	}

	for (UFlareCompany* Company : GetCompanies())
	{
		Company->Simulate();
	}

	// Clear bombs, Process meteorites
	for (UFlareSimulatedSector* Sector : Sectors)
	{
		Sector->ClearBombs();
		Sector->ProcessMeteorites();
	}

	// GenerateMeteorites
	for (UFlareSimulatedSector* Sector :Sectors)
	{
		Sector->GenerateMeteorites(MeteorStormInProgress);
	}

	CompanyMutualAssistance();
	CheckIntegrity();

	/**
	 *  Begin day
	 */
	FLOG("* Simulate > New day");

	WorldData.Date++;

	// Write FS consumption stats
	for (UFlareSimulatedSector* Sector :Sectors)
	{
		Sector->UpdateFleetSupplyConsumptionStats();
	}

	// Count repairing fleet
	int32 PlayerRepairingFleet = 0;
	int32 PlayerRefillingFleet = 0;
	for (UFlareFleet* Fleet : GetGame()->GetPC()->GetCompany()->GetCompanyFleets())
	{
		if(Fleet->IsRepairing())
		{
			PlayerRepairingFleet++;
		}

		if(Fleet->IsRefilling())
		{
			PlayerRefillingFleet++;
		}
	}


	// End trade, intercept, repair and refill, operations
	for (int CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
	{
		UFlareCompany* Company = Companies[CompanyIndex];

		for (int32 SpacecraftIndex = 0; SpacecraftIndex < Company->GetCompanySpacecrafts().Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = Company->GetCompanySpacecrafts()[SpacecraftIndex];
			if (!Spacecraft->IsStation())
			{
				Spacecraft->SetTrading(false);
				Spacecraft->SetIntercepted(false);
			}
			Spacecraft->Repair();
			Spacecraft->Refill();
			Spacecraft->Stabilize();
		}
	}


	int32 PlayerRepairingFleetAfter = 0;
	int32 PlayerRefillingFleetAfter = 0;
	for (UFlareFleet* Fleet : GetGame()->GetPC()->GetCompany()->GetCompanyFleets())
	{
		if(Fleet->IsRepairing())
		{
			PlayerRepairingFleetAfter++;
		}

		if(Fleet->IsRefilling())
		{
			PlayerRefillingFleetAfter++;
		}
	}

	if (PlayerRepairingFleetAfter < PlayerRepairingFleet)
	{
		FText RepairText;

		if (GetGame()->GetPC()->GetCompany()->GetCompanyFleets().Num() == 1)
		{
			RepairText = LOCTEXT("YouFleetRepairFinish", "Your fleet repairs are finished");
		}
		else if (PlayerRepairingFleet - PlayerRepairingFleetAfter > 1)
		{
			RepairText = LOCTEXT("MultipleFleetRepairFinish", "Some fleet repairs are finished");
		}
		else
		{
			RepairText = LOCTEXT("OneFleetRepairFinish", "One fleet repair is finished");
		}

		FFlareMenuParameterData MenuData;
		GetGame()->GetPC()->Notify(LOCTEXT("FeetRepaired", "Repaired"),
			RepairText,
			FName("fleet-repaired"),
			EFlareNotification::NT_Military,
			false,
			EFlareMenu::MENU_Orbit,
			MenuData);
	}

	if (PlayerRefillingFleetAfter < PlayerRefillingFleet)
	{
		FText RefillText;

		if(GetGame()->GetPC()->GetCompany()->GetCompanyFleets().Num() == 1)
		{
			RefillText = LOCTEXT("YouFleetRefill", "Your fleet refillings are finished");
		}
		else if (PlayerRefillingFleet - PlayerRefillingFleetAfter > 1)
		{
			RefillText = LOCTEXT("MultipleFleetRefillFinish", "Some fleet refillings are finished");
		}
		else
		{
			RefillText = LOCTEXT("OneFleetRefillFinish", "One fleet refilling is finished");
		}

		FFlareMenuParameterData MenuData;
		GetGame()->GetPC()->Notify(LOCTEXT("FeetRefilled", "Refilled"),
			RefillText,
			FName("fleet-refilled"),
			EFlareNotification::NT_Military,
			false,
			EFlareMenu::MENU_Orbit,
			MenuData);
	}

	// Process captures
	ProcessShipCapture();
	ProcessStationCapture();

	// Factories
	FLOG("* Simulate > Factories");
	for (UFlareFactory* Factory: Factories)
	{
		if(Factory->IsShipyard())
		{
			Factory->GetParent()->UpdateShipyardProduction();
		}
	}

	for (int FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
	{
		Factories[FactoryIndex]->Simulate();
	}


	// Peoples
	FLOG("* Simulate > Peoples");
	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		Sectors[SectorIndex]->GetPeople()->Simulate();
	}


	FLOG("* Simulate > Trade routes");

	// Trade routes
	for (int CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
	{
		TArray<UFlareTradeRoute*>& TradeRoutes = Companies[CompanyIndex]->GetCompanyTradeRoutes();

		for (int RouteIndex = 0; RouteIndex < TradeRoutes.Num(); RouteIndex++)
		{
			TradeRoutes[RouteIndex]->Simulate();
		}
	}

	FLOG("* Simulate > Travels");

	// Undock and make move AI ships
	for (UFlareSimulatedSector* Sector : Sectors)
	{
		for(UFlareSimulatedSpacecraft* Ship : Sector->GetSectorShips())
		{
			if(Ship->GetCompany() != PlayerCompany)
			{
				// Undock
				Ship->ForceUndock();
				if (!Ship->GetShipMaster())
				{
					Ship->SetSpawnMode(EFlareSpawnMode::Travel);
				}
			}
		}
	}

	// Travels
	TArray<UFlareTravel*> TravelsToProcess = Travels;
	for (int TravelIndex = 0; TravelIndex < TravelsToProcess.Num(); TravelIndex++)
	{
		TravelsToProcess[TravelIndex]->Simulate();
	}
	
	FLOG("* Simulate > Prices");
	// Price variation.
	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		Sectors[SectorIndex]->SimulatePriceVariation();
	}

	// People money migration
	SimulatePeopleMoneyMigration();

	// Process events

	// Swap Prices.
	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		Sectors[SectorIndex]->SwapPrices();
	}
	
	// Update reserve ships
	UpdateReserveShips();
	
	// Update storage station reservation
	UpdateStorageLocks();

	// Player being attacked ?
	ProcessIncomingPlayerEnemy();

	// Lets AI check if in battle
	CheckAIBattleState();

	for (UFlareCompany* Company : Companies)
	{
		Company->InvalidateCompanyValueCache();
	}

	double EndTs = FPlatformTime::Seconds();
	FLOGV("** Simulate day %d done in %.6fs", WorldData.Date-1, EndTs- StartTs);

	Game->GetQuestManager()->OnNextDay();

	GameLog::DaySimulated(WorldData.Date);

	// Check recovery
	{
		// Check if it the last ship
		bool EmptyFleet = true;
		for(int ShipIndex = 0; ShipIndex < GetGame()->GetPC()->GetPlayerFleet()->GetShips().Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = GetGame()->GetPC()->GetPlayerFleet()->GetShips()[ShipIndex];
			if(Ship->GetDamageSystem()->IsAlive() && !Ship->GetDamageSystem()->IsUncontrollable())
			{
				EmptyFleet = false;
				break;
			}
		}

		// If last, activate recovery
		if (EmptyFleet)
		{
			GetGame()->GetPC()->GetMenuManager()->OpenMenu(EFlareMenu::MENU_GameOver);
		}
	}

	// Check player position in leaderboard
	bool IsFirst = true;
	bool IsLast = true;

	int64 PlayerValue = PlayerCompany->GetCompanyValue().TotalValue;

	for(UFlareCompany* Company: GetCompanies())
	{
		if (Company == PlayerCompany)
		{
			 continue;
		}
		int64 CompanyValue = Company->GetCompanyValue().TotalValue;

		if(CompanyValue > PlayerValue)
		{
			IsFirst = false;
		}
		else if(CompanyValue < PlayerValue)
		{
			IsLast = false;
		}
	}

	if(!IsLast)
	{
		GetGame()->GetPC()->SetAchievementProgression("ACHIEVEMENT_NOT_LAST", 1);
	}

	if(IsFirst)
	{
		GetGame()->GetPC()->SetAchievementProgression("ACHIEVEMENT_FIRST", 1);
	}

	// Check if all ship
	 int32 ShipTypeCount = GetGame()->GetSpacecraftCatalog()->ShipCatalog.Num();

	 TArray<FFlareSpacecraftDescription*> Descriptions;

	 for(UFlareSimulatedSpacecraft* Ship : GetGame()->GetPC()->GetCompany()->GetCompanyShips())
	 {
		 if(Ship->GetDamageSystem()->IsAlive())
		 {
			 Descriptions.AddUnique(Ship->GetDescription());
		 }
	 }

	 if(Descriptions.Num() == ShipTypeCount)
	 {
		 GetGame()->GetPC()->SetAchievementProgression("ACHIEVEMENT_ALL_SHIPS", 1);
	 }
	 RunningPrimarySimulate = false;

}

void UFlareWorld::UpdateReserveShips()
{
	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		Sectors[SectorIndex]->UpdateReserveShips();
	}
}

void UFlareWorld::CheckAIBattleState()
{
	for (UFlareCompany* Company : Companies)
	{
		Company->GetAI()->CheckBattleState();
	}
}

void UFlareWorld::UpdateStorageLocks()
{
	for (UFlareSimulatedSector* Sector : Sectors)
	{
		for(UFlareSimulatedSpacecraft* Station : Sector->GetSectorStations())
		{
			if(Station->HasCapability(EFlareSpacecraftCapability::Storage))
			{
				Station->LockResources();
			}
		}
	}
}

void UFlareWorld::ProcessShipCapture()
{
	TArray<UFlareSimulatedSpacecraft*> ShipToCapture;

	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Sectors[SectorIndex];

		for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorShips().Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorShips()[SpacecraftIndex];

			if(Spacecraft->IsHarpooned()) {
				// Capture the ship if the following condition is ok :
				// - The harpoon owner must be at war this the ship owner
				// - The harpoon owner must in won state : military presence only for him

				UFlareCompany* HarpoonOwner = Spacecraft->GetHarpoonCompany();


				FFlareSectorBattleState  HarpoonOwnerBattleState = Sector->GetSectorBattleState(HarpoonOwner);
				FFlareSectorBattleState  SpacecraftOwnerBattleState = Sector->GetSectorBattleState(Spacecraft->GetCompany());


				if(HarpoonOwner
						&& Spacecraft->IsHostile(HarpoonOwner)
						&& !HarpoonOwnerBattleState.HasDanger)
				{
					// If battle won state, this mean the Harpoon owner has at least one dangerous ship
					// This also mean that no company at war with this company has a military ship

					ShipToCapture.Add(Spacecraft);
					// Need to keep the harpoon for capture process
				}
				else if(!SpacecraftOwnerBattleState.HasDanger)
				{
//					Spacecraft->RemoveCapturePoint(HarpoonOwner->GetIdentifier(), Spacecraft->GetCapturePointThreshold());
					Spacecraft->SetHarpooned(NULL);
				}
			}
			else
			{
				Spacecraft->ResetCapture();
			}
		}
	}

	FLOGV("ShipToCapture %d", ShipToCapture.Num());


	// Capture consist on :
	// - Kill rotation and velocity
	// - respawn the ship at near location
	// - change its owner

	int32 GameDifficulty = -1;
	GameDifficulty = Game->GetPC()->GetPlayerData()->DifficultyId;

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < ShipToCapture.Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = ShipToCapture[SpacecraftIndex];

		UFlareCompany* HarpoonOwner = Spacecraft->GetHarpoonCompany();
		UFlareCompany* Owner = Spacecraft->GetCompany();
		UFlareSimulatedSector* Sector =  Spacecraft->GetCurrentSector();
		FVector SpawnLocation =  Spacecraft->GetData().Location;
		FRotator SpawnRotation =  Spacecraft->GetData().Rotation;
		FFlareSpacecraftSave Data = Spacecraft->GetData();
		FFlareSpacecraftDescription* ShipDescription = Spacecraft->GetDescription();


		float NegotiationRatio = 1.f;
		if (HarpoonOwner->IsTechnologyUnlocked("negociations"))
		{
			NegotiationRatio *= 1.5;
		}
		if (Spacecraft->GetCompany()->IsTechnologyUnlocked("negociations"))
		{
			NegotiationRatio *= 0.5;
		}

		int32 CompanyCapturePoint = Sector->GetCompanyCapturePoints(HarpoonOwner) * NegotiationRatio;
		if (!Spacecraft->TryCapture(HarpoonOwner, CompanyCapturePoint))
		{
			continue;
		}

		Spacecraft->GetCompany()->DestroySpacecraft(Spacecraft);
		UFlareSimulatedSpacecraft* NewShip = Sector->CreateSpacecraft(ShipDescription, HarpoonOwner, SpawnLocation, SpawnRotation, &Data);
		GetGame()->GetQuestManager()->OnSpacecraftCaptured(Spacecraft, NewShip);

		if (GetGame()->GetPC()->GetCompany() == HarpoonOwner)
		{
			int32 ReputationDrop = -20;
			int32 NonPirateDrop = -5;

			switch (GameDifficulty)
				{
				case -1: // Easy
					ReputationDrop = -15;
					NonPirateDrop = - 4;
					break;
				case 0: // Normal
					break;
				case 1: // Hard
					ReputationDrop = -25;
					NonPirateDrop = -6;
					break;
				case 2: // Very Hard
					ReputationDrop = -30;
					NonPirateDrop = -7;
					break;
				case 3: // Expert
					ReputationDrop = -35;
					NonPirateDrop = -9;
					break;
				case 4: // Unfair
					ReputationDrop = -40;
					NonPirateDrop = -10;
					break;
				}

				Owner->GivePlayerReputation(ReputationDrop);
				if (Owner != GetGame()->GetScenarioTools()->Pirates)
				{
					HarpoonOwner->GivePlayerReputationToOthers(NonPirateDrop);
				}

				FFlareMenuParameterData MenuData;
				MenuData.Sector = Sector;

				GetGame()->GetPC()->Notify(LOCTEXT("ShipCaptured", "Ship captured"),
					FText::Format(LOCTEXT("ShipCapturedFormat", "You have captured a {0}-class ship in {1}. Its new name is {2}."),
						NewShip->GetDescription()->Name,
						FText::FromString(Sector->GetSectorName().ToString()),
						UFlareGameTools::DisplaySpacecraftName(NewShip)),
					FName("ship-captured"),
					EFlareNotification::NT_Military,
					false,
					EFlareMenu::MENU_Sector,
					MenuData);

				GetGame()->GetPC()->SetAchievementProgression("ACHIEVEMENT_CAPTURE", 1);
			}

		// Research steal
		{

			// TODO research steal techno ?
			int32 CapturerResearch = HarpoonOwner->GetResearchValue();
			int32 OwnerResearch = Owner->GetResearchValue();

			if(OwnerResearch > CapturerResearch)
			{
				// There is a Fchance to hava research reward

				float StealDevisor = 10.f;
				if (GetGame()->GetPC()->GetCompany() == HarpoonOwner)
				{
					switch (GameDifficulty)
					{
					case -1: // Easy
						StealDevisor = 9.f;
						break;
					case 0: // Normal
						break;
					case 1: // Hard
						StealDevisor = 11.f;
						break;
					case 2: // Very Hard
						StealDevisor = 12.f;
						break;
					case 3: // Expert
						StealDevisor = 14.f;
						break;
					case 4: // Unfair
						StealDevisor = 16.f;
						break;
					}
				}

				int32 ResearchSteal = FMath::Sqrt(float(OwnerResearch - CapturerResearch) / StealDevisor);
				HarpoonOwner->GiveResearch(ResearchSteal);

				if (ResearchSteal > 0 && GetGame()->GetPC()->GetCompany() == HarpoonOwner)
				{
					FFlareMenuParameterData MenuData;
					MenuData.Company = Owner;

					GetGame()->GetPC()->Notify(LOCTEXT("ShipResearchStolen", "Research stolen in the ship"),
						FText::Format(LOCTEXT("ShipResearchStolenFormat", "You stole {0} research during the ship capture."),
									FText::AsNumber(ResearchSteal)),
						FName("ship-research-stolen"),
						EFlareNotification::NT_Military,
						false,
						EFlareMenu::MENU_Sector,
						MenuData);
				}
			}
		}
	}
}

void UFlareWorld::ProcessStationCapture()
{
	TArray<UFlareSimulatedSpacecraft*> StationToCapture;
	TArray<UFlareCompany*> StationCapturer;

	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Sectors[SectorIndex];

		for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorStations().Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorStations()[SpacecraftIndex];
			FFlareSectorBattleState StationOwnerBattleState = Sector->GetSectorBattleState(Spacecraft->GetCompany());

			if (!StationOwnerBattleState.HasDanger)
			{
				// The station is not being captured
				Spacecraft->ResetCapture();
				continue;
			}

			// Find capturing companies
			for (int CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
			{
				UFlareCompany* Company = Companies[CompanyIndex];

				if (!Company->WantCapture(Spacecraft))
				{
					continue;
				}

				if (!Spacecraft->IsHostile(Company)
					|| Sector->GetSectorBattleState(Company).HasDanger)
				{
					// Friend don't capture and not winner don't capture
					continue;
				}

				// Capture
				float NegotiationRatio = 1.f;
				if(Company->IsTechnologyUnlocked("negociations"))
				{
					NegotiationRatio *= 1.5;
				}
				if(Spacecraft->GetCompany()->IsTechnologyUnlocked("negociations"))
				{
					NegotiationRatio *= 0.5;
				}

				int32 CompanyCapturePoint = Sector->GetCompanyCapturePoints(Company) * NegotiationRatio;
/*
				FLOGV("%s is capturing in %s. with %d capture points", 
				*Company->GetCompanyName().ToString(),
				*Sector->GetSectorName().ToString(),
				CompanyCapturePoint);
*/
				if(Spacecraft->TryCapture(Company, CompanyCapturePoint))
				{
					StationToCapture.Add(Spacecraft);
					StationCapturer.Add(Company);
					break;
				}
			}
		}
	}

	FLOGV("Station to capture %d", StationToCapture.Num());

	// Capture consist on :
	// - Kill rotation and velocity
	// - respawn the ship at near location
	// - change its owner
	for (int32 SpacecraftIndex = 0; SpacecraftIndex < StationToCapture.Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = StationToCapture[SpacecraftIndex];
		UFlareCompany* Capturer = StationCapturer[SpacecraftIndex];
		UFlareCompany* Owner = Spacecraft->GetCompany();

		UFlareSimulatedSector* Sector =  Spacecraft->GetCurrentSector();
		FVector SpawnLocation =  Spacecraft->GetData().Location;
		FRotator SpawnRotation =  Spacecraft->GetData().Rotation;
		FFlareSpacecraftSave Data = Spacecraft->GetData();
		FFlareSpacecraftDescription* ShipDescription = Spacecraft->GetDescription();

		TArray<TPair<FFlareSpacecraftDescription*, FFlareSpacecraftSave>> ChildStructure;

		if(Spacecraft->IsComplex())
		{
			for(UFlareSimulatedSpacecraft* Child: Spacecraft->GetComplexChildren())
			{
				ChildStructure.Add(TPair<FFlareSpacecraftDescription*, FFlareSpacecraftSave>(Child->GetDescription(), Child->GetData()));
			}
		}

		Owner->GetAI()->FinishedConstruction(Spacecraft);
		Owner->DestroySpacecraft(Spacecraft);
		UFlareSimulatedSpacecraft* NewShip = Sector->CreateSpacecraft(ShipDescription, Capturer, SpawnLocation, SpawnRotation, &Data);
		Capturer->CapturedStation(NewShip, Owner);

		for(TPair<FFlareSpacecraftDescription*, FFlareSpacecraftSave>& Pair : ChildStructure)
		{
			UFlareSimulatedSpacecraft* NewChildStation = Sector->CreateSpacecraft(Pair.Key, Capturer, SpawnLocation, SpawnRotation, &Pair.Value, false, false, NewShip->GetImmatriculation());
			Sector->AttachStationToComplexStation(NewChildStation, NewShip->GetImmatriculation(), Pair.Value.AttachComplexConnectorName);
		}

		GetGame()->GetQuestManager()->OnSpacecraftCaptured(Spacecraft, NewShip);
		
		if (GetGame()->GetPC()->GetCompany() == Capturer)
		{
			Owner->GivePlayerReputation(-10);

			if (Owner != GetGame()->GetScenarioTools()->Pirates)
			{
				GetGame()->GetPC()->GetCompany()->GivePlayerReputationToOthers(-5);
			}

			FFlareMenuParameterData MenuData;
			MenuData.Sector = Sector;

			GetGame()->GetPC()->Notify(LOCTEXT("StationCaptured", "Station captured"),
				FText::Format(LOCTEXT("StationCapturedFormat", "You have captured a {0} in {1}. Its new name is {2}."),
							NewShip->GetDescription()->Name,
							FText::FromString(Sector->GetSectorName().ToString()),
							UFlareGameTools::DisplaySpacecraftName(NewShip)),
				FName("station-captured"),
				EFlareNotification::NT_Military,
				false,
				EFlareMenu::MENU_Sector,
				MenuData);
		}

		// Research steal
		{

			// TODO research steal techno ?
			int32 CapturerResearch = Capturer->GetResearchValue();
			int32 OwnerResearch = Owner->GetResearchValue();

			if(OwnerResearch > CapturerResearch)
			{
				// There is a chance to hava research reward

				int32 ResearchSteal = FMath::Sqrt(float(OwnerResearch - CapturerResearch));

				Capturer->GiveResearch(ResearchSteal);

				if (ResearchSteal > 0 && GetGame()->GetPC()->GetCompany() == Capturer)
				{
					FFlareMenuParameterData MenuData;
					MenuData.Company = Owner;

					GetGame()->GetPC()->Notify(LOCTEXT("StationResearchStolen", "Research stolen in the station"),
						FText::Format(LOCTEXT("StationResearchStolenFormat", "You stole {0} research during the station capture."),
									FText::AsNumber(ResearchSteal)),
						FName("station-research-stolen"),
						EFlareNotification::NT_Military,
						false,
						EFlareMenu::MENU_Sector,
						MenuData);
				}
			}
		}
	}
}

void UFlareWorld::SimulatePeopleMoneyMigration()
{
	for (int SectorIndexA = 0; SectorIndexA < Sectors.Num(); SectorIndexA++)
	{
		UFlareSimulatedSector* SectorA = Sectors[SectorIndexA];

		for (int SectorIndexB = SectorIndexA + 1; SectorIndexB < Sectors.Num(); SectorIndexB++)
		{
			UFlareSimulatedSector* SectorB = Sectors[SectorIndexB];

			// Money and people migration
			float PopulationA = SectorA->GetPeople()->GetPopulation();
			float PopulationB = SectorB->GetPeople()->GetPopulation();

			if(PopulationA == 0 && PopulationB == 0)
			{
				// 2 sector without population. Do nothing
				continue;
			}
			else if (PopulationA  == 0)
			{
				// Origin sector has no population so it leak it's money
				uint32 TransfertA = SectorA->GetPeople()->GetMoney() / 1000;
				SectorA->GetPeople()->TakeMoney(TransfertA);
				SectorB->GetPeople()->Pay(TransfertA);
			}
			else if (PopulationB  == 0)
			{
				// Destination sector has no population so it leak it's money
				uint32 TransfertB = SectorB->GetPeople()->GetMoney() / 1000;
				SectorB->GetPeople()->TakeMoney(TransfertB);
				SectorA->GetPeople()->Pay(TransfertB);
			}
			else
			{
				// Both have population. The wealthier leak.
				float WealthA = SectorA->GetPeople()->GetWealth();
				float WealthB = SectorB->GetPeople()->GetWealth();
				float TotalWealth = WealthA + WealthB;

				float PercentRatio = 0.05f; // 5% at max
				float TravelDuration = FMath::Max(1.f, (float) UFlareTravel::ComputeTravelDuration(this, SectorA, SectorB, NULL));

				if(TotalWealth > 0)
				{
					if(WealthA > WealthB)
					{
						float LeakRatio = PercentRatio * 2 * ((WealthA / TotalWealth) - 0.5f) / TravelDuration;
						uint32 TransfertA = LeakRatio * SectorA->GetPeople()->GetMoney();
						SectorA->GetPeople()->TakeMoney(TransfertA);
						SectorB->GetPeople()->Pay(TransfertA);
					}
					else
					{
						float LeakRatio = 0.05f * 2 * ((WealthB / TotalWealth) - 0.5f) / TravelDuration; // 5% at max
						uint32 TransfertB = LeakRatio * SectorB->GetPeople()->GetMoney();
						SectorB->GetPeople()->TakeMoney(TransfertB);
						SectorA->GetPeople()->Pay(TransfertB);
					}
				}
			}
		}
	}
}

void UFlareWorld::FastForward()
{
	Simulate();
}

TMap<IncomingKey, IncomingValue> UFlareWorld::GetIncomingPlayerEnemy()
{
	// List sector with player possesion
	TArray<UFlareSimulatedSector*> PlayerSectors;
	TMap<IncomingKey, IncomingValue> IncomingMap;

	UFlareCompany* PlayerCompany = GetGame()->GetPC()->GetCompany();

	for(UFlareSimulatedSpacecraft* Spacecraft : GetGame()->GetPC()->GetCompany()->GetCompanySpacecrafts())
	{
		PlayerSectors.AddUnique(Spacecraft->GetCurrentSector());
	}

	// List dangerous travels
	for (UFlareTravel* Travel : GetTravels())
	{
		if(!Travel->IsPlayerHostile())
		{
			continue;
		}

		int32 EnemyValue = Travel->GetFleet()->GetCombatPoints(true);
		if(EnemyValue == 0)
		{
			continue;
		}

		if(!PlayerSectors.Contains(Travel->GetDestinationSector()))
		{
			continue;
		}

		int64 RemainingDuration = Travel->GetRemainingTravelDuration();
		if (RemainingDuration > 1 && !PlayerCompany->IsTechnologyUnlocked("early-warning"))
		{
			continue;
		}

		IncomingKey key;
		key.Company = Travel->GetFleet()->GetFleetCompany();
		key.RemainingDuration = RemainingDuration;
		key.DestinationSector = Travel->GetDestinationSector();

		IncomingValue& Value = IncomingMap.FindOrAdd(key);


		if (RemainingDuration == 1 || Travel->PickNeedNotification())
		{
			Value.NeedNotification = true;
		}

		Value.CombatValue += EnemyValue;
		Value.LightShipCount += Travel->GetFleet()->GetMilitaryShipCountBySize(EFlarePartSize::S);
		Value.HeavyShipCount += Travel->GetFleet()->GetMilitaryShipCountBySize(EFlarePartSize::L);
	}

	return IncomingMap;
}

void UFlareWorld::ProcessIncomingPlayerEnemy()
{
	if (!GetGame()->GetPC()->GetPlayerShip())
	{
		return;
	}

	FText SingleShip = LOCTEXT("ShipSingle", "ship");
	FText MultipleShips = LOCTEXT("ShipPlural", "ships");

	TMap<IncomingKey, IncomingValue> IncomingMap = GetIncomingPlayerEnemy();

	bool OneDayNotificationHide = true;
	bool MutipleDaysNotificationHide = true;
	int32 OneDayNotificationNeeds = 0;
	int32 MultipleDaysNotificationNeeds = 0;

	for(auto Entry: IncomingMap)
	{
		if (Entry.Key.RemainingDuration == 1)
		{
			OneDayNotificationNeeds++;
			if(Entry.Value.NeedNotification)
			{
				OneDayNotificationHide = false;
			}
		}
		else
		{
			MultipleDaysNotificationNeeds++;
			if(Entry.Value.NeedNotification)
			{
				MutipleDaysNotificationHide = false;
			}
		}
	}

	FFlareMenuParameterData Data;

	auto GetShipsText = [&](int32 LightShipCount, int32 HeavyShipCount)
	{
		// Fighters
		FText LightShipText;
		if (LightShipCount > 0)
		{
			LightShipText = FText::Format(LOCTEXT("PlayerAttackedLightsFormat", "{0} light {1}"),
				FText::AsNumber(LightShipCount),
				(LightShipCount > 1) ? MultipleShips : SingleShip);
		}

		// Heavies
		FText HeavyShipText;
		if (HeavyShipCount > 0)
		{
			HeavyShipText = FText::Format(LOCTEXT("PlayerAttackedHeaviesFormat", "{0} heavy {1}"),
				FText::AsNumber(HeavyShipCount),
				(HeavyShipCount > 1) ? MultipleShips : SingleShip);

			if (LightShipCount > 0)
			{
				HeavyShipText = FText::FromString(", " + HeavyShipText.ToString());
			}
		}

		return FText::Format(LOCTEXT("PlayerAttackedSoonShipsFormat","{0}{1}"),
										LightShipText,
										HeavyShipText);
	};

	auto GetUnknownShipText = [&](int32 UnknownShipCount)
	{
		return FText::Format(LOCTEXT("PlayerAttackedUnknownFormat", "{0} {1}"),
		FText::AsNumber(UnknownShipCount),
		(UnknownShipCount > 1) ? MultipleShips : SingleShip);
	};

	if(!OneDayNotificationHide && OneDayNotificationNeeds == 1)
	{
		for(auto Entry: IncomingMap)
		{
			if (Entry.Key.RemainingDuration == 1)
			{
				FText CompanyName = Entry.Key.Company->GetCompanyName();

				if(GetGame()->GetPC()->GetCompany()->IsTechnologyUnlocked("advanced-radar"))
				{
					GetGame()->GetPC()->Notify(LOCTEXT("PlayerAttackedSoon", "Incoming attack"),
						FText::Format(LOCTEXT("PlayerAttackedSoonFormat", "Your forces in {0} will be attacked tomorrow by {1} with {2} (Combat value: {3}). Prepare for battle."),
							Entry.Key.DestinationSector->GetSectorName(),
							CompanyName,
							GetShipsText(Entry.Value.LightShipCount, Entry.Value.HeavyShipCount),
							Entry.Value.CombatValue),
						FName("travel-raid-soon"),
						EFlareNotification::NT_Military,
						false,
						EFlareMenu::MENU_Orbit,
						Data);
				}
				else
				{
					// Unknown
					int32 UnknownShipCount = Entry.Value.HeavyShipCount + Entry.Value.LightShipCount;

					GetGame()->GetPC()->Notify(LOCTEXT("PlayerAttackedSoon", "Incoming attack"),
						FText::Format(LOCTEXT("PlayerAttackedSoonNoRadarFormat", "Your forces in {0} will be attacked tomorrow by {1} with {2}. Prepare for battle."),
							Entry.Key.DestinationSector->GetSectorName(),
							CompanyName,
							GetUnknownShipText(UnknownShipCount)),
						FName("travel-raid-soon"),
						EFlareNotification::NT_Military,
						false,
						EFlareMenu::MENU_Orbit,
						Data);
				}
			}
			break;
		}
	}
	else if(!OneDayNotificationHide && OneDayNotificationNeeds > 1)
	{
		if(GetGame()->GetPC()->GetCompany()->IsTechnologyUnlocked("advanced-radar"))
		{
			int32 LightShipCount = 0;
			int32 HeavyShipCount = 0;
			int32 CombatValue = 0;

			for(auto Entry: IncomingMap)
			{
				if (Entry.Key.RemainingDuration == 1)
				{
					LightShipCount += Entry.Value.LightShipCount;
					HeavyShipCount += Entry.Value.HeavyShipCount;
					CombatValue += Entry.Value.CombatValue;
				}
			}


			GetGame()->GetPC()->Notify(LOCTEXT("MultiplePlayerAttackedSoon", "Incoming attacks"),
				FText::Format(LOCTEXT("MultiplePlayerAttackedSoonFormat", "{0} attacks incoming tomorrow with {1} (Combat value: {2}). Prepare for battle."),
					OneDayNotificationNeeds,
					GetShipsText(LightShipCount, HeavyShipCount),
					CombatValue),
				FName("travel-raid-soon"),
				EFlareNotification::NT_Military,
				false,
				EFlareMenu::MENU_Orbit,
				Data);
		}
		else
		{
			int32 UnknownShipCount = 0;

			for(auto Entry: IncomingMap)
			{
				if (Entry.Key.RemainingDuration == 1)
				{
					UnknownShipCount += Entry.Value.LightShipCount;
					UnknownShipCount += Entry.Value.HeavyShipCount;
				}
			}

			GetGame()->GetPC()->Notify(LOCTEXT("MultiplePlayerAttackedSoon", "Incoming attacks"),
				FText::Format(LOCTEXT("MultiplePlayerAttackedSoonNoRadarFormat", "{0} attacks incoming tomorrow with {1}. Prepare for battle."),
					OneDayNotificationNeeds,
					GetUnknownShipText(UnknownShipCount)),
				FName("travel-raid-soon"),
				EFlareNotification::NT_Military,
				false,
				EFlareMenu::MENU_Orbit,
				Data);
		}
	}


	if(!MutipleDaysNotificationHide && MultipleDaysNotificationNeeds == 1)
	{
		for(auto Entry: IncomingMap)
		{
			if (Entry.Key.RemainingDuration > 1)
			{
				FText CompanyName = Entry.Key.Company->GetCompanyName();

				if(GetGame()->GetPC()->GetCompany()->IsTechnologyUnlocked("advanced-radar"))
				{
					FText FirstPart = FText::Format(LOCTEXT("PlayerAttackedDistant1Format", "Your forces in {0} will be attacked in {1} days"),
							Entry.Key.DestinationSector->GetSectorName(),
							FText::AsNumber(Entry.Key.RemainingDuration));

					GetGame()->GetPC()->Notify(LOCTEXT("PlayerAttackedDistant", "Incoming attack"),
						FText::Format(LOCTEXT("PlayerAttackedDistantFormat",  "{0} by {1} with {2} (Combat value: {3}). Prepare for battle."),
							FirstPart,
							CompanyName,
							GetShipsText(Entry.Value.LightShipCount, Entry.Value.HeavyShipCount),
							Entry.Value.CombatValue),
						FName("travel-raid-distant"),
						EFlareNotification::NT_Military,
						false,
						EFlareMenu::MENU_Orbit,
						Data);
				}
				else
				{
					int32 UnknownShipCount = Entry.Value.HeavyShipCount + Entry.Value.LightShipCount;

					GetGame()->GetPC()->Notify(LOCTEXT("PlayerAttackedDistant", "Incoming attack"),
						FText::Format(LOCTEXT("PlayerAttackedDistantNoRadarFormat", "Your forces in {0} will be attacked in {1} days by {2} with {3}. Prepare for battle."),
							Entry.Key.DestinationSector->GetSectorName(),
							FText::AsNumber(Entry.Key.RemainingDuration),
							CompanyName,
							GetUnknownShipText(UnknownShipCount)),
						FName("travel-raid-distant"),
						EFlareNotification::NT_Military,
						false,
						EFlareMenu::MENU_Orbit,
						Data);
				}
			}
			break;
		}
	}
	else if(!MutipleDaysNotificationHide && MultipleDaysNotificationNeeds > 1)
	{
		if(GetGame()->GetPC()->GetCompany()->IsTechnologyUnlocked("advanced-radar"))
		{
			int32 LightShipCount = 0;
			int32 HeavyShipCount = 0;
			int32 CombatValue = 0;

			for(auto Entry: IncomingMap)
			{
				if (Entry.Key.RemainingDuration > 1)
				{
					LightShipCount += Entry.Value.LightShipCount;
					HeavyShipCount += Entry.Value.HeavyShipCount;
					CombatValue += Entry.Value.CombatValue;
				}
			}


			GetGame()->GetPC()->Notify(LOCTEXT("MultiplePlayerAttackedDistant", "Incoming attacks"),
				FText::Format(LOCTEXT("MultipleDaysMultiplePlayerAttackedSoonFormat", "{0} attacks are incoming in a few days. You will be attacked with {1} (Combat value: {2}). Prepare for battle."),
					MultipleDaysNotificationNeeds,
					GetShipsText(LightShipCount, HeavyShipCount),
					CombatValue),
				FName("travel-raid-distant"),
				EFlareNotification::NT_Military,
				false,
				EFlareMenu::MENU_Orbit,
				Data);
		}
		else
		{
			int32 UnknownShipCount = 0;

			for(auto Entry: IncomingMap)
			{
				if (Entry.Key.RemainingDuration > 1)
				{
					UnknownShipCount += Entry.Value.LightShipCount;
					UnknownShipCount += Entry.Value.HeavyShipCount;
				}
			}

			GetGame()->GetPC()->Notify(LOCTEXT("MultiplePlayerAttackedDistant", "Incoming attacks"),
				FText::Format(LOCTEXT("MultipleMultipleDaysPlayerAttackedSoonNoRadarFormat", "{0} attacks are incoming in few days. You will be attacked with {1}. Prepare for battle."),
					MultipleDaysNotificationNeeds,
					GetUnknownShipText(UnknownShipCount)),
				FName("travel-raid-distant"),
				EFlareNotification::NT_Military,
				false,
				EFlareMenu::MENU_Orbit,
				Data);
		}
	}
}

void UFlareWorld::ForceDate(int64 Date)
{
	while(WorldData.Date < Date)
	{
		Simulate();
	}
}

inline static bool EventDateComparator (const FFlareWorldEvent& ip1, const FFlareWorldEvent& ip2)
 {
	 return (ip1.Date < ip2.Date);
 }

TArray<FFlareWorldEvent> UFlareWorld::GenerateEvents(UFlareCompany* PointOfView)
{
	TArray<FFlareWorldEvent> NextEvents;

	// TODO Implements PointOfView

	// Generate travel events
	for (int TravelIndex = 0; TravelIndex < Travels.Num(); TravelIndex++)
	{
		FFlareWorldEvent TravelEvent;

		TravelEvent.Date = WorldData.Date + Travels[TravelIndex]->GetRemainingTravelDuration();
		TravelEvent.Visibility = EFlareEventVisibility::Blocking;
		NextEvents.Add(TravelEvent);
	}

	// Generate factory events
	for (int FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
	{
		FFlareWorldEvent *FactoryEvent = Factories[FactoryIndex]->GenerateEvent();
		if (FactoryEvent)
		{
			NextEvents.Add(*FactoryEvent);
		}
	}

	NextEvents.Sort(&EventDateComparator);

	return NextEvents;
}

void UFlareWorld::ClearFactories(UFlareSimulatedSpacecraft *ParentSpacecraft)
{
	for (int FactoryIndex = Factories.Num() -1 ; FactoryIndex >= 0; FactoryIndex--)
	{
		UFlareFactory* Factory = Factories[FactoryIndex];
		if (Factory->GetParent() == ParentSpacecraft)
		{
			Factories.RemoveAt(FactoryIndex);
			if (ParentSpacecraft->IsShipyard())
			{
				Shipyards.Remove(ParentSpacecraft);
			}
		}
	}
}

void UFlareWorld::AddFactory(UFlareFactory* Factory)
{
	Factories.Add(Factory);

	if (Factory->GetParent()->IsShipyard() && !Factory->GetParent()->GetDescription()->IsDroneCarrier)
	{
		Shipyards.AddUnique(Factory->GetParent());
		FLOGV("UFlareWorld::AddFactory : AddShipyard %s as factory for %s",
		*Factory->GetDescription()->Name.ToString(),
		*Factory->GetParent()->GetImmatriculation().ToString());
	}
}

UFlareTravel* UFlareWorld::	StartTravel(UFlareFleet* TravelingFleet, UFlareSimulatedSector* DestinationSector, bool Force)
{
	if (!TravelingFleet || (!TravelingFleet->CanTravel() && !Force))
	{
		return NULL;
	}

	if (TravelingFleet->IsTraveling())
	{
		TravelingFleet->GetCurrentTravel()->ChangeDestination(DestinationSector);
		return TravelingFleet->GetCurrentTravel();
	}
	else if (TravelingFleet->GetCurrentSector() == DestinationSector && !Force)
	{
		//Try to start a travel to current sector
		return NULL;
	}
	else if(DestinationSector)
	{
		UFlareSimulatedSector* OriginSector = TravelingFleet->GetCurrentSector();

		// Remove intercepted ships
		if(OriginSector->GetSectorBattleState(TravelingFleet->GetFleetCompany()).HasDanger)
		{
			uint32 InterceptedShips = TravelingFleet->InterceptShips();

			if (InterceptedShips > 0)
			{
				if(TravelingFleet->GetFleetCompany() == Game->GetPC()->GetCompany())
				{
					FText SingleShip = LOCTEXT("ShipSingle", "ship");
					FText MultipleShips = LOCTEXT("ShipPlural", "ships");
					FText ShipText = (InterceptedShips > 1) ? MultipleShips : SingleShip;

					FFlareMenuParameterData Data;
					Data.Sector = OriginSector;
					GetGame()->GetPC()->Notify(LOCTEXT("ShipsIntercepted", "Ships intercepted"),
						FText::Format(LOCTEXT("ShipInterceptedFormat", "{0} {1} have been intercepted trying to escape from {2}."),
									  FText::AsNumber(InterceptedShips),
									  ShipText,
									  FText::FromString(OriginSector->GetSectorName().ToString())),
						FName("ships-intersepted"),
						EFlareNotification::NT_Military,
						false,
						EFlareMenu::MENU_Sector,
						Data);
				}

				if(InterceptedShips >= TravelingFleet->GetShipCount())
				{
					// All ships intercepted
					return NULL;
				}
			}
		}

		// Remove immobilized ships
		uint32 TravelingFleetSize = TravelingFleet->GetShipCount();
		uint32 RemovedShips = TravelingFleet->RemoveImmobilizedShips();
		if (RemovedShips < TravelingFleetSize)
		{
			// Make the fleet exit the sector

			OriginSector->RetireFleet(TravelingFleet);

			// Create the travel
			FFlareTravelSave TravelData;
			TravelData.FleetIdentifier = TravelingFleet->GetIdentifier();
			TravelData.OriginSectorIdentifier = OriginSector->GetIdentifier();
			TravelData.DestinationSectorIdentifier = DestinationSector->GetIdentifier();
			TravelData.DepartureDate = GetDate();
			UFlareTravel::InitTravelSector(TravelData.SectorData);
			UFlareTravel* Travel = LoadTravel(TravelData, TravelingFleet);
			GetGame()->GetQuestManager()->OnTravelStarted(Travel);
			return Travel;
		}
	}
	return NULL;
}

void UFlareWorld::DeleteTravel(UFlareTravel* Travel)
{
	Travels.Remove(Travel);
}

/*----------------------------------------------------
	Getters
----------------------------------------------------*/

TArray<UFlareSimulatedSpacecraft*> UFlareWorld::GetShipyardsFor(UFlareCompany* Company)
{
	TArray<UFlareSimulatedSpacecraft*> ShipyardList = GetShipyards();
	if (!Company) return ShipyardList;

	for (int32 ShipyardIndex = 0; ShipyardIndex < Shipyards.Num(); ShipyardIndex++)
	{
		UFlareSimulatedSpacecraft* Shipyard = Shipyards[ShipyardIndex];

		if (Shipyard->IsHostile(Company))
		{
			ShipyardList.Remove(Shipyard);
			continue;
		}

		UFlareSimulatedSector* Sector = Shipyard->GetCurrentSector();
		bool IsKnownSector = false;

		//todo: Optimize this known sectors lookup
		for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
		{
			UFlareSimulatedSector* KnownSector = Company->GetKnownSectors()[SectorIndex];
			if (KnownSector == Sector)
			{
				IsKnownSector = true;
				break;
			}
		}

		if (!IsKnownSector)
		{
			ShipyardList.Remove(Shipyard);
			continue;
		}
	}

	ShipyardList.Sort([&](UFlareSimulatedSpacecraft& A, UFlareSimulatedSpacecraft& B)
	{
		if (A.GetCompany() == Company)
		{
			return true;
		}

		return false;
	});

	return ShipyardList;
}

UFlareCompany* UFlareWorld::FindCompany(FName Identifier) const
{
	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];
		if (Company->GetIdentifier() == Identifier)
		{
			return Company;
		}
	}

	return NULL;
}

UFlareCompany* UFlareWorld::FindCompanyByShortName(FName CompanyShortName) const
{
	// Find company
	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];
		if (Company->GetShortName() == CompanyShortName)
		{
			return Company;
		}
	}

	return NULL;
}

UFlareSimulatedSector* UFlareWorld::FindSector(FName Identifier) const
{
	for (int i = 0; i < Sectors.Num(); i++)
	{
		UFlareSimulatedSector* Sector = Sectors[i];
		if (Sector->GetIdentifier() == Identifier)
		{
			return Sector;
		}
	}

	return NULL;
}

UFlareSimulatedSector* UFlareWorld::FindSectorBySpacecraft(FName SpacecraftIdentifier) const
{
	for (int i = 0; i < Sectors.Num(); i++)
	{
		UFlareSimulatedSector* Sector = Sectors[i];
		for (int j = 0; j < Sector->GetSectorSpacecrafts().Num(); j++)
		{
			if (Sector->GetSectorSpacecrafts()[j]->GetDescription()->Identifier == SpacecraftIdentifier)
			{
				return Sector;
			}
		}
	}
	return NULL;
}

UFlareFleet* UFlareWorld::FindFleet(FName Identifier) const
{
	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];
		UFlareFleet* Fleet = Company->FindFleet(Identifier);
		if (Fleet)
		{
			return Fleet;
		}
	}
	return NULL;
}

UFlareTradeRoute* UFlareWorld::FindTradeRoute(FName Identifier) const
{
	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];
		UFlareTradeRoute* TradeRoute = Company->FindTradeRoute(Identifier);
		if (TradeRoute)
		{
			return TradeRoute;
		}
	}
	return NULL;
}

UFlareSimulatedSpacecraft* UFlareWorld::FindSpacecraft(FName ShipImmatriculation)
{
	for (UFlareCompany* Company : Companies)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Company->FindSpacecraft(ShipImmatriculation);
		if (Spacecraft)
		{
			return Spacecraft;
		}
		Spacecraft = Company->FindSpacecraft(ShipImmatriculation, true);
		if (Spacecraft)
		{
			return Spacecraft;
		}
	}
/*
	// Now check destroyed ships
	for (UFlareCompany* Company : Companies)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Company->FindSpacecraft(ShipImmatriculation, true);
		if (Spacecraft)
		{
			return Spacecraft;
		}
	}
*/
	return NULL;
}


int64 UFlareWorld::GetWorldMoney()
{
	// World money is the sum of company money + factory money + people money

	int64 CompanyMoney = 0;
	int64 FactoryMoney = 0;
	int64 PeopleMoney = 0;

	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* Company = Companies[i];

		CompanyMoney += Company->GetMoney();

		TArray<UFlareSimulatedSpacecraft*>& Spacecrafts = Company->GetCompanySpacecrafts();
		for (int ShipIndex = 0; ShipIndex < Spacecrafts.Num() ; ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = Spacecrafts[ShipIndex];

			for (int32 FactoryIndex = 0; FactoryIndex < Spacecraft->GetFactories().Num(); FactoryIndex++)
			{
				UFlareFactory* Factory = Spacecraft->GetFactories()[FactoryIndex];
				FactoryMoney += Factory->GetReservedMoney();
			}
		}
	}

	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		PeopleMoney += Sectors[SectorIndex]->GetPeople()->GetMoney();
		PeopleMoney -= Sectors[SectorIndex]->GetPeople()->GetDept();
	}

	int64 WorldMoney = CompanyMoney + FactoryMoney + PeopleMoney;

	//FLOGV("World money: %lld", WorldMoney);
	//FLOGV("  - company: %lld", CompanyMoney);
	//FLOGV("  - factory: %lld", FactoryMoney);
	//FLOGV("  - people : %lld", PeopleMoney);

	return WorldMoney;
}

uint32 UFlareWorld::GetWorldPopulation()
{
	uint32 WorldPopulation = 0;

	for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		WorldPopulation += Sectors[SectorIndex]->GetPeople()->GetPopulation();
	}

	return WorldPopulation;
}
TArray<FFlareIncomingEvent> UFlareWorld::GetIncomingEvents()
{
	TArray<FFlareIncomingEvent> IncomingEvents;
	UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();
	FText SingleShip = LOCTEXT("ShipSingle", "ship");
	FText MultipleShips = LOCTEXT("ShipPlural", "ships");

	auto GetShipsText = [&](int32 LightShipCount, int32 HeavyShipCount)
	{
		// Fighters
		FText LightShipText;
		if (LightShipCount > 0)
		{
			LightShipText = FText::Format(LOCTEXT("PlayerAttackedLightsFormat", "{0} light {1}"),
				FText::AsNumber(LightShipCount),
				(LightShipCount > 1) ? MultipleShips : SingleShip);
		}

		// Heavies
		FText HeavyShipText;
		if (HeavyShipCount > 0)
		{
			HeavyShipText = FText::Format(LOCTEXT("PlayerAttackedHeaviesFormat", "{0} heavy {1}"),
				FText::AsNumber(HeavyShipCount),
				(HeavyShipCount > 1) ? MultipleShips : SingleShip);

			if (LightShipCount > 0)
			{
				HeavyShipText = FText::FromString(", " + HeavyShipText.ToString());
			}
		}

		return FText::Format(LOCTEXT("PlayerAttackedSoonShipsFormat","{0}{1}"),
										LightShipText,
										HeavyShipText);
	};

	auto GetUnknownShipText = [&](int32 UnknownShipCount)
	{
		return FText::Format(LOCTEXT("PlayerAttackedUnknownFormat", "{0} {1}"),
		FText::AsNumber(UnknownShipCount),
		(UnknownShipCount > 1) ? MultipleShips : SingleShip);
	};

	// Global Events
	FFlareWorldGameEventSave* GlobalMeteorStormEvent = GetGlobalEvent("Meteor Storm");
	bool MeteorStormInProgress = (WorldData.Date >= GlobalMeteorStormEvent->EventDate && WorldData.Date < GlobalMeteorStormEvent->EventDateEnd);
	if (MeteorStormInProgress)
	{
		int64 RemainingDuration = GlobalMeteorStormEvent->EventDateEnd - WorldData.Date;
		FFlareIncomingEvent MeteoriteEvent;

		MeteoriteEvent.Text = FText::Format(LOCTEXT("IncreasedMeteoroidActivities", "\u2022 <WarningText>Increased meteoroid activity ({0} left)</>"),
		UFlareGameTools::FormatDate(RemainingDuration, 1));
		MeteoriteEvent.RemainingDuration = RemainingDuration;
		IncomingEvents.Add(MeteoriteEvent);
	}

	// List incoming threats first
	TMap<IncomingKey, IncomingValue> IncomingMap = GetIncomingPlayerEnemy();
	for (auto Entry : IncomingMap)
	{
		UFlareCompany* Company = Entry.Key.Company;
		UFlareSimulatedSector* Sector = Entry.Key.DestinationSector;
		int32 EnemyValue = Entry.Value.CombatValue;
		int64 RemainingDuration = Entry.Key.RemainingDuration;

		if (RemainingDuration <= 1 || PlayerCompany->IsTechnologyUnlocked("early-warning"))
		{
			FText TravelText;
			FText TravelPartText = FText::Format(LOCTEXT("ThreatTextTravelPart", "Traveling to {0} ({1} left)"),
					Sector->GetSectorName(),
					UFlareGameTools::FormatDate(RemainingDuration, 1));

			if (PlayerCompany->IsTechnologyUnlocked("advanced-radar"))
			{

				TravelText = FText::Format(LOCTEXT("ThreatTextAdvancedFormat", "\u2022 <WarningText>{0} (Combat value of {1})</>\n    <WarningText>{2}</>\n    <WarningText>{3}</>"),
						Company->GetCompanyName(),
						EnemyValue,
						TravelPartText,
						GetShipsText(Entry.Value.LightShipCount, Entry.Value.HeavyShipCount));
			}	
			else
			{
				TravelText = FText::Format(LOCTEXT("ThreatTextFormat", "\u2022 <WarningText>{0}</>\n    <WarningText>{1}</>\n    <WarningText>{2}</>"),

					Company->GetCompanyName(),
					TravelPartText,
					GetUnknownShipText(Entry.Value.LightShipCount + Entry.Value.HeavyShipCount)
					);
			}

			// Add data
			FFlareIncomingEvent TravelEvent;
			TravelEvent.Text = TravelText;
			TravelEvent.RemainingDuration = RemainingDuration;
			IncomingEvents.Add(TravelEvent);
		}
	}

	// Warn of meteorites
	for (UFlareSimulatedSector* Sector : GetSectors())
	{
		if (Sector->GetMeteorites().Num())
		{
			bool Danger = false;
			int32 DangerDelay = 0;
			int32 DangerCount = 0;
			bool PlayerTarget = false;

			for (FFlareMeteoriteSave& Meteorite : Sector->GetMeteorites())
			{
				if (Meteorite.HasMissed || Meteorite.Damage >= Meteorite.BrokenDamage || !GetGame()->GetQuestManager()->IsInterestingMeteorite(Meteorite))
				{
					continue;
				}

				UFlareSimulatedSpacecraft* TargetSpacecraft = GetGame()->GetGameWorld()->FindSpacecraft(Meteorite.TargetStation);
				if (TargetSpacecraft)
				{
					if (!Danger || DangerDelay > Meteorite.DaysBeforeImpact)
					{
						Danger = true;
						DangerDelay = Meteorite.DaysBeforeImpact;
						DangerCount = 1;
						PlayerTarget = TargetSpacecraft->GetCompany() == PlayerCompany;
					}
					else if (DangerDelay == Meteorite.DaysBeforeImpact)
					{
						DangerCount++;
					}
				}
			}

			if (Danger)
			{
				FFlareIncomingEvent Event;
				Event.RemainingDuration = DangerDelay;
				FText MeteoriteText = (DangerCount > 1 ? LOCTEXT("MultipleMeteorites", "meteoroids") : LOCTEXT("OneMeteorite", "meteoroid"));

				if (DangerDelay == 0)
				{
					Event.Text = FText::Format(LOCTEXT("MeteoriteNowTextFormat", "\u2022 <WarningText>{0} {1} threatening {2} now !</>"),
						FText::AsNumber(DangerCount),
						MeteoriteText,
						Sector->GetSectorName());
				}
				else if (DangerDelay == 1 || PlayerCompany->IsTechnologyUnlocked("early-warning") || !PlayerTarget)
				{
					FText DelayText = (DangerDelay > 1 ? FText::Format(LOCTEXT("MeteoriteMultipleDaysFormat", "{0} days"), FText::AsNumber(DangerDelay)) : LOCTEXT("OneDay", "1 day"));

					if (PlayerCompany->IsTechnologyUnlocked("advanced-radar"))
					{
						Event.Text = FText::Format(LOCTEXT("MeteoriteSoonTextFormat", "\u2022 <WarningText>{0} {1} threatening {2} in {3} !</>"),
							FText::AsNumber(DangerCount),
							MeteoriteText,
							Sector->GetSectorName(),
							DelayText);
					}
					else
					{
						Event.Text = FText::Format(LOCTEXT("MeteoriteSoonNoRadarTextFormat", "\u2022 <WarningText>Meteoroid group threatening {0} in {1} !</>"),
							Sector->GetSectorName(),
							DelayText);
					}
				}
				if (!Event.Text.IsEmpty())
				{
					IncomingEvents.Add(Event);
				}
			}
		}
	}

	// List travels
	for (UFlareTravel* Travel : GetTravels())
	{
		if (Travel->GetFleet()->GetFleetCompany() == PlayerCompany)
		{
			int64 RemainingDuration = Travel->GetRemainingTravelDuration();
			FText TravelText;

			// Trade route version
			if (Travel->GetFleet()->GetCurrentTradeRoute())
			{
				TravelText = FText::Format(LOCTEXT("TravelTextTradeRouteFormat", "\u2022 {0} ({1}{2})\n    {3}"),
					Travel->GetFleet()->GetFleetName(),
					Travel->GetFleet()->GetCurrentTradeRoute()->GetTradeRouteName(),
					(Travel->GetFleet()->GetCurrentTradeRoute()->IsPaused() ? UFlareGameTools::AddLeadingSpace(LOCTEXT("FleetTradeRoutePausedFormat", "(paused)")) : FText()),
					Travel->GetFleet()->GetStatusInfo());
			}
			else
			{
				TravelText = FText::Format(LOCTEXT("TravelTextManualFormat", "\u2022 {0}\n    {1}"),
					Travel->GetFleet()->GetFleetName(),
					Travel->GetFleet()->GetStatusInfo());
			}

			// Add data
			FFlareIncomingEvent TravelEvent;
			TravelEvent.Text = TravelText;
			TravelEvent.RemainingDuration = RemainingDuration;
			IncomingEvents.Add(TravelEvent);
		}
	}

	// List ship productions
	for (int32 CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
	{
		TArray<UFlareSimulatedSpacecraft*>& CompanyStations = Companies[CompanyIndex]->GetCompanyStations();
		for (UFlareSimulatedSpacecraft* CompanyStation : CompanyStations)
		{
			if(!CompanyStation->IsShipyard())
			{
				continue;
			}

			for (int32 FactoryIndex = 0; FactoryIndex < CompanyStation->GetFactories().Num(); FactoryIndex++)
			{
				// Get shipyard if existing
				UFlareFactory* TargetFactory = CompanyStation->GetFactories()[FactoryIndex];
				FText SectorName = CompanyStation->GetCurrentSector()->GetSectorName();
				FName CompanyIdentifier = PlayerCompany->GetIdentifier();
				if (!TargetFactory->IsShipyard())
				{
					continue;
				}

				// Ship being built
				else if (TargetFactory->GetTargetShipCompany() == CompanyIdentifier)
				{
					int64 ProductionTime = TargetFactory->GetRemainingProductionDuration();

					FText ProductionText;

					if (TargetFactory->IsActive() && TargetFactory->IsNeedProduction() && !TargetFactory->HasCostReserved() && !TargetFactory->HasInputResources())
					{
						FString MissingResources = TargetFactory->GetInputResourcesRequiredString();
						ProductionText = FText::Format(LOCTEXT("ShipProductionNoResourcesProdTextFormat", "\u2022 {0} being built at {1} (missing {2})"),
						Game->GetSpacecraftCatalog()->Get(TargetFactory->GetTargetShipClass())->Name,
						SectorName,
						FText::FromString(MissingResources));
					}
					else
					{
						ProductionText = FText::Format(LOCTEXT("ShipProductionTextFormat", "\u2022 {0} being built at {1} ({2} left)"),
						Game->GetSpacecraftCatalog()->Get(TargetFactory->GetTargetShipClass())->Name,
						SectorName,
						UFlareGameTools::FormatDate(ProductionTime, 2));
					}

					FFlareIncomingEvent ProductionEvent;
					ProductionEvent.Text = ProductionText;
					ProductionEvent.RemainingDuration = ProductionTime;
					IncomingEvents.Add(ProductionEvent);
				}
			}

			TArray<FFlareShipyardOrderSave>& Orders = CompanyStation->GetShipyardOrderQueue();

			for(int32 i = 0; i < Orders.Num(); i++)
			{
				FFlareShipyardOrderSave& Order = Orders[i];

				if(Order.Company == PlayerCompany->GetIdentifier())
				{
					FFlareSpacecraftDescription* OrderDesc = Game->GetSpacecraftCatalog()->Get(Order.ShipClass);
					FText SectorName = CompanyStation->GetCurrentSector()->GetSectorName();
					int32 ProductionTime = CompanyStation->GetShipProductionTime(Order.ShipClass) + CompanyStation->GetEstimatedQueueAndProductionDuration(Order.ShipClass, i);
					FText ProductionText;

					if (CompanyStation->IsShipyardMissingResources())
					{
						ProductionText = FText::Format(LOCTEXT("ShipNoResourcesProdTextFormat", "\u2022 {0} ordered at {1} (missing resources)"),
						OrderDesc->Name,
						SectorName);
					}
					else
					{
						ProductionText = FText::Format(LOCTEXT("ShipWaitingProdTextFormat", "\u2022 {0} ordered at {1} ({2} left)"),
						OrderDesc->Name,
						SectorName,
						UFlareGameTools::FormatDate(ProductionTime, 2));
					}

					FFlareIncomingEvent ProductionEvent;
					ProductionEvent.Text = ProductionText;
					ProductionEvent.RemainingDuration = ProductionTime;
					IncomingEvents.Add(ProductionEvent);
				}
			}
		}
	}
	
	// List of battle
	for (UFlareSimulatedSector* Sector: GetSectors())
	{
		FFlareSectorBattleState BattleState = Sector->GetSectorBattleState(PlayerCompany);
		if(BattleState.InBattle)
		{
			if(Sector == Game->GetPC()->GetPlayerFleet()->GetCurrentSector())
			{
				if (BattleState.InBattle && BattleState.InFight && BattleState.InActiveFight)
				{

					FFlareIncomingEvent LocalBattleEvent;
					LocalBattleEvent.Text = LOCTEXT("LocalBattleFightEventTextFormat", "\u2022 <WarningText>Battle in progress here !</>");
					LocalBattleEvent.RemainingDuration = -1;
					IncomingEvents.Add(LocalBattleEvent);
				}
				else
				{
					FFlareIncomingEvent LocalBattleEvent;
					LocalBattleEvent.Text = FText::Format(LOCTEXT("LocalBattleEventTextFormat", "\u2022 <WarningText>{0} here !</>"),
													Sector->GetSectorBattleStateText(PlayerCompany),
													 Sector->GetSectorName());
					LocalBattleEvent.RemainingDuration = 0;
					IncomingEvents.Add(LocalBattleEvent);
				}
			}
			else
			{
				FFlareIncomingEvent RemoteBattleEvent;
				RemoteBattleEvent.Text = FText::Format(LOCTEXT("RemoteBattleEventTextFormat", "\u2022 <WarningText>{0} in {1}</>"),
												Sector->GetSectorBattleStateText(PlayerCompany),
												 Sector->GetSectorName());
				RemoteBattleEvent.RemainingDuration = 0;
				IncomingEvents.Add(RemoteBattleEvent);
			}
		}
	}

	// List of time to repair
	for (UFlareFleet* Fleet : PlayerCompany->GetCompanyFleets())
	{
		int32 RepairDuration = Fleet->GetRepairDuration();
		int32 RefillDuration = Fleet->GetRefillDuration();

		if (RepairDuration > 0)
		{
			FFlareIncomingEvent RepairEvent;
			RepairEvent.Text = FText::Format(LOCTEXT("RepairEventTextFormat", "\u2022 {0} being repaired ({1} left)"),
				Fleet->GetFleetName(), UFlareGameTools::FormatDate(RepairDuration, 2));
			RepairEvent.RemainingDuration = RepairDuration;
			IncomingEvents.Add(RepairEvent);
		}

		if (RefillDuration > 0)
		{
			FFlareIncomingEvent RefillEvent;
			RefillEvent.Text = FText::Format(LOCTEXT("RefillEventTextFormat", "\u2022 {0} being refilled ({1} left)"),
				Fleet->GetFleetName(), UFlareGameTools::FormatDate(RefillDuration, 2));;
			RefillEvent.RemainingDuration = RefillDuration;
			IncomingEvents.Add(RefillEvent);
		}
	}

	// Sort list
	IncomingEvents.Sort([](const FFlareIncomingEvent& ip1, const FFlareIncomingEvent& ip2)
	{
		return (ip1.RemainingDuration < ip2.RemainingDuration);
	});
	return IncomingEvents;
}

int32 UFlareWorld::GetTotalWorldCombatPoint()
{
	if (!HasTotalWorldCombatPointCache)
	{
		TotalWorldCombatPointCache = 0;

		for (UFlareCompany* OtherCompany : GetCompanies())
		{
			struct CompanyValue Value = OtherCompany->GetCompanyValue();
			TotalWorldCombatPointCache += Value.ArmyCurrentCombatPoints;
		}

		HasTotalWorldCombatPointCache = true;
	}

	return TotalWorldCombatPointCache;
}

#undef LOCTEXT_NAMESPACE