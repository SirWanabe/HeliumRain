
#include "FlareSpacecraftInfo.h"
#include "../../Flare.h"

#include "../../Data/FlareSpacecraftCatalog.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlareGameTools.h"
#include "../../Game/FlareTradeRoute.h"

#include "../../Economy/FlareFactory.h"
#include "../../Economy/FlareCargoBay.h"

#include "../../Player/FlareHUD.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#include "../../UI/Menus/FlareTradeMenu.h"

#include "FlareCargoInfo.h"

#define LOCTEXT_NAMESPACE "FlareSpacecraftInfo"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSpacecraftInfo::Construct(const FArguments& InArgs)
{
	// Data
	PC = InArgs._Player;
	OwnerWidget = InArgs._OwnerWidget->AsShared();
	NoInspect = InArgs._NoInspect;
	Minimized = InArgs._Minimized;
	UseSmallFont = InArgs._UseSmallFont;
	OnRemoved = InArgs._OnRemoved;
	WidthAdjuster = InArgs._WidthAdjuster;
	OriginatingMenu = InArgs._OriginatingMenu;
	AFlareGame* Game = InArgs._Player->GetGame();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	WidthAdjusted = Theme.ContentWidth;
	if (WidthAdjuster)
	{
		WidthAdjusted *= WidthAdjuster;
	}

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Left)
	[
		SNew(SBox)
		.WidthOverride(WidthAdjusted)
		.Padding(Theme.SmallContentPadding)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
			
				// Data block
				+ SHorizontalBox::Slot()
				[
					SNew(SVerticalBox)

					// Main line
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Class icon
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SImage).Image(this, &SFlareSpacecraftInfo::GetClassIcon)
						]

						// Upgrading Icon
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Image(FFlareStyleSet::GetIcon("Build"))
							.Visibility(this, &SFlareSpacecraftInfo::GetBuildVisibility)
						]

						// Capturing Icon
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SAssignNew(CapturingIcon, SImage)
							.Image(FFlareStyleSet::GetIcon("Capture"))
							.Visibility(EVisibility::Collapsed)
						]

						// Unlicenced Icon
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SAssignNew(UnlicencedIcon, SImage)
							.Image(FFlareStyleSet::GetIcon("Delete"))
							.Visibility(EVisibility::Collapsed)

						]

						// Ship name
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						.VAlign(VAlign_Center)
						[
							SAssignNew(SpacecraftName, STextBlock)
							.Text(this, &SFlareSpacecraftInfo::GetName)
							.TextStyle(&Theme.NameFont)
							.ColorAndOpacity(this, &SFlareSpacecraftInfo::GetTextColor)
						]

						// Ship class
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(this, &SFlareSpacecraftInfo::GetDescription)
							.TextStyle(&Theme.TextFont)
						]

						// Combat value icon
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						.Padding(FMargin(5, 0, 0, 0))
						[
							SNew(SImage)
							.Image(FFlareStyleSet::GetIcon("CombatValue"))
							.Visibility(this, &SFlareSpacecraftInfo::GetCombatValueVisibility)
						]

						// Combat value
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(this, &SFlareSpacecraftInfo::GetCombatValue)
							.TextStyle(&Theme.TextFont)
							.Visibility(this, &SFlareSpacecraftInfo::GetCombatValueVisibility)
						]

						// Status
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Right)
						[
							SAssignNew(ShipStatus, SFlareShipStatus)
						]
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
							.Player(InArgs._Player)
							.Visibility(this, &SFlareSpacecraftInfo::GetCompanyFlagVisibility)
						]

						// Spacecraft info
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(STextBlock)
							.Text(this, &SFlareSpacecraftInfo::GetSpacecraftInfo)
							.TextStyle(&Theme.TextFont)
							.Visibility(this, &SFlareSpacecraftInfo::GetSpacecraftInfoVisibility)
						]

						// Spacecraft info 2
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(STextBlock)
							.Text(this, &SFlareSpacecraftInfo::GetSpacecraftInfoAdditional)
							.ColorAndOpacity(this, &SFlareSpacecraftInfo::GetAdditionalTextColor)
							.TextStyle(&Theme.TextFont)
							.Visibility(this, &SFlareSpacecraftInfo::GetSpacecraftInfoVisibility)
						]
					]

					//company line two, for displaying in sector trading/status
					+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SHorizontalBox)
							// Spacecraft info
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.SmallContentPadding)
							[
								SNew(STextBlock)
								.Text(this, &SFlareSpacecraftInfo::GetSpacecraftLocalInfo)
								.TextStyle(&Theme.TextFont)
								.Visibility(this, &SFlareSpacecraftInfo::GetSpacecraftLocalInfoVisibility)
							]
						]
					
					// Message box
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(MessageBox, SVerticalBox)
					]

					// Cargo bay block 1
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.SmallContentPadding)
					.HAlign(HAlign_Left)
					[
						SAssignNew(CargoBay1, SHorizontalBox)
					]

					// Cargo bay block 2
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.SmallContentPadding)
					.HAlign(HAlign_Left)
					[
						SAssignNew(CargoBay2, SHorizontalBox)
					]
				]

				// Icon
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				[
					SNew(SImage).Image(this, &SFlareSpacecraftInfo::GetIcon)
					.Visibility(InArgs._NoInspect ? EVisibility::Hidden : EVisibility::Visible)
				]
			]

			// Buttons 
			+ SVerticalBox::Slot()
			.Padding(Theme.SmallContentPadding)
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// Inspect
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(InspectButton, SFlareButton)
					.Text(FText::FromString("-"))
					.HelpText(FText::FromString("-"))
					.HotkeyText(LOCTEXT("SpacecraftKey1", "M1"))
					.OnClicked(this, &SFlareSpacecraftInfo::OnInspect)
					.Width(4)
				]

				// Target
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(TargetButton, SFlareButton)
					.Text(LOCTEXT("Target", "TARGET"))
					.HelpText(this, &SFlareSpacecraftInfo::GetTargetButtonHint)
					.HotkeyText(LOCTEXT("SpacecraftKey2", "M2"))
					.OnClicked(this, &SFlareSpacecraftInfo::OnTarget)
					.IsDisabled(this, &SFlareSpacecraftInfo::IsTargetDisabled)
					.Width(4)
				]

				// Fly this ship
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(FlyButton, SFlareButton)
					.Text(LOCTEXT("ShipFly", "FLY"))
					.HotkeyText(LOCTEXT("SpacecraftKey3", "M3"))
					.OnClicked(this, &SFlareSpacecraftInfo::OnFly)
					.Width(4)
				]
			]

			// Buttons 2
			+ SVerticalBox::Slot()
			.Padding(Theme.SmallContentPadding)
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// Trade
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(TradeButton, SFlareButton)
					.Text(LOCTEXT("Trade", "TRADE"))
					.HotkeyText(LOCTEXT("SpacecraftKey4", "M4"))
					.OnClicked(this, &SFlareSpacecraftInfo::OnTrade)
					.Width(4)
				]
			
				// Dock here
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(DockButton, SFlareButton)
					.Text(LOCTEXT("Dock", "DOCK"))
					.HotkeyText(LOCTEXT("SpacecraftKey5", "M5"))
					.OnClicked(this, &SFlareSpacecraftInfo::OnDockAt)
					.Width(4)
				]

				// Undock
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(UndockButton, SFlareButton)
					.Text(LOCTEXT("Undock", "UNDOCK"))
					.HelpText(LOCTEXT("UndockInfo", "Undock from this spacecraft and go back to flying the ship"))
					.HotkeyText(LOCTEXT("SpacecraftKey5", "M5"))
					.OnClicked(this, &SFlareSpacecraftInfo::OnUndock)
					.Width(4)
				]

				// Upgrade
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(UpgradeButton, SFlareButton)
					.Text(LOCTEXT("Upgrade", "UPGRADE"))
					.HotkeyText(LOCTEXT("SpacecraftKey6", "M6"))
					.OnClicked(this, &SFlareSpacecraftInfo::OnUpgrade)
					.Width(4)
				]

				// Scrap
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(ScrapButton, SFlareButton)
					.Text(LOCTEXT("Scrap", "SCRAP"))
					.HotkeyText(LOCTEXT("SpacecraftKey7", "M7"))
					.OnClicked(this, &SFlareSpacecraftInfo::OnScrap)
					.Width(4)
				]
			]
		]
	];

	// Setup
	if (InArgs._Spacecraft)
	{
		SetSpacecraft(InArgs._Spacecraft);
	}
	Hide();
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareSpacecraftInfo::SetSpacecraft(UFlareSimulatedSpacecraft* Target)
{
	TargetSpacecraft = Target;
	ShipStatus->SetTargetShip(Target);

	// Get the save data info to retrieve the class data
	if (TargetSpacecraft && PC)
	{
		// Setup basic info
		CompanyFlag->SetCompany(TargetSpacecraft->GetCompany());
		FFlareSpacecraftSave* SaveData = TargetSpacecraft->Save();
		if (SaveData)
		{
			TargetSpacecraftDesc = PC->GetGame()->GetSpacecraftCatalog()->Get(SaveData->Identifier);
		}

		// Prepare cargo bay
		CargoBay1->ClearChildren();
		CargoBay2->ClearChildren();
		UFlareCompany* Company = TargetSpacecraft->GetCompany();
		TArray<FSortableCargoInfo> SortedCargoBay;

		// Fill the cargo bay
		if (!TargetSpacecraft->IsPlayerHostile())
		{
			// Get slots
			for (int32 CargoIndex = 0; CargoIndex < TargetSpacecraft->GetActiveCargoBay()->GetSlotCount() ; CargoIndex++)
			{
				FFlareCargo* Cargo = TargetSpacecraft->GetActiveCargoBay()->GetSlot(CargoIndex);

				if(Cargo->Lock == EFlareResourceLock::Hidden && Cargo->Quantity == 0)
				{
					continue;
				}

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
				];
			}
		}

		// Set "details" 
		if (Target->IsShipyard())
		{
			if (Target->GetCompany() == PC->GetCompany())
			{
				InspectButton->SetText(LOCTEXT("InspectOwnedShipyard", "BUILD SHIP"));
				InspectButton->SetHelpText(LOCTEXT("InspectShipyardInfo", "Order ships or manage ships production."));
			}
			else if(!Target->GetDescription()->IsDroneCarrier)
			{
				InspectButton->SetText(LOCTEXT("InspectShipyard", "BUY SHIP"));
				InspectButton->SetHelpText(LOCTEXT("InspectShipyardInfo", "Order ships or manage ships production."));
			}
			else
			{
				InspectButton->SetText(LOCTEXT("InspectRegular", "DETAILS"));
				InspectButton->SetHelpText(LOCTEXT("InspectRegularInfo", "Take a closer look at this spacecraft"));
			}
		}
		else
		{
			InspectButton->SetText(LOCTEXT("InspectRegular", "DETAILS"));
			InspectButton->SetHelpText(LOCTEXT("InspectRegularInfo", "Take a closer look at this spacecraft"));
		}
	}

	// Text font
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	const FTextBlockStyle* TextFont = &Theme.NameFont;
	if (TargetSpacecraft && TargetSpacecraft->IsValidLowLevel() && TargetSpacecraft == PC->GetPlayerShip())
	{
		TextFont = &Theme.NameFontBold;
	}
	SpacecraftName->SetTextStyle(TextFont);
}

