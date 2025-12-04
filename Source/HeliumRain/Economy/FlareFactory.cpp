
#include "FlareFactory.h"
#include "../Flare.h"

#include "../Data/FlareResourceCatalog.h"
#include "../Data/FlareSpacecraftCatalog.h"

#include "../Game/FlareWorld.h"
#include "../Game/FlareGame.h"
#include "../Game/FlareGameTools.h"
#include "../Game/FlareCompany.h"
#include "../Game/FlareSimulatedSector.h"

#include "../Economy/FlareCargoBay.h"

#include "../Player/FlarePlayerController.h"

#include "../Spacecrafts/FlareSimulatedSpacecraft.h"


#define LOCTEXT_NAMESPACE "FlareFactoryInfo"
#define MAX_DAMAGE_MALUS 10
#define FACTORY_EFFICIENCY_CHANGE_DURATIONCHANGE 0.00025f
#define FACTORY_EFFICIENCY_CHANGE_DOPRODUCTION 0.001f
#define FACTORY_EFFICIENCY_MIN 0.90f
#define FACTORY_EFFICIENCY_MAX 1.10f
//#define DEBUG_FACTORY

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareFactory::UFlareFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlareFactory::Load(UFlareSimulatedSpacecraft* ParentSpacecraft, const FFlareFactoryDescription* Description, const FFlareFactorySave& Data)
{
	Game = Cast<UFlareWorld>(GetOuter())->GetGame();

	FactoryData = Data;
	FactoryDescription = Description;
	Parent = ParentSpacecraft;
	CycleCostCacheLevel = -1;

	if (IsShipyard() && FactoryData.TargetShipClass == NAME_None && FactoryData.Active)
	{
		FLOG("WARNING: fix corrupted shipyard state");
		FactoryData.Active = false;
	}
}

void UFlareFactory::SetIsStationConstructionFactory(bool NewValue)
{
	IsStationConstructionFactory = NewValue;
}

FFlareFactorySave* UFlareFactory::Save()
{
	return &FactoryData;
}


bool UFlareFactory::OwnerCompanyHasRequiredTechnologies()
{
	UpdateHasrequiredtechnologies();
	return Hasrequiredtechnologies;
}

void UFlareFactory::SetHascheckedforrequiredtechnologies(bool NewValue)
{
	Hascheckedforrequiredtechnologies = NewValue;
	UpdateHasrequiredtechnologies();
}

void UFlareFactory::UpdateHasrequiredtechnologies()
{
	if (Hascheckedforrequiredtechnologies)
	{
		return;
	}

	bool PreviousHasrequiredtechnologies = Hasrequiredtechnologies;

	Hascheckedforrequiredtechnologies = true;
	Hasrequiredtechnologies = true;

	if (GetDescription()->RequiredTechnologies.Num() > 0)
	{
//		FLOGV("UFlareFactory::UpdateHasrequiredtechnologies Checking required technologies %s", *Parent->GetImmatriculation().ToString());
		for (int32 i = 0; i < GetDescription()->RequiredTechnologies.Num(); i++)
		{
			FName CurrentTechnology = GetDescription()->RequiredTechnologies[i];
//			FLOGV("UFlareFactory::UpdateHasrequiredtechnologies Found %s in required technologies", *CurrentTechnology.ToString());
			if (!Parent->GetCompany()->IsTechnologyUnlocked(CurrentTechnology))
			{
				FFlareTechnologyDescription* RequiredTechnology = GetGame()->GetTechnologyCatalog()->Get(CurrentTechnology);
				if (!RequiredTechnology)
				{
					continue;
				}

//				FLOGV("UFlareFactory::UpdateHasrequiredtechnologies %s not unlocked", *CurrentTechnology.ToString());
				Hasrequiredtechnologies = false;
				break;
			}
		}

		if (!PreviousHasrequiredtechnologies && Hasrequiredtechnologies)
		{
			Parent->LockResources();
		}
	}

	Factory_Efficiency_ProductionChange = FACTORY_EFFICIENCY_CHANGE_DURATIONCHANGE;
	Factory_Efficiency_DoProduction = FACTORY_EFFICIENCY_CHANGE_DOPRODUCTION;
	Factory_Efficiency_Minimum = FACTORY_EFFICIENCY_MIN;
	Factory_Efficiency_Maximum = FACTORY_EFFICIENCY_MAX;
/*
	if (Parent->GetCompany()->IsTechnologyUnlockedStation(Parent->GetDescription()))
	{
		Factory_Efficiency_Minimum -= 0.10f;
		Factory_Efficiency_Maximum -= 0.20f;
	}
*/
	for (int TechnologyEffectsIndex = 0; TechnologyEffectsIndex < GetDescription()->TechnologiesEffects.Num(); TechnologyEffectsIndex++)
	{
		FFlareFactoryTechnologyEffects CurrentTechnologyEffect = GetDescription()->TechnologiesEffects[TechnologyEffectsIndex];
		if (!Parent->GetCompany()->IsTechnologyUnlocked(CurrentTechnologyEffect.Identifier))
		{
			continue;
		}

		Factory_Efficiency_ProductionChange += CurrentTechnologyEffect.Efficiency_ProductionOngoing;
		Factory_Efficiency_DoProduction += CurrentTechnologyEffect.Efficiency_ProductionFinished;

		Factory_Efficiency_Minimum += CurrentTechnologyEffect.Efficiency_Minimum;
		Factory_Efficiency_Maximum += CurrentTechnologyEffect.Efficiency_Maximum;
	}
}

