#pragma once

#include "../../Flare.h"
#include "../../Spacecrafts/FlareSpacecraft.h"
#include "../HUD/FlareSubsystemStatus.h"
#include "../HUD/FlareWeaponStatus.h"
#include "../HUD/FlareMouseMenu.h"


class SFlareHUDMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareHUDMenu){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Set the ship to display data for */
	void OnPlayerShipChanged();

	void HudMenuTick(float InDeltaTime);

protected:

	/*----------------------------------------------------
		Events
	----------------------------------------------------*/

//	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Get the visibility of the top panel depending on overlay visibility */
	EVisibility GetTopPanelVisibility() const;

	EVisibility GetDockingVisibility() const;

	/** Get the current information to show */
	FText GetInfoText() const;

	/** Get the current secondary information to show */
	FText GetWarningText() const;

	/** Get the ratio of temperature in the "minimal / max acceptable" range */
	TOptional<float> GetTemperatureProgress() const;

	/** Get the color of the temperature bar */
	FSlateColor GetTemperatureColor() const;
	FSlateColor GetTemperatureColorNoAlpha() const;

	/** Get the current temperature */
	FText GetTemperatureText() const;

	/** Get the color and alpha of the overheating warning */
	FSlateColor GetOverheatColor(bool Text) const;
	
	/** Get the color and alpha of the Outage warning */
	FSlateColor GetOutageColor(bool Text) const;
	
	/** Get the text for the outage duration */
	FText GetOutageText() const;

	FSlateColor GetDockingDistanceColor() const;
	FSlateColor GetDockingDistanceColorNoAlpha() const;
	TOptional<float> GetDockingDistanceProgress() const;
	FText GetDockingDistanceText() const;

	FSlateColor GetDockingLinearColor() const;
	FSlateColor GetDockingLinearColorNoAlpha() const;
	TOptional<float> GetDockingLinearProgress() const;
	FText GetDockingLinearText() const;

	FSlateColor GetDockingRollColor() const;
	FSlateColor GetDockingRollColorNoAlpha() const;
	TOptional<float> GetDockingRollProgress() const;
	FText GetDockingRollText() const;

	FSlateColor GetDockingAngularColor() const;
	FSlateColor GetDockingAngularColorNoAlpha() const;
	TOptional<float> GetDockingAngularProgress() const;
	FText GetDockingAngularText() const;

	void SetDockingInfoToDefaults();
	
protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** HUD reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>      MenuManager;

	// Menu components
	TSharedPtr<STextBlock>               InfoText;
	TSharedPtr<STextBlock>               LowerInfoText;
	TSharedPtr<SFlareSubsystemStatus>    TemperatureStatus;
	TSharedPtr<SFlareSubsystemStatus>    PowerStatus;
	TSharedPtr<SFlareSubsystemStatus>    PropulsionStatus;
	TSharedPtr<SFlareSubsystemStatus>    RCSStatus;
	TSharedPtr<SFlareSubsystemStatus>    LifeSupportStatus;
	TSharedPtr<SFlareSubsystemStatus>    WeaponStatus;
	TSharedPtr<SVerticalBox>             WeaponContainer;

	// Target data
	float                                Temperature;
	float                                OverheatTemperature;
	bool                                 Overheating;
	bool                                 PowerOutage;

	// Effect data
	float                                PresentationFlashTime;
	float                                TimeSinceOverheatChanged;
	float                                TimeSinceOutageChanged;

	bool DockingInProgress;

	float DockingDistanceRatio;
	FText DockingDistanceText;

	float DockingLinearRatio;
	FText DockingLinearText;

	float DockingRollRatio;
	FText DockingRollText;

	float DockingAngularRatio;
	FText DockingAngularText;
};
