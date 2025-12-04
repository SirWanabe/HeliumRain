
#include "FlareOrbitalMenu.h"
#include "../../Flare.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareGameTools.h"
#include "../../Economy/FlareFactory.h"
#include "../../Data/FlareOrbitalMap.h"
#include "../../Data/FlareSpacecraftCatalog.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Spacecrafts/FlareSpacecraft.h"
#include "../Components/FlareSectorButton.h"


#define LOCTEXT_NAMESPACE "FlareOrbitalMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareOrbitalMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	Game = MenuManager->GetPC()->GetGame();
	ShipyardMenuOpen = false;

	// FF setup
	FastForwardPeriod = 0.5f;
	FastForwardStopRequested = false;

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SOverlay)

		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetImage("Sun"))
		]

		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
		
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.MaxWidth(1024)
				[
					SNew(SVerticalBox)
					// World status
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Top)
					.Padding(Theme.ContentPadding)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareOrbitalMenu::GetDateText)
					]
				]

				// Display modes
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(Theme.SmallContentPadding)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Width(3)
						.Text(LOCTEXT("DisplayModeFleets", "Fleets"))
						.HelpText(LOCTEXT("DisplayModeFleetsInfo", "Display fleets on the map"))
						.OnClicked(this, &SFlareOrbitalMenu::SetDisplayMode, EFlareOrbitalMode::Fleets)
						.IsDisabled(this, &SFlareOrbitalMenu::IsCurrentDisplayMode, EFlareOrbitalMode::Fleets)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Width(3)
						.Text(LOCTEXT("DisplayModeStations", "Stations"))
						.HelpText(LOCTEXT("DisplayModeStationsInfo", "Display stations on the map"))
						.OnClicked(this, &SFlareOrbitalMenu::SetDisplayMode, EFlareOrbitalMode::Stations)
						.IsDisabled(this, &SFlareOrbitalMenu::IsCurrentDisplayMode, EFlareOrbitalMode::Stations)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Width(3)
						.Text(LOCTEXT("DisplayModeShips", "Ships"))
						.HelpText(LOCTEXT("DisplayModeShipsInfo", "Display ships on the map"))
						.OnClicked(this, &SFlareOrbitalMenu::SetDisplayMode, EFlareOrbitalMode::Ships)
						.IsDisabled(this, &SFlareOrbitalMenu::IsCurrentDisplayMode, EFlareOrbitalMode::Ships)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Width(3)
						.Text(LOCTEXT("DisplayModeBattles", "Battles"))
						.HelpText(LOCTEXT("DisplayModeBattlesInfo", "Display battles on the map"))
						.OnClicked(this, &SFlareOrbitalMenu::SetDisplayMode, EFlareOrbitalMode::Battles)
						.IsDisabled(this, &SFlareOrbitalMenu::IsCurrentDisplayMode, EFlareOrbitalMode::Battles)
					]
				]

				// Skip day
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				.Padding(Theme.SmallContentPadding)
				[
					SNew(SFlareButton)
					.Width(3.5)
					.Text(FText::Format(LOCTEXT("FastForwardSingleFormat", "Skip day ({0})"),
						FText::FromString(AFlareMenuManager::GetKeyNameFromActionName("Simulate"))))
					.Icon(FFlareStyleSet::GetIcon("Load_Small"))
					.OnClicked(this, &SFlareOrbitalMenu::OnFastForwardClicked)
					.IsDisabled(this, &SFlareOrbitalMenu::IsFastForwardDisabled)
					.HelpText(LOCTEXT("FastForwardOneDayInfo", "Wait for one day - Travels, production, building will be accelerated"))
				]

				// Fast forward
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				.Padding(Theme.SmallContentPadding)
				[
					SAssignNew(FastForwardAuto, SFlareButton)
					.Width(4.5)
					.Toggle(true)
					.Text(this, &SFlareOrbitalMenu::GetFastForwardText)
					.Icon(this, &SFlareOrbitalMenu::GetFastForwardIcon)
					.OnClicked(this, &SFlareOrbitalMenu::OnFastForwardAutomaticClicked)
					.IsDisabled(this, &SFlareOrbitalMenu::IsFastForwardDisabled)
					.HelpText(LOCTEXT("FastForwardInfo", "Wait for the next event - Travels, production, building will be accelerated"))
				]
			]
		
			// Planetarium body
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SHorizontalBox)
			
				// Action column 
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SScrollBox)
					.Style(&Theme.ScrollBoxStyle)
					.ScrollBarStyle(&Theme.ScrollBarStyle)
		

					+ SScrollBox::Slot()
					[
						// Hide Events			
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.Padding(Theme.SmallContentPadding)
						.AutoWidth()
					[
							SAssignNew(ShowEventsButton, SFlareButton)
							.Width(3)
							.Text(LOCTEXT("ShowEvents", "Events"))
							.HelpText(LOCTEXT("ShowEventsInfo", "Toggle to enable events section"))
							.Toggle(true)
						]
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.Padding(Theme.SmallContentPadding)
						.AutoWidth()
						[
							SAssignNew(ShowTradeButton, SFlareButton)
							.Width(3)
							.Text(LOCTEXT("ShowTrade", "Trade"))
							.HelpText(LOCTEXT("ShowTradeInfo", "Toggle to enable trade routes section"))
							.Toggle(true)
						]
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.Padding(Theme.SmallContentPadding)
						.AutoWidth()
						[
							SAssignNew(ShowFleetButton, SFlareButton)
							.Width(3)
							.Text(LOCTEXT("ShowFleet", "Fleet"))
							.HelpText(LOCTEXT("ShowFleetInfo", "Toggle to enable fleets section"))
							.Toggle(true)
						]
					]

					// Events Text
					+SScrollBox::Slot()
					.HAlign(HAlign_Left)
					[
						SNew(SFlareTravelEventsInfo)
						.MenuManager(MenuManager)
						.Visibility(this, &SFlareOrbitalMenu::IsEventsVisible)
					]

					// Trade route list
					+ SScrollBox::Slot()
					.HAlign(HAlign_Left)
					[
						SAssignNew(TradeRouteInfo, SFlareTradeRouteInfo)
						.MenuManager(MenuManager)
						.Visibility(this, &SFlareOrbitalMenu::IsTradeVisible)
					]

					// Fleets 
					+ SScrollBox::Slot()
					.HAlign(HAlign_Left)
					[
						SAssignNew(OrbitalFleetsInfo, SFlareOrbitalFleetInfo)
						.MenuManager(MenuManager)
						.Visibility(this, &SFlareOrbitalMenu::IsAutomatedFleetsVisible)
					]

					// Automated Fleets 
					+ SScrollBox::Slot()
					.HAlign(HAlign_Left)
					[
						SAssignNew(AutomatedFleetsInfo, SFlareAutomatedFleetsInfo)
						.MenuManager(MenuManager)
						.Visibility(this, &SFlareOrbitalMenu::IsAutomatedFleetsVisible)
					]
				]
				// Moons 1
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SAssignNew(AnkaBox, SFlarePlanetaryBox)
					]

					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SAssignNew(HelaBox, SFlarePlanetaryBox)
					]
				]

				// Nema
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Top)
				[
					SAssignNew(NemaBox, SFlarePlanetaryBox)
				]
			
				// Moons 2
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Top)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Top)
					.Padding(FMargin(0, 100, 0, 50))
					[
						SAssignNew(AstaBox, SFlarePlanetaryBox)
					]

					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Top)
					[
						SAssignNew(AdenaBox, SFlarePlanetaryBox)
					]

					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
				]
			]
		]
	];
	ShowEventsButton->SetActive(true);
	ShowTradeButton->SetActive(true);
	ShowFleetButton->SetActive(true);
}

