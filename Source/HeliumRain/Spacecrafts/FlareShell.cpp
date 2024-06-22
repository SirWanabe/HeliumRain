
#include "FlareShell.h"
#include "../Flare.h"

#include "FlareSpacecraft.h"

#include "../Game/FlareGame.h"
#include "../Game/FlareGameTypes.h"
#include "../Game/FlareSkirmishManager.h"

#include "../Player/FlarePlayerController.h"

#include "Components/DecalComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine.h"

#define DELETION_DELAYSHELL 1.f
#define MAXIMUM_DETONATIONS_PER_TICK 10

/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

AFlareShell::AFlareShell(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	// Mesh data
	ShellComp = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));
	RootComponent = ShellComp;

	// Settings
	FlightEffects = NULL;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetActorTickInterval(0.025f);
	ManualTurret = false;
}
 
/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

void AFlareShell::Initialize(UFlareWeapon* Weapon, const FFlareSpacecraftComponentDescription* Description, FVector ShootDirection, FVector ParentVelocity, bool Tracer)
{
//	TraceDelegate.BindUObject(this, &AFlareShell::OnTraceCompleted);

	ShellDescription = Description;
	TracerShell = Tracer;
	ParentWeapon = Weapon;
	Armed = false;
	MinEffectiveDistance = 0.f;

	// Can't exist without description, can't return
	FCHECK(Description);

	ImpactSound = Description->WeaponCharacteristics.ImpactSound;
	DamageSound = Description->WeaponCharacteristics.DamageSound;

	ExplosionEffectTemplate = Description->WeaponCharacteristics.ExplosionEffect;
	ImpactEffectTemplate = Description->WeaponCharacteristics.ImpactEffect;
	ExplosionEffectScale = Description->WeaponCharacteristics.ExplosionEffectScale;
	ImpactEffectScale = Description->WeaponCharacteristics.ImpactEffectScale;

	FlightEffectsTemplate = Description->WeaponCharacteristics.GunCharacteristics.TracerEffect;
	ExplosionEffectMaterial = Description->WeaponCharacteristics.GunCharacteristics.ExplosionMaterial;
	
	float AmmoVelocity = Description->WeaponCharacteristics.GunCharacteristics.AmmoVelocity;
	float KineticEnergy = Description->WeaponCharacteristics.GunCharacteristics.KineticEnergy;
	
	ShellVelocity = ParentVelocity + ShootDirection * AmmoVelocity * 100;
	ShellMass = 2 * KineticEnergy * 1000 / FMath::Square(AmmoVelocity); // ShellPower is in Kilo-Joule, reverse kinetic energy equation

	LastLocation = GetActorLocation();
	// Spawn the flight effects
	if (TracerShell)
	{
		if (FlightEffects != NULL && FlightEffects != nullptr)
		{
			FlightEffects->SetTemplate(FlightEffectsTemplate);
			FlightEffects->Activate(true);
		}
		else
		{
			FlightEffects = UGameplayStatics::SpawnEmitterAttached(
			FlightEffectsTemplate,
			RootComponent,
			NAME_None,
			FVector(0, 0, 0),
			FRotator(0, 0, 0),
			EAttachLocation::KeepRelativeOffset,
			false);
		}
	}

	SetLifeSpan(ShellDescription->WeaponCharacteristics.GunCharacteristics.AmmoRange * 100 / ShellVelocity.Size()); // 10km
	ParentWeapon->GetSpacecraft()->GetGame()->GetActiveSector()->RegisterShell(this);
	PC = ParentWeapon->GetSpacecraft()->GetGame()->GetPC();

	ManualTurret = ParentWeapon->GetSpacecraft()->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_TURRET;
	LocalSector = ParentWeapon->GetSpacecraft()->GetGame()->GetActiveSector();
}

