
#include "FlareShipMenu.h"

#include "../../Flare.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareSector.h"
#include "../../Game/FlareGameTools.h"

#include "../../Data/FlareSpacecraftCatalog.h"
#include "../../Data/FlareSpacecraftComponentsCatalog.h"
#include "../../Data/FlareResourceCatalog.h"

#include "../../Economy/FlareCargoBay.h"

#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Player/FlareHUD.h"

#include "../Components/FlareRoundButton.h"
#include "../Components/FlarePartInfo.h"
#include "../Components/FlareFactoryInfo.h"


#define LOCTEXT_NAMESPACE "FlareShipMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareShipMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	AFlarePlayerController* PC = MenuManager->GetPC();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 TextWidth = Theme.ContentWidth - 2 * Theme.ContentPadding.Left - 2 * Theme.ContentPadding.Right;

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SVerticalBox)
		
		// Content
		+ SVerticalBox::Slot()
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			.HAlign(HAlign_Left)
			[
				SNew(SVerticalBox)

				// Object name
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.TitlePadding)
				[
					SAssignNew(ObjectName, STextBlock)
					.TextStyle(&Theme.SubTitleFont)
				]

				// Action box
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SAssignNew(ObjectActionMenu, SFlareSpacecraftInfo)
					.Player(PC)
					.NoInspect(true)
					.OwnerWidget(this)
				]

				// Ship rename
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Left)
				[
					SAssignNew(RenameBox, SHorizontalBox)

					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Top)
					.Padding(Theme.ContentPadding)
					.MaxWidth(96)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(LOCTEXT("RenameShipInfo", "Ship name"))
					]

					+ SHorizontalBox::Slot()
					.MaxWidth(Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(SBox)
						.WidthOverride(0.40 * Theme.ContentWidth)
						[
							SNew(SBorder)
							.BorderImage(&Theme.BackgroundBrush)
							.Padding(Theme.ContentPadding)
							[
								SAssignNew(ShipName, SEditableText)
								.AllowContextMenu(false)
								.Style(&Theme.TextInputStyle)
							]
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					.Padding(FMargin(87,0,0,0))
					[
						SNew(SFlareButton)
						.Text(LOCTEXT("Rename", "Rename"))
						.HelpText(LOCTEXT("RenameInfo", "Rename this ship"))
						.Icon(FFlareStyleSet::GetIcon("OK"))
						.OnClicked(this, &SFlareShipMenu::OnRename)
						.IsDisabled(this, &SFlareShipMenu::IsRenameDisabled)
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
									.OnGenerateWidget(this, &SFlareShipMenu::OnGenerateWhiteListComboLine)
									.OnSelectionChanged(this, &SFlareShipMenu::OnWhiteListComboLineSelectionChanged)
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
								.OnClicked(this, &SFlareShipMenu::OnSelectWhiteList)
								.IsDisabled(this, &SFlareShipMenu::IsWhiteListSelectDisabled)
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
								.OnClicked(this, &SFlareShipMenu::OnRemoveWhiteList)
								.IsDisabled(this, &SFlareShipMenu::IsWhiteListRemoveDisabled)
								.Width(1)
							]
						]
					]
				]

				// Object class
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.TitlePadding)
				[
					SAssignNew(ObjectClassName, STextBlock)
					.TextStyle(&Theme.SubTitleFont)
				]

				// Object description
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				[
					SAssignNew(ObjectDescription, STextBlock)
					.TextStyle(&Theme.TextFont)
					.WrapTextAt(Theme.ContentWidth)
				]

				// Complex resource info
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SAssignNew(ObjectProductionBreakdown, STextBlock)
					.TextStyle(&Theme.TextFont)
					.WrapTextAt(Theme.ContentWidth)
				]

				// Station complex
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Top)
					[
						SAssignNew(ComplexLayout, SImage)
						.Image(FFlareStyleSet::GetImage("Complex"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					[
						SAssignNew(ComplexList, SVerticalBox)
					]
				]

				// Shipyard actions
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Left)
				.Padding(Theme.ContentPadding)
				[
					SNew(SHorizontalBox)
					
					// Light ship
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Width(7)
						.Text(LOCTEXT("OrderLightShip", "Order light ship"))
						.HelpText(this, &SFlareShipMenu::GetLightShipTextInfo)
						.OnClicked(this, &SFlareShipMenu::OnOpenSpacecraftOrder, false)
						.IsDisabled(this, &SFlareShipMenu::IsShipSSelectorDisabled)
						.Visibility(this, &SFlareShipMenu::GetShipyardVisibility)
					]
					
					// Heavy ship
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Width(7)
						.Text(LOCTEXT("OrderHeavyShip", "Order heavy ship"))
						.HelpText(this, &SFlareShipMenu::GetHeavyShipTextInfo)
						.OnClicked(this, &SFlareShipMenu::OnOpenSpacecraftOrder, true)
						.IsDisabled(this, &SFlareShipMenu::IsShipLSelectorDisabled)
						.Visibility(this, &SFlareShipMenu::GetShipyardVisibility)
					]
				]
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Left)
					.Padding(Theme.ContentPadding)
					[
						SNew(SHorizontalBox)
					// Allow external Configuration
					+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(ExternalOrdersConfigurationButton,SFlareButton)
							.Width(7)
							.Text(this, &SFlareShipMenu::GetConfigurationText)
							.HelpText(this, &SFlareShipMenu::GetConfigurationHelp)
							.OnClicked(this, &SFlareShipMenu::OnOpenExternalConfig)
							.Visibility(this, &SFlareShipMenu::GetShipyardAllowExternalOrderConfigurationVisibility)
						]
					// Allow external
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SAssignNew(AllowExternalOrdersButton, SFlareButton)
						.Width(7)
						.Text(this, &SFlareShipMenu::GetExternalordersText)
						.HelpText(this, &SFlareShipMenu::GetExternalOrdersHelp)
						.OnClicked(this, &SFlareShipMenu::OnToggleAllowExternalOrders)
						.Visibility(this, &SFlareShipMenu::GetShipyardAllowExternalOrderVisibility)
						.Toggle(true)
					]

					// Allow external
					+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(AllowAutoConstructionButton, SFlareButton)
							.Width(7)
						.Text(this, &SFlareShipMenu::GetAutoConstructionText)
						.HelpText(this, &SFlareShipMenu::GetAutoConstructionHelp)
						.OnClicked(this, &SFlareShipMenu::OnToggleAllowAutoConstruction)
						.Visibility(this, &SFlareShipMenu::GetAutoConstructionVisibility)
						.Toggle(true)
						]

					
]

				// Shipyard list
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.Padding(Theme.ContentPadding)
				[
					SAssignNew(ShipyardList, SVerticalBox)
				]

				// Factory list
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SAssignNew(FactoryList, SVerticalBox)
				]
				// Factory box
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SAssignNew(UpgradeBox, SVerticalBox)
				]
				
				// Ship part characteristics
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(SBox)
					.HAlign(HAlign_Fill)
					.WidthOverride(Theme.ContentWidth)
					[
						SAssignNew(PartCharacteristicBox, SHorizontalBox)
					]
				]

				// Ship customization panel
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(ShipCustomizationBox, SVerticalBox)
			
					// Edit info
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(LOCTEXT("EditInfoText", "Click on a part to exchange it. Please return only loaded, functional parts. Combat damage will void the warranty."))
						.Visibility(this, &SFlareShipMenu::GetEditVisibility)
					]

					// Upgrade text
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareShipMenu::GetShipUpgradeDetails)
					.WrapTextAt(TextWidth)
					]

					// Components
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Engines
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.TitleButtonPadding)
						[
							SNew(SFlareRoundButton)
							.OnClicked(this, &SFlareShipMenu::ShowEngines)
							.Icon(this, &SFlareShipMenu::GetEngineIcon)
							.Text(this, &SFlareShipMenu::GetEngineText)
							.HelpText(LOCTEXT("EngineInfo", "Inspect the current orbital engines"))
							.InvertedBackground(true)
							.Visibility(this, &SFlareShipMenu::GetEngineVisibility)
							.HighlightColor(this, &SFlareShipMenu::GetEnginesHealthColor)
						]

						// RCS
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.TitleButtonPadding)
						[
							SNew(SFlareRoundButton)
							.OnClicked(this, &SFlareShipMenu::ShowRCSs)
							.Icon(this, &SFlareShipMenu::GetRCSIcon)
							.Text(this, &SFlareShipMenu::GetRCSText)
							.HelpText(LOCTEXT("RCSInfo", "Inspect the current attitude control thrusters (RCS)"))
							.InvertedBackground(true)
							.Visibility(this, &SFlareShipMenu::GetEngineVisibility)
							.HighlightColor(this, &SFlareShipMenu::GetRCSHealthColor)
						]

						// Weapons
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(WeaponButtonBox, SHorizontalBox)
						]
					]
				]

				// Ship part customization panel
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(ShipPartCustomizationBox, SVerticalBox)

					// Section title
					+ SVerticalBox::Slot()
					.Padding(Theme.TitlePadding)
					.AutoHeight()
					[
						SAssignNew(ShipPartPickerTitle, STextBlock)
						.Text(LOCTEXT("ShipParts", "Available components"))
						.TextStyle(&FFlareStyleSet::GetDefaultTheme().SubTitleFont)
					]

					// Ship part picker
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(PartList, SListView< TSharedPtr<FInterfaceContainer> >)
						.ListItemsSource(&PartListDataShared)
						.SelectionMode(ESelectionMode::Single)
						.OnGenerateRow(this, &SFlareShipMenu::GeneratePartInfo)
						.OnSelectionChanged(this, &SFlareShipMenu::OnPartPicked)
					]

					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.TitlePadding)
					[
						SAssignNew(UpgradeTitle, STextBlock)
						.TextStyle(&Theme.SubTitleFont)
						.Text(LOCTEXT("TransactionTitle", "Upgrade component"))
					]

					// Can't upgrade
					+ SVerticalBox::Slot()
					.Padding(Theme.ContentPadding)
					.AutoHeight()
					[
						SAssignNew(CantUpgradeReason, STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(LOCTEXT("CantUpgradeDamaged", "This system has been damaged and can't be exchanged. Please repair your ships through the Sector menu."))
					]

					// Button box
					+ SVerticalBox::Slot()
					.Padding(Theme.ContentPadding)
					.AutoHeight()
					[
						SAssignNew(BuyConfirmation, SFlareConfirmationBox)
						.ConfirmText(LOCTEXT("Confirm", "Upgrade component"))
						.CancelText(LOCTEXT("BackTopShip", "Back to ship"))
						.OnConfirmed(this, &SFlareShipMenu::OnPartConfirmed)
						.OnCancelled(this, &SFlareShipMenu::OnPartCancelled)
						.UpgradeBehavior(true)
						.PC(PC)
					]
				]

				// Object list
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Left)
					[
						SAssignNew(OwnedList, SFlareList)
						.MenuManager(MenuManager)
						.ShowOwnedShips(true)
					.Title(LOCTEXT("OwnedShips", "Owned ships"))
					]

				// Object list
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Left)
				[
					SAssignNew(ShipList, SFlareList)
					.MenuManager(MenuManager)
					.ShowOwnedShips(true)
					.Title(LOCTEXT("DockedShips", "Docked ships"))
				]
			]
		]
	];
}

