#include "FlareFleetMenu.h"
#include "../../Flare.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareCompany.h"
#include "../../Game/FlareFleet.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../Components/FlareRoundButton.h"

#include "../../Game/FlareSectorHelper.h"
#include "../../Game/FlareGameTools.h"
#include "../../Game/FlareScenarioTools.h"

#include "SComplexGradient.h"


#define LOCTEXT_NAMESPACE "FlareFleetMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareFleetMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Setup
	FleetToAdd = NULL;
	FleetToEdit = NULL;
	ShipToRemove = NULL;

	// Gradient
	TArray<FLinearColor> HueGradientColors;
	for (int32 i = 0; i < 7; ++i)
	{
		HueGradientColors.Add(FLinearColor((i % 6) * 60.f, 1.f, 1.f).HSVToLinearRGB());
	}

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SVerticalBox)

		// Content block
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)

			// Fleet list & tools
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			[
				SNew(SScrollBox)
				.Style(&Theme.ScrollBoxStyle)
				.ScrollBarStyle(&Theme.ScrollBarStyle)

				+ SScrollBox::Slot()
				[
					SNew(SBox)
					.Visibility(this, &SFlareFleetMenu::GetEditVisibility)
					[
						SNew(SVerticalBox)
						// Fleet details
						+ SVerticalBox::Slot()
						.AutoHeight()
						.VAlign(VAlign_Top)
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(LOCTEXT("ManageFleet", "Fleet details"))
						]

						// Fleet tools
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(SHorizontalBox)
							// Name field
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Center)
							.Padding(Theme.SmallContentPadding)
							[
								SAssignNew(EditFleetName, SEditableText)
								.AllowContextMenu(false)
								.Style(&Theme.TextInputStyle)
							]

							// Confirm name
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Right)
							.Padding(Theme.SmallContentPadding)
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Width(4)
								.Icon(FFlareStyleSet::GetIcon("OK"))
								.Text(LOCTEXT("Rename", "Rename"))
								.HelpText(this, &SFlareFleetMenu::GetRenameHintText)
								.OnClicked(this, &SFlareFleetMenu::OnRenameFleet)
								.IsDisabled(this, &SFlareFleetMenu::IsRenameDisabled)
							]

							// Finish
							+ SHorizontalBox::Slot()
								.HAlign(HAlign_Right)
								.Padding(Theme.SmallContentPadding)
								.AutoWidth()
								[
									SNew(SFlareButton)
									.Width(4)
									.Icon(FFlareStyleSet::GetIcon("Stop"))
									.Text(LOCTEXT("DoneEditing", "Back"))
									.HelpText(LOCTEXT("DoneEditingInfo", "Finish editing this fleet"))
									.OnClicked(this, &SFlareFleetMenu::OnEditFinished)
								]
							]

						// Color box
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							.Padding(FMargin(4.0f, 0.0f))
							[
								SNew(SComplexGradient)
								.GradientColors(HueGradientColors)
								.Orientation(Orient_Vertical)
							]
							+ SOverlay::Slot()
							[
								SNew(SSlider)
								.IndentHandle(false)
								.Orientation(Orient_Horizontal)

								.SliderBarColor(FLinearColor::Transparent)
								.Value(this, &SFlareFleetMenu::GetColorSpinBoxValue)
								.OnValueChanged(this, &SFlareFleetMenu::OnColorSpinBoxValueChanged)
							]
						]
					]
				]

				+ SScrollBox::Slot()
				[
					SNew(SVerticalBox)
					// Fleet list
					+ SVerticalBox::Slot()
					[
						SAssignNew(FleetList, SFlareList)
						.MenuManager(MenuManager)
						.FleetList(true)
						.ArrayMode(ESelectionMode::SingleToggle)
						.OnItemSelected(this, &SFlareFleetMenu::OnFleetSelected)
						.OnItemUnSelected(this, &SFlareFleetMenu::OnFleetUnSelected)
					]
				]
			]

			// Fleet details
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			[
				SNew(SVerticalBox)
				
				// Fleet details
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.TitlePadding)
				[
					SNew(SBox)
					.Visibility(this, &SFlareFleetMenu::GetEditVisibility)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(LOCTEXT("ManageShips", "Fleet composition"))
						]
						// Add & remove
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(SHorizontalBox)
							// Add
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
							.Padding(Theme.SmallContentPadding)
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Width(4)
								.Icon(FFlareStyleSet::GetIcon("MoveRight"))
								.Text(LOCTEXT("AddToFleet", "Merge fleet"))
								.HelpText(this, &SFlareFleetMenu::GetAddHintText)
								.IsDisabled(this, &SFlareFleetMenu::IsAddDisabled)
								.OnClicked(this, &SFlareFleetMenu::OnAddToFleet)
							]

							// Remove
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
							.Padding(Theme.SmallContentPadding)
							.AutoWidth()
							[
								SAssignNew(RemoveShipButton,SFlareButton)
								.Width(5)
								.Icon(FFlareStyleSet::GetIcon("MoveLeft"))
								.Text(LOCTEXT("RemoveFromFleet", "Remove ship"))
								.HelpText(this, &SFlareFleetMenu::GetRemoveHintText)
								.IsDisabled(this, &SFlareFleetMenu::IsRemoveDisabled)
								.OnClicked(this, &SFlareFleetMenu::OnRemoveFromFleet)
							]

							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
							.Padding(Theme.SmallContentPadding)
							.AutoWidth()
							[
								// Inspect trade route
								SAssignNew(TradeRouteButton, SFlareButton)
								.Text(LOCTEXT("TradeRoute", "TRADE ROUTE"))
								.HelpText(this, &SFlareFleetMenu::GetInspectTradeRouteHintText)
								.IsDisabled(this, &SFlareFleetMenu::IsInspectTradeRouteDisabled)
								.OnClicked(this, &SFlareFleetMenu::OnOpenTradeRoute)
								.Width(4)
							]
							+ SHorizontalBox::Slot()
								.HAlign(HAlign_Left)
								.Padding(Theme.SmallContentPadding)
								.AutoWidth()
								[
									// Auto Trade Button
									SAssignNew(AutoTradeButton, SFlareButton)
									.Text(LOCTEXT("AutoTrade", "AUTO-TRADE"))
									.HelpText(this, &SFlareFleetMenu::GetAutoTradeHintText)
									.IsDisabled(this, &SFlareFleetMenu::IsAutoTradeDisabled)
									.OnClicked(this, &SFlareFleetMenu::OnToggleAutoTrade)
									.Toggle(true)
									.Width(4)
								]
							]
							// Inspect trade route

						// Hide Travel
						+SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
							.Padding(Theme.SmallContentPadding)
							.AutoWidth()
							[
								// Hide Travel Button
								SAssignNew(HideTravelButton, SFlareButton)
								.Text(LOCTEXT("HideTravel", "HIDE-TRAVEL"))
						
							.HelpText(this, &SFlareFleetMenu::GetToggleHideTravelHintText)
							.IsDisabled(this, &SFlareFleetMenu::IsToggleHideTravelDisabled)
							.OnClicked(this, &SFlareFleetMenu::OnToggleHideTravel)
							.Toggle(true)
							.Width(4)
							]
						]

						// Select White List
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(WhiteListSelectionBox, SBox)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Top)
									.Padding(Theme.ContentPadding)
									.MaxWidth(96)
									[
										SNew(STextBlock)
										.TextStyle(&Theme.TextFont)
										.Text(LOCTEXT("WhiteListInfo", "White List"))
									]
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.HAlign(HAlign_Left)
									[
										SNew(SBox)
										.HAlign(HAlign_Left)
										.VAlign(VAlign_Top)
										.Padding(FMargin(0))
										[
											SAssignNew(WhiteListDropBox, SFlareDropList<UFlareCompanyWhiteList*>)
											.OptionsSource(&WhiteListOptions)
											.OnGenerateWidget(this, &SFlareFleetMenu::OnGenerateWhiteListComboLine)
											.OnSelectionChanged(this, &SFlareFleetMenu::OnWhiteListComboLineSelectionChanged)
											.HeaderWidth(6)
											.ItemWidth(6)
										]
									]
									+ SHorizontalBox::Slot()
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Top)
									.AutoWidth()
									[
										SNew(SFlareButton)
										.Text(LOCTEXT("SelectWhitelist", "Select"))
										.Icon(FFlareStyleSet::GetIcon("OK"))
										.OnClicked(this, &SFlareFleetMenu::OnSelectWhiteList)
										.IsDisabled(this, &SFlareFleetMenu::IsWhiteListSelectDisabled)
										.Width(3)
									]
									+ SHorizontalBox::Slot()
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Top)
									.AutoWidth()
									[
										SNew(SFlareButton)
										.Transparent(true)
										.Text(FText())
										.HelpText(LOCTEXT("RemoveWhiteListHelp", "Remove this white list"))
										.Icon(FFlareStyleSet::GetIcon("Stop"))
										.OnClicked(this, &SFlareFleetMenu::OnRemoveWhiteList)
										.IsDisabled(this, &SFlareFleetMenu::IsWhiteListRemoveDisabled)
										.Width(1)
									]
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(SHorizontalBox)
							// Refill
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SAssignNew(RefillButton, SFlareButton)
								.HelpText(LOCTEXT("RefillInfoFleet", "Refill all ships in this fleet so that they have the necessary fuel, ammo and resources to fight."))
								.IsDisabled(this, &SFlareFleetMenu::IsRefillDisabled)
								.OnClicked(this, &SFlareFleetMenu::OnRefillClicked)
								.Text(this, &SFlareFleetMenu::GetRefillText)
								.Width(8)
							]

							// Repair
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SAssignNew(RepairButton, SFlareButton)
								.HelpText(LOCTEXT("RepairInfoFleet", "Repair all ships in this fleet."))
								.IsDisabled(this, &SFlareFleetMenu::IsRepairDisabled)
								.OnClicked(this, &SFlareFleetMenu::OnRepairClicked)
								.Text(this, &SFlareFleetMenu::GetRepairText)
								.Width(8)
							]
						]
					]
				]

				// Ship list
				+ SVerticalBox::Slot()
				[
					SNew(SScrollBox)
					.Style(&Theme.ScrollBoxStyle)
					.ScrollBarStyle(&Theme.ScrollBarStyle)

					+ SScrollBox::Slot()
					[
						SAssignNew(ShipList, SFlareList)
						.MenuManager(MenuManager)
						.ArrayMode(ESelectionMode::Single)
						.OnItemSelected(this, &SFlareFleetMenu::OnSpacecraftSelected)
						.OnItemUnSelected(this, &SFlareFleetMenu::OnSpacecraftUnSelected)
						.UseCompactDisplay(true)
					]
				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareFleetMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareFleetMenu::Enter(UFlareFleet* TargetFleet)
{
	FLOGV("SFlareFleetMenu::Enter : TargetFleet=%p", TargetFleet);

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	
	FleetToAdd = NULL;
	ShipToRemove = NULL;
	FleetToEdit = TargetFleet;
	WhiteListOptions.Empty();

	// We are in edit mode
	if (FleetToEdit)
	{
		FleetList->SetTitle(LOCTEXT("OtherFleetsListTitle", "Other Fleets"));
		ShipList->SetUseCompactDisplay(true);
		EditFleetName->SetText(FleetToEdit->GetFleetName());
		MenuManager->GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("fleet-edited").PutName("fleet", FleetToEdit->GetIdentifier()));
		AutoTradeButton->SetActive(FleetToEdit->IsAutoTrading());
		HideTravelButton->SetActive(FleetToEdit->IsHiddenTravel());

		WhiteListOptions.Reserve(FleetToEdit->GetFleetCompany()->GetWhiteLists().Num());
		for (UFlareCompanyWhiteList* WhiteListEntry : FleetToEdit->GetFleetCompany()->GetWhiteLists())
		{
			WhiteListOptions.Add(WhiteListEntry);
		}

		if (WhiteListOptions.Num() > 0)
		{
			WhiteListSelectionBox->SetVisibility(EVisibility::Visible);
		}
		else
		{
			WhiteListSelectionBox->SetVisibility(EVisibility::Collapsed);
		}

		WhiteListDropBox->RefreshOptions();

		if (WhiteListOptions.Num() > 0)
		{
			if (FleetToEdit->GetSelectedWhiteList())
			{
				WhiteListDropBox->SetSelectedItem(FleetToEdit->GetSelectedWhiteList());
			}
			else
			{
				WhiteListDropBox->SetSelectedIndex(0);
			}
		}
	}

	// We are in preview mode
	else
	{
		WhiteListSelectionBox->SetVisibility(EVisibility::Collapsed);
		FleetList->SetTitle(LOCTEXT("AllFleetsListTitle", "Fleets"));
		ShipList->SetUseCompactDisplay(false);
		AutoTradeButton->SetActive(false);
		HideTravelButton->SetActive(false);
	}

	UpdateFleetList();
	UpdateShipList(FleetToEdit);
}

