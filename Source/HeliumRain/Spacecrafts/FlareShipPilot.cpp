
#include "FlareShipPilot.h"
#include "../Flare.h"

#include "FlareSpacecraft.h"

#include "../Data/FlareSpacecraftComponentsCatalog.h"

#include "../Game/FlareCompany.h"
#include "../Game/FlareGame.h"
#include "../Game/AI/FlareCompanyAI.h"
#include "../Quests/FlareQuest.h"
#include "../Quests/FlareQuestStep.h"

#include "../Player/FlarePlayerController.h"

#include "../Spacecrafts/FlareEngine.h"
#include "../Spacecrafts/FlareRCS.h"

DECLARE_CYCLE_STAT(TEXT("FlareShipPilot FollowLeaderShip"), STAT_FlareShipPilot_FollowLeaderShip, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot GetNewLeaderShip"), STAT_FlareShipPilot_GetNewLeaderShip, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot AvoidShip"), STAT_FlareShipPilot_AvoidShip, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot TryAntiCollision"), STAT_FlareShipPilot_TryAntiCollision, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot Tick"), STAT_FlareShipPilot_Tick, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot CarrierRelease"), STAT_FlareShipPilot_CarrierRelease, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot DroneReturnToCarrier"), STAT_FlareShipPilot_ReturnToCarrier, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot ArmedStation"), STAT_FlareShipPilot_ArmedStation, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot Military"), STAT_FlareShipPilot_Military, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot Cargo"), STAT_FlareShipPilot_Cargo, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot Fighter"), STAT_FlareShipPilot_Fighter, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot Bomber"), STAT_FlareShipPilot_Bomber, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot Missile"), STAT_FlareShipPilot_Missile, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot Idle"), STAT_FlareShipPilot_Idle, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot Flagship"), STAT_FlareShipPilot_Flagship, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot FindBestHostileTarget"), STAT_FlareShipPilot_FindBestHostileTarget, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareShipPilot ExitAvoidance"), STAT_FlareShipPilot_ExitAvoidance, STATGROUP_Flare);

#define PILOT_AI_TICKRATE_LARGE 0.025f

#define PILOT_ANTICOLLISION_SPEED -200.f
//Large ships get 4 subordinates = wing of 5 ships
#define LEADER_SQUAD_LARGE 4
//Small ships get 2 subordinates = wing of 3 ships
#define LEADER_SQUAD_SMALL 2

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareShipPilot::UFlareShipPilot(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
//main reason for setting up randomized reaction delays/times is to spread out the calculations of multiple ships to different game ticks
	ReactionTime = FMath::FRandRange(0.4, 0.7);
	ComponentSwitchReactionTime = FMath::FRandRange(4.5, 5.5);

	PilotAvoidCheckReactionTimeFast = FMath::FRandRange(1, 2);
	PilotAvoidCheckReactionTimeSlow = FMath::FRandRange(3, 5);
	NextExitAvoidanceCheckReactionFast = FMath::FRandRange(0.50, 0.75);
	NextExitAvoidanceCheckReactionSlow = FMath::FRandRange(3, 4);
	HostileTargetSwitchReactionTimeFast = FMath::FRandRange(1.0, 1.5);
	HostileTargetSwitchReactionTimeSlow = FMath::FRandRange(4, 6);
	CollisionVectorReactionTimeFast = FMath::FRandRange(0.25, 0.35);;
	CollisionVectorReactionTimeSlow = FMath::FRandRange(0.75, 1.25);

	TimeUntilNextHostileTargetSwitch = 0;
	TimeUntilNextReaction = 0;
	CurrentWaitTime = 0;
	DockWaitTime = FMath::FRandRange(30, 60);
	PilotTargetLocation = FVector::ZeroVector;
	PilotTargetStation = NULL;
	PilotLastTargetStation = NULL;
	SelectedWeaponGroupIndex = -1;
	MaxFollowDistance = 0;
	LockTarget = false;
	WantFire = false;
	InitiatedCombat = false;

	TimeSinceLastDockingAttempt = 0.0f;
	TimeUntilNextDockingAttempt = 0.0f;
	MaxTimeBetweenDockingAttempt = 60.0f;
}

/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareShipPilot::TickPilot(float DeltaSeconds)
{
	if (Ship->GetNavigationSystem()->IsAutoPilot())
	{
		TotalTimeAutoPiloting += DeltaSeconds;
	}
	else
	{
		TotalTimeAutoPiloting = 0;
	}

	if (Ship->GetSize() == EFlarePartSize::L)
	{
		PreviousTick += DeltaSeconds;
		if (PreviousTick < PILOT_AI_TICKRATE_LARGE)
		{
			return;
		}
		PreviousTick -= PILOT_AI_TICKRATE_LARGE;
	}

	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_Tick);

	if (Ship->IsStation())
	{
			// Basic pilot for stations
		if ((PilotTarget.SpacecraftTarget && !PilotTarget.SpacecraftTarget->GetParent()->GetDamageSystem()->IsUncontrollable()) || PilotTarget.BombTarget || PilotTarget.MeteoriteTarget)
		{
			Ship->SetWantUndockInternalShips(true);
		}
		else
		{
			Ship->SetWantUndockInternalShips(false);
		}

		if (Ship->IsMilitary())
		{
			ArmedStationPilot(DeltaSeconds);
		}
		return;
	}

	if(!Ship->GetParent()->GetDamageSystem()->IsUncontrollable())
	{
		if (TotalTimeAutoPiloting >= 3000)
		{
			Ship->GetNavigationSystem()->AbortAllCommands();
		}	

		LinearTargetVelocity = FVector::ZeroVector;
		AngularTargetVelocity = FVector::ZeroVector;
		UseOrbitalBoost = true;
		TimeUntilNextReaction -= DeltaSeconds;


		if (Ship->IsCapableCarrier() && PilotTarget.IsValid())
		{
			if ((PilotTarget.SpacecraftTarget && !PilotTarget.SpacecraftTarget->GetParent()->GetDamageSystem()->IsUncontrollable()) || PilotTarget.BombTarget || PilotTarget.MeteoriteTarget)
			{
				Ship->SetWantUndockInternalShips(true);
			}
			else
			{
				Ship->SetWantUndockInternalShips(false);
			}
		} 

		else if (Ship->GetParent()->GetShipMaster() && !Ship->GetParent()->GetShipMaster()->GetActive()->GetWantUndockInternalShips())
		{
			WantFire = false;
			if (DroneReturnToCarrier(DeltaSeconds))
			{
				return;
			}
		}

		if (Ship->IsMilitary())
		{
			MilitaryPilot(DeltaSeconds);
		}
		else
		{
			CargoPilot(DeltaSeconds);
		}
	}
}

void UFlareShipPilot::Initialize(const FFlareShipPilotSave* Data, UFlareCompany* Company, AFlareSpacecraft* OwnerShip)
{
	// Main data
	Ship = OwnerShip;
	PlayerCompany = Company;

	// Setup properties
	if (Data)
	{
		ShipPilotData = *Data;
	}
	AttackAngle = FMath::FRandRange(0, 360);
	LeaderShip = Ship;
}


/*----------------------------------------------------
	Pilot functions
----------------------------------------------------*/
bool UFlareShipPilot::DroneReturnToCarrier(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_ReturnToCarrier);
	AFlareSpacecraft* LocalMaster = Ship->GetParent()->GetShipMaster()->GetActive();
	if (LocalMaster)
	{
		if (Ship->GetNavigationSystem()->IsDocked())
		{
			Ship->GetNavigationSystem()->Undock();
		}

		FVector Location = LocalMaster->GetActorLocation();
		FVector DeltaLocation = LocalMaster->GetActorLocation() - Ship->GetActorLocation();
		float Distance = (DeltaLocation.Size() ) * 0.01f; // Distance in meters
		float DroneDockRadius = Ship->GetParent()->GetShipMaster()->GetDescription()->DroneDockRadius;
		if (!DroneDockRadius)
		{
			DroneDockRadius = 125;
		}

		if (Distance <= DroneDockRadius)
		{
			Ship->GetParent()->SetInternalDockedTo(Ship->GetParent()->GetShipMaster());
			Ship->GetGame()->GetActiveSector()->RemoveSpacecraft(Ship);
			Ship->SafeHide(false);
			LocalMaster->SetUndockedAllShips(false);
			return true;
		}

		FVector LocationTarget = LocalMaster->GetActorLocation();

		FVector TargetAxis = DeltaLocation;
		AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1, 0, 0), TargetAxis, FVector::ZeroVector, DeltaSeconds);

		LinearTargetVelocity = (LocalMaster->GetActorLocation() - Ship->GetActorLocation()).GetUnsafeNormal()  * Ship->GetNavigationSystem()->GetLinearMaxVelocity();

		LinearTargetVelocity += LinearTargetVelocity.GetUnsafeNormal() * FVector::DotProduct(PilotTarget.GetLinearVelocity() / 100.f, LinearTargetVelocity.GetUnsafeNormal());

//		LinearTargetVelocity = ExitAvoidance(Ship, LinearTargetVelocity, 0.4);
		AlignToTargetVelocityWithThrust(DeltaSeconds);

		// Anticollision
		LinearTargetVelocity = TryAnticollisionCorrection(Ship, LinearTargetVelocity, Ship->GetPreferedAnticollisionTime(), PilotHelper::AnticollisionConfig(), DeltaSeconds);
		return true;
	}
	return false;
}

void UFlareShipPilot::ArmedStationPilot(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_ArmedStation);
	if (Ship->GetParent()->GetDamageSystem()->IsDisarmed())
	{
		WantFire = false;
	}
	else
	{
		TimeUntilNextHostileTargetSwitch -= DeltaSeconds;
		if (TimeUntilNextHostileTargetSwitch <= 0)
		{
			FindBestHostileTarget(EFlareCombatTactic::AttackMilitary);
			if (InitiatedCombat)
			{
				TimeUntilNextHostileTargetSwitch = HostileTargetSwitchReactionTimeFast;
			}
			else
			{
				TimeUntilNextHostileTargetSwitch = HostileTargetSwitchReactionTimeSlow;
			}
		}

		if (!Ship->IsImmobileStation())
		{
			FlagShipMovement(DeltaSeconds);
		}
	}
}

void UFlareShipPilot::ClearTarget()
{
	PilotTarget.Clear();
	TimeUntilNextHostileTargetSwitch = 0;
}

void UFlareShipPilot::SetupNewTarget()
{
	EFlareCombatGroup::Type CombatGroup;

	if (Ship->GetSize() == EFlarePartSize::L)
	{
		CombatGroup = EFlareCombatGroup::Capitals;
	}
	else
	{
		CombatGroup = EFlareCombatGroup::Fighters;
	}

	CurrentTactic = Ship->GetCompany()->GetTacticManager()->GetCurrentTacticForShipGroup(CombatGroup);
	FindBestHostileTarget(CurrentTactic);

	if (InitiatedCombat)
	{
		TimeUntilNextHostileTargetSwitch = HostileTargetSwitchReactionTimeFast;
	}
	else
	{
		TimeUntilNextHostileTargetSwitch = HostileTargetSwitchReactionTimeSlow;
	}
}

