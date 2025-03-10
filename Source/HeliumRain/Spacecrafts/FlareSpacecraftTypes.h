#pragma once

#include "EngineMinimal.h"
#include "../UI/Components/FlareButton.h"
#include "Styling/SlateBrush.h"
#include "Sound/SoundCue.h"
#include "Engine.h"
#include "FlareSpacecraftTypes.generated.h"

class UFlareResourceCatalogEntry;
class UFlareFactoryCatalogEntry;


struct FFlareEngineTarget
{
	bool XVelocityControl;
	bool YVelocityControl;
	bool ZVelocityControl;
	FVector Target;

	void SetVelocity(FVector Velocity)
	{
		Target = Velocity;
		XVelocityControl = true;
		YVelocityControl = true;
		ZVelocityControl = true;
	}
};


/** Damage Type */
UENUM()
namespace EFlareDamage
{
	enum Type
	{
		DAM_None,
		DAM_Collision,
		DAM_Overheat,
		DAM_HighExplosive,
		DAM_ArmorPiercing,
		DAM_HEAT,
	};
}


/** Shell fuze type */
UENUM()
namespace EFlareShellFuzeType
{
	enum Type
	{
		Contact, // The shell explode on contact
		Proximity, // The shell explode near the target
	};
}

/** Shell damage type */
UENUM()
namespace EFlareShellDamageType
{
	enum Type
	{
		HighExplosive, // Explosion send shell pieces around at hight velocity.
		ArmorPiercing, // Not explosive shell, The damage are done by kinetic energy. Classique bullets.
		HEAT,          // Heat Explosive Anti Tank. The explosion is focalized in a hot beam of metal melting armor.
		LightSalvage,  // No actual damage, enable retrieval of light ships
		HeavySalvage,  // No actual damage, enable retrieval of heavy ships
	};
}

/** Part size values */
UENUM()
namespace EFlarePartSize
{
	enum Type
	{
		S,
		L,
		Num
	};
}

namespace EFlarePartSize
{
	inline FString ToString(EFlarePartSize::Type EnumValue)
	{
		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EFlarePartSize"), true);
		return EnumPtr->GetNameStringByIndex(EnumValue);
	}
}

/** Part type values */
UENUM()
namespace EFlareSpawnMode
{
	enum Type
	{
		Safe,
		Spawn,
		Travel,
		Exit,
		InternalDocked
	};
}

/** Resource lock type values */
UENUM()
namespace EFlareResourceLock
{
	enum Type
	{
		NoLock, // The slot is free
		Input, // The slot accept only sell
		Output, // The slot accept only buy
		Trade, // The slot is lock to a resource and accept buy and sell according price threshold
		Hidden, // The slot is not available yet
	};
}
/** Resource restruction type values */
UENUM()
namespace EFlareResourceRestriction
{
	enum Type
	{
		Everybody, // Everybody can trade with this slot
		OwnerOnly, // Only the owner can trade with this slot
		BuyersOnly, // Only potential buyers can trade with this slot
		BuyersOwnerOnly,  // Only potential owner buyers can trade with this slot
		SellersOnly, // Only potential sellers can trade with this slot
		SellersOwnerOnly, // Only potential owner sellers can trade with this slot
		Nobody, // Nobody can trade with this slot
	};
}

/** Ship component turret save data */
USTRUCT()
struct FFlareSpacecraftComponentTurretSave
{
	GENERATED_USTRUCT_BODY()

	/** Attribute name */
	UPROPERTY(EditAnywhere, Category = Save) float TurretAngle;

	/** Attribute value */
	UPROPERTY(EditAnywhere, Category = Save) float BarrelsAngle;
};

/** Ship component weapons save data */
USTRUCT()
struct FFlareSpacecraftComponentWeaponSave
{
	GENERATED_USTRUCT_BODY()

	/** Attribute name */
	UPROPERTY(EditAnywhere, Category = Save) int32 FiredAmmo;
};