void UFlareFactory::Simulate()
{
	FCHECK(Parent);
	FCHECK(Parent->GetCurrentSector());
	FCHECK(Parent->GetCurrentSector()->GetPeople());
	bool IncreasedEfficiency = false;

#ifdef DEBUG_FACTORY
	if (Parent->GetCompany() == Parent->GetGame()->GetPC()->GetCompany())
	{
		FLOGV("Factory Simulate running before initial check %s in %s", *Parent->GetImmatriculation().ToString(), *Parent->GetCurrentSector()->GetName());
	}
#endif

	// Check if production is running
	if (!OwnerCompanyHasRequiredTechnologies() || !FactoryData.Active || !IsNeedProduction())
	{
		// Don't produce if not needed
		PostProduction(IncreasedEfficiency);
		return;
	}

	int64 ProductionTime = GetProductionTime(GetCycleData());

#ifdef DEBUG_FACTORY
	if (Parent->GetCompany() == Parent->GetGame()->GetPC()->GetCompany())
	{
		FLOGV("Factory Simulate running after initial check %s in %s", *Parent->GetImmatriculation().ToString(), *Parent->GetCurrentSector()->GetName());
	}
#endif

	if (HasCostReserved())
	{

#ifdef DEBUG_FACTORY
		if (Parent->GetCompany() == Parent->GetGame()->GetPC()->GetCompany())
		{
			FLOGV("Factory Simulate %d production time Has Cost reserves to %s in %s", ProductionTime,  *Parent->GetImmatriculation().ToString(), *Parent->GetCurrentSector()->GetName());
		}
#endif

		if (FactoryData.ProductedDuration < ProductionTime)
		{
			FactoryData.ProductedDuration += 1;
			FactoryData.FactoryEfficiency = FMath::Clamp(FactoryData.FactoryEfficiency + Factory_Efficiency_ProductionChange, Factory_Efficiency_Minimum, Factory_Efficiency_Maximum);
			IncreasedEfficiency = true;

#ifdef DEBUG_FACTORY
			if (Parent->GetCompany() == Parent->GetGame()->GetPC()->GetCompany())
			{
				FLOGV("Factory Simulate Days increased to %s in %s", *Parent->GetImmatriculation().ToString(), *Parent->GetCurrentSector()->GetName());
			}
#endif
		}

		if (FactoryData.ProductedDuration < ProductionTime
			|| !HasOutputFreeSpace()
			|| (Parent->GetCurrentFleet() && Parent->GetCurrentFleet()->IsTraveling()))
		{
			// Still In production, or has no free space, or is a ship in a fleet that is travelling
			PostProduction(IncreasedEfficiency);

#ifdef DEBUG_FACTORY
			if (Parent->GetCompany() == Parent->GetGame()->GetPC()->GetCompany())
			{
				FLOGV("Factory Simulate Days increased but post production to %s in %s",  *Parent->GetImmatriculation().ToString(), *Parent->GetCurrentSector()->GetName());
			}
#endif
			return;
		}

#ifdef DEBUG_FACTORY
		if (Parent->GetCompany() == Parent->GetGame()->GetPC()->GetCompany())
		{
			FLOGV("Factory Simulate do production %s in %s",  *Parent->GetImmatriculation().ToString(), *Parent->GetCurrentSector()->GetName());
		}
#endif
		DoProduction();
	}

	TryBeginProduction();

	if (ProductionTime == 0  && HasCostReserved())
	{
		if (Parent->GetCurrentFleet() && Parent->GetCurrentFleet()->IsTraveling())
		{
			//Wait for travel to finish before finalizing the production
		}
		else
		{
			DoProduction();
		}
	}

	PostProduction(IncreasedEfficiency);

#ifdef DEBUG_FACTORY
	if (Parent->GetCompany() == Parent->GetGame()->GetPC()->GetCompany())
	{
		FLOGV("Factory Simulate end %s in %s", *Parent->GetImmatriculation().ToString(), *Parent->GetCurrentSector()->GetName());
	}
#endif
}

void UFlareFactory::TryBeginProduction()
{
	if (GetMarginRatio() < 0.f)
	{
		FLOGV("WARNING: Margin ratio for %s is : %f", *FactoryDescription->Name.ToString(), GetMarginRatio())
	}

	if (IsNeedProduction() && !HasCostReserved() && HasInputResources() && HasInputMoney())
	{
		BeginProduction();
	}
}

void UFlareFactory::UpdateDynamicState()
{
	if(FactoryData.TargetShipClass == NAME_None)
	{
		Parent->SetDynamicComponentState(TEXT("idle"));
	}
	else
	{
		Parent->SetDynamicComponentState(FactoryData.TargetShipClass,
				((float)GetProductedDuration() / (float)GetRemainingProductionDuration()));
	}
}

void UFlareFactory::Start()
{
	if(FactoryDescription->IsTelescope())
	{
		if(GetTelescopeTargetList().Num() == 0)
		{
			NotifyNoMoreSector();
			return;
		}
	}

	FactoryData.Active = true;
}

void UFlareFactory::StartShipBuilding(FFlareShipyardOrderSave& Order)
{
	if (FactoryData.TargetShipCompany == NAME_None && Order.Company != NAME_None)
	{
		FactoryData.TargetShipClass = Order.ShipClass;
		FactoryData.TargetShipCompany = Order.Company;
		FactoryData.ProductedDuration = 0;
		Parent->GetCompany()->GiveMoney(Order.AdvancePayment, FFlareTransactionLogEntry::LogShipOrderAdvance(GetParent(), Order.Company, Order.ShipClass));
	}
	Start();
	TryBeginProduction();
}

void UFlareFactory::Pause()
{
	FactoryData.Active = false;
}

void UFlareFactory::Stop()
{
	FactoryData.Active = false;
	CancelProduction();
}

void UFlareFactory::SetInfiniteCycle(bool Mode)
{
	FactoryData.InfiniteCycle = Mode;
}

void UFlareFactory::SetCycleCount(uint32 Count)
{
	FactoryData.CycleCount = Count;
}

void UFlareFactory::SetOutputLimit(FFlareResourceDescription* Resource, uint32 MaxSlot)
{
	bool ExistingResource = false;
	for (int32 CargoLimitIndex = 0 ; CargoLimitIndex < FactoryData.OutputCargoLimit.Num() ; CargoLimitIndex++)
	{
		if (FactoryData.OutputCargoLimit[CargoLimitIndex].ResourceIdentifier == Resource->Identifier)
		{
			ExistingResource = true;
			FactoryData.OutputCargoLimit[CargoLimitIndex].Quantity = MaxSlot;
			break;
		}
	}

	if (!ExistingResource)
	{
		FFlareCargoSave NewCargoLimit;
		NewCargoLimit.ResourceIdentifier = Resource->Identifier;
		NewCargoLimit.Quantity = MaxSlot;
		FactoryData.OutputCargoLimit.Add(NewCargoLimit);
	}
}

void UFlareFactory::ClearOutputLimit(FFlareResourceDescription* Resource)
{
	for (int32 CargoLimitIndex = 0 ; CargoLimitIndex < FactoryData.OutputCargoLimit.Num() ; CargoLimitIndex++)
	{
		if (FactoryData.OutputCargoLimit[CargoLimitIndex].ResourceIdentifier == Resource->Identifier)
		{
			FactoryData.OutputCargoLimit.RemoveAt(CargoLimitIndex);
			return;
		}
	}
}

float UFlareFactory::GetResourceInputMultiplier()
{
	float TotalResourceMargin = 1;
	if (IsShipyard())
	{
		float ShipyardfabricationBonus = ShipyardfabricationBonus = GetParent()->GetCompany()->GetTechnologyBonus_Float("shipyard-fabrication-bonus");
		if (ShipyardfabricationBonus)
		{
			TotalResourceMargin = FMath::Max(0.10f, TotalResourceMargin - ShipyardfabricationBonus);
		}
	}
	return TotalResourceMargin;
}

bool UFlareFactory::HasCostReserved()
{
	if (FactoryData.CostReserved < GetProductionCost())
	{
		return false;
	}

	float TotalResourceMargin = GetResourceInputMultiplier();

	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &GetCycleData().InputResources[ResourceIndex];
		int32 AdjustedQuantity = Resource->Quantity * TotalResourceMargin;

		bool ResourceFound = false;

		for (int32 ReservedResourceIndex = 0; ReservedResourceIndex < FactoryData.ResourceReserved.Num(); ReservedResourceIndex++)
		{
			if (FactoryData.ResourceReserved[ReservedResourceIndex].ResourceIdentifier ==  Resource->Resource->Data.Identifier)
			{
				if (FactoryData.ResourceReserved[ReservedResourceIndex].Quantity < AdjustedQuantity)
				{
					return false;
				}
				ResourceFound = true;
			}
		}

		if (!ResourceFound)
		{
			return false;
		}
	}

	return true;
}

