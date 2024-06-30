#pragma once

#include "FlareSpacecraftComponent.h"
#include "FlareWeapon.h"
#include "FlareTurretPilot.h"
#include "FlareTurret.generated.h"

class UFlareSpacecraftSubComponent;

UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class UFlareTurret : public UFlareWeapon
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	void Initialize(FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerShip, bool IsInMenu,AFlareSpacecraft* ActualOwnerShip) override;

	virtual void SetupFiringEffects() override;

	void TickForComponent(float DeltaTime) override;
	void TickForComponentAlive(float DeltaTime) override;

	void SetupComponentMesh() override;

	virtual FVector GetFireAxis() const;

	virtual FVector GetIdleAxis() const;

	virtual FVector GetMuzzleLocation(int GunIndex) const;

	virtual FVector GetTurretBaseLocation() const;

	/** Are we close to the target ? */
	virtual bool IsCloseToAim() const;

	virtual bool IsReacheableAxis(FVector TargetAxis);// const;

	virtual float GetMinLimitAtAngle(float Angle);// const;

	virtual void GetBoundingSphere(FVector& Location, float& Radius) override;

	virtual void ShowFiringEffects(int GunIndex) override;


protected:
	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** Mesh component */
	UPROPERTY()
	UFlareSpacecraftSubComponent*                    TurretComponent;
	UPROPERTY()
	UFlareSpacecraftSubComponent*                    BarrelComponent;

	// Pilot object
	UPROPERTY()
	UFlareTurretPilot*                               Pilot;

	TArray<UParticleSystemComponent*>                FiringEffects;

	// General data
	FVector  								         AimDirection;
	bool											 IsIgnoreManualAimCached;
	bool											 IsIgnoreManualAimCache;
	TMap<float, float>								 BarrelsMinAngleCache;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline UFlareTurretPilot* GetTurretPilot()
	{
		return Pilot;
	}

	bool IsIgnoreManualAim();// const;

};
