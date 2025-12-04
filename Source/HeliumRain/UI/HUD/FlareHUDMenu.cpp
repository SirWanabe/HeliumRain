
#include "FlareHUDMenu.h"
#include "../../Flare.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareGameTools.h"
#include "../../Player/FlareHUD.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareMenuManagerMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareHUDMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	Overheating = false;
	PowerOutage = false;
	PresentationFlashTime = 0.2f;
	TimeSinceOverheatChanged = PresentationFlashTime;
	TimeSinceOutageChanged = PresentationFlashTime;
	AFlarePlayerController* PC = MenuManager->GetPC();

	// Style
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.NeutralColor;
	FLinearColor EnemyColor = Theme.EnemyColor;
	NormalColor.A = Theme.DefaultAlpha;
	EnemyColor.A = Theme.DefaultAlpha;

	// Structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0))
	[
		SNew(SVerticalBox)

		// Info text
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			.AutoWidth()
			.Padding(Theme.ContentPadding)
			[
				SNew(SVerticalBox)
				// Info text
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				[
					SNew(SBox)
					.Padding(FMargin(150,0,0,0))
					.WidthOverride(1800)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Top)
					.Visibility(this, &SFlareHUDMenu::GetTopPanelVisibility)
					[
						SNew(SVerticalBox)

						// Info text 1
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Top)
						[
							SAssignNew(InfoText, STextBlock)
							.TextStyle(&Theme.NameFont)
							.Text(this, &SFlareHUDMenu::GetInfoText)
							.ColorAndOpacity(NormalColor)
						]

						// Info text 2
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Top)
						[
							SAssignNew(LowerInfoText, STextBlock)
							.TextStyle(&Theme.NameFont)
							.Text(this, &SFlareHUDMenu::GetWarningText)
							.ColorAndOpacity(EnemyColor)
						]
					]
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.Padding(Theme.SmallContentPadding)
			[
				SNew(SBox)
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				.Visibility(this, &SFlareHUDMenu::GetDockingVisibility)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Top)
					[
						SNew(SHorizontalBox)

						// Title
						+ SHorizontalBox::Slot()
						[
							SNew(SBox)
							.WidthOverride(25)
							.HAlign(HAlign_Right)
							.VAlign(VAlign_Top)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(LOCTEXT("Distance", "Distance"))
								.ColorAndOpacity(this, &SFlareHUDMenu::GetDockingDistanceColor)
							]
						]
							
						// Distance Icon
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SImage)
							.Image(FFlareStyleSet::GetIcon("MoveRight"))
							.ColorAndOpacity(this, &SFlareHUDMenu::GetDockingDistanceColorNoAlpha)
						]
						// Bar
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SBox)
							.MinDesiredWidth(150)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Center)
							[
								SNew(SProgressBar)
								.Style(&Theme.ProgressBarStyle)
								.Percent(this, &SFlareHUDMenu::GetDockingDistanceProgress)
								.FillColorAndOpacity(this, &SFlareHUDMenu::GetDockingDistanceColorNoAlpha)
							]
						]
						// Text
						+ SHorizontalBox::Slot()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SBox)
							.MinDesiredWidth(150)
							.Padding(Theme.SmallContentPadding)
							.VAlign(VAlign_Top)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(this, &SFlareHUDMenu::GetDockingDistanceText)
								.ColorAndOpacity(this, &SFlareHUDMenu::GetDockingDistanceColor)
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Top)
					[
						SNew(SHorizontalBox)

						// Title
						+ SHorizontalBox::Slot()
						[
							SNew(SBox)
							.WidthOverride(25)
							.HAlign(HAlign_Right)
							.VAlign(VAlign_Top)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(LOCTEXT("Lateral", "Lateral"))
								.ColorAndOpacity(this, &SFlareHUDMenu::GetDockingLinearColor)
							]
						]
							
						// Lateral Error Icon
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SImage)
							.Image(FFlareStyleSet::GetIcon("RCS"))
							.ColorAndOpacity(this, &SFlareHUDMenu::GetDockingLinearColorNoAlpha)
						]
						// Bar
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SBox)
							.MinDesiredWidth(150)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Center)
							[
								SNew(SProgressBar)
								.Style(&Theme.ProgressBarStyle)
								.Percent(this, &SFlareHUDMenu::GetDockingLinearProgress)
								.FillColorAndOpacity(this, &SFlareHUDMenu::GetDockingLinearColorNoAlpha)
							]
						]
						// Text
						+ SHorizontalBox::Slot()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SBox)
							.MinDesiredWidth(150)
							.Padding(Theme.SmallContentPadding)
							.VAlign(VAlign_Top)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(this, &SFlareHUDMenu::GetDockingLinearText)
								.ColorAndOpacity(this, &SFlareHUDMenu::GetDockingLinearColor)
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Top)
					[
						SNew(SHorizontalBox)

						// Title
						+ SHorizontalBox::Slot()
						[
							SNew(SBox)
							.WidthOverride(25)
							.HAlign(HAlign_Right)
							.VAlign(VAlign_Top)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(LOCTEXT("Roll", "Roll"))
								.ColorAndOpacity(this, &SFlareHUDMenu::GetDockingRollColor)
							]
						]

						// Roll Error Icon
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SImage)
							.Image(FFlareStyleSet::GetIcon("RCS"))
							.ColorAndOpacity(this, &SFlareHUDMenu::GetDockingRollColorNoAlpha)
						]
						// Bar
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SBox)
							.MinDesiredWidth(150)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Center)
							[
								SNew(SProgressBar)
								.Style(&Theme.ProgressBarStyle)
								.Percent(this, &SFlareHUDMenu::GetDockingRollProgress)
								.FillColorAndOpacity(this, &SFlareHUDMenu::GetDockingRollColorNoAlpha)
							]
						]
						// Text
						+ SHorizontalBox::Slot()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SBox)
							.MinDesiredWidth(150)
							.Padding(Theme.SmallContentPadding)
							.VAlign(VAlign_Top)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(this, &SFlareHUDMenu::GetDockingRollText)
								.ColorAndOpacity(this, &SFlareHUDMenu::GetDockingRollColor)
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Top)
					[
						SNew(SHorizontalBox)

						// Title
						+ SHorizontalBox::Slot()
						[
							SNew(SBox)
							.WidthOverride(25)
							.HAlign(HAlign_Right)
							.VAlign(VAlign_Top)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(LOCTEXT("Angular", "Angular"))
								.ColorAndOpacity(this, &SFlareHUDMenu::GetDockingAngularColor)
							]
						]	

						// Angular Error Icon
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SImage)
							.Image(FFlareStyleSet::GetIcon("RCS"))
							.ColorAndOpacity(this, &SFlareHUDMenu::GetDockingAngularColorNoAlpha)
						]
						// Bar
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SBox)
							.MinDesiredWidth(150)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Center)
							[
								SNew(SProgressBar)
								.Style(&Theme.ProgressBarStyle)
								.Percent(this, &SFlareHUDMenu::GetDockingAngularProgress)
								.FillColorAndOpacity(this, &SFlareHUDMenu::GetDockingAngularColorNoAlpha)
							]
						]
						// Text
						+ SHorizontalBox::Slot()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SBox)
							.MinDesiredWidth(150)
							.Padding(Theme.SmallContentPadding)
							.VAlign(VAlign_Top)
							[
								SNew(STextBlock)
								.TextStyle(&Theme.NameFont)
								.Text(this, &SFlareHUDMenu::GetDockingAngularText)
								.ColorAndOpacity(this, &SFlareHUDMenu::GetDockingAngularColor)
							]
						]
					]
				]
			]
		]

		// Overheating box
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.SubTitleFont)
			.Text(LOCTEXT("Overheating", "OVERHEATING !"))
			.ColorAndOpacity(this, &SFlareHUDMenu::GetOverheatColor, true)
		]

		// Outage box
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.SubTitleFont)
				.Text(LOCTEXT("PowerOutage", "POWER OUTAGE !"))
				.ColorAndOpacity(this, &SFlareHUDMenu::GetOutageColor, true)
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(this, &SFlareHUDMenu::GetOutageText)
				.TextStyle(&Theme.SubTitleFont)
				.ColorAndOpacity(this, &SFlareHUDMenu::GetOutageColor, true)
			]
		]

		// Weapon panel
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(0, 0, 0, 0))
		[
			SAssignNew(WeaponContainer, SVerticalBox)
		]

		// Status panel
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		[
			SNew(SHorizontalBox)
			
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(PowerStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_Power)
				.MenuManager(MenuManager)
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(LifeSupportStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_LifeSupport)
				.MenuManager(MenuManager)
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(TemperatureStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_Temperature)
				.MenuManager(MenuManager)
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(PropulsionStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_Propulsion)
				.MenuManager(MenuManager)
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(RCSStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_RCS)
				.MenuManager(MenuManager)
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(WeaponStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_Weapon)
				.MenuManager(MenuManager)
			]
		]

		// Overheating progress bar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(Theme.SmallContentPadding)
			[
				SNew(SHorizontalBox)
				// Icon
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(SImage)
					.Image(FFlareStyleSet::GetIcon("Temperature"))
					.ColorAndOpacity(this, &SFlareHUDMenu::GetTemperatureColorNoAlpha)
				]

				// Bar
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.MinDesiredWidth(500)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Center)
					[
						SNew(SProgressBar)
						.Style(&Theme.ProgressBarStyle)
						.Percent(this, &SFlareHUDMenu::GetTemperatureProgress)
						.FillColorAndOpacity(this, &SFlareHUDMenu::GetTemperatureColorNoAlpha)
					]
				]

				// Text
				+ SHorizontalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(SBox)
					.MinDesiredWidth(100)
					.Padding(Theme.SmallContentPadding)
					.VAlign(VAlign_Top)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.NameFont)
						.Text(this, &SFlareHUDMenu::GetTemperatureText)
						.ColorAndOpacity(this, &SFlareHUDMenu::GetTemperatureColor)
					]
				]
			]
		]
	];

	SetVisibility(EVisibility::Collapsed);
}

