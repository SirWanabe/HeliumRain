
#include "FlareCompany.h"
#include "Flare.h"
#include "FlareGame.h"
#include "FlareGameTools.h"
#include "FlareSector.h"
#include "FlareGameUserSettings.h"
#include "FlareScenarioTools.h"

#include "../Economy/FlareCargoBay.h"
#include "../Economy/FlareFactory.h"

#include "../Data/FlareResourceCatalog.h"
#include "../Data/FlareSpacecraftCatalog.h"
#include "../Data/FlareTechnologyCatalog.h"

#include "../Player/FlarePlayerController.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"

#include "AI/FlareCompanyAI.h"
#include "AI/FlareAIBehavior.h"
#include "../Game/FlareSectorHelper.h"

#define LOCTEXT_NAMESPACE "FlareCompany"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareCompany::UFlareCompany(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


/*----------------------------------------------------
	Save
----------------------------------------------------*/

void UFlareCompany::Load(const FFlareCompanySave& Data)
{
	Game = Cast<UFlareWorld>(GetOuter())->GetGame();
	CompanyData = Data;
	CompanyData.Identifier = FName(*GetName());

	ResearchBonuses.Add("repair-bonus", 0.f);
	ResearchBonuses.Add("diplomatic-penaltybonus", 0.f);
	ResearchBonuses.Add("diplomatic-bonus", 0.f);
	ResearchBonuses.Add("travel-bonus", 0.f);
	ResearchBonuses.Add("shipyard-fabrication-bonus", 0.f);

	// Player description ID is -1
	if (Data.CatalogIdentifier >= 0)
	{
		CompanyDescription = GetGame()->GetCompanyDescription(Data.CatalogIdentifier);
	}
	else
	{
		CompanyDescription = GetGame()->GetPlayerCompanyDescription();
	}

	// Spawn AI
	CompanyAI = NewObject<UFlareCompanyAI>(this, UFlareCompanyAI::StaticClass());
	FCHECK(CompanyAI);

	// Spawn tactic manager
	TacticManager = NewObject<UFlareTacticManager>(this, UFlareTacticManager::StaticClass());
	FCHECK(TacticManager);
	TacticManager->Load(this);

	// Load technologies
	for (int i = 0; i < CompanyData.UnlockedTechnologies.Num(); i++)
	{
		UnlockTechnology(CompanyData.UnlockedTechnologies[i], true);
	}	

	// Load ships
	for (int i = 0 ; i < CompanyData.ShipData.Num(); i++)
	{
		LoadSpacecraft(CompanyData.ShipData[i]);
	}

	// Load childstations
	for (int i = 0 ; i < CompanyData.ChildStationData.Num(); i++)
	{
		LoadSpacecraft(CompanyData.ChildStationData[i]);
	}

	// Load stations
	for (int i = 0 ; i < CompanyData.StationData.Num(); i++)
	{
		LoadSpacecraft(CompanyData.StationData[i]);
	}

	// Load destroyed spacecraft
	for (int i = 0 ; i < CompanyData.DestroyedSpacecraftData.Num(); i++)
	{
		LoadSpacecraft(CompanyData.DestroyedSpacecraftData[i]);
	}

	// Load all fleets
	for (int32 i = 0; i < CompanyData.Fleets.Num(); i++)
	{
		LoadFleet(CompanyData.Fleets[i]);
	}

	// Load emblem
	SetupEmblem();
	InvalidateCompanyValueCache();
	ClearTemporaryCaches();
}

void UFlareCompany::PostLoad()
{
	VisitedSectors.Empty();
	KnownSectors.Empty();
	CompanyTradeRoutes.Empty();
	LicenseStationSectors.Empty();

	// Load all white lists
	for (int32 i = 0; i < CompanyData.WhiteLists.Num(); i++)
	{
		LoadWhiteList(CompanyData.WhiteLists[i]);
	}

	SelectWhiteListDefault(CompanyData.DefaultWhiteListIdentifier);

	for (int i = 0; i < CompanyShips.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = CompanyShips[i];
		if (!Spacecraft->GetData().DefaultWhiteListIdentifier.IsNone())
		{
			Spacecraft->SelectWhiteListDefault(Spacecraft->GetData().DefaultWhiteListIdentifier);
		}
	}

	for (int i = 0; i < CompanyStations.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = CompanyStations[i];
		if (Spacecraft->IsStation())
		{
			if (!Spacecraft->GetData().DefaultWhiteListIdentifier.IsNone())
			{
				Spacecraft->SelectWhiteListDefault(Spacecraft->GetData().DefaultWhiteListIdentifier);
			}

			if (!(Spacecraft->IsComplexElement()))
			{
				AddOrRemoveCompanySectorStation(Spacecraft,false);
			}
		}
	}

	if (CompanyData.WhiteLists.Num() > 0)
	{
		for (int32 i = 0; i < CompanyFleets.Num(); i++)
		{
			UFlareFleet* Fleet = CompanyFleets[i];
			if (!Fleet->GetData()->DefaultWhiteListIdentifier.IsNone())
			{
				Fleet->SelectWhiteListDefault(Fleet->GetData()->DefaultWhiteListIdentifier);
			}
		}
	}

	for (int32 i = 0; i < CompanyData.Licenses.LicenseBuilding.Num(); i++)
	{
		UFlareSimulatedSector* Sector = GetGame()->GetGameWorld()->FindSector(CompanyData.Licenses.LicenseBuilding[i]);
		if (Sector)
		{
			BuyStationLicense(Sector, true);
		}
	}

	if (!CompanyData.Licenses.HasRecievedStartingLicenses)
	{
		CompanyData.Licenses.HasRecievedStartingLicenses = true;
		GiveAllStationSectorLicenses();
	}

	// Load all trade routes
	for (int32 i = 0; i < CompanyData.TradeRoutes.Num(); i++)
	{
		LoadTradeRoute(CompanyData.TradeRoutes[i]);
	}

	// Load sector knowledge
	for (int32 i = 0; i < CompanyData.SectorsKnowledge.Num(); i++)
	{
		UFlareSimulatedSector* Sector = GetGame()->GetGameWorld()->FindSector(CompanyData.SectorsKnowledge[i].SectorIdentifier);
		if (Sector)
		{
			switch (CompanyData.SectorsKnowledge[i].Knowledge) {
			case EFlareSectorKnowledge::Visited:
				VisitedSectors.Add(Sector);
				// No break
			case EFlareSectorKnowledge::Known:
				KnownSectors.Add(Sector);
				break;
			default:
				break;
			}
		}
		else
		{
			FLOGV("Fail to find known sector '%s'. Ignore it.", *CompanyData.SectorsKnowledge[i].SectorIdentifier.ToString());
		}
	}

	CompanyAI->Load(this, CompanyData.AI);
}

