
#include "FlareTechnologyMenu.h"
#include "../../Flare.h"

#include "../Components/FlareTechnologyInfo.h"
#include "../Components/FlareTabView.h"

#include "../../Data/FlareTechnologyCatalog.h"
#include "../../Data/FlareScannableCatalog.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlareCompany.h"
#include "../../Player/FlareMenuManager.h"

#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareTechnologyMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareTechnologyMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlarePlayerController* PC = MenuManager->GetPC();
	
	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SFlareTabView)

		// Technology block
		+ SFlareTabView::Slot()
		.Header(LOCTEXT("TechnologyMainTab", "Technologies"))
		.HeaderHelp(LOCTEXT("TechnologyMainTabInfo", "Technology tree & research status"))
		[
			SNew(SBox)
			.WidthOverride(2.2 * Theme.ContentWidth)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			[
				SNew(SHorizontalBox)

				// Info block
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(0.75 * Theme.ContentWidth)
					.HAlign(HAlign_Left)
					[
						SNew(SVerticalBox)
			
						// Title
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("CompanyTechnologyTitle", "Company technology"))
							.TextStyle(&Theme.SubTitleFont)
						]
			
						// Company info
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(CompanyTechnologyInfo,SRichTextBlock)
							.TextStyle(&Theme.TextFont)
							.DecoratorStyleSet(&FFlareStyleSet::Get())
							.WrapTextAt(0.7 * Theme.ContentWidth)
						]
			
						// Title
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(this, &SFlareTechnologyMenu::GetTechnologyName)
						]
			
						// Description
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(this, &SFlareTechnologyMenu::GetTechnologyDescription)
							.WrapTextAt(0.7 * Theme.ContentWidth)
						]
			
						// Button
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						.HAlign(HAlign_Left)
						[
							SAssignNew(UnlockTechnologyButton,SFlareButton)
							.Width(6)
							.Icon(FFlareStyleSet::GetIcon("ResearchValue"))
							.Text(LOCTEXT("UnlockTechOK", "Research technology"))
							.HelpText(LOCTEXT("UnlockTechInfo", "Research this technology by spending some of your available research budget"))
							.OnClicked(this, &SFlareTechnologyMenu::OnTechnologyUnlocked)
						]
					]
				]

				// Tree block
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				[			
					SNew(SVerticalBox)
			
					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.TitlePadding)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("AvailableTechnologyTitle", "Available technologies"))
						.TextStyle(&Theme.SubTitleFont)
					]

					// Data
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SAssignNew(TechnologyTree, SScrollBox)
						.Style(&Theme.ScrollBoxStyle)
						.ScrollBarStyle(&Theme.ScrollBarStyle)
					]
				]
			]
		]

		// Scannable block
		+ SFlareTabView::Slot()
		.Header(LOCTEXT("TechnologyScannableTab", "Artifacts"))
		.HeaderHelp(LOCTEXT("TechnologyScannableTabInfo", "Artifacts are found drifting in the world and unlock research data"))
		[
			SNew(SBox)
			.WidthOverride(2.2 * Theme.ContentWidth)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.HAlign(HAlign_Left)
				.WidthOverride(2.2 * Theme.ContentWidth)
				[
					SNew(SVerticalBox)

					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.TitlePadding)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("CompanyScannableTitle", "Artifacts found"))
						.TextStyle(&Theme.SubTitleFont)
					]

					// Count
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.Text(this, &SFlareTechnologyMenu::GetUnlockedScannableCount)
						.TextStyle(&Theme.TextFont)
						.WrapTextAt(2 * Theme.ContentWidth)
					]

					// Content
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Left)
					[
						SAssignNew(ArtifactList, SVerticalBox)
					]
				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareTechnologyMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareTechnologyMenu::ResetDefaults()
{
	TutorialQuestComplete = false;
}