/** Turret pilot save data */
USTRUCT()
struct FFlareTurretPilotSave
{
	GENERATED_USTRUCT_BODY()

	/** Pilot identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Pilot name */
	UPROPERTY(EditAnywhere, Category = Save)
	FString Name;

};

/** Ship component save data */
USTRUCT()
struct FFlareSpacecraftComponentSave
{
	GENERATED_USTRUCT_BODY()

	/** Component catalog identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName ComponentIdentifier;

	/** Ship slot identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName ShipSlotIdentifier;

	/** Taken damages */
	UPROPERTY(EditAnywhere, Category = Save)
	float Damage;

	/** Component turret data*/
	UPROPERTY(EditAnywhere, Category = Save)
	FFlareSpacecraftComponentTurretSave Turret;

	/** Component turret data*/
	UPROPERTY(EditAnywhere, Category = Save)
	FFlareSpacecraftComponentWeaponSave Weapon;

	/** Pilot */
	UPROPERTY(EditAnywhere, Category = Save)
	FFlareTurretPilotSave Pilot;

	int64 IsPoweredCacheIndex;
	bool IsPoweredCache;
};

/** Ship pilot save data */
USTRUCT()
struct FFlareShipPilotSave
{
	GENERATED_USTRUCT_BODY()

	/** Pilot identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Pilot name */
	UPROPERTY(EditAnywhere, Category = Save)
	FString Name;

};

/** Asteroid save data */
USTRUCT()
struct FFlareAsteroidSave
{
	GENERATED_USTRUCT_BODY()

	/** Asteroid identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Asteroid location */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector Location;

	/** Asteroid rotation */
	UPROPERTY(EditAnywhere, Category = Save)
	FRotator Rotation;

	/** Asteroid linear velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector LinearVelocity;

	/** Asteroid angular velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector AngularVelocity;

	/** Asteroid scale */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector Scale;

	/** Content identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 AsteroidMeshID;
};

/** Meteorite save data */
USTRUCT()
struct FFlareMeteoriteSave
{
	GENERATED_USTRUCT_BODY()

	/** Meteorite identifier */
	//UPROPERTY(EditAnywhere, Category = Save)
	//FName Identifier;

	/** Meteorite location */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector Location;

	/** Meteorite target offset */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector TargetOffset;

	/** Meteorite rotation */
	UPROPERTY(EditAnywhere, Category = Save)
	FRotator Rotation;

	/** Meteorite linear velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector LinearVelocity;

	/** Meteorite angular velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector AngularVelocity;

	/** Content identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 MeteoriteMeshID;

	/** Is it a metallic meteorite */
	UPROPERTY(EditAnywhere, Category = Save)
	bool IsMetal;

	/** Damage to take to brake */
	UPROPERTY(EditAnywhere, Category = Save)
	float BrokenDamage;

	/** Current damage */
	UPROPERTY(EditAnywhere, Category = Save)
	float Damage;

	/** TargetStation */
	UPROPERTY(EditAnywhere, Category = Save)
	FName TargetStation;

	/** Has the meteorite missed the target */
	UPROPERTY(EditAnywhere, Category = Save)
	bool HasMissed;

	/** Days before impacts */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 DaysBeforeImpact;
};

/** Spacecraft cargo save data */
USTRUCT()
struct FFlareCargoSave
{
	GENERATED_USTRUCT_BODY()

	/** Cargo resource */
	UPROPERTY(EditAnywhere, Category = Save)
	FName ResourceIdentifier;

	/** Cargo quantity */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 Quantity;

	/** Resource Lock type */
	UPROPERTY(EditAnywhere, Category = Save)
	TEnumAsByte<EFlareResourceLock::Type> Lock;

