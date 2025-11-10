#pragma once

#include "../Flare.h"
#include "../Game/FlareGameTypes.h"
#include "Engine/DataAsset.h"
#include "FlareStartingScenarioCatalogEntry.generated.h"

UCLASS()
class HELIUMRAIN_API UFlareStartingScenarioCatalogEntry : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	UPROPERTY(EditAnywhere, Category = Content)
	FText StartName;

	UPROPERTY(EditAnywhere, Category = Content)
	FText StartDescription;

	UPROPERTY(EditAnywhere, Category = Content)
	int32 StartingTechnologyLevel;

	/** Set to -1 to have research points automatically set to the cost of Auto-Docking research*/
	UPROPERTY(EditAnywhere, Category = Content)
	int32 StartingResearchPoints;

	UPROPERTY(EditAnywhere, Category = Content)
	int64 StartingMoney;

	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FName> StartingSectorKnowledge;

	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FName> StartingTechnology;

	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareCompanyStartingShips> StartingShips;
};