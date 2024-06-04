#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareDropList.h"
#include "../../Game/FlareTradeRoute.h"
#include "../../Game/FlareSimulatedSector.h"
#include "../../Game/FlareFleet.h"
#include "../../Data/FlareResourceCatalogEntry.h"
#include "../FlareUITypes.h"

class UFlareCompanyWhiteList;
struct FFlareWhiteListSave;

class SFlareWhiteListMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareWhiteListMenu) {}

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
	void Enter(UFlareCompanyWhiteList* WhiteList, UFlareCompany* Company);

	/** Exit this menu */
	void Exit();

protected:

	TSharedRef<SWidget> OnGenerateCompanyComboLine(UFlareCompany* Item);
	void OnCompanyComboLineSelectionChanged(UFlareCompany* Item, ESelectInfo::Type SelectInfo);

	/*----------------------------------------------------
		Content callbacks
	----------------------------------------------------*/
	
	void OnSetCompanyDefaultWhitelist();
	void OnConfirmChangeWhiteListNameClicked();
	void OnViewCompaniesToggle();
	void UpdateVisibilities();

	void OnToggleTradeToCompany(UFlareCompany* Company, FFlareWhiteListCompanyDataSave* CompanyData);
	void OnToggleTradeFromCompany(UFlareCompany* Company, FFlareWhiteListCompanyDataSave* CompanyData);
	FReply OnToggleResourceWhiteList(FName Identifier, FFlareWhiteListCompanyDataSave* CompanyData, bool TowardsOrFrom, TSharedPtr<SImage> ResourceEnabledImage);

	bool IsRenameDisabled() const;
	bool IsSetDefaultDisabled() const;

	EVisibility GetResourceVisibility(FFlareWhiteListCompanyDataSave* CompanyData, bool TowardsOrFrom) const;

	/*----------------------------------------------------
		Actions callbacks
	----------------------------------------------------*/

	EVisibility GetFactionSelectorVisibility() const;

	void GenerateWhiteListInfoBox();
	bool GenerateWhiteListInfoBoxFor(UFlareCompany* Company, TSharedPtr<SHorizontalBox> CurrentHorizontalBox);
	void GenerateAndDisplayResources(TSharedPtr<SVerticalBox> ResourceBox, UFlareCompany* Company, FFlareWhiteListCompanyDataSave* CompanyData, bool TowardsOrFrom);

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>            MenuManager;
	UFlareCompanyWhiteList*							   TargetWhiteList;
	UFlareCompany*									   TargetCompany;

	TArray<UFlareCompany*>						       CompanyList;
	
	TSharedPtr<SEditableText>                          EditWhiteListName;
	TSharedPtr<SFlareButton>                           ViewAllCompaniesToggle;
	TSharedPtr<SBox>			                       CompanySelectionSBox;
	TSharedPtr<SVerticalBox>			               CompanyWhiteListInfoBox;
	TSharedPtr<SFlareDropList<UFlareCompany*>>         CompanySelector;

	const FSlateBrush* ResourceOKIcon = FFlareStyleSet::GetIcon("OK_Small");
	const FSlateBrush* ResourceDisabledIcon = FFlareStyleSet::GetIcon("Disabled_Small");
};