	/** Resource restriction type */
	UPROPERTY(EditAnywhere, Category = Save)
	TEnumAsByte<EFlareResourceRestriction::Type> Restriction;
};

/** Station connection */
USTRUCT()
struct FFlareConnectionSave
{
	GENERATED_USTRUCT_BODY()

	/** Name of the connector */
	UPROPERTY(EditAnywhere, Category = Save)
	FName ConnectorName;

	/** Identifier of the target station */
	UPROPERTY(EditAnywhere, Category = Save)
	FName StationIdentifier;
};

/** Shipyard order save data */
USTRUCT()
struct FFlareShipyardOrderSave
{
	GENERATED_USTRUCT_BODY()
	/** Order ship class for shipyards */
	UPROPERTY(EditAnywhere, Category = Save)
	FName ShipClass;

	/** Order ship company owner */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Company;

	/** Order advance payment */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 AdvancePayment;

	/** Remaining production duration (only ship in construction) */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 RemainingProductionDuration;
};

/** Spacecraft factory save data */
USTRUCT()
struct FFlareFactorySave
{
	GENERATED_USTRUCT_BODY()

	/** Factory is active */
	UPROPERTY(EditAnywhere, Category = Save)
	bool Active;

	/** Money locked by the factory */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 CostReserved;

	/** Resource Reserved by the factory */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareCargoSave> ResourceReserved;

	/** Cumulated production duration for this production cycle */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 ProductedDuration;

	/** Factory production mode */
	UPROPERTY(EditAnywhere, Category = Save)
	bool InfiniteCycle;

	/** Factory cycle to produce */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 CycleCount;

	/** Max slot used for factory output */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareCargoSave> OutputCargoLimit;

	/** Target ship class for shipyards */
	UPROPERTY(EditAnywhere, Category = Save)
	FName TargetShipClass;

	/** Target ship company owner */
	UPROPERTY(EditAnywhere, Category = Save)
	FName TargetShipCompany;
};

/** Catalog binding between FFlareSpacecraftDescription and FFlareSpacecraftComponentDescription structure */
USTRUCT()
struct FFlareSpacecraftSlotGroupDescription
{
	GENERATED_USTRUCT_BODY()

	/** Group name */
	UPROPERTY(EditAnywhere, Category = Content) FText GroupName;

	/** Names of weapons allowed for this weapon group */
	UPROPERTY(EditAnywhere, Category = Content) TArray<FName> RestrictedWeapons;

	/** Name of default weapon for this weapon group*/
	UPROPERTY(EditAnywhere, Category = Content) FName DefaultWeapon;
};

/** Catalog binding between FFlareSpacecraftDescription and FFlareSpacecraftComponentDescription structure */
USTRUCT()
struct FFlareSpacecraftSlotDescription
{
	GENERATED_USTRUCT_BODY()

	/** Slot internal name */
	UPROPERTY(EditAnywhere, Category = Content) FName SlotIdentifier;

	/** Component description can be empty if configurable slot */
	UPROPERTY(EditAnywhere, Category = Content) FName ComponentIdentifier;

	/** Group index */
	UPROPERTY(EditAnywhere, Category = Content) int32 GroupIndex;
	
	/** Size of the slot  */
	UPROPERTY(EditAnywhere, Category = Content)
	TEnumAsByte<EFlarePartSize::Type> Size;

	/** Turret angle limits. The number of value indicate indicate the angular between each limit. For exemple 4 value are for 0°, 90°, -90° and 180°? */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<float> TurretBarrelsAngleLimit;

	/** Power components */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FName> PoweredComponents;

