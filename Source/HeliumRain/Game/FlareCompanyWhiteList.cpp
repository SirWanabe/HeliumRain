#include "FlareCompanyWhiteList.h"
#include "../Player/FlarePlayerController.h"
#include "FlareCompany.h"
#include "FlareSimulatedSector.h"
#include "FlareFleet.h"
#include "FlareGame.h"

#define LOCTEXT_NAMESPACE "UFlareCompanyWhiteList"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareCompanyWhiteList::UFlareCompanyWhiteList(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareCompanyWhiteList::Load(const FFlareWhiteListSave& Data, AFlareGame* NewGame)
{
	WhiteListData = Data;
	Game = NewGame;
}

FFlareWhiteListSave* UFlareCompanyWhiteList::Save()
{
	return &WhiteListData;
}

void UFlareCompanyWhiteList::OnDelete()
{
	for (int i = 0; i < ShipsWithWhiteList.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = ShipsWithWhiteList[i];
		if (Spacecraft->GetSelectedWhiteList() == this)
		{
			Spacecraft->SelectWhiteListDefault(nullptr);
		}
	}
	for (int i = 0; i < FleetsWithWhiteList.Num(); i++)
	{
		UFlareFleet* Fleet = FleetsWithWhiteList[i];
		if (Fleet->GetSelectedWhiteList() == this)
		{
			Fleet->SelectWhiteListDefault(nullptr);
		}
	}
}

/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

FFlareWhiteListCompanyDataSave* UFlareCompanyWhiteList::AddNewCompanyDataSave(FName Identifier)
{
	FFlareWhiteListCompanyDataSave NewCompanyData;
	NewCompanyData.Identifier = Identifier;

//Company data default settings
	NewCompanyData.CanTradeTo = true;
	NewCompanyData.CanTradeFrom = true;
	NewCompanyData.ResourcesTradeTo.Empty();
	NewCompanyData.ResourcesTradeFrom.Empty();

	TArray<UFlareResourceCatalogEntry*> ResourceList = Game->GetResourceCatalog()->Resources;
	for (int ResourceIndex = 0; ResourceIndex < ResourceList.Num(); ResourceIndex++)
	{
		UFlareResourceCatalogEntry* Resource = ResourceList[ResourceIndex];
		if (Resource)
		{
			FFlareResourceDescription* Data = &Resource->Data;
			if (Data)
			{
				NewCompanyData.ResourcesTradeTo.AddUnique(Data->Identifier);
				NewCompanyData.ResourcesTradeFrom.AddUnique(Data->Identifier);
			}
		}
	}

	WhiteListData.CompanyData.Add(NewCompanyData);
	return &WhiteListData.CompanyData.Last();
	return &NewCompanyData;
}

FFlareWhiteListCompanyDataSave* UFlareCompanyWhiteList::GetCompanyDataFor(UFlareCompany* CompanyLookup)
{
	if (CompanyLookup)
	{
		return GetCompanyDataFor(CompanyLookup->GetIdentifier());
	}
	return nullptr;
}

FFlareWhiteListCompanyDataSave* UFlareCompanyWhiteList::GetCompanyDataFor(FName IdentifierLookup)
{
	FFlareWhiteListCompanyDataSave* FoundCompanyData = nullptr;
	if (WhiteListData.CompanyDataTMAPS.Contains(IdentifierLookup))
	{
		FoundCompanyData = WhiteListData.CompanyDataTMAPS[IdentifierLookup];
	}
	else
	{
		for (int32 CompanyDataIndex = 0; CompanyDataIndex < WhiteListData.CompanyData.Num(); CompanyDataIndex++)
		{
			FFlareWhiteListCompanyDataSave* IteratingCompanyData = &WhiteListData.CompanyData[CompanyDataIndex];
			if (IteratingCompanyData->Identifier.ToString() == IdentifierLookup.ToString())
			{
				FoundCompanyData = IteratingCompanyData;
				WhiteListData.CompanyDataTMAPS.Add(IdentifierLookup, FoundCompanyData);
				break;
			}
		}
	}

	if (FoundCompanyData == nullptr)
	{
		FoundCompanyData = AddNewCompanyDataSave(IdentifierLookup);
		WhiteListData.CompanyDataTMAPS.Add(IdentifierLookup, FoundCompanyData);
	}

	return FoundCompanyData;
}

void UFlareCompanyWhiteList::SetWhiteListTradeTo(FFlareWhiteListCompanyDataSave* CompanyData, bool NewValue)
{
	CompanyData->CanTradeTo = NewValue;
}

void UFlareCompanyWhiteList::SetWhiteListTradeFrom(FFlareWhiteListCompanyDataSave* CompanyData, bool NewValue)
{
	CompanyData->CanTradeFrom = NewValue;
}

void UFlareCompanyWhiteList::AddShipToTracker(UFlareSimulatedSpacecraft* Ship)
{
	if (Ship)
	{
		ShipsWithWhiteList.AddUnique(Ship);
	}
}

void UFlareCompanyWhiteList::RemoveShipFromTracker(UFlareSimulatedSpacecraft* Ship)
{
	if (Ship)
	{
		ShipsWithWhiteList.Remove(Ship);
	}
}

void UFlareCompanyWhiteList::AddFleetToTracker(UFlareFleet* Fleet)
{
	if (Fleet)
	{
		FleetsWithWhiteList.AddUnique(Fleet);
	}
}

void UFlareCompanyWhiteList::RemoveFleetFromTracker(UFlareFleet* Fleet)
{
	if (Fleet)
	{
		FleetsWithWhiteList.Remove(Fleet);
	}
}
bool UFlareCompanyWhiteList::CanCompanyDataTradeTo(FFlareWhiteListCompanyDataSave* CompanyData, FFlareResourceDescription* Resource, FText& Reason)
{
	if (CompanyData)
	{
		if (!CompanyData->CanTradeTo)
		{
			Reason = LOCTEXT("CantTradeWhiteListTo", "Can't trade due to whitelist settings (Trade to)");
			return false;
		}

		if (Resource)
		{
			if (CompanyData->ResourcesTradeTo.Find(Resource->Identifier) == INDEX_NONE)
			{
				Reason = FText::Format(LOCTEXT("CantTradeWhiteListResourceTo", "Can't trade due to whitelist settings (Trade to, {0})"),
				Resource->Name);
				return false;
			}
		}
	}
	return true;
}

bool UFlareCompanyWhiteList::CanCompanyDataTradeFrom(FFlareWhiteListCompanyDataSave* CompanyData, FFlareResourceDescription* Resource, FText& Reason)
{
	if (CompanyData)
	{
		if (!CompanyData->CanTradeFrom)
		{
			return false;
		}

		if (Resource)
		{
			if (CompanyData->ResourcesTradeFrom.Find(Resource->Identifier) == INDEX_NONE)
			{
				Reason = FText::Format(LOCTEXT("CantTradeWhiteListResourceFrom", "Can't trade due to whitelist settings (Trade from, {0})"),
				Resource->Name);
				return false;
			}
		}
	}
	return true;
}

#undef LOCTEXT_NAMESPACE