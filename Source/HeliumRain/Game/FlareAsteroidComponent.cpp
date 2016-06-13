
#include "../Flare.h"
#include "FlareAsteroidComponent.h"
#include "FlareGame.h"

#include "StaticMeshResources.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareAsteroidComponent::UFlareAsteroidComponent(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	// Mesh data
	bTraceComplexOnMove = true;
	SetSimulatePhysics(true);
	SetLinearDamping(0);
	SetAngularDamping(0);

	// FX
	static ConstructorHelpers::FObjectFinder<UParticleSystem> IceEffectTemplateObj(TEXT("/Game/Master/Particles/PS_IceEffect.PS_IceEffect"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> DustEffectTemplateObj(TEXT("/Game/Master/Particles/PS_DustEffect.PS_DustEffect"));
	IceEffectTemplate = IceEffectTemplateObj.Object;
	DustEffectTemplate = DustEffectTemplateObj.Object;

	// Settings
	PrimaryComponentTick.bCanEverTick = true;
	IsIcyAsteroid = true;
	EffectsCount = FMath::RandRange(5, 10);
	EffectsScale = 0.05;
	EffectsUpdatePeriod = 0.17f;
	EffectsUpdateTimer = 0;
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void UFlareAsteroidComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IsIcyAsteroid)
	{
		// Create random effects
		for (int32 Index = 0; Index < EffectsCount; Index++)
		{
			EffectsKernels.Add(FMath::VRand());

			UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
				IceEffectTemplate,
				this,
				NAME_None,
				GetComponentLocation(),
				FRotator(),
				EAttachLocation::KeepWorldPosition,
				false);

			PSC->SetWorldScale3D(EffectsScale * FVector(1, 1, 1));
			Effects.Add(PSC);

			PSC->SetTemplate(IceEffectTemplate);
		}
	}
}

void UFlareAsteroidComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	float CollisionSize = GetCollisionShape().GetExtent().Size();
	EffectsUpdateTimer += DeltaTime;

	if (EffectsUpdateTimer > EffectsUpdatePeriod)
	{
		// World data
		AFlareGame* Game = Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
		FVector AsteroidLocation = GetComponentLocation();
		FVector SunDirection = Game->GetPlanetarium()->GetSunDirection();
		SunDirection.Normalize();
	
		// Compute new FX locations
		for (int32 Index = 0; Index < EffectsKernels.Num(); Index++)
		{
			FVector RandomDirection = FVector::CrossProduct(SunDirection, EffectsKernels[Index]);
			RandomDirection.Normalize();
			FVector StartPoint = AsteroidLocation + RandomDirection * CollisionSize;

			// Trace params
			FHitResult HitResult(ForceInit);
			FCollisionQueryParams TraceParams(FName(TEXT("Asteroid Trace")), false, NULL);
			TraceParams.bTraceComplex = true;
			TraceParams.bReturnPhysicalMaterial = false;
			ECollisionChannel CollisionChannel = ECollisionChannel::ECC_WorldDynamic;

			// Trace
			bool FoundHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, AsteroidLocation, CollisionChannel, TraceParams);
			if (FoundHit && HitResult.Component == this && Effects[Index])
			{
				FVector EffectLocation = HitResult.Location;

				if (!Effects[Index]->IsActive())
				{
					Effects[Index]->Activate();
				}
				Effects[Index]->SetWorldLocation(EffectLocation);
				Effects[Index]->SetWorldRotation(SunDirection.Rotation());
			}
			else
			{
				Effects[Index]->Deactivate();
			}
		}

		EffectsUpdateTimer = 0;
	}
}

void UFlareAsteroidComponent::SetIcy(bool Icy)
{
	IsIcyAsteroid = Icy;
}