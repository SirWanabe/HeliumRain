#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareList.h"

class UFlareSimulatedSpacecraft;


class SFlareFleetMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareFleetMenu){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Setup the widget */
	void Setup();

	/** Enter this menu */
	void Enter(UFlareFleet* TargetFleet);
	
	/** Exit this menu */
	void Exit();

	/** Update the fleet list */
	void UpdateFleetList(UFlareFleet* SelectedFleet = nullptr);

	/** Update the ship list */
	void UpdateShipList(UFlareFleet* Fleet);

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

protected:

	/*----------------------------------------------------
		Content callbacks
	----------------------------------------------------*/

	/** Can we see edit buttons */
	EVisibility GetEditVisibility() const;

	/** Is the "add to fleet" button disabled */
	bool IsAddDisabled() const;
	
	/** Get hint text about merging */
	FText GetAddHintText() const;

	/** Is the "remove from fleet" button disabled */
	bool IsRemoveDisabled() const;

	/** Get hint text about removing a ship from the fleet */
	FText GetRemoveHintText() const;

	/** Is the "rename fleet" button disabled */
	bool IsRenameDisabled() const;

	/** Get hint text about renaming a fleet */
	FText GetRenameHintText() const;

	/** Get hue value */
	float GetColorSpinBoxValue() const;

	/** Get the trade route hint text */
	FText GetInspectTradeRouteHintText() const;

	/** Can we inspect the trade route */
	bool IsInspectTradeRouteDisabled() const;

	/** Get the auto-trade hint text */
	FText GetAutoTradeHintText() const;

	/** Can we toggle auto trade */
	bool IsAutoTradeDisabled() const;

	/** Can we toggle hide travel */
	bool IsToggleHideTravelDisabled() const;

	/** Get the hide travel hint text */
	FText GetToggleHideTravelHintText() const;

	

	/*----------------------------------------------------
		Action callbacks
	----------------------------------------------------*/
	
	/** A spacecraft has been selected*/
	void OnSpacecraftSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer);
	/** A spacecraft has been unselected*/
	void OnSpacecraftUnSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer);

	/** A fleet has been selected */
	void OnFleetSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer);
	/** A fleet has been unselected */
	void OnFleetUnSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer);

	/** Editing done */
	void OnEditFinished();

	/** Add the selected ship to the selected fleet */
	void OnAddToFleet();

	/** Remove the selected ship from the selected fleet */
	void OnRemoveFromFleet();

	/** Rename the selected fleet */
	void OnRenameFleet();

	/** Hue value changed */
	void OnColorSpinBoxValueChanged(float NewValue);

	/** Filters toggled on/off */
	void OnToggleShowFlags();

	/** Open the trade route menu */
	void OnOpenTradeRoute();

	/** Toggle auto-trading */
	void OnToggleAutoTrade();

	/** Toggle Hide Travel*/
	void OnToggleHideTravel();

	void OnSelectWhiteList();
	void OnRemoveWhiteList();
	bool IsWhiteListSelectDisabled() const;
	bool IsWhiteListRemoveDisabled() const;
	TSharedRef<SWidget> OnGenerateWhiteListComboLine(UFlareCompanyWhiteList* Item);
	void OnWhiteListComboLineSelectionChanged(UFlareCompanyWhiteList* Item, ESelectInfo::Type SelectInfo);

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;
	
	// Game data
	UFlareFleet*                                    FleetToEdit;
	UFlareFleet*                                    FleetToAdd;
	UFlareSimulatedSpacecraft*                      ShipToRemove;

	// Menu components
	TSharedPtr<SFlareList>                          ShipList;
	TSharedPtr<SFlareList>                          FleetList;
	TSharedPtr<SEditableText>                       EditFleetName;

	TSharedPtr<SFlareButton>						RemoveShipButton;
	TSharedPtr<SFlareButton>						TradeRouteButton;
	TSharedPtr<SFlareButton>				        AutoTradeButton;
	TSharedPtr<SFlareButton>						HideTravelButton;
	TSharedPtr<SFlareButton>					    RefillButton;
	TSharedPtr<SFlareButton>					    RepairButton;

	TSharedPtr<SBox>									WhiteListSelectionBox;
	TSharedPtr<SFlareDropList<UFlareCompanyWhiteList*>> WhiteListDropBox;
	TArray<UFlareCompanyWhiteList*>						WhiteListOptions;
	UFlareCompanyWhiteList*								CurrentlySelectedWhiteList;

public:
	UFlareFleet* GetFleetEditing() const
	{
		return FleetToEdit;
	}
};
