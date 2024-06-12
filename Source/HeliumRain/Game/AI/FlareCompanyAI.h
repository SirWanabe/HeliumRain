#pragma once

#include "Object.h"
#include "../FlareGameTypes.h"
#include "../FlareWorldHelper.h"
#include "FlareAITradeHelper.h"
#include "FlareCompanyAI.generated.h"


class UFlareCompany;
class UFlareAIBehavior;

struct FFlareDiplomacyStats
{

};

struct AIWarContext
{
	TArray<UFlareCompany*> Allies;
	TArray<UFlareCompany*> Enemies;
	TArray<UFlareSimulatedSector*> KnownSectors;
	float AttackThreshold;

	void Generate();
};



struct DefenseSector
{
	UFlareSimulatedSector* Sector;
	UFlareSimulatedSector* TempBaseSector;
	int32 CombatPoints;
	int64 ArmyAntiSCombatPoints;
	int64 ArmyAntiLCombatPoints;
	int32 ArmyLargeShipCombatPoints;
	int32 ArmySmallShipCombatPoints;
	int32 LargeShipArmyCount;
	int32 SmallShipArmyCount;
	bool CapturingStation;
	bool CapturingShip;
	UFlareSimulatedSpacecraft* PrisonersKeeper;

	bool operator==(const DefenseSector& lhs)
	{
		return lhs.Sector == Sector;
	}

	bool operator!=(const DefenseSector& lhs)
	{
		return !(*this == lhs);
	}
};

UCLASS()
class HELIUMRAIN_API UFlareCompanyAI : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public API
	----------------------------------------------------*/

	/** Load the company AI from a save file */
	virtual void Load(UFlareCompany* ParentCompany, const FFlareCompanyAISave& Data);

	/** Save the company AI to a save file */
	virtual FFlareCompanyAISave* Save();

	/** Used to give commands for the active sector */
	virtual void SimulateActiveAI();
	virtual void SimulateActiveTradeShip(AFlareSpacecraft* Ship);

	/** Simulate a day */
	virtual void Simulate(bool GlobalWar, int32 TotalReservedResources);

	void CreateWorldResourceVariations();

	/** Try to purchase research */
	virtual void PurchaseResearch();

	/** Try to purchase sector station building license */
	virtual bool PurchaseSectorStationLicense(EFlareBudget::Type BudgetType);

	/** Destroy a spacecraft */
	virtual void DestroySpacecraft(UFlareSimulatedSpacecraft* Spacecraft);

	void UpdateIdleShipsStats(AITradeIdleShips& IdleShips);

	void TrackStationConstructionStatus(UFlareSimulatedSpacecraft* Station);
	void FinishedConstruction(UFlareSimulatedSpacecraft* FinishedStation);
	void CapturedStation(UFlareSimulatedSpacecraft* CapturedStation);

	bool IsAboveMinimumMoney();
	
	/*----------------------------------------------------
		Behavior API
	----------------------------------------------------*/

	void CargosEvasion();

	/** Update diplomacy changes */
	void UpdateDiplomacy(bool GlobalWar);

	void UpdateBestScore(float Score,
						  UFlareSimulatedSector* Sector,
						  FFlareSpacecraftDescription* StationDescription,
						  UFlareSimulatedSpacecraft *Station,
						  float* BestScore,
						  FFlareSpacecraftDescription** BestStationDescription,
						  UFlareSimulatedSpacecraft** BestStation,
						  UFlareSimulatedSector** BestSector);

	void RecalculateFullBudgets();
	int64 GetTotalBudgets();
	void AddIncomeToBudgets();
	bool CanSpendBudget(EFlareBudget::Type Type, int64 Amount);
	void RedistributeBudgetTowards(EFlareBudget::Type Type, EFlareBudget::Type FromBudget, float Ratio = 0.f);
	void RedistributeBudget(EFlareBudget::Type Type, int64 Amount);
	void SpendBudget(EFlareBudget::Type Type, int64 Amount);
	void SetBudget(EFlareBudget::Type Type, int64 Amount);
	void ModifyBudget(EFlareBudget::Type Type, int64 Amount);

	int64 GetBudget(EFlareBudget::Type Type);

	void ProcessBudget(TArray<EFlareBudget::Type> BudgetToProcess);

	void ProcessBudgetMilitary(int64 BudgetAmount, bool& Lock, bool& Idle);

	void ProcessBudgetTrade(int64 BudgetAmount, bool& Lock, bool& Idle);

	void ProcessBudgetStation(int64 BudgetAmount, bool Technology, bool& Lock, bool& Idle);

	/** Buy war ships */
	int64 UpdateWarShipAcquisition();

	/** Repair then refill all ships and stations */
	void RepairAndRefill();

	/*----------------------------------------------------
		Military AI
	----------------------------------------------------*/

	/** Move military ships */
	void UpdateMilitaryMovement();

	/** Move military ships while at war */
	void UpdateWarMilitaryMovement();

	/** Move military ships while in peace */
	void UpdatePeaceMilitaryMovement();

	/** Move carriers towards required resources if not maxed on drones*/
	void UpdateNeedyCarrierMovement(UFlareSimulatedSpacecraft* Ship, TArray<FFlareResourceDescription*> InputResources);

	UFlareSimulatedSector* FindNearestSectorWithPeace(AIWarContext& WarContext, UFlareSimulatedSector* OriginSector);

	UFlareSimulatedSector* FindNearestSectorWithFS(AIWarContext& WarContext, UFlareSimulatedSector* OriginSector);

	UFlareSimulatedSector* FindNearestSectorWithUpgradePossible(AIWarContext& WarContext, UFlareSimulatedSector* OriginSector);

	bool UpgradeShip(UFlareSimulatedSpacecraft* Ship, EFlarePartSize::Type WeaponTargetSize, bool AllowSalvager=false);

	bool UpgradeMilitaryFleet(AIWarContext& WarContext, WarTarget Target, DefenseSector& Sector, TArray<UFlareSimulatedSpacecraft*> &MovableShips);

	TArray<WarTargetIncomingFleet> GenerateWarTargetIncomingFleets(AIWarContext& WarContext, UFlareSimulatedSector* DestinationSector);

	TArray<WarTarget> GenerateWarTargetList(AIWarContext& WarContext);

	TArray<UFlareSimulatedSpacecraft*> GenerateWarShipList(AIWarContext& WarContext, UFlareSimulatedSector* Sector, UFlareSimulatedSpacecraft* ExcludeShip = NULL);

	TArray<DefenseSector> SortSectorsByDistance(UFlareSimulatedSector* BaseSector, TArray<DefenseSector> SectorsToSort);

	int64 GetDefenseSectorTravelDuration(TArray<DefenseSector>& DefenseSectorList, const DefenseSector& OriginSector);

	TArray<DefenseSector> GetDefenseSectorListInRange(TArray<DefenseSector>& DefenseSectorList, const DefenseSector& OriginSector, int64 MaxTravelDuration);

	TArray<DefenseSector> GenerateDefenseSectorList(AIWarContext& WarContext);

	void CheckBattleResolution();

	void CheckBattleState();

	bool HasHealthyTradeFleet() const;

	bool ComputeAvailableConstructionResourceAvailability(int32 MinimumQuantity) const;

	void NewSectorLoaded();
	void SetActiveAIDesireRepairs(bool NewValue);

