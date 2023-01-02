#pragma once

#include "../../Flare.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"


class AFlareMenuManager;


class SFlareSkirmishScoreMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareSkirmishScoreMenu){}

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


protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	/** Set the results for a player */
	void FillResults(TSharedPtr<SVerticalBox> Box, FText Title, struct FFlareSkirmishPlayerResult Result);

	/** Add one result line */
	void AddResult(TSharedPtr<SVerticalBox> Box, FText Name, int32 Result);

	/** Relaunch */
	void OnRetry();

	/** Quit the menu */
	void OnMainMenu();

	/** Go straight to the skirmish menu*/
	void OnSkirmishMenu();

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Data
	TWeakObjectPtr<class AFlareMenuManager>       MenuManager;

	// Slate widgets
	TSharedPtr<STextBlock>                        SkirmishResultText;
	TSharedPtr<STextBlock>                        SkirmishTimeText;
	TSharedPtr<SVerticalBox>                      PlayerResults;
	TSharedPtr<SVerticalBox>                      EnemyResults;

};
