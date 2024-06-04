
#include "FlareWhiteListMenu.h"
#include "../../Flare.h"

#include "../../Data/FlareResourceCatalog.h"

#include "../../Game/FlareTradeRoute.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareGameTools.h"
#include "../../Game/FlareTradeRoute.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#include "../Components/FlareButton.h"
#include "../Components/FlareSectorButton.h"


#define LOCTEXT_NAMESPACE "SFlareWhiteListMenu"
#define MAX_WAIT_LIMIT 29
#define MINIBAR_WIDTH_MULTI 0.35

#define CONDITIONS_HEIGHT_BOOST 16


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareWhiteListMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	AFlarePlayerController* PC = MenuManager->GetPC();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	int32 LabelWidth = 0.25 * Theme.ContentWidth;

	// Build structure
	ChildSlot
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SScrollBox)
		.Style(&Theme.ScrollBoxStyle)
		.ScrollBarStyle(&Theme.ScrollBarStyle)

		+ SScrollBox::Slot()
		.Padding(Theme.ContentPadding)
		[
			SNew(SBox)
			.WidthOverride(2.8 * Theme.ContentWidth)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					.Padding(Theme.ContentPadding)
					[
						SAssignNew(EditWhiteListName, SEditableText)
						.AllowContextMenu(false)
						.Style(&Theme.TextInputStyle)
					]
					// Rename Confirmation
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					.HAlign(HAlign_Right)
					[
						SNew(SFlareButton)
						.Width(4)
						.Text(LOCTEXT("Rename", "Rename"))
						.HelpText(LOCTEXT("ChangeNameInfo", "Rename this company white list"))
						.Icon(FFlareStyleSet::GetIcon("OK"))
						.OnClicked(this, &SFlareWhiteListMenu::OnConfirmChangeWhiteListNameClicked)
						.IsDisabled(this, &SFlareWhiteListMenu::IsRenameDisabled)
					]
				]

				//Company Selection
				+ SVerticalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					.Padding(Theme.ContentPadding)
					[
						SAssignNew(ViewAllCompaniesToggle,SFlareButton)
						.Width(4)
						.Toggle(true)
						.Text(LOCTEXT("ViewAllCompanies", "View All Companies"))
						.HelpText(LOCTEXT("ViewAllCompaniesInfo", "Enable to view settings for all companies simultaneously"))
						.OnClicked(this, &SFlareWhiteListMenu::OnViewCompaniesToggle)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					.HAlign(HAlign_Right)
					[
						SNew(SFlareButton)
						.Width(4)
						.Text(LOCTEXT("SetDefaultWhiteList", "Set to Default"))
						.HelpText(LOCTEXT("SetDefaultWhiteListInfo", "Set this Whitelist as the Company-wide default."))
						.OnClicked(this, &SFlareWhiteListMenu::OnSetCompanyDefaultWhitelist)
						.IsDisabled(this, &SFlareWhiteListMenu::IsSetDefaultDisabled)
					]
				]
				+ SVerticalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SAssignNew(CompanySelectionSBox,SBox)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Top)
							.HAlign(HAlign_Right)
							.Padding(Theme.ContentPadding)
							.AutoWidth()
							[
								SNew(SBox)
								.WidthOverride(Theme.ContentWidth / 2)
								.Padding(FMargin(0))
								.HAlign(HAlign_Left)
								[
									SAssignNew(CompanySelector, SFlareDropList<UFlareCompany*>)
									.OptionsSource(&CompanyList)
									.OnGenerateWidget(this, &SFlareWhiteListMenu::OnGenerateCompanyComboLine)
									.OnSelectionChanged(this, &SFlareWhiteListMenu::OnCompanyComboLineSelectionChanged)
									.HeaderWidth(6)
									.ItemWidth(6)
								]
							]
						]
					]
				]
				+ SVerticalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				.AutoHeight()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SAssignNew(CompanyWhiteListInfoBox, SVerticalBox)
				]
			]
		]
	];
	ViewAllCompaniesToggle->SetActive(true);
}

