
#include "FlareWorldEconomyMenu.h"
#include "../../Flare.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareGameTools.h"
#include "../../Game/FlareSectorHelper.h"
#include "../../Economy/FlareResource.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Data/FlareResourceCatalog.h"
#include "../Components/FlareTabView.h"

#define LOCTEXT_NAMESPACE "FlareWorldEconomyMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareWorldEconomyMenu::Construct(const FArguments& InArgs)
{
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SAssignNew(TabView, SFlareTabView)

		// Resources Tab
		+ SFlareTabView::Slot()
		.Header(LOCTEXT("EconomyMainTab", "Resources"))
		.HeaderHelp(LOCTEXT("EcononyMainTabHelp", "Resource economy information"))
		[
		SNew(SVerticalBox)
		// Selector and info
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(ECONOMY_TABLE_WIDTH_FULL * Theme.ContentWidth)
			.Padding(FMargin(0))
			.HAlign(HAlign_Fill)
			[
				SNew(SHorizontalBox)

				// Main resource info
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Left)
				[
					SNew(SVerticalBox)

					// Resource name
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.TitlePadding)
					[
						SNew(STextBlock)
						.Text(this, &SFlareWorldEconomyMenu::GetResourceName)
						.TextStyle(&Theme.SubTitleFont)
					]
		
					// Resource picker
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Icon
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Right)
						.Padding(Theme.ContentPadding)
						.AutoWidth()
						[
							SNew(SImage)
							.Image(this, &SFlareWorldEconomyMenu::GetResourceIcon)
						]

						// Info
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Left)
						.Padding(Theme.ContentPadding)
						[
							SNew(SVerticalBox)

							// Resource name
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SBox)
								.WidthOverride(Theme.ContentWidth / 2)
								.Padding(FMargin(0))
								.HAlign(HAlign_Left)
								[
									SAssignNew(ResourceSelector, SFlareDropList<UFlareResourceCatalogEntry*>)
									.OptionsSource(&MenuManager->GetPC()->GetGame()->GetResourceCatalog()->Resources)
									.OnGenerateWidget(this, &SFlareWorldEconomyMenu::OnGenerateResourceComboLine)
									.OnSelectionChanged(this, &SFlareWorldEconomyMenu::OnResourceComboLineSelectionChanged, false)
									.HeaderWidth(5)
									.ItemWidth(5)
									[
										SNew(SBox)
										.Padding(Theme.ListContentPadding)
										[
											SNew(STextBlock)
											.Text(this, &SFlareWorldEconomyMenu::OnGetCurrentResourceComboLine)
											.TextStyle(&Theme.TextFont)
										]
									]
								]
							]

							// Resource description
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.SmallContentPadding)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.TextFont)
								.Text(this, &SFlareWorldEconomyMenu::GetResourceDescription)
								.WrapTextAt(Theme.ContentWidth)
							]
						]
					]
				]

				// Resource info
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Right)
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(Theme.ContentWidth / 2)
					.Padding(FMargin(0))
					[
						SNew(SVerticalBox)

						// Resource info
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(this, &SFlareWorldEconomyMenu::GetResourceInfo)
							.WrapTextAt(Theme.ContentWidth / 2)
						]

						// Include hubs
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						.HAlign(HAlign_Left)
						[
							SAssignNew(IncludeTradingHubsButton, SFlareButton)
							.Text(LOCTEXT("IncludeHubs", "Include Storage Hubs"))
							.Toggle(true)
							.Width(6)
							.OnClicked(this, &SFlareWorldEconomyMenu::OnIncludeTradingHubsToggle)
						]
					]
				]
			]
		]

		// Content
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			.Padding(Theme.ContentPadding)
			[
				SNew(SBox)
				.WidthOverride(ECONOMY_TABLE_WIDTH_FULL * Theme.ContentWidth)
				.Padding(FMargin(0))
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Sector name
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_LARGE * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("SectorNameColumnTitle", "Sector"))
								.Width(ECONOMY_TABLE_BUTTON_LARGE)
								.Transparent(true)
								.Icon(this, &SFlareWorldEconomyMenu::GetSortIcon, EFlareEconomySort::ES_Sector)
								.OnClicked(this, &SFlareWorldEconomyMenu::ToggleSortType, EFlareEconomySort::ES_Sector)
							]
						]

						// Production
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("ResourceProductionColumnTitleInfo", "Production"))
								.Width(ECONOMY_TABLE_BUTTON_SMALL)
								.Transparent(true)
								.Icon(this, &SFlareWorldEconomyMenu::GetSortIcon, EFlareEconomySort::ES_Production)
								.OnClicked(this, &SFlareWorldEconomyMenu::ToggleSortType, EFlareEconomySort::ES_Production)
							]
						]

						// Consumption
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("ResourceConsumptionColumnTitleInfo", "Usage"))
								.Width(ECONOMY_TABLE_BUTTON_SMALL)
								.Transparent(true)
								.Icon(this, &SFlareWorldEconomyMenu::GetSortIcon, EFlareEconomySort::ES_Consumption)
								.OnClicked(this, &SFlareWorldEconomyMenu::ToggleSortType, EFlareEconomySort::ES_Consumption)
							]
						]

						// Stock
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("ResourceStockColumnTitleInfo", "Stock"))
								.Width(ECONOMY_TABLE_BUTTON_SMALL)
								.Transparent(true)
								.Icon(this, &SFlareWorldEconomyMenu::GetSortIcon, EFlareEconomySort::ES_Stock)
								.OnClicked(this, &SFlareWorldEconomyMenu::ToggleSortType, EFlareEconomySort::ES_Stock)
							]
						]

						// Capacity
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("ResourceCapacityColumnTitleInfo", "Needs"))
								.Width(ECONOMY_TABLE_BUTTON_SMALL)
								.Transparent(true)
								.Icon(this, &SFlareWorldEconomyMenu::GetSortIcon, EFlareEconomySort::ES_Needs)
								.OnClicked(this, &SFlareWorldEconomyMenu::ToggleSortType, EFlareEconomySort::ES_Needs)
							]
						]

						// Price
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("ResourcePriceColumnTitleInfo", "Price"))
								.Width(ECONOMY_TABLE_BUTTON_SMALL)
								.Transparent(true)
								.Icon(this, &SFlareWorldEconomyMenu::GetSortIcon, EFlareEconomySort::ES_Price)
								.OnClicked(this, &SFlareWorldEconomyMenu::ToggleSortType, EFlareEconomySort::ES_Price)
							]
						]

						// Price variation
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("ResourcePriceVariationAveragedColumnTitleInfo", "Variation"))
								.Width(ECONOMY_TABLE_BUTTON_SMALL)
								.Transparent(true)
								.Icon(this, &SFlareWorldEconomyMenu::GetSortIcon, EFlareEconomySort::ES_Variation)
								.OnClicked(this, &SFlareWorldEconomyMenu::ToggleSortType, EFlareEconomySort::ES_Variation)
							]
						]
					]

					// List
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(SectorList, SVerticalBox)
					]
				]
			]
		]
	]
	// Stations / Ships slot
	+ SFlareTabView::Slot()
	.Header(LOCTEXT("StationTab", "Stations / Ships"))
	.HeaderHelp(LOCTEXT("StationTabHelp", "Station and ship economy information"))
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
// List Box
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(Theme.SmallContentPadding)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SAssignNew(StationShipsButton, SFlareButton)
					.Text(LOCTEXT("StationsUpper", "STATIONS"))
					.HelpText(LOCTEXT("StationsHelp", "Filtering by stations"))
					.OnClicked(this, &SFlareWorldEconomyMenu::OnStationShip)
					.Width(5)
				]

				+ SVerticalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SAssignNew(FiltersButton, SFlareButton)
					.Text(LOCTEXT("Filter", "Filters"))
					.HelpText(LOCTEXT("FilterHelp", "Enable/Disable filters"))
				.OnClicked(this, &SFlareWorldEconomyMenu::OnToggleShowFlags)
				.Toggle(true)
				.Width(5)
				]

				+ SVerticalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SAssignNew(ResourceFiltersButton, SFlareButton)
					.Text(LOCTEXT("FilterResource", "Filter Resources"))
					.HelpText(LOCTEXT("FilterResourceHelp", "Enable/Disable Specific resource filters"))
					.OnClicked(this, &SFlareWorldEconomyMenu::OnToggleShowFlags)
					.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersVisibility)
					.Toggle(true)
					.Width(5)
				]

				+ SVerticalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SAssignNew(InputOutputButton, SFlareButton)
					.Text(LOCTEXT("Output", "OUTPUT"))
					.HelpText(LOCTEXT("OutputHelp", "Filter output"))
					.OnClicked(this, &SFlareWorldEconomyMenu::OnInputOutput)
					.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersResourceInputOutputVisibility)
					.Width(5)
				]

				// Resource name
				+SVerticalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SNew(SHorizontalBox)
					// Icon
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Top)
					.HAlign(HAlign_Right)
					.Padding(Theme.ContentPadding)
					.AutoWidth()
					[
						SNew(SImage)
						.Image(this, &SFlareWorldEconomyMenu::GetResourceIcon)
						.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersResourceVisibility)
					]

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Top)
					.HAlign(HAlign_Left)
					.Padding(Theme.ContentPadding)
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(Theme.ContentWidth / 2)
						.Padding(FMargin(0))
						.HAlign(HAlign_Left)
						[
							SAssignNew(StationResourceSelector, SFlareDropList<UFlareResourceCatalogEntry*>)
							.OptionsSource(&MenuManager->GetPC()->GetGame()->GetResourceCatalog()->Resources)
							.OnGenerateWidget(this, &SFlareWorldEconomyMenu::OnGenerateResourceComboLine)
							.OnSelectionChanged(this, &SFlareWorldEconomyMenu::OnResourceComboLineSelectionChanged, true)
							.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersResourceVisibility)
							.HeaderWidth(5)
							.ItemWidth(5)
							[
								SNew(SBox)
								.Padding(Theme.ListContentPadding)
								[
									SNew(STextBlock)
									.Text(this, &SFlareWorldEconomyMenu::OnGetCurrentResourceComboLine)
									.TextStyle(&Theme.TextFont)
									.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersResourceVisibility)
								]
							]
						]
					]
				]

				// Resource description
				+ SVerticalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareWorldEconomyMenu::GetResourceDescription)
					.WrapTextAt(Theme.ContentWidth * 0.60)
					.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersResourceVisibility)
				]

				+ SVerticalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SAssignNew(CompanyFiltersButton, SFlareButton)
					.Text(LOCTEXT("FilterCompany", "Filter Company"))
					.HelpText(LOCTEXT("FilterCompanyHelp", "Enable/Disable Company filter"))
					.OnClicked(this, &SFlareWorldEconomyMenu::OnToggleShowFlags)
					.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersVisibility)
					.Toggle(true)
					.Width(5)
				]

				+ SVerticalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SNew(SHorizontalBox)
					// Icon
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Top)
					.HAlign(HAlign_Right)
					.Padding(Theme.ContentPadding)
					.AutoWidth()
					[
						SNew(SImage)
						.Image(this, &SFlareWorldEconomyMenu::GetCompanyEmblem)
						.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersCompanyVisibility)
					]

					+ SHorizontalBox::Slot()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Right)
						.Padding(Theme.ContentPadding)
						.AutoWidth()
						[
							SNew(SBox)
							.WidthOverride(Theme.ContentWidth / 2)
							.Padding(FMargin(0))
							.HAlign(HAlign_Left)
							[
									SAssignNew(StationCompanySelector, SFlareDropList<UFlareCompany*>)
									.OptionsSource(&CompanyList)
									.OnGenerateWidget(this, &SFlareWorldEconomyMenu::OnGenerateCompanyComboLine)
									.OnSelectionChanged(this, &SFlareWorldEconomyMenu::OnCompanyComboLineSelectionChanged)
									.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersCompanyVisibility)
									.HeaderWidth(6)
									.ItemWidth(6)
								[
									SNew(SBox)
									.Padding(Theme.ListContentPadding)
									[
										SNew(STextBlock)
										.Text(this, &SFlareWorldEconomyMenu::OnGetCurrentResourceComboLine)
										.TextStyle(&Theme.TextFont)
										.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersCompanyVisibility)
									]
								]
							]
						]
					]

				// Company description
				+ SVerticalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareWorldEconomyMenu::GetCompanyDescription)
					.WrapTextAt(Theme.ContentWidth * 0.60)
					.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersCompanyVisibility)
				]

				// List of companies being filtered
				+SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				[
					SAssignNew(SelectedCompaniesText, STextBlock)
					.TextStyle(&Theme.TextFont)
					.WrapTextAt(Theme.ContentWidth * 0.60)
					.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersCompanyVisibility)
				]

				// Add/Remove selected company
				+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Left)
					[
						SAssignNew(AddRemoveSelectedCompany, SFlareButton)
						.Text(LOCTEXT("CompanyAdd", "Add Company"))
						.HelpText(LOCTEXT("CompanyAddHelp", "Add selected company"))
						.Width(6)
						.OnClicked(this, &SFlareWorldEconomyMenu::AddRemoveCompanyFilter)
						.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersCompanyVisibility)
					]

				// Include shipyards
				+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Left)
					[
						SAssignNew(ShipyardsFilterButton, SFlareButton)
						.Text(LOCTEXT("IncludeShipyards", "Filter Shipyards"))
						.HelpText(LOCTEXT("IncludeShipyardsHelp", "Filter Shipyards"))
						.Toggle(true)
						.Width(6)
						.OnClicked(this, &SFlareWorldEconomyMenu::OnToggleShowFlags)
						.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersStationModeVisibility)
					]

				// Include Travellers
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Left)
				[
					SAssignNew(IncludeTravelersButton, SFlareButton)
					.Text(LOCTEXT("IncludeTravelers", "Include travelers"))
					.Toggle(true)
					.Width(6)
					.OnClicked(this, &SFlareWorldEconomyMenu::OnToggleShowFlags)
					.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersShipModeVisibility)
				]

				// Include hubs
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Left)
				[
					SAssignNew(StorageHubButton, SFlareButton)
					.Text(LOCTEXT("IncludeHubs", "Include Storage Hubs"))
					.HelpText(LOCTEXT("IncludeHubsHelp", "Include Storage Hubs"))
					.Toggle(true)
					.Width(6)
					.OnClicked(this, &SFlareWorldEconomyMenu::OnToggleShowFlags)
					.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersStationModeVisibility)
				]
	
				+ SVerticalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SAssignNew(QuantityButton,SFlareButton)
					.Text(LOCTEXT("SortQuantity", "Sort Quantity"))
					.HelpText(LOCTEXT("SortQuantityHelp", "Sort by quantity"))
					.Toggle(true)
					.Width(6)
					.OnClicked(this, &SFlareWorldEconomyMenu::OnToggleShowFlags)
					.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersResourceVisibility)
				]

				+ SVerticalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SAssignNew(DistanceButton,SFlareButton)
					.Text(LOCTEXT("SortDistance", "Sort Distance"))
					.HelpText(LOCTEXT("SortDistanceHelp", "Sort by distance"))
					.Toggle(true)
					.Width(6)
					.OnClicked(this, &SFlareWorldEconomyMenu::OnToggleShowFlags)
					.Visibility(this, &SFlareWorldEconomyMenu::GetStationFiltersVisibility)
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(Theme.SmallContentPadding)

			[
				SNew(SScrollBox)
				.Style(&Theme.ScrollBoxStyle)
				.ScrollBarStyle(&Theme.ScrollBarStyle)

				+ SScrollBox::Slot()
				.Padding(Theme.ContentPadding)
				[
					SAssignNew(StationList, SFlareList)
					.MenuManager(MenuManager)
					.WidthAdjuster(1.25f)
					.UseCompactDisplay(false)
					.Title(LOCTEXT("Stations", "Stations"))
				]
			]
		]
	]

		// Population Tab
		+ SFlareTabView::Slot()
		.Header(LOCTEXT("PopulationMainTab", "Population"))
		.HeaderHelp(LOCTEXT("PopulationMainTabHelp", "Population economy information"))
		[
		SNew(SVerticalBox)

		// Content
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			.Padding(Theme.ContentPadding)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.MaxWidth(128)
				.Padding(Theme.SmallContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(FText(LOCTEXT("PopulationInfosFirst", "\n\n\u2022Population\n\u2022Money\n\u2022Wealth\n\u2022Happiness")))
					.WrapTextAt(Theme.ContentWidth / 2)
				]
				+ SHorizontalBox::Slot()
				.MaxWidth(128)
				.Padding(Theme.SmallContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareWorldEconomyMenu::GetPopulationInfosAverages)
					.WrapTextAt(Theme.ContentWidth / 2)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareWorldEconomyMenu::GetPopulationInfosTotals)
					.WrapTextAt(Theme.ContentWidth / 2)
				]
			]

			/*
			// Resource info
			+ SScrollBox::Slot()
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TextFont)
				.Text(this, &SFlareWorldEconomyMenu::GetResourceInfo)
				.WrapTextAt(Theme.ContentWidth / 2)
			]
			*/

			+ SScrollBox::Slot()
			.Padding(Theme.ContentPadding)
			[
				SNew(SBox)
				.WidthOverride(ECONOMY_TABLE_WIDTH_FULL * Theme.ContentWidth)
				.Padding(FMargin(0))
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Sector name
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_LARGE * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("SectorNameColumnTitle", "Sector"))
								.Width(ECONOMY_TABLE_BUTTON_LARGE)
								.Transparent(true)
								.Icon(this, &SFlareWorldEconomyMenu::GetSortIconPop, EFlareEconomySort::ES_Sector)
								.OnClicked(this, &SFlareWorldEconomyMenu::ToggleSortTypePop, EFlareEconomySort::ES_Sector)
							]
						]

						// Production
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("PopulationColumnTitleInfo", "Population"))
								.Width(ECONOMY_TABLE_BUTTON_SMALL)
								.Transparent(true)
								.Icon(this, &SFlareWorldEconomyMenu::GetSortIconPop, EFlareEconomySort::ES_Pop_Total)
								.OnClicked(this, &SFlareWorldEconomyMenu::ToggleSortTypePop, EFlareEconomySort::ES_Pop_Total)
							]
						]

						// Money
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("MoneyTitleInfo", "Money"))
								.HelpText(LOCTEXT("MoneyTitleHelp", "Total money belonging to the sector population"))
								.Width(ECONOMY_TABLE_BUTTON_SMALL)
								.Transparent(true)
								.Icon(this, &SFlareWorldEconomyMenu::GetSortIconPop, EFlareEconomySort::ES_Pop_Money)
								.OnClicked(this, &SFlareWorldEconomyMenu::ToggleSortTypePop, EFlareEconomySort::ES_Pop_Money)
							]
						]

						// Wealth
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("WealthColumnTitleInfo", "Wealth"))
								.HelpText(LOCTEXT("WealthColumnTitleHelp", "Total wealth of each individual population. Higher wealth increases migration desirability"))
								.Width(ECONOMY_TABLE_BUTTON_SMALL)
								.Transparent(true)
								.Icon(this, &SFlareWorldEconomyMenu::GetSortIconPop, EFlareEconomySort::ES_Pop_Wealth)
								.OnClicked(this, &SFlareWorldEconomyMenu::ToggleSortTypePop, EFlareEconomySort::ES_Pop_Wealth)
							]
						]

						// Capacity
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.ContentPadding)
						[
							SNew(SBox)
							.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("HappinessColumnTitleInfo", "Happiness"))
								.HelpText(LOCTEXT("HappinessColumnTitleHelp", "Total happiness of sector. Higher happiness increases population birth and also increases migration desirability"))
								.Width(ECONOMY_TABLE_BUTTON_SMALL)
								.Transparent(true)
								.Icon(this, &SFlareWorldEconomyMenu::GetSortIconPop, EFlareEconomySort::ES_Pop_Happiness)
								.OnClicked(this, &SFlareWorldEconomyMenu::ToggleSortTypePop, EFlareEconomySort::ES_Pop_Happiness)
							]
						]
					]

					// List
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(SectorPopList, SVerticalBox)
					]
				]
			]
		]
	]

];
	IncludeTravelersButton->SetActive(true);
	IncludeTradingHubsButton->SetActive(false);
	TargetCompanies.Empty();
	StationShipMode = false;
	StationShipsButton->SetText(LOCTEXT("Stations", "STATIONS"));
	StationShipsButton->SetHelpText(LOCTEXT("StationsHelp", "Filtering by stations"));

	InputOutputMode = true;
	InputOutputButton->SetHelpText(LOCTEXT("InputHelp", "Filter input"));
	InputOutputButton->SetText(LOCTEXT("Input", "INPUT"));

	// Default state
	IsCurrentSortDescending = false;
	IsCurrentPopSortDescending = false;
	CurrentSortType = EFlareEconomySort::ES_Sector;
	CurrentSortTypePop = EFlareEconomySort::ES_Sector;
}