/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareOrbitalMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareOrbitalMenu::Enter()
{
	FLOG("SFlareOrbitalMenu::Enter");
	
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	if (!DisplayMode)
	{
		DisplayMode = EFlareOrbitalMode::Fleets;
		PreviousDisplayMode = DisplayMode;
	}

	// Update stuff
	UpdateSectorBattleStates();
	StopFastForward();
	UpdateMap();
	TradeRouteInfo->Update();
	AutomatedFleetsInfo->Update();
	OrbitalFleetsInfo->Update();

	Game->SaveGame(MenuManager->GetPC(), true);

	if (ShipyardMenuOpen)
	{
		OnShipyard();
	}
}

void SFlareOrbitalMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);

	NemaBox->ClearChildren();
	AnkaBox->ClearChildren();
	AstaBox->ClearChildren();
	HelaBox->ClearChildren();
	AdenaBox->ClearChildren();

	TradeRouteInfo->Clear();
	AutomatedFleetsInfo->Clear();
	OrbitalFleetsInfo->Clear();
	StopFastForward();
	PreviouslySelectedSector = NULL;
}


void SFlareOrbitalMenu::SetShipyardOpen(bool ShipyardOpen)
{
	ShipyardMenuOpen = ShipyardOpen;
}

void SFlareOrbitalMenu::StopFastForward()
{
	TimeSinceFastForward = 0;
	FastForwardStopRequested = false;
	FastForwardAuto->SetActive(false);

	if (FastForwardActive)
	{
		FLOG("Stop fast forward");
		FastForwardActive = false;
		Game->SaveGame(MenuManager->GetPC(), true);
		Game->ActivateCurrentSector();
		OrbitalFleetsInfo->Update();
	}
}