void SFlareSpacecraftInfo::SetNoInspect(bool NewState)
{
	NoInspect = NewState;
}

void SFlareSpacecraftInfo::SetMinimized(bool NewState)
{
	Minimized = NewState;
	PC->GetMenuManager()->UnregisterSpacecraftInfo(this);

	if (GetVisibility() == EVisibility::Visible)
	{
		Show();
	}
}

void SFlareSpacecraftInfo::Show()
{
	FCHECK(PC);

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	MessageBox->ClearChildren();

	if (Minimized)
	{
		CargoBay1->SetVisibility(EVisibility::Collapsed);
		CargoBay2->SetVisibility(EVisibility::Collapsed);

		InspectButton->SetVisibility(EVisibility::Collapsed);
		TargetButton->SetVisibility(EVisibility::Collapsed);
		UpgradeButton->SetVisibility(EVisibility::Collapsed);
		TradeButton->SetVisibility(EVisibility::Collapsed);
		FlyButton->SetVisibility(EVisibility::Collapsed);
		DockButton->SetVisibility(EVisibility::Collapsed);
		UndockButton->SetVisibility(EVisibility::Collapsed);
		ScrapButton->SetVisibility(EVisibility::Collapsed);
	}
	else if (TargetSpacecraft && TargetSpacecraft->IsValidLowLevel())
	{
		// Useful data
		UFlareSimulatedSpacecraft* PlayerShip = PC->GetPlayerShip();
		AFlareSpacecraft* DockedStation = NULL;
		AFlareSpacecraft* ActiveTargetSpacecraft = NULL;
		if (TargetSpacecraft->IsActive())
		{
			ActiveTargetSpacecraft = TargetSpacecraft->GetActive();
			DockedStation = ActiveTargetSpacecraft->GetNavigationSystem()->GetDockStation();
		}

		// Helpers
		bool Owned = TargetSpacecraft->GetCompany()->GetPlayerHostility() == EFlareHostility::Owned;
		bool OwnedAndNotSelf = Owned && TargetSpacecraft != PlayerShip;
		bool IsFriendly = !TargetSpacecraft->IsPlayerHostile();
		bool IsOutsidePlayerFleet = (TargetSpacecraft->GetCurrentFleet() != PlayerShip->GetCurrentFleet()) || !ActiveTargetSpacecraft;
		bool IsDocked = ActiveTargetSpacecraft && (DockedStation || ActiveTargetSpacecraft->GetDockingSystem()->IsDockedShip(PlayerShip->GetActive()));
		bool IsStation = TargetSpacecraft->IsStation();
		bool IsCargoShip = (TargetSpacecraft->GetDescription()->CargoBayCount > 0) && !IsStation;
		bool IsCargoStation = (TargetSpacecraft->GetDescription()->CargoBayCount > 0) && IsStation && !Owned;
		bool IsAutoDocking = PlayerShip->GetCompany()->IsTechnologyUnlocked("auto-docking");

		// Permissions
		bool CanDock =     !IsDocked && IsFriendly && ActiveTargetSpacecraft && ActiveTargetSpacecraft->GetDockingSystem()->HasCompatibleDock(PlayerShip->GetActive());
		bool CanUpgradeDistant = (IsOutsidePlayerFleet || IsAutoDocking) && TargetSpacecraft->GetCurrentSector() && TargetSpacecraft->GetCurrentSector()->CanUpgrade(TargetSpacecraft->GetCompany());
		bool CanUpgradeDocked = ActiveTargetSpacecraft && DockedStation && DockedStation->GetParent()->HasCapability(EFlareSpacecraftCapability::Upgrade);
		bool CanUpgrade = !TargetSpacecraft->IsStation() && (CanUpgradeDistant || CanUpgradeDocked);
		bool CanTrade = (IsCargoShip || IsCargoStation) && (IsDocked || IsOutsidePlayerFleet || IsAutoDocking);
		
		bool CanScrapStation = TargetSpacecraft->IsStation() && TargetSpacecraft->CanScrapStation();
		bool CanScrap =    OwnedAndNotSelf && (CanUpgrade || CanScrapStation);
		
		// Is a battle in progress ?
		if (TargetSpacecraft->GetCurrentSector())
		{
			if (TargetSpacecraft->GetCurrentSector()->IsTravelSector())
			{
				CanTrade = false;
				CanUpgrade = false;
				CanScrap = false;
			}
			else
				{
				if (TargetSpacecraft->GetCurrentSector()->IsPlayerBattleInProgress())
				{
					CanTrade = false;
					CanUpgrade = false;
					CanScrap = false;
				}
				if (TargetSpacecraft->GetDamageSystem()->IsUncontrollable())
				{
					CanTrade = false;
					CanUpgrade = false;
				}
			}
		}

		// Button states : hide stuff that can never make sense (flying stations etc), disable other states after that

		CargoBay1->SetVisibility(CargoBay1->NumSlots() > 0 ? EVisibility::Visible : EVisibility::Collapsed);
		CargoBay2->SetVisibility(CargoBay2->NumSlots() > 0 ? EVisibility::Visible : EVisibility::Collapsed);

		// Buttons
		InspectButton->SetVisibility(NoInspect ?           EVisibility::Collapsed : EVisibility::Visible);
		UpgradeButton->SetVisibility(Owned && !IsStation ? EVisibility::Visible : EVisibility::Collapsed);
		FlyButton->SetVisibility(!Owned || IsStation ?     EVisibility::Collapsed : EVisibility::Visible);
		TargetButton->SetVisibility(TargetSpacecraft->IsActive() ? EVisibility::Visible : EVisibility::Collapsed);

//		TradeButton->SetVisibility(Owned && IsCargoShip ?                            EVisibility::Visible : EVisibility::Collapsed);
		TradeButton->SetVisibility((Owned && IsCargoShip) || IsCargoStation ?                            EVisibility::Visible : EVisibility::Collapsed);

		DockButton->SetVisibility(CanDock ?                                      EVisibility::Visible : EVisibility::Collapsed);
		UndockButton->SetVisibility(Owned && IsDocked && !IsOutsidePlayerFleet ? EVisibility::Visible : EVisibility::Collapsed);
		ScrapButton->SetVisibility(Owned ?                                       EVisibility::Visible : EVisibility::Collapsed);

		if (TargetSpacecraft->GetShipMaster() != NULL)
		{
			UpgradeButton->SetVisibility(EVisibility::Collapsed);
			TradeButton->SetVisibility(EVisibility::Collapsed);
			FlyButton->SetVisibility(EVisibility::Collapsed);
			DockButton->SetVisibility(EVisibility::Collapsed);
			UndockButton->SetVisibility(EVisibility::Collapsed);
		}

		// Flyable ships : disable when not flyable

		FText Reason;
		if (!TargetSpacecraft->CanBeFlown(Reason))
		{
			FlyButton->SetHelpText(Reason);
			FlyButton->SetDisabled(true);
		}

		else if (TargetSpacecraft == PlayerShip)
		{
			FlyButton->SetHelpText(LOCTEXT("CantFlySelfInfo", "You are already flying this ship"));
			FlyButton->SetDisabled(true);
		}
		else
		{
			FlyButton->SetHelpText(LOCTEXT("ShipFlyInfo", "Take command of this spacecraft"));
			FlyButton->SetDisabled(false);
		}

		// Can dock
		if (!PlayerShip->GetCompany()->IsTechnologyUnlocked("auto-docking"))
		{
			DockButton->SetHelpText(LOCTEXT("ShipAutoDockNeededInfo", "You need the Auto Docking technology to dock automatically at stations"));
			DockButton->SetDisabled(true);
		}
		else
		{
			DockButton->SetHelpText(LOCTEXT("ShipDockInfo", "Try to dock at this spacecraft"));
			DockButton->SetDisabled(false);
		}

		// Can undock
		if (TargetSpacecraft->IsTrading())
		{
			UndockButton->SetHelpText(LOCTEXT("ShipTradingUndockInfo", "This ship is trading and can't undock today"));
			UndockButton->SetDisabled(true);
		}
		else
		{
			UndockButton->SetHelpText(LOCTEXT("ShipUndockInfo", "Undock the ship and leave the station"));
			UndockButton->SetDisabled(false);
		}

		if (IsCargoStation)
		{
			bool FoundValidTrade = false;
			if (!PlayerShip->GetCompany()->IsTechnologyUnlocked("auto-docking"))
			{
				TradeButton->SetHelpText(LOCTEXT("ShipAutoDockNeededInfo", "You need the Auto Docking technology to dock automatically at stations"));
				TradeButton->SetDisabled(true);
			}
			else
			{
				if (TargetSpacecraft->GetCurrentSector())
				{
					for (UFlareSimulatedSpacecraft* Spacecraft : TargetSpacecraft->GetCurrentSector()->GetSectorSpacecrafts())
					{
						if (Spacecraft->GetCompany()->GetPlayerHostility() == EFlareHostility::Owned && !Spacecraft->IsStation() && Spacecraft->GetDescription()->CargoBayCount > 0 && (IsDocked || IsOutsidePlayerFleet || IsAutoDocking) && !Spacecraft->IsTrading())
						{
							TradeButton->SetHelpText(LOCTEXT("TradeInfoStation", "Trade with this station"));
							TradeButton->SetDisabled(false);
							FoundValidTrade = true;
							break;
						}
					}
				}
				if (!FoundValidTrade)
				{
					TradeButton->SetHelpText(LOCTEXT("CantTradeNoValidShips", "No available ships to trade with"));
					TradeButton->SetDisabled(true);
				}
			}
		}
		else
		{
			// Disable trade while flying unless docked
			if (CanTrade && !TargetSpacecraft->IsTrading())
			{
				TradeButton->SetHelpText(LOCTEXT("TradeInfo", "Trade with this spacecraft"));
				TradeButton->SetDisabled(false);
			}
			else if (CanTrade && TargetSpacecraft->IsTrading())
			{
				TradeButton->SetHelpText(LOCTEXT("CantTradeInProgressInfo", "Trading in progress"));
				TradeButton->SetDisabled(true);
			}
			else
			{
				TradeButton->SetHelpText(LOCTEXT("CantTradeInfo", "Trading requires to be docked in a peaceful sector, or outside the player fleet"));
				TradeButton->SetDisabled(true);
			}
		}

		// Disable upgrades
		if (CanUpgrade)
		{
			UpgradeButton->SetHelpText(LOCTEXT("UpgradeInfo", "Upgrade this spacecraft"));
			UpgradeButton->SetDisabled(false);
		}
		else
		{
			UpgradeButton->SetHelpText(LOCTEXT("CantUpgradeInfo", "Upgrading requires to be docked at a supply outpost in a peaceful sector, or outside the player fleet"));
			UpgradeButton->SetDisabled(true);
		}

		// Disable scrapping
		if (CanScrap)
		{
			ScrapButton->SetHelpText(LOCTEXT("ScrapInfo", "Permanently destroy this ship and get some resources back"));
			ScrapButton->SetDisabled(false);
		}
		else
		{
			if (TargetSpacecraft->IsStation())
			{
				ScrapButton->SetHelpText(LOCTEXT("CantScrapStationInfo", "Scrap all complex child stations before scrapping this station"));
			}
			else
			{
				ScrapButton->SetHelpText(LOCTEXT("CantScrapInfo", "Scrapping requires to be docked in a peaceful sector, or outside the player fleet"));
			}
			ScrapButton->SetDisabled(true);
		}

		// Update message box
		UpdateCaptureList();
		UpdateCapabilitiesInfo();
	}

	if (PC->GetMenuManager()->GetCurrentMenu() == EFlareMenu::MENU_Trade)
	{
		CargoBay1->SetVisibility(EVisibility::Visible);
		CargoBay2->SetVisibility(EVisibility::Visible);
	}

	PC->GetMenuManager()->RegisterSpacecraftInfo(this);
}

