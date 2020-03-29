
#include "FlareGameTools.h"
#include "../Flare.h"

#include "FlareGame.h"
#include "FlareCompany.h"
#include "FlarePlanetarium.h"
#include "FlareSectorHelper.h"

#include "../Data/FlareFactoryCatalogEntry.h"
#include "../Data/FlareResourceCatalog.h"
#include "../Data/FlareSpacecraftCatalog.h"
#include "../Data/FlareSpacecraftComponentsCatalog.h"

#include "../Economy/FlareCargoBay.h"

#include "../Player/FlareMenuManager.h"
#include "../Player/FlarePlayerController.h"

#include "Quests/FlareQuest.h"
#include "Quests/FlareQuestStep.h"
#include "Quests/FlareQuestManager.h"

#define LOCTEXT_NAMESPACE "FlareGameTools"

bool UFlareGameTools::FastFastForward = false;

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareGameTools::UFlareGameTools(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{

}


/*----------------------------------------------------
	Game tools
----------------------------------------------------*/



void UFlareGameTools::ForceSectorActivation(FName SectorIdentifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::ForceSectorActivation failed: no loaded world");
		return;
	}

	UFlareSimulatedSector* Sector = GetGameWorld()->FindSector(SectorIdentifier);

	if (!Sector)
	{
		FLOGV("AFlareGame::ForceSectorActivation failed: no sector with id '%s'", *SectorIdentifier.ToString());
		return;
	}

	GetGame()->ActivateSector(Sector);
}

void UFlareGameTools::ForceSectorDeactivation()
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::ForceSectorDeactivation failed: no loaded world");
		return;
	}

	GetGame()->DeactivateSector();
}

void UFlareGameTools::SetDefaultWeapon(FName NewDefaultWeaponIdentifier)
{
	FFlareSpacecraftComponentDescription* ComponentDescription = GetGame()->GetShipPartsCatalog()->Get(NewDefaultWeaponIdentifier);

	if (ComponentDescription && ComponentDescription->WeaponCharacteristics.IsWeapon)
	{
		GetGame()->SetDefaultWeaponIdentifier(NewDefaultWeaponIdentifier);
	}
	else
	{
		FLOGV("Bad weapon identifier: %s", *NewDefaultWeaponIdentifier.ToString())
	}
}

void UFlareGameTools::SetDefaultTurret(FName NewDefaultTurretIdentifier)
{
	FFlareSpacecraftComponentDescription* ComponentDescription = GetGame()->GetShipPartsCatalog()->Get(NewDefaultTurretIdentifier);

	if (ComponentDescription && ComponentDescription->WeaponCharacteristics.IsWeapon && ComponentDescription->WeaponCharacteristics.TurretCharacteristics.IsTurret)
	{
		GetGame()->SetDefaultTurretIdentifier(NewDefaultTurretIdentifier);
	}
	else
	{
		FLOGV("Bad weapon identifier: %s", *NewDefaultTurretIdentifier.ToString())
	}
}

void UFlareGameTools::SetCulture(FName CultureName)
{
	if(FInternationalization::Get().SetCurrentCulture(CultureName.ToString()))
	{
		FLOGV("Culture is now %s", *FInternationalization::Get().GetCurrentCulture()->GetDisplayName());
	}
	else
	{
		FLOGV("Fail to find culture %s", *CultureName.ToString());
	}

}

void UFlareGameTools::PickPoint()
{
	FLOG("AFlareGame::CreateShipInCompany");
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::PickPoint failed: no active sector");
		return;
	}

	AFlarePlayerController* PC = GetPC();
	AFlareSpacecraft* PlayerShip = PC->GetShipPawn();

	FVector CameraAimDirection = PlayerShip->GetCamera()->GetComponentRotation().Vector();
	FVector CameraLocation = PlayerShip->GetCamera()->GetComponentLocation();
	CameraAimDirection.Normalize();


	FHitResult HitResult(ForceInit);


	auto Trace = [this, &PlayerShip](const FVector& Start, const FVector& End, FHitResult& HitOut)
	{
		// Ignore Actors
		FCollisionQueryParams TraceParams(FName(TEXT("Pick point Trace")), true, PlayerShip);
		TraceParams.bTraceComplex = true;
		TraceParams.bReturnPhysicalMaterial = false;
		TraceParams.AddIgnoredActor(PlayerShip);

		// Re-initialize hit info
		HitOut = FHitResult(ForceInit);

		ECollisionChannel CollisionChannel = (ECollisionChannel) (ECC_WorldStatic | ECC_WorldDynamic | ECC_Pawn);

		// Trace!
		GetWorld()->LineTraceSingleByChannel(
			HitOut,		// result
			Start,	// start
			End , // end
			CollisionChannel, // collision channel
			TraceParams
		);

		// Hit any Actor?
		return (HitOut.GetActor() != NULL) ;
	};

	if (Trace(CameraLocation, CameraLocation + CameraAimDirection * 50000, HitResult))
	{
		FLOGV("PickPoint : FVector(%f, %f, %f)", HitResult.Location.X, HitResult.Location.Y, HitResult.Location.Z )
		UKismetSystemLibrary::DrawDebugPoint(PlayerShip->GetWorld(), HitResult.Location, 100, FColor::Green, 5.f);
	}
	else
	{
		FLOG("AFlareGame::PickPoint failed: no hit");
	}
}

void UFlareGameTools::TakeHighResScreenShot()
{
	GetPC()->TakeHighResScreenshot();
}


#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */


void UFlareGameTools::CheckEconomyBalance()
{
	FLOG("=============");
	FLOG("Economy check");
	FLOG("=============");
	FLOG("");

	TArray<UFlareResourceCatalogEntry*> ResourceEntries = GetGame()->GetResourceCatalog()->Resources;
	for(int ResourceIndex = 0; ResourceIndex < ResourceEntries.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &ResourceEntries[ResourceIndex]->Data;

		FLOGV("Resource '%s'", *Resource->Name.ToString());
		FLOGV("- Min price %.2f $", Resource->MinPrice/100.);
		FLOGV("- Max price %.2f $", Resource->MaxPrice/100.);
		FLOGV("- Transport fee %.2f $", Resource->TransportFee/100.);
	}



	TArray<UFlareSpacecraftCatalogEntry*>& StationCatalog = GetGame()->GetSpacecraftCatalog()->StationCatalog;

	for (int32 StationIndex = 0; StationIndex < StationCatalog.Num(); StationIndex++)
	{
		FFlareSpacecraftDescription* StationDescription = &StationCatalog[StationIndex]->Data;

		FLOGV("Station '%s'", *StationDescription->Name.ToString());
		FLOG( "-------");

		FLOGV("- Factory count: %d", StationDescription->Factories.Num());


		for (int FactoryIndex = 0; FactoryIndex < StationDescription->Factories.Num(); FactoryIndex++)
		{
			FFlareFactoryDescription* FactoryDescription = &StationDescription->Factories[FactoryIndex]->Data;

			if(FactoryDescription->IsShipyard())
			{
				FLOGV("  - Shipyard %d", FactoryIndex);
				continue;
			}

			int64 MinRevenue= 0;
			int64 MaxRevenue = 0;
			int64 MinCost = 0;
			int64 MaxCost = 0;


			FString Cycle;

			MinCost += FactoryDescription->CycleCost.ProductionCost;
			MaxCost += FactoryDescription->CycleCost.ProductionCost;

			for (int32 ResourceIndex = 0 ; ResourceIndex < FactoryDescription->CycleCost.InputResources.Num() ; ResourceIndex++)
			{
				const FFlareFactoryResource* Resource = &FactoryDescription->CycleCost.InputResources[ResourceIndex];

				MinCost += (Resource->Resource->Data.MinPrice + Resource->Resource->Data.TransportFee) * Resource->Quantity;
				MaxCost += (Resource->Resource->Data.MaxPrice + Resource->Resource->Data.TransportFee) * Resource->Quantity;

				if(Cycle.Len() > 0)
				{
					Cycle += " + ";
				}
				Cycle.Append(FString::FromInt(Resource->Quantity));
				Cycle.Append(FString(" "));
				Cycle.Append(Resource->Resource->Data.Name.ToString());
			}

			if(Cycle.Len() > 0)
			{
				Cycle += " + ";
			}
			Cycle.Append(FString::FromInt(FactoryDescription->CycleCost.ProductionCost/100));

			Cycle += " $ -> ";

			for (int32 ResourceIndex = 0 ; ResourceIndex < FactoryDescription->CycleCost.OutputResources.Num() ; ResourceIndex++)
			{
				const FFlareFactoryResource* Resource = &FactoryDescription->CycleCost.OutputResources[ResourceIndex];

				MinRevenue += (Resource->Resource->Data.MinPrice - (Resource->Resource->Data.TransportFee *0.5f)) * Resource->Quantity;
				MaxRevenue += (Resource->Resource->Data.MaxPrice - (Resource->Resource->Data.TransportFee *0.5f)) * Resource->Quantity;

				if(Cycle.Len() > 4)
				{
					Cycle += " + ";
				}
				Cycle += FString::FromInt(Resource->Quantity) + " " + Resource->Resource->Data.Name.ToString();
			}


			int64 MinBenefice = MinRevenue - MaxCost;
			int64 MaxBenefice = MaxRevenue - MinCost;


			float MinMargin = (float) MinBenefice / (float)MinRevenue;
			float MaxMargin = (float) MaxBenefice / (float) MaxRevenue;


			FLOGV("- Factory %d", FactoryIndex);
			FLOGV("  - Cycle : '%s'", *Cycle);
			FLOGV("  - Min revenue %.2f $", MinRevenue / 100.);
			FLOGV("  - Max revenue %.2f $", MaxRevenue / 100.);
			FLOGV("  - Min cost %.2f $", MinCost / 100.);
			FLOGV("  - Max cost %.2f $", MaxCost / 100.);
			FLOGV("  - Min benefice %.2f $", MinBenefice / 100.);
			FLOGV("  - Max benefice %.2f $", MaxBenefice / 100.);
			FLOGV("  - Min margin %.2f %%", MinMargin*100);
			FLOGV("  - Max margin %.2f %%", MaxMargin*100);


			if(MinMargin < 0)
			{

				for (int32 ResourceIndex = 0 ; ResourceIndex < FactoryDescription->CycleCost.OutputResources.Num() ; ResourceIndex++)
				{
					const FFlareFactoryResource* Resource = &FactoryDescription->CycleCost.OutputResources[ResourceIndex];

					float PriceOverflowPerResource = (float) MinBenefice / (float) Resource->Quantity;
					int64 BestMinPrice = Resource->Resource->Data.MinPrice - PriceOverflowPerResource;
					int64 BestMaxPrice = BestMinPrice * 1.2f;

					if(BestMinPrice !=  Resource->Resource->Data.MinPrice)
					{
						FLOG(RED "!!!!!!!!!!!!!!!!!!!!!!!!!!!" RESET);
						FLOG(RED "!! Min margin is too low !!" RESET);

						FLOGV(RED "!! %s min price %lld -> %lld !!" RESET,
							 *Resource->Resource->Data.Identifier.ToString(),
							 Resource->Resource->Data.MinPrice,
							 BestMinPrice);

						FLOGV(RED "!! %s max price %lld -> %lld !!" RESET,
							 *Resource->Resource->Data.Identifier.ToString(),
							 Resource->Resource->Data.MaxPrice,
							 BestMaxPrice);
						FLOG(RED "!!!!!!!!!!!!!!!!!!!!!!!!!!!" RESET);
					}
				}


			}

			if(MinMargin > 0)
			{

				for (int32 ResourceIndex = 0 ; ResourceIndex < FactoryDescription->CycleCost.OutputResources.Num() ; ResourceIndex++)
				{
					const FFlareFactoryResource* Resource = &FactoryDescription->CycleCost.OutputResources[ResourceIndex];

					float PriceOverflowPerResource = (float) MinBenefice / (float) Resource->Quantity;
					int64 BestMinPrice = Resource->Resource->Data.MinPrice - PriceOverflowPerResource;
					int64 BestMaxPrice = BestMinPrice * 1.2f;

					if(BestMinPrice !=  Resource->Resource->Data.MinPrice)
					{
						FLOG(RED "!!!!!!!!!!!!!!!!!!!!!!!!!!!" RESET);
						FLOG(RED "!! Min margin is too high !!" RESET);

						FLOGV(RED "!! %s min price %lld -> %lld !!" RESET,
							 *Resource->Resource->Data.Identifier.ToString(),
							 Resource->Resource->Data.MinPrice,
							 BestMinPrice);

						FLOGV(RED "!! %s max price %lld -> %lld !!" RESET,
							 *Resource->Resource->Data.Identifier.ToString(),
							 Resource->Resource->Data.MaxPrice,
							 BestMaxPrice);
						FLOG(RED "!!!!!!!!!!!!!!!!!!!!!!!!!!!" RESET);
					}
				}


			}
		}



		FLOG( "");
	}



}


void UFlareGameTools::PrintEconomyStatus()
{
	FLOG("=============");
	FLOG("Economy status");
	FLOG("=============");
	FLOG("");

	TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> WorldStats;
	WorldStats = WorldHelper::ComputeWorldResourceStats(GetGame(), true);


	TArray<UFlareResourceCatalogEntry*> ResourceEntries = GetGame()->GetResourceCatalog()->Resources;
	for(int ResourceIndex = 0; ResourceIndex < ResourceEntries.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &ResourceEntries[ResourceIndex]->Data;

		if (WorldStats.Contains(Resource))
		{
			FLOGV("Resource '%s'", *Resource->Name.ToString());
			FLOGV("- Stock: %d", WorldStats[Resource].Stock);
			FLOGV("- Production: %.2f", WorldStats[Resource].Production);
			FLOGV("- Consumption: %.2f", WorldStats[Resource].Consumption);
			if(WorldStats[Resource].Balance < 0)
			{
				FLOGV("- " RED "Balance: %.2f" RESET, WorldStats[Resource].Balance);
			}
			else
			{
				FLOGV("- Balance: %.2f", WorldStats[Resource].Balance);
			}
		}
	}


	int64 PeopleMoney = 0;
	int64 PeopleDept = 0;
	int64 Population = 0;

	for (int SectorIndex = 0; SectorIndex < GetGameWorld()->GetSectors().Num(); SectorIndex++)
	{
		PeopleMoney += GetGameWorld()->GetSectors()[SectorIndex]->GetPeople()->GetMoney();
		PeopleDept += GetGameWorld()->GetSectors()[SectorIndex]->GetPeople()->GetDept();
		Population += GetGameWorld()->GetSectors()[SectorIndex]->GetPeople()->GetPopulation();
	}

	int64 WorldMoney = GetGameWorld()->GetWorldMoney();

	FLOGV("World population: %lld ", Population);

	FLOGV("World money: %lld $", WorldMoney / 100);
	FLOGV("- People money: %lld $ (%f %%)", PeopleMoney/100, 100.f * (float)PeopleMoney / (float) WorldMoney);
	FLOGV("- People dept: %lld $ (%f %%)", PeopleDept/100, 100.f * (float)PeopleDept / (float) PeopleMoney);
}

void UFlareGameTools::SetAutoSave(bool Autosave)
{
	GetGame()->AutoSave = Autosave;
}

void UFlareGameTools::DisableAutoSave()
{
	SetAutoSave(false);
	GetGame()->SaveGame(GetGame()->GetPC(), false, true);
}

void UFlareGameTools::ReloadGameWithoutSave()
{
	//bool Autosave = GetGame()->AutoSave;
	//GetGame()->AutoSave = false;

	GetGame()->UnloadGame();
	GetGame()->GetPC()->GetMenuManager()->OpenMenu(EFlareMenu::MENU_LoadGame);



	//GetGame()->AutoSave = Autosave;
}

void UFlareGameTools::CompleteQuest()
{
	UFlareQuest* SelectedQuest = GetGame()->GetQuestManager()->GetSelectedQuest();
	if(SelectedQuest)
	{
		FLOGV("Complete quest '%s'", *SelectedQuest->GetQuestName().ToString());
		SelectedQuest->Success();
	}
	else
	{
		FLOG("No selected quest to complete");
	}
}

void UFlareGameTools::CompleteQuestStep()
{
	UFlareQuest* SelectedQuest = GetGame()->GetQuestManager()->GetSelectedQuest();
	if(SelectedQuest)
	{
		if (SelectedQuest->GetCurrentStep())
		{
			FLOGV("Complete quest step '%s' of quest '%s'", *SelectedQuest->GetQuestName().ToString(), *SelectedQuest->GetCurrentStep()->GetIdentifier().ToString());
			SelectedQuest->EndStep();
		}
		else
		{
			FLOGV("No current step to complete in quest '%s'", *SelectedQuest->GetQuestName().ToString());
		}
	}
	else
	{
		FLOG("No selected quest to complete a step");
	}
}


