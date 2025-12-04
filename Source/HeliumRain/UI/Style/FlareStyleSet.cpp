
#include "FlareStyleSet.h"
#include "../../Flare.h"


/*----------------------------------------------------
	Static data
----------------------------------------------------*/

#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( FPaths::GameContentDir() / "Slate/Fonts" / RelativePath + TEXT(".ttf"), __VA_ARGS__ )

TSharedPtr<FSlateStyleSet> FFlareStyleSet::Instance = NULL;


/*----------------------------------------------------
	Initialization and removal
----------------------------------------------------*/

void FFlareStyleSet::Initialize()
{
	if (!Instance.IsValid())
	{
		Instance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*Instance);
	}
}

void FFlareStyleSet::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*Instance);
	FCHECK(Instance.IsUnique());
	Instance.Reset();
}


/*----------------------------------------------------
	GUI code
----------------------------------------------------*/

TSharedRef< FSlateStyleSet > FFlareStyleSet::Create()
{
	TSharedRef<FSlateStyleSet> StyleRef = FSlateGameResources::New(FName("FlareStyle"), "/Game/Slate", "/Game/Slate");
	FSlateStyleSet& Style = StyleRef.Get();

	Style.Set("Flare.TableRow", FTableRowStyle()
		.SetEvenRowBackgroundBrush(FSlateNoResource())
		.SetEvenRowBackgroundHoveredBrush(FSlateNoResource())
		.SetOddRowBackgroundBrush(FSlateNoResource())
		.SetOddRowBackgroundHoveredBrush(FSlateNoResource())
		.SetSelectorFocusedBrush(FSlateNoResource())
		.SetActiveBrush(FSlateNoResource())
		.SetActiveHoveredBrush(FSlateNoResource())
		.SetInactiveBrush(FSlateNoResource())
		.SetInactiveHoveredBrush(FSlateNoResource())
	);

	Style.Set("Flare.CoreButton", FButtonStyle()
		.SetNormal(FSlateNoResource())
		.SetHovered(FSlateNoResource())
		.SetPressed(FSlateNoResource())
		.SetDisabled(FSlateNoResource())
		.SetNormalPadding(FMargin(0.0f, 0.0f, 0.0f, 1.0f))
		.SetPressedPadding(FMargin(0.0f, 0.0f, 0.0f, 1.0f))
	);

	Style.Set("Flare.ActiveCoreButton", FButtonStyle()
		.SetNormal(FSlateNoResource())
		.SetHovered(FSlateNoResource())
		.SetPressed(FSlateNoResource())
		.SetDisabled(FSlateNoResource())
		.SetNormalPadding(FMargin(0.0f, 0.0f, 0.0f, 1.0f))
		.SetPressedPadding(FMargin(0.0f, 1.0f, 0.0f, 0.0f))
	);

	const FFlareStyleCatalog& DefaultTheme = Style.GetWidgetStyle<FFlareStyleCatalog>("/Style/DefaultTheme");
	Style.Set("WarningText", FTextBlockStyle(DefaultTheme.TextFont)
		.SetColorAndOpacity(DefaultTheme.EnemyColor)
	);
	Style.Set("TradeText", FTextBlockStyle(DefaultTheme.TextFont)
		.SetColorAndOpacity(DefaultTheme.TradingColor)
	);
	Style.Set("HighlightText", FTextBlockStyle(DefaultTheme.TextFont)
		.SetColorAndOpacity(DefaultTheme.FriendlyColor)
	);

	return StyleRef;
}

FLinearColor FFlareStyleSet::GetHealthColor(float Health, bool WithAlpha)
{
	const FFlareStyleCatalog& Theme = GetDefaultTheme();
	FLinearColor NormalColour = Theme.NeutralColor;
	FLinearColor MidColour = Theme.MidDamageColor;
	FLinearColor DamageColour = Theme.DamageColor;
	FLinearColor Color;

	// Interpolate the damage color
	if (Health > 0.5)
	{
		float HealthHigh = 2 * (Health - 0.5);
		Color = FMath::Lerp(MidColour, NormalColour, HealthHigh);
	}
	else
	{
		float HealthLow = 2 * Health;
		Color = FMath::Lerp(DamageColour, MidColour, HealthLow);
	}

	// Add alpha if asked for
	if (WithAlpha)
	{
		Color.A *= Theme.DefaultAlpha;
	}
	return Color;
}

FLinearColor FFlareStyleSet::GetRatioColour(float Ratio, bool WithAlpha)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor Colour;

	FLinearColor NormalColour = FLinearColor(0, 255, 0);
	FLinearColor MidColour = Theme.MidDamageColor;
	FLinearColor DamageColour = Theme.DamageColor;

	// Interpolate the color
	if (Ratio > 0.5)
	{
		float High = 2 * (Ratio - 0.5);
		Colour = FMath::Lerp(MidColour, NormalColour, High);
	}
	else
	{
		float Low = 2 * Ratio;
		Colour = FMath::Lerp(DamageColour, MidColour, Low);
	}

	// Add alpha if asked for
	if (WithAlpha)
	{
		Colour.A *= Theme.DefaultAlpha;
	}

	return Colour;
}