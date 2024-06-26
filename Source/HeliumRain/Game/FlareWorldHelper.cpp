
#include "FlareWorldHelper.h"
#include "../Flare.h"

#include "../Data/FlareResourceCatalog.h"

#include "FlareGame.h"
#include "FlareWorld.h"
#include "FlareSectorHelper.h"
#include "FlareSimulatedSector.h"
#include "FlareScenarioTools.h"

#include "../Spacecrafts/FlareSimulatedSpacecraft.h"

DECLARE_CYCLE_STAT(TEXT("WorldHelper ComputeWorldResourceStats"), STAT_WorldHelper_ComputeWorldResourceStats, STATGROUP_Flare);


TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> WorldHelper::ComputeWorldResourceStats(AFlareGame* Game, bool IncludeStorage)
{
	SCOPE_CYCLE_COUNTER(STAT_WorldHelper_ComputeWorldResourceStats);


	TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> WorldStats;

	// Init
	WorldStats.Reserve(Game->GetResourceCatalog()->Resources.Num());
	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		WorldHelper::FlareResourceStats ResourceStats;
		ResourceStats.Production = 0;
		ResourceStats.Consumption = 0;
		ResourceStats.Balance = 0;
		ResourceStats.Stock = 0;
		ResourceStats.Capacity = 0;

		WorldStats.Add(Resource, ResourceStats);
	}

	for (int SectorIndex = 0; SectorIndex < Game->GetGameWorld()->GetSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Game->GetGameWorld()->GetSectors()[SectorIndex];

		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> SectorStats;

		SectorStats = SectorHelper::ComputeSectorResourceStats(Sector, IncludeStorage);

		for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
		{
			FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;

			WorldHelper::FlareResourceStats& SectorResourceStats = SectorStats[Resource];
			WorldHelper::FlareResourceStats& ResourceStats = WorldStats[Resource];
			ResourceStats.Production += SectorResourceStats.Production;
			ResourceStats.Consumption += SectorResourceStats.Consumption;
			ResourceStats.Balance += SectorResourceStats.Balance;
			ResourceStats.Stock += SectorResourceStats.Stock;
			ResourceStats.Capacity += SectorResourceStats.Capacity;

			WorldStats.Add(Resource, ResourceStats);
		}
	}


	// Balance
	for(int32 ResourceIndex = 0; ResourceIndex < Game->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Game->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		WorldHelper::FlareResourceStats *ResourceStats = &WorldStats[Resource];

		ResourceStats->Balance = ResourceStats->Production - ResourceStats->Consumption;

		/*FLOGV("World stats for %s: Production=%f Consumption=%f Balance=%f Stock=%d",
			  *Resource->Name.ToString(),
			  ResourceStats->Production,
			  ResourceStats->Consumption,
			  ResourceStats->Balance,
			  ResourceStats->Stock);*/
	}

	return WorldStats;
}
