#pragma once

#include "../Flare.h"
#include "../Data/FlareOrbitalMap.h"
#include "../Game/FlareSaveGame.h"
#include "../Game/FlareGameTypes.h"
#include "../Quests/FlareQuest.h"
#include "../UI/FlareUITypes.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "FlarePlayerController.generated.h"


class AFlareHUD;
class AFlareMenuPawn;
class AFlareCockpitManager;
class AFlareMenuManager;
class UFlareCameraShakeCatalog;
class UFlareOrbitalMap;

UCLASS(MinimalAPI)
class AFlarePlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
			Gameplay methods
	----------------------------------------------------*/

	virtual void BeginPlay() override;

	virtual void PlayerTick(float DeltaTime) override;
	
	/** Activate or deactivate the exterbal camera */
	void SetExternalCamera(bool NewState, bool SetFromPlayerInput = false);

	/** Fly this ship */
	void FlyShip(AFlareSpacecraft* Ship, bool PossessNow = true);

	/** Stop flying a ship */
	void ExitShip();

	/** The world is ending. Literally. */
	void PrepareForExit();

	/** Set the pause state */
	void SetWorldPause(bool Pause);
	
	/** Get the currently selected fleet */
	UFlareFleet* GetPlayerFleet();

	/** Quick switch to another ship */
	bool SwitchToNextShip(bool Instant = false);

	/** Is the player ship being targeted ? Get one of the attackers too. */
	void GetPlayerShipThreatStatus(bool& IsTargeted, bool& IsFiredUpon, bool& CollidingSoon, bool& ExitingSoon, bool& LowHealth, UFlareSimulatedSpacecraft*& Threat) const;

	/** Update a sector */
	void CheckSectorStateChanges(UFlareSimulatedSector* Sector);

	/** Discover or visit a sector */
	void DiscoverSector(UFlareSimulatedSector* Sector, bool MarkedAsVisited, bool NotifyPlayer);

	/** Set the current progress of an achievement */
	void SetAchievementProgression(FName Name, float CompletionRatio);

	bool UpdateOrbitalBattleStatesIfOpen();

	/** Increment the current progress of an achievement */
	void IncrementAchievementProgression(FName Name, float AddedRatio);

	/** Clear the current progress of achievements */
	void ClearAchievementProgression();

	/** Achievements are available now */
	void OnQueryAchievementsComplete(const FUniqueNetId& PlayerId, const bool bWasSuccessful);
	
	/** Triggered when a ship has been destroyed */
	void OnSpacecraftDestroyed();

	void UpdateOrbitMenuFleets(bool Instant);

		/*----------------------------------------------------
		Data management
	----------------------------------------------------*/

	/** Load the company info */
	void SetCompanyDescription(const FFlareCompanyDescription& SaveCompanyData);

	/** Load the player from a save file */
	void Load(const FFlarePlayerSave& SavePlayerData);

	/** Get the ship pawn from the game */
	void OnLoadComplete();

	/** Save the player to a save file */
	void Save(FFlarePlayerSave& SavePlayerData, FFlareCompanyDescription& SaveCompanyData);

	/** Set the player's company */
	void SetCompany(UFlareCompany* NewCompany);
	
	/** Call a sector is activated */
	void OnSectorActivated(UFlareSector* ActiveSector);

	/** Call a sector is deactivated */
	void OnSectorDeactivated();

	/** The battle state has changed, update music, notify, etc */
	void UpdateMusicTrack(FFlareSectorBattleState NewBattleState);

	/** Set the currently flown player ship */
	void SetPlayerShip(UFlareSimulatedSpacecraft* NewPlayerShip);

	/** We just hit this spacecraft with a weapon */
	void SignalHit(AFlareSpacecraft* HitSpacecraft, EFlareDamage::Type DamageType);

	/** We've been hit */
	void SpacecraftHit(EFlarePartSize::Type WeaponSize);

	/** We've impacted into something */
	void SpacecraftCrashed();
	
	/** Missile fired at us */
	void MissileFired(AFlareBomb* Bomb);

	/** Play a sound */
	void PlayLocalizedSound(USoundCue* Sound, FVector WorldLocation, USceneComponent* Component);

	/** Cleanup the PC owned stuff */
	void Clean();

	/** Check if a scannable was previously unlocked */
	bool IsScannableUnlocked(FName Identifier);

	/** Unlock a scannable */
	void UnlockScannable(FName Identifier);

	/** Get the unlocked scannable count */
	int32 GetUnlockedScannableCount() const
	{
		return PlayerData.UnlockedScannables.Num();
	}

	/** Get the unlocked scannables */
	TArray<FName>& GetUnlockedScannables()
	{
		return PlayerData.UnlockedScannables;
	}

	void UpdateMenuPawnShip(UFlareSimulatedSpacecraft* Ship);

	/*----------------------------------------------------
		Menus
	----------------------------------------------------*/

	/** Show a notification to the user */
	void Notify(FText Text, FText Info, FName Tag, EFlareNotification::Type Type = EFlareNotification::NT_Info, bool Pinned = false, EFlareMenu::Type TargetMenu = EFlareMenu::MENU_None, FFlareMenuParameterData TargetInfo = FFlareMenuParameterData());

	/** Setup the cockpit */
	void SetupCockpit();

	/** Setup the gamepad */
	void SetupGamepad();

	/** Spawn the menu pawn and prepare the UI */
	void SetupMenu();
	
	/** Entering the main menu */
	void OnEnterMenu();

	/** Exiting the main menu */
	void OnExitMenu();

	/** Check wether we are in the menu system */
	bool IsInMenu();

	/** Get the position of the mouse on the screen */
	FVector2D GetMousePosition();

	/** Signal that we are selecting weapons */
	void SetSelectingWeapon();

	/** id we select a weapon recently ? */
	bool IsSelectingWeapon() const;

	/** Is the player typing ? */
	bool IsTyping() const;

	/** Show a notification explaining if we succeeded in docking request */
	void NotifyDockingResult(bool Success, AFlareSpacecraft* DockingShip, UFlareSimulatedSpacecraft* Target);

	/** Notify when docking has actually happened */
	void NotifyDockingComplete(AFlareSpacecraft* DockStation, bool TellUser);

	/** Check the battle state of the game before FF */
	bool ConfirmFastForward(FSimpleDelegate OnConfirmed, FSimpleDelegate OnCanceled, bool Automatic);
	
	/** We went out of sector */
	void NotifyExitSector();

	/** Confirmed that we're going back */
	void OnConfirmBackToSector();


	/*----------------------------------------------------
		Objectives
	----------------------------------------------------*/

	/** Start a new objective */
	void StartObjective(FText Name, FFlarePlayerObjectiveData Data);

	/** Finalize the objective */
	void CompleteObjective();

	/** Check if there is an objective yet */
	bool HasObjective() const;

	/** Get the raw objective data */
	const FFlarePlayerObjectiveData* GetCurrentObjective() const;


	/*----------------------------------------------------
		Customization
	----------------------------------------------------*/

	/** Draw the banner */
	UFUNCTION()
	void DrawBanner(UCanvas* TargetCanvas, int32 Width, int32 Height);

	/** Set the color of engine exhausts */
	void SetBasePaintColor(FLinearColor Color);

	/** Set the color of ship paint */
	void SetPaintColor(FLinearColor Color);

	/** Set the color of ship overlays */
	void SetOverlayColor(FLinearColor Color);

	/** Set the color of ship lights */
	void SetLightColor(FLinearColor Color);

	/** Set the pattern index for ship paint */
	void SetPatternIndex(int32 Index);


	/*----------------------------------------------------
		Input
	----------------------------------------------------*/

	virtual void SetupInputComponent() override;

	/** Secondary input that is used whil in external camera */
	void MousePositionInput(FVector2D Val);

	/** Toggle the external view */
	void ToggleCamera();

	/** Toggle the menus */
	void ToggleMenu();

	/** Toggle the overlay */
	void ToggleOverlay();

	/** Send back to menu */
	void BackMenu();

	/** Enter a menu */
	void EnterMenuDown();
	void EnterMenuUp();

	/** Simulate a turn */
	void Simulate();

	/** Simulate a turn */
	void SimulateConfirmed();

	/** Toggle the performance logger */