void SFlareOrbitalMenu::RequestStopFastForward()
{
	FastForwardStopRequested = true;
}

void SFlareOrbitalMenu::RequestOrbitalFleetsUpdate(bool Instant)
{
	if (Instant)
	{
		OrbitalFleetsInfo->Update();
	}
	else
	{
		OrbitalFleetsUpdateRequested = true;
	}
}

UFlareFleet* SFlareOrbitalMenu::GetCurrentSelectedFleet()
{
	return OrbitalFleetsInfo->GetSelectedFleet();
}

void SFlareOrbitalMenu::UpdateSectorStates()
{
	RefreshTrackedButtons();
	UpdateSectorBattleStates();
}

void SFlareOrbitalMenu::UpdateSectorBattleStates()
{
	for (UFlareSimulatedSector* Sector : MenuManager->GetPC()->GetCompany()->GetKnownSectors())
	{
		Sector->UpdateSectorBattleState(MenuManager->GetPC()->GetCompany());
	}
}

void SFlareOrbitalMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (IsEnabled() && MenuManager.IsValid())
	{
		// Fast forward every FastForwardPeriod
		TimeSinceFastForward += InDeltaTime;
		if (FastForwardActive)
		{
			for (UFlareSimulatedSector* Sector : MenuManager->GetPC()->GetCompany()->GetKnownSectors())
			{
				MenuManager->GetPC()->CheckSectorStateChanges(Sector);
			}

			if (!FastForwardStopRequested && (TimeSinceFastForward > FastForwardPeriod || UFlareGameTools::FastFastForward))
			{
				MenuManager->GetGame()->GetGameWorld()->FastForward();
				TimeSinceFastForward = 0;
				RefreshTrackedButtons();
			}

			if (OrbitalFleetsUpdateRequested)
			{
				OrbitalFleetsUpdateRequested = false;
				OrbitalFleetsInfo->Update();
			}

			// Stop request
			if (FastForwardStopRequested)
			{
				StopFastForward();
			}
		}
	}
}

