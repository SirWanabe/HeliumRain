
#include "FlareSectorHelper.h"
#include "../Flare.h"

#include "../Data/FlareResourceCatalog.h"
#include "../Data/FlareSpacecraftComponentsCatalog.h"

#include "../Economy/FlareFactory.h"
#include "../Economy/FlareCargoBay.h"

#include "FlareCompany.h"
#include "FlareGame.h"
#include "FlareWorld.h"
#include "FlareScenarioTools.h"

#include "../Player/FlarePlayerController.h"


DECLARE_CYCLE_STAT(TEXT("FlareSectorHelper RepairFleets"), STAT_FlareSectorHelper_RepairFleets, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareSectorHelper RefillFleets"), STAT_FlareSectorHelper_RefillFleets, STATGROUP_Flare);


UFlareSimulatedSpacecraft*  SectorHelper::FindTradeStation(FlareTradeRequest Request)
{
	//FLOGV("FindTradeStation of %s for %s  (%d)", *Request.Resource->Acronym.ToString(), *Request.Client->GetImmatriculation().ToString(), (Request.Operation + 0));

	if(!Request.Client || !Request.Client->GetCurrentSector())
	{
		FLOG("Invalid find trade query");
		return NULL;
	}

	UFlareCompany* ClientCompany = Request.AllowUseNoTradeForMe ? Request.Client->GetCompany() : nullptr;
	UFlareSimulatedSector* Sector = Request.Client->GetCurrentSector();
	TArray<UFlareSimulatedSpacecraft*>& SectorStations = Sector->GetSectorStations();

	float UnloadQuantityScoreMultiplier = 0;
	float LoadQuantityScoreMultiplier = 0;
	float SellQuantityScoreMultiplier = 0;
	float BuyQuantityScoreMultiplier = 0;
	float FullRatioBonus = 0;
	float EmptyRatioBonus = 0;
	bool  ToOrFrom = false;
	bool  NeedInput = false;
	bool  NeedOutput = false;
	bool  Donation = Request.IsDonation;

	switch (Request.Operation)
	{
		case EFlareTradeRouteOperation::Load:
			LoadQuantityScoreMultiplier = Request.LoadUnloadPriority;
			BuyQuantityScoreMultiplier = Request.BuySellPriority;
			FullRatioBonus = 0.1;
			NeedOutput = true;
			break;
		case EFlareTradeRouteOperation::Unload:
			UnloadQuantityScoreMultiplier = Request.LoadUnloadPriority;
			SellQuantityScoreMultiplier = Request.BuySellPriority;
			EmptyRatioBonus = 0.1;
			NeedInput = true;
			break;
	}

	float BestScore = 0;
	UFlareSimulatedSpacecraft* BestStation = NULL;
	uint32 AvailableQuantity = Request.Client->GetActiveCargoBay()->GetResourceQuantity(Request.Resource, ClientCompany);
	uint32 FreeSpace = Request.Client->GetActiveCargoBay()->GetFreeSpaceForResource(Request.Resource, ClientCompany);
	FText Unused;

	for (int32 StationIndex = 0; StationIndex < SectorStations.Num(); StationIndex++)
	{
		UFlareSimulatedSpacecraft* Station = SectorStations[StationIndex];
		//FLOGV("   Check trade for %s", *Station->GetImmatriculation().ToString());
		// 
		if (NeedOutput)
		{
			if (!Station->CanTradeWith(Request.Client, Unused, Request.Resource))
			{
				continue;
			}
		}

		else if (NeedInput)
		{
			if (!Request.Client->CanTradeWith(Station, Unused, Request.Resource))
			{
				continue;
			}
		}

		if(!Request.AllowStorage && Station->HasCapability(EFlareSpacecraftCapability::Storage))
		{
			continue;
		}

		FFlareResourceUsage StationResourceUsage = Station->GetResourceUseType(Request.Resource);
		if(NeedOutput && (!StationResourceUsage.HasUsage(EFlareResourcePriceContext::FactoryOutput) &&
						  !StationResourceUsage.HasUsage(EFlareResourcePriceContext::ConsumerConsumption) &&
						  !StationResourceUsage.HasUsage(EFlareResourcePriceContext::HubOutput)))
		{
			//FLOG(" need output but dont provide it");
			continue;
		}

		if(NeedInput && (!StationResourceUsage.HasUsage(EFlareResourcePriceContext::FactoryInput) &&
						 !StationResourceUsage.HasUsage(EFlareResourcePriceContext::ConsumerConsumption) &&
						 !StationResourceUsage.HasUsage(EFlareResourcePriceContext::MaintenanceConsumption) &&
						 !StationResourceUsage.HasUsage(EFlareResourcePriceContext::HubInput)))
		{
			//FLOG(" need input but dont provide it");
			continue;
		}

		int32 StationFreeSpace = Station->GetActiveCargoBay()->GetFreeSpaceForResource(Request.Resource, ClientCompany);
		int32 StationResourceQuantity = Station->GetActiveCargoBay()->GetResourceQuantity(Request.Resource, ClientCompany);

		if (!Station->IsUnderConstruction() && Station->IsComplex() && !Request.AllowFullStock)
		{
			if(Station->GetActiveCargoBay()->WantBuy(Request.Resource, ClientCompany) && Station->GetActiveCargoBay()->WantSell(Request.Resource, ClientCompany))
			{
				int32 TotalCapacity = Station->GetActiveCargoBay()->GetTotalCapacityForResource(Request.Resource, ClientCompany);
				StationFreeSpace = FMath::Max(0, StationFreeSpace - TotalCapacity / 2);
				StationResourceQuantity = FMath::Max(0, StationResourceQuantity - TotalCapacity / 2);
			}
		}

		if (StationFreeSpace == 0 && StationResourceQuantity == 0)
		{
			//FLOG(" need quantity or resource");
			continue;
		}

		float Score = 0;
		float FullRatio =  (float) StationResourceQuantity / (float) (StationResourceQuantity + StationFreeSpace);
		float EmptyRatio = 1 - FullRatio;
		uint32 UnloadMaxQuantity  = 0;
		uint32 LoadMaxQuantity  = 0;

		if(!Station->IsUnderConstruction())
		{
			// Check cargo limit
			if(NeedOutput && Request.CargoLimit != -1 && FullRatio < Request.CargoLimit / Station->GetLevel())
			{
				continue;
			}

			if(NeedInput && Request.CargoLimit != -1 && FullRatio > (1.f - (1.f - Request.CargoLimit) / Station->GetLevel()))
			{
				continue;
			}
		}
		else if (Station->HasCapability(EFlareSpacecraftCapability::Storage))
		{
			continue;
		}

		if(Station->GetActiveCargoBay()->WantBuy(Request.Resource, ClientCompany))
		{
			UnloadMaxQuantity = StationFreeSpace;
			UnloadMaxQuantity  = FMath::Min(UnloadMaxQuantity , AvailableQuantity);
		}

		if(Station->GetActiveCargoBay()->WantSell(Request.Resource, ClientCompany))
		{
			LoadMaxQuantity = StationResourceQuantity;
			LoadMaxQuantity = FMath::Min(LoadMaxQuantity , FreeSpace);
		}

		if(Station->GetCompany() == Request.Client->GetCompany())
		{
			Score += UnloadMaxQuantity * UnloadQuantityScoreMultiplier;
			Score += LoadMaxQuantity * LoadQuantityScoreMultiplier;
		}
		else
		{
			FFlareResourceUsage ResourceUsage = Station->GetResourceUseType(Request.Resource);

			int32 ResourcePrice = 0;
			if(NeedInput)
			{
				ResourcePrice = Sector->GetTransfertResourcePrice(NULL, Station, Request.Resource);
			}
			else
			{
				ResourcePrice = Sector->GetTransfertResourcePrice(Station, NULL, Request.Resource);
			}

			if (!Donation)
			{
				uint32 MaxBuyableQuantity = Request.Client->GetCompany()->GetMoney() / SectorHelper::GetBuyResourcePrice(Sector, Request.Resource, ResourceUsage);
				LoadMaxQuantity = FMath::Min(LoadMaxQuantity, MaxBuyableQuantity);

				uint32 MaxSellableQuantity = Station->GetCompany()->GetMoney() / SectorHelper::GetSellResourcePrice(Sector, Request.Resource, ResourceUsage);
				UnloadMaxQuantity = FMath::Min(UnloadMaxQuantity, MaxSellableQuantity);
			}

			Score += UnloadMaxQuantity * SellQuantityScoreMultiplier;
			Score += LoadMaxQuantity * BuyQuantityScoreMultiplier;
		}

		Score *= 1 + (FullRatio * FullRatioBonus) + (EmptyRatio * EmptyRatioBonus);

		if(Station->IsUnderConstruction())
		{
			Score *= 10000;
			/*FLOGV("Station %s is under construction. Score %f, BestScore %f",
				  *Station->GetImmatriculation().ToString(),
				  Score,
				  BestScore)*/
		}
		else if(Station->IsShipyard())
			Score *= 2;
		else if(Station->HasCapability(EFlareSpacecraftCapability::Storage))
		{
			Score *= 0.01;
		}

		if(Score > 0 && Score > BestScore)
		{
			BestScore = Score;
			BestStation = Station;
		}
	}

	//FLOGV("FindTradeStation result %p", BestStation);

	return BestStation;
}

int64 SectorHelper::GetSellResourcePrice(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource, FFlareResourceUsage Usage)
{
	if(Usage.HasUsage(EFlareResourcePriceContext::FactoryInput) ||
	   Usage.HasUsage(EFlareResourcePriceContext::ConsumerConsumption) ||
	   Usage.HasUsage(EFlareResourcePriceContext::MaintenanceConsumption))
	{
		 if(!Usage.HasUsage(EFlareResourcePriceContext::FactoryOutput))
		 {
			return Sector->GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput);
		 }
	}

	if(Usage.HasUsage(EFlareResourcePriceContext::HubInput))
	{
		return Sector->GetResourcePrice(Resource, EFlareResourcePriceContext::HubInput);
	}

	return Sector->GetResourcePrice(Resource, EFlareResourcePriceContext::Default);
}

