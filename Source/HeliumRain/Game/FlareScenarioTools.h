#pragma once

#include "FlareSimulatedSector.h"
#include "FlareScenarioTools.generated.h"

class UFlareWorld;
class UFlareCompany;
class UFlareSimulatedSector;
class UFlareSimulatedSpacecraft;
struct FFlarePlayerSave;
struct FFlareCelestialBody;


UCLASS()
class HELIUMRAIN_API UFlareScenarioTools : public UObject
{
public:

	GENERATED_UCLASS_BODY()


	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	void Init(UFlareCompany* Company, FFlarePlayerSave* Player);

	void PostLoad();//(bool RandomizeStationLocations);
	
	// Game scnarios
	void GenerateEmptyScenario(bool RandomizeStationLocations = false, int32 EconomyIndex = 0);
	void GenerateFighterScenario(bool RandomizeStationLocations = false, int32 EconomyIndex = 0);
	void GenerateFreighterScenario(bool RandomizeStationLocations = false, int32 EconomyIndex = 0);
	void GenerateCustomScenario(int32 ScenarioIndex, bool RandomizeStationLocations = false, int32 EconomyIndex = 0);
	void GenerateDebugScenario(bool RandomizeStationLocations = false, int32 EconomyIndex = 0);
	void GeneratePlayerStartingResearch();
	void GeneratePlayerStartingSectorKnowledge();

	/** Add a new player ship */
	UFlareSimulatedSpacecraft* CreateRecoveryPlayerShip();

	
protected:

	/*----------------------------------------------------
		Common world
	----------------------------------------------------*/

	/** Setup the common world */
	void SetupWorld(bool RandomizeStationLocations = false, int32 EconomyIndex = 0);

	/** Setup asteroids */
	void SetupAsteroids();
	
	/** Discover all known sectors*/
	void SetupKnownSectors(UFlareCompany* Company);


public:

	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/

	/** Create the player ship */
	UFlareSimulatedSpacecraft* CreatePlayerShip(UFlareSimulatedSector* Sector, FName Class);

	/** Spawn a series of asteroids in this sector */
	void CreateAsteroids(UFlareSimulatedSector* Sector, int32 Count = 50, FVector DistributionShape = FVector(2, 50, 1));
		
	/** Create a ship */
	UFlareSimulatedSpacecraft* CreateShips(FName ShipClass, UFlareCompany* Company, UFlareSimulatedSector* Sector, uint32 Count);

	/** Create a station and fill its input */
	void CreateStations(FName StationClass, UFlareCompany* Company, UFlareSimulatedSector* Sector, uint32 Count, int32 Level = 1, FFlareStationSpawnParameters SpawnParameters = FFlareStationSpawnParameters(), bool RandomLocation = false, float VarianceMinInputs = 0.25, float VarianceMaxInputs = 0.75, float VarianceMinOutputs = 0.25, float VarianceMaxOutputs = 0.75);

	/** Setup the Blue Heart capital station */
	void CreateBlueHeart(double StationLevelBonus = 0);

	/** Setup the Boneyard capital station */
	void CreateBoneyard(double StationLevelBonus = 0);

	/** Setup the Night's Home capital station */
	void CreateNightsHome(double StationLevelBonus = 0);

	/** Setup the Farm capital station */
	void CreateTheFarm(double StationLevelBonus = 0);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Main data
	UFlareCompany*                             PlayerCompany;
	FFlarePlayerSave*                          PlayerData;
	AFlareGame*                                Game;
	UFlareWorld*                               World;

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	// Celestial body
	FFlareCelestialBody*                       Nema;
	FFlareCelestialBody*                       Anka;
	FFlareCelestialBody*                       Asta;
	FFlareCelestialBody*                       Hela;
	FFlareCelestialBody*                       Adena;

	// Notable sectors (Nema)
	UFlareSimulatedSector*                     TheDepths;
	UFlareSimulatedSector*                     FirstLight;
	UFlareSimulatedSector*                     MinersHome;
	UFlareSimulatedSector*                     Anomaly;
	UFlareSimulatedSector*                     BlueHeart;
	UFlareSimulatedSector*                     Lighthouse;
	UFlareSimulatedSector*                     BlueShores;
	UFlareSimulatedSector*                     TheSpire;
	UFlareSimulatedSector*                     Pendulum;
	UFlareSimulatedSector*                     TheFarm;