FText SFlareShipMenu::GetConfigurationText() const
{
	if (TargetDescription&&TargetDescription->IsDroneCarrier)
	{
		return LOCTEXT("CarrierConfiguration", "Carrier Configuration");
	}

	return LOCTEXT("ExternalInfoConfiguration", "External Info Configuration");

}

FText SFlareShipMenu::GetConfigurationHelp() const
{
	if (TargetDescription&&TargetDescription->IsDroneCarrier)
	{
		return LOCTEXT("CarrierConfigurationInfo", "Enable or disable specific ships which this carrier will build");
	}

	return LOCTEXT("ExternalInfoConfigurationInfo", "Enable or disable specific ships from being bought from other companies");
}

FText SFlareShipMenu::GetExternalordersText() const
{
	if (TargetDescription&&TargetDescription->IsDroneCarrier)
	{
		return LOCTEXT("AllowResupply", "Allow automatic ware resupply");
	}

	return LOCTEXT("AllowExternal", "Allow external orders");
}

FText SFlareShipMenu::GetExternalOrdersHelp() const
{
	if (TargetDescription&&TargetDescription->IsDroneCarrier)
	{
		return LOCTEXT("AllowResupplyInfo", "Allow this ship to automatically resupply from local sources");
	}

	return LOCTEXT("AllowExternalInfo", "Allow other companies to order ships here");
}


FText SFlareShipMenu::GetAutoConstructionText() const
{
	return LOCTEXT("AllowAutoConstruction", "Allow automatic construction");
}

FText SFlareShipMenu::GetAutoConstructionHelp() const
{
	return LOCTEXT("AllowAutoConstructionInfo", "Allow this ship to automatically queue up new build orders");
}

/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareShipMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);

	TargetSpacecraft = NULL;
	TargetDescription = NULL;
	RCSDescription = NULL;
	EngineDescription = NULL;
}

void SFlareShipMenu::Enter(UFlareSimulatedSpacecraft* Target, bool IsEditable)
{
	// Info
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	FLOGV("SFlareShipMenu::Enter : %s, CanEdit=%d", *Target->GetImmatriculation().ToString(), IsEditable);

	// Load data
	CanEdit = IsEditable;
	TargetSpacecraft = Target;
	TargetDescription = TargetSpacecraft->GetDescription();
	TargetSpacecraftData = Target->Save();
	LoadTargetSpacecraft();

	// Update lists
	UpdateProductionBreakdown();
	UpdateComplexList();
	UpdateShipyardList();
	UpdateFactoryList();
	UpdateUpgradeBox();

	// Move the viewer to the right
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		PC->GetMenuPawn()->SetCameraOffset(FVector2D(100, -30));
	}

	// Is the docking list visible ?
	UFlareSpacecraftDockingSystem* DockSystem = NULL;
	if (TargetSpacecraft->IsActive())
	{
		DockSystem = TargetSpacecraft->GetActive()->GetDockingSystem();
	}

	// Fill the docking list if it is visible
	if (TargetSpacecraft->IsStation())
	{
		if (DockSystem)
		{
			if (DockSystem->GetDockCount() > 0)
			{
				CheckDockedShips(DockSystem->GetDockedShips());
			}
			if (TargetSpacecraft->IsComplex())
			{
				for (UFlareSimulatedSpacecraft* Substation : TargetSpacecraft->GetComplexChildren())
				{
					CheckDockedShips(Substation->GetActive()->GetDockingSystem()->GetDockedShips());
				}
			}
		}
		ShipList->RefreshList();
	}
	else
	{
		ShipList->SetVisibility(EVisibility::Collapsed);
	}

	//fill the subordinates list if neccessary
	TArray<UFlareSimulatedSpacecraft*> ShipChildren = TargetSpacecraft->GetShipChildren();
	if (ShipChildren.Num() > 0)
	{
//		OwnedList->SetVisibility(EVisibility::Visible);

		for (int32 i = 0; i < ShipChildren.Num(); i++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = ShipChildren[i];
			if (Spacecraft)
			{
				if (Spacecraft->GetDamageSystem()->IsAlive())
				{
					OwnedList->AddShip(ShipChildren[i]);
				}
			}
			OwnedList->RefreshList();
		}
	}
	else
	{
		OwnedList->SetVisibility(EVisibility::Collapsed);
	}
}

void SFlareShipMenu::CheckDockedShips(TArray<AFlareSpacecraft*> DockedShips)
{
	for (int32 i = 0; i < DockedShips.Num(); i++)
	{
		AFlareSpacecraft* Spacecraft = DockedShips[i];

		if (Spacecraft)
		{
			FLOGV("SFlareShipMenu::Enter : Found docked ship %s", *Spacecraft->GetName());
		}
		if (DockedShips[i]->GetParent()->GetDamageSystem()->IsAlive())
		{
			ShipList->AddShip(DockedShips[i]->GetParent());
		}
	}
}