int64 SectorHelper::GetBuyResourcePrice(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource, FFlareResourceUsage Usage)
{

	if(!Usage.HasAnyUsage())
	{
		return Sector->GetResourcePrice(Resource, EFlareResourcePriceContext::Default);
	}

	int64 MaxPrice = 0;


	if(Usage.HasUsage(EFlareResourcePriceContext::ConsumerConsumption))
	{
		MaxPrice = FMath::Max(MaxPrice, Sector->GetResourcePrice(Resource, EFlareResourcePriceContext::ConsumerConsumption));
	}

	if(Usage.HasUsage(EFlareResourcePriceContext::MaintenanceConsumption))
	{
		MaxPrice = FMath::Max(MaxPrice, Sector->GetResourcePrice(Resource, EFlareResourcePriceContext::MaintenanceConsumption));
	}

	if(Usage.HasUsage(EFlareResourcePriceContext::FactoryInput))
	{
		MaxPrice = FMath::Max(MaxPrice, Sector->GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryInput));
	}

	if(Usage.HasUsage(EFlareResourcePriceContext::FactoryOutput))
	{
		MaxPrice = FMath::Max(MaxPrice, Sector->GetResourcePrice(Resource, EFlareResourcePriceContext::FactoryOutput));
	}

	if(Usage.HasUsage(EFlareResourcePriceContext::HubOutput))
	{
		return Sector->GetResourcePrice(Resource, EFlareResourcePriceContext::HubOutput);
	}

	return MaxPrice;
}