FFlareCompanySave* UFlareCompany::Save()
{
	CompanyData.Fleets.Empty();
	CompanyData.TradeRoutes.Empty();
	CompanyData.WhiteLists.Empty();
	CompanyData.ShipData.Empty();
	CompanyData.ChildStationData.Empty();
	CompanyData.StationData.Empty();
	CompanyData.DestroyedSpacecraftData.Empty();
	CompanyData.SectorsKnowledge.Empty();
	CompanyData.UnlockedTechnologies.Empty();
	CompanyData.Licenses.LicenseBuilding.Empty();

	TArray<FName> Keys;
	LicenseStationSectors.GetKeys(Keys);

	for (int i = 0; i < Keys.Num(); i++)
	{
		FName Identifier = Keys[i];
		UFlareSimulatedSector* Sector = LicenseStationSectors[Identifier];
		if (Sector)
		{
			CompanyData.Licenses.LicenseBuilding.Add(Sector->GetDescription()->Identifier);
		}
	}

	for (int i = 0 ; i < CompanyFleets.Num(); i++)
	{
		CompanyData.Fleets.Add(*CompanyFleets[i]->Save());
	}

	for (int i = 0 ; i < CompanyTradeRoutes.Num(); i++)
	{
		CompanyData.TradeRoutes.Add(*CompanyTradeRoutes[i]->Save());
	}

	for (int i = 0; i < CompanyWhiteLists.Num(); i++)
	{
		CompanyData.WhiteLists.Add(*CompanyWhiteLists[i]->Save());
	}

	for (int i = 0 ; i < CompanyShips.Num(); i++)
	{
		CompanyData.ShipData.Add(*CompanyShips[i]->Save());
	}

	for (int i = 0 ; i < CompanyChildStations.Num(); i++)
	{
		CompanyData.ChildStationData.Add(*CompanyChildStations[i]->Save());
	}

	for (int i = 0 ; i < CompanyStations.Num(); i++)
	{
		CompanyData.StationData.Add(*CompanyStations[i]->Save());
	}

	for (int i = 0 ; i < CompanyDestroyedSpacecrafts.Num(); i++)
	{
		CompanyData.DestroyedSpacecraftData.Add(*CompanyDestroyedSpacecrafts[i]->Save());
	}

	for (int i = 0 ; i < VisitedSectors.Num(); i++)
	{
		FFlareCompanySectorKnowledge SectorKnowledge;
		SectorKnowledge.Knowledge = EFlareSectorKnowledge::Visited;
		SectorKnowledge.SectorIdentifier = VisitedSectors[i]->GetIdentifier();
		CompanyData.SectorsKnowledge.Add(SectorKnowledge);
	}

	for (int i = 0 ; i < KnownSectors.Num(); i++)
	{
		// The visited sector are already saved
		if (!VisitedSectors.Contains(KnownSectors[i]))
		{
			FFlareCompanySectorKnowledge SectorKnowledge;
			SectorKnowledge.Knowledge = EFlareSectorKnowledge::Known;
			SectorKnowledge.SectorIdentifier = KnownSectors[i]->GetIdentifier();
			CompanyData.SectorsKnowledge.Add(SectorKnowledge);
		}
	}

	for (auto& Technology : UnlockedTechnologies)
	{
		CompanyData.UnlockedTechnologies.Add(Technology.Key);
	}

	CompanyData.CompanyValue = GetCompanyValue().TotalValue;
	CompanyData.AI = *CompanyAI->Save();

	return &CompanyData;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareCompany::Simulate()
{
	TotalDayMoneyGain = 0;
	ClearTemporaryCaches();
}

void UFlareCompany::SimulateAI(bool GlobalWar, int32 TotalReservedResources)
{
	TArray<UFlareSimulatedSpacecraft*> CompanyCarriersLocal = CompanyCarriers;
	while (CompanyCarriersLocal.Num() > 0)
	{
		int32 Index = FMath::RandRange(0, CompanyCarriersLocal.Num() - 1);
		UFlareSimulatedSpacecraft* Ship = CompanyCarriersLocal[Index];

		if (!Ship->GetDamageSystem()->IsAlive())
		{
			CompanyCarriersLocal.RemoveSwap(Ship);
			continue;
		}

		FFlareSpacecraftSave& ShipData = Ship->GetData();
		int32 CurrentCount = Ship->GetShipChildren().Num();
		TArray<FFlareResourceDescription*> InputResources;
		TArray<UFlareFactory*> Shipyards = Ship->GetShipyardFactories();

		int32 ActiveShipyards = 0;
		for (UFlareFactory* Shipyard : Shipyards)
		{
			if (Shipyard->IsProducing())
			{
				ActiveShipyards++;
			}
			for (int32 ResourceIndex = 0; ResourceIndex < Shipyard->GetInputResourcesCount(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = Shipyard->GetInputResource(ResourceIndex);
				InputResources.AddUnique(Resource);
			}
		}

		if (Ship->IsAllowAutoConstruction())
			// automatic construction of new ships
		{
			bool CheckingOrders = true;
			while (1)
			{
				if (CurrentCount + ActiveShipyards >= Ship->GetDescription()->DroneMaximum)
				{
					break;
				}

				FName SelectedName;
				if (ShipData.ShipyardOrderExternalConfig.Num() > 0)
				{
					int32 RandomIndex = FMath::RandRange(0, ShipData.ShipyardOrderExternalConfig.Num() - 1);
					SelectedName = ShipData.ShipyardOrderExternalConfig[RandomIndex];
				}
				else
				{
					break;
				}

				if (!Ship->ShipyardOrderShip(this, SelectedName, false))
				{
					break;
				}
				else
				{
					ActiveShipyards++;
				}
			}
		}

		if (Ship->IsAllowExternalOrder() && (this == this->GetGame()->GetPC()->GetCompany() && IsTechnologyUnlocked("auto-trade")) || this != this->GetGame()->GetPC()->GetCompany())
// auto resupply, player needs auto trade technology for it to work, AI Company main method is to "accidently" resupply the carrier, haphazardly with whatever is at hand locally.
		{
			UFlareFleet* Fleet = Ship->GetCurrentFleet();
			if (Fleet && !Fleet->IsTraveling())
			{
				UFlareSimulatedSector* LocalSector = Fleet->GetCurrentSector();
				if (LocalSector && !LocalSector->GetSectorBattleState(Fleet->GetFleetCompany()).HasDanger)
				{
					FFlareResourceDescription* LowestResource = NULL;
					int32 LowestResourceQuantity = 0;
					int32 MaximumCargoSlotCapacity = Ship->GetActiveCargoBay()->GetSlotCapacity();
					FText Unused;

					for (int32 CargoIndex = 0; CargoIndex < InputResources.Num(); CargoIndex++)
					{
						FFlareResourceDescription* Resource = InputResources[CargoIndex];
						bool FoundLocal = false;

						for (UFlareSimulatedSpacecraft* BuyingStation : LocalSector->GetSectorStations())
						{
							if (BuyingStation->IsUnderConstruction() || BuyingStation->IsHostile(this))
							{
								continue;
							}

							if (!BuyingStation->CanTradeWith(Ship, Unused, Resource))
							{
								continue;
							}

							if (BuyingStation->GetActiveCargoBay()->WantSell(Resource, this, true))
							{
								FoundLocal = true;
								break;
							}
						}

						if (FoundLocal)
						{
							int32 TakenResourceSpace = Ship->GetActiveCargoBay()->GetResourceQuantity(Resource, this);
							if ((!LowestResource || TakenResourceSpace < LowestResourceQuantity) && TakenResourceSpace < MaximumCargoSlotCapacity)
							{
								LowestResource = Resource;
								LowestResourceQuantity = TakenResourceSpace;
							}
						}
					}

					bool SuccessfulTrade = false;
					if (LowestResource)
					{
						UFlareSimulatedSpacecraft* BestStation = NULL;
						int32 BestStationQuantity = 0;

						UFlareSimulatedSpacecraft* BestCompanyStation = NULL;
						int32 BestStationCompanyQuantity = 0;

						for (UFlareSimulatedSpacecraft* BuyingStation : LocalSector->GetSectorStations())
						{
							if (BuyingStation->IsUnderConstruction() || BuyingStation->IsHostile(this))
							{
								continue;
							}

							if (!BuyingStation->CanTradeWith(Ship, Unused, LowestResource))
							{
								continue;
							}

							if (BuyingStation->GetActiveCargoBay()->WantSell(LowestResource, this, true))
							{
								int32 TakenResourceSpace = Ship->GetActiveCargoBay()->GetResourceQuantity(LowestResource, this);
								int32 AvailableResourceSpace = MaximumCargoSlotCapacity - TakenResourceSpace;
								int32 Stock = BuyingStation->GetActiveCargoBay()->GetResourceQuantity(LowestResource, this);
								if (Stock > AvailableResourceSpace)
								{
									Stock = AvailableResourceSpace;
								}

								if (BuyingStation->GetCompany() == this)
								{
									if (!BestCompanyStation || Stock > BestStationCompanyQuantity)
									{
										BestCompanyStation = BuyingStation;
										BestStationCompanyQuantity = Stock;
									}
								}
								else if (!BestStation || Stock > BestStationQuantity)
								{
									BestStation = BuyingStation;
									BestStationQuantity = Stock;
								}
							}
						}

						if (BestCompanyStation && !BestStation && BestStationCompanyQuantity > 0)
						{
							int32 Quantity = SectorHelper::Trade(BestCompanyStation,
								Ship,
								LowestResource,
								BestStationCompanyQuantity,
								0,
								NULL,
								false,
								1);
							if (Quantity > 0)
							{
								SuccessfulTrade = true;
							}
						}
						else if (BestCompanyStation && BestStation)
						{
							int32 ResourcePrice = LocalSector->GetTransfertResourcePrice(BestStation, Ship, LowestResource);
							int32 MaxAffordableQuantity = FMath::Max(0, int32(this->GetMoney() / ResourcePrice));
							BestStationQuantity = FMath::Min(BestStationQuantity, MaxAffordableQuantity);

							float Ratio = BestStationCompanyQuantity / BestStationQuantity;

							if (Ratio >= 0.50f)
							{
								//trade company
								int32 Quantity = SectorHelper::Trade(BestCompanyStation,
									Ship,
									LowestResource,
									BestStationCompanyQuantity,
									0,
									NULL,
									false,
									1);
								if (Quantity > 0)
								{
									SuccessfulTrade = true;
								}
							}
							else
							{
								//trade other
								int32 Quantity = SectorHelper::Trade(BestStation,
									Ship,
									LowestResource,
									BestStationQuantity,
									0,
									NULL,
									false,
									1);
								if (Quantity > 0)
								{
									SuccessfulTrade = true;
								}

							}
						}
						else if (BestStation && BestStationQuantity > 0)
						{
							int32 Quantity = SectorHelper::Trade(BestStation,
								Ship,
								LowestResource,
								BestStationQuantity,
								0,
								NULL,
								false,
								1);
							if (Quantity > 0)
							{
								SuccessfulTrade = true;
							}
						}
					}
					if (!SuccessfulTrade && this != this->GetGame()->GetPC()->GetCompany())
					{
						CompanyAI->UpdateNeedyCarrierMovement(Ship, InputResources);
					}
				}
			}
		}
		CompanyCarriersLocal.RemoveSwap(Ship);
	}

	CompanyAI->Simulate(GlobalWar, TotalReservedResources);
}

void UFlareCompany::CapturedStation(UFlareSimulatedSpacecraft* CapturedStation)
{
	if (CapturedStation)
	{
		GetAI()->CapturedStation(CapturedStation);
		CheckStationLicenseStateStation(CapturedStation);
	}
}

void UFlareCompany::NewSectorLoaded()
{
	if (CompanyAI)
	{
		CompanyAI->NewSectorLoaded();
	}
}

void UFlareCompany::SimulateActiveAI()
{
	if (CompanyAI)
	{
		CompanyAI->SimulateActiveAI();
	}
}

void UFlareCompany::ClearTemporaryCaches()
{
	LicenseStationCache.Empty();
}

EFlareHostility::Type UFlareCompany::GetPlayerHostility() const
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(Game->GetWorld()->GetFirstPlayerController());

	if (PC)
	{
		return GetHostility(PC->GetCompany());
	}

	return EFlareHostility::Neutral;
}

EFlareHostility::Type UFlareCompany::GetHostility(const UFlareCompany* TargetCompany) const
{
	if (TargetCompany == this)
	{
		return EFlareHostility::Owned;
	}
	else if (TargetCompany && CompanyData.HostileCompanies.Contains(TargetCompany->GetIdentifier()))
	{
		return EFlareHostility::Hostile;
	}

	return EFlareHostility::Neutral;
}

EFlareHostility::Type UFlareCompany::GetPlayerWarState() const
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(Game->GetWorld()->GetFirstPlayerController());

	if (PC)
	{
		return GetWarState(PC->GetCompany());
	}

	return EFlareHostility::Neutral;
}

EFlareHostility::Type UFlareCompany::GetWarState(const UFlareCompany* TargetCompany) const
{
	if (TargetCompany == this)
	{
		return EFlareHostility::Owned;
	}
	else if (GetHostility(TargetCompany) == EFlareHostility::Hostile || TargetCompany->GetHostility(this) == EFlareHostility::Hostile)
	{
		return EFlareHostility::Hostile;
	}

	return GetHostility(TargetCompany);
}

bool UFlareCompany::IsAtWar(const UFlareCompany* TargetCompany) const
{
	return GetWarState(TargetCompany) == EFlareHostility::Hostile;
}

void UFlareCompany::ClearLastWarDate()
{
	CompanyData.PlayerLastWarDate = 0;
}

void UFlareCompany::SetLastWarDate()
{
	CompanyData.PlayerLastWarDate = Game->GetGameWorld()->GetDate();
}

void UFlareCompany::ResetLastPeaceDate()
{
	CompanyData.PlayerLastPeaceDate = Game->GetGameWorld()->GetDate();
}

void UFlareCompany::ResetLastTributeDate()
{
	CompanyData.PlayerLastTributeDate = Game->GetGameWorld()->GetDate();
}

void UFlareCompany::SetHostilityTo(UFlareCompany* TargetCompany, bool Hostile)
{
	if (TargetCompany && TargetCompany != this)
	{
		bool WasHostile = CompanyData.HostileCompanies.Contains(TargetCompany->GetIdentifier());
		UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();

		if (Hostile && !WasHostile)
		{
			CompanyData.HostileCompanies.AddUnique(TargetCompany->GetIdentifier());
			if (TargetCompany == PlayerCompany)
			{
				if (PlayerCompany->GetHostility(this) != EFlareHostility::Hostile)
				{
					FString UniqueId = "war-declared-" + GetIdentifier().ToString();
					FFlareMenuParameterData Data;
					Game->GetPC()->Notify(LOCTEXT("CompanyDeclareWar", "War declared"),
						FText::Format(LOCTEXT("CompanyDeclareWarFormat", "{0} declared war on you"), FText::FromString(GetCompanyName().ToString())),
						FName(*UniqueId),
						EFlareNotification::NT_Military,
						false,
						EFlareMenu::MENU_Leaderboard,
						Data);
				}
				PlayerCompany->SetHostilityTo(this, true);
			}

			if (this == PlayerCompany && TargetCompany->GetHostility(this) != EFlareHostility::Hostile)
			{
				int32 ReputationDecrease = -50;
				int32 ReputationDecreaseOther = 0;
				int32 GameDifficulty = -1;
				GameDifficulty = Game->GetPC()->GetPlayerData()->DifficultyId;

				switch (GameDifficulty)
				{
				case -1: // Easy
					ReputationDecrease = -45;
					break;
				case 0: // Normal
					break;
				case 1: // Hard
					ReputationDecrease = -55;
					ReputationDecreaseOther = -4;
					break;
				case 2: // Very Hard
					ReputationDecrease = -60;
					ReputationDecreaseOther = -8;
					break;
				case 3: // Expert
					ReputationDecrease = -65;
					ReputationDecreaseOther = -12;
					break;
				case 4: // Unfair
					ReputationDecrease = -70;
					ReputationDecreaseOther = -16;
					break;
				}

				if (TargetCompany == Game->GetScenarioTools()->AxisSupplies)
				{
					if (!ReputationDecreaseOther)
					{
						//on normal/easy difficulty
						ReputationDecreaseOther = -4;
					}
					else
					{
						ReputationDecreaseOther *= 2;
					}
				}

				TargetCompany->GivePlayerReputation(ReputationDecrease);
				TargetCompany->GetAI()->GetData()->Pacifism = FMath::Min(50.f, TargetCompany->GetAI()->GetData()->Pacifism);
				TargetCompany->SetLastWarDate();
				
				if (ReputationDecreaseOther != 0)
				{
					for (UFlareCompany* OtherCompany : this->GetOtherCompanies())
					{
						if (OtherCompany == TargetCompany)
						{
							continue;
						}
						OtherCompany->GivePlayerReputation(ReputationDecreaseOther);
					}
				}
			}

			if (Game->GetQuestManager())
			{
				Game->GetQuestManager()->OnWarStateChanged(this, TargetCompany);
			}
		}
		else if(!Hostile && WasHostile)
		{
			CompanyData.HostileCompanies.Remove(TargetCompany->GetIdentifier());
			if(this == PlayerCompany)
			{
				TargetCompany->ResetLastPeaceDate();
			}

			if (TargetCompany == PlayerCompany)
			{
				if(PlayerCompany->GetHostility(this) == EFlareHostility::Hostile)
				{
					FFlareMenuParameterData Data;
					Game->GetPC()->Notify(LOCTEXT("CompanyWantPeace", "Peace proposed"),
						FText::Format(LOCTEXT("CompanyWantPeaceFormat", "{0} is offering peace with you"), FText::FromString(GetCompanyName().ToString())),
						FName("peace-proposed"),
						EFlareNotification::NT_Military,
						false,
						EFlareMenu::MENU_Leaderboard,
						Data);
				}
				else
				{
					FFlareMenuParameterData Data;
					Game->GetPC()->Notify(LOCTEXT("CompanyAcceptPeace", "Peace accepted"),
						FText::Format(LOCTEXT("CompanyAcceptPeaceFormat", "{0} accepted to make peace with you"), FText::FromString(GetCompanyName().ToString())),
						FName("peace-accepted"),
						EFlareNotification::NT_Military,
						false,
						EFlareMenu::MENU_Leaderboard,
						Data);
				}

				ClearLastWarDate();
			}

			if (Game->GetQuestManager())
			{
				Game->GetQuestManager()->OnWarStateChanged(this, TargetCompany);
			}
		}

		if (TargetCompany == PlayerCompany || this == PlayerCompany)
		{
			Game->GetPC()->UpdateOrbitalBattleStatesIfOpen();
		}

		UFlareSector* ActiveSector = Game->GetActiveSector();
		if (ActiveSector)
		{
			ActiveSector->GetSimulatedSector()->UpdateSectorBattleState(TargetCompany);
			ActiveSector->GetSimulatedSector()->UpdateSectorBattleState(this);
		}
	}
}