void SFlareSpacecraftInfo::Hide()
{
	PC->GetMenuManager()->UnregisterSpacecraftInfo(this);

	MessageBox->ClearChildren();
	TargetSpacecraft = NULL;
	TargetSpacecraftDesc = NULL;

	SetVisibility(EVisibility::Collapsed);
	SetEnabled(false);
}

void SFlareSpacecraftInfo::Hotkey(int32 Index)
{
	if (!TargetSpacecraft || !TargetSpacecraftDesc)
	{
		return;
	}

	switch (Index)
	{
		case 1:
			if (InspectButton->IsButtonAvailable())
			{
				OnInspect();
			}
			break;
		case 2:
			if (TargetButton->IsButtonAvailable())
			{
				OnTarget();
			}
			break;
		case 3:
			if (FlyButton->IsButtonAvailable())
			{
				OnFly();
			}
			break;
		case 4:
			if (TradeButton->IsButtonAvailable())
			{
				OnTrade();
			}
			break;
		case 5:
			if (DockButton->IsButtonAvailable())
			{
				OnDockAt();
			}
			if (UndockButton->IsButtonAvailable())
			{
				OnUndock();
			}
			break;
		case 6:
			if (UpgradeButton->IsButtonAvailable())
			{
				OnUpgrade();
			}
			break;
		case 7:
			if (ScrapButton->IsButtonAvailable())
			{
				OnScrap();
			}
			break;
		default:
			break;
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareSpacecraftInfo::UpdateCapabilitiesInfo()
{
	if (TargetSpacecraft && TargetSpacecraft->IsValidLowLevel())
	{
		if (PC->GetCurrentObjective() && PC->GetCurrentObjective()->IsTarget(TargetSpacecraft))
		{
			AddMessage(PC->GetCurrentObjective()->Name, FFlareStyleSet::GetIcon("ContractSmall"), NULL, 0);
		}

		if (TargetSpacecraft->IsStation() && TargetSpacecraft->IsUnderConstruction())
		{
			FText ConstructionInfo = LOCTEXT("ConstructionInfo", "This station is under construction and needs resources to be completed");
			AddMessage(ConstructionInfo, FFlareStyleSet::GetIcon("Build"), NULL, 0);
		}

		if (TargetSpacecraft->IsStation())
		{
			float Efficiency = TargetSpacecraft->GetStationEfficiency();
			int DurationMalus = FMath::FloorToInt(UFlareFactory::GetProductionMalus(Efficiency));

/*
	float Efficiency = Parent->GetStationEfficiency();
	float Malus = GetProductionMalus(Efficiency);
	int64 ProductionTime = FMath::FloorToInt(Cycle.ProductionTime * Malus);

*/
			if (DurationMalus > 1)
			{
				float DamageRatio = TargetSpacecraft->GetDamageRatio();
				FText EfficiencyMessage;

				if (DamageRatio < 1.f)
				{
					if (!TargetSpacecraft->GetOwnerHasStationLicense())
					{
						EfficiencyMessage = LOCTEXT("StationEfficiencyMessage_UnlicensedDamaged", "This unlicensed station is damaged");
					}
					else
					{
						EfficiencyMessage = LOCTEXT("StationEfficiencyMessage_Damaged", "This station is damaged");
					}
				}
				else if (!TargetSpacecraft->GetOwnerHasStationLicense())
				{
					EfficiencyMessage = LOCTEXT("StationEfficiencyMessage_Unlicensed", "This is an unlicensed station");
				}
				else if (!TargetSpacecraft->GetCompany()->IsTechnologyUnlockedStation(TargetSpacecraft->GetDescription()))
				{
					EfficiencyMessage = LOCTEXT("StationEfficiencyMessage_Unresearched", "This station is without technological knowledge");
				}

				AddMessage(FText::Format(LOCTEXT("StationEfficiencyFormat", "{0} and operates {1}x slower"), EfficiencyMessage,FText::AsNumber(DurationMalus)),
				FFlareStyleSet::GetIcon("Damage"),
				NULL,
				0);
			}
		}

		if (TargetSpacecraft->IsShipyard())
		{
			if (TargetSpacecraft->IsStation())
			{
				AddMessage(LOCTEXT("ShipyardCapability", "You can order and upgrade ships at this station"), FFlareStyleSet::GetIcon("Shipyard"), NULL, 0);
			}

			if(TargetSpacecraft->GetDescription()->IsDroneCarrier && PC->GetCompany() == TargetSpacecraft->GetCompany())
			{
				AddMessage(LOCTEXT("CarrierCapability", "You can order drones at this carrier"), FFlareStyleSet::GetIcon("Shipyard"), NULL, 0);
			}

			FText ProductionShips;
			FText QueueShips;

			if (TargetSpacecraft->GetOngoingProductionList().Num() == 1)
			{
				if (TargetSpacecraft->GetDescription()->IsDroneCarrier)
				{
					ProductionShips = LOCTEXT("ProductionShipsDrone", "drone");
				}
				else
				{
					ProductionShips = LOCTEXT("ProductionShipsShip", "ship");
				}
			}
			else
			{
				if (TargetSpacecraft->GetDescription()->IsDroneCarrier)
				{
					ProductionShips = LOCTEXT("ProductionShipsDrones", "drones");
				}
				else
				{
					ProductionShips = LOCTEXT("ProductionShipsShips", "ships");
				}
			}

			if (TargetSpacecraft->GetShipyardOrderQueue().Num() == 1)
			{
				if (TargetSpacecraft->GetDescription()->IsDroneCarrier)
				{
					QueueShips = LOCTEXT("QueueDrone", "drone");
				}
				else
				{
					QueueShips = LOCTEXT("QueueShip", "ship");
				}
			}
			else
			{
				if (TargetSpacecraft->GetDescription()->IsDroneCarrier)
				{
					QueueShips = LOCTEXT("QueueDrones", "drones");
				}
				else
				{
					QueueShips = LOCTEXT("QueueShips", "ships");
				}
			}

			AddMessage(FText::Format(LOCTEXT("ShipyardQueCount", "{0} {1} in production. {2} {3} in queue."), FText::AsNumber(TargetSpacecraft->GetOngoingProductionList().Num()), ProductionShips, FText::AsNumber(TargetSpacecraft->GetShipyardOrderQueue().Num()), QueueShips),
			FFlareStyleSet::GetIcon("Shipyard"),
			NULL,
			0);
		}
		else if (TargetSpacecraft->IsStation() && TargetSpacecraft->HasCapability(EFlareSpacecraftCapability::Upgrade))
		{
			AddMessage(LOCTEXT("UpgradeCapability", "You can upgrade your ship by docking at this station"), FFlareStyleSet::GetIcon("ShipUpgradeSmall"), NULL, 0);
		}

		if (TargetSpacecraft->IsStation() && TargetSpacecraft->HasCapability(EFlareSpacecraftCapability::Consumer))
		{
			AddMessage(LOCTEXT("ConsumerCapability", "This station can buy consumer resources"), FFlareStyleSet::GetIcon("Sector_Small"), NULL, 0);
		}
	}
}

bool SFlareSpacecraftInfo::UpdateCaptureList()
{
	bool CaptureInProgress = false;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	if (TargetSpacecraft && TargetSpacecraft->IsValidLowLevel() && TargetSpacecraft->IsStation())
	{
		// Player can capture
		if (PC->GetCompany()->CanStartCapture(TargetSpacecraft) && !PC->GetCompany()->WantCapture(TargetSpacecraft))
		{
//			int32 OwnedStationCount = TargetSpacecraft->GetCurrentSector()->GetSectorCompanyStationCount(PC->GetCompany(), true);
			int32 OwnedStationCount = PC->GetCompany()->GetCompanySectorStationsCount(TargetSpacecraft->GetCurrentSector(), true);
			int32 MaxStationCount = PC->GetCompany()->IsTechnologyUnlocked("dense-sectors") ?
			TargetSpacecraft->GetCurrentSector()->GetMaxStationsPerCompany() :
			TargetSpacecraft->GetCurrentSector()->GetMaxStationsPerCompany() / 2;

			FText CaptureText = FText::Format(LOCTEXT("CaptureFormat", "Start capturing this station ({0} / {1})"),
				FText::AsNumber(OwnedStationCount),
				MaxStationCount);

			// Show button
			MessageBox->AddSlot()
			.AutoHeight()
			.Padding(Theme.SmallContentPadding)
			.HAlign(HAlign_Left)
			[
				SNew(SFlareButton)
					.Text(CaptureText)
					.HelpText(LOCTEXT("CaptureInfo", "Capturing a station requires control of the sector for a few days"))
					.OnClicked(this, &SFlareSpacecraftInfo::OnCapture)
					.IsDisabled(this, &SFlareSpacecraftInfo::IsCaptureDisabled)
					.Icon(FFlareStyleSet::GetIcon("Capture"))
					.Width(14.8)
			];
		}

		// Find all companies that could capture this
		TArray<UFlareCompany*> Companies = PC->GetGame()->GetGameWorld()->GetCompanies();
		for (int32 CompanyIndex = 0; CompanyIndex < Companies.Num(); CompanyIndex++)
		{
			UFlareCompany* Company = Companies[CompanyIndex];
			if (TargetSpacecraft->GetCapturePoint(Company) > 0 || Company->WantCapture(TargetSpacecraft))
			{
				FText CaptureInfo = FText::Format(LOCTEXT("CaptureInfoFormat", "Capture in progress by {0} ({1})"),
					Company->GetCompanyName(),
					Company->GetPlayerHostilityText());

				CaptureInProgress = true;

				AddMessage(CaptureInfo,
					NULL,
					Company,
					static_cast<float>(TargetSpacecraft->GetCapturePoint(Company)) / static_cast<float>(TargetSpacecraft->GetCapturePointThreshold()));
			}
		}
	}

	return CaptureInProgress;
}

void SFlareSpacecraftInfo::AddMessage(FText Message, const FSlateBrush* Icon, UFlareCompany* CaptureCompany, float Progress)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Structure
	TSharedPtr<SHorizontalBox> Temp;
	MessageBox->AddSlot()
	.AutoHeight()
	.Padding(Theme.SmallContentPadding)
	[
		SNew(SBorder)
		.BorderImage(&Theme.NearInvisibleBrush)
		.Padding(Theme.SmallContentPadding)
		[
			SAssignNew(Temp, SHorizontalBox)
		]
	];

	// Icon
	if (Icon)
	{
		Temp->AddSlot()
		.AutoWidth()
		.Padding(Theme.SmallContentPadding)
		[
			SNew(SImage)
			.Image(Icon)
		];
	}

	// Flag
	if (CaptureCompany)
	{
		Temp->AddSlot()
		.AutoWidth()
		.Padding(Theme.SmallContentPadding)
		[
			SNew(SFlareCompanyFlag)
			.Company(CaptureCompany)
			.Player(PC)
		];
	}

	// Text info
	Temp->AddSlot()
	.Padding(Theme.SmallContentPadding)
	[
		SNew(STextBlock)
		.TextStyle(&Theme.TextFont)
		.Text(Message)
		.WrapTextAt(0.9 * WidthAdjusted)
	];

	// Progress bar
	if (CaptureCompany)
	{
		Temp->AddSlot()
		.AutoWidth()
		.Padding(Theme.SmallContentPadding)
		[
			SNew(SBox)
			.WidthOverride(100)
			.VAlign(VAlign_Center)
			[
				SNew(SProgressBar)
				.Percent(Progress)
				.BorderPadding(FVector2D(0, 0))
				.Style(&Theme.ProgressBarStyle)
			]
		];
	}
}

bool SFlareSpacecraftInfo::IsTargetDisabled() const
{
	if (TargetSpacecraft == PC->GetPlayerShip())
	{
		return true;
	}
	else if (!TargetSpacecraft->IsActive() || !PC->GetPlayerShip()->IsActive())
	{
		return true;
	}
	else if (PC->GetPlayerShip()->GetActive()->GetCurrentTarget().Is(TargetSpacecraft->GetActive()))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SFlareSpacecraftInfo::IsCaptureDisabled() const
{
	return false;
}

FText SFlareSpacecraftInfo::GetTargetButtonHint() const
{
	if (TargetSpacecraft == PC->GetPlayerShip())
	{
		return LOCTEXT("CantTargetSelf", "This is your own ship");
	}
	else if (!TargetSpacecraft->IsActive() || !PC->GetPlayerShip()->IsActive())
	{
		return LOCTEXT("CantTargetInactive", "Can't target ships in other sectors");
	}
	else if (PC->GetPlayerShip()->GetActive()->GetCurrentTarget().Is(TargetSpacecraft->GetActive()))
	{
		return LOCTEXT("CantTargetSame", "You are already targeting this spacecraft");
	}
	else
	{
		return LOCTEXT("TargetInfo", "Mark this spacecraft as your current target");
	}
}

void SFlareSpacecraftInfo::OnInspect()
{
	if (PC && TargetSpacecraft)
	{
		FLOGV("SFlareSpacecraftInfo::OnInspect : TargetSpacecraft=%p", TargetSpacecraft);
		FFlareMenuParameterData Data;
		Data.Spacecraft = TargetSpacecraft;
		PC->GetMenuManager()->OpenMenu(TargetSpacecraft->IsStation() ? EFlareMenu::MENU_Station : EFlareMenu::MENU_Ship, Data);
	}
}

void SFlareSpacecraftInfo::OnTarget()
{
	if (PC && PC->GetPlayerShip() && PC->GetPlayerShip()->IsActive() && TargetSpacecraft->IsActive())
	{
		FLOGV("SFlareSpacecraftInfo::OnTarget : TargetSpacecraft=%p", TargetSpacecraft);
		PC->GetPlayerShip()->GetActive()->SetCurrentTarget(TargetSpacecraft->GetActive());
	}
}

void SFlareSpacecraftInfo::OnUpgrade()
{
	if (PC && TargetSpacecraft)
	{
		FLOGV("SFlareSpacecraftInfo::OnUpgrade : TargetSpacecraft=%p", TargetSpacecraft);
		FFlareMenuParameterData Data;
		Data.Spacecraft = TargetSpacecraft;
		PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_ShipConfig, Data);
	}
}

void SFlareSpacecraftInfo::OnTrade()
{
	if (PC && TargetSpacecraft)
	{
		FFlareMenuParameterData Data;
		Data.Spacecraft = TargetSpacecraft;
		PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_Trade, Data);
	}
}

