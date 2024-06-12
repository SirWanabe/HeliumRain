
#include "FlareSectorButton.h"
#include "../../Flare.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlareGameTools.h"
#include "../../Game/FlareWorld.h"
#include "../../Game/FlareTravel.h"
#include "../../Game/FlareCompany.h"
#include "../../Game/FlareSimulatedSector.h"

#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#include "../Menus/FlareOrbitalMenu.h"


#define LOCTEXT_NAMESPACE "FlareSectorButton"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSectorButton::Construct(const FArguments& InArgs)
{
	// Setup
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor Color = FLinearColor::White;
	Color.A = 0.85f;

	// Arguments
	OnClicked = InArgs._OnClicked;
	Sector = InArgs._Sector;
	PlayerCompany = InArgs._PlayerCompany;
	OwnedShips = -1;
	OwnedStations = -1;
	NeutralShips = -1;
	NeutralStations = -1;
	EnemyShips = -1;
	EnemyStations = -1;

	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Center)
	[
		SNew(SBorder)
		.BorderImage(FFlareStyleSet::GetIcon("LargeButtonBackground"))
		.BorderBackgroundColor(Color)
		.Padding(FMargin(25))
		[
			SNew(SVerticalBox)
			// Upper line
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.SectorButtonTextPadding)
			[
				SAssignNew(SectorTitle, STextBlock)
				.WrapTextAt(3 * Theme.SectorButtonWidth)
				.TextStyle(&Theme.TextFont)
				.Justification(ETextJustify::Center)
			]

			// Main
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				// Image box
				SNew(SBox)
				.WidthOverride(Theme.SectorButtonWidth)
				.HeightOverride(Theme.SectorButtonHeight)
				[
					// Button (behavior only, no display)
					SNew(SButton)
					.OnClicked(this, &SFlareSectorButton::OnButtonClicked)
					.ContentPadding(FMargin(0))
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					[
						// Background
						SAssignNew(BackgroundBorder,SBorder)
						.Padding(Theme.SectorButtonPadding)
						.BorderImage(&Theme.SectorButtonBorder)
						[
							// Icon
							SAssignNew(ButtonImage, SImage)
						]
					]
				]
			]

			// Lower line
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.SectorButtonTextPadding)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SAssignNew(FleetBoxOne, SHorizontalBox)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SAssignNew(FleetBoxTwo, SHorizontalBox)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SAssignNew(FleetBoxThree, SHorizontalBox)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(SectorText, STextBlock)
					.WrapTextAt(3 * Theme.SectorButtonWidth)
					.TextStyle(&Theme.SmallFont)
					.Justification(ETextJustify::Center)
				]
			]
		]
	];

	RefreshButton();
}

TSharedPtr<SHorizontalBox> SFlareSectorButton::GetCurrentBox()
{
//Primarily full row 1 and 2, in extreme case when there's lots of fleets 
//place all into third row for a little bit, and from then do a 1-2-3 instead of 1-2
	if (TotalCount >= 30 && TotalCount < 40)
	{
		CurrentBox = FleetBoxThree;

	}
	else if (CurrentCount >= 7)
	{
		CurrentCount = 0;

		if (CurrentBox == FleetBoxOne)
		{
			CurrentBox = FleetBoxTwo;
		}
		else if (CurrentBox == FleetBoxTwo)
		{
			if(TotalCount <= 40)
			{ 
				CurrentBox = FleetBoxOne;
			}
			else
			{
				CurrentBox = FleetBoxThree;
			}
		}
		else if (CurrentBox == FleetBoxThree)
		{
			CurrentBox = FleetBoxOne;
		}
	}

	return CurrentBox;
}

void SFlareSectorButton::UpdateBackgroundImages()
{
	BackgroundBorder->SetBorderBackgroundColor(GetBorderColor());
	ButtonImage->SetImage(GetBackgroundBrush());
	ButtonImage->SetColorAndOpacity(GetMainColor());
}