void SFlareTechnologyMenu::Enter()
{
	FLOG("SFlareTechnologyMenu::Enter");

	// Menu data
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	// Sorting criteria for technologies
	struct FSortByLevelCategoryAndCost
	{
		FORCEINLINE bool operator()(const FFlareTechnologyDescription& A, const FFlareTechnologyDescription& B) const
		{
			if (A.Level > B.Level)
			{
				return true;
			}
			else if (A.Level < B.Level)
			{
				return false;
			}
			else if (A.Category < B.Category)
			{
				return true;
			}
			else if (A.Category > B.Category)
			{
				return false;
			}
			else
			{
				if (A.ResearchCost < B.ResearchCost)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		}
	};

	// List all technologies
	SelectedTechnology = NULL;
	TArray<const FFlareTechnologyDescription*> Technologies;

	for (UFlareTechnologyCatalogEntry* Technology : MenuManager->GetGame()->GetTechnologyCatalog()->TechnologyCatalog)
	{
		FFlareTechnologyDescription* TechnologyData = &Technology->Data;
		if (TechnologyData->AIOnly)
		{
			continue;
		}
		if (TechnologyData->ResearchableCompany.Num() > 0)
		{
			//company array has something, check if faction is allowed to research this
			if (!TechnologyData->ResearchableCompany.Contains(FName("PLAYER")))
			{
				continue;
			}
		}

		Technologies.Add(&Technology->Data);
	}

	Technologies.Sort(FSortByLevelCategoryAndCost());

	// Add technologies to the tree
	int CurrentLevel = -1;
	UFlareCompany* Company = MenuManager->GetPC()->GetCompany();

	TSharedPtr<SHorizontalBox> CurrentLevelRow;
	for (const FFlareTechnologyDescription* Technology : Technologies)
	{
		// Add a new row
		if (Technology->Level != CurrentLevel)
		{
			CurrentLevel = Technology->Level;

			// Row
			TechnologyTree->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Center)
			[
				SAssignNew(CurrentLevelRow, SHorizontalBox)
			];

			// Technology level
			CurrentLevelRow->AddSlot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(0, 0, 0, 10))
			.MaxWidth(64.f)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(FText::Format(LOCTEXT("CurrentLevelFormat", "{0}"), FText::AsNumber(CurrentLevel)))
				.ColorAndOpacity(this, &SFlareTechnologyMenu::GetTitleTextColor, CurrentLevel)
			];
			
			// Technology level cost
			CurrentLevelRow->AddSlot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(0, 0, 0, 10))
			[
				SNew(STextBlock)
				.TextStyle(&Theme.SmallFont)
				.Text(this, &SFlareTechnologyMenu::GetTechnologyLevelCost, CurrentLevel)
				.ColorAndOpacity(this, &SFlareTechnologyMenu::GetTechnologyLevelCostTextColor, CurrentLevel)
			];
		}

		CurrentLevelRow->AddSlot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(Theme.ContentPadding * 0.75)
		[
			SNew(SBox)
			.WidthOverride(Theme.ContentWidth)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			[
				SNew(SFlareTechnologyInfo)
				.MenuManager(MenuManager)
				.Technology(Technology)
				.OnClicked(FFlareButtonClicked::CreateSP(this, &SFlareTechnologyMenu::OnTechnologySelected, Technology))
			]
		];
	}

	// List artifacts
	ArtifactList->ClearChildren();
	for (FName UnlockedScannableIdentifier : MenuManager->GetPC()->GetUnlockedScannables())
	{
		auto Scannable = MenuManager->GetGame()->GetScannableCatalog()->Get(UnlockedScannableIdentifier);
		FCHECK(Scannable);

		ArtifactList->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(SVerticalBox)

				// Upper line
				+ SVerticalBox::Slot()
				.AutoHeight()
				[						
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(SImage)
						.Image(FFlareStyleSet::GetIcon("OK"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.TextStyle(&Theme.NameFont)
						.Text(Scannable->Name)
					]
				]

				// Lower line
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.SmallContentPadding)
				[						
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(Scannable->Description)
					.WrapTextAt(Theme.ContentWidth)
				]
			];
	}	

	SlatePrepass(FSlateApplicationBase::Get().GetApplicationScale());

	if (!TutorialQuestComplete)
	{
		UFlareQuest* TutorialQuest = MenuManager->GetGame()->GetQuestManager()->FindQuest("tutorial-technology");
		if (TutorialQuest)
		{
			if (MenuManager->GetGame()->GetQuestManager()->IsQuestOngoing(TutorialQuest) || MenuManager->GetGame()->GetQuestManager()->IsQuestSuccessfull(TutorialQuest))
			{
				TutorialQuestComplete = true;
			}
		}
	}

	UpdateAllInfos();
}

