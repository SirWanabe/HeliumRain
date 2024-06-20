
#pragma once

#include "Object.h"
#include "FlareGameTypes.h"
#include "FlareTravel.h"
#include "Planetarium/FlareSimulatedPlanetarium.h"
#include "FlareWorld.generated.h"



#define MAX_RCS_REPAIR_RATIO_BY_DAY 0.25f
#define MAX_REPAIR_RATIO_BY_DAY 0.125f
#define MAX_ENGINE_REPAIR_RATIO_BY_DAY 0.15f
#define MAX_WEAPON_REPAIR_RATIO_BY_DAY 0.1f
#define MAX_REFILL_RATIO_BY_DAY 0.3f


struct FFlareSectorSave;
struct FFlareSectorDescription;

class UFlareCompany;
class UFlareFleet;
class UFlareFactory;
class UFlareSector;
class UFlareSimulatedSector;
class UFlareBattle;

/** Hostility status */
UENUM()
namespace EFlareEventVisibility
{
	enum Type
	{
		Blocking,
		Warning,
		Silent
	};
}

/** Data for game events which can alter game balance IE: Global War*/
USTRUCT()
struct FFlareWorldGameEventSave
{
	GENERATED_USTRUCT_BODY()
	FName					EventName;
	int64					EventDate;
	int64					EventDateEnd;
};

/** World save data */
USTRUCT()
struct FFlareWorldSave
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	int64                    Date;

	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareWorldGameEventSave> GlobalEvents;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareCompanySave> CompanyData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSectorSave> SectorData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareTravelSave> TravelData;
};


/** World event data */
USTRUCT()
struct FFlareWorldEvent
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	int64                    Date;

	UPROPERTY(EditAnywhere, Category = Save)
	TEnumAsByte<EFlareEventVisibility::Type>  Visibility;
};


struct IncomingKey
{
	UFlareSimulatedSector* DestinationSector;
	int64 RemainingDuration;
	UFlareCompany* Company;

	bool operator==(const IncomingKey& other) const {
		return DestinationSector == other.DestinationSector
				&& RemainingDuration == other.RemainingDuration
				&& Company == other.Company;
	}

};

inline uint32 GetTypeHash(const IncomingKey& Key)
{
	return int32(Key.RemainingDuration);
}

struct IncomingValue
{
	bool NeedNotification = false;
	int32 LightShipCount = 0;
	int32 HeavyShipCount = 0;
	int32 CombatValue = 0;
};



UCLASS()
class HELIUMRAIN_API UFlareWorld: public UObject
{
    GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Save
	----------------------------------------------------*/

	/** Load the world from a save file */
	virtual void Load(const FFlareWorldSave& Data);

	/** Loading is done */
	virtual void PostLoad();

	/** Save the company to a save file */
	virtual FFlareWorldSave* Save();

	/** Spawn a company from save data */
	virtual UFlareCompany* LoadCompany(const FFlareCompanySave& CompanyData);

	/** Spawn a sector from save data */
	UFlareSimulatedSector* LoadSector(const FFlareSectorDescription* Description, const FFlareSectorSave& SectorData, const FFlareSectorOrbitParameters& OrbitParameters);

	UFlareTravel* LoadTravel(const FFlareTravelSave& TravelData, UFlareFleet* Fleet);

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	bool CheckIntegrity();

	void CompanyMutualAssistance();

	void ProcessShipCapture();

	void ProcessStationCapture();

	void CheckAIBattleState();

	void UpdateStorageLocks();

	void ProcessIncomingPlayerEnemy();

	void UpdateReserveShips();

	/** Simulate world for a day */
	void Simulate();

	void SimulatePeopleMoneyMigration();

	/** Simulate world from now to the next event */
	void FastForward();

	UFlareTravel* StartTravel(UFlareFleet* TravelingFleet, UFlareSimulatedSector* DestinationSector, bool Force=false);

	virtual void DeleteTravel(UFlareTravel* Travel);