void SFlareShipMenu::Exit()
{
	ObjectActionMenu->Hide();
	ObjectProductionBreakdown->SetVisibility(EVisibility::Collapsed);

	PartListData.Empty();
	PartList->RequestListRefresh();
	ShipList->Reset();
	ShipList->SetVisibility(EVisibility::Collapsed);

	OwnedList->Reset();
	OwnedList->SetVisibility(EVisibility::Collapsed);

	ComplexList->ClearChildren();
	ShipyardList->ClearChildren();
	FactoryList->ClearChildren();
	UpgradeBox->ClearChildren();

	TargetSpacecraft = NULL;
	TargetDescription = NULL;
	RCSDescription = NULL;
	EngineDescription = NULL;
	CurrentlySelectedWhiteList = NULL;

	SelectedComplexStation = NAME_None;
	SelectedComplexConnector = NAME_None;

	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareShipMenu::LoadTargetSpacecraft()
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		// Make the right boxes visible
		if (!CanEdit)
		{
			ObjectActionMenu->SetSpacecraft(TargetSpacecraft);
			ObjectActionMenu->Show();
		}

		ObjectName->SetVisibility(EVisibility::Visible);
		ShipPartCustomizationBox->SetVisibility(EVisibility::Collapsed);
		PartCharacteristicBox->SetVisibility(EVisibility::Collapsed);
		ShipCustomizationBox->SetVisibility(EVisibility::Visible);
		ShipList->SetVisibility(CanEdit ? EVisibility::Collapsed : EVisibility::Visible);

		OwnedList->SetVisibility(CanEdit ? EVisibility::Collapsed : EVisibility::Visible);
		FactoryList->SetVisibility(CanEdit ? EVisibility::Collapsed : EVisibility::Visible);
		ShipyardList->SetVisibility(CanEdit ? EVisibility::Collapsed : EVisibility::Visible);
		WhiteListOptions.Empty();

		if (TargetSpacecraft->GetCompany()->IsPlayerCompany())
		{
			if (TargetSpacecraft->GetCompany()->GetWhiteLists().Num() > 0)
			{
				WhiteListSelectionBox->SetVisibility(CanEdit ? EVisibility::Collapsed : EVisibility::Visible);
				WhiteListOptions.Reserve(TargetSpacecraft->GetCompany()->GetWhiteLists().Num());
				for (UFlareCompanyWhiteList* WhiteListEntry : TargetSpacecraft->GetCompany()->GetWhiteLists())
				{
					WhiteListOptions.Add(WhiteListEntry);
				}
				WhiteListDropBox->RefreshOptions();
				if (TargetSpacecraft->GetSelectedWhiteList())
				{
					WhiteListDropBox->SetSelectedItem(TargetSpacecraft->GetSelectedWhiteList());
				}
				else
				{
					WhiteListDropBox->SetSelectedIndex(0);
				}
			}
			else
			{
				WhiteListSelectionBox->SetVisibility(EVisibility::Collapsed);
			}

			if (TargetSpacecraft->IsComplex())
			{
				ObjectProductionBreakdown->SetVisibility(EVisibility::Visible);
			}
			else
			{
				ObjectProductionBreakdown->SetVisibility(EVisibility::Collapsed);
			}

			if (TargetSpacecraft->IsShipyard())
			{
				ExternalOrdersConfigurationButton->SetVisibility(CanEdit ? EVisibility::Collapsed : EVisibility::Visible);

				if (TargetSpacecraft->GetDescription()->IsDroneCarrier)
				{
					AllowAutoConstructionButton->SetVisibility(CanEdit ? EVisibility::Collapsed : EVisibility::Visible);
					if (TargetSpacecraft->GetCompany()->IsTechnologyUnlocked("auto-trade"))
					{
						AllowExternalOrdersButton->SetVisibility(CanEdit ? EVisibility::Collapsed : EVisibility::Visible);
					}
				}
				else
				{
					AllowAutoConstructionButton->SetVisibility(EVisibility::Collapsed);
					AllowExternalOrdersButton->SetVisibility(CanEdit ? EVisibility::Collapsed : EVisibility::Visible);
				}
			}
			else
			{
				ExternalOrdersConfigurationButton->SetVisibility(EVisibility::Collapsed);
				AllowExternalOrdersButton->SetVisibility(EVisibility::Collapsed);
				AllowAutoConstructionButton->SetVisibility(EVisibility::Collapsed);
			}

			// Renaming
			bool CanRename = !TargetSpacecraft->IsStation() && !CanEdit;
			if (CanRename)
			{
				RenameBox->SetVisibility(EVisibility::Visible);
				ShipName->SetText(TargetSpacecraft->GetNickName());
			}
			else
			{
				RenameBox->SetVisibility(EVisibility::Collapsed);
			}
		}
		else
		{
			ObjectProductionBreakdown->SetVisibility(EVisibility::Collapsed);
			ExternalOrdersConfigurationButton->SetVisibility(EVisibility::Collapsed);
			AllowExternalOrdersButton->SetVisibility(EVisibility::Collapsed);
			AllowAutoConstructionButton->SetVisibility(EVisibility::Collapsed);
			RenameBox->SetVisibility(EVisibility::Collapsed);
			WhiteListSelectionBox->SetVisibility(EVisibility::Collapsed);
		}

		// Get the description data
		UFlareSpacecraftComponentsCatalog* Catalog = PC->GetGame()->GetShipPartsCatalog();
		const FFlareSpacecraftDescription* ShipDesc = PC->GetGame()->GetSpacecraftCatalog()->Get(TargetSpacecraftData->Identifier);
		if (ShipDesc)
		{
			// Name
			FText Prefix = TargetSpacecraft->IsStation() ? LOCTEXT("Station", "Station") : LOCTEXT("Ship", "Ship");
			FText Suffix;
			FText NameText;

			if (TargetSpacecraft->GetShipMaster())
			{
				Suffix = FText::Format(LOCTEXT("ShipMaster", "\n\nMaster ship: {0}"), UFlareGameTools::DisplaySpacecraftName(TargetSpacecraft->GetShipMaster()));
			}

			NameText = FText::Format(LOCTEXT("NameText", "{0}{1}"), Prefix, Suffix);
			ObjectName->SetText(NameText);
			ObjectClassName->SetText(ShipDesc->Name);

//			FString PhaseOneEmitters = ShipDesc->Identifier.ToString();
//			ObjectClassName->SetText(FText::FromString(PhaseOneEmitters));

			// Description
			FText SpacecraftDescription = ShipDesc->Description;
			ObjectDescription->SetText(SpacecraftDescription);
			
			// Show the ship if it's not a substation
			if (!ShipDesc->IsSubstation)
			{
				PC->GetMenuPawn()->ShowShip(TargetSpacecraft);
			}
		}

		// Setup weapon descriptions
		WeaponDescriptions.Empty();
		for (int32 GroupIndex = 0; GroupIndex < ShipDesc->WeaponGroups.Num(); GroupIndex++)
		{
			FName SlotName = UFlareSimulatedSpacecraftWeaponsSystem::GetSlotIdentifierFromWeaponGroupIndex(ShipDesc, GroupIndex);

			for (int32 i = 0; i < TargetSpacecraftData->Components.Num(); i++)
			{
				FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(TargetSpacecraftData->Components[i].ComponentIdentifier);
				if (ComponentDescription->Type == EFlarePartType::Weapon && TargetSpacecraftData->Components[i].ShipSlotIdentifier == SlotName)
				{
					FCHECK(ComponentDescription);
					WeaponDescriptions.Add(ComponentDescription);
					break;
				}
			}
		}

		// Setup engine descriptions
		for (int32 i = 0; i < TargetSpacecraftData->Components.Num(); i++)
		{
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(TargetSpacecraftData->Components[i].ComponentIdentifier);
			if (ComponentDescription->Type == EFlarePartType::RCS)
			{
				RCSDescription = Catalog->Get(ComponentDescription->Identifier);
			}
			else if (ComponentDescription->Type == EFlarePartType::OrbitalEngine)
			{
				EngineDescription = Catalog->Get(ComponentDescription->Identifier);
			}
		}
		
		// Add a button for each weapon group
		WeaponButtonBox->ClearChildren();
		for (int32 i = 0; i < WeaponDescriptions.Num(); i++)
		{
			TSharedPtr<int32> IndexPtr(new int32(i));

			WeaponButtonBox->AddSlot()
				.AutoWidth()
				.Padding(FFlareStyleSet::GetDefaultTheme().TitleButtonPadding)
				.VAlign(VAlign_Top)
				[
					SNew(SFlareRoundButton)
					.OnClicked(this, &SFlareShipMenu::ShowWeapons, IndexPtr)
					.Icon(this, &SFlareShipMenu::GetWeaponIcon, IndexPtr)
					.Text(this, &SFlareShipMenu::GetWeaponText, IndexPtr)
					.HelpText(LOCTEXT("WeaponInfo", "Inspect this weapon group"))
					.InvertedBackground(true)
					.HighlightColor(this, &SFlareShipMenu::GetWeaponHealthColor)
				];
		}
		WeaponButtonBox->SetVisibility(WeaponDescriptions.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed);
	}
}

void SFlareShipMenu::LoadPart(FName InternalName)
{
	// Spawn the part
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		const FFlareSpacecraftComponentDescription* PartDesc = PC->GetGame()->GetShipPartsCatalog()->Get(InternalName);
		if (PartDesc)
		{
			// Show part
			ObjectClassName->SetText(PartDesc->Name);
			ObjectDescription->SetText(PartDesc->Description);
			PC->GetMenuPawn()->ShowPart(PartDesc);

			// Build info
			SFlarePartInfo::BuildInfoBlock(PartCharacteristicBox, PartDesc);
		}
	}

	// Make the right box visible
	ObjectActionMenu->Hide();
	ObjectProductionBreakdown->SetVisibility(EVisibility::Collapsed);
	ShipList->SetVisibility(EVisibility::Collapsed);
	OwnedList->SetVisibility(EVisibility::Collapsed);
	ObjectName->SetVisibility(EVisibility::Collapsed);
	ObjectDescription->SetVisibility(EVisibility::Visible);
	ShipPartCustomizationBox->SetVisibility(EVisibility::Visible);
	PartCharacteristicBox->SetVisibility(EVisibility::Visible);
	ShipCustomizationBox->SetVisibility(EVisibility::Collapsed);
	CantUpgradeReason->SetVisibility(EVisibility::Collapsed);
	ExternalOrdersConfigurationButton->SetVisibility(EVisibility::Collapsed);
	AllowExternalOrdersButton->SetVisibility(EVisibility::Collapsed);
	AllowAutoConstructionButton->SetVisibility(EVisibility::Collapsed);
	FactoryList->SetVisibility(EVisibility::Collapsed);
	ShipyardList->SetVisibility(EVisibility::Collapsed);

	// Boxes depending on edit mode
	UpgradeTitle->SetVisibility(CanEdit ? EVisibility::Visible : EVisibility::Collapsed);
	if (!CanEdit)
	{
		BuyConfirmation->Hide();
	}
}

void SFlareShipMenu::UpdateShipyard()
{
	UpdateShipyardList();
}


/*----------------------------------------------------
	UI updates
----------------------------------------------------*/

void SFlareShipMenu::UpdatePartList(FFlareSpacecraftComponentDescription* SelectItem)
{
	ShipPartPickerTitle->SetVisibility(CanEdit ? EVisibility::Visible : EVisibility::Collapsed);
	PartList->SetVisibility(CanEdit ? EVisibility::Visible : EVisibility::Collapsed);
	PartListDataShared.Empty();
	PartList->RequestListRefresh();

	if (CanEdit)
	{
		FLOGV("SFlareShipMenu::UpdatePartList : looking for %s", *SelectItem->Name.ToString());

		int32 Index = INDEX_NONE;

		// Copy items
		for (FFlareSpacecraftComponentDescription* Part : PartListData)
		{
			PartListDataShared.AddUnique(FInterfaceContainer::New(Part));
			if (Part == SelectItem)
			{
				Index = PartListDataShared.Num() - 1;
			}
		}

		ShipPartIndex = Index;
		CurrentPartIndex = Index;
		CurrentEquippedPartIndex = Index;
		CurrentEquippedPartDescription = SelectItem;

		// Update list
		PartList->RequestListRefresh();
		if (PartListDataShared.Num())
		{
			if (Index != INDEX_NONE)
			{
				PartList->SetSelection(PartListDataShared[Index]);
			}
			else
			{
				PartList->SetSelection(PartListDataShared[0]);
			}
		}
	}

	LoadPart(SelectItem->Identifier);
}

