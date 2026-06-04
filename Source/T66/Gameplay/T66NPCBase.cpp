// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66NPCBase.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66PlayerController.h"
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
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AT66NPCBase::AT66NPCBase()
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
	SafeZoneVisual->bReceivesDecals = false;
	SafeZoneVisual->TranslucencySortPriority = 5;
	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		SafeZoneVisual->SetStaticMesh(Sphere);
	}

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cylinder = FT66VisualUtil::GetBasicShapeCylinder())
	{
		VisualMesh->SetStaticMesh(Cylinder);
		// About hero-sized cylinder
		VisualMesh->SetRelativeScale3D(FVector(0.55f, 0.55f, 1.05f));
		FT66VisualUtil::GroundMeshToActorOrigin(VisualMesh);
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

void AT66NPCBase::BeginPlay()
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
	FT66VisualUtil::SnapToGround(this, GetWorld());
	ApplyVisuals();

	SafeZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AT66NPCBase::OnSafeZoneBeginOverlap);
	SafeZoneSphere->OnComponentEndOverlap.AddDynamic(this, &AT66NPCBase::OnSafeZoneEndOverlap);
	if (InteractionSphere)
	{
		InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AT66NPCBase::OnInteractionBeginOverlap);
		InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AT66NPCBase::OnInteractionEndOverlap);
		if (const AT66HeroBase* LocalHero = GetLocalHero())
		{
			LocalHeroInteractionOverlapCount = InteractionSphere->IsOverlappingActor(LocalHero) ? 1 : 0;
		}
	}

	// Apply imported character mesh if available (data-driven).
	bUsingCharacterVisual = false;
	if (ShouldApplyCharacterVisual())
	{
		if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
			{
				bUsingCharacterVisual = Visuals->ApplyCharacterVisual(NPCID, SkeletalMesh, VisualMesh, true, false, false, VisualMesh);
				if (!bUsingCharacterVisual && SkeletalMesh)
				{
					SkeletalMesh->SetVisibility(false, true);
				}
			}
		}
	}

	FT66VisualUtil::SnapToGround(this, GetWorld());
	ApplyVisuals();
	bGravitySettled = true;
	RefreshInteractionPrompt();

	if (bFacePlayerAlways)
	{
		SetActorTickInterval(0.15f);
	}
	else
	{
		SetActorTickEnabled(false);
	}
}

void AT66NPCBase::LoadFromDataTable()
{
	UWorld* World = GetWorld();
	UGameInstance* GIBase = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* GI = Cast<UT66GameInstance>(GIBase);
	if (!GI || NPCID.IsNone()) return;

	FT66NPCData Data;
	if (GI->GetNPCData(NPCID, Data))
	{
		ApplyNPCData(Data);
	}
}

void AT66NPCBase::ApplyNPCData(const FT66NPCData& Data)
{
	NPCName = Data.DisplayName.IsEmpty() ? FText::FromName(Data.NPCID) : Data.DisplayName;
	NPCColor = Data.NPCColor;
	SafeZoneRadius = Data.SafeZoneRadius;
}

void AT66NPCBase::ApplyVisuals()
{
	UMaterialInterface* ColorMat = FT66VisualUtil::GetFlatColorMaterial();

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
		const float ScaleXY = SafeZoneRadius / 50.f;
		const float ScaleZ = FMath::Max(1.0f, (SafeZoneRadius * 0.32f) / 50.f);
		SafeZoneVisual->SetRelativeScale3D(FVector(ScaleXY, ScaleXY, ScaleZ));
		SafeZoneVisual->SetRelativeLocation(FVector(0.f, 0.f, SafeZoneRadius * 0.12f));

		if (UMaterialInterface* BubbleMaterial = LoadObject<UMaterialInterface>(
			nullptr,
			TEXT("/Game/Stylized_VFX_StPack/Materials/M_Bubble.M_Bubble")))
		{
			SafeZoneVisual->SetMaterial(0, BubbleMaterial);
		}
		else if (ColorMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, this);
			if (Mat)
			{
				FT66VisualUtil::ConfigureFlatColorMaterial(Mat, FLinearColor(NPCColor.R, NPCColor.G, NPCColor.B, 1.f));
				SafeZoneVisual->SetMaterial(0, Mat);
			}
		}
	}
	if (VisualMesh && !bPreserveVisualMeshMaterials)
	{
		if (ColorMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, this);
			if (Mat)
			{
				FT66VisualUtil::ConfigureFlatColorMaterial(Mat, NPCColor);
				VisualMesh->SetMaterial(0, Mat);
			}
		}
	}
}

