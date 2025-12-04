#pragma once


#include "../Spacecrafts/FlareSpacecraft.h"
#include "FlareGameTypes.h"
#include "FlareCompany.h"
#include "FlareSector.h"
#include "Log/FlareLogApi.h"

#include "GameFramework/GameMode.h"
#include "FlareGame.generated.h"

#define DEBUG_AI_TRADING_STATS 0
#define NOTIFY_DEFAULT_TIMER 10.f

class UFlareSpacecraftCatalog;
class UFlareSpacecraftComponentsCatalog;
class UFlareCustomizationCatalog;
class UFlareMeteoriteCatalog;
class UFlareAsteroidCatalog;
class UFlareCompanyCatalog;
class UFlareResourceCatalog;
class UFlareTechnologyCatalog;
class UFlareScannableCatalog;
class UFlareGlobalSettings;
class UFlareStartingScenarioCatalog;
class UFlareOrbitalMap;

class UFlareSkirmishManager;
class UFlarePlanetarium;
class UFlareSector;
class UFlareSaveGame;
class UFlareSaveGameSystem;
class UFlareQuestManager;
class UFlareQuestCatalog;
class UFlareDebrisField;
class UFlareCacheSystem;
class UFlareSectorCatalogEntry;
class UFlareScenarioTools;
class AFlareMenuManager;
struct FFlarePlayerSave;


USTRUCT()
struct FFlareSaveSlotInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY() UFlareSaveGame*            Save;
	UPROPERTY() UMaterialInstanceDynamic*  Emblem;

	FSlateBrush                EmblemBrush;

	int32                      CompanyShipCount;
	int32                      CompanyStationCount;
	int32                      DifficultyID;
	int64                      CompanyValue;
	FText                      CompanyName;
	FName                      UUID;
};


UCLASS()
class HELIUMRAIN_API AFlareGame : public AGameMode
{
public:
	
	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void StartPlay() override;

	virtual void PostLogin(APlayerController* Player) override;

	virtual void Logout(AController* Player) override;

	void ReactivateSector();

	virtual void ActivateSector(UFlareSimulatedSector* Sector);

	virtual void ActivateCurrentSector();

	virtual UFlareSimulatedSector* DeactivateSector();

	virtual void Recovery();
	UFlareSimulatedSector* GetRecoverySector();

	virtual void SetWorldPause(bool Pause);

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void SlowerTickFunction();

	virtual void Scrap(FName ShipImmatriculation, FName TargetStationImmatriculation);

	virtual void ScrapStation(UFlareSimulatedSpacecraft* Station);

	/*----------------------------------------------------
		Save slots
	----------------------------------------------------*/

	/** Load metadata for all save slots */
	void ReadAllSaveSlots();

	/** Get the number of save slots */
	int32 GetSaveSlotCount() const;

	/** Get the current save slot number */
	int32 GetCurrentSaveSlot() const;

	/** Set the current slot to use for loading or creating a game */
	void SetCurrentSlot(int32 Index);

	/** Is this an existing save slot ? */
	bool DoesSaveSlotExist(int32 Index) const;

	/** Get metadata for this save slot */
	const FFlareSaveSlotInfo& GetSaveSlotInfo(int32 Index);

	/** Get the file name for a game save */
	FString GetSaveFileName(int32 Index) const;

	/** Load a game save */
	UFlareSaveGame* ReadSaveSlot(int32 Index);

	/** Remove a game save */
	bool DeleteSaveSlot(int32 Index);


	/*----------------------------------------------------
		Save
	----------------------------------------------------*/

	/** Create a new sandbox game */
	virtual void CreateGame(FFlareCompanyDescription CompanyData, int32 ScenarioIndex, int32 DifficultyIndex, int32 EconomyIndex, int32 PlayerEmblemIndex, bool PlayTutorial, bool PlayStory, bool RandomizeStationLocations, bool AICheats);

	/** Create a new skirmish game */
	void CreateSkirmishGame(UFlareSkirmishManager* Skirmish);

	/** Create a spacecraft based on a skirmish order */
	UFlareSimulatedSpacecraft* CreateSkirmishSpacecraft(UFlareSimulatedSector* Sector, UFlareCompany* Company, FFlareSkirmishSpacecraftOrder Order, FVector TargetPosition);

	/** Create a company */
	UFlareCompany* CreateCompany(int32 CatalogIdentifier);

    /** Load the game from this save file */
    virtual bool LoadGame(AFlarePlayerController* PC);

	/** Save the world to this save file */
	virtual bool SaveGame(AFlarePlayerController* PC, bool Async, bool Force = false);

	/** Unload the game*/
	virtual void UnloadGame();
	
	virtual void Clean();
	
	/*----------------------------------------------------
		Level streaming
	----------------------------------------------------*/

	/** Load a streaming level */
	bool LoadStreamingLevel(FName SectorLevel);

	/** Unload a streaming level */
	void UnloadStreamingLevel(FName SectorLevel);