void SFlareShipMenu::UpdateProductionBreakdown()
{
	if (TargetSpacecraft)
	{
		if (TargetSpacecraft->IsComplex() && !TargetSpacecraft->IsUnderConstruction())
		{
			TArray<UFlareFactory*>& Factories = TargetSpacecraft->GetFactories();
			TMap<FName, uint32> ResourceCosts;
			int32 DailyProductionCostEstimate = 0;
			FString ResourcesString;

			bool IsProducing = false;
			float Efficiency = TargetSpacecraft->GetStationEfficiency();
			int DurationMalus = FMath::RoundToInt(UFlareFactory::GetProductionMalus(Efficiency));
			const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
			for (int FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
			{
				UFlareFactory* Factory = TargetSpacecraft->GetFactories()[FactoryIndex];
				if (Factory->IsActive())
				{
					uint32 CycleProductionCost = Factory->GetProductionCost();
					int64 ProductionTime = Factory->GetDescription()->CycleCost.ProductionTime;
					FText ProductionTimeDisplay = UFlareGameTools::FormatDate(Factory->GetDescription()->CycleCost.ProductionTime, 2);

					DailyProductionCostEstimate += CycleProductionCost / (ProductionTime * DurationMalus);
					for (int32 ReservedResourceIndex = 0; ReservedResourceIndex < Factory->GetReservedResources().Num(); ReservedResourceIndex++)
					{
						FName ResourceIdentifier = Factory->GetReservedResources()[ReservedResourceIndex].ResourceIdentifier;
						int32 Quantity = Factory->GetReservedResources()[ReservedResourceIndex].Quantity;
						if (ResourceCosts.Contains(ResourceIdentifier))
						{
							ResourceCosts[ResourceIdentifier] -= (Quantity / (ProductionTime * DurationMalus));
						}
						else
						{
							ResourceCosts.Add(ResourceIdentifier, 0 - (Quantity / (ProductionTime * DurationMalus)));
						}
						IsProducing = true;
					}

					TArray<FFlareFactoryResource> OutputResources = Factory->GetLimitedOutputResources();
					for (int32 ResourceIndex = 0; ResourceIndex < OutputResources.Num(); ResourceIndex++)
					{
						const FFlareFactoryResource* Resource = &OutputResources[ResourceIndex];
						FName ResourceIdentifier = Resource->Resource->Data.Identifier;
						if (ResourceCosts.Contains(ResourceIdentifier))
						{
							ResourceCosts[ResourceIdentifier] += (Resource->Quantity / (ProductionTime * DurationMalus));
						}
						else
						{
							ResourceCosts.Add(ResourceIdentifier, (Resource->Quantity / (ProductionTime * DurationMalus)));
						}
						IsProducing = true;
					}
				}

			}

			FString FormattedNumber;
			if (DailyProductionCostEstimate > 0)
			{
				FormattedNumber = FString::FormatAsNumber(UFlareGameTools::DisplayMoney(DailyProductionCostEstimate));
				ResourcesString += FString::Printf(TEXT("-%s credits"), *FormattedNumber);
				IsProducing = true;
			}

			TArray<FName> Keys;
			ResourceCosts.GetKeys(Keys);
			for (int32 CostIndex = 0; CostIndex < Keys.Num(); CostIndex++)
			{
				FName CurrentKey = Keys[CostIndex];
				int32 Quantity = ResourceCosts[CurrentKey];
				FString FormattedQuantity;
				if (Quantity > 0)
				{
					FormattedQuantity = FString::FormatAsNumber(Quantity);
				}
				else
				{
					FormattedQuantity = FString::Printf(TEXT("%i"), Quantity); // FString needed here
				}
				FFlareResourceDescription* RealResource = MenuManager->GetGame()->GetResourceCatalog()->Get(CurrentKey);

				if (Quantity > 0)
				{
					ResourcesString += FString::Printf(TEXT(", +%s %s"), *FormattedQuantity, *RealResource->Name.ToString()); // FString needed here
				}
				else
				{
					ResourcesString += FString::Printf(TEXT(", %s %s"), *FormattedQuantity, *RealResource->Name.ToString()); // FString needed here
				}
			}

			if (IsProducing)
			{
				ObjectProductionBreakdown->SetText(FText::Format(LOCTEXT("StationComplexAggregate", "Estimated daily production:\n{0}"),
				FText::FromString(ResourcesString)));
			}
			else
			{
				ObjectProductionBreakdown->SetText(FText::FText());
			}
		}
	}
}

void SFlareShipMenu::UpdateFactoryList()
{
	FactoryList->ClearChildren();

	// Iterate on all factories
	if (TargetSpacecraft)
	{
		TArray<UFlareFactory*>& Factories = TargetSpacecraft->GetFactories();
		for (int FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
		{
			FactoryList->AddSlot()
			[
				SNew(SFlareFactoryInfo)
				.Factory(Factories[FactoryIndex])
				.MenuManager(MenuManager)
				.Visibility(this, &SFlareShipMenu::GetFactoryControlsVisibility)
			];
		}
	}
}

void SFlareShipMenu::UpdateUpgradeBox()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UpgradeBox->ClearChildren();

	if (!TargetSpacecraft
	 || !TargetSpacecraft->IsStation()
	 || TargetSpacecraft->GetCompany() != MenuManager->GetPC()->GetCompany())
	{
		return;
	}

	// Upgrade title
	UpgradeBox->AddSlot()
	.AutoHeight()
	.Padding(Theme.TitlePadding)
	[
		SNew(STextBlock)
		.TextStyle(&Theme.SubTitleFont)
		.Text(FText::Format(LOCTEXT("UpgradeTitleFormat", "Upgrade ({0}/{1})"),
			FText::AsNumber(TargetSpacecraft->GetLevel()),
			FText::AsNumber(TargetSpacecraft->GetDescription()->MaxLevel)))
	];

	// Look for upgrade
	bool IsBeingUpgraded = TargetSpacecraft->IsUnderConstruction(true) && TargetSpacecraft->GetLevel() > 1;
	if (!IsBeingUpgraded && TargetSpacecraft->IsComplex())
	{
		for (UFlareSimulatedSpacecraft* Substation : TargetSpacecraft->GetComplexChildren())
		{
			if (Substation->IsUnderConstruction(true) && Substation->GetLevel() > 1)
			{
				IsBeingUpgraded = true;
				break;
			}
		}
	}

	// Cancel upgrade
	if (IsBeingUpgraded)
	{
		if (TargetSpacecraft->GetConstructionCargoBay()->GetUsedCargoSpace() == 0)
		{
			UpgradeBox->AddSlot()
			.AutoHeight()
			.Padding(Theme.TitlePadding)
			.HAlign(HAlign_Left)
			[
				SNew(SFlareButton)
				.Text(LOCTEXT("ShipCancelUpgradeFly", "Cancel upgrade"))
				.HelpText(LOCTEXT("ShipCancelUpgradeFlyInfo", "Cancel the upgrade process for this station"))
				.OnClicked(this, &SFlareShipMenu::OnCancelUpgrade)
				.Width(12)
			];
		}
		else
		{
			UpgradeBox->AddSlot()
			.AutoHeight()
			.Padding(Theme.TitlePadding)
			.HAlign(HAlign_Left)
			[
				SNew(SFlareButton)
				.Text(LOCTEXT("ShipCancelUpgradeFly", "Cancel upgrade"))
				.HelpText(LOCTEXT("CantShipCancelUpgradeFlyInfo", "Empty the cargo bays to allow cancelling the upgrade process"))
				.IsDisabled(true)
				.Width(12)
			];
		}
	}

	// Max level
	else if (TargetSpacecraft->GetLevel() >= TargetSpacecraft->GetDescription()->MaxLevel)
	{
		UpgradeBox->AddSlot()
		.AutoHeight()
		.Padding(Theme.TitlePadding)
		.HAlign(HAlign_Left)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(LOCTEXT("MaxLevelInfo", "This station has reached the maximum level."))
		];
	}

	// Upgrade
	else
	{
		UpgradeBox->AddSlot()
		.AutoHeight()
		.Padding(Theme.TitlePadding)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.TextFont)
			.Text(LOCTEXT("CurrentLevelInfo", "Levels act as a multiplier to all station characteristics - a level 2 station acts like two level 1 stations."))
		];
		
		UFlareSimulatedSpacecraft* KeepMenuTarget = NULL;
		
		// Upgrade button
		UpgradeBox->AddSlot()
		.AutoHeight()
		.Padding(Theme.TitlePadding)
		.HAlign(HAlign_Left)
		[
			SNew(SFlareButton)
			.Width(12)
			.Text(GetUpgradeInfo(TargetSpacecraft))
			.HelpText(GetUpgradeHelpInfo(TargetSpacecraft))
			.Icon(FFlareStyleSet::GetIcon("Travel"))
			.OnClicked(this, &SFlareShipMenu::OnUpgradeStationClicked, KeepMenuTarget)
			.IsDisabled(this, &SFlareShipMenu::IsUpgradeStationDisabled, KeepMenuTarget)
		];
	}
}


/*----------------------------------------------------
	Complex UI
----------------------------------------------------*/

