#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareDropList.h"
#include "../Components/FlareList.h"
#include "../Components/FlareListItem.h"
#include "../../Game/FlareWorldHelper.h"
#include "../../Data/FlareResourceCatalogEntry.h"
#include "../FlareUITypes.h"


class UFlareSimulatedSector;
//class UFlarePeople;
class AFlareMenuManager;
struct FFlareResourceDescription;
class UFlareResourceCatalogEntry;

class SFlareWorldEconomyMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareWorldEconomyMenu){}

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
	void Enter(FFlareResourceDescription* Resource, UFlareSimulatedSector* Sector);

	/** Exit this menu */
	void Exit();

	void GenerateSectorList();
	void GenerateSectorPopList();


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Get the current sort icon to use */
	const FSlateBrush* GetSortIcon(EFlareEconomySort::Type Type) const;

	/** Get the current sort icon to use */
	const FSlateBrush* GetSortIconPop(EFlareEconomySort::Type Type) const;

	/** Set the current sort type to use */
	void ToggleSortType(EFlareEconomySort::Type Type);

	/** Set the current sort type to use */
	void ToggleSortTypePop(EFlareEconomySort::Type Type);


	FSlateColor GetPriceColor(UFlareSimulatedSector* Sector) const;

	/** Get the resource price info */
	FText GetResourceDescription() const;

	/** Get the company desc */
	FText GetCompanyDescription() const;

	/** Get the resource name */
	FText GetResourceName() const;

	/** Get the resource icon */
	const FSlateBrush* GetResourceIcon() const;

	/** Get the company icon */
	const FSlateBrush* GetCompanyEmblem() const;

	/** Get the resource usage & production info */
	FText GetResourceInfo() const;

	/** Get average/totals etc text for population submenu */
	FText GetPopulationInfosAverages() const;
	FText GetPopulationInfosTotals() const;

	/** Get the sector info */
	FText GetSectorText(UFlareSimulatedSector* Sector) const;

	/** Get the sector info color */
	FSlateColor GetSectorTextColor(UFlareSimulatedSector* Sector) const;

	/** Get the resource production info */
	FText GetResourceProductionInfo(UFlareSimulatedSector* Sector) const;

	/** Get the resource consumption info */
	FText GetResourceConsumptionInfo(UFlareSimulatedSector* Sector) const;

	/** Get the resource stock info */
	FText GetResourceStockInfo(UFlareSimulatedSector* Sector) const;

	/** Get the resource capacity info */
	FText GetResourceCapacityInfo(UFlareSimulatedSector* Sector) const;

	/** Get the resource price info */
	FText GetResourcePriceInfo(UFlareSimulatedSector* Sector) const;

	/** Get the population quantity info */
	FText GetPopTotalInfo(UFlareSimulatedSector* Sector) const;

	/** Get the population money info */
	FText GetPopMoneyInfo(UFlareSimulatedSector* Sector) const;

	/** Get the population wealth info */
	FText GetPopWealthInfo(UFlareSimulatedSector* Sector) const;

	/** Get the population happiness info */
	FText GetPopHappinessInfo(UFlareSimulatedSector* Sector) const;

	/** Get the resource price variation info */
	FText GetResourcePriceVariationInfo(UFlareSimulatedSector* Sector, TSharedPtr<int32> MeanDuration) const;

	TSharedRef<SWidget> OnGenerateResourceComboLine(UFlareResourceCatalogEntry* Item);
	void OnResourceComboLineSelectionChanged(UFlareResourceCatalogEntry* Item, ESelectInfo::Type SelectInfo, bool Station=false);
	FText OnGetCurrentResourceComboLine() const;

	TSharedRef<SWidget> OnGenerateCompanyComboLine(UFlareCompany* Item);
	void OnCompanyComboLineSelectionChanged(UFlareCompany* Item, ESelectInfo::Type SelectInfo);
	FText OnGetCurrentCompanyComboLine() const;

	void OnOpenSector(UFlareSimulatedSector* Sector);

	void OnOpenSectorPrices(UFlareSimulatedSector* Sector);

	void OnIncludeTradingHubsToggle();

	/** Generate Station List */
	void GenerateStationList();
	
	/** Refresh Station List */
	void RefreshStationList();
	bool PassesFilterList(UFlareSimulatedSpacecraft* StationCandidate);

	void RefreshAddRemoveSelectedCompanyButton();
	void AddRemoveCompanyFilter();

	/** Update filters */
	void OnToggleShowFlags();

	/** Change station/ship button */
	void OnStationShip();

	/** Change input/output button */
	void OnInputOutput();

	/** Hide the station filters */
	EVisibility GetStationFiltersStationModeVisibility() const;
	
	/** Hide the station filters */
	EVisibility GetStationFiltersShipModeVisibility() const;

	/** Hide the station filters */
	EVisibility GetStationFiltersResourceInputOutputVisibility() const;

	/** Hide the station filters */
	EVisibility GetStationFiltersVisibility() const;

	/** Hide the station filters */
	EVisibility GetStationFiltersResourceVisibility() const;

	/** Hide the station filters */
	EVisibility GetStationFiltersCompanyVisibility() const;

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Menu data
	TSharedPtr<class SFlareTabView>          TabView;

	// Target data
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;
	FFlareResourceDescription*                      TargetResource;
	UFlareCompany*									TargetCompany;
	TArray<UFlareCompany*>						    TargetCompanies;
	TSharedPtr<STextBlock>						    SelectedCompaniesText;

	TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> WorldStats;

	// Slate data
	TSharedPtr<SVerticalBox>                        SectorList;
	TSharedPtr<SVerticalBox>                        SectorPopList;
	TSharedPtr<SFlareDropList<UFlareResourceCatalogEntry*>> ResourceSelector;
	TSharedPtr<SFlareButton>                        IncludeTravelersButton;
	TSharedPtr<SFlareButton>                        IncludeTradingHubsButton;

	// Data
	bool                                            IsCurrentSortDescending;
	bool                                            IsCurrentPopSortDescending;
	TEnumAsByte<EFlareEconomySort::Type>            CurrentSortType;
	TEnumAsByte<EFlareEconomySort::Type>            CurrentSortTypePop;
	

	TSharedPtr<SFlareDropList<UFlareResourceCatalogEntry*>> StationResourceSelector;
	TSharedPtr<SFlareDropList<UFlareCompany*>>              StationCompanySelector;
	TArray<UFlareCompany*>						            CompanyList;

	TArray<UFlareSimulatedSpacecraft*>						  ShipsArray;
	TArray<UFlareSimulatedSpacecraft*>						  ShipsTravelArray;
	TArray<UFlareSimulatedSpacecraft*>						  StationsArray;
	TSharedPtr<SFlareList>									  StationList;

	TSharedPtr<SFlareButton>                                  FiltersButton;
	TSharedPtr<SFlareButton>                                  ResourceFiltersButton;
	TSharedPtr<SFlareButton>                                  CompanyFiltersButton;
	TSharedPtr<SFlareButton>                                  StorageHubButton;
	TSharedPtr<SFlareButton>                                  ShipyardsFilterButton;
	TSharedPtr<SFlareButton>                                  UnderConstructionFilterButton;
	TSharedPtr<SFlareButton>                                  AddRemoveSelectedCompany;
	TSharedPtr<SFlareButton>                                  QuantityButton;
	TSharedPtr<SFlareButton>                                  DistanceButton;
	TSharedPtr<SFlareButton>                                  InputOutputButton;
	TSharedPtr<SFlareButton>                                  StationShipsButton;
	bool												      InputOutputMode;
	bool												      StationShipMode;

	uint32													  KnownPopulationTotal;
	int64													  KnownPopulationMoney;
	float													  KnownPopulationWealth;
	float													  KnownPopulationHappiness;
};