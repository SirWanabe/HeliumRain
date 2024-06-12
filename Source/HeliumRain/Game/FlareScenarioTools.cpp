
#include "FlareScenarioTools.h"
#include "../Flare.h"

#include "../Data/FlareResourceCatalog.h"

#include "../Economy/FlareFactory.h"
#include "../Economy/FlareCargoBay.h"

#include "../Game/FlareWorld.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareSimulatedSector.h"

#include "../Player/FlarePlayerController.h"

#include "../Spacecrafts/FlareSimulatedSpacecraft.h"
#include "../Data/FlareSpacecraftCatalog.h"
#include "../Data/FlareStartingScenarioCatalogEntry.h"

#define LOCTEXT_NAMESPACE "FlareScenarioToolsInfo"
#define DEFAULT_PLAYER_MONEY 750000

/*----------------------------------------------------
	Public API
----------------------------------------------------*/

UFlareScenarioTools::UFlareScenarioTools(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareScenarioTools::Init(UFlareCompany* Company, FFlarePlayerSave* Player)
{
	Game = Cast<AFlareGame>(GetOuter());
	World = Game->GetGameWorld();
	PlayerCompany = Company;
	PlayerData = Player;

	Nema = World->GetPlanerarium()->FindCelestialBody("nema");
	Anka = World->GetPlanerarium()->FindCelestialBody("anka");
	Hela = World->GetPlanerarium()->FindCelestialBody("hela");
	Asta = World->GetPlanerarium()->FindCelestialBody("asta");
	Adena = World->GetPlanerarium()->FindCelestialBody("adena");

	// Notable sectors (Nema)
	TheDepths =   World->FindSector("the-depths");
	FirstLight =  World->FindSector("first-light");
	MinersHome =  World->FindSector("miners-home");
	Anomaly =     World->FindSector("anomaly");
	BlueHeart =   World->FindSector("blue-heart");
	Lighthouse =  World->FindSector("lighthouse");
	BlueShores =  World->FindSector("blue-shores");
	TheSpire =    World->FindSector("the-spire");
	Pendulum =    World->FindSector("pendulum");
	TheFarm =     World->FindSector("farm");
	
	// Notable sectors (Anka)
	Outpost =     World->FindSector("outpost");
	Crossroads =  World->FindSector("crossroads");
	Colossus =    World->FindSector("colossus");
	TheDig =      World->FindSector("the-dig");
	TheForge =    World->FindSector("the-forge");

	// Notable sectors (Hela)
	FrozenRealm = World->FindSector("frozen-realm");
	NightsHome =  World->FindSector("nights-home");
	ShoreOfIce =  World->FindSector("shore-of-ice");
	Ruins =       World->FindSector("ruins");
	WinterJunction = World->FindSector("winter-junction");

	// Notable sectors (Asta)
	Decay =       World->FindSector("decay");
	Boneyard =    World->FindSector("boneyard");
	Daedalus =    World->FindSector("daedalus");

	// Notable sectors (Adena)
	Solitude =    World->FindSector("solitude");
	Serenity =    World->FindSector("serenity");
	Tranquility = World->FindSector("tranquility");


	// Companies
	MiningSyndicate =      World->FindCompanyByShortName("MSY");
	HelixFoundries =       World->FindCompanyByShortName("HFR");
	Sunwatch =             World->FindCompanyByShortName("SUN");
	IonLane =              World->FindCompanyByShortName("ION");
	UnitedFarmsChemicals = World->FindCompanyByShortName("UFC");
	GhostWorksShipyards =  World->FindCompanyByShortName("GWS");
	NemaHeavyWorks =       World->FindCompanyByShortName("NHW");
	Pirates =              World->FindCompanyByShortName("PIR");
	AxisSupplies =         World->FindCompanyByShortName("AXS");
	BrokenMoon =           World->FindCompanyByShortName("BRM");
	InfiniteOrbit =        World->FindCompanyByShortName("IFO");
	Quantalium =           World->FindCompanyByShortName("QNT");

	// Resources
	Water =    Game->GetResourceCatalog()->Get("h2o");
	Food =     Game->GetResourceCatalog()->Get("food");
	Fuel =     Game->GetResourceCatalog()->Get("fuel");
	Plastics = Game->GetResourceCatalog()->Get("plastics");
	Hydrogen = Game->GetResourceCatalog()->Get("h2");
	Helium =   Game->GetResourceCatalog()->Get("he3");
	Silica =   Game->GetResourceCatalog()->Get("sio2");
	IronOxyde =Game->GetResourceCatalog()->Get("feo");
	Steel =    Game->GetResourceCatalog()->Get("steel");
	Tools =    Game->GetResourceCatalog()->Get("tools");
	Tech =     Game->GetResourceCatalog()->Get("tech");
	Carbon =     Game->GetResourceCatalog()->Get("carbon");
	Methane =     Game->GetResourceCatalog()->Get("ch4");
	FleetSupply =     Game->GetResourceCatalog()->Get("fleet-supply");

	// Ships
	ShipSolen = "ship-solen";
	ShipOmen = "ship-omen";
	ShipGhoul = "ship-ghoul";
	ShipOrca = "ship-orca";
	ShipPhalanx = "ship-phalanx";

	ShipSphinx = "ship-sphinx";
	ShipThemis = "ship-themis";
	ShipAtlas = "ship-atlas";
	
	ShipDragon = "ship-dragon";
	ShipKami = "ship-kami";
	ShipInvader = "ship-invader";
	ShipLeviathan = "ship-leviathan";
	ShipAnubis = "ship-anubis";

	// Stations
	StationFarm = "station-farm";
	StationSolarPlant = "station-solar-plant";
	StationHabitation = "station-habitation";
	StationIceMine = "station-ice-mine";
	StationIronMine = "station-iron-mine";
	StationSilicaMine = "station-silica-mine";
	StationSteelworks = "station-steelworks";
	StationToolFactory = "station-tool-factory";
	StationHydrogenPump = "station-h2-pump";
	StationMethanePump = "station-ch4-pump";
	StationHeliumPump = "station-he3-pump";
	StationCarbonRefinery = "station-carbon-refinery";
	StationPlasticsRefinery = "station-plastics-refinery";
	StationArsenal = "station-arsenal";
	StationShipyard = "station-shipyard";
	StationHub = "station-hub";
	StationOutpost = "station-outpost";
	StationFoundry = "station-foundry";
	StationFusion = "station-tokamak";
	StationResearch = "station-research";
}

void UFlareScenarioTools::PostLoad()
{
	// Add the Farm as a world update if it didn't exist yet
	if (TheFarm && TheFarm->GetSectorStations().Num() == 0)
	{
		CreateTheFarm();
		PlayerCompany->DiscoverSector(TheFarm);
	}
}

void UFlareScenarioTools::GenerateEmptyScenario(bool RandomizeStationLocations, int32 EconomyIndex)
{
	FLOG("UFlareScenarioTools::GenerateEmptyScenario");
	SetupWorld(RandomizeStationLocations, EconomyIndex);
}

void UFlareScenarioTools::GenerateFighterScenario(bool RandomizeStationLocations, int32 EconomyIndex)
{
	FLOG("UFlareScenarioTools::GenerateFighterScenario");
	FFlareCompanyDescription* UnprotectedPlayerCompanyDescription = const_cast<FFlareCompanyDescription*>(PlayerCompany->GetDescription());
	UnprotectedPlayerCompanyDescription->StartingMoney += DEFAULT_PLAYER_MONEY;
	SetupWorld(RandomizeStationLocations, EconomyIndex);

	CreatePlayerShip(FirstLight, "ship-ghoul");
	GeneratePlayerStartingResearch();
}

void UFlareScenarioTools::GenerateFreighterScenario(bool RandomizeStationLocations, int32 EconomyIndex)
{
	FLOG("UFlareScenarioTools::GenerateFreighterScenario");
	FFlareCompanyDescription* UnprotectedPlayerCompanyDescription = const_cast<FFlareCompanyDescription*>(PlayerCompany->GetDescription());
	UnprotectedPlayerCompanyDescription->StartingMoney += DEFAULT_PLAYER_MONEY;

	SetupWorld(RandomizeStationLocations, EconomyIndex);

	GeneratePlayerStartingSectorKnowledge();
	CreatePlayerShip(FirstLight, "ship-solen");
	GeneratePlayerStartingResearch();
}

void UFlareScenarioTools::GenerateCustomScenario(int32 ScenarioIndex, bool RandomizeStationLocations, int32 EconomyIndex)
{
	FLOG("UFlareScenarioTools::GenerateCustomScenario");

	UFlareStartingScenarioCatalog* StartingScenarios = Game->GetStartingScenarios();
	TArray<UFlareStartingScenarioCatalogEntry*> ScenariosArray = StartingScenarios->GetStartingScenarios();
	UFlareStartingScenarioCatalogEntry* CurrentScenario = ScenariosArray[ScenarioIndex - 2];
	FFlareCompanyDescription* UnprotectedPlayerCompanyDescription = const_cast<FFlareCompanyDescription*>(PlayerCompany->GetDescription());

	if (CurrentScenario)
	{
		UnprotectedPlayerCompanyDescription->StartingMoney += CurrentScenario->StartingMoney;

		if (CurrentScenario->StartingResearchPoints >= 0)
		{
			UnprotectedPlayerCompanyDescription->StartingResearchPoints += CurrentScenario->StartingResearchPoints;
		}
		else
		{
			GeneratePlayerStartingResearch();
		}

		if (CurrentScenario->StartingSectorKnowledge.Num() > 0)
		{
			for (int KnowIndex = 0; KnowIndex < CurrentScenario->StartingSectorKnowledge.Num(); KnowIndex++)
			{
				FName CurrentSectorID = CurrentScenario->StartingSectorKnowledge[KnowIndex];
				UnprotectedPlayerCompanyDescription->StartingSectorKnowledge.Add(CurrentSectorID);
			}
		}
		else
		{
			GeneratePlayerStartingSectorKnowledge();
		}

		for (int KnowIndex = 0; KnowIndex < CurrentScenario->StartingTechnology.Num(); KnowIndex++)
		{
			FName CurrentTech = CurrentScenario->StartingTechnology[KnowIndex];
			UnprotectedPlayerCompanyDescription->StartingTechnology.Add(CurrentTech);
		}

		for (int ShipIndex = 0; ShipIndex < CurrentScenario->StartingShips.Num(); ShipIndex++)
		{
			FFlareCompanyStartingShips CurrentShips = CurrentScenario->StartingShips[ShipIndex];
			UnprotectedPlayerCompanyDescription->StartingShips.Add(CurrentShips);
		}
	}
	else
	{
		UnprotectedPlayerCompanyDescription->StartingMoney += DEFAULT_PLAYER_MONEY;
		GeneratePlayerStartingSectorKnowledge();
		CreatePlayerShip(FirstLight, "ship-solen");
		GeneratePlayerStartingResearch();
	}
	SetupWorld(RandomizeStationLocations, EconomyIndex);
}

void UFlareScenarioTools::GenerateDebugScenario(bool RandomizeStationLocations, int32 EconomyIndex)
{
	FLOG("UFlareScenarioTools::GenerateFreighterScenario");
	SetupWorld(RandomizeStationLocations, EconomyIndex);

	// Discover all sectors
	if (!PlayerData->QuestData.PlayTutorial)
	{
		for (int SectorIndex = 0; SectorIndex < World->GetSectors().Num(); SectorIndex++)
		{
			PlayerCompany->DiscoverSector(World->GetSectors()[SectorIndex],true);
		}
	}

	// Add more stuff
	CreatePlayerShip(MinersHome, "ship-omen");
	CreatePlayerShip(FrozenRealm, "ship-omen");

	FFlareStationSpawnParameters SpawnParameters;
	CreateStations(StationIceMine, PlayerCompany, ShoreOfIce, 1,1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationIceMine, PlayerCompany, MinersHome, 1,1, SpawnParameters, RandomizeStationLocations);
}

void UFlareScenarioTools::GeneratePlayerStartingResearch()
{
	int32 AutoPilotCost = PlayerCompany->GetTechnologyCostFromID("auto-docking");
	if (AutoPilotCost)
	{
		PlayerCompany->GiveResearch(AutoPilotCost);
	}
	else
	{
		PlayerCompany->GiveResearch(20);
	}
}

void UFlareScenarioTools::GeneratePlayerStartingSectorKnowledge()
{
	// Setup player sector knowledge
	PlayerCompany->DiscoverSector(TheDepths);
	PlayerCompany->DiscoverSector(BlueHeart);
	PlayerCompany->DiscoverSector(TheSpire);
	PlayerCompany->DiscoverSector(Outpost);
	PlayerCompany->DiscoverSector(MinersHome);
	PlayerCompany->DiscoverSector(NightsHome);
	PlayerCompany->DiscoverSector(TheFarm);
	PlayerCompany->DiscoverSector(Lighthouse);
}

UFlareSimulatedSpacecraft* UFlareScenarioTools::CreateRecoveryPlayerShip()
{
	return CreatePlayerShip(FirstLight, "ship-solen");
}

/*----------------------------------------------------
	Common world
----------------------------------------------------*/

void UFlareScenarioTools::SetupWorld(bool RandomizeStationLocations, int32 EconomyIndex)
{
	// Setup common stuff
	SetupAsteroids();

	if (PlayerData->QuestData.PlayStory == 0)
	{
		PlayerCompany->DiscoverSector(Pendulum);
	}

	// Discover public sectors
	SetupKnownSectors(MiningSyndicate);
	SetupKnownSectors(HelixFoundries);
	SetupKnownSectors(Sunwatch);
	SetupKnownSectors(MiningSyndicate);
	SetupKnownSectors(UnitedFarmsChemicals);
	SetupKnownSectors(IonLane);
	SetupKnownSectors(GhostWorksShipyards);
	SetupKnownSectors(NemaHeavyWorks);
	SetupKnownSectors(InfiniteOrbit);
	SetupKnownSectors(BrokenMoon);
	SetupKnownSectors(Quantalium);

	double Companymoneymultiplier = 1;
	double Populationmultiplier = 1;
	double Playermoneymultiplier = 1;
	double StationLevelBonus = EconomyIndex;

	switch (EconomyIndex)
	{
		case 1: // Developing
			Companymoneymultiplier = 1.20;
			Populationmultiplier = 1.15;
			break;
		case 2: // Prospering
			Companymoneymultiplier = 1.40;
			Populationmultiplier = 1.30;
			break;
		case 3: // Maturing
			Companymoneymultiplier = 1.60;
			Populationmultiplier = 1.45;
			break;
		case 4:  // Accomplished
			Companymoneymultiplier = 2.00;
			Populationmultiplier = 1.60;
			break;
	}

	switch (PlayerData->DifficultyId)
	{
		case -1: // Easy
			Companymoneymultiplier -= 0.05;
			Playermoneymultiplier = 1.25;
			break;
		case 0: // Normal
			break;
		case 1: // Hard
			Companymoneymultiplier += 0.05;
			Playermoneymultiplier = 0.75;
			break;
		case 2: // Very Hard
			Companymoneymultiplier += 0.10;
			Playermoneymultiplier = 0.50;
			break;
		case 3: // Expert
			Companymoneymultiplier += 0.25;
			Playermoneymultiplier = 0.25;
			break;
		case 4: // Unfair
			Companymoneymultiplier += 0.40;
			Playermoneymultiplier = 0.10;
			break;
	}

	// Company setup main
	MiningSyndicate->GiveMoney(100000000 * Companymoneymultiplier, FFlareTransactionLogEntry::LogInitialMoney());
	HelixFoundries->GiveMoney(100000000 * Companymoneymultiplier, FFlareTransactionLogEntry::LogInitialMoney());
	Sunwatch->GiveMoney(100000000 * Companymoneymultiplier, FFlareTransactionLogEntry::LogInitialMoney());
	UnitedFarmsChemicals->GiveMoney(100000000 * Companymoneymultiplier, FFlareTransactionLogEntry::LogInitialMoney());
	IonLane->GiveMoney(100000000 * Companymoneymultiplier, FFlareTransactionLogEntry::LogInitialMoney());
	GhostWorksShipyards->GiveMoney(100000000 * Companymoneymultiplier, FFlareTransactionLogEntry::LogInitialMoney());
	NemaHeavyWorks->GiveMoney(100000000 * Companymoneymultiplier, FFlareTransactionLogEntry::LogInitialMoney());

	// Company setup secondary
//	PlayerCompany->GiveMoney(750000 * Playermoneymultiplier, FFlareTransactionLogEntry::LogInitialMoney());
	Pirates->GiveMoney(1000000 * Companymoneymultiplier, FFlareTransactionLogEntry::LogInitialMoney());
	BrokenMoon->GiveMoney(1000000 * Companymoneymultiplier, FFlareTransactionLogEntry::LogInitialMoney());
	InfiniteOrbit->GiveMoney(1000000 * Companymoneymultiplier, FFlareTransactionLogEntry::LogInitialMoney());
	Quantalium->GiveMoney(1000000 * Companymoneymultiplier, FFlareTransactionLogEntry::LogInitialMoney());
	AxisSupplies->GiveMoney(1000000 * Companymoneymultiplier, FFlareTransactionLogEntry::LogInitialMoney());

	FFlareStationSpawnParameters SpawnParameters;

	for (int CompanyIndex = 0; CompanyIndex < Game->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
		UFlareCompany* Company = Game->GetGameWorld()->GetCompanies()[CompanyIndex];
		int64 StartingMoney = Company->GetDescription()->StartingMoney;
		int32 StartingResearchPoints = Company->GetDescription()->StartingResearchPoints;

		TArray<FName> StartingSectorKnowledge = Company->GetDescription()->StartingSectorKnowledge;
		TArray<FName> StartingTechnology = Company->GetDescription()->StartingTechnology;
		TArray<FFlareCompanyStartingShips> StartingShips = Company->GetDescription()->StartingShips;

		if (StartingMoney)
		{
			if (Company == PlayerCompany)
			{
				Company->GiveMoney(StartingMoney * Playermoneymultiplier, FFlareTransactionLogEntry::LogInitialMoney());
			}
			else
			{
				Company->GiveMoney(StartingMoney * Companymoneymultiplier, FFlareTransactionLogEntry::LogInitialMoney());
			}
		}

		if (StartingResearchPoints)
		{
			Company->GiveResearch(StartingResearchPoints);
		}

		if (StartingSectorKnowledge.Num() > 0)
		{
			for (int KnowIndex = 0; KnowIndex < StartingSectorKnowledge.Num(); KnowIndex++)
			{
				FName CurrentSectorID = StartingSectorKnowledge[KnowIndex];
				UFlareSimulatedSector* RealSector = World->FindSector(CurrentSectorID);
				if (RealSector != NULL)
				{
					Company->DiscoverSector(RealSector, true);
				}
				else
				{
					FFlareCelestialBody* CelestialBody = World->GetPlanerarium()->FindCelestialBody(CurrentSectorID);
					if (CelestialBody)
					{
						for (int32 SectorIndex = 0; SectorIndex < Game->GetGameWorld()->GetSectors().Num(); SectorIndex++)
						{
							UFlareSimulatedSector* Sector = Game->GetGameWorld()->GetSectors()[SectorIndex];
							if (Sector->GetOrbitParameters()->CelestialBodyIdentifier == CelestialBody->Identifier)
							{
								Company->DiscoverSector(Sector, true);
							}
						}
					}
				}
			}
		}

		if (StartingTechnology.Num() > 0)
		{
			for (int KnowIndex = 0; KnowIndex < StartingTechnology.Num(); KnowIndex++)
			{
				FName CurrentTech = StartingTechnology[KnowIndex];
				Company->UnlockTechnology(CurrentTech, false, true, true);
			}
		}

		if (StartingShips.Num() > 0)
		{
			for (int ShipIndex = 0; ShipIndex < StartingShips.Num(); ShipIndex++)
			{
				FFlareCompanyStartingShips CurrentShips = StartingShips[ShipIndex];
				UFlareSimulatedSector* RealSector = World->FindSector(CurrentShips.SpawnSector);
				if (RealSector != NULL)
				{
					if (CurrentShips.IsStation == true)
					{
						CreateStations(CurrentShips.ShipIdentifier, Company, RealSector, CurrentShips.Quantity, CurrentShips.Level + StationLevelBonus, SpawnParameters, RandomizeStationLocations,CurrentShips.StartVarianceMinInputs,CurrentShips.StartVarianceMaxInputs,CurrentShips.StartVarianceMinOutputs,CurrentShips.StartVarianceMaxOutputs);
					}
					else
					{
						UFlareSimulatedSpacecraft* InitialShip = CreateShips(CurrentShips.ShipIdentifier, Company, RealSector, CurrentShips.Quantity);
						if (CurrentShips.IsPlayerShip && Company == PlayerCompany && InitialShip)
						{
							PlayerData->LastFlownShipIdentifier = InitialShip->GetImmatriculation();
							PlayerData->PlayerFleetIdentifier = InitialShip->GetCurrentFleet()->GetIdentifier();
						}
					}
				}
			}
		}
	}

	/*
	//for testing
	CreateShips(ShipInvader, PlayerCompany, FirstLight, 1);
	CreateShips(ShipGhoul, PlayerCompany, FirstLight, 40);
	CreateShips(ShipSolen, PlayerCompany, FirstLight, 30);
	CreateShips(ShipOrca, MiningSyndicate, FirstLight, 35);

	CreateStations(StationFarm, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationSolarPlant, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationHabitation, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationIceMine, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationIronMine, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationSilicaMine, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationSteelworks, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationToolFactory, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationHydrogenPump, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationMethanePump, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationHeliumPump, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationCarbonRefinery, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationPlasticsRefinery, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationArsenal, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationHub, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationOutpost, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationFoundry, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationFusion, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationResearch, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationShipyard, PlayerCompany, FirstLight, 1, 1, SpawnParameters, RandomizeStationLocations);
	PlayerCompany->GiveResearch(999999999);
	PlayerCompany->GiveMoney(99999999999, FFlareTransactionLogEntry::LogInitialMoney());
	for (int SectorIndex = 0; SectorIndex < World->GetSectors().Num(); SectorIndex++)
	{
		PlayerCompany->DiscoverSector(World->GetSectors()[SectorIndex],true);
	}
	//for testing
	*/
	// Give technology
	IonLane->UnlockTechnology("stations", false, true);
	IonLane->UnlockTechnology("chemicals", false, true);

	MiningSyndicate->UnlockTechnology("stations", false, true);
	MiningSyndicate->UnlockTechnology("mining", false, true);

	UnitedFarmsChemicals->UnlockTechnology("stations", false, true);
	UnitedFarmsChemicals->UnlockTechnology("orbital-pumps", false, true);
	UnitedFarmsChemicals->UnlockTechnology("chemicals", false, true);
	
	Sunwatch->UnlockTechnology("instruments", false, true);
	Sunwatch->UnlockTechnology("stations", false, true);
	Sunwatch->UnlockTechnology("metallurgy", false, true);

	NemaHeavyWorks->UnlockTechnology("stations", false, true);
	NemaHeavyWorks->UnlockTechnology("metallurgy", false, true);
	NemaHeavyWorks->UnlockTechnology("orbital-pumps", false, true);

	AxisSupplies->UnlockTechnology("stations", false, true);
	AxisSupplies->UnlockTechnology("metallurgy", false, true);

	HelixFoundries->UnlockTechnology("stations", false, true);
	HelixFoundries->UnlockTechnology("metallurgy", false, true);
	
	GhostWorksShipyards->UnlockTechnology("stations", false, true);
	GhostWorksShipyards->UnlockTechnology("mining", false, true);
	GhostWorksShipyards->UnlockTechnology("chemicals", false, true);
	GhostWorksShipyards->UnlockTechnology("metallurgy", false, true);
	GhostWorksShipyards->UnlockTechnology("shipyard-station", false, true);
	
	Pirates->UnlockTechnology("pirate-tech", false, true);
	Pirates->UnlockTechnology("quick-repair", false, true);

	BrokenMoon->UnlockTechnology("quick-repair", false, true);
	BrokenMoon->UnlockTechnology("stations", false, true);

	InfiniteOrbit->UnlockTechnology("fast-travel", false, true);
	InfiniteOrbit->UnlockTechnology("stations", false, true);

	Quantalium->UnlockTechnology("stations", false, true);
	Quantalium->UnlockTechnology("mining", false, true);
	Quantalium->UnlockTechnology("metallurgy", false, true);

	// Population setup
	if (BlueHeart)
	{
		BlueHeart->GetPeople()->GiveBirth(4000 * Populationmultiplier);
	}
	if (NightsHome)
	{
		NightsHome->GetPeople()->GiveBirth(2000 * Populationmultiplier);
	}
	if (FrozenRealm)
	{
		FrozenRealm->GetPeople()->GiveBirth(2000 * Populationmultiplier);
	}
	if (MinersHome)
	{
		MinersHome->GetPeople()->GiveBirth(2000 * Populationmultiplier);
	}
	if (TheForge)
	{
		TheForge->GetPeople()->GiveBirth(2000 * Populationmultiplier);
	}
	if (BlueShores)
	{
		BlueShores->GetPeople()->GiveBirth(1000 * Populationmultiplier);
	}
	
	// The Depths (ice mines)
	
	CreateStations(StationIceMine, MiningSyndicate, TheDepths, 4,1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationIceMine, GhostWorksShipyards, TheDepths, 1,1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);

	// Lighthouse (food and power)
	CreateStations(StationFarm, UnitedFarmsChemicals, Lighthouse, 3, 2 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationFarm, IonLane, Lighthouse, 1, 2 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationSolarPlant, Sunwatch, Lighthouse, 3, 3 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationSolarPlant, IonLane, Lighthouse, 1, 2 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationResearch, IonLane, Lighthouse, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);

	// Miner's Home (mines)
	CreateStations(StationHabitation, MiningSyndicate, MinersHome, 1, 2 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationHabitation, MiningSyndicate, TheDepths, 1, 2 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);

	CreateStations(StationIronMine, MiningSyndicate, MinersHome, 4, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationIronMine, MiningSyndicate, MinersHome, 2, 2 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationSteelworks, NemaHeavyWorks, MinersHome, 2, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationToolFactory, NemaHeavyWorks, MinersHome, 1, 2 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);

	// The Spire (pumps, refineries)
	CreateStations(StationMethanePump, UnitedFarmsChemicals, TheSpire, 2, 2 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationHydrogenPump, NemaHeavyWorks, TheSpire, 3, 2 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationHeliumPump, IonLane, TheSpire, 1, 2 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);

	// Blue Shores (refineries)
	CreateStations(StationHabitation, MiningSyndicate, MinersHome, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationPlasticsRefinery, NemaHeavyWorks, BlueShores, 1, 2 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationCarbonRefinery, UnitedFarmsChemicals, BlueShores, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationToolFactory, NemaHeavyWorks, BlueShores, 1, 2 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationResearch, Sunwatch, BlueShores, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);

	// Blue Heart
	CreateBlueHeart(StationLevelBonus);
//	CreateStations(StationCarbonRefinery, UnitedFarmsChemicals, BlueHeart, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationPlasticsRefinery, UnitedFarmsChemicals, BlueHeart, 1, 2 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);

	// The Farm
	CreateTheFarm(StationLevelBonus);

	// Anka HFR factory
	CreateStations(StationSteelworks, HelixFoundries, TheForge, 2, 2 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationSteelworks, HelixFoundries, TheForge, 2, 2 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationToolFactory, HelixFoundries, TheForge, 3, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationHabitation, Sunwatch, TheForge, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationHabitation, Sunwatch, Crossroads, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationHabitation, Sunwatch, TheDig, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);

	// Laboratories
	CreateStations(StationResearch, Quantalium, Crossroads, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationFoundry, Quantalium, Crossroads, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);

	// Anka bases for Broken Moon
	CreateStations(StationOutpost, BrokenMoon, Crossroads, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	
	// Night's Home
	CreateNightsHome(StationLevelBonus);
	
	// Hela secondary economy
	CreateStations(StationArsenal, AxisSupplies, FrozenRealm, 1, 2 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationFarm, GhostWorksShipyards, FrozenRealm, 2, 2 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationSolarPlant, GhostWorksShipyards, FrozenRealm, 2, 3 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationHub, GhostWorksShipyards, FrozenRealm, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationIceMine, GhostWorksShipyards, ShoreOfIce, 3, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationIceMine, MiningSyndicate, ShoreOfIce, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	
	// Boneyard
	Pirates->DiscoverSector(Boneyard, true);
	Pirates->DiscoverSector(Decay, true);
	Pirates->DiscoverSector(Daedalus, true);

	CreateBoneyard(StationLevelBonus);
	CreateStations(StationSolarPlant, Pirates, Boneyard, 2, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	SetupKnownSectors(Pirates);

	// Create hubs
	CreateStations(StationHub, IonLane, Crossroads, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationHub, IonLane, TheForge, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationHub, IonLane, TheSpire, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationHub, IonLane, Lighthouse, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationHub, IonLane, BlueHeart, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationHub, IonLane, MinersHome, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationHub, IonLane, NightsHome, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);

	// Create outposts
	CreateStations(StationOutpost, AxisSupplies, TheDepths, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationOutpost, AxisSupplies, MinersHome, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationOutpost, AxisSupplies, Lighthouse, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationOutpost, AxisSupplies, BlueHeart, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationOutpost, AxisSupplies, BlueShores, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationOutpost, AxisSupplies, TheSpire, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationOutpost, AxisSupplies, TheForge, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationOutpost, AxisSupplies, Crossroads, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationOutpost, AxisSupplies, TheDig, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationOutpost, AxisSupplies, FrozenRealm, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationOutpost, AxisSupplies, WinterJunction, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);
	CreateStations(StationOutpost, AxisSupplies, Tranquility, 1, 1 + StationLevelBonus, SpawnParameters, RandomizeStationLocations);

	// Create cargos
	// 48 solens (33 vanilla)
	CreateShips(ShipSolen, AxisSupplies, BlueHeart, 4);
	CreateShips(ShipSolen, BrokenMoon, Crossroads, 4);
	CreateShips(ShipSolen, GhostWorksShipyards, NightsHome, 4);
	CreateShips(ShipSolen, HelixFoundries, TheForge, 4);
	CreateShips(ShipSolen, InfiniteOrbit, Tranquility, 4);
	CreateShips(ShipSolen, IonLane, Lighthouse, 4);
	CreateShips(ShipSolen, MiningSyndicate, MinersHome, 4);
	CreateShips(ShipSolen, NemaHeavyWorks, MinersHome, 4);
	CreateShips(ShipSolen, Pirates, Boneyard, 4);
	CreateShips(ShipSolen, Quantalium, Crossroads, 4);
	CreateShips(ShipSolen, Sunwatch, Lighthouse, 4);
	CreateShips(ShipSolen, UnitedFarmsChemicals, TheSpire, 4);
	// 10 omens (10 vanilla)
	CreateShips(ShipOmen, IonLane, MinersHome, 4);
	CreateShips(ShipOmen, IonLane, FrozenRealm, 1);
	CreateShips(ShipOmen, UnitedFarmsChemicals, TheSpire, 2);
	CreateShips(ShipOmen, HelixFoundries, TheForge, 2);
	CreateShips(ShipOmen, InfiniteOrbit, Tranquility, 1);

	// Create military ships
	CreateShips(ShipGhoul, IonLane, MinersHome, 2);

	CreateShips(ShipGhoul, MiningSyndicate, MinersHome, 3);
	CreateShips(ShipOrca, MiningSyndicate, MinersHome, 2);

	CreateShips(ShipGhoul, NemaHeavyWorks, BlueHeart, 4);
	CreateShips(ShipInvader, NemaHeavyWorks, BlueHeart, 1);

	CreateShips(ShipGhoul, HelixFoundries, TheForge, 2);
	CreateShips(ShipDragon, HelixFoundries, TheForge, 1);

	CreateShips(ShipGhoul, GhostWorksShipyards, NightsHome, 2);
	CreateShips(ShipInvader, GhostWorksShipyards, NightsHome, 1);

	CreateShips(ShipGhoul, Pirates, Boneyard, 5);
	CreateShips(ShipDragon, Pirates, Boneyard, 1);
	CreateShips(ShipGhoul, BrokenMoon, Colossus, 2);

	if (EconomyIndex >= 1) // Developing
	{
		CreateStations(StationArsenal, BrokenMoon, Crossroads, 1, 1 + (StationLevelBonus - 1), SpawnParameters, RandomizeStationLocations);

		CreateStations(StationHabitation, InfiniteOrbit, BlueHeart, 1, 1 + (StationLevelBonus - 1), SpawnParameters, RandomizeStationLocations);
		CreateStations(StationSteelworks, NemaHeavyWorks, MinersHome, 1, 1 + (StationLevelBonus - 1), SpawnParameters, RandomizeStationLocations);
		CreateStations(StationSteelworks, HelixFoundries, TheForge, 1, 1 + (StationLevelBonus - 1), SpawnParameters, RandomizeStationLocations);
		CreateStations(StationPlasticsRefinery, NemaHeavyWorks, BlueShores, 1, 1 + (StationLevelBonus - 1), SpawnParameters, RandomizeStationLocations);
		CreateStations(StationPlasticsRefinery, UnitedFarmsChemicals, BlueHeart, 1, 1 + (StationLevelBonus - 1), SpawnParameters, RandomizeStationLocations);

		CreateShips(ShipSolen, GhostWorksShipyards, NightsHome, 4);
		CreateShips(ShipSolen, IonLane, Lighthouse, 4);
		CreateShips(ShipSolen, MiningSyndicate, MinersHome, 2);
		CreateShips(ShipSolen, NemaHeavyWorks, MinersHome, 2);
		CreateShips(ShipSolen, UnitedFarmsChemicals, TheSpire, 4);
		CreateShips(ShipSolen, Sunwatch, Lighthouse, 5);
		CreateShips(ShipSolen, HelixFoundries, TheForge, 4);
		CreateShips(ShipSolen, Pirates, Boneyard, 2);
		CreateShips(ShipSolen, Quantalium, Crossroads, 3);
		CreateShips(ShipSolen, InfiniteOrbit, Tranquility, 3);
		CreateShips(ShipSolen, BrokenMoon, Colossus, 1);
	}

	if (EconomyIndex >= 2) // Prospering
	{
		CreateStations(StationHabitation, InfiniteOrbit, NightsHome, 1, 1 + (StationLevelBonus - 2), SpawnParameters, RandomizeStationLocations);
		CreateStations(StationFoundry, Quantalium, TheSpire, 1, 1 + (StationLevelBonus - 2), SpawnParameters, RandomizeStationLocations);
		CreateStations(StationFusion, Sunwatch, TheDepths, 1, 1 + (StationLevelBonus - 2), SpawnParameters, RandomizeStationLocations);

		CreateShips(ShipSolen, AxisSupplies, BlueHeart, 2);
		CreateShips(ShipSolen, Quantalium, Crossroads, 2);
		CreateShips(ShipSolen, BrokenMoon, Colossus, 1);

		CreateShips(ShipOmen, IonLane, FrozenRealm, 4);
		CreateShips(ShipOmen, UnitedFarmsChemicals, TheSpire, 2);
		CreateShips(ShipOmen, HelixFoundries, TheForge, 2);
		CreateShips(ShipOmen, InfiniteOrbit, Tranquility, 4);
	}

	if (EconomyIndex >= 3) // Maturing
	{
		CreateStations(StationHabitation, InfiniteOrbit, TheForge, 1, 1 + (StationLevelBonus - 3), SpawnParameters, RandomizeStationLocations);
		CreateStations(StationFusion, Sunwatch, TheSpire, 1, 1 + (StationLevelBonus - 3), SpawnParameters, RandomizeStationLocations);
		CreateStations("station-shipyard", GhostWorksShipyards, TheForge, 1, 1, SpawnParameters, RandomizeStationLocations,0.50,1.0,0.50,1);
		CreateShips(ShipOmen, AxisSupplies, BlueHeart, 1);
		CreateShips(ShipOmen, Quantalium, Crossroads, 2);
		CreateShips(ShipOmen, BrokenMoon, Colossus, 2);
		CreateShips(ShipSphinx, IonLane, Tranquility, 1);
		CreateShips(ShipSphinx, InfiniteOrbit, Tranquility, 1);
	}
	if (EconomyIndex >= 4) // Accomplished
	{
		CreateStations(StationHabitation, InfiniteOrbit, MinersHome, 1, 1 + (StationLevelBonus - 4), SpawnParameters, RandomizeStationLocations);
		CreateShips(ShipOmen, Pirates, Boneyard, 1);
		CreateShips(ShipSphinx, Quantalium, Tranquility, 1);
		CreateShips(ShipSphinx, BrokenMoon, Tranquility, 1);
		CreateShips(ShipSphinx, GhostWorksShipyards, BlueHeart, 1);
	}

	if (PlayerData->DifficultyId>=1) // Hard
	{
		CreateShips(ShipGhoul, Pirates, Boneyard, 3);
		CreateShips(ShipGhoul, BrokenMoon, Colossus, 2);
	}
	if (PlayerData->DifficultyId >= 2) // Very Hard
	{
		CreateShips(ShipGhoul, Pirates, Boneyard, 3);
		CreateShips(ShipOrca, BrokenMoon, Colossus, 2);
	}
	if (PlayerData->DifficultyId >= 3) // Expert
	{
		CreateShips(ShipOrca, Pirates, Boneyard, 3);
		CreateShips(ShipPhalanx, BrokenMoon, Colossus, 2);
	}
	if (PlayerData->DifficultyId >= 4)  // Unfair
	{
		CreateShips(ShipPhalanx, Pirates, Boneyard, 2);
		CreateShips(ShipKami, Pirates, Boneyard, 1);
		CreateShips(ShipAnubis, NemaHeavyWorks, BlueHeart, 1);
		CreateShips(ShipAnubis, GhostWorksShipyards, NightsHome, 1);
		CreateShips(ShipInvader, Quantalium, NightsHome, 1);
	}

	for (int CompanyIndex = 0; CompanyIndex < Game->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
		UFlareCompany* Company = Game->GetGameWorld()->GetCompanies()[CompanyIndex];
		Company->GiveAllStationSectorLicenses();
		Company->GetAI()->GetData()->CalculatedDefaultBudget = true;
	}

}

void UFlareScenarioTools::SetupAsteroids()
{
	CreateAsteroids(FirstLight, 31, FVector(30, 7, 9));
	CreateAsteroids(MinersHome, 60, FVector(75, 15, 13));
	CreateAsteroids(TheDepths, 39, FVector(35, 5, 10));

	CreateAsteroids(Outpost, 47, FVector(49, 16, 8));
	CreateAsteroids(TheForge, 32, FVector(51, 12, 9));
	CreateAsteroids(TheDig, 42, FVector(47, 13, 20));

	CreateAsteroids(ShoreOfIce, 42, FVector(55, 9, 17));
	CreateAsteroids(Ruins, 45, FVector(38, 7, 9));

	CreateAsteroids(Boneyard, 38, FVector(37, 18, 7));

	CreateAsteroids(Serenity, 27, FVector(42, 9, 7));
}

void UFlareScenarioTools::SetupKnownSectors(UFlareCompany* Company)
{
	if (Company)
	{
		// Nema
		Company->DiscoverSector(TheDepths,true);
		Company->DiscoverSector(MinersHome, true);
		Company->DiscoverSector(BlueHeart, true);
		Company->DiscoverSector(Lighthouse, true);
		Company->DiscoverSector(BlueShores, true);
		Company->DiscoverSector(TheSpire, true);
		Company->DiscoverSector(TheFarm, true);
		// Unknown : FirstLight, Anomaly, Pendulum

		// Anka
		Company->DiscoverSector(Crossroads, true);
		Company->DiscoverSector(TheDig, true);
		Company->DiscoverSector(TheForge, true);
		// Unknown : Colossus, Outpost

		// Hela
		Company->DiscoverSector(NightsHome, true);
		Company->DiscoverSector(FrozenRealm, true);
		Company->DiscoverSector(WinterJunction, true);
		// Unknown : Ruins, ShoreOfIce

		// Asta
		// Unknown : Decay, Boneyard, Daedalus

		// Adena
		Company->DiscoverSector(Tranquility, true);
		// Unknown : Serenity, Solitude, Pharos
	}
}


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

UFlareSimulatedSpacecraft* UFlareScenarioTools::CreatePlayerShip(UFlareSimulatedSector* Sector, FName Class)
{
	UFlareSimulatedSpacecraft* InitialShip = nullptr;

	if (Sector)
	{
		InitialShip = Sector->CreateSpacecraft(Class, PlayerCompany, FVector::ZeroVector);
		PlayerData->LastFlownShipIdentifier = InitialShip->GetImmatriculation();
		PlayerData->PlayerFleetIdentifier = InitialShip->GetCurrentFleet()->GetIdentifier();
	}

	return InitialShip;
}

void UFlareScenarioTools::CreateAsteroids(UFlareSimulatedSector* Sector, int32 Count, FVector DistributionShape)
{
	if (Sector)
	{
		FLOGV("UFlareScenarioTools::CreateAsteroids : Trying to spawn %d asteroids", Count);

		// Compute parameters
		float MaxAsteroidDistance = 15000;
		int32 AsteroidCount = 0;
		int32 CellCount = DistributionShape.X * DistributionShape.Y * DistributionShape.Z * 4;
		int32 FailCount = 0;

		while (AsteroidCount < Count && FailCount < 5000)
		{
			for (int32 X = -DistributionShape.X; X <= DistributionShape.X; X++)
			{
				for (int32 Y = -DistributionShape.Y; Y <= DistributionShape.Y; Y++)
				{
					for (int32 Z = -DistributionShape.Z; Z <= DistributionShape.Z; Z++)
					{
						if (FMath::RandHelper(CellCount) <= Count)
						{
							bool CanSpawn = true;
							FVector AsteroidLocation = MaxAsteroidDistance * FVector(X, Y, Z);

							// Check for collision
							TArray<FFlareAsteroidSave> Asteroids = Sector->Save()->AsteroidData;
							for (int32 Index = 0; Index < Asteroids.Num(); Index++)
							{
								if ((Asteroids[Index].Location - AsteroidLocation).Size() < MaxAsteroidDistance)
								{
									CanSpawn = false;
									break;
								}
							}

							// Spawn the asteroid
							if (CanSpawn)
							{
								FString AsteroidName = FString("asteroid") + FString::FromInt(AsteroidCount);
								int32 AsteroidCatalogCount = Game->GetAsteroidCatalog() ? Game->GetAsteroidCatalog()->Asteroids.Num() : 0;
								Sector->CreateAsteroid(FMath::RandRange(0, AsteroidCatalogCount - 1), FName(*AsteroidName), AsteroidLocation);
								AsteroidCount++;
							}
							else
							{
								FailCount++;
							}
						}
					}
				}
			}
		}

		FLOGV("UFlareScenarioTools::CreateAsteroids : Spawned %d asteroids", AsteroidCount);
	}
}

UFlareSimulatedSpacecraft* UFlareScenarioTools::CreateShips(FName ShipClass, UFlareCompany* Company, UFlareSimulatedSector* Sector, uint32 Count)
{
	UFlareSimulatedSpacecraft* InitialShip = nullptr;
	if (Sector && Company)
	{
		for (uint32 Index = 0; Index < Count; Index++)
		{
			if (InitialShip == nullptr)
			{
				InitialShip = Sector->CreateSpacecraft(ShipClass, Company, FVector::ZeroVector);
			}
			else
			{
				Sector->CreateSpacecraft(ShipClass, Company, FVector::ZeroVector);
			}
		}
	}
	return InitialShip;
}

/*
UFlareSimulatedSpacecraft* UFlareScenarioTools::CreatePlayerShip(UFlareSimulatedSector* Sector, FName Class)
{
	UFlareSimulatedSpacecraft* InitialShip = nullptr;

	if (Sector)
	{
		InitialShip = Sector->CreateSpacecraft(Class, PlayerCompany, FVector::ZeroVector);
		PlayerData->LastFlownShipIdentifier = InitialShip->GetImmatriculation();
		PlayerData->PlayerFleetIdentifier = InitialShip->GetCurrentFleet()->GetIdentifier();
	}

	return InitialShip;
}
*/

TArray<UFlareSimulatedSector*> UFlareScenarioTools::GetRandomAllowedSectors(FName StationClass, UFlareCompany* Company) const
{
	FFlareSpacecraftDescription* Desc = Game->GetSpacecraftCatalog()->Get(StationClass);
	TArray<UFlareSimulatedSector*> AllowedSectors;
	if (Desc)
	{
		TArray<FText> Reasons;
		for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
		{
			UFlareSimulatedSector* CheckingSector = Company->GetKnownSectors()[SectorIndex];
			if (CheckingSector->CanBuildStation(Desc, Company, Reasons, true, false, false))
			{
				int32 MaximumAllowedCompany = 7;

				if (StationClass == StationHabitation || StationClass == StationOutpost || StationClass == StationHub)
				{
					MaximumAllowedCompany = 1;
				}

				// Can't be TOO random, it's not ideal for certain station types

				int32 FoundSame = 0;
				int32 FoundTotal = 0;
				for (UFlareSimulatedSpacecraft* Spacecraft : CheckingSector->GetSectorSpacecrafts())
				{
					FoundTotal++;
					if (Spacecraft->IsStation())
					{
						if (Spacecraft->GetDescription() == Desc && Spacecraft->GetCompany() == Company)
						{
							FoundSame++;
						}
					}
				}
				if (FoundSame < MaximumAllowedCompany && FoundTotal < 14)
				{
					AllowedSectors.Add(CheckingSector);
				}
			}
		}
	}
	return AllowedSectors;
}

void UFlareScenarioTools::CreateStations(FName StationClass, UFlareCompany* Company, UFlareSimulatedSector* Sector, uint32 Count, int32 Level, FFlareStationSpawnParameters SpawnParameters, bool RandomLocation, float VarianceMinInputs, float VarianceMaxInputs, float VarianceMinOutputs, float VarianceMaxOutputs)
{
	if (Sector && Company)
	{
		UFlareSimulatedSector* OriginalSector = Sector;

		for (uint32 Index = 0; Index < Count; Index++)
		{
			FFlareSpacecraftDescription* Desc = Game->GetSpacecraftCatalog()->Get(StationClass);
			if (!Desc)
			{
				Desc = Game->GetSpacecraftCatalog()->Get(FName(*("station-" + StationClass.ToString())));
				if (!Desc)
				{
					continue;
				}
			}

			if (Level > Desc->MaxLevel)
			{
				Level = Desc->MaxLevel;
			}

			if (RandomLocation)
			{
				TArray<UFlareSimulatedSector*> AllowedSectors=GetRandomAllowedSectors(StationClass,Company);
				if (AllowedSectors.Num() > 0)
				{
					Sector = AllowedSectors[FMath::RandRange(0, AllowedSectors.Num() - 1)];
				}
				else
				{
					Sector = OriginalSector;
				}
			}

			UFlareSimulatedSpacecraft* Station = Sector->CreateStation(StationClass, Company, false, SpawnParameters, Level);
			if (!Station)
			{
				continue;
			}

			if (Station->GetFactories().Num() > 0)
			{
				TArray<FName> InputIDS;
				TArray<FName> OutputIDS;
				for (UFlareFactory* ActiveFactory : Station->GetFactories())
				{
					// Give input resources
					for (int32 ResourceIndex = 0; ResourceIndex < ActiveFactory->GetDescription()->CycleCost.InputResources.Num(); ResourceIndex++)
					{
						const FFlareFactoryResource* Resource = &ActiveFactory->GetDescription()->CycleCost.InputResources[ResourceIndex];
						if (InputIDS.Find(Resource->Resource->Data.Identifier) != INDEX_NONE)
						{
							continue;
						}
						float StartRatio = FMath::FRandRange(VarianceMinInputs, VarianceMaxInputs);
						Station->GetActiveCargoBay()->GiveResources(&Resource->Resource->Data, Station->GetActiveCargoBay()->GetSlotCapacity() * StartRatio, Company);
						InputIDS.Add(Resource->Resource->Data.Identifier);
					}

					// Give output resources
					for (int32 ResourceIndex = 0; ResourceIndex < ActiveFactory->GetDescription()->CycleCost.OutputResources.Num(); ResourceIndex++)
					{
						const FFlareFactoryResource* Resource = &ActiveFactory->GetDescription()->CycleCost.OutputResources[ResourceIndex];
						if (OutputIDS.Find(Resource->Resource->Data.Identifier) != INDEX_NONE)
						{
							continue;
						}
						float StartRatio = FMath::FRandRange(VarianceMinOutputs, VarianceMaxOutputs);
						Station->GetActiveCargoBay()->GiveResources(&Resource->Resource->Data, Station->GetActiveCargoBay()->GetSlotCapacity() * StartRatio, Company);
						OutputIDS.Add(Resource->Resource->Data.Identifier);
					}
				}
			}

			// Give customer resources
			if (Station->HasCapability(EFlareSpacecraftCapability::Consumer))
			{
				for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->ConsumerResources.Num(); ResourceIndex++)
				{
					FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->ConsumerResources[ResourceIndex]->Data;
					float StartRatio = FMath::FRandRange(0.25, 0.75);
					Station->GetActiveCargoBay()->GiveResources(Resource, Station->GetActiveCargoBay()->GetSlotCapacity() * StartRatio, Company);
				}
			}

			// Give customer resources
			if (Station->HasCapability(EFlareSpacecraftCapability::Maintenance))
			{
				for (int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->MaintenanceResources.Num(); ResourceIndex++)
				{
					FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->MaintenanceResources[ResourceIndex]->Data;
					float StartRatio = FMath::FRandRange(0.25, 0.75);
					Station->GetActiveCargoBay()->GiveResources(Resource, Station->GetActiveCargoBay()->GetSlotCapacity() * StartRatio, Company);
				}
			}
		}
	}
}

void UFlareScenarioTools::CreateBlueHeart(double StationLevelBonus)
{
	if (BlueHeart && NemaHeavyWorks && UnitedFarmsChemicals && AxisSupplies)
	{
		float StationRadius = 50000;
		FVector UpVector(0, 0, 1);
		FVector BaseLocation = FVector(-200000.0, 0, 0);
		FFlareStationSpawnParameters StationParams;
		StationParams.AttachActorName = FName("BlueHeartCore");

		// BH Shipyard
		StationParams.Location = BaseLocation + FVector(StationRadius, 0, 0);
		StationParams.Rotation = FRotator::ZeroRotator;
		CreateStations("station-bh-shipyard", NemaHeavyWorks, BlueHeart, 1, 1, StationParams,false,0.50,1,0.50,1);

		// BH Arsenal
		StationParams.Location = BaseLocation + FVector(StationRadius, 0, 0).RotateAngleAxis(30, UpVector);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(0, 0, 30));
		CreateStations("station-bh-arsenal", AxisSupplies, BlueHeart, 1, 1 + StationLevelBonus, StationParams);

		// BH Hub
		StationParams.Location = BaseLocation + FVector(StationRadius, 0, 0).RotateAngleAxis(-30, UpVector);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(0, 0, -30));
		CreateStations("station-bh-hub", IonLane, BlueHeart, 1, 1 + StationLevelBonus, StationParams);

		// BH Habitation 1
		StationParams.Location = BaseLocation + FVector(StationRadius + 600, 0, 7168);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(180, 0, 0));
		CreateStations("station-bh-habitation", NemaHeavyWorks, BlueHeart, 1, 1 + StationLevelBonus, StationParams);

		// BH Habitation 2
		StationParams.Location = BaseLocation + FVector(StationRadius + 600, 0, -7168);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(0, 0, 0));
		CreateStations("station-bh-habitation", UnitedFarmsChemicals, BlueHeart, 1, 1 + StationLevelBonus, StationParams);

		FFlareStationSpawnParameters DummyStationParams;
		CreateStations(StationCarbonRefinery, UnitedFarmsChemicals, BlueHeart, 1, 2 + StationLevelBonus, DummyStationParams);
		CreateStations(StationPlasticsRefinery, UnitedFarmsChemicals, BlueHeart, 1, 2 + StationLevelBonus, DummyStationParams);
	}
}