FText UFlareCompany::GetShortInfoText()
{
	// Static text
	FText ShipText = LOCTEXT("Ship", "ship");
	FText ShipsText = LOCTEXT("Ships", "ships");

	// Build
	int32 ShipCount = GetCompanyShips().Num();
	FText ShipDescriptionText = FText::Format(LOCTEXT("ShipDescriptionFormat", "{0} {1}"), FText::AsNumber(ShipCount), ShipCount != 1 ? ShipsText : ShipText);
	return FText::Format(LOCTEXT("ShortInfoFormat", "{0} ({1} credits, {2})"), GetCompanyName(), FText::AsNumber(UFlareGameTools::DisplayMoney(GetMoney())), ShipDescriptionText);
}

inline static bool SortByColor(const FLinearColor& A, const FLinearColor& B)
{
	return (A.LinearRGBToHSV().R < B.LinearRGBToHSV().R);
}

FLinearColor UFlareCompany::PickFleetColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	TArray<FLinearColor> FleetColors;
	FleetColors.Add(Theme.FriendlyColor);
	for (int32 i = 0; i < 7; ++i)
	{
		FleetColors.Add(FLinearColor((i % 6) * 60.f, 1.f, 1.f).HSVToLinearRGB());
	}
	
	return FleetColors[(CompanyFleets.Num() - 1) % (FleetColors.Num() - 1)];
}

UFlareFleet* UFlareCompany::CreateFleet(FText FleetName, UFlareSimulatedSector* FleetSector)
{
	// Create the fleet
	FFlareFleetSave FleetData;
	FleetData.Identifier = FName(*(GetIdentifier().ToString() + "-" + FString::FromInt(CompanyData.FleetImmatriculationIndex++)));
	FleetData.Name = FleetName;
	FleetData.AutoTrade = false;
	FleetData.HideTravelList = false;
	FleetData.AutoTradeStatsDays = 0;
	FleetData.AutoTradeStatsLoadResources = 0;
	FleetData.AutoTradeStatsUnloadResources = 0;
	FleetData.AutoTradeStatsMoneySell = 0;
	FleetData.AutoTradeStatsMoneyBuy = 0;

	UFlareFleet* Fleet = LoadFleet(FleetData);
	Fleet->SetCurrentSector(FleetSector);
	FleetSector->AddFleet(Fleet);
	return Fleet;
}

UFlareFleet* UFlareCompany::CreateAutomaticFleet(UFlareSimulatedSpacecraft* Spacecraft)
{
	FText FleetName;
	int32 FleetIndex = 1;

	while(true)
	{
		if (Spacecraft->IsMilitary())
		{
			if (Spacecraft->GetDescription()->Size == EFlarePartSize::L)
			{
				FleetName = FText::Format(LOCTEXT("CombatFleetFormat", "Combat Fleet {0}"),
					FText::FromString(AFlareGame::ConvertToRoman(FleetIndex)));
			}
			else
			{
				FleetName = FText::Format(LOCTEXT("WarFleetFormat", "War Fleet {0}"),
					FText::FromString(AFlareGame::ConvertToRoman(FleetIndex)));
			}
		}
		else
		{
			FleetName = FText::Format(LOCTEXT("CivilianFleetFormat", "Trade Fleet {0}"),
				FText::AsNumber(FleetIndex));
		}

		// Check duplicate
		bool Duplicate = false;
		for (int i = 0 ; i < CompanyFleets.Num(); i++)
		{
			if (FleetName.CompareTo(CompanyFleets[i]->GetFleetName()) == 0)
			{
				Duplicate = true;
				break;
			}
		}

		if (Duplicate)
		{
			FleetIndex++;
		}
		else
		{
			break;
		}
	}
	
	UFlareFleet* NewFleet = CreateFleet(FleetName, Spacecraft->GetCurrentSector());
	NewFleet->AddShip(Spacecraft);
	NewFleet->SetFleetColor(PickFleetColor());

	return NewFleet;
}

UFlareFleet* UFlareCompany::LoadFleet(const FFlareFleetSave& FleetData)
{
	UFlareFleet* Fleet = NULL;

	// Create the new travel
	Fleet = NewObject<UFlareFleet>(this, UFlareFleet::StaticClass());
	Fleet->Load(FleetData);
	CompanyFleets.AddUnique(Fleet);

	//FLOGV("UFlareWorld::LoadFleet : loaded fleet '%s'", *Fleet->GetFleetName().ToString());

	return Fleet;
}

void UFlareCompany::RemoveFleet(UFlareFleet* Fleet)
{
	CompanyFleets.Remove(Fleet);
}

void UFlareCompany::MoveFleetUp(UFlareFleet* Fleet)
{
	int32 Index = CompanyFleets.IndexOfByKey(Fleet);

	if(Index != INDEX_NONE && Index > 0)
	{
		int32 SwapIndex = Index - 1;
		UFlareFleet* SwapFleet = CompanyFleets[SwapIndex];

		 CompanyFleets[SwapIndex] = Fleet;
		 CompanyFleets[Index] = SwapFleet;
	}
}

void UFlareCompany::MoveFleetDown(UFlareFleet* Fleet)
{
	int32 Index = CompanyFleets.IndexOfByKey(Fleet);

	if(Index != INDEX_NONE && Index < CompanyFleets.Num() - 1)
	{
		int32 SwapIndex = Index + 1;
		UFlareFleet* SwapFleet = CompanyFleets[SwapIndex];

		 CompanyFleets[SwapIndex] = Fleet;
		 CompanyFleets[Index] = SwapFleet;
	}
}

UFlareCompanyWhiteList* UFlareCompany::CreateCompanyWhiteList(FText WhiteListName)
{
	FFlareWhiteListSave WhiteListData;

	WhiteListData.Identifier = FName(*(GetIdentifier().ToString() + "-" + FString::FromInt(CompanyData.WhiteListImmatriculationIndex++)));
	WhiteListData.Name = WhiteListName;

	UFlareCompanyWhiteList* NewWhiteList = LoadWhiteList(WhiteListData);

	for (int CompanyIndex = 0; CompanyIndex < Game->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
		UFlareCompany* FoundCompany = Game->GetGameWorld()->GetCompanies()[CompanyIndex];
		NewWhiteList->AddNewCompanyDataSave(FoundCompany->GetIdentifier());
	}
	return NewWhiteList;
}

UFlareCompanyWhiteList* UFlareCompany::LoadWhiteList(const FFlareWhiteListSave& WhiteListData)
{
	UFlareCompanyWhiteList* NewCompanyWhiteList = NULL;
	NewCompanyWhiteList = NewObject<UFlareCompanyWhiteList>(this, UFlareCompanyWhiteList::StaticClass());
	NewCompanyWhiteList->Load(WhiteListData, Game);
	CompanyWhiteLists.AddUnique(NewCompanyWhiteList);
	return NewCompanyWhiteList;
}

void UFlareCompany::DeleteCompanyWhiteList(UFlareCompanyWhiteList* DeletingWhiteList)
{
	CompanyWhiteLists.Remove(DeletingWhiteList);
	if (CompanySelectedWhiteList == DeletingWhiteList)
	{
		CompanyData.DefaultWhiteListIdentifier = FName();
		CompanySelectedWhiteList = nullptr;
	}
	DeletingWhiteList->OnDelete();
}

void UFlareCompany::SelectWhiteListDefault(FName IdentifierSearch)
{
	UFlareCompanyWhiteList* FoundWhiteList = GetWhiteList(IdentifierSearch);
	SelectWhiteListDefault(FoundWhiteList);
}

void UFlareCompany::SelectWhiteListDefault(UFlareCompanyWhiteList* NewWhiteList)
{
	if (NewWhiteList)
	{
		CompanyData.DefaultWhiteListIdentifier = NewWhiteList->GetWhiteListIdentifier();
		CompanySelectedWhiteList = NewWhiteList;
	}
	else
	{
		CompanyData.DefaultWhiteListIdentifier = FName();
		CompanySelectedWhiteList = nullptr;
	}
}

UFlareCompanyWhiteList* UFlareCompany::GetWhiteList(FName IdentifierSearch)
{
	for (int32 WhiteListIndex = 0; WhiteListIndex < GetWhiteLists().Num(); WhiteListIndex++)
	{
		UFlareCompanyWhiteList* CurrentWhiteList = GetWhiteLists()[WhiteListIndex];
		if (CurrentWhiteList->GetWhiteListIdentifier() == IdentifierSearch)
		{
			return CurrentWhiteList;
		}
	}
	return nullptr;
}

bool UFlareCompany::CanTradeWhiteListFrom(UFlareCompany* OtherCompany, FFlareResourceDescription* Resource)
{
	if (!OtherCompany)
	{
		return true;
	}
	UFlareCompanyWhiteList* ActiveWhiteList = GetCompanySelectedWhiteList();
	if (ActiveWhiteList)
	{
		FFlareWhiteListCompanyDataSave* CompanyData = ActiveWhiteList->GetCompanyDataFor(OtherCompany);
		if (CompanyData)
		{
			FText Reason;
			if (!ActiveWhiteList->CanCompanyDataTradeTo(CompanyData, Resource, Reason))
			{
				return false;
			}
		}
	}
	return true;
}

bool UFlareCompany::CanTradeWhiteListTo(UFlareCompany* OtherCompany, FFlareResourceDescription* Resource)
{
	if (!OtherCompany)
	{
		return true;
	}
	UFlareCompanyWhiteList* ActiveWhiteList = GetCompanySelectedWhiteList();
	if (ActiveWhiteList)
	{
		FFlareWhiteListCompanyDataSave* CompanyData = ActiveWhiteList->GetCompanyDataFor(OtherCompany);
		if (CompanyData)
		{
			FText Reason;
			if (!ActiveWhiteList->CanCompanyDataTradeFrom(CompanyData, Resource, Reason))
			{
				return false;
			}
		}
	}
	return true;
}


UFlareTradeRoute* UFlareCompany::CreateTradeRoute(FText TradeRouteName)
{
	// Create the trade route
	FFlareTradeRouteSave TradeRouteData;
	TradeRouteData.Identifier = FName(*(GetIdentifier().ToString() + "-" + FString::FromInt(CompanyData.TradeRouteImmatriculationIndex++)));
	TradeRouteData.Name = TradeRouteName;
	TradeRouteData.TargetSectorIdentifier = NAME_None;
	TradeRouteData.CurrentOperationIndex = 0;
	TradeRouteData.CurrentOperationProgress = 0;
	TradeRouteData.CurrentOperationDuration = 0;
	TradeRouteData.IsPaused = false;
	UFlareTradeRoute* TradeRoute = LoadTradeRoute(TradeRouteData);
	TradeRoute->ResetStats();
	return TradeRoute;
}


UFlareTradeRoute* UFlareCompany::LoadTradeRoute(const FFlareTradeRouteSave& TradeRouteData)
{
	UFlareTradeRoute* TradeRoute = NULL;

	// Create the new travel
	TradeRoute = NewObject<UFlareTradeRoute>(this, UFlareTradeRoute::StaticClass());
	TradeRoute->Load(TradeRouteData);
	CompanyTradeRoutes.AddUnique(TradeRoute);

	//FLOGV("UFlareCompany::LoadTradeRoute : loaded trade route '%s'", *TradeRoute->GetTradeRouteName().ToString());

	return TradeRoute;
}

void UFlareCompany::RemoveTradeRoute(UFlareTradeRoute* TradeRoute)
{
	CompanyTradeRoutes.Remove(TradeRoute);
}

void UFlareCompany::CreatedSpaceCraft(UFlareSimulatedSpacecraft* Spacecraft)
{
	if (!Spacecraft)
	{
		return;
	}
	if (Spacecraft->IsStation())
	{
		if (Spacecraft->IsComplexElement())
		{
		}
		else
		{
			AddOrRemoveCompanySectorStation(Spacecraft, false);
		}
	}
}

