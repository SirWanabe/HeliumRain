#pragma once

#include "../Flare.h"
#include "FlareFleet.h"
#include "FlareTradeRoute.h"
#include "FlareCompanyWhiteList.h"
#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "FlareGameTypes.generated.h"

#define AI_MAX_STATION_PER_SECTOR 30

class AFlareSpacecraft;

/*----------------------------------------------------
	General gameplay enums
----------------------------------------------------*/

/** Hostility status */
UENUM()
namespace EFlareHostility
{
	enum Type
	{
		Hostile,
		Neutral,
		Friendly,
		Owned
	};
}

/** Hostility status */
UENUM()
namespace EFlareBudget
{
	enum Type
	{
		Military,
		Station,
		Technology,
		Trade,
		None
	};
}

/** Combat groups */
UENUM()
namespace EFlareCombatGroup
{
	enum Type
	{
		AllMilitary,
		Capitals,
		Fighters,
		Civilan
	};
}

/** Resource price context */
UENUM()
namespace EFlareResourcePriceContext
{
	enum Type
	{
		Default, /** Default price */
		FactoryInput, /** Price selling to a factory needing the resource */
		FactoryOutput, /** Price buying the resource to a factory */
		HubInput, /** Price selling to a hub needing the resource */
		HubOutput, /** Price buying the resource to a hub */
		ConsumerConsumption, /** Price selling to a the people */
		MaintenanceConsumption, /** Price selling to company using maintenance */
	};
}


struct FFlareResourceUsage
{
public:
	bool HasAnyUsage() const;

	bool HasUsage(EFlareResourcePriceContext::Type Usage) const;

	void AddUsage(EFlareResourcePriceContext::Type Usage);
private:
	TArray<EFlareResourcePriceContext::Type> Usages;
};

/** Combat tactics */
UENUM()
namespace EFlareCombatTactic
{
	enum Type
	{
		AttackAll,
		ProtectMe,
		AttackMilitary,
		AttackStations,
		AttackCivilians,
		StandDown
	};
}

/** Sector knowledge status */
UENUM()
namespace EFlareSectorKnowledge
{
	enum Type
	{
		Unknown, /** The existence of this sector is unknown */
		Known, /** The sector is visible on the map but its content is unknown */
		Visited /** The sector is visited, all static structure are visible */
	};
}

/** Technology type for user display */
UENUM()
namespace EFlareTechnologyCategory
{
	enum Type
	{
		General, /** All-purpose tech */
		Economy, /** Economy tech */
		Military /** Military tech */
	};
}


/*----------------------------------------------------
	General gameplay data
----------------------------------------------------*/

/** Company sector knowledge data */
USTRUCT()
struct FFlareCompanySectorKnowledge
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	FName SectorIdentifier;

	UPROPERTY(EditAnywhere, Category = Save)
	TEnumAsByte<EFlareSectorKnowledge::Type> Knowledge;
};

/** Technology description */
USTRUCT()
struct FFlareTechnologyDescription
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Content)
	FName Identifier;

	UPROPERTY(EditAnywhere, Category = Content)
	int32 ResearchCost = 20;

	UPROPERTY(EditAnywhere, Category = Content)
	int32 Level;

	UPROPERTY(EditAnywhere, Category = Content)
	FText Name;

	UPROPERTY(EditAnywhere, Category = Content)
	FText Description;

	UPROPERTY(EditAnywhere, Category = Content)
	TEnumAsByte<EFlareTechnologyCategory::Type> Category;

	UPROPERTY(EditAnywhere, Category = Content)
	bool PlayerOnly;

	UPROPERTY(EditAnywhere, Category = Content) bool AIOnly;

	/** Used to disable vanilla research, RequiredTechnologies array will still be read. Disabled technology can be used to disable ships/components too*/
	UPROPERTY(EditAnywhere, Category = Content) bool IsDisabled;

	UPROPERTY(EditAnywhere, Category = Content) float RepairBonus;
	UPROPERTY(EditAnywhere, Category = Content) float DiplomaticPenaltyBonus;
	UPROPERTY(EditAnywhere, Category = Content) float DiplomaticBonus;
	UPROPERTY(EditAnywhere, Category = Content) float TravelBonus;
	UPROPERTY(EditAnywhere, Category = Content) float ShipyardFabBonus;

	/** All required unlocked technologies to unlock this technology*/
	UPROPERTY(EditAnywhere, Category = Content) TArray<FName> RequiredTechnologies;

	/** Companies that can unlock this research. Company ShortName, or PLAYER*/
	UPROPERTY(EditAnywhere, Category = Content) TArray<FName> ResearchableCompany;

	/** An array of the names of which this Technology unlocks. If multiple technologies unlock the same item both of those technologies are required.
		Can unlock Stations,Ships and Components */
	UPROPERTY(EditAnywhere, Category = Content) TArray<FName> UnlockItems;

	/** This adjusts the priority of this technology being loaded. Higher numbers will override lower numbers. Equal numbers will override each other based on their load order*/
	UPROPERTY(EditAnywhere, Category = Content) int ModLoadPriority;
};

