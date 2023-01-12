
#include "FlareNewGameMenu.h"
#include "../../Flare.h"

#include "../../Data/FlareCompanyCatalog.h"
#include "../../Data/FlareSpacecraftComponentsCatalog.h"
#include "../../Data/FlareCustomizationCatalog.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlareSaveGame.h"

#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#include "../FlareUITypes.h"
#include "STextComboBox.h"
#include "GameFramework/PlayerState.h"


#define LOCTEXT_NAMESPACE "FlareNewGameMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareNewGameMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	Game = MenuManager->GetPC()->GetGame();

	// Game starts
	// ScenarioList.Add(MakeShareable(new FString(TEXT("Transport"))));
	// ScenarioList.Add(MakeShareable(new FString(TEXT("Defense"))));
	// ScenarioList.Add(MakeShareable(new FString(TEXT("Debug"))));

	DifficultyList.Add(MakeShareable(new FText(LOCTEXT("Easy", "Easy"))));
	DifficultyList.Add(MakeShareable(new FText(LOCTEXT("Normal", "Normal"))));
	DifficultyList.Add(MakeShareable(new FText(LOCTEXT("Hard", "Hard"))));
	DifficultyList.Add(MakeShareable(new FText(LOCTEXT("VeryHard", "Very Hard"))));
	DifficultyList.Add(MakeShareable(new FText(LOCTEXT("Expert", "Expert"))));
	DifficultyList.Add(MakeShareable(new FText(LOCTEXT("Unfair", "Unfair"))));

	EconomyList.Add(MakeShareable(new FText(LOCTEXT("Beginning", "Beginning"))));
	EconomyList.Add(MakeShareable(new FText(LOCTEXT("Developing", "Developing"))));
	EconomyList.Add(MakeShareable(new FText(LOCTEXT("Prospering", "Prospering"))));
	EconomyList.Add(MakeShareable(new FText(LOCTEXT("Maturing", "Maturing"))));
	EconomyList.Add(MakeShareable(new FText(LOCTEXT("Accomplished", "Accomplished"))));
	// Color
	FLinearColor Color = Theme.NeutralColor;
	Color.A = Theme.DefaultAlpha;

	// Name
	FText DefaultName = LOCTEXT("CompanyName", "Player Inc");
	FText DefaultIdentifier = LOCTEXT("CompanyCode", "PLY");
	FString PlayerName = MenuManager->GetPC()->PlayerState->GetPlayerName();
	if (PlayerName.Len())
	{
		DefaultName = FText::Format(LOCTEXT("CompanyNameFormat", "{0} Corp"), FText::FromString(PlayerName));
	}

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SHorizontalBox)

		// Content block
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.AutoWidth()
		[
			SNew(SVerticalBox)
		
			// Main form
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.ContentPadding)
			.HAlign(HAlign_Right)
			[
				SNew(SVerticalBox)
				
				// Title
				+ SVerticalBox::Slot()
				.Padding(Theme.TitlePadding)
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(&Theme.SubTitleFont)
					.Text(LOCTEXT("NewGameTitle", "NEMA COLONIAL ADMINISTRATION"))
				]

				// Scenario
				/*+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(LOCTEXT("NewGameScenario", "Company trade"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					[
						SNew(SBox)
						.WidthOverride(0.4 * Theme.ContentWidth)
						[
							SNew(SBorder)
							.BorderImage(&Theme.BackgroundBrush)
							.Padding(Theme.ContentPadding)
							[
								SAssignNew(ScenarioSelector, SFlareDropList<TSharedPtr<FString>>)
								.OptionsSource(&ScenarioList)
								.OnGenerateWidget(this, &SFlareNewGameMenu::OnGenerateComboLine)
								.OnSelectionChanged(this, &SFlareNewGameMenu::OnComboLineSelectionChanged)
								[
									SNew(SBox)
									.Padding(Theme.ListContentPadding)
									[
										SNew(STextBlock)
										.Text(this, &SFlareNewGameMenu::OnGetCurrentComboLine)
										.TextStyle(&Theme.TextFont)
									]
								]
							]
						]
					]
				]*/

				// Company name
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(LOCTEXT("NewGameCompany", "Company name (Up to 25 characters)"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					[
						SNew(SBox)
						.WidthOverride(0.4 * Theme.ContentWidth)
						[
							SNew(SBorder)
							.BorderImage(&Theme.BackgroundBrush)
							.Padding(Theme.ContentPadding)
							[
								SAssignNew(CompanyName, SEditableText)
								.AllowContextMenu(false)
								.Text(DefaultName)
								.Style(&Theme.TextInputStyle)
							]
						]
					]
				]
				
				// Company ID
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(LOCTEXT("NewGameIdentifier", "Hull identifier (Three letters only)"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					[
						SNew(SBox)
						.WidthOverride(0.4 * Theme.ContentWidth)
						[
							SNew(SBorder)
							.BorderImage(&Theme.BackgroundBrush)
							.Padding(Theme.ContentPadding)
							[
								SAssignNew(CompanyIdentifier, SEditableText)
								.AllowContextMenu(false)
								.Text(DefaultIdentifier)
								.Style(&Theme.TextInputStyle)
							]
						]
					]
				]

				// Company ID hint
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SAssignNew(CompanyIDHint, STextBlock)
					.TextStyle(&Theme.SmallFont)
				]
				
				// Emblem
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					// Help
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Top)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(LOCTEXT("EmblemTitle", "Company emblem"))
					]

					// Emblem picker
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					[
						SNew(SBox)
						.WidthOverride(0.4 * Theme.ContentWidth)
						[
							SAssignNew(EmblemPicker, SFlareDropList<int32>)
							.LineSize(2)
							.HeaderWidth(3)
							.HeaderHeight(3)
							.ItemWidth(3)
							.ItemHeight(2.7)
							.ShowColorWheel(false)
							.OnItemPicked(this, &SFlareNewGameMenu::OnEmblemPicked)
						]
					]
				]	
				//Difficulty
				+ SVerticalBox::Slot()
				.Padding(Theme.ContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(LOCTEXT("GameDifficulty", "Game Difficulty"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					[
						SNew(SBox)
						.WidthOverride(0.4 * Theme.ContentWidth)
						[

							SNew(SBorder)
							.BorderImage(&Theme.BackgroundBrush)
							.Padding(Theme.ContentPadding)
							[
								SAssignNew(DifficultySelector, SFlareDropList<TSharedPtr<FText>>)
								.OptionsSource(&DifficultyList)
								.HeaderWidth(6)
								.OnGenerateWidget(this, &SFlareNewGameMenu::OnGenerateComboLine)
								[
									SNew(SBox)
									.Padding(Theme.ListContentPadding)
									[
										SNew(STextBlock)
										.Text(this, &SFlareNewGameMenu::OnGetCurrentComboLine)
										.TextStyle(&Theme.TextFont)
									]
								]
							]
						]
					]
				]
				//this one
				+ SVerticalBox::Slot()
					.Padding(Theme.ContentPadding)
					.AutoHeight()
					.HAlign(HAlign_Fill)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
					.Text(LOCTEXT("Economy", "Starting Economy"))
					]

				+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					[
						SNew(SBox)
						.WidthOverride(0.4 * Theme.ContentWidth)
					[

						SNew(SBorder)
						.BorderImage(&Theme.BackgroundBrush)
					.Padding(Theme.ContentPadding)
					[
						SAssignNew(EconomySelector, SFlareDropList<TSharedPtr<FText>>)
						.OptionsSource(&EconomyList)
						.HeaderWidth(6)
						.OnGenerateWidget(this, &SFlareNewGameMenu::OnGenerateComboLine)
					[
						SNew(SBox)
						.Padding(Theme.ListContentPadding)
					[
						SNew(STextBlock)
						.Text(this, &SFlareNewGameMenu::OnGetCurrentComboLine)
					.TextStyle(&Theme.TextFont)
					]
				]
			]
		]
	]
]
				
					/*
												// List
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.ContentPadding)
							[
								SAssignNew(PlanetSelector, SFlareDropList<FFlareSectorCelestialBodyDescription>)
								.OptionsSource(&MenuManager->GetPC()->GetGame()->GetOrbitalBodies()->OrbitalBodies)
								.OnGenerateWidget(this, &SFlareSkirmishSetupMenu::OnGeneratePlanetComboLine)
								.HeaderWidth(6)
								.ItemWidth(6)
								[
									SNew(SBox)
									.Padding(Theme.ListContentPadding)
									[
										SNew(STextBlock)
										.Text(this, &SFlareSkirmishSetupMenu::OnGetCurrentPlanetComboLine)
										.TextStyle(&Theme.TextFont)
									]
								]
							]
					*/

				// Bottom box
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(SVerticalBox)

					// Tutorial
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Right)
					[
						SAssignNew(TutorialButton, SFlareButton)
						.Text(LOCTEXT("Tutorial", "Training contract"))
						.HelpText(LOCTEXT("TutorialInfo", "Start with a few training contracts"))
						.Toggle(true)
						.Width(6.5)
					]
				// Story
				+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Right)
					[
						SAssignNew(StoryButton, SFlareButton)
						.Text(LOCTEXT("Story", "Story contract"))
					.HelpText(LOCTEXT("StoryInfo", "Disable the storyline Pendulum quest"))
					.Toggle(true)
					.Width(6.5)
					]
				// Random Station Positions
				+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Right)
					[
						SAssignNew(RandomizeStationButton, SFlareButton)
						.Text(LOCTEXT("Randomize", "Random Station Positions"))
					.HelpText(LOCTEXT("RandomizeInfo", "Randomizes most of the starting stations possible sector positions"))
					.Toggle(true)
					.Width(6.5)
					]
				// Start
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Right)
					[
						SNew(SFlareButton)
						.Text(LOCTEXT("Start", "Start"))
						.HelpText(LOCTEXT("StartInfo", "Confirm the creation of a new game and start playing"))
						.Icon(FFlareStyleSet::GetIcon("Load"))
						.OnClicked(this, &SFlareNewGameMenu::OnLaunch)
						.IsDisabled(this, &SFlareNewGameMenu::IsLaunchDisabled)
						.Width(6.5)
					]
				]
			]
		]

		// Color box
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.Padding(Theme.ContentPadding)
		[
			SAssignNew(ColorBox, SFlareColorPanel)
			.MenuManager(MenuManager)
		]
	];

	TutorialButton->SetActive(true);
	StoryButton->SetActive(true);
	RandomizeStationButton->SetActive(false);
	//ScenarioSelector->SetSelectedIndex(0);

	DifficultySelector->RefreshOptions();
	EconomySelector->RefreshOptions();
	DifficultySelector->SetSelectedIndex(1);
	EconomySelector->SetSelectedIndex(0);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareNewGameMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareNewGameMenu::Enter()
{
	FLOG("SFlareNewGameMenu::Enter");

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	
	// Get decorator mesh
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		const FFlareSpacecraftComponentDescription* PartDesc = PC->GetGame()->GetShipPartsCatalog()->Get("object-safe");
		PC->GetMenuPawn()->SetCameraOffset(FVector2D(150, 30));
		PC->GetMenuPawn()->ShowPart(PartDesc);
		PC->GetMenuPawn()->SetCameraDistance(500);
	}

	// Setup colors
	ColorBox->SetupDefault();

	// List forbidden companies
	FString ForbiddenIDsString;
	UFlareCompanyCatalog* CompanyList = Game->GetCompanyCatalog();
	for (const FFlareCompanyDescription& Company : CompanyList->Companies)
	{
		ForbiddenIDs.Add(Company.ShortName);

		if (ForbiddenIDsString.Len())
		{
			ForbiddenIDsString += " - ";
		}
		ForbiddenIDsString += Company.ShortName.ToString();
	}
	FText ForbiddenIdsText = FText::Format(LOCTEXT("ForbiddenIdListFormat", "Some identifiers are forbidden ({0})"), FText::FromString(ForbiddenIDsString));
	CompanyIDHint->SetText(ForbiddenIdsText);

	// Fill emblems
	UFlareCustomizationCatalog* CustomizationCatalog = Game->GetCustomizationCatalog();
	for (int i = 0; i < CustomizationCatalog->GetEmblemCount(); i++)
	{
		EmblemPicker->AddItem(SNew(SImage).Image(CustomizationCatalog->GetEmblemBrush(i)));
	}

	EmblemPicker->SetSelectedIndex(0);
}

