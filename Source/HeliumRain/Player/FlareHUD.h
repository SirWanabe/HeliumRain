#pragma once

#include "GameFramework/HUD.h"
#include "FlareMenuManager.h"
#include "../Spacecrafts/Subsystems/FlareSimulatedSpacecraftDamageSystem.h"
#include "FlareHUD.generated.h"


class AFlareSpacecraft;
class SFlareHUDMenu;
class SFlareContextMenu;
class SFlareMouseMenu;
class UFlareWeapon;
class UCanvasRenderTarget2D;


/** Navigation HUD */
UCLASS()
class HELIUMRAIN_API AFlareHUD : public AHUD
{
public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Setup
	----------------------------------------------------*/

	virtual void BeginPlay() override;

	/** Setup the HUD */
	virtual void Setup(AFlareMenuManager* NewMenuManager);


	/*----------------------------------------------------
		HUD interaction
	----------------------------------------------------*/

	/** Toggle the HUD's presence */
	void ToggleHUD();

	/** Show the interface on the HUD (not the flight helpers) */
	void SetInteractive(bool Status);

	/** Set the wheel menu state */
	void SetWheelMenu(bool State, bool EnableActionOnClose = true, int32 NewSelectedWidget = -1);

	/** Move wheel menu cursor */
	void SetWheelCursorMove(FVector2D Move);

	/** Is the mouse menu open */
	bool IsWheelMenuOpen() const;

	/** Notify the HUD the played ship has changed */
	void OnTargetShipChanged();

	/** Decide if the HUD is displayed or not */
	void UpdateHUDVisibility();

	/** We just hit this spacecraft with a weapon */
	void SignalHit(AFlareSpacecraft* HitSpacecraft, EFlareDamage::Type DamageType);


	virtual void DrawHUD() override;

	virtual void Tick(float DeltaSeconds) override;

	/** Canvas callback for the cockpit's HUD*/
	UFUNCTION()
	void DrawHUDTexture(UCanvas* TargetCanvas, int32 Width, int32 Height);

	/** Toggle performance counters */
	void TogglePerformance();


	/*----------------------------------------------------
		Cockpit
	----------------------------------------------------*/

	/** Canvas callback for the cockpit's instruments*/
	UFUNCTION()
	void DrawCockpitInstruments(UCanvas* TargetCanvas, int32 Width, int32 Height);

	/** Draw on the current canvas the ship's subsystem status */
	void DrawCockpitSubsystems(AFlareSpacecraft* PlayerShip);

	/** Draw on the current canvas the ship's weapon or cargo status */
	void DrawCockpitEquipment(AFlareSpacecraft* PlayerShip);

	/** Draw on the current canvas the ship's target info */
	void DrawCockpitTarget(AFlareSpacecraft* PlayerShip);

	FText GetCarrierText(AFlareSpacecraft* PlayerShip);

	/** Draw a subsystem info line on a cockpit instrument */
	void DrawCockpitSubsystemInfo(EFlareSubsystem::Type Subsystem, FVector2D& Position);

	/** Draw a progress bar with icon and text */
	void DrawProgressBarIconText(FVector2D Position, UTexture2D* Icon, FText Text, FLinearColor Color, float Progress, int ProgressWidth);


	/*----------------------------------------------------
		HUD helpers
	----------------------------------------------------*/

	/** Update the context menu */
	void UpdateContextMenu(AFlareSpacecraft* PlayerShip);

	/** Get the temperature color, using custom threshold */
	static FLinearColor GetTemperatureColor(float Current, float Max);

	/** Get the health color */
	static FLinearColor GetHealthColor(float Current);

	/** Format a distance in meter */
	static FText FormatDistance(float Distance);


	/*----------------------------------------------------
		Internals
	----------------------------------------------------*/

protected:

	/** Should we draw the HUD ? */
	bool ShouldDrawHUD() const;

	/** Drawing back-end */
	void DrawHUDInternal();

	/** Drawing debug grid*/
	void DrawDebugGrid (FLinearColor Color);

	/** Draw speed indicator */
	void DrawSpeed(AFlarePlayerController* PC, AActor* Object, UTexture2D* Icon, FVector Speed);

	/** Draw a search arrow */
	void DrawSearchArrow(FVector TargetLocation, FLinearColor Color, bool Highlighted, float MaxDistance = 10000000);