int32 SectorHelper::Trade(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, int32 MaxQuantity, int64* TransactionPrice, UFlareTradeRoute* TradeRoute, bool IsDonation, int32 TradeReason)
{
	FText Unused;

	//FLOGV("Trade for %s", *Resource->Acronym.ToString());

	if(TransactionPrice )
	{
		*TransactionPrice = 0;
	}

	if(!SourceSpacecraft->CanTradeWith(DestinationSpacecraft, Unused,Resource))
	{
		FLOGV("Both spacecraft cannot trade: %s -> %s", *SourceSpacecraft->GetImmatriculation().ToString(), *DestinationSpacecraft->GetImmatriculation().ToString());
		return 0;
	}

	int32 ResourcePrice = SourceSpacecraft->GetCurrentSector()->GetTransfertResourcePrice(SourceSpacecraft, DestinationSpacecraft, Resource);
	int32 QuantityToTake = MaxQuantity;
	bool AllowDepts = SourceSpacecraft->GetGame()->GetQuestManager()->IsTradeQuestUseStation(DestinationSpacecraft, false) && SourceSpacecraft->GetCompany()->IsPlayerCompany();

	if (SourceSpacecraft->GetCompany() != DestinationSpacecraft->GetCompany() && !AllowDepts && !IsDonation)
	{
		// Limit transaction bay available money
		int32 MaxAffordableQuantity = FMath::Max(0, int32(DestinationSpacecraft->GetCompany()->GetMoney() / ResourcePrice));
		QuantityToTake = FMath::Min(QuantityToTake, MaxAffordableQuantity);
	}

	int32 ResourceCapacity;
	int32 AvailableQuantity;

	if (SourceSpacecraft->GetCompany() == DestinationSpacecraft->GetCompany())
	{
		ResourceCapacity = DestinationSpacecraft->GetActiveCargoBay()->GetFreeSpaceForResource(Resource, SourceSpacecraft->GetCompany(),false,0,true);
		AvailableQuantity = SourceSpacecraft->GetActiveCargoBay()->GetResourceQuantity(Resource, DestinationSpacecraft->GetCompany(),0,true);
	}
	else
	{
		ResourceCapacity = DestinationSpacecraft->GetActiveCargoBay()->GetFreeSpaceForResource(Resource, SourceSpacecraft->GetCompany());
		AvailableQuantity = SourceSpacecraft->GetActiveCargoBay()->GetResourceQuantity(Resource, DestinationSpacecraft->GetCompany());
	}

	QuantityToTake = FMath::Min(QuantityToTake, ResourceCapacity);
	QuantityToTake = FMath::Min(QuantityToTake, AvailableQuantity);
	
	int32 TakenResources;
	int32 GivenResources;

	if (SourceSpacecraft->GetCompany() == DestinationSpacecraft->GetCompany())
	{
		TakenResources = SourceSpacecraft->GetActiveCargoBay()->TakeResources(Resource, QuantityToTake, DestinationSpacecraft->GetCompany(),0,true);
		GivenResources = DestinationSpacecraft->GetActiveCargoBay()->GiveResources(Resource, TakenResources, SourceSpacecraft->GetCompany(),0,true);
	}
	else
	{
		TakenResources = SourceSpacecraft->GetActiveCargoBay()->TakeResources(Resource, QuantityToTake, DestinationSpacecraft->GetCompany());
		GivenResources = DestinationSpacecraft->GetActiveCargoBay()->GiveResources(Resource, TakenResources, SourceSpacecraft->GetCompany());
	}

	// Pay
	if (GivenResources > 0 && SourceSpacecraft->GetCompany() != DestinationSpacecraft->GetCompany())
	{
		int64 Price = ResourcePrice * GivenResources;

		if (!IsDonation)

		{
			DestinationSpacecraft->GetCompany()->TakeMoney(Price, AllowDepts, FFlareTransactionLogEntry::LogBuyResource(SourceSpacecraft, DestinationSpacecraft, Resource, GivenResources, TradeRoute));
			SourceSpacecraft->GetCompany()->GiveMoney(Price, FFlareTransactionLogEntry::LogSellResource(SourceSpacecraft, DestinationSpacecraft, Resource, GivenResources, TradeRoute));
			if (TransactionPrice)
			{
				*TransactionPrice = Price;
			}
		}

		UFlareCompany* PlayerCompany = SourceSpacecraft->GetGame()->GetPC()->GetCompany();

		int32 GameDifficulty = GameDifficulty = SourceSpacecraft->GetGame()->GetPC()->GetPlayerData()->DifficultyId;
		float ReputationGain = 0.01f;
		switch (GameDifficulty)
		{
		case -1: // Easy
			ReputationGain = 0.011f;
			break;
		case 0: // Normal
			ReputationGain = 0.01f;
			break;
		case 1: // Hard
			ReputationGain = 0.0075f;
			break;
		case 2: // Very Hard
			ReputationGain = 0.0050f;
			break;
		case 3: // Expert
			ReputationGain = 0.0125f;
			break;
		case 4: // Unfair
			ReputationGain = 0.003125;
			break;
		}

		if (IsDonation)
		{
			ReputationGain = ReputationGain * 1.50f;
		}

		if(SourceSpacecraft->GetCompany() == PlayerCompany && DestinationSpacecraft->GetCompany() != PlayerCompany)
		{
			DestinationSpacecraft->GetCompany()->GivePlayerReputation(GivenResources * ReputationGain, 100);
		}

		if(DestinationSpacecraft->GetCompany() == PlayerCompany && SourceSpacecraft->GetCompany() != PlayerCompany)
		{
			SourceSpacecraft->GetCompany()->GivePlayerReputation(GivenResources * ReputationGain, 100);
		}
	}
	
	// Set the trading state if not player fleet
	if (GivenResources > 0)
	{
		AFlarePlayerController* PC = SourceSpacecraft->GetGame()->GetPC();
		UFlareFleet* PlayerFleet = PC->GetPlayerFleet();
		FCHECK(PC);

		bool SetTrading = true;
		if (SourceSpacecraft->GetGame()->GetActiveSector() && SourceSpacecraft->GetGame()->GetActiveSector()->GetSimulatedSector() == SourceSpacecraft->GetCurrentSector())
		{
			SetTrading = false;
		}

		if (SetTrading)
		{
			if (SourceSpacecraft->GetCurrentFleet() != PlayerFleet && !SourceSpacecraft->IsStation())
			{
				SourceSpacecraft->SetTrading(true, TradeReason);
			}

			if (DestinationSpacecraft->GetCurrentFleet() != PlayerFleet && !DestinationSpacecraft->IsStation())
			{
				DestinationSpacecraft->SetTrading(true, TradeReason);
			}
		}

		if(PC->GetPlayerShip() == SourceSpacecraft || PC->GetPlayerShip() == DestinationSpacecraft)
		{
			PC->SetAchievementProgression("ACHIEVEMENT_TRADE", 1);
		}
	}

	SourceSpacecraft->GetGame()->GetQuestManager()->OnTradeDone(SourceSpacecraft, DestinationSpacecraft, Resource, GivenResources);

	//FLOGV("Trade GivenResources %d", GivenResources);
	return GivenResources;
}

