
#include "FlareTradeMenu.h"

#include "../../Flare.h"

#include "../../Data/FlareResourceCatalog.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlareSectorHelper.h"
#include "../../Game/FlareGameTools.h"
#include "../../Player/FlareHUD.h"

#include "../../Economy/FlareCargoBay.h"

#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#include "../Components/FlareRoundButton.h"
#include "../Components/FlareCargoInfo.h"
#include "../../Quests/FlareQuest.h"
#include "../../Quests/FlareQuestStep.h"
#include "../../Quests/FlareQuestCondition.h"

#define LOCTEXT_NAMESPACE "FlareTradeMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareTradeMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 TextWidth = Theme.ContentWidth - 2 * Theme.ContentPadding.Left - 2 * Theme.ContentPadding.Right;

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
		.VAlign(VAlign_Fill)
		[
			SNew(SHorizontalBox)

			// Left spacecraft aka the current ship
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			[
				SNew(SScrollBox)
				.Style(&Theme.ScrollBoxStyle)
				.ScrollBarStyle(&Theme.ScrollBarStyle)

				+ SScrollBox::Slot()
				[
					SNew(SBox)
					.HAlign(HAlign_Left)
					.WidthOverride(Theme.ContentWidth)
					[
						SNew(SVerticalBox)
						// Left ship selection list
						+ SVerticalBox::Slot()
						[
							SAssignNew(LeftShipList, SFlareList)
							.MenuManager(MenuManager)
							.DisableFilters(true)
							.Title(LOCTEXT("SelectSpacecraft", "Select a spacecraft to trade with"))
							.OnItemSelected(this, &SFlareTradeMenu::OnSpacecraftSelectedLeft)
						]

						// Current ship's name
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(this, &SFlareTradeMenu::GetLeftSpacecraftName)
						]


						// Current ship's cargo 1
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						.HAlign(HAlign_Left)
						[
							SAssignNew(LeftCargoBay1, SHorizontalBox)
						]

						// Current ship's cargo 2
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						.HAlign(HAlign_Left)
						[
							SAssignNew(LeftCargoBay2, SHorizontalBox)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						.HAlign(HAlign_Left)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Fill)

							// Change source (left side) button
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.SmallContentPadding)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("BackToSelectionSource", "SOURCE"))
								.HelpText(LOCTEXT("BackToSelectionSourceInfo", "Change which ship is trading"))
								.Icon(FFlareStyleSet::GetIcon("Stop"))
								.OnClicked(this, &SFlareTradeMenu::OnBackToSelectionLeft)
								.Visibility(this, &SFlareTradeMenu::GetBackToSelectionLeftVisibility)
								.Width(3)
							]

							// Undock button
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.SmallContentPadding)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("Undock", "UNDOCK"))
								.HelpText(LOCTEXT("ShipUndockInfo", "Undock the ship and leave the station"))
								.HotkeyText(LOCTEXT("SpacecraftKey5", "M5"))
								.OnClicked(this, &SFlareTradeMenu::OnUndock)
								.Visibility(this, &SFlareTradeMenu::GetUndockVisibility)
								.Width(3)
							]
							// Change the right side selection to the docked station for the left selected ship
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.SmallContentPadding)
							.HAlign(HAlign_Left)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("SelectDockedStation", "SELECT DOCKED"))
								.HelpText(LOCTEXT("SelectDockedStationInfo", "Change the trade target to the currently docked station"))
								.OnClicked(this, &SFlareTradeMenu::OnSelectDockedSelection)
								.Visibility(this, &SFlareTradeMenu::GetSelectDockedVisibility)
								.Width(3)
							]
						]
						// Help text
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(this, &SFlareTradeMenu::GetShipTradeDetails)
							.WrapTextAt(TextWidth)
						]
					]
				]
			]

			// Right spacecraft aka the ship we're going to trade with
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			[
				SNew(SBox)
				.HAlign(HAlign_Fill)
				.WidthOverride(Theme.ContentWidth)
				[
					SNew(SScrollBox)
					.Style(&Theme.ScrollBoxStyle)
					.ScrollBarStyle(&Theme.ScrollBarStyle)

					+ SScrollBox::Slot()
					[
						SNew(SVerticalBox)
						
						// Right ship selection list
						+ SVerticalBox::Slot()
						[
							SAssignNew(RightShipList, SFlareList)
							.MenuManager(MenuManager)
							.StationList(true)
							.Title(LOCTEXT("SelectStation", "Select a station to trade with"))
							.OnItemSelected(this, &SFlareTradeMenu::OnSpacecraftSelectedRight)
						]

						// Ship's name
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(this, &SFlareTradeMenu::GetRightSpacecraftName)
							.Visibility(this, &SFlareTradeMenu::GetTradingVisibility)
							.WrapTextAt(TextWidth)
						]

						// Company line
						+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.SmallContentPadding)
							[
								SNew(SHorizontalBox)

								// Company flag
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SAssignNew(CompanyFlag, SFlareCompanyFlag)
								.Player(MenuManager->GetPC())
								.Visibility(this, &SFlareTradeMenu::GetCompanyFlagVisibility)
							]
						// Spacecraft info
						+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.SmallContentPadding)
							[
								SNew(STextBlock)
								.Text(this, &SFlareTradeMenu::GetRightcraftInfo)
							.TextStyle(&Theme.TextFont)
							.Visibility(this, &SFlareTradeMenu::GetCompanyFlagVisibility)
							]
						]

						// Ship's cargo
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Fill)
						[

							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()

							[
								SNew(SVerticalBox)

								// Current ship's cargo 1
								+ SVerticalBox::Slot()
								.Padding(Theme.SmallContentPadding)
								.AutoHeight()
								.HAlign(HAlign_Left)
								[
									SAssignNew(RightCargoBay1, SHorizontalBox)
									.Visibility(this, &SFlareTradeMenu::GetTradingVisibility)
								]

								// Current ship's cargo 2
								+ SVerticalBox::Slot()
								.Padding(Theme.SmallContentPadding)
								.AutoHeight()
								.HAlign(HAlign_Left)
								[
									SAssignNew(RightCargoBay2, SHorizontalBox)
									.Visibility(this, &SFlareTradeMenu::GetTradingVisibility)
								]
							]

							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Right)
							.VAlign(VAlign_Center)
							.Padding(Theme.SmallContentPadding)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("BackToSelection", "Change target"))
								.HelpText(LOCTEXT("ChangeTargetInfo", "Change the currently selected trade target"))						
								.Icon(FFlareStyleSet::GetIcon("Stop"))
								.OnClicked(this, &SFlareTradeMenu::OnBackToSelectionRight)
								.Visibility(this, &SFlareTradeMenu::GetBackToSelectionRightVisibility)
								.Width(4)
							]
						]

						// Construction text
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(LOCTEXT("ConstructionInfo", "This station is under construction and needs resources to be completed."))
							.Visibility(this, &SFlareTradeMenu::GetConstructionInfosVisibility)
							.WrapTextAt(TextWidth)
						]

						// Help text
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(LOCTEXT("OtherShipHelpText", "Click on a resource to start trading."))
							.Visibility(this, &SFlareTradeMenu::GetTradingVisibility)
							.WrapTextAt(TextWidth)
						]

						// Price box
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SBox)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Top)
							.WidthOverride(Theme.ContentWidth)
							[
								SNew(SVerticalBox)

								// Title
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(Theme.TitlePadding)
								[
									SNew(STextBlock)
									.TextStyle(&Theme.SubTitleFont)
									.Text(LOCTEXT("TransactionTitle", "Transaction details"))
									.Visibility(this, &SFlareTradeMenu::GetTransactionDetailsVisibility)
									.WrapTextAt(TextWidth)
								]
								// Donation Toggle
								+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(Theme.ContentPadding)
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Center)
									[
										SAssignNew(DonationButton, SFlareButton)
										.Text(LOCTEXT("Donation", "Donate"))
									.HelpText(LOCTEXT("DonationInfo", "Donate Resources"))
									.Toggle(true)
									.OnClicked(this, &SFlareTradeMenu::OnDonationToggle)
									.Visibility(this, &SFlareTradeMenu::GetDonationButtonVisibility)
									.Width(4)
									]
								// Invalid transaction
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(Theme.ContentPadding)
								[
									SAssignNew(InvalidTransaction, STextBlock)
									.TextStyle(&Theme.NameFont)
									.Visibility(this, &SFlareTradeMenu::GetTransactionInvalidVisibility)
									.WrapTextAt(TextWidth)
								]

								// Quantity
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SHorizontalBox)

									// Slider
									+ SHorizontalBox::Slot()
									.HAlign(HAlign_Fill)
									.Padding(Theme.ContentPadding)
									[
										SAssignNew(QuantitySlider, SSlider)
										.Style(&Theme.SliderStyle)
										.Value(0)
										.OnValueChanged(this, &SFlareTradeMenu::OnResourceQuantityChanged)
										.Visibility(this, &SFlareTradeMenu::GetTransactionDetailsVisibility)
									]

									// Text box
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.HAlign(HAlign_Right)
									.Padding(Theme.ContentPadding)
									[
										SAssignNew(QuantityText, SEditableText)
										.AllowContextMenu(false)
										.Style(&Theme.TextInputStyle)
										.OnTextChanged(this, &SFlareTradeMenu::OnResourceQuantityEntered)
										.Visibility(this, &SFlareTradeMenu::GetTransactionDetailsVisibility)
									]
								]

								// Info
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(Theme.ContentPadding)
								[
									SNew(STextBlock)
									.TextStyle(&Theme.TextFont)
									.Text(this, &SFlareTradeMenu::GetTransactionDetails)
									.Visibility(this, &SFlareTradeMenu::GetTransactionDetailsVisibility)
									.WrapTextAt(TextWidth)
								]
								// Price
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SAssignNew(PriceBox, SFlareConfirmationBox)
									.ConfirmText(LOCTEXT("Confirm", "Confirm transfer"))
									.CancelText(LOCTEXT("BackTopShip", "Cancel"))
									.OnConfirmed(this, &SFlareTradeMenu::OnConfirmTransaction)
									.OnCancelled(this, &SFlareTradeMenu::OnCancelTransaction)
									.TradeBehavior(true)
									.PC(MenuManager->GetPC())
								]
							]
						]
					]
				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareTradeMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
	LeftShipList->SetVisibility(EVisibility::Collapsed);
	RightShipList->SetVisibility(EVisibility::Collapsed);
}

