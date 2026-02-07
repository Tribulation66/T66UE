// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66CombatComponent.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66HeroSpeedSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"

AT66HeroBase::AT66HeroBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// ========== Capsule Setup ==========
	GetCapsuleComponent()->SetCapsuleHalfHeight(88.f);
	GetCapsuleComponent()->SetCapsuleRadius(34.f);

	// ========== Camera Setup (Third-Person with Mouse Look) ==========
	
	// Create spring arm (camera boom)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // Distance behind the character
	CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 60.f)); // Offset up from center
	CameraBoom->bUsePawnControlRotation = true; // Rotate arm based on controller (mouse look)
	CameraBoom->bDoCollisionTest = true; // Pull camera in when hitting walls
	CameraBoom->ProbeSize = 12.f;
	CameraBoom->ProbeChannel = ECC_Camera;
	
	// Create follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // Camera doesn't rotate independently

	// ========== Placeholder Mesh Setup ==========
	
	PlaceholderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderMesh"));
	PlaceholderMesh->SetupAttachment(RootComponent);
	// Align primitive mesh to ground when capsule is grounded:
	// capsule half-height=88, cube/cylinder half-height=50 => relative Z = 50 - 88 = -38.
	PlaceholderMesh->SetRelativeLocation(FVector(0.f, 0.f, -38.f));
	PlaceholderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Capsule handles collision

	// Cache mesh assets in constructor
	CacheMeshAssets();
	
	// Default to cylinder (TypeA)
	if (CylinderMesh)
	{
		PlaceholderMesh->SetStaticMesh(CylinderMesh);
		// Scale to reasonable character size (cylinder is 100 units tall by default)
		PlaceholderMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.0f));
	}

	// ========== Combat range ring (visual) ==========
	AttackRangeRingISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("AttackRangeRingISM"));
	AttackRangeRingISM->SetupAttachment(RootComponent);
	AttackRangeRingISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackRangeRingISM->SetGenerateOverlapEvents(false);
	AttackRangeRingISM->CastShadow = false;
	AttackRangeRingISM->bReceivesDecals = false;
	AttackRangeRingISM->SetHiddenInGame(true, true);
	AttackRangeRingISM->SetVisibility(false, true);

	// Use simple cube segments to avoid a filled disk that "whites out" the floor.
	if (CubeMesh)
	{
		AttackRangeRingISM->SetStaticMesh(CubeMesh);
	}
	else if (CylinderMesh)
	{
		AttackRangeRingISM->SetStaticMesh(CylinderMesh);
	}

	// Wireframe reads best for a "range ring"; engine debug material path can vary, so fallback to project material.
	UMaterialInterface* RingMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineDebugMaterials/M_Wireframe.M_Wireframe"));
	if (!RingMat)
	{
		RingMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_PlaceholderColor.M_PlaceholderColor"));
	}
	if (!RingMat)
	{
		RingMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}
	if (RingMat)
	{
		AttackRangeRingISM->SetMaterial(0, RingMat);
	}

	// Sit on the ground plane under the capsule.
	AttackRangeRingISM->SetRelativeLocation(FVector(0.f, 0.f, -GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() + 2.f));
	AttackRangeRingISM->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));

	// ========== Character Movement Setup ==========
	
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = 1200.f;
		Movement->BrakingDecelerationWalking = 2048.f;
		Movement->JumpZVelocity = 500.f;
		Movement->AirControl = 0.35f;
		Movement->GravityScale = 1.5f;
		
		// Don't rotate character to match controller - only camera rotates
		Movement->bOrientRotationToMovement = true; // Character rotates to face movement direction
		Movement->RotationRate = FRotator(0.f, 540.f, 0.f);
	}

	// Double jump
	JumpMaxCount = 2;

	// Don't rotate pawn with controller (controller only rotates camera)
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CombatComponent = CreateDefaultSubobject<UT66CombatComponent>(TEXT("CombatComponent"));

	// Prepare built-in SkeletalMeshComponent for imported models.
	if (USkeletalMeshComponent* Skel = GetMesh())
	{
		Skel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Skel->SetVisibility(false, true); // shown only when a character visual mapping exists
	}
}

void AT66HeroBase::AddSafeZoneOverlap(int32 Delta)
{
	SafeZoneOverlapCount = FMath::Max(0, SafeZoneOverlapCount + Delta);
}

void AT66HeroBase::CacheMeshAssets()
{
	// Cache cylinder mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderFinder.Succeeded())
	{
		CylinderMesh = CylinderFinder.Object;
	}

	// Cache cube mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		CubeMesh = CubeFinder.Object;
	}
}