void SFlareSpacecraftInfo::OnFly()
{
	FText Unused;
	if (PC && TargetSpacecraft && TargetSpacecraft->IsActive() && TargetSpacecraft->CanBeFlown(Unused))
	{
		FLOGV("SFlareSpacecraftInfo::OnFly : flying active spacecraft '%s'", *TargetSpacecraft->GetImmatriculation().ToString());
		FFlareMenuParameterData Data;
		Data.Spacecraft = TargetSpacecraft;
		PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_FlyShip, Data);
	}
}

void SFlareSpacecraftInfo::OnDockAt()
{
	if (PC && TargetSpacecraft && TargetSpacecraft->IsActive() && TargetSpacecraft->GetActive()->GetDockingSystem()->GetDockCount() > 0)
	{
		bool DockingConfirmed = PC->GetShipPawn()->GetNavigationSystem()->DockAt(TargetSpacecraft->GetActive());
		PC->NotifyDockingResult(DockingConfirmed, TargetSpacecraft);
		if (DockingConfirmed)
		{
			PC->GetMenuManager()->CloseMenu();
		}
	}
}

void SFlareSpacecraftInfo::OnUndock()
{
	if (PC && TargetSpacecraft)
	{
		if (TargetSpacecraft->IsActive() && TargetSpacecraft->GetActive()->GetNavigationSystem()->IsDocked())
		{
			TargetSpacecraft->GetActive()->GetNavigationSystem()->Undock();
			if (TargetSpacecraft == PC->GetPlayerShip())
			{
				PC->GetMenuManager()->CloseMenu();
			}
		}
		else if(TargetSpacecraft->IsActive() && TargetSpacecraft->GetActive()->GetDockingSystem()->GetDockCount() > 0)
		{
			PC->GetShipPawn()->GetNavigationSystem()->Undock();
			if (TargetSpacecraft == PC->GetPlayerShip())
			{
				PC->GetMenuManager()->CloseMenu();
			}
		}
	}
}

