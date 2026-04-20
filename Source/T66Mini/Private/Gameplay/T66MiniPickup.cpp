// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiniPickup.h"

#include "Components/BillboardComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/T66MiniVisualSubsystem.h"
#include "Core/T66MiniVFXSubsystem.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Gameplay/Components/T66MiniShadowComponent.h"
#include "Gameplay/T66MiniGameMode.h"
#include "Gameplay/T66MiniPlayerPawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"

namespace
{
	UStaticMesh* T66MiniPickupLoadPlaneMesh()
	{
		static TWeakObjectPtr<UStaticMesh> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
		}

		return Cached.Get();
	}

	UMaterialInterface* T66MiniPickupLoadArenaMaterial()
	{
		static TWeakObjectPtr<UMaterialInterface> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
		}

		return Cached.Get();
	}

	void T66MiniPickupConfigureIndicator(UStaticMeshComponent* MeshComponent, UObject* Outer, const FLinearColor& Tint)
	{
		if (!MeshComponent)
		{
			return;
		}

		UMaterialInterface* BaseMaterial = T66MiniPickupLoadArenaMaterial();
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

		if (const UGameInstance* GameInstance = MeshComponent->GetWorld() ? MeshComponent->GetWorld()->GetGameInstance() : nullptr)
		{
			if (UT66MiniVisualSubsystem* VisualSubsystem = GameInstance->GetSubsystem<UT66MiniVisualSubsystem>())
			{
				if (UTexture2D* WhiteTexture = VisualSubsystem->GetWhiteTexture())
				{
					Material->SetTextureParameterValue(TEXT("DiffuseColorMap"), WhiteTexture);
					Material->SetTextureParameterValue(TEXT("BaseColorTexture"), WhiteTexture);
				}
			}
		}

		Material->SetVectorParameterValue(TEXT("Color"), Tint);
		Material->SetVectorParameterValue(TEXT("BaseColor"), Tint);
		Material->SetVectorParameterValue(TEXT("Tint"), Tint);
	}
}

AT66MiniPickup::AT66MiniPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.f / 30.f;
	bReplicates = true;
	SetReplicateMovement(true);
	NetUpdateFrequency = 12.f;
	MinNetUpdateFrequency = 6.f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->SetupAttachment(SceneRoot);
	CollisionComponent->InitSphereRadius(60.f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);

	SpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	SpriteComponent->SetupAttachment(SceneRoot);
	SpriteComponent->SetRelativeLocation(FVector(0.f, 0.f, 40.f));
	SpriteComponent->SetRelativeScale3D(FVector(1.75f, 1.75f, 1.75f));
	SpriteComponent->SetHiddenInGame(false, true);
	SpriteComponent->SetVisibility(true, true);

	IndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Indicator"));
	IndicatorMesh->SetupAttachment(SceneRoot);
	IndicatorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	IndicatorMesh->SetCastShadow(false);
	IndicatorMesh->SetStaticMesh(T66MiniPickupLoadPlaneMesh());
	IndicatorMesh->SetRelativeLocation(FVector(0.f, 0.f, 3.f));
	IndicatorMesh->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	IndicatorMesh->SetRelativeScale3D(FVector(0.28f, 0.28f, 1.0f));
	T66MiniPickupConfigureIndicator(IndicatorMesh, this, FLinearColor(0.96f, 0.84f, 0.28f, 0.42f));

	ShadowComponent = CreateDefaultSubobject<UT66MiniShadowComponent>(TEXT("Shadow"));
	ShadowComponent->InitializeShadow(SceneRoot, FVector(0.f, 0.f, 1.f), FVector(0.18f, 0.18f, 1.f), FLinearColor(0.f, 0.f, 0.f, 0.12f));
}

void AT66MiniPickup::BeginPlay()
{
	Super::BeginPlay();
	if (AT66MiniGameMode* MiniGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AT66MiniGameMode>() : nullptr)
	{
		MiniGameMode->RegisterLivePickup(this);
	}
	if (HasAuthority())
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AT66MiniPickup::HandleOverlap);
	}
	HoverPhase = FMath::FRandRange(0.f, UE_TWO_PI);
	RefreshVisuals();
}

void AT66MiniPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AT66MiniGameMode* MiniGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AT66MiniGameMode>() : nullptr)
	{
		MiniGameMode->UnregisterLivePickup(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AT66MiniPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT66MiniPickup, MaterialValue);
	DOREPLIFETIME(AT66MiniPickup, ExperienceValue);
	DOREPLIFETIME(AT66MiniPickup, HealValue);
	DOREPLIFETIME(AT66MiniPickup, VisualID);
	DOREPLIFETIME(AT66MiniPickup, GrantedItemID);
}

