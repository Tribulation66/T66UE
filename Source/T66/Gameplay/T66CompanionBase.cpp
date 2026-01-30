// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CompanionBase.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Core/T66RunStateSubsystem.h"

AT66CompanionBase::AT66CompanionBase()
{
	PrimaryActorTick.bCanEverTick = true;

	PlaceholderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderMesh"));
	SetRootComponent(PlaceholderMesh);
	PlaceholderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereFinder.Succeeded())
	{
		PlaceholderMesh->SetStaticMesh(SphereFinder.Object);
		PlaceholderMesh->SetRelativeScale3D(FVector(0.4f)); // Sphere for companion size
	}
}

void AT66CompanionBase::BeginPlay()
{
	Super::BeginPlay();
	if (PlaceholderMesh && PlaceholderMesh->GetMaterial(0))
	{
		PlaceholderMaterial = PlaceholderMesh->CreateAndSetMaterialInstanceDynamic(0);
	}
}

void AT66CompanionBase::InitializeCompanion(const FCompanionData& InData)
{
	CompanionID = InData.CompanionID;
	CompanionData = InData;
	SetPlaceholderColor(InData.PlaceholderColor);
}

void AT66CompanionBase::SetPlaceholderColor(FLinearColor Color)
{
	if (!PlaceholderMesh) return;
	UMaterialInterface* ColorMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_PlaceholderColor.M_PlaceholderColor"));
	if (!ColorMaterial) ColorMaterial = PlaceholderMesh->GetMaterial(0);
	if (ColorMaterial)
	{
		PlaceholderMaterial = UMaterialInstanceDynamic::Create(ColorMaterial, this);
		if (PlaceholderMaterial)
		{
			PlaceholderMesh->SetMaterial(0, PlaceholderMaterial);
			PlaceholderMaterial->SetVectorParameterValue(FName("Color"), Color);
		}
	}
}

void AT66CompanionBase::SetPreviewMode(bool bPreview)
{
	bIsPreviewMode = bPreview;
	// Make preview easier to see in UI.
	if (PlaceholderMesh)
	{
		PlaceholderMesh->SetRelativeScale3D(bIsPreviewMode ? FVector(0.85f) : FVector(0.4f));
	}
}

void AT66CompanionBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsPreviewMode) return;

	// Follow the player's pawn (hero)
	UWorld* World = GetWorld();
	if (!World) return;
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;
	APawn* Hero = PC->GetPawn();
	if (!Hero) return;

	// Target: hero location + offset in hero's local space (behind and to the side)
	FRotator HeroYaw = FRotator(0.f, Hero->GetActorRotation().Yaw, 0.f);
	FVector OffsetWorld = HeroYaw.RotateVector(FollowOffset);
	FVector TargetLoc = Hero->GetActorLocation() + OffsetWorld;

	FVector NewLoc = FMath::VInterpTo(GetActorLocation(), TargetLoc, DeltaTime, FollowSpeed);
	// Keep companion grounded (sphere bottom on ground).
	{
		FHitResult Hit;
		const FVector Start = NewLoc + FVector(0.f, 0.f, 2000.f);
		const FVector End = NewLoc - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
		{
			// Sphere base radius: 50 * 0.4 = 20
			static constexpr float CompanionRadius = 20.f;
			NewLoc.Z = Hit.ImpactPoint.Z + CompanionRadius;
		}
	}
	SetActorLocation(NewLoc, false, nullptr, ETeleportType::TeleportPhysics);

	// Face the hero
	FVector ToHero = Hero->GetActorLocation() - GetActorLocation();
	ToHero.Z = 0.f;
	if (ToHero.IsNearlyZero() == false)
	{
		FRotator NewRot = ToHero.Rotation();
		NewRot.Pitch = 0.f;
		NewRot.Roll = 0.f;
		SetActorRotation(NewRot);
	}

	// Heal the hero over time (up to max hearts).
	HealAccumSeconds += DeltaTime;
	if (HealAccumSeconds >= HealIntervalSeconds)
	{
		HealAccumSeconds = 0.f;
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				if (RunState->GetCurrentHearts() < RunState->GetMaxHearts())
				{
					RunState->HealHearts(HealAmountHearts);
				}
			}
		}
	}
}
