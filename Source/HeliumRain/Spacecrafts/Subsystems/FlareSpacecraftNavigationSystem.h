#pragma once
#include "FlareSpacecraftDockingSystem.h"
#include "../../Data/FlareResourceCatalog.h"
#include "FlareSpacecraftNavigationSystem.generated.h"

class AFlareSpacecraft;

class UPhysicsConstraintComponent;
struct FFlareSpacecraftComponentDescription;

/** Status of the ship */
UENUM()
namespace EFlareShipStatus
{
	enum Type
	{
		SS_Manual,
		SS_AutoPilot,
		SS_Docked
	};
}
namespace EFlareShipStatus
{
	inline FString ToString(EFlareShipStatus::Type EnumValue)
	{
		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EFlareShipStatus"), true);
		return EnumPtr->GetNameStringByIndex(EnumValue);
	}
}


/** Type of command */
UENUM()
namespace EFlareCommandDataType
{
	enum Type
	{
		CDT_None,
		CDT_Location,
		CDT_Rotation,
		CDT_BrakeLocation,
		CDT_BrakeRotation,
		CDT_Dock
	};
}
namespace EFlareCommandDataType
{
	inline FString ToString(EFlareCommandDataType::Type EnumValue)
	{
		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EFlareCommandDataType"), true);
		return EnumPtr->GetNameStringByIndex(EnumValue);
	}
}


/** Structure holding all data for a single command */
struct FFlareShipCommandData
{
	TEnumAsByte <EFlareCommandDataType::Type> Type;

	bool PreciseApproach;
	FVector LocationTarget;
	FVector VelocityTarget;
	FVector RotationTarget;
	FVector LocalShipAxis;

	AFlareSpacecraft* ActionTarget;

	int32 ActionTargetParam;

};


/** Docking phase */
UENUM()
namespace EFlareDockingPhase
{
	enum Type
	{
		Docked,
		Dockable,
		FinalApproach,
		Approach,
		RendezVous,
		Distant,
		Locked,
	};
}

/* Current docking parameter */
struct FFlareDockingParameters
{
	EFlareDockingPhase::Type DockingPhase;

	// Ship
	FVector ShipDockLocation;
	FVector ShipDockAxis;
	FVector ShipDockOffset;

	// Camera
	float ShipHorizontalCameraOffset;
	float ShipVerticalCameraOffset;
	FVector ShipCameraTargetLocation;

	// Station
	FVector StationDockLocation;
	FVector StationDockAxis;
	FVector StationDockTopAxis;
	FVector StationAngularVelocity;

	// Approach
	float DockToDockDistance;
	FVector DockToDockDeltaLocation;
	FVector LinearVelocityAtShipDistance;
	FVector ShipDockSelfRotationInductedLinearVelocity;
};

/** Spacecraft navigation system class */
UCLASS()
class HELIUMRAIN_API UFlareSpacecraftNavigationSystem : public UObject
{

public:

	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void TickSystem(float DeltaSeconds);

	/** Initialize this system */
	virtual void Initialize(AFlareSpacecraft* OwnerSpacecraft, FFlareSpacecraftSave* OwnerData);

	virtual void Start();

	void ResetTransactionInfo();

public:

	/*----------------------------------------------------
		System API
	----------------------------------------------------*/

	virtual bool IsManualPilot();

	virtual bool IsAutoPilot();

	virtual bool IsDocked();

	/** Set the new flight status */
	virtual void SetStatus(EFlareShipStatus::Type NewStatus);

	virtual void SetAngularAccelerationRate(float Acceleration);

	/*----------------------------------------------------
		Docking
	----------------------------------------------------*/

	virtual bool DockAt(AFlareSpacecraft* TargetStation);

	FFlareDockingParameters GetDockingParameters(FFlareDockingInfo StationDockInfo, FVector CameraLocation);

	/** Continue docking sequence has completed until effectif docking */
	virtual void DockingAutopilot(AFlareSpacecraft* DockStation, int32 DockId, float DeltaSeconds);

	/** Confirm that the docking sequence has completed */
	virtual void ConfirmDock(AFlareSpacecraft* DockStation, int32 DockId, bool TellUser = true, bool UseDockingRestraint = true);

	virtual void ForceFinishAutoPilots();

	/** Check if the colliding spacecraft is not the station we want to dock to */
	virtual void CheckCollisionDocking(AFlareSpacecraft* DockingCandidate);