	/** Is the turret controllable in manual aim mode */
	UPROPERTY(EditAnywhere, Category = Content) bool IgnoreManualAim;
};

/** Spacecraft capabilities */
UENUM()
namespace EFlareSpacecraftCapability
{
	enum Type
	{
		Consumer,
		Storage,
		Maintenance,
		Upgrade
	};
}

/** Build constraints for stations */
UENUM()
namespace EFlareBuildConstraint
{
	enum Type
	{
		FreeAsteroid,
		SunExposure,
		HideOnIce,
		HideOnNoIce,
		GeostationaryOrbit,
		SpecialSlotInComplex,
		NoComplex
	};
}

UENUM()
namespace CompanyShortNames
{
	enum Type
	{
		PLAYER,
		MSY,
		HFR,
		SUN,
		GWS,
		ION,
		UFC,
		PIR,
		NHW,
		AXS,
		BRM,
		IFO,
		QNT
	};
}

/** Factory input or output resource */
USTRUCT()
struct FFlareFactoryResource
{
	GENERATED_USTRUCT_BODY()

	/** Resource */
	UPROPERTY(EditAnywhere, Category = Content)
	UFlareResourceCatalogEntry* Resource;

	/** Quantity for the resource */
	UPROPERTY(EditAnywhere, Category = Content)
	int32 Quantity;
};

/** Factory action type values */
UENUM()
namespace EFlareFactoryAction
{
	enum Type
	{
		CreateShip,
		GainResearch,
		DiscoverSector,
		BuildStation
	};
}

/** Factory output action */
USTRUCT()
struct FFlareFactoryAction
{
	GENERATED_USTRUCT_BODY()

	/** Faction action. */
	UPROPERTY(EditAnywhere, Category = Save)
	TEnumAsByte<EFlareFactoryAction::Type> Action;

	/** Generic identifier */
	UPROPERTY(EditAnywhere, Category = Content)
	FName Identifier;

	/** Quantity for this action */
	UPROPERTY(EditAnywhere, Category = Content)
	uint32 Quantity;
};

/** Production cost */
USTRUCT()
struct FFlareProductionData
{
	GENERATED_USTRUCT_BODY()

	/** Time for 1 production cycle */
	UPROPERTY(EditAnywhere, Category = Content)
	int64 ProductionTime;

	/** Cost for 1 production cycle */
	UPROPERTY(EditAnywhere, Category = Content)
	uint32 ProductionCost;

	/** Input resources */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareFactoryResource> InputResources;

	/** Output resources */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareFactoryResource> OutputResources;
};

/** Factory description */
USTRUCT()
struct FFlareFactoryDescription
{
	GENERATED_USTRUCT_BODY()

	/** Name */
	UPROPERTY(EditAnywhere, Category = Content)
	FText Name;

	/** Description */
	UPROPERTY(EditAnywhere, Category = Content)
	FText Description;

	/** Resource identifier */
	UPROPERTY(EditAnywhere, Category = Content)
	FName Identifier;

	/** Output actions */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareFactoryAction> OutputActions;
	
	/** Auto-start */
	UPROPERTY(EditAnywhere, Category = Content)
	bool AutoStart;

	/** Enable to stop the ability to turn this production off*/
	UPROPERTY(EditAnywhere, Category = Content)
	bool CantTurnOff;

	/** Cycle cost & yields */
	UPROPERTY(EditAnywhere, Category = Content)
	FFlareProductionData CycleCost;

	/** Visible states */
	UPROPERTY(EditAnywhere, Category = Content)
	bool VisibleStates;

	bool IsShipyard() const
	{
		for (int32 Index = 0; Index < OutputActions.Num(); Index++)
		{
			if (OutputActions[Index].Action == EFlareFactoryAction::CreateShip)
			{
				return true;
			}
		}
		return false;
	}

	bool IsResearch() const
	{
		for (int32 Index = 0; Index < OutputActions.Num(); Index++)
		{
			if (OutputActions[Index].Action == EFlareFactoryAction::GainResearch)
			{
				return true;
			}
		}
		return false;
	}

	bool IsTelescope() const
	{
		for (int32 Index = 0; Index < OutputActions.Num(); Index++)
		{
			if (OutputActions[Index].Action == EFlareFactoryAction::DiscoverSector)
			{
				return true;
			}
		}
		return false;
	}
};

/** Catalog data structure for a spacecraft */
USTRUCT()
struct FFlareSpacecraftDescription
{
	GENERATED_USTRUCT_BODY()