TSharedRef<SWidget> SFlareWhiteListMenu::OnGenerateCompanyComboLine(UFlareCompany* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	return SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(STextBlock)
			.Text(Item->GetCompanyName())
			.TextStyle(&Theme.TextFont)
	];
}

/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareWhiteListMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareWhiteListMenu::Enter(UFlareCompanyWhiteList* WhiteList, UFlareCompany* Company)
{
	FLOG("SFlareWhiteListMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	TargetWhiteList = WhiteList;
	TargetCompany = Company;
	EditWhiteListName->SetText(TargetWhiteList->GetWhiteListName());

	if(CompanyList.Num() < 1)
	{
		CompanyList.Empty();
		for (int CompanyIndex = 0; CompanyIndex < MenuManager->GetPC()->GetGame()->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
		{
			UFlareCompany* FoundCompany = MenuManager->GetPC()->GetGame()->GetGameWorld()->GetCompanies()[CompanyIndex];
			CompanyList.Add(FoundCompany);
		}

		CompanySelector->RefreshOptions();
		CompanySelector->SetSelectedItem(MenuManager->GetPC()->GetGame()->GetGameWorld()->GetCompanies()[0]);
	}

	OnViewCompaniesToggle();
}

void SFlareWhiteListMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareWhiteListMenu::OnCompanyComboLineSelectionChanged(UFlareCompany* Item, ESelectInfo::Type SelectInfo)
{
	if (Item)
	{
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
		CompanyWhiteListInfoBox->ClearChildren();
		TSharedPtr<SHorizontalBox> CurrentHorizontalBox;
		CompanyWhiteListInfoBox->AddSlot()
		.AutoHeight()
		.Padding(FMargin(0))
		[
			SAssignNew(CurrentHorizontalBox, SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Right)
			.Padding(Theme.ListContentPadding)
			.AutoWidth()
		];
		GenerateWhiteListInfoBoxFor(Item, CurrentHorizontalBox);
	}
}

void SFlareWhiteListMenu::OnSetCompanyDefaultWhitelist()
{
	TargetCompany->SelectWhiteListDefault(TargetWhiteList);
}

void SFlareWhiteListMenu::OnConfirmChangeWhiteListNameClicked()
{
	if (TargetWhiteList)
	{
		TargetWhiteList->SetWhiteListName(EditWhiteListName->GetText());
	}
}

void SFlareWhiteListMenu::GenerateWhiteListInfoBox()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	CompanyWhiteListInfoBox->ClearChildren();

	TSharedPtr<SHorizontalBox> CurrentHorizontalBox;
	CompanyWhiteListInfoBox->AddSlot()
	.AutoHeight()
	.Padding(FMargin(0))
	[
		SAssignNew(CurrentHorizontalBox, SHorizontalBox)
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Top)
		.Padding(Theme.TitlePadding)
		.AutoWidth()
	];

	int SuccessfulCounts = 0;
	for (int CompanyIndex = 0; CompanyIndex < MenuManager->GetPC()->GetGame()->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
		UFlareCompany* Company = MenuManager->GetPC()->GetGame()->GetGameWorld()->GetCompanies()[CompanyIndex];
		if (GenerateWhiteListInfoBoxFor(Company, CurrentHorizontalBox))
		{
			SuccessfulCounts++;
		}

		if (SuccessfulCounts >= 3)
		{
			CompanyWhiteListInfoBox->AddSlot()
			.AutoHeight()
			.Padding(FMargin(0))
			[
				SAssignNew(CurrentHorizontalBox, SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Top)
				.Padding(Theme.TitlePadding)
				.AutoWidth()
			];
			SuccessfulCounts = 0;
		}
	}
}

