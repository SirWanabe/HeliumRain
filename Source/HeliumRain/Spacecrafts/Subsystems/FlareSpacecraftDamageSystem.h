#pragma once

#include "../FlareSpacecraftTypes.h"
#include "../FlareSimulatedSpacecraft.h"
#include "FlareSimulatedSpacecraftDamageSystem.h"
#include "FlareSpacecraftDamageSystem.generated.h"


class AFlareSpacecraft;
class UFlareSimulatedSpacecraftDamageSystem;
struct FFlareSpacecraftSave;
struct FFlareSpacecraftDescription;


/** Spacecraft damage system class */
UCLASS()
class HELIUMRAIN_API UFlareSpacecraftDamageSystem : public UObject
{

public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void TickSystem(float DeltaSeconds);

	/** Initialize this system */
	virtual void Initialize(AFlareSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData);

	virtual void Start();

	/** Set who's doing the damage */
	virtual void SetLastDamageCause(DamageCause Cause);


public:

	/*----------------------------------------------------
		System Interface
	----------------------------------------------------*/


	virtual float GetWeaponGroupHealth(int32 GroupIndex, bool WithAmmo = true) const;

	virtual float GetOverheatRatio(float HalfRatio) const;

	/*----------------------------------------------------
		System Interface
	----------------------------------------------------*/

	/** Update power status for all components */
	virtual void UpdatePower();

	virtual void CalculateInitialHeat();

	/** Method call if a electric component had been damaged */
	virtual void OnElectricDamage(float DamageRatio);

	virtual void OnCollision(class AActor* Other, FVector HitLocation, FVector NormalImpulse);

	virtual void ApplyDamage(float Energy, float Radius, FVector Location, EFlareDamage::Type DamageType, UFlareSimulatedSpacecraft* DamageSource, FString DamageCauser);

	virtual void NotifyHeatProductionChange(float HeatProductionChange);
	virtual void NotifyHeatSinkChange(float HeatSinkChange);

	void UpdateCurrentHeatValue(float DeltaSeconds);

protected:

	/** The ship is uncontrollable */
	virtual void OnControlLost();


	/** The ship was destroyed */
	void OnSpacecraftDestroyed(bool SuppressMessages=false,bool ForceExplosion = false);

	virtual void CheckRecovery();



	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	AFlareSpacecraft*                               Spacecraft;

	FFlareSpacecraftSave*                           Data;
	FFlareSpacecraftDescription*                    Description;
	UFlareSimulatedSpacecraftDamageSystem*          Parent;
	TArray<UActorComponent*>                        Components;

	bool                                            WasControllable; // True if was controllable at the last tick
	bool                                            WasAlive;
	bool											WasDisarmed;
	float											TimeSinceLastExternalDamage;

	DamageCause                                     LastDamageCause;
	AFlarePlayerController*							PC;

	float											TotalHeatProduction;
	float											TotalHeatSink;
	float											TotalHeatAfterSun;
	bool											HeatChange;

public:
	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/
	virtual float GetTimeSinceLastExternalDamage() const
	{
		return TimeSinceLastExternalDamage;
	}

	/** Force the ship to die*/
	void SetDead();
};