UFlareSimulatedSpacecraft* UFlareCompany::LoadSpacecraft(const FFlareSpacecraftSave& SpacecraftData)
{
	UFlareSimulatedSpacecraft* Spacecraft = NULL;
	//FLOGV("UFlareCompany::LoadSpacecraft ('%s')", *SpacecraftData.Immatriculation.ToString());

	FFlareSpacecraftDescription* Desc = Game->GetSpacecraftCatalog()->Get(SpacecraftData.Identifier);
	if (Desc)
	{
		Spacecraft = NewObject<UFlareSimulatedSpacecraft>(this, UFlareSimulatedSpacecraft::StaticClass());

		if(FindSpacecraft(SpacecraftData.Immatriculation))
		{
			FFlareSpacecraftSave FixedSpacecraftData = SpacecraftData;
			Game->Immatriculate(this, SpacecraftData.Identifier, &FixedSpacecraftData, SpacecraftData.AttachComplexStationName != NAME_None);
			FLOGV("WARNING : Double immatriculation, fix that. New immatriculation is %s", *FixedSpacecraftData.Immatriculation.ToString());
			Spacecraft->Load(FixedSpacecraftData);
		}
		else
		{
			Spacecraft->Load(SpacecraftData);
		}

		if(Spacecraft->IsDestroyed())
		{
			CompanyDestroyedSpacecrafts.AddUnique(Spacecraft);
		}
		else
		{
			if (Spacecraft->IsStation())
			{
				if (Spacecraft->GetDescription()->IsTelescope())
				{
					CompanyTelescopes.AddUnique(Spacecraft);
				}

				if(Spacecraft->IsComplexElement())
				{
					CompanyChildStations.AddUnique(Spacecraft);
				}
				else
				{
					CompanyStations.AddUnique(Spacecraft);
				}
			}
			else
			{
				if (Spacecraft->GetDescription()->IsDroneCarrier)
				{
					CompanyCarriers.AddUnique(Spacecraft);
				}

				CompanyShips.AddUnique(Spacecraft);
			}

			if (!Spacecraft->IsComplexElement())
			{
				CompanySpacecrafts.AddUnique((Spacecraft));
				if (CompanySpacecraftsCache.Contains(Spacecraft->GetImmatriculation()))
				{
					CompanySpacecraftsCache[Spacecraft->GetImmatriculation()]=Spacecraft;
				}
				else
				{
					CompanySpacecraftsCache.Add(Spacecraft->GetImmatriculation(), Spacecraft);
				}
			}
		}
	}
	else
	{
		FLOG("UFlareCompany::LoadSpacecraft : Failed (no description available)");
	}

	return Spacecraft;
}

void UFlareCompany::DestroySpacecraft(UFlareSimulatedSpacecraft* Spacecraft)
{
	FLOGV("UFlareCompany::DestroySpacecraft : Remove %s from company %s", *Spacecraft->GetImmatriculation().ToString(), *GetCompanyName().ToString());

	// Destroy child first
	if(Spacecraft->IsComplex())
	{
		for(UFlareSimulatedSpacecraft* Child: Spacecraft->GetComplexChildren())
		{
			DestroySpacecraft(Child);
		}
	}

	if (Spacecraft->IsStation())
	{
		AddOrRemoveCompanySectorStation(Spacecraft,true);
		Spacecraft->GetCompany()->GetAI()->FinishedConstruction(Spacecraft);
	}

	Spacecraft->ResetCapture();

	CompanySpacecraftsCache.Remove(Spacecraft->GetImmatriculation());
	CompanySpacecrafts.Remove(Spacecraft);
	CompanyStations.Remove(Spacecraft);
	CompanyChildStations.Remove(Spacecraft);
	CompanyTelescopes.Remove(Spacecraft);
	CompanyShips.Remove(Spacecraft);


	if (Spacecraft->GetDescription()->IsDroneCarrier)
	{
		CompanyCarriers.Remove(Spacecraft);
	}

	if (Spacecraft->GetShipMaster())
	{
		Spacecraft->SetOwnerShip(NULL);
	}

	TArray<UFlareSimulatedSpacecraft*> StepChildren = Spacecraft->GetShipChildren();
	if (StepChildren.Num() > 0)
	{
		//try to migrate any subordinate ships (drones)
		for (UFlareSimulatedSpacecraft* OwnedShips : StepChildren)
		{
			if (OwnedShips->GetShipMaster() == Spacecraft)
			{
 				OwnedShips->TryMigrateDrones();
			}
		}
		StepChildren.Empty();
	}

	if (Spacecraft->GetCurrentFleet())
	{
		Spacecraft->GetCurrentFleet()->RemoveShip(Spacecraft, true);
	}

	if (Spacecraft->GetCurrentSector())
	{
		Spacecraft->GetCurrentSector()->RemoveSpacecraft(Spacecraft);
	}

	GetGame()->GetGameWorld()->ClearFactories(Spacecraft);
	CompanyAI->DestroySpacecraft(Spacecraft);
	Spacecraft->SetDestroyed(true);
	CompanyDestroyedSpacecrafts.Add(Spacecraft);
}

void UFlareCompany::DiscoverSector(UFlareSimulatedSector* Sector, bool VisitedSector)

{
	if (Sector)
	{
		KnownSectors.AddUnique(Sector);
		if (VisitedSector)
			//visited sector bool for randomize station location game option
		{
			VisitedSectors.AddUnique(Sector);
		}
	}
}

void UFlareCompany::VisitSector(UFlareSimulatedSector* Sector)
{
	DiscoverSector(Sector);
	VisitedSectors.AddUnique(Sector);
	if (GetGame()->GetQuestManager())
	{
		GetGame()->GetQuestManager()->OnSectorVisited(Sector);
	}
}

bool UFlareCompany::TakeMoney(int64 Amount, bool AllowDepts, FFlareTransactionLogEntry TransactionContext)
{
	if (Amount < 0 || (Amount > CompanyData.Money && !AllowDepts))
	{
		FLOGV("UFlareCompany::TakeMoney : Failed to take %f money from %s (balance: %f)",
			Amount/100., *GetCompanyName().ToString(), CompanyData.Money/100.);
		return false;
	}
	else
	{
		CompanyData.Money -= Amount;
		TotalDayMoneyGain -= Amount;

		/*if (Amount > 0)
		{

			FLOGV("$ %s - %lld -> %llu", *GetCompanyName().ToString(), Amount, CompanyData.Money);
		}*/

		if (this == Game->GetPC()->GetCompany())
		{
			TransactionContext.Amount = -Amount;
			TransactionContext.Date = GetGame()->GetGameWorld()->GetDate();
			CompanyData.TransactionLog.Push(TransactionContext);
		}

		InvalidateCompanyValueCache();

		return true;
	}
}

void UFlareCompany::GiveMoney(int64 Amount, FFlareTransactionLogEntry TransactionContext)
{
	if (Amount < 0)
	{
		FLOGV("UFlareCompany::GiveMoney : Failed to give %f money from %s (balance: %f)",
			Amount/100., *GetCompanyName().ToString(), CompanyData.Money/100.);
		return;
	}

	CompanyData.Money += Amount;
	TotalDayMoneyGain += Amount;

	if (this == Game->GetPC()->GetCompany() && GetGame()->GetQuestManager())
	{
		GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("gain-money").PutInt32("amount", Amount));
	}

	if (this == Game->GetPC()->GetCompany())
	{
		TransactionContext.Amount = Amount;
		TransactionContext.Date = GetGame()->GetGameWorld()->GetDate();
		CompanyData.TransactionLog.Push(TransactionContext);
	}

	InvalidateCompanyValueCache();

	/*if (Amount > 0)
	{
		FLOGV("$ %s + %lld -> %llu", *GetCompanyName().ToString(), Amount, CompanyData.Money);
	}*/
}

void UFlareCompany::GiveResearch(int64 Amount)
{
	if (Amount < 0)
	{
		FLOGV("UFlareCompany::GiveMoney : Failed to give %d research from %s (balance: %d)", Amount, *GetCompanyName().ToString(), CompanyData.ResearchAmount);
		return;
	}


	if (this == Game->GetPC()->GetCompany())
	{
		CompanyData.ResearchAmount += Amount;
		if (GetGame()->GetQuestManager())
		{
			GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("gain-research").PutInt32("amount", Amount));
		}
	}
	else
	{
		bool AICheats = Game->GetPC()->GetPlayerData()->AICheats;
		if (AICheats)
		{
			// non-player company gains a research bonus
			int32 GameDifficulty = -1;
			GameDifficulty = Game->GetPC()->GetPlayerData()->DifficultyId;
			float Multiplier = 1.00f;
			switch (GameDifficulty)
			{
			case -1: // Easy
				Multiplier = 1.00f;
				break;
			case 0: // Normal
				Multiplier = 1.00f;
				break;
			case 1: // Hard
				Multiplier = 1.10f;
				break;
			case 2: // Very Hard
				Multiplier = 1.25f;
				break;
			case 3: // Expert
				Multiplier = 1.40f;
				break;
			case 4: // Unfair
				Multiplier = 1.55f;
				break;
			}

			CompanyData.ResearchAmount += Amount * Multiplier;
		}
		else
		{
			CompanyData.ResearchAmount += Amount;
		}
	}

}

void UFlareCompany::GivePlayerReputation(float Amount, float Max)
{
	if(Max < -100)
	{
		CompanyData.PlayerReputation = FMath::Max(-100.f, CompanyData.PlayerReputation + Amount);
	}
	else
	{
		// Clamp
		float MaxReputation = FMath::Max(Max, CompanyData.PlayerReputation);

		if (Amount < 0)
		{
			float TechnologyBonus = this->IsTechnologyUnlocked("diplomacy") ? 0.5f : 1.f;
			float TechnologyBonusSecondary = this->GetTechnologyBonus("diplomatic-penaltybonus");
			TechnologyBonus -= TechnologyBonusSecondary;
			Amount *= TechnologyBonus;
		}
		else
		{
			float TechnologyBonus = 1.f;
			float TechnologyBonusSecondary = this->GetTechnologyBonus("diplomatic-bonus");
			TechnologyBonus += TechnologyBonusSecondary;
			Amount *= TechnologyBonus;
		}
		CompanyData.PlayerReputation = FMath::Max(-100.f, CompanyData.PlayerReputation + Amount);
		CompanyData.PlayerReputation = FMath::Min(MaxReputation, CompanyData.PlayerReputation);
	}
}

void UFlareCompany::GivePlayerReputationToOthers(float Amount)
{
	for (UFlareCompany* OtherCompany : this->GetOtherCompanies())
//	for (int32 CompanyIndex = 0; CompanyIndex < Game->GetGameWorld()->GetCompanies().Num(); CompanyIndex++)
	{
//		UFlareCompany* OtherCompany = Game->GetGameWorld()->GetCompanies()[CompanyIndex];

//		if (OtherCompany != this)
//		{
		OtherCompany->GivePlayerReputation(Amount);
//		}
	}
}


float UFlareCompany::ComputeCompanyDiplomaticWeight()
{
	CompanyValue Value = GetCompanyValue();
	return (Value.TotalValue + Value.ArmyCurrentCombatPoints * 10);
}

bool UFlareCompany::WantWarWith(UFlareCompany* TargetCompany)
{
	if(this == Game->GetScenarioTools()->AxisSupplies || TargetCompany == Game->GetScenarioTools()->AxisSupplies)
	{
		return false;
	}

	UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();

	if(TargetCompany == PlayerCompany)
	{
		if(GetPlayerReputation() <= 0)
		{
			return true;
		}
	}
	else
	{
		int32 GameDifficulty = -1;
		GameDifficulty = Game->GetPC()->GetPlayerData()->DifficultyId;
		float RequiredMultiplier = 1;

		switch (GameDifficulty)
		{
		case -1: // Easy
			RequiredMultiplier = 0.20;
			break;
		case 0: // Normal
			RequiredMultiplier = 0.25;
			break;
		case 1: // Hard
			RequiredMultiplier = 0.275;
			break;
		case 2: // Very Hard
			RequiredMultiplier = 0.3025;
			break;
		case 3: // Expert
			RequiredMultiplier = 0.33275;
			break;
		case 4: // Unfair
			RequiredMultiplier = 0.366025;
			break;
		}

		if (PlayerCompany->GetCompanyValue().ArmyCurrentCombatPoints < (Game->GetGameWorld()->GetTotalWorldCombatPoint() * RequiredMultiplier))
		{
			float MyWeight = ComputeCompanyDiplomaticWeight();
			return TargetCompany->ComputeCompanyDiplomaticWeight() > MyWeight;
		}
	}

	return false;
}