/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

EVisibility SFlareWorldEconomyMenu::GetStationFiltersVisibility() const
{
	if (FiltersButton->IsActive())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SFlareWorldEconomyMenu::GetStationFiltersStationModeVisibility() const
{
	if (FiltersButton->IsActive()&&!StationShipMode)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SFlareWorldEconomyMenu::GetStationFiltersShipModeVisibility() const
{
	if (FiltersButton->IsActive() && StationShipMode)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SFlareWorldEconomyMenu::GetStationFiltersResourceVisibility() const
{
	if (FiltersButton->IsActive() && ResourceFiltersButton->IsActive())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SFlareWorldEconomyMenu::GetStationFiltersResourceInputOutputVisibility() const
{
	if (FiltersButton->IsActive() && ResourceFiltersButton->IsActive() && !StationShipMode)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SFlareWorldEconomyMenu::GetStationFiltersCompanyVisibility() const
{
	if (FiltersButton->IsActive() && CompanyFiltersButton->IsActive())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

void SFlareWorldEconomyMenu::OnToggleShowFlags()
{
	RefreshStationList();
}

void SFlareWorldEconomyMenu::OnStationShip()
{
	if (StationShipMode)
	{
		StationShipMode = false;
		StationShipsButton->SetText(LOCTEXT("StationsUpper", "STATIONS"));
		StationShipsButton->SetHelpText(LOCTEXT("StationsHelp", "Filtering by stations"));
	}
	else
	{
		StationShipMode = true;
		StationShipsButton->SetText(LOCTEXT("ShipsUpper", "SHIPS"));
		StationShipsButton->SetHelpText(LOCTEXT("ShipsHelp", "Filtering by ships"));

	}
	RefreshStationList();
}

void SFlareWorldEconomyMenu::OnInputOutput()
{
	if (InputOutputMode)
	{
		InputOutputMode = false;
		InputOutputButton->SetHelpText(LOCTEXT("OutputHelp", "Filter output"));
		InputOutputButton->SetText(LOCTEXT("Output", "OUTPUT"));
	}
	else
	{
		InputOutputMode = true;
		InputOutputButton->SetHelpText(LOCTEXT("InputHelp", "Filter input"));
		InputOutputButton->SetText(LOCTEXT("Input", "INPUT"));
	}
	RefreshStationList();
}



void SFlareWorldEconomyMenu::GenerateStationList()
{
	ShipsTravelArray.Empty();
	ShipsArray.Empty();
	StationsArray.Empty();
	UFlareCompany* Company = MenuManager->GetPC()->GetCompany();

	for (int32 SectorIndex = 0; SectorIndex < Company->GetVisitedSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* TargetSector = Company->GetVisitedSectors()[SectorIndex];
		TArray<UFlareSimulatedSpacecraft*> SectorStations = TargetSector->GetSectorStations();
		TArray<UFlareSimulatedSpacecraft*> SectorShips = TargetSector->GetSectorShips();

		StationsArray.Reserve(StationsArray.Num() + SectorStations.Num());
		for (int32 SpacecraftIndex = 0; SpacecraftIndex < SectorStations.Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* StationCandidate = SectorStations[SpacecraftIndex];

			if (StationCandidate && StationCandidate->GetDamageSystem()->IsAlive())
			{
				StationsArray.Add(StationCandidate);
			}
		}
		ShipsArray.Reserve(ShipsArray.Num() + SectorShips.Num());
		for (int32 SpacecraftIndex = 0; SpacecraftIndex < SectorShips.Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* ShipCanditate = SectorShips[SpacecraftIndex];
			if (ShipCanditate && ShipCanditate->GetDamageSystem()->IsAlive())
			{
				ShipsArray.Add(ShipCanditate);
			}
		}
		for (int32 TravelIndex = 0; TravelIndex < TargetSector->GetGame()->GetGameWorld()->GetTravels().Num(); TravelIndex++)
		{
			UFlareTravel* Travel = TargetSector->GetGame()->GetGameWorld()->GetTravels()[TravelIndex];
			if (Travel)
			{
				if (Travel->GetDestinationSector() == TargetSector)
				{
					UFlareFleet* TravelFleet = Travel->GetFleet();
					if (TravelFleet)
					{
						TArray<UFlareSimulatedSpacecraft*> FleetedShips = TravelFleet->GetShips();
						for (int ShipIndex = 0; ShipIndex < FleetedShips.Num(); ShipIndex++)
						{
							UFlareSimulatedSpacecraft* ShipCanditate = FleetedShips[ShipIndex];
							if (ShipCanditate)
							{
								ShipsTravelArray.Add(ShipCanditate);
							}
						}
					}
				}
			}
		}
	}
	RefreshStationList();
}

bool SFlareWorldEconomyMenu::PassesFilterList(UFlareSimulatedSpacecraft* StationCandidate)
{
	if (!StationCandidate)
	{
		return false;
	}

	if (!StationShipMode)
	{
		//station filter
		if (!StorageHubButton->IsActive() && StationCandidate->HasCapability(EFlareSpacecraftCapability::Storage))
		{
			return false;
			//filter out storage hubs if button not enabled
		}

		if (ShipyardsFilterButton->IsActive() && !StationCandidate->IsShipyard())
		{
			return false;
			// Filter out all non-shipyards if shipyard filter is active
		}
	}

	if (CompanyFiltersButton->IsActive())
	{
		if (TargetCompanies.Num() > 0)
		{
			if (!(TargetCompanies.Find(StationCandidate->GetCompany()) != INDEX_NONE) && TargetCompany != StationCandidate->GetCompany())
			{
				return false;
			}
		}
		else if (TargetCompany != StationCandidate->GetCompany())
		{
			return false;
		}
	}

	if (ResourceFiltersButton->IsActive())
	{
		if (!StationShipMode)
			//station filter
		{
			if (InputOutputMode)
			{
				if (!StationCandidate->GetActiveCargoBay()->WantBuy(TargetResource, nullptr))
				{
					return false;
				}

			}
			else
			{
				if (!StationCandidate->GetActiveCargoBay()->WantSell(TargetResource, nullptr))
				{
					return false;
				}
			}
		}
		else
		{
			//ship filter
			if (!StationCandidate->GetActiveCargoBay()->GetResourceQuantitySimple(TargetResource))
			{
				return false;
			}
		}
	}
	return true;
}

void SFlareWorldEconomyMenu::RefreshStationList()
{
	bool DisableDefaultSorting = false;
	StationList->Reset();

	TArray<UFlareSimulatedSpacecraft*> CurrentArray;
	if (!StationShipMode)
	{
		CurrentArray = StationsArray;
	}
	else
	{
		CurrentArray = ShipsArray;
	}

	TArray<UFlareSimulatedSpacecraft*> FilteredStationsArray;
	for (int32 SpacecraftIndex = 0; SpacecraftIndex < CurrentArray.Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* StationCandidate = CurrentArray[SpacecraftIndex];
		if (StationCandidate)
		{
			if (FiltersButton->IsActive())
			{
				if (PassesFilterList(StationCandidate))
				{
					FilteredStationsArray.Add(StationCandidate);
				}
			}
			else
			{
				FilteredStationsArray.Add(StationCandidate);
			}
		}
	}

	if (StationShipMode)
	{
		if ((FiltersButton->IsActive() && IncludeTravelersButton->IsActive()) || !FiltersButton->IsActive())
		{
			for (int32 SpacecraftIndex = 0; SpacecraftIndex < ShipsTravelArray.Num(); SpacecraftIndex++)
			{
				UFlareSimulatedSpacecraft* StationCandidate = ShipsTravelArray[SpacecraftIndex];
				if (StationCandidate)
				{
					if (FiltersButton->IsActive())
					{
						if (PassesFilterList(StationCandidate))
						{
							FilteredStationsArray.Add(StationCandidate);
						}
					}
					else
					{
						FilteredStationsArray.Add(StationCandidate);
					}
				}
			}
		}
	}

	if (FiltersButton->IsActive())
	{
		if (QuantityButton->IsActive()&&!DistanceButton->IsActive())
		{
			DisableDefaultSorting = true;
			if (ResourceFiltersButton->IsActive())
			{
				FilteredStationsArray.Sort([this](UFlareSimulatedSpacecraft& ShipA, UFlareSimulatedSpacecraft& ShipB)
				{
					int32 AQuantity = ShipA.GetActiveCargoBay()->GetResourceQuantitySimple(TargetResource);
					int32 BQuantity = ShipB.GetActiveCargoBay()->GetResourceQuantitySimple(TargetResource);
					return (AQuantity > BQuantity);
				});
			}
			else
			{
				FilteredStationsArray.Sort([this](UFlareSimulatedSpacecraft& ShipA, UFlareSimulatedSpacecraft& ShipB)
				{
					int32 AQuantity = ShipA.GetActiveCargoBay()->GetUsedCargoSpace();
					int32 BQuantity = ShipB.GetActiveCargoBay()->GetUsedCargoSpace();
					return (AQuantity > BQuantity);
				});
			}
		}

		else if (DistanceButton->IsActive() && !QuantityButton->IsActive() && MenuManager->GetPC()->GetPlayerFleet()->GetCurrentSector())
		{
			DisableDefaultSorting = true;
			FilteredStationsArray.Sort([this](UFlareSimulatedSpacecraft& ShipA ,UFlareSimulatedSpacecraft& ShipB)
			{
				UFlareSimulatedSector* PlayerSector = MenuManager->GetPC()->GetPlayerFleet()->GetCurrentSector();

				if (ShipA.GetCurrentSector() == PlayerSector)
				{
					return true;
				}
				else if (ShipB.GetCurrentSector() == PlayerSector)
				{
					return false;
				}
				else
				{
					int64 DistanceComparisonA = UFlareTravel::ComputeTravelDuration(ShipA.GetGame()->GetGameWorld(), PlayerSector, ShipA.GetCurrentSector(), ShipA.GetGame()->GetPC()->GetCompany());
					int64 DistanceComparisonB = UFlareTravel::ComputeTravelDuration(ShipB.GetGame()->GetGameWorld(), PlayerSector, ShipB.GetCurrentSector(), ShipB.GetGame()->GetPC()->GetCompany());
					if (DistanceComparisonA < DistanceComparisonB)
					{
						return true;
					}

					
//					return (ShipA.GetCurrentSector()->GetSectorName().ToString() < ShipB.GetCurrentSector()->GetSectorName().ToString());
					return false;
				}
			});
		}
		else if (DistanceButton->IsActive() && QuantityButton->IsActive() && MenuManager->GetPC()->GetPlayerFleet()->GetCurrentSector())
		{
			DisableDefaultSorting = true;
			FilteredStationsArray.Sort([this](UFlareSimulatedSpacecraft& ShipA, UFlareSimulatedSpacecraft& ShipB)
			{
				int32 AQuantity;
				int32 BQuantity;
				if (ResourceFiltersButton->IsActive())
				{
					AQuantity = ShipA.GetActiveCargoBay()->GetResourceQuantitySimple(TargetResource);
					BQuantity = ShipB.GetActiveCargoBay()->GetResourceQuantitySimple(TargetResource);
				}
				else
				{
					AQuantity = ShipA.GetActiveCargoBay()->GetUsedCargoSpace();
					BQuantity = ShipB.GetActiveCargoBay()->GetUsedCargoSpace();
				}

				if (AQuantity > BQuantity)
				{
					return true;
				}

				UFlareSimulatedSector* PlayerSector = MenuManager->GetPC()->GetPlayerFleet()->GetCurrentSector();

				if (ShipA.GetCurrentSector() == PlayerSector)
				{
					return true;
				}
				else if (ShipB.GetCurrentSector() == PlayerSector)
				{
					return false;
				}
				else
				{
					int64 DistanceComparisonA = UFlareTravel::ComputeTravelDuration(ShipA.GetGame()->GetGameWorld(), PlayerSector, ShipA.GetCurrentSector(), ShipA.GetGame()->GetPC()->GetCompany());
					int64 DistanceComparisonB = UFlareTravel::ComputeTravelDuration(ShipB.GetGame()->GetGameWorld(), PlayerSector, ShipB.GetCurrentSector(), ShipB.GetGame()->GetPC()->GetCompany());
					if (DistanceComparisonA < DistanceComparisonB)
					{
						return true;
					}
//					return (ShipA.GetCurrentSector()->GetSectorName().ToString() < ShipB.GetCurrentSector()->GetSectorName().ToString());
					return false;
				}
			});

		}
	}

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < FilteredStationsArray.Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* StationCandidate = FilteredStationsArray[SpacecraftIndex];
		if (StationCandidate)
		{
			StationList->AddShip(StationCandidate);
		}
	}

	FString FormattedNumber = FString::FormatAsNumber(StationList->GetItemCount());
	if (!StationShipMode)
	{
		StationList->SetTitle(FText::Format(LOCTEXT("StationsHeader", "Stations ({0})"),
		FText::FromString(FormattedNumber)));
	}
	else
	{
		StationList->SetTitle(FText::Format(LOCTEXT("ShipsHeader", "Ships ({0})"),
		FText::FromString(FormattedNumber)));
	}

	StationList->RefreshList(DisableDefaultSorting);

}

void SFlareWorldEconomyMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareWorldEconomyMenu::Enter(FFlareResourceDescription* Resource, UFlareSimulatedSector* Sector)
{
	FLOG("SFlareWorldEconomyMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	if (Resource)
	{
		//came from sector "details" menu, setting menu back to tab index 0 which is main economy tab
		TargetResource = Resource;
		TabView->SetCurrentTabIndex(0);
	}

	CompanyList.Empty();
	for (int CompanyIndex = 0; CompanyIndex < MenuManager->GetPC()->GetGame()->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
		UFlareCompany* Company = MenuManager->GetPC()->GetGame()->GetGameWorld()->GetCompanies()[CompanyIndex];
		CompanyList.Add(Company);
	}

	WorldStats = WorldHelper::ComputeWorldResourceStats(MenuManager->GetGame(), IncludeTradingHubsButton->IsActive());

	// Update resource selector
	ResourceSelector->RefreshOptions();
	StationResourceSelector->RefreshOptions();
	StationCompanySelector->RefreshOptions();

	if (TargetResource)
	{
		ResourceSelector->SetSelectedItem(MenuManager->GetPC()->GetGame()->GetResourceCatalog()->GetEntry(TargetResource));
		StationResourceSelector->SetSelectedItem(MenuManager->GetPC()->GetGame()->GetResourceCatalog()->GetEntry(TargetResource));
	}
	else
	{
		TargetResource = &MenuManager->GetPC()->GetGame()->GetResourceCatalog()->GetResourceList()[0]->Data;
	}

	if (TargetCompany)
	{
		StationCompanySelector->SetSelectedItem(TargetCompany);
	}
	else
	{
		StationCompanySelector->SetSelectedItem(MenuManager->GetPC()->GetGame()->GetGameWorld()->GetCompanies()[0]);
		TargetCompany = StationCompanySelector->GetSelectedItem();
	}

	GenerateSectorList();
	GenerateSectorPopList();

	GenerateStationList();
}

void SFlareWorldEconomyMenu::GenerateSectorPopList()
{
	SectorPopList->ClearChildren();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Get the sector list
	TArray<UFlareSimulatedSector*>& Sectors = MenuManager->GetPC()->GetCompany()->GetVisitedSectors();

	KnownPopulationTotal = 0;
	KnownPopulationMoney = 0;
	KnownPopulationWealth = 0.f;
	KnownPopulationHappiness = 0.f;

	// Apply the current sort
	Sectors.Sort([this](UFlareSimulatedSector& S1, UFlareSimulatedSector& S2)
	{
		bool Result = false;
		// Get sorting data
		uint32 Population1 = S1.GetPeople()->GetPopulation();
		uint32 Population2 = S2.GetPeople()->GetPopulation();
		int64 Money1 = S1.GetPeople()->GetMoney();
		int64 Money2 = S2.GetPeople()->GetMoney();
		float Wealth1 = S1.GetPeople()->GetWealth();
		float Wealth2 = S2.GetPeople()->GetWealth();
		float Happiness1 = S1.GetPeople()->GetHappiness();
		float Happiness2 = S2.GetPeople()->GetHappiness();

		// Apply sort
		switch (this->CurrentSortTypePop)
		{
		case EFlareEconomySort::ES_Sector:
			Result = S1.GetSectorName().ToString() > S2.GetSectorName().ToString();
			break;
		case EFlareEconomySort::ES_Pop_Total:
			Result = Population1 > Population2;
			break;
		case EFlareEconomySort::ES_Pop_Money:
			Result = Money1 > Money2;
			break;
		case EFlareEconomySort::ES_Pop_Wealth:
			Result = Wealth1 > Wealth2;
			break;
		case EFlareEconomySort::ES_Pop_Happiness:
			Result = Happiness1 > Happiness2;
			break;
		}

		return this->IsCurrentPopSortDescending ? Result : !Result;
	});

	// Sector list
	for (int32 SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Sectors[SectorIndex];

		if (Sector->GetPeople()->GetPopulation())
		{
			KnownPopulationTotal = KnownPopulationTotal + Sector->GetPeople()->GetPopulation();
		}

		if (Sector->GetPeople()->GetMoney())
		{
			KnownPopulationMoney = KnownPopulationMoney + (Sector->GetPeople()->GetMoney() / 100.0f);
		}

		if (Sector->GetPeople()->GetWealth())
		{
			KnownPopulationWealth = KnownPopulationWealth + (Sector->GetPeople()->GetWealth() / 100.0f);
		}

		if (Sector->GetPeople()->GetHappiness())
		{
			KnownPopulationHappiness = KnownPopulationHappiness + (FMath::Abs((Sector->GetPeople()->GetHappiness()) * 100.0f) / 2);
		}

		SectorPopList->AddSlot()
		.Padding(FMargin(0))
		[
			SNew(SBorder)
			.Padding(FMargin(1))
			.BorderImage((SectorIndex % 2 == 0 ? &Theme.EvenBrush : &Theme.OddBrush))
			[
				SNew(SHorizontalBox)

				// Sector
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(ECONOMY_TABLE_WIDTH_LARGE * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(SFlareButton)
						.Width(ECONOMY_TABLE_BUTTON_LARGE)
						.Text(this, &SFlareWorldEconomyMenu::GetSectorText, Sector)
						.Color(this, &SFlareWorldEconomyMenu::GetSectorTextColor, Sector)
						.Icon(FFlareStyleSet::GetIcon("Travel"))
						.OnClicked(this, &SFlareWorldEconomyMenu::OnOpenSector, Sector)
					]
				]

				// Population
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareWorldEconomyMenu::GetPopTotalInfo, Sector)
					]
				]

				// Money
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareWorldEconomyMenu::GetPopMoneyInfo, Sector)

					]
				]

				// Wealth
				+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(Theme.ContentPadding)
					[
						SNew(SBox)
						.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareWorldEconomyMenu::GetPopWealthInfo, Sector)
					]
				]

				// Happiness
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareWorldEconomyMenu::GetPopHappinessInfo, Sector)
					]
				]
			]
		];
	}
	SlatePrepass(FSlateApplicationBase::Get().GetApplicationScale());
}

