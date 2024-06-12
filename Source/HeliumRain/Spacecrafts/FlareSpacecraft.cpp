
#include "FlareSpacecraft.h"
#include "../Flare.h"

#include "FlareOrbitalEngine.h"
#include "FlareRCS.h"
#include "FlareWeapon.h"
#include "FlareTurret.h"
#include "FlareShipPilot.h"
#include "FlarePilotHelper.h"
#include "FlareInternalComponent.h"

#include "Particles/ParticleSystemComponent.h"

#include "../Data/FlareSpacecraftCatalog.h"
#include "../Data/FlareSpacecraftComponentsCatalog.h"

#include "../Player/FlarePlayerController.h"
#include "../Player/FlareMenuManager.h"

#include "../Game/FlareGame.h"
#include "../Game/FlareAsteroid.h"
#include "../Game/FlareScannable.h"
#include "../Game/FlareSkirmishManager.h"
#include "../Game/FlareGameUserSettings.h"
#include "../Game/FlareGameTools.h"
#include "../Game/AI/FlareCompanyAI.h"

#include "../UI/Menus/FlareShipMenu.h"

#include "../UI/Menus/FlareTradeMenu.h"

#include "Components/DecalComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Engine/Canvas.h"
#include "EngineUtils.h"
#include "Engine.h"

#include "../Game/FlareSectorHelper.h"


DECLARE_CYCLE_STAT(TEXT("FlareSpacecraft Systems"), STAT_FlareSpacecraft_Systems, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareSpacecraft CarrierRelease"), STAT_Flarespacecraft_CarrierRelease, STATGROUP_Flare); 
DECLARE_CYCLE_STAT(TEXT("FlareSpacecraft Player"), STAT_FlareSpacecraft_PlayerShip, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareSpacecraft Hit"), STAT_FlareSpacecraft_Hit, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareSpacecraft Aim"), STAT_FlareSpacecraft_Aim, STATGROUP_Flare);

#define LOCTEXT_NAMESPACE "FlareSpacecraft"
#define DELETION_DELAYCRAFT 3

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareSpacecraft::AFlareSpacecraft(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	// Ship name font
	static ConstructorHelpers::FObjectFinder<UFont> ShipNameFontObj(TEXT("/Game/Slate/Fonts/ShipNameFont.ShipNameFont"));
	ShipNameFont = ShipNameFontObj.Object;

	// Sound
	static ConstructorHelpers::FObjectFinder<USoundCue> WeaponLoadedSoundObj(TEXT("/Game/Sound/Firing/A_WeaponLoaded"));
	static ConstructorHelpers::FObjectFinder<USoundCue> WeaponUnloadedSoundObj(TEXT("/Game/Sound/Firing/A_WeaponUnloaded"));
	WeaponLoadedSound = WeaponLoadedSoundObj.Object;
	WeaponUnloadedSound = WeaponUnloadedSoundObj.Object;

	// Default dynamic state
	static ConstructorHelpers::FClassFinder<AActor> IdleShipyardTemplateObj(TEXT("/Game/Stations/Shipyard/States/BP_shipyard_idle"));
	IdleShipyardTemplate = IdleShipyardTemplateObj.Class;

	// Create static mesh component
	Airframe = PCIP.CreateDefaultSubobject<UFlareSpacecraftComponent>(this, TEXT("Airframe"));
	Airframe->SetSimulatePhysics(false);
	RootComponent = Airframe;

	// Camera settings
	CameraContainerYaw->AttachToComponent(Airframe, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
	CameraMaxPitch = 80;
	CameraPanSpeed = 200;

	// Joystick smoothing
	int32 SmoothFrames = 15;
	JoystickRollInputVal.SetSize(SmoothFrames);
	
	// Gameplay
	HasExitedSector = false;
	Paused = false;
	LoadedAndReady = false;
	AttachedToParentActor = false;
	TargetIndex = 0;
	TimeSinceSelection = 0;
	MaxTimeBeforeSelectionReset = 3.0;
	ScanningTimerDuration = 5.0f;
	StateManager = NULL;
	NavigationSystem = NULL;
	TimeSinceUncontrollable = FLT_MAX;
	PreviousJoystickThrottle = 0;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void AFlareSpacecraft::BeginPlay()
{
	Super::BeginPlay();

	// Setup asteroid components, if any
	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareAsteroidComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareAsteroidComponent* AsteroidComponent = Cast<UFlareAsteroidComponent>(Components[ComponentIndex]);
		if (AsteroidComponent)
		{
			bool IsIcy = false;

			AFlareGame* Game = Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
			if (Game)
			{
				if (Game->GetActiveSector())
				{
					IsIcy = Game->GetActiveSector()->GetDescription()->IsIcy;
				}
				else
				{
					UFlareSimulatedSpacecraft* Spacecraft = Game->GetPC()->GetMenuManager()->GetShipMenu()->GetTargetSpacecraft();
					if (Spacecraft)
					{
						IsIcy = Spacecraft->GetCurrentSector()->GetDescription()->IsIcy;
					}
				}
			}

			AsteroidComponent->SetupEffects(IsIcy);
		}
	}

	CurrentTarget.Clear();
}

void AFlareSpacecraft::TickSpacecraft(float DeltaSeconds)
{
	TimeToStopCached = false;
	// Main tick
	if (StateManager && !Paused)
	{
		if(GetParent()->GetDamageSystem()->IsUncontrollable() && GetParent()->GetDamageSystem()->IsDisarmed())
		{
			TimeSinceUncontrollable += DeltaSeconds;
		}
		else
		{
			TimeSinceUncontrollable	 = 0.f;
		}

		// Tick systems
		{
			SCOPE_CYCLE_COUNTER(STAT_FlareSpacecraft_Systems);
			StateManager->Tick(DeltaSeconds);
			if(!IsStation())
			{
				NavigationSystem->TickSystem(DeltaSeconds);
			}
			if (IsMilitaryArmed())
			{
				WeaponsSystem->TickSystem(DeltaSeconds);
			}
			if (!IsExploding)
			{
				DamageSystem->TickSystem(DeltaSeconds);
			}
		}

		if (IsExploding)
		{
			SetWantUndockInternalShips(true);
			TimeSinceLastExplosion -= DeltaSeconds;
			if (TimeSinceLastExplosion <= 0)
			{
				ExplodingShip();
			}
		}

		if (GetParent()->GetData().WantUndockInternalShips && !HasUndockedAllInternalShips)
		{
			CarrierReleaseInternalDockShips(DeltaSeconds);
		}

		// Lights
		TArray<UActorComponent*> LightComponents = GetComponentsByClass(USpotLightComponent::StaticClass());
		for (int32 ComponentIndex = 0; ComponentIndex < LightComponents.Num(); ComponentIndex++)
		{
			USpotLightComponent* Component = Cast<USpotLightComponent>(LightComponents[ComponentIndex]);
			if (Component)
			{
				Component->SetActive(!Parent->GetDamageSystem()->HasPowerOutage());
			}
		}

		// Player ship updates
		AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
		if (PC && this == PC->GetShipPawn())
		{
			SCOPE_CYCLE_COUNTER(STAT_FlareSpacecraft_PlayerShip);
			AFlareSpacecraft* PlayerShip = PC->GetShipPawn();

			// Reload the sector if player leave the limits
			if (!HasExitedSector && IsOutsideSector())
			{
				float Limits = GetGame()->GetActiveSector()->GetSectorLimits();
				float Distance = GetActorLocation().Size();
				FLOGV("%s exit sector distance to center=%f and limits=%f", *GetImmatriculation().ToString(), Distance, Limits);

				if (!GetGame()->IsSkirmish())
				{					
					// Reset the ship
					if (GetData().SpawnMode != EFlareSpawnMode::Travel)
					{
						GetData().SpawnMode = EFlareSpawnMode::Travel;
					}

					// Reload
					if (GetParent()->GetCurrentSector()->IsTravelSector())
					{
						FFlareMenuParameterData MenuParameters;
						MenuParameters.Spacecraft = GetParent();
						MenuParameters.Sector = GetParent()->GetCurrentSector();
						PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_ReloadSector, MenuParameters);
					}

					// Travel
					else
					{
						UFlareTravel* Travel = GetGame()->GetGameWorld()->StartTravel(GetParent()->GetCurrentFleet(), GetParent()->GetCurrentSector(), true);

						if (Travel)
						{
							FFlareMenuParameterData Data;
							Data.Travel = Travel;
							PC->GetMenuManager()->OpenMenu(EFlareMenu::MENU_Travel, Data);
						}
					}

					return;
				}
				else
				{
					GetGame()->GetSkirmishManager()->EndPlay();
				}
			}

			// Update progress of the scanning mode
			if (IsInScanningMode())
			{
				bool AngularIsActive, LinearIsActive, ScanningIsActive;
				float ScanningAngularRatio, ScanningLinearRatio, ScanningAnalyzisRatio;
				float ScanningDistance, ScanningSpeed;

				GetScanningProgress(AngularIsActive, LinearIsActive, ScanningIsActive,
					ScanningAngularRatio, ScanningLinearRatio, ScanningAnalyzisRatio,
					ScanningDistance, ScanningSpeed);

				if (ScanningIsActive)
				{
					ScanningTimer += DeltaSeconds * ScanningSpeed;
				}
				else
				{
					ScanningTimer = 0;
				}
			}
			
			// Detect manual docking
			IsManualDocking = false;
			IsAutoDocking = false;
			float MaxDistance = (GetSize() == EFlarePartSize::S) ? 25000 : 50000;
			float BestDistance = MaxDistance;
			for (int SpacecraftIndex = 0; SpacecraftIndex < GetGame()->GetActiveSector()->GetSpacecrafts().Num(); SpacecraftIndex++)
			{
				// Calculation data
				AFlareSpacecraft* Spacecraft = GetGame()->GetActiveSector()->GetSpacecrafts()[SpacecraftIndex];
				
				// Required conditions for docking
				if ((Spacecraft->GetActorLocation() - GetActorLocation()).Size() < MaxDistance
				 && Spacecraft != this
				 && Spacecraft->IsStation()
				 && Spacecraft->GetParent()->GetDamageSystem()->IsAlive()
				 && Spacecraft->GetDockingSystem()->GetDockCount() > 0
				 && GetWeaponsSystem()->GetActiveWeaponGroupIndex() < 0)
				{
					// Select closest dock
					FVector CameraLocation = Airframe->GetSocketLocation(FName("Camera"));
					FFlareDockingInfo BestDockingPort;
					for (int32 DockingPortIndex = 0; DockingPortIndex < Spacecraft->GetDockingSystem()->GetDockCount(); DockingPortIndex++)
					{
						float AutoDockDistance = (GetSize() == EFlarePartSize::S ? 250 : 500);
						FFlareDockingInfo DockingPort = Spacecraft->GetDockingSystem()->GetDockInfo(DockingPortIndex);
						FFlareDockingParameters DockingParameters = GetNavigationSystem()->GetDockingParameters(DockingPort, CameraLocation);

						// When under this distance, we're going to be docking
						if (DockingPort.DockSize == GetSize() && DockingParameters.DockToDockDistance < 2 * AutoDockDistance)
						{
							IsAutoDocking = true;
						}
						
						// Check if we should draw it
						if (DockingPort.DockSize == GetSize()
						 && GetNavigationSystem()->IsManualPilot()
						 && !GetStateManager()->IsExternalCamera()
						 && DockingParameters.DockingPhase != EFlareDockingPhase::Docked
						 && DockingParameters.DockingPhase != EFlareDockingPhase::Distant)
						{
							// Get distance
							FVector DockVector = DockingParameters.StationDockLocation - DockingParameters.ShipDockLocation;
							if (DockVector.Size() < BestDistance && FVector::DotProduct(DockVector, GetActorRotation().Vector()) > 0)
							{
								// Set parameters
								IsManualDocking = true;
								BestDistance = DockVector.Size();
								BestDockingPort = DockingPort;
								ManualDockingTarget = Spacecraft;
								ManualDockingStatus = DockingParameters;
								ManualDockingInfo = DockingPort;

								// Auto-dock when ready
								if (!Spacecraft->IsPlayerHostile()
									&& (DockingParameters.DockingPhase == EFlareDockingPhase::Dockable
									 || DockingParameters.DockingPhase == EFlareDockingPhase::FinalApproach
									 || DockingParameters.DockingPhase == EFlareDockingPhase::Approach)
								 && DockingParameters.DockToDockDistance < AutoDockDistance)
								{
									GetNavigationSystem()->DockAt(Spacecraft);
									PC->SetAchievementProgression("ACHIEVEMENT_MANUAL_DOCK", 1);
								}
							}
						}
					}
				}
			}

			// Set a default target if there is current target
			if (CurrentTarget.IsEmpty())
			{
				TArray<FFlareScreenTarget>& ScreenTargets = GetCurrentTargets();
				if (ScreenTargets.Num())
				{
					int32 ActualIndex = TargetIndex % ScreenTargets.Num();
					SetCurrentTarget(ScreenTargets[ActualIndex].Spacecraft);
				}
			}

			TimeSinceSelection += DeltaSeconds;
			// The FlareSpacecraftPawn do the camera effective update in its Tick so call it after camera order update
			// UpdateCameraPositions(DeltaSeconds);
		}

		// Make ship bounce lost ships if they are outside 1.5 * limit
		float Distance = GetActorLocation().Size();
		float Limits = GetGame()->GetActiveSector()->GetSectorLimits();
		if (Distance > Limits * 1.5f)
		{
			Airframe->SetPhysicsLinearVelocity(-Airframe->GetPhysicsLinearVelocity() / 2.f);
		}


		float SmoothedVelocityChangeSpeed = FMath::Clamp(DeltaSeconds * 8, 0.f, 1.f);
		SmoothedVelocity = SmoothedVelocity * (1 - SmoothedVelocityChangeSpeed) + GetLinearVelocity() * SmoothedVelocityChangeSpeed;
	}
}