void AFlareShell::Tick(float DeltaSeconds)
{
	if (IsDetonating)
	{
		ProcessCurrentDetonations();
		return;
	}

	if(IsSafeDestroyingRunning)
	{
		return;
	}
/*
	if (LastTraceHandle._Data.FrameNumber != 0)
	{
		FTraceDatum OutData;
		if (GetWorld()->QueryTraceData(LastTraceHandle, OutData))
		{
			// Clear out handle so next tick we don't enter
			LastTraceHandle._Data.FrameNumber = 0;
			// trace is finished, do stuff with results
			DoWorkWithTraceResults(OutData);
		}
	}
*/
	FVector ActorLocation = GetActorLocation();
	FVector NextActorLocation = ActorLocation + ShellVelocity * DeltaSeconds;
	SetActorLocationAndRotation(NextActorLocation, ShellVelocity.Rotation(), false);

	// 1 at 100m or less
	float Scale = 1;
	float BaseDistance = 10000.f;
	float MinScale = 0.1f;
	if(PC->GetShipPawn())
	{
		float LifeRatio = GetLifeSpan() / InitialLifeSpan;

		float LifeRatioScale = 1.f;

		if(LifeRatio < 0.1f)
		{
			LifeRatioScale = LifeRatio * 10.f;
		}

		float Distance = (NextActorLocation - PC->GetShipPawn()->GetActorLocation()).Size();
		if(Distance > BaseDistance)
		{
			Scale = (Distance / BaseDistance) * ((1.f-MinScale) * BaseDistance / Distance +MinScale) * LifeRatioScale;
		}
	}

	SetActorRelativeScale3D(FVector(0.6 + Scale * 0.4 , Scale, Scale));

	if (ShellDescription)
	{

		FHitResult HitResult(ForceInit);
		//TODO get async trace going without left over projectiles
//		RequestTrace(ActorLocation, NextActorLocation);

		if (Trace(ActorLocation, NextActorLocation, HitResult))
//		if(PreviousHitResult.GetActor() != nullptr)
		{
			OnImpact(HitResult, ShellVelocity);
//			OnImpact(PreviousHitResult, ShellVelocity);
		}

		if (ShellDescription->WeaponCharacteristics.FuzeType == EFlareShellFuzeType::Proximity)
		{
			//FLOGV("%s SecureTime: %f ActiveTime: %f",*GetName(), SecureTime, ActiveTime);


			if (SecureTime > 0)
			{
				SecureTime -= DeltaSeconds;
			}
			else if (ActiveTime > 0)
			{
				CheckFuze(ActorLocation, NextActorLocation);
				ActiveTime -= DeltaSeconds;
			}
		}
	}
}

void AFlareShell::CheckFuze(FVector ActorLocation, FVector NextActorLocation)
{
	if (!LocalSector)
	{
		return;
	}

	FVector Center = (NextActorLocation + ActorLocation) / 2;
	float NearThresoldSquared = FMath::Square(100000); // 1km

	auto CheckTarget = [&](PilotHelper::PilotTarget TargetCandidate)
	{
		if (ParentWeapon && TargetCandidate.Is(ParentWeapon->GetSpacecraft()))
		{
			// Ignore parent spacecraft
			return;
		}

		if (TargetCandidate.IsEmpty())
		{
			return;
		}

		// First check if near to filter distant ship
		if ((Center - TargetCandidate.GetActorLocation()).SizeSquared() > NearThresoldSquared)
		{
			return;
		}

		//FLOG("=================");
		//FLOGV("Proximity fuze near ship for %s",*GetHumanReadableName());


		FVector ShellDirection = ShellVelocity.GetUnsafeNormal();
		FVector CandidateOffset = TargetCandidate.GetActorLocation() - ActorLocation;
		FVector NextCandidateOffset = TargetCandidate.GetActorLocation() - NextActorLocation;

		// Min distance
		float MinDistance = FVector::CrossProduct(CandidateOffset, ShellDirection).Size() / ShellDirection.Size();

		// Check if the min distance is not in the past
		if (FVector::DotProduct(CandidateOffset, ShellDirection) < 0)
		{
			// The target is behind the shell
			MinDistance = CandidateOffset.Size();
		}

		bool MinInFuture = false;
		// Check if the min distance is not in the future
		if (FVector::DotProduct(NextCandidateOffset, ShellDirection) > 0)
		{
			// The target is before the shell
			MinDistance = NextCandidateOffset.Size();
			MinInFuture = true;
		}

		float DistanceToMinDistancePoint;
		if (CandidateOffset.Size() == MinDistance)
		{
			DistanceToMinDistancePoint = 0;
		}
		else if (NextCandidateOffset.Size() == MinDistance)
		{
			DistanceToMinDistancePoint = (NextActorLocation - ActorLocation).Size();
		}
		else
		{
			DistanceToMinDistancePoint = FMath::Sqrt(CandidateOffset.SizeSquared() - FMath::Square(MinDistance));
		}

		/*FLOGV("ShipCandidate->GetMeshScale() %f",TargetCandidate.GetMeshScale());
		FLOGV("DistanceToMinDistancePoint %f",DistanceToMinDistancePoint);
		FLOGV("Step distance %f",(NextActorLocation - ActorLocation).Size());
		FLOGV("MinDistance %f",MinDistance);
		FLOGV("Start Distance %f",(ActorLocation - TargetCandidate.GetActorLocation()).Size());
		FLOGV("End Distance %f",(NextActorLocation - TargetCandidate.GetActorLocation()).Size());*/


		// Check if need to detonnate
		float EffectiveDistance = MinDistance - TargetCandidate.GetMeshScale();


		if (EffectiveDistance < ShellDescription->WeaponCharacteristics.FuzeMinDistanceThresold *100)
		{
			// Detonate because of too near. Find the detonate point.


			float MinThresoldDistance = ShellDescription->WeaponCharacteristics.FuzeMinDistanceThresold *100 + TargetCandidate.GetMeshScale();

		// 	FLOGV("MinThresoldDistance %f",MinThresoldDistance);
			float DistanceToMinThresoldDistancePoint = FMath::Sqrt(FMath::Square(MinThresoldDistance) - FMath::Square(MinDistance));
		// 	FLOGV("DistanceToMinThresoldDistancePoint %f",DistanceToMinThresoldDistancePoint);

			float DistanceToDetonatePoint = DistanceToMinDistancePoint - DistanceToMinThresoldDistancePoint;
		// 	FLOGV("DistanceToDetonatePoint %f",DistanceToDetonatePoint);
			FVector DetonatePoint = ActorLocation + ShellDirection * DistanceToDetonatePoint;

			DetonateAt(DetonatePoint);
		}
		else if (Armed && EffectiveDistance > MinEffectiveDistance)
		{
			// We are armed and the distance as increase, detonate at nearest point
			FVector DetonatePoint = ActorLocation + ShellDirection * DistanceToMinDistancePoint;
			DetonateAt(DetonatePoint);
		}
		else if (EffectiveDistance < ShellDescription->WeaponCharacteristics.FuzeMaxDistanceThresold *100)
		{
			if (MinInFuture)
			{
				// In activation zone but we will be near in future, arm the fuze
				Armed = true;
				MinEffectiveDistance = EffectiveDistance;
			}
			else
			{
				// In activation zone and the min distance is reach in this step, detonate
				FVector DetonatePoint = ActorLocation + ShellDirection * DistanceToMinDistancePoint;
			// 	FLOGV("DistanceToMinDistancePoint %f",DistanceToMinDistancePoint);
				DetonateAt(DetonatePoint);
			}
		}
	};

	// Check targets against every actor in the level, using index-based, not for-range, because this can affect the container

	PilotHelper::PilotTarget FuzeTarget;
	UFlareSector* CurrentSector = LocalSector;
	for (int32 Index = 0; Index < CurrentSector->GetSpacecrafts().Num(); Index++)
	{
		if (IsDetonating || IsSafeDestroyingRunning)
		{
			break;
		}
		AFlareSpacecraft* Checker = CurrentSector->GetSpacecrafts()[Index];
		if (Checker)
		{
			FuzeTarget.Clear();
			FuzeTarget.SetSpacecraft(Checker);
			CheckTarget(FuzeTarget);
		}
	}

	for (int32 Index = 0; Index < CurrentSector->GetBombs().Num(); Index++)
	{
		if (IsDetonating || IsSafeDestroyingRunning)
		{
			break;
		}

		AFlareBomb* Checker = CurrentSector->GetBombs()[Index];
		if (Checker)
		{
			FuzeTarget.Clear();
			FuzeTarget.SetBomb(Checker);
			CheckTarget(FuzeTarget);
		}
	}

	for (int32 Index = 0; Index < CurrentSector->GetMeteorites().Num(); Index++)
	{
		if (IsDetonating || IsSafeDestroyingRunning)
		{
			break;
		}

		AFlareMeteorite* Checker = CurrentSector->GetMeteorites()[Index];
		if (Checker)
		{
			FuzeTarget.Clear();
			FuzeTarget.SetMeteorite(Checker);
			CheckTarget(FuzeTarget);
		}
	}
}