/** Scannable description */
USTRUCT()
struct FFlareScannableDescription
{
	GENERATED_USTRUCT_BODY()
		
	UPROPERTY(EditAnywhere, Category = Content)
	FName Identifier;

	UPROPERTY(EditAnywhere, Category = Content)
	FText Name;

	UPROPERTY(EditAnywhere, Category = Content)
	FText Description;
};

/** Company reputation save data */
USTRUCT()
struct FFlareCompanyReputationSave
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	FName CompanyIdentifier;

	UPROPERTY(EditAnywhere, Category = Save)
	float Reputation;

};

/** Company Licenses save data */
USTRUCT()
struct FFlareCompanyLicensesSave
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> LicenseBuilding;

	/** Used for giving starting licenses upon game load if neccessary */
	UPROPERTY(EditAnywhere, Category = Save)
	bool HasRecievedStartingLicenses;
};

/** Company AI save data */
USTRUCT()
struct FFlareCompanyAISave
{
	GENERATED_USTRUCT_BODY()

	int64 BudgetTechnology;
	int64 BudgetMilitary;
	int64 BudgetStation;
	int64 BudgetTrade;

	/* Modify AttackThreshold */
	float Caution;
	float Pacifism;

	FName ResearchProject;
	FName DesiredStationLicense;
	int64 DateBoughtLicense;

	bool CalculatedDefaultBudget;
};

/** Transaction type*/
UENUM()
namespace EFlareTransactionLogEntry
{
	enum Type
	{
		ManualResourcePurchase = 0,
		ManualResourceSell,
		TradeRouteResourcePurchase,
		TradeRouteResourceSell,
		FactoryWages,
		CancelFactoryWages,
		StationConstructionFees,
		StationUpgradeFees,
		CancelStationUpgradeFees,
		UpgradeShipPart,
		OrderShip,
		CancelOrderShip,
		OrderShipAdvance,
		PeoplePurchase,
		InitialMoney,
		PayRepair,
		PayRefill,
		PaidForRepair, // AI only
		PaidForRefill, // AI only
		SendTribute,
		PaidSectorStationLicense,
		ReceiveTribute, // AI only
		RecoveryFees,
		ScrapGain,
		Cheat, // cheat !
		QuestReward,
		MutualAssistance, // AI only
		ScrapCost, // AI only

		TYPE_COUNT
	};
}

class UFlareFactory;

USTRUCT()
struct FFlareTransactionLogEntry
{
	GENERATED_USTRUCT_BODY()