#if !UE_BUILD_SHIPPING
	void TogglePerformance();
#endif

	/** Waiting for a key */
	void SetWaitingForKey(bool State);

	/** Set busy */
	void SetBusy(bool Busy);

	// Menus
	void ShipMenu();
	void SectorMenu();
	void OrbitMenu();
	void EconomyMenu();
	void LeaderboardMenu();
	void CompanyMenu();
	void FleetMenu();
	void TechnologyMenu();
	void QuestMenu();
	void MainMenu();
	void SettingsMenu();
	void TradeMenu();

	// Hotkeys
	void SpacecraftKey1();
	void SpacecraftKey2();
	void SpacecraftKey3();
	void SpacecraftKey4();
	void SpacecraftKey5();
	void SpacecraftKey6();
	void SpacecraftKey7();

	/** Toggle the combat mode */
	void ToggleCombat();

	/** Force the pilot mode */
	void EnablePilot();

	bool GetIsPiloted();

	/** Force the pilot mode */
	void DisengagePilot();

	/** Toggle the HUD's presence */
	void ToggleHUD();

	/** Quick switch */
	void QuickSwitch();

	/** Move hidden cursor */
	void MouseInputX(float Val);

	/** Move hidden cursor */
	void MouseInputY(float Val);


	/** Gamepad lateral movement X */
	void GamepadLeftStickX(float Val);

	/** Gamepad lateral movement Y */
	void GamepadLeftStickY(float Val);

	/** Gamepad lateral movement X */
	void GamepadRightStickX(float Val);

	/** Gamepad lateral movement Y */
	void GamepadRightStickY(float Val);

	/** Gamepad thrust */
	void GamepadThrustInput(float Val);


	/** Joystick lateral movement X */
	void JoystickMoveHorizontalInput(float Val);

	/** Joystick lateral movement Y */
	void JoystickMoveVerticalInput(float Val);

	/** Joystick angular movement X */
	void JoystickYawInput(float Val);

	/** Joystick angular movement Y */
	void JoystickPitchInput(float Val);

	void LeftMouseButtonPressed();
	void LeftMouseButtonReleased();

	/** Hack for right mouse button triggering drag in external camera */
	void RightMouseButtonPressed();

	/** Hack for right mouse button triggering drag in external camera */
	void RightMouseButtonReleased();

	/** Take high res screenshot */
	void TakeHighResScreenshot();
	

	/*----------------------------------------------------
		Wheel menu
	----------------------------------------------------*/

	/** Open the wheel menu */
	void WheelPressed();

	/** Show middle mouse menus*/
	void WheelMenuShowMainMenu();
	void WheelMenuShowFleetMenu();
	void WheelMenuShowTravelMainMenu();
	void WheelMenuShowTravelOrbitalMenu(FFlareSectorCelestialBodyDescription OrbitalBody);
	void WheelMenuShowContracts();

	void WheelMenuConfirmTravelSelection(FFlareSectorCelestialBodyDescription OrbitalBody, UFlareSimulatedSector* Sector);
	void WheelMenuConfirmedTravel(UFlareSimulatedSector* Sector);
	void WheelMenuRepairAndRefill();

	FString GetQuestWheelIcon(UFlareQuest* Quest);
	void WheelMenuSelectQuest(UFlareQuest* Quest);

	/** Close the wheel menu */
	void WheelReleased();
	void CloseWheelMenu(bool EnableActionOnClose = true);

	/** Inspect the target */
	void InspectTargetSpacecraft();

	/** Fly the target */
	void FlyTargetSpacecraft();

	/** Dock at the target */
	void DockAtTargetSpacecraft();

	/** Set the required combat tactic */
	void SetTacticForCurrentGroup(EFlareCombatTactic::Type Tactic);

	/** Match speed with target spacecraft */
	void MatchSpeedWithTargetSpacecraft();

	/** Find the target spacecraft */
	void LookAtTargetSpacecraft();

	/** Open the upgrade menu */
	void UpgradeShip();

	/** Undock */
	void UndockShip();

	/** trade with the station we're docked to */
	void StartTrading();

	void LaunchDrones();
	void RetrieveDrones();

