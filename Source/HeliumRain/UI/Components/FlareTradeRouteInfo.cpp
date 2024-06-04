
#include "FlareTradeRouteInfo.h"
#include "../../Flare.h"
#include "../../Game/FlareGameTools.h"
#include "../../Game/FlareCompany.h"
#include "../../Game/FlareTradeRoute.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Game/FlareGame.h"

#define LOCTEXT_NAMESPACE "FlareTradeRouteInfo"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareTradeRouteInfo::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	// Build structure
	ChildSlot
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Fill)
	[
		SNew(SBox)
		.WidthOverride(0.6 * Theme.ContentWidth)
		.HAlign(HAlign_Fill)
		[
			SNew(SVerticalBox)

			// Trade routes title
			+ SVerticalBox::Slot()
			.Padding(Theme.TitlePadding)
			.AutoHeight()
			[
				UFlareUITypes::Header(LOCTEXT("Trade routes", "Trade routes"))
			]

			// New trade route button
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.ContentPadding)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SFlareButton)
					.Width(9)
					.Text(LOCTEXT("NewTradeRouteButton", "Add new trade route"))
					.HelpText(LOCTEXT("NewTradeRouteInfo", "Create a new trade route and edit it. You need an available fleet to create a new trade route."))
					.Icon(FFlareStyleSet::GetIcon("New"))
					.OnClicked(this, &SFlareTradeRouteInfo::OnNewTradeRouteClicked)
					.IsDisabled(this, &SFlareTradeRouteInfo::IsNewTradeRouteDisabled)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.ContentPadding)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SFlareButton)
					.Width(4.5)
					.Text(LOCTEXT("PauseTradeRouteButton", "Pause trade routes"))
					.HelpText(LOCTEXT("PauseTradeRouteInfo", "Pause all currently active trade routes"))
					.Icon(FFlareStyleSet::GetIcon("Pause"))
					.OnClicked(this, &SFlareTradeRouteInfo::OnPauseTradeRoutes)
					.IsDisabled(this, &SFlareTradeRouteInfo::IsPauseTradeRoutesDisabled)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SFlareButton)
					.Width(4.5)
					.Text(LOCTEXT("UnpauseTradeRouteButton", "Unpause trade routes"))
					.HelpText(LOCTEXT("UnpauseTradeRouteInfo", "Unpause all currently inactive trade routes"))
					.Icon(FFlareStyleSet::GetIcon("Load"))
					.OnClicked(this, &SFlareTradeRouteInfo::OnUnpauseTradeRoutes)
					.IsDisabled(this, &SFlareTradeRouteInfo::IsPauseTradeRoutesDisabled)
				]
			]

			// Trade route list
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Left)
			.AutoHeight()
			[
				SAssignNew(TradeRouteList, SVerticalBox)
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareTradeRouteInfo::Update()
{
	Clear();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// List trade routes
	for (UFlareTradeRoute* TradeRoute : MenuManager->GetPC()->GetCompany()->GetCompanyTradeRoutes())
	{
		TradeRouteList->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.Padding(Theme.ContentPadding)
		[
			SNew(SVerticalBox)

			// Buttons
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// Inspect
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SFlareButton)
					.Width(7)
					.Text(this, &SFlareTradeRouteInfo::GetTradeRouteName, TradeRoute)
					.HelpText(FText(LOCTEXT("InspectHelp", "Edit this trade route")))
					.OnClicked(this, &SFlareTradeRouteInfo::OnInspectTradeRouteClicked, TradeRoute)
				]

				// Pause
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SFlareButton)
					.Transparent(true)
					.Text(FText())
					.HelpText(LOCTEXT("PauseTradeRouteHelp", "Pause or restart this trade route"))
					.Icon(this, &SFlareTradeRouteInfo::GetTogglePauseTradeRouteIcon, TradeRoute)
					.OnClicked(this, &SFlareTradeRouteInfo::OnTogglePauseTradeRoute, TradeRoute)
					.Width(1)
				]

				// Remove
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SFlareButton)
					.Transparent(true)
					.Text(FText())
					.HelpText(LOCTEXT("RemoveTradeRouteHelp", "Remove this trade route"))
					.Icon(FFlareStyleSet::GetIcon("Stop"))
					.OnClicked(this, &SFlareTradeRouteInfo::OnDeleteTradeRoute, TradeRoute)
					.Width(1)
				]
			]

			// Infos
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(Theme.SmallContentPadding)
			[
				SNew(SRichTextBlock)
				.TextStyle(&Theme.TextFont)
				.Text(this, &SFlareTradeRouteInfo::GetDetailText, TradeRoute)
				.WrapTextAt(Theme.ContentWidth / 2)
				.DecoratorStyleSet(&FFlareStyleSet::Get())
			]
		];
	}
}

