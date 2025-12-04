
#include "FlareNewGameMenu.h"
#include "../../Flare.h"

#include "../../Data/FlareCompanyCatalog.h"
#include "../../Data/FlareSpacecraftComponentsCatalog.h"
#include "../../Data/FlareCustomizationCatalog.h"
#include "../../Data/FlareStartingScenarioCatalog.h"

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
//	ScenarioList.Add(MakeShareable(new FString(TEXT("Transport"))));
//	ScenarioList.Add(MakeShareable(new FString(TEXT("Defense"))));
//  ScenarioList.Add(MakeShareable(new FString(TEXT("Debug"))));

	ScenarioList.Add(MakeShareable(new FText(LOCTEXT("Trader", "Trader"))));
	ScenarioList.Add(MakeShareable(new FText(LOCTEXT("Fighter", "Fighter"))));

	UFlareStartingScenarioCatalog* StartingScenarios = Game->GetStartingScenarios();
	TArray<UFlareStartingScenarioCatalogEntry*> ScenariosArray = StartingScenarios->GetStartingScenarios();

	for (int32 Index = 0; Index < ScenariosArray.Num(); Index++)
	{
		UFlareStartingScenarioCatalogEntry* CurrentScenario = ScenariosArray[Index];
		ScenarioList.Add(MakeShareable(new FText(CurrentScenario->StartName)));
	}

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
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)
			+ SScrollBox::Slot()
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
									.OnSelectionChanged(this, &SFlareNewGameMenu::OnComboLineSelectionChanged)
									[
										SNew(SBox)
										.Padding(Theme.ListContentPadding)
										[
											SNew(STextBlock)
											.TextStyle(&Theme.TextFont)
										]
									]
								]
							]
						]
					]

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
									.OnSelectionChanged(this, &SFlareNewGameMenu::OnComboLineSelectionChanged)
								[
									SNew(SBox)
									.Padding(Theme.ListContentPadding)
									[
										SNew(STextBlock)
										.TextStyle(&Theme.TextFont)
									]
								]
							]
						]
					]
				]
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
							.Text(LOCTEXT("Scenario", "Starting Scenario"))
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
								SAssignNew(ScenarioSelector, SFlareDropList<TSharedPtr<FText>>)
								.HeaderWidth(6)
								.ItemWidth(5)
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
		]

					+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(StartingScenarioDescriptionText,STextBlock)
							.WrapTextAt(Theme.ContentWidth * 0.75)
							.TextStyle(&Theme.NameFont)
						]

					// Bottom box
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Fill)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
							SAssignNew(TutorialButton, SFlareButton)
							.Text(LOCTEXT("Tutorial", "Tutorial Contract"))
							.HelpText(LOCTEXT("TutorialInfo", "Enable or disable helpful tutorial missions"))
							.Toggle(true)
							.Width(6.5)
							.IsDisabled(this, &SFlareNewGameMenu::IsTutorialContractDisabled)
						]
						// Story
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
							SAssignNew(StoryButton, SFlareButton)
							.Text(LOCTEXT("Story", "Story contract"))
							.HelpText(LOCTEXT("StoryInfo", "Enable or disable the storyline Pendulum quest"))
							.Toggle(true)
							.Width(6.5)
						]
					]

						// Random Station Positions
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Fill)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
							SAssignNew(RandomizeStationButton, SFlareButton)
							.Text(LOCTEXT("Randomize", "Random Station Positions"))
							.HelpText(LOCTEXT("RandomizeInfo", "Randomizes the starting positions of most stations"))
							.Toggle(true)
							.Width(6.5)
							.IsDisabled(this, &SFlareNewGameMenu::IsRandomStationPositionsDisabled)

						]

						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
							SAssignNew(AICheatsButton, SFlareButton)
							.Text(LOCTEXT("AICheats", "AI Cheats"))
							.HelpText(LOCTEXT("AICheatInfo", "Enables small cheats to favour non-player companies on Hard or higher difficulties if enabled"))
							.Toggle(true)
							.Width(6.5)
						]
					]
				// Start

					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)

					[
						SNew(SFlareButton)
						.Text(LOCTEXT("Start", "Start Game"))
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
	AICheatsButton->SetActive(false);

	ScenarioSelector->RefreshOptions();
	DifficultySelector->RefreshOptions();
	EconomySelector->RefreshOptions();
	DifficultySelector->SetSelectedIndex(1);
	EconomySelector->SetSelectedIndex(0);
	UpdateStartingScenarioDescriptionText();
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
		PC->GetMenuPawn()->ResetContent(false,true);
		PC->GetMenuPawn()->ShowPart(PartDesc);
		PC->GetMenuPawn()->SetCameraDistance(500);
	}
	// Setup colors

	ColorBox->SetupDefault();

	// List forbidden companies
	FString ForbiddenIDsString;

	TArray<FFlareCompanyDescription*> CompanyWorkingList;
	MenuManager->GetGame()->GetCompanyCatalog()->GetCompanyList(CompanyWorkingList);

