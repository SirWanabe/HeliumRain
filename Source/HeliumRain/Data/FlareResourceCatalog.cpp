
#include "FlareResourceCatalog.h"
#include "FlareSpacecraftCatalog.h"
#include "../Flare.h"
#include "../Game/FlareGame.h"
#include "AssetRegistryModule.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareResourceCatalog::UFlareResourceCatalog(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}

void UFlareResourceCatalog::InitialSetup(AFlareGame* GameMode)
{
	TArray<FAssetData> AssetList;
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	Registry.SearchAllAssets(true);
	Registry.GetAssetsByClass(UFlareResourceCatalogEntry::StaticClass()->GetFName(), AssetList);

	for (int32 Index = 0; Index < AssetList.Num(); Index++)
	{
		//FLOGV("UFlareResourceCatalog::UFlareResourceCatalog : Found '%s'", *AssetList[Index].GetFullName());
		UFlareResourceCatalogEntry* Resource = Cast<UFlareResourceCatalogEntry>(AssetList[Index].GetAsset());
		FCHECK(Resource);
//		bool AddToCatalog = true;
//		if (AddToCatalog)
		{
			Resources.Add(Resource);
		}
	}

	for (int32 Index = 0; Index < Resources.Num(); Index++)
	{
		UFlareResourceCatalogEntry* Resource = Resources[Index];
		UFlareResourceCatalogEntry* OldEntry = NULL;
		for (UFlareResourceCatalogEntry* ResourceSub : Resources)
		{
			if (ResourceSub != Resource && ResourceSub->Data.Identifier == Resource->Data.Identifier && ResourceSub->Data.ModLoadPriority <= Resource->Data.ModLoadPriority)
			{
				OldEntry = ResourceSub;
				break;
			}
		}

		bool RemovedResource = false;
		if (OldEntry)
		{
//			ReplaceOldEntrySettings(OldEntry, Resource);

			if (Resources.Remove(OldEntry))
			{
				ModifiedResources.AddUnique(OldEntry->Data.Identifier);

				Index = FMath::Min(0, Index -= 1);

//				RemovedResource = true;

				if (OldEntry->Data.IsConsumerResource)
				{
					ConsumerResources.Remove(OldEntry);
				}

				if (OldEntry->Data.IsMaintenanceResource)
				{
					MaintenanceResources.Remove(OldEntry);
				}
			}
		}

		if (Resource->Data.IsDisabled)
		{
			if (Resources.Remove(Resource))
			{
				Index = FMath::Min(0, Index -= 1);
				RemovedResource = true;
			}
		}

		if (!RemovedResource)
		{
			if (Resource->Data.IsConsumerResource)
			{
				ConsumerResources.Add(Resource);
			}

			if (Resource->Data.IsMaintenanceResource)
			{
				MaintenanceResources.Add(Resource);
			}
		}
	}

	// Sort resources
	Resources.Sort(SortByResourceType);
	ConsumerResources.Sort(SortByResourceType);
	MaintenanceResources.Sort(SortByResourceType);

	if (GetModifiedResources().Num() > 0)
	{
// Due to resources being directly referenced in the data we must find and alter all their direct references to the current version of the resource

		TArray<FFlareSpacecraftDescription*> AllSpacecraftDescriptions;
		TArray<UFlareFactoryCatalogEntry*> UniqueFactoryEntries;

		GameMode->GetSpacecraftCatalog()->GetShipList(AllSpacecraftDescriptions);
		GameMode->GetSpacecraftCatalog()->GetStationList(AllSpacecraftDescriptions);

		for (int32 DescriptionIndex = 0; DescriptionIndex < AllSpacecraftDescriptions.Num(); DescriptionIndex++)
		{
			FFlareSpacecraftDescription* Candidate = AllSpacecraftDescriptions[DescriptionIndex];
			FLOGV("UFlareResourceCatalog::UFlareResourceCatalog : Checking spacecrafts for replacement resources '%s',  %d factory entries. %d current index", *Candidate->Identifier.ToString(), Candidate->Factories.Num(), DescriptionIndex);

			for (int32 FactoriesIndex = 0; FactoriesIndex < Candidate->Factories.Num(); FactoriesIndex++)
			{
				UFlareFactoryCatalogEntry* CurrentFactoryEntry = Candidate->Factories[FactoriesIndex];
				UniqueFactoryEntries.AddUnique(CurrentFactoryEntry);
			}

			for (int32 ResourceIndex = 0; ResourceIndex < Candidate->CycleCost.InputResources.Num(); ResourceIndex++)
			{
				FFlareFactoryResource* FactoryResource = &Candidate->CycleCost.InputResources[ResourceIndex];
				if (FactoryResource && FactoryResource->Resource)
				{
					FFlareResourceDescription* ReplacementResourceDescription = Get(FactoryResource->Resource->Data.Identifier);
					if (ReplacementResourceDescription)
					{
						FactoryResource->Resource->Data = *ReplacementResourceDescription;
					}
				}
			}

			for (int32 ResourceIndex = 0; ResourceIndex < Candidate->CycleCost.OutputResources.Num(); ResourceIndex++)
			{
				FFlareFactoryResource* FactoryResource = &Candidate->CycleCost.OutputResources[ResourceIndex];
				if (FactoryResource && FactoryResource->Resource)
				{
					FFlareResourceDescription* ReplacementResourceDescription = Get(FactoryResource->Resource->Data.Identifier);
					if (ReplacementResourceDescription)
					{
						FactoryResource->Resource->Data = *ReplacementResourceDescription;
					}
				}
			}
		}

		for (int32 FactoriesIndex = 0; FactoriesIndex < UniqueFactoryEntries.Num(); FactoriesIndex++)
		{
			UFlareFactoryCatalogEntry* CurrentFactoryEntry = UniqueFactoryEntries[FactoriesIndex];

			FLOGV("UFlareResourceCatalog::UFlareResourceCatalog : Looping through the factories '%s', Input %d Output %d", *CurrentFactoryEntry->Data.Identifier.ToString(), CurrentFactoryEntry->Data.CycleCost.InputResources.Num(), CurrentFactoryEntry->Data.CycleCost.OutputResources.Num());

			for (int32 ResourceIndex = 0; ResourceIndex < CurrentFactoryEntry->Data.CycleCost.InputResources.Num(); ResourceIndex++)
			{
				FFlareFactoryResource* FactoryResource = &CurrentFactoryEntry->Data.CycleCost.InputResources[ResourceIndex];
				if (FactoryResource && FactoryResource->Resource)
				{
					FFlareResourceDescription* ReplacementResourceDescription = Get(FactoryResource->Resource->Data.Identifier);
					if (ReplacementResourceDescription)
					{
						FactoryResource->Resource->Data = *ReplacementResourceDescription;
						FLOGV("UFlareResourceCatalog::UFlareResourceCatalog : Found replacement for input '%s'", *FactoryResource->Resource->Data.Identifier.ToString());
					}
				}
			}

			for (int32 ResourceIndex = 0; ResourceIndex < CurrentFactoryEntry->Data.CycleCost.OutputResources.Num(); ResourceIndex++)
			{
				FFlareFactoryResource* FactoryResource = &CurrentFactoryEntry->Data.CycleCost.OutputResources[ResourceIndex];
				if (FactoryResource && FactoryResource->Resource)
				{
					FFlareResourceDescription* ReplacementResourceDescription = Get(FactoryResource->Resource->Data.Identifier);
					if (ReplacementResourceDescription)
					{
						FactoryResource->Resource->Data = *ReplacementResourceDescription;
						FLOGV("UFlareResourceCatalog::UFlareResourceCatalog : Found replacement for output '%s'", *FactoryResource->Resource->Data.Identifier.ToString());
					}
				}
			}
		}
	}
}