bool SFlareWhiteListMenu::GenerateWhiteListInfoBoxFor(UFlareCompany* Company, TSharedPtr<SHorizontalBox> CurrentHorizontalBox)
{
	if (!Company || !TargetWhiteList)
	{
		return false;
	}

	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	const FSlateBrush* CompanyEmblem = Company->GetEmblem();
	FFlareWhiteListCompanyDataSave* CompanyData = TargetWhiteList->GetCompanyDataFor(Company->GetIdentifier());

	TSharedPtr<SFlareButton> ToggleTradeTowards;
	TSharedPtr<SFlareButton> ToggleTradeFrom;

	TSharedPtr<SVerticalBox> TradeTowardsResourceBox;
	TSharedPtr<SVerticalBox> TradeFromResourceBox;

	CurrentHorizontalBox->AddSlot()
	.AutoWidth()
	.Padding(FMargin(0))
	[
		SNew(SBox)
		.Padding(FMargin(50,10,50,10))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Left)
			.MaxHeight(140)
			[
				SNew(SHorizontalBox)
				// Icon
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Top)
				.Padding(Theme.ContentPadding)
				.AutoWidth()
				[
					SNew(SImage)
					.Image(CompanyEmblem)
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Left)
				.Padding(Theme.SmallContentPadding)
				.MaxWidth(512)
				[
					SNew(SVerticalBox)
					//Company Name
					+ SVerticalBox::Slot()
					.VAlign(VAlign_Top)
					.HAlign(HAlign_Left)
					.Padding(Theme.ContentPadding)
					.AutoHeight()
					[
						SNew(STextBlock)
						.TextStyle(&Theme.NameFont)
						.WrapTextAt(Theme.ContentWidth * 0.60)
						.Text(Company->GetDescription()->Name)
					]
					//Company Description
					+ SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Right)
					.Padding(Theme.ContentPadding)
					.MaxHeight(72)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.WrapTextAt(Theme.ContentWidth * 0.50)
						.Text(Company->GetDescription()->Description)
					]
				]
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Top)
				.Padding(Theme.ContentPadding)
				.AutoWidth()
				[
					SNew(SBorder)
					.Padding(Theme.SmallContentPadding)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Left)
						.Padding(Theme.SmallContentPadding)
						.AutoHeight()
						[
							SAssignNew(ToggleTradeTowards, SFlareButton)
							.Width(5)
							.Toggle(true)
							.Text(LOCTEXT("ToggleTradeTo", "Trade Towards"))
							.HelpText(LOCTEXT("TradeToInfo", "Enable to allow your assets to trade resources to this faction (IE: Sell to)"))
							.OnClicked(this, &SFlareWhiteListMenu::OnToggleTradeToCompany, Company, CompanyData)
						]
						+ SVerticalBox::Slot()
						.Padding(Theme.SmallContentPadding)
						.AutoHeight()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Left)
						[
							SAssignNew(TradeTowardsResourceBox, SVerticalBox)
							.Visibility(this, &SFlareWhiteListMenu::GetResourceVisibility, CompanyData, true)
						]
					]
				]

				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Top)
				.Padding(Theme.ContentPadding)
				.AutoWidth()
				[
					SNew(SBorder)
					.Padding(Theme.SmallContentPadding)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Left)
						.Padding(Theme.SmallContentPadding)
						.AutoHeight()
						[
							SAssignNew(ToggleTradeFrom, SFlareButton)
							.Width(5)
							.Toggle(true)
							.Text(LOCTEXT("ToggleTradeFrom", "Trade From"))
							.HelpText(LOCTEXT("TradeFromInfo", "Enable to allow your assets to trade resources from this faction (IE: Buy from)"))
							.OnClicked(this, &SFlareWhiteListMenu::OnToggleTradeFromCompany, Company, CompanyData)
						]
						+ SVerticalBox::Slot()
						.Padding(Theme.SmallContentPadding)
						.AutoHeight()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Left)
						[
							SAssignNew(TradeFromResourceBox, SVerticalBox)
							.Visibility(this, &SFlareWhiteListMenu::GetResourceVisibility, CompanyData, false)
						]
					]
				]
			]
		]
	];

	ToggleTradeTowards->SetActive(CompanyData->CanTradeTo);
	ToggleTradeFrom->SetActive(CompanyData->CanTradeFrom);

	GenerateAndDisplayResources(TradeTowardsResourceBox, Company, CompanyData, true);
	GenerateAndDisplayResources(TradeFromResourceBox, Company, CompanyData, false);
	return true;
}