	/** Spacecraft internal name */
	UPROPERTY(EditAnywhere, Category = Content) FName Identifier;

	/** Spacecraft name */
	UPROPERTY(EditAnywhere, Category = Content) FText Name;

	/** Spacecraft name */
	UPROPERTY(EditAnywhere, Category = Content) FText ShortName;

	/** Spacecraft description */
	UPROPERTY(EditAnywhere, Category = Content) FText Description;

	/** Spacecraft description */
	UPROPERTY(EditAnywhere, Category = Content) FText ImmatriculationCode;

	/** Spacecraft mass */
	UPROPERTY(EditAnywhere, Category = Content) float Mass;

	/** Build constraints for stations */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<TEnumAsByte<EFlareBuildConstraint::Type>> BuildConstraint;

	/** Companies that can buy this ship. Company ShortName, or PLAYER*/
	UPROPERTY(EditAnywhere, Category = Content) TArray<FName> BuyableCompany;
	
	/** Companies that can build this ship. Company ShortName, or PLAYER*/
	UPROPERTY(EditAnywhere, Category = Content) TArray<FName> BuildableCompany;

	/** Ships and Stations that can build this ship*/
	UPROPERTY(EditAnywhere, Category = Content) TArray<FName> BuildableShip;

	/** Drone carriers require at least one shipyard factory to be able to build their ships*/
	UPROPERTY(EditAnywhere, Category = Content) bool IsDroneCarrier;

	/** Maximum amount of drones the carrier can have built at once*/
	UPROPERTY(EditAnywhere, Category = Content) int32 DroneMaximum;

	/** Launching delay of carrier ships*/
	UPROPERTY(EditAnywhere, Category = Content) float DroneLaunchDelay;

	/** How close a drone needs to get before it can dock. 125 is the default*/
	UPROPERTY(EditAnywhere, Category = Content) float DroneDockRadius;

	/** If this is a drone it is not buildable via shipyards and has a different method*/
	UPROPERTY(EditAnywhere, Category = Content) bool IsDroneShip;

	/** This ship is disabled */
	UPROPERTY(EditAnywhere, Category = Content) bool IsDisabled;

	/** This ship is disabled but it will override the stats of an older version that was found with this ones. Useful if rebalancing ships that your mod doesn't have the models etc for*/
	UPROPERTY(EditAnywhere, Category = Content) bool IsDisabledOverrideStats;

	/** Disables this entry if any of the designated mods aren't detected*/
	UPROPERTY(EditAnywhere, Category = Content) TArray<FString> IsDisabledIfModsNotLoaded;
	
	/** Enables this entry only if any mods in the array aren't detected as loaded*/
	UPROPERTY(EditAnywhere, Category = Content) TArray<FString> IsEnabledIfModsNotLoaded;

	/** This adjusts the priority of this ship being loaded. Higher numbers will override lower numbers. Equal numbers will override each other based on their load order*/
	UPROPERTY(EditAnywhere, Category = Content) int ModLoadPriority;

	/** All required unlocked technologies to build station or ship.*/
	UPROPERTY(EditAnywhere, Category = Content) TArray<FName> RequiredTechnologies;

	/** Size of the ship components */
	UPROPERTY(EditAnywhere, Category = Save)
	TEnumAsByte<EFlarePartSize::Type> Size;

	/** Number of RCS */
	UPROPERTY(EditAnywhere, Category = Save) int32 RCSCount;

	/** Specific RCS this ship can equip */
	UPROPERTY(EditAnywhere, Category = Content) TArray<FName> RestrictedRCS;

	/** Optionally set a default RCS */
	UPROPERTY(EditAnywhere, Category = Content) FName DefaultRCS;

	/** Quantity of RCS which can thrust in same direction as primary engines */
	UPROPERTY(EditAnywhere, Category = Content) int32 RCSCapableOfMainEngineThrust;