void SFlareTradeRouteInfo::Clear()
{
	TradeRouteList->ClearChildren();
}

FText SFlareTradeRouteInfo::GetDetailText(UFlareTradeRoute* TradeRoute) const
{
	FCHECK(TradeRoute);
	const FFlareTradeRouteSave* TradeRouteData = TradeRoute->Save();
	FCHECK(TradeRouteData);

	// Get data
	int32 TotalOperations = TradeRouteData->StatsOperationSuccessCount + TradeRouteData->StatsOperationFailCount;
	int32 SuccessPercentage = (TotalOperations > 0) ? (FMath::RoundToInt(100.0f * TradeRouteData->StatsOperationSuccessCount / float(TotalOperations))) : 0;
	int32 CreditsGain = (TradeRouteData->StatsDays > 0) ? (FMath::RoundToInt(0.01f * float(TradeRouteData->StatsMoneySell - TradeRouteData->StatsMoneyBuy) / float(TradeRouteData->StatsDays))) : 0;

	// Format result
	if (CreditsGain > 0)
	{
		return UFlareGameTools::AddLeadingSpace(FText::Format(LOCTEXT("TradeRouteDetailsGain", "<TradeText>{0} credits per day, {1}% OK</>"),
			FText::AsNumber(CreditsGain),
			FText::AsNumber(SuccessPercentage)),
			3);
	}
	else
	{
		return UFlareGameTools::AddLeadingSpace(FText::Format(LOCTEXT("TradeRouteDetailsLoss", "<WarningText>{0} credits per day, {1}% OK</>"),
			FText::AsNumber(CreditsGain),
			FText::AsNumber(SuccessPercentage)),
			3);
	}
}


bool SFlareTradeRouteInfo::IsPauseTradeRoutesDisabled() const
{
	int32 TradeRouteCount = MenuManager->GetPC()->GetCompany()->GetCompanyTradeRoutes().Num();
	return (TradeRouteCount == 0);
}

bool SFlareTradeRouteInfo::IsNewTradeRouteDisabled() const
{
	int32 FleetCount = 0;
	TArray<UFlareFleet*>& Fleets = MenuManager->GetGame()->GetPC()->GetCompany()->GetCompanyFleets();

	for (int FleetIndex = 0; FleetIndex < Fleets.Num(); FleetIndex++)
	{
		if (!Fleets[FleetIndex]->GetCurrentTradeRoute() && Fleets[FleetIndex] != MenuManager->GetPC()->GetPlayerFleet())
		{
			FleetCount++;
		}
	}

	return (FleetCount == 0);
}

void SFlareTradeRouteInfo::OnNewTradeRouteClicked()
{
	UFlareTradeRoute* TradeRoute = MenuManager->GetPC()->GetCompany()->CreateTradeRoute(LOCTEXT("UntitledRoute", "Untitled Route"));
	FCHECK(TradeRoute);

	FFlareMenuParameterData Data;
	Data.Route = TradeRoute;
	MenuManager->OpenMenu(EFlareMenu::MENU_TradeRoute, Data);
}