	static FFlareTransactionLogEntry LogUpgradeShipPart(UFlareSimulatedSpacecraft* Spacecraft);
	static FFlareTransactionLogEntry LogOrderShip(UFlareSimulatedSpacecraft* Shipyard, FName OrderShipClass);
	static FFlareTransactionLogEntry LogCancelOrderShip(UFlareSimulatedSpacecraft* Shipyard, FName OrderShipClass);
	static FFlareTransactionLogEntry LogShipOrderAdvance(UFlareSimulatedSpacecraft* Shipyard, FName Company, FName ShipClass);
	static FFlareTransactionLogEntry LogFactoryWages(UFlareFactory* Factory);
	static FFlareTransactionLogEntry LogCancelFactoryWages(UFlareFactory* Factory);
	static FFlareTransactionLogEntry LogPeoplePurchase(UFlareSimulatedSpacecraft* Station, FFlareResourceDescription* Resource, int32 Quantity);
	static FFlareTransactionLogEntry LogInitialMoney();
	static FFlareTransactionLogEntry LogBuyResource(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, int32 GivenResources, UFlareTradeRoute* TradeRoute);
	static FFlareTransactionLogEntry LogSellResource(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, int32 GivenResources, UFlareTradeRoute* TradeRoute);
	static FFlareTransactionLogEntry LogPayMaintenance(UFlareSimulatedSpacecraft* SellerSpacecraft, int32 TakenQuantity, bool ForRepair);
	static FFlareTransactionLogEntry LogPaidForMaintenance(UFlareSimulatedSpacecraft* SellerSpacecraft, UFlareCompany* Company, int32 TakenQuantity, bool ForRepair);
	static FFlareTransactionLogEntry LogPaidSectorStationBuildingLicense(UFlareSimulatedSector* Sector);	
	static FFlareTransactionLogEntry LogSendTribute(UFlareCompany* Company);
	static FFlareTransactionLogEntry LogReceiveTribute(UFlareCompany* Company);
	static FFlareTransactionLogEntry LogRecoveryFees();
	static FFlareTransactionLogEntry LogBuildStationFees(FFlareSpacecraftDescription* StationDescription, UFlareSimulatedSector* Sector);
	static FFlareTransactionLogEntry LogUpgradeStationFees(UFlareSimulatedSpacecraft* Station);
	static FFlareTransactionLogEntry LogCancelUpgradeStationFees(UFlareSimulatedSpacecraft* Station);
	static FFlareTransactionLogEntry LogScrapCost(UFlareCompany* Company);
	static FFlareTransactionLogEntry LogScrapGain(UFlareSimulatedSpacecraft* ScrappedShip, UFlareCompany* Company);
	static FFlareTransactionLogEntry LogMutualAssistance();
	static FFlareTransactionLogEntry LogCheat();
	static FFlareTransactionLogEntry LogQuestReward(UFlareCompany* Company);

	int64 Date;
	int64 Amount;
	EFlareTransactionLogEntry::Type Type;
	FName Spacecraft;
	FName Sector;
	FName OtherCompany;
	FName OtherSpacecraft;
	FName Resource;
	int32 ResourceQuantity;
	FName ExtraIdentifier1;
	FName ExtraIdentifier2;

	static FText GetCategoryDescription(EFlareTransactionLogEntry::Type Type);

	UFlareCompany* GetOtherCompany(AFlareGame* Game) const;
	UFlareSimulatedSector* GetSector(AFlareGame* Game) const;
	UFlareSimulatedSpacecraft* GetSpacecraft(AFlareGame* Game) const;
	FText GetComment(AFlareGame* Game) const;
};

/** Game save data */
USTRUCT()
struct FFlareCompanySave
{
	GENERATED_USTRUCT_BODY()

	/** Save identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Index of the company description in the catalog, or -1 if player */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CatalogIdentifier;

	/** Money money money / Always funny / In a rich men's world */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 Money;

	/** Hostile companies */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> HostileCompanies;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSpacecraftSave> ShipData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSpacecraftSave> ChildStationData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSpacecraftSave> StationData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSpacecraftSave> DestroyedSpacecraftData;

	/** Company fleets */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareFleetSave> Fleets;

	/** Company trade routes */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareTradeRouteSave> TradeRoutes;

	/** Company white lists */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareWhiteListSave> WhiteLists;

	UPROPERTY(EditAnywhere, Category = Save)
	int32 FleetImmatriculationIndex;

	UPROPERTY(EditAnywhere, Category = Save)
	int32 TradeRouteImmatriculationIndex;

	UPROPERTY(EditAnywhere, Category = Save)
	int32 WhiteListImmatriculationIndex;

	UPROPERTY(EditAnywhere, Category = Save)
	FName DefaultWhiteListIdentifier;

