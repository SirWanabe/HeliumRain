#pragma once

#include "FlareSpacecraftTypes.h"
#include "../Game/FlareGameTypes.h"
#include "FlarePilotHelper.h"
#include "FlareShipPilot.generated.h"

class UFlareCompany;
class AFlareSpacecraft;
class UFlareSpacecraftComponent;


/** Ship pilot class */
UCLASS()
class HELIUMRAIN_API UFlareShipPilot : public UObject
{

public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void TickPilot(float DeltaSeconds);

	/** Initialize this pilot and register the master ship object */
	virtual void Initialize(const FFlareShipPilotSave* Data, UFlareCompany* Company, AFlareSpacecraft* OwnerShip);

	void ClearInvalidTarget(PilotHelper::PilotTarget InvalidTarget);

	bool GetInitiatedCombat();

protected:
	virtual void ClearTarget();
	/*----------------------------------------------------
		Pilot functions
	----------------------------------------------------*/
	virtual void CarrierReleaseInternalDockShips(float DeltaSeconds);

	virtual bool DroneReturnToCarrier(float DeltaSeconds);

	virtual void ArmedStationPilot(float DeltaSeconds);

	virtual void MilitaryPilot(float DeltaSeconds);

	virtual void CargoPilot(float DeltaSeconds);

	virtual bool DockAtStation(float DeltaSeconds);
	void GetRandomFriendlyStation(bool CanDock);

	virtual void FighterPilot(float DeltaSeconds);

	virtual void BomberPilot(float DeltaSeconds);

	virtual void MissilePilot(float DeltaSeconds);

	virtual void FlagShipPilot(float DeltaSeconds);

	virtual void FlagShipMovement(float DeltaSeconds);

	virtual void IdlePilot(float DeltaSeconds);

	virtual void FindPilotAvoidShip(float DeltaSeconds);

	struct AnticollisionConfig
	{
		AnticollisionConfig() : SpacecraftToIgnore(nullptr), IgnoreAllStations(false), SpeedCorrectionOnly(false) {}
		AFlareSpacecraft* SpacecraftToIgnore;
		bool IgnoreAllStations;
		bool SpeedCorrectionOnly;
	};

	virtual FVector TryAnticollisionCorrection(AFlareSpacecraft* TargetShip, FVector InitialVelocity, float PreventionDuration, AnticollisionConfig IgnoreConfig, float SpeedLimit, float DeltaSeconds);

	virtual void SelectedNewLeader(AFlareSpacecraft* OldLeaderShip);

	virtual void GetNewLeaderShip();

	virtual void FollowLeaderShip(int32 DefaultRadius = 50000);

public:
	/*----------------------------------------------------
		Helpers
	----------------------------------------------------*/

	/**
	 * Return the curved trajectory to avoid exiting sector
	 */
	FVector ExitAvoidance(AFlareSpacecraft* Ship, FVector InitialVelocityTarget,float CurveTrajectoryLimit,float DeltaSeconds);

	/**
	 * Set the nearest hostile small/large ships
	 */
	void GetNearestHostileShip(bool DangerousOnly);
	/**
	 * Return the nearest ship, alive or not
	 */
	virtual AFlareSpacecraft* GetNearestShip(bool IgnoreDockingShip) const;

	/**
	* Return the nearest station where docking is available
	*/
	virtual AFlareSpacecraft* GetNearestAvailableStation(bool RealStation) const;

	/** Return all friendly station in the sector */
	virtual TArray<AFlareSpacecraft*> GetFriendlyStations(bool CanDock) const;

	/**
	 * Return the angular velocity need to align the local ship axis to the target axis
	 */
	virtual FVector GetAngularVelocityToAlignAxis(FVector LocalShipAxis, FVector TargetAxis, FVector TargetAngularVelocity, float DeltaSeconds) const;

	virtual void SetupNewTarget();
	virtual void FindBestHostileTarget(EFlareCombatTactic::Type Tactic);
	virtual void SelectNewHostileTarget(PilotHelper::PilotTarget TargetCandidate);

	void AlignToTargetVelocityWithThrust(float DeltaSeconds);

public:

	/*----------------------------------------------------
		Pilot Output
	----------------------------------------------------*/

	/** Linear target velocity */
	virtual FVector GetLinearTargetVelocity() const;

	/** Angular target velocity */
	virtual FVector GetAngularTargetVelocity() const;

	/** Is pilot want to use orbital boost */
	virtual bool IsUseOrbitalBoost() const;

	/** Is pilot want to fire */
	virtual bool IsWantFire() const;

	/** Pilot weapon selection */
	virtual int32 GetPreferedWeaponGroup() const;

protected:

	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	AFlareSpacecraft*                            Ship;

	UPROPERTY()
	UFlareCompany*                               PlayerCompany;

	// Component description
	FFlareShipPilotSave                          ShipPilotData;

	// Output commands
	bool                                         UseOrbitalBoost;
	bool                                         WantFire;
	FVector                                      LinearTargetVelocity;
	FVector                                      AngularTargetVelocity;


	// Pilot brain TODO save in save
	float                                        ReactionTime;
	float                                        TimeUntilNextReaction;
	FVector                                      PilotTargetLocation;
	float								         DockWaitTime;
	float								         CurrentWaitTime;

	FVector                                      PreviousAntiCollisionVector;
	float										 LastNewCollisionVector;
	float										 CollisionVectorReactionTimeFast;
	float										 CollisionVectorReactionTimeSlow;

	UPROPERTY()
	AFlareSpacecraft*							 LeaderShip;

	// Pilot targets
	PilotHelper::PilotTarget                     PilotTarget;
	PilotHelper::PilotTarget                     LastPilotTarget;
	UPROPERTY()
	AFlareSpacecraft*                            PilotTargetStation;
	UPROPERTY()
	AFlareSpacecraft*                            PilotLastTargetStation;
	UPROPERTY()
	UFlareSpacecraftComponent*			         PilotTargetShipComponent;

	AFlareSpacecraft*							 NearestHostileShipSmall;
	AFlareSpacecraft*							 NearestHostileShipLarge;

	float                                        AttackAngle;
	float                                        AttackDistance;
	float                                        MaxFollowDistance;
	int32                                        AttackPhase;
	float                                        LastTargetDistance;
	int32                                        SelectedWeaponGroupIndex;
	bool                                         LockTarget;
	bool                                         LastWantFire;
	bool										 HasUndockedAllInternalShips;

	float										 TimeBeforeNextInternalUndock;
	float                                        TimeBeforeNextDrop;
	float                                        TimeSinceAiming;
	float                                        TimeUntilNextComponentSwitch;
	float										 ComponentSwitchReactionTime;

	float                                        DockingAttemptTries;
	float                                        TimeSinceLastDockingAttempt;
	float                                        TimeUntilNextDockingAttempt;
	float                                        MaxTimeBetweenDockingAttempt;

	UPROPERTY()
	AFlareSpacecraft*							 PilotAvoidShip;
	float										 TimeUntilNextPilotAvoidCheck;
	float								 		 PilotAvoidCheckReactionTimeFast;
	float										 PilotAvoidCheckReactionTimeSlow;
	float										 TimeUntilNextHostileTargetSwitch;
	float										 HostileTargetSwitchReactionTimeFast;
	float										 HostileTargetSwitchReactionTimeSlow;
	float										 TimeUntilNextExitAvoidanceCheck;
	float										 NextExitAvoidanceCheckReactionFast;
	float										 NextExitAvoidanceCheckReactionSlow;

	bool										 RunningExitAvoidance;
	bool										 InitiatedCombat;
	bool										 FoundOutOfCombatLeader;

	float										 PreviousTick;

	EFlareCombatTactic::Type			         CurrentTactic;


	/*----------------------------------------------------
		Helper
	----------------------------------------------------*/
	
public:

	inline PilotHelper::PilotTarget GetPilotTarget()
	{
		return PilotTarget;
	}

	inline UFlareSpacecraftComponent* GetPilotTargetShipComponent()
	{
		return PilotTargetShipComponent;
	}

	void SetUndockedAllShips(bool Set);
};