	/** Draw a designator block around a spacecraft */
	bool DrawHUDDesignator(AFlareSpacecraft* Spacecraft);

	/** Draw a designator corner */
	void DrawHUDDesignatorCorner(FVector2D Position, FVector2D ObjectSize, float IconSize, FVector2D MainOffset, float Rotation, FLinearColor HudColor, bool Dangerous, bool Highlighted);

	/** Draw a status block for the ship */
	FVector2D DrawHUDDesignatorStatus(FVector2D Position, float IconSize, AFlareSpacecraft* Ship);

	/** Draw a hint block for the ship */
	FVector2D DrawHUDDesignatorHint(FVector2D Position, float IconSize, AFlareSpacecraft* Ship, FLinearColor Color);

	/** Draw a docking helper around the current best target */
	void DrawDockingHelper();

	/** Draw a status icon */
	FVector2D DrawHUDDesignatorStatusIcon(FVector2D Position, float IconSize, UTexture2D* Texture, FLinearColor Color);

	/** Draw an icon */
	void DrawHUDIcon(FVector2D Position, float IconSize, UTexture2D* Texture, FLinearColor Color = FLinearColor::White, bool Center = false);

	/** Draw an icon */
	void DrawHUDIconRotated(FVector2D Position, float IconSize, UTexture2D* Texture, FLinearColor Color = FLinearColor::White, float Rotation = 0);

	/** Print a text with a shadow */
	void FlareDrawText(FText Text, FVector2D Position, FLinearColor Color = FLinearColor::White, bool Center = true, bool Large = false);

	/** Draw a texture */
	void FlareDrawTexture(UTexture* Texture, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float TextureU, float TextureV, float TextureUWidth, float TextureVHeight, FLinearColor TintColor = FLinearColor::White, EBlendMode BlendMode = BLEND_Translucent, float Scale = 1.f, bool bScalePosition = false, float Rotation = 0.f, FVector2D RotPivot = FVector2D::ZeroVector);

	/** Draw a line */
	void FlareDrawLine(FVector2D Start, FVector2D End, FLinearColor Color);

	/** Draw a progress bar */
	void FlareDrawProgressBar(FVector2D Position, int BarWidth, FLinearColor Color, float Ratio);

	/** Get an alpha fade to avoid overdrawing two objects */
	float GetFadeAlpha(FVector2D A, FVector2D B);

	/** Is this position inside the viewport + border */
	bool IsInScreen(FVector2D ScreenPosition) const;

	/** Get the appropriate hostility color */
	FLinearColor GetHostilityColor(AFlarePlayerController* PC, AFlareSpacecraft* Target);

	/** Is the player flying a military ship */
	bool IsFlyingMilitaryShip() const;
	
	/** Convert a world location to cockpit-space */
	bool ProjectWorldLocationToCockpit(FVector World, FVector2D& Cockpit);
	

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	// Menu reference
	UPROPERTY()
	AFlareMenuManager*                      MenuManager;

	// HUD texture material master template
	UPROPERTY(Category = Cockpit, EditAnywhere)
	UMaterial*                              HUDRenderTargetMaterialTemplate;

	// HUD texture material
	UPROPERTY(Category = Cockpit, EditAnywhere)
	UMaterialInstanceDynamic*               HUDRenderTargetMaterial;

	// HUD texture canvas
	UPROPERTY(Category = Cockpit, EditAnywhere)
	UCanvasRenderTarget2D*                  HUDRenderTarget;

	// Settings
	float                                   CombatMouseRadius;
	float                                   FocusDistance;
	int32                                   IconSize;
	FLinearColor                            HudColorNeutral;
	FLinearColor                            HudColorFriendly;
	FLinearColor                            HudColorEnemy;
	FLinearColor                            HudColorObjective;
	FLinearColor                            ShadowColor;

	// General data
	bool                                    HUDVisible;
	bool                                    IsInteractive;
	bool                                    IsDrawingHUD;
	int32                                   PreviousScreenPercentage;
	FVector2D                               ViewportSize;
	FVector2D                               PreviousViewportSize;
	AFlareSpacecraft*                       ContextMenuSpacecraft;

	// Drawing context
	FVector2D                               CurrentViewportSize;
	UCanvas*                                CurrentCanvas;

