#pragma once

#include "Game/FlareGameTypes.h"
#include "FlareQuest.h"
#include "FlareQuestManager.h"
#include "FlareQuestGenerator.generated.h"


class AFlareGame;
class UFlareQuestManager;
class UFlareQuest;
class UFlareSimulatedSector;
class UFlareQuestGenerated;

/** Quest generator */
UCLASS()
class HELIUMRAIN_API UFlareQuestGenerator: public UObject
{
	GENERATED_UCLASS_BODY()

public:

	virtual void Load(UFlareQuestManager* Parent, const FFlareQuestSave& Data);

	void LoadQuests(const FFlareQuestSave& Data);

	virtual void Save(FFlareQuestSave& Data);

	/*----------------------------------------------------
		Quest generation
	----------------------------------------------------*/

	static FText GeneratePersonName(TArray<FString> UsedNames);

	void GenerateIdentifer(FName QuestClass, FFlareBundle& Data);

	void GenerateSectorQuest(UFlareSimulatedSector* Sector);

	void GenerateMilitaryQuests();

	void GenerateMeteoriteQuest(UFlareSimulatedSpacecraft* TargetStation, float Power, int32 Count, int64 TimeBeforeImpact);

	void GenerateAttackQuests(UFlareCompany* AttackCompany, int32 AttackCombatPoints, WarTarget& Target, int64 TravelDuration);

	void RegisterQuest(UFlareQuestGenerated* Quest);

	bool FindUniqueTag(FName Tag);

	float ComputeQuestProbability(UFlareCompany* Company);

	FName GenerateVipTag(UFlareSimulatedSpacecraft* SourceSpacecraft);

	FName GenerateTradeTag(UFlareSimulatedSpacecraft* SourceSpacecraft, FFlareResourceDescription* Resource);


	FName GenerateDefenseTag(UFlareSimulatedSector* Sector, UFlareCompany* HostileCompany);

	FName GenerateAttackTag(UFlareSimulatedSector* Sector, UFlareCompany* OwnerCompany);

	FName GenerateHarassTag(UFlareCompany* OwnerCompany, UFlareCompany* HostileCompany);

	FName GenerateMeteoriteTag(UFlareSimulatedSector* Sector);

	bool IsGenerationEnabled();


protected:

   /*----------------------------------------------------
	   Protected data
   ----------------------------------------------------*/

	UFlareQuestManager*						QuestManager;

	AFlareGame*                             Game;

	int64                                   NextQuestIndex;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline AFlareGame* GetGame() const
	{
		return Game;
	}

	inline UFlareQuestManager* GetQuestManager() const
	{
		return QuestManager;
	}
protected:
	UPROPERTY()
	TArray<UFlareQuestGenerated*>	                 GeneratedQuests;
};


UCLASS()
class HELIUMRAIN_API UFlareQuestGenerated: public UFlareQuest
{
	GENERATED_UCLASS_BODY()

public:
	virtual bool Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);

	void AddGlobalFailCondition(UFlareQuestCondition* Condition);
	void SetupQuestGiver(UFlareCompany* Company, bool AddWarCondition);
	void SetupGenericReward(const FFlareBundle& Data);

	static void CreateGenericReward(FFlareBundle& Data, int64 QuestValue, UFlareCompany* Client);

	FFlareBundle* GetInitData() {
		return &InitData;
	}

	FName GetQuestClass() {
		return QuestClass;
	}

	void AcceptedWarContract(AFlareGame* Game, bool CheckQuestSector,bool MultipleHostiles);