protected:

	/*----------------------------------------------------
		Internal subsystems
	----------------------------------------------------*/
	
	/** Try to muster resources to build stations */
	void FindResourcesForStationConstruction();

	void ClearConstructionProject();
	
	/** Buy cargos ships */
	int64 UpdateCargoShipAcquisition();

	void AutoScrap();

	
	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/
	
	/** Order one ship at any shipyard */
	int64 OrderOneShip(const FFlareSpacecraftDescription* ShipDescription);

	const FFlareSpacecraftDescription* FindBestShipToBuild(bool Military);
	
	void GetBuildingShipNumber();

//	** Get a list of shipyard */
//	TArray<UFlareSimulatedSpacecraft*> FindShipyards();

	/** Get a list of wrecked cargos */
	TArray<UFlareSimulatedSpacecraft*> FindIncapacitatedCargos() const;
	
	int32 GetDamagedCargosCapacity();

	int32 GetCargosCapacity();

	/** Get a list of idle military */
	TArray<UFlareSimulatedSpacecraft*> FindIdleMilitaryShips() const;

	float GetShipyardUsageRatio() const;

	/** Generate a score for ranking construction projects, version 2 */
	float ComputeConstructionScoreForStation(UFlareSimulatedSector* Sector, FFlareSpacecraftDescription* StationDescription, FFlareFactoryDescription* FactoryDescription, UFlareSimulatedSpacecraft* Station, bool Technology) const;

	float ComputeStationPrice(UFlareSimulatedSector* Sector, FFlareSpacecraftDescription* StationDescription, UFlareSimulatedSpacecraft* Station) const;

	/** Print the resource flow */
	void DumpSectorResourceVariation(UFlareSimulatedSector* Sector, TMap<FFlareResourceDescription*, struct ResourceVariation>* Variation) const;


protected:

	/*----------------------------------------------------
		Data
	----------------------------------------------------*/

	// Gameplay data
	UFlareCompany*			               Company;
	FFlareCompanyAISave					   AIData;
	AFlareGame*                            Game;
	UPROPERTY()
	UFlareAIBehavior*                      Behavior;
	
	// Cache
	TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> WorldStats;
	TArray<UFlareSimulatedSpacecraft*>       Shipyards;
	TArray<UFlareSimulatedSector*>			 UndiscoveredSectors;
	TArray<UFlareSimulatedSpacecraft*>       UnderConstructionUpgradeStations;
	TArray<UFlareSimulatedSpacecraft*>       UnderConstructionStations;


	TMap<UFlareSimulatedSector*, SectorVariation> WorldResourceVariation;

	TArray<UFlareSimulatedSector*>            SectorWithBattle;

	int32									IdleCargoCapacity;
	int32									ReservedResources;
	int32									GlobalReservedResources;

	bool									WasAtWar;

	int32									BuildingMilitaryShips;
	int32									BuildingTradeShips;
	int32									MinimumMoney;

	bool									CheckedBuildingShips;

	bool									ActiveAIDesireRepairs;

public:

	TArray<EFlareBudget::Type> AllBudgets;

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	int32 GetReservedResources() const
	{
		return ReservedResources;
	}

	AFlareGame* GetGame() const
	{
		return Game;
	}



	int32 GetMinimumMoney() const
	{
		return MinimumMoney;
	}

	static void GetCompany();

	UFlareAIBehavior* GetBehavior()
	{
		return Behavior;
	}

	FFlareCompanyAISave* GetData()
	{
		return &AIData;
	}

};

