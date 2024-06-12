
#include "FlareTurretPilot.h"
#include "../Flare.h"

#include "FlarePilotHelper.h"
#include "FlareTurret.h"
#include "FlareRCS.h"
#include "FlareSpacecraft.h"
#include "FlareSpacecraftComponent.h"

#include "../Player/FlarePlayerController.h"
#include "../Game/FlareGame.h"
#include "../Game/AI/FlareCompanyAI.h"

DECLARE_CYCLE_STAT(TEXT("FlareTurretPilot Tick"), STAT_FlareTurretPilot_Tick, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareTurretPilot Tick Target"), STAT_FlareTurretPilot_Target, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareTurretPilot Tick Intersect"), STAT_FlareTurretPilot_Intersect, STATGROUP_Flare);
DECLARE_CYCLE_STAT(TEXT("FlareTurretPilot Tick Intersect Gun"), STAT_FlareTurretPilot_Intersect_Gun, STATGROUP_Flare);

DECLARE_CYCLE_STAT(TEXT("FlareTurretPilot GetNearestHostileShip"), STAT_FlareTurretPilot_GetNearestHostileShip, STATGROUP_Flare);

#define TURRET_AI_TICKRATE 0.10f

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareTurretPilot::UFlareTurretPilot(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	TargetSelectionReactionTimeFast = FMath::FRandRange(1.0, 1.5);
	TargetSelectionReactionTimeSlow = FMath::FRandRange(2.0, 3.0);
	TimeUntilNextTargetSelectionReaction = 0;
	TimeUntilFireReaction = 0;
}


/*----------------------------------------------------
	Gameplay events
----------------------------------------------------*/

void UFlareTurretPilot::Initialize(const FFlareTurretPilotSave* Data, UFlareCompany* Company, UFlareTurret* OwnerTurret, AFlareSpacecraft* OwnerShip)
{
	// Main data
	Turret = OwnerTurret;
	PlayerCompany = Company;
	WantFire = false;

	// Setup properties
	if (Data)
	{
		TurretPilotData = *Data;
	}

	SecurityRadius = 0;
	if (Turret->GetDescription()->WeaponCharacteristics.FuzeType == EFlareShellFuzeType::Proximity)
	{
		SecurityRadius = Turret->GetDescription()->WeaponCharacteristics.AmmoExplosionRadius + Turret->GetSpacecraft()->GetMeshScale() / 100;
		SecurityRadiusDistance = (SecurityRadius * 100);
	}
	
	if (Turret->GetDescription()->WeaponCharacteristics.GunCharacteristics.AmmoRate)
	{
		FireReactionTime = FMath::FRandRange(((Turret->GetDescription()->WeaponCharacteristics.GunCharacteristics.AmmoRate / 60.f) / 2), ((Turret->GetDescription()->WeaponCharacteristics.GunCharacteristics.AmmoRate / 60.f)));
	}
	else
	{
		FireReactionTime = FMath::FRandRange(0.1, 0.2);
	}
}

void UFlareTurretPilot::PlayerSetAim(FVector AimDirection, float AimDistance)
{
	ManualAimDirection = AimDirection;
	ManualAimDistance = AimDistance;
}

void UFlareTurretPilot::PlayerStartFire()
{
	if (Turret->IsCloseToAim())
	{
		WantFire = true;
	}
}

void UFlareTurretPilot::PlayerStopFire()
{
	WantFire = false;
}

void UFlareTurretPilot::ClearInvalidTarget(PilotHelper::PilotTarget InvalidTarget)
{
	if(PilotTarget == InvalidTarget)
	{
		ClearTarget();
	}
}

void UFlareTurretPilot::ClearTarget()
{
	PilotTarget.Clear();
	TimeUntilNextTargetSelectionReaction = 0;
}