void SectorHelper::GetAvailableFleetSupplyCount(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32& OwnedFS, int32& AvailableFS, int32& AffordableFS, UFlareSimulatedSpacecraft* ForShip, UFlareFleet* ForFleet)
{
	OwnedFS = 0;
	int32 NotOwnedFS = 0;

	FFlareResourceDescription* FleetSupply = Sector->GetGame()->GetScenarioTools()->FleetSupply;

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorSpacecrafts()[SpacecraftIndex];
		if (Spacecraft->IsHostile(Company))
		{
			// At war, no trade possible
			continue;
		}

		if (ForShip)
		{
			FText Unused;
			if (!ForShip->CanTradeWhiteListFrom(Spacecraft, Unused, FleetSupply) || !Spacecraft->CanTradeWhiteListTo(ForShip, Unused, FleetSupply))
			{
				//Ship whitelist restriction on trades
				continue;
			}
		}
		else if(ForFleet)
		{
			FText Unused;
			if (!ForFleet->CanTradeWhiteListFrom(Spacecraft, FleetSupply) || !Spacecraft->CanTradeWhiteListTo(ForFleet, FleetSupply))
			{
				//Fleet whitelist restriction on trades
				continue;
			}
		}
		else
		{
			if (!Company->CanTradeWhiteListFrom(Spacecraft->GetCompany(), FleetSupply) || !Spacecraft->GetCompany()->CanTradeWhiteListTo(Company, FleetSupply))
			{
				//Company level whitelist restriction on trades
				continue;
			}
		}

		int32 AvailableQuantity = Spacecraft->GetActiveCargoBay()->GetResourceQuantity(FleetSupply, Company);

		if (Company == Spacecraft->GetCompany())
		{
			OwnedFS += AvailableQuantity;
		}
		else
		{
			NotOwnedFS += AvailableQuantity;
		}
	}

	AvailableFS = OwnedFS + NotOwnedFS;
	int32 ResourcePrice = Sector->GetResourcePrice(FleetSupply, EFlareResourcePriceContext::MaintenanceConsumption);
	int32 MaxAffordableQuantity = FMath::Max(0, int32(Company->GetMoney() / ResourcePrice));
	AffordableFS = OwnedFS + FMath::Min(MaxAffordableQuantity, NotOwnedFS);
}

float SectorHelper::GetComponentMaxRepairRatio(FFlareSpacecraftComponentDescription* ComponentDescription)
{
	switch(ComponentDescription->Type)
	{
	case EFlarePartType::RCS:
		return MAX_RCS_REPAIR_RATIO_BY_DAY;
		break;
	case EFlarePartType::OrbitalEngine:
		return MAX_ENGINE_REPAIR_RATIO_BY_DAY;
		break;
	case EFlarePartType::Weapon:
		return MAX_WEAPON_REPAIR_RATIO_BY_DAY;
		break;
	default:
		return MAX_REPAIR_RATIO_BY_DAY;
	}
}

bool SectorHelper::HasShipRefilling(UFlareSimulatedSector* TargetSector, UFlareCompany* Company)
{
	for (UFlareSimulatedSpacecraft* Spacecraft : TargetSector->GetSectorSpacecrafts())
	{
		if (Company != Spacecraft->GetCompany()) {
			continue;
		}

		if (Spacecraft->GetRefillStock() > 0 && Spacecraft->NeedRefill())
		{
			return true;
		}

	}
	return false;
}

bool SectorHelper::HasShipRepairing(UFlareSimulatedSector* TargetSector, UFlareCompany* Company)
{
	for (UFlareSimulatedSpacecraft* Spacecraft : TargetSector->GetSectorSpacecrafts())
	{
		if (Company != Spacecraft->GetCompany()) {
			continue;
		}

		if (Spacecraft->GetRepairStock() > 0 && Spacecraft->GetDamageSystem()->GetGlobalDamageRatio() < 1.f)
		{
			return true;
		}
	}
	return false;
}

bool SectorHelper::HasShipRefilling(TArray<UFlareSimulatedSpacecraft*>& Ships, UFlareCompany* Company)
{
	for (UFlareSimulatedSpacecraft* Spacecraft : Ships)
	{
		if (Company != Spacecraft->GetCompany()) {
			continue;
		}

		if (Spacecraft->GetRefillStock() > 0 && Spacecraft->NeedRefill())
		{
			return true;
		}

	}
	return false;
}

bool SectorHelper::HasShipRepairing(TArray<UFlareSimulatedSpacecraft*>& Ships, UFlareCompany* Company)
{
	for (UFlareSimulatedSpacecraft* Spacecraft : Ships)
	{
		if (Company != Spacecraft->GetCompany()) {
			continue;
		}

		if (Spacecraft->GetRepairStock() > 0 && Spacecraft->GetDamageSystem()->GetGlobalDamageRatio() < 1.f)
		{
			return true;
		}
	}
	return false;
}

void SectorHelper::GetRepairFleetSupplyNeeds(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32& CurrentNeededFleetSupply, int32& TotalNeededFleetSupply, int64& MaxDuration, bool OnlyPossible)
{
	TArray<UFlareSimulatedSpacecraft*> CompanySpacecraft;

	if(OnlyPossible && Sector->IsInDangerousBattle(Company))
	{
		// Don't add any spacecraft
	}
	else
	{
		for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorSpacecrafts()[SpacecraftIndex];

			if (Company != Spacecraft->GetCompany()) {
				continue;
			}

			CompanySpacecraft.Add(Spacecraft);
		}
	}

	GetRepairFleetSupplyNeeds(Sector, CompanySpacecraft, CurrentNeededFleetSupply, TotalNeededFleetSupply, MaxDuration, OnlyPossible);
}

void SectorHelper::GetRepairFleetSupplyNeeds(UFlareSimulatedSector* Sector,  TArray<UFlareSimulatedSpacecraft*>& ships, int32& CurrentNeededFleetSupply, int32& TotalNeededFleetSupply, int64& MaxDuration, bool OnlyPossible)
{
	float PreciseCurrentNeededFleetSupply = 0;
	float PreciseTotalNeededFleetSupply = 0;
	MaxDuration = 0;

	UFlareSpacecraftComponentsCatalog* Catalog = Sector->GetGame()->GetShipPartsCatalog();

	for(UFlareSimulatedSpacecraft* Spacecraft: ships)
	{
		if (!Spacecraft->GetDamageSystem()->IsAlive()) {
			continue;
		}

		if(OnlyPossible && Sector->IsInDangerousBattle(Spacecraft->GetCompany()))
		{
			continue;
		}

		float SpacecraftPreciseCurrentNeededFleetSupply = 0;
		float SpacecraftPreciseTotalNeededFleetSupply = 0;

		// List components
		for (int32 ComponentIndex = 0; ComponentIndex < Spacecraft->GetData().Components.Num(); ComponentIndex++)
		{
			FFlareSpacecraftComponentSave* ComponentData = &Spacecraft->GetData().Components[ComponentIndex];
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

			float DamageRatio = Spacecraft->GetDamageSystem()->GetDamageRatio(ComponentDescription, ComponentData);

			float TechnologyBonus = 1;
			if (Spacecraft->GetCompany()->IsTechnologyUnlocked("quick-repair"))// ? 1.5f : 1.f;
			{
				FFlareTechnologyDescription* QuickRepairTech = Sector->GetGame()->GetTechnologyCatalogDescription("quick-repair");
				if (!QuickRepairTech->RepairBonus)
				{
					TechnologyBonus = 1.5f;
				}
			}

			float TechnologyBonusSecondary = Spacecraft->GetCompany()->GetTechnologyBonus("repair-bonus");
			TechnologyBonus += TechnologyBonusSecondary;

			float ComponentMaxRepairRatio = GetComponentMaxRepairRatio(ComponentDescription) * (Spacecraft->GetSize() == EFlarePartSize::L ? 0.2f : 1.f) * TechnologyBonus;

			float CurrentRepairRatio = FMath::Min(ComponentMaxRepairRatio, (1.f - DamageRatio));
			float TotalRepairRatio = 1.f - DamageRatio;

			int64 Duration =  FMath::CeilToInt(TotalRepairRatio / ComponentMaxRepairRatio);
			if(Duration > MaxDuration)
			{
				MaxDuration = Duration;
			}

			SpacecraftPreciseCurrentNeededFleetSupply += CurrentRepairRatio * UFlareSimulatedSpacecraftDamageSystem::GetRepairCost(ComponentDescription);
			SpacecraftPreciseTotalNeededFleetSupply += TotalRepairRatio * UFlareSimulatedSpacecraftDamageSystem::GetRepairCost(ComponentDescription);
		}

		PreciseCurrentNeededFleetSupply += FMath::Max(0.f, SpacecraftPreciseCurrentNeededFleetSupply - Spacecraft->GetRepairStock());
		PreciseTotalNeededFleetSupply += FMath::Max(0.f, SpacecraftPreciseTotalNeededFleetSupply - Spacecraft->GetRepairStock());
	}

	// Round to ceil

	if(PreciseCurrentNeededFleetSupply < 0.001)
	{
		PreciseCurrentNeededFleetSupply = 0;
	}

	if(PreciseTotalNeededFleetSupply < 0.001)
	{
		PreciseTotalNeededFleetSupply = 0;
	}
	CurrentNeededFleetSupply = FMath::CeilToInt(PreciseCurrentNeededFleetSupply);
	TotalNeededFleetSupply = FMath::CeilToInt(PreciseTotalNeededFleetSupply);
}