//	UFlareCompanyCatalog* CompanyList = Game->GetCompanyCatalog();
//	for (const FFlareCompanyDescription& Company : CompanyList->Companies)
	for (FFlareCompanyDescription* Company : CompanyWorkingList)
	{
		ForbiddenIDs.Add(Company->ShortName);

		if (ForbiddenIDsString.Len())
		{
			ForbiddenIDsString += " - ";
		}
		ForbiddenIDsString += Company->ShortName.ToString();
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
}

void SFlareNewGameMenu::UpdateStartingScenarioDescriptionText()
{
	int32 ScenarioIndex = ScenarioList.Find(ScenarioSelector->GetSelectedItem());
	FText ScenarioText;
	FText EconomyText;
	FText DifficultyText;
	FText StartGameText;

	if (ScenarioIndex == 0)
	{
		ScenarioText = LOCTEXT("FreighterDescription", "Emerge as a brand new company in First Light piloting a single Solen trade ship.");
	}
	else if (ScenarioIndex == 1)
	{
		ScenarioText = LOCTEXT("FighterDescription", "Emerge as a brand new company in First Light piloting a single Ghoul combat fighter.");
	}
	else
	{
		UFlareStartingScenarioCatalog* StartingScenarios = Game->GetStartingScenarios();
		TArray<UFlareStartingScenarioCatalogEntry*> ScenariosArray = StartingScenarios->GetStartingScenarios();
		UFlareStartingScenarioCatalogEntry* CurrentScenario = ScenariosArray[ScenarioIndex - 2];
		ScenarioText = FText::Format(LOCTEXT("CustomText", "{0}"),
		CurrentScenario->StartDescription);
	}
	int32 EconomyIndex = EconomyList.Find(EconomySelector->GetSelectedItem());

	// Developing
	if (EconomyIndex == 1)
	{

		EconomyText = LOCTEXT("EconomyText1", "\n\
Company money + 20%\n\
Population             + 15%\n\
Station levels       + 1\n\
\n\
Companies gain additional Solen trade ships.\n\
Companies gain additional stations.\n\
");

	}

	else if (EconomyIndex == 2)
	{

		EconomyText = LOCTEXT("EconomyText2", "\n\
Company money + 40%\n\
Population             + 30%\n\
Station levels       + 2\n\
\n\
Companies gain additional Solen and Omen trade ships.\n\
Companies gain additional stations.\n\
");

	}

	else if (EconomyIndex == 3)
	{

		EconomyText = LOCTEXT("EconomyText3", "\n\
Company money + 60%\n\
Population             + 45%\n\
Station levels       + 3\n\
\n\
Companies gain additional Solen, Omen and Sphinx trade ships.\n\
Companies gain additional stations. One additional Shipyard for Ghost Works Shipyards.\n\
");

	}

	else if (EconomyIndex == 4)
	{

		EconomyText = LOCTEXT("EconomyText4", "\n\
Company money + 100%\n\
Population             + 60%\n\
Station levels       + 4\n\
\n\
Companies gain additional Solen, Omen and Sphinx trade ships.\n\
Companies gain additional stations. One additional Shipyard for Ghost Works Shipyards.\n\
");

	}
	// Easy
	int32 DifficultyIndex = DifficultyList.Find(DifficultySelector->GetSelectedItem());
	if (DifficultyIndex == 0)
	{
		DifficultyText = LOCTEXT("DifficultyText0", "Global War event against the player is disabled.\n\
Reduced reputation losses.");
	}
	// Hard
	else if (DifficultyIndex == 2)
	{
		DifficultyText = LOCTEXT("DifficultyText2", "Pirates and Broken Moon gain additional Ghoul fighters.\n\
\n\
Reduced Global War event cooldown against the player.\n\
Increased reputation losses. Further reputation losses if the player holds 80% of the money circulating among companies.\n\
");
	}
	// Very Hard
	else if (DifficultyIndex == 3)
	{
		DifficultyText = LOCTEXT("DifficultyText3", "Pirates and Broken Moon gain additional Ghoul and Orca fighters.\n\
\n\
Reduced Global War event cooldown against the player, slightly higher chance of occurance.\n\
Increased reputation losses. Further reputation losses if the player holds 70% of the money circulating among companies.\n\
");
	}
	// Expert
	else if (DifficultyIndex == 4)
	{
		DifficultyText = LOCTEXT("DifficultyText4", "Pirates and Broken Moon gain additional Ghoul, Orca and Phalanx fighters.\n\
\n\
Reduced Global War event cooldown against the player, slightly higher chance of occurance.\n\
Increased reputation losses. Further reputation losses if the player holds 60% of the money circulating among companies.\n\
");
	}
	// Unfair
	else if (DifficultyIndex == 5)
	{
		DifficultyText = LOCTEXT("DifficultyText5", "Pirates and Broken Moon gain additional Ghoul, Orca and Phalanx fighters.\n\
Pirates gain an additional Kami.\n\
Quantalium gain an Invader.\n\
Nema Heavy Works and Ghost Works Shipyards gain an Anubis cruiser.\n\
\n\
Reduced Global War event cooldown against the player, higher chance of occurance.\n\
Increased reputation losses. Further reputation losses if the player holds 50% of the money circulating with companies.\n\
");
	}

	StartGameText = FText::Format(LOCTEXT("StartGameInfo", "\n\
{0}\n\
{1}\n\
{2}"),
ScenarioText, EconomyText, DifficultyText);
	
	StartingScenarioDescriptionText->SetText(StartGameText);
}