	/** List of known or visited sectors */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareCompanySectorKnowledge> SectorsKnowledge;

	/** Company Licenses data */
	UPROPERTY(EditAnywhere, Category = Save)
	FFlareCompanyLicensesSave Licenses;

	/** Company AI data */
	UPROPERTY(EditAnywhere, Category = Save)
	FFlareCompanyAISave AI;

	/** Player reputation */
	UPROPERTY(EditAnywhere, Category = Save)
	float PlayerReputation;

	/** Value of all company assets */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 CompanyValue;

	/** Date of last tribute given to the player */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 PlayerLastTributeDate;

	/** Date of last peace with the player */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 PlayerLastPeaceDate;

	/** Date of last war with the player */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 PlayerLastWarDate;

	/** Unlocked technologies */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> UnlockedTechnologies;

	/** Science amount */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 ResearchAmount;

	/** Research inflation ratio*/
	UPROPERTY(EditAnywhere, Category = Save)
	float ResearchRatio;

	/** Research inflation ratio*/
	UPROPERTY(EditAnywhere, Category = Save)
	int32 ResearchSpent;

	/** List of capture target */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> CaptureOrders;

	/** List of company transactions */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareTransactionLogEntry> TransactionLog;

	/** Quantity of damage deal to others companies */
	UPROPERTY(EditAnywhere, Category = Save)
	float Retaliation;
};

USTRUCT()
struct FFlareCompanyStartingShips
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere, Category = Company)
	FName ShipIdentifier;
	UPROPERTY(EditAnywhere, Category = Company)
	FName SpawnSector;
	UPROPERTY(EditAnywhere, Category = Company)
	int32 Quantity = 1;

	/** If this is a station what level is it? */
	UPROPERTY(EditAnywhere, Category = Company)
	uint32 Level = 1;
	UPROPERTY(EditAnywhere, Category = Company)
	bool IsStation = false;

	/** For starting scenarios, is this the player ship? At least one entry must be set to "true"*/
	UPROPERTY(EditAnywhere, Category = Company)
	bool IsPlayerShip = false;
};

USTRUCT()
struct FFlareCompanyAIDescription
{
	GENERATED_USTRUCT_BODY()

	/** How strongly the faction wants to build stations that produce this resource */
	UPROPERTY(EditAnywhere, Category = Company)
	TMap<FName, float> ResourceAffilities;

	/** How strongly the faction wants to build their stations in this sector or moon*/
	UPROPERTY(EditAnywhere, Category = Company)
	TMap<FName, float>		SectorAffilities;

	/** Default: 1*/
	UPROPERTY(EditAnywhere, Category = Company)
	float TradingBuy;
	/** Default: 1*/
	UPROPERTY(EditAnywhere, Category = Company)
	float TradingSell;

	/** Default: 1*/
	UPROPERTY(EditAnywhere, Category = Company)
	float ShipyardAffility;
	/** Default: 0.5*/
	UPROPERTY(EditAnywhere, Category = Company)
	float ConsumerAffility;
	/** Default: 0.1*/
	UPROPERTY(EditAnywhere, Category = Company)
	float MaintenanceAffility;

	/** Default: 0.05*/
	UPROPERTY(EditAnywhere, Category = Company)
	float BudgetTechnologyWeight;
	/** Default: 0.30*/
	UPROPERTY(EditAnywhere, Category = Company)
	float BudgetMilitaryWeight;
	/** Default: 0.34*/
	UPROPERTY(EditAnywhere, Category = Company)
	float BudgetStationWeight;
	/** Default: 0.31*/
	UPROPERTY(EditAnywhere, Category = Company)
	float BudgetTradeWeight;

	UPROPERTY(EditAnywhere, Category = Company)
	/** Default: 0.00*/
	float BudgetWarTechnologyWeight;
	UPROPERTY(EditAnywhere, Category = Company)
	/** Default: 0.75*/
	float BudgetWarMilitaryWeight;
	UPROPERTY(EditAnywhere, Category = Company)
	/** Default: 0.125*/
	float BudgetWarStationWeight;
	UPROPERTY(EditAnywhere, Category = Company)
	/** Default: 0.125*/
	float BudgetWarTradeWeight;