void AT66HeroBase::BeginPlay()
{
	Super::BeginPlay();

	// Cache baseline friction values so stage effects can temporarily override safely.
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		BaseGroundFriction = Movement->GroundFriction;
		BaseBrakingFrictionFactor = Movement->BrakingFrictionFactor;
		BaseBrakingDecelerationWalking = Movement->BrakingDecelerationWalking;
	}

	// Foundation: cosmetic toggles (e.g. Gooner Mode) live in PlayerSettings.
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			if (PS->GetGoonerMode())
			{
				// TODO: When real character meshes exist, apply barefoot cosmetic set here.
				UE_LOG(LogTemp, Log, TEXT("Gooner Mode enabled (placeholder; no meshes to modify yet)."));
			}
		}
	}

	// Create dynamic material for color changes
	if (PlaceholderMesh && PlaceholderMesh->GetMaterial(0))
	{
		PlaceholderMaterial = PlaceholderMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	// Touch damage: overlap with enemy applies 1 heart (RunState handles i-frames)
	if (GetCapsuleComponent() && !bIsPreviewMode)
	{
		GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AT66HeroBase::OnCapsuleBeginOverlap);
	}

	// Debug: Log camera status
	UE_LOG(LogTemp, Log, TEXT("HeroBase BeginPlay - Location: (%.1f, %.1f, %.1f)"),
		GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);
	
	if (FollowCamera)
	{
		UE_LOG(LogTemp, Log, TEXT("HeroBase - FollowCamera is active: %s"), 
			FollowCamera->IsActive() ? TEXT("YES") : TEXT("NO"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HeroBase - FollowCamera is NULL!"));
	}

	// Cache run state for derived stats (level + last-stand).
	CachedRunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (CachedRunState)
	{
		CachedRunState->HeroProgressChanged.AddDynamic(this, &AT66HeroBase::HandleHeroDerivedStatsChanged);
		CachedRunState->SurvivalChanged.AddDynamic(this, &AT66HeroBase::HandleHeroDerivedStatsChanged);
		CachedRunState->InventoryChanged.AddDynamic(this, &AT66HeroBase::HandleHeroDerivedStatsChanged);
		CachedRunState->PanelVisibilityChanged.AddDynamic(this, &AT66HeroBase::HandleHUDPanelVisibilityChanged);
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		BaseMaxWalkSpeed = Movement->MaxWalkSpeed;
	}
	HandleHeroDerivedStatsChanged();
	HandleHUDPanelVisibilityChanged();

	// Ensure ring updates after components (CombatComponent) finish BeginPlay.
	if (UWorld* World = GetWorld())
	{
		FTimerHandle Tmp;
		World->GetTimerManager().SetTimer(Tmp, this, &AT66HeroBase::UpdateAttackRangeRing, 0.15f, false);
	}
}

void AT66HeroBase::HandleHeroDerivedStatsChanged()
{
	if (!CachedRunState) return;
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement) return;

	const float Mult = CachedRunState->GetHeroMoveSpeedMultiplier()
		* CachedRunState->GetItemMoveSpeedMultiplier()
		* CachedRunState->GetLastStandMoveSpeedMultiplier()
		* CachedRunState->GetStageMoveSpeedMultiplier()
		* CachedRunState->GetStatusMoveSpeedMultiplier();
	Movement->MaxWalkSpeed = FMath::Clamp(BaseMaxWalkSpeed * Mult, 200.f, 10000.f);

	UpdateAttackRangeRing();
}

void AT66HeroBase::HandleHUDPanelVisibilityChanged()
{
	UpdateAttackRangeRing();
}

void AT66HeroBase::RefreshAttackRangeRing()
{
	UpdateAttackRangeRing();
}

void AT66HeroBase::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsPreviewMode || !OtherActor) return;
	if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(OtherActor))
	{
		UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
		if (UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->ApplyDamage(1);
		}
	}
}