void SFlareWorldEconomyMenu::GenerateSectorList()
{
	SectorList->ClearChildren();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	if (TargetResource == NULL)
	{
		return;
	}

	// Get the sector list
	TArray<UFlareSimulatedSector*>& Sectors = MenuManager->GetPC()->GetCompany()->GetVisitedSectors();

	// Apply the current sort
	Sectors.Sort([this](UFlareSimulatedSector& S1, UFlareSimulatedSector& S2)
	{
		bool Result = false;

		// Get sorting data
		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> Stats1 = SectorHelper::ComputeSectorResourceStats(&S1, IncludeTradingHubsButton->IsActive());
		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> Stats2 = SectorHelper::ComputeSectorResourceStats(&S2, IncludeTradingHubsButton->IsActive());
		int64 ResourcePrice1 = S1.GetResourcePrice(TargetResource, EFlareResourcePriceContext::Default);
		int64 ResourcePrice2 = S2.GetResourcePrice(TargetResource, EFlareResourcePriceContext::Default);
		int64 LastResourcePrice1 = S1.GetResourcePrice(TargetResource, EFlareResourcePriceContext::Default, 30);
		int64 LastResourcePrice2 = S2.GetResourcePrice(TargetResource, EFlareResourcePriceContext::Default, 30);
		float Variation1 = (((float)ResourcePrice1) / ((float)LastResourcePrice1) - 1);
		float Variation2 = (((float)ResourcePrice2) / ((float)LastResourcePrice2) - 1);

		// Apply sort
		switch (this->CurrentSortType)
		{
		case EFlareEconomySort::ES_Sector:
			Result = S1.GetSectorName().ToString() > S2.GetSectorName().ToString();
			break;
		case EFlareEconomySort::ES_Production:
			Result = (Stats1[this->TargetResource].Production > Stats2[this->TargetResource].Production);
			break;
		case EFlareEconomySort::ES_Consumption:
			Result = (Stats1[this->TargetResource].Consumption > Stats2[this->TargetResource].Consumption);
			break;
		case EFlareEconomySort::ES_Stock:
			Result = (Stats1[this->TargetResource].Stock > Stats2[this->TargetResource].Stock);
			break;
		case EFlareEconomySort::ES_Needs:
			Result = (Stats1[this->TargetResource].Capacity > Stats2[this->TargetResource].Capacity);
			break;
		case EFlareEconomySort::ES_Price:
			Result = ResourcePrice1 > ResourcePrice2;
			break;
		case EFlareEconomySort::ES_Variation:
			Result = Variation1 > Variation2;
			break;
		}

		return this->IsCurrentSortDescending ? Result : !Result;
	});

	// Sector list
	for (int32 SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Sectors[SectorIndex];

		SectorList->AddSlot()
		.Padding(FMargin(0))
		[
			SNew(SBorder)
			.Padding(FMargin(1))
			.BorderImage((SectorIndex % 2 == 0 ? &Theme.EvenBrush : &Theme.OddBrush))
			[
				SNew(SHorizontalBox)

				// Sector
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(ECONOMY_TABLE_WIDTH_LARGE * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(SFlareButton)
						.Width(ECONOMY_TABLE_BUTTON_LARGE)
						.Text(this, &SFlareWorldEconomyMenu::GetSectorText, Sector)
						.Color(this, &SFlareWorldEconomyMenu::GetSectorTextColor, Sector)
						.Icon(FFlareStyleSet::GetIcon("Travel"))
						.OnClicked(this, &SFlareWorldEconomyMenu::OnOpenSector, Sector)
					]
				]

				// Production
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareWorldEconomyMenu::GetResourceProductionInfo, Sector)
					]
				]

				// Consumption
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareWorldEconomyMenu::GetResourceConsumptionInfo, Sector)
					]
				]

				// Stock
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareWorldEconomyMenu::GetResourceStockInfo, Sector)
					]
				]

				// Capacity
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareWorldEconomyMenu::GetResourceCapacityInfo, Sector)
					]
				]

				// Price
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.ColorAndOpacity(this, &SFlareWorldEconomyMenu::GetPriceColor, Sector)
						.Text(this, &SFlareWorldEconomyMenu::GetResourcePriceInfo, Sector)
					]
				]

				// Price variation
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(ECONOMY_TABLE_WIDTH_SMALL * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareWorldEconomyMenu::GetResourcePriceVariationInfo, Sector, TSharedPtr<int32>(new int32(30)))
					]
				]

				// Details
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.WidthOverride(ECONOMY_TABLE_WIDTH_MEDIUM * Theme.ContentWidth)
					[
						SNew(SFlareButton)
						.Text(LOCTEXT("DetailButton", "Details"))
						.Icon(FFlareStyleSet::GetIcon("Travel"))
						.OnClicked(this, &SFlareWorldEconomyMenu::OnOpenSectorPrices, Sector)
						.Width(4)
					]
				]
			]
		];
	}
	SlatePrepass(FSlateApplicationBase::Get().GetApplicationScale());
}

void SFlareWorldEconomyMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
	SectorList->ClearChildren();
	SectorPopList->ClearChildren();
	StationList->Reset();
	StationsArray.Empty();
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

const FSlateBrush* SFlareWorldEconomyMenu::GetSortIcon(EFlareEconomySort::Type Type) const
{
	if (Type == CurrentSortType)
	{
		if (IsCurrentSortDescending)
		{
			return FFlareStyleSet::GetIcon("MoveDown");
		}
		else
		{
			return FFlareStyleSet::GetIcon("MoveUp");
		}
	}
	else
	{
		return FFlareStyleSet::GetIcon("MoveUpDown");
	}
}

const FSlateBrush* SFlareWorldEconomyMenu::GetSortIconPop(EFlareEconomySort::Type Type) const
{
	if (Type == CurrentSortTypePop)
	{
		if (IsCurrentPopSortDescending)
		{
			return FFlareStyleSet::GetIcon("MoveDown");
		}
		else
		{
			return FFlareStyleSet::GetIcon("MoveUp");
		}
	}
	else
	{
		return FFlareStyleSet::GetIcon("MoveUpDown");
	}
}



void SFlareWorldEconomyMenu::ToggleSortType(EFlareEconomySort::Type Type)
{
	if (Type == CurrentSortType)
	{
		IsCurrentSortDescending = !IsCurrentSortDescending;
	}
	else
	{
		IsCurrentSortDescending = true;
		CurrentSortType = Type;
	}

	GenerateSectorList();
}