/*----------------------------------------------------
	World tools
----------------------------------------------------*/

int64 UFlareGameTools::GetWorldDate()
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::GetWorldDate failed: no loaded world");
		return 0;
	}
	FLOGV("World date: %lld", GetGameWorld()->GetDate());
	return GetGameWorld()->GetDate();
}

void UFlareGameTools::SetWorldDate(int64 Date)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::SetWorldDate failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::SetWorldDate failed: a sector is active");
		return;
	}

	GetGameWorld()->ForceDate(Date);
}


void UFlareGameTools::Simulate()
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::Simulate failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::Simulate failed: a sector is active");
		return;
	}

	GetGame()->DeactivateSector();
	GetGameWorld()->Simulate();
	GetGame()->ActivateCurrentSector();
}

void UFlareGameTools::SetPlanatariumTimeMultiplier(float Multiplier)
{
	GetGame()->GetPlanetarium()->SetTimeMultiplier(Multiplier);
}

void UFlareGameTools::RevealMap()
{
	if (!GetGameWorld())
	{
		FLOG("UFlareGameTools::RevealMap failed: no loaded world");
		return;
	}

	GetGame()->DeactivateSector();
	for (int i = 0; i < GetGameWorld()->GetSectors().Num(); i++)
	{
		UFlareSimulatedSector* Sector = GetGameWorld()->GetSectors()[i];
		GetPC()->DiscoverSector(Sector, true, false);
	}
	GetGame()->ActivateCurrentSector();
}

void UFlareGameTools::SetFastFastForward(bool FFF)
{
	FastFastForward = FFF;
}

/*----------------------------------------------------
	Company tools
----------------------------------------------------*/

void UFlareGameTools::DeclareWar(FName Company1ShortName, FName Company2ShortName)
{
	if (!GetGameWorld())
	{
		FLOG("UFlareGameTools::DeclareWar failed: no loaded world");
		return;
	}


	UFlareCompany* Company1 = GetGameWorld()->FindCompanyByShortName(Company1ShortName);
	UFlareCompany* Company2 = GetGameWorld()->FindCompanyByShortName(Company2ShortName);

	if (Company1 && Company2 && Company1 != Company2)
	{
		FLOGV("Declare war between %s and %s", *Company1->GetCompanyName().ToString(), *Company2->GetCompanyName().ToString());
		Company1->SetHostilityTo(Company2, true);
		Company2->SetHostilityTo(Company1, true);

		// Notify war
		AFlarePlayerController* PC = GetPC();
		FText WarText = LOCTEXT("War", "War has been declared");
		FText WarInfoText = FText::Format(LOCTEXT("WarStringInfoFormat", "{0}, {1} are now at war"), Company1->GetCompanyName(), Company2->GetCompanyName());
		PC->Notify(WarText, WarInfoText, NAME_None, EFlareNotification::NT_Military);
	}
}

void UFlareGameTools::MakePeace(FName Company1ShortName, FName Company2ShortName)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::MakePeace failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::MakePeace failed: a sector is active");
		return;
	}


	UFlareCompany* Company1 = GetGameWorld()->FindCompanyByShortName(Company1ShortName);
	UFlareCompany* Company2 = GetGameWorld()->FindCompanyByShortName(Company2ShortName);

	if (Company1 && Company2)
	{
		Company1->SetHostilityTo(Company2, false);
		Company2->SetHostilityTo(Company1, false);
	}
}

void UFlareGameTools::PrintCompany(FName CompanyShortName)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintCompany failed: no loaded world");
		return;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("AFlareGame::PrintCompany failed: no company with short name '%s'", * CompanyShortName.ToString());
		return;
	}

	FLOGV("> PrintCompany: %s - %s (%s)", *Company->GetIdentifier().ToString(), *Company->GetCompanyName().ToString(), *Company->GetShortName().ToString());
	struct CompanyValue Value = Company->GetCompanyValue();
	FLOGV("  > %f $", Value.TotalValue / 100.);
	FLOGV("    - Money %f $", Value.MoneyValue / 100.);
	FLOGV("    - Stocks %f $", Value.StockValue/ 100.);
	FLOGV("    - Spacecrafts %f $", Value.SpacecraftsValue/ 100.);
	FLOGV("      - Ships %f $", Value.ShipsValue/ 100.);
	FLOGV("      - Stations %f $", Value.StationsValue/ 100.);


	FLOGV("    - Army %f $", Value.ArmyValue/ 100.);
	FLOGV("    - Diplomatic weigh %f", Company->ComputeCompanyDiplomaticWeight());
	FLOGV("    - Army current combat point %d", Value.ArmyCurrentCombatPoints);
	FLOGV("    - Army total combat point %d", Value.ArmyTotalCombatPoints);
	TArray<UFlareFleet*> CompanyFleets = Company->GetCompanyFleets();
	FLOGV("  > %d fleets", CompanyFleets.Num());
	for (int i = 0; i < CompanyFleets.Num(); i++)
	{
		UFlareFleet* Fleet = CompanyFleets[i];
		FLOGV("   %2d - %s: %s", i,  *Fleet->GetIdentifier().ToString(), *Fleet->GetFleetName().ToString());
	}

	TArray<UFlareTradeRoute*> CompanyTradeRoutes = Company->GetCompanyTradeRoutes();
	FLOGV("  > %d trade routes", CompanyTradeRoutes.Num());
	for (int i = 0; i < CompanyTradeRoutes.Num(); i++)
	{
		UFlareTradeRoute* TradeRoute = CompanyTradeRoutes[i];
		FLOGV("   %2d - %s: %s", i,  *TradeRoute->GetIdentifier().ToString(), *TradeRoute->GetTradeRouteName().ToString());
	}

	TArray<UFlareSimulatedSpacecraft*> CompanySpacecrafts = Company->GetCompanySpacecrafts();
	FLOGV("  > %d spacecrafts (%d ships and %d stations)", CompanySpacecrafts.Num(), Company->GetCompanyShips().Num(), Company->GetCompanyStations().Num());
	for (int i = 0; i < CompanySpacecrafts.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = CompanySpacecrafts[i];
		FLOGV("   %2d - %s", i,  *Spacecraft->GetImmatriculation().ToString());
	}

	TArray<UFlareCompany*> Companies = GetGame()->GetGameWorld()->GetCompanies();
	FLOG("  > Diplomacy");
	FLOGV("  > - Player reputation %f", Company->GetPlayerReputation());
	for (int i = 0; i < Companies.Num(); i++)
	{
		UFlareCompany* OtherCompany = Companies[i];
		if(OtherCompany == Company)
		{
			continue;
		}
		TArray<UFlareCompany*> Allies;

		FLOGV("   - %s want %s with %s (confidence: %f). %s want %s with %s (confidence: %f)",
			  *Company->GetCompanyName().ToString(),
			  (Company->GetHostility(OtherCompany) == EFlareHostility::Hostile ? *FString("war") : *FString("peace")),
			  *OtherCompany->GetCompanyName().ToString(),
			  Company->GetConfidenceLevel(OtherCompany, Allies),
			  *OtherCompany->GetCompanyName().ToString(),
			  (OtherCompany->GetHostility(Company) == EFlareHostility::Hostile ? *FString("war") : *FString("peace")),
			  *Company->GetCompanyName().ToString(),
			  OtherCompany->GetConfidenceLevel(Company, Allies));
	}
}

void UFlareGameTools::PrintCompanyByIndex(int32 Index)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintCompanyByIndex failed: no loaded world");
		return;
	}

	TArray<UFlareCompany*> Companies = GetGameWorld()->GetCompanies();
	if (Index < 0 || Index > Companies.Num() -1)
	{
		FLOGV("AFlareGame::PrintCompanyByIndex failed: invalid index %d, with %d companies.", Index, Companies.Num());
		return;
	}

	PrintCompany(Companies[Index]->GetShortName());
}

void UFlareGameTools::GivePlayerReputation(FName CompanyShortName, float Amount)
{
	if (!GetGameWorld())
	{
		FLOG("UFlareGameTools::GivePlayerReputation failed: no loaded world");
		return;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("UFlareGameTools::GivePlayerReputation failed: no company with short name '%s'", * CompanyShortName.ToString());
		return;
	}

	Company->GivePlayerReputation(Amount);
}