void UFlareScenarioTools::CreateBoneyard(double StationLevelBonus)
{
	if (Boneyard && Pirates)
	{
		float CoreLength = 4096;
		float BoneLength = -16384;
		float BoneHeight = 8192;
		FVector FrontVector(1, 0, 0);
		FVector BaseLocation = FVector(-200000.0, 0, 0);
		FFlareStationSpawnParameters StationParams;
		StationParams.AttachActorName = FName("BoneyardCore");

		// BY Shipyard
		StationParams.Location = BaseLocation + FVector(CoreLength, 0, 0);
		StationParams.Rotation = FRotator::ZeroRotator;
		CreateStations("station-by-shipyard", Pirates, Boneyard, 1, 1, StationParams,false,0.50,1,0.50,1);

		// BY Arsenal
		StationParams.Location = BaseLocation + FVector(BoneLength, 0, BoneHeight).RotateAngleAxis(120, FrontVector);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(0, 210, 90));
		CreateStations("station-by-arsenal", Pirates, Boneyard, 1, 1 + StationLevelBonus, StationParams);

		// BY Hub
		StationParams.Location = BaseLocation + FVector(2 * BoneLength, 0, BoneHeight).RotateAngleAxis(-120, FrontVector);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(0, -150, -90));
		CreateStations("station-by-hub", Pirates, Boneyard, 1, 1 + StationLevelBonus, StationParams);

		// BY Habitation
		StationParams.Location = BaseLocation + FVector(3 * BoneLength, 0, BoneHeight);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(180, 90, 0));
		CreateStations("station-by-habitation", Pirates, Boneyard, 1, 1 + StationLevelBonus, StationParams);
	}
}