void AFlareShell::OnImpact(const FHitResult& HitResult, const FVector& HitVelocity)
{
	bool DestroyProjectile = true;
	
	if (HitResult.Actor.IsValid() && HitResult.Component.IsValid() && ParentWeapon)
	{
		// Compute projectile energy.
		FVector ProjectileVelocity = HitVelocity / 100;
		FVector TargetVelocity = HitResult.Component->GetPhysicsLinearVelocity() / 100;
		FVector ImpactVelocity = ProjectileVelocity - TargetVelocity;
		FVector ImpactVelocityAxis = ImpactVelocity.GetUnsafeNormal();
	
		// Compute parameters
		float ShellEnergy = 0.5f * ShellMass * ImpactVelocity.SizeSquared() / 1000; // Damage in KJ
		
		float AbsorbedEnergy = ApplyDamage(HitResult.Actor.Get(), HitResult.GetComponent(), HitResult.Location, ImpactVelocityAxis, HitResult.ImpactNormal, ShellEnergy, ShellDescription->WeaponCharacteristics.AmmoDamageRadius, EFlareDamage::DAM_ArmorPiercing);
		bool Richochet = (AbsorbedEnergy < ShellEnergy);

		if (Richochet)
		{
			float RemainingEnergy = ShellEnergy - AbsorbedEnergy;
			float RemainingVelocity = FMath::Sqrt(2 * RemainingEnergy * 1000 / ShellMass);
			if (RemainingVelocity > 0)
			{
				DestroyProjectile = false;
				FVector BounceDirection = ShellVelocity.GetUnsafeNormal().MirrorByVector(HitResult.ImpactNormal);
				ShellVelocity = BounceDirection * RemainingVelocity * 100;
				SetActorLocation(HitResult.Location);
			}
			else
			{
				DestroyProjectile = true;
			}
		}
		else
		{
			AFlareSpacecraft* Spacecraft = Cast<AFlareSpacecraft>(HitResult.Actor.Get());
			if (ShellDescription->WeaponCharacteristics.DamageType == EFlareShellDamageType::HEAT)
			{
				AFlareAsteroid* Asteroid = Cast<AFlareAsteroid>(HitResult.Actor.Get());
				AFlareMeteorite* Meteorite = Cast<AFlareMeteorite>(HitResult.Actor.Get());
				if (Spacecraft)
				{
					Spacecraft->GetDamageSystem()->ApplyDamage(ShellDescription->WeaponCharacteristics.ExplosionPower,
						ShellDescription->WeaponCharacteristics.AmmoDamageRadius, HitResult.Location, EFlareDamage::DAM_HEAT, ParentWeapon->GetSpacecraft()->GetParent(), GetName());

					float ImpulseForce = 1000 * ShellDescription->WeaponCharacteristics.ExplosionPower * ShellDescription->WeaponCharacteristics.AmmoDamageRadius;

					// Physics impulse
					Spacecraft->Airframe->AddImpulseAtLocation( ShellVelocity.GetUnsafeNormal(), HitResult.Location);
				}
				else if (Asteroid)
				{
					float ImpulseForce = 1000 * ShellDescription->WeaponCharacteristics.ExplosionPower * ShellDescription->WeaponCharacteristics.AmmoDamageRadius;
					Asteroid->GetAsteroidComponent()->AddImpulseAtLocation( ShellVelocity.GetUnsafeNormal(), HitResult.Location);
				}
				else if (Meteorite)
				{
					float ImpulseForce = 1000 * ShellDescription->WeaponCharacteristics.ExplosionPower * ShellDescription->WeaponCharacteristics.AmmoDamageRadius;
					Meteorite->GetMeteoriteComponent()->AddImpulseAtLocation( ShellVelocity.GetUnsafeNormal(), HitResult.Location);
					Meteorite->ApplyDamage(ShellDescription->WeaponCharacteristics.ExplosionPower,
										   ShellDescription->WeaponCharacteristics.AmmoDamageRadius, HitResult.Location, EFlareDamage::DAM_HEAT, ParentWeapon->GetSpacecraft()->GetParent(), GetName());
				}

			}

			// Spawn penetration effect
			if(!(Spacecraft && Spacecraft->GetCameraMode() == EFlareCameraMode::Immersive))
			{
				UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
					ExplosionEffectTemplate,
					HitResult.GetComponent(),
					NAME_None,
					HitResult.Location,
					HitResult.ImpactNormal.Rotation(),
					EAttachLocation::KeepWorldPosition,
					true,
					EPSCPoolMethod::AutoRelease);
				if (PSC)
				{
					PSC->SetWorldScale3D(ExplosionEffectScale * FVector(1, 1, 1));
				}
			}

			// Spawn hull damage effect
			UFlareSpacecraftComponent* HullComp = Cast<UFlareSpacecraftComponent>(HitResult.GetComponent());
			if (HullComp)
			{
				HullComp->StartDamagedEffect(HitResult.Location, HitResult.ImpactNormal.Rotation(), ParentWeapon->GetDescription()->Size);
			}

			// Remove flight effects
			if (FlightEffects)
			{
				FlightEffects->Deactivate();
			}
		}
	}

	if (DestroyProjectile)
	{
		SafeDestroy();
	}
}