void SFlareTradeRouteInfo::OnInspectTradeRouteClicked(UFlareTradeRoute* TradeRoute)
{
	FFlareMenuParameterData Data;
	Data.Route = TradeRoute;
	MenuManager->OpenMenu(EFlareMenu::MENU_TradeRoute, Data);
}

void SFlareTradeRouteInfo::OnDeleteTradeRoute(UFlareTradeRoute* TradeRoute)
{
	MenuManager->Confirm(LOCTEXT("AreYouSure", "ARE YOU SURE ?"),
		LOCTEXT("ConfirmDeleteTR", "Do you really want to remove this trade route ?"),
		FSimpleDelegate::CreateSP(this, &SFlareTradeRouteInfo::OnDeleteTradeRouteConfirmed, TradeRoute));
}

void SFlareTradeRouteInfo::OnDeleteTradeRouteConfirmed(UFlareTradeRoute* TradeRoute)
{
	FCHECK(TradeRoute);
	TradeRoute->Dissolve();
	Update();
	MenuManager->UpdateOrbitMenuFleets();
}

void SFlareTradeRouteInfo::OnPauseTradeRoutes()
{
	MenuManager->Confirm(LOCTEXT("AreYouSure", "ARE YOU SURE ?"),
		LOCTEXT("ConfirmPauseTRS", "Do you really want to pause all trade routes ?"),
		FSimpleDelegate::CreateSP(this, &SFlareTradeRouteInfo::OnPauseTradeRoutesConfirmed));
}

void SFlareTradeRouteInfo::OnPauseTradeRoutesConfirmed()
{
	for (UFlareTradeRoute* TradeRoute : MenuManager->GetPC()->GetCompany()->GetCompanyTradeRoutes())
	{
		TradeRoute->SetPaused(true);
	}
	MenuManager->UpdateOrbitMenuFleets();
}

void SFlareTradeRouteInfo::OnUnpauseTradeRoutes()
{
	MenuManager->Confirm(LOCTEXT("AreYouSure", "ARE YOU SURE ?"),
		LOCTEXT("ConfirmUnpauseTRS", "Do you really want to unpause all trade routes ?"),
		FSimpleDelegate::CreateSP(this, &SFlareTradeRouteInfo::OnUnpauseTradeRoutesConfirmed));
}

void SFlareTradeRouteInfo::OnUnpauseTradeRoutesConfirmed()
{
	for (UFlareTradeRoute* TradeRoute : MenuManager->GetPC()->GetCompany()->GetCompanyTradeRoutes())
	{
		TradeRoute->SetPaused(false);
	}
	MenuManager->UpdateOrbitMenuFleets();
}

FText SFlareTradeRouteInfo::GetTradeRouteName(UFlareTradeRoute* TradeRoute) const
{
	return FText::Format(LOCTEXT("TradeRouteNameFormat", "{0}{1}"),
		TradeRoute->GetTradeRouteName(),
		(TradeRoute->IsPaused() ? UFlareGameTools::AddLeadingSpace(LOCTEXT("FleetTradeRoutePausedFormat", "(Paused)")) : FText()));
}

const FSlateBrush* SFlareTradeRouteInfo::GetTogglePauseTradeRouteIcon(UFlareTradeRoute* TradeRoute) const
{
	if (TradeRoute->IsPaused())
	{
		return FFlareStyleSet::GetIcon("Load");
	}
	else
	{
		return FFlareStyleSet::GetIcon("Pause");
	}
}

void SFlareTradeRouteInfo::OnTogglePauseTradeRoute(UFlareTradeRoute* TradeRoute)
{
	TradeRoute->SetPaused(!TradeRoute->IsPaused());
	MenuManager->UpdateOrbitMenuFleets();
}

#undef LOCTEXT_NAMESPACE