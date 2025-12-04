#pragma once

#include "Object.h"
#include "../Spacecrafts/FlareSpacecraft.h"
#include "../Spacecrafts/FlareBomb.h"
#include "FlareAsteroid.h"
#include "../Quests/FlareMeteorite.h"
#include "FlareSimulatedSector.h"
#include "FlareSector.generated.h"

class UFlareSimulatedSector;
class AFlareGame;
class AFlareAsteroid;

UCLASS()
class HELIUMRAIN_API UFlareSector : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
	  Save
	----------------------------------------------------*/

	/** Load the sector from a save file */
	virtual void Load(UFlareSimulatedSector* Parent);

	/** Save the sector to a save file */
	virtual void Save();

	virtual void Tick(float DeltaSeconds);

	/** Destroy the sector */
	virtual void DestroySector();

	void ShipLeftSector(AFlareSpacecraft* Spacecraft);

	/** Remove spacecraft from sector */
	virtual void RemoveSpacecraft(AFlareSpacecraft* Spacecraft, bool RemoveSectorSpacecrafts = true);
	virtual void SpaceCraftExplosionCheck(AFlareSpacecraft* Spacecraft);
	
	/** Add dead spacecraft to our dead array*/
	virtual void AddDestroyedSpacecraft(AFlareSpacecraft* Spacecraft, bool ForceExplosion = false);

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	AFlareAsteroid* LoadAsteroid(const FFlareAsteroidSave& AsteroidData);

	AFlareMeteorite* LoadMeteorite(FFlareMeteoriteSave& MeteoriteData);
	void RemoveSectorMeteorite(AFlareMeteorite* Meteorite);

	AFlareSpacecraft* LoadSpacecraft(UFlareSimulatedSpacecraft* ParentSpacecraft, bool Reposition = false);

	void SetSpacecraftSpawnPosition(UFlareSimulatedSpacecraft* ParentSpacecraft, AFlareSpacecraft* Spacecraft);

	void FinishLoadSpacecraftReattachRedock(AFlareSpacecraft* Spacecraft);
	void FinishLoadSpacecraft(AFlareSpacecraft* Spacecraft,bool Reposition);

	AFlareBomb* LoadBomb(const FFlareBombSave& BombData);

	void RegisterBomb(AFlareBomb* Bomb);

	void UnregisterBomb(AFlareBomb* Bomb);

	void RegisterShell(AFlareShell* Shell);

	void UnregisterShell(AFlareShell* Shell);

	virtual void SetPause(bool Pause);

	AActor* GetNearestBody(FVector Location, float* NearestDistance, bool IncludeSize = true, AActor* ActorToIgnore = NULL);

	void PlaceSpacecraft(AFlareSpacecraft* Spacecraft, FVector Location, FRotator Rotation, float RandomLocationRadiusIncrement = 100000, bool RandomLocRadiusBoost = true, float InitialLocationRadius = 100000, bool MultiplyLocationOrAdd = true, bool PositiveOrNegative = FMath::RandBool());
	void PlaceSpacecraftDrone(AFlareSpacecraft* Spacecraft, FVector Location, FRotator Rotation, float RandomLocationRadiusIncrement = 100000, float InitialLocationRadius = 100000, bool PositiveOrNegative = FMath::RandBool());

	void AddReinforcingShip(UFlareSimulatedSpacecraft* Ship);

	void SimulateLocalCompanyAI();
	void UpdateSectorBattleStates();

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UFlareSimulatedSector*         ParentSector;

	UPROPERTY()
	TArray<AFlareSpacecraft*>      SectorStations;

	UPROPERTY()
	TArray<AFlareSpacecraft*>      SectorShips;

	UPROPERTY()
	TArray<AFlareSpacecraft*>      SectorSpacecrafts;
	
	UPROPERTY()
	TArray<AFlareAsteroid*>        SectorAsteroids;

	UPROPERTY()
	TArray<AFlareMeteorite*>       SectorMeteorites;

	UPROPERTY()
	TArray<AFlareBomb*>            SectorBombs;
	UPROPERTY()
	TArray<AFlareShell*>           SectorShells;

	int64						   LocalTime;
	bool						   SectorRepartitionCache;
	bool                           IsDestroyingSector;
	FVector                        SectorCenter;
	float                          SectorRadius;

	TArray<UFlareCompany*> UniqueCompanies;
	TMap<UFlareCompany*, TArray<AFlareSpacecraft*>> CompanyShipsPerCompanyCache;
	TMap<UFlareCompany*, TArray<AFlareSpacecraft*>> CompanySpacecraftsPerCompanyCache;
	TMap<FName, AFlareSpacecraft*> SectorSpacecraftsCache;
public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	bool SignalLocalSectorUpdateSectorBattleStates;
	bool IsPaused = false;

	inline TArray<AFlareBomb*> GetSectorBombs() const
	{
		return SectorBombs;
	}

	inline TArray<AFlareShell*> GetSectorShells() const
	{
		return SectorShells;
	}

	inline AFlareGame* GetGame()
	{
		return ParentSector->GetGame();
	}

	UFlareSimulatedSector* GetSimulatedSector()
	{
		return ParentSector;
	}

	TArray<AFlareSpacecraft*> GetCompanyShips(UFlareCompany* Company);

	TArray<AFlareSpacecraft*> GetCompanySpacecrafts(UFlareCompany* Company);

	AFlareSpacecraft* FindSpacecraft(FName Immatriculation);

	inline const FFlareSectorDescription* GetDescription() const
	{
		return ParentSector->GetDescription();
	}
	
	inline TArray<AFlareSpacecraft*>& GetSpacecrafts()
	{
		return SectorSpacecrafts;
	}

	bool GetIsDestroyingSector();

	inline TArray<AFlareSpacecraft*>& GetStations()
	{
		return SectorStations;
	}

	inline TArray<AFlareSpacecraft*>& GetShips()
	{
		return SectorShips;
	}

	inline TArray<AFlareAsteroid*>& GetAsteroids()
	{
		return SectorAsteroids;
	}

	inline TArray<AFlareMeteorite*>& GetMeteorites()
	{
		return SectorMeteorites;
	}

	inline TArray<AFlareBomb*>& GetBombs()
	{
		return SectorBombs;
	}

	inline int64 GetLocalTime()
	{
		return LocalTime;
	}

	void GenerateSectorRepartitionCache();

	FVector GetSectorCenter();

	float GetSectorRadius();
/*
	static float GetSectorLimits()
	{
		return 1500000; // 15 km
	}
*/
	float GetSectorLimits();
};