void SectorHelper::GetRefillFleetSupplyNeeds(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32& CurrentNeededFleetSupply, int32& TotalNeededFleetSupply, int64& MaxDuration, bool OnlyPossible)
{
	TArray<UFlareSimulatedSpacecraft*> CompanySpacecraft;

	if(OnlyPossible && Sector->IsInDangerousBattle(Company))
	{
		// Don't add any spacecraft
	}
	else
	{
		for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorSpacecrafts()[SpacecraftIndex];

			if (Company != Spacecraft->GetCompany()) {
				continue;
			}

			CompanySpacecraft.Add(Spacecraft);
		}
	}

	GetRefillFleetSupplyNeeds(Sector, CompanySpacecraft, CurrentNeededFleetSupply, TotalNeededFleetSupply, MaxDuration, OnlyPossible);
}

void SectorHelper::GetRefillFleetSupplyNeeds(UFlareSimulatedSector* Sector, TArray<UFlareSimulatedSpacecraft*>& ships, int32& CurrentNeededFleetSupply, int32& TotalNeededFleetSupply, int64& MaxDuration, bool OnlyPossible)
{
	float PreciseCurrentNeededFleetSupply = 0;
	float PreciseTotalNeededFleetSupply = 0;
	MaxDuration = 0;
	UFlareSpacecraftComponentsCatalog* Catalog = Sector->GetGame()->GetShipPartsCatalog();

	for(UFlareSimulatedSpacecraft* Spacecraft: ships)
	{
		if (!Spacecraft->GetDamageSystem()->IsAlive()) {
			continue;
		}

		if(OnlyPossible && Sector->IsInDangerousBattle(Spacecraft->GetCompany()))
		{
			continue;
		}

		float SpacecraftPreciseCurrentNeededFleetSupply = 0;
		float SpacecraftPreciseTotalNeededFleetSupply = 0;

		// List components
		for (int32 ComponentIndex = 0; ComponentIndex < Spacecraft->GetData().Components.Num(); ComponentIndex++)
		{
			FFlareSpacecraftComponentSave* ComponentData = &Spacecraft->GetData().Components[ComponentIndex];
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

			if(ComponentDescription->Type == EFlarePartType::Weapon)
			{

				int32 MaxAmmo = ComponentDescription->WeaponCharacteristics.AmmoCapacity;
				int32 CurrentAmmo = MaxAmmo - ComponentData->Weapon.FiredAmmo;

				float FillRatio = (float) CurrentAmmo / (float) MaxAmmo;


				float CurrentRefillRatio = FMath::Min(MAX_REFILL_RATIO_BY_DAY * (Spacecraft->GetSize() == EFlarePartSize::L ? 0.2f : 1.f) , (1.f - FillRatio));
				float TotalRefillRatio = 1.f - FillRatio;

				int64 Duration =  FMath::CeilToInt(TotalRefillRatio / (MAX_REFILL_RATIO_BY_DAY * (Spacecraft->GetSize() == EFlarePartSize::L ? 0.2f : 1.f)));
				if(Duration > MaxDuration)
				{
					MaxDuration = Duration;
				}

				SpacecraftPreciseCurrentNeededFleetSupply += CurrentRefillRatio * UFlareSimulatedSpacecraftDamageSystem::GetRefillCost(ComponentDescription);
				SpacecraftPreciseTotalNeededFleetSupply += TotalRefillRatio * UFlareSimulatedSpacecraftDamageSystem::GetRefillCost(ComponentDescription);
			}
		}

		PreciseCurrentNeededFleetSupply += FMath::Max(0.f, SpacecraftPreciseCurrentNeededFleetSupply - Spacecraft->GetRefillStock());
		PreciseTotalNeededFleetSupply += FMath::Max(0.f, SpacecraftPreciseTotalNeededFleetSupply - Spacecraft->GetRefillStock());
	}

	// Round to ceil
	CurrentNeededFleetSupply = FMath::CeilToInt(PreciseCurrentNeededFleetSupply);
	TotalNeededFleetSupply = FMath::CeilToInt(PreciseTotalNeededFleetSupply);
}