	// Notable sectors (Anka)
	UFlareSimulatedSector*                     Outpost;
	UFlareSimulatedSector*                     Colossus;
	UFlareSimulatedSector*                     Crossroads;
	UFlareSimulatedSector*                     TheDig;
	UFlareSimulatedSector*                     TheForge;

	// Notable sectors (Hela)
	UFlareSimulatedSector*                     FrozenRealm;
	UFlareSimulatedSector*                     NightsHome;
	UFlareSimulatedSector*                     ShoreOfIce;
	UFlareSimulatedSector*                     Ruins;
	UFlareSimulatedSector*                     WinterJunction;

	// Notable sectors (Asta)
	UFlareSimulatedSector*                     Decay;
	UFlareSimulatedSector*                     Boneyard;
	UFlareSimulatedSector*                     Daedalus;

	// Notable sectors (Adena)
	UFlareSimulatedSector*                     Solitude;
	UFlareSimulatedSector*                     Tranquility;
	UFlareSimulatedSector*                     Serenity;
	
	// Companies
	UFlareCompany*                             MiningSyndicate;
	UFlareCompany*                             HelixFoundries;
	UFlareCompany*                             Sunwatch;
	UFlareCompany*                             IonLane;
	UFlareCompany*                             UnitedFarmsChemicals;
	UFlareCompany*                             GhostWorksShipyards;
	UFlareCompany*                             NemaHeavyWorks;
	UFlareCompany*                             AxisSupplies;
	UFlareCompany*                             Pirates;
	UFlareCompany*                             BrokenMoon;
	UFlareCompany*                             InfiniteOrbit;
	UFlareCompany*                             Quantalium;

	// Resources
	FFlareResourceDescription*                 Water;
	FFlareResourceDescription*                 Food;
	FFlareResourceDescription*                 Fuel;
	FFlareResourceDescription*                 Plastics;
	FFlareResourceDescription*                 Hydrogen;
	FFlareResourceDescription*                 Helium;
	FFlareResourceDescription*                 Silica;
	FFlareResourceDescription*                 IronOxyde;
	FFlareResourceDescription*                 Steel;
	FFlareResourceDescription*                 Tools;
	FFlareResourceDescription*                 Tech;
	FFlareResourceDescription*                 Carbon;
	FFlareResourceDescription*                 Methane;
	FFlareResourceDescription*                 FleetSupply;

	// Ships
	FName                                      ShipSolen;
	FName                                      ShipOmen;
	FName                                      ShipGhoul;
	FName                                      ShipOrca;
	FName                                      ShipPhalanx;

	FName                                      ShipSphinx;
	FName                                      ShipThemis;
	FName                                      ShipAtlas;
	FName                                      ShipDragon;
	FName                                      ShipKami;
	FName                                      ShipInvader;
	FName                                      ShipLeviathan;
	FName                                      ShipAnubis;

	// Stations
	FName                                      StationFarm;
	FName                                      StationSolarPlant;
	FName                                      StationHabitation;
	FName                                      StationIceMine;
	FName                                      StationIronMine;
	FName                                      StationSilicaMine;
	FName                                      StationSteelworks;
	FName                                      StationToolFactory;
	FName                                      StationHydrogenPump;
	FName                                      StationHeliumPump;
	FName                                      StationMethanePump;
	FName                                      StationCarbonRefinery;
	FName                                      StationPlasticsRefinery;
	FName                                      StationArsenal;
	FName                                      StationShipyard;
	FName                                      StationHub;
	FName                                      StationOutpost;
	FName									   StationFoundry;
	FName									   StationFusion;
	FName									   StationResearch;

	/** Used for finding random sector for a station to go to*/
	TArray<UFlareSimulatedSector*> GetRandomAllowedSectors(FName StationClass, UFlareCompany* Company) const;

};