void UFlareTurretPilot::TickPilot(float DeltaSeconds)
{
	if (!Turret->GetSpacecraft()->GetGame()->GetActiveSector())
	{
		return;
	}

	SCOPE_CYCLE_COUNTER(STAT_FlareTurretPilot_Tick);
	PreviousTick += DeltaSeconds;
	TimeUntilNextTargetSelectionReaction -= DeltaSeconds;
	TimeUntilFireReaction -= DeltaSeconds;
	TimeUntilNextComponentSwitch -= DeltaSeconds;

	if (PreviousTick < TURRET_AI_TICKRATE)
	{
		return;
	}

	PreviousTick -= TURRET_AI_TICKRATE;

	// Is the player manually controlling this ? If not, pick a target
	bool IsManuallyFiring = Turret->GetSpacecraft()->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_TURRET;
	if (!IsManuallyFiring)
	{
		ProcessTurretTargetSelection();
		AimAxis = FVector::ZeroVector;
	}

	// Fire director
	if (IsManuallyFiring && !Turret->IsIgnoreManualAim())
	{
		AimAxis = ManualAimDirection;

		// Try getting a target
		AActor* HitTarget = NULL;
		Turret->IsSafeToFire(0, HitTarget);
		if (!HitTarget || HitTarget == Turret->GetSpacecraft())
		{
			HitTarget = Turret->GetSpacecraft()->GetCurrentTarget().GetActor();
		}

		// Aim the turret toward the target or a distant point
		if (IsValid(HitTarget) && Cast<UPrimitiveComponent>(HitTarget->GetRootComponent()))
		{
			FVector Location = HitTarget->GetActorLocation();
			FVector Velocity = Cast<UPrimitiveComponent>(HitTarget->GetRootComponent())->GetPhysicsLinearVelocity() / 100;
			Turret->SetTarget(Location, Velocity);
		}
		else
		{
			FVector FireTargetLocation = Turret->GetMuzzleLocation(0) + ManualAimDirection * 100000;
			Turret->SetTarget(FireTargetLocation, FVector::ZeroVector);
		}
	}

	// Auto pilot
	else if (PilotTarget.IsValid())
	{
		if(PilotTarget.SpacecraftTarget)
		{
			if (TimeUntilNextComponentSwitch <= 0)
			{
				//FLOGV("%s Switch because of timeout", *Turret->GetReadableName());
				PilotTargetShipComponent = NULL;
			}
			else if (PilotTargetShipComponent != nullptr && IsValid(PilotTargetShipComponent) && !PilotTargetShipComponent->GetIsSafeDestroyingRunning())
			{
				if (!PilotTarget.Is(PilotTargetShipComponent->GetSpacecraft()))
				{
					//FLOGV("%s Switch because the component %s is not in the target ship", *Turret->GetReadableName(), *PilotTargetShipComponent->GetReadableName());
					PilotTargetShipComponent = NULL;
				}
				else if (PilotTargetShipComponent->GetUsableRatio() <= 0)
				{
					//FLOGV("%s Switch because the component %s is destroyed", *Turret->GetReadableName(), *PilotTargetShipComponent->GetReadableName());
					PilotTargetShipComponent = NULL;
				}
			}

			if (!PilotTargetShipComponent)
			{
				PilotTargetShipComponent = PilotHelper::GetBestTargetComponent(PilotTarget.SpacecraftTarget);
				TimeUntilNextComponentSwitch = 10;
				//FLOGV("%s Select new target component %s ", *Turret->GetReadableName(), *PilotTargetShipComponent->GetReadableName());
			}
		}
		else
		{
			PilotTargetShipComponent = NULL;
		}
		
		float PredictionDelay = 0;
		float AmmoVelocity = Turret->GetAmmoVelocity() * 100;

		FVector TurretVelocity = 100 * Turret->GetSpacecraft()->GetLinearVelocity();
		FVector AmmoIntersectionPredictedLocation;
		FVector TurretLocation = Turret->GetTurretBaseLocation();

		FVector PilotTargetLocation = (PilotTargetShipComponent ? PilotTargetShipComponent->GetComponentLocation() : PilotTarget.GetActorLocation());

		float AmmoIntersectionPredictedTime = SpacecraftHelper::GetIntersectionPosition(PilotTargetLocation, PilotTarget.GetLinearVelocity(), TurretLocation, TurretVelocity, AmmoVelocity, PredictionDelay, &AmmoIntersectionPredictedLocation);
		FVector PredictedFireTargetLocation;
		if (AmmoIntersectionPredictedTime > 0)
		{
			PredictedFireTargetLocation = AmmoIntersectionPredictedLocation;
		}
		else
		{
			PredictedFireTargetLocation = PilotTargetLocation;
		}
		
		AimAxis = (PredictedFireTargetLocation - TurretLocation).GetUnsafeNormal();
		if (TimeUntilFireReaction < 0)
		{
			TimeUntilFireReaction = FireReactionTime;
			WantFire = false;
			/*FLOGV("%s Have target AimAxis=%s",*Turret->GetReadableName(),  * AimAxis.ToString());
			FLOGV("%s AmmoIntersectionPredictedTime=%f",*Turret->GetReadableName(),  AmmoIntersectionPredictedTime);
			FLOGV("%s AmmoVelocity=%f",*Turret->GetReadableName(),  AmmoVelocity);*/

			float TargetSize = PilotTarget.GetMeshScale() / 100.f + Turret->GetAimRadius() * 2; // Radius in meters
			FVector DeltaLocation = (PilotTargetLocation - TurretLocation) / 100.f;
			float Distance = DeltaLocation.Size(); // Distance in meters
			//Distance
			//FLOGV("%s Distance=%f",*Turret->GetReadableName(),  Distance);

			// If at range and aligned fire on the target
			// TODO increase tolerance if target is near
			if (Distance <= Turret->GetDescription()->WeaponCharacteristics.GunCharacteristics.AmmoRange && AmmoIntersectionPredictedTime > 0 && AmmoIntersectionPredictedTime < 10.f)
			{
				SCOPE_CYCLE_COUNTER(STAT_FlareTurretPilot_Intersect);


				//FLOG("Near enough");
				FVector FireAxis = Turret->GetFireAxis();

				for (int GunIndex = 0; GunIndex < Turret->GetGunCount(); GunIndex++)
				{
					SCOPE_CYCLE_COUNTER(STAT_FlareTurretPilot_Intersect_Gun);

					FVector MuzzleLocation = Turret->GetMuzzleLocation(GunIndex);

					// Compute target Axis for each gun
					FVector AmmoIntersectionLocation;
					float AmmoIntersectionTime = SpacecraftHelper::GetIntersectionPosition(PilotTargetLocation, PilotTarget.GetLinearVelocity(), MuzzleLocation, TurretVelocity, AmmoVelocity, 0, &AmmoIntersectionLocation);
					if (AmmoIntersectionTime < 0)
					{
						// No ammo intersection, don't fire
						continue;
					}
					FVector FireTargetAxis = (AmmoIntersectionLocation - MuzzleLocation).GetUnsafeNormal();
					/*FLOGV("Gun %d FireAxis=%s", GunIndex, *FireAxis.ToString());
					FLOGV("Gun %d FireTargetAxis=%s", GunIndex, *FireTargetAxis.ToString());*/

					float AngularPrecisionDot = FVector::DotProduct(FireTargetAxis, FireAxis);
					float AngularPrecision = FMath::Acos(AngularPrecisionDot);
					float AngularSize = FMath::Atan(TargetSize / Distance);

					/*FLOGV("Gun %d Distance=%f", GunIndex, Distance);
					FLOGV("Gun %d TargetSize=%f", GunIndex, TargetSize);
					FLOGV("Gun %d AngularSize=%f", GunIndex, AngularSize);
					FLOGV("Gun %d AngularPrecision=%f", GunIndex, AngularPrecision);*/
					bool DangerousTarget = IsTargetDangerous(PilotTarget);
					if (AngularPrecision < (DangerousTarget ? AngularSize * 0.25 : AngularSize * 0.2))
					{
						//CheckFriendlyFire(UFlareSector* Sector, UFlareCompany* MyCompany, FVector FireBaseLocation, FVector FireBaseVelocity , float AmmoVelocity, FVector FireAxis, float MaxDelay, float AimRadius)
						if (!PilotHelper::CheckFriendlyFire(Turret->GetSpacecraft()->GetGame()->GetActiveSector(), PlayerCompany, MuzzleLocation, TurretVelocity, AmmoVelocity, FireTargetAxis, AmmoIntersectionTime, Turret->GetAimRadius()))
						{
							FVector Location = PilotTarget.GetActorLocation();
							FVector Velocity = PilotTarget.GetLinearVelocity() / 100;
							Turret->SetTarget(Location, Velocity);
							WantFire = true;
							break;
						}
						else
						{
							//FLOG("Friendly fire avoidance");
						}
					}
				}
			}
		}
	}
	else
	{
		WantFire = false;
	}
}