void SFlareFleetMenu::Exit()
{
	SetEnabled(false);

	ShipList->Reset();
	FleetList->Reset();

	FleetToEdit = NULL;
	FleetToAdd = NULL;
	ShipToRemove = NULL;

	SetVisibility(EVisibility::Collapsed);
}

void SFlareFleetMenu::OnToggleShowFlags()
{
	UpdateFleetList();
}

void SFlareFleetMenu::UpdateFleetList(UFlareFleet* SelectedFleet)
{
	FleetList->Reset();
	FleetList->SetVisibility(EVisibility::Visible);

	int32 FleetCount = MenuManager->GetPC()->GetCompany()->GetCompanyFleets().Num();
	FLOGV("SFlareFleetMenu::UpdateFleetList : found %d fleets", FleetCount);
	
	for (int32 FleetIndex = 0; FleetIndex < FleetCount; FleetIndex++)
	{
		UFlareFleet* Fleet = MenuManager->GetPC()->GetCompany()->GetCompanyFleets()[FleetIndex];

		if (Fleet && Fleet->GetShips().Num() && FleetToEdit != Fleet)
		{
			FleetList->AddFleet(Fleet);
		}
	}
	FleetList->RefreshList();
}

void SFlareFleetMenu::UpdateShipList(UFlareFleet* Fleet)
{
	ShipList->Reset();

	if (Fleet)
	{
		int32 ShipCount = Fleet->GetShips().Num();
		FLOGV("SFlareFleetMenu::UpdateShipList : found %d ships", ShipCount);

		for (int32 SpacecraftIndex = 0; SpacecraftIndex < ShipCount; SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* ShipCandidate = Fleet->GetShips()[SpacecraftIndex];
			if (ShipCandidate && ShipCandidate->GetDamageSystem()->IsAlive())
			{
				ShipList->AddShip(ShipCandidate);
			}
		}

		ShipList->SetTitle(Fleet->GetFleetName());
	}
	else
	{
		ShipList->SetTitle(LOCTEXT("NoFleetSelectedTitle", "No fleet selected"));
	}

	ShipList->RefreshList();
}