void UFlareGameTools::TakeCompanyControl(FName CompanyShortName)
{
	if (!GetGameWorld())
	{
		FLOG("UFlareGameTools::TakeCompanyControl failed: no loaded world");
		return;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("UFlareGameTools::TakeCompanyControl failed: no company with short name '%s'", * CompanyShortName.ToString());
		return;
	}

	GetGame()->GetPC()->SetCompanyDescription(*Company->GetDescription());
	FFlarePlayerSave SavePlayerData;
	SavePlayerData.CompanyIdentifier = Company->GetIdentifier();
	SavePlayerData.LastFlownShipIdentifier = NAME_None;
	GetGame()->GetPC()->Load(SavePlayerData);
}


/*----------------------------------------------------
	Fleet tools
----------------------------------------------------*/

void UFlareGameTools::CreateFleet(FString FleetName, FName FirstShipImmatriculation)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::CreateFleet failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::CreateFleet failed: a sector is active");
		return;
	}

	UFlareSimulatedSpacecraft* Ship = GetGameWorld()->FindSpacecraft(FirstShipImmatriculation);
	if (!Ship)
	{
		FLOGV("AFlareGame::CreateFleet failed: no Ship with immatriculation '%s'", *FirstShipImmatriculation.ToString());
		return;
	}


	UFlareCompany* FleetCompany = Ship->GetCompany();
	UFlareSimulatedSector* FleetSector = Ship->GetCurrentSector();


	UFlareFleet* Fleet = FleetCompany->CreateFleet(FText::FromString(FleetName), FleetSector);
	Fleet->AddShip(Ship);
}

void UFlareGameTools::DisbandFleet(FName FleetIdentifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::DisbandFleet failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::DisbandFleet failed: a sector is active");
		return;
	}

	UFlareFleet* Fleet = GetGameWorld()->FindFleet(FleetIdentifier);
	if (!Fleet)
	{
		FLOGV("AFlareGame::DisbandFleet failed: no fleet with id '%s'", *FleetIdentifier.ToString());
		return;
	}

	Fleet->Disband();
}


void UFlareGameTools::AddToFleet(FName FleetIdentifier, FName ShipImmatriculation)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::AddToFleet failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::AddToFleet failed: a sector is active");
		return;
	}

	UFlareFleet* Fleet = GetGameWorld()->FindFleet(FleetIdentifier);
	if (!Fleet)
	{
		FLOGV("AFlareGame::AddToFleet failed: no fleet with id '%s'", *FleetIdentifier.ToString());
		return;
	}

	UFlareSimulatedSpacecraft* Ship = GetGameWorld()->FindSpacecraft(ShipImmatriculation);
	if (!Ship)
	{
		FLOGV("AFlareGame::AddToFleet failed: no Ship with immatriculation '%s'", *ShipImmatriculation.ToString());
		return;
	}
	Fleet->AddShip(Ship);
}


void UFlareGameTools::RemoveFromFleet(FName FleetIdentifier, FName ShipImmatriculation)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::RemoveFromFleet failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::RemoveFromFleet failed: a sector is active");
		return;
	}

	UFlareFleet* Fleet = GetGameWorld()->FindFleet(FleetIdentifier);
	if (!Fleet)
	{
		FLOGV("AFlareGame::RemoveFromFleet failed: no fleet with id '%s'", *FleetIdentifier.ToString());
		return;
	}

	UFlareSimulatedSpacecraft* Ship = GetGameWorld()->FindSpacecraft(ShipImmatriculation);
	if (!Ship)
	{
		FLOGV("AFlareGame::RemoveFromFleet failed: no Ship with immatriculation '%s'", *ShipImmatriculation.ToString());
		return;
	}
	Fleet->RemoveShip(Ship);
}

void UFlareGameTools::MergeFleets(FName Fleet1Identifier, FName Fleet2Identifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::MergeFleets failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::MergeFleets failed: a sector is active");
		return;
	}

	UFlareFleet* Fleet1 = GetGameWorld()->FindFleet(Fleet1Identifier);
	if (!Fleet1)
	{
		FLOGV("AFlareGame::MergeFleets failed: no fleet with id '%s'", *Fleet1Identifier.ToString());
		return;
	}

	UFlareFleet* Fleet2 = GetGameWorld()->FindFleet(Fleet2Identifier);
	if (!Fleet2)
	{
		FLOGV("AFlareGame::MergeFleets failed: no fleet with id '%s'", *Fleet2Identifier.ToString());
		return;
	}

	Fleet1->Merge(Fleet2);
}

/*----------------------------------------------------
	Trade route tools
----------------------------------------------------*/

void UFlareGameTools::CreateTradeRoute(FString TradeRouteName, FName CompanyShortName)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::CreateTradeRoute failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::CreateTradeRoute failed: a sector is active");
		return;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("AFlareGame::CreateTradeRoute failed: no company with short name '%s'", * CompanyShortName.ToString());
		return;
	}

	Company->CreateTradeRoute(FText::FromString(TradeRouteName));
}

void UFlareGameTools::DissolveTradeRoute(FName TradeRouteIdentifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::DisbandFleet failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::DisbandFleet failed: a sector is active");
		return;
	}

	UFlareTradeRoute* TradeRoute = GetGameWorld()->FindTradeRoute(TradeRouteIdentifier);
	if (!TradeRoute)
	{
		FLOGV("AFlareGame::DissolveTradeRoute failed: no trade route with id '%s'", *TradeRouteIdentifier.ToString());
		return;
	}

	TradeRoute->Dissolve();
}


void UFlareGameTools::AddToTradeRoute(FName TradeRouteIdentifier, FName FleetIdentifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::AddToTradeRoute failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::AddToTradeRoute failed: a sector is active");
		return;
	}

	UFlareTradeRoute* TradeRoute = GetGameWorld()->FindTradeRoute(TradeRouteIdentifier);
	if (!TradeRoute)
	{
		FLOGV("AFlareGame::AddToTradeRoute failed: no trade route with id '%s'", *TradeRouteIdentifier.ToString());
		return;
	}

	UFlareFleet* Fleet = GetGameWorld()->FindFleet(FleetIdentifier);
	if (!Fleet)
	{
		FLOGV("AFlareGame::AddToTradeRoute failed: no fleet with id '%s'", *FleetIdentifier.ToString());
		return;
	}

	TradeRoute->AssignFleet(Fleet);
}


void UFlareGameTools::RemoveFromTradeRoute(FName TradeRouteIdentifier, FName FleetIdentifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::RemoveFromTradeRoute failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::RemoveFromTradeRoute failed: a sector is active");
		return;
	}

	UFlareTradeRoute* TradeRoute = GetGameWorld()->FindTradeRoute(TradeRouteIdentifier);
	if (!TradeRoute)
	{
		FLOGV("AFlareGame::RemoveFromTradeRoute failed: no trade route with id '%s'", *TradeRouteIdentifier.ToString());
		return;
	}

	UFlareFleet* Fleet = GetGameWorld()->FindFleet(FleetIdentifier);
	if (!Fleet)
	{
		FLOGV("AFlareGame::RemoveFromTradeRoute failed: no fleet with id '%s'", *FleetIdentifier.ToString());
		return;
	}

	TradeRoute->RemoveFleet(Fleet);
}

/*----------------------------------------------------
	Travel tools
----------------------------------------------------*/

void UFlareGameTools::StartTravel(FName FleetIdentifier, FName SectorIdentifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::StartTravel failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::StartTravel failed: a sector is active");
		return;
	}

	UFlareFleet* Fleet = GetGameWorld()->FindFleet(FleetIdentifier);
	if (!Fleet)
	{
		FLOGV("AFlareGame::StartTravel failed: no fleet with id '%s'", *FleetIdentifier.ToString());
		return;
	}

	UFlareSimulatedSector* Sector = GetGameWorld()->FindSector(SectorIdentifier);

	if (!Sector)
	{
		FLOGV("AFlareGame::StartTravel failed: no sector with id '%s'", *SectorIdentifier.ToString());
		return;
	}

	GetGameWorld()->StartTravel(Fleet, Sector);
}