protected:
	
	/*----------------------------------------------------
		Gameplay data
	----------------------------------------------------*/

	/** Camera shakes */
	UPROPERTY()
	UFlareCameraShakeCatalog*                CameraShakeCatalog;

	// Sound manager
	UPROPERTY()
	UFlareSoundManager*                      SoundManager;

	/** Speed effect template */
	UPROPERTY()
	UParticleSystem*                         LowSpeedEffectTemplate;

	/** Speed effect template */
	UPROPERTY()
	UParticleSystem*                         HighSpeedEffectTemplate;

	/** Speed effect */
	UPROPERTY()
	UParticleSystemComponent*                LowSpeedEffect;

	/** Speed effect */
	UPROPERTY()
	UParticleSystemComponent*                HighSpeedEffect;

	/** Player save structure */
	UPROPERTY()
	FFlarePlayerSave                         PlayerData;

	/** Company data */
	UPROPERTY()
	FFlareCompanyDescription                 CompanyData;

	/** Player pawn used inside menus */
	UPROPERTY()
	AFlareMenuPawn*                          MenuPawn;

	/** Currently flown ship */
	UPROPERTY()
	AFlareSpacecraft*                        ShipPawn;

	/** Currently flown ship */
	UPROPERTY()
	UFlareSimulatedSpacecraft*               PlayerShip;

	/** Player fleet */
	UPROPERTY()
	UFlareFleet*                             PlayerFleet;

	/** Player owned company */
	UPROPERTY()
	UFlareCompany*                           Company;

	/** Menu management */
	UPROPERTY()
	AFlareMenuManager*                       MenuManager;

	/** Cockpit system */
	UPROPERTY()
	AFlareCockpitManager*                    CockpitManager;

	// Company advertisement texture
	UPROPERTY()
	UCanvasRenderTarget2D*                   CompanyBanner;

	// Banner font
	UPROPERTY()
	UFont*                                   BannerFont;

	/** Objective */
	UPROPERTY()
	FFlarePlayerObjectiveData                CurrentObjective;

	const FSlateBrush*                       BackgroundDecorator;

	// Achievements
	TSharedPtr<const class FUniqueNetId>     UserId;
	class IOnlineSubsystem*                  OnlineSub;
	bool                                     AchievementsAvailable;

	int32                                    QuickSwitchNextOffset;
	float                                    WeaponSwitchTime;
	float                                    TimeSinceWeaponSwitch;
	float                                    CombatZoomFOVRatio;

	bool                                     WaitingForKey;
	bool                                     RightMousePressed;
	bool                                     HasCurrentObjective;
	bool                                     IsBusy;
	bool									 SimulatedConfirmed;
	bool									 PlayerSetCameraMode;
	float                                    LastSimulateTime;

	FFlareMovingAverage<float>               JoystickHorizontalInputVal;
	FFlareMovingAverage<float>               JoystickVerticalInputVal;
	FFlareMovingAverage<float>               JoystickPitchInputVal;
	FFlareMovingAverage<float>               JoystickYawInputVal;

	FFlareSectorBattleState                  LastBattleState;
	TMap<UFlareSimulatedSector*, FFlareSectorBattleState> LastSectorBattleStates;