/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareHUDMenu::OnPlayerShipChanged()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Is this a civilian ship ?
	UFlareSimulatedSpacecraft* PlayerShip = MenuManager->GetPC()->GetPlayerShip();
	WeaponStatus->SetVisibility(PlayerShip->IsMilitaryArmed() ? EVisibility::Visible : EVisibility::Hidden);
	WeaponContainer->ClearChildren();

	// Update weapon list
	if (PlayerShip && PlayerShip->IsMilitaryArmed())
	{
		TArray<FFlareWeaponGroup*>& WeaponGroupList = PlayerShip->GetActive()->GetWeaponsSystem()->GetWeaponGroupList();

		// Add weapon indicators
		for (int32 i = WeaponGroupList.Num() - 1; i >= 0; i--)
		{
			WeaponContainer->AddSlot()
			.AutoHeight()
			[
				SNew(SFlareWeaponStatus)
				.MenuManager(MenuManager)
				.TargetWeaponGroupIndex(i)
			];
		}

		// No weapon
		WeaponContainer->AddSlot()
		.AutoHeight()
		[
			SNew(SFlareWeaponStatus)
			.MenuManager(MenuManager)
			.TargetWeaponGroupIndex(-1)
		];

		if (PlayerShip->GetDescription()->IsDroneCarrier)
		{
			// Drones
			WeaponContainer->AddSlot()
			.AutoHeight()
			[
				SNew(SFlareWeaponStatus)
				.MenuManager(MenuManager)
				.TargetWeaponGroupIndex(-2)
			];
		}
	}
}


