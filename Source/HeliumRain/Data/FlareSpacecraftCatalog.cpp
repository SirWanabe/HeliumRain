
#include "FlareSpacecraftCatalog.h"
#include "../Flare.h"
#include "AssetRegistryModule.h"
#include "../Game/FlareGame.h"
#include "../Player/FlareMenuManager.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

struct FSortByEntrySize
{
	FORCEINLINE bool operator()(const UFlareSpacecraftCatalogEntry& EntryA, const UFlareSpacecraftCatalogEntry& EntryB) const
	{
		const FFlareSpacecraftDescription* A = &EntryA.Data;
		const FFlareSpacecraftDescription* B = &EntryB.Data;

		if (A->IsStation() && B->IsStation())
		{
			if (A->IsSubstation && !B->IsSubstation)
			{
				return true;
			}
			else if (!A->IsSubstation && B->IsSubstation)
			{
				return false;
			}
		}
		else if (A->IsStation() && !B->IsStation())
		{
			return true;
		}
		else if (!A->IsStation() && B->IsStation())
		{
			return false;
		}
		else if (A->IsMilitary() && B->IsMilitary())
		{
			return A->CombatPoints > B->CombatPoints;
		}
		else if (A->IsMilitary() && !B->IsMilitary())
		{
			return true;
		}
		else if (!A->IsMilitary() && B->IsMilitary())
		{
			return false;
		}
		else if (A->Mass > B->Mass)
		{
			return true;
		}
		else if (A->Mass < B->Mass)
		{
			return false;
		}

		return false;
	}
};

UFlareSpacecraftCatalog::UFlareSpacecraftCatalog(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}

void UFlareSpacecraftCatalog::InitialSetup(AFlareGame* GameMode)
{
	TArray<FAssetData> AssetList;
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	Registry.SearchAllAssets(true);
	Registry.GetAssetsByClass(UFlareSpacecraftCatalogEntry::StaticClass()->GetFName(), AssetList);

	for (int32 Index = 0; Index < AssetList.Num(); Index++)
	{
		//FLOGV("UFlareSpacecraftCatalog::UFlareSpacecraftCatalog : Found '%s'", *AssetList[Index].GetFullName());
		UFlareSpacecraftCatalogEntry* Spacecraft = Cast<UFlareSpacecraftCatalogEntry>(AssetList[Index].GetAsset());
		FCHECK(Spacecraft);

		bool AddToCatalog = true;
		if (Spacecraft->Data.IsDisabledIfModsNotLoaded.Num())
		{
			if (GameMode->GetModStrings().Num())
			{
				bool FoundMod = false;
				for (FString MenuModStrings : GameMode->GetModStrings())
				{
					if (Spacecraft->Data.IsDisabledIfModsNotLoaded.Find(MenuModStrings))
					{
						FoundMod = true;
						break;
					}
				}
				if (!FoundMod)
				{
					AddToCatalog = false;
				}
			}
			else
			{
				AddToCatalog = false;
			}
		}

		if (AddToCatalog && Spacecraft->Data.IsEnabledIfModsNotLoaded.Num())
		{
			if (GameMode->GetModStrings().Num())
			{
				for (FString MenuModStrings : GameMode->GetModStrings())
				{
					if (Spacecraft->Data.IsEnabledIfModsNotLoaded.Find(MenuModStrings))
					{
						AddToCatalog = false;
						break;
					}
				}
			}
		}

		if (AddToCatalog)
		{
			if (Spacecraft->Data.IsStation())
			{
				StationCatalog.Add(Spacecraft);
			}
			else
			{
				ShipCatalog.Add(Spacecraft);
			}
		}
	}

	for (int32 Index = 0; Index < StationCatalog.Num(); Index++)
	{
		UFlareSpacecraftCatalogEntry* Spacecraft = StationCatalog[Index];
		UFlareSpacecraftCatalogEntry* OldEntry = NULL;
		for (UFlareSpacecraftCatalogEntry* ShipSub : StationCatalog)
		{
			if (ShipSub != Spacecraft && ShipSub->Data.Identifier == Spacecraft->Data.Identifier && ShipSub->Data.ModLoadPriority <= Spacecraft->Data.ModLoadPriority)
			{
				OldEntry = ShipSub;
				break;
			}
		}

		if (OldEntry)
		{
			if (Spacecraft->Data.IsDisabledOverrideStats)
			{
				FFlareSpacecraftDescription* OldEntryDesc = &OldEntry->Data;
				if (OldEntryDesc)
				{
					ReplaceOldEntrySettings(OldEntryDesc, Spacecraft);
				}
			}
			else
			{
				if (StationCatalog.Remove(OldEntry))
				{
					Index = FMath::Min(0, Index -= 1);
				}
			}
		}
		if (Spacecraft->Data.IsDisabled)
		{
			if (StationCatalog.Remove(Spacecraft))
			{
				Index = FMath::Min(0, Index -= 1);
			}
		}
	}

	for (int32 Index = 0; Index < ShipCatalog.Num(); Index++)
	{
		UFlareSpacecraftCatalogEntry* Spacecraft = ShipCatalog[Index];
		UFlareSpacecraftCatalogEntry* OldEntry = NULL;
		for (UFlareSpacecraftCatalogEntry* ShipSub : ShipCatalog)
		{
			if (ShipSub != Spacecraft && ShipSub->Data.Identifier == Spacecraft->Data.Identifier && ShipSub->Data.ModLoadPriority <= Spacecraft->Data.ModLoadPriority)
			{
				OldEntry = ShipSub;
				break;
			}
		}
		if (OldEntry)
		{
			if (Spacecraft->Data.IsDisabledOverrideStats)
			{
				FFlareSpacecraftDescription* OldEntryDesc = &OldEntry->Data;
				if (OldEntryDesc)
				{
					ReplaceOldEntrySettings(OldEntryDesc, Spacecraft);
				}
			}
			else
			{
				if (ShipCatalog.Remove(OldEntry))
				{
					Index = FMath::Min(0, Index-= 1);
				}
			}

		}
		if (Spacecraft->Data.IsDisabled)
		{
			if (ShipCatalog.Remove(Spacecraft))
			{
				Index = FMath::Min(0, Index -= 1);
			}
		}
	}

	StationCatalog.Sort(FSortByEntrySize());
	ShipCatalog.Sort(FSortByEntrySize());
}