void SFlareOrbitalMenu::UpdateMap()
{
	TArray<FFlareSectorCelestialBodyDescription>& OrbitalBodies = Game->GetOrbitalBodies()->OrbitalBodies;

	SectorButtons.Empty();
	SectorButtons.Reserve(MenuManager->GetPC()->GetCompany()->GetKnownSectors().Num());
	UpdateMapForBody(NemaBox, &OrbitalBodies[0]);
	UpdateMapForBody(AnkaBox, &OrbitalBodies[1]);
	UpdateMapForBody(AstaBox, &OrbitalBodies[2]);
	UpdateMapForBody(HelaBox, &OrbitalBodies[3]);
	UpdateMapForBody(AdenaBox, &OrbitalBodies[4]);
}

struct FSortByAltitudeAndPhase
{
	inline bool operator()(UFlareSimulatedSector& SectorA, UFlareSimulatedSector& SectorB) const
	{
		if (SectorA.GetOrbitParameters()->Altitude == SectorB.GetOrbitParameters()->Altitude)
		{
			// Compare phase
			if(SectorA.GetOrbitParameters()->Phase == SectorB.GetOrbitParameters()->Phase)
			{
				FLOGV("WARNING: %s and %s are at the same phase", *SectorA.GetSectorName().ToString(), *SectorB.GetSectorName().ToString())
			}

			return SectorA.GetOrbitParameters()->Phase < SectorB.GetOrbitParameters()->Phase;
		}
		else
		{
			return (SectorA.GetOrbitParameters()->Altitude < SectorB.GetOrbitParameters()->Altitude);
		}
	}
};

void SFlareOrbitalMenu::UpdateMapForBody(TSharedPtr<SFlarePlanetaryBox> Map, const FFlareSectorCelestialBodyDescription* Body)
{
	// Setup the planetary map
	Map->SetPlanetImage(&Body->CelestialBodyPicture);
	Map->SetRadius(Body->CelestialBodyRadiusOnMap, 110);
	Map->ClearChildren();

	// Find highest altitude
	int32 MaxAltitude = 0;

	for (UFlareSimulatedSector* Sector : MenuManager->GetGame()->GetGameWorld()->GetSectors())
	{
		if (Sector->GetOrbitParameters()->Altitude > MaxAltitude
		 && Sector->GetOrbitParameters()->CelestialBodyIdentifier == Body->CelestialBodyIdentifier)
		{
			MaxAltitude = Sector->GetOrbitParameters()->Altitude;
		}
	}
	// Add the name
	Map->AddSlot()
	.Altitude(MaxAltitude)
	.Phase(0)
	[
		SNew(STextBlock)
		.TextStyle(&FFlareStyleSet::GetDefaultTheme().SubTitleFont)
		.Text(Body->CelestialBodyName)
	];

	// Get sectors
	TArray<UFlareSimulatedSector*> KnownSectors = MenuManager->GetPC()->GetCompany()->GetKnownSectors();
	KnownSectors = KnownSectors.FilterByPredicate(
		[&](UFlareSimulatedSector* Sector)
		{
			return Sector->GetOrbitParameters()->CelestialBodyIdentifier == Body->CelestialBodyIdentifier;
		});
	KnownSectors.Sort(FSortByAltitudeAndPhase());

	// Add the sectors
	for (int32 SectorIndex = 0; SectorIndex < KnownSectors.Num(); SectorIndex++)
	{
		UFlareSimulatedSector* Sector = KnownSectors[SectorIndex];
		TSharedPtr<int32> IndexPtr(new int32(MenuManager->GetPC()->GetCompany()->GetKnownSectors().Find(Sector)));
		double Altitude = Sector->GetOrbitParameters()->Altitude;
		double Phase = Sector->GetOrbitParameters()->Phase;

		Map->AddSlot()
		.Altitude(Altitude)
		.Phase(Phase)
		[
			SAssignNew(CurrentSectorButton, SFlareSectorButton)
			.Sector(Sector)
			.PlayerCompany(MenuManager->GetPC()->GetCompany())
			.OnClicked(this, &SFlareOrbitalMenu::OnOpenSector, IndexPtr)
		];
		SectorButtons.AddUnique(CurrentSectorButton);
	}
	CurrentSectorButton = NULL;
	RefreshTrackedButtons();
}

