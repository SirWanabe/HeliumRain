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

	/** Destroy the sector */
	virtual void DestroySector();

	/** Remove spacecraft from sector */
	virtual void RemoveSpacecraft(AFlareSpacecraft* Spacecraft, bool RemoveSectorSpacecrafts = true);
	virtual void SpaceCraftExplosionCheck(AFlareSpacecraft* Spacecraft);
	
	/** Add dead spacecraft to our dead array*/
	virtual void AddDestroyedSpacecraft(AFlareSpacecraft* Spacecraft);

	/*----------------------------------------------------
		Gameplay
	----------------------------------------------------*/

	AFlareAsteroid* LoadAsteroid(const FFlareAsteroidSave& AsteroidData);

	AFlareMeteorite* LoadMeteorite(FFlareMeteoriteSave& MeteoriteData);

	AFlareSpacecraft* LoadSpacecraft(UFlareSimulatedSpacecraft* ParentSpacecraft);

	AFlareBomb* LoadBomb(const FFlareBombSave& BombData);

	void RegisterBomb(AFlareBomb* Bomb);

	void UnregisterBomb(AFlareBomb* Bomb);

	void RegisterShell(AFlareShell* Shell);

	void UnregisterShell(AFlareShell* Shell);

	virtual void SetPause(bool Pause);

	AActor* GetNearestBody(FVector Location, float* NearestDistance, bool IncludeSize = true, AActor* ActorToIgnore = NULL);

	void PlaceSpacecraft(AFlareSpacecraft* Spacecraft, FVector Location, float RandomLocationRadiusIncrement = 100000, bool RandomLocRadiusBoost = true, float InitialLocationRadius = 100000, bool MultiplyLocationOrAdd = true);

	void AddReinforcingShip(UFlareSimulatedSpacecraft* Ship);

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
//	float						   ShellTick;

	TMap<UFlareCompany*, TArray<AFlareSpacecraft*>> CompanyShipsPerCompanyCache;
	TMap<UFlareCompany*, TArray<AFlareSpacecraft*>> CompanySpacecraftsPerCompanyCache;
	TMap<FName, AFlareSpacecraft*> SectorSpacecraftsCache;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

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


	/*inline FName GetIdentifier() const
	{
		return SectorData.Identifier;
	}*/

	inline TArray<AFlareSpacecraft*>& GetSpacecrafts()
	{
		return SectorSpacecrafts;
	}

	/*inline FFlarePeopleSave* GetPeople(){
		return &SectorData.PeopleData;
	}*/

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

	static float GetSectorLimits()
	{
		return 1500000; // 15 km
	}

};