void SFlareTradeMenu::Enter(UFlareSimulatedSector* ParentSector, UFlareSimulatedSpacecraft* LeftSpacecraft, UFlareSimulatedSpacecraft* RightSpacecraft)
{
	FLOGV("SFlareTradeMenu::Enter ParentSector=%p LeftSpacecraft=%p RightSpacecraft=%p", ParentSector, LeftSpacecraft, RightSpacecraft);
	
	// Setup
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	SetTransactionInvalidDetails();
	TargetSector = ParentSector;
	TargetLeftSpacecraft = LeftSpacecraft;
	RightShipList->Reset();
	WasActiveSector = false;

	// First-person trading override
	if (TargetLeftSpacecraft)
	{
		AFlareSpacecraft* PhysicalSpacecraft = TargetLeftSpacecraft->GetActive();
		if (TargetLeftSpacecraft->IsActive())
		{
			WasActiveSector = true;
			if (PhysicalSpacecraft->GetNavigationSystem()->IsDocked())
			{
				TargetRightSpacecraft = PhysicalSpacecraft->GetNavigationSystem()->GetDockStation()->GetParent();
			}
			else
			{
				TargetRightSpacecraft = RightSpacecraft;
			}
		}
		else
		{
			TargetRightSpacecraft = RightSpacecraft;
		}
	}
	else
	{
		TargetRightSpacecraft = RightSpacecraft;
		if (TargetRightSpacecraft->IsActive())
		{
			WasActiveSector = true;
		}
	}

	// Not first person - list spacecrafts
	//	if (TargetLeftSpacecraft->GetCurrentFleet() != MenuManager->GetPC()->GetPlayerFleet())
//	if (TargetLeftSpacecraft != MenuManager->GetPC()->GetPlayerShip())

	UpdateLeftShips();

// Add stations (right side)
	for (int32 SpacecraftIndex = 0; SpacecraftIndex < ParentSector->GetSectorStations().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* StationCandidate = ParentSector->GetSectorStations()[SpacecraftIndex];
		if (StationCandidate && StationCandidate != LeftSpacecraft && StationCandidate != RightSpacecraft
			&& StationCandidate->GetActiveCargoBay()->GetSlotCount() > 0
			&& !StationCandidate->IsPlayerHostile())
		{
			RightShipList->AddShip(StationCandidate);
		}
	}

	// Setup widgets
	AFlarePlayerController* PC = MenuManager->GetPC();

	PreviousTradeDirection = 0;
	FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay1, LeftCargoBay2);
	FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay1, RightCargoBay2);
	RightShipList->RefreshList();

	// Show selector if still needed
	if (TargetRightSpacecraft)
	{
		RightShipList->SetVisibility(EVisibility::Collapsed);
		CompanyFlag->SetCompany(TargetRightSpacecraft->GetCompany());
	}
	else
	{
		RightShipList->SetVisibility(EVisibility::Visible);
	}

	if (TargetLeftSpacecraft)
	{
		LeftShipList->SetVisibility(EVisibility::Collapsed);
	}
	else
	{
		LeftShipList->SetVisibility(EVisibility::Visible);
	}

	DonationButton->SetActive(false);
}


