#pragma once

#include "../../Flare.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareList.h"
#include "../Components/FlareConfirmationBox.h"

class UFlareSimulatedSpacecraft;
class IFlareSectorInterface;
struct FFlareResourceDescription;


class SFlareTradeMenu : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareTradeMenu){}

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
	void Enter(UFlareSimulatedSector* ParentSector, UFlareSimulatedSpacecraft* LeftSpacecraft, UFlareSimulatedSpacecraft* RightSpacecraft);

		/** Fill a content pane with the trading information for Target spacecraft to deal with Other */
	void FillTradeBlock(UFlareSimulatedSpacecraft* TargetSpacecraft, UFlareSimulatedSpacecraft* OtherSpacecraft,
		TSharedPtr<SHorizontalBox> CargoBay1, TSharedPtr<SHorizontalBox> CargoBay2);

	/** Exit this menu */
	void Exit();

//	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;

protected:

	bool CheckIsActiveSector();

	/*----------------------------------------------------
		Callbacks
	----------------------------------------------------*/
	
	/** Is the trading part visible or not */
	EVisibility GetTradingVisibility() const;

	/** Is the right side company flag visible or not */
	EVisibility GetCompanyFlagVisibility() const;

	/** Is the trading part visible or not */
	EVisibility GetConstructionInfosVisibility() const;

	/** Is the "back to selection" visible or not */
	EVisibility GetBackToSelectionLeftVisibility() const;
	EVisibility GetBackToSelectionRightVisibility() const;

	/** Is the undock button visible or not */
	EVisibility GetUndockVisibility() const;

	/** Is the select docked button visible or not */
	EVisibility GetSelectDockedVisibility() const;

	/** Are the transaction details visible ? */
	EVisibility GetTransactionDetailsVisibility() const;

	/** Should the donation button be visible ? */
	EVisibility GetDonationButtonVisibility() const;

	/** Are the transaction details visible ? */
	EVisibility GetTransactionInvalidVisibility() const;
	
	/** Get the name of the left spacecraft */
	FText GetLeftSpacecraftName() const;

	/** Get the name of the right spacecraft */
	FText GetRightSpacecraftName() const;
	
	/** Get the transaction details */
	FText GetTransactionDetails() const;

	/** Get the trade ship details/dynamic information */
	FText GetShipTradeDetails() const;

	/** Get the company name or the current fleet's name*/
	FText GetRightcraftInfo() const;

	/** Get the transaction invalid details */
	void SetTransactionInvalidDetails(bool DockingConfirmed = true);

	/** Get the resource prices */
	FText GetResourcePriceInfo(FFlareResourceDescription* Resource) const;
	
	/** A spacecraft has been selected, hide the list and show the cargo */
	void OnSpacecraftSelectedLeft(TSharedPtr<FInterfaceContainer> SpacecraftContainer);
	void OnSpacecraftSelectedRight(TSharedPtr<FInterfaceContainer> SpacecraftContainer);

	void SelectSpacecraftRight(UFlareSimulatedSpacecraft* Spacecraft);

	void OnUndock();

	/** Start transferring a resource */
	void OnTransferResources(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource);

	/** Toggle button pushed */
	void OnDonationToggle();
		
	/** Changed resource quantity, recompute price **/
	void OnResourceQuantityChanged(float Value);

	/** Changed resource quantity, recompute price **/
	void OnResourceQuantityEntered(const FText& TextValue);

	/** Accept a transaction */
	void OnConfirmTransaction();

	/** Cancel a transaction */
	void OnCancelTransaction();

	void OnSelectDockedSelection();

	/** Go back to choosing a ship to trade with */
	void OnBackToSelectionLeft();
	void OnBackToSelectionRight();

	/** Update price on confirm button */
	void UpdatePrice();
	
	/** Update which ships appear in the left ships menu*/
	void UpdateLeftShips();

	/** Return true if the transaction is valid*/
	bool IsTransactionValid(FText& Reason) const;

	/** Refresh Previously done tradeblocks using the previous set of settings*/
	bool RefreshTradeBlocks() const;

	/** How much of this can we afford */
	int32 GetMaxTransactionAmount() const;

	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/

	int32 SetSliderQuantity(int32 Quantity);

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// HUD reference
	UPROPERTY()
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;

	// Menu components
	TSharedPtr<SFlareList>                          LeftShipList;
	TSharedPtr<SFlareList>                          RightShipList;
	TSharedPtr<SHorizontalBox>                      LeftCargoBay1;
	TSharedPtr<SHorizontalBox>                      LeftCargoBay2;
	TSharedPtr<SHorizontalBox>                      RightCargoBay1;
	TSharedPtr<SHorizontalBox>                      RightCargoBay2;
	TSharedPtr<SSlider>                             QuantitySlider;
	TSharedPtr<SEditableText>                       QuantityText;
	TSharedPtr<SFlareConfirmationBox>               PriceBox;
	TSharedPtr<SFlareButton>						DonationButton;
	TSharedPtr<SFlareCompanyFlag>				    CompanyFlag;

	TSharedPtr<STextBlock>						    InvalidTransaction;

	// Data
	UFlareSimulatedSector*                          TargetSector;
	UFlareSimulatedSpacecraft*                      TargetLeftSpacecraft;
	UFlareSimulatedSpacecraft*                      TargetRightSpacecraft;
	UFlareSimulatedSpacecraft*						FirstShipCandidateLeft;

	// Current transaction
	UFlareSimulatedSpacecraft*                      TransactionSourceSpacecraft;
	UFlareSimulatedSpacecraft*                      TransactionDestinationSpacecraft;
	FFlareResourceDescription*                      TransactionResource;
	uint32                                          TransactionQuantity;
	uint32											PreviousTradeDirection;

	bool											MultipleOwnedShipsOrFleets;

public:

	/*----------------------------------------------------
		Getter
	----------------------------------------------------*/

	UFlareSimulatedSpacecraft* GetTargetLeftShip()
	{
		return TargetLeftSpacecraft;
	}

	UFlareSimulatedSpacecraft* GetTargetRightShip()
	{
		return TargetRightSpacecraft;
	}

	UFlareSimulatedSpacecraft* GetTransactionSourceSpacecraft()
	{
		return TransactionSourceSpacecraft;
	}
	UFlareSimulatedSpacecraft* GetTransactionDestinationSpacecraft()
	{
		return TransactionDestinationSpacecraft;
	}

	FFlareResourceDescription* GetTransactionResource()
	{
		return TransactionResource;
	}

	uint32 GetTransactionTransactionQuantity()
	{
		return TransactionQuantity;
	}

	bool GetRefreshTradeBlocks()
	{
		return RefreshTradeBlocks();
	}
};