/*----------------------------------------------------
	Content callbacks
----------------------------------------------------*/
void SFlareWhiteListMenu::GenerateAndDisplayResources(TSharedPtr<SVerticalBox> ResourceBox, UFlareCompany* Company, FFlareWhiteListCompanyDataSave* CompanyData, bool TowardsOrFrom)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	TArray<UFlareResourceCatalogEntry*> ResourceList = Company->GetGame()->GetResourceCatalog()->Resources;
	int CurrentCount = 0;
	TSharedPtr<SHorizontalBox> CurrentHorizontalBox;

	ResourceBox->ClearChildren();
	ResourceBox->AddSlot()
	.AutoHeight()
	.Padding(FMargin(0))
	[
		SAssignNew(CurrentHorizontalBox, SHorizontalBox)
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Right)
		.Padding(Theme.ListContentPadding)
		.AutoWidth()
	];

	for (int ResourceIndex = 0; ResourceIndex < ResourceList.Num(); ResourceIndex++)
	{
		UFlareResourceCatalogEntry* Resource = ResourceList[ResourceIndex];
		if (Resource)
		{
			FFlareResourceDescription* Data = &Resource->Data;
			if (Data)
			{
				if (CurrentCount >= 3)
				{
					CurrentCount = 0;
					ResourceBox->AddSlot()
					.AutoHeight()
					.Padding(FMargin(0))
					[
						SAssignNew(CurrentHorizontalBox, SHorizontalBox)
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Right)
						.Padding(Theme.ListContentPadding)
						.AutoWidth()
					];
				}

				TSharedPtr<SImage> ResourceEnabledImage;

				CurrentHorizontalBox->AddSlot()
				.AutoWidth()
				.Padding(FMargin(1,1))
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					// Button (behaviour only, no display)
					SNew(SButton)
					.OnClicked(this, &SFlareWhiteListMenu::OnToggleResourceWhiteList,Data->Identifier, CompanyData, TowardsOrFrom, ResourceEnabledImage)
					.ContentPadding(FMargin(0))
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					[
						SNew(SBorder)
						.Padding(FMargin(0))
						.BorderImage(&Data->Icon)
						[
							SNew(SBox)
							.WidthOverride(Theme.ResourceWidth)
							.HeightOverride(Theme.ResourceHeight)
							.Padding(FMargin(0))
							[
								SNew(SVerticalBox)
								// Resource name
								+ SVerticalBox::Slot()
								.Padding(Theme.SmallContentPadding)
								.VAlign(VAlign_Top)
								.HAlign(HAlign_Left)
								[
									SNew(STextBlock)
									.TextStyle(&Theme.TextFont)
									.Text(Data->Acronym)
								]

								// Enabled / Disabled display
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(FMargin(3.f, 0.5f, 3.f, 3.f))
								.VAlign(VAlign_Bottom)
								.HAlign(HAlign_Right)
								[
									SAssignNew(ResourceEnabledImage,SImage)
								]
							]
						]
					]
				];
				CurrentCount++;
				if (TowardsOrFrom)
				{
					if (CompanyData->ResourcesTradeTo.Find(Data->Identifier) == INDEX_NONE)
					{
						ResourceEnabledImage->SetImage(ResourceDisabledIcon);
					}
					else
					{
						ResourceEnabledImage->SetImage(ResourceOKIcon);
					}
				}
				else
				{
					if (CompanyData->ResourcesTradeFrom.Find(Data->Identifier) == INDEX_NONE)
					{
						ResourceEnabledImage->SetImage(ResourceDisabledIcon);
					}
					else
					{
						ResourceEnabledImage->SetImage(ResourceOKIcon);
					}
				}
			}
		}
	}
}