	/** Callback for a loaded streaming level */
	UFUNCTION(BlueprintCallable, Category = GameMode)
	void OnLevelLoaded();

	/** Callback for an unloaded streaming level */
	UFUNCTION(BlueprintCallable, Category = GameMode)
	void OnLevelUnLoaded();


	/*----------------------------------------------------
		Immatriculations
	----------------------------------------------------*/

	/** Build a unique immatriculation string for this object */
	void Immatriculate(UFlareCompany* Company, FName TargetClass, FFlareSpacecraftSave* SpacecraftSave, bool IsChildStation);

	FName GenerateIdentifier(FName BaseName);

	/** Fill the database with station or capital ship names */
	void InitSpacecraftNameDatabase();

	/** Convert a number to roman */
	static FString ConvertToRoman(uint32 Val);

	/** Get a spacecraft name */
	FText PickSpacecraftName(UFlareCompany* Owner, bool IsStation, FString BaseSuffix);

	TArray<FString> GetModStrings() const;
	TArray<FString>							ActiveModStrings;

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/
	
	FTimerHandle							   SlowTick;

	/** Planetary system */
	UPROPERTY()
	AFlarePlanetarium*                         Planetarium;

    /** World */
    UPROPERTY()
    UFlareWorld*                               World;

	/** Quest manager*/
	UPROPERTY()
	UFlareQuestManager*                        QuestManager;

	/** Skirmish manager*/
	UPROPERTY()
	UFlareSkirmishManager*                     SkirmishManager;

	/** Active sector */
	UPROPERTY()
	UFlareSector*                              ActiveSector;

	/** Activating sector */
	UPROPERTY()
	UFlareSimulatedSector*                     ActivatingSector;

	/** Debris field */
	UPROPERTY()
	UFlareDebrisField*                         DebrisFieldSystem;

	UPROPERTY()
	UFlareCacheSystem*						   CacheSystem;


	

	/** Player controller */
	UPROPERTY()
	AFlarePlayerController*			           PlayerController;

	/** Save game system*/
	UPROPERTY()
	UFlareSaveGameSystem*                      SaveGameSystem;

	/** Scenario tools */
	UPROPERTY()
	UFlareScenarioTools*                       ScenarioTools;
	
	// Post process volume
	UPROPERTY()
	APostProcessVolume*                        PostProcessVolume;

	FFlarePlayerSave*                          PlayerData;

	/*----------------------------------------------------
		Catalogs
	----------------------------------------------------*/
	
	/** Reference to all available ship models */
	UPROPERTY()
	UFlareSpacecraftCatalog*                   SpacecraftCatalog;

	/** Reference to all available ship parts */
	UPROPERTY()
	UFlareSpacecraftComponentsCatalog*         ShipPartsCatalog;

	/** Reference to colors and patterns */
	UPROPERTY()
	UFlareCustomizationCatalog*                CustomizationCatalog;

	/** Reference to meteorite data */
	UPROPERTY()
	UFlareMeteoriteCatalog*                    MeteoriteCatalog;

	/** Reference to asteroid data */
	UPROPERTY()
	UFlareAsteroidCatalog*                     AsteroidCatalog;

	/** Reference to company data */
	UPROPERTY()
	UFlareCompanyCatalog*                      CompanyCatalog;

	/** Reference to the orbital setup */
	UPROPERTY()
	UFlareOrbitalMap*                          OrbitalBodies;

	/** Sector description list */
	UPROPERTY()
	TArray<UFlareSectorCatalogEntry*>          SectorList;

	/** Reference to all quests */
	UPROPERTY()
	UFlareQuestCatalog*                        QuestCatalog;

	/** Reference to all quests */
	UPROPERTY()
	UFlareResourceCatalog*                     ResourceCatalog;

	/** Reference to all technologies */
	UPROPERTY()
	UFlareTechnologyCatalog*                   TechnologyCatalog;

	/** Reference to all scannables */
	UPROPERTY()
	UFlareScannableCatalog*                    ScannableCatalog;

	/** Reference to the global modifiers */
	UPROPERTY()
	UFlareGlobalSettings*                    GlobalSettings;

	/** Reference to starting scenarios*/
	UPROPERTY()
	UFlareStartingScenarioCatalog*                StartingScenarioCatalog;

		/** Default asteroid */
	UPROPERTY()
	UStaticMesh*                               DefaultAsteroid;
	

	/*----------------------------------------------------
		Immatriculation and save data
	----------------------------------------------------*/

	int32                                      CurrentImmatriculationIndex;
	int32                                      CurrentIdentifierIndex;
	TArray<FText>                              CapitalShipNameList;
	TArray<FText>                              StationNameList;

	FName                                      DefaultWeaponIdentifier;
	FName                                      DefaultTurretIdentifier;

	int32                                      CurrentSaveIndex;
	bool                                       LoadedOrCreated;
	int32                                      SaveSlotCount;
	int32                                      CurrentStreamingLevelIndex;
	bool                                       IsLoadingStreamingLevel;

