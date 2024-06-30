#pragma once

#include "FlareSpacecraftComponent.h"
#include "../Game/FlareCache.h"
#include "FlareWeapon.generated.h"


class AFlareShell;
class AFlareBomb;
struct FFlareWeaponGroup;

UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareWeapon : public UFlareSpacecraftComponent
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	void Initialize(FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerShip, bool IsInMenu, AFlareSpacecraft* ActualOwnerShip) override;

	/** Setup this weapon's effects */
	virtual void SetupFiringEffects();

	virtual FFlareSpacecraftComponentSave* Save() override;

//	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	void TickForComponent(float DeltaTime) override;
	void TickForComponentAlive(float DeltaTime) override;

	virtual void SetVisibleInUpgrade(bool Visible) override;

	virtual void UpdateCustomization() override;

	/** Start firing */
	virtual void StartFire();

	/** Stop firing */
	virtual void StopFire();

	/** Is this weapon ready to fire a shell ?*/
	virtual bool IsReadyToFire() const;

	/** Return the current amount of heat production in KW */
	virtual float GetHeatProduction() const override;

	virtual void OnRefilled();

	virtual FVector GetFireAxis() const;

	virtual FVector GetMuzzleLocation(int GunIndex) const;

	virtual int GetGunCount() const;

	virtual bool IsTurret() const;

	virtual bool IsSafeToFire(int GunIndex, AActor*& HitTarget);

	virtual bool Trace(const FVector& Start, const FVector& End, FHitResult& HitOut) const;

	FTraceHandle RequestTrace(const FVector& Start, const FVector& End);
	void OnTraceCompleted(const FTraceHandle& Handle, FTraceDatum& Data);
	void DoWorkWithTraceResults(const FTraceDatum& TraceData);

	/** Return the aim need minimum radius. 0 if not proximity fuze */
	virtual float GetAimRadius() const;

	virtual bool FireGun(int GunIndex);

	/** Show the special effects on firing */
	virtual void ShowFiringEffects(int GunIndex);

	virtual bool FireBomb();

	virtual void ConfigureShellFuze(AFlareShell* Shell);

	/** Set the target data */
	virtual void SetTarget(FVector TargetLocation, FVector TargetVelocity);

	FVector ComputeParallaxCorrection(int GunIndex);

	virtual void OnAttachmentChanged() override;

	virtual void BeginDestroy() override;

	virtual void FillBombs();

	virtual void RemoveBomb(AFlareBomb* Removing);

	virtual void ClearBombs();

	virtual FText GetSlotName() const;

	inline void SetWeaponGroup(FFlareWeaponGroup* Group)
	{
		WeaponGroup = Group;
	}

	virtual bool IsDestroyedEffectRelevant() override;

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	FTraceHandle LastTraceHandle;
	FTraceDelegate TraceDelegate;

	/** Firing sound */
	UPROPERTY()
	USoundCue*                  FiringSound;

	/** Empty sound */
	UPROPERTY()
	USoundCue*                  EmptySound;

	/** Bomb sound */
	UPROPERTY()
	USoundCue*                  BombDroppedSound;

	/** Special effects on firing (template) */
	UPROPERTY()
	UParticleSystem*            FiringEffectTemplate;

	/** Special effects on firing (component) */
	UPROPERTY()
	UParticleSystemComponent*   FiringEffect;

	// Target info
	bool                        TargetDefined;
	FVector                     TargetLocation;
	FVector                     TargetVelocity;

	// Weapon properties
	float                       FiringRate;
	float                       FiringPeriod;
	float                       AmmoVelocity;
	FActorSpawnParameters       ProjectileSpawnParams;

	TArray<AFlareBomb*>         Bombs;

	// State
	bool                        Firing;
	float                       TimeSinceLastShell;
	int                         LastFiredGun;
	FFlareWeaponGroup*          WeaponGroup;

	bool						PreviousTraceIsSafe;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline int32 GetCurrentAmmo() const
	{
		return GetMaxAmmo() - ShipComponentData->Weapon.FiredAmmo;
	}

	inline TArray<AFlareBomb*> GetBombs() const
	{
		return Bombs;
	}

	inline bool GetPreviousTraceIsSafe() const
	{
		return PreviousTraceIsSafe;
	}

	inline int32 GetMaxAmmo() const
	{
		return ComponentDescription->WeaponCharacteristics.AmmoCapacity;
	}

	inline FFlareSpacecraftComponentDescription* GetComponentDescription() const
	{
		return ComponentDescription;
	}

	inline UParticleSystem* GetFiringEffectTemplate() const
	{
		return FiringEffectTemplate;
	}

	inline float GetAmmoVelocity() const
	{
		return AmmoVelocity;
	}

	inline bool isFiring() const
	{
		return Firing && GetCurrentAmmo() > 0;
	}

	virtual UStaticMesh* GetMesh(bool PresentationMode) const;

	inline FFlareWeaponGroup* GetWeaponGroup()
	{
		return WeaponGroup;
	}

};
