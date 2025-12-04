
#include "FlareCompanyCatalog.h"
#include "../Flare.h"
#include "AssetRegistryModule.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareCompanyCatalog::UFlareCompanyCatalog(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	TArray<FAssetData> AssetList;
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	Registry.SearchAllAssets(true);
	Registry.GetAssetsByClass(UFlareCompanyCatalogEntry::StaticClass()->GetFName(), AssetList);

	for (int32 Index = 0; Index < AssetList.Num(); Index++)
	{
		FLOGV("UFlareCompanyCatalog::UFlareCompanyCatalog : Found '%s'", *AssetList[Index].GetFullName());

		UFlareCompanyCatalogEntry* CompanyEntry = Cast<UFlareCompanyCatalogEntry>(AssetList[Index].GetAsset());
		FCHECK(CompanyEntry);
		CompanyCatalog.Add(CompanyEntry);
	}

	if (CompanyCatalog.Num() > 0)
	{
		for (int32 i = 0; i < Companies.Num(); i++)
		{
			FFlareCompanyDescription& Candidate = Companies[i];
			UFlareCompanyCatalogEntry* CompanyEntry = NewObject<UFlareCompanyCatalogEntry>(this, UFlareCompanyCatalogEntry::StaticClass());
			FCHECK(CompanyEntry);
			CompanyCatalog.Add(CompanyEntry);
		}

		for (int32 Index = 0; Index < CompanyCatalog.Num(); Index++)
		{
			UFlareCompanyCatalogEntry* CompanyEntry = CompanyCatalog[Index];
			UFlareCompanyCatalogEntry* OldEntry = NULL;
			for (UFlareCompanyCatalogEntry* CompanySub : CompanyCatalog)
			{
				if (CompanySub != CompanyEntry && CompanySub->Data.ShortName == CompanyEntry->Data.ShortName && CompanySub->Data.ModLoadPriority <= CompanyEntry->Data.ModLoadPriority)
				{
					OldEntry = CompanySub;
					break;
				}
			}
			if (OldEntry)
			{
				if (CompanyCatalog.Remove(OldEntry))
				{
					Index = FMath::Min(0, Index -= 1);
				}
			}
		}
	}

	struct FSortByName
	{

		FORCEINLINE bool operator()(const UFlareCompanyCatalogEntry& A, const UFlareCompanyCatalogEntry& B) const
		{
			return A.Data.Name.ToString() < B.Data.Name.ToString();
		}
	};

	CompanyCatalog.Sort(FSortByName());
}

const void UFlareCompanyCatalog::GetCompanyList(TArray<FFlareCompanyDescription*>& OutData)
{
	if (CompanyCatalog.Num())
	{
		for (UFlareCompanyCatalogEntry* CompanySub : CompanyCatalog)
		{
			FFlareCompanyDescription& Candidate = CompanySub->Data;
			OutData.Add(&Candidate);
		}
	}
	else
	{
		for (int32 i = 0; i < Companies.Num(); i++)
		{
			FFlareCompanyDescription& Candidate = Companies[i];
			OutData.Add(&Candidate);
		}
	}
}