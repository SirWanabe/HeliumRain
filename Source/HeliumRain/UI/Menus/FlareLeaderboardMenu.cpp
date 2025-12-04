
#include "FlareLeaderboardMenu.h"
#include "../../Flare.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#include "../Components/FlareListItem.h"
#include "../Components/FlareCompanyInfo.h"

#define LOCTEXT_NAMESPACE "FlareLeaderboardMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareLeaderboardMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	Game = MenuManager->GetGame();

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SVerticalBox)

		// Title
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Theme.ContentPadding)
		[
			SNew(STextBlock)
			.Text(this, &SFlareLeaderboardMenu::GetWindowTitle)
			.TextStyle(&Theme.TitleFont)
			.Justification(ETextJustify::Center)
		]

		// Company list
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.Padding(Theme.TitlePadding)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("LeaderboardTitle", "Leading companies"))
					.TextStyle(&Theme.SubTitleFont)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox)
					.WidthOverride(Theme.ContentWidth * 1.7)
					[
						SAssignNew(CompanyList, SListView< TSharedPtr<FInterfaceContainer> >)
						.ListItemsSource(&CompanyListData)
						.SelectionMode(ESelectionMode::Single)
						.OnGenerateRow(this, &SFlareLeaderboardMenu::GenerateCompanyInfo)
						.OnSelectionChanged(this, &SFlareLeaderboardMenu::OnCompanySelectionChanged)
					]
				]
			]
		]
	];

}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareLeaderboardMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareLeaderboardMenu::OnCompanySelectionChanged(TSharedPtr<FInterfaceContainer> Item, ESelectInfo::Type SelectInfo)
{
	FLOG("SFlareLeaderboardMenu::OnCompanySelectionChanged");

	TSharedPtr<SFlareListItem> ItemWidget = StaticCastSharedPtr<SFlareListItem>(CompanyList->WidgetFromItem(Item));
	SelectedItem = Item;

	// Update selection
	if (PreviousSelection.IsValid())
	{
		PreviousSelection->SetSelected(false);
	}

	if (ItemWidget.IsValid())
	{
		ItemWidget->SetSelected(true);
		PreviousSelection = ItemWidget;
	}

//	this->SetCompany(Company);
}

void SFlareLeaderboardMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	if (CompanyList->GetNumItemsSelected() > 0)
	{
		UFlareCompany* Company = CompanyList->GetSelectedItems()[0]->CompanyPtr;
		SelectedCompany = Company;
	}
}

void SFlareLeaderboardMenu::SetCompany(UFlareCompany* Company)
{
	if (Company)
	{
		SelectedCompany = Company;
	}
}

FText SFlareLeaderboardMenu::GetWindowTitle() const
{
	if (SelectedCompany && SelectedCompany != MenuManager->GetPC()->GetCompany())
	{
		return FText::Format(LOCTEXT("DiplomacyTitleOther", "Diplomacy from {0} POV"),
		SelectedCompany->GetCompanyName());
	}

	return LOCTEXT("DiplomacyTitle", "Diplomacy from player POV");
}

void SFlareLeaderboardMenu::Enter()
{
	FLOG("SFlareLeaderboardMenu::Enter");

	// Reset data
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	const TArray<UFlareCompany*>& Companies = Game->GetGameWorld()->GetCompanies();

	// Add companies
	CompanyListData.Empty();
	for (int32 Index = 0; Index < Companies.Num(); Index++)
	{
		CompanyListData.AddUnique(FInterfaceContainer::New(Companies[Index]));
	}

	// Sorting rules
	struct FSortByValue
	{
		FORCEINLINE bool operator()(const TSharedPtr<FInterfaceContainer> PtrA, const TSharedPtr<FInterfaceContainer> PtrB) const
		{
			return (PtrA->CompanyPtr->GetCompanyValue().TotalValue > PtrB->CompanyPtr->GetCompanyValue().TotalValue);
		}
	};

	SelectedCompany = MenuManager->GetPC()->GetCompany();
	// Sort
	CompanyListData.Sort(FSortByValue());
	CompanyList->RequestListRefresh();
}

void SFlareLeaderboardMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
	CompanyListData.Empty();
	CompanyList->RequestListRefresh();
	CompanyList->ClearSelection();

	SelectedCompany = NULL;
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

UFlareCompany* SFlareLeaderboardMenu::GetSelectedCompany() const
{
	return SelectedCompany;
}


TSharedRef<ITableRow> SFlareLeaderboardMenu::GenerateCompanyInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SFlareListItem, OwnerTable)
	.Content()
	[
		SNew(SFlareCompanyInfo)
		.Player(MenuManager->GetPC())
		.Company(Item->CompanyPtr)
		.Rank(CompanyListData.Find(Item) + 1)
	];
}


#undef LOCTEXT_NAMESPACE