void AFlareSpacecraft::CarrierReleaseInternalDockShips(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_Flarespacecraft_CarrierRelease);
	TimeBeforeNextInternalUndock -= DeltaSeconds;
	if (TimeBeforeNextInternalUndock <= 0)
	{
		bool UndockedAShip = false;
		for (UFlareSimulatedSpacecraft* OwnedShips : GetParent()->GetShipChildren())
		{
			if (!OwnedShips->IsActive() && OwnedShips->IsInternalDockedTo(GetParent()) && !OwnedShips->IsReserve())
			{
				if (OwnedShips->GetGame()->GetActiveSector() && OwnedShips->GetGame()->GetActiveSector()->GetSimulatedSector() == OwnedShips->GetCurrentSector())
				{
					OwnedShips->GetGame()->GetActiveSector()->LoadSpacecraft(OwnedShips);
					UndockedAShip = true;
					break;
				}
			}
		}

		if (GetParent()->GetDescription()->DroneLaunchDelay > 0)
		{
			TimeBeforeNextInternalUndock = GetParent()->GetDescription()->DroneLaunchDelay;
		}
		else
		{
			TimeBeforeNextInternalUndock = 1;
		}

		if (!UndockedAShip)
		{
			HasUndockedAllInternalShips = true;
		}
	}
}

void AFlareSpacecraft::LaunchRetrieveDrones()
{
	SetWantUndockInternalShips(!GetParent()->GetData().WantUndockInternalShips);
}

bool AFlareSpacecraft::GetWantUndockInternalShips()
{
	return GetParent()->GetData().WantUndockInternalShips;
}

void AFlareSpacecraft::SetWantUndockInternalShips(bool Set)
{
	GetParent()->GetData().WantUndockInternalShips = Set;
}

void AFlareSpacecraft::SetUndockedAllShips(bool Set)
{
	HasUndockedAllInternalShips = Set;
}

void AFlareSpacecraft::UpdateSpacecraftPawn(float DeltaSeconds)
{
	Super::TickSpacecraft(DeltaSeconds);
}

void AFlareSpacecraft::SetCurrentTarget(PilotHelper::PilotTarget const& Target)
{
	if (CurrentTarget != Target)
	{
		CurrentTarget = Target;

		FName TargetName = Target.SpacecraftTarget ? Target.SpacecraftTarget->GetImmatriculation() : NAME_None;

		if (GetGame()->GetQuestManager())
		{
			GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("target-changed").PutName("target", TargetName));
		}
	}
}

inline static bool IsCloserToCenter(const FFlareScreenTarget& TargetA, const FFlareScreenTarget& TargetB)
{
	return (TargetA.DistanceFromScreenCenter < TargetB.DistanceFromScreenCenter);
}

TArray<FFlareScreenTarget>& AFlareSpacecraft::GetCurrentTargets()
{
	Targets.Empty();

	FVector CameraLocation = GetCamera()->GetComponentLocation();
	FVector CameraAimDirection = GetCamera()->GetComponentRotation().Vector();
	CameraAimDirection.Normalize();

	for (AFlareSpacecraft* Spacecraft: GetGame()->GetActiveSector()->GetSpacecrafts())
	{
		if(Spacecraft == this)
		{
			continue;
		}

		if(Spacecraft->IsComplexElement())
		{
			continue;
		}

		if (Spacecraft->GetParent()->IsDestroyed())
		{
			continue;
		}

		if(!Spacecraft->GetParent()->GetDamageSystem()->IsAlive())
		{
			continue;
		}

		FVector LocationOffset = Spacecraft->GetActorLocation() - CameraLocation;
		FVector SpacecraftDirection = LocationOffset.GetUnsafeNormal();

		float Dot = FVector::DotProduct(CameraAimDirection, SpacecraftDirection);
		FFlareScreenTarget Target;
		Target.Spacecraft = Spacecraft;
		Target.DistanceFromScreenCenter = 1.f-Dot;

		Targets.Add(Target);
	}

	Targets.Sort(&IsCloserToCenter);
	return Targets;
}

void AFlareSpacecraft::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareSpacecraft_Hit);

	// Strictly disallow self-collision : this should never happen
	if (Other == this)
	{
		FString SelfCollisionReport = FString::Printf(
			TEXT("AFlareSpacecraft::NotifyHit : %s (%d) self-collision: %s/%s collided with %s/%s in %s"),
			*GetImmatriculation().ToString(),
			IsPresentationMode(),
			(this ? *this->GetName() : TEXT("null")),
			(MyComp ? *MyComp->GetName() : TEXT("null")),
			(Other ? *Other->GetName() : TEXT("null")),
			(OtherComp ? *OtherComp->GetName() : TEXT("null")),
			*GetParent()->GetCurrentSector()->GetSectorName().ToString()
		);

		FLOGV("%s", *SelfCollisionReport);

		FFlareModule::ReportError(SelfCollisionReport);
	}

	// ghoul 10m/s -> asteroid : 5919376.500000
	// ghoul 10m/s -> outpost  : 8190371.000000
	// ghoul 10m/s -> hub : 4157000.750000 + 4161034.500000 =                                                        8318035.25
	// ghoul 10m/s -> asteroid 0 : 5320989.500000 + 1186121.500000 + 920986.437500 =                                 7428097.4375

	// ghoul 20m/s -> asteroid 0 : 13991330.000000 + 1485886.250000 =                                               15477216.25
	// ghoul 20m/s -> outpost : 59829.214844 + 159081.968750  + 513187.187500  + 1729557.875000 + 16334147.000000 = 18795803.246094
	// ghoul 20m/s -> hub : 9352049.000000 8732764.000000 =                                                         18084813.0

	// orca 10m/s -> hub : 8604104.000000 + 12141461.000000 =                                                       20745565.0
	// dragon 10m/s -> hub :7552520.500000 + 148669488.000000 =                                                    156222008.5
	// invader 10m/s -> hub :                                                                                      425907776.000000

	// ghoul 6301.873047            13 19.930628237551
	// orca 19790.914062            10 48.2368290322174
	// dragon 136862.984375         11 41.4482092543008
	// invader 530312.250000        8 03.1264146736191


   // ghoul 20m/s	                                2455.970812449119
   // ghoul 50m/s	20529212.000000 + 20547770.000000    =  41076982.0 / 6301.873047 65.18 m/s

   // ghoul 100 m/s 40483100.000000 + 40519540.000000                    =  81002640.0 / 6301.873047 128 53 m/s

   // 2190338.000000  + 394164960.000000 + 142830160.000000 + 482529472.000000 = 1021714930.0 / 530312.250000 =


	// DrawDebugSphere(GetWorld(), Hit.Location, 10, 12, FColor::Blue, true);

	Super::ReceiveHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Colliding a kill pending actor, or itself
	if (!Other)
	{
		return;
	}

	//FLOGV("AFlareSpacecraft Hit  Mass %f NormalImpulse %s NormalImpulse.Size() %f", GetSpacecraftMass(), *NormalImpulse.ToString(), NormalImpulse.Size());
	DamageSystem->OnCollision(Other, HitLocation, NormalImpulse);
	//FLOGV("%s collide %s", *GetImmatriculation().ToString(), *Other->GetName());
	GetStateManager()->OnCollision();

	// If hit, check if the is a docking in progress. If yes, check if the ship is correctly aligned
	AFlareSpacecraft* OtherSpacecraft = Cast<AFlareSpacecraft>(Other);
	if (OtherSpacecraft)
	{
		//FLOGV(">>>>>>>>>> COLL %s %s", *GetImmatriculation().ToString(), *OtherSpacecraft->GetImmatriculation().ToString());
		//FLOGV(">>>>>>>>>> DIST %s", *AFlareHUD::FormatDistance( (LOC1 - LOC2).Size() / 100).ToString() );

		// The other actor is a spacecraft, check if it's not the station we want to dock to.
		GetNavigationSystem()->CheckCollisionDocking(OtherSpacecraft);
	}

	AFlareMeteorite* Meteorite = Cast<AFlareMeteorite>(Other);
	if(Meteorite)
	{
		Meteorite->OnCollision(this, HitLocation);
	}
}

TArray<AFlareBomb*> AFlareSpacecraft::GetIncomingBombs()
{
	return IncomingBombs;
}

int32 AFlareSpacecraft::GetIncomingActiveBombQuantity()
{
	return GetIncomingBombs().Num();
}

bool AFlareSpacecraft::IsSafeEither()
{
	if (IsSafeDestroying() || BegunSafeDestroy)
	{
		return true;
	}
	return false;
}

bool AFlareSpacecraft::CheckIsExploding()
{
	return IsExploding;
}

bool AFlareSpacecraft::IsSafeDestroying()
{
	return IsSafeDestroyingRunning;
}


FFlareSpacecraftComponentDescription* AFlareSpacecraft::GetDefaultWeaponFallback(bool IsFinalExplosion)
{
	UFlareSpacecraftComponentsCatalog* Catalog = GetGame()->GetShipPartsCatalog();
	if (!Catalog)
	{
		return NULL;
	}

	FFlareSpacecraftComponentDescription* ReturnValue = NULL;
	int32 WhatFallBack = FMath::FRandRange(1, 10);
	// small weapons
	if (WhatFallBack == 10)
	{
		ReturnValue = Catalog->Get(FName("weapon-eradicator"));
	}
	else if (WhatFallBack == 9)
	{
		ReturnValue = Catalog->Get(FName("weapon-mjolnir"));
	}
	else if (WhatFallBack == 8)
	{
		ReturnValue = Catalog->Get(FName("weapon-hammer"));
	}
	//large weapons
	//	ReturnValue = Catalog->Get(FName("weapon-hades"));
	//	hades not an appropriate visual effect
	else if (WhatFallBack >= 6)
	{
		ReturnValue = Catalog->Get(FName("weapon-ares"));
	}
	else
	{
		ReturnValue = Catalog->Get(FName("weapon-artemis"));
	}

	return ReturnValue;
}

float AFlareSpacecraft::GetExplosionScaleFactor(FFlareSpacecraftComponentDescription* WeaponFallback, bool IsFinalExplosion)
{
	UFlareSpacecraftComponentsCatalog* Catalog = GetGame()->GetShipPartsCatalog();
	if (!Catalog)
	{
		return 1;
	}

	float ScaleFactor = 0;

	if (WeaponFallback == Catalog->Get(FName("weapon-eradicator")))
	{
		if (IsFinalExplosion)
		{
			if (GetParent()->GetSize() == EFlarePartSize::S)
			{
				ScaleFactor = FMath::RandRange(5,8);
			}
		}
	}
	else if (WeaponFallback == Catalog->Get(FName("weapon-mjolnir")))
	{
		if (IsFinalExplosion)
		{
			if (GetParent()->GetSize() == EFlarePartSize::S)
			{
				ScaleFactor = FMath::RandRange(6, 10);
			}
		}
	}
	else if (WeaponFallback == Catalog->Get(FName("weapon-hammer")))
	{
		if (IsFinalExplosion)
		{
			if (GetParent()->GetSize() == EFlarePartSize::S)
			{
				ScaleFactor = FMath::RandRange(6, 9);
			}
			else
			{
				ScaleFactor = FMath::RandRange(40, 45);
			}
		}
	}
	else if (WeaponFallback == Catalog->Get(FName("weapon-hades")))
	{
		if (IsFinalExplosion)
		{
			if (GetParent()->GetSize() == EFlarePartSize::S)
			{
				ScaleFactor = FMath::RandRange(1, 2);
			}
			else
			{
				ScaleFactor = FMath::RandRange(3, 5);
			}
		}
		else
		{
			if (GetParent()->GetSize() == EFlarePartSize::S)
			{
				ScaleFactor = 1;
			}
			else
			{
				ScaleFactor = FMath::RandRange(2, 3);
			}
		}
	}
	else if (WeaponFallback == Catalog->Get(FName("weapon-ares")))
	{
		if (IsFinalExplosion)
		{
			if (GetParent()->GetSize() == EFlarePartSize::S)
			{
				ScaleFactor = FMath::RandRange(2, 4);
			}
			else
			{
				ScaleFactor = FMath::RandRange(28,32);
			}
		}
		else
		{
			if (GetParent()->GetSize() == EFlarePartSize::S)
			{
				ScaleFactor = 1;
			}
			else
			{
				ScaleFactor = FMath::RandRange(7, 10);
			}
		}
	}
	else if (WeaponFallback == Catalog->Get(FName("weapon-artemis")))
	{
	}

	if (ScaleFactor == 0)
	{
		if (IsFinalExplosion)
		{
			if (GetParent()->GetSize() == EFlarePartSize::S)
			{
				ScaleFactor = FMath::RandRange(8, 11);
			}
			else
			{
				ScaleFactor = FMath::RandRange(49, 51);
			}
		}
		else
		{
			if (GetParent()->GetSize() == EFlarePartSize::S)
			{
				ScaleFactor = FMath::RandRange(1, 2);
			}
			else
			{
				ScaleFactor = FMath::RandRange(8, 11);
			}
		}
	}

	if (GetDescription()->IsDroneShip)
	{
		ScaleFactor /= 2;
	}
	
	return ScaleFactor;
}

