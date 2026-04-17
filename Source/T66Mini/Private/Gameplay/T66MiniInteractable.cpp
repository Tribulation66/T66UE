// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiniInteractable.h"

#include "Components/BillboardComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/T66MiniVisualSubsystem.h"
#include "Core/T66MiniVFXSubsystem.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "EngineUtils.h"
#include "Gameplay/Components/T66MiniShadowComponent.h"
#include "Gameplay/T66MiniGameMode.h"
#include "Gameplay/T66MiniPlayerController.h"
#include "Gameplay/T66MiniPlayerPawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"

namespace
{
	UStaticMesh* T66MiniInteractableLoadPlaneMesh()
	{
		static TWeakObjectPtr<UStaticMesh> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
		}

		return Cached.Get();
	}

	UMaterialInterface* T66MiniInteractableLoadArenaMaterial()
	{
		static TWeakObjectPtr<UMaterialInterface> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
		}

		return Cached.Get();
	}

	void T66MiniInteractableConfigureIndicator(UStaticMeshComponent* MeshComponent, UObject* Outer, const FLinearColor& Tint)
	{
		if (!MeshComponent)
		{
			return;
		}

		UMaterialInterface* BaseMaterial = T66MiniInteractableLoadArenaMaterial();
		if (!BaseMaterial)
		{
			return;
		}

		UMaterialInstanceDynamic* Material = MeshComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, BaseMaterial);
		if (!Material)
		{
			Material = UMaterialInstanceDynamic::Create(BaseMaterial, Outer ? Outer : MeshComponent);
			if (!Material)
			{
				return;
			}

			MeshComponent->SetMaterial(0, Material);
		}

		if (UTexture* WhiteTexture = LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture")))
		{
			Material->SetTextureParameterValue(TEXT("DiffuseColorMap"), WhiteTexture);
			Material->SetTextureParameterValue(TEXT("BaseColorTexture"), WhiteTexture);
		}

		Material->SetVectorParameterValue(TEXT("Color"), Tint);
		Material->SetVectorParameterValue(TEXT("BaseColor"), Tint);
		Material->SetVectorParameterValue(TEXT("Tint"), Tint);
	}
}

AT66MiniInteractable::AT66MiniInteractable()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->SetupAttachment(SceneRoot);
	CollisionComponent->InitSphereRadius(90.f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);

	SpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	SpriteComponent->SetupAttachment(SceneRoot);
	SpriteComponent->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	SpriteComponent->SetRelativeScale3D(FVector(2.25f, 2.25f, 2.25f));
	SpriteComponent->SetHiddenInGame(false, true);
	SpriteComponent->SetVisibility(true, true);

	IndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Indicator"));
	IndicatorMesh->SetupAttachment(SceneRoot);
	IndicatorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	IndicatorMesh->SetCastShadow(false);
	IndicatorMesh->SetStaticMesh(T66MiniInteractableLoadPlaneMesh());
	IndicatorMesh->SetRelativeLocation(FVector(0.f, 0.f, 4.f));
	IndicatorMesh->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	IndicatorMesh->SetRelativeScale3D(FVector(0.42f, 0.42f, 1.0f));
	T66MiniInteractableConfigureIndicator(IndicatorMesh, this, FLinearColor(0.22f, 0.74f, 0.96f, 0.40f));

	ShadowComponent = CreateDefaultSubobject<UT66MiniShadowComponent>(TEXT("Shadow"));
	ShadowComponent->InitializeShadow(SceneRoot, FVector(0.f, 0.f, 1.f), FVector(0.24f, 0.24f, 1.f), FLinearColor(0.f, 0.f, 0.f, 0.14f));
}

void AT66MiniInteractable::BeginPlay()
{
	Super::BeginPlay();
	if (AT66MiniGameMode* MiniGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AT66MiniGameMode>() : nullptr)
	{
		MiniGameMode->RegisterLiveInteractable(this);
	}
	if (HasAuthority())
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AT66MiniInteractable::HandleOverlap);
	}
	HoverPhase = FMath::FRandRange(0.f, UE_TWO_PI);
	RefreshVisuals();
}