void SFlareSectorButton::RefreshButton()
{
	if (Sector == nullptr || !IsValid(Sector))
	{
		SectorTitle->SetText(FText());
		SectorText->SetVisibility(EVisibility::Collapsed);
		SectorText->SetText(FText());
		return;
	}

	SectorTitle->SetShadowColorAndOpacity(GetShadowColor());
	SectorTitle->SetText(Sector->GetSectorName());

	SectorText->SetShadowColorAndOpacity(GetShadowColor());
	SectorText->SetText(GetSectorText());
	SectorText->SetVisibility(GetBottomTextVisibility());

	UpdateBackgroundImages();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	EFlareOrbitalMode::Type DisplayMode = MenuManager->GetOrbitMenu()->GetDisplayMode();
	if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_TradeRoute)
	{
		DisplayMode = EFlareOrbitalMode::Stations;
	}

	if (DisplayMode != EFlareOrbitalMode::Fleets)
	{
		FleetBoxOne->SetVisibility(EVisibility::Collapsed);
		FleetBoxTwo->SetVisibility(EVisibility::Collapsed);
		FleetBoxThree->SetVisibility(EVisibility::Collapsed);
	}

	if (DisplayMode == EFlareOrbitalMode::Fleets)
	{
		FleetBoxOne->SetVisibility(EVisibility::Visible);
		FleetBoxTwo->SetVisibility(EVisibility::Visible);
		FleetBoxThree->SetVisibility(EVisibility::Visible);

		CurrentBox = FleetBoxOne;
		CurrentCount = 0;
		TotalCount = 0;

		for (int32 CountIndex = 0; CountIndex < TrackedFleets.Num(); CountIndex++)
		{
			UFlareFleet* Fleet = TrackedFleets[CountIndex];
			bool RemoveFleet = false;

			int32 TravelingFleetPosition = TrackedTravellingFleets.Find(Fleet);
			if (!Fleet)
			{
				RemoveFleet = true;
			}
			else if (TravelingFleetPosition != INDEX_NONE)
			{
				if (Fleet->GetCurrentSector() == Sector)
				{
					RemoveFleet = true;
					TrackedTravellingFleets.RemoveAt(TravelingFleetPosition);
					TrackedFleetDateBoxes.RemoveAt(TravelingFleetPosition);
				}
				else if (Fleet->GetCurrentTravel())
				{
					if (Fleet->GetCurrentTravel()->GetDestinationSector() != Sector)
					{
						RemoveFleet = true;
						TrackedTravellingFleets.RemoveAt(TravelingFleetPosition);
						TrackedFleetDateBoxes.RemoveAt(TravelingFleetPosition);
					}
					else
					{
						TSharedPtr<STextBlock> DateBox = TrackedFleetDateBoxes[TravelingFleetPosition];
						if (DateBox)
						{
							FText DateText = UFlareGameTools::FormatDate(Fleet->GetCurrentTravel()->GetRemainingTravelDuration(), 1);
							DateBox->SetText(DateText);
						}
					}
				}
			}
			else if (Fleet->GetCurrentSector() != Sector)
			{
				RemoveFleet = true;
			}

			if (RemoveFleet)
			{
				FleetBoxOne->RemoveSlot(TrackedFleetBoxes[CountIndex].ToSharedRef());
				FleetBoxTwo->RemoveSlot(TrackedFleetBoxes[CountIndex].ToSharedRef());
				FleetBoxThree->RemoveSlot(TrackedFleetBoxes[CountIndex].ToSharedRef());
				TrackedFleets.RemoveAt(CountIndex);
				TrackedFleetBoxes.RemoveAt(CountIndex);
				CountIndex--;
			}
		}

		const FSlateBrush* FleetSmall = FFlareStyleSet::GetIcon("Fleet_Small");
		const FSlateBrush* FleetSmallMilitary = FFlareStyleSet::GetIcon("Fleet_Small_Military");

		// Get local fleets
		for (UFlareFleet* Fleet : Sector->GetSectorFleets())
		{
			if (Fleet->GetFleetCompany()->IsPlayerCompany())
			{
				CurrentCount++;
				TotalCount++;
				CurrentBox = GetCurrentBox();

				if (TrackedFleets.Find(Fleet) != INDEX_NONE)
				{
					continue;
				}

				TSharedPtr<SVerticalBox>		FleetBox;
				FLinearColor FleetColor = Fleet->GetFleetColor();
				const FSlateBrush* FleetIcon = (Fleet->GetCombatPoints(false) > 0) ?
				FleetSmallMilitary : FleetSmall;
				CurrentBox->AddSlot()
				.AutoWidth()
				[
					SAssignNew(FleetBox,SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SImage)
						.Image(FleetIcon)
						.ColorAndOpacity(FleetColor)
					]
				];

				TrackedFleets.AddUnique(Fleet);
				TrackedFleetBoxes.AddUnique(FleetBox);
			}
		}

		// Get incoming fleets
		for (UFlareTravel* Travel : MenuManager->GetGame()->GetGameWorld()->GetTravels())
		{
			if (Travel->GetFleet()->GetFleetCompany()->IsPlayerCompany() && Sector == Travel->GetDestinationSector())
			{
				CurrentCount++;
				TotalCount++;
				CurrentBox = GetCurrentBox();

				if (TrackedFleets.Find(Travel->GetFleet()) != INDEX_NONE)
				{
					continue;
				}

				FLinearColor FleetColor = Travel->GetFleet()->GetFleetColor();
				FText DateText = UFlareGameTools::FormatDate(Travel->GetRemainingTravelDuration(), 1);
				FleetColor.A = 0.5f;

				TSharedPtr<SVerticalBox>		FleetBox;
				TSharedPtr<STextBlock>			DateBox;

				CurrentBox->AddSlot()
				.AutoWidth()
				[
					SAssignNew(FleetBox, SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SImage)
						.Image(FleetSmall)
						.ColorAndOpacity(FleetColor)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(DateBox,STextBlock)
						.Text(DateText)
						.TextStyle(&Theme.SmallFont)
					]
				];

				TrackedTravellingFleets.AddUnique(Travel->GetFleet());
				TrackedFleets.AddUnique(Travel->GetFleet());
				TrackedFleetBoxes.AddUnique(FleetBox);
				TrackedFleetDateBoxes.AddUnique(DateBox);
			}
		}