void SFlareWorldEconomyMenu::ToggleSortTypePop(EFlareEconomySort::Type Type)
{
	if (Type == CurrentSortTypePop)
	{
		IsCurrentPopSortDescending = !IsCurrentPopSortDescending;
	}
	else
	{
		IsCurrentPopSortDescending = true;
		CurrentSortTypePop = Type;
	}

	GenerateSectorPopList();
}

FSlateColor SFlareWorldEconomyMenu::GetPriceColor(UFlareSimulatedSector* Sector) const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	if (TargetResource)
	{

		FLinearColor HighPriceColor = Theme.FriendlyColor;
		FLinearColor MeanPriceColor = Theme.NeutralColor;
		FLinearColor LowPriceColor = Theme.EnemyColor;

		float ResourcePrice = Sector->GetPreciseResourcePrice(TargetResource);

		float PriceRatio = (ResourcePrice - TargetResource->MinPrice) / (float)(TargetResource->MaxPrice - TargetResource->MinPrice);

		if (PriceRatio > 0.5)
		{
			return FMath::Lerp(MeanPriceColor, HighPriceColor, 2.f * (PriceRatio - 0.5));
		}
		else
		{
			return FMath::Lerp(LowPriceColor, MeanPriceColor, 2.f * PriceRatio);
		}

		return FMath::Lerp(LowPriceColor, HighPriceColor, PriceRatio);
	}
	return Theme.FriendlyColor;
}