//#define DEBUG_CONFIDENCE
float UFlareCompany::GetConfidenceLevel(UFlareCompany* TargetCompany, TArray<UFlareCompany*>& Allies)
{
	// Confidence level go from 1 to -1
	// 1 if if the army value of the company and its potential allies is infinite compared to the opposent
	// -1 if if the army value of the company and its potential allies is zero compared to the opposent
	//
	// The enemies are people at war with me, the reference company if provide, people than want war with me but are not allied
	// The allies are all the people at war the reference company or that want war with Reference company

	UFlareCompany* PlayerCompany = Game->GetPC()->GetCompany();

	Allies.Empty();
	// Find allies
	for (UFlareCompany* CompanyCandidate : TargetCompany->GetOtherCompanies())
	{
		if(IsAtWar(CompanyCandidate))
		{
			continue;
		}

		if (CompanyCandidate == this)
		{
			Allies.Add(CompanyCandidate);
		}
		else if (CompanyCandidate->IsAtWar(TargetCompany))
		{
			Allies.Add(CompanyCandidate);
		}
		else if (CompanyCandidate != PlayerCompany && CompanyCandidate->GetAI()->GetData()->Pacifism == 0 && CompanyCandidate->GetWarCount(PlayerCompany) == 0 )
		{
			// Open to war
#ifdef DEBUG_CONFIDENCE
			FLOGV("GetConfidenceLevel ally %s to %s : %s want war ? %d",
				  *GetCompanyName().ToString(),
				  *TargetCompany->GetCompanyName().ToString(),
				  * CompanyCandidate->GetCompanyName().ToString(),
				  CompanyCandidate->WantWarWith(TargetCompany));
#endif
			if (CompanyCandidate->WantWarWith(TargetCompany))
			{
					Allies.Add(CompanyCandidate);
			}
		}
	}

	TArray<UFlareCompany*> Enemies;

	// Find enemies
	for (UFlareCompany* OtherCompany : GetOtherCompanies())
	{
		if (OtherCompany == TargetCompany)
		{
			Enemies.Add(TargetCompany);
			continue;
		}

		bool IsEnemy = false;

		if (IsAtWar(OtherCompany))
		{
#ifdef DEBUG_CONFIDENCE
			FLOGV("GetConfidenceLevel enemy %s to %s : %s at war ? %d",
				  *GetCompanyName().ToString(),
				  *TargetCompany->GetCompanyName().ToString(),
				  *OtherCompany->GetCompanyName().ToString(),
				  IsAtWar(OtherCompany));
#endif
			IsEnemy = true;
		}
		else if (OtherCompany != PlayerCompany && OtherCompany->GetAI()->GetData()->Pacifism == 0 && OtherCompany->GetWarCount(PlayerCompany) == 0 )
		{
#ifdef DEBUG_CONFIDENCE
			FLOGV("GetConfidenceLevel enemy %s to %s : %s want war ? %d",
				  *GetCompanyName().ToString(),
				  *TargetCompany->GetCompanyName().ToString(),
				  *OtherCompany->GetCompanyName().ToString(),
				  OtherCompany->WantWarWith(this));
#endif
			if (OtherCompany->WantWarWith(this) && !Allies.Contains(OtherCompany))
			{
				IsEnemy = true;
			}
		}

		if (IsEnemy)
		{
			Enemies.Add(OtherCompany);
		}
	}

	// Compute army values
	int32 EnemiesArmyCombatPoints = 0;
	int32 AlliesArmyCombatPoints = 0;
#ifdef DEBUG_CONFIDENCE
	FLOGV("Compute confidence for %s (ref: %s)", *GetCompanyName().ToString(), (TargetCompany ? *TargetCompany->GetCompanyName().ToString(): *FString("none")));
#endif
	for (int32 EnemyIndex = 0; EnemyIndex < Enemies.Num(); EnemyIndex++)
	{
		UFlareCompany* EnemyCompany = Enemies[EnemyIndex];
		EnemiesArmyCombatPoints += EnemyCompany->GetCompanyValue().ArmyCurrentCombatPoints;
#ifdef DEBUG_CONFIDENCE
		FLOGV("- enemy: %s (%d)", *EnemyCompany->GetCompanyName().ToString(), EnemyCompany->GetCompanyValue().ArmyCurrentCombatPoints);
#endif
	}

	for (int32 AllyIndex = 0; AllyIndex < Allies.Num(); AllyIndex++)
	{
		UFlareCompany* AllyCompany = Allies[AllyIndex];

		// Allies can be enemy to only a part of my ennemie. Cap to the value of the enemy if not me
		int32 AllyArmyCombatPoints= AllyCompany->GetCompanyValue().ArmyCurrentCombatPoints;
#ifdef DEBUG_CONFIDENCE
		FLOGV("- ally: %s (%d)", *AllyCompany->GetCompanyName().ToString(), AllyArmyCombatPoints);
#endif
		AlliesArmyCombatPoints += AllyArmyCombatPoints;
	}
#ifdef EnemiesArmyCombatPoints
	FLOGV("EnemiesArmyCombatPoints=%d AlliesArmyCombatPoints=%d", EnemiesArmyCombatPoints, AlliesArmyCombatPoints);
#endif
	// Compute confidence
	if(AlliesArmyCombatPoints == EnemiesArmyCombatPoints)
	{
		return 0;
	}
	else if(AlliesArmyCombatPoints > EnemiesArmyCombatPoints)
	{
		if(EnemiesArmyCombatPoints == 0)
		{
			return 1;
		}

		float Ratio =  (float) AlliesArmyCombatPoints /  (float) EnemiesArmyCombatPoints;
		float Confidence = 1.f-1.f / (Ratio + 1);
#ifdef DEBUG_CONFIDENCE
	FLOGV("Ratio=%f Confidence=%f", Ratio, Confidence);
#endif
		return Confidence;
	}
	else
	{
		if(AlliesArmyCombatPoints == 0)
		{
			return -1;
		}

		float Ratio =  (float) EnemiesArmyCombatPoints /  (float) AlliesArmyCombatPoints;
		float Confidence = 1.f-1.f / (Ratio + 1);
#ifdef DEBUG_CONFIDENCE
	FLOGV("Ratio=%f Confidence=%f", Ratio, -Confidence);
#endif
		return -Confidence;
	}

}

bool UFlareCompany::AtWar()
{
	for (UFlareCompany* OtherCompany : Game->GetGameWorld()->GetCompanies())
	{
		if(OtherCompany == this)
		{
			continue;
		}

		if(GetWarState(OtherCompany) == EFlareHostility::Hostile)
		{
			return true;
		}
	}
	return false;
}

int32 UFlareCompany::GetTransportCapacity()
{
	int32 CompanyCapacity = 0;

	for(UFlareSimulatedSpacecraft* Ship : CompanyShips)
	{
		if(Ship->GetDamageSystem()->IsStranded())
		{
			continue;
		}

		CompanyCapacity += Ship->GetActiveCargoBay()->GetCapacity();
	}

	return CompanyCapacity;
}

TArray<UFlareCompany*> UFlareCompany::GetOtherCompanies(bool Shuffle)
{
	if (!OtherCompaniesCache.Num())
	{
		TArray<UFlareCompany*> GameCompanies = GetGame()->GetGameWorld()->GetCompanies();
		OtherCompaniesCache.Reserve(GameCompanies.Num() - 1);

		for (UFlareCompany* Company : GameCompanies)
		{
			if (Company != this)
			{
				OtherCompaniesCache.Add(Company);
			}
		}
		/*
		Game->GetPC()->Notify(
			LOCTEXT("NewCache", "Company info cached"),
			LOCTEXT("NewCache", "Company info cached"),
			"companyfirstcache",
			EFlareNotification::NT_Info);

*/
	}
/*
	else
	{

		Game->GetPC()->Notify(
			LOCTEXT("CacheExists", "Company cached exists"),
			LOCTEXT("CacheExists", "Company cached exists"),
			"companysecondcache",
			EFlareNotification::NT_Info);
	}
*/
	if(Shuffle)
	{

		TArray<UFlareCompany*> ShuffleCompanies;
		ShuffleCompanies.Reserve(OtherCompaniesCache.Num());

		for (UFlareCompany* Company : OtherCompaniesCache)
		{
			ShuffleCompanies.Add(Company);
		}

		int32 LastIndex = ShuffleCompanies.Num() -1;

		for (int32 i = 0; i < LastIndex; ++i)
		{
			int32 Index = FMath::RandRange(0, LastIndex);
			if (i != Index)
			{
				ShuffleCompanies.Swap(i, Index);
			}
		}

/*
		TArray<UFlareCompany*> OtherCompanies;
		TArray<UFlareCompany*> ShuffleCompanies;

		OtherCompanies.Reserve(OtherCompaniesCache.Num());
		ShuffleCompanies.Reserve(OtherCompaniesCache.Num());

		for (UFlareCompany* Company : OtherCompaniesCache)
		{
			OtherCompanies.Add(Company);
		}

		while(OtherCompanies.Num())
		{
			int32 Index = FMath::RandRange(0, OtherCompanies.Num() - 1);
			ShuffleCompanies.Add(OtherCompanies[Index]);
//			OtherCompanies.RemoveAt(Index);
			OtherCompanies.RemoveAtSwap(Index);
		}
*/
/*
		Game->GetPC()->Notify(
			LOCTEXT("Shuffled", "Returned Shuffled Cache"),
			FText::Format(LOCTEXT("Shuffled", "Shuffle returned {0}"),
			ShuffleCompanies.Num()),
			"shuffle",
			EFlareNotification::NT_Info);
			*/
		return ShuffleCompanies;
	}
	else
	{
/*
		Game->GetPC()->Notify(
			LOCTEXT("Existing", "Returned Existing Cache"),
			FText::Format(LOCTEXT("Existing", "Returned Existing Cache {0}"),
			OtherCompaniesCache.Num()),
			"Existing",
			EFlareNotification::NT_Info);
*/
		return OtherCompaniesCache;
	}
}

bool UFlareCompany::HasKnowResourceInput(FFlareResourceDescription* Resource)
{
	for(UFlareSimulatedSector* Sector : VisitedSectors)
	{
		for(UFlareSimulatedSpacecraft* Station : Sector->GetSectorStations())
		{
			FFlareResourceUsage StationResourceUsage = Station->GetResourceUseType(Resource);

			if(StationResourceUsage.HasUsage(EFlareResourcePriceContext::FactoryInput) &&
				StationResourceUsage.HasUsage(EFlareResourcePriceContext::ConsumerConsumption) &&
				StationResourceUsage.HasUsage(EFlareResourcePriceContext::MaintenanceConsumption))
			{
				continue;
			}

			FLOGV("HasKnowResourceBuyer: %s want buy %s in %s",
				  *Station->GetImmatriculation().ToString(),
				  *Resource->Name.ToString(),
				  *Sector->GetSectorName().ToString())
			return true;
		}
	}

	return false;
}

bool UFlareCompany::HasKnowResourceOutput(FFlareResourceDescription* Resource)
{
	for(UFlareSimulatedSector* Sector : VisitedSectors)
	{
		for(UFlareSimulatedSpacecraft* Station : Sector->GetSectorStations())
		{
			FFlareResourceUsage StationResourceUsage = Station->GetResourceUseType(Resource);

			if(StationResourceUsage.HasUsage(EFlareResourcePriceContext::FactoryOutput))
			{
				continue;
			}

			FLOGV("HasKnowResourceSeller: %s want sell %s in %s",
				  *Station->GetImmatriculation().ToString(),
				  *Resource->Name.ToString(),
				  *Sector->GetSectorName().ToString())
			return true;
		}
	}

	return false;
}

int32 UFlareCompany::GetWarCount(UFlareCompany* ExcludeCompany) const
//replaced
{
	{
		int32 WarCount = 0;

		for (UFlareCompany* OtherCompany : Game->GetGameWorld()->GetCompanies())
		{
			if (OtherCompany == this)// || OtherCompany == ExcludeCompany)
			{
				continue;
			}

			if (IsAtWar(OtherCompany))
			{
				++WarCount;
			}
		}

		return WarCount;
	}

	//	int32 WarCount = CompanyData.HostileCompanies.Num();
//	return WarCount;
}

int64 UFlareCompany::GetTributeCost(UFlareCompany* Company)
{
	int32 GameDifficulty = -1;
	GameDifficulty = Game->GetPC()->GetPlayerData()->DifficultyId;
	double Multiplier = 0.01;

	if (Company == Game->GetPC()->GetCompany())
	{
		//tributing TOWARDS the player
		switch (GameDifficulty)
		{
		case -1: // Easy
			Multiplier = 0.015;
			break;
		case 0: // Normal
			Multiplier = 0.01;
			break;
		case 1: // Hard
			Multiplier = 0.0075;
			break;
		case 2: // Very Hard
			Multiplier = 0.0050;
			break;
		case 3: // Expert
			Multiplier = 0.0025;
			break;
		case 4: // Unfair
			Multiplier = 0.00125;
			break;
		}
	}
	else if (this == Game->GetPC()->GetCompany())
	{
		switch (GameDifficulty)
		{
		case -1: // Easy
			Multiplier = 0.01;
			break;
		case 0: // Normal
			Multiplier = 0.01;
			break;
		case 1: // Hard
			Multiplier = 0.015;
			break;
		case 2: // Very Hard
			Multiplier = 0.02;
			break;
		case 3: // Expert
			Multiplier = 0.04;
			break;
		case 4: // Unfair
			Multiplier = 0.06;
			break;
		}
	}
	return Multiplier * GetCompanyValue().TotalValue + 0.1 * GetCompanyValue().MoneyValue;
}