/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

bool SFlareNewGameMenu::IsTutorialContractDisabled() const
{
	if (RandomizeStationButton->IsActive())
	{
		return true;
	}

	int32 ScenarioIndex = ScenarioList.Find(ScenarioSelector->GetSelectedItem());
	if (ScenarioIndex > 1)
	{
		TutorialButton->SetActive(false);
		return true;
	}
	return false;
}

bool SFlareNewGameMenu::IsRandomStationPositionsDisabled() const
{
	if (TutorialButton->IsActive())
	{
		return true;
	}
	return false;
}

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
		int32 ScenarioIndex = ScenarioList.Find(ScenarioSelector->GetSelectedItem());
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
		Data.AICheats = AICheatsButton->IsActive();

		MenuManager->OpenMenu(EFlareMenu::MENU_CreateGame, Data);
	}
}

FText SFlareNewGameMenu::OnGetCurrentComboLine() const
{
	TSharedPtr<FText> Item = ScenarioSelector->GetSelectedItem();
	return Item.IsValid() ? (*Item) : (*ScenarioList[0]);
}

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

void SFlareNewGameMenu::OnComboLineSelectionChanged(TSharedPtr<FText> StringItem, ESelectInfo::Type SelectInfo)
{
	UpdateStartingScenarioDescriptionText();
}

void SFlareNewGameMenu::OnEmblemPicked(int32 Index)
{
}

#undef LOCTEXT_NAMESPACE