	UPROPERTY(EditAnywhere, Category = Company)
	/** Default: 1.05*/
	float WarDeclared_StationBudgetFactor;
	UPROPERTY(EditAnywhere, Category = Company)
	/** Default: 0.85*/
	float WarDeclared_TechnologyBudgetFactor;
	UPROPERTY(EditAnywhere, Category = Company)
	/** Default: 1.25*/
	float WarDeclared_TradeBudgetFactor;
	UPROPERTY(EditAnywhere, Category = Company)
	/** Default: 1.10*/
	float WarDeclared_TransferToMilitaryBudgetFactor;

	UPROPERTY(EditAnywhere, Category = Company)
	/** Default: 0.05*/
	float WarDeclared_TransferToMilitaryBudgetStationPercent;
	UPROPERTY(EditAnywhere, Category = Company)
	/** Default: 0.05*/
	float WarDeclared_TransferToMilitaryBudgetTechnologyPercent;
	UPROPERTY(EditAnywhere, Category = Company)
	/** Default: 0.05*/
	float WarDeclared_TransferToMilitaryBudgetTradePercent;

	UPROPERTY(EditAnywhere, Category = Company)
	/** Default: 1*/
	int32 MaxTradeShipsBuildingPeace;
	UPROPERTY(EditAnywhere, Category = Company)
	/** Default: 0*/
	int32 MaxTradeShipsBuildingWar;
	UPROPERTY(EditAnywhere, Category = Company)
	/** Default: 1*/
	int32 MaxMilitaryShipsBuildingPeace;
	UPROPERTY(EditAnywhere, Category = Company)
	/** Default: 100*/
	int32 MaxMilitaryShipsBuildingWar;

	/** Default: 0.99*/
	UPROPERTY(EditAnywhere, Category = Company)
	float AttackThreshold;
	/** Default: 0.5*/
	UPROPERTY(EditAnywhere, Category = Company)
	float RetreatThreshold;
	/** Default: 0.01*/
	UPROPERTY(EditAnywhere, Category = Company)
	float DefeatAdaptation;

	/** Default: -0.1*/
	UPROPERTY(EditAnywhere, Category = Company)
	float ConfidenceTarget;
	/** Default: 0.2*/
	UPROPERTY(EditAnywhere, Category = Company)
	float DeclareWarConfidence;
	/** Default: -0.8*/
	UPROPERTY(EditAnywhere, Category = Company)
	float PayTributeConfidence;

	/** While at war factions pacifism increases by this rate per day. Default: 0.8*/
	UPROPERTY(EditAnywhere, Category = Company)
	float PacifismIncrementRate;
	/** While at peace and faction has 75% of its military available pacifism decreases by this rate per day. Multiplied by +0.50 if faction is in number #1 spot or +0.25 if the next weaker faction is at least 30% weaker. Default: 0.6*/
	UPROPERTY(EditAnywhere, Category = Company)
	float PacifismDecrementRate;

	/** When accepting a tribute, reset pacifism to this value. Default 75.*/
	UPROPERTY(EditAnywhere, Category = Company)
	float PacifismAfterTribute;

	/** When building military ships faction requires (Price*CostSafetyMargin) + (TotalDailyProductionCost*DailyProductionCostSensitivityEconomic) money avilable.  Default: 30*/
	UPROPERTY(EditAnywhere, Category = Company)
	float DailyProductionCostSensitivityMilitary;
	/** When building stations or trade ships faction requires (Price*CostSafetyMargin) + (TotalDailyProductionCost*DailyProductionCostSensitivityEconomic) money avilable. Default: 7*/
	UPROPERTY(EditAnywhere, Category = Company)
	float DailyProductionCostSensitivityEconomic;

	/** When building a military ship faction must have ship cost * CostSafetyMarginMilitaryShip money to build. Default: 1.1*/
	UPROPERTY(EditAnywhere, Category = Company)
	float CostSafetyMarginMilitaryShip;
	/** When building a trade ship faction must have ship cost * CostSafetyMarginTradeShip money to build. Default: 1.05*/
	UPROPERTY(EditAnywhere, Category = Company)
	float CostSafetyMarginTradeShip;
	/** When building a station faction must have station cost * CostSafetyMarginStation money to build. Default: 1.1*/
	UPROPERTY(EditAnywhere, Category = Company)
	float CostSafetyMarginStation;

