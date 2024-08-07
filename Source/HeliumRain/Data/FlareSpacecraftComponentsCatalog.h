#pragma once

#include "FlareSpacecraftComponentsCatalogEntry.h"
#include "FlareSpacecraftComponentsCatalog.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareSpacecraftComponentsCatalog : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/
	
	/** Orbital engines */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareSpacecraftComponentsCatalogEntry*> EngineCatalog;

	/** RCS engines */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareSpacecraftComponentsCatalogEntry*> RCSCatalog;

	/** Weapons */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareSpacecraftComponentsCatalogEntry*> WeaponCatalog;

	/** Internal modules */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareSpacecraftComponentsCatalogEntry*> InternalComponentsCatalog;

	/** Meta objects */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareSpacecraftComponentsCatalogEntry*> MetaCatalog;

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	void InitialSetup(AFlareGame* GameMode);
	void SetupModArrays(TArray<UFlareSpacecraftComponentsCatalogEntry*>& PassedArray);

	void ReplaceOldEntrySettings(FFlareSpacecraftComponentDescription* OldEntryDesc, UFlareSpacecraftComponentsCatalogEntry* Component);

	/** Get a part description */
	FFlareSpacecraftComponentDescription* Get(FName Identifier) const;

	/** Search all engines and get one that fits */
	const void GetEngineList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size, UFlareCompany* FilterCompany = NULL, UFlareSimulatedSpacecraft* FilterShip = NULL, FFlareSpacecraftComponentDescription* IgnoreDescription = NULL);
	const void GetEngineList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size, UFlareCompany* FilterCompany = NULL, FName FilterShip = NAME_None, FFlareSpacecraftDescription* FilterDescription = NULL, FFlareSpacecraftComponentDescription* IgnoreDescription = NULL);

	/** Search all RCS and get one that fits */
	const void GetRCSList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size, UFlareCompany* FilterCompany = NULL, UFlareSimulatedSpacecraft* FilterShip = NULL, FFlareSpacecraftComponentDescription* IgnoreDescription = NULL);
	const void GetRCSList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size, UFlareCompany* FilterCompany = NULL, FName FilterShip = NAME_None, FFlareSpacecraftDescription* FilterDescription = NULL, FFlareSpacecraftComponentDescription* IgnoreDescription = NULL);

	/** Search all weapons and get one that fits */
	const void GetWeaponList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size, UFlareCompany* FilterCompany = NULL, UFlareSimulatedSpacecraft* FilterShip = NULL, FFlareSpacecraftSlotGroupDescription* WeaponGroupDesc = NULL, FFlareSpacecraftComponentDescription* IgnoreDescription = NULL);
	const void GetWeaponList(TArray<FFlareSpacecraftComponentDescription*>& OutData, TEnumAsByte<EFlarePartSize::Type> Size, UFlareCompany* FilterCompany = NULL, FName FilterShip = NAME_None, FFlareSpacecraftSlotGroupDescription* WeaponGroupDesc = NULL, FFlareSpacecraftComponentDescription* IgnoreDescription = NULL);
};