void SFlareTradeMenu::FillTradeBlock(UFlareSimulatedSpacecraft* TargetSpacecraft, UFlareSimulatedSpacecraft* OtherSpacecraft,
	TSharedPtr<SHorizontalBox> CargoBay1, TSharedPtr<SHorizontalBox> CargoBay2)
{

	if (CargoBay1 && CargoBay2)
	{
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
		// Prepare cargo bay
		CargoBay1->ClearChildren();
		CargoBay2->ClearChildren();
		TArray<FSortableCargoInfo> SortedCargoBay;

		// Both spacecrafts are set
		if (TargetSpacecraft)
		{
			// Get slots
			for (int32 CargoIndex = 0; CargoIndex < TargetSpacecraft->GetActiveCargoBay()->GetSlotCount(); CargoIndex++)
			{
				FFlareCargo* Cargo = TargetSpacecraft->GetActiveCargoBay()->GetSlot(CargoIndex);
				FSortableCargoInfo CargoInfo;
				CargoInfo.Cargo = Cargo;
				CargoInfo.CargoInitialIndex = CargoIndex;
				SortedCargoBay.Add(CargoInfo);
			}

			// Sort and fill
			SortedCargoBay.Sort(UFlareCargoBay::SortBySlotType);
			for (int32 CargoIndex = 0; CargoIndex < SortedCargoBay.Num(); CargoIndex++)
			{
				TSharedPtr<SHorizontalBox> Bay = (CargoIndex < 8) ? CargoBay1 : CargoBay2;
				Bay->AddSlot()
					[
						SNew(SFlareCargoInfo)
						.Spacecraft(TargetSpacecraft)
						.CargoIndex(SortedCargoBay[CargoIndex].CargoInitialIndex)
						.OnClicked(this, &SFlareTradeMenu::OnTransferResources,
						TargetSpacecraft,
						OtherSpacecraft,
						SortedCargoBay[CargoIndex].Cargo->Resource)
					];
			}
		}
	}
}

void SFlareTradeMenu::Exit()
{
	SetEnabled(false);

	// Reset cargo
	PreviousTradeDirection = -1;
//	TargetLeftSpacecraft = NULL;
//	TargetRightSpacecraft = NULL;
	LeftCargoBay1->ClearChildren();
	LeftCargoBay2->ClearChildren();
	RightCargoBay1->ClearChildren();
	RightCargoBay2->ClearChildren();

	// Reset transaction data
	TransactionDestinationSpacecraft = NULL;
	TransactionSourceSpacecraft = NULL;
	TransactionResource = NULL;

	// Reset menus
	PriceBox->Hide();
	LeftShipList->Reset();
	LeftShipList->SetVisibility(EVisibility::Collapsed);
	RightShipList->Reset();
	RightShipList->SetVisibility(EVisibility::Collapsed);
	SetVisibility(EVisibility::Collapsed);
}
/*
void SFlareTradeMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	if (IsEnabled() && !WasActiveSector)
	{
		if (TargetLeftSpacecraft && TargetLeftSpacecraft->IsActive())
		{
			Enter(TargetSector, TargetLeftSpacecraft, TargetRightSpacecraft);
		}
	}
}
*/
/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

EVisibility SFlareTradeMenu::GetCompanyFlagVisibility() const
{

	// Crash mitigation - If parent is hidden, so are we, don't try to use the target (#178)
//	if ((OwnerWidget.IsValid() && OwnerWidget->GetVisibility() != EVisibility::Visible)
	if (MenuManager->GetPC() && !MenuManager->GetPC()->GetMenuManager()->IsUIOpen())
	{
		return EVisibility::Collapsed;
	}

// Check the target
	if (TargetRightSpacecraft && TargetRightSpacecraft->IsValidLowLevel())
	{
		UFlareCompany* TargetCompany = TargetRightSpacecraft->GetCompany();
		if (TargetCompany && MenuManager->GetPC() && TargetCompany == MenuManager->GetPC()->GetCompany())
		{
			return EVisibility::Collapsed;
		}
	}
	else
	{
		return EVisibility::Collapsed;
	}

	return EVisibility::Visible;
}

EVisibility SFlareTradeMenu::GetTradingVisibility() const
{
	return (TargetRightSpacecraft) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlareTradeMenu::GetConstructionInfosVisibility() const
{
	return (TargetRightSpacecraft && TargetRightSpacecraft->IsStation() && TargetRightSpacecraft->IsUnderConstruction()) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlareTradeMenu::GetSelectDockedVisibility() const
{
	if (TargetLeftSpacecraft && TargetLeftSpacecraft->IsActive() && TargetLeftSpacecraft->GetActive()->GetNavigationSystem()->IsDocked())
	{
		if ((TargetRightSpacecraft && TargetRightSpacecraft->IsActive() && TargetLeftSpacecraft->GetActive()->GetNavigationSystem()->GetDockStation() != TargetRightSpacecraft->GetActive()) || RightShipList->GetVisibility() == EVisibility::Visible)
		{
			return EVisibility::Visible;
		}
	}

	return EVisibility::Collapsed;
}

EVisibility SFlareTradeMenu::GetUndockVisibility() const
{
	if (TargetLeftSpacecraft&&TargetLeftSpacecraft->IsActive() && TargetLeftSpacecraft->GetActive()->GetNavigationSystem()->IsDocked())// && MenuManager->GetPC()->GetPlayerShip() == TargetLeftSpacecraft)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

void SFlareTradeMenu::OnUndock()
{
	if (TargetLeftSpacecraft)
	{
		if (TargetLeftSpacecraft->IsActive())
		{
			if (TargetLeftSpacecraft->GetActive()->GetNavigationSystem()->IsDocked())
			{
				TargetLeftSpacecraft->GetActive()->GetNavigationSystem()->Undock();
				if (MenuManager->GetPC()->GetPlayerShip() == TargetLeftSpacecraft)
				{
					MenuManager->CloseMenu();
				}
			}
			else if (TargetLeftSpacecraft->GetActive()->GetDockingSystem()->GetDockCount() > 0)
			{
				MenuManager->GetPC()->GetShipPawn()->GetNavigationSystem()->Undock();
				if (MenuManager->GetPC()->GetPlayerShip() == TargetLeftSpacecraft)
				{
					MenuManager->CloseMenu();
				}
			}
		}
	}
}

EVisibility SFlareTradeMenu::GetBackToSelectionLeftVisibility() const
{
	if (!IsEnabled())
	{
		return EVisibility::Collapsed;
	}

	if (!TargetLeftSpacecraft)
	{
		return EVisibility::Collapsed;
	}

	if (!MultipleOwnedShipsOrFleets)
	{
		if (FirstShipCandidateLeft == nullptr)
		{
			{
				return EVisibility::Collapsed;
			}
		}
		else if (FirstShipCandidateLeft == TargetLeftSpacecraft)
		{
			return EVisibility::Collapsed;
		}
	}

	// First-person trading override
	AFlareSpacecraft* PhysicalSpacecraft = TargetLeftSpacecraft->GetActive();
	if (PhysicalSpacecraft)
	{
		bool IsAutoDocking = MenuManager->GetPC()->GetCompany()->IsTechnologyUnlocked("auto-docking");
		if (!IsAutoDocking)
		{
			return EVisibility::Collapsed;
		}
	}

	return EVisibility::Visible;

}

EVisibility SFlareTradeMenu::GetBackToSelectionRightVisibility() const
{
	if (!IsEnabled())
	{
		return EVisibility::Collapsed;
	}

	FText Reason;
	if (TargetLeftSpacecraft)
	{
		// First-person trading override
		AFlareSpacecraft* PhysicalSpacecraft = TargetLeftSpacecraft->GetActive();
		if (PhysicalSpacecraft)
		{
			bool IsAutoDocking = MenuManager->GetPC()->GetCompany()->IsTechnologyUnlocked("auto-docking");
			if (IsAutoDocking)
			{
				return TargetRightSpacecraft ? EVisibility::Visible : EVisibility::Collapsed;
			}
			else
			{
				return EVisibility::Collapsed;
			}
			return TargetRightSpacecraft ? EVisibility::Visible : EVisibility::Collapsed;
		}
	}
	return TargetRightSpacecraft ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlareTradeMenu::GetDonationButtonVisibility() const
{
	FText Unused;
	if (IsEnabled() && IsTransactionValid(Unused) && TransactionSourceSpacecraft)
	{
		if (TransactionDestinationSpacecraft && TransactionSourceSpacecraft->GetCompany() == MenuManager->GetPC()->GetCompany())
		{
			if (TransactionDestinationSpacecraft->GetCompany() != MenuManager->GetPC()->GetCompany())
			{
				return EVisibility::Visible;
			}
		}
	}
	return EVisibility::Collapsed;
}

EVisibility SFlareTradeMenu::GetTransactionDetailsVisibility() const
{

	FText Unused;
	return IsEnabled() && IsTransactionValid(Unused) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlareTradeMenu::GetTransactionInvalidVisibility() const
{
	return (IsEnabled() && TargetRightSpacecraft) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SFlareTradeMenu::GetLeftSpacecraftName() const
{
	if (TargetLeftSpacecraft)
	{
		return UFlareGameTools::DisplaySpacecraftName(TargetLeftSpacecraft);
	}
	else
	{
		return FText();
	}
}

FText SFlareTradeMenu::GetRightcraftInfo() const
{
	// Crash mitigation - If parent is hidden, so are we, don't try to use the target (#178)

	AFlarePlayerController* PC = MenuManager->GetPC();

	if (PC && !MenuManager->IsUIOpen())
	{
		return FText();
	}

	/*
	if ((OwnerWidget.IsValid() && OwnerWidget->GetVisibility() != EVisibility::Visible)
		|| (PC && !PC->GetMenuManager()->IsUIOpen()))
	{
		return FText();
	}
*/
	if (TargetLeftSpacecraft && IsValid(TargetRightSpacecraft))
	{
		// Get the object's distance
//		return LOCTEXT("Crash", "Is Valid");

		FText DistanceText;
		bool TradeMenu = 0;

		if (PC->GetPlayerShip() && PC->GetPlayerShip() != TargetRightSpacecraft)
		{
			AFlareSpacecraft* PlayerShipPawn = TargetLeftSpacecraft->GetActive();
			AFlareSpacecraft* TargetSpacecraftPawn = TargetRightSpacecraft->GetActive();

			if (PlayerShipPawn && TargetSpacecraftPawn)
			{
				if (PlayerShipPawn->GetNavigationSystem()->GetDockStation() == TargetSpacecraftPawn)
				{
					DistanceText = LOCTEXT("DockedHereStation", "Docked - ");
				}
				else
				{
					float Distance = (PlayerShipPawn->GetActorLocation() - TargetSpacecraftPawn->GetActorLocation()).Size();
					DistanceText = FText::Format(LOCTEXT("PlayerDistanceFormat", "{0} - "), AFlareHUD::FormatDistance(Distance / 100));
				}
			}
		}
		else
		{
			DistanceText = LOCTEXT("PlayerShipText", "Your ship - ");
		}

		// Class text
		FText ClassText;
		if (TargetRightSpacecraft->IsStation())
		{
			ClassText = FText::FromString(TargetRightSpacecraft->GetDescription()->Name.ToString() + " - ");
		}

		// Our company
		UFlareCompany* TargetCompany = TargetRightSpacecraft->GetCompany();

		// Other company
		if (TargetCompany && PC && TargetCompany != PC->GetCompany())
		{
			return FText::Format(LOCTEXT("OwnedByFormat", "{0}{1}{2}"),
				DistanceText,
				ClassText,
				TargetCompany->GetCompanyName());
		}
	}
	return FText();
}

FText SFlareTradeMenu::GetRightSpacecraftName() const
{
	if (TargetRightSpacecraft)
	{
		return UFlareGameTools::DisplaySpacecraftName(TargetRightSpacecraft);
	}
	else
	{
		return LOCTEXT("NoSelectedSpacecraft", "No spacecraft selected");
	}
}

FText SFlareTradeMenu::GetTransactionDetails() const
{
	if (TransactionSourceSpacecraft && TransactionDestinationSpacecraft && TransactionResource)
	{
		FText TradeStatus = FText();

		FText UnitPrice;
		FText AffordableInfo;

		if (TransactionSourceSpacecraft->GetCompany() != MenuManager->GetPC()->GetCompany() || TransactionDestinationSpacecraft->GetCompany() != MenuManager->GetPC()->GetCompany())
		{
			int64 BaseResourcePrice = TransactionSourceSpacecraft->GetCurrentSector()->GetResourcePrice(TransactionResource, EFlareResourcePriceContext::Default);
			int64 TransactionResourcePrice = TransactionSourceSpacecraft->GetCurrentSector()->GetTransfertResourcePrice(TransactionSourceSpacecraft, TransactionDestinationSpacecraft, TransactionResource);

			int64 Fee = TransactionResourcePrice - BaseResourcePrice;
			if(TransactionDestinationSpacecraft->GetCompany() == MenuManager->GetPC()->GetCompany())
			{
				TradeStatus = FText(LOCTEXT("TradeInfo", "Buying"));
					UnitPrice = FText::Format(LOCTEXT("PurchaseUnitPriceFormat", "\nPurchase price: {0} credits/unit ({1} {2} {3} transport fee)"),
						UFlareGameTools::DisplayMoney(TransactionResourcePrice),
						UFlareGameTools::DisplayMoney(BaseResourcePrice),
						(Fee < 0 ? LOCTEXT("Minus", "-") : LOCTEXT("Plus", "+")),
						UFlareGameTools::DisplayMoney(FMath::Abs(Fee)));
			}
			else
			{
				if (DonationButton->IsActive())
				{
					TradeStatus = FText(LOCTEXT("TradeInfo", "Donating"));
				}
				else
				{
					TradeStatus = FText(LOCTEXT("TradeInfo", "Selling"));
				}

				UnitPrice = FText::Format(LOCTEXT("SellUnitPriceFormat", "\nSell price: {0} credits/unit ({1} {2} {3} transport fee)"),
					UFlareGameTools::DisplayMoney(TransactionResourcePrice),
					UFlareGameTools::DisplayMoney(BaseResourcePrice),
					(Fee < 0 ? LOCTEXT("Minus", "-"): LOCTEXT("Plus", "+")),
					UFlareGameTools::DisplayMoney(FMath::Abs(Fee)));
			}
		}
		else
		{
			TradeStatus = FText(LOCTEXT("TradeInfo", "Trading"));
		}

		// Add buyer capability if it's not the player
		if (TransactionDestinationSpacecraft->GetCompany() != MenuManager->GetPC()->GetCompany())
		{
			AffordableInfo = FText::Format(LOCTEXT("TradeAffordableFormat", "\nThe buyer has {0} credits available."),
				UFlareGameTools::DisplayMoney(TransactionDestinationSpacecraft->GetCompany()->GetMoney()));
		}

		FText MainInfo = FText::Format(LOCTEXT("TradeInfoFormat", "{0} {1}x {2} from {3} to {4}."),
			TradeStatus,
			FText::AsNumber(TransactionQuantity),
			TransactionResource->Name,
			UFlareGameTools::DisplaySpacecraftName(TransactionSourceSpacecraft),
			UFlareGameTools::DisplaySpacecraftName(TransactionDestinationSpacecraft));

		return FText::Format(LOCTEXT("TradeInfoMergeFormat", "{0}{1}{2}"), MainInfo, UnitPrice, AffordableInfo);
	}
	else
	{
		return FText();
	}
}

FText SFlareTradeMenu::GetShipTradeDetails() const
{
		if(!TargetLeftSpacecraft)
		{
			return FText();
		}

		AFlareSpacecraft* PhysicalSpacecraft = TargetLeftSpacecraft->GetActive();

		if (PhysicalSpacecraft)
		{
			UFlareSpacecraftNavigationSystem* Spacecraftnavigation = PhysicalSpacecraft->GetNavigationSystem();
			if (Spacecraftnavigation)
			{
				UFlareSimulatedSpacecraft* TransactionDestination = Spacecraftnavigation->GetTransactionDestination();
				UFlareSimulatedSpacecraft* TransactionDestinationDock = Spacecraftnavigation->GetTransactionDestinationDock();
				UFlareSimulatedSpacecraft* TransactionSourceShip = Spacecraftnavigation->GetTransactionSourceShip();
				FFlareResourceDescription* TransactionResourceL = Spacecraftnavigation->GetTransactionResource();
				uint32 TransactionQuantityL = Spacecraftnavigation->GetTransactionQuantity();

				if (TransactionDestination && TransactionSourceShip && TransactionResourceL && Spacecraftnavigation && TransactionDestinationDock)
				{
					FText Formatted;
					FText DistanceText;
					FText TradeStatus = FText();

					AFlareSpacecraft* TargetSpacecraftPawn = TransactionDestinationDock->GetActive();
					float Distance = (PhysicalSpacecraft->GetActorLocation() - TargetSpacecraftPawn->GetActorLocation()).Size();
					DistanceText = FText::Format(LOCTEXT("PlayerDistanceFormat", "{0} - "), AFlareHUD::FormatDistance(Distance / 100));

					if (MenuManager->GetPC()->GetCompany() == TransactionDestinationDock->GetCompany())
					{
						TradeStatus = FText(LOCTEXT("TradeInfoTransfer", "trade"));
					}

					else if (TransactionDestination == TransactionDestinationDock)
					{
						if (Spacecraftnavigation->GetTransactionDonation())
						{
							TradeStatus = FText(LOCTEXT("TradeInfoSell", "donate"));
						}
						else
						{
							TradeStatus = FText(LOCTEXT("TradeInfoSell", "sell"));
						}
					}
					else
					{
						TradeStatus = FText(LOCTEXT("TradeInfoSell", "buy"));
					}

					Formatted = FText::Format(LOCTEXT("SpacecraftInfoFormat", "{0}docking at {1} to {2} {3} {4}"),
						DistanceText,
						UFlareGameTools::DisplaySpacecraftName(TransactionDestinationDock),
						TradeStatus,
						FText::AsNumber(TransactionQuantityL),
						TransactionResourceL->Name);
					return Formatted;
				}
				else
				{
					return FText(LOCTEXT("HelpText", "Your ship is ready to trade with another spacecraft."));
				}
			}
			else
			{
				return FText(LOCTEXT("HelpText", "Your ship is ready to trade with another spacecraft."));
			}
		}
		else
		{
			return FText(LOCTEXT("HelpText", "Your ship is ready to trade with another spacecraft."));
		}
}

void SFlareTradeMenu::SetTransactionInvalidDetails(bool DockingConfirmed)
{
	FText FormulatedText;
	if (TransactionSourceSpacecraft && TransactionDestinationSpacecraft && TransactionResource)
	{
		FText Reason;
		IsTransactionValid(Reason);

		if (!Reason.IsEmptyOrWhitespace())
		{
			FormulatedText = FText::Format(LOCTEXT("TradeInvalidInfoFormat", "Can't trade {0} from {1} to {2} !\n\u2022 {3}\n"),
			TransactionResource->Name,
			UFlareGameTools::DisplaySpacecraftName(TransactionSourceSpacecraft),
			UFlareGameTools::DisplaySpacecraftName(TransactionDestinationSpacecraft),
			Reason);
		}
	}

	if (!DockingConfirmed)
	{
		FormulatedText = FText::Format(LOCTEXT("TradeInvalidInfoFormat", "{0}\u2022 Docking failed\n\u2022 Destination station requires a free docking port"),
		FormulatedText);
	}

	InvalidTransaction->SetText(FormulatedText);
}

FText SFlareTradeMenu::GetResourcePriceInfo(FFlareResourceDescription* Resource) const
{
	if (TargetSector)
	{
		FNumberFormattingOptions MoneyFormat;
		MoneyFormat.MaximumFractionalDigits = 2;

		int32 MeanDuration = 50;
		int64 ResourcePrice = TargetSector->GetResourcePrice(Resource, EFlareResourcePriceContext::Default);
		int64 LastResourcePrice = TargetSector->GetResourcePrice(Resource, EFlareResourcePriceContext::Default, MeanDuration - 1);

		FText VariationText;


		if(ResourcePrice != LastResourcePrice)
		{
			float Variation = (((float) ResourcePrice) / ((float) LastResourcePrice) - 1);

			VariationText = UFlareGameTools::AddLeadingSpace(FText::Format(LOCTEXT("ResourceVariationFormat", "({0}{1}%)"),
							(Variation > 0 ?
								 LOCTEXT("ResourceVariationFormatSignPlus","+") :
								 LOCTEXT("ResourceVariationFormatSignMinus","-")),
						  FText::AsNumber(FMath::Abs(Variation) * 100.0f, &MoneyFormat)));
		}

		return FText::Format(LOCTEXT("ResourceMainPriceFormat", "{0} credits{2} - Transport fee : {1} credits "),
			FText::AsNumber(ResourcePrice / 100.0f, &MoneyFormat),
			FText::AsNumber(Resource->TransportFee / 100.0f, &MoneyFormat),
			VariationText);
	}

	return FText();
}

void SFlareTradeMenu::OnSpacecraftSelectedLeft(TSharedPtr<FInterfaceContainer> SpacecraftContainer)
{
	UFlareSimulatedSpacecraft* Spacecraft = SpacecraftContainer->SpacecraftPtr;
	if (Spacecraft)
	{
		UFlareSimulatedSpacecraft* OldLeftSpacecraft = TargetLeftSpacecraft;
		TargetLeftSpacecraft = Spacecraft;

		if (TargetLeftSpacecraft != OldLeftSpacecraft)
		{
			PreviousTradeDirection = 0;
			FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay1, LeftCargoBay2);
			FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay1, RightCargoBay2);
		}
	}
	LeftShipList->SetVisibility(EVisibility::Collapsed);
	RightShipList->RefreshList();
	SetTransactionInvalidDetails();
}

void SFlareTradeMenu::OnSpacecraftSelectedRight(TSharedPtr<FInterfaceContainer> SpacecraftContainer)
{
	UFlareSimulatedSpacecraft* Spacecraft = SpacecraftContainer->SpacecraftPtr;
	SelectSpacecraftRight(Spacecraft);
}

void SFlareTradeMenu::SelectSpacecraftRight(UFlareSimulatedSpacecraft* Spacecraft)
{
	if (Spacecraft)
	{
		// Store spacecrafts
		TargetRightSpacecraft = Spacecraft;
		CompanyFlag->SetCompany(TargetRightSpacecraft->GetCompany());

		// Reset menus
		PriceBox->Hide();
		PreviousTradeDirection = 1;
		FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay1, RightCargoBay2);
		FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay1, LeftCargoBay2);
		RightShipList->SetVisibility(EVisibility::Collapsed);
	}
	SetTransactionInvalidDetails();
}

void SFlareTradeMenu::OnDonationToggle()
{
	if (DonationButton->IsActive())
	{
	}

	else
	{
	}

	TransactionQuantity = GetMaxTransactionAmount();
	QuantitySlider->SetValue(1.0f);
	QuantityText->SetText(FText::AsNumber(TransactionQuantity));
	UpdatePrice();
	SetTransactionInvalidDetails();
}

void SFlareTradeMenu::OnTransferResources(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource)
{
	FLOGV("OnTransferResources %p %p", SourceSpacecraft, DestinationSpacecraft);
	if (DestinationSpacecraft && DestinationSpacecraft && Resource)
	{
		// Store transaction dataQuantityText
		DonationButton->SetActive(false);
		TransactionSourceSpacecraft = SourceSpacecraft;
		TransactionDestinationSpacecraft = DestinationSpacecraft;
		TransactionResource = Resource;
		TransactionQuantity = GetMaxTransactionAmount();

		QuantitySlider->SetValue(1.0f);
		QuantityText->SetText(FText::AsNumber(TransactionQuantity));

		UpdatePrice();
		UFlareQuest* SelectedQuest = MenuManager->GetGame()->GetQuestManager()->GetSelectedQuest();

		if (SelectedQuest && SelectedQuest->GetCurrentStep())
		{
			auto PlayerShip = (SourceSpacecraft->IsStation() ? DestinationSpacecraft : SourceSpacecraft);
			for(UFlareQuestCondition* Condition : SelectedQuest->GetCurrentConditions())
			{
				UFlareQuestConditionBuyAtStation* BuyAtStationCondition = Cast<UFlareQuestConditionBuyAtStation>(Condition);
				if (BuyAtStationCondition && BuyAtStationCondition->GetResource() == Resource && BuyAtStationCondition->GetTargetStation() == TransactionSourceSpacecraft)
				{
					int32 MissingQuantity = BuyAtStationCondition->GetTargetQuantity() - BuyAtStationCondition->GetCurrentProgression();				
					int32 PreferredQuantity = MissingQuantity - PlayerShip->GetActiveCargoBay()->GetResourceQuantity(Resource, PlayerShip->GetCompany());

					if (TransactionDestinationSpacecraft == PlayerShip)
					{
						PreferredQuantity = MissingQuantity;
					}

					int32 AssignedQuantity = SetSliderQuantity(PreferredQuantity);
					QuantityText->SetText(FText::AsNumber(AssignedQuantity));
					break;
				}

				UFlareQuestConditionSellAtStation* SellAtStationCondition = Cast<UFlareQuestConditionSellAtStation>(Condition);
				if (SellAtStationCondition && SellAtStationCondition->GetResource() == Resource && (SellAtStationCondition->GetTargetStation() == TransactionDestinationSpacecraft || DestinationSpacecraft==PlayerShip))
				{
					int32 MissingQuantity = SellAtStationCondition->GetTargetQuantity() - SellAtStationCondition->GetCurrentProgression();
					int32 PreferredQuantity = MissingQuantity - PlayerShip->GetActiveCargoBay()->GetResourceQuantity(Resource, PlayerShip->GetCompany());
					if (TransactionSourceSpacecraft == PlayerShip)
					{
						PreferredQuantity = MissingQuantity;
					}

					int32 AssignedQuantity = SetSliderQuantity(PreferredQuantity);
					QuantityText->SetText(FText::AsNumber(AssignedQuantity));
					break;
				}
			}
		}
	}
	SetTransactionInvalidDetails();
}

void SFlareTradeMenu::OnResourceQuantityChanged(float Value)
{
	int32 ResourceMaxQuantity = GetMaxTransactionAmount();

	// Calculate transaction amount, depending on max value (step mechanism)
	TransactionQuantity = FMath::Lerp((int32)1, ResourceMaxQuantity, Value);
	if (ResourceMaxQuantity >= 1000 && (TransactionQuantity - ResourceMaxQuantity) > 50)
	{
		TransactionQuantity = (TransactionQuantity / 50) * 50;
	}
	else if (ResourceMaxQuantity >= 100 && (TransactionQuantity - ResourceMaxQuantity) > 10)
	{
		TransactionQuantity = (TransactionQuantity / 10) * 10;
	}
	
	// Force slider value, update quantity
	if (ResourceMaxQuantity == 1)
	{
		QuantitySlider->SetValue(1.0f);
	}
	else
	{
		QuantitySlider->SetValue((float)(TransactionQuantity - 1) / (float)(ResourceMaxQuantity - 1));
	}

	QuantityText->SetText(FText::AsNumber(TransactionQuantity));

	UpdatePrice();
}

void SFlareTradeMenu::OnResourceQuantityEntered(const FText& TextValue)
{
	if (TextValue.ToString().IsNumeric())
	{
		SetSliderQuantity(FCString::Atoi(*TextValue.ToString()));
	}
}

void SFlareTradeMenu::OnConfirmTransaction()
{
	bool ResetVariables = true;
	bool Donation = false;
	bool DockingConfirmed = true;

	if(TransactionSourceSpacecraft && TransactionDestinationSpacecraft)
	{
		if (DonationButton->IsActive())
		{
			if (TransactionDestinationSpacecraft->GetCompany() != TransactionSourceSpacecraft->GetCompany())
			{
				if (TransactionDestinationSpacecraft->GetCompany() != MenuManager->GetPC()->GetCompany())
				{
					Donation = true;
				}
			}
		}

	// Actual transaction
	if (TransactionSourceSpacecraft->GetCurrentSector() && TransactionResource)
	{
		if (TargetLeftSpacecraft && TargetLeftSpacecraft->IsActive() && TargetRightSpacecraft && TargetRightSpacecraft->IsActive())
		{
			AFlareSpacecraft* PhysicalSpacecraft = TargetLeftSpacecraft->GetActive();
			AFlareSpacecraft* PhysicalSpacecraftDock = TargetRightSpacecraft->GetActive();
			WasActiveSector = true;

			if (PhysicalSpacecraftDock->GetDockingSystem()->IsDockedShip(PhysicalSpacecraft))
			{
				SectorHelper::Trade(TransactionSourceSpacecraft,
					TransactionDestinationSpacecraft,
					TransactionResource,
					TransactionQuantity,
					NULL, NULL, Donation);
				RightShipList->RefreshList();
			}
			else
			{
				DockingConfirmed = PhysicalSpacecraft->GetNavigationSystem()->DockAtAndTrade(PhysicalSpacecraftDock, TransactionResource, TransactionQuantity, TransactionSourceSpacecraft, TransactionDestinationSpacecraft, Donation);
				if (!DockingConfirmed)
				{
					ResetVariables = false;
				}
				else if (PhysicalSpacecraft->IsPlayerShip())
				{
					if (!MultipleOwnedShipsOrFleets)
					{
	//player ship is the only viable ship for trade menu, close menu.
						MenuManager->CloseMenu();
					}
				}
			}
		}
		else
		{
			SectorHelper::Trade(TransactionSourceSpacecraft,
				TransactionDestinationSpacecraft,
				TransactionResource,
				TransactionQuantity,
				NULL, NULL, Donation);
		}
	}
	}

	// Reset transaction data
	if (ResetVariables)
	{
		TransactionDestinationSpacecraft = NULL;
		TransactionSourceSpacecraft = NULL;
		TransactionResource = NULL;
		TransactionQuantity = 0;
		PriceBox->Hide();
	}

	// Reset menus
	PreviousTradeDirection = true;
	FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay1, RightCargoBay2);
	FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay1, LeftCargoBay2);
	MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->InfoSound);
	UpdateLeftShips();
	SetTransactionInvalidDetails(DockingConfirmed);
}