protected:
	FName QuestClass;
	FFlareBundle InitData;
	UFlareQuestGenerator* QuestGenerator;
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedVipTransport: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "vip-transport"; }

	/** Load the quest from description file */
	virtual bool Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedResourceSale: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "resource-sale"; }

	virtual int32 GetReservedQuantity(UFlareSimulatedSpacecraft* Station, FFlareResourceDescription* Resource);

	/** Load the quest from description file */
	virtual bool Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedResourcePurchase: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "resource-purchase"; }

	virtual int32 GetReservedCapacity(UFlareSimulatedSpacecraft* Station, FFlareResourceDescription* Resource);

	/** Load the quest from description file */
	virtual bool Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company, TArray<FFlareResourceDescription*>& AvailableResources);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedResourceTrade: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "resource-trade"; }

	virtual int32 GetReservedCapacity(UFlareSimulatedSpacecraft* Station, FFlareResourceDescription* Resource);

	virtual int32 GetReservedQuantity(UFlareSimulatedSpacecraft* Station, FFlareResourceDescription* Resource);

	/** Load the quest from description file */
	virtual bool Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedStationDefense: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "station-defense"; }

	/** Load the quest from description file */
	virtual bool Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	virtual void Accept(AFlareGame* Game);
	virtual void QuestSuccess(AFlareGame* Game);
	virtual void QuestFailed(AFlareGame* Game);

	//static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company, UFlareCompany* HostileCompany);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedJoinAttack: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "join-attack"; }

	/** Load the quest from description file */
	virtual bool Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	virtual void Accept(AFlareGame* Game);
	virtual void QuestSuccess(AFlareGame* Game);
	virtual void QuestFailed(AFlareGame* Game);
	//static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareCompany* Company, int32 AttackCombatPoints, WarTarget& Target, int64 TravelDuration);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedSectorDefense: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "sector-defense"; }

	/** Load the quest from description file */
	virtual bool Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	virtual void Accept(AFlareGame* Game);
	virtual void QuestSuccess(AFlareGame* Game);
	virtual void QuestFailed(AFlareGame* Game);
	//static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareCompany* Company, UFlareCompany* HostileCompany, int32 AttackCombatPoints, WarTarget& Target, int64 TravelDuration);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedCargoHunt: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "cargo-hunt"; }

	/** Load the quest from description file */
	virtual bool Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	virtual void Accept(AFlareGame* Game);
	virtual void QuestSuccess(AFlareGame* Game);
	virtual void QuestFailed(AFlareGame* Game);
	//static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareCompany* Company, UFlareCompany* HostileCompany);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedMilitaryHunt: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "military-hunt"; }

	/** Load the quest from description file */
	virtual bool Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	virtual void Accept(AFlareGame* Game);
	virtual void QuestSuccess(AFlareGame* Game);
	virtual void QuestFailed(AFlareGame* Game);
	//static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareCompany* Company, UFlareCompany* HostileCompany);
};



//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedStationDefense2: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "station-defense-2"; }

	/** Load the quest from description file */
	virtual bool Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	virtual void Accept(AFlareGame* Game);
	virtual void QuestSuccess(AFlareGame* Game);
	virtual void QuestFailed(AFlareGame* Game);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareSimulatedSector* Sector, UFlareCompany* Company, UFlareCompany* HostileCompany);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedJoinAttack2: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "join-attack-2"; }

	/** Load the quest from description file */
	virtual bool Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	virtual void Accept(AFlareGame* Game);
	virtual void QuestSuccess(AFlareGame* Game);
	virtual void QuestFailed(AFlareGame* Game);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareCompany* Company, int32 AttackCombatPoints, WarTarget& Target, int64 TravelDuration);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedSectorDefense2: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "sector-defense-2"; }

	/** Load the quest from description file */
	virtual bool Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	virtual void Accept(AFlareGame* Game);
	virtual void QuestSuccess(AFlareGame* Game);
	virtual void QuestFailed(AFlareGame* Game);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareCompany* Company, UFlareCompany* HostileCompany, int32 AttackCombatPoints, WarTarget& Target, int64 TravelDuration);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedCargoHunt2: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "cargo-hunt-2"; }

	/** Load the quest from description file */
	virtual bool Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	virtual void Accept(AFlareGame* Game);
	virtual void QuestSuccess(AFlareGame* Game);
	virtual void QuestFailed(AFlareGame* Game);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareCompany* Company, UFlareCompany* HostileCompany);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedMilitaryHunt2: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "military-hunt-2"; }

	/** Load the quest from description file */
	virtual bool Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	virtual void Accept(AFlareGame* Game);
	virtual void QuestSuccess(AFlareGame* Game);
	virtual void QuestFailed(AFlareGame* Game);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareCompany* Company, UFlareCompany* HostileCompany);
};

//////////////////////////////////////////////////////
UCLASS()
class HELIUMRAIN_API UFlareQuestGeneratedMeteoriteInterception: public UFlareQuestGenerated
{
	GENERATED_UCLASS_BODY()

public:
	static FName GetClass() { return "meteorite-destruction"; }

	/** Load the quest from description file */
	virtual bool Load(UFlareQuestGenerator* Parent, const FFlareBundle& Data);
	static UFlareQuestGenerated* Create(UFlareQuestGenerator* Parent, UFlareSimulatedSpacecraft* Target, float Power, int32 Count, int64 TimeBeforeImpact);
};
