// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HeroBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AT66HeroBase::AT66HeroBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create placeholder mesh component
	PlaceholderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderMesh"));
	PlaceholderMesh->SetupAttachment(RootComponent);
	PlaceholderMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	// Use engine's built-in cylinder mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		PlaceholderMesh->SetStaticMesh(CylinderMesh.Object);
		// Scale to reasonable character size (cylinder is 100 units tall by default)
		PlaceholderMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.0f));
	}

	// Adjust capsule for character
	GetCapsuleComponent()->SetCapsuleHalfHeight(88.f);
	GetCapsuleComponent()->SetCapsuleRadius(34.f);

	// Basic movement settings
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = 600.f;
		Movement->BrakingDecelerationWalking = 2048.f;
	}
}

void AT66HeroBase::BeginPlay()
{
	Super::BeginPlay();

	// Create dynamic material for color changes
	if (PlaceholderMesh && PlaceholderMesh->GetMaterial(0))
	{
		PlaceholderMaterial = PlaceholderMesh->CreateAndSetMaterialInstanceDynamic(0);
	}
}

void AT66HeroBase::InitializeHero(const FHeroData& InHeroData)
{
	HeroID = InHeroData.HeroID;
	HeroData = InHeroData;

	// Apply the placeholder color from hero data
	SetPlaceholderColor(InHeroData.PlaceholderColor);

	UE_LOG(LogTemp, Log, TEXT("Hero initialized: %s"), *InHeroData.DisplayName.ToString());
}

void AT66HeroBase::SetPlaceholderColor(FLinearColor Color)
{
	if (PlaceholderMaterial)
	{
		PlaceholderMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
	}
	else if (PlaceholderMesh)
	{
		// Material not yet created (BeginPlay hasn't run), create it now
		PlaceholderMaterial = PlaceholderMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (PlaceholderMaterial)
		{
			PlaceholderMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
		}
	}
}
