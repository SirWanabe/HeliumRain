#pragma once

#include "Object.h"
#include "FlareCache.generated.h"

class AFlareGame;
class UFlareSimulatedSector;
class AFlareShell;
class AFlareBomb;
class AFlareAsteroid;
class AFlareMeteorite;

USTRUCT()
struct FFlareSpacecraftCacheHelper
{
	GENERATED_USTRUCT_BODY()

	TArray<AFlareSpacecraft*> TrackedSpacecraft;
};

UCLASS()
class HELIUMRAIN_API UFlareCacheSystem : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public interface
	----------------------------------------------------*/
	void InitialSetup(AFlareGame* GameMode);

	void StoreCachedSpacecraft(AFlareSpacecraft* Spacecraft);
	AFlareSpacecraft* RetrieveCachedSpacecraft(UClass* SpacecraftType);

	void StoreCachedShell(AFlareShell* Shell);
	AFlareShell* RetrieveCachedShell();

	void StoreCachedBomb(AFlareBomb* Bomb);
	AFlareBomb* RetrieveCachedBomb();

	void StoreCachedAsteroid(AFlareAsteroid* Asteroid);
	AFlareAsteroid* RetrieveCachedAsteroid();

	void StoreCachedMeteorite(AFlareMeteorite* Meteorite);
	AFlareMeteorite* RetrieveCachedMeteorite();

	void StoreCachedDebris(AStaticMeshActor* Debris);
	AStaticMeshActor* RetrieveCachedDebris();


private:

	/*----------------------------------------------------
		Internals
	----------------------------------------------------*/

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	/** Game reference */
	UPROPERTY()
	AFlareGame*                                Game;

	UPROPERTY()
	TArray<AFlareShell*>					   CachedShells;

	UPROPERTY()
	TArray<AFlareBomb*>						   CachedBombs;

	UPROPERTY()
	TArray<AFlareAsteroid*>					   CachedAsteroids;

	UPROPERTY()
	TArray<AFlareMeteorite*>				   CachedMeteorites;

	UPROPERTY()
	TArray<AStaticMeshActor*>				   CachedDebris;

	UPROPERTY()
	TMap<UClass*, FFlareSpacecraftCacheHelper> CachedSpacecraft;
};