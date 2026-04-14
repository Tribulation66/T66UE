// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Components/T66MiniShadowComponent.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "VFX/T66MiniVfxShared.h"

UT66MiniShadowComponent::UT66MiniShadowComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UT66MiniShadowComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureShadowMesh();
}

void UT66MiniShadowComponent::InitializeShadow(
	USceneComponent* InAttachParent,
	const FVector& InRelativeLocation,
	const FVector& InRelativeScale,
	const FLinearColor& InTint)
{
	AttachParent = InAttachParent;
	RelativeLocation = InRelativeLocation;
	RelativeScale = InRelativeScale;
	Tint = InTint;
	if (GetOwner() && GetOwner()->HasActorBegunPlay())
	{
		EnsureShadowMesh();
	}
}

void UT66MiniShadowComponent::EnsureShadowMesh()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (!ShadowMesh)
	{
		ShadowMesh = NewObject<UStaticMeshComponent>(Owner, TEXT("MiniShadowMesh"));
		if (!ShadowMesh)
		{
			return;
		}

		Owner->AddInstanceComponent(ShadowMesh);
		ShadowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ShadowMesh->SetCastShadow(false);
		ShadowMesh->SetStaticMesh(T66MiniVfx::LoadPlaneMesh());
		ShadowMesh->RegisterComponent();
		USceneComponent* ParentComponent = AttachParent.Get() ? AttachParent.Get() : Owner->GetRootComponent();
		ShadowMesh->AttachToComponent(ParentComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}

	ShadowMesh->SetRelativeLocation(RelativeLocation);
	ShadowMesh->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	ShadowMesh->SetRelativeScale3D(RelativeScale);
	T66MiniVfx::ApplyTintedMaterial(ShadowMesh, this, nullptr, Tint);
}