void SFlareShipMenu::UpdateComplexList()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Reset
	ComplexList->ClearChildren();
	SelectedComplexStation = NAME_None;
	SelectedComplexConnector = NAME_None;
	
	if (TargetSpacecraft && TargetSpacecraft->IsComplex())
	{
		// Complex is under construction, can't build
		if (TargetSpacecraft->IsUnderConstruction())
		{
			ComplexList->AddSlot()
			.Padding(Theme.SmallContentPadding)
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(&Theme.NearInvisibleBrush)
				.Padding(Theme.SmallContentPadding)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(SImage)
						.Image(FFlareStyleSet::GetIcon("Build"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.WrapTextAt(Theme.ContentWidth)
						.Text(LOCTEXT("AddComplexConstruction", "Complete construction of this station to enable building more station elements"))
					]
				]
			];

			ComplexLayout->SetVisibility(EVisibility::Collapsed);
		}

		// Complex is ready
		else
		{
			ComplexLayout->SetVisibility(EVisibility::Visible);

			int32 ConnectorNumber = 1;
			for (FFlareDockingInfo& Connector : TargetSpacecraft->GetStationConnectors())
			{
				// Existing element - Granted in this context means a station is there (Occupied means active)
				if (Connector.Granted)
				{
					UFlareSimulatedSpacecraft* ComplexElement = TargetSpacecraft->GetCompany()->FindChildStation(Connector.ConnectedStationName);
					FCHECK(ComplexElement);
					FFlareSpacecraftDescription* ComplexElementDesc = ComplexElement->GetDescription();
					FCHECK(ComplexElementDesc);
					
					// Add structure of the station info
					TSharedPtr<SHorizontalBox> Box;
					ComplexList->AddSlot()
					.AutoHeight()
					.HAlign(HAlign_Left)
					.Padding(Theme.SmallContentPadding)
					[
						SAssignNew(Box, SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SFlareButton)
							.Text(FText::Format(LOCTEXT("UpgradeComplexFormat", "Lv {0} {1}"), FText::AsNumber(ComplexElement->GetLevel()), ComplexElementDesc->Name))
							.HelpText(ComplexElementDesc->Description)
							.IsDisabled(true)
							.Width(6)
						]
					];

					// Add leveling info
					if (ComplexElement->GetLevel() >= ComplexElement->GetDescription()->MaxLevel)
					{
						Box->AddSlot()
						.AutoWidth()
						[
							SNew(SFlareButton)
							.Text(LOCTEXT("MaxLevelInfo", "This station has reached the maximum level."))
							.IsDisabled(true)
							.Width(10)
						];
					}
					else
					{
						Box->AddSlot()
						.AutoWidth()
						[
							SNew(SFlareButton)
							.Text(GetUpgradeInfo(ComplexElement))
							.HelpText(GetUpgradeHelpInfo(ComplexElement))
							.OnClicked(this, &SFlareShipMenu::OnUpgradeStationClicked, ComplexElement)
							.IsDisabled(this, &SFlareShipMenu::IsUpgradeStationDisabled, ComplexElement)
							.Width(10)
						];
					}

					// Add scrap button
					Box->AddSlot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Text(LOCTEXT("Scrap", "Scrap"))
						.HelpText(LOCTEXT("ScrapComplexStationInfo", "Scrap this station element"))
						.OnClicked(this, &SFlareShipMenu::OnScrapComplexElement, ComplexElement)
						.Width(2)
					];
				}

				// New element can be added here
				else
				{
					FText NewSlotSpecialText;
					if (UFlareSimulatedSpacecraft::IsSpecialComplexSlot(Connector.Name))
					{
						NewSlotSpecialText = LOCTEXT("AddComplexStationSpecial", "(Central slot)");
					}

					FText NewSlotText = FText::Format(LOCTEXT("AddComplexStationFormat", "({0}) Add a new station element to the complex {1}"),
						FText::AsNumber(ConnectorNumber),
						NewSlotSpecialText);

					ComplexList->AddSlot()
					.AutoHeight()
					.HAlign(HAlign_Left)
					.Padding(Theme.SmallContentPadding)
					[
						SNew(SFlareButton)
						.Text(NewSlotText)
						.HelpText(LOCTEXT("AddComplexStationInfo", "Build a new station element on this complex"))
						.OnClicked(this, &SFlareShipMenu::OnBuildElementClicked, Connector.Name)
						.Width(18)
					];
				}

				ConnectorNumber++;
			}
		}
	}
	else
	{
		ComplexLayout->SetVisibility(EVisibility::Collapsed);
	}
}

void SFlareShipMenu::OnBuildElementClicked(FName ConnectorName)
{
	SelectedComplexStation = TargetSpacecraft->GetImmatriculation();
	SelectedComplexConnector = ConnectorName;

	FFlareMenuParameterData Data;
	Data.Spacecraft = TargetSpacecraft;
	Data.ComplexConnectorName = ConnectorName;
	MenuManager->OpenSpacecraftOrder(Data, FOrderDelegate::CreateSP(this, &SFlareShipMenu::OnBuildStationSelected));
}

void SFlareShipMenu::OnBuildStationSelected(FFlareSpacecraftDescription* StationDescription)
{
	if (StationDescription && TargetSpacecraft)
	{
		// Can we build ?
		TArray<FText> Reasons;
		FString ResourcesString;
		UFlareFleet* PlayerFleet = MenuManager->GetPC()->GetPlayerFleet();
		UFlareSimulatedSector* TargetSector = TargetSpacecraft->GetCurrentSector();

		// Build !
		FFlareStationSpawnParameters SpawnParameters;
		SpawnParameters.AttachComplexStationName = SelectedComplexStation;
		SpawnParameters.AttachComplexConnectorName = SelectedComplexConnector;
		UFlareSimulatedSpacecraft* NewStation = TargetSector->BuildStation(StationDescription, MenuManager->GetPC()->GetCompany(), SpawnParameters);
		
		// Handle menus
		if (PlayerFleet && PlayerFleet->GetCurrentSector() == TargetSector && MenuManager->GetPC()->GetPlayerShip())
		{
			FFlareMenuParameterData MenuParameters;
			MenuParameters.Spacecraft = MenuManager->GetPC()->GetPlayerShip();
			MenuParameters.Sector = TargetSector;
			MenuManager->OpenMenu(EFlareMenu::MENU_ReloadSector, MenuParameters);
		}
		else
		{
			FFlareMenuParameterData MenuParameters;
			MenuParameters.Sector = TargetSector;
			MenuManager->OpenMenu(EFlareMenu::MENU_Sector, MenuParameters);
		}

		// Notify
		FFlareMenuParameterData NotificationParameters;
		NotificationParameters.Spacecraft = NewStation;
		MenuManager->GetPC()->Notify(
			LOCTEXT("ComplexStationBuilt", "Complex expanded"),
			LOCTEXT("ComplexStationBuiltInfo", "A new station element has been added to your complex."),
			"complex-station-built",
			EFlareNotification::NT_Economy,
			false,
			EFlareMenu::MENU_Station,
			NotificationParameters);

		SelectedComplexStation = NAME_None;
		SelectedComplexConnector = NAME_None;
	}
}

void SFlareShipMenu::OnUpgradeStationClicked(UFlareSimulatedSpacecraft* Spacecraft)
{
	UFlareSimulatedSector* Sector = TargetSpacecraft->GetCurrentSector();
	if (Sector)
	{
		if (Spacecraft)
		{
			Sector->UpgradeStation(Spacecraft);
			MenuManager->Reload();
		}
		else if (TargetSpacecraft)
		{
			Sector->UpgradeStation(TargetSpacecraft);
			FFlareMenuParameterData Data;
			Data.Spacecraft = TargetSpacecraft;
			MenuManager->OpenMenu(EFlareMenu::MENU_Station, Data);
		}
	}
}

void SFlareShipMenu::OnScrapComplexElement(UFlareSimulatedSpacecraft* Spacecraft)
{
	TMap<FFlareResourceDescription*, int32> ScrapResources = Spacecraft->ComputeScrapResources();
	TMap<FFlareResourceDescription*, int32> NotDistributedScrapResources = Spacecraft->GetCurrentSector()->DistributeResources(ScrapResources, Spacecraft, Spacecraft->GetCompany(), true);

	auto GenerateResourceList = [](TMap<FFlareResourceDescription*, int32>& Resources)
	{
		bool First = true;
		FText Text;
		for(auto Resource: Resources)
		{
			if(First)
			{
				First = false;
				Text = FText::Format(LOCTEXT("FirstResource", "{0} {1}"), Resource.Value, Resource.Key->Name);
			}
			else
			{
				Text = FText::Format(LOCTEXT("NotFirstResource", "{0}, {1} {2}"), Text, Resource.Value, Resource.Key->Name);
			}
		}

		return Text;

	};

	FText GainText = GenerateResourceList(ScrapResources);
	FText LossesText;

	if(NotDistributedScrapResources.Num() > 0)
	{
		LossesText = FText::Format(LOCTEXT("LooseOnScrap", "\nThere is not enough space in your local stations and ships to store all resources.\n You will lose {0}."),
					  GenerateResourceList(NotDistributedScrapResources));
	}


	MenuManager->Confirm(LOCTEXT("AreYouSure", "ARE YOU SURE ?"),
		FText::Format(LOCTEXT("ConfirmScrap", "Do you really want to break up this station for its resources ?\nYou will get {0}{1}"),
					  GainText,
					  LossesText),
		FSimpleDelegate::CreateSP(this, &SFlareShipMenu::OnScrapConfirmed, Spacecraft));
}

void SFlareShipMenu::OnScrapConfirmed(UFlareSimulatedSpacecraft* Spacecraft)
{
	MenuManager->GetGame()->ScrapStation(Spacecraft);
	MenuManager->Reload();
}


/*----------------------------------------------------
	Shipyards
----------------------------------------------------*/

void SFlareShipMenu::UpdateShipyardList()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	ShipyardList->ClearChildren();

	if (TargetSpacecraft && TargetSpacecraft->IsShipyard())
	{
		// Currently building
		int32 Index = 0;
		UFlareSpacecraftCatalog* SpacecraftCatalog = MenuManager->GetGame()->GetSpacecraftCatalog();
		for (FFlareShipyardOrderSave& Order : TargetSpacecraft->GetOngoingProductionList())
		{
			FFlareSpacecraftDescription* OrderDescription = SpacecraftCatalog->Get(Order.ShipClass);
			UFlareCompany* OrderCompany = MenuManager->GetGame()->GetGameWorld()->FindCompany(Order.Company);

			FText Duration;
			if(OrderCompany ==  MenuManager->GetPC()->GetCompany())
			{
				Duration = FText::Format(LOCTEXT("ShipInQueueDurationFormat","for {0} days "), Order.RemainingProductionDuration);
			}

			ShipyardList->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			.Padding(Theme.SmallContentPadding)
			[
				SNew(SHorizontalBox)

				// Status
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(FText::Format(LOCTEXT("ShipInProductionFormat", "\u2022 In production {0}: {1} for {2}"),
						Duration,
						OrderDescription->Name,
						OrderCompany->GetCompanyName()))
					.WrapTextAt(Theme.ContentWidth)
				]
			];

			Index++;
		}

		// Iterate on production queue
		Index = 0;
		for (FFlareShipyardOrderSave& Order : TargetSpacecraft->GetShipyardOrderQueue())
		{
			ShipyardList->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			.Padding(Theme.SmallContentPadding)
			[
				SNew(SHorizontalBox)

				// Status
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(0.85 * Theme.ContentWidth)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareShipMenu::GetShipOrderStatus, Index)
						.WrapTextAt(0.8 * Theme.ContentWidth)
					]
				]

				// Cancel
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SFlareButton)
					.Width(2)
					.Text(LOCTEXT("CancelShip", "Cancel"))
					.HelpText(LOCTEXT("CancelShipInfo", "Remove this ship order from the production queue"))
					.OnClicked(this, &SFlareShipMenu::OnCancelSpacecraftOrder, Index)
					.Visibility(this, &SFlareShipMenu::GetCancelShipOrderVisibility, Index)
					.Transparent(true)
					.Small(true)
					.Icon(FFlareStyleSet::GetIcon("Stop"))
				]
			];

			Index++;
		}

		AllowExternalOrdersButton->SetActive(TargetSpacecraft->IsAllowExternalOrder());
		AllowAutoConstructionButton->SetActive(TargetSpacecraft->IsAllowAutoConstruction());
	}
}

