#pragma once

#include "../../Flare.h"
#include "../FlareUITypes.h"

struct FFlareTechnologyDescription;


class SFlareTechnologyMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareTechnologyMenu){}

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

	void ResetDefaults();

protected:

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/

	void UpdateAllInfos();
	void UpdateTechnologyButtonInfo();
	void UpdateCompanyTechnologyInfo();

	/** Company info text */
	FText GetCompanyTechnologyInfo() const;

	/** Technology name text */
	FText GetTechnologyName() const;

	/** Technology description text */
	FText GetTechnologyDescription() const;

	/** Coloring for the level text */
	FSlateColor GetTitleTextColor(int32 RowLevel) const;
	FSlateColor GetTechnologyLevelCostTextColor(int32 RowLevel) const;
	

	/** Technology selected */
	void OnTechnologySelected(const FFlareTechnologyDescription* Technology);
	
	/** The selected technology is unlocked */
	void OnTechnologyUnlocked();

	/** Count of unlocked scannables */
	FText GetUnlockedScannableCount() const;

	/** Coloring for the upgrade tech text */
	FSlateColor GetTechnologyUpgradeTextColor(int32 RowLevel) const;

	FText GetTechnologyLevelCost(int32 Level) const;

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	bool											TutorialQuestComplete = false;

	// General data
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;
	const FFlareTechnologyDescription*              SelectedTechnology;

	// Slate objects
	TSharedPtr<SScrollBox>                          TechnologyTree;
	TSharedPtr<SVerticalBox>                        ArtifactList;
	TSharedPtr<SRichTextBlock>						CompanyTechnologyInfo;
	TSharedPtr<SFlareButton>						UnlockTechnologyButton;
	

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	const FFlareTechnologyDescription* GetSelectedTechnology()
	{
		return SelectedTechnology;
	}
	const bool GetTutorialQuestComplete()
	{
		return TutorialQuestComplete;
	}
};