void UFlareGameTools::PrintTravelList()
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintTravelList failed: no loaded world");
		return;
	}


	FLOGV("> PrintTravelList: %d travels", GetGameWorld()->GetTravels().Num());

	for (int i = 0; i < GetGameWorld()->GetTravels().Num(); i++)
	{
		UFlareTravel* Travel = GetGameWorld()->GetTravels()[i];
		FLOGV("%2d - %s to %s	",
			i,
			*Travel->GetFleet()->GetFleetName().ToString(),
			*Travel->GetDestinationSector()->GetSectorName().ToString());
	}
}


void UFlareGameTools::PrintTravelByIndex(int32 Index)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintTravelByIndex failed: no loaded world");
		return;
	}

	TArray<UFlareTravel*>Travels= GetGameWorld()->GetTravels();
	if (Index < 0 || Index > Travels.Num() -1)
	{
		FLOGV("AFlareGame::PrintTravelByIndex failed: invalid index %d, with %d travels.", Index, Travels.Num());
		return;
	}

	UFlareTravel* Travel = GetGameWorld()->GetTravels()[Index];
	FLOGV("> PrintTravel %s to %s",
		*Travel->GetFleet()->GetFleetName().ToString(),
		*Travel->GetDestinationSector()->GetSectorName().ToString());
	FLOGV("  - Departure date: day %lld ", Travel->GetDepartureDate());
	FLOGV("  - Elapsed time: %lld days", Travel->GetElapsedTime());

}

/*----------------------------------------------------
	Sector tools
----------------------------------------------------*/

UFlareSimulatedSpacecraft* UFlareGameTools::CreateShipForMeInSector(FName ShipClass, FName SectorIdentifier, bool UnderConstruction)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::CreateShipForMeInSector failed: no world");
		return NULL;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::CreateStationForMe failed: a sector is active");
		return NULL;
	}



	UFlareSimulatedSector* Sector = GetGameWorld()->FindSector(SectorIdentifier);

	if (!Sector)
	{
		FLOGV("AFlareGame::CreateShipForMeInSector failed: no sector '%s'", *SectorIdentifier.ToString());
		return NULL;
	}

	AFlarePlayerController* PC = GetPC();

	UFlareSimulatedSpacecraft* ShipPawn = Sector->CreateStation(ShipClass, PC->GetCompany(), UnderConstruction);

	return ShipPawn;
}

UFlareSimulatedSpacecraft* UFlareGameTools::CreateStationInCompanyAttachedInSector(FName StationClass, FName CompanyShortName, FName SectorIdentifier, bool UnderConstruction)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::CreateStationInCompanyAttachedInSector failed: no world");
		return NULL;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::CreateStationInCompanyAttachedInSector failed: a sector is active");
		return NULL;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("AFlareGame::CreateStationInCompanyAttachedInSector failed : No company named '%s'", *CompanyShortName.ToString());
		return NULL;
	}

	UFlareSimulatedSector* Sector = GetGameWorld()->FindSector(SectorIdentifier);

	if (!Sector)
	{
		FLOGV("AFlareGame::CreateStationInCompanyAttachedInSector failed: no sector '%s'", *SectorIdentifier.ToString());
		return NULL;
	}

	UFlareSimulatedSpacecraft* NewStation = Sector->CreateStation(StationClass, Company, UnderConstruction);
	return NewStation;
}


UFlareSimulatedSpacecraft* UFlareGameTools::CreateChildStation(FName StationClass, FName ComplexName, int32 ConnectorId, bool UnderConstruction)
{
	bool IsActiveSector = GetActiveSector() != nullptr;


	UFlareSimulatedSpacecraft* Complex = GetGameWorld()->FindSpacecraft(ComplexName);
	if (!Complex)
	{
		FLOGV("UFlareSector::CreateChildStation failed : No complex named '%s'", *ComplexName.ToString());
		return NULL;
	}

	if(!Complex->IsComplex())
	{
		FLOGV("UFlareSector::CreateChildStation failed : '%s' is not a complex", *ComplexName.ToString());
		return NULL;
	}

	if(ConnectorId >= Complex->GetStationConnectors().Num())
	{
		FLOGV("UFlareSector::CreateChildStation failed : '%d' is not a valid complex connector index", ConnectorId);
		return NULL;
	}

	FFlareDockingInfo& Connector = Complex->GetStationConnectors()[ConnectorId];

	if (Connector.Granted)
	{
		FLOGV("UFlareSector::CreateChildStation failed : connector '%d' is already granted", ConnectorId);
		return NULL;
	}

	if(IsActiveSector)
	{
		GetGame()->DeactivateSector();
	}

	FFlareStationSpawnParameters SpawnParameters;
	SpawnParameters.AttachComplexStationName = Complex->GetImmatriculation();
	SpawnParameters.AttachComplexConnectorName = Connector.Name;
	UFlareSimulatedSpacecraft* NewStation = Complex->GetCurrentSector()->CreateStation(StationClass, Complex->GetCompany(), UnderConstruction, SpawnParameters);
	if(IsActiveSector)
	{
		GetGame()->ActivateCurrentSector();
	}

	return NewStation;
}

void UFlareGameTools::PrintSectorList()
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintSectorList failed: no loaded world");
		return;
	}


	FLOGV("> PrintSectorList: %d sectors", GetGameWorld()->GetSectors().Num());

	for (int i = 0; i < GetGameWorld()->GetSectors().Num(); i++)
	{
		UFlareSimulatedSector* Sector = GetGameWorld()->GetSectors()[i];
		FLOGV("%2d - %s: %s (%s)", i, *Sector->GetIdentifier().ToString(), *Sector->GetSectorName().ToString(), *Sector->GetSectorCode());
	}
}


void UFlareGameTools::PrintSector(FName SectorIdentifier)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintSector failed: no loaded world");
		return;
	}

	UFlareSimulatedSector* Sector = GetGameWorld()->FindSector(SectorIdentifier);
	if (!Sector)
	{
		FLOGV("AFlareGame::PrintSector failed: no sector with identifier '%s'", * SectorIdentifier.ToString());
		return;
	}

	FLOGV("> PrintSector: %s - %s (%s)", *Sector->GetIdentifier().ToString(), *Sector->GetSectorName().ToString(), *Sector->GetSectorCode());

	TArray<UFlareFleet*> SectorFleets = Sector->GetSectorFleets();
	FLOGV("  > %d fleets", SectorFleets.Num());
	for (int i = 0; i < SectorFleets.Num(); i++)
	{
		UFlareFleet* Fleet = SectorFleets[i];
		FLOGV("   %2d - %s: %s", i,  *Fleet->GetIdentifier().ToString(), *Fleet->GetFleetName().ToString());
	}

	TArray<UFlareSimulatedSpacecraft*> SectorShips = Sector->GetSectorShips();
	FLOGV("  > %d ships", SectorShips.Num());
	for (int i = 0; i < SectorShips.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = SectorShips[i];
		FLOGV("   %2d - %s (%d)", i,  *Spacecraft->GetImmatriculation().ToString(), Spacecraft->IsStation());
	}

	TArray<UFlareSimulatedSpacecraft*> SectorStations = Sector->GetSectorStations();
	FLOGV("  > %d stations", SectorStations.Num());
	for (int i = 0; i < SectorStations.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = SectorStations[i];
		FLOGV("%d", i);
		FLOGV("%s", *Spacecraft->GetImmatriculation().ToString());
		FLOGV("   %2d - %s (%d)", i,  *Spacecraft->GetImmatriculation().ToString(), Spacecraft->IsStation());
	}

	//Prices
	FLOG("  > prices");
	for(int32 ResourceIndex = 0; ResourceIndex < GetGame()->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &GetGame()->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		FLOGV("   - %s : %f credits", *Resource->Name.ToString(), Sector->GetPreciseResourcePrice(Resource) / 100.);
	}

	//People
	FLOG("  > people");
	Sector->GetPeople()->PrintInfo();
}


void UFlareGameTools::PrintSectorByIndex(int32 Index)
{
	if (!GetGameWorld())
	{
		FLOG("UFlareGameTools::PrintSectorByIndex failed: no loaded world");
		return;
	}

	TArray<UFlareSimulatedSector*> Sectors= GetGameWorld()->GetSectors();
	if (Index < 0 || Index > Sectors.Num() -1)
	{
		FLOGV("UFlareGameTools::PrintSectorByIndex failed: invalid index %d, with %d sectors.", Index, Sectors.Num());
		return;
	}

	PrintSector(Sectors[Index]->GetIdentifier());
}