	// Hit target
	AFlareSpacecraft*                       PlayerHitSpacecraft;
	bool                                    HasPlayerHit;
	EFlareDamage::Type                      PlayerDamageType;
	float                                   PlayerHitTime;
	float                                   PlayerHitDisplayTime;

	// Designator content
	UTexture2D*                             HUDReticleIcon;
	UTexture2D*                             HUDCombatReticleIcon;
	UTexture2D*                             HUDBackReticleIcon;
	UTexture2D*                             HUDAimIcon;
	UTexture2D*                             HUDAimHitIcon;
	UTexture2D*                             HUDBombAimIcon;
	UTexture2D*                             HUDBombMarker;
	UTexture2D*                             HUDAimHelperIcon;
	UTexture2D*                             HUDNoseIcon;
	UTexture2D*                             HUDObjectiveIcon;
	UTexture2D*                             HUDCombatMouseIcon;
	UTexture2D*                             HUDSearchArrowIcon;
	UTexture2D*                             HUDSearchDockArrowIcon;
	UTexture2D*                             HUDHighlightSearchArrowTexture;
	UTexture2D*                             HUDDesignatorCornerTexture;
	UTexture2D*                             HUDDesignatorMilCornerTexture;
	UTexture2D*                             HUDDesignatorCornerSelectedTexture;
	UTexture2D*                             HUDDesignatorSelectionTexture;
	UTexture2D*                             HUDDockingCircleTexture;
	UTexture2D*                             HUDDockingAxisTexture;
	UTexture2D*                             HUDDockingForbiddenTexture;
	UTexture2D*                             HUDDockingMonitorIcon;

	// Ship status content
	UTexture2D*                             HUDTemperatureIcon;
	UTexture2D*                             HUDPowerIcon;
	UTexture2D*                             HUDPropulsionIcon;
	UTexture2D*                             HUDRCSIcon;
	UTexture2D*                             HUDHealthIcon;
	UTexture2D*                             HUDWeaponIcon;
	UTexture2D*                             HUDHarpoonedIcon;
	UTexture2D*                             HUDOrientationIcon;
	UTexture2D*                             HUDDistanceIcon;
	UTexture2D*                             HUDContractIcon;
	UTexture2D*                             HUDShipyardIcon;
	UTexture2D*                             HUDUpgradeIcon;
	UTexture2D*                             HUDConstructionIcon;
	UTexture2D*                             HUDConsumerIcon;

	// Font
	UFont*                                  HUDFontSmall;
	UFont*                                  HUDFont;
	UFont*                                  HUDFontLarge;

	// Instruments
	FVector2D                               TopInstrument;
	FVector2D                               LeftInstrument;
	FVector2D                               RightInstrument;
	FVector2D                               InstrumentSize;
	FVector2D                               InstrumentLine;
	int                                     SmallProgressBarSize;
	int                                     LargeProgressBarSize;
	int                                     ProgressBarHeight;
	int									    DockedDrones;
	int										TotalDrones;
	float                                   ProgressBarInternalMargin;
	float                                   ProgressBarTopMargin;
	float                                   CockpitIconSize;
	
	// Slate menus
	TSharedPtr<SFlareHUDMenu>               HUDMenu;
	TSharedPtr<SFlareMouseMenu>             MouseMenu;
	TSharedPtr<SOverlay>                    ContextMenuContainer;
	TSharedPtr<SFlareContextMenu>           ContextMenu;
	FVector2D                               ContextMenuPosition;

	// Power
	float                                   CurrentPowerTime;
	float                                   PowerTransitionTime;

	// Debug
	bool                                    ShowPerformance;
	float                                   PerformanceTimer;
	float                                   FrameTime;
	float                                   GameThreadTime;
	float                                   RenderThreadTime;
	float                                   GPUFrameTime;
	FText                                   PerformanceText;

public:

	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	const FVector2D& GetContextMenuLocation() const
	{
		return ContextMenuPosition;
	}

	TSharedPtr<SFlareMouseMenu> GetMouseMenu() const
	{
		return MouseMenu;
	}

	FVector2D GetViewportSize() const
	{
		return ViewportSize;
	}

	UCanvas* GetCanvas() const
	{
		return Canvas;
	}

	FText GetPerformanceText() const
	{
		return PerformanceText;
	}

	bool IsHUDVisible() const
	{
		return HUDVisible;
	}

};