void SFlareNewGameMenu::Exit()
{
	SetEnabled(false);
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC)
	{
		PC->GetMenuPawn()->SetIsEnabled(false);
	}
	SetVisibility(EVisibility::Collapsed);
	EmblemPicker->ClearItems();
//	DifficultySelector->ClearItems();
//	EconomySelector->ClearItems();
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

bool SFlareNewGameMenu::IsLaunchDisabled() const
{
	FString CompanyNameData = CompanyName->GetText().ToString();
	FString CompanyIdentifierData = CompanyIdentifier->GetText().ToString();

	if (CompanyNameData.Len() > 25 || CompanyIdentifierData.Len() != 3)
	{
		return true;
	}
	else if (ForbiddenIDs.Find(FName(*CompanyIdentifierData)) != INDEX_NONE)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SFlareNewGameMenu::OnLaunch()
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	UFlareCustomizationCatalog* CustomizationCatalog = Game->GetCustomizationCatalog();
	const FFlareCompanyDescription* CurrentCompanyData = PC->GetCompanyDescription();

	if (PC && Game && !Game->IsLoadedOrCreated())
	{
		// Get data
		FText CompanyNameData = FText::FromString(CompanyName->GetText().ToString().Left(25)); // FString needed here
		FName CompanyIdentifierData = FName(*CompanyIdentifier->GetText().ToString().ToUpper().Left(3)); // FString needed here
		int32 ScenarioIndex = 0;//ScenarioList.Find(ScenarioSelector->GetSelectedItem());
		int32 DifficultyIndex = DifficultyList.Find(DifficultySelector->GetSelectedItem());
		int32 EconomyIndex = EconomyList.Find(EconomySelector->GetSelectedItem());
		int32 EmblemIndex = EmblemPicker->GetSelectedIndex();

		FLOGV("SFlareNewGameMenu::OnLaunch '%s', ID '%s', ScenarioIndex %d", *CompanyNameData.ToString(), *CompanyIdentifierData.ToString(), ScenarioIndex);

		// Create company data
		CompanyData.Name = CompanyNameData;
		CompanyData.ShortName = CompanyIdentifierData;
		CompanyData.Emblem = CustomizationCatalog->GetEmblem(EmblemIndex);
		CompanyData.CustomizationBasePaintColor = CurrentCompanyData->CustomizationBasePaintColor;
		CompanyData.CustomizationPaintColor = CurrentCompanyData->CustomizationPaintColor;
		CompanyData.CustomizationOverlayColor = CurrentCompanyData->CustomizationOverlayColor;
		CompanyData.CustomizationLightColor = CurrentCompanyData->CustomizationLightColor;
		CompanyData.CustomizationPatternIndex = CurrentCompanyData->CustomizationPatternIndex;
		
		// Create game
		FFlareMenuParameterData Data;
		Data.CompanyDescription = &CompanyData;
		Data.ScenarioIndex = ScenarioIndex;
		Data.DifficultyIndex = DifficultyIndex;
		Data.EconomyIndex = EconomyIndex;
		Data.PlayerEmblemIndex = EmblemIndex;
		Data.PlayTutorial = TutorialButton->IsActive();
		Data.PlayStory = StoryButton->IsActive();
		Data.RandomizeStations = RandomizeStationButton->IsActive();
		MenuManager->OpenMenu(EFlareMenu::MENU_CreateGame, Data);
	}
}

FText SFlareNewGameMenu::OnGetCurrentComboLine() const
{
	TSharedPtr<FString> Item = ScenarioSelector->GetSelectedItem();
	return Item.IsValid() ? FText::FromString(*Item) : FText::FromString(*ScenarioList[0]);
}

//TSharedRef<SWidget> SFlareNewGameMenu::OnGenerateComboLine(TSharedPtr<FString> Item)
TSharedRef<SWidget> SFlareNewGameMenu::OnGenerateComboLine(TSharedPtr<FText> Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(STextBlock)
//		.Text(FText::FromString(*Item)) // FString needed here
		.Text(*Item) // FString needed here
		.TextStyle(&Theme.TextFont)
	];
}

void SFlareNewGameMenu::OnComboLineSelectionChanged(TSharedPtr<FString> StringItem, ESelectInfo::Type SelectInfo)
{
}

void SFlareNewGameMenu::OnEmblemPicked(int32 Index)
{
}

#undef LOCTEXT_NAMESPACE