void UFlareScenarioTools::CreateNightsHome(double StationLevelBonus)
{
	if (NightsHome && GhostWorksShipyards && AxisSupplies)
	{
		float StationRadius = 22480;
		FVector UpVector(0, 0, 1);
		FVector BaseLocation = FVector(-3740, 0, -153600);
		FFlareStationSpawnParameters StationParams;
		StationParams.AttachActorName = FName("NightsHomeCore");

		// NH Arsenal
		StationParams.Location = BaseLocation + FVector(StationRadius, 0, 0).RotateAngleAxis(-135, UpVector);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(90, 0, -135));
		CreateStations("station-nh-arsenal", AxisSupplies, NightsHome, 1, 2 + StationLevelBonus, StationParams);

		// NH Shipyard
		StationParams.Location = BaseLocation + FVector(StationRadius, 0, 0).RotateAngleAxis(180, UpVector);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(0, 0, 180));
		CreateStations("station-nh-shipyard", GhostWorksShipyards, NightsHome, 1, 1, StationParams,false,0.50,1,0.50,1);

		// NH Habitation
		StationParams.Location = BaseLocation + FVector(StationRadius, 0, 0).RotateAngleAxis(135, UpVector);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(-90, 0, 135));
		CreateStations("station-nh-habitation", GhostWorksShipyards, NightsHome, 1, 1 + StationLevelBonus, StationParams);
	}
}