void AFlareSpacecraft::BeginExplodingShip()
{
	if (IsExploding)
	{
		return;
	}

	IsExploding = true;
	TimeSinceLastExplosion = 0;
	ExplodingTimes = 0;
	if (Parent->GetSize() == EFlarePartSize::L)
	{	
		ExplodingTimesMax = FMath::RandRange(10, 20);
	} 
	else if (Parent->GetSize() == EFlarePartSize::S)
	{
		ExplodingTimesMax = FMath::RandRange(3, 5);
	}
}

void AFlareSpacecraft::ExplodingShip()
{
	ExplodingTimes++;

	if (ExplodingTimes >= ExplodingTimesMax)
	{
		IsExploding = false;
		SafeHide();
		return;
	}

	FFlareSpacecraftComponentDescription* DefaultWeaponFallback = GetDefaultWeaponFallback(false);
	if (!DefaultWeaponFallback)
	{
		TimeSinceLastExplosion = 0.25f;
		return;
	}

	UParticleSystem* ExplosionEffectTemplate = DefaultWeaponFallback->WeaponCharacteristics.ExplosionEffect;
	float SingleProbability = FMath::FRand();
	int32 ExplosionsToDo = 1;
	if (SingleProbability > 0.75)
	{
		ExplosionsToDo = FMath::FRandRange(2, 5);
	}

	TimeSinceLastExplosion = FMath::FRandRange((0.25f * ExplosionsToDo), (0.50f * ExplosionsToDo));

	if (ExplosionEffectTemplate)
	{
		FVector ExplosionLocation = FMath::VRand() * FMath::FRand() * 1000 + GetActorLocation();

		if (IsPlayerShip())
		{
			TArray<UActorComponent*> Components = GetComponentsByClass(UPrimitiveComponent::StaticClass());
			if (Components.Num() > 0)
			{
				UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(Components[0]);
				GetPC()->PlayLocalizedSound(DefaultWeaponFallback->WeaponCharacteristics.ImpactSound, ExplosionLocation, Component);
			}
		}

		float MinSize = 1;
		float MaxSize = 1;
		FVector ActorScale = GetActorScale();

		if (GetParent()->GetSize() == EFlarePartSize::S)
		{
		}
		else
		{
			MinSize = 2;
			MaxSize = 6;
		}

		UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
		float DebrisChanceSetting = MyGameSettings->DebrisQuantity / 10;

		while (ExplosionsToDo > 0)
		{
			if (GetGame()->GetActiveSector())
			{
				float ScaleFactor = GetExplosionScaleFactor(DefaultWeaponFallback, false);
				SingleProbability = FMath::FRand();
				if (SingleProbability <= (DebrisChanceSetting - (0.05 * ExplosionsToDo)))
				{
					GetGame()->GetDebrisFieldSystem()->CreateDebris(GetGame()->GetActiveSector()->GetSimulatedSector(), ExplosionLocation, 1, MinSize, MaxSize);
				}

				FRotator Rotation = FRotator(FMath::FRandRange(0, 360), FMath::FRandRange(0, 360), FMath::FRandRange(0, 360));

				UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ExplosionEffectTemplate,
				ExplosionLocation,
				Rotation,
				true,
				EPSCPoolMethod::AutoRelease);
				if (PSC)
				{
					PSC->SetWorldScale3D(ScaleFactor * ActorScale);
				}
				ExplosionLocation = FMath::VRand() * FMath::FRand() * 1000 + GetActorLocation();
			}
			else
			{
				break;
			}
			ExplosionsToDo--;
			DefaultWeaponFallback = GetDefaultWeaponFallback(false);
			ExplosionEffectTemplate = DefaultWeaponFallback->WeaponCharacteristics.ExplosionEffect;
		}
	}
}

void AFlareSpacecraft::SafeHide(bool ShowExplosion)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		AFlareSpacecraft* PlayerShip = PC->GetShipPawn();
		if (PlayerShip && PlayerShip == this)
		{
			//PC in exploding ship, try to change and if fail abort final explosion
			if (!PC->SwitchToNextShip())
			{
				return;
			}
		}
	}

	if (ShowExplosion && GetGame()->GetActiveSector())
	{
		FFlareSpacecraftComponentDescription* DefaultWeaponFallback = GetDefaultWeaponFallback(true);
		if (DefaultWeaponFallback)
		{
			UParticleSystem* ExplosionEffectTemplate = DefaultWeaponFallback->WeaponCharacteristics.ExplosionEffect;
			if (ExplosionEffectTemplate)
			{
				float MinSize = 1;
				float MaxSize = 2;
				int32 Quantity = 1;

				if (GetParent()->GetSize() == EFlarePartSize::S)
				{
					Quantity = FMath::FRandRange(2, 3);
				}
				else
				{
					Quantity = FMath::FRandRange(3, 5);
					MinSize = 3;
					MaxSize = 7;
				}

				UFlareGameUserSettings* MyGameSettings = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings());
				float DebrisChanceSetting = MyGameSettings->DebrisQuantity;
				if (DebrisChanceSetting > 0)
				{
					GetGame()->GetDebrisFieldSystem()->CreateDebris(GetGame()->GetActiveSector()->GetSimulatedSector(), GetActorLocation(), Quantity, MinSize, MaxSize);
				}

				UFlareSpacecraftComponentsCatalog* Catalog = GetGame()->GetShipPartsCatalog();
				FVector ActorScale = GetActorScale();

				bool OmniDirectional = false;
				if (Catalog && (DefaultWeaponFallback == Catalog->Get(FName("weapon-eradicator"))))
				{
					OmniDirectional = true;
				}
				if(OmniDirectional)
				{
					int8 ExplosionsToDo = 4;
					//5 explosions
					while (ExplosionsToDo >= 0)
					{
						float ScaleFactor = GetExplosionScaleFactor(DefaultWeaponFallback, true);
						int32 CurrentRotation = 90 * ExplosionsToDo;
						FRotator Rotation = FRotator(CurrentRotation, CurrentRotation, CurrentRotation);

						UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ExplosionEffectTemplate,
						GetActorLocation(),
						Rotation,
						true,
						EPSCPoolMethod::AutoRelease);

						if (PSC)
						{
							PSC->SetWorldScale3D(ScaleFactor * ActorScale);
						}

						ExplosionsToDo--;
					}
				}
				else
				{
					UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ExplosionEffectTemplate,
						GetActorLocation(),
						GetActorRotation(),
						true,
						EPSCPoolMethod::AutoRelease);

					if (PSC)
					{
						float ScaleFactor = GetExplosionScaleFactor(DefaultWeaponFallback, true);
						PSC->SetWorldScale3D(ScaleFactor * ActorScale);
					}
				}
			}
		}
	}
	SafeDestroy();
}

void AFlareSpacecraft::SafeDestroy()
{
	if (!IsSafeDestroyingRunning)
	{
		if (GetGame()->GetActiveSector() && !GetGame()->GetActiveSector()->GetIsDestroyingSector() && GetGame()->GetActiveSector()->GetSimulatedSector() == GetParent()->GetCurrentSector())
		{
			GetGame()->GetActiveSector()->RemoveSpacecraft(this,true);
		}

		Airframe->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		IsSafeDestroyingRunning = true;
		this->SetActorTickEnabled(false);
		Airframe->SetSimulatePhysics(false);
		this->SetActorHiddenInGame(true);
		this->SetActorEnableCollision(false);

		SetLifeSpan(DELETION_DELAYCRAFT);
		IsExploding = false;

		StopFire();

		if (IsMilitaryArmed())
		{
			GetWeaponsSystem()->DeactivateWeapons();
		}

		ResetCurrentTarget();
		SignalIncomingBombsDeadTarget();
		InSectorSquad.Empty();

		// Notify PC
		if (!IsPresentationMode() && Parent)
		{
			if (Parent->GetActive() == this)
			{
				Parent->SetActiveSpacecraft(NULL);
			}
		}

		UnTrackAllIncomingBombs();

		TArray<UActorComponent*> Components = GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
		for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
		{
			UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
			Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Component->SafeDestroy();
		}

		//disable the collision of former dynamic components (primarily for the shipyard representation of a ship being built) just encase it still exists when entering a sector which would 
		//cause it to collide with the new version of itself. this wouldn't be neccessary if the active spaceship was reused

		TArray<UActorComponent*> DynamicComponents = GetComponentsByClass(UChildActorComponent::StaticClass());
		for (int32 ComponentIndex = 0; ComponentIndex < DynamicComponents.Num(); ComponentIndex++)
		{
			UChildActorComponent* Component = Cast<UChildActorComponent>(DynamicComponents[ComponentIndex]);
			if (Component && Component->GetChildActor())
			{
				Component->GetChildActor()->SetActorEnableCollision(false);
				TArray<UActorComponent*> SubDynamicComponents = Component->GetChildActor()->GetComponentsByClass(UChildActorComponent::StaticClass());
				for (int32 SubComponentIndex = 0; SubComponentIndex < SubDynamicComponents.Num(); SubComponentIndex++)
				{
					UChildActorComponent* SubDynamicComponent = Cast<UChildActorComponent>(SubDynamicComponents[SubComponentIndex]);

					if (SubDynamicComponent->GetChildActor())
					{
						SubDynamicComponent->GetChildActor()->SetActorEnableCollision(false);
					}
				}
			}
		}
	}
}

void AFlareSpacecraft::LifeSpanExpired()
{
	FinishSafeDestroy();
}

void AFlareSpacecraft::FinishSafeDestroy()
{
	BegunSafeDestroy = this->Destroy();
}

void AFlareSpacecraft::Destroyed()
{
	Super::Destroyed();

	// Stop lights
	TArray<UActorComponent*> LightComponents = GetComponentsByClass(USpotLightComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < LightComponents.Num(); ComponentIndex++)
	{
		USpotLightComponent* Component = Cast<USpotLightComponent>(LightComponents[ComponentIndex]);
		if (Component)
		{
			Component->SetActive(false);
		}
	}

	// Clear bombs
	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareWeapon* Weapon = Cast<UFlareWeapon>(Components[ComponentIndex]);
		if (Weapon)
		{
			Weapon->ClearBombs();
		}
	}
	CurrentTarget.Clear();
}

void AFlareSpacecraft::TrackIncomingBomb(AFlareBomb* Bomb)
{
	if (!Bomb)
	{
		return;
	}
	IncomingBombs.AddUnique(Bomb);
	UFlareFleet* CurrentFleet = GetParent()->GetCurrentFleet();
	if (CurrentFleet)
	{
		CurrentFleet->TrackIncomingBomb(Bomb);
	}
}

void AFlareSpacecraft::SignalIncomingBombsDeadTarget()
{
	for (AFlareBomb* Bomb : GetIncomingBombs())
	{
		Bomb->CurrentTargetDied();
		UnTrackIncomingBomb(Bomb);
	}
}

void AFlareSpacecraft::UnTrackAllIncomingBombs()
{
	for (AFlareBomb* Bomb : GetIncomingBombs())
	{
		UnTrackIncomingBomb(Bomb);
	}
}

void AFlareSpacecraft::UnTrackIncomingBomb(AFlareBomb* Bomb)
{
	if (!Bomb)
	{
		return;
	}
	IncomingBombs.RemoveSwap(Bomb);
	UFlareFleet* CurrentFleet = GetParent()->GetCurrentFleet();
	if (CurrentFleet)
	{
		CurrentFleet->UnTrackIncomingBomb(Bomb);
	}
}

//FinishAutoDocks()
void AFlareSpacecraft::FinishAutoPilots()
{
	if (NavigationSystem)
	{
		if (NavigationSystem->IsAutoPilot())
		{
			NavigationSystem->ForceFinishAutoPilots();
			GetData().Location = GetActorLocation();
			GetData().Rotation = GetActorRotation();
		}
	}
	if(GetDescription()->IsDroneShip)
	{
		if (GetParent()->GetShipMaster() && GetParent()->GetShipMaster()->GetActive() &&  !GetParent()->GetShipMaster()->GetActive()->GetWantUndockInternalShips())
		{
			GetParent()->SetInternalDockedTo(GetParent()->GetShipMaster());
		}
	}
}

void AFlareSpacecraft::AddToInsectorSquad(AFlareSpacecraft* Adding)
{
	InSectorSquad.AddUnique(Adding);
}

void AFlareSpacecraft::RemoveFromInsectorSquad(AFlareSpacecraft* Removing)
{
	InSectorSquad.RemoveSwap(Removing);
}

void AFlareSpacecraft::SetPause(bool Pause)
{
	if (Paused == Pause)
	{
		return;
	}

	CustomTimeDilation = (Pause ? 0.f : 1.0);
	if (Pause)
	{
		CurrentTarget.Clear();
		Save(); // Save must be performed with the old pause state
		//FLOGV("%s save linear velocity : %s", *GetImmatriculation().ToString(), *GetData().LinearVelocity.ToString());
	}

	Airframe->SetSimulatePhysics(!Pause);

	Paused = Pause;

	SetActorHiddenInGame(Pause);

	if (!Pause)
	{
		//FLOGV("%s restore linear velocity : %s", *GetImmatriculation().ToString(), *GetData().LinearVelocity.ToString());
		Airframe->SetPhysicsLinearVelocity(GetData().LinearVelocity);
		Airframe->SetPhysicsAngularVelocityInDegrees(GetData().AngularVelocity);
		SmoothedVelocity = GetLinearVelocity();
	}
}