void SFlareTechnologyMenu::Exit()
{
	SetEnabled(false);

	SelectedTechnology = NULL;
	TechnologyTree->ClearChildren();

	ArtifactList->ClearChildren();

	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareTechnologyMenu::UpdateAllInfos()
{
	UpdateCompanyTechnologyInfo();
	UpdateTechnologyButtonInfo();
}

void SFlareTechnologyMenu::UpdateTechnologyButtonInfo()
{
	if (!TutorialQuestComplete)
	{
		UnlockTechnologyButton->SetText(LOCTEXT("UnlockTechImpossible", "Can't research"));
		UnlockTechnologyButton->SetHelpText(LOCTEXT("UnlockTechTutorialRequiredInfo", "Technology research is disabled until the research tutorial"));
		UnlockTechnologyButton->SetDisabled(true);
		return;
	}

	UFlareCompany* Company = MenuManager->GetPC()->GetCompany();
	FText Reason;
	if (SelectedTechnology)
	{
		if (!Company->IsTechnologyAvailable(SelectedTechnology->Identifier, Reason))
		{
			if (Company->GetTechnologyLevel() < SelectedTechnology->Level)
			{
				int32 LevelDifference = SelectedTechnology->Level - Company->GetTechnologyLevel();
				int32 TechnologyUpgradeCost = Company->GetTechnologyLevelUpgradeCost(LevelDifference);

				UnlockTechnologyButton->SetText(LOCTEXT("UnlockTechUpgradeTech", "Upgrade Tech Level"));
				UnlockTechnologyButton->SetHelpText(FText::Format(LOCTEXT("UnlockTechUpgradeTechHint", "Upgrade to Tech Level {0} by spending {1} research points"),
				FText::AsNumber(Company->GetTechnologyLevel() + LevelDifference),
				FText::AsNumber(TechnologyUpgradeCost)));
				if (Company->GetResearchAmount() >= TechnologyUpgradeCost)
				{
					UnlockTechnologyButton->SetDisabled(false);
					return;
				}
			}
			else
			{
				UnlockTechnologyButton->SetText(LOCTEXT("UnlockTechImpossible", "Can't research"));
				UnlockTechnologyButton->SetHelpText(Reason);
			}
		}
		else
		{
			UnlockTechnologyButton->SetText(LOCTEXT("UnlockTechOK", "Research technology"));
			UnlockTechnologyButton->SetHelpText(FText::Format(LOCTEXT("UnlockTechInfoCosts", "Research {0} by spending {1} research points"),
			SelectedTechnology->Name,
			FText::AsNumber(MenuManager->GetPC()->GetCompany()->GetTechnologyCost(SelectedTechnology))));
			UnlockTechnologyButton->SetDisabled(false);
			return;
		}
	}
	else
	{
		UnlockTechnologyButton->SetText(LOCTEXT("UnlockTechOK", "Research technology"));
		UnlockTechnologyButton->SetHelpText(LOCTEXT("UnlockTechInfo", "Research this technology by spending some of your available research budget"));
	}
	UnlockTechnologyButton->SetDisabled(true);
}

void SFlareTechnologyMenu::UpdateCompanyTechnologyInfo()
{
	UFlareCompany* Company = MenuManager->GetPC()->GetCompany();
	if (!TutorialQuestComplete)
	{
		CompanyTechnologyInfo->SetText(FText::Format(LOCTEXT("TechnologyCompanyTutorialRequiredFormat", "\u2022 <WarningText>Technology research is disabled until the research tutorial.</>\n\u2022 You have <HighlightText>{0} research points</>."),
			FText::AsNumber(Company->GetResearchAmount()),
			FText::AsNumber(Company->GetResearchSpent())));
		return;
	}

	CompanyTechnologyInfo->SetText(FText::Format(LOCTEXT("TechnologyCompanyFormat", "\u2022 You can currently research technology up to <HighlightText>level {0}</>.\n\u2022 You have <HighlightText>{1} research points</>.\n\u2022 You have already spent {2} research points."),
		FText::AsNumber(Company->GetTechnologyLevel()),
		FText::AsNumber(Company->GetResearchAmount()),
		FText::AsNumber(Company->GetResearchSpent())));
}

FText SFlareTechnologyMenu::GetTechnologyName() const
{
	if (SelectedTechnology)
	{
		return SelectedTechnology->Name;
	}
	else
	{
		return LOCTEXT("TechnologyDetailsTitle", "Technology details");
	}
}

FText SFlareTechnologyMenu::GetTechnologyDescription() const
{
	if (SelectedTechnology)
	{
		return SelectedTechnology->Description;
	}
	else
	{
		return LOCTEXT("NotechnologySelected", "No technology selected.");
	}
}

FSlateColor SFlareTechnologyMenu::GetTitleTextColor(int32 RowLevel) const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UFlareCompany* Company = MenuManager->GetPC()->GetCompany();

	if (RowLevel <= Company->GetTechnologyLevel())
	{
		return Theme.NeutralColor;
	}
	else
	{
		return Theme.UnknownColor;
	}
}