FText SFlareWorldEconomyMenu::GetCompanyDescription() const
{
	if (TargetCompany)
	{
		const FFlareCompanyDescription* Desc = TargetCompany->GetDescription();
		if (Desc)
		{
			return Desc->Description;
		}
	}

	return FText();
}

FText SFlareWorldEconomyMenu::GetResourceDescription() const
{
	if (TargetResource)
	{
		return TargetResource->Description;
	}

	return FText();
}

FText SFlareWorldEconomyMenu::GetResourceName() const
{
	if (TargetResource)
	{
		return FText::Format(LOCTEXT("ResourceNameFormat", "Economy status for {0}"), TargetResource->Name);
	}

	return LOCTEXT("NoResourceSelected", "No resource selected");
}

const FSlateBrush* SFlareWorldEconomyMenu::GetCompanyEmblem() const
{
	return (TargetCompany ? TargetCompany->GetEmblem() : NULL);
}



const FSlateBrush* SFlareWorldEconomyMenu::GetResourceIcon() const
{
	if (TargetResource)
	{
		return &TargetResource->Icon;
	}

	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	return &Theme.ResourceBackground;
}

FText SFlareWorldEconomyMenu::GetPopulationInfosAverages() const
{
	TArray<UFlareSimulatedSector*>& Sectors = MenuManager->GetPC()->GetCompany()->GetVisitedSectors();

	FNumberFormattingOptions WealthFormat;
	WealthFormat.MaximumFractionalDigits = 0;

	FNumberFormattingOptions HapinessFormat;
	HapinessFormat.MaximumFractionalDigits = 2;

	return FText::Format(LOCTEXT("PopulationInfosAverages","Average\n\n{0}\n{1}\n{2}\n{3}%"),
		KnownPopulationTotal / Sectors.Num(),
		KnownPopulationMoney / Sectors.Num(),
		FText::AsNumber(KnownPopulationWealth / Sectors.Num(), &HapinessFormat),
		FText::AsNumber(KnownPopulationHappiness / Sectors.Num(),&HapinessFormat)
		);
}

