
#include "FlareSpacecraftSpinningComponent.h"
#include "../Flare.h"
#include "FlareSpacecraft.h"
#include "Subsystems/FlareSpacecraftDamageSystem.h"
#include "../Game/FlareGame.h"
#include "../Game/FlarePlanetarium.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftSpinningComponent::UFlareSpacecraftSpinningComponent(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, RotationAxisRoll(true)
	, RotationAxisYaw(false)
	, RotationSpeed(20)
{
	GetBodyInstance()->bAutoWeld = false;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareSpacecraftSpinningComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UFlareSpacecraftSpinningComponent::Initialize(FFlareSpacecraftComponentSave* Data, UFlareCompany* Company, AFlareSpacecraftPawn* OwnerSpacecraftPawn, bool IsInMenu, AFlareSpacecraft* ActualOwnerShip)
{
	Super::Initialize(Data, Company, OwnerSpacecraftPawn, IsInMenu, ActualOwnerShip);

	SetCollisionProfileName("IgnoreOnlyPawn");
	NeedTackerInit = true;
	// Rotation axis

	if (RotationAxisRoll)
	{
		Axis = FRotator(0, 0, 1);
	}
	else if (RotationAxisYaw)
	{
		Axis = FRotator(0, 1, 0);
	}
	else
	{
		Axis = FRotator(1, 0, 0);
	}

	X = FVector (1, 0, 0);
	Y = FVector (0, 1, 0);
	Z = FVector (0, 0, 1);
}

void UFlareSpacecraftSpinningComponent::TickComponent(float DeltaSeconds, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaSeconds, TickType, ThisTickFunction);
	AFlareSpacecraft* Ship = GetSpacecraft();
	if (!Ship)
	{
		return;
	}

	// Update
	if (!Ship->IsPresentationMode() && Ship->GetParent()->GetDamageSystem()->IsAlive())
	{
		// Sun-looker
		if (LookForSun)
		{
			AFlarePlanetarium* Planetarium = Ship->GetGame()->GetPlanetarium();
			if (Planetarium && Planetarium->IsReady())
			{

				FVector SunDirection = Planetarium->GetSunDirection();
				FVector LocalSunDirection = GetComponentToWorld().GetRotation().Inverse().RotateVector(SunDirection);
				FVector LocalPlanarSunDirection = LocalSunDirection;

				if (RotationAxisRoll)
				{
					LocalPlanarSunDirection.X = 0;
				}
				else if (RotationAxisYaw)
				{
					LocalPlanarSunDirection.Z = 0;
				}
				else
				{
					LocalPlanarSunDirection.Y = 0;
				}

				if(!LocalPlanarSunDirection.IsNearlyZero())
				{
//					FVector X(1, 0, 0);
					float Angle = 0;
					FVector RotationAxis = X;
					LocalPlanarSunDirection.Normalize();

					if (RotationAxisRoll)
					{
						Angle =  -FMath::RadiansToDegrees(FMath::Atan2(LocalPlanarSunDirection.Y,LocalPlanarSunDirection.Z));
						RotationAxis = X;
					}
					else if (RotationAxisYaw)
					{
						Angle =  - FMath::RadiansToDegrees(FMath::Atan2(LocalPlanarSunDirection.X,LocalPlanarSunDirection.Y));
//						FVector Z(0, 0, 1);
						RotationAxis = Z;
					}
					else
					{
						Angle =  FMath::RadiansToDegrees(FMath::Atan2(LocalPlanarSunDirection.Z,LocalPlanarSunDirection.X));
//						FVector Y(0, 1, 0);
						RotationAxis = Y;
					}

					Angle = FMath::UnwindDegrees(Angle);

					float DeltaAngle = FMath::Sign(Angle) * FMath::Min(RotationSpeed * DeltaSeconds, FMath::Abs(Angle));

					if(NeedTackerInit)
					{
						DeltaAngle = Angle;
						NeedTackerInit = false;
					}

					if(FMath::Abs(Angle) > 0.01)
					{
						FRotator Delta = FQuat(RotationAxis, FMath::DegreesToRadians(DeltaAngle)).Rotator();
						AddLocalRotation(Delta);
					}
				}
			}
		}

		// Simple spinner
		else
		{
/*
			// Rotation axis
			FRotator Axis;
			if (RotationAxisRoll)
			{
				Axis = FRotator(0, 0, 1);
			}
			else if (RotationAxisYaw)
			{
				Axis = FRotator(0, 1, 0);
			}
			else
			{
				Axis = FRotator(1, 0, 0);
			}
*/
			AddLocalRotation(RotationSpeed * DeltaSeconds * Axis);
		}
	}
}