void UFlareShipPilot::MilitaryPilot(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_Military);

	if (Ship->GetNavigationSystem()->IsDocked() && ( Ship->GetParent()->GetCompany() != PlayerCompany || Ship->GetParent()->GetCurrentSector()->GetSectorBattleState(PlayerCompany).HasDanger))
	{
		PilotLastTargetStation = PilotTargetStation;
		PilotTargetStation = NULL;
		CurrentWaitTime = 0;
		WantFire = false;
		// Let's undock
		Ship->GetNavigationSystem()->Undock();
		return;
	}

	else if (Ship->GetNavigationSystem()->IsAutoPilot())
	{
		WantFire = false;
		// Wait manoeuver
		return;
	}

	bool Idle = true;
	if(Ship->GetParent()->GetDamageSystem()->IsDisarmed())
	{
		// Idle
	}
	else
	{
		TimeUntilNextComponentSwitch -= DeltaSeconds;
		TimeUntilNextHostileTargetSwitch -= DeltaSeconds;

		if (TimeUntilNextHostileTargetSwitch <= 0)
		{
			//todo: Add neccessary events so that target checks out-side of combat are no longer required

			SetupNewTarget();

			if (LeaderShip == Ship)
			{
				if (InitiatedCombat)
				{
					if (FoundOutOfCombatLeader)
					{
						FoundOutOfCombatLeader = false;
						GetNewLeaderShip();
					}
				}		
			}
			else if (LeaderShip->GetParent()->GetDamageSystem()->IsDisarmed())
			{
				GetNewLeaderShip();
			}
			else
			{
				PilotHelper::PilotTarget LeaderTargetCandidate = LeaderShip->GetPilot()->GetPilotTarget();
				if (LeaderTargetCandidate.IsValid())
				{
					if (LeaderTargetCandidate.SpacecraftTarget)
					{
						if (LeaderTargetCandidate.SpacecraftTarget->GetParent()->GetDamageSystem()->IsAlive())
						{
							if (Ship->IsHostile(LeaderTargetCandidate.SpacecraftTarget->GetCompany()))
							{
								SelectNewHostileTarget(LeaderTargetCandidate);
							}
							else
							{
								//leader has a friendly/neutral target so we'll look on our own
								SetupNewTarget();
							}
						}
						else
						{
							//leader has a dead target so we'll look on our own
							SetupNewTarget();
						}
					}
				}
				else
				{
					SetupNewTarget();
				}
			}
		}

		if (Ship->GetSize() == EFlarePartSize::S && PilotTarget.IsValid() && SelectedWeaponGroupIndex >= 0)
		{
			if (PilotTarget.SpacecraftTarget)
			{
				if (TimeUntilNextComponentSwitch <= 0 && !LockTarget)
				{
					PilotTargetShipComponent = NULL;
				}
				else if (PilotTargetShipComponent)
				{
					if (!PilotTarget.Is(PilotTargetShipComponent->GetSpacecraft()))
					{
						PilotTargetShipComponent = NULL;
					}
					else if (PilotTargetShipComponent->GetUsableRatio() <= 0)
					{
						PilotTargetShipComponent = NULL;
					}
				}

				if (!PilotTargetShipComponent)
				{
					if (LeaderShip == Ship || LeaderShip == Ship->GetGame()->GetPC()->GetShipPawn())
					{ 
						PilotTargetShipComponent = PilotHelper::GetBestTargetComponent(PilotTarget.SpacecraftTarget);
					}
					else
					{
						PilotTargetShipComponent = LeaderShip->GetPilot()->GetPilotTargetShipComponent();
					}
					TimeUntilNextComponentSwitch = ComponentSwitchReactionTime;
				}
			}
			else
			{
				PilotTargetShipComponent = NULL;
			}

			if (PilotTarget.IsValid())
			{
				EFlareWeaponGroupType::Type WeaponType = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Type;
				if (WeaponType == EFlareWeaponGroupType::WG_GUN)
				{
					FighterPilot(DeltaSeconds);
					Idle = false;
				}
				else if (WeaponType == EFlareWeaponGroupType::WG_BOMB)
				{
					BomberPilot(DeltaSeconds);
					Idle = false;
				}
				else if (WeaponType == EFlareWeaponGroupType::WG_MISSILE)
				{
					MissilePilot(DeltaSeconds);
					Idle = false;
				}
			}
		}
		else if (Ship->GetSize() == EFlarePartSize::L && PilotTarget.IsValid())
		{
			WantFire = false;
			FlagShipPilot(DeltaSeconds);
			Idle = false;
		}
	}

	if (Idle)
	{
		PilotTarget.Clear();
		WantFire = false;
		IdlePilot(DeltaSeconds);
	} 
	// TODO S or L ship dispatch
}

void UFlareShipPilot::CargoPilot(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_Cargo);

	TimeSinceLastDockingAttempt += DeltaSeconds;
	FindPilotAvoidShip(DeltaSeconds);

	// If enemy near, run away !
	if (PilotAvoidShip)
	{
		FVector DeltaLocation = (PilotAvoidShip->GetActorLocation() - Ship->GetActorLocation()) / 100.f;
		float Distance = DeltaLocation.Size(); // Distance in meters

		// There is at least one hostile enemy
		if (Distance < 4000)
		{
			Ship->GetNavigationSystem()->AbortAllCommands();
			if(Ship->GetNavigationSystem()->IsDocked())
			{
				Ship->GetNavigationSystem()->Undock();
			}
			LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity() * 100;
			UseOrbitalBoost = true;
		}
		else
		{
			PilotAvoidShip = NULL;
		}
	}

	if (Ship->GetNavigationSystem()->IsAutoPilot())
	{
		return;
	}

	//Cargo ships can "dock for repairs" if needed as long as they aren't already doing something
	if (!Ship->GetParent()->GetShipMaster() && Ship->GetParent()->IsRepairing())
	{
		if (Ship->GetNavigationSystem()->IsDocked())
		{
			return;
		}
		else
		{
			GetRandomFriendlyStation();
			if (DockAtStation(DeltaSeconds))
			{
				return;
			}
		}
	}
	else if(Ship->GetParent()->GetCompany() != Ship->GetGame()->GetPC()->GetCompany())
	{
		// Docking wait time for non-player ships
		if (!PilotAvoidShip && Ship->GetNavigationSystem()->IsDocked())
		{
			// Dock
			if (CurrentWaitTime <= 0)
			{
				CurrentWaitTime = DockWaitTime;
			}

			// Undock
			else if (CurrentWaitTime < 0.1)
			{
				Ship->GetNavigationSystem()->Undock();
				// Swap target station
				PilotLastTargetStation = PilotTargetStation;
				PilotTargetStation = NULL;
				CurrentWaitTime = 0;
			}

			// Wait
			else
			{
				CurrentWaitTime -= DeltaSeconds;
			}

			return;
		}
		else
		{
			// If no station target, find a target : a random friendly station different from the last station
			{
				GetRandomFriendlyStation();
				if (DockAtStation(DeltaSeconds))
				{
					return;
				}
			}
		}
	}

	// Exit avoidance

	//FLOGV("%s LinearTargetVelocity before exit avoidance = %s",  *Ship->GetImmatriculation().ToString(), *LinearTargetVelocity.ToString());

	LinearTargetVelocity = ExitAvoidance(Ship, LinearTargetVelocity, 0.4,DeltaSeconds);
	AlignToTargetVelocityWithThrust(DeltaSeconds);

	//FLOGV("%s LinearTargetVelocity before anticollision = %s",  *Ship->GetImmatriculation().ToString(), *LinearTargetVelocity.ToString());

	// Anticollision
	LinearTargetVelocity = TryAnticollisionCorrection(Ship, LinearTargetVelocity, Ship->GetPreferedAnticollisionTime(), PilotHelper::AnticollisionConfig(), DeltaSeconds);

	//FLOGV("%s Location = %s Velocity = %s LinearTargetVelocity = %s",  *Ship->GetImmatriculation().ToString(), * Ship->GetActorLocation().ToString(), *Ship->GetLinearVelocity().ToString(), *LinearTargetVelocity.ToString());
}

void UFlareShipPilot::GetRandomFriendlyStation()
{
	if (!PilotTargetStation)
	{
		TArray<AFlareSpacecraft*> FriendlyStations = GetFriendlyStations();
		if (FriendlyStations.Num() > 0)
		{
			int32 Index = FMath::RandHelper(FriendlyStations.Num());
			if (PilotLastTargetStation != FriendlyStations[Index])
			{
				PilotTargetStation = FriendlyStations[Index];
			}
		}
	}
}

bool UFlareShipPilot::DockAtStation(float DeltaSeconds)
{
	if (PilotTargetStation)
	{
		if (TimeSinceLastDockingAttempt > TimeUntilNextDockingAttempt && Ship->GetNavigationSystem()->DockAt(PilotTargetStation))
		{
			TimeSinceLastDockingAttempt = 0;
			TimeUntilNextDockingAttempt = FMath::FRand() * MaxTimeBetweenDockingAttempt;
			return true;
		}
		PilotTargetStation = NULL;
	}
	return false;
}