void AFlareSpacecraft::Redock()
{
	// Re-dock if we were docked
	if (GetData().DockedTo != NAME_None && !IsPresentationMode())
	{
		// TODO use sector iterator
		FLOGV("AFlareSpacecraft::Redock : Looking for station '%s'", *GetData().DockedTo.ToString());

		for (int32 SpacecraftIndex = 0; SpacecraftIndex < GetGame()->GetActiveSector()->GetSpacecrafts().Num(); SpacecraftIndex++)
		{
			AFlareSpacecraft* Station = GetGame()->GetActiveSector()->GetSpacecrafts()[SpacecraftIndex];

			if (Station->GetImmatriculation() == GetData().DockedTo)
			{
				FLOGV("AFlareSpacecraft::Redock : Found dock station '%s'", *Station->GetImmatriculation().ToString());

				if(Station->GetDockingSystem()->GetDockCount() <= GetData().DockedAt)
				{
					FLOGV("WARNING !!! AFlareSpacecraft::Redock : invalid port index %d (%d docking port in station)", GetData().DockedAt, Station->GetDockingSystem()->GetDockCount());
					break;
				}

				RedockTo(Station);
				NavigationSystem->ConfirmDock(Station, GetData().DockedAt, false);

				// Replace ship at docking port
/*
				FFlareDockingInfo DockingPort = Station->GetDockingSystem()->GetDockInfo(GetData().DockedAt);
				FFlareDockingParameters DockingParameters = GetNavigationSystem()->GetDockingParameters(DockingPort, FVector::ZeroVector);
				FTransform DockedTransform;
				FQuat DockRotation = FQuat(UKismetMathLibrary::MakeRotFromXZ(-DockingParameters.StationDockAxis, DockingParameters.StationDockTopAxis));
				DockedTransform.SetRotation(DockRotation);

				FRotator RollRotation = FRotator::ZeroRotator;
				RollRotation.Roll = GetData().DockedAngle;
				DockedTransform.ConcatenateRotation(FQuat(RollRotation));

				FVector ShipLocalDockOffset = GetActorTransform().GetRotation().Inverse().RotateVector(DockingParameters.ShipDockOffset);
				FVector RotatedOffset = DockRotation.RotateVector(ShipLocalDockOffset);
				FVector DockLocation = DockingParameters.StationDockLocation - RotatedOffset;
				DockedTransform.SetTranslation(DockLocation);

				SetActorTransform(DockedTransform, false, nullptr, ETeleportType::TeleportPhysics);
				NavigationSystem->ConfirmDock(Station, GetData().DockedAt, false);
*/
				break;
			}
		}
	}
}

void AFlareSpacecraft::RedockTo(AFlareSpacecraft* Station)
{
	FFlareDockingInfo DockingPort = Station->GetDockingSystem()->GetDockInfo(GetData().DockedAt);
	FFlareDockingParameters DockingParameters = GetNavigationSystem()->GetDockingParameters(DockingPort, FVector::ZeroVector);
/*
	FVector ShipDockPosition = GetRootComponent()->GetSocketLocation(FName("Dock"));
	FVector ShipDockOffset =  ShipDockPosition - GetActorLocation();
*/

	FVector LocationTarget = DockingParameters.StationDockLocation - DockingParameters.ShipDockOffset;
	FVector AxisTarget = -DockingParameters.StationDockAxis;
	FRotator AxisRotation = AxisTarget.ToOrientationRotator();
	SetActorLocationAndRotation(LocationTarget, AxisRotation, false, nullptr, ETeleportType::TeleportPhysics);
}

/*
	FFlareDockingInfo DockingPort = Station->GetDockingSystem()->GetDockInfo(GetData().DockedAt);
	FFlareDockingParameters DockingParameters = GetNavigationSystem()->GetDockingParameters(DockingPort, FVector::ZeroVector);

	FVector LocationTarget = DockingParameters.StationDockLocation - DockingParameters.ShipDockOffset;
	FVector AxisTarget = -DockingParameters.StationDockAxis;
	FRotator AxisRotation = AxisTarget.ToOrientationRotator();
	SetActorLocationAndRotation(LocationTarget, AxisRotation, false, nullptr, ETeleportType::TeleportPhysics);

	DockingParameters = GetNavigationSystem()->GetDockingParameters(DockingPort, FVector::ZeroVector);
	LocationTarget = DockingParameters.StationDockLocation - DockingParameters.ShipDockOffset;
	SetActorLocationAndRotation(LocationTarget, AxisRotation, false, nullptr, ETeleportType::TeleportPhysics);
*/

float AFlareSpacecraft::GetSpacecraftMass() const
{
	float Mass = GetDescription()->Mass;
	if (Mass)
	{
		return Mass;
	}
	else if (AttachedToParentActor)
	{
		return Cast<UPrimitiveComponent>(GetAttachParentActor()->GetRootComponent())->GetMass();
	}
	else if (Airframe)
	{
		return Airframe->GetMass();
	}
	else
	{
		return 0;
	}
}


/*----------------------------------------------------
	Player interface
----------------------------------------------------*/

void AFlareSpacecraft::ResetCurrentTarget()
{
	SetCurrentTarget(PilotHelper::PilotTarget());
}

void AFlareSpacecraft::ClearInvalidTarget(PilotHelper::PilotTarget InvalidTarget)
{
	if(CurrentTarget == InvalidTarget)
	{
		ResetCurrentTarget();
	}

	GetPilot()->ClearInvalidTarget(InvalidTarget);

	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);

		UFlareTurret* Turret = Cast<UFlareTurret>(Components[ComponentIndex]);
		if (Turret)
		{
			Turret->GetTurretPilot()->ClearInvalidTarget(InvalidTarget);
		}
	}
}

PilotHelper::PilotTarget AFlareSpacecraft::GetCurrentTarget()
{
	// Crash "preventer" - ensure we've got a really valid target, this isn't a solution, but it seems to only happen when using CreateShip commands
	if (IsValidLowLevel() && CurrentTarget.IsValid())
	{
		return CurrentTarget;
	}
	else
	{
		return PilotHelper::PilotTarget();
	}
}

bool AFlareSpacecraft::IsInScanningMode()
{
	const FFlarePlayerObjectiveData* Objective = GetPC()->GetCurrentObjective();

	// Look for player ship with no weapons
	if (IsPlayerShip() && !GetWeaponsSystem()->GetActiveWeaponGroup())
	{
		// Needs a valid target objective
		if (Objective && Objective->TargetList.Num() > 0 && (Objective->TargetSectors.Num() == 0 || Objective->TargetSectors.Contains(GetParent()->GetCurrentSector())))
		{
			if (Objective->TargetList[0].RequiresScan)
			{
				return true;
			}
		}
		
		// Look for a valid scannable
		else
		{
			TArray<AActor*> ScannableCandidates;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFlareScannable::StaticClass(), ScannableCandidates);
			for (auto ScannableCandidate : ScannableCandidates)
			{
				AFlareScannable* Scannable = Cast<AFlareScannable>(ScannableCandidate);
				if (Scannable && Scannable->IsActive())
				{
					return true;
				}
			}
		}
	}

	return false;
}

void AFlareSpacecraft::GetScanningProgress(bool& AngularIsActive, bool& LinearIsActive, bool& ScanningIsActive,
	float& AngularProgress, float& LinearProgress, float& AnalyzisProgress, float& ScanningDistance, float& ScanningSpeed)
{
	if (IsInScanningMode())
	{
		// Look for an active "scan" objective - legacy pipeline with only one waypoint possible
		// Should be removed, ideally, in favor of the scannable actor... TODO :) 
		if (GetPC()->GetCurrentObjective())
		{
			for (auto Target : GetPC()->GetCurrentObjective()->TargetList)
			{
				if (Target.Active)
				{
					GetScanningProgressInternal(Target.Location, Target.Radius, AngularIsActive, LinearIsActive, ScanningIsActive,
						AngularProgress, LinearProgress, AnalyzisProgress, ScanningDistance, ScanningSpeed);
					
					IsScanningWaypoint = true;

					return;
				}
			}
		}

		// Reset to default values
		AngularIsActive = false;
		LinearIsActive = false;
		ScanningIsActive = false;
		AngularProgress = 0;
		LinearProgress = 0;
		AnalyzisProgress = 0;

		// Look for active scannables and reset scanning after unlocking one
		TArray<AActor*> ScannableCandidates;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFlareScannable::StaticClass(), ScannableCandidates);
		for (auto ScannableCandidate : ScannableCandidates)
		{
			AFlareScannable* Scannable = Cast<AFlareScannable>(ScannableCandidate);
			if (Scannable && Scannable->IsActive())
			{
				GetScanningProgressInternal(Scannable->GetActorLocation(), 10000, AngularIsActive, LinearIsActive, ScanningIsActive,
					AngularProgress, LinearProgress, AnalyzisProgress, ScanningDistance, ScanningSpeed);

				if (AnalyzisProgress >= 1.0f)
				{
					ScanningTimer = 0;
					Scannable->Unlock();
				}

				IsScanningWaypoint = false;

				return;
			}
		}
	}

	// Reset to default values
	AngularIsActive = false;
	LinearIsActive = false;
	ScanningIsActive = false;
	AngularProgress = 0;
	LinearProgress = 0;
	AnalyzisProgress = 0;
}

void AFlareSpacecraft::GetScanningProgressInternal(const FVector& Location, const float Radius, bool& AngularIsActive, bool& LinearIsActive, bool& ScanningIsActive,
	float& AngularProgress, float& LinearProgress, float& AnalyzisProgress, float& ScanningDistance, float& ScanningSpeed)
{
	FVector TargetDisplacement = Location - GetActorLocation();
	float Distance = TargetDisplacement.Size();

	// Angular alignment
	float Alignement = FVector::DotProduct(GetFrontVector(), TargetDisplacement.GetSafeNormal());
	if (Alignement > 0)
	{
		AngularProgress = FMath::Max(0.f, 1.f - FMath::Acos(Alignement));
	}
	else
	{
		AngularProgress = 0;
	}

	// Linear & full progress
	LinearProgress = 1 - FMath::Clamp(Distance / 100000, 0.0f, 1.0f); // 1000m
	AnalyzisProgress = FMath::Clamp(ScanningTimer / ScanningTimerDuration, 0.0f, 1.0f);

	// Conditions
	float const AlignementThreshold = 0.7f;
	AngularIsActive = (AngularProgress > AlignementThreshold);
	LinearIsActive = (Distance < Radius);
	ScanningSpeed = 0.f;
	ScanningDistance = (Distance / 100);

	// Active scan
	ScanningIsActive = AngularIsActive && LinearIsActive;
	if (ScanningIsActive)
	{
		float AlignementRatioB = AlignementThreshold / (1 - AlignementThreshold);
		float AlignementRatio = (1 + AlignementRatioB) *  AngularProgress - AlignementRatioB;
		float DistanceRatio = -(1.f / Radius) * Distance + 1.f;

		ScanningSpeed = 1.f * AlignementRatio * DistanceRatio;
	}
}

bool AFlareSpacecraft::IsWaypointScanningFinished() const
{
	return (IsScanningWaypoint && ScanningTimer >= ScanningTimerDuration);
}

bool AFlareSpacecraft::GetAttachedToParentActor()
{
	return AttachedToParentActor;
}

bool AFlareSpacecraft::GetManualDockingProgress(AFlareSpacecraft*& OutStation, FFlareDockingParameters& OutParameters, FText& OutInfo) const
{
	// No target
	if (IsManualDocking)
	{
		// Dock is occupied, don't draw
		if (ManualDockingInfo.Occupied)
		{
			OutStation = NULL;
			return false;
		}

		// Dock is already used, draw as enemy
		else if (ManualDockingInfo.Granted)
		{
			OutInfo = LOCTEXT("ManualDockingUsed", "This dock is already reserved");
			OutStation = ManualDockingTarget;
			OutParameters = ManualDockingStatus;
			return false;
		}

		// Enemy station, draw as enemy
		else if (ManualDockingTarget->IsPlayerHostile())
		{
			OutInfo = LOCTEXT("ManualDockingEnemy", "This station is hostile");
			OutStation = ManualDockingTarget;
			OutParameters = ManualDockingStatus;
			return false;
		}

		// Docking in progress, draw as OK
		else
		{
			OutInfo = UFlareSpacecraftNavigationSystem::GetDockingPhaseName(ManualDockingStatus.DockingPhase);
			OutStation = ManualDockingTarget;
			OutParameters = ManualDockingStatus;
			return true;
		}
	}
	
	// No docking port detected
	else
	{
		OutStation = NULL;
		return false;
	}
}


/*----------------------------------------------------
	Ship interface
----------------------------------------------------*/