/*----------------------------------------------------
	Data getters
----------------------------------------------------*/

void UFlareResourceCatalog::ReplaceOldEntrySettings(UFlareResourceCatalogEntry* OldResourceEntry, UFlareResourceCatalogEntry* NewResource)
{
	OldResourceEntry->Data.Name = NewResource->Data.Name;
	OldResourceEntry->Data.Acronym = NewResource->Data.Acronym;
	OldResourceEntry->Data.Description = NewResource->Data.Description;
	OldResourceEntry->Data.DisplayIndex = NewResource->Data.DisplayIndex;
	OldResourceEntry->Data.Icon = NewResource->Data.Icon;

	OldResourceEntry->Data.IsConsumerResource = NewResource->Data.IsConsumerResource;
	OldResourceEntry->Data.IsMaintenanceResource = NewResource->Data.IsMaintenanceResource;
	OldResourceEntry->Data.IsManufacturingResource = NewResource->Data.IsManufacturingResource;
	OldResourceEntry->Data.IsRawResource = NewResource->Data.IsRawResource;

	OldResourceEntry->Data.MaxPrice = NewResource->Data.MaxPrice;
	OldResourceEntry->Data.MinPrice = NewResource->Data.MinPrice;
	OldResourceEntry->Data.TransportFee = NewResource->Data.TransportFee;
}

FFlareResourceDescription* UFlareResourceCatalog::Get(FName Identifier) const
{
	auto FindByName = [=](const UFlareResourceCatalogEntry* Candidate)
	{
		return Candidate->Data.Identifier == Identifier;
	};

	UFlareResourceCatalogEntry* const* Entry = Resources.FindByPredicate(FindByName);
	if (Entry && *Entry)
	{
		return &((*Entry)->Data);
	}

	return NULL;
}

UFlareResourceCatalogEntry* UFlareResourceCatalog::GetEntry(FFlareResourceDescription* Resource) const
{
	for (int32 ResourceIndex = 0; ResourceIndex < Resources.Num(); ResourceIndex++)
	{
		if(Resource == &Resources[ResourceIndex]->Data)
		{
			return Resources[ResourceIndex];
		}
	}
	return NULL;
}