void UFlareGameTools::GiveBirth(int32 SectorIndex, uint32 Population)
{
	if (!GetGameWorld())
	{
		FLOG("UFlareGameTools::GiveBirth failed: no loaded world");
		return;
	}

	TArray<UFlareSimulatedSector*> Sectors= GetGameWorld()->GetSectors();
	if (SectorIndex < 0 || SectorIndex > Sectors.Num() -1)
	{
		FLOGV("UFlareGameTools::GiveBirth failed: invalid index %d, with %d sectors.", SectorIndex, Sectors.Num());
		return;
	}

	FName SectorIdentifier = Sectors[SectorIndex]->GetIdentifier();

	UFlareSimulatedSector* Sector = GetGameWorld()->FindSector(SectorIdentifier);
	if (!Sector)
	{
		FLOGV("AFlareGame::GiveBirth failed: no sector with identifier '%s'", * SectorIdentifier.ToString());
		return;
	}

	Sector->GetPeople()->GiveBirth(Population);
}

void UFlareGameTools::Scrap(FName ShipImmatriculation, FName TargetStationImmatriculation)
{
	if (!GetGameWorld())
	{
		FLOG("UFlareGameTools::Scrap failed: no loaded world");
		return;
	}

	GetGame()->Scrap(ShipImmatriculation, TargetStationImmatriculation);
}



/*----------------------------------------------------
	Trade tools
----------------------------------------------------*/

void UFlareGameTools::PrintCargoBay(FName ShipImmatriculation)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintCargoBay failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::PrintCargoBay failed: a sector is active");
		return;
	}

	UFlareSimulatedSpacecraft* Ship = GetGameWorld()->FindSpacecraft(ShipImmatriculation);
	if (!Ship)
	{
		FLOGV("AFlareGame::PrintCargoBay failed: no Ship with immatriculation '%s'", *ShipImmatriculation.ToString());
		return;
	}
	UFlareCargoBay* CargoBay = Ship->GetActiveCargoBay();

	FLOGV("Cargo bay for '%s' : ", *ShipImmatriculation.ToString());
	for (int32 CargoIndex = 0; CargoIndex < CargoBay->GetSlotCount(); CargoIndex++)
	{
		FFlareCargo* Cargo = CargoBay->GetSlot(CargoIndex);
		FLOGV("  - %s : %u / %u ", (Cargo->Resource ? *Cargo->Resource->Name.ToString() : TEXT("[Empty]")), Cargo->Quantity, CargoBay->GetSlotCapacity());
	}
}

void UFlareGameTools::GiveResources(FName ShipImmatriculation, FName ResourceIdentifier, uint32 Quantity)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::GiveResources failed: no loaded world");
		return;
	}

	FFlareResourceDescription* Resource = GetGame()->GetResourceCatalog()->Get(ResourceIdentifier);
	if (!Resource)
	{
		FLOGV("AFlareGame::GiveResources failed: no resource with id '%s'", *ResourceIdentifier.ToString());
		return;
	}

	UFlareSimulatedSpacecraft* Ship = GetGameWorld()->FindSpacecraft(ShipImmatriculation);
	if (!Ship)
	{
		FLOGV("AFlareGame::GiveResources failed: no Ship with immatriculation '%s'", *ShipImmatriculation.ToString());
		return;
	}
	Ship->GetActiveCargoBay()->GiveResources(Resource, Quantity, Ship->GetCompany());
}

void UFlareGameTools::TakeResources(FName ShipImmatriculation, FName ResourceIdentifier, uint32 Quantity)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::TakeResources failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("AFlareGame::TakeResources failed: a sector is active");
		return;
	}

	FFlareResourceDescription* Resource = GetGame()->GetResourceCatalog()->Get(ResourceIdentifier);
	if (!Resource)
	{
		FLOGV("AFlareGame::TakeResources failed: no resource with id '%s'", *ResourceIdentifier.ToString());
		return;
	}

	UFlareSimulatedSpacecraft* Ship = GetGameWorld()->FindSpacecraft(ShipImmatriculation);
	if (!Ship)
	{
		FLOGV("AFlareGame::TakeResources failed: no Ship with immatriculation '%s'", *ShipImmatriculation.ToString());
		return;
	}
	Ship->GetActiveCargoBay()->TakeResources(Resource, Quantity, Ship->GetCompany());
}

void UFlareGameTools::TakeMoney(FName CompanyShortName, int64 Amount)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::TakeMoney failed: no loaded world");
		return;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("AFlareGame::TakeMoney failed: no company with short name '%s'", * CompanyShortName.ToString());
		return;
	}

	Company->TakeMoney(Amount, true, FFlareTransactionLogEntry::LogCheat());
}

void UFlareGameTools::GiveMoney(FName CompanyShortName, int64 Amount)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::GiveMoney failed: no loaded world");
		return;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("AFlareGame::GiveMoney failed: no company with short name '%s'", * CompanyShortName.ToString());
		return;
	}

	Company->GiveMoney(Amount, FFlareTransactionLogEntry::LogCheat());
}

void UFlareGameTools::GiveResearch(FName CompanyShortName, int64 Amount)
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::GiveResearch failed: no loaded world");
		return;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("AFlareGame::GiveResearch failed: no company with short name '%s'", *CompanyShortName.ToString());
		return;
	}

	Company->GiveResearch(Amount);
}

void UFlareGameTools::TransferResources(FName SourceImmatriculation, FName DestinationImmatriculation, FName ResourceIdentifier, uint32 Quantity)
{
	if (!GetGameWorld())
	{
		FLOG("UFlareGameTools::TransferResources failed: no loaded world");
		return;
	}

	if (GetActiveSector())
	{
		FLOG("UFlareGameTools::TransferResources failed: a sector is active");
		return;
	}

	FFlareResourceDescription* Resource = GetGame()->GetResourceCatalog()->Get(ResourceIdentifier);
	if (!Resource)
	{
		FLOGV("UFlareGameTools::TransferResources failed: no resource with id '%s'", *ResourceIdentifier.ToString());
		return;
	}

	UFlareSimulatedSpacecraft* SourceSpacecraft = GetGameWorld()->FindSpacecraft(SourceImmatriculation);
	if (!SourceSpacecraft)
	{
		FLOGV("UFlareGameTools::TransferResources failed: no source spacecraft with immatriculation '%s'", *SourceImmatriculation.ToString());
		return;
	}

	UFlareSimulatedSpacecraft* DestinationSpacecraft = GetGameWorld()->FindSpacecraft(DestinationImmatriculation);
	if (!DestinationSpacecraft)
	{
		FLOGV("UFlareGameTools::TransferResources failed: no destination spacecraft with immatriculation '%s'", *DestinationImmatriculation.ToString());
		return;
	}


	SectorHelper::Trade(SourceSpacecraft, DestinationSpacecraft, Resource, Quantity);
}

/*----------------------------------------------------
	Active Sector tools
----------------------------------------------------*/

UFlareSimulatedSpacecraft* UFlareGameTools::CreateStationForMe(FName StationClass, bool UnderConstruction)
{
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateStationForMe failed: no active sector");
		return NULL;
	}

	return CreateStationInCompany(StationClass, GetPC()->GetCompany()->GetShortName(), 100, UnderConstruction);
}

UFlareSimulatedSpacecraft* UFlareGameTools::CreateStationInCompany(FName StationClass, FName CompanyShortName, float Distance, bool UnderConstruction)
{
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateStationInCompany failed: no active sector");
		return NULL;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("UFlareSector::CreateStationInCompany failed : No company named '%s'", *CompanyShortName.ToString());
		return NULL;
	}

	AFlarePlayerController* PC = GetPC();
	AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();

	FFlareStationSpawnParameters StationParams;
	if (ExistingShipPawn)
	{
		StationParams.Location = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(Distance * FVector(100, 0, 0));
	}

	UFlareSimulatedSector* ActiveSector = GetGame()->DeactivateSector();
	UFlareSimulatedSpacecraft* NewStation = ActiveSector->CreateStation(StationClass, Company, UnderConstruction, StationParams);
	GetGame()->ActivateCurrentSector();
	
	FFlareMenuParameterData Data;
	Data.Spacecraft = ExistingShipPawn->GetParent();
	PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_FlyShip, Data);

	return NewStation;
}

UFlareSimulatedSpacecraft* UFlareGameTools::CreateShipForMe(FName ShipClass)
{
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateShipForMe failed: no active sector");
		return NULL;
	}

	AFlarePlayerController* PC = GetPC();
	return CreateShipInCompany(ShipClass, PC->GetCompany()->GetShortName(), 100);
}