//		SlatePrepass(FSlateApplicationBase::Get().GetApplicationScale());
	}
	else if (DisplayMode == EFlareOrbitalMode::Ships && Sector->GetSectorShips().Num() > 0)
	{
		EnemyShips = 0;
		OwnedShips = 0;
		NeutralShips = 0;

		TArray<UFlareSimulatedSpacecraft*> TotalShips = Sector->GetSectorShips();
		int32 TotalSectorShips = TotalShips.Num();

		for (int32 CountIndex = 0; CountIndex < TotalSectorShips; CountIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = TotalShips[CountIndex];

			if (Ship->GetCompany() == PlayerCompany)
			{
				++OwnedShips;
				continue;
			}
			else if (Ship->IsHostile(PlayerCompany))
			{
				++EnemyShips;
				continue;
			}
			else
			{
				++NeutralShips;
				continue;
			}
		}
	}
	else if (DisplayMode == EFlareOrbitalMode::Stations && Sector->GetSectorStations().Num() > 0)
	{
		EnemyStations = 0;
		OwnedStations = 0;
		NeutralStations = 0;
		TArray<UFlareSimulatedSpacecraft*> TotalStations = Sector->GetSectorStations();
		int32 TotalSectorStations = TotalStations.Num();

		for (int32 CountIndex = 0; CountIndex < TotalSectorStations; CountIndex++)
		{
			UFlareSimulatedSpacecraft* Station = TotalStations[CountIndex];
			if (Station->GetCompany() == PlayerCompany)
			{
				++OwnedStations;
				continue;
			}
			else if (Station->IsHostile(PlayerCompany))
			{
				++EnemyStations;
				continue;
			}
			else
			{
				++NeutralStations;
				continue;
			}
		}
	}
}