FText SFlareWorldEconomyMenu::GetPopulationInfosTotals() const
{
	FNumberFormattingOptions WealthFormat;
	WealthFormat.MaximumFractionalDigits = 0;

	FNumberFormattingOptions HapinessFormat;
	HapinessFormat.MaximumFractionalDigits = 2;

	return FText::Format(LOCTEXT("PopulationInfosTotals", "Total\n\n{0}\n{1}\n{2}\n{3}%"),
		KnownPopulationTotal,
		KnownPopulationMoney,
		FText::AsNumber(KnownPopulationWealth, &HapinessFormat),
		FText::AsNumber(KnownPopulationHappiness, &HapinessFormat)
	);
}

FText SFlareWorldEconomyMenu::GetResourceInfo() const
{
	if (TargetResource)
	{
		if (WorldStats.Contains(TargetResource))
		{
			FNumberFormattingOptions Format;
			Format.MaximumFractionalDigits = 1;

			// Balance info
			FText BalanceText;
			float Balance = WorldStats[TargetResource].Balance;
			if (Balance > 0)
			{
				BalanceText = FText::Format(LOCTEXT("BalanceInfoPlusFormat", "+{0} / day"),
					FText::AsNumber(Balance, &Format));
			}
			else
			{
				BalanceText = FText::Format(LOCTEXT("BalanceInfoNegFormat", "{0} / day"),
					FText::AsNumber(Balance, &Format));
			}
			
			FText Part1 = FText::Format(LOCTEXT("StockInfoFormatPart1", "\u2022Transport fee: {0} credits\n\u2022 Worldwide stock: {1}\n\u2022 Worldwide needs: {2}\n"),
										UFlareGameTools::DisplayMoney(TargetResource->TransportFee),
										FText::AsNumber(WorldStats[TargetResource].Stock),
										FText::AsNumber(WorldStats[TargetResource].Capacity));
			FText Part2 = FText::Format(LOCTEXT("StockInfoFormatPart2", "\u2022 Worldwide production: {0} / day\n\u2022 Worldwide usage: {1} / day\n"),
										FText::AsNumber(WorldStats[TargetResource].Production, &Format),
										FText::AsNumber(WorldStats[TargetResource].Consumption, &Format));

			// Generate info
			return FText::Format(LOCTEXT("StockInfoFormat",
					"{0}{1}\u2022 Balance: {2}"),
				Part1,
				Part2,
				BalanceText);
		}
	}

	return FText();
}

FText SFlareWorldEconomyMenu::GetSectorText(UFlareSimulatedSector* Sector) const
{
	return FText::Format(LOCTEXT("SectorInfoFormat", "{0} ({1})"),
		Sector->GetSectorName(),
		Sector->GetSectorFriendlynessText(MenuManager->GetPC()->GetCompany()));
}

FSlateColor SFlareWorldEconomyMenu::GetSectorTextColor(UFlareSimulatedSector* Sector) const
{
	return Sector->GetSectorFriendlynessColor(MenuManager->GetPC()->GetCompany());
}

FText SFlareWorldEconomyMenu::GetPopTotalInfo(UFlareSimulatedSector* Sector) const
{
	if (Sector)
	{
		if (Sector->GetPeople()->GetPopulation())
		{
			return FText::Format(LOCTEXT("SectorPopulationAmount", "{0}"),
			FText::AsNumber(Sector->GetPeople()->GetPopulation()));
		}
	}
	return FText(LOCTEXT("SectorPopulationAmountEmpty", "0"));
}

FText SFlareWorldEconomyMenu::GetPopMoneyInfo(UFlareSimulatedSector* Sector) const
{
	if (Sector)
	{
		if (Sector->GetPeople()->GetMoney())
		{
			FNumberFormattingOptions MoneyFormat;
			MoneyFormat.MaximumFractionalDigits = 0;

			return FText::Format(LOCTEXT("SectorPopMoneyAmount", "{0}"),
			FText::AsNumber(Sector->GetPeople()->GetMoney() / 100.0f, &MoneyFormat));
		}
	}
	return FText(LOCTEXT("SectorPopulationAmountEmpty", "0"));
}

FText SFlareWorldEconomyMenu::GetPopWealthInfo(UFlareSimulatedSector* Sector) const
{
	if (Sector)
	{
		if (Sector->GetPeople()->GetWealth())
		{
			FNumberFormattingOptions MoneyFormat;
			MoneyFormat.MaximumFractionalDigits = 0;

			return FText::Format(LOCTEXT("SectorPopWealthAmount", "{0}"),
				FText::AsNumber(Sector->GetPeople()->GetWealth() / 100.0f, &MoneyFormat));
		}
	}
	return FText(LOCTEXT("SectorPopulationAmountEmpty", "0"));
}

FText SFlareWorldEconomyMenu::GetPopHappinessInfo(UFlareSimulatedSector* Sector) const
{
	if (Sector)
	{
		if (Sector->GetPeople()->GetHappiness())
		{
			FNumberFormattingOptions MoneyFormat;
			MoneyFormat.MaximumFractionalDigits = 2;

			return FText::Format(LOCTEXT("SectorHappinessAmount", "{0}%"),
			FText::AsNumber(FMath::Abs((Sector->GetPeople()->GetHappiness()) * 100.0f) / 2, &MoneyFormat));
			//devided by two because happiness goes to 200%. not really sure to display upto that amount or upto 100%
		}
	}
	return FText(LOCTEXT("SectorPopulationAmountEmpty", "0"));
}


FText SFlareWorldEconomyMenu::GetResourceProductionInfo(UFlareSimulatedSector* Sector) const
{
	if (TargetResource)
	{
		FNumberFormattingOptions Format;
		Format.MaximumFractionalDigits = 1;

		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> Stats = SectorHelper::ComputeSectorResourceStats(Sector, IncludeTradingHubsButton->IsActive());
		return FText::Format(LOCTEXT("ResourceMainProductionFormat", "{0}"),
			FText::AsNumber(Stats[TargetResource].Production, &Format));
	}

	return FText();
}

FText SFlareWorldEconomyMenu::GetResourceConsumptionInfo(UFlareSimulatedSector* Sector) const
{
	if (TargetResource)
	{
		FNumberFormattingOptions Format;
		Format.MaximumFractionalDigits = 1;

		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> Stats = SectorHelper::ComputeSectorResourceStats(Sector, IncludeTradingHubsButton->IsActive());
		return FText::Format(LOCTEXT("ResourceMainConsumptionFormat", "{0}"),
			FText::AsNumber(Stats[TargetResource].Consumption, &Format));
	}

	return FText();
}