FSlateColor SFlareTechnologyMenu::GetTechnologyLevelCostTextColor(int32 RowLevel) const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	UFlareCompany* Company = MenuManager->GetPC()->GetCompany();

	if (!TutorialQuestComplete || RowLevel > Company->GetTechnologyLevel())
	{
		int32 LevelDifference = RowLevel - Company->GetTechnologyLevel();
		int32 TechUpgradeCost = Company->GetTechnologyLevelUpgradeCost(LevelDifference);
		if (Company->GetResearchAmount() >= TechUpgradeCost)
		{
			return Theme.NeutralColor;
		}

		return Theme.UnknownColor;
	}

	return Theme.NeutralColor;
}

FSlateColor SFlareTechnologyMenu::GetTechnologyUpgradeTextColor(int32 RowLevel) const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UFlareCompany* Company = MenuManager->GetPC()->GetCompany();

	if (!TutorialQuestComplete || RowLevel > Company->GetTechnologyLevel())
	{
		return Theme.UnknownColor;
	}
	return Theme.NeutralColor;
}

void SFlareTechnologyMenu::OnTechnologySelected(const FFlareTechnologyDescription* Technology)
{
	SelectedTechnology = Technology;
	UpdateTechnologyButtonInfo();
}

void SFlareTechnologyMenu::OnTechnologyUnlocked()
{
	if (SelectedTechnology)
	{
		UFlareCompany* Company = MenuManager->GetPC()->GetCompany();
		if(!Company->UnlockTechnology(SelectedTechnology->Identifier))
		{
			if (Company->GetTechnologyLevel() < SelectedTechnology->Level)
			{
				int32 LevelDifference = SelectedTechnology->Level - Company->GetTechnologyLevel();
				Company->UpgradeTechnologyLevel(LevelDifference);
			}
		}
		UpdateAllInfos();
	}
}

FText SFlareTechnologyMenu::GetUnlockedScannableCount() const
{
	return FText::Format(LOCTEXT("ScannableCountFormat", "You have found {0} out of {1} artifacts. Explore the world and discover new sectors to unlock more research data !"),
		FText::AsNumber(MenuManager->GetPC()->GetUnlockedScannableCount()),
		FText::AsNumber(MenuManager->GetGame()->GetScannableCatalog()->ScannableCatalog.Num()));
}

FText SFlareTechnologyMenu::GetTechnologyLevelCost(int32 Level) const
{
	int32 CompanyLevel = MenuManager->GetPC()->GetCompany()->GetTechnologyLevel();
	if (CompanyLevel < Level)
	{
		int32 LevelDifference = Level - CompanyLevel;
		return FText::AsNumber(MenuManager->GetPC()->GetCompany()->GetTechnologyLevelUpgradeCost(LevelDifference));
	}
	return FText();
}

#undef LOCTEXT_NAMESPACE