/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareSectorButton::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseEnter(MyGeometry, MouseEvent);

	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	UpdateBackgroundImages();
	if (MenuManager && Sector && Sector->IsValidLowLevel())
	{
		FText SectorStatus = Sector->GetSectorFriendlynessText(PlayerCompany);
		FText SectorNameText = FText::Format(LOCTEXT("SectorNameFormat", "{0} ({1})"), Sector->GetSectorName(), SectorStatus);

		FText SectorInfoText = Sector->GetSectorDescription();
		// Get local fleets
		for (UFlareFleet* Fleet : Sector->GetSectorFleets())
		{
			if (Fleet->GetFleetCompany()->IsPlayerCompany())
			{
				SectorInfoText = FText::Format(LOCTEXT("SectorInfoFleetLocalFormat", "{0}\n\u2022 Your fleet {1} is here."),
					SectorInfoText,
					Fleet->GetFleetName());
			}
		}

		// Get incoming fleets
		for (UFlareTravel* Travel : MenuManager->GetGame()->GetGameWorld()->GetTravels())
		{
			if (Travel->GetFleet()->GetFleetCompany()->IsPlayerCompany() && Sector == Travel->GetDestinationSector())
			{
				FText DateText = UFlareGameTools::FormatDate(Travel->GetRemainingTravelDuration(), 1);
				SectorInfoText = FText::Format(LOCTEXT("SectorInfoFleetTravelFormat", "{0}\n\u2022 Your fleet {1} will arrive in {2}."),
					SectorInfoText,
					Travel->GetFleet()->GetFleetName(),
					DateText);
			}
		}

		MenuManager->ShowTooltip(this, SectorNameText, SectorInfoText);
	}
}

void SFlareSectorButton::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseLeave(MouseEvent);

	UpdateBackgroundImages();
	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	if (MenuManager)
	{
		MenuManager->HideTooltip(this);
	}
}

bool SFlareSectorButton::ShouldDisplayFleets() const
{
	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	EFlareOrbitalMode::Type DisplayMode = MenuManager->GetOrbitMenu()->GetDisplayMode();

	if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_TradeRoute)
	{
		DisplayMode = EFlareOrbitalMode::Stations;
	}

	if (DisplayMode == EFlareOrbitalMode::Fleets)
	{
		return true;
	}
	else
	{
		return false;
	}
}

UFlareSimulatedSector* SFlareSectorButton::GetSector()
{
	return Sector;
}

EVisibility SFlareSectorButton::GetBottomTextVisibility() const
{
	if (ShouldDisplayFleets())
	{
		return EVisibility::Collapsed;
	}
	else
	{
		return EVisibility::Visible;
	}
}

