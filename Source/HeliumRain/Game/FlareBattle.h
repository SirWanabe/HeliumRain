#pragma once

#include "Object.h"
#include "FlareSimulatedSector.h"
#include "FlareBattle.generated.h"

class UFlareSpacecraftComponentsCatalog;


UCLASS()
class HELIUMRAIN_API UFlareBattle : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
	  Save
	----------------------------------------------------*/

	/** Load the battle state */
	virtual void Load(UFlareSimulatedSector* BattleSector);

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	void Simulate();

	bool SimulateTurn();

	bool SimulateShipTurn(UFlareSimulatedSpacecraft* Ship);

	bool SimulateSmallShipTurn(UFlareSimulatedSpacecraft* Ship);

	bool SimulateLargeShipTurn(UFlareSimulatedSpacecraft* Ship);

	bool SimulateShipAttack(UFlareSimulatedSpacecraft* Ship, int32 WeaponGroupIndex, UFlareSimulatedSpacecraft* ShipTarget, FFlareMeteoriteSave* MeteoriteTarget);

	bool SimulateShipWeaponAttack(UFlareSimulatedSpacecraft* Ship, FFlareSpacecraftComponentDescription* WeaponDescription, FFlareSpacecraftComponentSave* Weapon, UFlareSimulatedSpacecraft* ShipTarget, FFlareMeteoriteSave* MeteoriteTarget);

	void SimulateBulletDamage(FFlareSpacecraftComponentDescription* WeaponDescription, UFlareSimulatedSpacecraft* ShipTarget, FFlareMeteoriteSave* MeteoriteTarget, UFlareSimulatedSpacecraft* DamageSource);

	void SimulateBombDamage(FFlareSpacecraftComponentDescription* WeaponDescription, UFlareSimulatedSpacecraft* Target, UFlareSimulatedSpacecraft* DamageSource);

	void ApplyDamage(UFlareSimulatedSpacecraft* Target, float Energy, EFlareDamage::Type DamageType, UFlareSimulatedSpacecraft* DamageSource);
	void ApplyMeteoriteDamage(FFlareMeteoriteSave* MeteoriteTarget, float Energy, UFlareSimulatedSpacecraft* DamageSource);
	
	UFlareSimulatedSpacecraft* GetBestTarget(UFlareSimulatedSpacecraft* Ship, struct BattleTargetPreferences Preferences);
	FFlareMeteoriteSave* GetMeteoriteTarget();
	int32 GetBestTargetComponent(UFlareSimulatedSpacecraft* TargetSpacecraft);

	void FindFightingCompanies();

protected:
	UFlareSimulatedSector*                  Sector;
	AFlareGame*                             Game;
	UFlareCompany*                          PlayerCompany;
	UFlareSpacecraftComponentsCatalog*      Catalog;
	TArray<UFlareCompany*>					FightingCompanies;
	TArray<UFlareSimulatedSpacecraft*>		InitialSectorViableFightingShips;
	TArray<UFlareSimulatedSpacecraft*>		CurrentSectorViableFightingShips;
	TArray<FFlareMeteoriteSave*>			LocalMeteorites;

	int										CurrentBattleTurn;
	int										MaximumTurns;
	bool									SwitchedToFightingMeteorites;
	bool									PlayerAssetsFighting;
	bool									FoundFightingCompanies;
	bool									ShipDisabledPreviousTurn;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	AFlareGame* GetGame() const
	{
		return Game;
	}

        bool HasBattle();
};