/*----------------------------------------------------
	Events
----------------------------------------------------*/

//void SFlareHUDMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
void SFlareHUDMenu::HudMenuTick(float InDeltaTime)
{
//	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	UFlareSimulatedSpacecraft* PlayerShip = MenuManager->GetPC()->GetPlayerShip();
	if (PlayerShip)
	{
		// Is alive ?
		// SetVisibility(PlayerShip->GetDamageSystem()->IsAlive() ? EVisibility::Visible : EVisibility::Collapsed);

		// Overheating status
		TimeSinceOverheatChanged += InDeltaTime;
		Temperature = PlayerShip->GetDamageSystem()->GetTemperature();
		OverheatTemperature = PlayerShip->GetDamageSystem()->GetOverheatTemperature();

		// Alert the player if the ship is near the overheat temperature
		bool NewOverheating = (PlayerShip->GetDamageSystem()->GetTemperature() > PlayerShip->GetDamageSystem()->GetOverheatTemperature() * 0.95);
		if (NewOverheating != Overheating)
		{
			TimeSinceOverheatChanged = 0;
		}

		Overheating = NewOverheating;

		// Outage status
		TimeSinceOutageChanged += InDeltaTime;
		bool NewPowerOutage = PlayerShip->GetDamageSystem()->HasPowerOutage();
		if (NewPowerOutage != PowerOutage)
		{
			TimeSinceOutageChanged = 0;
		}

		PowerOutage = NewPowerOutage;

		AFlareSpacecraft* PlayerActiveShip = PlayerShip->GetActive();
		if (PlayerActiveShip)
		{
			// Docking info
			FText DockInfo;
			AFlareSpacecraft* DockSpacecraft = NULL;
			FFlareDockingParameters DockParameters;
			DockingInProgress = PlayerActiveShip->GetManualDockingProgress(DockSpacecraft, DockParameters, DockInfo);
			if (DockSpacecraft)
			{
				// Angular / linear errors
				float AngularDot = FVector::DotProduct(DockParameters.ShipDockAxis.GetSafeNormal(), -DockParameters.StationDockAxis.GetSafeNormal());
				float AngularError = FMath::RadiansToDegrees(FMath::Acos(AngularDot));
				float LinearError = FVector::VectorPlaneProject(DockParameters.DockToDockDeltaLocation, DockParameters.StationDockAxis).Size() / 100;

				// Roll error
				FVector StationTopAxis = DockParameters.StationDockTopAxis;
				FVector TopAxis = PlayerActiveShip->GetActorRotation().RotateVector(FVector(0, 0, 1));
				FVector LocalTopAxis = FVector::VectorPlaneProject(TopAxis, DockParameters.StationDockAxis).GetSafeNormal();
				float RollDot = FVector::DotProduct(StationTopAxis, LocalTopAxis);
				float RollError = FMath::RadiansToDegrees(FMath::Acos(RollDot));

				// Ratios
				float AngularTarget = 2; // 2° max error
				float LinearTarget = 1; // 2m max error
				float DistanceTarget = (PlayerActiveShip->GetSize() == EFlarePartSize::S) ? 250 : 500;
				DockingDistanceRatio = 1 - FMath::Clamp(DockParameters.DockToDockDistance / 10000, 0.0f, 1.0f); // 100m distance range
				DockingRollRatio = 1 - FMath::Clamp(RollError / 30, 0.0f, 1.0f); // 30° range
				DockingAngularRatio = 1 - FMath::Clamp(AngularError / 30, 0.0f, 1.0f); // 30° range
				DockingLinearRatio = 1 - FMath::Clamp(LinearError / 10, 0.0f, 1.0f); // 10m range
				
				DockingDistanceText = FText::Format(LOCTEXT("DockingDistanceHUDFormat", "{0}m"),
					FText::AsNumber(FMath::RoundToInt(DockParameters.DockToDockDistance / 100)));
				DockingRollText = FText::Format(LOCTEXT("DockingRollHUDFormat", "{0}\u00B0"),
					FText::AsNumber(FMath::RoundToInt(RollError)));
				DockingAngularText = FText::Format(LOCTEXT("DockingAlignmentHUDFormat", "{0}\u00B0"),
					FText::AsNumber(FMath::RoundToInt(AngularError)));
				DockingLinearText = FText::Format(LOCTEXT("DockingOffsetHUDFormat", "{0}m"),
					FText::AsNumber(FMath::RoundToInt(LinearError)));
			}
			else
			{
				SetDockingInfoToDefaults();
			}
		}
		else
		{
			SetDockingInfoToDefaults();
		}
	}
}