void SFlareTradeMenu::OnSelectDockedSelection()
{
	if (TargetLeftSpacecraft && TargetLeftSpacecraft->GetActive() && TargetLeftSpacecraft->GetActive()->GetNavigationSystem()->GetDockStation())
	{
		SelectSpacecraftRight(TargetLeftSpacecraft->GetActive()->GetNavigationSystem()->GetDockStation()->GetParent());
	}
}

void SFlareTradeMenu::OnBackToSelectionLeft()
{
	TargetLeftSpacecraft = NULL;
	// Reset transaction data
	LeftCargoBay1->ClearChildren();
	LeftCargoBay2->ClearChildren();
	LeftShipList->ClearSelection();
	LeftShipList->SetVisibility(EVisibility::Visible);
	OnCancelTransaction();
}

void SFlareTradeMenu::OnBackToSelectionRight()
{
	TargetRightSpacecraft = NULL;

	OnCancelTransaction();
/*
	// Reset transaction data
	TransactionDestinationSpacecraft = NULL;
	TransactionSourceSpacecraft = NULL;
	TransactionResource = NULL;
	TransactionQuantity = 0;
*/
	// Reset menus
	PriceBox->Hide();
	RightCargoBay1->ClearChildren();
	RightCargoBay2->ClearChildren();
	RightShipList->ClearSelection();
	RightShipList->SetVisibility(EVisibility::Visible);
}