void AT66HeroBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Stage slide: temporary low-friction movement.
	if (StageSlideSecondsRemaining > 0.f)
	{
		StageSlideSecondsRemaining = FMath::Max(0.f, StageSlideSecondsRemaining - DeltaSeconds);
		if (StageSlideSecondsRemaining <= 0.f)
		{
			if (UCharacterMovementComponent* Movement = GetCharacterMovement())
			{
				Movement->GroundFriction = BaseGroundFriction;
				Movement->BrakingFrictionFactor = BaseBrakingFrictionFactor;
				Movement->BrakingDecelerationWalking = BaseBrakingDecelerationWalking;
			}
		}
	}

	// Hero speed: ramp from input (10% of max per second); drive MaxWalkSpeed and animation state.
	if (!bIsPreviewMode)
	{
		if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			if (UT66HeroSpeedSubsystem* SpeedSub = GI->GetSubsystem<UT66HeroSpeedSubsystem>())
			{
				// Movement input: use last movement input (set by controller when it calls AddMovementInput).
				const FVector LastInput = GetLastMovementInputVector();
				const bool bHasMovementInput = LastInput.SizeSquared() > 0.01f;
				SpeedSub->Update(DeltaSeconds, bHasMovementInput);
				if (UCharacterMovementComponent* Movement = GetCharacterMovement())
				{
					Movement->MaxWalkSpeed = SpeedSub->GetCurrentSpeed();
				}
				// Animation: two states only — Alert (idle) and Run (any movement). Companion uses same state.
				if (GetMesh() && GetMesh()->IsVisible() && (CachedAlertAnim || CachedRunAnim || CachedWalkAnim))
				{
					const int32 NewState = SpeedSub->GetMovementAnimState(); // 0=Idle, 2=Run
					if (static_cast<int32>(LastMovementAnimState) != NewState)
					{
						LastMovementAnimState = static_cast<EMovementAnimState>(NewState);
						UAnimationAsset* ToPlay = nullptr;
						if (NewState == 0)
							ToPlay = CachedAlertAnim;
						else
							ToPlay = CachedRunAnim ? CachedRunAnim : CachedWalkAnim;
						if (ToPlay)
							GetMesh()->PlayAnimation(ToPlay, true);
					}
				}
			}
		}
	}
}

void AT66HeroBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedRunState)
	{
		CachedRunState->HeroProgressChanged.RemoveDynamic(this, &AT66HeroBase::HandleHeroDerivedStatsChanged);
		CachedRunState->SurvivalChanged.RemoveDynamic(this, &AT66HeroBase::HandleHeroDerivedStatsChanged);
		CachedRunState->InventoryChanged.RemoveDynamic(this, &AT66HeroBase::HandleHeroDerivedStatsChanged);
		CachedRunState->PanelVisibilityChanged.RemoveDynamic(this, &AT66HeroBase::HandleHUDPanelVisibilityChanged);
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->OnComponentBeginOverlap.RemoveDynamic(this, &AT66HeroBase::OnCapsuleBeginOverlap);
	}

	Super::EndPlay(EndPlayReason);
}

void AT66HeroBase::UpdateAttackRangeRing()
{
	if (!AttackRangeRingISM)
	{
		return;
	}

	// Hide in preview mode.
	if (bIsPreviewMode)
	{
		AttackRangeRingISM->SetHiddenInGame(true, true);
		AttackRangeRingISM->SetVisibility(false, true);
		return;
	}

	const bool bPanelsVisible = CachedRunState ? CachedRunState->GetHUDPanelsVisible() : true;
	if (!bPanelsVisible)
	{
		AttackRangeRingISM->SetHiddenInGame(true, true);
		AttackRangeRingISM->SetVisibility(false, true);
		return;
	}

	const float Range = (CombatComponent ? CombatComponent->AttackRange : 0.f);
	const float ClampedRange = FMath::Clamp(Range, 200.f, 50000.f);

	// Build/update a segmented ring once per range change (instanced segments, not a filled disc).
	AttackRangeRingISM->ClearInstances();

	// Make this read like a thin circle (not blocky chunks).
	static constexpr int32 NumSeg = 192;
	const float Circ = 2.f * PI * ClampedRange;
	const float SegLen = FMath::Clamp((Circ / static_cast<float>(NumSeg)) * 1.10f, 8.f, 80.f); // slight overlap
	const float SegThick = 8.f;   // thin line thickness
	const float SegHeight = 2.5f; // thin line height

	for (int32 i = 0; i < NumSeg; ++i)
	{
		const float T = static_cast<float>(i) / static_cast<float>(NumSeg);
		const float A = 2.f * PI * T;
		const float X = FMath::Cos(A) * ClampedRange;
		const float Y = FMath::Sin(A) * ClampedRange;

		// Tangent-aligned segment so it reads like a continuous ring.
		const float YawDeg = FMath::RadiansToDegrees(A) + 90.f;
		const FVector Loc(X, Y, 3.f);
		const FRotator Rot(0.f, YawDeg, 0.f);

		// /Engine/BasicShapes/Cube: 100uu. Scale to desired world size.
		const FVector Scale(SegLen / 100.f, SegThick / 100.f, SegHeight / 100.f);
		const FTransform Xf(Rot, Loc, Scale);
		AttackRangeRingISM->AddInstance(Xf, false);
	}
	AttackRangeRingISM->MarkRenderStateDirty();

	AttackRangeRingISM->SetHiddenInGame(false, true);
	AttackRangeRingISM->SetVisibility(true, true);
}

