#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareDropList.h"
#include "../../Game/FlareTradeRoute.h"
#include "../../Game/FlareSimulatedSector.h"
#include "../../Game/FlareFleet.h"
#include "../../Data/FlareResourceCatalogEntry.h"
#include "../FlareUITypes.h"

class UFlareTradeRoute;
struct FFlareTradeRouteSectorOperationSave;
typedef TPair<FFlareResourceDescription*, int64> TFlareResourceDeal;

class SFlareTradeRouteMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareTradeRouteMenu) {}

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
	void Enter(UFlareTradeRoute* TradeRoute);

	/** Exit this menu */
	void Exit();

//	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	void GenerateSectorList();

	void GenerateFleetList();

protected:

	/*----------------------------------------------------
		Content callbacks
	----------------------------------------------------*/
	
	/** Get route name */
	FText GetTradeRouteName() const;

	/** Get info for the current fleet */
	FText GetFleetInfo() const;
	
	/** Get info for the selected trade route step */
	FText GetSelectedStepInfo() const;

	/** Get info for the selected sector */
	FText GetSelectedSectorInfo() const;

	/** Get info for the next trade route step */
	FText GetNextStepInfo() const;

	/** Get the icon for the pause button */
	const FSlateBrush* GetPauseIcon() const;

	/** Get a description of a trade operation */
	FText GetOperationInfo(FFlareTradeRouteSectorOperationSave* Operation) const;
	
	void OnSelectSector(UFlareSimulatedSector* ChosenSector);
	void ConfirmChangeSector(UFlareSimulatedSector* ChosenSector);

	// Sector list
	TSharedRef<SWidget> OnGenerateSectorComboLine(UFlareSimulatedSector* TargetSector);
	TArray<TFlareResourceDeal> GetSellableResources(UFlareSimulatedSector* TargetSector) const;
	TArray<TFlareResourceDeal> GetBuyableResources(UFlareSimulatedSector* TargetSector) const;
	void AddResourceDeals(TSharedPtr<SHorizontalBox> ResourcesBox, TArray<TFlareResourceDeal> Resources);
	
	FText OnGetCurrentSectorComboLine() const;
	
	bool IsOperationDownDisabled() const;
	bool IsOperationUpDisabled() const;

	/** Can add sector */
	bool IsAddSectorDisabled() const;

	/** Can rename trade oute */
	bool IsRenameDisabled() const;

	TSharedRef<SWidget> OnGenerateResourceComboLine(UFlareResourceCatalogEntry* Item);

	void OnResourceComboLineSelectionChanged(UFlareResourceCatalogEntry* Item, ESelectInfo::Type SelectInfo);

	FText OnGetCurrentResourceComboLine() const;

	TSharedRef<SWidget> OnGenerateOperationComboLine(TSharedPtr<FText> Item);

	void UpdateSelectableConditionsArrays(bool RefreshOptions = true);
	FText OnGetCurrentOperationConditionsComboLine() const;
	FText OnGetCurrentOperationComboLine() const;

	void OnOperationConditionsComboLineSelectionChanged(TSharedPtr<FText> Item, ESelectInfo::Type SelectInfo);
	void OnOperationComboLineSelectionChanged(TSharedPtr<FText> Item, ESelectInfo::Type SelectInfo);

	/** Can edit operation */
	EVisibility GetOperationDetailsVisibility() const;

	/** Can edit trade route */
	EVisibility GetMainVisibility() const;

	EVisibility GetVisibilityOperationConditions() const;
	EVisibility GetVisibilityConditionToggled(TSharedPtr<SFlareButton> ToggleConditionButton) const;

	/** Should options specific to trade load/unload show?*/
	EVisibility GetVisibilityTradeOperation() const;
	EVisibility GetVisibilityTradeSubOperation() const;

	/** Add button */
	FText GetAddSectorText() const;

	FText GetOperationTitleText(FFlareTradeRouteSectorOperationSave* Operation, int IndexValue) const;

	/** Get status */
	FText GetOperationStatusText(FFlareTradeRouteSectorOperationSave* Operation, FName SectorName) const;

	/** Highlight color */
	FSlateColor GetOperationHighlight(FFlareTradeRouteSectorOperationSave* Operation) const;

	/** Get the load text's button */
	FText GetLoadText() const;

	/** Get the unload text's button */
	FText GetUnloadText() const;
	
	/** Fleet info line */
	TSharedRef<SWidget> OnGenerateFleetComboLine(UFlareFleet* Item);

	FText OnGetCurrentFleetComboLine() const;
	
	/** Can assign a fleet*/
	EVisibility GetAssignFleetVisibility() const;
	
	bool IsEditOperationDisabled(FFlareTradeRouteSectorOperationSave* Operation) const;

	EVisibility GetSelectButtonVisibility(FFlareTradeRouteSectorOperationSave* Operation) const;

	EVisibility GetTradeHubsButtonVisibility() const;
	EVisibility GetDonationButtonVisibility() const;

	EVisibility GetQuantityLimitVisibility() const;

	EVisibility GetInventoryLimitVisibility() const;

	EVisibility GetWaitOptionsVisibility() const;
	EVisibility GetWaitLimitVisibility() const;

	FText GetConditionPercentageValue(FFlareTradeRouteOperationConditionSave* Condition) const;

	/*----------------------------------------------------
		Actions callbacks
	----------------------------------------------------*/

	/** Reset statistics */
	void OnResetStatistics();

	/** Confirm a new name for this trade route */
	void OnConfirmChangeRouteNameClicked();

	void OnSectorComboLineSelectionChanged(UFlareSimulatedSector* Item, ESelectInfo::Type SelectInfo);

	/** Sector added */
	void OnAddSectorClicked();

	/** Move trade route */
	void OnMoveLeft(UFlareSimulatedSector* Sector);
	void OnMoveRight(UFlareSimulatedSector* Sector);
	bool IsMoveLeftDisabled(UFlareSimulatedSector* Sector) const;
	bool IsMoveRightDisabled(UFlareSimulatedSector* Sector) const;

	/** Sector removed */
	void OnRemoveSectorClicked(UFlareSimulatedSector* Sector);

	/** Edit & Delete */
	void UpdateSelectedOperation();
	void OnEditOperationClicked(FFlareTradeRouteSectorOperationSave* Operation, UFlareSimulatedSector* Sector, int SectorIndex, int OperationIndex);
	void OnDeleteOperationClicked(FFlareTradeRouteSectorOperationSave* Operation);
	void OnOperationAltered();

	/** Select route*/
	void OnSelectOperationClicked(FFlareTradeRouteSectorOperationSave* Operation, UFlareSimulatedSector* Sector, int SectorIndex, int OperationIndex);

	/** Done editing current operation */
	void OnDoneClicked();

	/** Skip current operation */
	void OnSkipOperationClicked();

	/** Pause current operation */
	void OnPauseTradeRouteClicked();

	/** Step controls */
	void OnConditionPercentageChanged(float Value, FFlareTradeRouteOperationConditionSave* Condition);

	FSlateColor GetConditionOneBoolColor(FFlareTradeRouteOperationConditionSave* Condition) const;
	FSlateColor GetConditionThreeBoolColor(FFlareTradeRouteOperationConditionSave* Condition) const;
	FText GetConditionBoolTwoName(FFlareTradeRouteOperationConditionSave* Condition) const;

	void UpdateConditionsToggleName();
	void OnConditionsToggle();
	void OnConditionsAdd();
	void OnConditionView(FFlareTradeRouteOperationConditionSave* Condition);
	void OnConditionDelete(FFlareTradeRouteOperationConditionSave* Condition);
	void RefreshConditionsMenus();

	void OnConditionAlterSkipFail(FFlareTradeRouteOperationConditionSave* Condition);
	void OnConditionAlterBoolOne(FFlareTradeRouteOperationConditionSave* Condition);
	void OnConditionAlterBoolTwo(FFlareTradeRouteOperationConditionSave* Condition);
	void OnConditionAlterBoolThree(FFlareTradeRouteOperationConditionSave* Condition);

	void OnOperationDonationToggle();
	void OnOperationTradeWithHubsToggle();
	void OnOperationUpClicked();
	void OnOperationDownClicked();
	void OnQuantityLimitToggle();
	void OnInventoryLimitToggle();
	void OnWaitLimitToggle();
	void OnTradeSelfChanged(float Value);
	void OnTradeOthersChanged(float Value);
	void OnQuantityLimitChanged(float Value);
	void OnInventoryLimitChanged(float Value);
	void OnQuantityLimitEntered(const FText& TextValue);
	void OnInventoryLimitEntered(const FText& TextValue);
	void OnWaitLimitChanged(float Value);

	/** Load the current resource */
	void OnAddOperationClicked(UFlareSimulatedSector* Sector);
	
	/** Fleet selection */
	void OnFleetComboLineSelectionChanged(UFlareFleet* Item, ESelectInfo::Type SelectInfo);
	void OnAssignFleetClicked();
	void OnUnassignFleetClicked(UFlareFleet* Fleet);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>            MenuManager;

	// Menu data
	int32                                              MaxSectorsInRoute;
	UFlareTradeRoute*                                  TargetTradeRoute;
	FFlareTradeRouteSectorOperationSave*			   SelectedOperation;
	FFlareTradeRouteSectorOperationSave*               EditSelectedOperation;
	
	//Sector Index and Operation index for EditSelectedOperation
	int32											   EditSelectedOperationSI;
	int32											   EditSelectedOperationOI;

	UFlareSimulatedSector*                             SelectedSector;

	// Sector data
	TSharedPtr<SFlareDropList<UFlareSimulatedSector*>> SectorSelector;
	TArray<UFlareSimulatedSector*>                     SectorList;

	// Fleet list
	TSharedPtr<SFlareDropList<UFlareFleet*>>           FleetSelector;
	TArray<UFlareFleet*>                               FleetList;

	// Items
	TSharedPtr<SFlareDropList<UFlareResourceCatalogEntry*>> ResourceSelector;
	TSharedPtr<SFlareDropList<TSharedPtr<FText> >>     OperationSelector;
	TArray<TSharedPtr<FText>>                          OperationNameList;
	TArray<EFlareTradeRouteOperation::Type>            OperationList;

	TSharedPtr<SFlareButton>                           AddConditionButton;
	TSharedPtr<SFlareDropList<TSharedPtr<FText> >>     OperationConditionsSelector;
	TArray<TSharedPtr<FText>>                          OperationConditionsNameList;
	TArray<EFlareTradeRouteOperationConditions::Type>  OperationConditionsList;

	TArray<TSharedPtr<FText>>                          OperationConditionsNameListDefault;
	TArray<EFlareTradeRouteOperationConditions::Type>  OperationConditionsListDefault;

	TSharedPtr<SEditableText>                          EditRouteName;
	TSharedPtr<SHorizontalBox>                         TradeSectorList;
	TSharedPtr<SVerticalBox>                           TradeFleetList;
	TSharedPtr<SVerticalBox>                           ConditionsList;

	TSharedPtr<SSlider>                                TradeSelfSlider;
	TSharedPtr<STextBlock>				               TradeSelfLimitText;

	TSharedPtr<SSlider>                                TradeOthersSlider;
	TSharedPtr<STextBlock>				               TradeOthersLimitText;

	TSharedPtr<SFlareButton>                           ToggleConditionsButton;

	TSharedPtr<SFlareButton>                           DonateButton;
	TSharedPtr<SFlareButton>                           TradeWithHubsButton;
	TSharedPtr<SFlareButton>                           QuantityLimitButton;
	TSharedPtr<SFlareButton>                           InventoryLimitButton;
	TSharedPtr<SFlareButton>                           WaitLimitButton;
	TSharedPtr<SSlider>                                QuantityLimitSlider;
	TSharedPtr<SSlider>                                InventoryLimitSlider;
	TSharedPtr<SSlider>                                WaitLimitSlider;
	TSharedPtr<SEditableText>                          QuantityLimitText;
	TSharedPtr<SEditableText>                          InventoryLimitText;

	// Deals
	TSharedPtr<SHorizontalBox>                         EditSuggestedPurchasesBox;
	TSharedPtr<SHorizontalBox>                         EditSuggestedSalesBox;
	TArray<TFlareResourceDeal>                         CurrentlyBoughtResources;
	TArray<TFlareResourceDeal>                         CurrentlySoldResources;
};
