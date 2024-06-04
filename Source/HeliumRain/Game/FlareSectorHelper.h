#pragma once

#include "FlareWorldHelper.h"
#include "../Economy/FlareResource.h"
#include "../Game/FlareTradeRoute.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"

struct SectorHelper
{
	struct FlareTradeRequest
	{
		FFlareResourceDescription* Resource;
		EFlareTradeRouteOperation::Type Operation;
		int32 MaxQuantity;
		float CargoLimit;
		float LoadUnloadPriority;
		float BuySellPriority;
		UFlareSimulatedSpacecraft *Client;
		bool IsDonation = false;
		bool AllowStorage = false;
		bool AllowFullStock = false;
		bool AllowUseNoTradeForMe = true;
	};

	static UFlareSimulatedSpacecraft*  FindTradeStation(FlareTradeRequest Request);

	static int32 Trade(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, int32 MaxQuantity, int64* TransactionPrice = NULL, UFlareTradeRoute* TradeRoute = nullptr, bool IsDonation = false, int32 TradeReason = 0);

	static void GetAvailableFleetSupplyCount(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32& OwnedFS, int32& AvailableFS, int32& AffordableFS, UFlareSimulatedSpacecraft* ForShip = nullptr, UFlareFleet* ForFleet = nullptr);

	static float GetComponentMaxRepairRatio(FFlareSpacecraftComponentDescription* ComponentDescription);

	static bool HasShipRefilling(UFlareSimulatedSector* TargetSector, UFlareCompany* Company);

	static bool HasShipRepairing(UFlareSimulatedSector* TargetSector, UFlareCompany* Company);

	static bool HasShipRefilling(TArray<UFlareSimulatedSpacecraft*>& Ships, UFlareCompany* Company);

	static bool HasShipRepairing(TArray<UFlareSimulatedSpacecraft*>& Ships, UFlareCompany* Company);

	static void GetRepairFleetSupplyNeeds(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32& CurrentNeededFleetSupply, int32& TotalNeededFleetSupply, int64& MaxDuration, bool OnlyPossible);

	static void GetRefillFleetSupplyNeeds(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32& CurrentNeededFleetSupply, int32& TotalNeededFleetSupply, int64& MaxDuration, bool OnlyPossible);
	
	static void GetRepairFleetSupplyNeeds(UFlareSimulatedSector* Sector, TArray<UFlareSimulatedSpacecraft*>& ships, int32& CurrentNeededFleetSupply, int32& TotalNeededFleetSupply, int64& MaxDuration, bool OnlyPossible);

	static void GetRefillFleetSupplyNeeds(UFlareSimulatedSector* Sector, TArray<UFlareSimulatedSpacecraft*>& ships, int32& CurrentNeededFleetSupply, int32& TotalNeededFleetSupply, int64& MaxDuration, bool OnlyPossible);


	static void RepairFleets(UFlareSimulatedSector* Sector, UFlareCompany* Company, UFlareFleet* Fleet = nullptr);

	static void RefillFleets(UFlareSimulatedSector* Sector, UFlareCompany* Company, UFlareFleet* Fleet = nullptr);

	static void ConsumeFleetSupply(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32 ConsumedFS, bool ForRepair, UFlareFleet* ForFleet = nullptr);

	static int32 GetArmyCombatPoints(UFlareSimulatedSector* Sector, bool ReduceByDamage);

	static int32 GetHostileArmyCombatPoints(UFlareSimulatedSector* Sector, UFlareCompany* Company, bool ReduceByDamage);

	static int32 GetCompanyArmyCombatPoints(UFlareSimulatedSector* Sector, UFlareCompany* Company, bool ReduceByDamage);

	static TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> ComputeSectorResourceStats(UFlareSimulatedSector* Sector, bool IncludeStorage);

	static int64 GetSellResourcePrice(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource, FFlareResourceUsage Usage);

	static int64 GetBuyResourcePrice(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource, FFlareResourceUsage Usage);
};