void AFlareShell::DetonateAt(FVector DetonatePoint)
{
	if (!LocalSector)
	{
		return;
	}

	DetonationPoint = DetonatePoint;
	UGameplayStatics::SpawnEmitterAtLocation(this,
	ExplosionEffectTemplate,
	DetonatePoint);

	IsDetonating = true;
	DetonationSpacecraftCandidates.Empty();
	DetonationBombCandidates.Empty();
	DetonationMeteoriteCandidates.Empty();

	for (int32 Index = 0; Index < LocalSector->GetSpacecrafts().Num(); Index++)
	{
		AFlareSpacecraft* Checker = LocalSector->GetSpacecrafts()[Index];
		if (Checker)
		{
			DetonationSpacecraftCandidates.Add(Checker);
		}
	}

	for (int32 Index = 0; Index < LocalSector->GetBombs().Num(); Index++)
	{
		AFlareBomb* Checker = LocalSector->GetBombs()[Index];
		if (Checker)
		{
			DetonationBombCandidates.Add(Checker);
		}
	}

	for (int32 Index = 0; Index < LocalSector->GetMeteorites().Num(); Index++)
	{
		AFlareMeteorite* Checker = LocalSector->GetMeteorites()[Index];
		if (Checker)
		{
			DetonationMeteoriteCandidates.Add(Checker);
		}
	}
	
	SafeDestroy();
	ProcessCurrentDetonations();
}