UFlareSimulatedSpacecraft* UFlareGameTools::CreateShipInCompany(FName ShipClass, FName CompanyShortName, float Distance)
{
	FLOG("AFlareGame::CreateShipInCompany");
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateShipInCompany failed: no active sector");
		return NULL;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("UFlareSector::CreateShipInCompany failed : No company named '%s'", *CompanyShortName.ToString());
		return NULL;
	}

	AFlarePlayerController* PC = GetPC();
	AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
	FVector TargetPosition = FVector::ZeroVector;
	if (ExistingShipPawn)
	{
		TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(Distance * FVector(100, 0, 0));
	}

	UFlareSimulatedSector* ActiveSector = GetGame()->DeactivateSector();
	UFlareSimulatedSpacecraft* NewShip = ActiveSector->CreateSpacecraft(ShipClass, Company, TargetPosition);
	GetGame()->ActivateCurrentSector();

	FFlareMenuParameterData Data;
	Data.Spacecraft = PC->GetPlayerShip();
	PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_FlyShip, Data);

	return NewShip;
}

void UFlareGameTools::CreateShipsInCompany(FName ShipClass, FName CompanyShortName, float Distance, int32 Count)
{
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateShipsInCompany failed: no active sector");
		return;
	}

	UFlareCompany* Company = GetGameWorld()->FindCompanyByShortName(CompanyShortName);
	if (!Company)
	{
		FLOGV("UFlareSector::CreateShipInCompany failed : No company named '%s'", *CompanyShortName.ToString());
		return;
	}

	AFlarePlayerController* PC = GetPC();

	AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
	FVector TargetPosition = FVector::ZeroVector;
	if (ExistingShipPawn)
	{
		TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(Distance * FVector(100, 0, 0));
	}

	UFlareSimulatedSector* ActiveSector = GetGame()->DeactivateSector();

	for (int32 ShipIndex = 0; ShipIndex < Count; ShipIndex++)
	{
		ActiveSector->CreateSpacecraft(ShipClass, Company, TargetPosition);
	}

	GetGame()->ActivateCurrentSector();
}

void UFlareGameTools::CreateQuickBattle(float Distance, FName Company1Name, FName Company2Name, FName ShipClass1, int32 ShipClass1Count, FName ShipClass2, int32 ShipClass2Count)
{
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateQuickBattle failed: no active sector");
		return;
	}

	UFlareCompany* Company1 = GetGameWorld()->FindCompanyByShortName(Company1Name);
	if (!Company1)
	{
		FLOGV("UFlareSector::CreateShipInCompany failed : No company named '%s'", *Company1Name.ToString());
		return;
	}

	UFlareCompany* Company2 = GetGameWorld()->FindCompanyByShortName(Company2Name);
	if (!Company2)
	{
		FLOGV("UFlareSector::CreateShipInCompany failed : No company named '%s'", *Company2Name.ToString());
		return;
	}

	DeclareWar(Company1Name, Company2Name);

	AFlarePlayerController* PC = GetPC();

	AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
	FVector TargetPosition1 = FVector::ZeroVector;
	FVector TargetPosition2 = FVector::ZeroVector;
	if (ExistingShipPawn)
	{
		TargetPosition1 = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(Distance / 2.f * FVector(100, 0, 0));
		TargetPosition2 = ExistingShipPawn->GetActorLocation() - ExistingShipPawn->GetActorRotation().RotateVector(Distance / 2.f * FVector(100, 0, 0));
	}

	UFlareSimulatedSector* ActiveSector = GetGame()->DeactivateSector();

	for (int32 ShipIndex = 0; ShipIndex < ShipClass1Count; ShipIndex++)
	{
		ActiveSector->CreateSpacecraft(ShipClass1, Company1, TargetPosition1);
		ActiveSector->CreateSpacecraft(ShipClass1, Company2, TargetPosition2);
	}

	for (int32 ShipIndex = 0; ShipIndex < ShipClass2Count; ShipIndex++)
	{
		ActiveSector->CreateSpacecraft(ShipClass2, Company1, TargetPosition1);
		ActiveSector->CreateSpacecraft(ShipClass2, Company2, TargetPosition2);
	}

	GetGame()->ActivateCurrentSector();
}


void UFlareGameTools::CreateAsteroid(int32 ID, FName Name)
{
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateAsteroid failed: no active sector");
		return;
	}


	AFlarePlayerController* PC = GetPC();

	AFlareSpacecraft* ExistingShipPawn = PC->GetShipPawn();
	FVector TargetPosition = FVector::ZeroVector;
	if (ExistingShipPawn)
	{
		TargetPosition = ExistingShipPawn->GetActorLocation() + ExistingShipPawn->GetActorRotation().RotateVector(200 * FVector(100, 0, 0));
	}

	UFlareSimulatedSector* ActiveSector = GetGame()->DeactivateSector();

	ActiveSector->CreateAsteroid(ID, Name, TargetPosition);

	GetGame()->ActivateCurrentSector();
}

void UFlareGameTools::CreateMeteoriteGroup(FName SectorIdentifier, float PowerRatio)
{
	if (!GetActiveSector())
	{
		FLOG("AFlareGame::CreateMeteoriteGroup failed: no active sector");
		return;
	}

	UFlareSimulatedSector* Sector = GetGameWorld()->FindSector(SectorIdentifier);

	if (!Sector)
	{
		FLOGV("AFlareGame::CreateMeteoriteGroup failed: no sector '%s'", *SectorIdentifier.ToString());
		return ;
	}


	if(Sector->GetSectorStations().Num() == 0)
	{
		FLOG("AFlareGame::CreateMeteoriteGroup failed: no station");
		return;
	}

	UFlareSimulatedSpacecraft* TargetStation = Sector->GetSectorStations()[FMath::RandRange(0, Sector->GetSectorStations().Num()-1)];

	if(!TargetStation)
	{
		FLOG("AFlareGame::CreateMeteoriteGroup failed: invalide target station");
		return;
	}

	UFlareSimulatedSector* ActiveSector = GetGame()->DeactivateSector();

	ActiveSector->GenerateMeteoriteGroup(TargetStation, PowerRatio);

	GetGame()->ActivateCurrentSector();
}

void UFlareGameTools::PrintCompanyList()
{
	if (!GetGameWorld())
	{
		FLOG("AFlareGame::PrintCompanyList failed: no loaded world");
		return;
	}


	FLOGV("> PrintCompanyList: %d companies", GetGameWorld()->GetCompanies().Num());

	for (int i = 0; i < GetGameWorld()->GetCompanies().Num(); i++)
	{
		UFlareCompany* Company = GetGameWorld()->GetCompanies()[i];
		FLOGV("%2d - %s: %s (%s)", i, *Company->GetIdentifier().ToString(), *Company->GetCompanyName().ToString(), *Company->GetShortName().ToString());
	}
}

/*----------------------------------------------------
	Helper
----------------------------------------------------*/

FVector UFlareGameTools::ColorToVector(FLinearColor Color)
{
	return FVector(Color.R,Color.G,Color.B);
}