	/** How many ships a company owns before considering scrapping. Default 60.*/
	UPROPERTY(EditAnywhere, Category = Company)
	int	Scrap_Minimum_Ships;

	/** Minimum amount of S Cargo ships to keep when scrapping. Default 10*/
	UPROPERTY(EditAnywhere, Category = Company)
	int	Scrap_Min_S_Cargo;

	/** Minimum amount of S Military ships to keep when scrapping. Default 10*/
	UPROPERTY(EditAnywhere, Category = Company)
	int	Scrap_Min_S_Military;

	/** When building a new station, company considers the station cost as multiplied by this value. Default 1.2.*/
	UPROPERTY(EditAnywhere, Category = Company)
	float BuildStationWorthMultiplier;

	/** Maximum amount of shipyards this company can build. Default 5.*/
	UPROPERTY(EditAnywhere, Category = Company)
	int	Station_Shipyard_Maximum;

	/** When considering buying a military ship, how much is each Drone slot worth to the company in Combat Points. Default 3.*/
	UPROPERTY(EditAnywhere, Category = Company)
	float BuildDroneCombatWorth;

	/** How many ships should faction own before switching to building L sized ships exclusively. Default: 50*/
	UPROPERTY(EditAnywhere, Category = Company)
	int32 BuildLTradeOnlyTreshhold;
	/** How many ships should faction own before switching to building L sized ships exclusively. Default: 50*/
	UPROPERTY(EditAnywhere, Category = Company)
	int32 BuildLMilitaryOnlyTreshhold;

	/** Default: 1*/
	UPROPERTY(EditAnywhere, Category = Company)
	int32 BuildMilitaryDiversity;
	/** Default: 1*/
	UPROPERTY(EditAnywhere, Category = Company)
	int32 BuildTradeDiversity;

	/** Default: 5*/
	UPROPERTY(EditAnywhere, Category = Company)
	int32 BuildMilitaryDiversitySize;
	/** Default: 5*/
	UPROPERTY(EditAnywhere, Category = Company)
	int32 BuildTradeDiversitySize;
	/** Default: 5*/
	UPROPERTY(EditAnywhere, Category = Company)
	int32 BuildMilitaryDiversitySizeBase;
	/** Default: 10*/
	UPROPERTY(EditAnywhere, Category = Company)
	int32 BuildTradeDiversitySizeBase;

	/** Default: 365*/
	UPROPERTY(EditAnywhere, Category = Company)
	int64 DaysUntilTryGetStationLicense;

	/** Chance faction will ignore its diversity ratio and try to build a better military ship. Default: 0.05*/
	UPROPERTY(EditAnywhere, Category = Company)
	float BuildEfficientMilitaryChance;
	/** Chance faction will ignore its diversity ratio and try to build a better trade ship. Default: 0.10*/
	UPROPERTY(EditAnywhere, Category = Company)
	float BuildEfficientTradeChance;
	/** Extra chance ontop of BuildEfficientMilitaryChance that faction will ignore its diversity ratio and build a better small military ship. Default: 0.10*/
	UPROPERTY(EditAnywhere, Category = Company)
	float BuildEfficientMilitaryChanceSmall;
	/** Extra chance ontop of BuildEfficientTradeChance that faction will ignore its diversity ratio and build a better small trade ship.  Default: 0.30*/
	UPROPERTY(EditAnywhere, Category = Company)
	float BuildEfficientTradeChanceSmall;

	/** Maximum possible percentage of Military S ships equipped with capture harpoons. Default: 0.10*/
	UPROPERTY(EditAnywhere, Category = Company)
	float UpgradeMilitarySalvagerSRatio;
	/** Maximum possible percentage of Military L ships equipped with capture harpoons. Default: 0.10*/
	UPROPERTY(EditAnywhere, Category = Company)
	float UpgradeMilitarySalvagerLRatio;