/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareHUDMenu::SetDockingInfoToDefaults()
{
	DockingInProgress = false;

	DockingDistanceRatio = 0;
	DockingDistanceText = FText();

	DockingRollRatio = 0;
	DockingRollText = FText();

	DockingLinearRatio = 0;
	DockingLinearText = FText();

	DockingAngularRatio = 0;
	DockingAngularText = FText();
}

EVisibility SFlareHUDMenu::GetTopPanelVisibility() const
{
	return MenuManager->IsOverlayOpen() ? EVisibility::Hidden : EVisibility::Visible;
}

EVisibility SFlareHUDMenu::GetDockingVisibility() const
{
	if (MenuManager->IsOverlayOpen())
	{
		return EVisibility::Hidden;
	}

	bool IsDocked = false;

	if (MenuManager->GetPC()->GetPlayerShip() && MenuManager->GetPC()->GetPlayerShip()->GetActive())
	{
		IsDocked = MenuManager->GetPC()->GetPlayerShip()->GetActive()->GetNavigationSystem()->IsDocked();
	}

	return DockingInProgress && !IsDocked ? EVisibility::Visible : EVisibility::Hidden;
}


FText SFlareHUDMenu::GetInfoText() const
{
	UFlareSimulatedSpacecraft* PlayerShip = MenuManager->GetPC()->GetPlayerShip();
	if (PlayerShip && PlayerShip->IsActive() && (!MenuManager->GetPC()->UseCockpit || (PlayerShip->GetActive() && PlayerShip->GetActive()->GetStateManager()->IsExternalCamera() && !PlayerShip->GetActive()->GetStateManager()->IsExternalCameraPanning())) && MenuManager->GetPC()->GetGame()->GetActiveSector())
	{
		AFlareSpacecraft* ActivePlayerShip = PlayerShip->GetActive();

		// Get mode info
		FText ModeText;
		FText AutopilotText;
		if (ActivePlayerShip->GetNavigationSystem()->IsDocked())
		{
			ModeText = LOCTEXT("Docked", "Docked");
		}
		else
		{
			ModeText = ActivePlayerShip->GetWeaponsSystem()->GetWeaponModeInfo();
			if (ActivePlayerShip->GetNavigationSystem()->IsAutoPilot())
			{
				AutopilotText = UFlareGameTools::AddLeadingSpace(LOCTEXT("AUTOPILOT", "(Autopilot)"));
			}
		}
		
		// Sector info
		UFlareSimulatedSector* CurrentSector = PlayerShip->GetGame()->GetActiveSector()->GetSimulatedSector();
		FText SectorText = FText::Format(LOCTEXT("CurrentSectorFormat", "{0} ({1})"),
			CurrentSector->GetSectorName(),
			CurrentSector->GetSectorFriendlynessText(PlayerShip->GetCompany()));

		FText Pretext;
		if (ActivePlayerShip->GetStateManager()->GetPlayerManualLockDirection())
		{
			Pretext = LOCTEXT("DirectionLocked", "(DL) ");
		}

		// Full flight info
		FText FlightInfo = FText::Format(LOCTEXT("ShipInfoTextFormat", "{0}{1} m/s - {2} {3} in {4}"),
			Pretext,
			FText::AsNumber(FMath::RoundToInt(ActivePlayerShip->GetLinearVelocity().Size())),
			ModeText,
			AutopilotText,
			SectorText);

		// Battle status
		FText BattleText = CurrentSector->GetSectorBattleStateText(MenuManager->GetPC()->GetCompany());

		// Target info
		FText TargetText;
		AFlareSpacecraft* TargetShip = PlayerShip->GetActive()->GetCurrentTarget().SpacecraftTarget;
		if (TargetShip && TargetShip->IsValidLowLevel())
		{
			TargetText = FText::Format(LOCTEXT("CurrentTargetFormat", "Targeting {0}"),
				UFlareGameTools::DisplaySpacecraftName(TargetShip->GetParent()));
		}
		else
		{
			TargetText = LOCTEXT("CurrentNoTarget", "No target");
		}

		// Build result
		FString Result = FlightInfo.ToString() + " - " + TargetText.ToString() + "\n" + CurrentSector->GetSectorBalanceText().ToString();
		if (BattleText.ToString().Len())
		{
			Result += " - " + BattleText.ToString();
		}

		// Add performance
		FText PerformanceText = MenuManager->GetPC()->GetNavHUD()->GetPerformanceText();
		if (PerformanceText.ToString().Len())
		{
			Result += "\n" + PerformanceText.ToString();
		}

		return FText::FromString(Result);
	}

	return FText();
}