bool UFlareFactory::HasInputMoney()
{
	bool AllowDepts = Parent->GetCompany() != Parent->GetGame()->GetPC()->GetCompany();

	if(AllowDepts)
	{
		return true;
	}
	return Parent->GetCompany()->GetMoney() >= GetProductionCost();
}

bool UFlareFactory::HasInputResources()
{
	float TotalResourceMargin = GetResourceInputMultiplier();
	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &GetCycleData().InputResources[ResourceIndex];
		int32 AdjustedQuantity = Resource->Quantity * TotalResourceMargin;
		if (!Parent->GetActiveCargoBayFromFactory(this)->HasResources(&Resource->Resource->Data, AdjustedQuantity, Parent->GetCompany(),0,true))
		{
			return false;
		}
	}
	return true;
}

FString UFlareFactory::GetInputResourcesRequiredString()
{
	FString ReturnValue;

	float TotalResourceMargin = GetResourceInputMultiplier();
	for (int32 ResourceIndex = 0; ResourceIndex < GetCycleData().InputResources.Num(); ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &GetCycleData().InputResources[ResourceIndex];
		int32 AdjustedQuantity = Resource->Quantity * TotalResourceMargin;
		int32 CurrentResourceCount = Parent->GetActiveCargoBayFromFactory(this)->GetResourceQuantitySimple(&Resource->Resource->Data);
		if (CurrentResourceCount < AdjustedQuantity)
		{
			if (ReturnValue.Len())
			{
				ReturnValue += ", ";
			}

			int32 MissingResources = AdjustedQuantity - CurrentResourceCount;
			ReturnValue += FString::Printf(TEXT("%s: %d"), *Resource->Resource->Data.Name.ToString(), MissingResources);
		}
	}
	return ReturnValue;
}

bool UFlareFactory::HasOutputFreeSpace()
{
	UFlareCargoBay* CargoBay = Parent->GetActiveCargoBayFromFactory(this);
	TArray<FFlareFactoryResource> OutputResources = GetLimitedOutputResources();
/*
	if (Parent->IsComplexElement())
	{
		FLOGV("%d output size after GetLimitedOutputResources to %s in %s", OutputResources.Num(), *Parent->GetImmatriculation().ToString(), *Parent->GetCurrentSector()->GetName());
	}
*/
	// First, fill already existing locked slots
	for (int32 CargoIndex = 0 ; CargoIndex < CargoBay->GetSlotCount() ; CargoIndex++)
	{
		if(!CargoBay->CheckRestriction(CargoBay->GetSlot(CargoIndex), Parent->GetCompany(),0,true))
		{
//			continue;
		}

		if(CargoBay->GetSlot(CargoIndex)->Lock == EFlareResourceLock::NoLock)
		{
//			continue;
		}

		for (int32 ResourceIndex = OutputResources.Num() -1 ; ResourceIndex >= 0; ResourceIndex--)
		{
			if (&OutputResources[ResourceIndex].Resource->Data == CargoBay->GetSlot(CargoIndex)->Resource)
			{
				// Same resource
					
				int32 AvailableCapacity = CargoBay->GetSlotCapacity() - CargoBay->GetSlot(CargoIndex)->Quantity;

				if (AvailableCapacity > 0)
				{
					int32 QuantityAfterEfficiency = OutputResources[ResourceIndex].Quantity * FactoryData.FactoryEfficiency;
					QuantityAfterEfficiency -= FMath::Min(AvailableCapacity, QuantityAfterEfficiency);

					if (QuantityAfterEfficiency <= 0)
					{
						OutputResources.RemoveAt(ResourceIndex);
					}
					else
					{
						OutputResources[ResourceIndex].Quantity = QuantityAfterEfficiency;
					}

					// Never 2 output with the same resource, so, break
					break;
				}
			}
		}
	}

	// Fill free cargo locked slots
	for (int32 CargoIndex = 0 ; CargoIndex < CargoBay->GetSlotCount() ; CargoIndex++)
	{
		if (OutputResources.Num() == 0)
		{
			// No more resource to try to put
			break;
		}

		if(CargoBay->GetSlot(CargoIndex)->Lock == EFlareResourceLock::NoLock)
		{
			continue;
		}

		if (CargoBay->GetSlot(CargoIndex)->Quantity == 0 && CargoBay->GetSlot(CargoIndex)->Resource == NULL)
		{
			// Empty slot, fill it
			int32 QuantityAfterEfficiency = OutputResources[0].Quantity * FactoryData.FactoryEfficiency;
			QuantityAfterEfficiency -= FMath::Min(CargoBay->GetSlotCapacity(), QuantityAfterEfficiency);

			if (QuantityAfterEfficiency <= 0)
			{
				OutputResources.RemoveAt(0);
			}
			else
			{
				OutputResources[0].Quantity = QuantityAfterEfficiency;
			}
		}
	}
/*
	if (Parent->IsComplexElement())
	{
		FLOGV("%d output size after Attempting to find existing or free crago to %s in %s", OutputResources.Num(), *Parent->GetImmatriculation().ToString(), *Parent->GetCurrentSector()->GetName());
	}
*/
	return OutputResources.Num() == 0;
}

void UFlareFactory::BeginProduction()
{
	bool AllowDepts = !IsShipyard()
			|| (GetTargetShipCompany() != NAME_None && GetTargetShipCompany() != Parent->GetCompany()->GetIdentifier());

	if (GetProductionCost() > 0)
	{
		if(!Parent->GetCompany()->TakeMoney(GetProductionCost(), AllowDepts, FFlareTransactionLogEntry::LogFactoryWages(this)))
		{
			return;
		}
	}

	float TotalResourceMargin = GetResourceInputMultiplier();
	// Consume input resources
	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &GetCycleData().InputResources[ResourceIndex];
		FFlareCargoSave* AlreadyReservedCargo = NULL;

		// Check in reserved resources
		for (int32 ReservedResourceIndex = 0; ReservedResourceIndex < FactoryData.ResourceReserved.Num(); ReservedResourceIndex++)
		{
			if (FactoryData.ResourceReserved[ReservedResourceIndex].ResourceIdentifier ==  Resource->Resource->Data.Identifier)
			{
				AlreadyReservedCargo = &FactoryData.ResourceReserved[ReservedResourceIndex];
				break;
			}
		}

		uint32 ResourceToTake = Resource->Quantity * TotalResourceMargin;

		if (AlreadyReservedCargo)
		{
			ResourceToTake -= AlreadyReservedCargo->Quantity;
		}

		if (ResourceToTake <= 0)
		{
			continue;
		}

		// < Resource->Quantity
		if (!Parent->GetActiveCargoBayFromFactory(this)->TakeResources(&Resource->Resource->Data, ResourceToTake, Parent->GetCompany(), 0, true))
		{
			FLOGV("Fail to take %d resource '%s' to %s in %s", Resource->Quantity, *Resource->Resource->Data.Name.ToString(), *Parent->GetImmatriculation().ToString(), *Parent->GetCurrentSector()->GetName());
		}

		if (AlreadyReservedCargo)
		{
			AlreadyReservedCargo->Quantity += ResourceToTake;
		}
		else
		{
			FFlareCargoSave NewReservedResource;
			NewReservedResource.Quantity = ResourceToTake;
			NewReservedResource.ResourceIdentifier = Resource->Resource->Data.Identifier;
			FactoryData.ResourceReserved.Add(NewReservedResource);
		}
	}

	FactoryData.CostReserved = GetProductionCost();
}