void SFlareTradeMenu::OnCancelTransaction()
{
	// Reset transaction data
	TransactionDestinationSpacecraft = NULL;
	TransactionSourceSpacecraft = NULL;
	TransactionResource = NULL;
	TransactionQuantity = 0;

	// Reset menus
	PriceBox->Hide();
	PreviousTradeDirection = 1;
	FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay1, RightCargoBay2);
	FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay1, LeftCargoBay2);
	MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->NegativeClickSound);
	SetTransactionInvalidDetails();
}

void SFlareTradeMenu::UpdateLeftShips()
// Add ships (left side)
{
	LeftShipList->Reset();
	MultipleOwnedShipsOrFleets = false;
	int16 Addedships = 0;

	// Data
	FirstShipCandidateLeft = nullptr;
	UFlareCompany* CheckingCompany = MenuManager->GetPC()->GetCompany();
	if (TargetLeftSpacecraft)
	{
		CheckingCompany = TargetLeftSpacecraft->GetCompany();
	}

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < TargetSector->GetSectorShips().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* ShipCandidate = TargetSector->GetSectorShips()[SpacecraftIndex];
		if (ShipCandidate && ShipCandidate->GetCompany() == CheckingCompany && ShipCandidate->GetActiveCargoBay()->GetSlotCount() > 0 && !ShipCandidate->IsTrading())
		{
			LeftShipList->AddShip(ShipCandidate);
			Addedships++;
			if (FirstShipCandidateLeft == nullptr)
			{
				FirstShipCandidateLeft = ShipCandidate;
			}
		}
	}

	if (Addedships > 1)
	{
		MultipleOwnedShipsOrFleets = true;
	}
	LeftShipList->RefreshList();
}