	/** Number of orbital engine */
	UPROPERTY(EditAnywhere, Category = Save) int32 OrbitalEngineCount;

	/** Specific Engines this ship can equip */
	UPROPERTY(EditAnywhere, Category = Content) TArray<FName> RestrictedEngines;

	/** Optionally set a default Engine */
	UPROPERTY(EditAnywhere, Category = Content) FName DefaultEngine;

	/** Enable this if you want a ship with turrets to be generally treated as a non-military ship */
	UPROPERTY(EditAnywhere, Category = Save) bool IsNotMilitary;

	/** Enable this if you want a ship with engines to be considered a station */
	UPROPERTY(EditAnywhere, Category = Save) bool IsAStation;

	/** Enable this if you want a ship or station to not be capturable*/
	UPROPERTY(EditAnywhere, Category = Save) bool IsUncapturable;

	/** Weapon groups */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareSpacecraftSlotGroupDescription> WeaponGroups;

	/** Gun slots */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareSpacecraftSlotDescription> GunSlots;
	/** Turret slots */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareSpacecraftSlotDescription> TurretSlots;

	/** Internal component slots */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FFlareSpacecraftSlotDescription> InternalComponentSlots;
	
	/** Templates for WIP versions in shipyards */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UClass*> ConstructionStateTemplates;

	/** Max angular velocity in degree/s */
	UPROPERTY(EditAnywhere, Category = Content)	float AngularMaxVelocity;

	/** Max linear velocity in m/s */
	UPROPERTY(EditAnywhere, Category = Content)	float LinearMaxVelocity;

	/** Heat capacity un KJ/K */
	UPROPERTY(EditAnywhere, Category = Content) float HeatCapacity;
	
	/** Spacecraft mesh */
	UPROPERTY(EditAnywhere, Category = Content) UClass* SpacecraftTemplate;

	/** Spacecraft mesh preview image */
	UPROPERTY(EditAnywhere, Category = Content) FSlateBrush MeshPreviewBrush;

	/** Engine Power sound*/
	UPROPERTY(EditAnywhere, Category = Content) USoundCue* PowerSound;

	/** Cargo bay count.*/
	UPROPERTY(EditAnywhere, Category = Content)
	uint32 CargoBayCount;

	/** Cargo bay capacity.*/
	UPROPERTY(EditAnywhere, Category = Content)
	uint32 CargoBayCapacity;

	/** Factories*/
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<UFlareFactoryCatalogEntry*> Factories;

	/** Is people can consume resource in this station */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<TEnumAsByte<EFlareSpacecraftCapability::Type>> Capabilities;

	/** Cycle cost & yields */
	UPROPERTY(EditAnywhere, Category = Content)
	FFlareProductionData CycleCost;

	/** Spacecraft combat points */
	UPROPERTY(EditAnywhere, Category = Content)
	int32 CombatPoints;

	/** Max level.*/
	UPROPERTY(EditAnywhere, Category = Content)
	int32 MaxLevel;

	/** How many capture points this ship adds. Fallback Small ship = 1, Large ship = 10.*/
	UPROPERTY(EditAnywhere, Category = Content)
	int32 CapturePointContribution;

	/** Capture point to capture (scale with level.*/
	UPROPERTY(EditAnywhere, Category = Content)
	int32 CapturePointThreshold;

	/** Is this a fixed station attached to world geometry */
	UPROPERTY(EditAnywhere, Category = Content)
	bool IsSubstation;
	
	/** List of station connectors */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<FName> StationConnectorNames;
	
	/** Number of station connectors */
	UPROPERTY(EditAnywhere, Category = Content)
	int32 StationConnectorCount;

	/** Save version for ship/station. If SaveVersion is higher than save value the ship will try to refresh components on load to prevent crashing*/
	UPROPERTY(EditAnywhere, Category = Content)
	int32 SaveVersion;