void SFlareSpacecraftInfo::OnScrap()
{
	// Scrapping a station
	if (TargetSpacecraft->IsStation())
	{
			TMap<FFlareResourceDescription*, int32> ScrapResources = TargetSpacecraft->ComputeScrapResources();
			TMap<FFlareResourceDescription*, int32> NotDistributedScrapResources = TargetSpacecraft->GetCurrentSector()->DistributeResources(ScrapResources, TargetSpacecraft, TargetSpacecraft->GetCompany(), true);

			auto GenerateResourceList = [](TMap<FFlareResourceDescription*, int32>& Resources)
			{
				if(Resources.Num() == 0)
				{
					return LOCTEXT("NoResource", "nothing");
				}

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

			PC->GetMenuManager()->Confirm(LOCTEXT("AreYouSure", "ARE YOU SURE ?"),
									  FText::Format(LOCTEXT("ConfirmScrapInfo", "Do you really want to break up this station for its resources ?\nYou will get {0}{1}"),
													GainText,
													LossesText),
			FSimpleDelegate::CreateSP(this, &SFlareSpacecraftInfo::OnScrapConfirmed));
	}

	// Scrapping a ship
	else
	{
		PC->GetMenuManager()->Confirm(LOCTEXT("AreYouSure", "ARE YOU SURE ?"),
			LOCTEXT("ConfirmScrap", "Do you really want to break up this spacecraft for credits ?"),
			FSimpleDelegate::CreateSP(this, &SFlareSpacecraftInfo::OnScrapConfirmed));
	}
}

void SFlareSpacecraftInfo::OnScrapConfirmed()
{
	if (PC && TargetSpacecraft && TargetSpacecraft->IsValidLowLevel())
	{
		FLOGV("SFlareSpacecraftInfo::OnScrap : scrapping '%s'", *TargetSpacecraft->GetImmatriculation().ToString());

		// Scrapping a station
		if (TargetSpacecraft->IsStation() && TargetSpacecraft->CanScrapStation())
		{
			PC->GetGame()->ScrapStation(TargetSpacecraft);
			if (PC->GetMenuManager()->GetCurrentMenu() == EFlareMenu::MENU_Ship ||
				PC->GetMenuManager()->GetCurrentMenu() == EFlareMenu::MENU_Station ||
				PC->GetMenuManager()->GetCurrentMenu() == EFlareMenu::MENU_ShipConfig)
			{
				if (PC->GetMenuManager()->GetShipMenuTergetSpacecraft() == TargetSpacecraft)
				{
					PC->GetMenuManager()->Back();
				}
			}
			else
			{
				PC->GetMenuManager()->Reload();
			}
		}

		// Scrapping a ship
		else
		{
			UFlareSimulatedSpacecraft* TargetStation = NULL;
			TArray<UFlareSimulatedSpacecraft*> SectorStations = TargetSpacecraft->GetCurrentSector()->GetSectorStations();

			// Find a suitable station
			for (int Index = 0; Index < SectorStations.Num(); Index++)
			{
				if (SectorStations[Index]->GetCompany() == PC->GetCompany())
				{
					TargetStation = SectorStations[Index];
					FLOGV("SFlareSpacecraftInfo::OnScrap : found player station '%s'", *TargetStation->GetImmatriculation().ToString());
					break;
				}
			}

			if (TargetStation == NULL)
			{
				int32 RandomStationindex = FMath::RandRange(0, SectorStations.Num() - 1);
				TargetStation = SectorStations[RandomStationindex];
			}

			// Scrap
			if (TargetStation)
			{
				FLOGV("SFlareSpacecraftInfo::OnScrap : scrapping at '%s'", *TargetStation->GetImmatriculation().ToString());

				OnRemoved.ExecuteIfBound(TargetSpacecraft);
				PC->GetGame()->Scrap(TargetSpacecraft->GetImmatriculation(), TargetStation->GetImmatriculation());
				if (PC->GetMenuManager()->GetCurrentMenu() == EFlareMenu::MENU_Ship ||
					PC->GetMenuManager()->GetCurrentMenu() == EFlareMenu::MENU_Station ||
					PC->GetMenuManager()->GetCurrentMenu() == EFlareMenu::MENU_ShipConfig)
				{
					if (PC->GetMenuManager()->GetShipMenuTergetSpacecraft() == TargetSpacecraft)
					{
						PC->GetMenuManager()->Back();
					}
				}
				else
				{
					PC->GetMenuManager()->Reload();
				}

			}
			else
			{
				FLOG("SFlareSpacecraftInfo::OnScrap : couldn't find a valid station here !");
			}
		}
	}
}

void SFlareSpacecraftInfo::OnCapture()
{
	if (TargetSpacecraft && TargetSpacecraft->IsValidLowLevel())
	{
		PC->GetCompany()->StartCapture(TargetSpacecraft);

		MessageBox->ClearChildren();
		UpdateCaptureList();
		UpdateCapabilitiesInfo();
	}
}


/*----------------------------------------------------
	Content
----------------------------------------------------*/

FText SFlareSpacecraftInfo::GetName() const
{
//	return FText::FromName(TargetSpacecraftDesc->Identifier);
	return UFlareGameTools::DisplaySpacecraftName(TargetSpacecraft);
}

FSlateColor SFlareSpacecraftInfo::GetTextColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	if (TargetSpacecraft && TargetSpacecraft->IsValidLowLevel())
	{
		if (PC->GetCurrentObjective() && PC->GetCurrentObjective()->IsTarget(TargetSpacecraft))
		{
			return Theme.ObjectiveColor;
		}
		else if (TargetSpacecraft == PC->GetPlayerShip())
		{
			return Theme.FriendlyColor;
		}
		else if (TargetSpacecraft->IsPlayerHostile())
		{
			return Theme.EnemyColor;
		}
	}

	return Theme.NeutralColor;
}