void UFlareFactory::CancelProduction()
{
	Parent->GetCompany()->GiveMoney(FactoryData.CostReserved, FFlareTransactionLogEntry::LogCancelFactoryWages(this));
	FactoryData.CostReserved = 0;

	// Restore reserved resources
	for (int32 ReservedResourceIndex = FactoryData.ResourceReserved.Num()-1; ReservedResourceIndex >=0 ; ReservedResourceIndex--)
	{
		FFlareResourceDescription*Resource = Game->GetResourceCatalog()->Get(FactoryData.ResourceReserved[ReservedResourceIndex].ResourceIdentifier);

		int32 GivenQuantity = Parent->GetActiveCargoBayFromFactory(this)->GiveResources(Resource, FactoryData.ResourceReserved[ReservedResourceIndex].Quantity, Parent->GetCompany(),0,true);

		if (GivenQuantity >= FactoryData.ResourceReserved[ReservedResourceIndex].Quantity)
		{
			FactoryData.ResourceReserved.RemoveAt(ReservedResourceIndex);
		}
		else
		{
			FactoryData.ResourceReserved[ReservedResourceIndex].Quantity -= GivenQuantity;
		}
	}

	FactoryData.ProductedDuration = 0;
	FactoryData.TargetShipClass = NAME_None;
	FactoryData.TargetShipCompany = NAME_None;

	if (IsShipyard())
	{
		// Wait next ship order
		FactoryData.Active = false;
		Parent->UpdateShipyardProduction();
	}
}

void UFlareFactory::DoProduction()
{
	if (Parent->GetOwnerHasStationLicense())
	{
		FactoryData.FactoryEfficiency = FMath::Clamp(FactoryData.FactoryEfficiency + Factory_Efficiency_DoProduction, Factory_Efficiency_Minimum, Factory_Efficiency_Maximum);
	}

	// Pay cost
	uint32 PaidCost = FMath::Min(GetProductionCost(), FactoryData.CostReserved);
	FactoryData.CostReserved -= PaidCost;
	Parent->GetCurrentSector()->GetPeople()->Pay(PaidCost);

	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &GetCycleData().InputResources[ResourceIndex];

		for (int32 ReservedResourceIndex = FactoryData.ResourceReserved.Num()-1; ReservedResourceIndex >=0 ; ReservedResourceIndex--)
		{
			if (FactoryData.ResourceReserved[ReservedResourceIndex].ResourceIdentifier != Resource->Resource->Data.Identifier)
			{
				continue;
			}

			if (Resource->Quantity >= FactoryData.ResourceReserved[ReservedResourceIndex].Quantity)
			{
				FactoryData.ResourceReserved.RemoveAt(ReservedResourceIndex);
			}
			else
			{
				FactoryData.ResourceReserved[ReservedResourceIndex].Quantity -= Resource->Quantity;
			}
		}
	}

	// Generate output resources
	TArray<FFlareFactoryResource> OutputResources = GetLimitedOutputResources();
	for (int32 ResourceIndex = 0 ; ResourceIndex < OutputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &OutputResources[ResourceIndex];
		int32 QuantityAfterEfficiency = Resource->Quantity * FactoryData.FactoryEfficiency;
		if (!Parent->GetActiveCargoBayFromFactory(this)->GiveResources(&Resource->Resource->Data, QuantityAfterEfficiency, Parent->GetCompany(),0,true))
		{
			FLOGV("Fail to give %d resource '%s' to %s in %s", QuantityAfterEfficiency, *Resource->Resource->Data.Name.ToString(), *Parent->GetImmatriculation().ToString(), *Parent->GetCurrentSector()->GetName());
		}
	}

	// Perform output actions
	for (int32 ActionIndex = 0 ; ActionIndex < FactoryDescription->OutputActions.Num() ; ActionIndex++)
	{
		const FFlareFactoryAction* Action = &FactoryDescription->OutputActions[ActionIndex];
		switch(Action->Action)
		{
			case EFlareFactoryAction::CreateShip:
				PerformCreateShipAction(Action);
				break;

			case EFlareFactoryAction::DiscoverSector:
				PerformDiscoverSectorAction(Action);
				break;

			case EFlareFactoryAction::GainResearch:
				PerformGainResearchAction(Action);
				break;

			case EFlareFactoryAction::BuildStation:
				PerformBuildStationAction(Action);
				break;

			default:
				FLOGV("Warning ! Not implemented factory action %d", (Action->Action+0));
		}
	}

	FactoryData.ProductedDuration = 0;
	if (!HasInfiniteCycle())
	{
		FactoryData.CycleCount--;
	}
}

void UFlareFactory::PostProduction(bool IncreasedEfficiency)
{
	if (!IncreasedEfficiency)
	{
		FactoryData.FactoryEfficiency = FMath::Clamp(FactoryData.FactoryEfficiency - Factory_Efficiency_DoProduction, Factory_Efficiency_Minimum, Factory_Efficiency_Maximum);
	}
	
	if (FactoryDescription->VisibleStates)
	{
		UpdateDynamicState();
	}
}

FFlareWorldEvent *UFlareFactory::GenerateEvent()
{
	if (!FactoryData.Active || !IsNeedProduction())
	{
		return NULL;
	}

	// Check if production is running
	if (HasCostReserved())
	{
		if (!HasOutputFreeSpace())
		{
			return NULL;
		}

		NextEvent.Date= GetGame()->GetGameWorld()->GetDate() + GetProductionTime(GetCycleData()) - FactoryData.ProductedDuration;
		NextEvent.Visibility = EFlareEventVisibility::Silent;
		return &NextEvent;
	}
	return NULL;
}

void UFlareFactory::PerformCreateShipAction(const FFlareFactoryAction* Action)
{
	FFlareSpacecraftDescription* ShipDescription = GetGame()->GetSpacecraftCatalog()->Get(FactoryData.TargetShipClass);

	if (ShipDescription)
	{
		UFlareCompany* Company = Parent->GetCompany();

		if(FactoryData.TargetShipCompany != NAME_None)
		{
			Company = Game->GetGameWorld()->FindCompany(FactoryData.TargetShipCompany);
		}

		FVector SpawnPosition = Parent->GetSpawnLocation();
		for (uint32 Index = 0; Index < Action->Quantity; Index++)
		{
			// Get data
			UFlareSimulatedSpacecraft* ParentTypecast = Parent;

			UFlareSimulatedSpacecraft* Spacecraft = Parent->GetCurrentSector()->CreateSpacecraft(ShipDescription, Company, SpawnPosition, FRotator::ZeroRotator, NULL, 0, false, NAME_None, Parent);	
			AFlarePlayerController* PC = Parent->GetGame()->GetPC();
			FFlareMenuParameterData Data;
			Data.Spacecraft = Spacecraft;


			// Notify PC
			if (PC && Spacecraft && Spacecraft->GetCompany() == PC->GetCompany() && !Spacecraft->GetDescription()->IsDroneShip)
			{
				PC->Notify(LOCTEXT("ShipBuilt", "Ship production complete"),
					FText::Format(LOCTEXT("ShipBuiltFormat", "Your ship {0} is ready to use !"), UFlareGameTools::DisplaySpacecraftName(Spacecraft)),
					FName("ship-production-complete"),
					EFlareNotification::NT_Economy,
					NOTIFY_DEFAULT_TIMER,
					EFlareMenu::MENU_Ship,
					Data);

				Game->GetQuestManager()->OnEvent(FFlareBundle().PutTag("build-ship").PutInt32("size", Spacecraft->GetSize()));

				PC->SetAchievementProgression("ACHIEVEMENT_SHIP", 1);
			}
		}
	}

	FactoryData.TargetShipClass = NAME_None;
	FactoryData.TargetShipCompany = NAME_None;

	// No more ship to produce
	Stop();
}

