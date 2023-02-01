
#include "FlareTechnologyCatalog.h"
#include "../Flare.h"
#include "AssetRegistryModule.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareTechnologyCatalog::UFlareTechnologyCatalog(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	TArray<FAssetData> AssetList;
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	Registry.SearchAllAssets(true);
	Registry.GetAssetsByClass(UFlareTechnologyCatalogEntry::StaticClass()->GetFName(), AssetList);

	for (int32 Index = 0; Index < AssetList.Num(); Index++)
	{
		//FLOGV("UFlareTechnologyCatalog::UFlareTechnologyCatalog : Found '%s'", *AssetList[Index].GetFullName());
		UFlareTechnologyCatalogEntry* Technology = Cast<UFlareTechnologyCatalogEntry>(AssetList[Index].GetAsset());
		FCHECK(Technology);
		TechnologyCatalog.Add(Technology);
	}

	for (int32 Index = 0; Index < TechnologyCatalog.Num(); Index++)
	{
		UFlareTechnologyCatalogEntry* Technology = TechnologyCatalog[Index];
		UFlareTechnologyCatalogEntry* OldEntry = NULL;
		for (UFlareTechnologyCatalogEntry* TechnologySub : TechnologyCatalog)
		{
			if (TechnologySub != Technology && TechnologySub->Data.Identifier == Technology->Data.Identifier && TechnologySub->Data.ModLoadPriority <= Technology->Data.ModLoadPriority)
			{
				OldEntry = TechnologySub;
				break;
			}
		}

		if (OldEntry)
		{
			if (TechnologyCatalog.Remove(OldEntry))
			{
				Index = FMath::Min(0, Index -= 1);
			}
		}

		if (Technology->Data.IsDisabled)
		{
			if (TechnologyCatalog.Remove(Technology))
			{
				Index = FMath::Min(0, Index -= 1);
			}
		}
	}
}


/*----------------------------------------------------
	Data getters
----------------------------------------------------*/

FFlareTechnologyDescription* UFlareTechnologyCatalog::Get(FName Identifier) const
{
	auto FindByName = [=](const UFlareTechnologyCatalogEntry* Candidate)
	{
		return Candidate->Data.Identifier == Identifier;
	};

	UFlareTechnologyCatalogEntry* const* Entry = TechnologyCatalog.FindByPredicate(FindByName);
	if (Entry && *Entry)
	{
		return &((*Entry)->Data);
	}

	return NULL;
}

int UFlareTechnologyCatalog::GetMaxTechLevel() const
{
	return MaximumTechnologyLevel;
}