void SFlareTradeMenu::UpdatePrice()
{
	// Update price
	uint32 ResourceUnitPrice = 0;

	if (TransactionSourceSpacecraft && TransactionSourceSpacecraft->GetCurrentSector())
	{
		ResourceUnitPrice = TransactionSourceSpacecraft->GetCurrentSector()->GetTransfertResourcePrice(
			TransactionSourceSpacecraft,
			TransactionDestinationSpacecraft,
			TransactionResource);
	}

	FText Unused;
	if (IsTransactionValid(Unused))
	{
		if (TransactionSourceSpacecraft && TransactionDestinationSpacecraft && TransactionDestinationSpacecraft->GetCompany() != TransactionSourceSpacecraft->GetCompany() && ResourceUnitPrice > 0)
		{
			if (TransactionDestinationSpacecraft->GetCompany() != MenuManager->GetPC()->GetCompany() && DonationButton->IsActive())
			{
				PriceBox->Show(0, MenuManager->GetPC()->GetCompany());
			}
			else
			{
				int64 TransactionPrice = TransactionQuantity * ResourceUnitPrice;
				bool AllowDepts = TransactionDestinationSpacecraft->GetResourceUseType(TransactionResource).HasUsage(EFlareResourcePriceContext::ConsumerConsumption) || MenuManager->GetGame()->GetQuestManager()->IsTradeQuestUseStation(TransactionDestinationSpacecraft, false);
				PriceBox->Show(TransactionPrice, TransactionDestinationSpacecraft->GetCompany(), AllowDepts);
			}

		}
		else
		{
			PriceBox->Show(0, MenuManager->GetPC()->GetCompany());
		}
	}
	else
	{
		PriceBox->Hide();
	}
}

int32 SFlareTradeMenu::SetSliderQuantity(int32 Quantity)
{
	int32 ResourceMaxQuantity = GetMaxTransactionAmount();

	TransactionQuantity = FMath::Clamp(Quantity, 0, ResourceMaxQuantity);
	FLOGV("SFlareTradeMenu::SetSliderQuantity number %d / %d", TransactionQuantity, ResourceMaxQuantity)

	if (ResourceMaxQuantity == 1)
	{
		QuantitySlider->SetValue(1.0f);
	}
	else
	{
		QuantitySlider->SetValue((float)(TransactionQuantity - 1) / (float)(ResourceMaxQuantity - 1));
	}

	UpdatePrice();

	return TransactionQuantity;
}