void AT66MiniInteractable::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AT66MiniGameMode* MiniGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AT66MiniGameMode>() : nullptr)
	{
		MiniGameMode->UnregisterLiveInteractable(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AT66MiniInteractable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT66MiniInteractable, InteractableType);
	DOREPLIFETIME(AT66MiniInteractable, MaterialReward);
	DOREPLIFETIME(AT66MiniInteractable, GoldReward);
	DOREPLIFETIME(AT66MiniInteractable, ExperienceReward);
	DOREPLIFETIME(AT66MiniInteractable, HealAmount);
	DOREPLIFETIME(AT66MiniInteractable, VisualID);
	DOREPLIFETIME(AT66MiniInteractable, bRequiresManualInteract);
}

void AT66MiniInteractable::InitializeInteractable(
	const ET66MiniInteractableType InType,
	UTexture2D* InTexture,
	const float InLifetimeSeconds,
	const FString& InVisualID,
	const int32 InMaterialReward,
	const int32 InGoldReward,
	const float InExperienceReward,
	const float InHealAmount,
	const bool bInRequiresManualInteract)
{
	InteractableType = InType;
	LifetimeRemaining = InLifetimeSeconds;
	VisualID = InVisualID;
	MaterialReward = InMaterialReward;
	GoldReward = InGoldReward;
	ExperienceReward = InExperienceReward;
	HealAmount = InHealAmount;
	bRequiresManualInteract = bInRequiresManualInteract;
	if (InTexture)
	{
		SpriteComponent->SetSprite(InTexture);
	}
	else
	{
		RefreshVisuals();
	}
}

void AT66MiniInteractable::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		UpdateLifetimePresentation();
		return;
	}

	LifetimeRemaining -= DeltaSeconds;
	if (LifetimeRemaining <= 0.f)
	{
		Destroy();
		return;
	}

	HoverPhase += DeltaSeconds * 1.8f;
	SpriteComponent->SetRelativeLocation(FVector(0.f, 0.f, 70.f + (FMath::Sin(HoverPhase) * 10.f)));
	IndicatorMesh->SetRelativeScale3D(FVector(0.38f + (FMath::Sin(HoverPhase * 1.35f) * 0.05f), 0.38f + (FMath::Sin(HoverPhase * 1.35f) * 0.05f), 1.0f));
	UpdateLifetimePresentation();

	if (!bRequiresManualInteract)
	{
		if (AT66MiniPlayerPawn* PlayerPawn = FindClosestPlayerPawn(true))
		{
			const float PickupDistance = CollisionComponent->GetScaledSphereRadius() + 28.f;
			if (FVector::DistSquared2D(GetActorLocation(), PlayerPawn->GetActorLocation()) <= FMath::Square(PickupDistance))
			{
				ConsumeInteractable(PlayerPawn);
			}
		}
	}
}

void AT66MiniInteractable::HandleOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	const int32 OtherBodyIndex,
	const bool bFromSweep,
	const FHitResult& SweepResult)
{
	AT66MiniPlayerPawn* PlayerPawn = Cast<AT66MiniPlayerPawn>(OtherActor);
	if (!PlayerPawn)
	{
		return;
	}

	if (bRequiresManualInteract)
	{
		return;
	}

	ConsumeInteractable(PlayerPawn);
}

bool AT66MiniInteractable::TryInteract(AT66MiniPlayerPawn* PlayerPawn)
{
	if (!HasAuthority() || !PlayerPawn || bConsumed)
	{
		return false;
	}

	const float RequiredDistance = CollisionComponent->GetScaledSphereRadius() + 42.f;
	if (FVector::DistSquared2D(GetActorLocation(), PlayerPawn->GetActorLocation()) > FMath::Square(RequiredDistance))
	{
		return false;
	}

	ConsumeInteractable(PlayerPawn);
	return true;
}