void SFlareWhiteListMenu::UpdateVisibilities()
{
	CompanySelectionSBox->SetVisibility(GetFactionSelectorVisibility());
}

EVisibility SFlareWhiteListMenu::GetFactionSelectorVisibility() const
{
	if (!ViewAllCompaniesToggle->IsActive())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

bool SFlareWhiteListMenu::IsRenameDisabled() const
{
	if (TargetWhiteList && TargetWhiteList->GetWhiteListName().ToString() != EditWhiteListName->GetText().ToString())
	{
		FString NameData = EditWhiteListName->GetText().ToString();
		if (NameData.Len() > 23)
		{
			return true;
		}
		return false;
	}
	return true;
}

bool SFlareWhiteListMenu::IsSetDefaultDisabled() const
{
	if (TargetCompany->GetCompanySelectedWhiteList() != TargetWhiteList)
	{
		return false;
	}
	return true;
}


void SFlareWhiteListMenu::OnViewCompaniesToggle()
{
	UpdateVisibilities();
	if (ViewAllCompaniesToggle->IsActive())
	{
		GenerateWhiteListInfoBox();
	}
	else
	{
		OnCompanyComboLineSelectionChanged(CompanySelector->GetSelectedItem(), ESelectInfo::Direct);
	}
}

FReply SFlareWhiteListMenu::OnToggleResourceWhiteList(FName Identifier, FFlareWhiteListCompanyDataSave* CompanyData, bool TowardsOrFrom, TSharedPtr<SImage> ResourceEnabledImage)
{
	if (TowardsOrFrom)
	{
		if (CompanyData->ResourcesTradeTo.Find(Identifier) == INDEX_NONE)
		{
			CompanyData->ResourcesTradeTo.AddUnique(Identifier);
			ResourceEnabledImage->SetImage(ResourceOKIcon);
		}
		else
		{
			CompanyData->ResourcesTradeTo.Remove(Identifier);
			ResourceEnabledImage->SetImage(ResourceDisabledIcon);
		}
	}
	else
	{
		if (CompanyData->ResourcesTradeFrom.Find(Identifier) == INDEX_NONE)
		{
			CompanyData->ResourcesTradeFrom.AddUnique(Identifier);
			ResourceEnabledImage->SetImage(ResourceOKIcon);
		}
		else
		{
			CompanyData->ResourcesTradeFrom.Remove(Identifier);
			ResourceEnabledImage->SetImage(ResourceDisabledIcon);
		}
	}

	return FReply::Handled();
}

EVisibility SFlareWhiteListMenu::GetResourceVisibility(FFlareWhiteListCompanyDataSave* CompanyData, bool TowardsOrFrom) const
{
	if (TowardsOrFrom)
	{
		if (CompanyData->CanTradeTo)
		{
			return EVisibility::Visible;
		}
	}
	else
	{
		if (CompanyData->CanTradeFrom)
		{
			return EVisibility::Visible;
		}
	}
	return EVisibility::Collapsed;
}

void SFlareWhiteListMenu::OnToggleTradeToCompany(UFlareCompany* Company, FFlareWhiteListCompanyDataSave* CompanyData)
{
	if (CompanyData->CanTradeTo)
	{
		TargetWhiteList->SetWhiteListTradeTo(CompanyData,false);
	}
	else
	{
		TargetWhiteList->SetWhiteListTradeTo(CompanyData,true);
	}
}

void SFlareWhiteListMenu::OnToggleTradeFromCompany(UFlareCompany* Company, FFlareWhiteListCompanyDataSave* CompanyData)
{
	if (CompanyData->CanTradeFrom)
	{
		TargetWhiteList->SetWhiteListTradeFrom(CompanyData,false);
	}
	else
	{
		TargetWhiteList->SetWhiteListTradeFrom(CompanyData,true);
	}
}
/*----------------------------------------------------
	Action callbacks
----------------------------------------------------*/

#undef LOCTEXT_NAMESPACE