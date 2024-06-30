
#include "FlareOrbitalFleetInfo.h"
#include "../../Flare.h"
#include "../../Game/FlareGameTools.h"
#include "../../Game/FlareCompany.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Game/FlareGame.h"

#define LOCTEXT_NAMESPACE "FlareOrbitalFleetInfo"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareOrbitalFleetInfo::Construct(const FArguments& InArgs)
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

			// Automated fleets title
			+ SVerticalBox::Slot()
			.Padding(Theme.TitlePadding)
			.AutoHeight()
			[
				UFlareUITypes::Header(LOCTEXT("OrbitalFleets", "Fleets"))
			]

			// Fleet list
			+ SVerticalBox::Slot()
			[
				SAssignNew(FleetList, SFlareList)
				.MenuManager(MenuManager)
				.FleetList(true)
				.UseCompactDisplay(true)
				.UseSmallFont(true)
				.OriginatingMenu(FString("Orbital"))
				.ArrayMode(ESelectionMode::SingleToggle)
				.OnItemSelected(this, &SFlareOrbitalFleetInfo::OnFleetSelected)
				.OnItemUnSelected(this, &SFlareOrbitalFleetInfo::OnFleetUnSelected)
			]
		]
	];
	FleetList->SetTitle(LOCTEXT("OrbitalFleetsTitle", ""));
}

/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareOrbitalFleetInfo::Clear()
{
	FleetList->Reset();
	SelectedFleet = nullptr;
}

void SFlareOrbitalFleetInfo::Update()
{
	FleetList->Reset();
	FleetList->SetVisibility(EVisibility::Visible);

	int32 FleetCount = MenuManager->GetPC()->GetCompany()->GetCompanyFleets().Num();
	FLOGV("SFlareOrbitalFleetInfo::Update : found %d fleets", FleetCount);

	for (int32 FleetIndex = 0; FleetIndex < FleetCount; FleetIndex++)
	{
		UFlareFleet* Fleet = MenuManager->GetPC()->GetCompany()->GetCompanyFleets()[FleetIndex];
		if (Fleet->IsVisibleForOrbitalFleetList())
		{
			FleetList->AddFleet(Fleet);
		}
	}
	FleetList->RefreshList();
}

void SFlareOrbitalFleetInfo::OnFleetUnSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer)
{
	SelectedFleet = nullptr;
}

void SFlareOrbitalFleetInfo::OnFleetSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer)
{
	UFlareFleet* Fleet = SpacecraftContainer->FleetPtr;
	if (Fleet)
	{
		SelectedFleet = Fleet;
	}
}

UFlareFleet* SFlareOrbitalFleetInfo::GetSelectedFleet()
{
	return SelectedFleet;
}

#undef LOCTEXT_NAMESPACE