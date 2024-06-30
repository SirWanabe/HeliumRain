
#include "FlareSpacecraftComponentsCatalog.h"
#include "../Flare.h"
#include "../Player/FlarePlayerController.h"
#include "AssetRegistryModule.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

static bool SortByWeaponType(const UFlareSpacecraftComponentsCatalogEntry& A, const UFlareSpacecraftComponentsCatalogEntry& B)
{
	if (A.Data.WeaponCharacteristics.BombCharacteristics.IsBomb && !B.Data.WeaponCharacteristics.BombCharacteristics.IsBomb)
	{
		return false;
	}
	else
	{
		return (A.Data.CombatPoints > B.Data.CombatPoints);
	}
}

static bool SortByCost(const UFlareSpacecraftComponentsCatalogEntry& A, const UFlareSpacecraftComponentsCatalogEntry& B)
{
	return (A.Data.Cost > B.Data.Cost);
}


UFlareSpacecraftComponentsCatalog::UFlareSpacecraftComponentsCatalog(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}

void UFlareSpacecraftComponentsCatalog::InitialSetup(AFlareGame* GameMode)
{
	TArray<FAssetData> AssetList;
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	Registry.SearchAllAssets(true);
	Registry.GetAssetsByClass(UFlareSpacecraftComponentsCatalogEntry::StaticClass()->GetFName(), AssetList);

	for (int32 Index = 0; Index < AssetList.Num(); Index++)
	{
		//FLOGV("UFlareSpacecraftComponentsCatalog::UFlareSpacecraftComponentsCatalog : Found '%s'", *AssetList[Index].GetFullName());
		UFlareSpacecraftComponentsCatalogEntry* SpacecraftComponent = Cast<UFlareSpacecraftComponentsCatalogEntry>(AssetList[Index].GetAsset());
		FCHECK(SpacecraftComponent);

		bool AddToCatalog = true;
		if (SpacecraftComponent->Data.IsDisabledIfModsNotLoaded.Num())
		{
			if (GameMode->GetModStrings().Num())
			{
				bool FoundMod = false;
				for (FString MenuModStrings : GameMode->GetModStrings())
				{
					if (SpacecraftComponent->Data.IsDisabledIfModsNotLoaded.Find(MenuModStrings))
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

		if (AddToCatalog && SpacecraftComponent->Data.IsEnabledIfModsNotLoaded.Num())
		{
			if (GameMode->GetModStrings().Num())
			{
				for (FString MenuModStrings : GameMode->GetModStrings())
				{
					if (SpacecraftComponent->Data.IsEnabledIfModsNotLoaded.Find(MenuModStrings))
					{
						AddToCatalog = false;
						break;
					}
				}
			}
		}

		if (AddToCatalog)
		{
			if (SpacecraftComponent->Data.Type == EFlarePartType::OrbitalEngine)
			{
				EngineCatalog.Add(SpacecraftComponent);
			}

			else if (SpacecraftComponent->Data.Type == EFlarePartType::RCS)
			{
				RCSCatalog.Add(SpacecraftComponent);
			}

			else if (SpacecraftComponent->Data.Type == EFlarePartType::Weapon)
			{
				WeaponCatalog.Add(SpacecraftComponent);
			}

			else if (SpacecraftComponent->Data.Type == EFlarePartType::InternalComponent)
			{
				InternalComponentsCatalog.Add(SpacecraftComponent);
			}

			else if (SpacecraftComponent->Data.Type == EFlarePartType::Meta)
			{
				MetaCatalog.Add(SpacecraftComponent);
			}
		}
	}

	SetupModArrays(EngineCatalog);
	SetupModArrays(RCSCatalog);
	SetupModArrays(WeaponCatalog);
	SetupModArrays(InternalComponentsCatalog);
	SetupModArrays(MetaCatalog);

	// Sort entries for the upgrade menu
	EngineCatalog.Sort(SortByCost);
	RCSCatalog.Sort(SortByCost);
	WeaponCatalog.Sort(SortByWeaponType);
}

void UFlareSpacecraftComponentsCatalog::SetupModArrays(TArray<UFlareSpacecraftComponentsCatalogEntry*>& PassedArray)
{
	for (int32 Index = 0; Index < PassedArray.Num(); Index++)
	{
		UFlareSpacecraftComponentsCatalogEntry* ComponentEntry = PassedArray[Index];
		UFlareSpacecraftComponentsCatalogEntry* OldEntry = NULL;
		for (UFlareSpacecraftComponentsCatalogEntry* ComponentSub : PassedArray)
		{
			if (ComponentSub != ComponentEntry && ComponentSub->Data.Identifier == ComponentEntry->Data.Identifier && ComponentSub->Data.ModLoadPriority <= ComponentEntry->Data.ModLoadPriority)
			{
				OldEntry = ComponentSub;
				break;
			}
		}

		if (OldEntry)
		{
			if (ComponentEntry->Data.IsDisabledOverrideStats)
			{
				FFlareSpacecraftComponentDescription* OldEntryDesc = &OldEntry->Data;
				if (OldEntryDesc)
				{
					ReplaceOldEntrySettings(OldEntryDesc, ComponentEntry);
					if (PassedArray.Remove(OldEntry))
					{
						Index = FMath::Min(0, Index -= 1);
					}
				}
			}
			else
			{
				if (PassedArray.Remove(OldEntry))
				{
					Index = FMath::Min(0, Index -= 1);
				}
			}
		}

		if (ComponentEntry->Data.IsDisabled)
		{
			if (PassedArray.Remove(ComponentEntry))
			{
				Index = FMath::Min(0, Index -= 1);
			}
		}

	}
}

void UFlareSpacecraftComponentsCatalog::ReplaceOldEntrySettings(FFlareSpacecraftComponentDescription* OldEntryDesc, UFlareSpacecraftComponentsCatalogEntry* Component)
{
	OldEntryDesc->Name = Component->Data.Name;
	OldEntryDesc->Description = Component->Data.Description;
	OldEntryDesc->Cost = Component->Data.Cost;
	OldEntryDesc->CombatPoints = Component->Data.CombatPoints;
	OldEntryDesc->RepairCost = Component->Data.RepairCost;
	OldEntryDesc->Armor = Component->Data.Armor;
	OldEntryDesc->HitPoints = Component->Data.HitPoints;

	if (Component->Data.Mesh)
	{
		OldEntryDesc->Mesh = Component->Data.Mesh;
	}
	if (Component->Data.DestroyedEffect)
	{
		OldEntryDesc->DestroyedEffect = Component->Data.DestroyedEffect;
	}
	if (Component->Data.DamagedEffect)
	{
		OldEntryDesc->DamagedEffect = Component->Data.DamagedEffect;
	}
	if (Component->Data.MeshPreviewBrush.GetImageType() != ESlateBrushImageType::NoImage)
	{
		OldEntryDesc->MeshPreviewBrush = Component->Data.MeshPreviewBrush;
	}

	OldEntryDesc->GeneralCharacteristics.LifeSupport = Component->Data.GeneralCharacteristics.LifeSupport;
	OldEntryDesc->GeneralCharacteristics.ElectricSystem = Component->Data.GeneralCharacteristics.ElectricSystem;
	OldEntryDesc->GeneralCharacteristics.HeatSink = Component->Data.GeneralCharacteristics.HeatSink;
	OldEntryDesc->GeneralCharacteristics.HeatProduction = Component->Data.GeneralCharacteristics.HeatProduction;
	OldEntryDesc->GeneralCharacteristics.Cargo = Component->Data.GeneralCharacteristics.Cargo;

	OldEntryDesc->EngineCharacteristics.IsEngine = Component->Data.EngineCharacteristics.IsEngine;
	OldEntryDesc->EngineCharacteristics.EnginePower = Component->Data.EngineCharacteristics.EnginePower;
	OldEntryDesc->EngineCharacteristics.AngularAccelerationRate = Component->Data.EngineCharacteristics.AngularAccelerationRate;
	if(Component->Data.EngineCharacteristics.EngineSound)
	{
		OldEntryDesc->EngineCharacteristics.EngineSound = Component->Data.EngineCharacteristics.EngineSound;
	}

	OldEntryDesc->WeaponCharacteristics.IsWeapon = Component->Data.WeaponCharacteristics.IsWeapon;
	OldEntryDesc->WeaponCharacteristics.Order = Component->Data.WeaponCharacteristics.Order;
	OldEntryDesc->WeaponCharacteristics.RefillCost = Component->Data.WeaponCharacteristics.RefillCost;
	OldEntryDesc->WeaponCharacteristics.AlternedWeapon = Component->Data.WeaponCharacteristics.AlternedWeapon;
	OldEntryDesc->WeaponCharacteristics.DamageType = Component->Data.WeaponCharacteristics.DamageType;
	OldEntryDesc->WeaponCharacteristics.ExplosionPower = Component->Data.WeaponCharacteristics.ExplosionPower;
	OldEntryDesc->WeaponCharacteristics.AmmoFragmentCount = Component->Data.WeaponCharacteristics.AmmoFragmentCount;
	OldEntryDesc->WeaponCharacteristics.AmmoDamageRadius = Component->Data.WeaponCharacteristics.AmmoDamageRadius;
	OldEntryDesc->WeaponCharacteristics.AmmoCapacity = Component->Data.WeaponCharacteristics.AmmoCapacity;
	OldEntryDesc->WeaponCharacteristics.FuzeType = Component->Data.WeaponCharacteristics.FuzeType;
	OldEntryDesc->WeaponCharacteristics.FuzeMinDistanceThresold = Component->Data.WeaponCharacteristics.FuzeMinDistanceThresold;
	OldEntryDesc->WeaponCharacteristics.FuzeMaxDistanceThresold = Component->Data.WeaponCharacteristics.FuzeMaxDistanceThresold;
	if (Component->Data.WeaponCharacteristics.ImpactSound)
	{
		OldEntryDesc->WeaponCharacteristics.ImpactSound = Component->Data.WeaponCharacteristics.ImpactSound;
	}
	if (Component->Data.WeaponCharacteristics.DamageSound)
	{
		OldEntryDesc->WeaponCharacteristics.DamageSound = Component->Data.WeaponCharacteristics.DamageSound;
	}
	if (Component->Data.WeaponCharacteristics.FiringSound)
	{
		OldEntryDesc->WeaponCharacteristics.FiringSound = Component->Data.WeaponCharacteristics.FiringSound;
	}
	if (Component->Data.WeaponCharacteristics.ExplosionEffect)
	{
		OldEntryDesc->WeaponCharacteristics.ExplosionEffect = Component->Data.WeaponCharacteristics.ExplosionEffect;
	}
	OldEntryDesc->WeaponCharacteristics.ExplosionEffectScale = Component->Data.WeaponCharacteristics.ExplosionEffectScale;
	if (Component->Data.WeaponCharacteristics.ImpactEffect)
	{
		OldEntryDesc->WeaponCharacteristics.ImpactEffect = Component->Data.WeaponCharacteristics.ImpactEffect;
	}

	OldEntryDesc->WeaponCharacteristics.ImpactEffectScale = Component->Data.WeaponCharacteristics.ImpactEffectScale;

	OldEntryDesc->WeaponCharacteristics.GunCharacteristics.IsGun = Component->Data.WeaponCharacteristics.GunCharacteristics.IsGun;
	OldEntryDesc->WeaponCharacteristics.GunCharacteristics.KineticEnergy = Component->Data.WeaponCharacteristics.GunCharacteristics.KineticEnergy;
	OldEntryDesc->WeaponCharacteristics.GunCharacteristics.AmmoRate = Component->Data.WeaponCharacteristics.GunCharacteristics.AmmoRate;
	OldEntryDesc->WeaponCharacteristics.GunCharacteristics.AmmoVelocity = Component->Data.WeaponCharacteristics.GunCharacteristics.AmmoVelocity;
	OldEntryDesc->WeaponCharacteristics.GunCharacteristics.AmmoRange = Component->Data.WeaponCharacteristics.GunCharacteristics.AmmoRange;
	OldEntryDesc->WeaponCharacteristics.GunCharacteristics.AmmoPrecision = Component->Data.WeaponCharacteristics.GunCharacteristics.AmmoPrecision;
	OldEntryDesc->WeaponCharacteristics.GunCharacteristics.GunCount = Component->Data.WeaponCharacteristics.GunCharacteristics.GunCount;
	OldEntryDesc->WeaponCharacteristics.GunCharacteristics.AlternedFire = Component->Data.WeaponCharacteristics.GunCharacteristics.AlternedFire;
	OldEntryDesc->WeaponCharacteristics.GunCharacteristics.SemiAutomaticFire = Component->Data.WeaponCharacteristics.GunCharacteristics.SemiAutomaticFire;

	if (Component->Data.WeaponCharacteristics.GunCharacteristics.FiringEffect)
	{
		OldEntryDesc->WeaponCharacteristics.GunCharacteristics.FiringEffect = Component->Data.WeaponCharacteristics.GunCharacteristics.FiringEffect;
	}
	if (Component->Data.WeaponCharacteristics.GunCharacteristics.TracerEffect)
	{
		OldEntryDesc->WeaponCharacteristics.GunCharacteristics.TracerEffect = Component->Data.WeaponCharacteristics.GunCharacteristics.TracerEffect;
	}
	if (Component->Data.WeaponCharacteristics.GunCharacteristics.ExplosionMaterial)
	{
		OldEntryDesc->WeaponCharacteristics.GunCharacteristics.ExplosionMaterial = Component->Data.WeaponCharacteristics.GunCharacteristics.ExplosionMaterial;
	}

	OldEntryDesc->WeaponCharacteristics.TurretCharacteristics.IsTurret = Component->Data.WeaponCharacteristics.TurretCharacteristics.IsTurret;
	OldEntryDesc->WeaponCharacteristics.TurretCharacteristics.TurretAngularVelocity = Component->Data.WeaponCharacteristics.TurretCharacteristics.TurretAngularVelocity;
	OldEntryDesc->WeaponCharacteristics.TurretCharacteristics.BarrelsAngularVelocity = Component->Data.WeaponCharacteristics.TurretCharacteristics.BarrelsAngularVelocity;
	OldEntryDesc->WeaponCharacteristics.TurretCharacteristics.TurretMaxAngle = Component->Data.WeaponCharacteristics.TurretCharacteristics.TurretMaxAngle;
	OldEntryDesc->WeaponCharacteristics.TurretCharacteristics.TurretMinAngle = Component->Data.WeaponCharacteristics.TurretCharacteristics.TurretMinAngle;
	OldEntryDesc->WeaponCharacteristics.TurretCharacteristics.BarrelsMaxAngle = Component->Data.WeaponCharacteristics.TurretCharacteristics.BarrelsMaxAngle;
	OldEntryDesc->WeaponCharacteristics.TurretCharacteristics.BarrelsMinAngle = Component->Data.WeaponCharacteristics.TurretCharacteristics.BarrelsMinAngle;

	if (Component->Data.WeaponCharacteristics.TurretCharacteristics.TurretMesh)
	{
		OldEntryDesc->WeaponCharacteristics.TurretCharacteristics.TurretMesh = Component->Data.WeaponCharacteristics.TurretCharacteristics.TurretMesh;
	}
	if (Component->Data.WeaponCharacteristics.TurretCharacteristics.BarrelsMesh)
	{
		OldEntryDesc->WeaponCharacteristics.TurretCharacteristics.BarrelsMesh = Component->Data.WeaponCharacteristics.TurretCharacteristics.BarrelsMesh;
	}
	if (Component->Data.WeaponCharacteristics.TurretCharacteristics.TurretRotationSound)
	{
		OldEntryDesc->WeaponCharacteristics.TurretCharacteristics.TurretRotationSound = Component->Data.WeaponCharacteristics.TurretCharacteristics.TurretRotationSound;
	}
	if (Component->Data.WeaponCharacteristics.TurretCharacteristics.BarrelRotationSound)
	{
		OldEntryDesc->WeaponCharacteristics.TurretCharacteristics.BarrelRotationSound = Component->Data.WeaponCharacteristics.TurretCharacteristics.BarrelRotationSound;
	}

	OldEntryDesc->WeaponCharacteristics.BombCharacteristics.IsBomb = Component->Data.WeaponCharacteristics.BombCharacteristics.IsBomb;

	if (Component->Data.WeaponCharacteristics.BombCharacteristics.BombMesh)
	{
		OldEntryDesc->WeaponCharacteristics.BombCharacteristics.BombMesh = Component->Data.WeaponCharacteristics.BombCharacteristics.BombMesh;
	}

	OldEntryDesc->WeaponCharacteristics.BombCharacteristics.ActivationDistance = Component->Data.WeaponCharacteristics.BombCharacteristics.ActivationDistance;
	OldEntryDesc->WeaponCharacteristics.BombCharacteristics.DropLinearVelocity = Component->Data.WeaponCharacteristics.BombCharacteristics.DropLinearVelocity;
	OldEntryDesc->WeaponCharacteristics.BombCharacteristics.DropAngularVelocity = Component->Data.WeaponCharacteristics.BombCharacteristics.DropAngularVelocity;
	OldEntryDesc->WeaponCharacteristics.BombCharacteristics.ActivationTime = Component->Data.WeaponCharacteristics.BombCharacteristics.ActivationTime;
	OldEntryDesc->WeaponCharacteristics.BombCharacteristics.MaxAcceleration = Component->Data.WeaponCharacteristics.BombCharacteristics.MaxAcceleration;
	OldEntryDesc->WeaponCharacteristics.BombCharacteristics.NominalVelocity = Component->Data.WeaponCharacteristics.BombCharacteristics.NominalVelocity;
	OldEntryDesc->WeaponCharacteristics.BombCharacteristics.MaxBurnDuration = Component->Data.WeaponCharacteristics.BombCharacteristics.MaxBurnDuration;
	OldEntryDesc->WeaponCharacteristics.BombCharacteristics.AngularAcceleration = Component->Data.WeaponCharacteristics.BombCharacteristics.AngularAcceleration;

	OldEntryDesc->WeaponCharacteristics.AntiSmallShipValue = Component->Data.WeaponCharacteristics.AntiSmallShipValue;
	OldEntryDesc->WeaponCharacteristics.AntiLargeShipValue = Component->Data.WeaponCharacteristics.AntiLargeShipValue;
	OldEntryDesc->WeaponCharacteristics.AntiStationValue = Component->Data.WeaponCharacteristics.AntiStationValue;
}

/*----------------------------------------------------
	Data getters
----------------------------------------------------*/

FFlareSpacecraftComponentDescription* UFlareSpacecraftComponentsCatalog::Get(FName Identifier) const
{
	FFlareSpacecraftComponentDescription* Part = NULL;

	auto FindByName = [=](const UFlareSpacecraftComponentsCatalogEntry* Candidate)
	{
		return Candidate && Candidate->Data.Identifier == Identifier;
	};

	UFlareSpacecraftComponentsCatalogEntry* const* Temp = EngineCatalog.FindByPredicate(FindByName);
	if (Temp)
	{
		Part = &(*Temp)->Data;
	}

	if (!Part)
	{
		Temp = RCSCatalog.FindByPredicate(FindByName);
		if (Temp)
		{
			Part = &(*Temp)->Data;
		}
	}

	if (!Part)
	{
		Temp = WeaponCatalog.FindByPredicate(FindByName);
		if (Temp)
		{
			Part = &(*Temp)->Data;
		}
	}

	if (!Part)
	{
		Temp = InternalComponentsCatalog.FindByPredicate(FindByName);
		if (Temp)
		{
			Part = &(*Temp)->Data;
		}
	}
	
	if (!Part)
	{
		Temp = MetaCatalog.FindByPredicate(FindByName);
		if (Temp)
		{
			Part = &(*Temp)->Data;
		}
	}

	return Part;
}

const void UFlareSpacecraftComponentsCatalog::GetEngineList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size, UFlareCompany* FilterCompany, UFlareSimulatedSpacecraft* FilterShip, FFlareSpacecraftComponentDescription* IgnoreDescription)
{
	for (int32 i = 0; i < EngineCatalog.Num(); i++)
	{
		FFlareSpacecraftComponentDescription& Candidate = EngineCatalog[i]->Data;
		if (FilterShip && FilterShip->GetDescription()->RestrictedEngines.Num())
		{
			if (FilterShip->GetDescription()->RestrictedEngines.Find(Candidate.Identifier) == INDEX_NONE)
			{
				continue;
			}
		}

		if (Candidate.Size == Size && (FilterCompany == NULL || FilterCompany->IsTechnologyUnlockedPart(&Candidate)) && (FilterShip == NULL || !FilterCompany->IsPartRestricted(&Candidate, FilterShip)))
		{
			if (IgnoreDescription && Candidate.Identifier == IgnoreDescription->Identifier)
			{
				continue;
			}
			OutData.Add(&Candidate);
		}
	}
}

const void UFlareSpacecraftComponentsCatalog::GetEngineList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size, UFlareCompany* FilterCompany, FName FilterShip, FFlareSpacecraftDescription* FilterDescription, FFlareSpacecraftComponentDescription* IgnoreDescription)
{
	for (int32 i = 0; i < EngineCatalog.Num(); i++)
	{
		FFlareSpacecraftComponentDescription& Candidate = EngineCatalog[i]->Data;
		if (FilterDescription && FilterDescription->RestrictedEngines.Num())
		{
			if (FilterDescription->RestrictedEngines.Find(Candidate.Identifier) == INDEX_NONE)
			{
				continue;
			}
		}

		if (Candidate.Size == Size && (FilterCompany == NULL || FilterCompany->IsTechnologyUnlockedPart(&Candidate)) && (FilterShip == NAME_None || !FilterCompany->IsPartRestricted(&Candidate, FilterShip)))
		{
			if (IgnoreDescription && Candidate.Identifier == IgnoreDescription->Identifier)
			{
				continue;
			}
			OutData.Add(&Candidate);
		}
	}
}

const void UFlareSpacecraftComponentsCatalog::GetRCSList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size, UFlareCompany* FilterCompany, UFlareSimulatedSpacecraft* FilterShip, FFlareSpacecraftComponentDescription* IgnoreDescription)
{
	for (int32 i = 0; i < RCSCatalog.Num(); i++)
	{
		FFlareSpacecraftComponentDescription& Candidate = RCSCatalog[i]->Data;
		if (FilterShip && FilterShip->GetDescription()->RestrictedRCS.Num())
		{
			if (FilterShip->GetDescription()->RestrictedRCS.Find(Candidate.Identifier) == INDEX_NONE)
			{
				continue;
			}
		}

		if (Candidate.Size == Size && (FilterCompany == NULL || FilterCompany->IsTechnologyUnlockedPart(&Candidate)) && (FilterShip == NULL || !FilterCompany->IsPartRestricted(&Candidate, FilterShip)))
		{
			if (IgnoreDescription && Candidate.Identifier == IgnoreDescription->Identifier)
			{
				continue;
			}
			OutData.Add(&Candidate);
		}
	}
}