EFlareOrbitalMode::Type SFlareOrbitalMenu::GetDisplayMode() const
{
	return DisplayMode;
}

float SFlareOrbitalMenu::GetTimeSinceFFWD() const
{
	return TimeSinceFastForward;
}

bool SFlareOrbitalMenu::GetFastForwardActive() const
{
	return FastForwardActive;
}

bool SFlareOrbitalMenu::IsCurrentDisplayMode(EFlareOrbitalMode::Type Mode) const
{
	return (Mode == GetDisplayMode());
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareOrbitalMenu::RefreshTrackedButtons()
{
	for (TSharedPtr<SFlareSectorButton> Button : SectorButtons)
	{
		if (Button.IsValid())
		{
			Button->RefreshButton();
		}
		else
		{
			SectorButtons.RemoveSwap(Button);
		}
	}
}

void SFlareOrbitalMenu::FleetsBegunTravel(UFlareSimulatedSector* TravelingFrom, UFlareSimulatedSector* TravelingTo, UFlareTravel* OldTravel)
{
	UFlareSimulatedSector* TravellingSector = nullptr;
	if (OldTravel && OldTravel->GetOldDestinationSector())
	{
		TravellingSector = OldTravel->GetOldDestinationSector();
	}

	for (TSharedPtr<SFlareSectorButton> Button : SectorButtons)
	{
		if (Button.IsValid())
		{
			if (Button->GetSector() == TravelingFrom || Button->GetSector() == TravelingTo || Button->GetSector() == TravellingSector)
			{
				Button->RefreshButton();
			}
		}
		else
		{
			SectorButtons.RemoveSwap(Button);
		}
	}
}

FText SFlareOrbitalMenu::GetFastForwardText() const
{
	if (!IsEnabled())
	{
		return FText();
	}

	if (!FastForwardAuto->IsActive())
	{
		bool BattleInProgress = false;
		bool BattleLostWithRetreat = false;
		bool BattleLostWithoutRetreat = false;

		for (int32 SectorIndex = 0; SectorIndex < MenuManager->GetPC()->GetCompany()->GetKnownSectors().Num(); SectorIndex++)
		{
			UFlareSimulatedSector* Sector = MenuManager->GetPC()->GetCompany()->GetKnownSectors()[SectorIndex];

			FFlareSectorBattleState BattleState = Sector->GetSectorBattleState(MenuManager->GetPC()->GetCompany());
			if (BattleState.InBattle)
			{
				if(BattleState.InFight)
				{
					BattleInProgress = true;
				}
				else if (!BattleState.BattleWon)
				{
					if(BattleState.RetreatPossible)
					{
						BattleLostWithRetreat = true;
					}
					else
					{
						BattleLostWithoutRetreat = true;
					}
				}
			}
		}

		if (BattleInProgress)
		{
			return LOCTEXT("NoFastForwardBattleText", "Battle in progress");
		}
		else if (BattleLostWithRetreat)
		{
			return LOCTEXT("FastForwardBattleLostWithRetreatText", "Fast forward (!)");
		}
		else if (BattleLostWithoutRetreat)
		{
			return LOCTEXT("FastForwardBattleLostWithoutRetreatText", "Fast forward (!)");
		}
		else
		{
			return LOCTEXT("FastForwardText", "Fast forward");
		}
	}
	else
	{
		return LOCTEXT("FastForwardingText", "Fast forwarding...");
	}
}

const FSlateBrush* SFlareOrbitalMenu::GetFastForwardIcon() const
{
	if (FastForwardAuto->IsActive())
	{
		return FFlareStyleSet::GetIcon("Stop");
	}
	else
	{
		return FFlareStyleSet::GetIcon("FastForward");
	}
}

bool SFlareOrbitalMenu::IsFastForwardDisabled() const
{
	if (IsEnabled())
	{
		UFlareWorld* GameWorld = MenuManager->GetGame()->GetGameWorld();
		
		if (GameWorld && (GameWorld->GetTravels().Num() > 0 || true)) // Not true if there is pending todo event
		{
			return false;
		}
	}

	return true;
}

FText SFlareOrbitalMenu::GetDateText() const
{
	if (IsEnabled())
	{
		UFlareWorld* GameWorld = MenuManager->GetGame()->GetGameWorld();
		UFlareCompany* PlayerCompany = MenuManager->GetPC()->GetCompany();

		if (GameWorld && PlayerCompany)
		{
			int64 Credits = PlayerCompany->GetMoney();
			FText DateText = UFlareGameTools::GetDisplayDate(GameWorld->GetDate());
			return FText::Format(LOCTEXT("DateCreditsInfoFormat", "Date : {0} - {1} credits"), DateText, FText::AsNumber(UFlareGameTools::DisplayMoney(Credits)));
		}
	}
	return FText();
}

FText SFlareOrbitalMenu::GetTravelText() const
{
	if (IsEnabled())
	{
		UFlareWorld* GameWorld = MenuManager->GetGame()->GetGameWorld();
		if (GameWorld)
		{
			TArray<FFlareIncomingEvent> IncomingEvents = GameWorld->GetIncomingEvents();

			// Generate list
			FString Result;
			for (FFlareIncomingEvent& Event : IncomingEvents)
			{
				Result += Event.Text.ToString() + "\n";
			}
			if (Result.Len() == 0)
			{
				Result = LOCTEXT("NoTravel", "No event.").ToString();
			}

			return FText::FromString(Result);
		}
	}

	return FText();
}

FVector2D SFlareOrbitalMenu::GetWidgetSize(int32 Index) const
{
	int WidgetSize = 200;
	FVector2D BaseSize(WidgetSize, WidgetSize);
	return BaseSize;
}


/*----------------------------------------------------
	Action callbacks
----------------------------------------------------*/

void SFlareOrbitalMenu::OnShipyard()
{
	ShipyardMenuOpen = true;
	MenuManager->OpenShipyardOrder();
}

void SFlareOrbitalMenu::SetDisplayMode(EFlareOrbitalMode::Type Mode)
{
	if (Mode != PreviousDisplayMode)
	{
		PreviousDisplayMode = Mode;
		DisplayMode = Mode;
		RefreshTrackedButtons();
	}
}

void SFlareOrbitalMenu::OnOpenSector(TSharedPtr<int32> Index)
{
	UFlareSimulatedSector* Sector = MenuManager->GetPC()->GetCompany()->GetKnownSectors()[*Index];
	PreviouslySelectedSector = Sector;
	if (ShowFleetButton->IsActive() && OrbitalFleetsInfo->GetSelectedFleet() && (
	   (!OrbitalFleetsInfo->GetSelectedFleet()->GetCurrentTravel() && OrbitalFleetsInfo->GetSelectedFleet()->GetCurrentSector() != Sector) 
	|| (OrbitalFleetsInfo->GetSelectedFleet()->GetCurrentTravel() && OrbitalFleetsInfo->GetSelectedFleet()->GetCurrentTravel()->GetDestinationSector() != Sector)))
	{
		UFlareFleet* TargetFleet = OrbitalFleetsInfo->GetSelectedFleet();
		bool Escape = TargetFleet->GetCurrentSector()->GetSectorBattleState(TargetFleet->GetFleetCompany()).HasDanger
			&& (TargetFleet != MenuManager->GetPC()->GetPlayerFleet() || TargetFleet->GetShipCount() > 1);
		bool Abandon = TargetFleet->GetImmobilizedShipCount() != 0;

		if (!Abandon && !Escape)
		{
			OnStartSelectedFleetTravelConfirmed();
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
				MenuManager->Confirm(TitleText,
					ConfirmText,
					FSimpleDelegate::CreateSP(this, &SFlareOrbitalMenu::OnStartSelectedFleetTravelConfirmed),
					FSimpleDelegate::CreateSP(this, &SFlareOrbitalMenu::OnStartSelectedFleetTravelCanceled));
			}
		}
	}
	else
	{
		FFlareMenuParameterData Data;
		Data.Sector = Sector;
		MenuManager->OpenMenu(EFlareMenu::MENU_Sector, Data);
	}
}

