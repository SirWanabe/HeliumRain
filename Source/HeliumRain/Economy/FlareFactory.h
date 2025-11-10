
#pragma once
#include "FlareResource.h"
#include "../Data/FlareResourceCatalogEntry.h"
#include "../Game/FlareWorld.h"
#include "FlareFactory.generated.h"

class UFlareSimulatedSpacecraft;



UCLASS()
class HELIUMRAIN_API UFlareFactory : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/*----------------------------------------------------
	   Save
	----------------------------------------------------*/

	/** Load the factory from a save file */
	virtual void Load(UFlareSimulatedSpacecraft* ParentSpacecraft, const FFlareFactoryDescription* Description, const FFlareFactorySave& Data);

	/** Save the factory to a save file */
	virtual FFlareFactorySave* Save();

	/*----------------------------------------------------
	   Gameplay
	----------------------------------------------------*/

	void SetHascheckedforrequiredtechnologies(bool NewValue);
	void UpdateHasrequiredtechnologies();
	bool OwnerCompanyHasRequiredTechnologies();

	void Simulate();

	void TryBeginProduction();

	void UpdateDynamicState();

	void Start();

	void StartShipBuilding(FFlareShipyardOrderSave& Order);

	void Pause();

	void Stop();

	void SetInfiniteCycle(bool Mode);

	void SetCycleCount(uint32 Count);

	void SetOutputLimit(FFlareResourceDescription* Resource, uint32 MaxSlot);

	void ClearOutputLimit(FFlareResourceDescription* Resource);

	void SetTargetShipClass(FName Identifier);

	bool HasInputMoney();

	bool HasInputResources();

	FString GetInputResourcesRequiredString();

	bool HasOutputFreeSpace();

	bool HasCostReserved();

	float GetResourceInputMultiplier();

	void BeginProduction();

	void CancelProduction();

	void DoProduction();
	void PostProduction(bool IncreasedEfficiency = false);

	FFlareWorldEvent *GenerateEvent();

	void PerformCreateShipAction(const FFlareFactoryAction* Action);

	void PerformDiscoverSectorAction(const FFlareFactoryAction* Action);
	
	void PerformGainResearchAction(const FFlareFactoryAction* Action);

	void PerformBuildStationAction(const FFlareFactoryAction* Action);

	TArray<UFlareSimulatedSector*> GetTelescopeTargetList();

	void NotifyNoMoreSector();

protected:

	/*----------------------------------------------------
	   Protected data
	----------------------------------------------------*/

	// Gameplay data
	FFlareFactorySave                       FactoryData;

	AFlareGame*                             Game;

	const FFlareFactoryDescription*         FactoryDescription;

	UFlareSimulatedSpacecraft*				Parent;
	FFlareWorldEvent                        NextEvent;
	uint32                                  ScaledProductionCost;
	FFlareProductionData					CycleCostCache;
	int32									CycleCostCacheLevel;
	bool									IsStationConstructionFactory;

	bool									Hascheckedforrequiredtechnologies;
	bool									Hasrequiredtechnologies;

	float									Factory_Efficiency_ProductionChange;
	float									Factory_Efficiency_DoProduction;
	float									Factory_Efficiency_Minimum;
	float									Factory_Efficiency_Maximum;


public:

	FFlareFactoryDescription           ConstructionFactoryDescription;


	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	void SetIsStationConstructionFactory(bool NewValue);

	inline AFlareGame* GetGame() const
	{
		return Game;
	}

	inline const FFlareFactoryDescription* GetDescription() const
	{
		return FactoryDescription;
	}

	inline UFlareSimulatedSpacecraft* GetParent()
	{
		return Parent;
	}

	const FFlareProductionData& GetCycleData();

	const FFlareProductionData& GetCycleDataForShipClass(FName Class);

	/** Is it a ship production line */
	bool IsShipyard() const;

	/** Is it a light ship production line */
	bool IsSmallShipyard() const;

	/** Is it a heavy ship production line */
	bool IsLargeShipyard() const;

	inline FFlareFactorySave GetFactoryData() const
	{
		return FactoryData;
	}

	FName GetTargetShipClass() const
	{
		return FactoryData.TargetShipClass;
	}

	FName GetTargetShipCompany() const
	{
		return FactoryData.TargetShipCompany;
	}

	uint32 GetProductionCost(const FFlareProductionData* Data = NULL);

	int64 GetRemainingProductionDuration();

	TArray<FFlareFactoryResource> GetLimitedOutputResources();

	inline int64 GetProductedDuration()
	{
		return FactoryData.ProductedDuration;
	}

	inline int64 GetProductionDuration()
	{
		return GetProductionTime(GetCycleData());
	}

	inline bool IsActive()
	{
		return FactoryData.Active;
	}

	inline bool IsPaused()
	{
		return !FactoryData.Active && FactoryData.ProductedDuration > 0;
	}

	bool ShouldShowFactoryEfficiencyPercentage();
	bool IsProducing();

	inline bool GetIsStationConstructionFactory()
	{
		return IsStationConstructionFactory;
	}

	inline bool HasInfiniteCycle()
	{
		return FactoryData.InfiniteCycle;
	}

	inline uint32 GetCycleCount()
	{
		return FactoryData.CycleCount;
	}

	inline bool IsNeedProduction()
	{
		return HasInfiniteCycle() || GetCycleCount() > 0;
	}

	uint32 GetOutputLimit(FFlareResourceDescription* Resource);

	bool HasOutputLimit(FFlareResourceDescription* Resource);

	int32 GetInputResourcesCount();

	int32 GetOutputResourcesCount();

	FFlareResourceDescription* GetInputResource(int32 Index);

	FFlareResourceDescription* GetOutputResource(int32 Index);

	uint32 GetInputResourceQuantity(int32 Index);

	uint32 GetOutputResourceQuantity(int32 Index);

	uint32 GetInputResourceQuantity(FFlareResourceDescription* Resource);

	uint32 GetInputResourceQuantityCycles(FFlareResourceDescription* Resource);

	int64 GetInputResourceDailyProductionInCredits();

	uint32 GetOutputResourceQuantity(FFlareResourceDescription* Resource);

	bool HasOutputResource(FFlareResourceDescription* Resource);

	bool HasInputResource(FFlareResourceDescription* Resource);

	float GetFactoryEfficiency();
	float GetFactoryEfficiency_Minimum();
	float GetFactoryEfficiency_Maximum();
	/** Get this factory's cycle costs */
	FText GetFactoryCycleCost(const FFlareProductionData* Data);

	/** Get this factory's cycle data */
	FText GetFactoryCycleInfo();

	/** Get this factory's status text */
	FText GetFactoryStatus();

	inline TArray<FFlareCargoSave>& GetReservedResources()
	{
		return FactoryData.ResourceReserved;
	}

	inline int64 GetReservedMoney()
	{
		return FactoryData.CostReserved;
	}

	int64 GetProductionBalance();

	/** Do the production efficiency to duration malus conversion */
	static float GetProductionMalus(float Efficiency);

	int64 GetProductionTime(const struct FFlareProductionData& Cycle);

	float GetMarginRatio();
};