/*
{
	if(PilotTargetStation)
	{
		FVector DeltaLocation = (PilotTargetStation->GetActorLocation() - Ship->GetActorLocation()) / 100.f;
		float Distance = DeltaLocation.Size(); // Distance in meters

		if (Distance < 1000 && TimeSinceLastDockingAttempt > TimeUntilNextDockingAttempt)
		{
			if (!Ship->GetNavigationSystem()->DockAt(PilotTargetStation))
			{
				DockingAttemptTries = 0;
				if (DockingAttemptTries < 3)
				{
					TimeSinceLastDockingAttempt = 0;
					TimeUntilNextDockingAttempt = FMath::FRand() * MaxTimeBetweenDockingAttempt;
					LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
					return true;
				}
				else
				{
					TimeSinceLastDockingAttempt = 0;
					TimeUntilNextDockingAttempt = 0;
					PilotTargetStation = NULL;
				}
			}
			else
			{
				DockingAttemptTries = 0;
				return true;
			}

		}
		else if (Distance < 200)
		{
			LinearTargetVelocity = FVector::ZeroVector;
		}
		else
		{
			LinearTargetVelocity = DeltaLocation.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
		}
	}
	return false;
}
*/
void UFlareShipPilot::FighterPilot(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_Fighter);

	float AmmoVelocity = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons[0]->GetAmmoVelocity() * 100;

	FVector BaseLocation = Ship->GetCamera()->GetComponentLocation();

	bool DangerousTarget = PilotHelper::IsTargetDangerous(PilotTarget);

	//float PreferedVelocity = FMath::Max(PilotTargetShip->GetLinearVelocity().Size() * 2.0f, Ship->GetNavigationSystem()->GetLinearMaxVelocity());
	float PreferedVelocity = Ship->GetNavigationSystem()->GetLinearMaxVelocity() * 2;

	//FLOGV("%s target %s",  *Ship->GetImmatriculation().ToString(),  *PilotTargetShip->GetImmatriculation().ToString());
	// The pilot have a target, track and kill it

	FVector PilotAimTargetLocation = (PilotTargetShipComponent ? PilotTargetShipComponent->GetComponentLocation() : PilotTarget.GetActorLocation());

	FVector LocalNose = FVector(1.f, 0.f, 0.f);
	FVector DeltaLocation = (PilotAimTargetLocation - BaseLocation) / 100.f;
	float Distance = DeltaLocation.Size(); // Distance in meters
	float TargetSize = PilotTarget.GetMeshScale() / 100.f; // Radius in meters
	FVector TargetAxis = DeltaLocation.GetUnsafeNormal();
	FVector ShipVelocity = 100 * Ship->GetLinearVelocity();
	FVector PilotTargetShipVelocity = PilotTarget.GetLinearVelocity();

	// Use position prediction
	float PredictionDelay = DeltaSeconds;
	FVector PredictedShipLocation = BaseLocation + ShipVelocity * PredictionDelay;
	FVector PredictedPilotTargetShipLocation = PilotAimTargetLocation + PilotTargetShipVelocity * PredictionDelay;
	FVector PredictedDeltaLocation = (PredictedPilotTargetShipLocation - PredictedShipLocation) / 100.f;
	FVector PredictedTargetAxis = PredictedDeltaLocation.GetUnsafeNormal();
	float PredictedDistance = PredictedDeltaLocation.Size(); // Distance in meters

	FVector AmmoIntersectionLocation;
	float AmmoIntersectionTime = SpacecraftHelper::GetIntersectionPosition(PilotAimTargetLocation, PilotTarget.GetLinearVelocity(), BaseLocation, ShipVelocity, AmmoVelocity, 0, &AmmoIntersectionLocation);

	FVector FireTargetAxis;
	if (AmmoIntersectionTime > 0)
	{
		FireTargetAxis = (AmmoIntersectionLocation - BaseLocation).GetUnsafeNormal();
	}
	else
	{
		FireTargetAxis = (PilotAimTargetLocation - BaseLocation).GetUnsafeNormal();
	}

	FVector AmmoIntersectionPredictedLocation;
	float AmmoIntersectionPredictedTime = SpacecraftHelper::GetIntersectionPosition(PilotAimTargetLocation, PilotTarget.GetLinearVelocity(), BaseLocation, ShipVelocity, AmmoVelocity, PredictionDelay, &AmmoIntersectionPredictedLocation);

	/*if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
	{
		DrawDebugSphere(Ship->GetWorld(), AmmoIntersectionPredictedLocation, 100, 12, FColor::White, true);
	}*/

	FVector PredictedFireTargetAxis;
	if (AmmoIntersectionPredictedTime > 0)
	{
		PredictedFireTargetAxis = (AmmoIntersectionPredictedLocation - PredictedShipLocation).GetUnsafeNormal();
	}
	else
	{
		PredictedFireTargetAxis = (PredictedDeltaLocation* 100.f - PredictedShipLocation).GetUnsafeNormal();
	}

	FRotator ShipAttitude = Ship->GetActorRotation();

	// Bullet velocity
	FVector BulletVelocity = ShipAttitude.Vector();
	BulletVelocity.Normalize();
	BulletVelocity *= AmmoVelocity;

	FVector BulletDirection = Ship->Airframe->GetComponentToWorld().GetRotation().Inverse().RotateVector((ShipVelocity + BulletVelocity)).GetUnsafeNormal();


	FVector DeltaVelocity = PilotTarget.GetLinearVelocity() / 100. - ShipVelocity / 100.;

	FVector PredictedTargetAngularVelocity = - 180 / (PI * PredictedDistance) * FVector::CrossProduct(DeltaVelocity, PredictedTargetAxis);

	// TargetAngularVelocity = FVector(0,0,0);

	// First allow align nose to target bullet interception point
	// TODO Use BulletDirection instead of LocalNose
	// AngularTargetVelocity = GetAngularVelocityToAlignAxis(LocalNose, FireTargetAxis, DeltaSeconds);
	// TODO find target angular velocity

	FVector FireAxis = Ship->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalNose);

	float TargetAxisAngularPrecisionDot = FVector::DotProduct(PredictedFireTargetAxis, FireAxis);
	float TargetAxisAngularPrecision = FMath::Acos(TargetAxisAngularPrecisionDot);
	float AngularNoise;

	float Experience = 0.5;

	if(WantFire)
	{
		float Recoil = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Description->WeaponCharacteristics.GunCharacteristics.AmmoPrecision;
		TimeSinceAiming -= (5 + 100 * Recoil) * DeltaSeconds / Experience;
		TimeSinceAiming = FMath::Max(0.f, TimeSinceAiming);
	}
	if(FMath::RadiansToDegrees(TargetAxisAngularPrecision) < 30)
	{
		TimeSinceAiming += DeltaSeconds;
		AngularNoise = 20 / (1+ Experience * FMath::Square(TimeSinceAiming)) ; // 10 degree
	}
	else
	{
		TimeSinceAiming = 0;
		AngularNoise = 20; // 1 degree
	}


	/*if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
	{
		FLOGV("TargetAxisAngularPrecisionDot=%f", TargetAxisAngularPrecisionDot);
		FLOGV("TargetAxisAngularPrecision=%f", TargetAxisAngularPrecision);
		FLOGV("TimeSinceAiming=%f", TimeSinceAiming);
		FLOGV("AngularNoise=%f", AngularNoise);
		FLOGV("Distance=%f", Distance);
		FLOGV("AttackPhase=%d", AttackPhase);
		AngularNoise = 0;
	}*/

	FVector PredictedFireTargetAxisWithError = FMath::VRandCone(PredictedFireTargetAxis, FMath::DegreesToRadians(AngularNoise));


	/*if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
	{
		DrawDebugLine(Ship->GetWorld(), Ship->GetActorLocation(), BaseLocation + PredictedFireTargetAxis *200000, FColor::Green, true);
	}*/

	AngularTargetVelocity = GetAngularVelocityToAlignAxis(LocalNose, PredictedFireTargetAxisWithError, PredictedTargetAngularVelocity, DeltaSeconds);

	/*FLOGV("Distance=%f", Distance);
	FLOGV("PilotTargetShip->GetLinearVelocity()=%s", *(PilotTargetShip->GetLinearVelocity().ToString()));
	FLOGV("TargetAxis=%s", *TargetAxis.ToString());
	FLOGV("TargetAngularVelocity=%s", *TargetAngularVelocity.ToString());
	FLOGV("AngularTargetVelocity=%s", *AngularTargetVelocity.ToString());
	FLOGV("Ship->Airframe->GetPhysicsAngularVelocity()=%s", *(Ship->Airframe->GetPhysicsAngularVelocity().ToString()));
*/
	/*FLOGV("DeltaLocation=%s", *DeltaLocation.ToString());
	FLOGV("TargetAxis=%s", *TargetAxis.ToString());
	FLOGV("FireTargetAxis=%s", *FireTargetAxis.ToString());
	FLOGV("BulletVelocity=%s", *BulletVelocity.ToString());
	FLOGV("BulletDirection=%s", *BulletDirection.ToString());

*/

	// Attack Phases
	// 0 - Prepare attack : change velocity to approch the target
	// 1 - Attacking : target is approching
	// 2 - Withdraw : target is passed, wait a security distance to attack again
	bool ClearTarget = false;
	if (AttackPhase == 0)
	{
		if (FVector::DotProduct(DeltaLocation, DeltaVelocity) < 0)
		{
			// Target is approching, prepare attack
			AttackPhase = 1;
			if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
			{
				FLOG("Pass to phase 1");
			}
			LastTargetDistance = Distance;
		}
		else
		{

			LinearTargetVelocity = PredictedFireTargetAxis * PreferedVelocity;
			LinearTargetVelocity += LinearTargetVelocity.GetUnsafeNormal() * FVector::DotProduct(PilotTarget.GetLinearVelocity() / 100.f,LinearTargetVelocity.GetUnsafeNormal());

			UseOrbitalBoost = true;
		}
	}

	if (AttackPhase == 1)
	{
		if (FVector::DotProduct(DeltaLocation, DeltaVelocity) > 0) // TODO : check if I can't use dot product
		{
			// Target is passed
			AttackPhase = 2;
			if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
			{
				FLOG("Pass to phase 2");
			}
		}
		else
		{
			float SecurityDistance = (DangerousTarget ? 1200 : 800) + TargetSize * 4;
			FQuat AttackDistanceQuat = FQuat(TargetAxis, AttackAngle);
			FVector TopVector = Ship->GetActorRotation().RotateVector(FVector(0,0,AttackDistance));
			FVector AttackMargin =  AttackDistanceQuat.RotateVector(TopVector);

			if(Distance < SecurityDistance)
			{
				PreferedVelocity /=2;
			}


			LinearTargetVelocity = (AttackMargin + DeltaLocation).GetUnsafeNormal() * PreferedVelocity;
			LinearTargetVelocity += LinearTargetVelocity.GetUnsafeNormal() * FVector::DotProduct(PilotTarget.GetLinearVelocity() / 100.f,LinearTargetVelocity.GetUnsafeNormal());

			if (Distance > SecurityDistance || DangerousTarget)
			{
				UseOrbitalBoost = true;
			}
		}

		LastTargetDistance = Distance;
	}

	if (AttackPhase == 2)
	{
		float SecurityDistance = (DangerousTarget ? 1200 : 800) + TargetSize * 4;
		if (Distance > SecurityDistance)
		{
			// Security distance reach
			LinearTargetVelocity = PredictedFireTargetAxis * PreferedVelocity;
			LinearTargetVelocity += LinearTargetVelocity.GetUnsafeNormal() * FVector::DotProduct(PilotTarget.GetLinearVelocity() / 100.f,LinearTargetVelocity.GetUnsafeNormal());

			AttackPhase = 0;
			if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
			{
				FLOG("Pass to phase 0");
			}
			ClearTarget = true;
		}
		else
		{
			LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * PreferedVelocity;
			LinearTargetVelocity += LinearTargetVelocity.GetUnsafeNormal() * FVector::DotProduct(PilotTarget.GetLinearVelocity() / 100.f,LinearTargetVelocity.GetUnsafeNormal());

			if (DangerousTarget)
			{
				UseOrbitalBoost = true;
			}
		}
	}

	if (TimeUntilNextReaction <= 0)
	{
		TimeUntilNextReaction = ReactionTime;

		WantFire = false;

		/*if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
		{
			FLOGV("FireAxis=%s", *FireAxis.ToString());

			DrawDebugSphere(Ship->GetWorld(), BaseLocation, 50, 12, FColor::Green, true);
			//DrawDebugSphere(Ship->GetWorld(), AmmoIntersectionLocation, 100, 12, FColor::Red, true);
		}*/
		if(PilotTarget != Ship->GetCurrentTarget())
		{
			Ship->SetCurrentTarget(PilotTarget);
		}

		// If at range and aligned fire on the target
		// TODO increase tolerance if target is near
		if (AmmoIntersectionTime > 0 && AmmoIntersectionTime < 1.5)
		{
			TArray <UFlareWeapon*> Weapons = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons;
			for (int WeaponIndex = 0; WeaponIndex < Weapons.Num(); WeaponIndex++)
			{

				UFlareWeapon* Weapon = Weapons[WeaponIndex];
				if (Weapon->GetUsableRatio() <= 0)
				{
					continue;
				}

				for (int GunIndex = 0; GunIndex < Weapon->GetGunCount(); GunIndex++)
				{
					FVector MuzzleLocation = Weapon->GetMuzzleLocation(GunIndex);

					// Compute target Axis for each gun
					FVector GunAmmoIntersectionLocation = AmmoIntersectionLocation;
					float GunAmmoIntersectionTime = AmmoIntersectionTime;



					//FVector GunFireTargetAxis = (GunAmmoIntersectionLocation - MuzzleLocation - AmmoIntersectionPredictedTime * ShipVelocity).GetUnsafeNormal();
					FVector GunFireTargetAxis = (GunAmmoIntersectionLocation - MuzzleLocation).GetUnsafeNormal();
					/*FLOGV("Gun %d FireAxis=%s", GunIndex, *FireAxis.ToString());
					FLOGV("Gun %d GunFireTargetAxis=%s", GunIndex, *GunFireTargetAxis.ToString());
		*/
					float AngularPrecisionDot = FVector::DotProduct(GunFireTargetAxis, Weapon->ComputeParallaxCorrection(GunIndex));
					float AngularPrecision = FMath::Acos(AngularPrecisionDot);
					float AngularSize = FMath::Atan(TargetSize / Distance);


					/*if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
					{

						DrawDebugSphere(Ship->GetWorld(), GunAmmoIntersectionLocation, 50, 12, FColor::Red, true);
						DrawDebugLine(Ship->GetWorld(), MuzzleLocation, GunAmmoIntersectionLocation, FColor::Red, true);
						DrawDebugLine(Ship->GetWorld(), MuzzleLocation, MuzzleLocation + GunFireTargetAxis *100000, FColor::Blue, true);
						//DrawDebugLine(Ship->GetWorld(), MuzzleLocation, MuzzleLocation + GunFireTargetAxis2 *100000, FColor::Green, true);
						DrawDebugLine(Ship->GetWorld(), MuzzleLocation, MuzzleLocation + Weapon->ComputeParallaxCorrection(GunIndex) *100000, FColor::White, true);
					}*/

					/*if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
					{
						FLOGV("GunFireTargetAxis=%s", *GunFireTargetAxis.ToString());
						FLOGV("GunFireTargetAxis=%s", *GunFireTargetAxis.ToString());
						FLOGV("ComputeParallaxCorrection=%s", *Weapon->ComputeParallaxCorrection(GunIndex).ToString());

						FLOGV("Gun %d Distance=%f", GunIndex, Distance);
						FLOGV("Gun %d TargetSize=%f", GunIndex, TargetSize);
						FLOGV("Gun %d AngularSize=%f", GunIndex, AngularSize);
						FLOGV("Gun %d AngularPrecision=%f", GunIndex, AngularPrecision);
						FLOGV("Gun %d limit=%f", GunIndex, (DangerousTarget ? AngularSize * 0.5 : AngularSize * 0.2));
					}*/

					if(PilotTarget.BombTarget)
					{
						AngularSize *= 10.f;
					}

					if (AngularPrecision < (DangerousTarget ? AngularSize * 0.5 : AngularSize * 0.2))
					{
						/*if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
						{
							FLOG("Gun precision ok, check FF");
						}*/
						if (!PilotHelper::CheckFriendlyFire(Ship->GetGame()->GetActiveSector(), PlayerCompany, MuzzleLocation, ShipVelocity, AmmoVelocity, GunFireTargetAxis, GunAmmoIntersectionTime, 0))
						{
							FVector Location = PilotTarget.GetActorLocation();
							FVector Velocity = PilotTarget.GetLinearVelocity() / 100;
							Weapon->SetTarget(Location, Velocity);
							WantFire = true;
							/*if(Ship->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip())
							{
								FLOG("Gun fire");
								//DrawDebugSphere(Ship->GetWorld(), PilotTargetShip->GetRootComponent()->Get, 50, 12, FColor::Cyan, true);
							}*/


							break;
						}
					}
				}
				if (WantFire)
				{
					break;
				}
			}
		}

	}
	/*
	// Manage orbital boost
	if (Ship->GetParent()->GetDamageSystem()->GetTemperature() > Ship->GetParent()->GetDamageSystem()->GetOverheatTemperature() * 0.75)
	{
		//UseOrbitalBoost = false;
	}
	*/

	// Find friend barycenter
	// Go to friend barycenter
	// If near
		// Turn to opposite from barycentre
	// else
		// Turn to direction

	float TimeSinceDamage = Ship->GetDamageSystem()->GetTimeSinceLastExternalDamage();

	if (TimeSinceDamage < 5.)
	{
		UseOrbitalBoost = true;
		FVector NoseAxis = Ship->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalNose);
		LinearTargetVelocity = LinearTargetVelocity.GetUnsafeNormal() * 0.5 + NoseAxis * 0.5 *  Ship->GetNavigationSystem()->GetLinearMaxVelocity() * 3.0;
	}

	// Exit avoidance
	if (PilotTarget.IsEmpty() ||
			Distance > 2000 ||
			(PilotTarget.SpacecraftTarget && !PilotTarget.SpacecraftTarget->GetParent()->GetDamageSystem()->IsUncontrollable()
			&& PilotTarget.SpacecraftTarget != Ship->GetGame()->GetPC()->GetShipPawn()))
	{
		LinearTargetVelocity = ExitAvoidance(Ship, LinearTargetVelocity, 0.8, DeltaSeconds);
	}

	// Anticollision
	LinearTargetVelocity = TryAnticollisionCorrection(Ship, LinearTargetVelocity, Ship->GetPreferedAnticollisionTime(), PilotHelper::AnticollisionConfig(), DeltaSeconds);


	if (ClearTarget)
	{
		PilotTarget.Clear();
	}
}