	int32 GetCapacity() const;

	bool IsAnUncapturable() const;

	bool IsStation() const;

	bool IsImmobileStation() const;

	bool IsShipyard() const;

	bool IsMilitary() const;
	bool IsMilitaryArmed() const;
	bool CheckIsNotMilitary() const;

	bool IsResearch() const;
	bool IsTelescope() const;
	static const FSlateBrush* GetIcon(FFlareSpacecraftDescription* Characteristic);
};
/** Spacecraft save data */
USTRUCT()
struct FFlareSpacecraftSave
{
	GENERATED_USTRUCT_BODY()

	/** Destroyed state */
	UPROPERTY(EditAnywhere, Category = Save)
	bool IsDestroyed;

	/** Destroyed state */
	UPROPERTY(EditAnywhere, Category = Save)
	bool IsUnderConstruction;

	/** Ship location */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector Location;

	/** Ship rotation */
	UPROPERTY(EditAnywhere, Category = Save)
	FRotator Rotation;

	/** The spawn mode of the ship. */
	UPROPERTY(EditAnywhere, Category = Save)
	TEnumAsByte<EFlareSpawnMode::Type> SpawnMode;

	/** Ship linear velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector LinearVelocity;

	/** Ship angular velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector AngularVelocity;

	/** Ship angular velocity */
	UPROPERTY(EditAnywhere, Category = Save)
	bool WantUndockInternalShips;

	/** Ship immatriculation. Readable for the player */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Immatriculation;

	/** Ship nickname. Readable for the player */
	UPROPERTY(EditAnywhere, Category = Save)
	FText NickName;

	/** Ship catalog identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Ship company identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName CompanyIdentifier;

	/** Components list */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareSpacecraftComponentSave> Components;

	/** We are docked at this station */
	UPROPERTY(EditAnywhere, Category = Save)
	FName DockedTo;

	/** We are docked at this location, internally */
	UPROPERTY(EditAnywhere, Category = Save)
	FName DockedAtInternally;

	/** We are docked at this specific dock */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 DockedAt;

	/** We are docked at this specific dock */
	UPROPERTY(EditAnywhere, Category = Save)
	float DockedAngle;

	/** Accululated heat in KJ */
	UPROPERTY(EditAnywhere, Category = Save)
	float Heat;

	/** Duration until the end of the power outage, in seconds */
	UPROPERTY(EditAnywhere, Category = Save)
	float PowerOutageDelay;

	/** Pending power outage downtime, in seconds */
	UPROPERTY(EditAnywhere, Category = Save)
	float PowerOutageAcculumator;

	/** Pilot */
	UPROPERTY(EditAnywhere, Category = Save)
	FFlareShipPilotSave Pilot;

	/** Production Cargo bay content */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareCargoSave> ProductionCargoBay;

	/** Construction Cargo bay content */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareCargoSave> ConstructionCargoBay;

	/** Factory states */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareFactorySave> FactoryStates;

	/** Asteroid we're stuck to */
	UPROPERTY(EditAnywhere, Category = Save)
	FFlareAsteroidSave AsteroidData;

	/** Factory states */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> SalesExcludedResources;

	/** Current state identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName DynamicComponentStateIdentifier;

	/** Current state progress */
	UPROPERTY(EditAnywhere, Category = Save)
	float DynamicComponentStateProgress;

	/** Station current level */
	int32 Level;

	/** Is a trade in progress */
	bool IsTrading;

	/** Simple number to represent the reason for trading. A value of 1 is reserved for carriers automatically trading for their resources*/
	int32 TradingReason;

	/** Is ship intercepted */
	bool IsIntercepted;

	/** Resource refill stock */
	float RepairStock;

	/** Resource refill stock */
	float RefillStock;

	/** Company that harpooned us */
	UPROPERTY(EditAnywhere, Category = Save)
	FName HarpoonCompany;