void SFlareOrbitalMenu::OnStartSelectedFleetTravelConfirmed()
{
	if (ShowFleetButton->IsActive() && OrbitalFleetsInfo->GetSelectedFleet())
	{
		UFlareFleet* TargetFleet = OrbitalFleetsInfo->GetSelectedFleet();
		if (TargetFleet)
		{
			UFlareSimulatedSector* TravelingFrom = TargetFleet->GetCurrentSector();
			UFlareTravel* OldTravel = TargetFleet->GetCurrentTravel();
			UFlareTravel* Travel = MenuManager->GetGame()->GetGameWorld()->StartTravel(TargetFleet, PreviouslySelectedSector);
			if (Travel)
			{
				FleetsBegunTravel(TravelingFrom, PreviouslySelectedSector, OldTravel);
			}
		}
	}
}

void SFlareOrbitalMenu::OnStartSelectedFleetTravelCanceled()
{
}

void SFlareOrbitalMenu::OnFastForwardClicked()
{
	if (MenuManager->GetPC()->GetSimulatedConfirmed())
	{
		return;
	}

	// Confirm and go on
	bool CanGoAhead = MenuManager->GetPC()->ConfirmFastForward(FSimpleDelegate::CreateSP(this, &SFlareOrbitalMenu::OnFastForwardConfirmed, false), FSimpleDelegate(), false);
	if (CanGoAhead)
	{
		OnFastForwardConfirmed(false);
	}
}

