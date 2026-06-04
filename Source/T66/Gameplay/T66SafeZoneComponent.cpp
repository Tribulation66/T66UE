// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66SafeZoneComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"

UT66SafeZoneComponent::UT66SafeZoneComponent()
{
	InitSphereRadius(650.f);
}

void UT66SafeZoneComponent::ConfigureSafeZone(const float InRadius)
{
	SetSphereRadius(FMath::Max(1.f, InRadius));
	UpdateSafeZoneVisual();
}

void UT66SafeZoneComponent::BeginPlay()
{
	Super::BeginPlay();

	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetGenerateOverlapEvents(true);
	SetCollisionObjectType(ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	OnComponentBeginOverlap.AddDynamic(this, &UT66SafeZoneComponent::HandleSafeZoneBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UT66SafeZoneComponent::HandleSafeZoneEndOverlap);

	if (UWorld* World = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->RegisterSafeZone(this);
		}
	}

	EnsureSafeZoneVisual();
	UpdateSafeZoneVisual();
}

void UT66SafeZoneComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AT66HeroBase* Hero = OverlappingHero.Get())
	{
		Hero->AddSafeZoneOverlap(-1);
		OverlappingHero.Reset();
	}

	if (UWorld* World = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->UnregisterSafeZone(this);
		}
	}

	OnComponentBeginOverlap.RemoveDynamic(this, &UT66SafeZoneComponent::HandleSafeZoneBeginOverlap);
	OnComponentEndOverlap.RemoveDynamic(this, &UT66SafeZoneComponent::HandleSafeZoneEndOverlap);

	if (SafeZoneVisualComponent)
	{
		SafeZoneVisualComponent->DestroyComponent();
		SafeZoneVisualComponent = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UT66SafeZoneComponent::HandleSafeZoneBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;
	(void)bFromSweep;
	(void)SweepResult;

	AT66HeroBase* Hero = ResolveHero(OtherActor);
	if (!Hero || OverlappingHero.IsValid())
	{
		return;
	}

	OverlappingHero = Hero;
	Hero->AddSafeZoneOverlap(+1);
}

void UT66SafeZoneComponent::HandleSafeZoneEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;

	AT66HeroBase* Hero = ResolveHero(OtherActor);
	if (!Hero || OverlappingHero.Get() != Hero)
	{
		return;
	}

	OverlappingHero.Reset();
	Hero->AddSafeZoneOverlap(-1);
}

AT66HeroBase* UT66SafeZoneComponent::ResolveHero(AActor* OtherActor) const
{
	return Cast<AT66HeroBase>(OtherActor);
}

void UT66SafeZoneComponent::EnsureSafeZoneVisual()
{
	if (SafeZoneVisualComponent && SafeZoneVisualComponent->IsRegistered())
	{
		return;
	}

	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return;
	}

	const FName ComponentName(*FString::Printf(TEXT("%s_SafeZoneBubbleVisual"), *GetName()));
	SafeZoneVisualComponent = NewObject<UStaticMeshComponent>(Owner, ComponentName, RF_Transient);
	if (!SafeZoneVisualComponent)
	{
		return;
	}

	SafeZoneVisualComponent->Mobility = EComponentMobility::Movable;
	SafeZoneVisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SafeZoneVisualComponent->SetGenerateOverlapEvents(false);
	SafeZoneVisualComponent->SetCastShadow(false);
	SafeZoneVisualComponent->bReceivesDecals = false;
	SafeZoneVisualComponent->TranslucencySortPriority = 5;
	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		SafeZoneVisualComponent->SetStaticMesh(Sphere);
	}
	if (UMaterialInterface* BubbleMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Game/Stylized_VFX_StPack/Materials/M_Bubble.M_Bubble")))
	{
		SafeZoneVisualComponent->SetMaterial(0, BubbleMaterial);
	}
	else
	{
		FT66VisualUtil::ApplyT66Color(SafeZoneVisualComponent, Owner, FLinearColor(0.65f, 0.95f, 0.72f, 0.35f));
	}

	SafeZoneVisualComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	SafeZoneVisualComponent->RegisterComponent();
}

void UT66SafeZoneComponent::UpdateSafeZoneVisual()
{
	if (!SafeZoneVisualComponent)
	{
		return;
	}

	const float Radius = FMath::Max(1.f, GetUnscaledSphereRadius());
	const float ScaleXY = Radius / 50.f;
	const float ScaleZ = FMath::Max(1.0f, (Radius * 0.32f) / 50.f);
	SafeZoneVisualComponent->SetRelativeLocation(FVector(0.f, 0.f, Radius * 0.12f));
	SafeZoneVisualComponent->SetRelativeScale3D(FVector(ScaleXY, ScaleXY, ScaleZ));
	SafeZoneVisualComponent->SetHiddenInGame(false, true);
	SafeZoneVisualComponent->SetVisibility(true, true);
}

#if !UE_BUILD_SHIPPING
bool UT66SafeZoneComponent::HasSafeZoneVisualForAutomation() const
{
	return SafeZoneVisualComponent
		&& SafeZoneVisualComponent->IsRegistered()
		&& SafeZoneVisualComponent->GetStaticMesh() != nullptr
		&& SafeZoneVisualComponent->IsVisible()
		&& !SafeZoneVisualComponent->bHiddenInGame;
}
#endif