void AT66MiniPickup::InitializePickup(
	const int32 InMaterialValue,
	const float InExperienceValue,
	const float InHealValue,
	UTexture2D* InTexture,
	const FString& InVisualID,
	const float InLifetimeRemaining,
	const FName InGrantedItemID)
{
	MaterialValue = InMaterialValue;
	ExperienceValue = InExperienceValue;
	HealValue = InHealValue;
	VisualID = InVisualID;
	LifetimeRemaining = InLifetimeRemaining;
	GrantedItemID = InGrantedItemID;
	if (InTexture)
	{
		SpriteComponent->SetSprite(InTexture);
	}
	else
	{
		RefreshVisuals();
	}
}

void AT66MiniPickup::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	LifetimeRemaining -= DeltaSeconds;
	if (LifetimeRemaining <= 0.f)
	{
		Destroy();
		return;
	}

	HoverPhase += DeltaSeconds * 2.5f;
	SpriteComponent->SetRelativeLocation(FVector(0.f, 0.f, 40.f + (FMath::Sin(HoverPhase) * 8.f)));
	IndicatorMesh->SetRelativeScale3D(FVector(0.24f + (FMath::Sin(HoverPhase * 1.5f) * 0.04f), 0.24f + (FMath::Sin(HoverPhase * 1.5f) * 0.04f), 1.0f));
	if (LifetimeRemaining <= 2.0f)
	{
		const bool bBlinkVisible = FMath::Fmod(GetWorld()->GetTimeSeconds() * 9.0f, 1.0f) > 0.30f;
		SpriteComponent->SetVisibility(bBlinkVisible, true);
	}
	else
	{
		SpriteComponent->SetVisibility(true, true);
	}

	if (AT66MiniPlayerPawn* PlayerPawn = FindClosestPlayerPawn(true))
	{
		const FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
		const float Distance = ToPlayer.Size2D();
		const float MagnetRadius = PlayerPawn->GetPickupMagnetRadius();
		const float PullSpeed = PlayerPawn->GetPickupMagnetPullSpeed();
		const float PickupDistance = CollisionComponent->GetScaledSphereRadius() + 26.f;
		if (Distance < MagnetRadius && Distance > KINDA_SMALL_NUMBER)
		{
			SetActorLocation(GetActorLocation() + (ToPlayer.GetSafeNormal2D() * FMath::Min(DeltaSeconds * PullSpeed, Distance)));
		}

		if (Distance <= PickupDistance)
		{
			CollectPickup(PlayerPawn);
		}
	}
}

void AT66MiniPickup::HandleOverlap(
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

	CollectPickup(PlayerPawn);
}

void AT66MiniPickup::CollectPickup(AT66MiniPlayerPawn* PlayerPawn)
{
	if (!HasAuthority() || !PlayerPawn || bCollected)
	{
		return;
	}

	bCollected = true;
	PlayerPawn->GainMaterials(MaterialValue);
	PlayerPawn->GainExperience(ExperienceValue);
	PlayerPawn->Heal(HealValue);
	const bool bGrantedItem = !GrantedItemID.IsNone() && PlayerPawn->AcquireItem(GrantedItemID);
	if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
	{
		const FLinearColor PickupTint = bGrantedItem
			? FLinearColor(0.44f, 0.82f, 1.0f, 0.34f)
			: FLinearColor(1.0f, 0.88f, 0.34f, 0.30f);
		VfxSubsystem->SpawnPulse(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 4.f), FVector(0.20f, 0.20f, 1.f), 0.14f, PickupTint, 0.75f);
		VfxSubsystem->PlayPickupSfx(this);
	}
	Destroy();
}

AT66MiniPlayerPawn* AT66MiniPickup::FindClosestPlayerPawn(const bool bRequireAlive) const
{
	if (const AT66MiniGameMode* MiniGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AT66MiniGameMode>() : nullptr)
	{
		return MiniGameMode->FindClosestPlayerPawn(GetActorLocation(), bRequireAlive);
	}
	return nullptr;
}

void AT66MiniPickup::RefreshVisuals()
{
	UT66MiniVisualSubsystem* VisualSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
	if (!VisualID.IsEmpty() && VisualSubsystem)
	{
		if (UTexture2D* PickupTexture = VisualSubsystem->LoadInteractableTexture(VisualID))
		{
			SpriteComponent->SetSprite(PickupTexture);
		}
	}
}

void AT66MiniPickup::OnRep_VisualState()
{
	RefreshVisuals();
}