FSlateColor SFlareHUDMenu::GetDockingDistanceColor() const
{
	return FFlareStyleSet::GetRatioColour(FMath::Clamp(DockingDistanceRatio, 0.0f, 1.0f), true);
}

FSlateColor SFlareHUDMenu::GetDockingDistanceColorNoAlpha() const
{
	return FFlareStyleSet::GetRatioColour(FMath::Clamp(DockingDistanceRatio, 0.0f, 1.0f), false);
}

TOptional<float> SFlareHUDMenu::GetDockingDistanceProgress() const
{
	return (FMath::Clamp(DockingDistanceRatio, 0.0f, 1.0f) / 1);
}

FText SFlareHUDMenu::GetDockingDistanceText() const
{
	return DockingDistanceText;
}

FSlateColor SFlareHUDMenu::GetDockingLinearColor() const
{
	return FFlareStyleSet::GetRatioColour(FMath::Clamp(DockingLinearRatio, 0.0f, 1.0f), true);
}

FSlateColor SFlareHUDMenu::GetDockingLinearColorNoAlpha() const
{
	return FFlareStyleSet::GetRatioColour(FMath::Clamp(DockingLinearRatio, 0.0f, 1.0f), false);
}

TOptional<float> SFlareHUDMenu::GetDockingLinearProgress() const
{
	return (FMath::Clamp(DockingLinearRatio, 0.0f, 1.0f) / 1);
}

