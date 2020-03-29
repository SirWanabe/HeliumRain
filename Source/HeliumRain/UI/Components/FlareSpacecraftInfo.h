#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareCompanyFlag.h"
#include "../Components/FlareShipStatus.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Game/FlareCompany.h"


DECLARE_DELEGATE_OneParam(FFlareObjectRemoved, UFlareSimulatedSpacecraft*)


class SFlareSpacecraftInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareSpacecraftInfo)
		: _Player(NULL)
		, _Spacecraft(NULL)
		, _OwnerWidget(NULL)
		, _NoInspect(false)
		, _Minimized(false)
	{}

	SLATE_ARGUMENT(AFlarePlayerController*, Player)
	SLATE_ARGUMENT(UFlareSimulatedSpacecraft*, Spacecraft)
	SLATE_ARGUMENT(SWidget*, OwnerWidget)

	SLATE_EVENT(FFlareObjectRemoved, OnRemoved)

	SLATE_ARGUMENT(bool, NoInspect)
	SLATE_ARGUMENT(bool, Minimized)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);
	
	/** Set a spacecraft as content */
	void SetSpacecraft(UFlareSimulatedSpacecraft* Target);

	/** Set the no-inspect mode */
	void SetNoInspect(bool NewState);

	/** Set the minimized mode */
	void SetMinimized(bool NewState);

	/** SHow the menu */
	void Show();

	/** Hide the menu */
	void Hide();

	/** A hotkey was received */
	void Hotkey(int32 Index);


	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/
	
	/** Update a list of capturing companies */
	bool UpdateCaptureList();

	/** Update the capabilities info */
	void UpdateCapabilitiesInfo();

	/** Add a message info */
	void AddMessage(FText Message, const FSlateBrush* Icon, UFlareCompany* CaptureCompany, float Progress);

	/** Can we target the current spacecraft ? */
	bool IsTargetDisabled() const;

	/** Can we capture the current spacecraft ? */
	bool IsCaptureDisabled() const;

	/** Can we target the current spacecraft ? Hint text */
	FText GetTargetButtonHint() const;

	/** Inspect the current spacecraft */
	void OnInspect();

	/** Target the current spacecraft */
	void OnTarget();

	/** Upgrade the current spacecraft */
	void OnUpgrade();

	/** Trade with the current spacecraft */
	void OnTrade();

	/** Fly the current spacecraft */
	void OnFly();

	/** Try to dock at the station */
	void OnDockAt();

	/** Undock */
	void OnUndock();

	/** Scrap this spacecraft */
	void OnScrap();

	/** Scrap this spacecraft */
	void OnScrapConfirmed();

	/** Capture this spacecraft */
	void OnCapture();
	

	/*----------------------------------------------------
		Content
	----------------------------------------------------*/

	/** Get the target name */
	FText GetName() const;

	/** Get the text color */
	FSlateColor GetTextColor() const;

	/** Get the target class name */
	FText GetDescription() const;

	/** Get the target combat value */
	FText GetCombatValue() const;

	/** Get the target icon */
	const FSlateBrush* GetIcon() const;

	/** Get the target class icon */
	const FSlateBrush* GetClassIcon() const;

	/** Hide the combat value for non-military */
	EVisibility GetCombatValueVisibility() const;

	/** Hide building icon if station not upgrading/building */
	EVisibility GetBuildVisibility() const;

	
	/** Hide the company flag if owned */
	EVisibility GetCompanyFlagVisibility() const;

	/** Visibility setting for the company/fleet/station data */
	EVisibility GetSpacecraftInfoVisibility() const;

	/** Visibility setting for the local trade data */
	EVisibility GetSpacecraftLocalInfoVisibility() const;

	/** Get the company name or the current fleet's name or the production status */
	FText GetSpacecraftInfo() const;

	/** Get the local ship status information if trading*/
	FText GetSpacecraftLocalInfo() const;

	/** Get the current fleet's name */
	FText GetSpacecraftInfoAdditional() const;

	/** Get the text color */
	FSlateColor GetAdditionalTextColor() const;
	

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Game data
	AFlarePlayerController*           PC;
	bool                              NoInspect;
	bool                              Minimized;

	// Target data	
	UFlareSimulatedSpacecraft*        TargetSpacecraft;
	FFlareSpacecraftDescription*      TargetSpacecraftDesc;
	FFlareObjectRemoved               OnRemoved;

	// Slate data (buttons)
	TSharedPtr<SVerticalBox>          MessageBox;
	TSharedPtr<SFlareButton>          InspectButton;
	TSharedPtr<SFlareButton>          TargetButton;
	TSharedPtr<SFlareButton>          UpgradeButton;
	TSharedPtr<SFlareButton>          TradeButton;
	TSharedPtr<SFlareButton>          FlyButton;
	TSharedPtr<SFlareButton>          DockButton;
	TSharedPtr<SFlareButton>          UndockButton;
	TSharedPtr<SFlareButton>          ScrapButton;
	TSharedPtr<SFlareButton>          OrderShipButton;
	TSharedPtr<SFlareButton>          OrderHeavyShipButton;

	// Slate data (various)
	TSharedPtr<STextBlock>            SpacecraftName;
	TSharedPtr<SWidget>               OwnerWidget;
	TSharedPtr<SFlareShipStatus>      ShipStatus;
	TSharedPtr<SFlareCompanyFlag>     CompanyFlag;
	TSharedPtr<SHorizontalBox>        CargoBay1;
	TSharedPtr<SHorizontalBox>        CargoBay2;

};