FText SFlareSpacecraftInfo::GetDescription() const
{
	// Common text
	FText DefaultText = LOCTEXT("Default", "UNKNOWN OBJECT");

	// Description builder
	if (TargetSpacecraftDesc && TargetSpacecraft->IsValidLowLevel())
	{
		if(TargetSpacecraft && TargetSpacecraft->IsStation())
		{
			return FText::Format(LOCTEXT("DescriptionStationSectorFormat", "(Lv {0}, {1})"),
				FText::AsNumber(TargetSpacecraft->GetLevel()),
				TargetSpacecraft->GetCurrentSector()->GetSectorName());
		}
		else
		{
			return FText::Format(LOCTEXT("DescriptionFormat", "({0})"), TargetSpacecraftDesc->Name);
		}
	}

	return DefaultText;
}

FText SFlareSpacecraftInfo::GetCombatValue() const
{
	FText Result;

	if (TargetSpacecraft->IsValidLowLevel())
	{
		if (TargetSpacecraft->GetCombatPoints(true) > 0 || TargetSpacecraft->GetCombatPoints(false) > 0)
		{
			if (TargetSpacecraft->GetShipChildren().Num() > 0)
			{
				int32 Childrencombat = 0;
				int32 Childrencombatmax = 0;

				for (UFlareSimulatedSpacecraft* OwnedShips : TargetSpacecraft->GetShipChildren())
				{
					Childrencombat += OwnedShips->GetCombatPoints(true);
					Childrencombatmax += OwnedShips->GetCombatPoints(false);
				}

				Result = FText::Format(LOCTEXT("GetCombatValueFormat", "{0}/{1}\n{2}/{3} ({4}/{5})"),
					FText::AsNumber(TargetSpacecraft->GetCombatPoints(true)),
					FText::AsNumber(TargetSpacecraft->GetCombatPoints(false)),
					FText::AsNumber(Childrencombat),
					FText::AsNumber(Childrencombatmax),
					FText::AsNumber(TargetSpacecraft->GetShipChildren().Num()),
					FText::AsNumber(TargetSpacecraft->GetDescription()->DroneMaximum)
					);
			}
			else
			{
				Result = FText::Format(LOCTEXT("GetCombatValueFormat", "{0}/{1}"),
					FText::AsNumber(TargetSpacecraft->GetCombatPoints(true)),
					FText::AsNumber(TargetSpacecraft->GetCombatPoints(false)));
			}
		}
		else
		{
			Result = LOCTEXT("GetCombatValueZero", "0");
		}
	}

	return Result;
}