	/** If enabled this company won't directly initiate war declarations but will join other groups which are fighting*/
	UPROPERTY(EditAnywhere, Category = Company)
	bool IsMercenaryCompany;

	/** Preset research order. If unable to research anything in array will choose random available research. Default: science,instruments*/
	UPROPERTY(EditAnywhere, Category = Company)
	TArray<FName> ResearchOrder;
};

/** Catalog data */
USTRUCT()
struct FFlareCompanyDescription
{
	GENERATED_USTRUCT_BODY()

	/** Name */
	UPROPERTY(EditAnywhere, Category = Company)
	FText Name;

	/** Short name */
	UPROPERTY(EditAnywhere, Category = Company)
	FName ShortName;

	/** Name */
	UPROPERTY(EditAnywhere, Category = Company)
	FText Description;

	/** Emblem */
	UPROPERTY(EditAnywhere, Category = Company)
	UTexture2D* Emblem;

	/** Billboard */
	UPROPERTY(EditAnywhere, Category = Company)
	TArray<UTexture2D*> Billboards;

	/** Base color in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	FLinearColor CustomizationBasePaintColor;

	/** Paint color in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	FLinearColor CustomizationPaintColor;

	/** Overlay color in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	FLinearColor CustomizationOverlayColor;

	/** Lights color in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	FLinearColor CustomizationLightColor;

	/** Pattern index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationPatternIndex;

	UPROPERTY(EditAnywhere, Category = Company)
	int32 StartingResearchPoints;

	/** Passive daily research generating ability for this company */
	UPROPERTY(EditAnywhere, Category = Company)
	int32 PassiveResearchGeneration;

	UPROPERTY(EditAnywhere, Category = Company)
	int64 StartingMoney;

	UPROPERTY(EditAnywhere, Category = Company)
	TArray<FName> StartingSectorKnowledge;

	UPROPERTY(EditAnywhere, Category = Company)
	TArray<FName> StartingTechnology;

	UPROPERTY(EditAnywhere, Category = Company)
	TArray<FFlareCompanyStartingShips> StartingShips;

	UPROPERTY(EditAnywhere, Category = Content)
	FFlareCompanyAIDescription AI_Behaviours;

	/** This adjusts the priority of this faction being loaded. Higher numbers will override lower numbers. Equal numbers will override each other based on their load order*/
	UPROPERTY(EditAnywhere, Category = Content) int ModLoadPriority;
};

/** Incoming event description */
USTRUCT()
struct FFlareIncomingEvent
{
	GENERATED_USTRUCT_BODY()

	/** Event text */
	UPROPERTY(EditAnywhere, Category = Save)
	FText Text;

	/** Days until event */
	UPROPERTY(EditAnywhere, Category = Content)
	int64 RemainingDuration;
};


/*----------------------------------------------------
	Basic structures
----------------------------------------------------*/

struct CompanyValue
{
	int64 MoneyValue;
	int64 StockValue;
	int64 ShipsValue;
	int64 ArmyValue;
	int32 ArmyTotalCombatPoints;
	int32 ArmyCurrentCombatPoints;

	int64 StationsValue;

	/** Ships + Stations*/
	int64 SpacecraftsValue;

	/** Money + Spacecrafts + Stock */
	int64 TotalValue;
	int64 TotalDailyProductionCost;

	int32 TotalShipCount;
	int32 TotalDroneCount;
	int32 TotalShipCountMilitaryS;
	int32 TotalShipCountMilitarySSalvager;
	int32 TotalShipCountMilitaryL;
	int32 TotalShipCountMilitaryLSalvager;
	int32 TotalShipCountTradeS;
	int32 TotalShipCountTradeL;

};

struct WarTargetIncomingFleet
{
	int64 TravelDuration;
	int32 ArmyCombatPoints;
};