	virtual bool Undock();

	virtual void BreakDock();

	virtual AFlareSpacecraft* GetDockStation();

	/** Get a friendly text for the current phase */
	static FText GetDockingPhaseName(EFlareDockingPhase::Type Phase);

	/** Get the recommended speed for the current phase */
	static float GetDockingPhaseSpeed(EFlareDockingPhase::Type Phase);


	/*----------------------------------------------------
		Navigation commands and helpers
	----------------------------------------------------*/

	/** Brake (linear) */
	void PushCommandLinearBrake(const FVector& VelocityTarget = FVector::ZeroVector);

	/** Brake (angular) */
	void PushCommandAngularBrake();

	/** Go there */
	void PushCommandLocation(const FVector& Location, bool Precise = false, AFlareSpacecraft* NewActionTarget = nullptr);

	/** Turn this way */
	void PushCommandRotation(const FVector& RotationTarget, const FVector& LocalShipAxis);

	/** Dock to this */
	void PushCommandDock(const FFlareDockingInfo& DockingInfo);

	/** Enqueue an autopilot command */
	void PushCommand(const FFlareShipCommandData& Command);

	/** Clear the current autopilot command */
	void ClearCurrentCommand();

	/** Get the current command */
	FFlareShipCommandData GetCurrentCommand();

	/** Abort all the current pushed autopilot commands */
	void AbortAllCommands(bool AttemptUndock = true, bool ClearTransactionInfo = true);

	/** Get the dock offset from the origin of the ship in local space */
	virtual FVector GetDockOffset();

	/** Get the dock world location */
	virtual FVector GetDockLocation();

	/** Make sure this point is not in a path collider */
	virtual bool IsPointColliding(FVector Candidate, AActor* Ignore);

	virtual AFlareSpacecraft* GetNearestShip(AFlareSpacecraft* DockingStation) const;

	virtual void OnControlLost();

	/*----------------------------------------------------
		Internal attitude control
	----------------------------------------------------*/

	/** Automatically update the current linear attitude */
	bool UpdateLinearAttitudeAuto(float DeltaSeconds, FVector TargetLocation, FVector TargetVelocity, float MaxVelocity, float SecurityRatio);

	/** Brake */
	bool UpdateLinearBraking(FFlareShipCommandData& Command, float DeltaSeconds, FVector TargetVelocity);

	/** Automatically update the current angular attitude */
	bool UpdateAngularAttitudeAuto(FFlareShipCommandData& Command, float DeltaSeconds);

	/** Brake */
	bool UpdateAngularBraking(float DeltaSeconds);

	/** Return the angular velocity need to align the local ship axis to the target axis */
	virtual FVector GetAngularVelocityToAlignAxis(FVector LocalShipAxis, FVector TargetAxis, FVector TargetAngularVelocity, float DeltaSeconds);


	/*----------------------------------------------------
		Physics
	----------------------------------------------------*/

	/** Update the attitude control */
	void PhysicSubTick(float DeltaSeconds);

	/** Update the ship's center of mass */
	void UpdateCOM();

protected:
	/*----------------------------------------------------
		Protected data
	----------------------------------------------------*/

	UPROPERTY()
	AFlareSpacecraft*                               Spacecraft;

	UPROPERTY()
	UPhysicsConstraintComponent*                    DockConstraint;

	FFlareSpacecraftSave*                           Data;
	FFlareSpacecraftDescription*                    Description;
	TArray<UActorComponent*>                        Components;

	TEnumAsByte <EFlareShipStatus::Type>		    Status;

	// Configuration properties
	float                                    AngularDeadAngle;
	float                                    AngularMaxVelocity; // degree/s
	float                                    AngularAccelerationRate;
	float                                    LinearDeadDistance;
	float                                    LinearMaxVelocity; // m/s
	float                                    LinearMaxDockingVelocity; // m/s
	float                                    NegligibleSpeedRatio;

	// Navigation
	TArray <AActor*>                         PathColliders;
	TQueue <FFlareShipCommandData>           CommandData;
	float                                    AnticollisionAngle;
	bool                                     HasUsedOrbitalBoost;

	// Physics simulation
	FFlareEngineTarget                       LinearEngineTarget;
	FVector                                  AngularTargetVelocity;
	bool                                     UseOrbitalBoost;
	FVector                                  COM;