void AFlareShell::ProcessCurrentDetonations()
{
	int32 CurrentDetonationQuantity = 1;
	auto CheckTarget = [&](PilotHelper::PilotTarget Target)
	{
		if (Target.IsEmpty())
		{
			return;
		}

		//FLOGV("DetonateAt CheckTarget for %s",*Target.GetActor()->GetName());

		// First check if in radius area
		FVector CandidateOffset = Target.GetActorLocation() - DetonationPoint;
		float CandidateDistance = CandidateOffset.Size();
		float CandidateSize = Target.GetMeshScale();

		if (CandidateDistance > ShellDescription->WeaponCharacteristics.AmmoExplosionRadius * 100 + CandidateSize)
		{
			//FLOG("Too far");
			return;
		}

		CurrentDetonationQuantity++;

		// DrawDebugSphere(ParentWeapon->GetSpacecraft()->GetWorld(), ShipCandidate->GetActorLocation(), CandidateSize, 12, FColor::Magenta, true);

		//FLOGV("CandidateOffset at %s",*CandidateOffset.ToString());

		// Find exposed surface
		// Apparent radius
		float ApparentRadius = FMath::Sqrt(FMath::Square(CandidateDistance) + FMath::Square(CandidateSize));

		float Angle = FMath::Acos(CandidateDistance / ApparentRadius);

		// DrawDebugSphere(ParentWeapon->GetSpacecraft()->GetWorld(), DetonatePoint, ApparentRadius, 12, FColor::Yellow, true);

		float ExposedSurface = 2 * PI * ApparentRadius * (ApparentRadius - CandidateDistance);
		float TotalSurface = 4 * PI * FMath::Square(ApparentRadius);

		float ExposedSurfaceRatio = ExposedSurface / TotalSurface;
		int FragmentCount = FMath::RandRange(0, 2) + ShellDescription->WeaponCharacteristics.AmmoFragmentCount * ExposedSurfaceRatio;

		/*FLOGV("CandidateDistance %f",CandidateDistance);
		FLOGV("CandidateSize %f",CandidateSize);
		FLOGV("ApparentRadius %f",ApparentRadius);
		FLOGV("Angle %f",FMath::RadiansToDegrees(Angle));
		FLOGV("ExposedSurface %f",ExposedSurface);
		FLOGV("TotalSurface %f",TotalSurface);
		FLOGV("ExposedSurfaceRatio %f",ExposedSurfaceRatio);
		FLOGV("FragmentCount %d",FragmentCount);*/

		TArray<UActorComponent*> Components = Target.GetActor()->GetComponentsByClass(UPrimitiveComponent::StaticClass());

		//FLOGV("Component cont %d",Components.Num());
		for (int i = 0; i < FragmentCount; i++)
		{

			FVector HitDirection = FMath::VRandCone(CandidateOffset, Angle);

			bool HasHit = false;
			FHitResult BestHitResult;
			float BestHitDistance = 0;

			for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
			{
				UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(Components[ComponentIndex]);
				if (Component)
				{
					FHitResult HitResult(ForceInit);
					FCollisionQueryParams TraceParams(FName(TEXT("Fragment Trace")), true, this);
					TraceParams.bTraceComplex = true;
					TraceParams.bReturnPhysicalMaterial = false;
					Component->LineTraceComponent(HitResult, DetonationPoint, DetonationPoint + HitDirection * 2 * CandidateDistance, TraceParams);

					if (HitResult.Actor.IsValid()) {
						float HitDistance = (HitResult.Location - DetonationPoint).Size();
						if (!HasHit || HitDistance < BestHitDistance)
						{
							BestHitDistance = HitDistance;
							BestHitResult = HitResult;
						}

						//FLOGV("Fragment %d hit %s at a distance=%f",i, *Component->GetReadableName(), HitDistance);
						HasHit = true;
					}
				}
			}

			if (HasHit)
			{
				//UKismetSystemLibrary::DrawDebugLine(ParentWeapon->GetSpacecraft()->GetWorld(), DetonatePoint, BestHitResult.Location, FColor::Green, 1000.f);

				AFlareSpacecraft* Spacecraft = Cast<AFlareSpacecraft>(BestHitResult.Actor.Get());
				if (Spacecraft)
				{
					float FragmentPowerEffet = FMath::FRandRange(0.f, 2.f);
					float FragmentRangeEffet = FMath::FRandRange(0.5f, 1.5f);
					ApplyDamage(Spacecraft, BestHitResult.GetComponent()
						, BestHitResult.Location
						, HitDirection
						, BestHitResult.ImpactNormal
						, FragmentPowerEffet * ShellDescription->WeaponCharacteristics.ExplosionPower
						, FragmentRangeEffet  * ShellDescription->WeaponCharacteristics.AmmoDamageRadius
						, EFlareDamage::DAM_HighExplosive);

					// Play sound
					AFlareSpacecraftPawn* SpacecraftPawn = Cast<AFlareSpacecraftPawn>(Spacecraft);
					if (SpacecraftPawn->IsPlayerShip())
					{
						SpacecraftPawn->GetPC()->PlayLocalizedSound(ImpactSound, BestHitResult.Location, BestHitResult.GetComponent());
					}
				}
				else
				{
					AFlareBomb* Bomb = Cast<AFlareBomb>(BestHitResult.Actor.Get());
					if (Bomb)
					{
						Bomb->OnBombDetonated(nullptr, nullptr, BestHitResult.Location, BestHitResult.ImpactNormal,ParentWeapon->GetSpacecraft()->GetCompany());
					}
					else
					{
						AFlareMeteorite* Meteorite = Cast<AFlareMeteorite>(BestHitResult.Actor.Get());
						if (Meteorite)
						{
							float FragmentPowerEffet = FMath::FRandRange(0.f, 2.f);
							float FragmentRangeEffet = FMath::FRandRange(0.5f, 1.5f);
							ApplyDamage(Meteorite, BestHitResult.GetComponent()
								, BestHitResult.Location
								, HitDirection
								, BestHitResult.ImpactNormal
								, FragmentPowerEffet * ShellDescription->WeaponCharacteristics.ExplosionPower
								, FragmentRangeEffet  * ShellDescription->WeaponCharacteristics.AmmoDamageRadius
								, EFlareDamage::DAM_HighExplosive);
						}
					}
				}
			}
		}
	};

	PilotHelper::PilotTarget ShrapnelTarget;
	while(DetonationSpacecraftCandidates.Num())
	{
		if (CurrentDetonationQuantity > MAXIMUM_DETONATIONS_PER_TICK)
		{
			break;
		}

		AFlareSpacecraft* Checker = DetonationSpacecraftCandidates[0];
		if (Checker)
		{
			ShrapnelTarget.Clear();
			ShrapnelTarget.SetSpacecraft(Checker);
			CheckTarget(ShrapnelTarget);
			DetonationSpacecraftCandidates.RemoveSwap(Checker);
		}
		else
		{
			DetonationSpacecraftCandidates.RemoveAtSwap(0);
		}
	}

	while (DetonationBombCandidates.Num())
	{
		if (CurrentDetonationQuantity > MAXIMUM_DETONATIONS_PER_TICK)
		{
			break;
		}

		AFlareBomb* Checker = DetonationBombCandidates[0];
		if (Checker)
		{
			ShrapnelTarget.Clear();
			ShrapnelTarget.SetBomb(Checker);
			CheckTarget(ShrapnelTarget);
			DetonationBombCandidates.RemoveSwap(Checker);
		}
		else
		{
			DetonationBombCandidates.RemoveAt(0);
		}
	}

	while (DetonationMeteoriteCandidates.Num())
	{
		if (CurrentDetonationQuantity > MAXIMUM_DETONATIONS_PER_TICK)
		{
			break;
		}

		AFlareMeteorite* Checker = DetonationMeteoriteCandidates[0];
		if (Checker)
		{
			ShrapnelTarget.Clear();
			ShrapnelTarget.SetMeteorite(Checker);
			CheckTarget(ShrapnelTarget);
			DetonationMeteoriteCandidates.RemoveSwap(Checker);
		}
		else
		{
			DetonationMeteoriteCandidates.RemoveAt(0);
		}
	}

	if (!DetonationSpacecraftCandidates.Num() && !DetonationBombCandidates.Num() && !DetonationMeteoriteCandidates.Num())
	{
		IsDetonating = false;
		FinishSafeDestroy();
	}
}