struct WarTarget
{
	UFlareSimulatedSector* Sector;
	int32 EnemyArmyCombatPoints;
	int32 EnemyArmyLCombatPoints;
	int32 EnemyArmySCombatPoints;
	int64 EnemyStationCount;
	int64 EnemyCargoCount;
	int32 OwnedArmyCombatPoints;
	int32 OwnedArmyAntiSCombatPoints;
	int32 OwnedArmyAntiLCombatPoints;
	int64 OwnedStationCount;
	int64 OwnedCargoCount;
	int64 OwnedMilitaryCount;
	TArray<WarTargetIncomingFleet> WarTargetIncomingFleets; // List player company fleets
	TArray<UFlareCompany*> ArmedDefenseCompanies;
};


/*----------------------------------------------------
	Low-level tools
----------------------------------------------------*/

/** Game save data */
USTRUCT()
struct FFlareFloatBuffer
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	int32 MaxSize;

	UPROPERTY(EditAnywhere, Category = Save)
	int32 WriteIndex;

	UPROPERTY(EditAnywhere, Category = Save)
	TArray<float> Values;


	void Init(int32 Size);

	void Resize(int32 Size);

	void Append(float NewValue);

	float GetValue(int32 Age);

	float GetMean(int32 StartAge, int32 EndAge);
};


USTRUCT()
struct FVectorArray
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FVector> Entries;
};

USTRUCT()
struct FNameArray
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FName> Entries;
};


USTRUCT()
struct FPtr
{
	GENERATED_USTRUCT_BODY()

	void* Entry;
};

/** Generic storage system */
USTRUCT()
struct FFlareBundle
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, float> FloatValues;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, int32> Int32Values;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, FTransform> TransformValues;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, FVectorArray> VectorArrayValues;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, FNameArray> NameArrayValues;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, FName> NameValues;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, FString> StringValues;

	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> Tags;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, FPtr> PtrValues;

	bool HasFloat(FName Key) const;
	bool HasInt32(FName Key) const;
	bool HasTransform(FName Key) const;
	bool HasVectorArray(FName Key) const;
	bool HasName(FName Key) const;
	bool HasNameArray(FName Key) const;
	bool HasString(FName Key) const;
	bool HasTag(FName Tag) const;
	bool HasPtr(FName Tag) const;


	float GetFloat(FName Key, float Default = 0.f) const;
	int32 GetInt32(FName Key, int32 Default = 0) const;
	FTransform GetTransform(FName Key, const FTransform Default = FTransform::Identity) const;
	TArray<FVector> GetVectorArray(FName Key) const;
	FName GetName(FName Key) const;
	TArray<FName> GetNameArray(FName Key) const;
	FString GetString(FName Key) const;
	void* GetPtr(FName Key) const;

	FFlareBundle& PutFloat(FName Key, float Value);
	FFlareBundle& PutInt32(FName Key, int32 Value);
	FFlareBundle& PutTransform(FName Key, const FTransform Value);
	FFlareBundle& PutVectorArray(FName Key, const TArray<FVector> Value);
	FFlareBundle& PutName(FName Key, FName Value);
	FFlareBundle& PutNameArray(FName Key, const TArray<FName> Value);
	FFlareBundle& PutString(FName Key, FString Value);
	FFlareBundle& PutTag(FName Tag);
	FFlareBundle& PutPtr(FName Key, void* Value);

	void Clear();
};

struct DamageCause
{
	DamageCause();
	DamageCause(EFlareDamage::Type DamageTypeParam);
	DamageCause(UFlareSimulatedSpacecraft* SpacecraftParam, EFlareDamage::Type DamageTypeParam);
	DamageCause(UFlareCompany* CompanyParam, EFlareDamage::Type DamageTypeParam);

	UFlareSimulatedSpacecraft* Spacecraft;
	UFlareCompany* Company;
	EFlareDamage::Type DamageType;
	bool ManualTurret = false;
};


/*----------------------------------------------------
	Helper class
----------------------------------------------------*/

UCLASS()
class HELIUMRAIN_API UFlareGameTypes : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/** Get the group name */
	static FText GetCombatGroupDescription(EFlareCombatGroup::Type Type);

	/** Get the tactic name */
	static FText GetCombatTacticDescription(EFlareCombatTactic::Type Type);

	/** Get the icon for this combat group */
	static const FSlateBrush* GetCombatGroupIcon(EFlareCombatGroup::Type Type);


};