	bool									   IsLoggingOut;

	UPROPERTY()
	TArray<FFlareSaveSlotInfo>                 SaveSlots;

public:

	/*----------------------------------------------------
		Debug
	----------------------------------------------------*/
	bool                                       AutoSave;


public:
	
	/*----------------------------------------------------
		Get & Set
	----------------------------------------------------*/

	UFUNCTION(BlueprintCallable, Category = "Flare")
	UFlareWorld* GetGameWorld() const
	{
		return World;
	}

	UFUNCTION(BlueprintCallable, Category = "Flare")
	AFlarePlayerController* GetPC() const
	{
		return PlayerController;
	}

	UFUNCTION(BlueprintCallable, Category = "Flare")
	APostProcessVolume* GetPostProcessVolume() const
	{
		return PostProcessVolume;
	}

	UFUNCTION(BlueprintCallable, Category = "Flare")
	UFlareSector* GetActiveSector() const
	{
		return ActiveSector;
	}

	UFUNCTION(BlueprintCallable, Category = "Flare")
	AFlarePlanetarium* GetPlanetarium() const
	{
		return Planetarium;
	}

	UFUNCTION(BlueprintCallable, Category = "Flare")
	UFlareQuestManager* GetQuestManager() const
	{
		return QuestManager;
	}

	UFlareDebrisField* GetDebrisFieldSystem() const
	{
		return DebrisFieldSystem;
	}                        

	UFlareCacheSystem* GetCacheSystem() const
	{
		return CacheSystem;
	}

	const FFlareCompanyDescription* GetCompanyDescription(int32 Index) const;

	const FFlareCompanyDescription* GetPlayerCompanyDescription() const;
	
	const int32 GetCompanyCatalogCount() const;
	
	bool IsSkirmish() const;

	FFlareTechnologyDescription* GetTechnologyCatalogDescription(FName Identifier) const;

	inline UFlareSpacecraftCatalog* GetSpacecraftCatalog() const
	{
		return SpacecraftCatalog;
	}

	inline UFlareOrbitalMap* GetOrbitalBodies() const
	{
		return OrbitalBodies;
	}

	inline TArray<UFlareSectorCatalogEntry*>& GetSectorCatalog()
	{
		return SectorList;
	}

	inline UFlareSpacecraftComponentsCatalog* GetShipPartsCatalog() const
	{
		return ShipPartsCatalog;
	}

	inline UFlareCustomizationCatalog* GetCustomizationCatalog() const
	{
		return CustomizationCatalog;
	}

	inline UFlareAsteroidCatalog* GetAsteroidCatalog() const
	{
		return AsteroidCatalog;
	}

	inline UFlareMeteoriteCatalog* GetMeteoriteCatalog() const
	{
		return MeteoriteCatalog;
	}

	inline UFlareCompanyCatalog* GetCompanyCatalog() const
	{
		return CompanyCatalog;
	}

	inline UFlareQuestCatalog* GetQuestCatalog() const
	{
		return QuestCatalog;
	}

	inline UFlareResourceCatalog* GetResourceCatalog() const
	{
		return ResourceCatalog;
	}

	inline UFlareTechnologyCatalog* GetTechnologyCatalog() const
	{
		return TechnologyCatalog;
	}

	inline UFlareScannableCatalog* GetScannableCatalog() const
	{
		return ScannableCatalog;
	}

	inline UFlareGlobalSettings* GetGlobalSettings() const
	{
		return GlobalSettings;
	}

	inline UFlareStartingScenarioCatalog* GetStartingScenarios() const
	{
		return StartingScenarioCatalog;
	}	

	UFlareSkirmishManager* GetSkirmishManager() const
	{
		return SkirmishManager;
	}

	FFlarePlayerSave* GetPlayerData() const
	{
		return PlayerData;
	}
		
	inline bool IsLoadedOrCreated() const
	{
		return LoadedOrCreated;
	}

	inline FName GetDefaultWeaponIdentifier() const
	{
		return DefaultWeaponIdentifier;
	}

	inline FName GetDefaultTurretIdentifier() const
	{
		return DefaultTurretIdentifier;
	}

	inline void SetDefaultWeaponIdentifier(FName Identifier)
	{
		DefaultWeaponIdentifier = Identifier;
	}

	inline void SetDefaultTurretIdentifier(FName Identifier)
	{
		DefaultTurretIdentifier = Identifier;
	}

	inline UStaticMesh* GetDefaultAsteroid() const
	{
		return DefaultAsteroid;
	}

	UFlareScenarioTools* GetScenarioTools()
	{
		return ScenarioTools;
	}

	bool IsLoadingLevel() const
	{
		return IsLoadingStreamingLevel;
	}

	bool IsLoggingoff() const
	{
		return IsLoggingOut;
	}


	FText GetBuildDate() const;

};