// Compute a proximity score (lower is better)
// Hundreds are bodies, dozens are altitude, units are phase
static int ComputeProximityScore(const UFlareSimulatedSector& SectorA, const UFlareSimulatedSector& SectorB)
{
	int Score = 0;

	// Get parameters
	const FFlareSectorOrbitParameters* ParamsA = SectorA.GetOrbitParameters();
	const FFlareSectorOrbitParameters* ParamsB = SectorB.GetOrbitParameters();

	// Same planetary body as source
	if (ParamsA->CelestialBodyIdentifier != ParamsB->CelestialBodyIdentifier)
	{
		Score += 100;
	}

	// Altitude
	float MaxAltitude = 10000;
	float AltitudeDistance = FMath::Abs(ParamsA->Altitude - ParamsB->Altitude);
	AltitudeDistance = FMath::Clamp(AltitudeDistance, 0.0f, MaxAltitude);
	Score += 10 * (AltitudeDistance / MaxAltitude);

	// Phase
	float MaxPhase = 360;
	float PhaseDistance = FMath::Abs(ParamsA->Phase - ParamsB->Phase);
	PhaseDistance = FMath::Clamp(PhaseDistance, 0.0f, MaxPhase);
	Score += PhaseDistance / MaxPhase;

	return Score;
}

struct FSortByProximity
{
	UFlareSimulatedSector* SourceSector;
	FSortByProximity(UFlareSimulatedSector* Sector)
		: SourceSector(Sector)
	{}

	bool operator()(const UFlareSimulatedSector& SectorA, const UFlareSimulatedSector& SectorB) const
	{
		return (ComputeProximityScore(SectorA, *SourceSector) < ComputeProximityScore(SectorB, *SourceSector));
	}
};


TArray<UFlareSimulatedSector*> UFlareFactory::GetTelescopeTargetList()
{
	UFlareCompany* Company = Parent->GetCompany();

	// List all unknown sectors
	TArray<UFlareSimulatedSector*> Candidates;
	for (auto CandidateSector : Parent->GetGame()->GetGameWorld()->GetSectors())
	{
		if (!Company->IsKnownSector(CandidateSector) && !CandidateSector->GetDescription()->IsHiddenFromTelescopes && Parent->GetLevel() >= CandidateSector->GetDescription()->TelescopeDiscoveryLevel)
		{
			Candidates.Add(CandidateSector);
		}
	}

	return Candidates;
}

void UFlareFactory::NotifyNoMoreSector()
{
	UFlareCompany* Company = Parent->GetCompany();
	AFlarePlayerController* PC = Parent->GetGame()->GetPC();
	if (Company == PC->GetCompany())
	{
		UFlareSimulatedSector* CurrentSector = Parent->GetCurrentSector();

		FFlareMenuParameterData Data;
		Data.Sector = CurrentSector;

		Game->GetPC()->Notify(
		LOCTEXT("NoMoreDiscovery", "All sectors found"),
		LOCTEXT("NoMoreDiscoveryFormat", "Your astronomers have mapped the entire sky, and reached the limits of their telescopes. All sectors have been discovered."),
		"no-more-discovery",
		EFlareNotification::NT_Info,
		NOTIFY_DEFAULT_TIMER,
		EFlareMenu::MENU_Sector,
		Data);
	}
}

void UFlareFactory::PerformDiscoverSectorAction(const FFlareFactoryAction* Action)
{
	UFlareCompany* Company = Parent->GetCompany();

	TArray<UFlareSimulatedSector*> Candidates = GetTelescopeTargetList();

	// Sort by score
	UFlareSimulatedSector* CurrentSector = Parent->GetCurrentSector();
	Candidates.Sort(FSortByProximity(CurrentSector));

	// Discover sector
	if (Candidates.Num())
	{
		AFlarePlayerController* PC = Parent->GetGame()->GetPC();
		UFlareSimulatedSector* TargetSector = NULL;

		// Pick a sector
		int TelescopeRange = 2;
		int Index = FMath::RandHelper(TelescopeRange);
		Index = FMath::Clamp(Index, 0, Candidates.Num()-1);
		TargetSector = Candidates[Index];

		// Player-owned telescope (should always be the case according to #99) or other company ?
		if (Company == PC->GetCompany())
		{
			PC->DiscoverSector(TargetSector, false, true);

			if(Candidates.Num() <= 1)
			{
				NotifyNoMoreSector();
			}
			Stop();
		}
		else
		{
			Company->DiscoverSector(TargetSector);
			if (Candidates.Num() <= 1)
			{
				Stop();
			}
		}
	}
	else
	{
		NotifyNoMoreSector();
		FLOG("UFlareFactory::PerformDiscoverSectorAction : could not find a sector !");
		Stop();
	}
}

void UFlareFactory::PerformGainResearchAction(const FFlareFactoryAction* Action)
{
	UFlareCompany* Company = Parent->GetCompany();

	Company->GiveResearch((Action->Quantity * Parent->GetLevel()) * FactoryData.FactoryEfficiency);
	AFlarePlayerController* PC = Parent->GetGame()->GetPC();

	// Notify PC
	if (Parent->GetCompany() == PC->GetCompany())
	{
		Game->GetQuestManager()->OnEvent(FFlareBundle().PutTag("produce-research").PutInt32("amount", Action->Quantity));
	}
}


void UFlareFactory::PerformBuildStationAction(const FFlareFactoryAction* Action)
{
	Parent->FinishConstruction();

	AFlarePlayerController* PC = Parent->GetGame()->GetPC();

	// Notify PC
	if (Parent->GetCompany() == PC->GetCompany())
	{
		// Get data
		FFlareMenuParameterData Data;
		if (Parent->IsComplexElement())
		{
			Data.Spacecraft = Parent->GetComplexMaster();
		}
		else
		{
			Data.Spacecraft = Parent;
		}

		PC->Notify(LOCTEXT("StationBuild", "Station build"),
			FText::Format(LOCTEXT("StationBuiltFormat", "Your station {0} is ready to use !"), UFlareGameTools::DisplaySpacecraftName(Data.Spacecraft)),
			FName("station-production-complete"),
			EFlareNotification::NT_Economy,
			NOTIFY_DEFAULT_TIMER,
			EFlareMenu::MENU_Station,
			Data);

		Game->GetQuestManager()->OnEvent(FFlareBundle().PutTag("finish-station-construction")
										 .PutInt32("upgrade", int32(Parent->GetLevel() > 1))
										 .PutName("identifier", Parent->GetDescription()->Identifier)
										 .PutName("sector", Parent->GetCurrentSector()->GetIdentifier()));

		PC->SetAchievementProgression("ACHIEVEMENT_STATION", 1);
	}
}