void UFlareShipPilot::BomberPilot(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_Bomber);

	FVector PilotAimTargetLocation = (PilotTargetShipComponent ? PilotTargetShipComponent->GetComponentLocation() : PilotTarget.GetActorLocation());


	// Weapon info
	UFlareWeapon* CurrentWeapon = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons[0];
	bool IsSalvage = CurrentWeapon->GetDescription()->WeaponCharacteristics.DamageType == EFlareShellDamageType::HeavySalvage
		|| CurrentWeapon->GetDescription()->WeaponCharacteristics.DamageType == EFlareShellDamageType::LightSalvage;

	// Get speed and location data
	LinearTargetVelocity = FVector::ZeroVector;
	FVector DeltaLocation = (PilotAimTargetLocation - Ship->GetActorLocation()) / 100.f;
	FVector TargetAxis = DeltaLocation.GetUnsafeNormal();
	float Distance = DeltaLocation.Size(); // Distance in meters

	// Attack Phases
	// 0 - Prepare attack : change velocity to approch the target
	// 1 - Attacking : target is approching with boost
	// 3 - Drop : Drop util its not safe to stay
	// 2 - Withdraw : target is passed, wait a security distance to attack again

	// Get mass coefficient for use as a reference
	float WeigthCoef = FMath::Sqrt(Ship->GetSpacecraftMass()) / FMath::Sqrt(5425.f) * (2-Ship->GetParent()->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_RCS)) ; // 1 for ghoul at 100%
	float PreferedVelocity = FMath::Max((PilotTarget.GetLinearVelocity().Size() / 100.f) * 3.0f, Ship->GetNavigationSystem()->GetLinearMaxVelocity());
	TimeUntilNextReaction /= 5;

	// Compute distances and reaction times
	float AlignTime = 12 * WeigthCoef;
	float DropTime = (IsSalvage ? 3 : 5) * WeigthCoef;
	float EvadeTime = 2 * WeigthCoef;
	float TimeBetweenDrop = (IsSalvage ? 5.0 : 0.5) * WeigthCoef;

	// Setup behaviour flags
	UseOrbitalBoost = false;
	bool WantClearTarget = false;
	bool AlignToSpeed = false;
	bool HardBoost = false;
	bool Anticollision = true;

	// Approach phase
	if (AttackPhase == 0)
	{
		// Target is approching, prepare attack
		float ApproachDistance = 15 * PreferedVelocity * WeigthCoef;
		if (Distance < ApproachDistance)
		{
			AttackPhase = 1;
			LockTarget = true;
		}
		else
		{
			LinearTargetVelocity = TargetAxis * PreferedVelocity;
			// Exit avoidance
			LinearTargetVelocity = ExitAvoidance(Ship, LinearTargetVelocity, 0.5, DeltaSeconds);
			AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), TargetAxis, FVector::ZeroVector, DeltaSeconds);
			UseOrbitalBoost = true;
		}
	}

	// Attack phase
	if (AttackPhase == 1)
	{
		FVector AmmoIntersectionLocation;
		float AmmoVelocity = CurrentWeapon->GetAmmoVelocity() * 100;
		float AmmoIntersectionTime = SpacecraftHelper::GetIntersectionPosition(PilotAimTargetLocation, PilotTarget.GetLinearVelocity(), Ship->GetActorLocation(), Ship->Airframe->GetPhysicsLinearVelocity(), AmmoVelocity, 0.0, &AmmoIntersectionLocation);

		AlignToSpeed = true;

		// Near enough, start bombing
		if (AmmoIntersectionTime > 0 && AmmoIntersectionTime < DropTime)
		{
			AttackPhase = 2;
			LastWantFire = false;
			TimeBeforeNextDrop = 0;
		}
		else if (AmmoIntersectionTime > 0 && AmmoIntersectionTime < AlignTime)
		{
			FVector ChargeAxis = (AmmoIntersectionLocation - Ship->GetActorLocation()).GetUnsafeNormal();
			LinearTargetVelocity = ChargeAxis * PreferedVelocity;
			UseOrbitalBoost = true;
			HardBoost = true;
			Anticollision = false;
		}
		else
		{
			LinearTargetVelocity = TargetAxis * PreferedVelocity;
		}

		LastTargetDistance = Distance;
	}

	// Bombing phase
	if (AttackPhase == 2)
	{
		FVector AmmoIntersectionLocation;
		float AmmoVelocity = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons[0]->GetAmmoVelocity() * 100;
		float AmmoIntersectionTime = SpacecraftHelper::GetIntersectionPosition(PilotAimTargetLocation, PilotTarget.GetLinearVelocity(), Ship->GetActorLocation(), Ship->Airframe->GetPhysicsLinearVelocity(), AmmoVelocity, 0.0, &AmmoIntersectionLocation);
		FVector FrontVector = Ship->GetFrontVector();
		FVector ChargeAxis = (AmmoIntersectionLocation - Ship->GetActorLocation()).GetUnsafeNormal();

		Anticollision = false;
		AlignToSpeed = true;

		// Security distance reached
		if (AmmoIntersectionTime < EvadeTime || FVector::DotProduct(FrontVector, ChargeAxis) < 0.6 || AmmoIntersectionTime > AlignTime)
		{
			AttackPhase = 3;
		}
		else if (FVector::DotProduct(FrontVector, ChargeAxis) > 0.9 && AmmoIntersectionTime < DropTime)
		{
			if (TimeBeforeNextDrop > 0)
			{
				TimeBeforeNextDrop -= DeltaSeconds;
			}
			else
			{
				WantFire = !LastWantFire;
				LastWantFire = WantFire;
				if (WantFire)
				{
					TimeBeforeNextDrop = TimeBetweenDrop;
					WantClearTarget = true;
				}
			}

			LinearTargetVelocity = ChargeAxis * PreferedVelocity;
		}
	}

	// Evasion phase
	if (AttackPhase == 3)
	{
		// Security distance reached
		float SecurityDistance = 1500;
		FVector DeltaVelocity = (PilotTarget.GetLinearVelocity() / 100.f - Ship->GetLinearVelocity()) / 100.;
		if (Distance > SecurityDistance)
		{
			AttackPhase = 0;
			WantClearTarget = true;
			LockTarget = false;
		}
		else if (FVector::DotProduct(DeltaLocation, DeltaVelocity) < 0)
		{
			AlignToSpeed = true;
			FQuat AvoidQuat = FQuat(DeltaLocation.GetUnsafeNormal(), AttackAngle);
			FVector TopVector = Ship->GetActorRotation().RotateVector(FVector(0,0,PilotTarget.GetMeshScale()));
			FVector Avoid =  AvoidQuat.RotateVector(TopVector);

			LinearTargetVelocity = Avoid.GetUnsafeNormal() * PreferedVelocity;
			UseOrbitalBoost = true;
			HardBoost = true;
		}
		else
		{
			UseOrbitalBoost = true;
			HardBoost = true;
			LinearTargetVelocity = -TargetAxis * PreferedVelocity * 2;
			AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), -TargetAxis, FVector::ZeroVector, DeltaSeconds);
		}
	}
	
	// Compute speed
	if (AlignToSpeed)
	{
		if (Ship->GetLinearVelocity().IsNearlyZero())
		{
			AngularTargetVelocity = FVector::ZeroVector;
		}
		else
		{
			FVector LinearVelocityAxis = Ship->GetLinearVelocity().GetUnsafeNormal();
			AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), LinearVelocityAxis, FVector::ZeroVector, DeltaSeconds);
		}
	}
	else
	{
		AlignToTargetVelocityWithThrust(DeltaSeconds);
	}

	// Anticollision
	if (Anticollision)
	{
		PilotHelper::AnticollisionConfig IgnoreConfig;
		IgnoreConfig.SpacecraftToIgnore = PilotTarget.SpacecraftTarget;
		LinearTargetVelocity = TryAnticollisionCorrection(Ship, LinearTargetVelocity, Ship->GetPreferedAnticollisionTime(), IgnoreConfig, DeltaSeconds);
	}
	
	if (WantClearTarget)
	{
		ClearTarget();
	}
}

