#pragma once

#include "Object.h"
#include "FlareCompanyWhiteList.generated.h"

USTRUCT()
struct FResourcesWhiteList
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY()
	TArray<FName> Entries;
};


/** Trade route conditions data */
USTRUCT()
struct FFlareWhiteListCompanyDataSave
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> ResourcesTradeTo;
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> ResourcesTradeFrom;

	UPROPERTY(EditAnywhere, Category = Save)
	bool CanTradeTo;
	UPROPERTY(EditAnywhere, Category = Save)
	bool CanTradeFrom;
};


USTRUCT()
struct FFlareWhiteListSave
{
	GENERATED_USTRUCT_BODY()

	/** Given Name */
	UPROPERTY(EditAnywhere, Category = Save)
	FText Name;
	/** identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareWhiteListCompanyDataSave> CompanyData;
	TMap<FName, FFlareWhiteListCompanyDataSave*> CompanyDataTMAPS;

};

UCLASS()
class HELIUMRAIN_API UFlareCompanyWhiteList : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
	  Save
	----------------------------------------------------*/

	virtual void Load(const FFlareWhiteListSave& Data, AFlareGame* NewGame);
	virtual FFlareWhiteListSave* Save();
	void OnDelete();

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	void AddShipToTracker(UFlareSimulatedSpacecraft* Ship);
	void RemoveShipFromTracker(UFlareSimulatedSpacecraft* Ship);
	void AddFleetToTracker(UFlareFleet* Fleet);
	void RemoveFleetFromTracker(UFlareFleet* Fleet);

protected:

	AFlareGame*                            Game;
	FFlareWhiteListSave                    WhiteListData;
	TArray<UFlareSimulatedSpacecraft*>	   ShipsWithWhiteList;
	TArray<UFlareFleet*>				   FleetsWithWhiteList;

public:

	virtual void SetWhiteListName(FText NewName)
	{
		WhiteListData.Name = NewName;
	}

	FFlareWhiteListCompanyDataSave* AddNewCompanyDataSave(FName Identifier);

	void SetWhiteListTradeTo(FFlareWhiteListCompanyDataSave* CompanyData, bool NewValue);
	void SetWhiteListTradeFrom(FFlareWhiteListCompanyDataSave* CompanyData, bool NewValue);

	bool CanCompanyDataTradeTo(FFlareWhiteListCompanyDataSave* CompanyData, FFlareResourceDescription* Resource, FText& Reason);
	bool CanCompanyDataTradeFrom(FFlareWhiteListCompanyDataSave* CompanyData, FFlareResourceDescription* Resource, FText& Reason);

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	FFlareWhiteListCompanyDataSave* GetCompanyDataFor(UFlareCompany* CompanyLookup);
	FFlareWhiteListCompanyDataSave* GetCompanyDataFor(FName IdentifierLookup);

	AFlareGame* GetGame() const
	{
		return Game;
	}

	FText GetWhiteListName() const
	{
		return WhiteListData.Name;
	}

	FName GetWhiteListIdentifier() const
	{
		return WhiteListData.Identifier;
	}
};