bool AT66NPCBase::Interact(APlayerController* PC)
{
	// Base NPC does nothing.
	return false;
}

void AT66NPCBase::RefreshInteractionPrompt()
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !GetWorld())
	{
		HideInteractionPrompt();
		return;
	}

	const AT66HeroBase* LocalHero = GetLocalHero();
	if (!LocalHero || LocalHeroInteractionOverlapCount <= 0 || NPCName.IsEmpty())
	{
		HideInteractionPrompt();
		return;
	}

	if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		T66PC->ShowInteractionPrompt(this, NPCName);
	}
}

void AT66NPCBase::HideInteractionPrompt()
{
	if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		T66PC->HideInteractionPrompt(this);
	}
}

bool AT66NPCBase::IsLocalHeroActor(const AActor* OtherActor) const
{
	return OtherActor && GetWorld() && OtherActor == UGameplayStatics::GetPlayerPawn(this, 0);
}

const AT66HeroBase* AT66NPCBase::GetLocalHero() const
{
	return GetWorld() ? Cast<AT66HeroBase>(UGameplayStatics::GetPlayerPawn(this, 0)) : nullptr;
}

float AT66NPCBase::GetFeetOffset() const
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

void AT66NPCBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideInteractionPrompt();

	// [GOLD] Unregister from the actor registry.
	if (UWorld* W = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = W->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->UnregisterNPC(this);
		}
	}

	if (InteractionSphere)
	{
		InteractionSphere->OnComponentBeginOverlap.RemoveDynamic(this, &AT66NPCBase::OnInteractionBeginOverlap);
		InteractionSphere->OnComponentEndOverlap.RemoveDynamic(this, &AT66NPCBase::OnInteractionEndOverlap);
	}
	if (SafeZoneSphere)
	{
		SafeZoneSphere->OnComponentBeginOverlap.RemoveDynamic(this, &AT66NPCBase::OnSafeZoneBeginOverlap);
		SafeZoneSphere->OnComponentEndOverlap.RemoveDynamic(this, &AT66NPCBase::OnSafeZoneEndOverlap);
	}

	Super::EndPlay(EndPlayReason);
}

void AT66NPCBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const bool bWasGravitySettled = bGravitySettled;

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
			FLagScopedScope LagScope(GetWorld(), TEXT("NPCBase::Tick (LineTrace gravity settle)"), 2.0f);
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

	if (!bWasGravitySettled && bGravitySettled)
	{
		if (bFacePlayerAlways)
		{
			SetActorTickInterval(0.15f);
		}
		else
		{
			SetActorTickEnabled(false);
			return;
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

void AT66NPCBase::OnSafeZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero) return;

	HeroOverlapCount++;
	Hero->AddSafeZoneOverlap(+1);
}

void AT66NPCBase::OnSafeZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero) return;

	HeroOverlapCount = FMath::Max(0, HeroOverlapCount - 1);
	Hero->AddSafeZoneOverlap(-1);
}

void AT66NPCBase::OnInteractionBeginOverlap(
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

	if (!IsLocalHeroActor(OtherActor))
	{
		return;
	}

	++LocalHeroInteractionOverlapCount;
	RefreshInteractionPrompt();
}

void AT66NPCBase::OnInteractionEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;

	if (!IsLocalHeroActor(OtherActor))
	{
		return;
	}

	LocalHeroInteractionOverlapCount = FMath::Max(0, LocalHeroInteractionOverlapCount - 1);
	RefreshInteractionPrompt();
}