void SectorHelper::RepairFleets(UFlareSimulatedSector* Sector, UFlareCompany* Company, UFlareFleet* Fleet, bool UseOwnedFleetMaterialsOnly)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareSectorHelper_RepairFleets);
	if (Sector->IsInDangerousBattle(Company))
	{
		return;
	}

	int32 CurrentNeededFleetSupply;
	int32 TotalNeededFleetSupply;
	int32 OwnedFS;
	int32 AvailableFS;
	int32 AffordableFS;
	int64 MaxDuration;
	TArray<UFlareSimulatedSpacecraft *> ShipsLookingAt;

	if(Fleet == nullptr)
	{
		GetRepairFleetSupplyNeeds(Sector, Company, CurrentNeededFleetSupply, TotalNeededFleetSupply, MaxDuration, true);
		GetAvailableFleetSupplyCount(Sector, Company, OwnedFS, AvailableFS, AffordableFS);
		ShipsLookingAt = Sector->GetSectorSpacecrafts();
	}
	else
	{
		GetRepairFleetSupplyNeeds(Sector, Fleet->GetShips(), CurrentNeededFleetSupply, TotalNeededFleetSupply, MaxDuration, true);
		GetAvailableFleetSupplyCount(Sector, Fleet->GetFleetCompany(), OwnedFS, AvailableFS, AffordableFS, nullptr, Fleet);
		ShipsLookingAt = Fleet->GetShips();
	}

	if (UseOwnedFleetMaterialsOnly)
	{
		AvailableFS = OwnedFS;
		AffordableFS = OwnedFS;
	}

	// Note not available fleet supply as consumed
	Sector->OnFleetSupplyConsumed(FMath::Max(0, TotalNeededFleetSupply - AvailableFS));

	if(AffordableFS == 0 || TotalNeededFleetSupply == 0)
	{
		// No repair possible
		//FLOGV("No repair possible for %s in %s", *Company->GetCompanyName().ToString(), *Sector->GetSectorName().ToString())
		return;
	}

	float RepairRatio = FMath::Min(1.f,(float) AffordableFS /  (float) TotalNeededFleetSupply);
	float RemainingFS = (float) AffordableFS;
	UFlareSpacecraftComponentsCatalog* Catalog = Company->GetGame()->GetShipPartsCatalog();

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < ShipsLookingAt.Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = ShipsLookingAt[SpacecraftIndex];

		if (Company != Spacecraft->GetCompany() || !Spacecraft->GetDamageSystem()->IsAlive()) {
			continue;
		}

		float SpacecraftPreciseTotalNeededFleetSupply = 0;


		// List components
		for (int32 ComponentIndex = 0; ComponentIndex < Spacecraft->GetData().Components.Num(); ComponentIndex++)
		{
			FFlareSpacecraftComponentSave* ComponentData = &Spacecraft->GetData().Components[ComponentIndex];
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

			float DamageRatio = Spacecraft->GetDamageSystem()->GetDamageRatio(ComponentDescription, ComponentData);
			float TotalRepairRatio = 1.f - DamageRatio;

			SpacecraftPreciseTotalNeededFleetSupply += TotalRepairRatio * UFlareSimulatedSpacecraftDamageSystem::GetRepairCost(ComponentDescription);


		}

		float SpacecraftNeededWithoutStock = SpacecraftPreciseTotalNeededFleetSupply - Spacecraft->GetRepairStock();
		float SpacecraftNeededWithoutStockScaled = FMath::Max(0.f, SpacecraftNeededWithoutStock * RepairRatio);
		float ConsumedFS = FMath::Min(RemainingFS, SpacecraftNeededWithoutStockScaled);
		Spacecraft->OrderRepairStock(ConsumedFS);
		RemainingFS -= ConsumedFS;

		if(RemainingFS <= 0)
		{
			break;
		}
	}

	int32 ConsumedFS = FMath::CeilToInt((float) AffordableFS - RemainingFS);
	ConsumeFleetSupply(Sector, Company, ConsumedFS, true, Fleet);

	if(ConsumedFS > 0 && Company == Sector->GetGame()->GetPC()->GetCompany())
	{
		Sector->GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("repair-start"));
	}
}

void SectorHelper::RefillFleets(UFlareSimulatedSector* Sector, UFlareCompany* Company, UFlareFleet* Fleet, bool UseOwnedFleetMaterialsOnly)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareSectorHelper_RefillFleets);

	int32 CurrentNeededFleetSupply;
	int32 TotalNeededFleetSupply;
	int32 OwnedFS;
	int32 AvailableFS;
	int32 AffordableFS;
	int64 MaxDuration;

	TArray<UFlareSimulatedSpacecraft *> ShipsLookingAt;

	if (Fleet == nullptr)
	{
		GetRefillFleetSupplyNeeds(Sector, Company, CurrentNeededFleetSupply, TotalNeededFleetSupply, MaxDuration, true);
		GetAvailableFleetSupplyCount(Sector, Company, OwnedFS, AvailableFS, AffordableFS);
		ShipsLookingAt = Sector->GetSectorSpacecrafts();
	}
	else
	{
		SectorHelper::GetRefillFleetSupplyNeeds(Sector, Fleet->GetShips(), CurrentNeededFleetSupply, TotalNeededFleetSupply, MaxDuration, true);
		SectorHelper::GetAvailableFleetSupplyCount(Sector, Fleet->GetFleetCompany(), OwnedFS, AvailableFS, AffordableFS, nullptr, Fleet);
		ShipsLookingAt = Fleet->GetShips();
	}

	if (UseOwnedFleetMaterialsOnly)
	{
		AvailableFS = OwnedFS;
		AffordableFS = OwnedFS;
	}

	// Note not available fleet supply as consumed
	Sector->OnFleetSupplyConsumed(FMath::Max(0, TotalNeededFleetSupply - AvailableFS));

	if(Sector->IsInDangerousBattle(Company) || AffordableFS == 0 || TotalNeededFleetSupply == 0)
	{
		// No refill possible
		return;
	}

	float MaxRefillRatio = FMath::Min(1.f,(float) AffordableFS /  (float) TotalNeededFleetSupply);
	float RemainingFS = (float) AffordableFS;
	UFlareSpacecraftComponentsCatalog* Catalog = Company->GetGame()->GetShipPartsCatalog();

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < ShipsLookingAt.Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = ShipsLookingAt[SpacecraftIndex];

		if (Company != Spacecraft->GetCompany() || !Spacecraft->GetDamageSystem()->IsAlive()) {
			continue;
		}

		float SpacecraftPreciseTotalNeededFleetSupply = 0;

		// List components
		for (int32 ComponentIndex = 0; ComponentIndex < Spacecraft->GetData().Components.Num(); ComponentIndex++)
		{
			FFlareSpacecraftComponentSave* ComponentData = &Spacecraft->GetData().Components[ComponentIndex];
			FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

			if(ComponentDescription->Type == EFlarePartType::Weapon)
			{
				int32 MaxAmmo = ComponentDescription->WeaponCharacteristics.AmmoCapacity;
				int32 CurrentAmmo = MaxAmmo - ComponentData->Weapon.FiredAmmo;

				float FillRatio = (float) CurrentAmmo / (float) MaxAmmo;

				float TotalRefillRatio = 1.f - FillRatio;

				SpacecraftPreciseTotalNeededFleetSupply += TotalRefillRatio * UFlareSimulatedSpacecraftDamageSystem::GetRefillCost(ComponentDescription);
			}
		}

		float SpacecraftNeededWithoutStock = SpacecraftPreciseTotalNeededFleetSupply - Spacecraft->GetRefillStock();
		float SpacecraftNeededWithoutStockScaled = FMath::Max(0.f, SpacecraftNeededWithoutStock * MaxRefillRatio);

		float ConsumedFS = FMath::Min(RemainingFS, SpacecraftNeededWithoutStockScaled);

		Spacecraft->OrderRefillStock(ConsumedFS);
		RemainingFS -= ConsumedFS;

		if(RemainingFS <= 0)
		{
			break;
		}
	}

	int32 ConsumedFS = FMath::CeilToInt((float) AffordableFS - RemainingFS);

	ConsumeFleetSupply(Sector, Company, ConsumedFS, false, Fleet);

	if(ConsumedFS > 0 && Company == Sector->GetGame()->GetPC()->GetCompany())
	{
		Sector->GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("refill-start"));
	}
}