const FSlateBrush* SFlareSpacecraftInfo::GetIcon() const
{
	if (TargetSpacecraftDesc)
	{
		return &TargetSpacecraftDesc->MeshPreviewBrush;
	}
	return NULL;
}

const FSlateBrush* SFlareSpacecraftInfo::GetClassIcon() const
{
	if (TargetSpacecraftDesc)
	{
		return FFlareSpacecraftDescription::GetIcon(TargetSpacecraftDesc);
	}
	return NULL;
}

EVisibility SFlareSpacecraftInfo::GetCombatValueVisibility() const
{
	// Crash mitigation - If parent is hidden, so are we, don't try to use the target (#178)
	if ((OwnerWidget.IsValid() && OwnerWidget->GetVisibility() != EVisibility::Visible)
		|| (PC && !PC->GetMenuManager()->IsUIOpen()))
	{
		return EVisibility::Hidden;
	}

	// Check the target
	if (TargetSpacecraft && TargetSpacecraft->IsValidLowLevel())
	{
		if (TargetSpacecraft->IsStation() || !TargetSpacecraft->IsMilitaryArmed())
		{
			return EVisibility::Collapsed;
		}
	}

	return EVisibility::Visible;
}
EVisibility SFlareSpacecraftInfo::GetBuildVisibility() const
{
	// Crash mitigation - If parent is hidden, so are we, don't try to use the target (#178)
	if ((OwnerWidget.IsValid() && OwnerWidget->GetVisibility() != EVisibility::Visible)
		|| (PC && !PC->GetMenuManager()->IsUIOpen()))
	{
		return EVisibility::Hidden;
	}

	// Check the target
	if (TargetSpacecraft && TargetSpacecraft->IsValidLowLevel())
	{
		if (TargetSpacecraft->IsStation())
		{
			if (TargetSpacecraft->IsBeingCaptured())
			{
				CapturingIcon->SetVisibility(EVisibility::Visible);
			}
			else
			{
				CapturingIcon->SetVisibility(EVisibility::Collapsed);
			}

			if (TargetSpacecraft->GetOwnerHasStationLicense())
			{
				UnlicencedIcon->SetVisibility(EVisibility::Collapsed);
			}
			else
			{
				UnlicencedIcon->SetColorAndOpacity(FLinearColor::Red);
				UnlicencedIcon->SetVisibility(EVisibility::Visible);
			}

			if (!TargetSpacecraft->IsUnderConstruction())

			{
				return EVisibility::Collapsed;
			}
		}
		else
		{
			return EVisibility::Collapsed;
		}
	}

	return EVisibility::Visible;
}

EVisibility SFlareSpacecraftInfo::GetCompanyFlagVisibility() const
{
	// Crash mitigation - If parent is hidden, so are we, don't try to use the target (#178)
	if ((OwnerWidget.IsValid() && OwnerWidget->GetVisibility() != EVisibility::Visible)
		|| (PC && !PC->GetMenuManager()->IsUIOpen()))
	{
		return EVisibility::Collapsed;
	}

/*
	// Not visible while trading
	else if (PC->GetMenuManager()->GetCurrentMenu() == EFlareMenu::MENU_Trade)
	{
		return EVisibility::Collapsed;
	}
*/
	// Check the target
	if (TargetSpacecraft && TargetSpacecraft->IsValidLowLevel())
	{
		UFlareCompany* TargetCompany = TargetSpacecraft->GetCompany();
		if (TargetCompany && PC && TargetCompany == PC->GetCompany())
		{
			return EVisibility::Collapsed;
		}
	}

	return EVisibility::Visible;
}

EVisibility SFlareSpacecraftInfo::GetSpacecraftLocalInfoVisibility() const
{
	if (IsValid(TargetSpacecraft))
	{
/*
		bool TradeMenu = false;
		if (PC->GetMenuManager()->GetCurrentMenu() == EFlareMenu::MENU_Trade && PC->GetMenuManager()->IsUIOpen())
		{
			TradeMenu = true;
		}
*/
		UFlareCompany* TargetCompany = TargetSpacecraft->GetCompany(); // && TargetCompany == PC->GetCompany()
		if (IsValid(TargetCompany) && PC)// && !TradeMenu)
		{
			if (!TargetSpacecraft->IsStation())
			{
				AFlareSpacecraft* ActiveShip = TargetSpacecraft->GetActive();
				if (ActiveShip)
				{
					UFlareSpacecraftNavigationSystem* Spacecraftnavigation = ActiveShip->GetNavigationSystem();
					if (Spacecraftnavigation)
					{
						UFlareSimulatedSpacecraft* TransactionDestination = Spacecraftnavigation->GetTransactionDestination();
						UFlareSimulatedSpacecraft* TransactionDestinationDock = Spacecraftnavigation->GetTransactionDestinationDock();
						UFlareSimulatedSpacecraft* TransactionSourceShip = Spacecraftnavigation->GetTransactionSourceShip();
						FFlareResourceDescription* TransactionResource = Spacecraftnavigation->GetTransactionResource();

						FFlareSpacecraftComponentDescription* TransactionNewPartDesc = Spacecraftnavigation->GetTransactionNewPartDesc();
						int32 TransactionNewPartWeaponGroupIndex = Spacecraftnavigation->GetTransactionNewPartWeaponIndex();
						if (TransactionDestination && TransactionSourceShip && TransactionResource && Spacecraftnavigation && TransactionDestinationDock)
						{
							return EVisibility::Visible;
						}
						else if (TransactionDestinationDock && TransactionNewPartDesc && TransactionNewPartWeaponGroupIndex>=0)
						{
							return EVisibility::Visible;
						}
					}
				}
			}
		}
	}
	return EVisibility::Collapsed;
}
EVisibility SFlareSpacecraftInfo::GetSpacecraftInfoVisibility() const
{
	return EVisibility::Visible;
}