EVisibility SFlareShipMenu::GetShipyardVisibility() const
{
	if (TargetSpacecraft && TargetSpacecraft->IsShipyard())
	{
		if (TargetDescription && TargetDescription->IsDroneCarrier)
		{
			return EVisibility::Collapsed;
		}

		AFlarePlayerController* PC = MenuManager->GetPC();
		if (PC)
		{
			if (TargetSpacecraft->GetCompany() != PC->GetCompany())
			{
				if (TargetSpacecraft->IsPlayerHostile())
				{
					return EVisibility::Collapsed;
				}
			}
		}

		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SFlareShipMenu::GetShipyardAllowExternalOrderConfigurationVisibility() const
{
	if (TargetSpacecraft && TargetSpacecraft->IsShipyard() && TargetSpacecraft->GetCompany() == MenuManager->GetPC()->GetCompany())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

EVisibility SFlareShipMenu::GetShipyardAllowExternalOrderVisibility() const
{
	if (TargetSpacecraft && TargetSpacecraft->IsShipyard() && TargetSpacecraft->GetCompany() == MenuManager->GetPC()->GetCompany())
	{
		if (TargetSpacecraft->GetDescription()->IsDroneCarrier)
		{
			if(!TargetSpacecraft->GetCompany()->IsTechnologyUnlocked("auto-trade"))
			{
				return EVisibility::Collapsed;
			}
		}
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

EVisibility SFlareShipMenu::GetAutoConstructionVisibility() const
{
	if (TargetSpacecraft && TargetSpacecraft->IsShipyard() && TargetSpacecraft->GetCompany() == MenuManager->GetPC()->GetCompany() && TargetSpacecraft->GetDescription()->IsDroneCarrier)
	{
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

bool SFlareShipMenu::IsShipSSelectorDisabled() const
{
	if (TargetSpacecraft && TargetSpacecraft->IsShipyard())
	{
		AFlarePlayerController* PC = MenuManager->GetPC();
		if (PC && TargetSpacecraft->IsPlayerHostile())
		{
			return true;
		}

		for (FFlareShipyardOrderSave& Order : TargetSpacecraft->GetShipyardOrderQueue())
		{
			if (Order.Company == MenuManager->GetPC()->GetCompany()->GetIdentifier())
			{
				FFlareSpacecraftDescription* OrderShip = MenuManager->GetGame()->GetSpacecraftCatalog()->Get(Order.ShipClass);

				if(OrderShip->Size == EFlarePartSize::S)
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool SFlareShipMenu::IsShipLSelectorDisabled() const
{
	if (TargetSpacecraft && TargetSpacecraft->IsShipyard())
	{
		AFlarePlayerController* PC = MenuManager->GetPC();
		if (PC && TargetSpacecraft->IsPlayerHostile())
		{
			return true;
		}
		for (FFlareShipyardOrderSave& Order : TargetSpacecraft->GetShipyardOrderQueue())
		{
			if (Order.Company == MenuManager->GetPC()->GetCompany()->GetIdentifier())
			{
				FFlareSpacecraftDescription* OrderShip = MenuManager->GetGame()->GetSpacecraftCatalog()->Get(Order.ShipClass);

				if(OrderShip->Size == EFlarePartSize::L)
				{
					return true;
				}
			}
		}
	}

	return false;
}

EVisibility SFlareShipMenu::GetCancelShipOrderVisibility(int32 Index) const
{
	if (TargetSpacecraft && TargetSpacecraft->IsShipyard() && Index < TargetSpacecraft->GetShipyardOrderQueue().Num())
	{
		FFlareShipyardOrderSave& Order = TargetSpacecraft->GetShipyardOrderQueue()[Index];

		if (Order.Company == MenuManager->GetPC()->GetCompany()->GetIdentifier())
		{
			return EVisibility::Visible;
		}
	}

	return EVisibility::Collapsed;
}

FText SFlareShipMenu::GetLightShipTextInfo() const
{
	if (IsShipSSelectorDisabled())
	{
		return LOCTEXT("OrderShipInfoDisabled", "You already have an order in the production queue and can't add a new one");
	}
	else
	{
		return LOCTEXT("OrderLightShipInfo", "Pick a light ship class to build at this shipyard");
	}
}

FText SFlareShipMenu::GetHeavyShipTextInfo() const
{
	if (IsShipLSelectorDisabled())
	{
		return LOCTEXT("OrderShipInfoDisabled", "You already have an order in the production queue and can't add a new one");
	}
	else
	{
		return LOCTEXT("OrderHeavyShipInfo", "Pick a heavy ship class to build at this shipyard");
	}
}


FText SFlareShipMenu::GetShipOrderStatus(int32 Index) const
{
	FText Reason;

	if(Index >= TargetSpacecraft->GetShipyardOrderQueue().Num())
	{
		return FText();
	}


	if (TargetSpacecraft && TargetSpacecraft->IsShipyard() && Index == 0)
	{
		FText Status= TargetSpacecraft->GetNextShipyardOrderStatus();

		Reason = FText::Format(LOCTEXT("ShipOrderStatusFormat", "\n({0})"), Status);
	}


	UFlareSpacecraftCatalog* SpacecraftCatalog = MenuManager->GetGame()->GetSpacecraftCatalog();
	FFlareShipyardOrderSave& Order = TargetSpacecraft->GetShipyardOrderQueue()[Index];

	FFlareSpacecraftDescription* OrderDescription = SpacecraftCatalog->Get(Order.ShipClass);
	UFlareCompany* OrderCompany = MenuManager->GetGame()->GetGameWorld()->FindCompany(Order.Company);

	if(!OrderDescription)
	{
		return FText();
	}


	AFlarePlayerController* PC = MenuManager->GetPC();

	FText Duration;
	if(OrderCompany == PC->GetCompany())
	{
		int32 ProductionTime = TargetSpacecraft->GetShipProductionTime(Order.ShipClass) + TargetSpacecraft->GetEstimatedQueueAndProductionDuration(Order.ShipClass, Index);

		Duration = FText::Format(LOCTEXT("ShipInQueueDurationFormat"," for {0} days"), ProductionTime);
	}


	return FText::Format(LOCTEXT("ShipInQueueFormat", "\u2022 In queue{3}: {0} for {1}{2}"),
							OrderDescription->Name,
							OrderCompany->GetCompanyName(),
							Reason,
							Duration);

}

void SFlareShipMenu::OnOpenSpacecraftOrder(bool IsHeavy)
{
	FFlareMenuParameterData Data;
	Data.SpacecraftOrderConfig = 0;
	Data.Spacecraft = TargetSpacecraft;
	Data.SpacecraftOrderHeavy = IsHeavy;
	MenuManager->OpenSpacecraftOrder(Data, FOrderDelegate());
}

void SFlareShipMenu::OnCancelSpacecraftOrder(int32 Index)
{
	if (TargetSpacecraft && TargetSpacecraft->IsShipyard())
	{
		TargetSpacecraft->CancelShipyardOrder(Index);
		UpdateShipyard();
	}
}

void SFlareShipMenu::OnToggleAllowExternalOrders()
{
	if (TargetSpacecraft && TargetSpacecraft->IsShipyard())
	{
		bool NewStatus = AllowExternalOrdersButton->IsActive();
		TargetSpacecraft->SetAllowExternalOrder(NewStatus);
		UpdateShipyard();
	}
}

void SFlareShipMenu::OnToggleAllowAutoConstruction()
{
	if (TargetSpacecraft && TargetSpacecraft->IsShipyard())
	{
		bool NewStatus = AllowAutoConstructionButton->IsActive();
		TargetSpacecraft->SetAllowAutoConstruction(NewStatus);
	}
}



void SFlareShipMenu::OnOpenExternalConfig()
{
	FFlareMenuParameterData Data;
	Data.Spacecraft = TargetSpacecraft;
	Data.SpacecraftOrderConfig = 1;
	MenuManager->OpenSpacecraftOrder(Data, FOrderDelegate());
}

/*----------------------------------------------------
	Content callbacks
----------------------------------------------------*/

FText SFlareShipMenu::GetShipName() const
{
	return TargetSpacecraft->GetNickName();
}

void SFlareShipMenu::OnSelectWhiteList()
{
	if (CurrentlySelectedWhiteList)
	{
		TargetSpacecraft->SelectWhiteListDefault(CurrentlySelectedWhiteList);
		int32 CurrentlySelectedIndex = WhiteListDropBox->GetSelectedIndex();
		WhiteListDropBox->RefreshOptions();
		WhiteListDropBox->SetSelectedIndex(CurrentlySelectedIndex);
	}
}

void SFlareShipMenu::OnRemoveWhiteList()
{
	TargetSpacecraft->SelectWhiteListDefault(nullptr);
	int32 CurrentlySelectedIndex = WhiteListDropBox->GetSelectedIndex();
	WhiteListDropBox->RefreshOptions();
	WhiteListDropBox->SetSelectedIndex(CurrentlySelectedIndex);
}

bool SFlareShipMenu::IsWhiteListRemoveDisabled() const
{
	if (TargetSpacecraft && TargetSpacecraft->GetSelectedWhiteList())
	{
		return false;
	}
	return true;
}

bool SFlareShipMenu::IsWhiteListSelectDisabled() const
{
	if (TargetSpacecraft && (!TargetSpacecraft->GetSelectedWhiteList() || TargetSpacecraft->GetSelectedWhiteList() && TargetSpacecraft->GetSelectedWhiteList() != CurrentlySelectedWhiteList))
	{
		return false;
	}

	return true;
}

bool SFlareShipMenu::IsRenameDisabled() const
{
	FString ShipNameData = ShipName->GetText().ToString();

	if (ShipNameData.Len() > 15)
	{
		return true;
	}
	if (ShipNameData == TargetSpacecraft->GetNickName().ToString())
	{
		return true;
	}
	else
	{
		return false;
	}
}

const FSlateBrush* SFlareShipMenu::GetTitleIcon() const
{
	if (TargetSpacecraft && TargetSpacecraft->IsStation())
	{
		return AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Station);
	}
	else
	{
		return AFlareMenuManager::GetMenuIcon(CanEdit ? EFlareMenu::MENU_ShipConfig : EFlareMenu::MENU_Ship);
	}
}

FText SFlareShipMenu::GetTitleText() const
{
	if (TargetSpacecraft && TargetSpacecraft->IsStation())
	{
		return  LOCTEXT("StationMenuTitle", "Station");
	}
	else
	{
		return (CanEdit ? LOCTEXT("ShipConfigMenuTitle", "Ship upgrade") : LOCTEXT("ShipMenuTitle", "Ship"));
	}
}

EVisibility SFlareShipMenu::GetEngineVisibility() const
{
	return (TargetSpacecraft && !TargetSpacecraft->IsStation() ? EVisibility::Visible : EVisibility::Collapsed);
}

EVisibility SFlareShipMenu::GetFactoryControlsVisibility() const
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC && TargetSpacecraft && !TargetSpacecraft->IsPlayerHostile())
	{
		return EVisibility::Visible;
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

EVisibility SFlareShipMenu::GetEditVisibility() const
{
	return (CanEdit ? EVisibility::Visible : EVisibility::Collapsed);
}

const FSlateBrush* SFlareShipMenu::GetRCSIcon() const
{
	return (RCSDescription ? &RCSDescription->MeshPreviewBrush : NULL);
}

FText SFlareShipMenu::GetRCSText() const
{
	FText Result;

	if (RCSDescription)
	{
		Result = RCSDescription->Name;
	}

	return Result;
}

FSlateColor SFlareShipMenu::GetRCSHealthColor() const
{
	float ComponentHealth = 1;

	if (TargetSpacecraft)
	{
		ComponentHealth = TargetSpacecraft->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_RCS);
	}

	return FFlareStyleSet::GetHealthColor(ComponentHealth, false);
}

const FSlateBrush* SFlareShipMenu::GetEngineIcon() const
{
	return (EngineDescription ? &EngineDescription->MeshPreviewBrush : NULL);
}

FText SFlareShipMenu::GetEngineText() const
{
	FText Result;

	if (EngineDescription)
	{
		Result = EngineDescription->Name;
	}

	return Result;
}

FSlateColor SFlareShipMenu::GetEnginesHealthColor() const
{
	float ComponentHealth = 1;

	if (TargetSpacecraft)
	{
		ComponentHealth = TargetSpacecraft->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Propulsion);
	}

	return FFlareStyleSet::GetHealthColor(ComponentHealth, false);
}

const FSlateBrush* SFlareShipMenu::GetWeaponIcon(TSharedPtr<int32> Index) const
{
	if (*Index < WeaponDescriptions.Num())
	{
		FFlareSpacecraftComponentDescription* Desc = WeaponDescriptions[*Index];
		return (Desc ? &Desc->MeshPreviewBrush : NULL);
	}
	return NULL;
}

FText SFlareShipMenu::GetWeaponText(TSharedPtr<int32> GroupIndex) const
{
	FText Result;
	FText Comment;

	if (*GroupIndex < WeaponDescriptions.Num())
	{
		FFlareSpacecraftComponentDescription* Desc = WeaponDescriptions[*GroupIndex];
		if (Desc)
		{
			Result = Desc->Name;
		}
	}

	// Get group name
	if (TargetSpacecraft)
	{
		const FFlareSpacecraftDescription* ShipDesc = TargetSpacecraft->GetDescription();

		if (ShipDesc)
		{
			if (ShipDesc->Size == EFlarePartSize::L)
			{
				FCHECK(*GroupIndex >= 0 && *GroupIndex < ShipDesc->WeaponGroups.Num());
				Comment = ShipDesc->WeaponGroups[*GroupIndex].GroupName;
			}
			else
			{
				FCHECK(*GroupIndex >= 0 && *GroupIndex < ShipDesc->WeaponGroups.Num());
				Comment = ShipDesc->WeaponGroups[*GroupIndex].GroupName;
			}
		}
	}

	return FText::Format(LOCTEXT("WeaponTextFormat", "{0}\n({1})"), Result, Comment);
}

FSlateColor SFlareShipMenu::GetWeaponHealthColor() const
{
	float ComponentHealth = 1;

	if (TargetSpacecraft)
	{
		ComponentHealth = TargetSpacecraft->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_Weapon);
	}

	return FFlareStyleSet::GetHealthColor(ComponentHealth, false);
}

TSharedRef<SWidget> SFlareShipMenu::OnGenerateWhiteListComboLine(UFlareCompanyWhiteList* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	const FSlateBrush* WhiteListIcon = NULL;
	if (TargetSpacecraft->GetSelectedWhiteList() == Item)
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

void SFlareShipMenu::OnWhiteListComboLineSelectionChanged(UFlareCompanyWhiteList* Item, ESelectInfo::Type SelectInfo)
{
	CurrentlySelectedWhiteList = Item;
}

TSharedRef<ITableRow> SFlareShipMenu::GeneratePartInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedPtr<SFlarePartInfo> Temp;
	TSharedPtr<SFlareListItem> TempWidget;

	// Create the row
	TSharedRef<ITableRow> res = SAssignNew(TempWidget, SFlareListItem, OwnerTable)
		.Width(5)
		.Height(2)
		.Content()
		[
			SAssignNew(Temp, SFlarePartInfo)
			.Description(Item->PartDescription)
			.ShowOwnershipInfo(true)
		];

	// Update the selection to force select the first item
	int32 Index = PartListData.Find(Item->PartDescription);
	if (Index == CurrentEquippedPartIndex)
	{
		Temp->SetOwned(true);
		TSharedPtr<SFlareListItem> ItemWidget = StaticCastSharedPtr<SFlareListItem>(TempWidget);
		if (ItemWidget.IsValid())
		{
			ItemWidget->SetSelected(true);
			PreviousSelection = ItemWidget;
		}
	}

	return res;
}

FText SFlareShipMenu::GetUpgradeHelpInfo(UFlareSimulatedSpacecraft* Spacecraft)
{
	FText HelpText;
	if (Spacecraft)
	{
		TArray<FText> Reasons;
		UFlareSimulatedSector* Sector = Spacecraft->GetCurrentSector();
		if (Sector)
		{
			Sector->CanUpgradeStation(Spacecraft, Reasons);
		}

		if (Reasons.Num() > 0)
		{
			FString ReasonsString;

			for (int ReasonsIndex = 0; ReasonsIndex < Reasons.Num(); ReasonsIndex++)
			{
				FText TextReason = Reasons[ReasonsIndex];
				if (ReasonsIndex + 1 < Reasons.Num())
				{
					ReasonsString += FString::Printf(TEXT("\u2022%s\n"), *TextReason.ToString());
				}
				else
				{
					ReasonsString += FString::Printf(TEXT("\u2022%s"), *TextReason.ToString());
				}
			}
			HelpText = FText::Format(LOCTEXT("UpgradeHelpInfoFormat", "{0}"),
			FText::FromString(ReasonsString));
		}
		else
		{
			if (Spacecraft->IsComplexElement())
			{
				HelpText = FText(LOCTEXT("UpgradeComplexStationInfo", "Upgrade this station element"));
			}
		}
	}
	return HelpText;
}

FText SFlareShipMenu::GetUpgradeInfo(UFlareSimulatedSpacecraft* Spacecraft)
{
	// Add resources
	FString ResourcesString;
	for (int ResourceIndex = 0; ResourceIndex < Spacecraft->GetDescription()->CycleCost.InputResources.Num(); ResourceIndex++)
	{
		FFlareFactoryResource* FactoryResource = &Spacecraft->GetDescription()->CycleCost.InputResources[ResourceIndex];
		ResourcesString += FString::Printf(TEXT(", %u %s"), FactoryResource->Quantity, *FactoryResource->Resource->Data.Name.ToString()); // FString needed here
	}

	// Final text
	FText ProductionCost = FText::Format(LOCTEXT("UpgradeCostFormat", "Upgrade to level {0} ({1} credits{2})"),
		FText::AsNumber(Spacecraft->GetLevel() + 1),
		FText::AsNumber(UFlareGameTools::DisplayMoney(Spacecraft->GetStationUpgradeFee())),
		FText::FromString(ResourcesString));

	return ProductionCost;
}

bool SFlareShipMenu::IsUpgradeStationDisabled(UFlareSimulatedSpacecraft* Spacecraft) const
{
	AFlarePlayerController* PC = MenuManager->GetPC();

	UFlareSimulatedSector* Sector = TargetSpacecraft->GetCurrentSector();
	if (Sector)
	{
		if (Spacecraft)
		{
			TArray<FText> Reasons;
			return !Sector->CanUpgradeStation(Spacecraft, Reasons);
		}
		else if (TargetSpacecraft)
		{
			TArray<FText> Reasons;
			return !Sector->CanUpgradeStation(TargetSpacecraft, Reasons);
		}
	}

	return true;
}


/*----------------------------------------------------
	Action callbacks
----------------------------------------------------*/

void SFlareShipMenu::OnRename()
{
	TargetSpacecraft->SetNickName(ShipName->GetText());
	MenuManager->GetPC()->OnLoadComplete();
	MenuManager->GetPC()->GetMenuPawn()->UpdateCustomization();
}

void SFlareShipMenu::ShowRCSs()
{
	PartListData.Empty();

	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		FFlareSpacecraftComponentDescription* PartDesc = NULL;
		UFlareSpacecraftComponentsCatalog* Catalog = PC->GetGame()->GetShipPartsCatalog();

		// Browse all the parts in the save until we find the right one
		for (int32 Index = 0; Index < TargetSpacecraftData->Components.Num(); Index++)
		{
			FFlareSpacecraftComponentDescription* Desc = Catalog->Get(TargetSpacecraftData->Components[Index].ComponentIdentifier);
			if (Desc && Desc->Type == EFlarePartType::RCS)
			{
				PartDesc = Desc;
				break;
			}
		}

		Catalog->GetRCSList(PartListData, TargetSpacecraft->GetDescription()->Size, MenuManager->GetPC()->GetCompany(), TargetSpacecraft);
		FLOGV("SFlareShipMenu::ShowRCSs : %d parts", PartListData.Num());
		UpdatePartList(PartDesc);
	}
}

void SFlareShipMenu::ShowEngines()
{
	PartListData.Empty();

	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		FFlareSpacecraftComponentDescription* PartDesc = NULL;
		UFlareSpacecraftComponentsCatalog* Catalog = PC->GetGame()->GetShipPartsCatalog();
		
		// Browse all the parts in the save until we find the right one
		for (int32 Index = 0; Index < TargetSpacecraftData->Components.Num(); Index++)
		{
			FFlareSpacecraftComponentDescription* Desc = Catalog->Get(TargetSpacecraftData->Components[Index].ComponentIdentifier);
			if (Desc && Desc->Type == EFlarePartType::OrbitalEngine)
			{
				PartDesc = Desc;
				break;
			}
		}

		Catalog->GetEngineList(PartListData, TargetSpacecraft->GetDescription()->Size, MenuManager->GetPC()->GetCompany(), TargetSpacecraft);
		FLOGV("SFlareShipMenu::ShowEngines : %d parts", PartListData.Num());
		UpdatePartList(PartDesc);
	}
}

void SFlareShipMenu::ShowWeapons(TSharedPtr<int32> WeaponGroupIndex)
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	FCHECK(PC);

	UFlareSpacecraftComponentsCatalog* Catalog = PC->GetGame()->GetShipPartsCatalog();
	FCHECK(Catalog);

	FFlareSpacecraftDescription* ShipDesc = TargetSpacecraft->GetDescription();
	FCHECK(ShipDesc);

	// Setup data
	PartListData.Empty();
	int32 CurrentSearchIndex = 0;
	CurrentWeaponGroupIndex = *WeaponGroupIndex;
	FFlareSpacecraftComponentDescription* PartDesc = NULL;

	// Browse all the parts in the save until we find the right one
	for (int32 Index = 0; Index < TargetSpacecraftData->Components.Num(); Index++)
	{
		FFlareSpacecraftComponentDescription* Desc = Catalog->Get(TargetSpacecraftData->Components[Index].ComponentIdentifier);
		if (Desc && Desc->Type == EFlarePartType::Weapon)
		{
			FName TargetSlotName = UFlareSimulatedSpacecraftWeaponsSystem::GetSlotIdentifierFromWeaponGroupIndex(ShipDesc, CurrentWeaponGroupIndex);

			if (TargetSpacecraftData->Components[Index].ShipSlotIdentifier == TargetSlotName)
			{
				PartDesc = Desc;
				break;
			}
			else
			{
				CurrentSearchIndex++;
			}
		}
	}

	FFlareSpacecraftSlotGroupDescription* WeaponGroupDesc = &TargetDescription->WeaponGroups[CurrentWeaponGroupIndex];
	Catalog->GetWeaponList(PartListData, TargetDescription->Size, MenuManager->GetPC()->GetCompany(), TargetSpacecraft, WeaponGroupDesc);

	FLOGV("SFlareShipMenu::ShowWeapons : %d parts", PartListData.Num());
	UpdatePartList(PartDesc);
}

void SFlareShipMenu::OnPartPicked(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo)
{
	int32 Index = PartListData.Find(Item->PartDescription);
	AFlarePlayerController* PC = MenuManager->GetPC();
	
	if (PC && Item && Item->PartDescription && Index != CurrentPartIndex)
	{
		AFlareMenuPawn* Viewer = PC->GetMenuPawn();
		RenameBox->SetVisibility(EVisibility::Collapsed);

		// Ensure this part can be changed
		bool CanBeChanged = TargetSpacecraft->CanUpgrade(Item->PartDescription->Type);

		// Load the part
		if (Viewer)
		{
			Viewer->SetSlideDirection(Index > CurrentPartIndex);
			LoadPart(Item->PartDescription->Identifier);
		}
		CurrentPartIndex = Index;
		
		// Show the confirmation dialog
		if (CurrentPartIndex != ShipPartIndex)
		{
			if (CanBeChanged)
			{
				int64 TransactionCost = TargetSpacecraft->GetUpgradeCost(Item->PartDescription, CurrentEquippedPartDescription);
				BuyConfirmation->Show(TransactionCost, PC->GetCompany());
				CantUpgradeReason->SetVisibility(EVisibility::Collapsed);
			}
			else
			{
				BuyConfirmation->Hide();
				if (CanEdit)
				{
					CantUpgradeReason->SetVisibility(EVisibility::Visible);
				}
				else
				{
					CantUpgradeReason->SetVisibility(EVisibility::Collapsed);
				}
			}
		}
		else
		{
			BuyConfirmation->Hide();
			CantUpgradeReason->SetVisibility(EVisibility::Collapsed);
		}
	}

	// De-select old
	if (PreviousSelection.IsValid())
	{
		PreviousSelection->SetSelected(false);
	}

	// Re-select new
	TSharedPtr<SFlareListItem> ItemWidget = StaticCastSharedPtr<SFlareListItem>(PartList->WidgetFromItem(Item));
	if (ItemWidget.IsValid())
	{
		ItemWidget->SetSelected(true);
		PreviousSelection = ItemWidget;
	}
}

void SFlareShipMenu::OnPartConfirmed()
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	FCHECK(PC);
	UFlareSpacecraftComponentsCatalog* Catalog = PC->GetGame()->GetShipPartsCatalog();

	// Edit the correct save data property
	FFlareSpacecraftComponentDescription* NewPartDesc = PartListData[CurrentPartIndex];
	CurrentEquippedPartIndex = CurrentPartIndex;
	CurrentEquippedPartDescription = NewPartDesc;
		
	// Upgrade the ship
	if (TargetSpacecraft)
	{
		if (TargetSpacecraft->GetActive())
		{
			AFlareSpacecraft* ActiveShip = TargetSpacecraft->GetActive();
			if (ActiveShip)
			{
				AFlareSpacecraft* DockedStation = ActiveShip->GetNavigationSystem()->GetDockStation();
				if (DockedStation)
				{
					UFlareSimulatedSpacecraft* StationInterface = DockedStation->GetParent();
					if (StationInterface)
					{
						if (!StationInterface->IsHostile(TargetSpacecraft->GetCompany())
							&& StationInterface->HasCapability(EFlareSpacecraftCapability::Upgrade))
						{
							TargetSpacecraft->UpgradePart(NewPartDesc, CurrentWeaponGroupIndex);
						}
					}
				}
				else
				{
					bool DockingConfirmed = ActiveShip->GetNavigationSystem()->DockAtAndUpgrade(NewPartDesc, CurrentWeaponGroupIndex);
					if (DockingConfirmed && ActiveShip->IsPlayerShip())
					{
						MenuManager->CloseMenu();
					}
				}
			}
		}
		else
		{
			TargetSpacecraft->UpgradePart(NewPartDesc, CurrentWeaponGroupIndex);
		}
	}
	// Get back to the ship config
	BuyConfirmation->Hide();
	LoadTargetSpacecraft();
	MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->InfoSound);
}

void SFlareShipMenu::OnPartCancelled()
{
	BuyConfirmation->Hide();
	LoadTargetSpacecraft();
	MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->NegativeClickSound);
}

void SFlareShipMenu::OnCancelUpgrade()
{
	TargetSpacecraft->CancelUpgrade();
	MenuManager->Reload();
}

FText SFlareShipMenu::GetShipUpgradeDetails() const
{
	AFlareSpacecraft* PhysicalSpacecraft = TargetSpacecraft->GetActive();
	if (PhysicalSpacecraft)
	{
		UFlareSpacecraftNavigationSystem* Spacecraftnavigation = PhysicalSpacecraft->GetNavigationSystem();
		if (Spacecraftnavigation)
		{
			UFlareSimulatedSpacecraft* TransactionDestinationDock = Spacecraftnavigation->GetTransactionDestinationDock();
			FFlareSpacecraftComponentDescription* TransactionNewPartDesc = Spacecraftnavigation->GetTransactionNewPartDesc();
			int32 TransactionNewPartWeaponGroupIndex = Spacecraftnavigation->GetTransactionNewPartWeaponIndex();

			if (TransactionDestinationDock && TransactionNewPartDesc && TransactionNewPartWeaponGroupIndex >= 0)
			{
				uint32 TransactionQuantity = Spacecraftnavigation->GetTransactionQuantity();
				FText Formatted;
				FText DistanceText;
				FText TradeStatus = LOCTEXT("TradeInfoUpgrade", "upgrade");

				AFlareSpacecraft* TargetSpacecraftPawn = TransactionDestinationDock->GetActive();
				float Distance = (PhysicalSpacecraft->GetActorLocation() - TargetSpacecraftPawn->GetActorLocation()).Size();
				DistanceText = FText::Format(LOCTEXT("PlayerDistanceFormat", "{0} - "), AFlareHUD::FormatDistance(Distance / 100));

				Formatted = FText::Format(LOCTEXT("SpacecraftInfoFormatUpgrade", "{0}docking at {1} to {2} {3}"),
					DistanceText,
					UFlareGameTools::DisplaySpacecraftName(TransactionDestinationDock),
					TradeStatus,
					TransactionNewPartDesc->Name);
				return Formatted;
			}
		}
	}
	return FText();
}

#undef LOCTEXT_NAMESPACE