void SectorHelper::ConsumeFleetSupply(UFlareSimulatedSector* Sector, UFlareCompany* Company, int32 ConsumedFS, bool ForRepair, UFlareFleet* ForFleet)
{
	Sector->OnFleetSupplyConsumed(ConsumedFS);
	FFlareResourceDescription* FleetSupply = Sector->GetGame()->GetScenarioTools()->FleetSupply;

	// First check for owned FS
	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorSpacecrafts()[SpacecraftIndex];

		if (Company != Spacecraft->GetCompany())
		{
			continue;
		}

		int TakenQuantity = Spacecraft->GetActiveCargoBay()->TakeResources(FleetSupply, ConsumedFS, Company);
		ConsumedFS -= TakenQuantity;

		if(ConsumedFS <= 0)
		{
			return;
		}
	}

	// Then buy unowned FS
	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorSpacecrafts()[SpacecraftIndex];

		if (Company == Spacecraft->GetCompany())
		{
			continue;
		}

		if (Spacecraft->IsHostile(Company))
		{
			// At war, no trade possible
			continue;
		}

		if (ForFleet)
		{
			FText Unused;
			if (!ForFleet->CanTradeWhiteListFrom(Spacecraft, FleetSupply) || !Spacecraft->CanTradeWhiteListTo(ForFleet, FleetSupply))
			{
				//Fleet whitelist restriction on trades
				continue;
			}
		}
		else
		{
			if (!Company->CanTradeWhiteListFrom(Spacecraft->GetCompany(), FleetSupply) || !Spacecraft->GetCompany()->CanTradeWhiteListTo(Company, FleetSupply))
			{
				//Company level whitelist restriction on trades
				continue;
			}
		}

		int TakenQuantity = Spacecraft->GetActiveCargoBay()->TakeResources(FleetSupply, ConsumedFS, Company);

		if(TakenQuantity > 0)
		{
			int32 ResourcePrice = Sector->GetResourcePrice(FleetSupply, EFlareResourcePriceContext::MaintenanceConsumption);

			int64 Cost = TakenQuantity * ResourcePrice;
			Company->TakeMoney(Cost, true, FFlareTransactionLogEntry::LogPayMaintenance(Spacecraft, TakenQuantity, ForRepair));
			Spacecraft->GetCompany()->GiveMoney(Cost, FFlareTransactionLogEntry::LogPaidForMaintenance(Spacecraft, Company, TakenQuantity, ForRepair));

			if(Spacecraft->GetCurrentFleet() && Spacecraft->GetCurrentFleet()->IsAutoTrading())
			{
				Spacecraft->GetCurrentFleet()->GetData()->AutoTradeStatsUnloadResources += TakenQuantity;
				Spacecraft->GetCurrentFleet()->GetData()->AutoTradeStatsMoneySell += Cost;
#if DEBUG_AI_TRADING_STATS

			FLOGV("Auto trading %s sell %d %s to %s for %lld", *Spacecraft->GetImmatriculation().ToString(), TakenQuantity, *FleetSupply->Name.ToString(), *Company->GetCompanyName().ToString(), Cost);
			FLOGV("AutoTradeStatsDays=%d LoadResources=%d UnloadResources=%d MoneyBuy=%lld MoneySell=%lld",
				  Spacecraft->GetCurrentFleet()->GetData()->AutoTradeStatsDays,
				  Spacecraft->GetCurrentFleet()->GetData()->AutoTradeStatsLoadResources,
				  Spacecraft->GetCurrentFleet()->GetData()->AutoTradeStatsUnloadResources,
				  Spacecraft->GetCurrentFleet()->GetData()->AutoTradeStatsMoneyBuy,
				  Spacecraft->GetCurrentFleet()->GetData()->AutoTradeStatsMoneySell);

#endif
			}

//			FLOGV("%s bought %d fleet supplies for %lld",*Company->GetCompanyName().ToString(), TakenQuantity, Cost);
			ConsumedFS -= TakenQuantity;

			if(ConsumedFS <= 0)
			{
				return;
			}
		}
	}
}

int32 SectorHelper::GetArmyCombatPoints(UFlareSimulatedSector* Sector, bool ReduceByDamage)
{
	int32 AllCombatPoints = 0;

	for (UFlareSimulatedSpacecraft* Spacecraft : Sector->GetSectorSpacecrafts())
	{
		AllCombatPoints += Spacecraft->GetCombatPoints(ReduceByDamage);
	}
	return AllCombatPoints;
}

int32 SectorHelper::GetHostileArmyCombatPoints(UFlareSimulatedSector* Sector, UFlareCompany* Company, bool ReduceByDamage)
{
	int32 HostileCombatPoints = 0;

	for(UFlareSimulatedSpacecraft* Spacecraft: Sector->GetSectorSpacecrafts())
	{
		if(!Spacecraft->IsHostile(Company))
		{
			continue;
		}
		HostileCombatPoints += Spacecraft->GetCombatPoints(ReduceByDamage);
	}
	return HostileCombatPoints;
}

int32 SectorHelper::GetCompanyArmyCombatPoints(UFlareSimulatedSector* Sector, UFlareCompany* Company, bool ReduceByDamage)
{
	int32 CompanyCombatPoints = 0;

	for(UFlareSimulatedSpacecraft* Spacecraft: Sector->GetSectorSpacecrafts())
	{
		if(Spacecraft->GetCompany() != Company)
		{
			continue;
		}
		CompanyCombatPoints += Spacecraft->GetCombatPoints(ReduceByDamage);
	}
	return CompanyCombatPoints;
}


TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> SectorHelper::ComputeSectorResourceStats(UFlareSimulatedSector* Sector, bool IncludeStorage)
{
	TMap<FFlareResourceDescription*, WorldHelper::FlareResourceStats> WorldStats;
	WorldStats.Reserve(Sector->GetGame()->GetResourceCatalog()->Resources.Num());

	// Init
	for(int32 ResourceIndex = 0; ResourceIndex < Sector->GetGame()->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Sector->GetGame()->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		WorldHelper::FlareResourceStats ResourceStats;
		ResourceStats.Production = 0;
		ResourceStats.Consumption = 0;
		ResourceStats.Balance = 0;
		ResourceStats.Stock = 0;
		ResourceStats.Capacity = 0;

		WorldStats.Add(Resource, ResourceStats);
	}

	for (int SpacecraftIndex = 0; SpacecraftIndex < Sector->GetSectorSpacecrafts().Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Sector->GetSectorSpacecrafts()[SpacecraftIndex];

		if(!IncludeStorage && Spacecraft->HasCapability(EFlareSpacecraftCapability::Storage))
		{
			continue;
		}

		// Stock
		TArray<FFlareCargo>& CargoBaySlots = Spacecraft->GetActiveCargoBay()->GetSlots();
		for (int CargoIndex = 0; CargoIndex < CargoBaySlots.Num(); CargoIndex++)
		{
			FFlareCargo& Cargo = CargoBaySlots[CargoIndex];

			if (!Cargo.Resource)
			{
				continue;
			}

			WorldHelper::FlareResourceStats *ResourceStats = &WorldStats[Cargo.Resource];

			FFlareResourceUsage Usage = Spacecraft->GetResourceUseType(Cargo.Resource);

			if(Usage.HasUsage(EFlareResourcePriceContext::FactoryInput) || Usage.HasUsage(EFlareResourcePriceContext::ConsumerConsumption) || Usage.HasUsage(EFlareResourcePriceContext::MaintenanceConsumption) || Usage.HasUsage(EFlareResourcePriceContext::HubInput))
			{
				ResourceStats->Capacity += Spacecraft->GetActiveCargoBay()->GetSlotCapacity() - Cargo.Quantity;
			}

			if(Usage.HasUsage(EFlareResourcePriceContext::FactoryOutput) || Usage.HasUsage(EFlareResourcePriceContext::MaintenanceConsumption) || Usage.HasUsage(EFlareResourcePriceContext::HubOutput))
			{
				ResourceStats->Stock += Cargo.Quantity;
			}
		}

		for (int32 FactoryIndex = 0; FactoryIndex < Spacecraft->GetFactories().Num(); FactoryIndex++)
		{
			UFlareFactory* Factory = Spacecraft->GetFactories()[FactoryIndex];

			if(Factory->IsShipyard() && !Factory->IsActive())
			{
				const FFlareProductionData* ProductionData = Spacecraft->GetNextOrderShipProductionData(Factory->IsLargeShipyard()? EFlarePartSize::L : EFlarePartSize::S);

				if (ProductionData)
				{
					for(const FFlareFactoryResource& FactoryResource : ProductionData->InputResources)
					{
						const FFlareResourceDescription* Resource = &FactoryResource.Resource->Data;
						WorldHelper::FlareResourceStats *ResourceStats = &WorldStats[Resource];

						int64 ProductionDuration = ProductionData->ProductionTime;

						float Flow = 0;

						if (ProductionDuration == 0)
						{
							Flow = 1;
						}
						else
						{
							Flow = (float) FactoryResource.Quantity / float(ProductionDuration);
						}

						ResourceStats->Consumption += Flow;
					}
				}

				continue;
			}

			if ((!Factory->IsActive() || !Factory->IsNeedProduction()))
			{
				// No resources needed
				continue;
			}

			// Input flow
			for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetInputResourcesCount(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = Factory->GetInputResource(ResourceIndex);
				WorldHelper::FlareResourceStats *ResourceStats = &WorldStats[Resource];
				int64 ProductionDuration = Factory->GetProductionDuration();
				float Flow = 0;
				if (ProductionDuration == 0)
				{
					Flow = 1;
				}
				else
				{
					Flow = (float)Factory->GetInputResourceQuantity(ResourceIndex) / float(ProductionDuration);
				}

				ResourceStats->Consumption += Flow;
			}

			// Ouput flow
			for (int32 ResourceIndex = 0; ResourceIndex < Factory->GetOutputResourcesCount(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = Factory->GetOutputResource(ResourceIndex);
				WorldHelper::FlareResourceStats *ResourceStats = &WorldStats[Resource];
				int64 ProductionDuration = Factory->GetProductionDuration();
				if (ProductionDuration == 0)
				{
					ProductionDuration = 10;
				}

				float Flow = (float)Factory->GetOutputResourceQuantity(ResourceIndex) / float(ProductionDuration);
				ResourceStats->Production += Flow;
			}
		}
	}

	// FS
	FFlareResourceDescription* FleetSupply = Sector->GetGame()->GetScenarioTools()->FleetSupply;
	WorldHelper::FlareResourceStats *FSResourceStats = &WorldStats[FleetSupply];
	FFlareFloatBuffer* Stats = &Sector->GetData()->FleetSupplyConsumptionStats;
	float MeanConsumption = Stats->GetMean(0, Stats->MaxSize-1);
	FSResourceStats->Consumption += MeanConsumption;
	FSResourceStats->Capacity += Stats->GetValue(0);


	// Customer flow
	for (int32 ResourceIndex = 0; ResourceIndex < Sector->GetGame()->GetResourceCatalog()->ConsumerResources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Sector->GetGame()->GetResourceCatalog()->ConsumerResources[ResourceIndex]->Data;
		WorldHelper::FlareResourceStats *ResourceStats = &WorldStats[Resource];

		ResourceStats->Consumption += Sector->GetPeople()->GetRessourceConsumption(Resource, false);
	}

	// Balance
	for(int32 ResourceIndex = 0; ResourceIndex < Sector->GetGame()->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
	{
		FFlareResourceDescription* Resource = &Sector->GetGame()->GetResourceCatalog()->Resources[ResourceIndex]->Data;
		WorldHelper::FlareResourceStats *ResourceStats = &WorldStats[Resource];

		ResourceStats->Balance = ResourceStats->Production - ResourceStats->Consumption;

		/*FLOGV("World stats for %s: Production=%f Consumption=%f Balance=%f Stock=%d",
			  *Resource->Name.ToString(),
			  ResourceStats->Production,
			  ResourceStats->Consumption,
			  ResourceStats->Balance,
			  ResourceStats->Stock);*/
	}

	return WorldStats;

}