// Check targets against every actor in the level, using index-based, not for-range, because this can affect the container


float AFlareShell::ApplyDamage(AActor *ActorToDamage, UPrimitiveComponent* HitComponent, FVector ImpactLocation,  FVector ImpactAxis,  FVector ImpactNormal, float ImpactPower, float ImpactRadius, EFlareDamage::Type DamageType)
{
	float Incidence = FVector::DotProduct(ImpactNormal, -ImpactAxis);
	float Armor = 1; // Full armored

	if (Incidence < 0)
	{
		// Parasite hit after rebound, ignore
		return 0;
	}

	// Hit a component
	UFlareSpacecraftComponent* ShipComponent = Cast<UFlareSpacecraftComponent>(HitComponent);
	if (ShipComponent)
	{
		 Armor = ShipComponent->GetArmorAtLocation(ImpactLocation);
	}

	// Check armor peneration
	int32 PenetrateArmor = false;
	float PenerationIncidenceLimit = 0.7f;
	if (Incidence > PenerationIncidenceLimit)
	{
		PenetrateArmor = true; // No ricochet
	}
	else if (Armor == 0)
	{
		PenetrateArmor = true; // Armor destruction
	}

	// Hit a component : damage in KJ
	float AbsorbedEnergy = (PenetrateArmor ? ImpactPower : FMath::Square(Incidence) * ImpactPower);
	AFlareSpacecraft* Spacecraft = Cast<AFlareSpacecraft>(ActorToDamage);

	if (Spacecraft && ParentWeapon)
	{
		DamageCause Cause(Cast<AFlareSpacecraft>(ParentWeapon->GetOwner())->GetParent(), DamageType);
		Cause.ManualTurret = ManualTurret;
		Spacecraft->GetDamageSystem()->SetLastDamageCause(Cause);
		Spacecraft->GetDamageSystem()->ApplyDamage(AbsorbedEnergy, ImpactRadius, ImpactLocation, DamageType, ParentWeapon->GetSpacecraft()->GetParent(), GetName());

		// Physics impulse
		Spacecraft->Airframe->AddImpulseAtLocation(5000 * ImpactRadius * AbsorbedEnergy * (PenetrateArmor ? ImpactAxis : -ImpactNormal), ImpactLocation);

		// Play sound
		AFlareSpacecraftPawn* SpacecraftPawn = Cast<AFlareSpacecraftPawn>(Spacecraft);
		if (SpacecraftPawn->IsPlayerShip())
		{
			SpacecraftPawn->GetPC()->PlayLocalizedSound(PenetrateArmor ? DamageSound : ImpactSound, ImpactLocation, HitComponent);
		}

		// Quest progress
		if (ParentWeapon->GetSpacecraft()->GetGame()->GetQuestManager()
			&& ParentWeapon->GetSpacecraft()->GetParent() == ParentWeapon->GetSpacecraft()->GetGame()->GetPC()->GetPlayerShip())
		{
			ParentWeapon->GetSpacecraft()->GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("hit-ship").PutName("immatriculation", Spacecraft->GetImmatriculation()));
		}

		// Skirmish scoring
		if (ParentWeapon->GetSpacecraft()->GetGame()->IsSkirmish())
		{
			bool HitByPlayer = ParentWeapon->GetSpacecraft()->GetCompany() == ParentWeapon->GetSpacecraft()->GetPC()->GetCompany();
			ParentWeapon->GetSpacecraft()->GetGame()->GetSkirmishManager()->AmmoHit(HitByPlayer);
		}
	}
	else
	{
		AFlareAsteroid* Asteroid = Cast<AFlareAsteroid>(ActorToDamage);
		if (Asteroid)
		{
			// Physics impulse
			Asteroid->GetAsteroidComponent()->AddImpulseAtLocation(5000 * ImpactRadius * AbsorbedEnergy * (PenetrateArmor ? ImpactAxis : -ImpactNormal), ImpactLocation);
			if (ParentWeapon->GetSpacecraft()->GetGame()->GetQuestManager()
				&& ParentWeapon->GetSpacecraft()->GetParent() == ParentWeapon->GetSpacecraft()->GetGame()->GetPC()->GetPlayerShip())
			{
				ParentWeapon->GetSpacecraft()->GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("hit-asteroid"));
			}
		}
		else
		{
			AFlareMeteorite* Meteorite = Cast<AFlareMeteorite>(ActorToDamage);
			if (Meteorite)
			{
				// Physics impulse
				Meteorite->GetMeteoriteComponent()->AddImpulseAtLocation(5000 * ImpactRadius * AbsorbedEnergy * (PenetrateArmor ? ImpactAxis : -ImpactNormal), ImpactLocation);
				if (ParentWeapon->GetSpacecraft()->GetGame()->GetQuestManager()
					&& ParentWeapon->GetSpacecraft()->GetParent() == ParentWeapon->GetSpacecraft()->GetGame()->GetPC()->GetPlayerShip())
				{
					ParentWeapon->GetSpacecraft()->GetGame()->GetQuestManager()->OnEvent(FFlareBundle().PutTag("hit-meteorite"));
				}
				Meteorite->ApplyDamage(AbsorbedEnergy, ImpactRadius, ImpactLocation, DamageType, ParentWeapon->GetSpacecraft()->GetParent(), GetName());
			}
			else
			{
				AFlareBomb* Bomb = Cast<AFlareBomb>(ActorToDamage);
				if (Bomb)
				{
					FHitResult Hit;
					Bomb->NotifyHit(HitComponent, this, NULL, false, ImpactLocation, ImpactNormal, FVector::ZeroVector, Hit);
				}
			}
		}
	}

	// Spawn impact decal
	if (HitComponent)
	{
		float DecalSize = FMath::FRandRange(50, 100);
		UDecalComponent* Decal = UGameplayStatics::SpawnDecalAttached(
			ExplosionEffectMaterial,
			DecalSize * FVector(1, 1, 1),
			HitComponent,
			NAME_None,
			ImpactLocation,
			ImpactNormal.Rotation(),
			EAttachLocation::KeepWorldPosition,
			120);
		if (Decal)
		{
			// Instanciate and configure the decal material
			UMaterialInterface* DecalMaterial = Decal->GetMaterial(0);
			UMaterialInstanceDynamic* DecalMaterialInst = UMaterialInstanceDynamic::Create(DecalMaterial, GetWorld());
			if (DecalMaterialInst)
			{
				DecalMaterialInst->SetScalarParameterValue("RandomParameter", FMath::FRandRange(1, 0));
				DecalMaterialInst->SetScalarParameterValue("RandomParameter2", FMath::FRandRange(1, 0));
				DecalMaterialInst->SetScalarParameterValue("IsShipHull", HitComponent->IsA(UFlareSpacecraftComponent::StaticClass()));
				Decal->SetMaterial(0, DecalMaterialInst);
			}
		}
	}

	// Apply FX
	if (HitComponent && !(Spacecraft && Spacecraft->GetCameraMode() == EFlareCameraMode::Immersive))
	{
		UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
			ImpactEffectTemplate,
			HitComponent,
			NAME_None,
			ImpactLocation,
			ImpactNormal.Rotation(),
			EAttachLocation::KeepWorldPosition,
			true,
			EPSCPoolMethod::AutoRelease);
		if (PSC)
		{
			PSC->SetWorldScale3D(ImpactEffectScale * FVector(1, 1, 1));
		}
	}

	return AbsorbedEnergy;
}

