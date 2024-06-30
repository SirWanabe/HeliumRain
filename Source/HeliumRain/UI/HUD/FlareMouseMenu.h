#pragma once

#include "../../Flare.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Spacecrafts/FlareSpacecraft.h"


DECLARE_DELEGATE(FFlareMouseMenuClicked)


class SFlareMouseMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareMouseMenu){}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Add a widget */
	void AddWidget(FString Icon, FText Legend);
	void AddWidget(FString Icon, FText Legend, FFlareMouseMenuClicked Action, bool SelectDefault = false);

	void SetPendingMenu(FFlareMouseMenuClicked NewPendingMenu);

	/** Remove all widgets */
	void ClearWidgets();

	/** Open the menu */
	void Open(int32 NewSelectedWidget = -1);

	/** Hide the menu */
	void Close(bool EnableAction = true);
	void SelectOption(int32 IndexOverride = -1);

	/** Check if the menu is open */
	bool IsOpen() const;

	/** Move wheel cursor */
	void SetWheelCursorMove(FVector2D Move);

	inline int32 GetSelectedWidget() const
	{
		return SelectedWidget;
	}

protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	/** Blur intensity */
	float GetWheelMenuBlurStrength() const;

	/** Get a widget's position on the screen */
	FVector2D GetWidgetPosition(int32 Index) const;

	/** Get a widget's size on the screen */
	FVector2D GetWidgetSize(int32 Index) const;

	/** Get a widget's color multiplier */
	FSlateColor GetWidgetColor(int32 Index) const;


	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/

	/** Add a widget */
	void AddWidgetInternal(FString Icon, FText Legend, int32 Index);

	/** Get the widget's direction */
	FVector2D GetDirection(int32 Index) const;

	/** Get colinearity */
	float GetColinearity(int32 Index) const;

	/** Check if a widget has been selected */
	bool HasSelection() const;
	
	/** Reset the current opening/closing animation */
	void SetAnimDirection(bool Opening);


protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** HUD reference */
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>      MenuManager;

	/** Player reference */
	UPROPERTY()
	AFlarePlayerController*              PC;

	// HUD content
	TSharedPtr<SCanvas>                  HUDCanvas;

	// HUD settings
	int32                                WidgetDistance;
	int32                                WidgetSize;
	float                                AnimTime;
	float                                ColinearityPower;
	float                                AutoResetTime;
	float                                Sensitivity;

	// HUD data
	TArray<FFlareMouseMenuClicked>       Actions;
	FVector2D                            ViewportCenter;
	FVector2D                            MouseOffset;
	FVector2D                            PreviousMove;
	int32                                SelectedWidget;
	int32                                SelectedIndex;
	float                                CurrentTime;
	float                                TimeSinceActive;
	float                                IsOpening;
	bool								 MouseMenuAutoReset;
};