void UFlareShipPilot::MissilePilot(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_Missile);

	FVector PilotAimTargetLocation = (PilotTargetShipComponent ? PilotTargetShipComponent->GetComponentLocation() : PilotTarget.GetActorLocation());

	// Weapon info
	UFlareWeapon* CurrentWeapon = Ship->GetWeaponsSystem()->GetWeaponGroup(SelectedWeaponGroupIndex)->Weapons[0];
	bool IsSalvage = CurrentWeapon->GetDescription()->WeaponCharacteristics.DamageType == EFlareShellDamageType::HeavySalvage
		|| CurrentWeapon->GetDescription()->WeaponCharacteristics.DamageType == EFlareShellDamageType::LightSalvage;

	// Get speed and location data
	LinearTargetVelocity = FVector::ZeroVector;
	FVector DeltaLocation = (PilotAimTargetLocation - Ship->GetActorLocation()) / 100.f;
	FVector TargetAxis = DeltaLocation.GetUnsafeNormal();
	float Distance = DeltaLocation.Size(); // Distance in meters

	// Attack Phases
	// 0 - Prepare attack : change velocity to approch the target
	// 1 - Attacking : target is approching with boost
	// 3 - Drop : Drop util its not safe to stay
	// 2 - Withdraw : target is passed, wait a security distance to attack again

	// Get mass coefficient for use as a reference
	float WeigthCoef = FMath::Sqrt(Ship->GetSpacecraftMass()) / FMath::Sqrt(5425.f) * (2-Ship->GetParent()->GetDamageSystem()->GetSubsystemHealth(EFlareSubsystem::SYS_RCS)) ; // 1 for ghoul at 100%
	float PreferedVelocity = FMath::Max((PilotTarget.GetLinearVelocity().Size() / 100.f) * 3.0f, Ship->GetNavigationSystem()->GetLinearMaxVelocity());
	TimeUntilNextReaction /= 5;

	// Compute distances and reaction times
	float ApproachDistance = 500000; // 5 km
	float TimeBetweenDrop = (IsSalvage ? 5.0 : 0.5) * WeigthCoef;

	// Setup behaviour flags
	UseOrbitalBoost = false;

	if (Distance < ApproachDistance)
	{
		LinearTargetVelocity = -TargetAxis * PreferedVelocity * 0.2;
	}
	else
	{
		LinearTargetVelocity = TargetAxis * PreferedVelocity;
	}

	AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), TargetAxis, FVector::ZeroVector, DeltaSeconds);
	UseOrbitalBoost = true;
	FVector FrontVector = Ship->GetFrontVector();

	if (Distance < ApproachDistance && FVector::DotProduct(FrontVector, TargetAxis) > 0.8)
	{
		// Attack
		if (TimeBeforeNextDrop > 0)
		{
			TimeBeforeNextDrop -= DeltaSeconds;
		}
		else
		{
			WantFire = !LastWantFire;
			LastWantFire = WantFire;
			if (WantFire)
			{
				ClearTarget();
				TimeBeforeNextDrop = TimeBetweenDrop;
			}
		}
	}

	// Exit avoidance
	LinearTargetVelocity = ExitAvoidance(Ship, LinearTargetVelocity, 0.5, DeltaSeconds);

	// Anticollision
	PilotHelper::AnticollisionConfig IgnoreConfig;
	IgnoreConfig.SpacecraftToIgnore = PilotTarget.SpacecraftTarget;
	LinearTargetVelocity = TryAnticollisionCorrection(Ship, LinearTargetVelocity, Ship->GetPreferedAnticollisionTime(), IgnoreConfig, DeltaSeconds);
}

void UFlareShipPilot::IdlePilot(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_Idle);

	// TODO find better
	//UseOrbitalBoost = false;

	// If there is enemy fly away

	FindPilotAvoidShip(DeltaSeconds);
	// If enemy near, run away !
	if (PilotAvoidShip)
	{
		FVector DeltaLocation = (PilotAvoidShip->GetActorLocation() - Ship->GetActorLocation()) / 100.f;
		float Distance = DeltaLocation.Size(); // Distance in meters

		// There is at least one hostile enemy
		if (Distance < 10000) // 10 km
		{
			Ship->GetNavigationSystem()->AbortAllCommands();
			LinearTargetVelocity = -DeltaLocation.GetUnsafeNormal() * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
			UseOrbitalBoost = true;
		}
/*
		if (Distance > 2000 && Ship->GetParent()->GetDamageSystem()->GetTemperature() > Ship->GetParent()->GetDamageSystem()->GetOverheatTemperature() * 0.95)
		{
			// Too hot and no imminent danger
			//UseOrbitalBoost = false;
		}
*/
	}

	// If not, and outside the player fleet, find a leader and follow it
	else
	{
		// Ship is refilling or repairing, find a station to dock with
		if (!Ship->GetParent()->GetShipMaster() && (Ship->GetParent()->IsRefilling() || Ship->GetParent()->IsRepairing()))
		{
			if (!Ship->GetNavigationSystem()->IsDocked())
			{
				TimeSinceLastDockingAttempt += DeltaSeconds;
				GetRandomFriendlyStation();
				DockAtStation(DeltaSeconds);
			}
			return;
		}

		if (!LeaderShip || (LeaderShip && (!LeaderShip->GetParent()->GetDamageSystem()->IsAlive()) || LeaderShip->GetParent()->GetDamageSystem()->IsUncontrollable()) || !FoundOutOfCombatLeader)
		{
			GetNewLeaderShip();
			FoundOutOfCombatLeader = true;
			InitiatedCombat = false;
		}

		// If is the leader, find a location in a 10 km radius and patrol
		if (LeaderShip == Ship)
		{
			// Go to a random point at 10000 m from the center

			// If at less than 500 m from this point, get another random point

			float TargetLocationToShipDistance = (PilotTargetLocation - Ship->GetActorLocation()).Size();

			if (TargetLocationToShipDistance < 50000 || PilotTargetLocation.IsZero())
			{
				FVector PatrolCenter = FVector::ZeroVector;

				// Use random station
				AFlareSpacecraft* NearestStation =  GetNearestAvailableStation(true);

				if (NearestStation)
				{
					PatrolCenter = NearestStation->GetActorLocation();
				}

				PilotTargetLocation = PatrolCenter + FMath::VRand() * FMath::FRand() * 400000;
			}
			LinearTargetVelocity = (PilotTargetLocation - Ship->GetActorLocation()).GetUnsafeNormal()  * Ship->GetNavigationSystem()->GetLinearMaxVelocity() * 0.8;
		}
		else
		{
			FollowLeaderShip();
		}
	}

	AngularTargetVelocity = FVector::ZeroVector;
	 
	// Exit avoidance
	if (Ship->GetCompany() != Ship->GetGame()->GetPC()->GetCompany())
	{
		LinearTargetVelocity = ExitAvoidance(Ship, LinearTargetVelocity, 0.5, DeltaSeconds);
	}

	AlignToTargetVelocityWithThrust(DeltaSeconds);

	// Anticollision
	LinearTargetVelocity = TryAnticollisionCorrection(Ship, LinearTargetVelocity, Ship->GetPreferedAnticollisionTime(), PilotHelper::AnticollisionConfig(), DeltaSeconds);

    //FLOGV("%s Leader ship LinearTargetVelocity=%s", *LinearTargetVelocity.ToString());
}

FVector UFlareShipPilot::TryAnticollisionCorrection(AFlareSpacecraft* TargetShip, FVector InitialVelocity, float PreventionDuration, PilotHelper::AnticollisionConfig IgnoreConfig,float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_TryAntiCollision);
	LastNewCollisionVector -= DeltaSeconds;
	if (LastNewCollisionVector <= 0)
	{
		PreviousAntiCollisionVector = PilotHelper::AnticollisionCorrection(TargetShip, LinearTargetVelocity, TargetShip->GetPreferedAnticollisionTime(), IgnoreConfig, PILOT_ANTICOLLISION_SPEED);
		if (PreviousAntiCollisionVector == InitialVelocity)
		{
			LastNewCollisionVector = CollisionVectorReactionTimeSlow;
		}
		else
		{
			LastNewCollisionVector = CollisionVectorReactionTimeFast;
		}
	}
	return PreviousAntiCollisionVector;
}

