#pragma once

#include "../../Flare.h"
#include "../Components/FlarePlanetaryBox.h"
#include "../../Game/FlareSimulatedSector.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareSectorButton.h"
#include "../Components/FlareTradeRouteInfo.h"

class AFlareMenuManager;


UENUM()
namespace EFlareOrbitalMode
{
	enum Type
	{
		Fleets,
		Stations,
		Ships,
		Battles
	};
}

class SFlareOrbitalMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareOrbitalMenu){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Setup the widget */
	void Setup();

	/** Enter this menu */
	void Enter();

	/** Exit this menu */
	void Exit();

	/** Stop fast forward */
	void StopFastForward();

	/** A notification was received, stop */
	void RequestStopFastForward();

	/** Get the display mode */
	EFlareOrbitalMode::Type GetDisplayMode() const;

	float GetTimeSinceFFWD() const;
	bool GetFastForwardActive() const;

	/** Is this mode the current one */
	bool IsCurrentDisplayMode(EFlareOrbitalMode::Type Mode) const;

	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;

	/** Set Shipyard Open var */
	void SetShipyardOpen(bool ShipyardOpen);

	void ClearSectorButtonCaches();

protected:

	/*----------------------------------------------------
		Drawing
	----------------------------------------------------*/
	
	/** Update the map with new data from the planetarium */
	void UpdateMap();

	/** Update the map for a specific celestial body */
	void UpdateMapForBody(TSharedPtr<SFlarePlanetaryBox> Map, const FFlareSectorCelestialBodyDescription* Body);
		

	/*----------------------------------------------------
		Content callbacks
	----------------------------------------------------*/

	/** Get the text for the fast-forward feature */
	FText GetFastForwardText() const;

	/** Get the icon for the fast forward button */
	const FSlateBrush* GetFastForwardIcon() const;

	/** Visibility setting for the fast-forward feature */
	bool IsFastForwardDisabled() const;

	/** Get the current date */
	FText GetDateText() const;

	/** Get the travel text */
	FText GetTravelText() const;

	/** Get a widget's position on the screen */
//	FVector2D GetWidgetPosition(int32 Index) const;

	/** Get a widget's size on the screen */
	FVector2D GetWidgetSize(int32 Index) const;

/*----------------------------------------------------
		Action callbacks
	----------------------------------------------------*/

	/** Set the display mode */
	void SetDisplayMode(EFlareOrbitalMode::Type Mode);

	/** Shipyard Menu button */
	void OnShipyard();

	/** Open a sector */
	void OnOpenSector(TSharedPtr<int32> Index);
	
	/** Check if we can fast forward */
	void OnFastForwardClicked();

	/** Check if we can fast forward */
	void OnFastForwardAutomaticClicked();

	/** Fast forward to the next event */
	void OnFastForwardConfirmed(bool Automatic);

	/** Cancel fast forward */
	void OnFastForwardCanceled();


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	// Game data
	AFlareGame*                                 Game;
	TWeakObjectPtr<class AFlareMenuManager>     MenuManager;

	bool									    ShipyardMenuOpen;

	// Fast forward
	bool                                        FastForwardActive;
	bool                                        FastForwardStopRequested;
	float                                       FastForwardPeriod;
	float                                       TimeSinceFastForward;

	TEnumAsByte<EFlareOrbitalMode::Type>        DisplayMode;

	// Components
	TSharedPtr<SFlarePlanetaryBox>              NemaBox;
	TSharedPtr<SFlarePlanetaryBox>              AstaBox;
	TSharedPtr<SFlarePlanetaryBox>              AnkaBox;
	TSharedPtr<SFlarePlanetaryBox>              HelaBox;
	TSharedPtr<SFlarePlanetaryBox>              AdenaBox;
	TSharedPtr<SFlareButton>                    FastForwardAuto;
	TSharedPtr<SFlareTradeRouteInfo>            TradeRouteInfo;

	TSharedPtr<SFlareSectorButton>              CurrentSectorButton;
	TArray<TSharedPtr<SFlareSectorButton>>		SectorButtons;
};