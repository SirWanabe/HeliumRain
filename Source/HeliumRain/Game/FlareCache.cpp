
#include "FlareCache.h"
#include "../Flare.h"
#include "FlareGame.h"

#define LOCTEXT_NAMESPACE "FlareCacheSystem"
#define DEBUG_SPACECRAFT_CACHE

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

//TODO: perhaps use TMap or multi-layered Array and more generic get/store for each type
//Key would probably be a string

UFlareCacheSystem::UFlareCacheSystem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}
void UFlareCacheSystem::InitialSetup(AFlareGame* GameMode)
{
	FCHECK(GameMode);
	Game = GameMode;

	CachedShells.Empty();
	CachedBombs.Empty();
	CachedAsteroids.Empty();
	CachedDebris.Empty();
	CachedSpacecraft.Empty();
}

void UFlareCacheSystem::StoreCachedSpacecraft(AFlareSpacecraft* Spacecraft)
{
	//TODO: Investigate what is still running on loaded/cached spacecraft which leads to progressively slower FPS during a game.
//	return;

	if (CachedSpacecraft.Contains(Spacecraft->GetParent()->GetDescription()->SpacecraftTemplate))
	{
		CachedSpacecraft[Spacecraft->GetParent()->GetDescription()->SpacecraftTemplate].TrackedSpacecraft.Add(Spacecraft);

#ifdef DEBUG_SPACECRAFT_CACHE
		FLOGV("UFlareCacheSystem::StoreCachedSpacecraft Store '%s' (template '%s') as already existing cache entry",
			*Spacecraft->GetParent()->GetImmatriculation().ToString(),
			*Spacecraft->GetParent()->GetDescription()->SpacecraftTemplate->GetName())
#endif

	}
	else
	{
		FFlareSpacecraftCacheHelper NewCacheHelper;
		NewCacheHelper.TrackedSpacecraft.Add(Spacecraft);
		CachedSpacecraft.Add(Spacecraft->GetParent()->GetDescription()->SpacecraftTemplate, NewCacheHelper);

#ifdef DEBUG_SPACECRAFT_CACHE
		FLOGV("UFlareCacheSystem::StoreCachedSpacecraft Store '%s' (template '%s') as NewCacheHelper",
			*Spacecraft->GetParent()->GetImmatriculation().ToString(),
			*Spacecraft->GetParent()->GetDescription()->SpacecraftTemplate->GetName())
#endif

	}
}

AFlareSpacecraft* UFlareCacheSystem::RetrieveCachedSpacecraft(UClass* SpacecraftType)
{
	//TODO:
//	return nullptr;

	if (CachedSpacecraft.Contains(SpacecraftType) && CachedSpacecraft[SpacecraftType].TrackedSpacecraft.Num() > 0)
	{

#ifdef DEBUG_SPACECRAFT_CACHE
		FLOGV("UFlareCacheSystem::RetrieveCachedSpacecraft Retrieve '%s' for 'new' ship",
			*SpacecraftType->GetName())
#endif

			return CachedSpacecraft[SpacecraftType].TrackedSpacecraft.Pop();
	}
	return nullptr;
}

void UFlareCacheSystem::StoreCachedShell(AFlareShell* Shell)
{
	if (Shell)
	{
		CachedShells.Add(Shell);
	}
}

AFlareShell* UFlareCacheSystem::RetrieveCachedShell()
{
	if (CachedShells.Num() > 0)
	{
		AFlareShell* Shell = CachedShells.Pop();
		return Shell;
	}
	return NULL;
}

void UFlareCacheSystem::StoreCachedBomb(AFlareBomb* Bomb)
{
	if (Bomb)
	{
		CachedBombs.Add(Bomb);
	}
}

AFlareBomb* UFlareCacheSystem::RetrieveCachedBomb()
{
	if (CachedBombs.Num() > 0)
	{
		AFlareBomb* Bomb = CachedBombs.Pop();
		return Bomb;
	}
	return NULL;
}

void UFlareCacheSystem::StoreCachedAsteroid(AFlareAsteroid* Asteroid)
{
	if (Asteroid)
	{
		CachedAsteroids.Add(Asteroid);
	}
}

AFlareAsteroid* UFlareCacheSystem::RetrieveCachedAsteroid()
{
	if (CachedAsteroids.Num() > 0)
	{
		AFlareAsteroid* Asteroid = CachedAsteroids.Pop();
		return Asteroid;
	}
	return NULL;
}

void UFlareCacheSystem::StoreCachedMeteorite(AFlareMeteorite* Meteorite)
{
	if (Meteorite)
	{
		CachedMeteorites.Add(Meteorite);
	}
}

AFlareMeteorite* UFlareCacheSystem::RetrieveCachedMeteorite()
{
	if (CachedMeteorites.Num() > 0)
	{
		AFlareMeteorite* Meteorite = CachedMeteorites.Pop();
		return Meteorite;
	}
	return NULL;
}

void UFlareCacheSystem::StoreCachedDebris(AStaticMeshActor* Debris)
{
	if (Debris)
	{
		CachedDebris.Add(Debris);
	}
}

AStaticMeshActor* UFlareCacheSystem::RetrieveCachedDebris()
{
	if (CachedDebris.Num() > 0)
	{
		AStaticMeshActor* Debris = CachedDebris.Pop();
		return Debris;
	}
	return NULL;
}

#undef LOCTEXT_NAMESPACE