void SFlareOrbitalMenu::OnFastForwardAutomaticClicked()
{
	if (FastForwardAuto->IsActive())
	{
		// Avoid too fast double fast forward
		if (!FastForwardActive && TimeSinceFastForward < FastForwardPeriod)
		{
			FastForwardAuto->SetActive(false);
			return;
		}

		// Confirm and go on
		bool CanGoAhead = MenuManager->GetPC()->ConfirmFastForward(FSimpleDelegate::CreateSP(this, &SFlareOrbitalMenu::OnFastForwardConfirmed, true), FSimpleDelegate::CreateSP(this, &SFlareOrbitalMenu::OnFastForwardCanceled), true);
		if (CanGoAhead)
		{
			OnFastForwardConfirmed(true);
		}
	}
	else
	{
		StopFastForward();
	}
}

void SFlareOrbitalMenu::OnFastForwardConfirmed(bool Automatic)
{
	if (MenuManager->GetCurrentMenu() != EFlareMenu::MENU_Orbit)
	{
		return;
	}

	FLOGV("Start fast forward, automatic = %d", Automatic);

	if (Automatic)
	{
		// Mark FF
		FastForwardActive = true;
		FastForwardStopRequested = false;
		RequestOrbitalFleetsUpdate();

		// Prepare for FF
		Game->SaveGame(MenuManager->GetPC(), true);
		Game->DeactivateSector();
	}
	else
	{
		MenuManager->GetPC()->SimulateConfirmed();
		RefreshTrackedButtons();
		AutomatedFleetsInfo->Update();
		if (OrbitalFleetsUpdateRequested)
		{
			OrbitalFleetsUpdateRequested = false;
			OrbitalFleetsInfo->Update();
		}
	}
}

void SFlareOrbitalMenu::OnFastForwardCanceled()
{
	FastForwardAuto->SetActive(false);
}

EVisibility SFlareOrbitalMenu::IsEventsVisible() const
{
	return ShowEventsButton->IsActive() ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlareOrbitalMenu::IsTradeVisible() const
{
	return ShowTradeButton->IsActive() ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlareOrbitalMenu::IsAutomatedFleetsVisible() const
{
	return ShowFleetButton->IsActive() ? EVisibility::Visible : EVisibility::Collapsed;
}

#undef LOCTEXT_NAMESPACE