/*----------------------------------------------------
	Content callbacks
----------------------------------------------------*/

FText SFlareFleetMenu::GetInspectTradeRouteHintText() const
{
	if (FleetToEdit && FleetToEdit->GetCurrentTradeRoute())
	{
		return LOCTEXT("TradeRouteInfo", "Edit the trade route this fleet is assigned to");
	}
	else
	{
		return LOCTEXT("CantEditNoTradeRoute", "Assign a trade route to this fleet in the company menu");
	}
}

bool SFlareFleetMenu::IsInspectTradeRouteDisabled() const
{
	if (FleetToEdit && FleetToEdit->GetCurrentTradeRoute())
	{
		return false;
	}
	else
	{
		return true;
	}
}

FText SFlareFleetMenu::GetAutoTradeHintText() const
{
	if (!FleetToEdit)
	{
		return FText();
	}
	if (!FleetToEdit->GetFleetCompany()->IsTechnologyUnlocked("auto-trade"))
	{
		return LOCTEXT("CantAutoTradeNoTech", "The Automated Trading technology is required");
	}
	else if (FleetToEdit->GetCurrentTradeRoute())
	{
		return LOCTEXT("CantAutoTradeOnTradeRoute", "Fleets assigned to a trade route can't do automatic trading");
	}
	else if (FleetToEdit == FleetToEdit->GetGame()->GetPC()->GetPlayerFleet())
	{
		return LOCTEXT("CantAutoTradeWithPlayerFleet", "Your personal fleet can't trade automatically");
	}
	else
	{
		return LOCTEXT("AutoTradeInfo", "Command this fleet to start automatic trading");
	}
}