FText SFlareSpacecraftInfo::GetSpacecraftInfo() const
{
	// Crash mitigation - If parent is hidden, so are we, don't try to use the target (#178)
	if ((OwnerWidget.IsValid() && OwnerWidget->GetVisibility() != EVisibility::Visible)
		|| (PC && !PC->GetMenuManager()->IsUIOpen()))
	{
		return FText();
	}

	if (IsValid(TargetSpacecraft))
	{
		// Get the object's distance
		FText DistanceText;
		bool TradeMenu = 0;
		if (PC->GetMenuManager()->GetCurrentMenu() == EFlareMenu::MENU_Trade && PC->GetMenuManager()->IsUIOpen())
		{
			TradeMenu = true;
		}

		if (PC->GetPlayerShip() && PC->GetPlayerShip() != TargetSpacecraft)
		{
			AFlareSpacecraft* PlayerShipPawn = PC->GetPlayerShip()->GetActive();
			AFlareSpacecraft* TargetSpacecraftPawn = TargetSpacecraft->GetActive();
			if (PC->GetMenuManager()->GetCurrentMenu() == EFlareMenu::MENU_Trade)
			{
				UFlareSimulatedSpacecraft* TradeLeftShip = PC->GetMenuManager()->GetTradeMenu()->GetTargetLeftShip();
				if (TradeLeftShip)
				{
					PlayerShipPawn = TradeLeftShip->GetActive();
				}
				else
				{
					PlayerShipPawn = nullptr;
				}
			}

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
		if (TargetSpacecraft->IsStation())
		{
			ClassText = FText::FromString(TargetSpacecraft->GetDescription()->Name.ToString() + " - ");
		}
		
		// Our company
		UFlareCompany* TargetCompany = TargetSpacecraft->GetCompany();
		if (TargetCompany && PC && TargetCompany == PC->GetCompany() && !TradeMenu)
		{
			// Station : show production, if simulated
			if (TargetSpacecraft->IsStation())
			{
				FText ProductionStatusText = FText();
				TArray<UFlareFactory*>& Factories = TargetSpacecraft->GetFactories();

				if (Factories.Num() > 0)
				{
					for (int FactoryIndex = 0; FactoryIndex < Factories.Num(); FactoryIndex++)
					{
						FText NewLineText = (FactoryIndex > 0) ? FText::FromString("\n") : FText();
						UFlareFactory* Factory = Factories[FactoryIndex];

						ProductionStatusText = FText::Format(LOCTEXT("ProductionStatusFormat", "{0}{1}{2} : {3}"),
							ProductionStatusText,
							NewLineText,
							Factory->GetDescription()->Name,
							Factory->GetFactoryStatus());
					}

					return FText::Format(LOCTEXT("StationInfoFormat", "{0}{1}{2}"),
						DistanceText,
						ClassText,
						ProductionStatusText);
				}
				else
				{
					return FText::Format(LOCTEXT("StationInfoFormatNoFactories", "{0}{1}No factories"),
						DistanceText,
						ClassText);
				}
			}

			// Ship : show fleet info - GetSpacecraftInfoAdditional() will feed the rest (no end spacing because margins)
			else
			{
				UFlareFleet* Fleet = TargetSpacecraft->GetCurrentFleet();
				if (Fleet)
				{
					return FText::Format(LOCTEXT("SpacecraftInfoFormat", "{0}{1} -"), DistanceText, Fleet->GetStatusInfo());
				}
				return FText();
			}
		}

		// Other company
		else if (TargetCompany)
		{
			if (PC->GetMenuManager()->IsUIOpen() && PC->GetMenuManager()->GetCurrentMenu() == EFlareMenu::MENU_WorldEconomy)
			{
				UFlareSimulatedSector* CurrentSector = TargetSpacecraft->GetCurrentSector();
				if (CurrentSector)
				{
					return FText::Format(LOCTEXT("OwnedByFormat", "{0} - {1}{2}{3} ({4})"),
						TargetSpacecraft->GetCurrentSector()->GetSectorName(),
						DistanceText,
						ClassText,
						TargetCompany->GetCompanyName(),
						TargetCompany->GetPlayerHostilityText()
					);
				}
			}
			else if (TradeMenu)
				{
					return FText::Format(LOCTEXT("OwnedByFormat", "{0}{1}{2}"),
						DistanceText,
						ClassText,
						TargetCompany->GetCompanyName());
				}
				else
				{
					return FText::Format(LOCTEXT("OwnedByFormat", "{0}{1}{2} ({3})"),
						DistanceText,
						ClassText,
						TargetCompany->GetCompanyName(),
						TargetCompany->GetPlayerHostilityText());
				}
			}
		}
	return FText();
}

FText SFlareSpacecraftInfo::GetSpacecraftInfoAdditional() const
{
	// Crash mitigation - If parent is hidden, so are we, don't try to use the target (#178)
	if ((OwnerWidget.IsValid() && OwnerWidget->GetVisibility() != EVisibility::Visible)
		|| (PC && !PC->GetMenuManager()->IsUIOpen()))
	{
		return FText();
	}

	// Fleet info
	if (TargetSpacecraft && TargetSpacecraft->IsValidLowLevel())
	{
		UFlareCompany* TargetCompany = TargetSpacecraft->GetCompany();
		UFlareFleet* Fleet = TargetSpacecraft->GetCurrentFleet();

		if (TargetCompany && PC && TargetCompany == PC->GetCompany() && !TargetSpacecraft->IsStation() && Fleet)
		{
			FText FleetAssignedText;
			if (Fleet->GetCurrentTradeRoute())
			{
				FleetAssignedText = UFlareGameTools::AddLeadingSpace(FText::Format(LOCTEXT("FleetAssignedFormat", "- {0}{1}"),
					Fleet->GetCurrentTradeRoute()->GetTradeRouteName(),
					(Fleet->GetCurrentTradeRoute()->IsPaused() ? UFlareGameTools::AddLeadingSpace(LOCTEXT("FleetTradeRoutePausedFormat", "(Paused)")) : FText())));
			}

			return FText::Format(LOCTEXT("FleetFormat", "{0} ({1} / {2}){3}"),
				Fleet->GetFleetName(),
				FText::AsNumber(Fleet->GetShipCount()),
				FText::AsNumber(Fleet->GetMaxShipCount()),
				FleetAssignedText);
		}
	}

	return FText();
}

FText SFlareSpacecraftInfo::GetSpacecraftLocalInfo() const
{
	// Crash mitigation - If parent is hidden, so are we, don't try to use the target (#178)
	if ((OwnerWidget.IsValid() && OwnerWidget->GetVisibility() != EVisibility::Visible)
		|| (PC && !PC->GetMenuManager()->IsUIOpen()))
	{
		return FText();
	}

	if (IsValid(TargetSpacecraft))
	{
		// Get the object's distance
		bool TradeMenu = false;
		if (PC->GetMenuManager()->GetCurrentMenu() == EFlareMenu::MENU_Trade && PC->GetMenuManager()->IsUIOpen())
		{
			TradeMenu = true;
		}

		// Our company
		UFlareCompany* TargetCompany = TargetSpacecraft->GetCompany();
//		&& TargetCompany == PC->GetCompany()
		if (TargetCompany && PC)// && !TradeMenu)
		{
			if (!TargetSpacecraft->IsStation())
			{
				AFlareSpacecraft* ActiveShip = TargetSpacecraft->GetActive();
				if (ActiveShip)
				{
					UFlareSpacecraftNavigationSystem* Spacecraftnavigation = ActiveShip->GetNavigationSystem();
					if (Spacecraftnavigation)
					{
						UFlareSimulatedSpacecraft* TransactionDestination = Spacecraftnavigation->GetTransactionDestination();
						UFlareSimulatedSpacecraft* TransactionDestinationDock = Spacecraftnavigation->GetTransactionDestinationDock();
						UFlareSimulatedSpacecraft* TransactionSourceShip = Spacecraftnavigation->GetTransactionSourceShip();
						FFlareResourceDescription* TransactionResource = Spacecraftnavigation->GetTransactionResource();

						FFlareSpacecraftComponentDescription* TransactionNewPartDesc = Spacecraftnavigation->GetTransactionNewPartDesc();
						int32 TransactionNewPartWeaponGroupIndex = Spacecraftnavigation->GetTransactionNewPartWeaponIndex();


						if (TransactionDestination && TransactionSourceShip && TransactionResource && Spacecraftnavigation && TransactionDestinationDock)
						{
							uint32 TransactionQuantity = Spacecraftnavigation->GetTransactionQuantity();
							FText Formatted;
							FText DistanceText;
							FText TradeStatus = FText();

							AFlareSpacecraft* TargetSpacecraftPawn = TransactionDestinationDock->GetActive();
							float Distance = (ActiveShip->GetActorLocation() - TargetSpacecraftPawn->GetActorLocation()).Size();
							DistanceText = FText::Format(LOCTEXT("PlayerDistanceFormat", "{0} - "), AFlareHUD::FormatDistance(Distance / 100));

							if (TargetSpacecraft->GetCompany() == TransactionDestinationDock->GetCompany())
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
								FText::AsNumber(TransactionQuantity),
								TransactionResource->Name);
							return Formatted;
						}
						else if (TransactionDestinationDock && TransactionNewPartDesc && TransactionNewPartWeaponGroupIndex>=0)
						{
							uint32 TransactionQuantity = Spacecraftnavigation->GetTransactionQuantity();
							FText Formatted;
							FText DistanceText;
							FText TradeStatus = LOCTEXT("TradeInfoUpgrade", "upgrade");

							AFlareSpacecraft* TargetSpacecraftPawn = TransactionDestinationDock->GetActive();
							float Distance = (ActiveShip->GetActorLocation() - TargetSpacecraftPawn->GetActorLocation()).Size();
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

			}
		}
	}
	return FText();
}

FSlateColor SFlareSpacecraftInfo::GetAdditionalTextColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	if (TargetSpacecraft && TargetSpacecraft->IsValidLowLevel() && TargetSpacecraft->GetCurrentFleet())
	{
		if (TargetSpacecraft->GetCompany()->GetWarState(PC->GetCompany()) == EFlareHostility::Owned)
		{
			return TargetSpacecraft->GetCurrentFleet()->GetFleetColor();
		}
	}

	return Theme.NeutralColor;
}

#undef LOCTEXT_NAMESPACE