void UFlareSpacecraftCatalog::ReplaceOldEntrySettings(FFlareSpacecraftDescription* OldEntryDesc, UFlareSpacecraftCatalogEntry* Spacecraft)
{
	OldEntryDesc->Name = Spacecraft->Data.Name;
	OldEntryDesc->ShortName = Spacecraft->Data.ShortName;
	OldEntryDesc->Description = Spacecraft->Data.Description;

	OldEntryDesc->Mass = Spacecraft->Data.Mass;
	OldEntryDesc->AngularMaxVelocity = Spacecraft->Data.AngularMaxVelocity;
	OldEntryDesc->LinearMaxVelocity = Spacecraft->Data.LinearMaxVelocity;
	OldEntryDesc->HeatCapacity = Spacecraft->Data.HeatCapacity;

	OldEntryDesc->CombatPoints = Spacecraft->Data.CombatPoints;
	OldEntryDesc->MaxLevel = Spacecraft->Data.MaxLevel;
	OldEntryDesc->CapturePointThreshold = Spacecraft->Data.CapturePointThreshold;

	OldEntryDesc->CycleCost.ProductionTime = Spacecraft->Data.CycleCost.ProductionTime;
	OldEntryDesc->CycleCost.ProductionCost = Spacecraft->Data.CycleCost.ProductionCost;

	OldEntryDesc->CycleCost.InputResources.Empty();
	for (FFlareFactoryResource FactoryResource : Spacecraft->Data.CycleCost.InputResources)
	{
		OldEntryDesc->CycleCost.InputResources.Add(FactoryResource);
	}

	OldEntryDesc->CycleCost.OutputResources.Empty();
	for (FFlareFactoryResource FactoryResource : Spacecraft->Data.CycleCost.OutputResources)
	{
		OldEntryDesc->CycleCost.OutputResources.Add(FactoryResource);
	}

	if (Spacecraft->Data.WeaponGroups.Num() > 0)
	{
		OldEntryDesc->WeaponGroups.Empty();
		for (FFlareSpacecraftSlotGroupDescription NewWeaponGroup : Spacecraft->Data.WeaponGroups)
		{
			OldEntryDesc->WeaponGroups.Add(NewWeaponGroup);
		}
	}

	if (Spacecraft->Data.InternalComponentSlots.Num() > 0)
	{
		OldEntryDesc->InternalComponentSlots.Empty();
		for (FFlareSpacecraftSlotDescription NewInternalComponent : Spacecraft->Data.InternalComponentSlots)
		{
			OldEntryDesc->InternalComponentSlots.Add(NewInternalComponent);
		}
	}

	if (Spacecraft->Data.Factories.Num() > 0)
	{
		OldEntryDesc->Factories.Empty();
		for (UFlareFactoryCatalogEntry* NewFactory : Spacecraft->Data.Factories)
		{
			OldEntryDesc->Factories.Add(NewFactory);
		}
	}

	OldEntryDesc->CargoBayCapacity = Spacecraft->Data.CargoBayCapacity;
	OldEntryDesc->CargoBayCount = Spacecraft->Data.CargoBayCount;

	OldEntryDesc->DroneLaunchDelay = Spacecraft->Data.DroneLaunchDelay;
	OldEntryDesc->IsUncapturable = Spacecraft->Data.IsUncapturable;
	OldEntryDesc->DroneMaximum = Spacecraft->Data.DroneMaximum;
	OldEntryDesc->IsDroneCarrier = Spacecraft->Data.IsDroneCarrier;
	OldEntryDesc->IsDroneShip = Spacecraft->Data.IsDroneShip;
}

/*----------------------------------------------------
	Data getters
----------------------------------------------------*/

const void UFlareSpacecraftCatalog::GetSpacecraftList(TArray<FFlareSpacecraftDescription*>& OutData)
{
	for (UFlareSpacecraftCatalogEntry* ShipSub : ShipCatalog)
	{
		FFlareSpacecraftDescription Candidate = ShipSub->Data;
		OutData.Add(&Candidate);
	}
}

FFlareSpacecraftDescription* UFlareSpacecraftCatalog::Get(FName Identifier) const
{
	auto FindByName = [=](const UFlareSpacecraftCatalogEntry* Candidate)
	{
		return Candidate->Data.Identifier == Identifier;
	};

	UFlareSpacecraftCatalogEntry* const* Entry = ShipCatalog.FindByPredicate(FindByName);
	if (Entry && *Entry)
	{
		return &((*Entry)->Data);
	}

	Entry = StationCatalog.FindByPredicate(FindByName);
	if (Entry && *Entry)
	{
		return &((*Entry)->Data);
	}

	return NULL;
}