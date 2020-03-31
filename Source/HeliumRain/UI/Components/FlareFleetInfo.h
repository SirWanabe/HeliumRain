#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareCompanyFlag.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Game/FlareCompany.h"

#include "../../Spacecrafts/Subsystems/FlareSimulatedSpacecraftDamageSystem.h"

DECLARE_DELEGATE_OneParam(FFlareObjectRemoved, UFlareSimulatedSpacecraft*)


class SFlareFleetInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareFleetInfo)
		: _Player(NULL)
		, _Fleet(NULL)
		, _OwnerWidget(NULL)
		, _Minimized(false)
	{}

	SLATE_ARGUMENT(AFlarePlayerController*, Player)
		SLATE_ARGUMENT(UFlareFleet*, Fleet)
		SLATE_ARGUMENT(SWidget*, OwnerWidget)

		SLATE_ARGUMENT(bool, Minimized)

		SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Set a fleet as content */
	void SetFleet(UFlareFleet* Fleet);

	/** Set the minimized mode */
	void SetMinimized(bool NewState);

	/** Get the refill text */
	FText GetRefillText() const;

	/** Visibility setting for the refill button */
	bool IsRefillDisabled() const;

	/** Refill all fleets */
	void OnRefillClicked();

	/** Get the repair text */
	FText GetRepairText() const;

	/** Visibility setting for the repair button */
	bool IsRepairDisabled() const;

	/** Repair all fleets */
	void OnRepairClicked();

	/** SHow the menu */
	void Show();

	/** Hide the menu */
	void Hide();

	/** Upgrade fleet status icons */
	void UpdateFleetStatus();


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Inspect the current target */
	void OnInspect();

	/** Open the trade route menu */
	void OnOpenTradeRoute();

	/** Toggle auto-trading */
	void OnToggleAutoTrade();


	/*----------------------------------------------------
		Content
	----------------------------------------------------*/

	/** Get the target name */
	FText GetName() const;

	/** Get the text color */
	FSlateColor GetTextColor() const;


	/** Get the edit hint text */
	FText GetInspectHintText() const;

	/** Can we inspect */
	bool IsInspectDisabled() const;

	/** Get the trade route hint text */
	FText GetInspectTradeRouteHintText() const;

	/** Can we inspect the trade route */
	bool IsInspectTradeRouteDisabled() const;

	/** Get the auto-trade hint text */
	FText GetAutoTradeHintText() const;

	/** Can we toggle auto trade */
	bool IsAutoTradeDisabled() const;


	/** Get the fleet composition */
	FText GetComposition() const;

	/** Get the fleet combat value */
	FText GetCombatValue() const;

	/** Get the fleet description */
	FText GetDescription() const;

	/** Hide the company flag if owned */
	EVisibility GetCompanyFlagVisibility() const;

protected:
	/** Get the current health */
	TOptional<float> GetGlobalHealth() const;
	FSlateColor GetIconColor(EFlareSubsystem::Type Type) const;
	EVisibility GetRefillingVisibility() const;
	EVisibility GetRepairingVisibility() const;

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Game data
	AFlarePlayerController*           PC;
	bool                              Minimized;
	bool							  IsStranded;
	bool							  IsUncontrollable;
	bool							  IsDisarmed;

	bool							  NeedRefill;
	bool							  IsRepairing;
	float							  FleetHealth;

	// Target data	
	UFlareFleet*                      TargetFleet;
	FText                             TargetName;

	// Slate data (buttons)
	TSharedPtr<SFlareButton>          InspectButton;
	TSharedPtr<SFlareButton>          TradeRouteButton;
	TSharedPtr<SFlareButton>          AutoTradeButton;
	TSharedPtr<SFlareButton>          RefillButton;
	TSharedPtr<SFlareButton>          RepairButton;

	// Slate data (various)
	TSharedPtr<STextBlock>            FleetName;
	TSharedPtr<SWidget>               OwnerWidget;
	TSharedPtr<SFlareCompanyFlag>     CompanyFlag;
};
