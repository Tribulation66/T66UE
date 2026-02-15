// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66HeroBase.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66GameInstance.h"
#include "Data/T66DataTypes.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Gameplay/T66VisualUtil.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AT66HouseNPCBase::AT66HouseNPCBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f; // cheap "turn-to-face" update

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
	if (UStaticMesh* Cylinder = FT66VisualUtil::GetBasicShapeCylinder())
	{
		SafeZoneVisual->SetStaticMesh(Cylinder);
	}

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cylinder = FT66VisualUtil::GetBasicShapeCylinder())
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

	// [GOLD] Register with the actor registry (replaces TActorIterator world scans for NPCs).
	if (UWorld* W = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = W->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->RegisterNPC(this);
		}
	}

	LoadFromDataTable();
	ApplyVisuals();

	// Snap NPC cylinder to the ground so it doesn't float/sink. Ignore self so the trace hits terrain, not our own collision.
	// NOTE: We purposely use VisualMesh bounds (not full actor bounds) because SafeZoneVisual is pinned to Z=5.
	if (UWorld* World = GetWorld())
	{
		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(T66HouseNPCGroundSnap), false, this);
		const FVector Here = GetActorLocation();
		const FVector Start = Here + FVector(0.f, 0.f, 2000.f);
		const FVector End = Here - FVector(0.f, 0.f, 6000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
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

	// If we're using a skeletal mesh, re-snap so the character's feet are on the ground. Skeleton origin is usually at pelvis/center, so we must raise by the mesh bottom offset.
	if (bUsingCharacterVisual)
	{
		if (UWorld* World = GetWorld())
		{
			FHitResult Hit;
			FCollisionQueryParams Params(SCENE_QUERY_STAT(T66HouseNPCGroundSnapSkeletal), false, this);
			const FVector Here = GetActorLocation();
			const FVector Start = Here + FVector(0.f, 0.f, 2000.f);
			const FVector End = Here - FVector(0.f, 0.f, 6000.f);
			if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
			{
				// Raise so the bottom of the character (not the actor origin) is on the ground. Use skeletal mesh bounds half-height or a safe default.
				float GroundOffset = 95.f;
				if (SkeletalMesh && SkeletalMesh->Bounds.SphereRadius > 1.f)
				{
					GroundOffset = SkeletalMesh->Bounds.BoxExtent.Z;
				}
				const FVector Normal = Hit.ImpactNormal.GetSafeNormal(1e-4f, FVector::UpVector);
				SetActorLocation(Hit.ImpactPoint + Normal * GroundOffset, false, nullptr, ETeleportType::TeleportPhysics);
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
	UMaterialInterface* ColorMat = FT66VisualUtil::GetPlaceholderColorMaterial();

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

float AT66HouseNPCBase::GetFeetOffset() const
{
	if (bUsingCharacterVisual && SkeletalMesh && SkeletalMesh->Bounds.SphereRadius > 1.f)
	{
		return SkeletalMesh->Bounds.BoxExtent.Z;
	}
	if (VisualMesh)
	{
		return VisualMesh->Bounds.BoxExtent.Z;
	}
	return 52.5f;
}

void AT66HouseNPCBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// [GOLD] Unregister from the actor registry.
	if (UWorld* W = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = W->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->UnregisterNPC(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void AT66HouseNPCBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Gravity settling: trace down and move toward ground until resting (stops floating).
	// Perf: run trace only every Nth tick to reduce LineTrace cost with many NPCs.
	if (!bGravitySettled && GetWorld())
	{
		const float Age = GetGameTimeSinceCreation();
		if (Age < GravitySettleDuration)
		{
			++GravitySettleTickCounter;
			if (GravitySettleTickCounter % GravitySettleTraceEveryNTicks != 0)
			{
				// Skip trace this tick; will retry next time.
			}
			else
			{
			FLagScopedScope LagScope(GetWorld(), TEXT("HouseNPCBase::Tick (LineTrace gravity settle)"), 2.0f);
			FHitResult Hit;
			FCollisionQueryParams Params(SCENE_QUERY_STAT(T66HouseNPCGravitySettle), false, this);
			const FVector Here = GetActorLocation();
			const FVector Start = Here + FVector(0.f, 0.f, 100.f);
			const FVector End = Here - FVector(0.f, 0.f, 5000.f);
			if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
			{
				const float FeetOffset = GetFeetOffset();
				const float BottomZ = Here.Z - FeetOffset;
				const float GroundZ = Hit.ImpactPoint.Z;
				const float Gap = BottomZ - GroundZ;
				if (Gap > GravitySettleTolerance)
				{
					const float Move = FMath::Min(DeltaSeconds * GravitySettleSpeed, Gap);
					SetActorLocation(GetActorLocation() - FVector(0.f, 0.f, Move), false, nullptr, ETeleportType::TeleportPhysics);
					ApplyVisuals();
				}
				else
				{
					// Snap to exact ground and stop
					SetActorLocation(FVector(Here.X, Here.Y, GroundZ + FeetOffset), false, nullptr, ETeleportType::TeleportPhysics);
					ApplyVisuals();
					bGravitySettled = true;
				}
			}
			}
		}
		else
		{
			bGravitySettled = true;
		}
	}

	if (!bFacePlayerAlways) return;
	if (!GetWorld()) return;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	const FVector Here = GetActorLocation();
	const FVector There = PlayerPawn->GetActorLocation();
	FVector ToPlayer = (There - Here);
	ToPlayer.Z = 0.f;
	if (ToPlayer.IsNearlyZero()) return;
	if (ToPlayer.SizeSquared() > (FacePlayerMaxDistance * FacePlayerMaxDistance)) return;

	const FRotator Current = GetActorRotation();
	FRotator Desired = ToPlayer.Rotation();
	Desired.Pitch = 0.f;
	Desired.Roll = 0.f;

	const FRotator NewRot = FMath::RInterpTo(Current, Desired, DeltaSeconds, FacePlayerYawInterpSpeed);
	SetActorRotation(NewRot);
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

