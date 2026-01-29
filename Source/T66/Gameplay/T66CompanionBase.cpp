// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CompanionBase.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"

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
	SetActorLocation(NewLoc);

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
}