FText SFlareWorldEconomyMenu::GetResourceStockInfo(UFlareSimulatedSector* Sector) const
{
	if (TargetResource)
	{
		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> Stats = SectorHelper::ComputeSectorResourceStats(Sector, IncludeTradingHubsButton->IsActive());
		return FText::Format(LOCTEXT("ResourceMainStockFormat", "{0}"),
			FText::AsNumber(Stats[TargetResource].Stock));
	}

	return FText();
}


FText SFlareWorldEconomyMenu::GetResourceCapacityInfo(UFlareSimulatedSector* Sector) const
{
	if (TargetResource)
	{

		TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> Stats = SectorHelper::ComputeSectorResourceStats(Sector, IncludeTradingHubsButton->IsActive());
		return FText::Format(LOCTEXT("ResourceMainCapacityFormat", "{0}"),
			FText::AsNumber(Stats[TargetResource].Capacity));
	}

	return FText();
}

FText SFlareWorldEconomyMenu::GetResourcePriceInfo(UFlareSimulatedSector* Sector) const
{
	if (TargetResource)
	{
		FNumberFormattingOptions MoneyFormat;
		MoneyFormat.MaximumFractionalDigits = 2;

		int64 ResourcePrice = Sector->GetResourcePrice(TargetResource, EFlareResourcePriceContext::Default);

		return FText::Format(LOCTEXT("ResourceMainPriceFormat", "{0} credits"),
			FText::AsNumber(ResourcePrice / 100.0f, &MoneyFormat));
	}

	return FText();
}

FText SFlareWorldEconomyMenu::GetResourcePriceVariationInfo(UFlareSimulatedSector* Sector, TSharedPtr<int32> MeanDuration) const
{
	if (TargetResource)
	{
		FNumberFormattingOptions MoneyFormat;
		MoneyFormat.MaximumFractionalDigits = 2;

		int64 ResourcePrice = Sector->GetResourcePrice(TargetResource, EFlareResourcePriceContext::Default);
		int64 LastResourcePrice = Sector->GetResourcePrice(TargetResource, EFlareResourcePriceContext::Default, *MeanDuration);

		if(ResourcePrice != LastResourcePrice)
		{
			float Variation = (((float) ResourcePrice) / ((float) LastResourcePrice) - 1);

			if(FMath::Abs(Variation) >= 0.0001)
			{
				return FText::Format(LOCTEXT("ResourceVariationFormat", "{0}{1}%"),
								(Variation > 0 ?
									LOCTEXT("ResourceVariationFormatSignPlus","+") :
									LOCTEXT("ResourceVariationFormatSignMinus","-")),
							  FText::AsNumber(FMath::Abs(Variation) * 100.0f, &MoneyFormat));
			}
		}
		return LOCTEXT("ResourceMainPriceNoVariationFormat", "-");
	}

	return FText();
}


TSharedRef<SWidget> SFlareWorldEconomyMenu::OnGenerateResourceComboLine(UFlareResourceCatalogEntry* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(STextBlock)
		.Text(Item->Data.Name)
		.TextStyle(&Theme.TextFont)
	];
}

FText SFlareWorldEconomyMenu::OnGetCurrentResourceComboLine() const
{
	UFlareResourceCatalogEntry* Item = ResourceSelector->GetSelectedItem();
	if (Item)
	{
		if (Item->Data.Name.CompareTo(Item->Data.Acronym) == 0)
		{
			return Item->Data.Name;
		}
		else
		{
			return FText::Format(LOCTEXT("ComboResourceLineFormat", "{0} ({1})"), Item->Data.Name, Item->Data.Acronym);
		}
	}
	else
	{
		return LOCTEXT("SelectResource", "Select a resource");
	}
}

TSharedRef<SWidget> SFlareWorldEconomyMenu::OnGenerateCompanyComboLine(UFlareCompany* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	return SNew(SBox)
		.Padding(Theme.ListContentPadding)
		[
			SNew(STextBlock)
			.Text(Item->GetCompanyName())
		.TextStyle(&Theme.TextFont)
		];
}

FText SFlareWorldEconomyMenu::OnGetCurrentCompanyComboLine() const
{
	UFlareCompany* Item = StationCompanySelector->GetSelectedItem();

	if (Item)
	{
		return FText::Format(LOCTEXT("ComboResourceLineFormat", "{0}"), Item->GetCompanyName());
	}
	else
	{
		return LOCTEXT("SelectCompany", "Select a company");
	}
}

void SFlareWorldEconomyMenu::OnCompanyComboLineSelectionChanged(UFlareCompany* Item, ESelectInfo::Type SelectInfo)
{
	if (Item)
	{
		TargetCompany = Item;
	}
	else
	{
		TargetCompany = NULL;
	}

	RefreshAddRemoveSelectedCompanyButton();
	RefreshStationList();
}

void SFlareWorldEconomyMenu::RefreshAddRemoveSelectedCompanyButton()
{
	if (TargetCompanies.Find(TargetCompany) != INDEX_NONE)
	{
		AddRemoveSelectedCompany->SetText(LOCTEXT("CompanyRemove", "Remove Company"));
		AddRemoveSelectedCompany->SetHelpText(LOCTEXT("CompanyRemoveHelp", "Remove selected company"));
	}
	else
	{
		AddRemoveSelectedCompany->SetText(LOCTEXT("CompanyAdd", "Add Company"));
		AddRemoveSelectedCompany->SetHelpText(LOCTEXT("CompanyAddHelp", "Add selected company"));
	}

	FText Result;
	if (TargetCompanies.Num() > 0)
	{
		for (int Index = 0; Index < TargetCompanies.Num(); Index++)
		{
			UFlareCompany* CurrentCompany = TargetCompanies[Index];
			if (CurrentCompany)
			{
				const FFlareCompanyDescription* Desc = CurrentCompany->GetDescription();
				if (Desc)
				{
					Result = FText::Format(LOCTEXT("FilteredCompanies", "{0} \n\u2022 {1}"),
					Result, Desc->Name);
				}
			}
		}
	}
	SelectedCompaniesText->SetText(Result);
}

void SFlareWorldEconomyMenu::AddRemoveCompanyFilter()
{
	if (TargetCompanies.Find(TargetCompany) != INDEX_NONE)
	{
		TargetCompanies.Remove(TargetCompany);
	}
	else
	{
		TargetCompanies.Add(TargetCompany);
	}

	RefreshAddRemoveSelectedCompanyButton();
	RefreshStationList();
}

void SFlareWorldEconomyMenu::OnResourceComboLineSelectionChanged(UFlareResourceCatalogEntry* Item, ESelectInfo::Type SelectInfo, bool Station)
{
	if (Item)
	{
		TargetResource  = &Item->Data;
	}
	else
	{
		TargetResource = NULL;
	}

	if (!Station)
	{
		StationResourceSelector->SetSelectedItem(ResourceSelector->GetSelectedItem());
		GenerateSectorList();
	}
	else
	{
		ResourceSelector->SetSelectedItem(StationResourceSelector->GetSelectedItem());
		RefreshStationList();
	}
}

void SFlareWorldEconomyMenu::OnOpenSector(UFlareSimulatedSector* Sector)
{
	FFlareMenuParameterData Data;
	Data.Sector = Sector;
	MenuManager->OpenMenu(EFlareMenu::MENU_Sector, Data);
}

void SFlareWorldEconomyMenu::OnOpenSectorPrices(UFlareSimulatedSector* Sector)
{
	FFlareMenuParameterData Data;
	Data.Sector = Sector;
	MenuManager->OpenMenu(EFlareMenu::MENU_ResourcePrices, Data);
}

void SFlareWorldEconomyMenu::OnIncludeTradingHubsToggle()
{
	GenerateSectorList();
	WorldStats = WorldHelper::ComputeWorldResourceStats(MenuManager->GetGame(), IncludeTradingHubsButton->IsActive());
}

#undef LOCTEXT_NAMESPACE