/*----------------------------------------------------
	Getters
----------------------------------------------------*/

const FFlareProductionData& UFlareFactory::GetCycleData()
{
	if (IsShipyard() && FactoryData.TargetShipClass != NAME_None)
	{
		return GetCycleDataForShipClass(FactoryData.TargetShipClass);
	}
	else if (Parent->GetLevel() == CycleCostCacheLevel)
	{
		return CycleCostCache;
	}
	else
	{
		CycleCostCacheLevel = Parent->IsUnderConstruction() ? 1 : Parent->GetLevel();
		CycleCostCache.ProductionTime = FactoryDescription->CycleCost.ProductionTime;
		CycleCostCache.ProductionCost = FactoryDescription->CycleCost.ProductionCost * CycleCostCacheLevel;
		CycleCostCache.InputResources = FactoryDescription->CycleCost.InputResources;
/*
GetModifiedResources()
*/
		for (int32 ResourceIndex = 0 ; ResourceIndex < CycleCostCache.InputResources.Num() ; ResourceIndex++)
		{
			FFlareFactoryResource* Resource = &CycleCostCache.InputResources[ResourceIndex];
			Resource->Resource->Data.Identifier;

			Resource->Quantity *= CycleCostCacheLevel;
		}

		CycleCostCache.OutputResources = FactoryDescription->CycleCost.OutputResources;
		for (int32 ResourceIndex = 0 ; ResourceIndex < CycleCostCache.OutputResources.Num() ; ResourceIndex++)
		{
			FFlareFactoryResource* Resource = &CycleCostCache.OutputResources[ResourceIndex];
			Resource->Quantity *= CycleCostCacheLevel;
		}
		return CycleCostCache;
	}
}

const FFlareProductionData& UFlareFactory::GetCycleDataForShipClass(FName Class)
{
	return GetGame()->GetSpacecraftCatalog()->Get(Class)->CycleCost;
}

bool UFlareFactory::IsShipyard() const
{
	return GetDescription()->IsShipyard();
}

bool UFlareFactory::IsSmallShipyard() const
{
	return IsShipyard() && !GetDescription()->Identifier.ToString().Contains("large");
}

bool UFlareFactory::IsLargeShipyard() const
{
	return IsShipyard() && GetDescription()->Identifier.ToString().Contains("large");
}

uint32 UFlareFactory::GetProductionCost(const FFlareProductionData* Data)
{
	const FFlareProductionData* CycleData = Data ? Data : &GetCycleData();
	FCHECK(CycleData);

	ScaledProductionCost = CycleData->ProductionCost;
	float Multiplier = 1.f;
	if (Parent->IsComplexElement())
	{
		Multiplier += 0.25f;
	}
	if (!Parent->GetOwnerHasStationLicense())
	{
		Multiplier += 0.50f;
	}
	return (ScaledProductionCost * Multiplier);
}

int64 UFlareFactory::GetRemainingProductionDuration()
{
	return GetProductionTime(GetCycleData()) - FactoryData.ProductedDuration;
}

TArray<FFlareFactoryResource> UFlareFactory::GetLimitedOutputResources()
{
	UFlareCargoBay* CargoBay = Parent->GetActiveCargoBayFromFactory(this);
	TArray<FFlareFactoryResource> OutputResources = GetCycleData().OutputResources;
	for (int32 CargoLimitIndex = 0 ; CargoLimitIndex < FactoryData.OutputCargoLimit.Num() ; CargoLimitIndex++)
	{
		int32 MaxCapacity = CargoBay->GetSlotCapacity() * FactoryData.OutputCargoLimit[CargoLimitIndex].Quantity;
		FFlareResourceDescription* Resource = Game->GetResourceCatalog()->Get(FactoryData.OutputCargoLimit[CargoLimitIndex].ResourceIdentifier);
		int32 MaxAddition;
		int32 CurrentQuantity = CargoBay->GetResourceQuantity(Resource, GetParent()->GetCompany(),0,true);

		if (CurrentQuantity >= MaxCapacity)
		{
			MaxAddition = 0;
		}
		else
		{
			MaxAddition = MaxCapacity - CurrentQuantity;
		}

		for (int32 ResourceIndex = OutputResources.Num() -1 ; ResourceIndex >= 0; ResourceIndex--)
		{
			if (&OutputResources[ResourceIndex].Resource->Data == Resource)
			{
				int32 QuantityAfterEfficiency = OutputResources[ResourceIndex].Quantity * FactoryData.FactoryEfficiency;

				QuantityAfterEfficiency = FMath::Min(MaxAddition, QuantityAfterEfficiency);

				if (QuantityAfterEfficiency <= 0)
				{
					OutputResources.RemoveAt(ResourceIndex);
				}
				else
				{
					OutputResources[ResourceIndex].Quantity = QuantityAfterEfficiency;
				}

				break;
			}
		}
	}
	return OutputResources;
}

uint32 UFlareFactory::GetOutputLimit(FFlareResourceDescription* Resource)
{
	for (int32 CargoLimitIndex = 0 ; CargoLimitIndex < FactoryData.OutputCargoLimit.Num() ; CargoLimitIndex++)
	{
		if (FactoryData.OutputCargoLimit[CargoLimitIndex].ResourceIdentifier == Resource->Identifier)
		{
			return FactoryData.OutputCargoLimit[CargoLimitIndex].Quantity;
		}
	}
	FLOGV("No output limit for %s", *Resource->Identifier.ToString());
	return 0;
}

bool UFlareFactory::HasOutputLimit(FFlareResourceDescription* Resource)
{
	for (int32 CargoLimitIndex = 0 ; CargoLimitIndex < FactoryData.OutputCargoLimit.Num() ; CargoLimitIndex++)
	{
		if (FactoryData.OutputCargoLimit[CargoLimitIndex].ResourceIdentifier == Resource->Identifier)
		{
			return true;
		}
	}
	return false;
}

int32 UFlareFactory::GetInputResourcesCount()
{
	return GetCycleData().InputResources.Num();
}

int32 UFlareFactory::GetOutputResourcesCount()
{
	return GetCycleData().OutputResources.Num();
}

FFlareResourceDescription* UFlareFactory::GetInputResource(int32 Index)
{
	return &GetCycleData().InputResources[Index].Resource->Data;
}

FFlareResourceDescription* UFlareFactory::GetOutputResource(int32 Index)
{
	return &GetCycleData().OutputResources[Index].Resource->Data;
}

uint32 UFlareFactory::GetInputResourceQuantity(int32 Index)
{
	return GetCycleData().InputResources[Index].Quantity;
}

uint32 UFlareFactory::GetOutputResourceQuantity(int32 Index)
{
	return GetCycleData().OutputResources[Index].Quantity * FactoryData.FactoryEfficiency;
}