FTraceHandle AFlareShell::RequestTrace(const FVector& Start, const FVector& End)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return FTraceHandle();
	}

	ECollisionChannel CollisionChannel = (ECollisionChannel)(ECC_WorldStatic | ECC_WorldDynamic | ECC_Pawn);

	bool bTraceComplex = false;
	bool bIgnoreSelf = true;
	TArray<AActor*> ActorsToIgnore;

	FCollisionQueryParams TraceParams(FName(TEXT("Shell Trace")), true, this);
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.AddIgnoredActor(this);
	if (ParentWeapon)
	{
		TraceParams.AddIgnoredActor(ParentWeapon->GetSpacecraft());
	}

	return World->AsyncLineTraceByChannel(EAsyncTraceType::Single,
		Start, End,
		CollisionChannel,
		TraceParams,
		FCollisionResponseParams::DefaultResponseParam,
		&TraceDelegate);
}

void AFlareShell::DoWorkWithTraceResults(const FTraceDatum& TraceData)
{
	TArray< struct FHitResult > OutHits = TraceData.OutHits;
	if (OutHits.Num())
	{
		PreviousHitResult = OutHits[0];
	}
}

void AFlareShell::OnTraceCompleted(const FTraceHandle& Handle, FTraceDatum& Data)
{
	ensure(Handle == LastTraceHandle);
	DoWorkWithTraceResults(Data);
	LastTraceHandle._Data.FrameNumber = 0; // reset it
}

