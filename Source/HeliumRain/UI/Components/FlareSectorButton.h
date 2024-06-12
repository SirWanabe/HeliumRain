#pragma once

#include "../../Flare.h"
#include "../FlareUITypes.h"
#include "FlareButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"

class UFlareCompany;
class UFlareSimulatedSector;

class SFlareSectorButton : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareSectorButton)
		: _Sector(NULL)
		, _PlayerCompany(NULL)
	{}

	SLATE_ARGUMENT(UFlareSimulatedSector*, Sector)
	SLATE_ARGUMENT(UFlareCompany*, PlayerCompany)
	SLATE_EVENT(FFlareButtonClicked, OnClicked)
			
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	TSharedPtr<SHorizontalBox> GetCurrentBox();
	UFlareSimulatedSector* GetSector();

protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Mouse entered (tooltip) */
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/** Mouse left (tooltip) */
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	/** Should we display fleets */
	bool ShouldDisplayFleets() const;

	/** Get the visibility of text */
	EVisibility GetBottomTextVisibility() const;

	/** Get the text to display */
	FText GetSectorText() const;

	/** Brush callback */
	const FSlateBrush* GetBackgroundBrush() const;

	/** Get the main color */
	FSlateColor GetMainColor() const;

	/** Get the color to use for the border */
	FSlateColor GetBorderColor() const;

	/** Get the shadow color */
	FLinearColor GetShadowColor() const;

	/** Mouse clicked */
	FReply OnButtonClicked();

protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/
	
	// Data
	FFlareButtonClicked            OnClicked;
	UFlareSimulatedSector*         Sector;
	UFlareCompany*                 PlayerCompany;

	// Slate data
	TSharedPtr<STextBlock>			 SectorTitle;
	TSharedPtr<STextBlock>			 SectorText;
	TSharedPtr<SBorder>			     BackgroundBorder;
	TSharedPtr<SImage>			     ButtonImage;
	TSharedPtr<SHorizontalBox>       CurrentBox;
	TSharedPtr<SHorizontalBox>       FleetBoxOne;
	TSharedPtr<SHorizontalBox>       FleetBoxTwo;
	TSharedPtr<SHorizontalBox>       FleetBoxThree;
	int32							 NeutralShips;
	int32							 NeutralStations;
	int32							 OwnedShips;
	int32							 OwnedStations;
	int32							 EnemyShips;
	int32							 EnemyStations;
	int32							 CurrentCount;
	int32							 TotalCount;
	TArray<UFlareFleet*>			 TrackedFleets;
	TArray<UFlareFleet*>			 TrackedTravellingFleets;
	TArray<TSharedPtr<SVerticalBox>> TrackedFleetBoxes;
	TArray<TSharedPtr<STextBlock>>   TrackedFleetDateBoxes;

	

public:
	void	RefreshButton();
	void	UpdateBackgroundImages();
};