	TPair<TArray<int>, TArray<int>> XEngines;
	TPair<TArray<int>, TArray<int>> YEngines;
	TPair<TArray<int>, TArray<int>> ZEngines;

	//local upgrade memory
	FFlareSpacecraftComponentDescription*   TransactionNewPartDesc;
	int32									TransactionNewPartWeaponGroupIndex;

	//local trade memory
	FFlareResourceDescription*				 TransactionResource;
	uint32									 TransactionQuantity;
	UFlareSimulatedSpacecraft*				 TransactionSourceShip;
	UFlareSimulatedSpacecraft*			     TransactionDestination;
	UFlareSimulatedSpacecraft*			     TransactionDestinationDock;
	bool								     TransactionDonation;

	TMap<FVector, TMap<bool, float>>		 MaxTorqueCache;
	float									 MaxTorqueCacheClearTime;

public:

	virtual bool DockAtAndTrade(AFlareSpacecraft* TargetStation, FFlareResourceDescription* TransactionResource_, uint32 TransactionQuantity_, UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, bool Donation = 0);
	virtual bool DockAtAndUpgrade(FFlareSpacecraftComponentDescription* NewPartDesc, int32 CurrentWeaponGroupIndex);
	virtual bool DockAtAndUpgrade(FFlareSpacecraftComponentDescription* NewPartDesc, int32 CurrentWeaponGroupIndex, AFlareSpacecraft* TargetStation);

	/*----------------------------------------------------
		Getters (Attitude)
	----------------------------------------------------*/

	/**
	 * Return the maximum current (with damages) trust the ship can provide in a specific axis.
	 * Engines : List of engine to use to compute the max thrust
	 * Axis : Axis of the thurst
	 * WithObitalEngines : if false, ignore orbitals engines
	 */
	FVector GetTotalMaxThrustInAxis(TArray<UActorComponent*>& Engines, FVector Axis, bool WithOrbitalEngines) const;


	/**
	 * Return the maximum current (with damages) trust the ship can provide with specifics engines.
	 * Engines : List of engine to use to compute the max thrust
	 * UsefulEngines : engine to sum
	 * WithObitalEngines : if false, ignore orbitals engines
	 */
	float GetTotalMaxThrustWithEngines(TArray<UActorComponent*>& Engines, TArray<int>& UsefulEngines, bool WithOrbitalEngines);

	/**
	 * Return the maximum torque the ship can provide in a specific axis.
	 * Engines : List of engine to use to compute the max thrust
	 * TorqueDirection : Axis of the torque
	 * WithDamages : if true, use current thrust value and not theorical thrust value
	 */
	float GetTotalMaxTorqueInAxis(TArray<UActorComponent*>& Engines, FVector TorqueDirection, bool WithDamages);


	/*----------------------------------------------------
		Getters
	----------------------------------------------------*/

	inline float GetAngularAccelerationRate() const
	{
		return AngularAccelerationRate;
	}

	inline float GetAngularMaxVelocity() const
	{
		return AngularMaxVelocity;
	}

	inline float GetLinearMaxVelocity() const
	{
		return LinearMaxVelocity;
	}

	inline EFlareShipStatus::Type GetStatus() const
	{
		return Status;
	}

	inline bool IsUsingOrbitalEngines() const
	{
		return HasUsedOrbitalBoost;
	}

	inline UFlareSimulatedSpacecraft* GetTransactionDestination() const
	{
		return TransactionDestination;
	}
	inline UFlareSimulatedSpacecraft* GetTransactionDestinationDock() const
	{
		return TransactionDestinationDock;
	}

	inline FFlareSpacecraftComponentDescription* GetTransactionNewPartDesc() const
	{
		return TransactionNewPartDesc;
	}
	inline int32 GetTransactionNewPartWeaponIndex() const
	{
		return TransactionNewPartWeaponGroupIndex;
	}

	inline UFlareSimulatedSpacecraft* GetTransactionSourceShip() const
	{
		return TransactionSourceShip;
	}

	inline FFlareResourceDescription* GetTransactionResource() const
	{
		return TransactionResource;
	}

	inline uint32 GetTransactionQuantity() const
	{
		return TransactionQuantity;
	}

	inline bool GetTransactionDonation() const
	{
		return TransactionDonation;
	}
};