	/** Current capture points */
	UPROPERTY(EditAnywhere, Category = Save)
		TMap<FName, int32> CapturePoints;

	/** Actor to attach to */
	UPROPERTY(EditAnywhere, Category = Save)
		FName AttachActorName;

	/** Station complex to attach to */
	UPROPERTY(EditAnywhere, Category = Save)
		FName AttachComplexStationName;

	/** Station complex dock to attach to */
	UPROPERTY(EditAnywhere, Category = Save)
		FName AttachComplexConnectorName;

	/** Is a in sector reserve */
	bool IsReserve;

	/** Allow other company to order ships */
	bool AllowExternalOrder;

	/* Allow autoconstruction capability**/
	bool AllowAutoConstruction;

	/** List of ship order */
	TArray<FFlareShipyardOrderSave> ShipyardOrderQueue;

	/** List of connected stations */
	TArray<FFlareConnectionSave> ConnectedStations;

	//	/** List of ship order */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> ShipyardOrderExternalConfig;

	/** Ship that is our owner (IE for carriers)*/
	UPROPERTY(EditAnywhere, Category = Save)
	FName OwnerShipName;

	/** Ships that are our subordinates (IE for carriers)*/
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> OwnedShipNames;

	/* Whitelist Identifier*/
	UPROPERTY(EditAnywhere, Category = Save)
	FName DefaultWhiteListIdentifier;

	/** Save version for ship/station. If changed will attempt to refresh components on load to prevent crashing*/
	UPROPERTY(EditAnywhere, Category = Content)
	int32 SaveVersion;
};

struct SpacecraftHelper
{
	static float GetIntersectionPosition(FVector TargetLocation,
										 FVector TargetVelocity,
										 FVector SourceLocation,
										 FVector SourceVelocity,
										 float ProjectileSpeed,
										 float PredictionDelay,
										 FVector* ResultPosition);

	static float GetIntersectionPosition2(FVector TargetLocation,
										 FVector TargetVelocity,
										 FVector TargetAcceleration,
										 FVector SourceLocation,
										 FVector SourceVelocity,
										 float ProjectileSpeed,
										 float PredictionDelay,
										 FVector* ResultPosition);

	static EFlareDamage::Type GetWeaponDamageType(EFlareShellDamageType::Type ShellDamageType);

};


/** Data structure for a spacecraft in skirmish */
USTRUCT()
struct FFlareSkirmishSpacecraftOrder
{
	GENERATED_USTRUCT_BODY()

	// Base structure
	const FFlareSpacecraftDescription* Description;

	// Upgrades
	FName EngineType;
	FName RCSType;
	TArray<FName> WeaponTypes;

	// Menu hint
	bool ForPlayer;
	bool IsReserve;
	int32 Quantity = 1;
	TSharedPtr<SSlider> QuantitySlider;
	TSharedPtr<SFlareButton> ReserveButton;
	
	// Constructors
	FFlareSkirmishSpacecraftOrder(const FFlareSpacecraftDescription* Desc)
		: Description(Desc)
	{}
	FFlareSkirmishSpacecraftOrder()
		: Description(NULL)
	{}

	// Constructor helper for lists
	static TSharedPtr<FFlareSkirmishSpacecraftOrder> New(const FFlareSpacecraftDescription* Desc)
	{
		return MakeShareable(new FFlareSkirmishSpacecraftOrder(Desc));
	}

//todo: perhaps wrap the quantity/reserve buttons into their own struct
	void SetQuantitySlider(TSharedPtr<SSlider> NewQuantitySlider)
	{
		QuantitySlider = NewQuantitySlider;
	}
	void SetReserveButton(TSharedPtr<SFlareButton> NewReserveButton)
	{
		ReserveButton = NewReserveButton;
	}
};

UCLASS()
class HELIUMRAIN_API UFlareSpacecraftTypes : public UObject
{
	GENERATED_UCLASS_BODY()

public:
};
