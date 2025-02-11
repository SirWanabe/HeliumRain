#pragma once

#include "../../Flare.h"
#include "../Components/FlareListItem.h"
#include "../Components/FlareSpacecraftInfo.h"
#include "../Components/FlareFleetInfo.h"

DECLARE_DELEGATE_OneParam(FFlareListItemSelected, TSharedPtr<FInterfaceContainer>)

class SFlareList : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareList)
	: _UseCompactDisplay(false)
	, _UseSmallFont(false)
   	, _StationList(false)
	, _FleetList(false)
	, _WidthAdjuster(0.f)
	, _ShowOwnedShips(false)
	, _DisableFilters(false)
	, _ArrayMode(ESelectionMode::Single)
	{}			 

	SLATE_ARGUMENT(bool, UseCompactDisplay)
	SLATE_ARGUMENT(bool, UseSmallFont)
	SLATE_ARGUMENT(bool, StationList)
	SLATE_ARGUMENT(bool, FleetList)
	SLATE_ARGUMENT(float, WidthAdjuster)
	SLATE_ARGUMENT(bool, ShowOwnedShips)
	SLATE_ARGUMENT(bool, DisableFilters)
	SLATE_ARGUMENT(TEnumAsByte<ESelectionMode::Type>, ArrayMode)

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)
	SLATE_EVENT(FFlareListItemSelected, OnItemSelected)
	SLATE_EVENT(FFlareListItemSelected, OnItemUnSelected)
	SLATE_ARGUMENT(FText, Title)
	SLATE_ARGUMENT(FString, OriginatingMenu)

	SLATE_END_ARGS()

public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Add a new fleet to the list */
	void AddFleet(UFlareFleet* Fleet);

	/** Select a fleet in the list */
	void SelectFleet(UFlareFleet* Fleet);

	/** Add a new ship to the list */
	void AddShip(UFlareSimulatedSpacecraft* Ship);

	/** Update the list display from content */
	void RefreshList(bool DisableSort = false);

	/** Updates button names/display if fleet*/
	void SetButtonNamesFleet();

	/** Clear the current selection */
	void ClearSelection();

	/** Change title */
	void SetTitle(FText NewTitle);

	/** Set the compact mode status */
	void SetUseCompactDisplay(bool Status);

	/** Remove all entries from the list */
	void Reset();

	void UnSelectedObjects();

	/** Get the number of items */
	int32 GetItemCount() const
	{
		return ObjectList.Num();
	}

	/** Get the object list */
	TArray< TSharedPtr<FInterfaceContainer> > GetObjectList() const
	{
		return ObjectList;
	}

	void RefreshPreviousWidget();

protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Show a "no objects" text when the data is empty */
	EVisibility GetNoObjectsVisibility() const;

	/** Show ship filters when ships are present */
	EVisibility GetShipFiltersVisibility() const;

	EVisibility GetFreighterFiltersVisibility() const;

	/** Show ship filters when ships are present */
	EVisibility GetStationFilterVisibility() const;

	/** Show fleet filters when ships are present and not in fleet menu */
	EVisibility GetFleetFilterVisibility() const;

	/** Show fleet filters when fleets are present */
	EVisibility GetFleetFiltersVisibility() const;
	
	/** Target item generator */
	TSharedRef<ITableRow> GenerateTargetInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** Target item selected */
	void OnTargetSelected(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo);

	void UnselectPreviousWidget();

	/** Update filters */
	void OnToggleShowFlags();

	/** Ship removed */
	void OnShipRemoved(UFlareSimulatedSpacecraft* Ship);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>                      MenuManager;

	// Menu components
	TSharedPtr<STextBlock>                                       Title;
	FString														 OriginatingMenu;

	TSharedPtr< SListView< TSharedPtr<FInterfaceContainer> > >   WidgetList;
	TArray< TSharedPtr<FInterfaceContainer> >                    ObjectList;
	TArray< TSharedPtr<FInterfaceContainer> >                    FilteredObjectList;

	TSharedPtr<FInterfaceContainer>                              SelectedObject;

	TArray< TSharedPtr<SFlareListItem> >						 SelectedWidgets;
	TSharedPtr<SFlareListItem>                                   PreviousWidget;

	// Filters
	TSharedPtr<SFlareButton>                                     ShowStationsButton;
	TSharedPtr<SFlareButton>                                     ShowMilitaryButton;
	TSharedPtr<SFlareButton>                                     ShowFreightersButton;
	TSharedPtr<SFlareButton>                                     GroupFleetsButton;

	// State data
	FFlareListItemSelected                                       OnItemUnSelected;
	FFlareListItemSelected                                       OnItemSelected;
	bool                                                         UseCompactDisplay;
	bool														 UseSmallFont;
	bool                                                         HasShips;
	bool                                                         HasFleets;
	bool														 StationList;
	bool														 FleetList;
	bool														 LastDisableSort;
	bool														 ShowOwnedShips;
	bool														 DisableFilters;
	bool														 UseExpandedDisplay;
	float														 WidthAdjuster;
	TEnumAsByte<ESelectionMode::Type>							 ArraySelectionMode;
};