FText SFlareHUDMenu::GetDockingLinearText() const
{
	return DockingLinearText;
}

FSlateColor SFlareHUDMenu::GetDockingRollColor() const
{
	return FFlareStyleSet::GetRatioColour(FMath::Clamp(DockingRollRatio, 0.0f, 1.0f), true);
}

FSlateColor SFlareHUDMenu::GetDockingRollColorNoAlpha() const
{
	return FFlareStyleSet::GetRatioColour(FMath::Clamp(DockingRollRatio, 0.0f, 1.0f), false);
}

TOptional<float> SFlareHUDMenu::GetDockingRollProgress() const
{
	return (FMath::Clamp(DockingRollRatio, 0.0f, 1.0f) / 1);
}

FText SFlareHUDMenu::GetDockingRollText() const
{
	return DockingRollText;
}

FSlateColor SFlareHUDMenu::GetDockingAngularColor() const
{
	return FFlareStyleSet::GetRatioColour(FMath::Clamp(DockingAngularRatio, 0.0f, 1.0f), true);
}

FSlateColor SFlareHUDMenu::GetDockingAngularColorNoAlpha() const
{
	return FFlareStyleSet::GetRatioColour(FMath::Clamp(DockingAngularRatio, 0.0f, 1.0f), false);
}

TOptional<float> SFlareHUDMenu::GetDockingAngularProgress() const
{
	return (FMath::Clamp(DockingAngularRatio, 0.0f, 1.0f) / 1);
}

FText SFlareHUDMenu::GetDockingAngularText() const
{
	return DockingAngularText;
}