void UFlareShipPilot::FollowLeaderShip(int32 DefaultRadius)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_FollowLeaderShip);

	if (Ship->IsCapableCarrier() && !Ship->GetHasUndockedInternalShips())
	{
		return;
	}

	// Follow the leader

	if (Ship->GetCompany() == Ship->GetGame()->GetPC()->GetCompany())
	{
		// To keep current with the current player ship
		GetNewLeaderShip();
	}

	float FollowRadius = DefaultRadius + FMath::Pow(LeaderShip->GetInSectorSquad().Num() * 4 * FMath::Pow(10000, 3), 1 / 3.);
	if ((LeaderShip->GetActorLocation() - Ship->GetActorLocation()).Size() < FollowRadius)
	{
		LinearTargetVelocity = LeaderShip->GetLinearVelocity();
	}
	else
	{
		LinearTargetVelocity = (LeaderShip->GetActorLocation() - Ship->GetActorLocation()).GetUnsafeNormal()  * Ship->GetNavigationSystem()->GetLinearMaxVelocity();
	}
}

void UFlareShipPilot::GetNewLeaderShip()
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_GetNewLeaderShip);

	AFlareSpacecraft* OldLeaderShip = LeaderShip;

	if (Ship->GetCompany() == Ship->GetGame()->GetPC()->GetCompany())
	{
		LeaderShip = Ship->GetGame()->GetPC()->GetShipPawn();
		if (LeaderShip && LeaderShip->GetParent()->GetDamageSystem()->IsAlive())
		{
			SelectedNewLeader(OldLeaderShip);
			return;
		}
	}

	TArray<AFlareSpacecraft*> Spacecrafts = Ship->GetGame()->GetActiveSector()->GetCompanySpacecrafts(Ship->GetCompany());
	AFlareSpacecraft* NewLeaderShip = LeaderShip;

	for (int ShipIndex = 0; ShipIndex < Spacecrafts.Num(); ShipIndex++)
	{
		AFlareSpacecraft* CandidateShip = Spacecrafts[ShipIndex];
		if (Ship == CandidateShip)
		{
			continue;
		}

		if (!CandidateShip->IsMilitary())
		{
			continue;
		}

		if (!LeaderShip)
		{
			LeaderShip = CandidateShip;
			continue;
		}

		float LeaderMass = LeaderShip->GetSpacecraftMass();
		float CandidateMass = CandidateShip->GetSpacecraftMass();

		if (LeaderMass == CandidateMass)
		{
			if (LeaderShip->GetImmatriculation() < CandidateShip->GetImmatriculation())
			{
				continue;
			}
		}
		else if (LeaderMass > CandidateMass)
		{
			continue;
		}

		if (InitiatedCombat)
		{
			int32 LeaderShipSquadQuantity = CandidateShip->GetInSectorSquad().Num();
			int32 MaximumShips = 1;

			if (CandidateShip->GetSize() == EFlarePartSize::L)
			{
				MaximumShips = LEADER_SQUAD_LARGE;
			}
			else
			{
				MaximumShips = LEADER_SQUAD_SMALL;
			}

			if (LeaderShipSquadQuantity >= MaximumShips)
			{
				continue;
			}
		}

		NewLeaderShip = CandidateShip;
		break;
	}

	if (NewLeaderShip)
	{
		LeaderShip = NewLeaderShip;
	}
	else
		//couldn't find a leader so we'll resort to setting ourself
	{
		LeaderShip = Ship;
	}

	SelectedNewLeader(OldLeaderShip);
}

void UFlareShipPilot::SelectedNewLeader(AFlareSpacecraft* OldLeaderShip)
{
	if (LeaderShip)
	{
		if (LeaderShip != Ship)
		{
			LeaderShip->AddToInsectorSquad(Ship);
		}
		if (OldLeaderShip&&OldLeaderShip != LeaderShip)
		{
			OldLeaderShip->RemoveFromInsectorSquad(Ship);
		}
	}
}


void UFlareShipPilot::FlagShipPilot(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_Flagship);

	// Go to a random point at 800 m from the target

	// If at less than 50 m from this point, get another random point

	// If the target to farther than 2000 to the target point, change point

	if (LeaderShip != Ship)
	{
		if (LeaderShip->GetParent()->GetDamageSystem()->IsDisarmed() || LeaderShip->GetParent()->GetDamageSystem()->IsUncontrollable())
		{
			LeaderShip = Ship;
		}
		else
		{
			FollowLeaderShip(125000);
			return;
		}
	}
	FlagShipMovement(DeltaSeconds);
}

void UFlareShipPilot::FlagShipMovement(float DeltaSeconds)
{
	if (Ship->IsCapableCarrier() && !Ship->GetHasUndockedInternalShips())
	{
		return;
	}

	float TargetLocationToTargetShipDistance = (PilotTargetLocation - PilotTarget.GetActorLocation()).Size();
	float TargetLocationToShipDistance = (PilotTargetLocation - Ship->GetActorLocation()).Size();

	bool NewTargetLocation = false;
	if (TargetLocationToTargetShipDistance > 200000)
	{
		// Target location too far from target ship
		NewTargetLocation = true;
	}
	else if (TargetLocationToShipDistance < 50000)
	{
		// Near to target location
		NewTargetLocation = true;
	}

	if (NewTargetLocation || PilotTargetLocation.IsZero())
	{ 
		PilotTargetLocation = FMath::VRand() * FMath::FRand() * 80000 + PilotTarget.GetActorLocation();
		//FLOGV("New TargetDeltaLocation %f", (PilotTargetLocation - PilotTarget.GetActorLocation()).Size());
		//UKismetSystemLibrary::DrawDebugSphere(Ship->GetWorld(), PilotTargetLocation, 1000, 12, FColor::Red, 1000.f);
	}

	//UKismetSystemLibrary::DrawDebugSphere(Ship->GetWorld(), PilotTargetLocation, 20, 12, FColor::Orange, 1000.f);
	//UKismetSystemLibrary::DrawDebugPoint(Ship->GetWorld(), PilotTarget.GetActorLocation(), 2, FColor::Green, 1000.f);
	//FLOGV("FlagShipPilot PilotTargetLocation: %s PilotTarget.GetActorLocation(): %s", *PilotTargetLocation.ToString(), *PilotTarget.GetActorLocation().ToString());
	//FLOGV("TargetDeltaLocation %f", (PilotTargetLocation - PilotTarget.GetActorLocation()).Size());

	LinearTargetVelocity = (PilotTargetLocation - Ship->GetActorLocation()).GetUnsafeNormal()  * Ship->GetNavigationSystem()->GetLinearMaxVelocity() * 2;
	LinearTargetVelocity += LinearTargetVelocity.GetUnsafeNormal() * FVector::DotProduct(PilotTarget.GetLinearVelocity() / 100.f,LinearTargetVelocity.GetUnsafeNormal());

	// TODO Bomb avoid

	FVector DeltaLocation = PilotTarget.GetActorLocation() - Ship->GetActorLocation();
	float Distance = DeltaLocation.Size() * 0.01f; // Distance in meters

	//FLOGV("DeltaLocation %f", DeltaLocation.Size());

	AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), DeltaLocation.GetUnsafeNormal(), FVector::ZeroVector, DeltaSeconds);
	AngularTargetVelocity = AngularTargetVelocity.GetClampedToMaxSize(10.f);

	//FLOGV("LinearTargetVelocity: %s AngularTargetVelocity: %s", *LinearTargetVelocity.ToString(), *AngularTargetVelocity.ToString())
	//FLOGV("AngularVelocity: %s", *Ship->Airframe->GetPhysicsAngularVelocity().ToString())

	// Exit avoidance
	if(PilotTarget.IsEmpty() || Distance > 2000 || (!PilotTarget.Is(Ship->GetGame()->GetPC()->GetShipPawn())))
	{
		LinearTargetVelocity = ExitAvoidance(Ship, LinearTargetVelocity, 0.7, DeltaSeconds);
	}

	// Anticollision
	LinearTargetVelocity = TryAnticollisionCorrection(Ship, LinearTargetVelocity, Ship->GetPreferedAnticollisionTime(), PilotHelper::AnticollisionConfig(), DeltaSeconds);
	FVector FrontAxis = Ship->Airframe->GetComponentToWorld().GetRotation().RotateVector(FVector(1,0,0));

	if (FVector::DotProduct(FrontAxis, LinearTargetVelocity.GetUnsafeNormal()) > 0.9 && (LinearTargetVelocity - Ship->Airframe->GetPhysicsLinearVelocity()).Size() > 500)
	{
		UseOrbitalBoost = true;
	}

	// Find friend barycenter
	// Go to friend barycenter
	// If near
		// Turn to opposite from barycentre
	// else
		// Turn to direction

	WantFire = false;
	/*
	// Manage orbital boost
	if (Ship->GetParent()->GetDamageSystem()->GetTemperature() > Ship->GetParent()->GetDamageSystem()->GetOverheatTemperature() * 0.9)
	{
		//UseOrbitalBoost = false;
	}
	*/
}



void UFlareShipPilot::FindBestHostileTarget(EFlareCombatTactic::Type Tactic)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_FindBestHostileTarget);

	PilotHelper::PilotTarget TargetCandidate;

	struct PilotHelper::TargetPreferences TargetPreferences;
	TargetPreferences.IsLarge = 1;
	TargetPreferences.IsSmall = 1;
	TargetPreferences.IsStation = Ship->GetCompany()->IsPlayerCompany() ? 0.f : 1.f;
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
	TargetPreferences.MaxDistance = 1000000;
	TargetPreferences.DistanceWeight = 0.5;
	TargetPreferences.AttackTarget = NULL;
	TargetPreferences.AttackTargetWeight = 15;
	TargetPreferences.AttackMeWeight = 10;
	TargetPreferences.LastTarget = PilotTarget;
	TargetPreferences.LastTargetWeight = 10;
	TargetPreferences.PreferredDirection = Ship->GetFrontVector();
	TargetPreferences.MinAlignement = -1;
	TargetPreferences.AlignementWeight = 0.5;
	TargetPreferences.BaseLocation = Ship->GetActorLocation();
	TargetPreferences.IsBomb = 5.f;
	TargetPreferences.MaxBombDistance = 200000.f;

	TargetPreferences.IsMeteorite = 0.0001f;

	Ship->GetWeaponsSystem()->UpdateTargetPreference(TargetPreferences);


	float MinAmmoRatio = 1.f;


	UFlareSpacecraftComponentsCatalog* Catalog = Ship->GetGame()->GetShipPartsCatalog();
	for (int32 ComponentIndex = 0; ComponentIndex < Ship->GetData().Components.Num(); ComponentIndex++)
	{
		FFlareSpacecraftComponentSave* ComponentData = &Ship->GetData().Components[ComponentIndex];
		FFlareSpacecraftComponentDescription* ComponentDescription = Catalog->Get(ComponentData->ComponentIdentifier);

		if (ComponentDescription->Type == EFlarePartType::Weapon)
		{
			float AmmoRatio = float(ComponentDescription->WeaponCharacteristics.AmmoCapacity - ComponentData->Weapon.FiredAmmo) / ComponentDescription->WeaponCharacteristics.AmmoCapacity;
			if (AmmoRatio < MinAmmoRatio)
			{
				MinAmmoRatio = AmmoRatio;
			}
		}
	}
	//FLOGV("%s MinAmmoRatio=%f", *Ship->GetImmatriculation().ToString(), MinAmmoRatio);

	if (MinAmmoRatio < 0.9)
	{
		TargetPreferences.IsUncontrollableSmallMilitary = 0.0;
	}

	if (MinAmmoRatio < 0.5)
	{
		TargetPreferences.IsNotMilitary = 0.0;
	}

	if (Tactic == EFlareCombatTactic::AttackStations)
	{
		TargetPreferences.IsStation = 10.0f;
	}
	else if (Tactic == EFlareCombatTactic::AttackMilitary)
	{
		TargetPreferences.IsStation = 0.0f;
	}
	else if (Tactic == EFlareCombatTactic::AttackCivilians)
	{
		TargetPreferences.IsStation = 0.0f;
		TargetPreferences.IsMilitary = 0.1;
		TargetPreferences.IsNotMilitary = 1.0;
		TargetPreferences.IsNotDangerous = 1.0;
	}
	else if (Tactic == EFlareCombatTactic::ProtectMe)
	{
		TargetPreferences.IsStation = 0.0f;
		// Protect me is only available for player ship
		if (Ship->GetCompany() == Ship->GetGame()->GetPC()->GetCompany())
		{
			TargetPreferences.AttackTarget = Ship->GetGame()->GetPC()->GetShipPawn();
			TargetPreferences.AttackTargetWeight = 1.0;
		}
	}

	TargetCandidate = PilotHelper::GetBestTarget(Ship, TargetPreferences);
	SelectNewHostileTarget(TargetCandidate);
}