void AFlareSpacecraft::Load(UFlareSimulatedSpacecraft* ParentSpacecraft)
{
	// Update local data
	SetMeshBox = false;
	Parent = ParentSpacecraft;
	if (!IsPresentationMode())
	{
		Airframe->SetSimulatePhysics(true);
		Parent->SetActiveSpacecraft(this);
	}

	if (Parent->GetData().AttachActorName != NAME_None)
	{
		Airframe->SetSimulatePhysics(false);
		Airframe->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	/*FLOGV("AFlareSpacecraft::Load %s", *ParentSpacecraft->GetImmatriculation().ToString());*/

	// Load ship description
	UFlareSpacecraftComponentsCatalog* Catalog = GetGame()->GetShipPartsCatalog();

	// Unload
/*
	if(NavigationSystem)
	{
		NavigationSystem->BreakDock();
	}
*/
	// Initialize damage system
	if (!DamageSystem)
	{
		DamageSystem = NewObject<UFlareSpacecraftDamageSystem>(this, UFlareSpacecraftDamageSystem::StaticClass());
	}
	DamageSystem->Initialize(this, &GetData());

	// Initialize navigation system
	if (!NavigationSystem)
	{
		NavigationSystem = NewObject<UFlareSpacecraftNavigationSystem>(this, UFlareSpacecraftNavigationSystem::StaticClass());
	}
	else
	{
		NavigationSystem->BreakDock();
	}

	NavigationSystem->Initialize(this, &GetData());

	// Initialize docking system
	if (!DockingSystem)
	{
		DockingSystem = NewObject<UFlareSpacecraftDockingSystem>(this, UFlareSpacecraftDockingSystem::StaticClass());
	}
	DockingSystem->Initialize(this, &GetData());

	// Initialize weapons system
	if (!WeaponsSystem)
	{
		WeaponsSystem = NewObject<UFlareSpacecraftWeaponsSystem>(this, UFlareSpacecraftWeaponsSystem::StaticClass());
	}
	WeaponsSystem->Initialize(this, &GetData());

	// Look for parent company
	SetOwnerCompany(ParentSpacecraft->GetCompany());

	// Load dynamic components
	UpdateDynamicComponents();

	// Initialize components
	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
		FFlareSpacecraftComponentSave* ComponentData = NULL;

		// Find component the corresponding component data comparing the slot id
		bool Found = false;
		for (int32 i = 0; i < GetData().Components.Num(); i++)
		{
			if (Component->SlotIdentifier == GetData().Components[i].ShipSlotIdentifier)
			{
				ComponentData = &GetData().Components[i];
				Found = true;
				break;
			}
		}

		// If no data, this is a cosmetic component and it don't need to be initialized
		if (!Found)
		{
			continue;
		}

		// Reload the component
		ReloadPart(Component, ComponentData);

		// Set RCS description
		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);
		if (ComponentDescription->Type == EFlarePartType::RCS)
		{
			SetRCSDescription(ComponentDescription);
		}

		// Set orbital engine description
		else if (ComponentDescription->Type == EFlarePartType::OrbitalEngine)
		{
			SetOrbitalEngineDescription(ComponentDescription);
		}

		// Find the cockpit
		if (ComponentDescription->GeneralCharacteristics.LifeSupport)
		{
			ShipCockit = Component;
		}
	}

	// Look for an asteroid component
	ApplyAsteroidData();

	// Setup ship name texture target
	if (!ShipNameTexture)
	{
		ShipNameTexture = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), 384, 384);// 256, 256);
		FCHECK(ShipNameTexture);
		ShipNameTexture->OnCanvasRenderTargetUpdate.AddDynamic(this, &AFlareSpacecraft::DrawShipName);
	}
	ShipNameTexture->ClearColor = FLinearColor::Black;

	// Customization
	UpdateCustomization();

	// If not rcs, add passive stabilization
	if (GetDescription()->RCSCount == 0)
	{
		Airframe->SetLinearDamping(0.1);
		Airframe->SetAngularDamping(0.1);
	}

	// Initialize pilot
	if (!Pilot)
	{
		Pilot = NewObject<UFlareShipPilot>(this, UFlareShipPilot::StaticClass());
	}

	Pilot->Initialize(&GetData().Pilot, GetCompany(), this);

	if (!StateManager)
	{
		StateManager = NewObject<UFlareSpacecraftStateManager>(this, UFlareSpacecraftStateManager::StaticClass());
	}

	StateManager->Initialize(this);
	
	// Subsystems
	DamageSystem->Start();
	NavigationSystem->Start();
	DockingSystem->Start();
	WeaponsSystem->Start();
	DamageSystem->CalculateInitialHeat();

	SmoothedVelocity = GetLinearVelocity();

	if (IsPaused())
	{
		Airframe->SetSimulatePhysics(false);
	}

	CurrentTarget.Clear();
}

void AFlareSpacecraft::FinishLoadandReady()
{

	Redock();
	// Attach to parent actor, if any
	if (GetData().AttachActorName != NAME_None && !GetAttachedToParentActor())
	{
		TryAttachParentActor();
	}

	// Attach to parent complex station, if any
	if (IsComplexElement() && !GetAttachedToParentActor())
	{
		TryAttachParentComplex();
	}

	LoadedAndReady = true;
}

void AFlareSpacecraft::Save()
{
	// Physical data
	GetData().Location = GetActorLocation();
	GetData().Rotation = GetActorRotation();
	if (!IsPaused())
	{
		GetData().LinearVelocity = Airframe->GetPhysicsLinearVelocity();
		GetData().AngularVelocity = Airframe->GetPhysicsAngularVelocityInDegrees();
	}

	// Save all components datas
	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
		Component->Save();
	}
}

void AFlareSpacecraft::SetOwnerCompany(UFlareCompany* NewCompany)
{
	SetCompany(NewCompany);

	AFlarePlayerController* PC = GetGame()->GetPC();
	if (PC)
	{
		Airframe->Initialize(NULL, Company, this, false, PC->GetShipPawn());
	}
	else
	{
		Airframe->Initialize(NULL, Company, this, false, nullptr);
	}
}

UFlareSpacecraftDamageSystem* AFlareSpacecraft::GetDamageSystem() const
{
	return DamageSystem;
}

UFlareSpacecraftNavigationSystem* AFlareSpacecraft::GetNavigationSystem() const
{
	return NavigationSystem;
}

UFlareSpacecraftWeaponsSystem* AFlareSpacecraft::GetWeaponsSystem() const
{
	return WeaponsSystem;
}

UFlareSpacecraftDockingSystem* AFlareSpacecraft::GetDockingSystem() const
{
	return DockingSystem;
}

void AFlareSpacecraft::SetAsteroidData(FFlareAsteroidSave* Data)
{
	// Copy data
	GetData().AsteroidData.Identifier = Data->Identifier;
	GetData().AsteroidData.AsteroidMeshID = Data->AsteroidMeshID;
	GetData().AsteroidData.Scale = Data->Scale;

	// Apply
	ApplyAsteroidData();
	SetActorLocationAndRotation(Data->Location, Data->Rotation);
}

void AFlareSpacecraft::TryAttachParentComplex()
{
	// Find station
	AFlareSpacecraft* AttachStation = GetComplex();

	// Attach to station
	if (AttachStation)
	{
		for (FFlareDockingInfo& MasterConnector : AttachStation->GetParent()->GetStationConnectors())
		{
			FCHECK(GetParent()->GetStationConnectors().Num() > 0);
			FFlareDockingInfo& SlaveConnector = GetParent()->GetStationConnectors()[0];

			// "Occupied" is set to true when the dock is ready
			if (MasterConnector.Name == GetData().AttachComplexConnectorName && MasterConnector.Occupied && SlaveConnector.Occupied)
			{
				FCHECK(MasterConnector.Granted);
				FCHECK(MasterConnector.ConnectedStationName == GetImmatriculation());

				FVector MasterDockLocation =  AttachStation->Airframe->GetComponentTransform().TransformPosition(MasterConnector.LocalLocation);
				FVector SlaveDockLocation =  Airframe->GetComponentTransform().TransformPosition(SlaveConnector.LocalLocation);

				FVector SlaveDockOffset = SlaveDockLocation - GetActorLocation();

				FVector MasterDockAxis = AttachStation->Airframe->GetComponentToWorld().GetRotation().RotateVector(MasterConnector.LocalAxis).GetUnsafeNormal();
				FVector MasterDockTopAxis = AttachStation->Airframe->GetComponentToWorld().GetRotation().RotateVector(MasterConnector.LocalTopAxis).GetUnsafeNormal();

				FTransform DockedTransform;
				FQuat MasterDockRotation = FQuat(UKismetMathLibrary::MakeRotFromXZ(-MasterDockAxis, MasterDockTopAxis));
				FQuat SlaveLocalDockRotation = FQuat(UKismetMathLibrary::MakeRotFromXZ(SlaveConnector.LocalAxis, SlaveConnector.LocalTopAxis));

				FQuat DockRotation = MasterDockRotation * SlaveLocalDockRotation.Inverse();

				DockedTransform.SetRotation(DockRotation);

				FVector ShipLocalDockOffset = GetActorTransform().GetRotation().Inverse().RotateVector(SlaveDockOffset);
				FVector RotatedOffset = DockRotation.RotateVector(ShipLocalDockOffset);
				FVector DockLocation = MasterDockLocation - RotatedOffset;
				DockedTransform.SetTranslation(DockLocation);

				SetActorTransform(DockedTransform, false, nullptr, ETeleportType::TeleportPhysics);

				// Attach
				Airframe->SetSimulatePhysics(true);
				Airframe->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				AttachToActor(AttachStation, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
				AttachedToParentActor = true;
			}
		}

	}
}

void AFlareSpacecraft::TryAttachParentActor()
{
	// Find actor
	AActor* AttachActor = NULL;
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if ((*ActorItr)->GetName() == GetData().AttachActorName.ToString())
		{
			AttachActor = *ActorItr;
			break;
		}
	}

	// Attach to actor
	if (AttachActor)
	{
		Airframe->SetSimulatePhysics(true);
		Airframe->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AttachToActor(AttachActor, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
		AttachedToParentActor = true;
	}
}

void AFlareSpacecraft::ApplyAsteroidData()
{
	if (GetData().AsteroidData.Identifier != NAME_None)
	{
		TArray<UActorComponent*> Components = GetComponentsByClass(UActorComponent::StaticClass());
		for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
		{
			UActorComponent* Component = Components[ComponentIndex];
			if (Component->GetName().Contains("Asteroid"))
			{
				//FLOGV("AFlareSpacecraft::ApplyAsteroidData : Found asteroid component '%s', previously set from '%s'",
				//	*Component->GetName(), *GetData().AsteroidData.Identifier.ToString());

				// Get a valid sector
				UFlareSimulatedSector* Sector = GetParent()->GetCurrentSector();
				if (!Sector)
				{
					Sector = GetOwnerSector();
				}

				// Setup the asteroid
				bool IsIcy = Sector->GetDescription()->IsIcy;
				UStaticMeshComponent* AsteroidComponentCandidate = Cast<UStaticMeshComponent>(Component);
				AFlareAsteroid::SetupAsteroidMesh(GetGame(), AsteroidComponentCandidate, GetData().AsteroidData, IsIcy);
				break;
			}
		}
	}
}

UFlareSimulatedSector* AFlareSpacecraft::GetOwnerSector()
{
	if (GetGame()->GetGameWorld())
	{
		TArray<UFlareSimulatedSector*> Sectors = GetGame()->GetGameWorld()->GetSectors();

		for (int32 Index = 0; Index < Sectors.Num(); Index++)
		{
			UFlareSimulatedSector* CandidateSector = Sectors[Index];
			for (int32 ShipIndex = 0; ShipIndex < CandidateSector->GetSectorSpacecrafts().Num(); ShipIndex++)
			{
				if (CandidateSector->GetSectorSpacecrafts()[ShipIndex]->Save()->Identifier == GetData().Identifier)
				{
					return CandidateSector;
				}
			}
		}
	}

	return NULL;
}

void AFlareSpacecraft::UpdateDynamicComponents()
{
	UClass* ConstructionTemplate = NULL;

	// Get construction template from target ship class
	FFlareSpacecraftDescription* TargetShipDescription = GetGame()->GetSpacecraftCatalog()->Get(GetData().DynamicComponentStateIdentifier);
	if (TargetShipDescription)
	{
		TArray<UClass*>& StateTemplates = TargetShipDescription->ConstructionStateTemplates;
		if (StateTemplates.Num())
		{
			int32 TemplateIndex = FMath::Clamp((int32)(GetData().DynamicComponentStateProgress * StateTemplates.Num()), 0, StateTemplates.Num() - 1);
			ConstructionTemplate = StateTemplates[TemplateIndex];
		}
		else if (GetData().DynamicComponentStateIdentifier != FName("idle"))
		{
			FLOGV("AFlareSpacecraft::UpdateDynamicComponents : no construction state template for '%s'", *GetData().DynamicComponentStateIdentifier.ToString());
		}
	}
	
	// Apply construction template to all dynamic components
	TArray<UActorComponent*> DynamicComponents = GetComponentsByClass(UChildActorComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < DynamicComponents.Num(); ComponentIndex++)
	{
		UChildActorComponent* Component = Cast<UChildActorComponent>(DynamicComponents[ComponentIndex]);
		if (Component)
		{
			// Apply template
			if (ConstructionTemplate)
			{
//check current child actor class because otherwise it will still reapply the child actor and the relative position can become ever increasingly offset
				if (Component->GetChildActorClass() != ConstructionTemplate)
				{
					Component->SetChildActorClass(ConstructionTemplate);
				}
			}
			else
			{
				if (Component->GetChildActorClass() != IdleShipyardTemplate)
				{
					Component->SetChildActorClass(IdleShipyardTemplate);
				}
			}

			// Setup children
			if (Component->GetChildActor())
			{
				TArray<UActorComponent*> SubDynamicComponents = Component->GetChildActor()->GetComponentsByClass(UChildActorComponent::StaticClass());
				for (int32 SubComponentIndex = 0; SubComponentIndex < SubDynamicComponents.Num(); SubComponentIndex++)
				{
					UChildActorComponent* SubDynamicComponent = Cast<UChildActorComponent>(SubDynamicComponents[SubComponentIndex]);

					if (SubDynamicComponent->GetChildActor())
					{
						SubDynamicComponent->GetChildActor()->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true), NAME_None);
						SubDynamicComponent->GetChildActor()->SetOwner(this);

						UFlareSpacecraftComponent* ChildRootComponent = Cast<UFlareSpacecraftComponent>(SubDynamicComponent->GetChildActor()->GetRootComponent());
						if (ChildRootComponent)
						{
							FFlareSpacecraftComponentSave Data;
							Data.Damage = 0;
							Data.ComponentIdentifier = NAME_None;
							ChildRootComponent->Initialize(&Data, Company, this, false, this);
						}
					}
				}

				SubDynamicComponents = Component->GetChildActor()->GetComponentsByClass(UStaticMeshComponent::StaticClass());
				for (int32 SubComponentIndex = 0; SubComponentIndex < SubDynamicComponents.Num(); SubComponentIndex++)
				{
					UStaticMeshComponent* SubDynamicComponent = Cast<UStaticMeshComponent>(SubDynamicComponents[SubComponentIndex]);

					if (SubDynamicComponent)
					{
						USceneComponent* ParentDefaultAttachComponent = GetDefaultAttachComponent();
						if (ParentDefaultAttachComponent)
						{
							SubDynamicComponent->AttachToComponent(ParentDefaultAttachComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true), NAME_None);
						}
					}
				}
			}
		}
	}
}