bool AFlareShell::Trace(const FVector& Start, const FVector& End, FHitResult& HitOut)
{
	// Ignore Actors
	FCollisionQueryParams TraceParams(FName(TEXT("Shell Trace")), true, this);
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.AddIgnoredActor(this);
	if (ParentWeapon)
	{
		TraceParams.AddIgnoredActor(ParentWeapon->GetSpacecraft());
	}

	// Re-initialize hit info
	HitOut = FHitResult(ForceInit);

	ECollisionChannel CollisionChannel = (ECollisionChannel) (ECC_WorldStatic | ECC_WorldDynamic | ECC_Pawn);

	// Trace!
	GetWorld()->LineTraceSingleByChannel(
		HitOut,		// result
		Start,	// start
		End , // end
		CollisionChannel, // collision channel
		TraceParams
	);

	// Hit any Actor?
	return (HitOut.GetActor() != NULL) ;
}

void AFlareShell::LifeSpanExpired()
{
	if (!IsDetonating)
	{
		FinishSafeDestroy();
	}
}

void AFlareShell::SafeDestroy()
{
	if (!IsSafeDestroyingRunning)
	{
		IsSafeDestroyingRunning = true;
		this->SetActorHiddenInGame(true);
		this->SetActorEnableCollision(false);
		if (!IsDetonating)
		{
			this->SetActorTickEnabled(false);
			SetLifeSpan(DELETION_DELAYSHELL);
		}
	}
}

void AFlareShell::UnsetSafeDestroyed()
{
	SetLifeSpan(0);
	IsSafeDestroyingRunning = false;
	SafeDestroyed = false;
	this->SetActorHiddenInGame(false);
	this->SetActorEnableCollision(true);
	this->SetActorTickEnabled(true);
	SetActorScale3D(FVector(1, 1, 1));
}

void AFlareShell::FinishSafeDestroy()
{
	if (!SafeDestroyed)
	{
		SafeDestroy();
		this->SetActorTickEnabled(false);

		IsDetonating = false;
		SafeDestroyed = true;

		DetonationSpacecraftCandidates.Empty();
		DetonationBombCandidates.Empty();
		DetonationMeteoriteCandidates.Empty();

		AFlareGame* Game = Cast<AFlareGame>(GetWorld()->GetAuthGameMode());
		FCHECK(Game);

		ImpactSound = nullptr;
		DamageSound = nullptr;
		ExplosionEffectTemplate = nullptr;
		ImpactEffectTemplate = nullptr;
		FlightEffectsTemplate = nullptr;

		if (FlightEffects)
		{
			FlightEffects->Deactivate();
		}

		PreviousHitResult.Reset();
		ExplosionEffectMaterial = nullptr;
		ShellDescription = nullptr;
		ParentWeapon = nullptr;
		PC = nullptr;

		UFlareSector* Sector = Game->GetActiveSector();
		if (Sector&&IsValid(Sector))
		{
			Sector->UnregisterShell(this);
		}

		Game->GetCacheSystem()->StoreCachedShell(this);
		LocalSector = nullptr;
	}
}

/*
void AFlareShell::Destroyed()
{
	Super::Destroyed();
}
*/

UFlareWeapon* AFlareShell::GetParentWeapon()
{
	return ParentWeapon;
}

void AFlareShell::SetFuzeTimer(float TargetSecureTime, float TargetActiveTime)
{
	SecureTime = TargetSecureTime;
	ActiveTime = TargetActiveTime;
}

void AFlareShell::SetPause(bool Pause)
{
	SetActorHiddenInGame(Pause);
	CustomTimeDilation = (Pause ? 0.f : 1.0);
}