FText SFlareSectorButton::GetSectorText() const
{
	// Get display mode

	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	FText SectorText;

	EFlareOrbitalMode::Type DisplayMode = MenuManager->GetOrbitMenu()->GetDisplayMode();
	if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_TradeRoute)
	{
		DisplayMode = EFlareOrbitalMode::Stations;
	}

	// If the sector is known, display it
	if (Sector && PlayerCompany->HasVisitedSector(Sector))
	{
		if (DisplayMode == EFlareOrbitalMode::Stations && Sector->GetSectorStations().Num() > 0)
		{
			int32 Warcount = PlayerCompany->GetWarCount(PlayerCompany);
			int32 TotalSectorStations = Sector->GetSectorStations().Num();
			FString DetailedStationString;

			if (MenuManager->GetCurrentMenu() == EFlareMenu::MENU_Orbit)
			{
				if (OwnedStations > 0)
				{
					FString FormattedNumber = FString::FormatAsNumber(OwnedStations);
					DetailedStationString += FString::Printf(TEXT(", %s owned"), *FormattedNumber);
				}

				if (NeutralStations > 0 && NeutralStations != TotalSectorStations)
				{
					FString FormattedNumber = FString::FormatAsNumber(NeutralStations);
					DetailedStationString += FString::Printf(TEXT(", %s neut"), *FormattedNumber);
				}
				if (Warcount > 0)
				{
					if (EnemyStations > 0)
					{
						FString FormattedNumber = FString::FormatAsNumber(EnemyStations);
						DetailedStationString += FString::Printf(TEXT(", %s"), *FormattedNumber);
						if (EnemyStations == 1)
						{
							DetailedStationString += FString::Printf(TEXT(" foe"));
						}
						else
						{
							DetailedStationString += FString::Printf(TEXT(" foes"));
						}
					}
				}
			}

			SectorText = Sector->GetSectorStations().Num() == 1 ? LOCTEXT("Station", "{0} station") : LOCTEXT("Stations", "{0} stations{1}");
			SectorText = FText::Format(SectorText, FText::AsNumber(TotalSectorStations), FText::FromString(DetailedStationString));
			}

		else if (DisplayMode == EFlareOrbitalMode::Ships && Sector->GetSectorShips().Num() > 0)
		{
			int32 Warcount = PlayerCompany->GetWarCount(PlayerCompany);
			int32 TotalSectorShips = Sector->GetSectorShips().Num();
			FString DetailedShipString;

			if (OwnedShips > 0)
			{
				FString FormattedNumber = FString::FormatAsNumber(OwnedShips);
				DetailedShipString += FString::Printf(TEXT(", %s owned"), *FormattedNumber);
			}

			if (NeutralShips > 0 && NeutralShips != TotalSectorShips)
			{
				FString FormattedNumber = FString::FormatAsNumber(NeutralShips);
				DetailedShipString += FString::Printf(TEXT(", %s neut"), *FormattedNumber);
			}

			if (Warcount > 0)
			{
				if (EnemyShips > 0)
				{
					FString FormattedNumber = FString::FormatAsNumber(EnemyShips);
					DetailedShipString += FString::Printf(TEXT(", %s"), *FormattedNumber);
					if (EnemyShips == 1)
					{
						DetailedShipString += FString::Printf(TEXT(" foe"));
					}
					else
					{
						DetailedShipString += FString::Printf(TEXT(" foes"));
					}
				}
			}
			SectorText = Sector->GetSectorShips().Num() == 1 ? LOCTEXT("Ship", "{0} ship") : LOCTEXT("Ships", "{0} ships{1}");
			SectorText = FText::Format(SectorText, FText::AsNumber(TotalSectorShips), FText::FromString(DetailedShipString));
		}

		else if (DisplayMode == EFlareOrbitalMode::Battles)
		{
			SectorText = Sector->GetSectorBattleStateText(PlayerCompany);
		}
	}

	return SectorText;
}

const FSlateBrush* SFlareSectorButton::GetBackgroundBrush() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	return (IsHovered() ? &Theme.SectorButtonActiveBackground : &Theme.SectorButtonBackground);
	return &Theme.SectorButtonBackground;
}

FSlateColor SFlareSectorButton::GetMainColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	FLinearColor Color = FLinearColor::White;

	if (Sector == MenuManager->GetPC()->GetPlayerFleet()->GetCurrentSector())
	{
		Color = Theme.FriendlyColor;
	}

	Color = (IsHovered() ? Color : Color.Desaturate(0.1));
	return FSlateColor(Color);
}

FSlateColor SFlareSectorButton::GetBorderColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlareMenuManager* MenuManager = AFlareMenuManager::GetSingleton();
	FLinearColor Color = FLinearColor::White;

	if (Sector)
	{
		if (MenuManager->GetPC()->GetCurrentObjective() && MenuManager->GetPC()->GetCurrentObjective()->IsTarget(Sector))
		{
			return Theme.ObjectiveColor;
		}
		else
		{
			Color = Sector->GetSectorFriendlynessColor(PlayerCompany);
		}
	}

	Color = (IsHovered() ? Color : Color.Desaturate(0.1));
	return FSlateColor(Color);
}

FLinearColor SFlareSectorButton::GetShadowColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor Color = Theme.SmallFont.ShadowColorAndOpacity;
	FLinearColor AlphaColor = FLinearColor::White;
	Color.A = AlphaColor.A;
	return Color;
}

FReply SFlareSectorButton::OnButtonClicked()
{
	if (OnClicked.IsBound())
	{
		OnClicked.Execute();
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE