// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66HeroBase.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Data/T66DataTypes.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"

AT66HouseNPCBase::AT66HouseNPCBase()
{
	PrimaryActorTick.bCanEverTick = false;

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetSphereRadius(150.f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionObjectType(ECC_Pawn);
	InteractionSphere->SetGenerateOverlapEvents(true);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = InteractionSphere;

	SafeZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SafeZoneSphere"));
	SafeZoneSphere->SetSphereRadius(SafeZoneRadius);
	SafeZoneSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SafeZoneSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	SafeZoneSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SafeZoneSphere->SetupAttachment(RootComponent);

	SafeZoneVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SafeZoneVisual"));
	SafeZoneVisual->SetupAttachment(RootComponent);
	SafeZoneVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SafeZoneVisual->SetCastShadow(false);
	// Use a flat disc on the ground to visualize the safe zone (avoids huge "bubble sphere" confusion).
	if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
	{
		SafeZoneVisual->SetStaticMesh(Cylinder);
	}

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
	{
		VisualMesh->SetStaticMesh(Cylinder);
		// About hero-sized cylinder
		VisualMesh->SetRelativeScale3D(FVector(0.55f, 0.55f, 1.05f));
	}

	NameText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NameText"));
	NameText->SetupAttachment(RootComponent);
	NameText->SetHorizontalAlignment(EHTA_Center);
	NameText->SetVerticalAlignment(EVRTA_TextCenter);
	NameText->SetWorldSize(40.f);
	NameText->SetRelativeLocation(FVector(0.f, 0.f, 250.f));
	NameText->SetTextRenderColor(FColor::White);

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(RootComponent);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMesh->SetVisibility(false, true);
}

void AT66HouseNPCBase::BeginPlay()
{
	Super::BeginPlay();

	LoadFromDataTable();
	ApplyVisuals();

	// Snap NPC cylinder to the ground so it doesn't float/sink.
	// NOTE: We purposely use VisualMesh bounds (not full actor bounds) because SafeZoneVisual is pinned to Z=5.
	if (UWorld* World = GetWorld())
	{
		FHitResult Hit;
		const FVector Here = GetActorLocation();
		const FVector Start = Here + FVector(0.f, 0.f, 2000.f);
		const FVector End = Here - FVector(0.f, 0.f, 6000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
		{
			const float HalfHeight = (VisualMesh ? VisualMesh->Bounds.BoxExtent.Z : 52.5f);
			SetActorLocation(Hit.ImpactPoint + FVector(0.f, 0.f, HalfHeight), false, nullptr, ETeleportType::TeleportPhysics);
			// Re-apply so SafeZoneVisual's world Z=5 is updated after the move.
			ApplyVisuals();
		}
	}

	SafeZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AT66HouseNPCBase::OnSafeZoneBeginOverlap);
	SafeZoneSphere->OnComponentEndOverlap.AddDynamic(this, &AT66HouseNPCBase::OnSafeZoneEndOverlap);

	// Apply imported character mesh if available (data-driven).
	bUsingCharacterVisual = false;
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
		{
			bUsingCharacterVisual = Visuals->ApplyCharacterVisual(NPCID, SkeletalMesh, VisualMesh, true);
			if (!bUsingCharacterVisual && SkeletalMesh)
			{
				SkeletalMesh->SetVisibility(false, true);
			}
		}
	}

	// If we're using a skeletal mesh, treat actor origin as "ground contact" and re-snap to ground.
	if (bUsingCharacterVisual)
	{
		if (UWorld* World = GetWorld())
		{
			FHitResult Hit;
			const FVector Here = GetActorLocation();
			const FVector Start = Here + FVector(0.f, 0.f, 2000.f);
			const FVector End = Here - FVector(0.f, 0.f, 6000.f);
			if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
			{
				SetActorLocation(Hit.ImpactPoint, false, nullptr, ETeleportType::TeleportPhysics);
				ApplyVisuals();
			}
		}
	}
}

void AT66HouseNPCBase::LoadFromDataTable()
{
	UT66GameInstance* GI = GetWorld() ? Cast<UT66GameInstance>(GetWorld()->GetGameInstance()) : nullptr;
	if (!GI || NPCID.IsNone()) return;

	FHouseNPCData Data;
	if (GI->GetHouseNPCData(NPCID, Data))
	{
		ApplyNPCData(Data);
	}
}

void AT66HouseNPCBase::ApplyNPCData(const FHouseNPCData& Data)
{
	NPCName = Data.DisplayName.IsEmpty() ? FText::FromName(Data.NPCID) : Data.DisplayName;
	NPCColor = Data.NPCColor;
	SafeZoneRadius = Data.SafeZoneRadius;
}

void AT66HouseNPCBase::ApplyVisuals()
{
	UMaterialInterface* ColorMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_PlaceholderColor.M_PlaceholderColor"));

	if (NameText)
	{
		NameText->SetText(NPCName);
	}
	if (SafeZoneSphere)
	{
		SafeZoneSphere->SetSphereRadius(SafeZoneRadius);
	}
	if (SafeZoneVisual)
	{
		const float ScaleXY = SafeZoneRadius / 50.f; // cylinder radius ~50
		SafeZoneVisual->SetRelativeScale3D(FVector(ScaleXY, ScaleXY, 0.03f)); // thin sheet
		// Put the disc at ground-ish (world Z ~ 5) so it's always visible even if NPC is spawned above ground.
		SafeZoneVisual->SetWorldLocation(FVector(GetActorLocation().X, GetActorLocation().Y, 5.f));

		if (ColorMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, this);
			if (Mat)
			{
				Mat->SetVectorParameterValue(FName("Color"), FLinearColor(NPCColor.R, NPCColor.G, NPCColor.B, 1.f));
				SafeZoneVisual->SetMaterial(0, Mat);
			}
		}
	}
	if (VisualMesh)
	{
		if (ColorMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, this);
			if (Mat)
			{
				Mat->SetVectorParameterValue(FName("Color"), NPCColor);
				VisualMesh->SetMaterial(0, Mat);
			}
		}
	}
}

bool AT66HouseNPCBase::Interact(APlayerController* PC)
{
	// Base NPC does nothing.
	return false;
}

void AT66HouseNPCBase::OnSafeZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero) return;

	HeroOverlapCount++;
	Hero->AddSafeZoneOverlap(+1);
}

void AT66HouseNPCBase::OnSafeZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero) return;

	HeroOverlapCount = FMath::Max(0, HeroOverlapCount - 1);
	Hero->AddSafeZoneOverlap(-1);
}