FText SFlareHUDMenu::GetWarningText() const
{
	FString Info;

	UFlareSimulatedSpacecraft* PlayerShip = MenuManager->GetPC()->GetPlayerShip();
	UFlareSimulatedSector* CurrentSector = PlayerShip->GetGame()->GetActiveSector()->GetSimulatedSector();

	if (CurrentSector)
	{
		// Get player threats
		bool Targeted, FiredUpon, CollidingSoon, ExitingSoon, LowHealth;
		UFlareSimulatedSpacecraft* Threat;
		MenuManager->GetPC()->GetPlayerShipThreatStatus(Targeted, FiredUpon, CollidingSoon, ExitingSoon, LowHealth, Threat);

		// Fired on ?
		if (FiredUpon)
		{
			if (Threat)
			{
				FText WarningText = FText::Format(LOCTEXT("ThreatFiredUponFormat", "UNDER FIRE FROM {0} ({1})"),
					UFlareGameTools::DisplaySpacecraftName(Threat),
					FText::FromString(Threat->GetCompany()->GetShortName().ToString()));
				Info = WarningText.ToString();
			}
			else
			{
				FText WarningText = FText(LOCTEXT("ThreatFiredUponMissile", "INCOMING MISSILE"));
				Info = WarningText.ToString();
			}
		}

		// Collision ?
		else if (CollidingSoon)
		{
			FText WarningText = LOCTEXT("ThreatCollisionFormat", "IMMINENT COLLISION");
			Info = WarningText.ToString();
		}

		// Leaving sector ?
		else if (ExitingSoon)
		{
			FText WarningText = LOCTEXT("ThreatLeavingFormat", "LEAVING SECTOR");
			Info = WarningText.ToString();
		}

		// Targeted ?
		else if (Targeted)
		{
			FText WarningText = FText::Format(LOCTEXT("ThreatTargetFormat", "TARGETED BY {0} ({1})"),
				UFlareGameTools::DisplaySpacecraftName(Threat),
				FText::FromString(Threat->GetCompany()->GetShortName().ToString()));
			Info = WarningText.ToString();
		}

		// Low health
		else if (LowHealth)
		{
			FText WarningText = LOCTEXT("ThreatHealthFormat", "LIFE SUPPORT COMPROMISED");
			Info = WarningText.ToString();
		}
	}

	return FText::FromString(Info);
}

TOptional<float> SFlareHUDMenu::GetTemperatureProgress() const
{
	float WidgetMin = 500.0f;
	float WidgetRange = OverheatTemperature - WidgetMin;
	return ((Temperature - WidgetMin) / WidgetRange);
}

FSlateColor SFlareHUDMenu::GetTemperatureColor() const
{
	float Distance = Temperature - 0.9f * OverheatTemperature;
	float Ratio = FMath::Clamp(FMath::Abs(Distance) / 10.0f, 0.0f, 1.0f);

	if (Distance < 0)
	{
		Ratio = 0.0f;
	}

	return FFlareStyleSet::GetHealthColor(1 - Ratio, true);
}

FSlateColor SFlareHUDMenu::GetTemperatureColorNoAlpha() const
{
	float Distance = Temperature - 0.9f * OverheatTemperature;
	float Ratio = FMath::Clamp(FMath::Abs(Distance) / 10.0f, 0.0f, 1.0f);

	if (Distance < 0)
	{
		Ratio = 0.0f;
	}

	return FFlareStyleSet::GetHealthColor(1 - Ratio, false);
}

FText SFlareHUDMenu::GetTemperatureText() const
{
	return FText::Format(LOCTEXT("TemperatureFormat", "{0} K"), FText::AsNumber((int32)Temperature));
}

FSlateColor SFlareHUDMenu::GetOverheatColor(bool Text) const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	FLinearColor Color = Theme.EnemyColor;
	float Ratio = FMath::Clamp(TimeSinceOverheatChanged / PresentationFlashTime, 0.0f, 1.0f);
	Color.A *= (Overheating ? Ratio : (1 - Ratio));

	if (Text)
	{
		Color.A *= Theme.DefaultAlpha;
	}

	return Color;
}

FSlateColor SFlareHUDMenu::GetOutageColor(bool Text) const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	FLinearColor Color = Theme.NeutralColor;
	float Ratio = FMath::Clamp(TimeSinceOutageChanged / PresentationFlashTime, 0.0f, 1.0f);
	Color.A *= (PowerOutage ? Ratio : (1 - Ratio));

	if (Text)
	{
		Color.A *= Theme.DefaultAlpha;
	}

	return Color;
}

FText SFlareHUDMenu::GetOutageText() const
{
	FText Result;

	UFlareSimulatedSpacecraft* PlayerShip = MenuManager->GetPC()->GetPlayerShip();

	if (PlayerShip)
	{
		return FText::Format(LOCTEXT("PwBackInFormat", "Power back in {0}..."),
			FText::AsNumber((int32)(PlayerShip->GetDamageSystem()->GetPowerOutageDuration()) + 1));
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE