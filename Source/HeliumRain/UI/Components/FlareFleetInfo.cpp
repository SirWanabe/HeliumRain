
#include "FlareFleetInfo.h"
#include "../../Flare.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareSectorHelper.h"
#include "../../Game/FlareGameTools.h"
#include "../../Game/FlareScenarioTools.h"

#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Game/FlareGameTools.h"
#include "../../UI/Menus/FlareFleetMenu.h"
#include "../../UI/Menus/FlareSectorMenu.h"
#include "../../UI/Menus/FlareOrbitalMenu.h"

#define LOCTEXT_NAMESPACE "FlareSpacecraftInfo"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareFleetInfo::Construct(const FArguments& InArgs)
{
	// Data
	PC = InArgs._Player;
	OwnerWidget = InArgs._OwnerWidget->AsShared();
	Minimized = InArgs._Minimized;
	UseSmallFont = InArgs._UseSmallFont;
	OriginatingMenu = InArgs._OriginatingMenu;

	AFlareGame* Game = InArgs._Player->GetGame();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	FTextBlockStyle TextFont;
	if (UseSmallFont)
	{
		TextFont = Theme.SmallFont;
	}
	else
	{
		TextFont = Theme.TextFont;
	}

	// Create the layout
	ChildSlot
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Left)
		[
			SNew(SBox)
			.WidthOverride(Theme.ContentWidth)
		.Padding(Theme.SmallContentPadding)
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

			// Fleet name
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(Theme.SmallContentPadding)
		.VAlign(VAlign_Center)
		[
			SAssignNew(FleetName, STextBlock)
			.Text(this, &SFlareFleetInfo::GetName)
		.TextStyle(&Theme.NameFont)
		.ColorAndOpacity(this, &SFlareFleetInfo::GetTextColor)
		]

	// Fleet composition
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(Theme.SmallContentPadding)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(this, &SFlareFleetInfo::GetComposition)
			.TextStyle(&TextFont)
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
		]

	// Combat value
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(this, &SFlareFleetInfo::GetCombatValue)
			.TextStyle(&TextFont)
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
		.Visibility(this, &SFlareFleetInfo::GetCompanyFlagVisibility)
		]

	// Fleet info
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(Theme.SmallContentPadding)
		[
			SNew(STextBlock)
			.Text(this, &SFlareFleetInfo::GetDescription)
			.TextStyle(&TextFont)
		]
		]

	// Fleet info line (because unlike individual ships players can rename their fleets so it could be possible for fleet info to scroll off window
	+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.SmallContentPadding)
		[
			SNew(SHorizontalBox)
			// Health
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		[
			SNew(SBox)
			.WidthOverride(50)
		.VAlign(VAlign_Center)
		[
			SNew(SProgressBar)
			.Percent(this, &SFlareFleetInfo::GetGlobalHealth)
		.BorderPadding(FVector2D(0, 0))
		.Style(&Theme.ProgressBarStyle)
		]
		]
	// Power icon
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("Propulsion"))
		.ColorAndOpacity(this, &SFlareFleetInfo::GetIconColor, EFlareSubsystem::SYS_Propulsion)
		]

	// RCS icon
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("RCS"))
		.ColorAndOpacity(this, &SFlareFleetInfo::GetIconColor, EFlareSubsystem::SYS_RCS)
		]

	// Weapon icon
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("Shell"))
		.ColorAndOpacity(this, &SFlareFleetInfo::GetIconColor, EFlareSubsystem::SYS_Weapon)
		]

	// Refilling icon
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(FMargin(5, 0, 5, 0))
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("Tank"))
		.Visibility(this, &SFlareFleetInfo::GetRefillingVisibility)
		]

	// Reparing icon
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(FMargin(5, 0, 5, 0))
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("Repair"))
		.Visibility(this, &SFlareFleetInfo::GetRepairingVisibility)
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
			.Text(LOCTEXT("Edit", "EDIT"))
		.HelpText(this, &SFlareFleetInfo::GetInspectHintText)
		.IsDisabled(this, &SFlareFleetInfo::IsInspectDisabled)
		.OnClicked(this, &SFlareFleetInfo::OnInspect)
		.Width(4)
		]

	// Inspect trade route
	+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SAssignNew(TradeRouteButton, SFlareButton)
			.Text(LOCTEXT("TradeRoute", "TRADE ROUTE"))
		.HelpText(this, &SFlareFleetInfo::GetInspectTradeRouteHintText)
		.IsDisabled(this, &SFlareFleetInfo::IsInspectTradeRouteDisabled)
		.OnClicked(this, &SFlareFleetInfo::OnOpenTradeRoute)
		.Width(6)
		]

	// Inspect trade route
	+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SAssignNew(AutoTradeButton, SFlareButton)
			.Text(LOCTEXT("AutoTrade", "AUTO-TRADE"))
		.HelpText(this, &SFlareFleetInfo::GetAutoTradeHintText)
		.IsDisabled(this, &SFlareFleetInfo::IsAutoTradeDisabled)
		.OnClicked(this, &SFlareFleetInfo::OnToggleAutoTrade)
		.Toggle(true)
		.Width(6)
		]
		]
	+ SVerticalBox::Slot()
		.Padding(Theme.SmallContentPadding)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			// Refill
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SAssignNew(RefillButton, SFlareButton)
			.HelpText(LOCTEXT("RefillInfoFleet", "Refill all ships in this fleet so that they have the necessary fuel, ammo and resources to fight."))
		.IsDisabled(this, &SFlareFleetInfo::IsRefillDisabled)
		.OnClicked(this, &SFlareFleetInfo::OnRefillClicked)
		.Text(this, &SFlareFleetInfo::GetRefillText)
		.Width(8)
		]

	// Repair
	+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SAssignNew(RepairButton, SFlareButton)
			.HelpText(LOCTEXT("RepairInfoFleet", "Repair all ships in this fleet."))
		.IsDisabled(this, &SFlareFleetInfo::IsRepairDisabled)
		.OnClicked(this, &SFlareFleetInfo::OnRepairClicked)
		.Text(this, &SFlareFleetInfo::GetRepairText)
		.Width(8)
		]
	]

	// Travel
	+ SVerticalBox::Slot()
	.Padding(Theme.SmallContentPadding)
	.HAlign(HAlign_Left)
	.AutoHeight()
	[
		SNew(SHorizontalBox)

		// Sector list
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(8 * Theme.ButtonWidth)
			[
				SAssignNew(SectorSelector, SFlareDropList<UFlareSimulatedSector*>)
				.OptionsSource(&SectorList)
				.OnGenerateWidget(this, &SFlareFleetInfo::OnGenerateSectorComboLine)
				.OnSelectionChanged(this, &SFlareFleetInfo::OnSectorComboLineSelectionChanged)
				.HeaderWidth(8)
				.ItemWidth(8)
				[
					SNew(SBox)
					.Padding(Theme.ListContentPadding)
					[
						SNew(STextBlock)
						.Text(this, &SFlareFleetInfo::OnGetCurrentSectorComboLine)
						.TextStyle(&Theme.TextFont)
					]
				]
			]	
		]
		// Button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		[
			SAssignNew(TravelButton,SFlareButton)
			.Width(8)
			.Text(this, &SFlareFleetInfo::GetTravelText)
			.IsDisabled(this, &SFlareFleetInfo::IsTravelDisabled)
			.HelpText(LOCTEXT("TravelInfoFleet", "Start travelling to the selected sector with the selected fleet"))
			.Icon(FFlareStyleSet::GetIcon("Travel"))
			.OnClicked(this, &SFlareFleetInfo::OnTravelHereClicked)
		]
	]
]
]
];

	// Setup
	if (InArgs._Fleet)
	{
		SetFleet(InArgs._Fleet);
	}

	Hide();
}

/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

bool SFlareFleetInfo::IsTravelDisabled() const
{
	if (TargetFleet == NULL)
	{
		return true;
	}
	else if (TargetSectorTravel->IsTravelSector())
	{
		return true;
	}
	else if (!TargetFleet->CanTravel(TargetSectorTravel) || TargetFleet->GetCurrentSector() == TargetSectorTravel)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SFlareFleetInfo::OnTravelHereClicked()
{
	if (TargetFleet)
	{
		bool Escape = TargetFleet->GetCurrentSector()->GetSectorBattleState(TargetFleet->GetFleetCompany()).HasDanger
			&& (TargetFleet != PC->GetPlayerFleet() || TargetFleet->GetShipCount() > 1);
		bool Abandon = TargetFleet->GetImmobilizedShipCount() != 0;

		if (!Abandon && !Escape)
		{
			OnStartTravelConfirmed();
		}
		else
		{
			FText TitleText;
			if (Escape)
			{
				TitleText = LOCTEXT("ConfirmTravelEscapeTitle", "ESCAPE ?");
			}
			else if (Abandon)
			{
				TitleText = LOCTEXT("ConfirmTravelAbandonTitle", "ABANDON SHIPS ?");
			}

			FText ConfirmText = TargetFleet->GetTravelConfirmText();

			if (TargetFleet->GetUnableToTravelShips() < TargetFleet->GetShips().Num())
			{
				// Open the confirmation
				PC->GetMenuManager()->Confirm(TitleText,
					ConfirmText,
					FSimpleDelegate::CreateSP(this, &SFlareFleetInfo::OnStartTravelConfirmed));
			}
		}
	}
}

void SFlareFleetInfo::OnStartTravelConfirmed()
{
	if (TargetFleet&&TargetSectorTravel)
	{
		UFlareSimulatedSector* TravelingFrom = TargetFleet->GetCurrentSector();
		UFlareTravel* OldTravel = TargetFleet->GetCurrentTravel();
		UFlareTravel* Travel = PC->GetGame()->GetGameWorld()->StartTravel(TargetFleet, TargetSectorTravel);
		if (Travel)
		{
			if (PC->GetMenuManager()->GetCurrentMenu() == EFlareMenu::MENU_Fleet)
			{
				PC->GetMenuManager()->GetFleetMenu()->UpdateFleetList(TargetFleet);
			}
			else if (PC->GetMenuManager()->GetCurrentMenu() == EFlareMenu::MENU_Sector)
			{
				PC->GetMenuManager()->GetSectorMenu()->UpdateShipLists();
			}
			else if (PC->GetMenuManager()->GetCurrentMenu() == EFlareMenu::MENU_Orbit)
			{
				PC->GetMenuManager()->GetOrbitMenu()->FleetsBegunTravel(TravelingFrom, TargetSectorTravel, OldTravel);
			}
		}
	}
}

TSharedRef<SWidget> SFlareFleetInfo::OnGenerateSectorComboLine(UFlareSimulatedSector* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FText Name  = Item->GetSectorName();

	return SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(STextBlock)
		.Text(Name)
		.TextStyle(&Theme.TextFont)
	];
}

FText SFlareFleetInfo::GetTravelText() const
{
	if (TargetSectorTravel)
	{
		FText Reason;
		if (!TargetFleet)
		{
			return Reason;
		}

		if (TargetFleet->GetCurrentSector() == TargetSectorTravel)
		{
			return LOCTEXT("TravelAlreadyHereFormat", "Already there");
		}
		else if (TargetSectorTravel->IsTravelSector())
		{
			return LOCTEXT("CantTravelToTravelFormat", "Can't travel to a moving fleet");
		}
		else if (!TargetFleet->CanTravel(Reason, TargetSectorTravel))
		{
			return Reason;
		}
		else
		{
			int64 TravelDuration = UFlareTravel::ComputeTravelDuration(PC->GetGame()->GetGameWorld(), TargetFleet->GetCurrentSector(), TargetSectorTravel, PC->GetCompany());

			FText TravelWord;

			if (TargetFleet->GetCurrentSector()->GetSectorBattleState(TargetFleet->GetFleetCompany()).HasDanger)
			{
				TravelWord = LOCTEXT("EscapeWord", "Escape");
			}
			else
			{
				TravelWord = LOCTEXT("TravelWord", "Travel");
			}

			if (TravelDuration == 1)
			{
				return FText::Format(LOCTEXT("ShortTravelFormat", "{0} (1 day)"), TravelWord);
			}
			else
			{
				return FText::Format(LOCTEXT("TravelFormat", "{0} ({1} days)"), TravelWord, FText::AsNumber(TravelDuration));
			}
		}
	}
	else
	{
		return LOCTEXT("TravelNoSelectionFleet", "No sector selected");
	}
}

void SFlareFleetInfo::OnSectorComboLineSelectionChanged(UFlareSimulatedSector* Item, ESelectInfo::Type SelectInfo)
{
	if (Item)
	{
		TargetSectorTravel = Item;
	}
}

FText SFlareFleetInfo::OnGetCurrentSectorComboLine() const
{
	UFlareSimulatedSector* SelectedSector = SectorSelector->GetSelectedItem();
	if (SelectedSector)
	{
		return SelectedSector->GetSectorName();
	}
	else
	{
		return LOCTEXT("SelectSector", "Select a sector");
	}
}

FText SFlareFleetInfo::GetRepairText() const
{
	if (!TargetFleet)
	{
		return FText();
	}

	UFlareSimulatedSector* TargetSector = TargetFleet->GetCurrentSector();

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

	SectorHelper::GetRepairFleetSupplyNeeds(TargetSector, TargetFleet->GetShips(), NeededFS, TotalNeededFS, MaxDuration, true);
	SectorHelper::GetAvailableFleetSupplyCount(TargetSector, TargetFleet->GetFleetCompany(), OwnedFS, AvailableFS, AffordableFS, nullptr, TargetFleet);

	if (IsRepairDisabled())
	{
		if (TotalNeededFS > 0)
		{
			// Repair needed
			if (TargetSector->IsInDangerousBattle(PC->GetCompany()))
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
		else if (SectorHelper::HasShipRepairing(TargetFleet->GetShips(), PC->GetCompany()))
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

bool SFlareFleetInfo::IsRepairDisabled() const
{
	if (!TargetFleet)
	{
		return true;
	}

	UFlareSimulatedSector* TargetSector = TargetFleet->GetCurrentSector();
	if (!TargetSector || TargetSector->IsInDangerousBattle(PC->GetCompany()))
	{
		return true;
	}

	int32 NeededFS;
	int32 TotalNeededFS;
	int64 MaxDuration;

	SectorHelper::GetRepairFleetSupplyNeeds(TargetSector, TargetFleet->GetShips(), NeededFS, TotalNeededFS, MaxDuration, false);

	if (TotalNeededFS > 0)
	{
		// Repair needed

		int32 AvailableFS;
		int32 OwnedFS;
		int32 AffordableFS;

		SectorHelper::GetAvailableFleetSupplyCount(TargetSector, PC->GetCompany(), OwnedFS, AvailableFS, AffordableFS, nullptr, TargetFleet);

		if (AffordableFS == 0)
		{
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

FText SFlareFleetInfo::GetRefillText() const
{
	if (!TargetFleet)
	{
		return FText();
	}

	UFlareSimulatedSector* TargetSector = TargetFleet->GetCurrentSector();
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

	SectorHelper::GetRefillFleetSupplyNeeds(TargetSector, TargetFleet->GetShips(), NeededFS, TotalNeededFS, MaxDuration, true);
	SectorHelper::GetAvailableFleetSupplyCount(TargetSector, TargetFleet->GetFleetCompany(), OwnedFS, AvailableFS, AffordableFS, nullptr, TargetFleet);

	if (IsRefillDisabled())
	{
		if (TotalNeededFS > 0)
		{
			// Refill needed
			if (TargetSector->IsInDangerousBattle(PC->GetCompany()))
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
		else if (SectorHelper::HasShipRefilling(TargetFleet->GetShips(), PC->GetCompany()))
		{

			// Refill in progress
			return LOCTEXT("RefillInProgress", "Refill in progress...");
		}
		else
		{
			// No refill needed
			return LOCTEXT("NoFleetToRefill", "No ship needs refilling");
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

bool SFlareFleetInfo::IsRefillDisabled() const
{
	if (!TargetFleet)
	{
		return true;
	}

	UFlareSimulatedSector* TargetSector = TargetFleet->GetCurrentSector();

	if (!TargetSector || TargetSector->IsInDangerousBattle(PC->GetCompany()))
	{
		return true;
	}

	int32 NeededFS;
	int32 TotalNeededFS;
	int64 MaxDuration;

	SectorHelper::GetRefillFleetSupplyNeeds(TargetSector, TargetFleet->GetShips(), NeededFS, TotalNeededFS, MaxDuration, false);

	if (TotalNeededFS > 0)
	{
		// Refill needed

		int32 AvailableFS;
		int32 OwnedFS;
		int32 AffordableFS;

		SectorHelper::GetAvailableFleetSupplyCount(TargetSector, TargetFleet->GetFleetCompany(), OwnedFS, AvailableFS, AffordableFS, nullptr, TargetFleet);

		if (AffordableFS == 0)
		{
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

void SFlareFleetInfo::OnRefillClicked()
{
	if (TargetFleet)
	{
		SectorHelper::RefillFleets(TargetFleet->GetCurrentSector(), TargetFleet->GetFleetCompany(), TargetFleet);
	}
}

void SFlareFleetInfo::OnRepairClicked()
{
	if (TargetFleet)
	{
		SectorHelper::RepairFleets(TargetFleet->GetCurrentSector(), TargetFleet->GetFleetCompany(), TargetFleet);
	}
}

void SFlareFleetInfo::SetFleet(UFlareFleet* Fleet)
{
	TargetFleet = Fleet;

	if (TargetFleet && PC)
	{
		CompanyFlag->SetCompany(TargetFleet->GetFleetCompany());
		TargetName = TargetFleet->GetFleetName();
		UpdateSectors();
	}

	// Text font
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	const FTextBlockStyle* TextFont = &Theme.NameFont;
	if (TargetFleet && TargetFleet == PC->GetPlayerShip()->GetCurrentFleet())
	{
		TextFont = &Theme.NameFontBold;
	}
	FleetName->SetTextStyle(TextFont);
	UpdateFleetStatus();
}

void SFlareFleetInfo::UpdateSectors()
{
	// Sector list
	SectorList.Empty();
	UFlareCompany* Company = TargetFleet->GetFleetCompany();
	SectorList.Reserve(Company->GetKnownSectors().Num());
	for (int32 SectorIndex = 0; SectorIndex < Company->GetKnownSectors().Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = Company->GetKnownSectors()[SectorIndex];
//		UFlareSimulatedSector* CurrentSector = TargetFleet->GetCurrentSector();
//		if (Sector != CurrentSector)
		SectorList.Add(Sector);
	}

//	SectorList.Sort(SFlareFleetInfo::SortByName);
	SectorList.Sort([this](UFlareSimulatedSector& A, UFlareSimulatedSector& B)
	{
		return A.GetSectorName().ToString() < B.GetSectorName().ToString();
	});

	SectorSelector->RefreshOptions();

	if (TargetFleet->GetFleetCompany() == PC->GetCompany())
	{
		if (PC->GetMenuManager()->GetCurrentMenu() == EFlareMenu::MENU_Fleet)
		{
			UFlareFleet* FleetToEdit = PC->GetMenuManager()->GetFleetMenu()->GetFleetEditing();
			if (FleetToEdit)
			{
				SectorSelector->SetSelectedItem(FleetToEdit->GetCurrentSector());
			}
			else
			{
				SectorSelector->SetSelectedItem(PC->GetPlayerFleet()->GetCurrentSector());
			}
		}
		else
		{
			SectorSelector->SetSelectedItem(PC->GetPlayerFleet()->GetCurrentSector());
		}
	}
	else
	{
		SectorSelector->SetSelectedIndex(0);
	}

	TargetSectorTravel = SectorSelector->GetSelectedItem();
}
/*
bool SFlareFleetInfo::SortByName(const UFlareSimulatedSector& A, const UFlareSimulatedSector& B)
{
	return A.GetName() < B.GetName();
}
*/
void SFlareFleetInfo::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	if (TargetFleet&&PC)
	{
		UFlareFleet* PCFleet = PC->GetPlayerFleet();
		if (PCFleet&&TargetFleet->GetCurrentSector() == PCFleet->GetCurrentSector())
		{
			UpdateFleetStatus();
		}
	}
}

void SFlareFleetInfo::SetMinimized(bool NewState)
{
	Minimized = NewState;

	if (GetVisibility() == EVisibility::Visible)
	{
		Show();
	}
}

void SFlareFleetInfo::Show()
{
	SetVisibility(EVisibility::Visible);

	if (Minimized)
	{
		InspectButton->SetVisibility(EVisibility::Collapsed);
		TradeRouteButton->SetVisibility(EVisibility::Collapsed);
		AutoTradeButton->SetVisibility(EVisibility::Collapsed);
		RefillButton->SetVisibility(EVisibility::Collapsed);
		RepairButton->SetVisibility(EVisibility::Collapsed);
		SectorSelector->SetVisibility(EVisibility::Collapsed);
		TravelButton->SetVisibility(EVisibility::Collapsed);
	}

	else if (TargetFleet && TargetFleet->IsValidLowLevel())
	{
		if (TargetFleet->GetFleetCompany() == TargetFleet->GetGame()->GetPC()->GetCompany())
		{
			InspectButton->SetVisibility(EVisibility::Visible);
			TradeRouteButton->SetVisibility(EVisibility::Visible);
			AutoTradeButton->SetVisibility(EVisibility::Visible);
			RefillButton->SetVisibility(EVisibility::Visible);
			RepairButton->SetVisibility(EVisibility::Visible);
			SectorSelector->SetVisibility(EVisibility::Visible);
			TravelButton->SetVisibility(EVisibility::Visible);
			AutoTradeButton->SetActive(TargetFleet->IsAutoTrading());
		}
		else
		{
			InspectButton->SetVisibility(EVisibility::Collapsed);
			TradeRouteButton->SetVisibility(EVisibility::Collapsed);
			AutoTradeButton->SetVisibility(EVisibility::Collapsed);
			RefillButton->SetVisibility(EVisibility::Collapsed);
			RepairButton->SetVisibility(EVisibility::Collapsed);
			SectorSelector->SetVisibility(EVisibility::Collapsed);
			TravelButton->SetVisibility(EVisibility::Collapsed);
		}
	}
}

void SFlareFleetInfo::Hide()
{
	TargetFleet = NULL;
	SetVisibility(EVisibility::Collapsed);
}


void SFlareFleetInfo::UpdateFleetStatus()
{
	FLinearColor Result = FLinearColor::Black;
	Result.A = 0.0f;

	if (TargetFleet)
	{
		TArray<UFlareSimulatedSpacecraft*> FleetShips = TargetFleet->GetShips();
		IsStranded = false;
		IsUncontrollable = false;
		IsDisarmed = false;
		NeedRefill = false;
		IsRepairing = false;
		FleetHealth = 0;

		for (int ShipIndex = 0; ShipIndex < FleetShips.Num(); ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = FleetShips[ShipIndex];

			// Ignore stations
			if (Ship && Ship->IsValidLowLevel() && !Ship->IsStation())
			{
				UFlareSimulatedSpacecraftDamageSystem* DamageSystem = Ship->GetDamageSystem();

				if (!IsStranded)
				{
					IsStranded = DamageSystem->IsStranded();
				}

				if (!IsUncontrollable)
				{
					IsUncontrollable = DamageSystem->IsUncontrollable();
				}

				if (Ship->IsMilitary())
				{
					if (!IsDisarmed)
					{
						IsDisarmed = DamageSystem->IsDisarmed();
					}
				}

				if (Ship->GetRefillStock() > 0 && Ship->NeedRefill())
				{
					NeedRefill = true;
				}
				if (Ship->GetRepairStock() > 0 && Ship->GetDamageSystem()->GetGlobalDamageRatio() < 1.f)
				{
					IsRepairing = true;
				}
			}
			FleetHealth += Ship->GetDamageSystem()->GetGlobalHealth();
		}
		FleetHealth /= FleetShips.Num();
	}
}

/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareFleetInfo::OnInspect()
{
	if (PC && TargetFleet)
	{
		FLOGV("SFlareFleetInfo::OnInspect : TargetFleet=%p", TargetFleet);
		FFlareMenuParameterData Data;
		Data.Fleet = TargetFleet;
		PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_Fleet, Data);
	}
}

void SFlareFleetInfo::OnOpenTradeRoute()
{
	if (PC && TargetFleet && TargetFleet->GetCurrentTradeRoute())
	{
		FLOGV("SFlareFleetInfo::OnOpenTradeRoute : TargetFleet=%p", TargetFleet);
		FFlareMenuParameterData Data;
		Data.Route = TargetFleet->GetCurrentTradeRoute();
		PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_TradeRoute, Data);
	}
}

void SFlareFleetInfo::OnToggleAutoTrade()
{
	if (PC && TargetFleet)
	{
		FLOGV("SFlareFleetInfo::OnToggleAutoTrade : TargetFleet=%p", TargetFleet);
		TargetFleet->SetAutoTrading(!TargetFleet->IsAutoTrading());
		AutoTradeButton->SetActive(TargetFleet->IsAutoTrading());
	}
}


/*----------------------------------------------------
	Content
----------------------------------------------------*/

TOptional<float> SFlareFleetInfo::GetGlobalHealth() const
{
	return FleetHealth;
}

EVisibility SFlareFleetInfo::GetRefillingVisibility() const
{
	if (NeedRefill)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SFlareFleetInfo::GetRepairingVisibility() const
{
	if (IsRepairing)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

FText SFlareFleetInfo::GetName() const
{
	return TargetName;
}

FSlateColor SFlareFleetInfo::GetTextColor() const
{
	FLinearColor Result;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	if (TargetFleet && TargetFleet->IsValidLowLevel())
	{
		if (TargetFleet->GetFleetCompany()->GetWarState(PC->GetCompany()) == EFlareHostility::Owned)
		{
			return TargetFleet->GetFleetColor();
		}
		else if (TargetFleet->GetFleetCompany()->GetWarState(PC->GetCompany()) == EFlareHostility::Hostile)
		{
			return Theme.EnemyColor;
		}
		else
		{
			return Theme.NeutralColor;
		}
	}

	return Result;
}

FText SFlareFleetInfo::GetInspectHintText() const
{
	if (TargetFleet->IsTraveling())
	{
		return LOCTEXT("CantEditTravelFleet", "Can't edit travelling fleets");
	}
	else if (TargetFleet->GetCurrentSector()->IsPlayerBattleInProgress())
	{
		return LOCTEXT("CantEditBattleFleet", "Can't edit fleets during battles");
	}
	else
	{
		return LOCTEXT("EditInfo", "Change the composition of this fleet");
	}
}

bool SFlareFleetInfo::IsInspectDisabled() const
{
	if (TargetFleet->IsTraveling())
	{
		return true;
	}
	else if (TargetFleet->GetCurrentSector()->IsPlayerBattleInProgress())
	{
		return true;
	}
	else
	{
		return false;
	}
}

FText SFlareFleetInfo::GetInspectTradeRouteHintText() const
{
	if (TargetFleet && TargetFleet->GetCurrentTradeRoute())
	{
		return LOCTEXT("TradeRouteInfo", "Edit the trade route this fleet is assigned to");
	}
	else
	{
		return LOCTEXT("CantEditNoTradeRoute", "Assign a trade route to this fleet in the company menu");
	}
}

bool SFlareFleetInfo::IsInspectTradeRouteDisabled() const
{
	if (TargetFleet && TargetFleet->GetCurrentTradeRoute())
	{
		return false;
	}
	else
	{
		return true;
	}
}

FText SFlareFleetInfo::GetAutoTradeHintText() const
{
	if (!TargetFleet->GetFleetCompany()->IsTechnologyUnlocked("auto-trade"))
	{
		return LOCTEXT("CantAutoTradeNoTech", "The Automated Trading technology is required");
	}
	else if (TargetFleet->GetCurrentTradeRoute())
	{
		return LOCTEXT("CantAutoTradeOnTradeRoute", "Fleets assigned to a trade route can't do automatic trading");
	}
	else if (TargetFleet == TargetFleet->GetGame()->GetPC()->GetPlayerFleet())
	{
		return LOCTEXT("CantAutoTradeWithPlayerFleet", "Your personal fleet can't trade automatically");
	}
	else
	{
		return LOCTEXT("AutoTradeInfo", "Command this fleet to start automatic trading");
	}
}

bool SFlareFleetInfo::IsAutoTradeDisabled() const
{
	if (!TargetFleet->GetFleetCompany()->IsTechnologyUnlocked("auto-trade"))
	{
		return true;
	}
	else if (TargetFleet->GetCurrentTradeRoute())
	{
		return true;
	}
	else if (TargetFleet == TargetFleet->GetGame()->GetPC()->GetPlayerFleet())
	{
		return true;
	}
	else
	{
		return false;
	}
}

FSlateColor SFlareFleetInfo::GetIconColor(EFlareSubsystem::Type Type) const
{
	FLinearColor Result = FLinearColor::Black;
	Result.A = 0.0f;
	bool IsIncapacitated = false;
	switch (Type)
	{
	case EFlareSubsystem::SYS_Propulsion:    IsIncapacitated = IsStranded;         break;
	case EFlareSubsystem::SYS_RCS:           IsIncapacitated = IsUncontrollable;   break;
	case EFlareSubsystem::SYS_Weapon:        IsIncapacitated = IsDisarmed;         break;
	default: break;
	}
	// Show in red when disabled
	if (IsIncapacitated)
	{
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
		Result = Theme.DamageColor;
		Result.A = 1.0f;
	}
	return Result;
}

FText SFlareFleetInfo::GetComposition() const
{
	FText Result;

	if (TargetFleet)
	{
		FText SingleShip = LOCTEXT("ShipSingle", "ship");
		FText MultipleShips = LOCTEXT("ShipPlural", "ships");
		int32 HeavyShipCount = TargetFleet->GetMilitaryShipCountBySize(EFlarePartSize::L);
		int32 LightShipCount = TargetFleet->GetMilitaryShipCountBySize(EFlarePartSize::S);
		int32 CivilianShipCount = TargetFleet->GetShipCount() - HeavyShipCount - LightShipCount;

		// Fighters
		FText LightShipText;
		if (LightShipCount > 0)
		{
			LightShipText = FText::Format(LOCTEXT("FleetCompositionLightFormat", "{0} light"),
				FText::AsNumber(LightShipCount));
		}

		// Heavies
		FText HeavyShipText;
		if (HeavyShipCount > 0)
		{
			HeavyShipText = FText::Format(LOCTEXT("FleetCompositionHeavyFormat", "{0} heavy"),
				FText::AsNumber(HeavyShipCount));

			if (LightShipCount > 0)
			{
				HeavyShipText = FText::FromString(", " + HeavyShipText.ToString());
			}
		}

		// Civilians
		FText CivilianShipText;
		if (CivilianShipCount > 0)
		{
			CivilianShipText = FText::Format(LOCTEXT("FleetCompositionCivilianFormat", "{0} civilian"),
				FText::AsNumber(CivilianShipCount));

			if (LightShipCount > 0 || HeavyShipCount > 0)
			{
				CivilianShipText = FText::FromString(", " + CivilianShipText.ToString());
			}
		}

		Result = FText::Format(LOCTEXT("FleetCompositionFormat", "({0}{1}{2})"), LightShipText, HeavyShipText, CivilianShipText);
	}

	return Result;
}

FText SFlareFleetInfo::GetCombatValue() const
{
	FText Result;

	if (TargetFleet)
	{
		if (TargetFleet->GetCombatPoints(true) > 0 || TargetFleet->GetCombatPoints(false) > 0)
		{
			Result = FText::Format(LOCTEXT("GetCombatValueFormat", "{0}/{1}"),
				FText::AsNumber(TargetFleet->GetCombatPoints(true)),
				FText::AsNumber(TargetFleet->GetCombatPoints(false)));
		}
		else
		{
			Result = LOCTEXT("GetCombatValueZero", "0");
		}
	}

	return Result;
}

FText SFlareFleetInfo::GetDescription() const
{
	FText Result;

	if (TargetFleet)
	{
		FText FleetAssignedText;
		if (TargetFleet->GetCurrentTradeRoute())
		{
			FleetAssignedText = UFlareGameTools::AddLeadingSpace(FText::Format(LOCTEXT("FleetAssignedFormat", "- {0}{1}"),
				TargetFleet->GetCurrentTradeRoute()->GetTradeRouteName(),
				(TargetFleet->GetCurrentTradeRoute()->IsPaused() ? UFlareGameTools::AddLeadingSpace(LOCTEXT("FleetTradeRoutePausedFormat", "(Paused)")) : FText())));
		}

		FText FleetDescriptionText = FText::Format(LOCTEXT("FleetFormat", "{0} ({1} / {2}){3}"),
			TargetFleet->GetFleetName(),
			FText::AsNumber(TargetFleet->GetShipCount()),
			FText::AsNumber(TargetFleet->GetMaxShipCount()),
			FleetAssignedText);

		Result = FText::Format(LOCTEXT("FleetInfoFormat", "{0} - {1}"), TargetFleet->GetStatusInfo(), FleetDescriptionText);
	}

	return Result;
}

 
EVisibility SFlareFleetInfo::GetCompanyFlagVisibility() const
{
	// Crash mitigation - If parent is hidden, so are we, don't try to use the target (#178)
	if ((OwnerWidget.IsValid() && OwnerWidget->GetVisibility() != EVisibility::Visible)
		|| (PC && !PC->GetMenuManager()->IsUIOpen()))
	{
		return EVisibility::Collapsed;
	}

	// Check the target
	if (TargetFleet && TargetFleet->IsValidLowLevel())
	{
		UFlareCompany* TargetCompany = TargetFleet->GetFleetCompany();
		if (TargetCompany && PC && TargetCompany == PC->GetCompany())
		{
			return EVisibility::Collapsed;
		}
	}

	return EVisibility::Visible;
}


#undef LOCTEXT_NAMESPACE