UFlareInternalComponent* AFlareSpacecraft::GetInternalComponentAtLocation(FVector Location) const
{
	float MinDistance = 100000; // 1km
	UFlareInternalComponent* ClosestComponent = NULL;

	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareInternalComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareInternalComponent* InternalComponent = Cast<UFlareInternalComponent>(Components[ComponentIndex]);

		FVector ComponentLocation;
		float ComponentSize;
		InternalComponent->GetBoundingSphere(ComponentLocation, ComponentSize);

		float Distance = (ComponentLocation - Location).Size() - ComponentSize;
		if (Distance < MinDistance)
		{
			ClosestComponent = InternalComponent;
			MinDistance = Distance;
		}
	}
	return ClosestComponent;
}


/*----------------------------------------------------
	Customization
----------------------------------------------------*/

void AFlareSpacecraft::SetOrbitalEngineDescription(FFlareSpacecraftComponentDescription* Description)
{
	OrbitalEngineDescription = Description;
}

void AFlareSpacecraft::SetRCSDescription(FFlareSpacecraftComponentDescription* Description)
{
	RCSDescription = Description;

	// Find the RCS turn and power rating, since RCSs themselves don't do anything
	if (Description)
	{
		if (Airframe && Description->EngineCharacteristics.AngularAccelerationRate > 0 && !IsPresentationMode())
		{
			float Mass = GetSpacecraftMass() / 100000;
			NavigationSystem->SetAngularAccelerationRate(Description->EngineCharacteristics.AngularAccelerationRate / (60 * Mass));
		}
	}
}

void AFlareSpacecraft::UpdateCustomization()
{
	// Send the update event to subsystems
	Super::UpdateCustomization();
	Airframe->UpdateCustomization();
	if (ShipNameTexture)
	{
		ShipNameTexture->UpdateResource();
	}

	// Customize lights
	TArray<UActorComponent*> LightComponents = GetComponentsByClass(USpotLightComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < LightComponents.Num(); ComponentIndex++)
	{
		USpotLightComponent* Component = Cast<USpotLightComponent>(LightComponents[ComponentIndex]);
		if (Component)
		{
			FLinearColor LightColor = UFlareSpacecraftComponent::NormalizeColor(Company->GetLightColor());
			LightColor = LightColor.Desaturate(0.5);
			Component->SetLightColor(LightColor);
		}
	}

	// Customize decal materials
	TArray<UActorComponent*> DecalComponents = GetComponentsByClass(UDecalComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < DecalComponents.Num(); ComponentIndex++)
	{
		UDecalComponent* Component = Cast<UDecalComponent>(DecalComponents[ComponentIndex]);
		if (Component)
		{
			// Ship name decal
			if (Component->GetName().Contains("ShipNameDecal"))
			{
				if (ShipNameTexture && !ShipNameDecalMaterial)
				{
					ShipNameDecalMaterial = UMaterialInstanceDynamic::Create(Component->GetMaterial(0), GetWorld());
				}
				if (Company && ShipNameDecalMaterial)
				{
					FLinearColor BasePaintColor = Company->GetPaintColor();
					FLinearColor ShipNameColor = (BasePaintColor.GetLuminance() > 0.5) ? FLinearColor::Black : FLinearColor::White;
					ShipNameDecalMaterial->SetVectorParameterValue("NameColor", ShipNameColor);
					ShipNameDecalMaterial->SetTextureParameterValue("NameTexture", ShipNameTexture);
					Component->SetMaterial(0, ShipNameDecalMaterial);
				}
			}

			// Company decal
			else
			{
				if (!DecalMaterial)
				{
					DecalMaterial = UMaterialInstanceDynamic::Create(Component->GetMaterial(0), GetWorld());
				}
				if (Company && DecalMaterial)
				{
					Company->CustomizeMaterial(DecalMaterial);
					Component->SetMaterial(0, DecalMaterial);
				}
			}
		}
	}
}

void AFlareSpacecraft::StartPresentation()
{
	Super::StartPresentation();

	if (Airframe)
	{
		Airframe->SetSimulatePhysics(false);
		Airframe->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	CurrentTarget.Clear();
}

void AFlareSpacecraft::DrawShipName(UCanvas* TargetCanvas, int32 Width, int32 Height)
{
	if (TargetCanvas)
	{
		// Cleanup immatriculation on capitals
		bool HidePrefix = GetDescription()->Size == EFlarePartSize::L;
		FText Text = UFlareGameTools::DisplaySpacecraftName(GetParent(), HidePrefix, true);
		
		// Centering
		float XL, YL;
		TargetCanvas->TextSize(ShipNameFont, Text.ToString(), XL, YL);
		float X = (TargetCanvas->ClipX / 2.0f) - (XL / 2.0f);
		float Y = (TargetCanvas->ClipY / 2.0f) - (YL / 2.0f);

		// Drawing
		FCanvasTextItem TextItem(FVector2D(X, Y), Text, ShipNameFont, FLinearColor::White);
		TextItem.Scale = FVector2D(0.90f, 0.90f);
		TextItem.bOutlined = true;
		TextItem.OutlineColor = FLinearColor::Green;
		TargetCanvas->DrawItem(TextItem);
	}
}


/*----------------------------------------------------
		Damage system
----------------------------------------------------*/

void AFlareSpacecraft::OnRepaired()
{
	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);
		Component->OnRepaired();
	}
}

void AFlareSpacecraft::OnRefilled()
{
	// Reload and repair
	TArray<UActorComponent*> Components = GetComponentsByClass(UFlareSpacecraftComponent::StaticClass());
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UFlareSpacecraftComponent* Component = Cast<UFlareSpacecraftComponent>(Components[ComponentIndex]);

		UFlareWeapon* Weapon = Cast<UFlareWeapon>(Components[ComponentIndex]);
		if (Weapon)
		{
			Weapon->OnRefilled();
		}
	}
}

void AFlareSpacecraft::OnDocked(AFlareSpacecraft* DockStation, bool TellUser, FFlareResourceDescription* TransactionResource, uint32 TransactionQuantity, UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, bool TransactionDonation, FFlareSpacecraftComponentDescription* TransactionNewPartDesc, int32 TransactionNewPartWeaponGroupIndex)
{
	// Signal the PC
	AFlarePlayerController* PC = GetPC();
	bool FlewByPlayer = false;
	if (PC && IsFlownByPlayer())
	{
		FlewByPlayer = true;
		PC->NotifyDockingComplete(DockStation, TellUser);
	}

	GetData().Location = GetActorLocation();
	GetData().Rotation = GetActorRotation();

	if (TransactionNewPartDesc&&TransactionNewPartWeaponGroupIndex>=0)
		// this ship was told to do an insector upgrade
	{
		UFlareSimulatedSpacecraft* ShipSimmed = GetParent();
		bool SuccessfulUpgrade = ShipSimmed->UpgradePart(TransactionNewPartDesc, TransactionNewPartWeaponGroupIndex);
		if (SuccessfulUpgrade)
		{
			FText UpgradeStatus = FText(LOCTEXT("UpgradeInfoSuccess", "Upgraded"));

			FText FirstHalf;

			if (!FlewByPlayer)
			{
				UFlareSimulatedSpacecraft* StationSimmed = DockStation->GetParent();
				FirstHalf = FText::Format(LOCTEXT("LocalUpgradeFormat", "{0} is now docked at {1}\n"),
					UFlareGameTools::DisplaySpacecraftName(ShipSimmed),
					UFlareGameTools::DisplaySpacecraftName(StationSimmed)
				);
			}

			FText Formatted = FText::Format(LOCTEXT("LocalUpgradeSuccessInfo", "{0}{1} {2}"),
				FirstHalf,
				UpgradeStatus,
				TransactionNewPartDesc->Name);

			if (GetParent()->GetCompany() == PC->GetCompany())
			{
				PC->Notify(
					LOCTEXT("RemoteUpgradeSuccessTitle", "Remote Upgrade Info"),
					Formatted,
					"upgrade-success",
					EFlareNotification::NT_Info);
			}
		}
		else
		{
			FText Formatted = FText::Format(LOCTEXT("LocalUpgradeFailInfo", "Failed to upgrade {0}"),
			TransactionNewPartDesc->Name);

			if (GetParent()->GetCompany() == PC->GetCompany())
			{
				PC->Notify(
					LOCTEXT("RemoteUpgradeFail", "Remote Upgrade Info"),
					Formatted,
					"upgrade-fail",
					EFlareNotification::NT_Info);
			}
		}
	}

	else if (TransactionResource&&TransactionQuantity&&SourceSpacecraft&&DestinationSpacecraft)
		// this ship was told to do a manual insector trade
	{
		UFlareSimulatedSpacecraft* ShipSimmed = GetParent();
		UFlareSimulatedSpacecraft* StationSimmed = DockStation->GetParent();

		int32 Quantity = SectorHelper::Trade(
			SourceSpacecraft,
			DestinationSpacecraft,
			TransactionResource,
			TransactionQuantity,
			NULL, NULL, TransactionDonation);

		if (GetParent()->GetCompany() == PC->GetCompany())
		{

			FText TradeStatus = FText();

			if (PC->GetCompany() == StationSimmed->GetCompany())
			{
				TradeStatus = FText(LOCTEXT("TradeInfoTransfer", "Traded"));
			}

			else if (DestinationSpacecraft == StationSimmed)
			{
				if (TransactionDonation)
				{
					TradeStatus = FText(LOCTEXT("TradeInfoSell", "Donated"));
				}
				else
				{
					TradeStatus = FText(LOCTEXT("TradeInfoSell", "Sold"));
				}
			}
			else
			{
				TradeStatus = FText(LOCTEXT("TradeInfoSell", "Bought"));
			}

			FText FirstHalf;

			if (!FlewByPlayer)
			{
				FirstHalf = FText::Format(LOCTEXT("LocalTradeFormat", "{0} is now docked at {1}\n"),
					UFlareGameTools::DisplaySpacecraftName(ShipSimmed),
					UFlareGameTools::DisplaySpacecraftName(StationSimmed)
				);
			}

			FText Formatted = FText::Format(LOCTEXT("LocalTradeSuccess", "{0}{1} {2} {3}"),
				FirstHalf,
				TradeStatus,
				FText::AsNumber(Quantity),
				TransactionResource->Name);

			PC->Notify(
				LOCTEXT("RemoteTradeSuccess", "Remote Trade Info"),
				Formatted,
				"trade-success",
				EFlareNotification::NT_Info);
		}

		AFlareMenuManager* PlayerMenu = PC->GetMenuManager();

		if (PlayerMenu && PlayerMenu->IsMenuOpen() && PlayerMenu->GetCurrentMenu() == EFlareMenu::MENU_Trade)
		{
/*
			TransactionResource = NULL;
			TransactionQuantity = NULL;
			SourceSpacecraft = NULL;
			DestinationSpacecraft = NULL;
			TransactionDonation = 0;
*/
			UFlareSimulatedSpacecraft* TradeMenuLeftTargetShip = PlayerMenu->GetTradeMenu()->GetTargetLeftShip();
			UFlareSimulatedSpacecraft* TradeMenuRightTargetShip = PlayerMenu->GetTradeMenu()->GetTargetRightShip();

			if (TradeMenuLeftTargetShip == ShipSimmed || TradeMenuLeftTargetShip == StationSimmed || TradeMenuRightTargetShip == ShipSimmed || TradeMenuRightTargetShip == StationSimmed)
			{
				PlayerMenu->GetTradeMenu()->GetRefreshTradeBlocks();
			}
		}
	}
}

void AFlareSpacecraft::OnUndocked(AFlareSpacecraft* DockStation)
{
	// Signal the PC
	AFlarePlayerController* PC = GetGame()->GetPC();
	if (IsFlownByPlayer() && PC)
	{
		if (StateManager->IsExternalCamera())
		{
			PC->SetExternalCamera(false);
		}

		PC->Notify(
			LOCTEXT("Undocked", "Undocked"),
			FText::Format(LOCTEXT("UndockedInfoFormat", "Undocked from {0}"), UFlareGameTools::DisplaySpacecraftName(DockStation->GetParent())),
			"undocking-success",
			EFlareNotification::NT_Info);
	}
}


/*----------------------------------------------------
	Input
----------------------------------------------------*/