void UFlareCompany::PayTribute(UFlareCompany* Company, bool AllowDepts)
{
	int64 Cost = GetTributeCost(Company);

	if (Cost <= GetMoney() || AllowDepts)
	{
		FLOGV("UFlareCompany::PayTribute: %s paying %ld to %s", *GetCompanyName().ToString(), Cost, *Company->GetCompanyName().ToString());

		// Exchange money
		TakeMoney(Cost, AllowDepts, FFlareTransactionLogEntry::LogSendTribute(Company));
		Company->GiveMoney(Cost, FFlareTransactionLogEntry::LogReceiveTribute(this));

		// Reset pacifism
		GetAI()->GetData()->Pacifism = FMath::Max(GetAI()->GetBehavior()->PacifismAfterTribute, GetAI()->GetData()->Pacifism);
		Company->GetAI()->GetData()->Pacifism = FMath::Max(Company->GetAI()->GetBehavior()->PacifismAfterTribute, Company->GetAI()->GetData()->Pacifism);
		// Reset hostilities

		if (this == Company->GetGame()->GetPC()->GetCompany())
		{
			Company->GivePlayerReputation(5);
		}

		SetHostilityTo(Company, false);
		Company->SetHostilityTo(this, false);

		if(Company == Game->GetPC()->GetCompany())
		{
			ResetLastTributeDate();
		}
	}
	else
	{
		FLOG("UFlareCompany::PayTribute: not enough money to pay tribute");
	}
}

void UFlareCompany::StartCapture(UFlareSimulatedSpacecraft* Station)
{
	if(!CanStartCapture(Station))
	{
		return;
	}

	CompanyData.CaptureOrders.AddUnique(Station->GetImmatriculation());
}

void UFlareCompany::StopCapture(UFlareSimulatedSpacecraft* Station)
{

	CompanyData.CaptureOrders.Remove(Station->GetImmatriculation());
}

bool UFlareCompany::CanStartCapture(UFlareSimulatedSpacecraft* Station)
{

	if(CompanyData.CaptureOrders.Contains(Station->GetImmatriculation()))
	{
		return false;
	}

	int32 StationCount = this->GetCompanySectorStationsCount(Station->GetCurrentSector(), true);
	int32 MaxStationCount = IsTechnologyUnlocked("dense-sectors") ? Station->GetCurrentSector()->GetMaxStationsPerCompany() : Station->GetCurrentSector()->GetMaxStationsPerCompany() / 2;

	if(StationCount >= MaxStationCount)
	{

		return false;
	}

	if ((GetWarState(Station->GetCompany()) != EFlareHostility::Hostile)
		|| !Station->GetCurrentSector()->GetSectorBattleState(this).BattleWon)
	{
		// Friend don't capture and not winner don't capture
		return false;
	}

	if (Station->GetDescription()->IsUncapturable)
	{
		return false;
	}
	
	return true;

}

/*----------------------------------------------------
	Customization
----------------------------------------------------*/

void UFlareCompany::UpdateCompanyCustomization()
{
	// Update spacecraft if there is an active sector
	UFlareSector* ActiveSector = Game->GetActiveSector();
	if (ActiveSector)
	{
		for (int32 i = 0; i < ActiveSector->GetSpacecrafts().Num(); i++)
		{
			ActiveSector->GetSpacecrafts()[i]->UpdateCustomization();
		}
	}

	// Update the emblem
	SetupEmblem();
}

void UFlareCompany::CustomizeMaterial(UMaterialInstanceDynamic* Mat)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(Game->GetWorld()->GetFirstPlayerController());

	// Apply settings to the material instance
	UFlareSpacecraftComponent::CustomizeMaterial(Mat, Game,
		GetBasePaintColor(),
		GetPaintColor(),
		GetOverlayColor(),
		GetLightColor(),
		GetPatternIndex(),
		CompanyDescription->Emblem);
}

void UFlareCompany::SetupEmblem()
{
	// Create the parameter
	FVector2D EmblemSize = 128 * FVector2D::UnitVector;
	UMaterial* BaseEmblemMaterial = Cast<UMaterial>(FFlareStyleSet::GetIcon("CompanyEmblem")->GetResourceObject());
	CompanyEmblem = UMaterialInstanceDynamic::Create(BaseEmblemMaterial, GetWorld());
	UFlareCustomizationCatalog* Catalog = Game->GetCustomizationCatalog();

	// Setup the material
	CompanyEmblem->SetTextureParameterValue("Emblem", CompanyDescription->Emblem);
	CompanyEmblem->SetVectorParameterValue("BasePaintColor", CompanyDescription->CustomizationBasePaintColor);
	CompanyEmblem->SetVectorParameterValue("PaintColor", CompanyDescription->CustomizationPaintColor);
	CompanyEmblem->SetVectorParameterValue("OverlayColor", CompanyDescription->CustomizationOverlayColor);
	CompanyEmblem->SetVectorParameterValue("GlowColor", CompanyDescription->CustomizationLightColor);

	// Create the brush dynamically
	CompanyEmblemBrush.ImageSize = EmblemSize;
	CompanyEmblemBrush.SetResourceObject(CompanyEmblem);
}

const FSlateBrush* UFlareCompany::GetEmblem() const
{
	return &CompanyEmblemBrush;
}


/*----------------------------------------------------
	Technology
----------------------------------------------------*/

