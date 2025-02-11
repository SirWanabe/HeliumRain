
#include "FlareTravelEventsInfo.h"
#include "../../Flare.h"
#include "../../Game/FlareGameTools.h"
#include "../../Game/FlareCompany.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Game/FlareGame.h"

#define LOCTEXT_NAMESPACE "FlareTravelEventsInfo"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareTravelEventsInfo::Construct(const FArguments& InArgs)
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

			// Events title
			+ SVerticalBox::Slot()
			.Padding(Theme.TitlePadding)
			.AutoHeight()
			[
				UFlareUITypes::Header(LOCTEXT("TravelsTitle", "Events"))
			]

			// Events list
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Left)
			.AutoHeight()
			[
				SNew(SRichTextBlock)
				.TextStyle(&Theme.TextFont)
				.Text(this, &SFlareTravelEventsInfo::GetTravelText)
				.WrapTextAt(0.5 * Theme.ContentWidth)
				.DecoratorStyleSet(&FFlareStyleSet::Get())
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

FText SFlareTravelEventsInfo::GetTravelText() const
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

#undef LOCTEXT_NAMESPACE