void AT66MiniInteractable::ConsumeInteractable(AT66MiniPlayerPawn* PlayerPawn)
{
	if (!HasAuthority() || !PlayerPawn || bConsumed)
	{
		return;
	}

	bConsumed = true;

	FLinearColor RewardTint(0.30f, 0.82f, 1.0f, 0.30f);
	float PickupPitch = 1.32f;

	switch (InteractableType)
	{
	case ET66MiniInteractableType::Fountain:
		PlayerPawn->GrantHeart(1, true);
		RewardTint = FLinearColor(0.34f, 1.0f, 0.58f, 0.34f);
		PickupPitch = 1.22f;
		break;

	case ET66MiniInteractableType::TreasureChest:
		PlayerPawn->GainGold(GoldReward > 0 ? GoldReward : FMath::Max(0, MaterialReward));
		PlayerPawn->GainExperience(ExperienceReward);
		RewardTint = FLinearColor(1.0f, 0.86f, 0.34f, 0.30f);
		break;

	case ET66MiniInteractableType::LootCrate:
		if (AT66MiniPlayerController* MiniController = Cast<AT66MiniPlayerController>(PlayerPawn->GetController()))
		{
			if (MiniController->StartLootCratePresentation())
			{
				PlayerPawn->GainExperience(ExperienceReward);
				RewardTint = FLinearColor(0.44f, 0.82f, 1.0f, 0.34f);
				PickupPitch = 1.26f;
				break;
			}
		}

		bConsumed = false;
		return;

	case ET66MiniInteractableType::QuickReviveMachine:
		PlayerPawn->GrantQuickRevive();
		RewardTint = FLinearColor(0.42f, 1.0f, 0.68f, 0.38f);
		PickupPitch = 1.10f;
		break;

	default:
		PlayerPawn->GainMaterials(MaterialReward);
		PlayerPawn->GainGold(GoldReward);
		PlayerPawn->GainExperience(ExperienceReward);
		if (HealAmount > 0.f)
		{
			PlayerPawn->Heal(HealAmount);
		}
		break;
	}

	if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
	{
		VfxSubsystem->SpawnPulse(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 4.f), FVector(0.26f, 0.26f, 1.f), 0.18f, RewardTint, 0.80f);
		VfxSubsystem->PlayPickupSfx(this, 0.18f, PickupPitch);
	}

	Destroy();
}

AT66MiniPlayerPawn* AT66MiniInteractable::FindClosestPlayerPawn(const bool bRequireAlive) const
{
	if (const AT66MiniGameMode* MiniGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AT66MiniGameMode>() : nullptr)
	{
		return MiniGameMode->FindClosestPlayerPawn(GetActorLocation(), bRequireAlive);
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AT66MiniPlayerPawn* BestPawn = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();
	for (TActorIterator<AT66MiniPlayerPawn> It(World); It; ++It)
	{
		AT66MiniPlayerPawn* Candidate = *It;
		if (!Candidate || (bRequireAlive && !Candidate->IsHeroAlive()))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared2D(GetActorLocation(), Candidate->GetActorLocation());
		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestPawn = Candidate;
		}
	}

	return BestPawn;
}

void AT66MiniInteractable::UpdateLifetimePresentation()
{
	const bool bNearExpiry = LifetimeRemaining <= 2.2f;
	if (!bNearExpiry)
	{
		SpriteComponent->SetVisibility(true, true);
		IndicatorMesh->SetVisibility(true, true);
		return;
	}

	const bool bBlinkVisible = FMath::Fmod(GetWorld()->GetTimeSeconds() * 8.0f, 1.0f) > 0.30f;
	SpriteComponent->SetVisibility(bBlinkVisible, true);
	IndicatorMesh->SetVisibility(true, true);
	const float ExpiryScale = 0.30f + (0.08f * (FMath::Sin(GetWorld()->GetTimeSeconds() * 9.0f) * 0.5f + 0.5f));
	IndicatorMesh->SetRelativeScale3D(FVector(ExpiryScale, ExpiryScale, 1.0f));
}

void AT66MiniInteractable::RefreshVisuals()
{
	UT66MiniVisualSubsystem* VisualSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
	if (!VisualID.IsEmpty() && VisualSubsystem)
	{
		if (UTexture2D* InteractableTexture = VisualSubsystem->LoadInteractableTexture(VisualID))
		{
			SpriteComponent->SetSprite(InteractableTexture);
		}
	}
}

void AT66MiniInteractable::OnRep_InteractableVisualState()
{
	RefreshVisuals();
}