bool UFlareCompany::IsTechnologyUnlocked(FName Identifier) const
{
	if (UnlockedTechnologies.Contains(Identifier))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool UFlareCompany::IsSectorStationLicenseUnlocked(FName Identifier) const
{
	if (LicenseStationSectors.Contains(Identifier))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool UFlareCompany::IsTechnologyAvailable(FName Identifier, FText& Reason, bool IgnoreCost) const
{
	FFlareTechnologyDescription* Technology = GetGame()->GetTechnologyCatalog()->Get(Identifier);
 
	if (Technology == NULL)
	{
		return false;
	}

	if (GetTechnologyLevel() < Technology->Level)
	{
		Reason = LOCTEXT("CantUnlockTechLevel", "You don't have the technology level required to research this technology");
		return false;
	}
	else if (!IgnoreCost && GetResearchAmount() < GetTechnologyCost(Technology))
	{
		Reason = LOCTEXT("CantUnlockTechCost", "You haven't done enough research for this technology");
		return false;
	}
	else if (IsTechnologyUnlocked(Identifier))
	{
		Reason = LOCTEXT("CantUnlockTechAlready", "You have already researched this technology");
		return false;
	}
	else if (Technology->RequiredTechnologies.Num() > 0)
	{
		bool AllTechsUnlocked = true;
		TArray<FText> RequiredTechnologiesEntries;
		for (int32 i = 0; i < Technology->RequiredTechnologies.Num(); i++)
		{
			FName CurrentTechnology = Technology->RequiredTechnologies[i];
			if (!IsTechnologyUnlocked(CurrentTechnology))
			{
				FFlareTechnologyDescription* Technology = GetGame()->GetTechnologyCatalog()->Get(CurrentTechnology);
				if (!Technology)
				{
					continue;
				}
				AllTechsUnlocked = false;
				RequiredTechnologiesEntries.Add(Technology->Name);
			}
		}

		if (RequiredTechnologiesEntries.Num() > 0)
		{
			FString RequiredTechsString;
			for (int32 i = 0; i < RequiredTechnologiesEntries.Num(); i++)
			{
				FText CurrentTechnology = RequiredTechnologiesEntries[i];
				if (RequiredTechsString.Len())
				{
					RequiredTechsString += ", ";
				}
				RequiredTechsString += FString::Printf(TEXT("%s"), *CurrentTechnology.ToString());
			}
			Reason = FText::Format(LOCTEXT("MissingTechnologiesFormat", "Missing required technologies: {0}"), FText::FromString(RequiredTechsString));
		}
		return AllTechsUnlocked;
	}
	else
	{
		return true;
	}
}

bool UFlareCompany::HasStationTechnologyUnlocked() const
{
	TArray<UFlareSpacecraftCatalogEntry*>& StationCatalog = GetGame()->GetSpacecraftCatalog()->StationCatalog;

	// Loop on catalog
	for (UFlareSpacecraftCatalogEntry* Entry : StationCatalog)
	{
		if (!Entry->Data.IsSubstation && IsTechnologyUnlockedStation(&Entry->Data))
		{
			return true;
		}
	}
	return false;
}

bool UFlareCompany::IsTechnologyUnlockedShip(const FFlareSpacecraftDescription* Description) const
{
	FName Identifier = Description->Identifier;

	if (Description->RequiredTechnologies.Num() > 0)
	{
		for (FName CurrentTechnology : Description->RequiredTechnologies)
		{
			if (!IsTechnologyUnlocked(CurrentTechnology))
			{
				return false;
			}
		}
	}
	return true;
}

bool UFlareCompany::IsTechnologyUnlockedStation(const FFlareSpacecraftDescription* Description) const
{
	FName Identifier = Description->Identifier;

	if (Description->RequiredTechnologies.Num() > 0)
	{
		bool AllTechsUnlocked = true;
		for (int32 i = 0; i < Description->RequiredTechnologies.Num(); i++)
		{
			FName CurrentTechnology = Description->RequiredTechnologies[i];
			if (!IsTechnologyUnlocked(CurrentTechnology))
			{
				AllTechsUnlocked = false;
				break;
			}
		}
		return AllTechsUnlocked;
	}

	else if (Identifier == "station-habitation" ||
			Identifier == "station-outpost" ||
			Identifier == "station-solar-plant")
	{
		return IsTechnologyUnlocked("stations");
	}
	else if(Identifier == "station-ice-mine"||
			Identifier == "station-silica-mine" ||
			Identifier == "station-iron-mine")
	{
		return IsTechnologyUnlocked("mining");
	}
	else if(Identifier == "station-carbon-refinery" ||
			Identifier == "station-farm" ||
			Identifier == "station-plastics-refinery")
	{
		return IsTechnologyUnlocked("chemicals");
	}
	else if(Identifier == "station-ch4-pump"||
			Identifier == "station-he3-pump" ||
			Identifier == "station-h2-pump")
	{
		return IsTechnologyUnlocked("orbital-pumps");
	}
	else if(Identifier == "station-steelworks" ||
			Identifier == "station-tool-factory" ||
			Identifier == "station-arsenal")
	{
		return IsTechnologyUnlocked("metallurgy");
	}
	else if(Identifier == "station-shipyard")
	{
		return IsTechnologyUnlocked("shipyard-station");
	}
	else if(Identifier == "station-tokamak" ||
			Identifier == "station-hub" ||
			Identifier == "station-complex" ||
			Identifier == "station-foundry")
	{
		return IsTechnologyUnlocked("advanced-stations");
	}
	else if(Identifier == "station-telescope" ||
			Identifier == "station-research")
	{
		return IsTechnologyUnlocked("science");
	}


	FLOGV("WARNING: station %s don't need technology", *Description->Identifier.ToString());
	return true;
}

bool UFlareCompany::IsPartRestricted(const FFlareSpacecraftComponentDescription* Description, UFlareSimulatedSpacecraft* Ship) const
{
	FName Identifier = Description->Identifier;

	if (Description->RestrictedShips.Num() > 0)
	{
		if (!Description->RestrictedShips.Contains(Ship->GetDescription()->Identifier))
		{
			return true;
		}
	}
	return false;
}

bool UFlareCompany::IsPartRestricted(const FFlareSpacecraftComponentDescription* Description, FName Ship) const
{
	if (Description->RestrictedShips.Num() > 0)
	{
		if (!Description->RestrictedShips.Contains(Ship))
		{
			return true;
		}
	}
	return false;
}

bool UFlareCompany::IsTechnologyUnlockedPart(const FFlareSpacecraftComponentDescription* Description) const
{
	FName Identifier = Description->Identifier;

	if (Description->RequiredTechnologies.Num() > 0)
	{
		bool AllTechsUnlocked = true;
		for (int32 i = 0; i < Description->RequiredTechnologies.Num(); i++)
		{
			FName CurrentTechnology = Description->RequiredTechnologies[i];
			if (!IsTechnologyUnlocked(CurrentTechnology))
			{
				AllTechsUnlocked = false;
				break;
			}
		}
		return AllTechsUnlocked;
	}

	if (Identifier == "weapon-heavy-salvage" ||
		Identifier == "weapon-light-salvage")
	{
		return IsTechnologyUnlocked("pirate-tech");
	}

	if (Identifier == "weapon-hades" ||
		Identifier == "weapon-mjolnir")
	{
		return IsTechnologyUnlocked("flak");
	}

	if (Identifier == "weapon-wyrm" ||
		Identifier == "weapon-sparrow" ||
		Identifier == "weapon-hydra")
	{
		return IsTechnologyUnlocked("bombing");
	}

	return true;
}

int32 UFlareCompany::GetTechnologyCost(const FFlareTechnologyDescription* Technology) const
{
	if (Technology->ResearchCost)
	{
		return Technology->ResearchCost * Technology->Level * CompanyData.ResearchRatio;
	}
	return 20 * Technology->Level * CompanyData.ResearchRatio;
}

int32 UFlareCompany::GetTechnologyCostFromID(const FName Identifier) const
{
	FFlareTechnologyDescription* Technology = GetGame()->GetTechnologyCatalog()->Get(Identifier);
	FText Unused;

	if (Identifier != NAME_None && Technology)
	{
		if (Technology->ResearchCost)
		{
			return Technology->ResearchCost * Technology->Level * CompanyData.ResearchRatio;
		}
		return 20 * Technology->Level * CompanyData.ResearchRatio;
	}
	return 0;
}

float UFlareCompany::GetTechnologyBonus(FName Identifier) const
{
	if (ResearchBonuses.Contains(Identifier))
	{
		return ResearchBonuses[Identifier];
	}
	return 0;
}

int32 UFlareCompany::GetTechnologyLevel() const
{
	// 0 technologies -> Level 1
	// 1 technologies -> Level 2
	// 2 technologies -> Level 3
	// 3 technologies -> Level 4
	return FMath::Clamp(1 + UnlockedTechnologies.Num(), 1, GetGame()->GetTechnologyCatalog()->GetMaxTechLevel());
}

int32 UFlareCompany::GetResearchAmount() const
{
	return CompanyData.ResearchAmount;
}

int32 UFlareCompany::GetResearchSpent() const
{
	return CompanyData.ResearchSpent;
}

int32 UFlareCompany::GetResearchValue() const
{
	return GetResearchSpent() + GetResearchAmount();
}

TArray<UFlareSimulatedSpacecraft*> UFlareCompany::GetCompanySectorStations(UFlareSimulatedSector* Sector)
{
	if (Sector)
	{
		if (CompanyStationsBySectors.Contains(Sector))
		{
			return CompanyStationsBySectors[Sector];
		}

		TArray<UFlareSimulatedSpacecraft*> NewCompanyStations;
		CompanyStationsBySectors.Add(Sector, NewCompanyStations);
		return NewCompanyStations;
	}
	TArray<UFlareSimulatedSpacecraft*> SectorStations;
	return SectorStations;
}

int32 UFlareCompany::GetCompanySectorStationsCount(UFlareSimulatedSector* Sector, bool IncludeCapture)
{
	int32 OwnedStationCount = GetCompanySectorStations(Sector).Num();
	if (IncludeCapture)
	{
		OwnedStationCount += this->GetCaptureOrderCountInSector(Sector);
	}

	return OwnedStationCount;
}

void UFlareCompany::AddOrRemoveCompanySectorStation(UFlareSimulatedSpacecraft* Station, bool Remove)
{
	if (Station)
	{
		UFlareSimulatedSector* Sector = Station->GetCurrentSector();
		if (Sector)
		{
			if (Remove)
			{
				if (CompanyStationsBySectors.Contains(Sector))
				{
					CompanyStationsBySectors[Sector].Remove(Station);
				}
			}
			else
			{
				if (CompanyStationsBySectors.Contains(Sector))
				{
					CompanyStationsBySectors[Sector].AddUnique(Station);
/*
					GetGame()->GetPC()->Notify(
						LOCTEXT("TestInfo0", "Test Notification"),
						FText::Format(
							LOCTEXT("TestInfoFormat0", "{0} adding entry to existing value, found {1} in {2}"),
							GetCompanyName(), CompanyStationsBySectors[Sector].Num(),Sector->GetSectorName()),
						"discover-sector0",
						EFlareNotification::NT_Info,
						false);
*/
				}
				else
				{
					TArray<UFlareSimulatedSpacecraft*> NewCompanyStations;
					NewCompanyStations.AddUnique(Station);
					CompanyStationsBySectors.Add(Sector, NewCompanyStations);
/*
					GetGame()->GetPC()->Notify(
						LOCTEXT("TestInfo1", "Test Notification"),
						FText::Format(
							LOCTEXT("TestInfoFormat1", "{0} adding brand new entry in their stations by sector array"),
							GetCompanyName()),
						"discover-sector1",
						EFlareNotification::NT_Info,
						false);
*/
				}
			}
		}
	}
}

bool UFlareCompany::CanBuyStationLicense(FName Identifier)
{
	UFlareSimulatedSector* BuyingSector = GetGame()->GetGameWorld()->FindSector(Identifier);
	return CanBuyStationLicense(BuyingSector);
}

void UFlareCompany::BuyStationLicense(FName Identifier)
{
	UFlareSimulatedSector* BuyingSector = GetGame()->GetGameWorld()->FindSector(Identifier);
	BuyStationLicense(BuyingSector, false);
}

bool UFlareCompany::CanBuyStationLicense(UFlareSimulatedSector* BuyingSector)
{
	if (BuyingSector)
	{
		if (!IsSectorStationLicenseUnlocked(BuyingSector->GetDescription()->Identifier))
		{
			int64 LicenseCost = GetStationLicenseCost(BuyingSector);
			if (LicenseCost <= GetMoney())
			{
				return true;
			}
		}
	}
	return false;
}

void UFlareCompany::BuyStationLicense(UFlareSimulatedSector* BuyingSector, bool FromSave)
{
	if (BuyingSector)
	{
		if (!IsSectorStationLicenseUnlocked(BuyingSector->GetDescription()->Identifier) || FromSave)
		{
			if (!FromSave)
			{
				int64 LicenseCost = GetStationLicenseCost(BuyingSector);
				TakeMoney(LicenseCost, true, FFlareTransactionLogEntry::LogPaidSectorStationBuildingLicense(BuyingSector));
				BuyingSector->GetPeople()->Pay(LicenseCost);
			}

			LicenseStationSectors.Add(BuyingSector->GetDescription()->Identifier, BuyingSector);
			ClearTemporaryCaches();
			ChangeStationLicenseState(BuyingSector, true);
		}
	}
}

void UFlareCompany::GiveAllStationSectorLicenses()
{
	//for brand new and also old savegames
	for (UFlareSimulatedSpacecraft* Station : GetCompanyStations())
	{
		BuyStationLicense(Station->GetCurrentSector(), true);
	}
}

void UFlareCompany::CheckStationLicenseStateStation(UFlareSimulatedSpacecraft* Station, bool NewState)
{
	if (Station)
	{
		bool ChangeFactoryValueTo = false;
		if (NewState)
		{
			ChangeFactoryValueTo = NewState;
		}
		else
		{
			UFlareSimulatedSector* CurrentSector = Station->GetCurrentSector();
			if (CurrentSector)
			{
				if(IsSectorStationLicenseUnlocked(Station->GetCurrentSector()->GetDescription()->Identifier))
				{
					ChangeFactoryValueTo = true;
				}
			}
		}
		ChangeStationLicenseStateAction(Station, ChangeFactoryValueTo);
	}
}

void UFlareCompany::ChangeStationLicenseState(UFlareSimulatedSector* BuyingSector, bool NewState)
{
	for (UFlareSimulatedSpacecraft* Station : GetCompanySectorStations(BuyingSector))
	{
		if(Station->GetCompany() == this)
		{
			ChangeStationLicenseStateAction(Station, NewState);
		}
	}
}

void UFlareCompany::ChangeStationLicenseStateAction(UFlareSimulatedSpacecraft* Station, bool NewState)
{
	if (Station)
	{
		Station->SetOwnerHasStationLicense(NewState);
	}
}

int64 UFlareCompany::GetTotalStationLicenseValue()
{
	int64 TotalLicenseValue = 0;

	TMap<FName, UFlareSimulatedSector*> LicensedSectors = GetLicenseStationSectors();
	TArray<FName> Keys;
	LicensedSectors.GetKeys(Keys);

	for (int i = 0; i < Keys.Num(); i++)
	{
		FName Identifier = Keys[i];
		UFlareSimulatedSector* Sector = LicensedSectors[Identifier];
		if (Sector)
		{
			TotalLicenseValue += GetStationLicenseCost(Sector);
		}
	}

	return TotalLicenseValue;
}

int64 UFlareCompany::GetStationLicenseCost(FName Identifier)
{
	UFlareSimulatedSector* BuyingSector = GetGame()->GetGameWorld()->FindSector(Identifier);
	return GetStationLicenseCost(BuyingSector);
}

int64 UFlareCompany::GetStationLicenseCost(UFlareSimulatedSector* BuyingSector)
{
	if (LicenseStationCache.Contains(BuyingSector))
	{
		return LicenseStationCache[BuyingSector];
	}

	int64 LicenseCost = 1000000;
	int32 TotalUnlicencedStations = 0;
	float Multiplier = 1.0;

	if (BuyingSector->GetDescription()->IsIcy)
	{
		LicenseCost += 500000;
	}

	if (BuyingSector->GetDescription()->IsGeostationary)
	{
		LicenseCost += 500000;
	}

	int32 CompaniesKnownSectors = 0;
	int32 CompaniesStationPermits = 0;
	for (UFlareCompany* LocalCompany : Game->GetGameWorld()->GetCompanies())
	{
		if (LocalCompany->GetCompanyStationLicenses().Find(BuyingSector->GetDescription()->Identifier) != INDEX_NONE)
		{
			//having a permit overrides the multiplier effect of just knowing the sector
			++CompaniesStationPermits;
		}
		else if (LocalCompany->GetKnownSectors().Find(BuyingSector) != INDEX_NONE)
		{
			++CompaniesKnownSectors;
		}
	}

	Multiplier += (CompaniesKnownSectors / 4) + CompaniesStationPermits;
	LicenseCost = LicenseCost * Multiplier;

	for (UFlareSimulatedSpacecraft* Station : GetCompanySectorStations(BuyingSector))
	{
		if (Station->GetCompany() == this)
		{
			TotalUnlicencedStations++;
		}
	}

	LicenseCost = LicenseCost + (500000 * this->GetCompanyStationLicenses().Num()) + (BuyingSector->GetPeople()->GetPopulation() * 50) + (50000 * TotalUnlicencedStations);

	LicenseStationCache.Add(BuyingSector, LicenseCost);
	return LicenseCost;
}

void UFlareCompany::UnlockTechnology(FName Identifier, bool FromSave, bool Force, bool HideMessage)
{
	FFlareTechnologyDescription* Technology = GetGame()->GetTechnologyCatalog()->Get(Identifier);
	FText Unused;

	if (Identifier != NAME_None && Technology && (IsTechnologyAvailable(Identifier, Unused) || FromSave || Force))
	{
		// Unlock
		UnlockedTechnologies.Add(Identifier, Technology);

		ResearchBonuses["repair-bonus"] += Technology->RepairBonus;
		ResearchBonuses["diplomatic-penaltybonus"] += Technology->DiplomaticPenaltyBonus;
		ResearchBonuses["diplomatic-bonus"] += Technology->DiplomaticBonus;
		ResearchBonuses["travel-bonus"] += Technology->TravelBonus;
		ResearchBonuses["shipyard-fabrication-bonus"] += Technology->ShipyardFabBonus;

		if (!FromSave)
		{
			int32 Cost = GetTechnologyCost(Technology);

			if (!Force)
			{
				CompanyData.ResearchAmount -= Cost;
			}

			CompanyData.ResearchSpent += Cost;

			// Check before research
			float CurrentResearchInflation = IsTechnologyUnlocked("instruments") ? 1.22 : 1.3;
			CompanyData.ResearchRatio *= CurrentResearchInflation;

			if (this == Game->GetPC()->GetCompany())
			{
				if (!HideMessage)
				{
					FString UniqueId = "technology-unlocked-" + Identifier.ToString();
					Game->GetPC()->Notify(LOCTEXT("CompanyUnlockTechnology", "Technology unlocked"),
						FText::Format(LOCTEXT("CompanyUnlockTechnologyFormat", "You have researched {0} for your company !"), Technology->Name),
						FName(*UniqueId),
						EFlareNotification::NT_Info,
						false);
					GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("unlock-technology").PutName("technology", Identifier).PutInt32("level", Technology->Level));
				}

				GetGame()->GetPC()->SetAchievementProgression("ACHIEVEMENT_ONE_TECHNOLOGY", 1);
				if(UnlockedTechnologies.Num() >= GetGame()->GetTechnologyCatalog()->TechnologyCatalog.Num())
				{
					GetGame()->GetPC()->SetAchievementProgression("ACHIEVEMENT_ALL_TECHNOLOGIES", 1);
				}
			}

			GameLog::UnlockResearch(this, Technology);
		}
	}
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/


const struct CompanyValue UFlareCompany::GetCompanyValue(UFlareSimulatedSector* SectorFilter, bool IncludeIncoming) const
{
	bool globalRequest = SectorFilter == nullptr;

	if(CompanyValueCacheValid && globalRequest)
	{
		return CompanyValueCache;
	}

	// Company value is the sum of :
	// - money
	// - value of its spacecraft
	// - value of the stock in these spacecraft
	// - value of the resources used in factory
	// - daily money requirements to keep stations running
	CompanyValue CompanyValue;
	CompanyValue.MoneyValue = GetMoney();
	CompanyValue.StockValue = 0;
	CompanyValue.ShipsValue = 0;
	CompanyValue.ArmyValue = 0;
	CompanyValue.ArmyCurrentCombatPoints = 0;
	CompanyValue.ArmyTotalCombatPoints = 0;
	CompanyValue.StationsValue = 0;
	CompanyValue.TotalDailyProductionCost = 0;
	CompanyValue.TotalShipCount = 0;
	CompanyValue.TotalDroneCount = 0;
	CompanyValue.TotalShipCountMilitaryLSalvager = 0;
	CompanyValue.TotalShipCountMilitarySSalvager = 0;
	CompanyValue.TotalShipCountMilitaryS = 0;
	CompanyValue.TotalShipCountMilitaryL = 0;
	CompanyValue.TotalShipCountTradeS = 0;
	CompanyValue.TotalShipCountTradeL = 0;

	for (int SpacecraftIndex = 0; SpacecraftIndex < CompanySpacecrafts.Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = CompanySpacecrafts[SpacecraftIndex];

		UFlareSimulatedSector *ReferenceSector =  Spacecraft->GetCurrentSector();

		if (!ReferenceSector)
		{
			if (Spacecraft->GetCurrentFleet() && Spacecraft->GetCurrentFleet()->GetCurrentTravel())
			{
				ReferenceSector = Spacecraft->GetCurrentFleet()->GetCurrentTravel()->GetDestinationSector();
				if (!ReferenceSector)
				{
					ReferenceSector = Spacecraft->GetCurrentFleet()->GetCurrentTravel()->GetOldDestinationSector();
				}
			}
			else
			{
				FLOGV("Spacecraft %s is lost : no current sector, no travel", *Spacecraft->GetImmatriculation().ToString());
				continue;
			}

		}

		if(SectorFilter && IncludeIncoming && SectorFilter != ReferenceSector)
		{
			// Not in sector filter
			continue;
		}

		if (!IsValid(ReferenceSector) || ReferenceSector == nullptr)
		{
			continue;
		}

		// Value of the spacecraft
		int64 SpacecraftPrice = UFlareGameTools::ComputeSpacecraftPrice(Spacecraft->GetDescription()->Identifier, ReferenceSector, true);

		if(Spacecraft->IsStation())
		{
			CompanyValue.StationsValue += SpacecraftPrice * Spacecraft->GetLevel();
		}
		else
		{
			CompanyValue.ShipsValue += SpacecraftPrice;
			if (Spacecraft->GetDescription()->IsDroneShip)
			{
				CompanyValue.TotalDroneCount++;
			}
			else
			{
				CompanyValue.TotalShipCount++;
			}
		}

		if(Spacecraft->IsMilitary())
		{
			CompanyValue.ArmyValue += SpacecraftPrice;
			CompanyValue.ArmyTotalCombatPoints += Spacecraft->GetCombatPoints(false);
			CompanyValue.ArmyCurrentCombatPoints += Spacecraft->GetCombatPoints(true);

			if (Spacecraft->GetSize() == EFlarePartSize::S)
			{
				CompanyValue.TotalShipCountMilitaryS++;
				if (Spacecraft->GetEquippedSalvagerCount())
				{
					CompanyValue.TotalShipCountMilitarySSalvager++;
				}
			}
			else
			{
				CompanyValue.TotalShipCountMilitaryL++;
				if (Spacecraft->GetEquippedSalvagerCount())
				{
					CompanyValue.TotalShipCountMilitaryLSalvager++;
				}
			}
		}
		else if (Spacecraft->GetSize() == EFlarePartSize::S)
		{
			CompanyValue.TotalShipCountTradeS++;
		}
		else
		{
			CompanyValue.TotalShipCountTradeL++;
		}

		// Value of the stock
		{
		TArray<FFlareCargo>& CargoBaySlots = Spacecraft->GetProductionCargoBay()->GetSlots();
		for (int CargoIndex = 0; CargoIndex < CargoBaySlots.Num(); CargoIndex++)
		{
			FFlareCargo& Cargo = CargoBaySlots[CargoIndex];

			if (!Cargo.Resource)
			{
				continue;
			}

			CompanyValue.StockValue += ReferenceSector->GetResourcePrice(Cargo.Resource, EFlareResourcePriceContext::Default) * Cargo.Quantity;
		}
		}

		{
		TArray<FFlareCargo>& CargoBaySlots = Spacecraft->GetConstructionCargoBay()->GetSlots();
		for (int CargoIndex = 0; CargoIndex < CargoBaySlots.Num(); CargoIndex++)
		{
			FFlareCargo& Cargo = CargoBaySlots[CargoIndex];

			if (!Cargo.Resource)
			{
				continue;
			}

			CompanyValue.StockValue += ReferenceSector->GetResourcePrice(Cargo.Resource, EFlareResourcePriceContext::Default) * Cargo.Quantity;
		}
		}

		// Value of factory stock
		for (int32 FactoryIndex = 0; FactoryIndex < Spacecraft->GetFactories().Num(); FactoryIndex++)
		{
			UFlareFactory* Factory = Spacecraft->GetFactories()[FactoryIndex];

			int64 FactoryDuration = Factory->GetProductionDuration();
			if (FactoryDuration > 0)
			{


//				if (Game->GetPC()->GetCompany() == this)
					FText ProductionCostText;
					uint32 CycleProductionCost = Factory->GetProductionCost();
					if (CycleProductionCost > 0)
					{

						if (Factory->HasCostReserved())
						{
							CompanyValue.TotalDailyProductionCost += CycleProductionCost / FactoryDuration;
						}

						ProductionCostText = FText::Format(LOCTEXT("ProductionCostFormat", "{0} credits"), FText::AsNumber(UFlareGameTools::DisplayMoney(CycleProductionCost)));
					}
					/*
					Game->GetPC()->Notify(
						LOCTEXT("Prodcost", "Fact duration"),
						FText::Format(LOCTEXT("Prodcost", "Factory Duration {0}, money {1}"),
							FactoryDuration,
							ProductionCostText),
						"Prodcost",
						EFlareNotification::NT_Info);
					*/
			}

			for (int32 ReservedResourceIndex = 0 ; ReservedResourceIndex < Factory->GetReservedResources().Num(); ReservedResourceIndex++)
			{
				FName ResourceIdentifier = Factory->GetReservedResources()[ReservedResourceIndex].ResourceIdentifier;
				uint32 Quantity = Factory->GetReservedResources()[ReservedResourceIndex].Quantity;

				FFlareResourceDescription* Resource = Game->GetResourceCatalog()->Get(ResourceIdentifier);
				if (Resource)
				{
					CompanyValue.StockValue += ReferenceSector->GetResourcePrice(Resource, EFlareResourcePriceContext::Default) * Quantity;
				}
				else
				{
					FLOGV("WARNING: Invalid reserved resource %s (%d reserved) for %s)", *ResourceIdentifier.ToString(), Quantity, *Spacecraft->GetImmatriculation().ToString())
				}
			}
		}
	}
/*
//	if (Game->GetPC()->GetCompany() == this)
		Game->GetPC()->Notify(
			LOCTEXT("Prodcost1", "Returned Existing Cache"),
			FText::Format(LOCTEXT("Prodcost1", "Daily Production Cost {0}"),
				CompanyValue.TotalDailyProductionCost),
			"Prodcost1",
			EFlareNotification::NT_Info);
		*/

	CompanyValue.SpacecraftsValue = CompanyValue.ShipsValue + CompanyValue.StationsValue;
	CompanyValue.TotalValue = CompanyValue.MoneyValue + CompanyValue.StockValue + CompanyValue.SpacecraftsValue;

	if(globalRequest)
	{
		CompanyValueCacheValid = true;
		CompanyValueCache = CompanyValue;
	}

	return CompanyValue;
}

UFlareSimulatedSpacecraft* UFlareCompany::FindSpacecraft(FName ShipImmatriculation, bool Destroyed)
{
	if (CompanySpacecraftsCache.Contains(ShipImmatriculation))
	{
		return CompanySpacecraftsCache[ShipImmatriculation];
	}

	if (Destroyed)
	{
		for (UFlareSimulatedSpacecraft* Spacecraft : CompanyDestroyedSpacecrafts)
		{
			if (Spacecraft->GetImmatriculation() == ShipImmatriculation)
			{
				return Spacecraft;
			}
		}
	}
/*
	if(!Destroyed )
	{
		for (UFlareSimulatedSpacecraft* Spacecraft : CompanySpacecrafts)
		{
			if (Spacecraft->GetImmatriculation() == ShipImmatriculation)
			{
				return Spacecraft;
			}
		}
	}
	else
	{
		for (UFlareSimulatedSpacecraft* Spacecraft : CompanyDestroyedSpacecrafts)
		{
			if (Spacecraft->GetImmatriculation() == ShipImmatriculation)
			{
				return Spacecraft;
			}
		}
	}
*/
	return NULL;
}

bool UFlareCompany::HasVisitedSector(const UFlareSimulatedSector* Sector) const
{
	return Sector && VisitedSectors.Contains(Sector);
}

FText UFlareCompany::GetPlayerHostilityText() const
{
	FText Status;

	switch (GetPlayerWarState())
	{
		case EFlareHostility::Neutral:
			Status = LOCTEXT("Neutral", "Neutral");
			break;
		case EFlareHostility::Friendly:
			Status = LOCTEXT("Friendly", "Friendly");
			break;
		case EFlareHostility::Owned:
			Status = LOCTEXT("Owned", "Owned");
			break;
		case EFlareHostility::Hostile:
			Status = LOCTEXT("Hostile", "Hostile");
			break;
	}

	return Status;
}

bool UFlareCompany::IsPlayerCompany() const
{
	return this == Game->GetPC()->GetCompany();
}

bool UFlareCompany::WantCapture(UFlareSimulatedSpacecraft const* Station) const
{
	return CompanyData.CaptureOrders.Contains(Station->GetImmatriculation());
}

int32 UFlareCompany::GetCaptureOrderCountInSector(UFlareSimulatedSector const* Sector) const
{
	int32 Orders = 0;

	for(UFlareSimulatedSpacecraft const* Station: Sector->GetSectorStations())
	{
		if(WantCapture(Station))
		{
			++Orders;
		}
	}
	return Orders;
}

int32 UFlareCompany::GetCaptureShipOrderCountInSector(UFlareSimulatedSector* Sector)
{
	int32 Orders = 0;
	for (UFlareSimulatedSpacecraft* Ship : Sector->GetSectorShips())
	{
		FFlareSpacecraftSave& Data = Ship->GetData();
		if (Data.HarpoonCompany==this->GetIdentifier())
		{
			++Orders;
		}
	}
	return Orders;
}

UFlareSimulatedSpacecraft* UFlareCompany::FindChildStation(FName StationImmatriculation)
{
	for (UFlareSimulatedSpacecraft* Spacecraft : CompanyChildStations)
	{
		if (Spacecraft->GetImmatriculation() == StationImmatriculation)
		{
			return Spacecraft;
		}
	}
	return nullptr;
}

void UFlareCompany::AddRetaliation(float Retaliation)
{
	CompanyData.Retaliation += Retaliation;
}

void UFlareCompany::RemoveRetaliation(float Retaliation)
{
	CompanyData.Retaliation -= Retaliation;
}

TArray<UFlareSimulatedSector*> UFlareCompany::GetUndiscoveredSectors()
{
	// List all unknown sectors
	TArray<UFlareSimulatedSector*> Candidates;
	for (auto CandidateSector : GetGame()->GetGameWorld()->GetSectors())
	{
		if (!IsKnownSector(CandidateSector) && !CandidateSector->GetDescription()->IsHiddenFromTelescopes)
		{
			Candidates.Add(CandidateSector);
		}
	}
	return Candidates;
}

#undef LOCTEXT_NAMESPACE