void AT66HeroBase::ApplyStageSlide(float DurationSeconds)
{
	if (DurationSeconds <= 0.f) return;
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		// Very low braking so the hero "slides" along the ground.
		Movement->GroundFriction = 0.15f;
		Movement->BrakingFrictionFactor = 0.05f;
		Movement->BrakingDecelerationWalking = 128.f;
	}
	StageSlideSecondsRemaining = FMath::Max(StageSlideSecondsRemaining, DurationSeconds);
}

void AT66HeroBase::InitializeHero(const FHeroData& InHeroData, ET66BodyType InBodyType, FName InSkinID, bool bPreviewMode)
{
	HeroID = InHeroData.HeroID;
	HeroData = InHeroData;
	BodyType = InBodyType;

	// Set the body type mesh (Cylinder or Cube)
	SetBodyType(InBodyType);

	// Apply the placeholder color from hero data
	SetPlaceholderColor(InHeroData.PlaceholderColor);

	FName SkinID = InSkinID.IsNone() ? FName(TEXT("Default")) : InSkinID;

	UE_LOG(LogTemp, Log, TEXT("Hero initialized: %s, BodyType: %s, Skin: %s, Color: (%.2f, %.2f, %.2f)"),
		*InHeroData.DisplayName.ToString(),
		InBodyType == ET66BodyType::TypeA ? TEXT("TypeA") : TEXT("TypeB"),
		*SkinID.ToString(),
		InHeroData.PlaceholderColor.R,
		InHeroData.PlaceholderColor.G,
		InHeroData.PlaceholderColor.B);

	// Resolve hero visual by HeroID + BodyType + SkinID (e.g. Hero_1_TypeA, Hero_1_TypeB_Beachgoer).
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
		{
			const FName VisualID = UT66CharacterVisualSubsystem::GetHeroVisualID(HeroID, InBodyType, SkinID);
			// Hero selection preview uses alert animation so the character is non-static in the showcase.
			const bool bUseAlertAnimation = bPreviewMode;
			UE_LOG(LogTemp, Log, TEXT("[ANIM] HeroBase::InitializeHero HeroID=%s BodyType=%s SkinID=%s bPreviewMode=%d bUseAlertAnimation=%d VisualID=%s"),
				*HeroID.ToString(), InBodyType == ET66BodyType::TypeA ? TEXT("A") : TEXT("B"), *SkinID.ToString(),
				bPreviewMode ? 1 : 0, bUseAlertAnimation ? 1 : 0, *VisualID.ToString());
			const bool bApplied = Visuals->ApplyCharacterVisual(VisualID, GetMesh(), PlaceholderMesh, true, bUseAlertAnimation, bPreviewMode);
			if (!bApplied)
			{
				if (GetMesh())
				{
					GetMesh()->SetVisibility(false, true);
				}
				if (PlaceholderMesh)
				{
					PlaceholderMesh->SetVisibility(true, true);
				}
			}
			else if (!bPreviewMode)
			{
				// Cache alert/walk/run anims and init hero speed params (subsystem drives speed + animation state).
				UAnimationAsset* WalkRaw = nullptr;
				UAnimationAsset* RunRaw = nullptr;
				UAnimationAsset* AlertRaw = nullptr;
				Visuals->GetMovementAnimsForVisual(VisualID, WalkRaw, RunRaw, AlertRaw);
				CachedWalkAnim = WalkRaw;
				CachedRunAnim = RunRaw;
				CachedAlertAnim = AlertRaw;
				// Force first Tick to play alert (speed 0); if we left Idle we wouldn't call PlayAnimation.
				LastMovementAnimState = EMovementAnimState::Walk;
				if (UT66HeroSpeedSubsystem* SpeedSub = GI->GetSubsystem<UT66HeroSpeedSubsystem>())
				{
					SpeedSub->SetParams(InHeroData.MaxSpeed, InHeroData.AccelerationPercentPerSecond);
				}
			}
		}
	}
}