bool SFlareFleetMenu::IsToggleHideTravelDisabled() const
{
	if (FleetToEdit == FleetToEdit->GetGame()->GetPC()->GetPlayerFleet())
	{
		return true;
	}
	return false;
}

FText SFlareFleetMenu::GetToggleHideTravelHintText() const
{
	if (!FleetToEdit)
	{
		return FText();
	}
	else if (FleetToEdit == FleetToEdit->GetGame()->GetPC()->GetPlayerFleet())
	{
		return LOCTEXT("CantHideTravelPlayerFleetInfo", "Your personal fleet can't be hidden from the Sector travel list");
	}
	else
	{
		return LOCTEXT("HideTravelInfo", "Hide this fleet from the Sector travel list");
	}
}


bool SFlareFleetMenu::IsAutoTradeDisabled() const
{
	if (!FleetToEdit)
	{
		return true;
	}
	if (!FleetToEdit->GetFleetCompany()->IsTechnologyUnlocked("auto-trade"))
	{
		return true;
	}
	else if (FleetToEdit->GetCurrentTradeRoute())
	{
		return true;
	}
	else if (FleetToEdit == FleetToEdit->GetGame()->GetPC()->GetPlayerFleet())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SFlareFleetMenu::OnOpenTradeRoute()
{
	if (FleetToEdit && FleetToEdit->GetGame()->GetPC() && FleetToEdit->GetCurrentTradeRoute())
	{
		FLOGV("SFlareFleetInfo::OnOpenTradeRoute : TargetFleet=%p", FleetToEdit);
		FFlareMenuParameterData Data;
		Data.Route = FleetToEdit->GetCurrentTradeRoute();
		FleetToEdit->GetGame()->GetPC()->GetMenuManager()->OpenMenu(EFlareMenu::MENU_TradeRoute, Data);
	}
}

void SFlareFleetMenu::OnToggleAutoTrade()
{
	if (FleetToEdit && FleetToEdit->GetGame()->GetPC())
	{
		FLOGV("SFlareFleetInfo::OnToggleAutoTrade : TargetFleet=%p", FleetToEdit);
		FleetToEdit->SetAutoTrading(!FleetToEdit->IsAutoTrading());
		AutoTradeButton->SetActive(FleetToEdit->IsAutoTrading());
	}
}

void SFlareFleetMenu::OnToggleHideTravel()
{
	if (FleetToEdit && FleetToEdit->GetGame()->GetPC())
	{
		FLOGV("SFlareFleetInfo::OnTogglHideTravel : TargetFleet=%p", FleetToEdit);
		FleetToEdit->SetHideTravel(!FleetToEdit->IsHiddenTravel());
		HideTravelButton->SetActive(FleetToEdit->IsHiddenTravel());
	}
}


FText SFlareFleetMenu::GetRepairText() const
{
	if (!FleetToEdit)
	{
		return FText();
	}

	UFlareSimulatedSector* TargetSector = FleetToEdit->GetCurrentSector();

	if (!TargetSector)
	{
		return FText();
	}

	int32 AvailableFS;
	int32 OwnedFS;
	int32 AffordableFS;
	int32 NeededFS;
	int32 TotalNeededFS;
	int64 MaxDuration;

	SectorHelper::GetRepairFleetSupplyNeeds(TargetSector, FleetToEdit->GetShips(), NeededFS, TotalNeededFS, MaxDuration, true);
	SectorHelper::GetAvailableFleetSupplyCount(TargetSector, FleetToEdit->GetFleetCompany(), OwnedFS, AvailableFS, AffordableFS, nullptr, FleetToEdit);

	if (IsRepairDisabled())
	{
		if (TotalNeededFS > 0)
		{
			// Repair needed
			if (TargetSector->IsInDangerousBattle(MenuManager->GetPC()->GetCompany()))
			{
				return LOCTEXT("CantRepairBattle", "Can't repair here : battle in progress!");
			}
			else if (AvailableFS == 0) {
				return LOCTEXT("CantRepairNoFS", "Can't repair here : no fleet supply available !");
			}
			else
			{
				return LOCTEXT("CantRepairNoMoney", "Can't repair here : not enough money !");
			}
		}
		else if (SectorHelper::HasShipRepairing(FleetToEdit->GetShips(), MenuManager->GetPC()->GetCompany()))
		{
			// Repair in progress
			return LOCTEXT("RepairInProgress", "Repair in progress...");
		}
		else
		{
			// No repair needed
			return LOCTEXT("NoShipToRepair", "No ship needs repairing");
		}
	}
	else
	{
		int32 UsedFs = FMath::Min(AffordableFS, TotalNeededFS);
		int32 UsedOwnedFs = FMath::Min(OwnedFS, UsedFs);
		int32 UsedNotOwnedFs = UsedFs - UsedOwnedFs;
		FFlareResourceDescription* FleetSupply = TargetSector->GetGame()->GetScenarioTools()->FleetSupply;

		int64 UsedNotOwnedFsCost = UsedNotOwnedFs * TargetSector->GetResourcePrice(FleetSupply, EFlareResourcePriceContext::MaintenanceConsumption);


		FText OwnedCostText;
		FText CostSeparatorText;
		FText NotOwnedCostText;

		if (UsedOwnedFs > 0)
		{
			OwnedCostText = FText::Format(LOCTEXT("RepairOwnedCostFormat", "{0} fleet supplies"), FText::AsNumber(UsedOwnedFs));
		}

		if (UsedNotOwnedFsCost > 0)
		{
			NotOwnedCostText = FText::Format(LOCTEXT("RepairNotOwnedCostFormat", "{0} credits"), FText::AsNumber(UFlareGameTools::DisplayMoney(UsedNotOwnedFsCost)));
		}

		if (UsedOwnedFs > 0 && UsedNotOwnedFsCost > 0)
		{
			CostSeparatorText = UFlareGameTools::AddLeadingSpace(LOCTEXT("CostSeparatorText", "+ "));
		}

		FText CostText = FText::Format(LOCTEXT("RepairCostFormat", "{0}{1}{2}"), OwnedCostText, CostSeparatorText, NotOwnedCostText);

		return FText::Format(LOCTEXT("RepairOkayFormat", "Repair all ships ({0}, {1} days)"),
			CostText,
			FText::AsNumber(MaxDuration));
	}
}

bool SFlareFleetMenu::IsRepairDisabled() const
{
	if (!FleetToEdit)
	{
		return true;
	}

	UFlareSimulatedSector* TargetSector = FleetToEdit->GetCurrentSector();
	if (!TargetSector || TargetSector->IsInDangerousBattle(MenuManager->GetPC()->GetCompany()))
	{
		return true;
	}

	int32 NeededFS;
	int32 TotalNeededFS;
	int64 MaxDuration;

	SectorHelper::GetRepairFleetSupplyNeeds(TargetSector, FleetToEdit->GetShips(), NeededFS, TotalNeededFS, MaxDuration, false);

	if (TotalNeededFS > 0)
	{
		// Repair needed

		int32 AvailableFS;
		int32 OwnedFS;
		int32 AffordableFS;

		SectorHelper::GetAvailableFleetSupplyCount(TargetSector, MenuManager->GetPC()->GetCompany(), OwnedFS, AvailableFS, AffordableFS,nullptr, FleetToEdit);

		if (AffordableFS == 0) {
			return true;
		}

		// There is somme affordable FS, can repair
		return false;
	}
	else
	{
		// No repair needed
		return true;
	}
}

FText SFlareFleetMenu::GetRefillText() const
{
	if (!FleetToEdit)
	{
		return FText();
	}

	UFlareSimulatedSector* TargetSector = FleetToEdit->GetCurrentSector();
	if (!TargetSector)
	{
		return FText();
	}

	int32 AvailableFS;
	int32 OwnedFS;
	int32 AffordableFS;
	int32 NeededFS;
	int32 TotalNeededFS;
	int64 MaxDuration;

	SectorHelper::GetRefillFleetSupplyNeeds(TargetSector, FleetToEdit->GetShips(), NeededFS, TotalNeededFS, MaxDuration, true);
	SectorHelper::GetAvailableFleetSupplyCount(TargetSector, FleetToEdit->GetFleetCompany(), OwnedFS, AvailableFS, AffordableFS, nullptr, FleetToEdit);

	if (IsRefillDisabled())
	{
		if (TotalNeededFS > 0)
		{
			// Refill needed
			if (TargetSector->IsInDangerousBattle(MenuManager->GetPC()->GetCompany()))
			{
				return LOCTEXT("CantRefillBattle", "Can't refill : battle in progress!");
			}
			else if (AvailableFS == 0) {
				return LOCTEXT("CantRefillNoFS", "Can't refill : no fleet supply available !");
			}
			else
			{
				return LOCTEXT("CantRefillNoMoney", "Can't refill : not enough money !");
			}
		}
		else if (SectorHelper::HasShipRefilling(FleetToEdit->GetShips(), MenuManager->GetPC()->GetCompany()))
		{

			// Refill in progress
			return LOCTEXT("RefillInProgress", "Refill in progress...");
		}
		else
		{
			// No refill needed
			return LOCTEXT("NoShipToRefill", "No ship needs refilling");
		}
	}
	else
	{
		int32 UsedFs = FMath::Min(AffordableFS, TotalNeededFS);
		int32 UsedOwnedFs = FMath::Min(OwnedFS, UsedFs);
		int32 UsedNotOwnedFs = UsedFs - UsedOwnedFs;
		FFlareResourceDescription* FleetSupply = TargetSector->GetGame()->GetScenarioTools()->FleetSupply;

		int64 UsedNotOwnedFsCost = UsedNotOwnedFs * TargetSector->GetResourcePrice(FleetSupply, EFlareResourcePriceContext::MaintenanceConsumption);

		FText OwnedCostText;
		FText CostSeparatorText;
		FText NotOwnedCostText;

		if (UsedOwnedFs > 0)
		{
			OwnedCostText = FText::Format(LOCTEXT("RefillOwnedCostFormat", "{0} fleet supplies"), FText::AsNumber(UsedOwnedFs));
		}

		if (UsedNotOwnedFsCost > 0)
		{
			NotOwnedCostText = FText::Format(LOCTEXT("RefillNotOwnedCostFormat", "{0} credits"), FText::AsNumber(UFlareGameTools::DisplayMoney(UsedNotOwnedFsCost)));
		}

		if (UsedOwnedFs > 0 && UsedNotOwnedFsCost > 0)
		{
			CostSeparatorText = UFlareGameTools::AddLeadingSpace(LOCTEXT("CostSeparatorText", "+ "));
		}

		FText CostText = FText::Format(LOCTEXT("RefillCostFormat", "{0}{1}{2}"), OwnedCostText, CostSeparatorText, NotOwnedCostText);

		return FText::Format(LOCTEXT("RefillOkayFormat", "Refill ({0}, {1} days)"),
			CostText,
			FText::AsNumber(MaxDuration));
	}
}

bool SFlareFleetMenu::IsRefillDisabled() const
{
	if (!FleetToEdit)
	{
		return true;
	}

	UFlareSimulatedSector* TargetSector = FleetToEdit->GetCurrentSector();

	if (!TargetSector || TargetSector->IsInDangerousBattle(MenuManager->GetPC()->GetCompany()))
	{
		return true;
	}

	int32 NeededFS;
	int32 TotalNeededFS;
	int64 MaxDuration;

	SectorHelper::GetRefillFleetSupplyNeeds(TargetSector, FleetToEdit->GetShips(), NeededFS, TotalNeededFS, MaxDuration, false);

	if (TotalNeededFS > 0)
	{
		// Refill needed

		int32 AvailableFS;
		int32 OwnedFS;
		int32 AffordableFS;

		SectorHelper::GetAvailableFleetSupplyCount(TargetSector, FleetToEdit->GetFleetCompany(), OwnedFS, AvailableFS, AffordableFS, nullptr, FleetToEdit);

		if (AffordableFS == 0) {
			return true;
		}

		// There is somme affordable FS, can refill
		return false;
	}
	else
	{
		// No refill needed
		return true;
	}
}

void SFlareFleetMenu::OnRefillClicked()
{
	if (FleetToEdit)
	{
		SectorHelper::RefillFleets(FleetToEdit->GetCurrentSector(), FleetToEdit->GetFleetCompany(), FleetToEdit);
	}
}

void SFlareFleetMenu::OnRepairClicked()
{
	if (FleetToEdit)
	{
		SectorHelper::RepairFleets(FleetToEdit->GetCurrentSector(), FleetToEdit->GetFleetCompany(), FleetToEdit);
	}
}


EVisibility SFlareFleetMenu::GetEditVisibility() const
{
	return FleetToEdit ? EVisibility::Visible : EVisibility::Collapsed;
}

bool SFlareFleetMenu::IsAddDisabled() const
{
	FText Unused;
	if (!FleetToAdd || !FleetToEdit || FleetToAdd == FleetToEdit || (FleetToEdit && !FleetToEdit->CanMerge(FleetToAdd, Unused)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

FText SFlareFleetMenu::GetAddHintText() const
{
	if (FleetToAdd && FleetToEdit)
	{
		FText Reason;
		if (FleetToEdit->CanMerge(FleetToAdd, Reason))
		{
			return LOCTEXT("AddToFleetInfo", "Merge selected fleet with this one");
		}
		else
		{
			return Reason;
		}
	}
	else
	{
		return LOCTEXT("NoFleetToEditToAddToInfo", "No fleet selected for merging");
	}
}

bool SFlareFleetMenu::IsRemoveDisabled() const
{
	if (ShipToRemove == NULL
		|| ShipToRemove == MenuManager->GetPC()->GetPlayerShip()
		|| FleetToEdit == NULL)
	{
		return true;
	}
	else if (ShipToRemove && FleetToAdd && FleetToAdd->CanAddShip(ShipToRemove))
	{
		return false;
	}
	else if (FleetToEdit->GetShips().Num() <= 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

FText SFlareFleetMenu::GetRemoveHintText() const
{
	if (ShipToRemove)
	{
		if(ShipToRemove == MenuManager->GetPC()->GetPlayerShip())
		{
			return LOCTEXT("CantRemovePlayerShipFromFleetInfo", "Can't remove the ship you are currenly flying");
		}
		else if (FleetToEdit->GetShips().Num() > 1 || (FleetToAdd && FleetToAdd->CanAddShip(ShipToRemove)))
		{
			return LOCTEXT("RemoveFromFleetInfo", "Remove the selected ship from the fleet");
		}
		else
		{
			return LOCTEXT("CantRemoveFromFleetInfo", "Can't remove the only ship in the fleet");
		}
	}
	else
	{
		return LOCTEXT("NoFleetToEditToRemoveFromInfo", "No ship selected for removal");
	}
}

bool SFlareFleetMenu::IsRenameDisabled() const
{
	if (!FleetToEdit)
	{
		return true;
	}
	else
	{
		if (EditFleetName->GetText().ToString() == FleetToEdit->GetFleetName().ToString())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

FText SFlareFleetMenu::GetRenameHintText() const
{
	if (FleetToEdit)
	{
		if (EditFleetName->GetText().ToString() == FleetToEdit->GetFleetName().ToString())
		{
			return LOCTEXT("NoSameRenameInfo", "Enter a new name for this fleet in order to rename it");
		}
		else
		{
			return LOCTEXT("FleetRenameInfo", "Rename this fleet");
		}
	}
	else
	{
		return LOCTEXT("NoFleetToRenameInfo", "No fleet is selected for renaming");
	}
}

float SFlareFleetMenu::GetColorSpinBoxValue() const
{
	return FleetToEdit->GetFleetColor().LinearRGBToHSV().R / 360.0f;
}


/*----------------------------------------------------
	Action callbacks
----------------------------------------------------*/

void SFlareFleetMenu::OnSpacecraftSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer)
{
	UFlareSimulatedSpacecraft* Spacecraft = SpacecraftContainer->SpacecraftPtr;
	if (Spacecraft)
	{
		ShipToRemove = Spacecraft;
		if (FleetToEdit)
		{
			if (FleetToAdd && FleetToAdd->CanAddShip(ShipToRemove))
			{
				RemoveShipButton->SetText(LOCTEXT("RemoveFromFleetAdd", "Remove ship & add"));
			}
			else
			{
				RemoveShipButton->SetText(LOCTEXT("RemoveFromFleet", "Remove ship"));
			}
		}
		else
		{
			RemoveShipButton->SetText(LOCTEXT("RemoveFromFleet", "Remove ship"));
		}
		FLOGV("SFlareFleetMenu::OnSpacecraftSelected : ship to remove '%s'", *Spacecraft->GetImmatriculation().ToString());
	}
}

void SFlareFleetMenu::OnSpacecraftUnSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer)
{
	if (SpacecraftContainer)
	{
		UFlareSimulatedSpacecraft* Spacecraft = SpacecraftContainer->SpacecraftPtr;
		if (Spacecraft)
		{
			if (ShipToRemove == Spacecraft)
			{
				ShipToRemove = nullptr;
			}
		}
	}
}


void SFlareFleetMenu::OnFleetUnSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer)
{
	RemoveShipButton->SetText(LOCTEXT("RemoveFromFleet", "Remove ship"));
	if (FleetToEdit)
	{
		FleetToAdd = nullptr;
	}
}

void SFlareFleetMenu::OnFleetSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer)
{
	UFlareFleet* Fleet = SpacecraftContainer->FleetPtr;
	if (Fleet)
	{
		// Fleet editing
		if (FleetToEdit)
		{
			FleetToAdd = Fleet;

//			if (FleetToEdit->GetCurrentSector() == FleetToAdd->GetCurrentSector())
			if (ShipToRemove && FleetToAdd->CanAddShip(ShipToRemove))
			{
				RemoveShipButton->SetText(LOCTEXT("RemoveFromFleetAdd", "Remove ship & add"));
			}
			else
			{
				RemoveShipButton->SetText(LOCTEXT("RemoveFromFleet", "Remove ship"));
			}

			FLOGV("SFlareFleetMenu::OnFleetSelected : fleet to add/edit '%s'", *Fleet->GetFleetName().ToString());
		}

		// Simple preview : list ships
		else
		{
			UpdateShipList(Fleet);
		}

		MenuManager->GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("fleet-selected").PutName("fleet", Fleet->GetIdentifier()));
	}
}

void SFlareFleetMenu::OnEditFinished()
{
	MenuManager->OpenMenu(EFlareMenu::MENU_Fleet);
}

void SFlareFleetMenu::OnAddToFleet()
{
	FCHECK(FleetToEdit);
	FCHECK(FleetToAdd);

	FLOGV("SFlareFleetMenu::OnAddToFleet : adding '%s'", *FleetToAdd->GetFleetName().ToString());
	FleetToEdit->Merge(FleetToAdd);

	UpdateShipList(FleetToEdit);
	UpdateFleetList();
	FleetToAdd = NULL;
	ShipToRemove = NULL;
}

void SFlareFleetMenu::OnRemoveFromFleet()
{
	FCHECK(FleetToEdit);
	FCHECK(ShipToRemove);

	FLOGV("SFlareFleetMenu::OnRemoveFromFleet : removing '%s'", *ShipToRemove->GetImmatriculation().ToString());
	bool FinishedEdit = false;

	if (FleetToAdd && FleetToAdd->CanAddShip(ShipToRemove))
	{
		int32 ShipQuantity = ShipToRemove->GetCurrentFleet()->GetShips().Num();
		FleetToEdit->RemoveShip(ShipToRemove,false,false);
		FleetToAdd->AddShip(ShipToRemove);

		if (ShipQuantity <= 1)
		{
			FinishedEdit = true;
			SetEnabled(false);
			OnEditFinished();
		}
	}
	else
	{
		FleetToEdit->RemoveShip(ShipToRemove);
		UpdateFleetList();
		FleetToAdd = NULL;
	}

		//	ShipList

	if (!FinishedEdit)
	{
		UpdateShipList(FleetToEdit);
		ShipToRemove = NULL;
	}
}

void SFlareFleetMenu::OnRenameFleet()
{
	FCHECK(FleetToEdit);

	FText NewText = EditFleetName->GetText();
	FLOGV("SFlareFleetMenu::OnRenameFleet : renaming as '%s'", *NewText.ToString());

	FleetToEdit->SetFleetName(NewText);
}

void SFlareFleetMenu::SFlareFleetMenu::OnColorSpinBoxValueChanged(float NewValue)
{
	FLinearColor Color = FLinearColor(360.0f * NewValue, 1.0f, 1.0f, 1.0f).HSVToLinearRGB();
	FleetToEdit->SetFleetColor(Color);
}

void SFlareFleetMenu::OnSelectWhiteList()
{
	if (CurrentlySelectedWhiteList)
	{
		FleetToEdit->SelectWhiteListDefault(CurrentlySelectedWhiteList);
		int32 CurrentlySelectedIndex = WhiteListDropBox->GetSelectedIndex();
		WhiteListDropBox->RefreshOptions();
		WhiteListDropBox->SetSelectedIndex(CurrentlySelectedIndex);
	}
}

void SFlareFleetMenu::OnRemoveWhiteList()
{
	FleetToEdit->SelectWhiteListDefault(nullptr);
	int32 CurrentlySelectedIndex = WhiteListDropBox->GetSelectedIndex();
	WhiteListDropBox->RefreshOptions();
	WhiteListDropBox->SetSelectedIndex(CurrentlySelectedIndex);
}

bool SFlareFleetMenu::IsWhiteListRemoveDisabled() const
{
	if (FleetToEdit && FleetToEdit->GetSelectedWhiteList())
	{
		return false;
	}
	return true;
}

bool SFlareFleetMenu::IsWhiteListSelectDisabled() const
{
	if (FleetToEdit && (!FleetToEdit->GetSelectedWhiteList() || FleetToEdit->GetSelectedWhiteList() && FleetToEdit->GetSelectedWhiteList() != CurrentlySelectedWhiteList))
	{
		return false;
	}

	return true;
}

TSharedRef<SWidget> SFlareFleetMenu::OnGenerateWhiteListComboLine(UFlareCompanyWhiteList* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	const FSlateBrush* WhiteListIcon = NULL;
	if (FleetToEdit->GetSelectedWhiteList() == Item)
	{
		WhiteListIcon = FFlareStyleSet::GetIcon("New");
	}

	TSharedPtr<SWidget> Layout = SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.MaxWidth(32)
			[
				SNew(SImage)
				.Image(WhiteListIcon)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(Item ? Item->GetWhiteListName() : LOCTEXT("NoFilterByWhiteList", "No filter"))
				.TextStyle(&Theme.TextFont)
			]
	];
	return Layout.ToSharedRef();
}

void SFlareFleetMenu::OnWhiteListComboLineSelectionChanged(UFlareCompanyWhiteList* Item, ESelectInfo::Type SelectInfo)
{
	CurrentlySelectedWhiteList = Item;
}

#undef LOCTEXT_NAMESPACE