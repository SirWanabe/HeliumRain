
#include "FlareSpacecraftSpinningComponent.h"
#include "../Flare.h"
#include "FlareSpacecraft.h"
#include "Subsystems/FlareSpacecraftDamageSystem.h"
#include "../Game/FlareGame.h"
#include "../Game/FlarePlanetarium.h"
#include "../Player/FlarePlayerController.h"

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

#define REAL_ATTENTION_RATE 0
#define HIGH_ATTENTION_RATE 0.0166666666666667f
#define MODERATE_ATTENTION_RATE 0.025f
#define LOW_ATTENTION_RATE 0.05
#define LOWEST_ATTENTION_RATE 0.1f

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
	PrimaryComponentTick.TickInterval = HIGH_ATTENTION_RATE;
}

void UFlareSpacecraftSpinningComponent::TickForComponent(float DeltaSeconds)
{
	Super::TickForComponent(DeltaSeconds);
	AFlareSpacecraft* Ship = GetSpacecraft();
	if (!Ship)
	{
		return;
	}

	// Update
	if (!Ship->IsPresentationMode() && Ship->GetParent()->GetDamageSystem()->IsAlive())
	{
		SpinLODCheck += DeltaSeconds;
		if (SpinLODCheck >= 1 && Ship->GetPC())
		{
			AFlareSpacecraft* PlayerShipPawn = Ship->GetPC()->GetShipPawn();
			if (PlayerShipPawn)
			{
				// 100 = 1.0KM
				float Distance = (PlayerShipPawn->GetActorLocation() - GetComponentLocation()).Size() / 1000;
				if (Distance < 5)
				{
					PrimaryComponentTick.TickInterval = REAL_ATTENTION_RATE;
				}
				else if (Distance < 100)
				{
					PrimaryComponentTick.TickInterval = HIGH_ATTENTION_RATE;
				}
				else if (Distance < 250)
				{
					PrimaryComponentTick.TickInterval = MODERATE_ATTENTION_RATE;
				}
				else if (Distance < 500)
				{
					PrimaryComponentTick.TickInterval = LOW_ATTENTION_RATE;
				}
				else
				{
					PrimaryComponentTick.TickInterval = LOWEST_ATTENTION_RATE;
				}
			}
			else
			{
				PrimaryComponentTick.TickInterval = LOWEST_ATTENTION_RATE;
			}
			SpinLODCheck = 0;
		}

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

				if (!LocalPlanarSunDirection.IsNearlyZero())
				{
					float Angle = 0;
					FVector RotationAxis = X;
					LocalPlanarSunDirection.Normalize();

					if (RotationAxisRoll)
					{
						Angle = -FMath::RadiansToDegrees(FMath::Atan2(LocalPlanarSunDirection.Y, LocalPlanarSunDirection.Z));
						RotationAxis = X;
					}
					else if (RotationAxisYaw)
					{
						Angle = -FMath::RadiansToDegrees(FMath::Atan2(LocalPlanarSunDirection.X, LocalPlanarSunDirection.Y));
						RotationAxis = Z;
					}
					else
					{
						Angle = FMath::RadiansToDegrees(FMath::Atan2(LocalPlanarSunDirection.Z, LocalPlanarSunDirection.X));
						RotationAxis = Y;
					}

					Angle = FMath::UnwindDegrees(Angle);

					float DeltaAngle = FMath::Sign(Angle) * FMath::Min(RotationSpeed * DeltaSeconds, FMath::Abs(Angle));

					if (NeedTackerInit)
					{
						DeltaAngle = Angle;
						NeedTackerInit = false;
					}

					if (FMath::Abs(Angle) > 0.01)
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
			AddLocalRotation(RotationSpeed * DeltaSeconds * Axis);
		}
	}
	else
	{
		PrimaryComponentTick.TickInterval = LOWEST_ATTENTION_RATE;
	}
}