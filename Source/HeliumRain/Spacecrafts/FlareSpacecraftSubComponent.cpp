
#include "FlareSpacecraftSubComponent.h"
#include "../Flare.h"
#include "FlareSpacecraft.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSpacecraftSubComponent::UFlareSpacecraftSubComponent(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/


void UFlareSpacecraftSubComponent::SetParentSpacecraftComponent(UFlareSpacecraftComponent* Component)
{
	ParentComponent = Component;
}


void UFlareSpacecraftSubComponent::TickForComponent(float DeltaTime)
{
	Super::TickForComponent(DeltaTime);



	SetHealth(FMath::Clamp(1 + (ParentComponent->GetDamageRatio()-1) / (1 -BROKEN_RATIO) , 0.f , 1.f));
	SetTemperature(ParentComponent->GetSpacecraft()->IsPresentationMode() ? 290 : ParentComponent->GetLocalTemperature());
}

float UFlareSpacecraftSubComponent::GetArmorAtLocation(FVector Location)
{
	return ParentComponent->GetArmorAtLocation(Location);
}

float UFlareSpacecraftSubComponent::ApplyDamage(float Energy, EFlareDamage::Type DamageType, UFlareSimulatedSpacecraft* DamageSource)
{
	return ParentComponent->ApplyDamage(Energy, DamageType, DamageSource);
}