FText UFlareGameTools::FormatTime(int64 Time, int Deep)
{
	if (Time < SECONDS_IN_MINUTE)
	{
		return FText::Format(LOCTEXT("FormatSecondConcat", "{0} {1}"),
							 FText::AsNumber(Time),
							 Time > 1 ? LOCTEXT("FormatSeconds", "seconds") : LOCTEXT("FormatSecond", "second"));
	}
	else if (Time < SECONDS_IN_HOUR)
	{
		int64 Minutes = Time / SECONDS_IN_MINUTE;
		int64 RemainingSeconds = Time % SECONDS_IN_MINUTE;

		FText MinutesString;

		if (Minutes > 0)
		{
			MinutesString = FText::Format(LOCTEXT("FormatMinuteConcat", "{0} {1}"),
										 FText::AsNumber(Minutes),
										 Minutes > 1 ? LOCTEXT("FormatMinutes", "minutes") : LOCTEXT("FormatMinute", "minute"));
		}

		if (Deep > 0 && RemainingSeconds > 0)
		{
			MinutesString = UFlareGameTools::AddLeadingSpace(FText::Format(LOCTEXT("FormatMinuteConcat2", "{0} {1}"), MinutesString, FormatTime(RemainingSeconds, Deep - 1)));
		}

		return MinutesString;
	}
	else if (Time < SECONDS_IN_DAY)
	{
		int64 Hours = Time / SECONDS_IN_HOUR;
		int64 RemainingSeconds = Time % SECONDS_IN_HOUR;
		int64 RemainingMinutes = RemainingSeconds / SECONDS_IN_MINUTE;

		FText HoursString;

		if (Hours > 0)
		{
			HoursString = FText::Format(LOCTEXT("FormatHourConcat", "{0} {1}"),
										 FText::AsNumber(Hours),
										 Hours > 1 ? LOCTEXT("FormatHours", "hours") : LOCTEXT("FormatHour", "hour"));
		}

		if (Deep > 0 && RemainingMinutes > 0)
		{
			HoursString = UFlareGameTools::AddLeadingSpace(FText::Format(LOCTEXT("FormatHourConcat2", "{0} {1}"), HoursString, FormatTime(RemainingSeconds, Deep - 1)));
		}

		return HoursString;
	}
	else if (Time < SECONDS_IN_YEAR)
	{
		int64 Days = Time / SECONDS_IN_DAY;
		int64 RemainingSeconds = Time % SECONDS_IN_DAY;
		int64 RemainingHours = RemainingSeconds / SECONDS_IN_HOUR;

		FText DaysString;

		if (Days > 0)
		{
			DaysString = FText::Format(LOCTEXT("FormatDayConcat", "{0} {1}"),
										 FText::AsNumber(Days),
										 Days > 1 ? LOCTEXT("FormatDays", "days") : LOCTEXT("FormatDay", "day"));
		}

		if (Deep > 0 && RemainingHours > 0)
		{
			DaysString = UFlareGameTools::AddLeadingSpace(FText::Format(LOCTEXT("FormatDayConcat2", "{0} {1}"), DaysString, FormatTime(RemainingSeconds, Deep - 1)));
		}

		return DaysString;
	}
	else
	{
		int64 Years = Time / SECONDS_IN_YEAR;
		int64 RemainingSeconds = Time % SECONDS_IN_YEAR;
		int64 RemainingDays = RemainingSeconds / SECONDS_IN_DAY;

		FText YearsString;

		if (Years > 0)
		{
			YearsString = FText::Format(LOCTEXT("FormatYearConcat", "{0} {1}"),
										 FText::AsNumber(Years),
										 Years > 1 ? LOCTEXT("FormatYears", "years") : LOCTEXT("FormatYear", "year"));
		}

		if (Deep > 0 && RemainingDays > 0)
		{
			YearsString = UFlareGameTools::AddLeadingSpace(FText::Format(LOCTEXT("FormatYearConcat2", "{0} {1}"), YearsString, FormatTime(RemainingSeconds, Deep - 1)));
		}

		return YearsString;
	}
}

FText UFlareGameTools::FormatDate(int64 Days, int Deep)
{
	if (Days < DAYS_IN_YEAR)
	{
		if (Days < 1)
		{
			Days = 1;
		}
		return FText::Format(LOCTEXT("FormatDateDaysConcat", "{0} {1}"),
												 FText::AsNumber(Days),
												 Days > 1 ? LOCTEXT("FormatDateDays", "days") : LOCTEXT("FormatDateDay", "day"));
	}
	else
	{
		int64 Years = Days / DAYS_IN_YEAR;
		int64 RemainingDays = Days % DAYS_IN_YEAR;

		FText YearsString;

		if (Years > 0)
		{
			YearsString = FText::Format(LOCTEXT("FormatDateYearConcat", "{0} {1}"),
										 FText::AsNumber(Years),
										 Years > 1 ? LOCTEXT("FormatDateYears", "years") : LOCTEXT("FormatDateYear", "year"));
		}

		if (Deep > 0 && RemainingDays > 0)
		{
			YearsString = UFlareGameTools::AddLeadingSpace(FText::Format(LOCTEXT("FormatDateYearConcat2", "{0} {1}"), YearsString, FormatDate(RemainingDays, Deep - 1)));
		}

		return YearsString;
	}
}

FText UFlareGameTools::AddLeadingSpace(FText Text, int Count)
{
	FString SpaceString = "";

	for (int Index = 0; Index < Count; Index++)
	{
		SpaceString += " ";
	}

	return FText::Format(LOCTEXT("LeadingSpaceFormat", "{0}{1}"), FText::FromString(SpaceString), Text);
}

FText UFlareGameTools::GetDisplayDate(int64 Days)
{
	int64 Years = GetYearFromDate(Days);
	int64 RemainingDays = Days % DAYS_IN_YEAR;

	return FText::Format(LOCTEXT("DateFormat", "year {0}, day {1}"),
		FText::AsNumber(Years + 1),
		FText::AsNumber(RemainingDays + 1));
}

int64 UFlareGameTools::GetYearFromDate(int64 Date)
{
	return START_YEAR + (Date / DAYS_IN_YEAR);
}

int64 UFlareGameTools::ComputeSpacecraftPrice(FName ShipClass, UFlareSimulatedSector* Sector, bool WithMargin, bool ConstructionPrice, bool LocalPrice, UFlareCompany* Company)
{
	FFlareSpacecraftDescription* Desc = Sector->GetGame()->GetSpacecraftCatalog()->Get(ShipClass);

	if (!Desc)
	{
		FLOGV("ComputeSpacecraftPrice failed: Unkwnon ship %s", *ShipClass.ToString());
		return 0;
	}

	// Base cost
	int64 Cost = 0;

	// For stations, use the sectore penalty
	if (Desc->IsStation())
	{
		if(ConstructionPrice && Company)
		{
			Cost += Sector->GetStationConstructionFee(Cost, Company);
		}
	}
	else
	{
		Cost += Desc->CycleCost.ProductionCost *1.10f;
	}

	// Add input resource cost
	for (int ResourceIndex = 0; ResourceIndex < Desc->CycleCost.InputResources.Num() ; ResourceIndex++)
	{
		FFlareFactoryResource* Resource = &Desc->CycleCost.InputResources[ResourceIndex];
		int64 ResourcePrice;
		if(LocalPrice)
		{
			ResourcePrice = Sector->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::Default);
		}
		else
		{
			ResourcePrice = Resource->Resource->Data.MinPrice;
		}

		Cost += Resource->Quantity * (ResourcePrice *1.10f);
	}

	// Substract output resource
	for (int ResourceIndex = 0; ResourceIndex < Desc->CycleCost.OutputResources.Num() ; ResourceIndex++)
	{
		FFlareFactoryResource* Resource = &Desc->CycleCost.OutputResources[ResourceIndex];
		int64 ResourcePrice;
		if(LocalPrice)
		{
			ResourcePrice = Sector->GetResourcePrice(&Resource->Resource->Data, EFlareResourcePriceContext::Default);
		}
		else
		{
			ResourcePrice = Resource->Resource->Data.MinPrice;
		}

		Cost -= Resource->Quantity * ResourcePrice;
	}

	// Upgrade value

	return FMath::Max((int64) 0, Cost) * (WithMargin ? 1.2f : 1.0f);
}

FText UFlareGameTools::DisplaySpacecraftName(UFlareSimulatedSpacecraft* Spacecraft, bool ForceHidePrefix, bool ToUpper)
{
	FString Text = Spacecraft->GetNickName().ToString();
	FString ImmatText = Spacecraft->GetImmatriculation().ToString().ToUpper();

	// Process name
	Text = Text.Replace(*FString("-"), *FString(" "));
	Text = Text.Replace(*FString("_"), *FString(" "));
	if (ToUpper)
	{
		Text = Text.ToUpper(); // FString needed here
	}

	// Add immatriculation
	if (!ForceHidePrefix)
	{
		ImmatText = ImmatText.Left(6);

		Text = ImmatText + " " + Text;
	}

	return FText::FromString(Text);
}


/*----------------------------------------------------
	Getter
----------------------------------------------------*/

AFlarePlayerController* UFlareGameTools::GetPC() const
{
	return Cast<AFlarePlayerController>(GetOuter());
}

AFlareGame* UFlareGameTools::GetGame() const
{
	return GetPC()->GetGame();
}


UFlareWorld* UFlareGameTools::GetGameWorld() const
{
	return GetGame()->GetGameWorld();
}

UFlareSector* UFlareGameTools::GetActiveSector() const
{
	return GetGame()->GetActiveSector();
}

#undef LOCTEXT_NAMESPACE
