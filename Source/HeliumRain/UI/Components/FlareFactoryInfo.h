#pragma once

#include "../../Flare.h"
#include "Widgets/SCompoundWidget.h"
#include "../FlareUITypes.h"

class UFlareFactory;
struct FFlareResourceDescription;

class SFlareFactoryInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareFactoryInfo)
		: _Factory(NULL)
	{}

	SLATE_ARGUMENT(UFlareFactory*, Factory)
	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Update this factory's limits */
	void UpdateFactoryLimits();

	/** Get the factory */
	UFlareFactory* GetFactory()
	{
		return TargetFactory;
	}


protected:

	/*----------------------------------------------------
		Content callbacks
	----------------------------------------------------*/

	/** Get the factory name */
	FText GetFactoryName() const;

	/** Get the factory descriptipn */
	FText GetFactoryDescription() const;

	/** Get the factory cycle description */
	FText GetFactoryCycleInfo() const;

	/** Get the factory status info */
	FText GetFactoryStatus() const;
	
	/** Get the current progress */
	TOptional<float> GetProductionProgress() const;

	/** Get the current start visibility */
	EVisibility GetStartProductionVisibility() const;

	/** Get the current stop visibility */
	EVisibility GetStopProductionVisibility() const;

	/** Get the current + visibility */
	EVisibility GetIncreaseOutputLimitVisibility(FFlareResourceDescription* Resource) const;

	/** Get the current - visibility */
	EVisibility GetDecreaseOutputLimitVisibility(FFlareResourceDescription* Resource) const;
	

	/*----------------------------------------------------
		Action callbacks
	----------------------------------------------------*/

	/** Start production */
	void OnStartProduction();

	/** Stop production */
	void OnStopProduction();

	/** Refresh ship menu*/
	void RefreshShipMenu();
	
	/** Decrease the output storage limit */
	void OnDecreaseOutputLimit(FFlareResourceDescription* Resource);

	/** Increase the output storage limit */
	void OnIncreaseOutputLimit(FFlareResourceDescription* Resource);


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	// Spacecraft data
	UFlareFactory*                                  TargetFactory;

	// Slate data
	TSharedPtr<SVerticalBox>                        LimitList;
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;


};
