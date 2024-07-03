#pragma once

#include "../Flare.h"
#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "FlareMeteorite.generated.h"

class UFlareSimulatedSpacecraft;
class UFlareSector;
class AFlareSpacecraft;


UCLASS(Blueprintable, ClassGroup = (Flare, Ship), meta = (BlueprintSpawnableComponent))
class AFlareMeteorite : public AActor
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void BeginPlay() override;

	void TickMeteorite(float DeltaSeconds);

	/** Properties setup */
	virtual void Load(FFlareMeteoriteSave* Data, UFlareSector* ParentSector);

	/** Save the asteroid to a save file */
	virtual FFlareMeteoriteSave* Save();

	/** Set as paused */
	virtual void SetPause(bool Paused);

	/** Setup the meteorite mesh */
	void SetupMeteoriteMesh();

	void ApplyDamage(float Energy, float Radius, FVector Location, EFlareDamage::Type DamageType, UFlareSimulatedSpacecraft* DamageSource, FString DamageCauser);

	virtual void OnCollision(class AActor* Other, FVector HitLocation);

public:

   /*----------------------------------------------------
	   Protected data
   ----------------------------------------------------*/

	// Mesh
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly)
	class UStaticMeshComponent*                  Meteorite;

	void UnSafeDestroy();
	void SafeDestroy();

protected:
	
	// Core breakup effect
	UPROPERTY()
	UStaticMesh*                                  MeteoriteCoreMesh;

	// Rock breakup effect
	UPROPERTY()
	UParticleSystem*                              RockEffectTemplate;

	// Metal breakup effect
	UPROPERTY()
	UParticleSystem*                              MetalEffectTemplate;

	// Data
	FFlareMeteoriteSave*                          MeteoriteData;
	bool                                          Paused;
	bool										  IsSafeDestroyingRunning;
	UFlareSector*                                 Parent;
	AFlareSpacecraft*                             Target;

	float                                         LifeTime;


public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	bool IsBroken();

	bool HasMissed()
	{
		return MeteoriteData->HasMissed;
	}

	UStaticMeshComponent* GetMeteoriteComponent()
	{
		return Meteorite;
	}

};

