#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareDropList.h"
#include "../Components/FlareColorPanel.h"
#include "../../Game/FlareSaveGame.h"
#include "../FlareUITypes.h"


class AFlareMenuManager;
class AFlareGame;

class SFlareNewGameMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareNewGameMenu){}

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

	/** Sanity check */
	bool IsLaunchDisabled() const;

	/** Start the game */
	void OnLaunch();

	/** Get the combo line headline */
	FText OnGetCurrentComboLine() const;

	/** Generate a combo box line */
	TSharedRef<SWidget> OnGenerateComboLine(TSharedPtr<FString> Item);

	/** Combo line selection changed */
	void OnComboLineSelectionChanged(TSharedPtr<FString> StringItem, ESelectInfo::Type SelectInfo);

	/** Emblem picked */
	void OnEmblemPicked(int32 Index);

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	// Game data
	AFlareGame*                                 Game;
	TWeakObjectPtr<class AFlareMenuManager>     MenuManager;
	FFlareCompanyDescription                    CompanyData;
	TArray<FName>                               ForbiddenIDs;
	
	// Slate widgets
	TSharedPtr<STextBlock>                      CompanyIDHint;
	TSharedPtr<SEditableText>                   CompanyName;
	TSharedPtr<SEditableText>                   CompanyIdentifier;
	TSharedPtr<SFlareColorPanel>                ColorBox;
	TSharedPtr<SFlareButton>                    TutorialButton;
	TSharedPtr<SFlareButton>                    StoryButton;
	TSharedPtr<SFlareButton>                    RandomizeStationButton;
	TSharedPtr<SFlareDropList<int32>>           EmblemPicker;

	// Scenario data
	TSharedPtr<SFlareDropList<TSharedPtr<FString> >> ScenarioSelector;
	TSharedPtr<SFlareDropList<TSharedPtr<FString> >> DifficultySelector;
	TArray<TSharedPtr<FString>>                 ScenarioList;
	TArray<TSharedPtr<FString>>				    DifficultyList;
//	TArray<TSharedPtr<FText>>					DifficultyList;
};