bool UFlareFactory::HasOutputResource(FFlareResourceDescription* Resource)
{
	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().OutputResources.Num() ; ResourceIndex++)
	{
		FFlareResourceDescription* ResourceCandidate = &GetCycleData().OutputResources[ResourceIndex].Resource->Data;
		if (ResourceCandidate == Resource)
		{
			return true;
		}
	}

	return false;
}

bool UFlareFactory::HasInputResource(FFlareResourceDescription* Resource)
{
	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().InputResources.Num() ; ResourceIndex++)
	{
		FFlareResourceDescription* ResourceCandidate = &GetCycleData().InputResources[ResourceIndex].Resource->Data;
		if (ResourceCandidate == Resource)
		{
			return true;
		}
	}

	return false;
}

uint32 UFlareFactory::GetInputResourceQuantity(FFlareResourceDescription* Resource)
{
	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().InputResources.Num() ; ResourceIndex++)
	{
		FFlareResourceDescription* ResourceCandidate = &GetCycleData().InputResources[ResourceIndex].Resource->Data;
		if (ResourceCandidate == Resource)
		{
			return GetCycleData().InputResources[ResourceIndex].Quantity;
		}
	}

	return 0;
}

uint32 UFlareFactory::GetInputResourceQuantityCycles(FFlareResourceDescription* Resource)
{
	for (int32 ResourceIndex = 0; ResourceIndex < GetCycleData().InputResources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* ResourceCandidate = &GetCycleData().InputResources[ResourceIndex].Resource->Data;
		if (ResourceCandidate == Resource)
		{
			if (GetCycleData().ProductionTime > 0)
			{
				return GetCycleData().InputResources[ResourceIndex].Quantity / GetCycleData().ProductionTime;
			}
		}
	}
	return -1;
}

uint32 UFlareFactory::GetOutputResourceQuantity(FFlareResourceDescription* Resource)
{
	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().OutputResources.Num() ; ResourceIndex++)
	{
		FFlareResourceDescription* ResourceCandidate = &GetCycleData().OutputResources[ResourceIndex].Resource->Data;
		if (ResourceCandidate == Resource)
		{
			return GetCycleData().OutputResources[ResourceIndex].Quantity;
		}
	}
	return 0;
}

int64 UFlareFactory::GetInputResourceDailyProductionInCredits()
{
	int64 ReturnValue = 0;

	float TotalResourceMargin = GetResourceInputMultiplier();

	for (int32 ResourceIndex = 0; ResourceIndex < GetCycleData().InputResources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* ResourceCandidate = &GetCycleData().InputResources[ResourceIndex].Resource->Data;
		ReturnValue += ((GetCycleData().InputResources[ResourceIndex].Quantity / GetCycleData().ProductionTime) * ResourceCandidate->MinPrice);
	}

	return ReturnValue;
}

float UFlareFactory::GetFactoryEfficiency()
{
	return FactoryData.FactoryEfficiency;
}

float UFlareFactory::GetFactoryEfficiency_Minimum()
{
	return Factory_Efficiency_Minimum;
}

float UFlareFactory::GetFactoryEfficiency_Maximum()
{
	return Factory_Efficiency_Maximum;
}

FText UFlareFactory::GetFactoryCycleCost(const FFlareProductionData* Data)
{
	FText ProductionCostText;
	FText CommaTextReference = UFlareGameTools::AddLeadingSpace(LOCTEXT("Comma", "+"));

	// Cycle cost in credits
	uint32 CycleProductionCost = GetProductionCost(Data);
	if (CycleProductionCost > 0)
	{
		ProductionCostText = FText::Format(LOCTEXT("ProductionCostFormat", "{0} credits"), FText::AsNumber(UFlareGameTools::DisplayMoney(CycleProductionCost)));
	}

	float TotalResourceMargin = GetResourceInputMultiplier();

	// Cycle cost in resources
	for (int ResourceIndex = 0; ResourceIndex < Data->InputResources.Num(); ResourceIndex++)
	{
		FText CommaText = ProductionCostText.IsEmpty() ? FText() : CommaTextReference;
		const FFlareFactoryResource* FactoryResource = &Data->InputResources[ResourceIndex];
		FCHECK(FactoryResource);
		int32 AdjustedQuantity = FactoryResource->Quantity * TotalResourceMargin;

		ProductionCostText = FText::Format(LOCTEXT("ProductionResourcesFormat", "{0}{1} {2} {3}"),
			ProductionCostText, CommaText, FText::AsNumber(AdjustedQuantity), FactoryResource->Resource->Data.Acronym);
	}

	return ProductionCostText;
}

FText UFlareFactory::GetFactoryCycleInfo()
{
	FText CommaTextReference = UFlareGameTools::AddLeadingSpace(LOCTEXT("Comma", "+"));
	FText ProductionOutputText;

	// No ship class selected
	if (IsShipyard() && FactoryData.TargetShipClass == NAME_None)
	{
		return LOCTEXT("SelectShipClass", "No ship in construction.");
	}

	FText ProductionCostText = GetFactoryCycleCost(&GetCycleData());

	FNumberFormattingOptions NumeralDisplayOptions;
	NumeralDisplayOptions.MaximumFractionalDigits = 0;

	// Cycle output in factory actions
	for (int ActionIndex = 0; ActionIndex < GetDescription()->OutputActions.Num(); ActionIndex++)
	{
		FText CommaText = ProductionOutputText.IsEmpty() ? FText() : CommaTextReference;
		const FFlareFactoryAction* FactoryAction = &GetDescription()->OutputActions[ActionIndex];
		FCHECK(FactoryAction);

		switch (FactoryAction->Action)
		{
			// Ship production
			case EFlareFactoryAction::CreateShip:
				ProductionOutputText = FText::Format(LOCTEXT("CreateShipActionFormat", "{0}{1} {2} {3}"),
					ProductionOutputText, CommaText, FText::AsNumber(FactoryAction->Quantity),
					GetGame()->GetSpacecraftCatalog()->Get(FactoryData.TargetShipClass)->Name);
				break;

			// Sector discovery
			case EFlareFactoryAction::DiscoverSector:
				ProductionOutputText = FText::Format(LOCTEXT("DiscoverSectorActionFormat", "{0}{1} sector"),
					ProductionOutputText, CommaText);
				break;

			// Research gain
			case EFlareFactoryAction::GainResearch:
				ProductionOutputText = FText::Format(LOCTEXT("GainResearchActionFormat", "{0}{1}{2} research"),
					ProductionOutputText, CommaText, FText::AsNumber((FactoryAction->Quantity * Parent->GetLevel()) * FactoryData.FactoryEfficiency, &NumeralDisplayOptions));
				break;

			// Build station
			case EFlareFactoryAction::BuildStation:
				ProductionOutputText = LOCTEXT("BuildStationActionFormat", "finish station construction");
				break;

			default:
				FLOGV("SFlareShipMenu::UpdateFactoryLimitsList : Unimplemented factory action %d", (FactoryAction->Action + 0));
		}
	}

	// Cycle output in resources
	for (int ResourceIndex = 0; ResourceIndex < GetCycleData().OutputResources.Num(); ResourceIndex++)
	{
		FText CommaText = ProductionOutputText.IsEmpty() ? FText() : CommaTextReference;
		const FFlareFactoryResource* FactoryResource = &GetCycleData().OutputResources[ResourceIndex];
		FCHECK(FactoryResource);

		ProductionOutputText = FText::Format(LOCTEXT("ProductionOutputFormat", "{0}{1} {2} {3}"),
		ProductionOutputText, CommaText, FText::AsNumber(FactoryResource->Quantity * FactoryData.FactoryEfficiency, &NumeralDisplayOptions), FactoryResource->Resource->Data.Acronym);
	}

	return FText::Format(LOCTEXT("FactoryCycleInfoFormat", "Production cycle : {0} \u2192 {1} in {2}"),
		ProductionCostText, ProductionOutputText,
		UFlareGameTools::FormatDate(GetProductionTime(GetCycleData()), 2));
}