void AFlareSpacecraft::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	FCHECK(InputComponent);

	PlayerInputComponent->BindAxis("NormalYawInput", this, &AFlareSpacecraft::YawInput);
	PlayerInputComponent->BindAxis("NormalPitchInput", this, &AFlareSpacecraft::PitchInput);
	PlayerInputComponent->BindAxis("NormalRollInput", this, &AFlareSpacecraft::RollInput);
	PlayerInputComponent->BindAxis("NormalThrustInput", this, &AFlareSpacecraft::ThrustInput);

	PlayerInputComponent->BindAxis("JoystickRollInput", this, &AFlareSpacecraft::JoystickRollInput);
	PlayerInputComponent->BindAxis("JoystickThrustInput", this, &AFlareSpacecraft::JoystickThrustInput);

	PlayerInputComponent->BindAxis("MoveVerticalInput", this, &AFlareSpacecraft::MoveVerticalInput);
	PlayerInputComponent->BindAxis("MoveHorizontalInput", this, &AFlareSpacecraft::MoveHorizontalInput);

	PlayerInputComponent->BindAction("ZoomIn", EInputEvent::IE_Released, this, &AFlareSpacecraft::ZoomIn);
	PlayerInputComponent->BindAction("ZoomOut", EInputEvent::IE_Released, this, &AFlareSpacecraft::ZoomOut);
	PlayerInputComponent->BindAction("ZoomIn", EInputEvent::IE_Repeat, this, &AFlareSpacecraft::ZoomIn);
	PlayerInputComponent->BindAction("ZoomOut", EInputEvent::IE_Repeat, this, &AFlareSpacecraft::ZoomOut);

	PlayerInputComponent->BindAction("CombatZoom", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::CombatZoomIn);
	PlayerInputComponent->BindAction("CombatZoom", EInputEvent::IE_Released, this, &AFlareSpacecraft::CombatZoomOut);

	PlayerInputComponent->BindAction("LaunchRetrieveDrones", EInputEvent::IE_Released, this, &AFlareSpacecraft::LaunchRetrieveDrones);

	PlayerInputComponent->BindAction("FaceForward", EInputEvent::IE_Released, this, &AFlareSpacecraft::FaceForward);
	PlayerInputComponent->BindAction("FaceBackward", EInputEvent::IE_Released, this, &AFlareSpacecraft::FaceBackward);
	PlayerInputComponent->BindAction("Brake", EInputEvent::IE_Released, this, &AFlareSpacecraft::Brake);
	PlayerInputComponent->BindAction("LockDirection", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::LockDirectionOn);
	PlayerInputComponent->BindAction("LockDirection", EInputEvent::IE_Released, this, &AFlareSpacecraft::LockDirectionOff);

	PlayerInputComponent->BindAction("StartFire", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::StartFire);
	PlayerInputComponent->BindAction("StartFire", EInputEvent::IE_Released, this, &AFlareSpacecraft::StopFire);

	PlayerInputComponent->BindAction("FindTarget", EInputEvent::IE_Released, this, &AFlareSpacecraft::FindTarget);

	PlayerInputComponent->BindAction("LeftMouse", EInputEvent::IE_Pressed, this, &AFlareSpacecraft::LeftMousePress);
	PlayerInputComponent->BindAction("LeftMouse", EInputEvent::IE_Released, this, &AFlareSpacecraft::LeftMouseRelease);

	PlayerInputComponent->BindAction("NextWeapon", EInputEvent::IE_Released, this, &AFlareSpacecraft::NextWeapon);
	PlayerInputComponent->BindAction("PreviousWeapon", EInputEvent::IE_Released, this, &AFlareSpacecraft::PreviousWeapon);

	PlayerInputComponent->BindAction("NextTarget", EInputEvent::IE_Released, this, &AFlareSpacecraft::NextTarget);
	PlayerInputComponent->BindAction("PreviousTarget", EInputEvent::IE_Released, this, &AFlareSpacecraft::PreviousTarget);

	PlayerInputComponent->BindAction("AlternateNextTarget", EInputEvent::IE_Released, this, &AFlareSpacecraft::AlternateNextTarget);
	PlayerInputComponent->BindAction("AlternatePreviousTarget", EInputEvent::IE_Released, this, &AFlareSpacecraft::AlternatePreviousTarget);
}

void AFlareSpacecraft::StartFire()
{
	StateManager->SetPlayerFiring(true);
}

void AFlareSpacecraft::StopFire()
{
	StateManager->SetPlayerFiring(false);
}

void AFlareSpacecraft::LeftMousePress()
{
	StateManager->SetPlayerLeftMouse(true);
}

void AFlareSpacecraft::LeftMouseRelease()
{
	StateManager->SetPlayerLeftMouse(false);
}

void AFlareSpacecraft::DeactivateWeapon()
{
	if (IsMilitaryArmed())
	{
		FLOG("AFlareSpacecraft::DeactivateWeapon");
		GetWeaponsSystem()->DeactivateWeapons();
		GetStateManager()->EnablePilot(false);

		if (GetParent() == GetGame()->GetPC()->GetPlayerShip())
		{
			// Fighters have an unloading sound
			if (GetDescription()->Size == EFlarePartSize::S)
			{
				if (GetWeaponsSystem()->GetActiveWeaponGroup())
				{
					GetPC()->ClientPlaySound(WeaponUnloadedSound);
				}
			}
			if (GetGame()->GetQuestManager())
			{
				GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("deactivate-weapon"));
			}
		}
	}

	if (GetParent() == GetGame()->GetPC()->GetPlayerShip())
	{
		GetPC()->SetSelectingWeapon();
	}
}

void AFlareSpacecraft::ActivateWeaponGroup1()
{
	ActivateWeaponGroupByIndex(0);
}

void AFlareSpacecraft::ActivateWeaponGroup2()
{
	ActivateWeaponGroupByIndex(1);
}

void AFlareSpacecraft::ActivateWeaponGroup3()
{
	ActivateWeaponGroupByIndex(2);
}

void AFlareSpacecraft::ActivateWeaponGroupByIndex(int32 Index)
{
	FLOGV("AFlareSpacecraft::ActivateWeaponGroup : %d", Index);

	if (IsMilitaryArmed() && Index < GetWeaponsSystem()->GetWeaponGroupCount())
	{
		GetStateManager()->EnablePilot(false);

		// Fighters have a loading sound
		if (GetDescription()->Size == EFlarePartSize::S)
		{
			if (Index != GetWeaponsSystem()->GetActiveWeaponGroupIndex())
			{
				GetPC()->ClientPlaySound(WeaponLoadedSound);
			}
		}

		// Change group
		GetWeaponsSystem()->ActivateWeaponGroup(Index);
		if (GetWeaponsSystem()->GetActiveWeaponType() != EFlareWeaponGroupType::WG_NONE)
		{
			StateManager->SetExternalCamera(false);
		}

		if (GetGame()->GetQuestManager() && GetParent() == GetGame()->GetPC()->GetPlayerShip())
		{
			GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("activate-weapon").PutInt32("index", Index));
		}
	}

	GetPC()->SetSelectingWeapon();
}

void AFlareSpacecraft::NextWeapon()
{
	UFlareSpacecraftWeaponsSystem* WeaponSystems = GetWeaponsSystem();
	
	if (WeaponSystems && IsMilitaryArmed())
	{
		int32 CurrentIndex = WeaponSystems->GetActiveWeaponGroupIndex() + 1;
		CurrentIndex = FMath::Clamp(CurrentIndex, 0, WeaponSystems->GetWeaponGroupCount() - 1);
		FLOGV("AFlareSpacecraft::NextWeapon : %d", CurrentIndex);

		ActivateWeaponGroupByIndex(CurrentIndex);
	}
}

void AFlareSpacecraft::PreviousWeapon()
{
	UFlareSpacecraftWeaponsSystem* WeaponSystems = GetWeaponsSystem();
	
	// Fighter
	if (WeaponSystems)
	{
		int32 CurrentIndex = WeaponSystems->GetActiveWeaponGroupIndex() - 1;
		CurrentIndex = FMath::Clamp(CurrentIndex, -1, WeaponSystems->GetWeaponGroupCount() - 1);
		FLOGV("AFlareSpacecraft::NextWeapon : %d", CurrentIndex);

		if (CurrentIndex >= 0)
		{
			ActivateWeaponGroupByIndex(CurrentIndex);
		}
		else
		{
			DeactivateWeapon();
		}
	}
}

void AFlareSpacecraft::NextTarget()
{
	// Data
	TArray<FFlareScreenTarget>& ScreenTargets = GetCurrentTargets();
	auto FindCurrentTarget = [=](const FFlareScreenTarget& Candidate)
	{
		return Candidate.Spacecraft == CurrentTarget.SpacecraftTarget;
	};

	// Is visible on screen
	if (TimeSinceSelection < MaxTimeBeforeSelectionReset && ScreenTargets.FindByPredicate(FindCurrentTarget))
	{
		TargetIndex++;
		TargetIndex = FMath::Min(TargetIndex, ScreenTargets.Num() - 1);
		SetCurrentTarget(ScreenTargets[TargetIndex].Spacecraft);
		FLOGV("AFlareSpacecraft::NextTarget : %d", TargetIndex);
	}

	// Else reset
	else
	{
		TargetIndex = 0;
		SetCurrentTarget(PilotHelper::PilotTarget());
		FLOG("AFlareSpacecraft::NextTarget : reset to center");
	}

	TimeSinceSelection = 0;
}

void AFlareSpacecraft::PreviousTarget()
{
	// Data
	TArray<FFlareScreenTarget>& ScreenTargets = GetCurrentTargets();
	auto FindCurrentTarget = [=](const FFlareScreenTarget& Candidate)
	{
		return Candidate.Spacecraft == CurrentTarget.SpacecraftTarget;
	};

	// Is visible on screen
	if (TimeSinceSelection < MaxTimeBeforeSelectionReset && ScreenTargets.FindByPredicate(FindCurrentTarget))
	{
		TargetIndex--;
		TargetIndex = FMath::Max(TargetIndex, 0);
		SetCurrentTarget(ScreenTargets[TargetIndex].Spacecraft);
		FLOGV("AFlareSpacecraft::PreviousTarget : %d", TargetIndex);
	}

	// Else reset
	else
	{
		TargetIndex = 0;
		FLOG("AFlareSpacecraft::PreviousTarget : reset to center");
	}

	TimeSinceSelection = 0;
}

void AFlareSpacecraft::AlternateNextTarget()
{
	NextTarget();
}

void AFlareSpacecraft::AlternatePreviousTarget()
{
	PreviousTarget();
}


void AFlareSpacecraft::YawInput(float Val)
{
	StateManager->SetPlayerAimMouse(FVector2D(Val, 0));
}

void AFlareSpacecraft::PitchInput(float Val)
{
	float Inverter = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings())->InvertY ? -1.0f : 1.0f;

	StateManager->SetPlayerAimMouse(FVector2D(0, Inverter * Val));
}

void AFlareSpacecraft::RollInput(float Val)
{
	StateManager->SetPlayerRollAngularVelocityKeyboard(-Val * NavigationSystem->GetAngularMaxVelocity());
}

void AFlareSpacecraft::ThrustInput(float Val)
{
	StateManager->SetPlayerXLinearVelocity(Val);
}

void AFlareSpacecraft::MoveVerticalInput(float Val)
{
	StateManager->SetPlayerZLinearVelocity(Val);
}

void AFlareSpacecraft::MoveHorizontalInput(float Val)
{
	StateManager->SetPlayerYLinearVelocity(Val);
}


void AFlareSpacecraft::GamepadMoveVerticalInput(float Val)
{
	if (FMath::Abs(Val) < Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings())->TranslationDeadZone)
	{
		Val = 0;
	}

	StateManager->SetPlayerZLinearVelocityGamepad(Val * NavigationSystem->GetLinearMaxVelocity());
}

void AFlareSpacecraft::GamepadMoveHorizontalInput(float Val)
{
	if (FMath::Abs(Val) < Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings())->TranslationDeadZone)
	{
		Val = 0;
	}

	StateManager->SetPlayerYLinearVelocityGamepad(Val * NavigationSystem->GetLinearMaxVelocity());
}

void AFlareSpacecraft::GamepadThrustInput(float Val)
{
	StateManager->SetPlayerXLinearVelocityGamepad(-Val * NavigationSystem->GetLinearMaxVelocity());
}

void AFlareSpacecraft::GamepadYawInput(float Val)
{
	if (FMath::Abs(Val) < Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings())->RotationDeadZone)
	{
		Val = 0;
	}

	StateManager->SetPlayerAimGamepadYaw(Val);
}

void AFlareSpacecraft::GamepadPitchInput(float Val)
{
	float Inverter = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings())->InvertY ? -1.0f : 1.0f;

	if (FMath::Abs(Val) < Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings())->RotationDeadZone)
	{
		Val = 0;
	}

	StateManager->SetPlayerAimGamepadPitch(Inverter  * -Val);
}


void AFlareSpacecraft::JoystickYawInput(float Val)
{
	if (FMath::Abs(Val) < Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings())->RotationDeadZone)
	{
		Val = 0;
	}

	StateManager->SetPlayerAimJoystickYaw(Val);
}

void AFlareSpacecraft::JoystickPitchInput(float Val)
{
	float Inverter = Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings())->InvertY ? -1.0f : 1.0f;

	if (FMath::Abs(Val) < Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings())->RotationDeadZone)
	{
		Val = 0;
	}

	StateManager->SetPlayerAimJoystickPitch(Inverter  * -Val);
}