const void UFlareSpacecraftComponentsCatalog::GetRCSList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size, UFlareCompany* FilterCompany, FName FilterShip, FFlareSpacecraftDescription* FilterDescription, FFlareSpacecraftComponentDescription* IgnoreDescription)
{
	for (int32 i = 0; i < RCSCatalog.Num(); i++)
	{
		FFlareSpacecraftComponentDescription& Candidate = RCSCatalog[i]->Data;

		if (FilterDescription && FilterDescription->RestrictedRCS.Num())
		{
			if (FilterDescription->RestrictedRCS.Find(Candidate.Identifier) == INDEX_NONE)
			{
				continue;
			}
		}

		if (Candidate.Size == Size && (FilterCompany == NULL || FilterCompany->IsTechnologyUnlockedPart(&Candidate)) && (FilterShip == NAME_None || !FilterCompany->IsPartRestricted(&Candidate, FilterShip)))
		{
			if (IgnoreDescription && Candidate.Identifier == IgnoreDescription->Identifier)
			{
				continue;
			}
			OutData.Add(&Candidate);
		}
	}
}

const void UFlareSpacecraftComponentsCatalog::GetWeaponList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size, UFlareCompany* FilterCompany, UFlareSimulatedSpacecraft* FilterShip, FFlareSpacecraftSlotGroupDescription* WeaponGroupDesc, FFlareSpacecraftComponentDescription* IgnoreDescription)
{
	for (int32 i = 0; i < WeaponCatalog.Num(); i++)
	{
		FFlareSpacecraftComponentDescription& Candidate = WeaponCatalog[i]->Data;

		if (WeaponGroupDesc && WeaponGroupDesc->RestrictedWeapons.Num())
		{
			if (WeaponGroupDesc->RestrictedWeapons.Find(Candidate.Identifier) == INDEX_NONE)
			{
				continue;
			}
		}

		if (Candidate.Size == Size && (FilterCompany == NULL || FilterCompany->IsTechnologyUnlockedPart(&Candidate)) && (FilterShip == NULL || !FilterCompany->IsPartRestricted(&Candidate,FilterShip)))
		{
			if (IgnoreDescription && Candidate.Identifier == IgnoreDescription->Identifier)
			{
				continue;
			}
			OutData.Add(&Candidate);
		}
	}
}

const void UFlareSpacecraftComponentsCatalog::GetWeaponList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size, UFlareCompany* FilterCompany, FName FilterShip, FFlareSpacecraftSlotGroupDescription* WeaponGroupDesc, FFlareSpacecraftComponentDescription* IgnoreDescription)
{
	for (int32 i = 0; i < WeaponCatalog.Num(); i++)
	{
		FFlareSpacecraftComponentDescription& Candidate = WeaponCatalog[i]->Data;

		if (WeaponGroupDesc && WeaponGroupDesc->RestrictedWeapons.Num())
		{
			if (WeaponGroupDesc->RestrictedWeapons.Find(Candidate.Identifier) == INDEX_NONE)
			{
				continue;
			}
		}

		if (Candidate.Size == Size && (FilterCompany == NULL || FilterCompany->IsTechnologyUnlockedPart(&Candidate)) && (FilterShip == NAME_None || !FilterCompany->IsPartRestricted(&Candidate, FilterShip)))
		{
			if (IgnoreDescription && Candidate.Identifier == IgnoreDescription->Identifier)
			{
				continue;
			}
			OutData.Add(&Candidate);
		}
	}
}