void AT66HeroBase::SetPlaceholderColor(FLinearColor Color)
{
	if (!PlaceholderMesh) return;

	// Store the color for later use (e.g., after mesh changes)
	CurrentPlaceholderColor = Color;

	// Create a dynamic material and set its color override
	// The approach: use SetVectorParameterValue which internally creates the parameter if needed
	
	// First, try to load our custom placeholder material
	UMaterialInterface* ColorMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_PlaceholderColor.M_PlaceholderColor"));
	
	if (!ColorMaterial)
	{
		// Use the mesh's current material as base
		ColorMaterial = PlaceholderMesh->GetMaterial(0);
	}

	if (ColorMaterial)
	{
		// Create a dynamic material instance
		PlaceholderMaterial = UMaterialInstanceDynamic::Create(ColorMaterial, this);
		
		if (PlaceholderMaterial)
		{
			// Apply the material first
			PlaceholderMesh->SetMaterial(0, PlaceholderMaterial);
			
			// Set color using the parameter name that BasicShapeMaterial uses
			// BasicShapeMaterial in UE5 uses "Color" as the parameter name
			PlaceholderMaterial->SetVectorParameterValue(FName("Color"), Color);
			
			UE_LOG(LogTemp, Log, TEXT("SetPlaceholderColor: Material applied with color (%.2f, %.2f, %.2f)"), 
				Color.R, Color.G, Color.B);
		}
	}
	
	// Additional approach: Set the mesh's custom primitive data (works with materials that read it)
	// This is a fallback that some materials use
	if (PlaceholderMesh)
	{
		PlaceholderMesh->SetDefaultCustomPrimitiveDataFloat(0, Color.R);
		PlaceholderMesh->SetDefaultCustomPrimitiveDataFloat(1, Color.G);
		PlaceholderMesh->SetDefaultCustomPrimitiveDataFloat(2, Color.B);
	}
}

void AT66HeroBase::SetBodyType(ET66BodyType NewBodyType)
{
	BodyType = NewBodyType;

	if (!PlaceholderMesh) return;

	UStaticMesh* TargetMesh = nullptr;
	FVector TargetScale = FVector(1.f);

	if (NewBodyType == ET66BodyType::TypeA)
	{
		// Type A = Cylinder
		TargetMesh = CylinderMesh;
		TargetScale = FVector(0.5f, 0.5f, 1.0f); // Cylinder scaled for character proportions
	}
	else
	{
		// Type B = Cube
		TargetMesh = CubeMesh;
		TargetScale = FVector(0.5f, 0.5f, 1.5f); // Cube scaled taller for character proportions
	}

	if (TargetMesh)
	{
		PlaceholderMesh->SetStaticMesh(TargetMesh);
		PlaceholderMesh->SetRelativeScale3D(TargetScale);
		
		// Re-create material instance for new mesh
		if (PlaceholderMesh->GetMaterial(0))
		{
			PlaceholderMaterial = PlaceholderMesh->CreateAndSetMaterialInstanceDynamic(0);
			// Re-apply color if we have hero data
			if (!HeroData.HeroID.IsNone())
			{
				SetPlaceholderColor(HeroData.PlaceholderColor);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Hero body type set to: %s"),
		NewBodyType == ET66BodyType::TypeA ? TEXT("TypeA (Cylinder)") : TEXT("TypeB (Cube)"));
}

void AT66HeroBase::SetPreviewMode(bool bPreview)
{
	bIsPreviewMode = bPreview;

	if (bPreview)
	{
		// Disable camera in preview mode
		if (FollowCamera)
		{
			FollowCamera->SetActive(false);
		}
		
		// Disable movement
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->DisableMovement();
		}
	}
	else
	{
		// Enable camera
		if (FollowCamera)
		{
			FollowCamera->SetActive(true);
		}
		
		// Enable movement
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->SetMovementMode(MOVE_Walking);
		}
	}
}

void AT66HeroBase::DashForward()
{
	if (bIsPreviewMode) return;
	UWorld* World = GetWorld();
	if (!World) return;

	const float Now = static_cast<float>(World->GetTimeSeconds());
	float Cooldown = DashCooldownSeconds;
	if (UT66RunStateSubsystem* RunState = World->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>())
	{
		Cooldown *= RunState->GetDashCooldownMultiplier();
	}
	Cooldown = FMath::Clamp(Cooldown, 0.05f, 10.f);
	if (Now - LastDashTime < Cooldown) return;
	LastDashTime = Now;

	// Dash in facing direction.
	FVector Dir = GetActorForwardVector();
	Dir.Z = 0.f;
	if (!Dir.Normalize())
	{
		Dir = FVector(1.f, 0.f, 0.f);
	}

	LaunchCharacter(Dir * DashStrength, true, true);
}