void AFlareSpacecraft::JoystickRollInput(float Val)
{
	if (FMath::Abs(Val) < Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings())->RollDeadZone)
	{
		Val = 0;
	}

	JoystickRollInputVal.Add(Val);
	Val = JoystickRollInputVal.Get();

	StateManager->SetPlayerRollAngularVelocityJoystick(-Val * NavigationSystem->GetAngularMaxVelocity());
}

void AFlareSpacecraft::JoystickThrustInput(float Val)
{
		float TargetSpeed = 0;
		float Exponent = 2;
		float ZeroSpeed = 0.1f;
		float MinSpeed = -0.5f * NavigationSystem->GetLinearMaxVelocity();
		float MaxSpeed = 2.0f * NavigationSystem->GetLinearMaxVelocity();
	float JoystickThrottleThreshold = 0.005f;

	if (NavigationSystem)
	{
		// Throttle-specific deadzone control
		if (FMath::Abs(Val) < JoystickThrottleThreshold)
		{
			Val = 0;
		}
		if (FMath::Abs(Val - PreviousJoystickThrottle) < JoystickThrottleThreshold)
		{
			return;
		}
		PreviousJoystickThrottle = Val;

		// Forward only
		if (Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings())->ForwardOnlyThrust)
		{
			float NormalizedVal = (Val / 2.0f) - 0.5f;
			TargetSpeed = MaxSpeed * FMath::Pow(NormalizedVal, Exponent);
			TargetSpeed = FMath::Clamp(TargetSpeed, ZeroSpeed, MaxSpeed);
		}

		// Bidirectional mode
		else
		{
			if (Val < 0)
			{
				TargetSpeed = MaxSpeed * FMath::Pow(FMath::Abs(Val), Exponent);
				TargetSpeed = FMath::Clamp(TargetSpeed, ZeroSpeed, MaxSpeed);
			}
			else if (Val > 0)
			{
				TargetSpeed = MinSpeed * FMath::Pow(FMath::Abs(Val), Exponent);
				TargetSpeed = FMath::Clamp(TargetSpeed, MinSpeed, -ZeroSpeed);
			}
		}

		StateManager->SetPlayerXLinearVelocityJoystick(TargetSpeed);
	}
}

void AFlareSpacecraft::JoystickMoveVerticalInput(float Val)
{
	if (FMath::Abs(Val) < Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings())->TranslationDeadZone)
	{
		Val = 0;
	}

	StateManager->SetPlayerZLinearVelocityJoystick(Val * NavigationSystem->GetLinearMaxVelocity());
}

void AFlareSpacecraft::JoystickMoveHorizontalInput(float Val)
{
	if (FMath::Abs(Val) < Cast<UFlareGameUserSettings>(GEngine->GetGameUserSettings())->TranslationDeadZone)
	{
		Val = 0;
	}

	StateManager->SetPlayerYLinearVelocityJoystick(Val * NavigationSystem->GetLinearMaxVelocity());
}


void AFlareSpacecraft::ZoomIn()
{
	StateManager->ExternalCameraZoom(true);
}

void AFlareSpacecraft::ZoomOut()
{
	StateManager->ExternalCameraZoom(false);
}

void AFlareSpacecraft::CombatZoomIn()
{
	StateManager->SetCombatZoom(true);
}

void AFlareSpacecraft::CombatZoomOut()
{
	StateManager->SetCombatZoom(false);
}

void AFlareSpacecraft::FaceForward()
{
	if (!StateManager->IsPilotMode() && NavigationSystem->IsManualPilot())
	{
		NavigationSystem->PushCommandRotation(Airframe->GetPhysicsLinearVelocity(), FVector(1,0,0));
	}
}

void AFlareSpacecraft::FaceBackward()
{
	if (!StateManager->IsPilotMode() && NavigationSystem->IsManualPilot())
	{
		NavigationSystem->PushCommandRotation((-Airframe->GetPhysicsLinearVelocity()), FVector(1, 0, 0));
	}
}

void AFlareSpacecraft::Brake()
{
	if (!StateManager->IsPilotMode() && NavigationSystem->IsManualPilot())
	{
		BrakeToVelocity();
	}
}

void AFlareSpacecraft::BrakeToVelocity(const FVector& VelocityTarget)
{
	if (!StateManager->IsPilotMode() && NavigationSystem->IsManualPilot())
	{
		NavigationSystem->PushCommandLinearBrake(VelocityTarget);
	}
}

void AFlareSpacecraft::LockDirectionOn()
{
	StateManager->SetPlayerLockDirection(true);
}

void AFlareSpacecraft::LockDirectionOff()
{
	StateManager->SetPlayerLockDirection(false);
}

void AFlareSpacecraft::FindTarget()
{
	PilotHelper::PilotTarget TargetCandidate;

	struct PilotHelper::TargetPreferences TargetPreferences;
	TargetPreferences.IsLarge = 1;
	TargetPreferences.IsSmall = 1;
	TargetPreferences.IsStation = 1;
	TargetPreferences.IsNotStation = 1;
	TargetPreferences.IsMilitary = 1;
	TargetPreferences.IsNotMilitary = 0.1;
	TargetPreferences.IsDangerous = 1;
	TargetPreferences.IsNotDangerous = 0.01;
	TargetPreferences.IsStranded = 1;
	TargetPreferences.IsNotStranded = 0.5;
	TargetPreferences.IsUncontrollableCivil = 0.0;
	TargetPreferences.IsUncontrollableSmallMilitary = 0.0;
	TargetPreferences.IsUncontrollableLargeMilitary = 0.0;
	TargetPreferences.IsNotUncontrollable = 1;
	TargetPreferences.IsHarpooned = 0;
	TargetPreferences.TargetStateWeight = 1;
	TargetPreferences.MaxDistance = 2000000;
	TargetPreferences.DistanceWeight = 0.5;
	TargetPreferences.AttackTarget = nullptr;
	TargetPreferences.AttackTargetWeight = 15.f;
	TargetPreferences.AttackMeWeight = 10;
	TargetPreferences.LastTarget = PilotHelper::PilotTarget(CurrentTarget);
	TargetPreferences.LastTargetWeight = -10; // Avoid current target
	TargetPreferences.PreferredDirection = GetFrontVector();
	TargetPreferences.MinAlignement = -1;
	TargetPreferences.AlignementWeight = 0.2;
	TargetPreferences.BaseLocation = GetActorLocation();
	TargetPreferences.IsBomb = 0;
	TargetPreferences.IsMeteorite = 0;

	GetWeaponsSystem()->UpdateTargetPreference(TargetPreferences, GetWeaponsSystem()->GetActiveWeaponGroup());

	TargetCandidate = PilotHelper::GetBestTarget(this, TargetPreferences);

	if (TargetCandidate.IsEmpty() && GetPC()->GetCurrentObjective() && GetPC()->GetCurrentObjective()->TargetSpacecrafts.Num())
	{
		for(UFlareSimulatedSpacecraft* Candidate : GetPC()->GetCurrentObjective()->TargetSpacecrafts)
		{
			if(!Candidate->IsActive())
			{
				continue;
			}

			AFlareSpacecraft* ActiveCandidate = Candidate->GetActive();

			if(ActiveCandidate == this || ActiveCandidate == CurrentTarget.SpacecraftTarget)
			{
				continue;
			}

			TargetCandidate.SetSpacecraft(ActiveCandidate);
			break;
		}
	}

	if (TargetCandidate.SpacecraftTarget)
	{
		SetCurrentTarget(TargetCandidate.SpacecraftTarget);
	}
	else
	{
		// Notify PC
		GetPC()->Notify(LOCTEXT("NoBestTarget", "No target"),
				LOCTEXT("NoBestTargetFormat", "No appropriate target was found."),
				FName("no-best-target"),
				EFlareNotification::NT_Military);
	}
}


/*----------------------------------------------------
		Getters
----------------------------------------------------*/

FText AFlareSpacecraft::GetShipStatus() const
{
	FText PauseText;
	FText ModeText;
	FText AutopilotText;
	UFlareSpacecraftNavigationSystem* Nav = GetNavigationSystem();
	FFlareShipCommandData Command = Nav->GetCurrentCommand();
	UFlareSector* CurrentSector = GetGame()->GetActiveSector();

	// Modifiers
	if (Paused)
	{
		PauseText = LOCTEXT("GamePausedModInfo", "Game paused - ");
	}
	if (Nav->IsAutoPilot())
	{
		AutopilotText = LOCTEXT("AutopilotMod", "(Auto)");
	}

	// Get mode info
	FText ActionInfo;
	if (GetStateManager()->IsPilotMode())
	{
		ActionInfo = LOCTEXT("AutoPilotModeInfo", "Auto-piloted");
	}
	else if (Command.Type == EFlareCommandDataType::CDT_Location && Command.ActionTarget)
	{
		ActionInfo = LOCTEXT("UndockingFrom", "Undocking");
	}
	else
	{
		ActionInfo = GetWeaponsSystem()->GetWeaponModeInfo();
	}

	// Build mode text
	if (Nav->IsDocked())
	{
		ModeText = FText::Format(LOCTEXT("DockedFormat", "Docked in {0}"),
			CurrentSector->GetSimulatedSector()->GetSectorName());
	}
	else if (Parent->GetCurrentFleet()->IsTraveling())
	{
		ModeText = FText::Format(LOCTEXT("TravelingAtFormat", "Traveling to {0}"),
			Parent->GetCurrentFleet()->GetCurrentTravel()->GetDestinationSector()->GetSectorName());
	}
	else if (Command.Type == EFlareCommandDataType::CDT_Dock)
	{
		AFlareSpacecraft* Target = Command.ActionTarget;
/*
		FFlareDockingInfo StationDockInfo = Command.ActionTarget->GetDockingSystem()->GetDockInfo(Command.ActionTargetParam);
		FFlareDockingParameters DockingParameters = Nav->GetDockingParameters(StationDockInfo, FVector::ZeroVector);
		FText DockingPhase = Nav->GetDockingPhaseName(DockingParameters.DockingPhase);
*/
		ModeText = FText::Format(LOCTEXT("DockingAtFormat", "Docking at {0}"),
		UFlareGameTools::DisplaySpacecraftName(Target->GetParent(), true));
	}
/*
	else if (Command.Type == EFlareCommandDataType::CDT_Location && Command.ActionTarget)
	{
	//too many characters
		ModeText = FText::Format(LOCTEXT("UndockingFrom", "Undocking from {0}"),
		UFlareGameTools::DisplaySpacecraftName(Command.ActionTarget->GetParent(), true));
	}
*/
	else if (!Paused)
	{
		int32 Speed = GetLinearVelocity().Size();
		Speed = IsMovingForward() ? Speed : -Speed;
		ModeText = FText::Format(LOCTEXT("SpeedNotPausedFormat", "{0} m/s - {1} in {2}"),
			FText::AsNumber(FMath::RoundToInt(Speed)),
			ActionInfo,
			CurrentSector->GetSimulatedSector()->GetSectorName());
	}
	else
	{
		ModeText = FText::Format(LOCTEXT("SpeedPausedFormat-", "{0} in {1}"),
			GetWeaponsSystem()->GetWeaponModeInfo(),
			CurrentSector->GetSimulatedSector()->GetSectorName());
	}

	return FText::Format(LOCTEXT("ShipInfoTextFormat", "{0}{1} {2}"),
		PauseText, ModeText, AutopilotText);
}

/** Linear velocity, in m/s in world reference*/
FVector AFlareSpacecraft::GetLinearVelocity() const
{
	return Airframe->GetPhysicsLinearVelocity() / 100;
}

float AFlareSpacecraft::GetPreferedAnticollisionTime() const
{
	return GetTimeToStop() * 1.5f;
}

float AFlareSpacecraft::GetAgressiveAnticollisionTime() const
{
	return GetTimeToStop() * 0.3f;
}

float AFlareSpacecraft::GetTimeToStop() const
{
	if(TimeToStopCached)
	{
		return TimeToStopCache;
	}

	TimeToStopCached = true;

	FVector CurrentVelocity = GetLinearVelocity();

	if (FMath::IsNearlyZero(CurrentVelocity.SizeSquared()))
	{
		TimeToStopCache = 0;
	}
	else
	{
		FVector CurrentVelocityAxis = CurrentVelocity.GetUnsafeNormal();
		TArray<UActorComponent*> Engines = GetComponentsByClass(UFlareEngine::StaticClass());

		FVector Acceleration = GetNavigationSystem()->GetTotalMaxThrustInAxis(Engines, CurrentVelocityAxis, false) / GetSpacecraftMass();
		float AccelerationInAngleAxis =  FMath::Abs(FVector::DotProduct(Acceleration, CurrentVelocityAxis));

		TimeToStopCache = (CurrentVelocity.Size() / (AccelerationInAngleAxis));
	}
	return TimeToStopCache;
}

bool AFlareSpacecraft::IsOutsideSector() const
{
	float Distance = GetActorLocation().Size();
	float Limits = GetGame()->GetActiveSector()->GetSectorLimits();
	return Distance > Limits;
}

AFlareSpacecraft* AFlareSpacecraft::GetComplex() const
{
	for (AFlareSpacecraft* StationCandidate: GetGame()->GetActiveSector()->GetSpacecrafts())
	{
		if (StationCandidate->GetImmatriculation() == GetConstData().AttachComplexStationName)
		{
			return StationCandidate;
		}
	}
	return nullptr;
}

bool AFlareSpacecraft::IsComplexElement() const
{
	return GetConstData().AttachComplexStationName != NAME_None;
}

bool AFlareSpacecraft::IsPlayerHostile() const
{
	return IsHostile(GetPC()->GetCompany());
}

#undef LOCTEXT_NAMESPACE