bool SFlareTradeMenu::RefreshTradeBlocks() const
{
	//USED to refresh tradeblocks when a ship with a local trade order (which goes to dock at a station) is finished, to stop any possible updates not having values not set properly afterwards

	PriceBox->Hide();

	MenuManager->GetTradeMenu()->TransactionSourceSpacecraft = NULL;
	MenuManager->GetTradeMenu()->TransactionDestinationSpacecraft = NULL;
	MenuManager->GetTradeMenu()->TransactionResource = NULL;
	MenuManager->GetTradeMenu()->TransactionQuantity = 0;

	if(PreviousTradeDirection == 1)
		{
		MenuManager->GetTradeMenu()->FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay1, RightCargoBay2);
		MenuManager->GetTradeMenu()->FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay1, LeftCargoBay2);
		RightShipList->RefreshList();
		return true;
	}
	else if (PreviousTradeDirection == 0)
	{
		MenuManager->GetTradeMenu()->FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay1, LeftCargoBay2);
		MenuManager->GetTradeMenu()->FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay1, RightCargoBay2);
		RightShipList->RefreshList();
		return true;
	}

	return false;
}

bool SFlareTradeMenu::IsTransactionValid(FText& Reason) const
{
	// Possible transaction

	if (TransactionSourceSpacecraft && TransactionDestinationSpacecraft && TransactionResource)
	{
		int32 ResourcePrice = TransactionSourceSpacecraft->GetCurrentSector()->GetTransfertResourcePrice(TransactionSourceSpacecraft, TransactionDestinationSpacecraft, TransactionResource);
		int32 MaxAffordableQuantity =  FMath::Max(int64(0), TransactionDestinationSpacecraft->GetCompany()->GetMoney() / ResourcePrice);
		int32 ResourceMaxQuantity;
		if (TransactionDestinationSpacecraft->GetCompany() == TransactionSourceSpacecraft->GetCompany())
		{
			ResourceMaxQuantity = FMath::Min(TransactionSourceSpacecraft->GetActiveCargoBay()->GetResourceQuantity(TransactionResource, MenuManager->GetPC()->GetCompany(),0,true),
			TransactionDestinationSpacecraft->GetActiveCargoBay()->GetFreeSpaceForResource(TransactionResource, MenuManager->GetPC()->GetCompany(),false,0,true));
		}
		else
		{
			ResourceMaxQuantity = FMath::Min(TransactionSourceSpacecraft->GetActiveCargoBay()->GetResourceQuantity(TransactionResource, MenuManager->GetPC()->GetCompany()),
			TransactionDestinationSpacecraft->GetActiveCargoBay()->GetFreeSpaceForResource(TransactionResource, MenuManager->GetPC()->GetCompany()));
		}

		// Cases of failure + reason
		if (!TransactionSourceSpacecraft->CanTradeWith(TransactionDestinationSpacecraft, Reason, TransactionResource))
		{
			return false;
		}
		 
		// Special exception for same company
		else if (TransactionSourceSpacecraft->GetCompany() == TransactionDestinationSpacecraft->GetCompany())
		{
			return true;
		}

		else if (!TransactionSourceSpacecraft->GetActiveCargoBay()->WantSell(TransactionResource, MenuManager->GetPC()->GetCompany()))
		{
			Reason = LOCTEXT("CantTradeSell", "This resource isn't sold by the seller. Input resources are never sold.");
			return false;
		}
		else if (!TransactionDestinationSpacecraft->GetActiveCargoBay()->WantBuy(TransactionResource, MenuManager->GetPC()->GetCompany()))
		{
			Reason = LOCTEXT("CantTradeBuy", "This resource isn't bought by the buyer. Output resources are never bought. The buyer needs an empty slot, or one with the matching resource.");
			return false;
		}

		else if (MaxAffordableQuantity == 0 && !MenuManager->GetGame()->GetQuestManager()->IsTradeQuestUseStation(TransactionDestinationSpacecraft, false))
		{

			if(TransactionDestinationSpacecraft->GetCompany() == MenuManager->GetPC()->GetCompany())
			{
				Reason = LOCTEXT("YouCantTradePrice", "You can't afford to buy any of this resource.");
				return false;
			}
			else
			{
				Reason = LOCTEXT("CantTradePrice", "The buyer can't afford to buy any of this resource.");
			}
		}
		else if (ResourceMaxQuantity == 0)
		{
			if(TransactionDestinationSpacecraft->GetCompany() == MenuManager->GetPC()->GetCompany())
			{
				Reason = LOCTEXT("YouCantTradeQuantity", "You don't have any space left to buy any of this resource (one resource type per cargo bay).");
			}
			else
			{
				Reason = LOCTEXT("CantTradeQuantity", "The buyer doesn't have any space left to buy any of this resource.");
			}
			return false;
		}
	}

	// No possible transaction
	else
	{
		return false;
	}

	return true;
}

int32 SFlareTradeMenu::GetMaxTransactionAmount() const
{
	if (!TransactionSourceSpacecraft || !TransactionDestinationSpacecraft)
	{
		return FMath::Min(0, 0);
	}

	int32 ResourcePrice = TransactionSourceSpacecraft->GetCurrentSector()->GetTransfertResourcePrice(TransactionSourceSpacecraft, TransactionDestinationSpacecraft, TransactionResource);
	int32 MaxAffordableQuantity = 0;

	if(TransactionDestinationSpacecraft->GetCompany() == TransactionSourceSpacecraft->GetCompany() || MenuManager->GetGame()->GetQuestManager()->IsTradeQuestUseStation(TransactionDestinationSpacecraft, false)
	|| (TransactionDestinationSpacecraft->GetCompany() != MenuManager->GetPC()->GetCompany() && DonationButton->IsActive())
	)
	{
		MaxAffordableQuantity = INT_MAX;
	}
	else
	{
		MaxAffordableQuantity = FMath::Max(int64(0), TransactionDestinationSpacecraft->GetCompany()->GetMoney()) / ResourcePrice;
	}

	int32 ResourceMaxQuantity;
	if (TransactionDestinationSpacecraft->GetCompany() == TransactionSourceSpacecraft->GetCompany())
	{
		ResourceMaxQuantity = FMath::Min(TransactionSourceSpacecraft->GetActiveCargoBay()->GetResourceQuantity(TransactionResource, MenuManager->GetPC()->GetCompany(), 0, true),
			TransactionDestinationSpacecraft->GetActiveCargoBay()->GetFreeSpaceForResource(TransactionResource, MenuManager->GetPC()->GetCompany(),false, 0, true));
	}
	else
	{
		ResourceMaxQuantity = FMath::Min(TransactionSourceSpacecraft->GetActiveCargoBay()->GetResourceQuantity(TransactionResource, MenuManager->GetPC()->GetCompany()),
			TransactionDestinationSpacecraft->GetActiveCargoBay()->GetFreeSpaceForResource(TransactionResource, MenuManager->GetPC()->GetCompany()));
	}
	return FMath::Min(MaxAffordableQuantity, ResourceMaxQuantity);
}

#undef LOCTEXT_NAMESPACE