void UFlareShipPilot::SelectNewHostileTarget(PilotHelper::PilotTarget TargetCandidate)
{
	if (TargetCandidate.IsValid())
	{
		bool NewTarget = false;
		bool NewWeapon = false;

		if(TargetCandidate != PilotTarget)
		{
			if(LastPilotTarget != PilotTarget && LastPilotTarget != TargetCandidate)
			{
				TimeSinceAiming = 0;
			}

			PilotTarget = TargetCandidate;
			Ship->SetCurrentTarget(PilotTarget);
			LastPilotTarget = TargetCandidate;

			NewTarget = true;
		}

		// Find best weapon
		int32 WeaponGroupIndex = Ship->GetWeaponsSystem()->FindBestWeaponGroup(PilotTarget);
		if (SelectedWeaponGroupIndex != WeaponGroupIndex)
		{
			SelectedWeaponGroupIndex = WeaponGroupIndex;
			NewWeapon = true;
		}

		if (NewTarget || NewWeapon)
		{
			AttackPhase = 0;
			AttackAngle = FMath::FRandRange(0, 360);
			float TargetSize = PilotTarget.GetMeshScale() / 100.f; // Radius in meters
			AttackDistance = FMath::FRandRange(100, 200) + TargetSize;
			MaxFollowDistance = TargetSize * 60; // Distance in meters
			TimeBeforeNextDrop = FMath::FRandRange(0.1, 1.0);
			TimeSinceAiming /= 2;
			InitiatedCombat = true;
			LockTarget = false;
		}
	}
	else
	{
		PilotTarget.Clear();
		SelectedWeaponGroupIndex = -1;
	}
}

bool UFlareShipPilot::GetInitiatedCombat()
{
	return InitiatedCombat;
}

void UFlareShipPilot::ClearInvalidTarget(PilotHelper::PilotTarget InvalidTarget)
{
	if(PilotTarget == InvalidTarget)
	{
		PilotTarget.Clear();
	}

	if(LastPilotTarget == InvalidTarget)
	{
		LastPilotTarget.Clear();
	}
}

int32 UFlareShipPilot::GetPreferedWeaponGroup() const
{
	return SelectedWeaponGroupIndex;
}

/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

FVector UFlareShipPilot::ExitAvoidance(AFlareSpacecraft* TargetShip, FVector InitialVelocityTarget, float CurveTrajectoryLimit, float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_ExitAvoidance);

	// DEBUG
	/*if(TargetShip->GetImmatriculation() != "PIRSPX-Arrow")
	{

		return InitialVelocityTarget;
	}*/

	TimeUntilNextExitAvoidanceCheck -= DeltaSeconds;
	if (TimeUntilNextExitAvoidanceCheck > 0 && !RunningExitAvoidance)
	{
		return InitialVelocityTarget;
	}

	float TimeToStop;
	FVector CurrentVelocity = Ship->GetLinearVelocity();

	if (FMath::IsNearlyZero(CurrentVelocity.SizeSquared()))
	{
		TimeToStop = 0;
	}
	else
	{

		FVector CurrentVelocityAxis = CurrentVelocity.GetUnsafeNormal();

		// TODO Cache
		TArray<UActorComponent*> Engines = Ship->GetComponentsByClass(UFlareEngine::StaticClass());


		FVector Acceleration = Ship->GetNavigationSystem()->GetTotalMaxThrustInAxis(Engines, CurrentVelocityAxis, false) / Ship->GetSpacecraftMass();
		float AccelerationInAngleAxis =  FMath::Abs(FVector::DotProduct(Acceleration, CurrentVelocityAxis));

		TimeToStop= (CurrentVelocity.Size() / (AccelerationInAngleAxis));
	}

	float DistanceToStop = (CurrentVelocity.Size() / (2)) * (Ship->GetPreferedAnticollisionTime());


	FVector SectorCenter = TargetShip->GetGame()->GetActiveSector()->GetSectorCenter();


/*	DrawDebugLine(TargetShip->GetWorld(), TargetShip->GetActorLocation(), TargetShip->GetGame()->GetPC()->GetPlayerShip()->GetActive()->GetActorLocation(), FColor::Cyan, false, 1.0);
	DrawDebugLine(TargetShip->GetWorld(),  FVector::ZeroVector, TargetShip->GetGame()->GetPC()->GetPlayerShip()->GetActive()->GetActorLocation(), FColor::Cyan, false, 1.0);
	DrawDebugLine(TargetShip->GetWorld(), FVector::ZeroVector, TargetShip->GetActorLocation(), FColor::Cyan, false, 1.0);


	DrawDebugLine(TargetShip->GetWorld(), TargetShip->GetActorLocation(), TargetShip->GetActorLocation() + TargetShip->GetVelocity() * 1000, FColor::Red, false, 1.0);
	DrawDebugLine(TargetShip->GetWorld(), TargetShip->GetActorLocation(), TargetShip->GetActorLocation() + InitialVelocityTarget * 1000, FColor::Blue, false, 1.0);
*/

	float ShipCenterDistance = (TargetShip->GetActorLocation() - SectorCenter).Size();
	float SectorLimits = TargetShip->GetGame()->GetActiveSector()->GetSectorLimits() / 3;
	/*FLOGV("%s ExitAvoidance ShipCenterDistance=%f SectorLimits=%f InitialVelocityTarget=%s",
		  *TargetShip->GetImmatriculation().ToString(),
		  ShipCenterDistance /100, SectorLimits /100, *InitialVelocityTarget.ToString());


	FLOGV("CurrentVelocity %s", *CurrentVelocity.ToString());
	FLOGV("TimeToFinalVelocity %f", TimeToStop);
	FLOGV("DistanceToStop %f", DistanceToStop);
	FLOGV("SectorLimits * CurveTrajectoryLimit %f", SectorLimits * CurveTrajectoryLimit);
*/
	if(DistanceToStop * 100 > SectorLimits * CurveTrajectoryLimit)
	{
		float OverflowRatio = (DistanceToStop * 100) / (SectorLimits * CurveTrajectoryLimit);
		// Clamp target velocity size
		InitialVelocityTarget = InitialVelocityTarget.GetClampedToSize(0, (CurrentVelocity.Size()) / OverflowRatio);

	//	FLOGV("Clamp target velocity %s (%f) OverflowRatio %f", *InitialVelocityTarget.ToString(), InitialVelocityTarget.Size(), OverflowRatio);
	}


	if (ShipCenterDistance > SectorLimits * CurveTrajectoryLimit)
	{
		RunningExitAvoidance = true;
		// Curve the trajectory to avoid exit
		float CurveRatio = FMath::Min(1.f, (ShipCenterDistance - SectorLimits * CurveTrajectoryLimit) / (SectorLimits * (1-CurveTrajectoryLimit)));


		FVector CenterDirection = (TargetShip->GetGame()->GetActiveSector()->GetSectorCenter() - TargetShip->GetActorLocation()).GetUnsafeNormal();


		//DrawDebugLine(TargetShip->GetWorld(), TargetShip->GetActorLocation(), TargetShip->GetActorLocation() + CenterDirection * 100000, FColor::White, false, 1.0);

		//FLOGV("CenterDirection %s", *CenterDirection.ToCompactString());


		if (InitialVelocityTarget.IsNearlyZero()) {
			//FLOGV("TargetShip->GetNavigationSystem()->GetLinearMaxVelocity() %f", TargetShip->GetNavigationSystem()->GetLinearMaxVelocity());
			//DrawDebugLine(TargetShip->GetWorld(), TargetShip->GetActorLocation(), TargetShip->GetActorLocation() + CenterDirection * TargetShip->GetNavigationSystem()->GetLinearMaxVelocity() * 10000, FColor::Green, false, 1.0);

			return CenterDirection * TargetShip->GetNavigationSystem()->GetLinearMaxVelocity();
		}
		else
		{
			FVector InitialVelocityTargetDirection = InitialVelocityTarget.GetUnsafeNormal();

			FVector ExitAvoidanceDirection = (CurveRatio * CenterDirection + (1 - CurveRatio) * InitialVelocityTargetDirection).GetUnsafeNormal();
			FVector ExitAvoidanceVelocity = ExitAvoidanceDirection *  InitialVelocityTarget.Size();

			/*FLOGV("CurveRatio %f", CurveRatio);

			FLOGV("InitialVelocityTargetDirection %s", *InitialVelocityTargetDirection.ToCompactString());
			FLOGV("ExitAvoidanceDirection %s", *ExitAvoidanceDirection.ToCompactString());
			FLOGV("ExitAvoidanceVelocity %s %f", *ExitAvoidanceVelocity.ToCompactString(), ExitAvoidanceVelocity.Size());
			DrawDebugLine(TargetShip->GetWorld(), TargetShip->GetActorLocation(), TargetShip->GetActorLocation() + ExitAvoidanceVelocity * 10000, FColor::Green, false, 1.0);
*/
			return ExitAvoidanceVelocity;
		}
		TimeUntilNextExitAvoidanceCheck = NextExitAvoidanceCheckReactionFast;
	}
	else
	{
		TimeUntilNextExitAvoidanceCheck = NextExitAvoidanceCheckReactionSlow;
		RunningExitAvoidance = false;
		return InitialVelocityTarget;
	}
}