public:

	/*----------------------------------------------------
		Config
	----------------------------------------------------*/
		
	/** Whether to use the 3D cockpit */
	bool                                     UseCockpit;

	/** Whether to use motion blur */
	bool                                     UseMotionBlur;

	/** Whether to use temporal AA */
	bool                                     UseTemporalAA;

	/** Whether to pause the game in menus */
	bool                                     PauseGameInMenus;
	
	void SetUseCockpit(bool New);

	void SetUseMotionBlur(bool New);

	void SetPauseGameInMenus(bool New);

	void SetMusicVolume(int32 New);

	void SetMasterVolume(int32 New);


public:

	/*----------------------------------------------------
		Getters for game classes
	----------------------------------------------------*/

	UFUNCTION(BlueprintCallable, Category = "Flare")
	inline UFlareCompany* GetCompany() const
	{
		return Company;
	}

	inline const FFlareCompanyDescription* GetCompanyDescription() const
	{
		return &CompanyData;
	}

	inline UCanvasRenderTarget2D* GetPlayerBanner() const
	{
		return CompanyBanner;
	}

	UFUNCTION(BlueprintCallable, Category = "Flare")
	inline AFlareMenuPawn* GetMenuPawn() const
	{
		return MenuPawn;
	}

	UFUNCTION(BlueprintCallable, Category = "Flare")
	inline AFlareSpacecraft* GetShipPawn() const
	{
		return ShipPawn;
	}

	AFlareGame* GetGame() const;

	UFUNCTION(BlueprintCallable, Category = "Flare")
	inline AFlareMenuManager* GetMenuManager() const
	{
		return MenuManager;
	}

	UFUNCTION(BlueprintCallable, Category = "Flare")
		AFlareHUD* GetNavHUD() const;

	UFUNCTION(BlueprintCallable, Category = "Flare")
	inline AFlareCockpitManager* GetCockpitManager()
	{
		return CockpitManager;
	}

	UFUNCTION(BlueprintCallable, Category = "Flare")
	inline UFlareSoundManager* GetSoundManager()
	{
		return SoundManager;
	}

	inline UFlareTacticManager* GetTacticManager()
	{
		return GetCompany()->GetTacticManager();
	}

	/** Return the last flown ship. Return NULL if no last flown ship, or if it is destroyed */
	UFlareSimulatedSpacecraft* GetPlayerShip();

	FFlarePlayerSave* GetPlayerData()
	{
		return &PlayerData;
	}

	const FSlateBrush* GetBackgroundDecorator() const
	{
		return BackgroundDecorator;
	}

	bool GetSimulatedConfirmed() const
	{
		return SimulatedConfirmed;
	}

	bool IsGameBusy() const
	{
		return IsBusy;
	}

	bool GetPlayerSetCameraMode() const
	{
		return PlayerSetCameraMode;
	}

	/** Convert vertical to horizontal FOV - We want vertical for the cockpit, UE uses horizontal */
	float VerticalToHorizontalFOV(float VerticalFOV) const;

	/** Get the maximum vertical FOV */
	float GetMaxVerticalFOV() const;

	/** Get the default vertical FOV */
	float GetMinVerticalFOV() const;

	/** Get the default horizontal FOV */
	float GetMinFOV() const;

	/** Get the unzoomed horizontal FOV */
	float GetNormalFOV() const;

	/** Get the desired horizontal FOV */
	float GetCurrentFOV() const;

};