void UFlareScenarioTools::CreateTheFarm(double StationLevelBonus)
{
	if (TheFarm && UnitedFarmsChemicals && AxisSupplies && Sunwatch)
	{
		float StationLength = 48640 + 2048 + 2000; // cap base + cap offset + station offset
		float StationRadius = 24576;
		FVector BaseLocation = FVector(-200000.0, 0, 0);
		FFlareStationSpawnParameters StationParams;
		StationParams.AttachActorName = FName("FarmCore");

		// TF Arsenal
		StationParams.Location = BaseLocation + FVector(StationRadius, StationLength, 0);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(-90, 0, 90));
		CreateStations("station-tf-arsenal", AxisSupplies, TheFarm, 1, 1 + StationLevelBonus, StationParams);

		// TF Hub
		StationParams.Location = BaseLocation + FVector(-StationRadius, StationLength, 0);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(90, 0, 90));
		CreateStations("station-tf-hub", AxisSupplies, TheFarm, 1, 1 + StationLevelBonus, StationParams);

		// TF Habitation
		StationParams.Location = BaseLocation + FVector(0, StationLength, StationRadius);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(0, 0, 90));
		CreateStations("station-tf-habitation", UnitedFarmsChemicals, TheFarm, 1, 1 + StationLevelBonus, StationParams);

		// TF Power plant
		StationParams.Location = BaseLocation + FVector(0, StationLength, -StationRadius);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(180, 0, 90));
		CreateStations("station-tf-solar-plant", Sunwatch, TheFarm, 1, 1 + StationLevelBonus, StationParams);

		// TF Farm 1
		StationParams.Location = BaseLocation + FVector(0, -StationLength, StationRadius);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(0, 0, -90));
		CreateStations("station-tf-farm", Sunwatch, TheFarm, 1, 1 + StationLevelBonus, StationParams);

		// TF Farm 2
		StationParams.Location = BaseLocation + FVector(0, -StationLength, -StationRadius);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(180, 0, -90));
		CreateStations("station-tf-farm", Sunwatch, TheFarm, 1, 1 + StationLevelBonus, StationParams);

		// TF Farm 3
		StationParams.Location = BaseLocation + FVector(StationRadius, -StationLength, 0);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(90, 0, -90));
		CreateStations("station-tf-farm", Sunwatch, TheFarm, 1, 1 + StationLevelBonus, StationParams);

		// TF Farm 4
		StationParams.Location = BaseLocation + FVector(-StationRadius, -StationLength, 0);
		StationParams.Rotation = FRotator::MakeFromEuler(FVector(-90, 0, -90));
		CreateStations("station-tf-farm", Sunwatch, TheFarm, 1, 1 + StationLevelBonus, StationParams);
	}
}

#undef LOCTEXT_NAMESPACE