	/** Force new date */
	virtual void ForceDate(int64 Date);

	/** Generate all the next events in the world. If PointOfView is set, return the next event this company known */
	TArray<FFlareWorldEvent> GenerateEvents(UFlareCompany* PointOfView = NULL);

	/** Clear all factories associated to ParentSpacecraft */
	void ClearFactories(UFlareSimulatedSpacecraft *ParentSpacecraft);

	/** Add a factory to world */
	void AddFactory(UFlareFactory* Factory);

	FFlareWorldGameEventSave* GetGlobalEvent(FName EventSearch);

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Gameplay data
	FFlareWorldSave                       WorldData;

	/** Sectors */
	UPROPERTY()
	TArray<UFlareSimulatedSector*>        Sectors;

	/** Companies */
	UPROPERTY()
	TArray<UFlareCompany*>                Companies;

	/** Factories */
	UPROPERTY()
	TArray<UFlareFactory*>                Factories;

	/** Shipyards */
	UPROPERTY()
	TArray<UFlareSimulatedSpacecraft*>    Shipyards;

	UPROPERTY()
	TArray<UFlareTravel*>                Travels;

	UPROPERTY()
	UFlareSimulatedPlanetarium*			 Planetarium;

	UPROPERTY()
	UFlareBattle*					  	 BattleSimulation;

	AFlareGame*                          Game;

	bool RunningPrimarySimulate;
	bool WorldMoneyReferenceInit;
	TArray<UFlareCompany*> SortedCompanyValues;
	TArray<UFlareCompany*> SortedCompanyCombatValues;
	int64 TotalCompaniesMoney;

public:
	int64 WorldMoneyReference;

	int32 TotalWorldCombatPointCache;
	bool HasTotalWorldCombatPointCache = false;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Game;
	}

	int64 GetTotalCompaniesMoney() const
	{
		return TotalCompaniesMoney;
	}

		FFlareWorldSave* GetData()
	{
		return &WorldData;
	}

	inline UFlareSimulatedPlanetarium* GetPlanerarium()
	{
		return Planetarium;
	}

	inline TArray<UFlareSimulatedSector*>& GetSectors()
	{
		return Sectors;
	}

	inline TArray<UFlareSimulatedSpacecraft*>& GetShipyards()
	{
		return Shipyards;
	}

	TArray<UFlareSimulatedSpacecraft*> GetShipyardsFor(UFlareCompany* Company);

	inline TArray<UFlareCompany*>& GetSortedCompanyValues()
	{
		return SortedCompanyValues;
	}

	inline TArray<UFlareCompany*>& GetSortedCompanyCombatValues()
	{
		return SortedCompanyCombatValues;
	}

	inline TArray<UFlareTravel*>& GetTravels()
	{
		return Travels;
	}

	inline int64 GetDate()
	{
		return WorldData.Date;
	}

	
	inline bool GetPrimarySimulate()
	{
		return RunningPrimarySimulate;
	}

	UFlareCompany* FindCompany(FName Identifier) const;

	UFlareCompany* FindCompanyByShortName(FName CompanyShortName) const;

	UFlareSimulatedSector* FindSector(FName Identifier) const;

	UFlareSimulatedSector* FindSectorBySpacecraft(FName SpacecraftIdentifier) const;

	UFlareFleet* FindFleet(FName Identifier) const;

	UFlareTradeRoute* FindTradeRoute(FName Identifier) const;

	UFlareSimulatedSpacecraft* FindSpacecraft(FName ShipImmatriculation);

	inline const TArray<UFlareCompany*>& GetCompanies() const
	{
		return Companies;
	}

	int64 GetWorldMoney();

	uint32 GetWorldPopulation();

	TArray<FFlareIncomingEvent> GetIncomingEvents();

	int32 GetTotalWorldCombatPoint();

	TMap<IncomingKey, IncomingValue> GetIncomingPlayerEnemy();

};
