#pragma once

#include "../../Flare.h"
#include "Object.h"

#include "../../Data/FlareSpacecraftCatalog.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareRoundButton.h"
#include "../Components/FlareListItem.h"
#include "../FlareUITypes.h"

class AFlareMenuManager;

class SFlareHelpMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareHelpMenu) {}

	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)

	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Interaction
	----------------------------------------------------*/

	/** Craft a line for the list */
	TSharedRef<ITableRow> OnGenerateSpacecraftLine(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable, bool OrderIsShipValue = false);

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Setup the widget */
	void Setup();

	/** Enter this menu */
	void Enter();

	/** Exit this menu */
	void Exit();

	/** Part list generator */
	TSharedRef<ITableRow> GeneratePartInfo(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable, int32 Width = 5);

	/** Craft a line for the list */
	TSharedRef<ITableRow> OnGenerateCompanyLine(TSharedPtr<FInterfaceContainer> Item, const TSharedRef<STableViewBase>& OwnerTable);

protected:

	AFlareGame*                          Game;

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/


protected:

	void SetupResourceDisplay(TSharedPtr<SVerticalBox> HudBox, TArray<FFlareResourceDescription*> ResourcesData, FText CategoryName, FText CategorySubText);

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	TWeakObjectPtr<class AFlareMenuManager>    MenuManager;

	TSharedPtr<SListView<TSharedPtr<FInterfaceContainer>>>    StationSelector;
	TSharedPtr<SListView<TSharedPtr<FInterfaceContainer>>>    ShipSelector;

	// Slate objects
	TArray<TSharedPtr<FInterfaceContainer>>    StationList;
	TArray<TSharedPtr<FInterfaceContainer>>    ShipList;
//	TArray<TSharedPtr<FInterfaceContainer>>    CompanyList;

	UPROPERTY()
	TArray< TSharedPtr<FInterfaceContainer> >                     CompanyListData;
	UPROPERTY()
	TSharedPtr< SListView< TSharedPtr<FInterfaceContainer> > >    CompanyList;

	TArray< TSharedPtr<FInterfaceContainer> >                     EngineListData;
	TSharedPtr< SListView< TSharedPtr<FInterfaceContainer> > >    EngineList;

	TArray< TSharedPtr<FInterfaceContainer> >                     RCSListData;
	TSharedPtr< SListView< TSharedPtr<FInterfaceContainer> > >    RCSList;

	TArray< TSharedPtr<FInterfaceContainer> >                     WeaponListData;
	TSharedPtr< SListView< TSharedPtr<FInterfaceContainer> > >    WeaponList;

	TSharedPtr<SVerticalBox>									  ResourcesBox;
	bool														  SetupResources;
};