FText UFlareFactory::GetFactoryStatus()
{
	FText ProductionStatusText;
	UFlareWorld* GameWorld = Game->GetGameWorld();

	if (IsActive())
	{
		if (!IsNeedProduction())
		{
			ProductionStatusText = LOCTEXT("ProductionNotNeeded", "Production not needed");
		}
		else if (HasCostReserved())
		{
			FText HasFreeSpace = HasOutputFreeSpace() ? FText() : LOCTEXT("ProductionNoSpace", ", not enough space");

			// Shipyards are a special case
			if (IsShipyard() && GetTargetShipClass() != NAME_None)
			{
				ProductionStatusText = FText::Format(LOCTEXT("ShipProductionInProgressFormat", "Building {0} for {1} ({2}{3})"),
					Game->GetSpacecraftCatalog()->Get(GetTargetShipClass())->Name,
					GameWorld->FindCompany(GetTargetShipCompany())->GetCompanyName(),
					UFlareGameTools::FormatDate(GetRemainingProductionDuration(), 2),
					HasFreeSpace);
			}
			else
			{
				ProductionStatusText = FText::Format(LOCTEXT("RegularProductionInProgressFormat", "Producing ({0}{1})"),
					UFlareGameTools::FormatDate(GetRemainingProductionDuration(), 2),
					HasFreeSpace);
			}
		}
		else if (HasInputMoney() && HasInputResources())
		{
			if (IsShipyard() && GetTargetShipClass() != NAME_None)
			{
				ProductionStatusText = FText::Format(LOCTEXT("ShipProductionWillStartFormat", "Starting building {0} for {1}"),
					Game->GetSpacecraftCatalog()->Get(GetTargetShipClass())->Name,
					GameWorld->FindCompany(GetTargetShipCompany())->GetCompanyName());
			}
			else
			{
				ProductionStatusText = LOCTEXT("ProductionWillStart", "Starting");
			}
		}
		else
		{
			if (!HasInputMoney())
			{
				if (IsShipyard() && GetTargetShipClass() != NAME_None)
				{
					ProductionStatusText = FText::Format(LOCTEXT("ShipProductionNotEnoughMoneyFormat", "Waiting for credits to build {0} for {1}"),
						Game->GetSpacecraftCatalog()->Get(GetTargetShipClass())->Name,
						GameWorld->FindCompany(GetTargetShipCompany())->GetCompanyName());
				}
				else
				{
					ProductionStatusText = LOCTEXT("ProductionNotEnoughMoney", "Waiting for credits");
				}
			}

			if (!HasInputResources())
			{
				if (IsShipyard() && GetTargetShipClass() != NAME_None)
				{
					ProductionStatusText = FText::Format(LOCTEXT("ShipProductionNotEnoughResourcesFormat", "Waiting for resources to build {0} for {1}"),
						Game->GetSpacecraftCatalog()->Get(GetTargetShipClass())->Name,
						GameWorld->FindCompany(GetTargetShipCompany())->GetCompanyName());
				}
				else
				{
					ProductionStatusText = LOCTEXT("ProductionNotEnoughResources", "Waiting for resources");
				}
			}
		}
	}
	else if (IsPaused())
	{
		ProductionStatusText = FText::Format(LOCTEXT("ProductionPaused", "Paused ({0} to completion)"),
			UFlareGameTools::FormatDate(GetRemainingProductionDuration(), 2));
	}
	else
	{
		ProductionStatusText = LOCTEXT("ProductionStopped", "Stopped");
	}

	return ProductionStatusText;
}
bool UFlareFactory::ShouldShowFactoryEfficiencyPercentage()
{
	for (int ActionIndex = 0; ActionIndex < GetDescription()->OutputActions.Num(); ActionIndex++)
	{
		const FFlareFactoryAction* FactoryAction = &GetDescription()->OutputActions[ActionIndex];
		FCHECK(FactoryAction);

		switch (FactoryAction->Action)
		{
			// Ship production
			case EFlareFactoryAction::CreateShip:
				return false;
			break;

			// Sector discovery
			case EFlareFactoryAction::DiscoverSector:
				return false;
			break;

			// Build station
			case EFlareFactoryAction::BuildStation:
				return false;
			break;
		}
	}
	return true;
}

bool UFlareFactory::IsProducing()
{
	if (IsActive())
	{
		if (!IsNeedProduction())
		{
			return false;
		}
		else if (HasCostReserved())
		{
			if(HasOutputFreeSpace())
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else if (HasInputMoney() && HasInputResources())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}


float UFlareFactory::GetMarginRatio()
{
	int64 SellPrice = 0;

	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().OutputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &GetCycleData().OutputResources[ResourceIndex];

		SellPrice += Parent->GetCurrentSector()->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::FactoryInput) * Resource->Quantity;
	}

	if(SellPrice == 0)
	{
		return 0;
	}

	float Margin = (float) GetProductionBalance() / (float) SellPrice;

	return Margin;
}

int64 UFlareFactory::GetProductionBalance()
{
	// Factory rentability

	int64 Balance = 0;
	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().InputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &GetCycleData().InputResources[ResourceIndex];

		Balance -= Parent->GetCurrentSector()->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::FactoryOutput) * Resource->Quantity;
	}

	Balance -= GetProductionCost();

	for (int32 ResourceIndex = 0 ; ResourceIndex < GetCycleData().OutputResources.Num() ; ResourceIndex++)
	{
		const FFlareFactoryResource* Resource = &GetCycleData().OutputResources[ResourceIndex];

		Balance += Parent->GetCurrentSector()->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::FactoryInput) * Resource->Quantity;
	}

	return Balance;
}

float UFlareFactory::GetProductionMalus(float Efficiency)
{
	float ReturnValue = (1.f + (1.f - Efficiency) * (MAX_DAMAGE_MALUS - 1.f));
	return ReturnValue;
}

int64 UFlareFactory::GetProductionTime(const struct FFlareProductionData& Cycle)
{
	float Efficiency = Parent->GetStationEfficiency();
	float Malus = GetProductionMalus(Efficiency);
	int64 ProductionTime = FMath::FloorToInt(Cycle.ProductionTime * Malus);
	if(FactoryDescription->IsTelescope())
	{
		return FMath::Max(int64(1), ProductionTime / Parent->GetLevel());
	}
	else
	{
		return ProductionTime;
	}
}

#undef LOCTEXT_NAMESPACE