/*----------------------------------------------------
	Internal
----------------------------------------------------*/

void UFlareTurretPilot::ProcessTurretTargetSelection()
{
	if (TimeUntilNextTargetSelectionReaction > 0)
	{
		if (PilotTarget.IsValid())
		{
			if (PilotTarget.SpacecraftTarget && !PilotTarget.SpacecraftTarget->GetParent()->GetDamageSystem()->IsAlive())
			{
				ClearTarget();
				TimeUntilNextTargetSelectionReaction = TargetSelectionReactionTimeSlow;
			}
			else
			{
				return;
			}
		}
		else
		{
			ClearTarget();
			TimeUntilNextTargetSelectionReaction = TargetSelectionReactionTimeSlow;
		}
	}
	else
	{
		if (PilotTarget.SpacecraftTarget && PilotTarget.SpacecraftTarget != nullptr)
		{
			if (PilotTarget.SpacecraftTarget->GetParent()->GetDescription()->Size == EFlarePartSize::S)
			{
				TimeUntilNextTargetSelectionReaction = TargetSelectionReactionTimeFast;
			}
		}
		if (TimeUntilNextTargetSelectionReaction <= 0)
		{
			TimeUntilNextTargetSelectionReaction = TargetSelectionReactionTimeSlow;
		}
	}

	SCOPE_CYCLE_COUNTER(STAT_FlareTurretPilot_Target);

	bool CheckIndividualTarget = false;
	bool DoTargetChecks = true;
	FVector PilotLocation = Turret->GetTurretBaseLocation();
/*
	AFlareSpacecraft* WeaponGroupCandidate = Turret->GetWeaponGroup()->Target;
	if (WeaponGroupCandidate)
	{
		if (WeaponGroupCandidate->GetParent()->GetDamageSystem()->IsAlive())
		{
			Turret->GetWeaponGroup()->Target = NULL;
		}
		else
		{
			FVector TargetAxis = (WeaponGroupCandidate->GetActorLocation() - PilotLocation).GetUnsafeNormal();
			if (Turret->IsReacheableAxis(TargetAxis))
			{
				float Distance = (PilotLocation - PilotTarget.GetActorLocation()).Size();
				if (Distance < SecurityRadiusDistance || Distance > Turret->GetDescription()->WeaponCharacteristics.GunCharacteristics.AmmoRange)
				{
					PilotTarget = WeaponGroupCandidate;
					DoTargetChecks = false;
				}
				else
				{
					Turret->GetWeaponGroup()->Target = NULL;
				}
			}
			else
			{
				Turret->GetWeaponGroup()->Target = NULL;
			}
		}
	}
*/
	if (DoTargetChecks)
	{
		UFlareFleet* CurrentFleet = Turret->GetSpacecraft()->GetParent()->GetCurrentFleet();
		if ((CurrentFleet && CurrentFleet->GetIncomingBombs().Num() > 0) || (!CurrentFleet && Turret->GetSpacecraft()->GetIncomingBombs().Num() > 0) || Turret->GetSpacecraft()->GetGame()->GetActiveSector()->GetMeteorites().Num() > 0)
		{
			CheckIndividualTarget = true;
		}
		else
		{
			PilotTarget = Turret->GetSpacecraft()->GetCurrentTarget();
			if (PilotTarget.IsValid())
			{
				if (PilotTarget.SpacecraftTarget)
				{
					if (PilotTarget.SpacecraftTarget->GetParent()->GetDamageSystem()->IsAlive())
					{
						//the station + military check is just encase someone throws weapons on a station via third party mods
						if ((PilotTarget.SpacecraftTarget->IsStation() && !PilotTarget.SpacecraftTarget->IsMilitary()) || !Turret->GetSpacecraft()->IsHostile(PilotTarget.SpacecraftTarget->GetCompany()) || PilotTarget.SpacecraftTarget->GetParent()->GetDamageSystem()->IsUncontrollable() || (PilotTarget.SpacecraftTarget->IsMilitary() && PilotTarget.SpacecraftTarget->GetParent()->GetDamageSystem()->IsDisarmed()))
						{
							CheckIndividualTarget = true;
						}
					}
					else
					{
						CheckIndividualTarget = true;
					}
				}
				else
				{
					CheckIndividualTarget = true;
				}

				if (!CheckIndividualTarget)
				{
					FVector TargetAxis = (PilotTarget.GetActorLocation() - PilotLocation).GetUnsafeNormal();
					if (!Turret->IsReacheableAxis(TargetAxis))
					{
						CheckIndividualTarget = true;
					}
					else
					{
						float Distance = (PilotLocation - PilotTarget.GetActorLocation()).Size();
						if (Distance < SecurityRadiusDistance || Distance > Turret->GetDescription()->WeaponCharacteristics.GunCharacteristics.AmmoRange)
						{
							CheckIndividualTarget = true;
						}
					}
				}
			}
			else
			{
				CheckIndividualTarget = true;
			}
		}
	}

	if (CheckIndividualTarget)
	{
		EFlareCombatTactic::Type Tactic = Turret->GetSpacecraft()->GetParent()->GetCompany()->GetTacticManager()->GetCurrentTacticForShipGroup(EFlareCombatGroup::Capitals);
		PilotTarget = GetNearestHostileTarget(true, Tactic);
/*		if (PilotTarget.SpacecraftTarget)
		{
			Turret->GetWeaponGroup()->Target = PilotTarget.SpacecraftTarget;
		}
*/
	}
}