void UFlareShipPilot::FindPilotAvoidShip(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareShipPilot_AvoidShip);
	TimeUntilNextPilotAvoidCheck -= DeltaSeconds;

	if (PilotAvoidShip!= nullptr && IsValid(PilotAvoidShip))
	{
		if (PilotAvoidShip->IsSafeDestroying())
		{
			TimeUntilNextPilotAvoidCheck = 0;
		}
	}

	if (TimeUntilNextPilotAvoidCheck <= 0)
	{
		GetNearestHostileShip(true);

		if (NearestHostileShipSmall)
		{
			PilotAvoidShip = NearestHostileShipSmall;
		}
		else
		{
			PilotAvoidShip = NearestHostileShipLarge;
		}

		if (!PilotAvoidShip)
		{
			TimeUntilNextPilotAvoidCheck = PilotAvoidCheckReactionTimeSlow;
			InitiatedCombat = false;
		}
		else
		{
			TimeUntilNextPilotAvoidCheck = PilotAvoidCheckReactionTimeFast;
			InitiatedCombat = true;
		}
	}
}

void UFlareShipPilot::GetNearestHostileShip(bool DangerousOnly)
{
	// For now an host ship is a the nearest host ship with the following critera:
	// - Alive
	// - Is dangerous if needed
	// - From another company
	// - Is the nearest

	NearestHostileShipSmall = NULL;
	NearestHostileShipLarge = NULL;

	if (!Ship || Ship->IsPendingKill() || Ship->IsSafeDestroying() || !Ship->GetGame()->GetActiveSector())
	{
		return;
	}

	FVector PilotLocation = Ship->GetActorLocation();
	float MinDistanceSquaredSmall = -1;
	float MinDistanceSquaredLarge = -1;

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Ship->GetGame()->GetActiveSector()->GetSpacecrafts().Num(); SpacecraftIndex++)
	{
		AFlareSpacecraft* ShipCandidate = Ship->GetGame()->GetActiveSector()->GetSpacecrafts()[SpacecraftIndex];

		if (!ShipCandidate->GetParent()->GetDamageSystem()->IsAlive())
		{
			continue;
		}

		if (ShipCandidate == PilotTargetStation)
		{
			continue;
		}

		if (DangerousOnly && ! PilotHelper::IsTargetDangerous(PilotHelper::PilotTarget(ShipCandidate)))
		{
			continue;
		}

		// Tutorial exception
		if (Ship->GetGame()->GetQuestManager()
		 && ShipCandidate->GetParent() == Ship->GetGame()->GetPC()->GetPlayerShip()
		 && ShipCandidate->GetCurrentTarget() == Ship
		 && Ship->GetGame()->GetQuestManager()->FindQuest("tutorial-fighter")->GetStatus() == EFlareQuestStatus::ONGOING
		 && Ship->GetGame()->GetQuestManager()->FindQuest("tutorial-fighter")->GetCurrentStep()->GetIdentifier() == "hit-cargo")
		{
			// don't skip
		}
		else if (!ShipCandidate->IsHostile(Ship->GetCompany()))
		{
			continue;
		}

		float DistanceSquared = (PilotLocation - ShipCandidate->GetActorLocation()).SizeSquared();
		if (ShipCandidate->GetSize() == EFlarePartSize::S)
		{
			if (NearestHostileShipSmall == NULL || DistanceSquared < MinDistanceSquaredSmall)
			{
				MinDistanceSquaredSmall = DistanceSquared;
				NearestHostileShipSmall = ShipCandidate;
			}
		}
		else
		{
			if (MinDistanceSquaredLarge == NULL || DistanceSquared < MinDistanceSquaredLarge)
			{
				MinDistanceSquaredLarge = DistanceSquared;
				NearestHostileShipLarge = ShipCandidate;
			}
		}
	}
}

AFlareSpacecraft* UFlareShipPilot::GetNearestShip(bool IgnoreDockingShip) const
{
	// For now an host ship is a the nearest host ship with the following critera:
	// - Alive or not
	// - From any company
	// - Is the nearest
	// - Is not me

	FVector PilotLocation = Ship->GetActorLocation();
	float MinDistanceSquared = -1;
	AFlareSpacecraft* NearestShip = NULL;

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Ship->GetGame()->GetActiveSector()->GetSpacecrafts().Num(); SpacecraftIndex++)
	{
		AFlareSpacecraft* ShipCandidate = Ship->GetGame()->GetActiveSector()->GetSpacecrafts()[SpacecraftIndex];

		if (ShipCandidate == Ship)
		{
			continue;
		}

		if (IgnoreDockingShip && Ship->GetDockingSystem()->IsGrantedShip(ShipCandidate) && !ShipCandidate->GetParent()->GetDamageSystem()->IsUncontrollable())
		{
			// Constrollable ship are not dangerous for collision
			continue;
		}

		if (IgnoreDockingShip && Ship->GetDockingSystem()->IsDockedShip(ShipCandidate))
		{
			// Docked shipship are not dangerous for collision, even if they are dead or offlline
			continue;
		}

		float DistanceSquared = (PilotLocation - ShipCandidate->GetActorLocation()).SizeSquared();
		if (NearestShip == NULL || DistanceSquared < MinDistanceSquared)
		{
			MinDistanceSquared = DistanceSquared;
			NearestShip = ShipCandidate;
		}
	}
	return NearestShip;
}

FVector UFlareShipPilot::GetAngularVelocityToAlignAxis(FVector LocalShipAxis, FVector TargetAxis, FVector TargetAngularVelocity, float DeltaSeconds) const
{
	TArray<UActorComponent*> Engines = Ship->GetComponentsByClass(UFlareEngine::StaticClass());

	FVector AngularVelocity = Ship->Airframe->GetPhysicsAngularVelocityInDegrees();
	FVector WorldShipAxis = Ship->Airframe->GetComponentToWorld().GetRotation().RotateVector(LocalShipAxis);

	WorldShipAxis.Normalize();
	TargetAxis.Normalize();

	FVector RotationDirection = FVector::CrossProduct(WorldShipAxis, TargetAxis);
	RotationDirection.Normalize();
	float Dot = FVector::DotProduct(WorldShipAxis, TargetAxis);
	float angle = FMath::RadiansToDegrees(FMath::Acos(Dot));

	FVector DeltaVelocity = TargetAngularVelocity - AngularVelocity;
	FVector DeltaVelocityAxis = DeltaVelocity;
	DeltaVelocityAxis.Normalize();

	float TimeToFinalVelocity;

	if (FMath::IsNearlyZero(DeltaVelocity.SizeSquared()))
	{
		TimeToFinalVelocity = 0;
	}
	else {
		FVector SimpleAcceleration = DeltaVelocityAxis * Ship->GetNavigationSystem()->GetAngularAccelerationRate();
	    // Scale with damages
		float DamageRatio = Ship->GetNavigationSystem()->GetTotalMaxTorqueInAxis(Engines, DeltaVelocityAxis, true) / Ship->GetNavigationSystem()->GetTotalMaxTorqueInAxis(Engines, DeltaVelocityAxis, false);
	    FVector DamagedSimpleAcceleration = SimpleAcceleration * DamageRatio;

	    FVector Acceleration = DamagedSimpleAcceleration;
	    float AccelerationInAngleAxis =  FMath::Abs(FVector::DotProduct(DamagedSimpleAcceleration, RotationDirection));

	    TimeToFinalVelocity = (DeltaVelocity.Size() / AccelerationInAngleAxis);
	}

	float AngleToStop = (DeltaVelocity.Size() / 2) * (FMath::Max(TimeToFinalVelocity,DeltaSeconds));

	FVector RelativeResultSpeed;

	if (AngleToStop > angle) {
		RelativeResultSpeed = TargetAngularVelocity;
	}
	else
	{
		float MaxPreciseSpeed = FMath::Min((angle - AngleToStop) / (DeltaSeconds * 0.75f), Ship->GetNavigationSystem()->GetAngularMaxVelocity());

		RelativeResultSpeed = RotationDirection;
		RelativeResultSpeed *= MaxPreciseSpeed;
	}

	return RelativeResultSpeed;
}
 
AFlareSpacecraft* UFlareShipPilot::GetNearestAvailableStation(bool RealStation) const
{
	FVector PilotLocation = Ship->GetActorLocation();
	float MinDistanceSquared = -1;
	AFlareSpacecraft* NearestStation = NULL;

	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Ship->GetGame()->GetActiveSector()->GetStations().Num(); SpacecraftIndex++)
	{
		AFlareSpacecraft* StationCandidate = Ship->GetGame()->GetActiveSector()->GetStations()[SpacecraftIndex];

		if (StationCandidate == Ship)
		{
			continue;
		}

		if (!StationCandidate->GetDockingSystem()->HasAvailableDock(Ship))
		{
			continue;
		}

		if (RealStation && !StationCandidate->IsStation())
		{
			continue;
		}

		if (StationCandidate->GetCompany() != Ship->GetCompany())
		{
			continue;
		}

		float DistanceSquared = (PilotLocation - StationCandidate->GetActorLocation()).SizeSquared();
		if (NearestStation == NULL || DistanceSquared < MinDistanceSquared)
		{
			MinDistanceSquared = DistanceSquared;
			NearestStation = StationCandidate;
		}
	}
	return NearestStation;
}

TArray<AFlareSpacecraft*> UFlareShipPilot::GetFriendlyStations() const
{
	TArray<AFlareSpacecraft*> FriendlyStations;
	for (int32 SpacecraftIndex = 0; SpacecraftIndex < Ship->GetGame()->GetActiveSector()->GetStations().Num(); SpacecraftIndex++)
	{
		AFlareSpacecraft* StationCandidate = Ship->GetGame()->GetActiveSector()->GetStations()[SpacecraftIndex];
		if (StationCandidate->GetDockingSystem()->GetDockCount() > 0)
		{
			if (StationCandidate->IsHostile(Ship->GetCompany()) || !StationCandidate->GetDockingSystem()->HasAvailableDock(Ship))
			{
				continue;
			}
			FriendlyStations.Add(StationCandidate);
		}
	}
	return FriendlyStations;
}

void UFlareShipPilot::AlignToTargetVelocityWithThrust(float DeltaSeconds)
{
	if(!LinearTargetVelocity.IsNearlyZero()) {
		FVector LinearTargetVelocityAxis = LinearTargetVelocity.GetUnsafeNormal();
		FVector ThrustVector = (LinearTargetVelocity - 100 * Ship->GetLinearVelocity());
		FVector ThrustAxis;
		if(ThrustVector.IsNearlyZero())
		{
			 ThrustAxis = LinearTargetVelocityAxis;
		}
		else
		{
			ThrustAxis = ThrustVector.GetUnsafeNormal();
		}

		FVector OrientationVector = (ThrustAxis * 0.3  + LinearTargetVelocityAxis * 0.7).GetUnsafeNormal();

		AngularTargetVelocity = GetAngularVelocityToAlignAxis(FVector(1,0,0), OrientationVector, FVector::ZeroVector, DeltaSeconds);
	}
}


/*----------------------------------------------------
	Pilot Output
----------------------------------------------------*/

FVector UFlareShipPilot::GetLinearTargetVelocity() const
{
	return LinearTargetVelocity;
}

FVector UFlareShipPilot::GetAngularTargetVelocity() const
{
	return AngularTargetVelocity;
}

bool UFlareShipPilot::IsUseOrbitalBoost() const
{
	return UseOrbitalBoost;
}

bool UFlareShipPilot::IsWantFire() const
{
	return WantFire;
}