PilotHelper::PilotTarget UFlareTurretPilot::GetNearestHostileTarget(bool ReachableOnly, EFlareCombatTactic::Type Tactic)
{
	SCOPE_CYCLE_COUNTER(STAT_FlareTurretPilot_GetNearestHostileShip);

	// For now an host ship is a the nearest host ship with the following critera:
	// - Alive
	// - Is dangerous if needed
	// - From another company
	// - Is the nearest
	FVector PilotLocation = Turret->GetTurretBaseLocation();
	float MaxDot = 0;
	PilotHelper::PilotTarget NearestHostileTarget;
	FVector FireAxis = Turret->GetFireAxis();

	struct PilotHelper::TargetPreferences TargetPreferences;
	TargetPreferences.IsLarge = 1;
	TargetPreferences.IsSmall = 1;
	TargetPreferences.IsStation = 0;
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
	TargetPreferences.TargetStateWeight = 1;
	TargetPreferences.MaxDistance = 5000000;
	TargetPreferences.DistanceWeight = 0.1;
	TargetPreferences.AttackTarget = NULL;
	TargetPreferences.AttackTargetWeight = 15;
	TargetPreferences.AttackMeWeight = 10;
	TargetPreferences.LastTarget = PilotTarget;
	TargetPreferences.LastTargetWeight = 5.;
	TargetPreferences.IsBomb = 5.f;
	TargetPreferences.MaxBombDistance = 400000;
	TargetPreferences.IsMeteorite = 0.0001f;

	TargetPreferences.PreferredDirection = FireAxis;
	TargetPreferences.MinAlignement = -1;
	TargetPreferences.AlignementWeight = 1.0;
	TargetPreferences.BaseLocation = PilotLocation;

	TargetPreferences.IsLarge = Turret->GetDescription()->WeaponCharacteristics.AntiLargeShipValue;
	TargetPreferences.IsSmall = Turret->GetDescription()->WeaponCharacteristics.AntiSmallShipValue;
	TargetPreferences.IsStation = Turret->GetDescription()->WeaponCharacteristics.AntiStationValue;


	float AmmoRatio = float(Turret->GetCurrentAmmo()) / Turret->GetMaxAmmo();

	if(AmmoRatio <0.9)
	{
		TargetPreferences.IsUncontrollableSmallMilitary = 0.0;
	}

	if(AmmoRatio < 0.5)
	{
		TargetPreferences.IsNotMilitary = 0.0;
	}

	if (Tactic == EFlareCombatTactic::AttackStations)
	{
		TargetPreferences.IsStation *= 10;
	}
	else if (Tactic == EFlareCombatTactic::AttackMilitary)
	{
		TargetPreferences.IsStation = 0;
	}
	else if (Tactic == EFlareCombatTactic::AttackCivilians)
	{
		TargetPreferences.IsStation = 0;
		TargetPreferences.IsMilitary = 0.1;
		TargetPreferences.IsNotMilitary = 1.0;
		TargetPreferences.IsNotDangerous = 1.0;
	}
	else if (Tactic == EFlareCombatTactic::ProtectMe)
	{
		TargetPreferences.IsStation = 0;
		// Protect me is only available for player ship
		if (Turret->GetSpacecraft()->GetParent()->GetCompany() == Turret->GetSpacecraft()->GetGame()->GetPC()->GetCompany())
		{
			TargetPreferences.AttackTarget = Turret->GetSpacecraft()->GetGame()->GetPC()->GetShipPawn();
			TargetPreferences.AttackTargetWeight = 1.0;
		}
	}

	while(NearestHostileTarget.IsEmpty())
	{
		NearestHostileTarget = PilotHelper::GetBestTarget(Turret->GetSpacecraft(), TargetPreferences);

		if(NearestHostileTarget.IsEmpty())
		{ // No target
			break;
		}

		float Distance = (PilotLocation - NearestHostileTarget.GetActorLocation()).Size();
		if (Distance < SecurityRadiusDistance || Distance > Turret->GetDescription()->WeaponCharacteristics.GunCharacteristics.AmmoRange)
		{
			TargetPreferences.IgnoreList.Add(NearestHostileTarget);
			continue;
		}

		if (ReachableOnly)
		{
			FVector TargetAxis = (NearestHostileTarget.GetActorLocation() - PilotLocation).GetUnsafeNormal();
			if (!Turret->IsReacheableAxis(TargetAxis))
			{
				TargetPreferences.IgnoreList.Add(NearestHostileTarget);
				continue;
			}
		}
	}
	return NearestHostileTarget;
}

bool UFlareTurretPilot::IsTargetDangerous(PilotHelper::PilotTarget const& Target) const
{
	if(Target.SpacecraftTarget)
	{
		return Target.SpacecraftTarget->GetParent()->IsMilitaryArmed() && !Target.SpacecraftTarget->GetParent()->GetDamageSystem()->IsDisarmed();
	}
	else if(Target.BombTarget)
	{
		return true;
	}

	return false;
}

void UFlareTurretPilot::SetWantFire(bool NewValue)
{
	WantFire = NewValue;
}

/*----------------------------------------------------
	Pilot Output
----------------------------------------------------*/

FVector UFlareTurretPilot::GetTargetAimAxis() const
{
	return AimAxis;
}

bool UFlareTurretPilot::IsWantFire() const
{
	return WantFire && !Turret->GetSpacecraft()->GetParent()->GetDamageSystem()->IsDisarmed();
}