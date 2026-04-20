// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiniEnemyBase.h"

#include "Components/BillboardComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniVFXSubsystem.h"
#include "Core/T66MiniVisualSubsystem.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Gameplay/Components/T66MiniDirectionResolverComponent.h"
#include "Gameplay/Components/T66MiniHitFlashComponent.h"
#include "Gameplay/Components/T66MiniShadowComponent.h"
#include "Gameplay/Components/T66MiniSpritePresentationComponent.h"
#include "Gameplay/T66MiniGameMode.h"
#include "Gameplay/T66MiniEnemyProjectile.h"
#include "Gameplay/T66MiniPickup.h"
#include "Gameplay/T66MiniPlayerPawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"

namespace
{
	const FT66MiniItemDefinition* T66MiniChooseRandomItemDefinition(const UT66MiniDataSubsystem* DataSubsystem)
	{
		if (!DataSubsystem || DataSubsystem->GetItems().Num() == 0)
		{
			return nullptr;
		}

		const int32 PickedIndex = FMath::RandRange(0, DataSubsystem->GetItems().Num() - 1);
		return &DataSubsystem->GetItems()[PickedIndex];
	}

	UStaticMesh* T66MiniEnemyLoadPlaneMesh()
	{
		static TWeakObjectPtr<UStaticMesh> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
		}

		return Cached.Get();
	}

	UMaterialInterface* T66MiniEnemyLoadArenaMaterial()
	{
		static TWeakObjectPtr<UMaterialInterface> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
		}

		return Cached.Get();
	}

	void T66MiniEnemyConfigureIndicator(UStaticMeshComponent* MeshComponent, UObject* Outer, const FLinearColor& Tint)
	{
		if (!MeshComponent)
		{
			return;
		}

		UMaterialInterface* BaseMaterial = T66MiniEnemyLoadArenaMaterial();
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

AT66MiniEnemyBase::AT66MiniEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	NetUpdateFrequency = 16.f;
	MinNetUpdateFrequency = 8.f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->SetupAttachment(SceneRoot);
	CollisionComponent->InitSphereRadius(70.f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	SpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	SpriteComponent->SetupAttachment(SceneRoot);
	SpriteComponent->SetRelativeLocation(FVector(0.f, 0.f, 54.f));
	SpriteComponent->SetRelativeScale3D(FVector(1.20f, 1.20f, 1.20f));
	SpriteComponent->SetHiddenInGame(false, true);
	SpriteComponent->SetVisibility(true, true);

	SpritePresentationComponent = CreateDefaultSubobject<UT66MiniSpritePresentationComponent>(TEXT("SpritePresentation"));
	SpritePresentationComponent->BindSprite(SpriteComponent);

	DirectionResolverComponent = CreateDefaultSubobject<UT66MiniDirectionResolverComponent>(TEXT("DirectionResolver"));
	DirectionResolverComponent->BindPresentation(SpritePresentationComponent);

	ShadowComponent = CreateDefaultSubobject<UT66MiniShadowComponent>(TEXT("Shadow"));
	ShadowComponent->InitializeShadow(SceneRoot, FVector(0.f, 0.f, 1.0f), FVector(0.30f, 0.30f, 1.f), FLinearColor(0.f, 0.f, 0.f, 0.20f));

	HitFlashComponent = CreateDefaultSubobject<UT66MiniHitFlashComponent>(TEXT("HitFlash"));
	HitFlashComponent->InitializeFlash(SceneRoot, FVector(0.f, 0.f, 5.f), FVector(0.28f, 0.28f, 1.f), FLinearColor(0.98f, 0.34f, 0.20f, 0.42f));

	IndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Indicator"));
	IndicatorMesh->SetupAttachment(SceneRoot);
	IndicatorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	IndicatorMesh->SetCastShadow(false);
	IndicatorMesh->SetStaticMesh(T66MiniEnemyLoadPlaneMesh());
	IndicatorMesh->SetRelativeLocation(FVector(0.f, 0.f, 4.f));
	IndicatorMesh->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	IndicatorMesh->SetRelativeScale3D(FVector(0.24f, 0.24f, 1.0f));
	T66MiniEnemyConfigureIndicator(IndicatorMesh, this, FLinearColor(0.84f, 0.18f, 0.18f, 0.34f));
}

void AT66MiniEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	RefreshPresentationFromState();
}

void AT66MiniEnemyBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT66MiniEnemyBase, EnemyID);
	DOREPLIFETIME(AT66MiniEnemyBase, bIsBoss);
	DOREPLIFETIME(AT66MiniEnemyBase, MaxHealth);
	DOREPLIFETIME(AT66MiniEnemyBase, CurrentHealth);
}

void AT66MiniEnemyBase::InitializeEnemy(
	const FName InEnemyID,
	const bool bInBoss,
	UTexture2D* InTexture,
	const float InMaxHealth,
	const float InMoveSpeed,
	const float InTouchDamage,
	const int32 InMaterialDrop,
	const float InExperienceDrop,
	const float InCurrentHealth,
	const ET66MiniEnemyBehaviorProfile InBehaviorProfile,
	const ET66MiniEnemyFamily InFamily,
	const float InFireIntervalSeconds,
	const float InProjectileSpeed,
	const float InProjectileDamage,
	const float InPreferredRange)
{
	EnemyID = InEnemyID;
	bIsBoss = bInBoss;
	BehaviorProfile = InBehaviorProfile;
	EnemyFamily = bIsBoss ? ET66MiniEnemyFamily::Boss : InFamily;
	MaxHealth = InMaxHealth;
	CurrentHealth = InCurrentHealth >= 0.f ? FMath::Min(InCurrentHealth, InMaxHealth) : InMaxHealth;
	MoveSpeed = InMoveSpeed;
	TouchDamage = InTouchDamage;
	MaterialDrop = InMaterialDrop;
	ExperienceDrop = InExperienceDrop;
	FireIntervalSeconds = FMath::Clamp(InFireIntervalSeconds, 0.45f, 4.0f);
	ProjectileSpeed = FMath::Clamp(InProjectileSpeed, 400.f, 2600.f);
	ProjectileDamage = FMath::Max(1.f, InProjectileDamage);
	PreferredRange = FMath::Max(220.f, InPreferredRange);
	ProjectileCooldownRemaining = (EnemyFamily == ET66MiniEnemyFamily::Ranged || EnemyFamily == ET66MiniEnemyFamily::Boss)
		? FMath::FRandRange(FireIntervalSeconds * 0.25f, FireIntervalSeconds * 0.9f)
		: 0.f;
	ChargeCooldownRemaining = (BehaviorProfile == ET66MiniEnemyBehaviorProfile::GoatCharger || BehaviorProfile == ET66MiniEnemyBehaviorProfile::Duelist)
		? FMath::FRandRange(1.0f, 2.2f)
		: 0.f;
	if (EnemyFamily == ET66MiniEnemyFamily::Boss)
	{
		ChargeCooldownRemaining = FMath::FRandRange(1.4f, 2.2f);
	}
	ChargeDurationRemaining = 0.f;
	ChargeDirection = FVector::ForwardVector;
	RefreshPresentationFromState();
	if (InTexture)
	{
		SpriteComponent->SetSprite(InTexture);
	}
}

void AT66MiniEnemyBase::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDead)
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	for (int32 Index = ActiveDots.Num() - 1; Index >= 0; --Index)
	{
		FActiveDot& Dot = ActiveDots[Index];
		Dot.RemainingDuration -= DeltaSeconds;
		Dot.TickAccumulator += DeltaSeconds;

		while (Dot.TickAccumulator >= Dot.TickInterval && Dot.RemainingDuration > 0.f)
		{
			Dot.TickAccumulator -= Dot.TickInterval;
			ApplyDamage(Dot.TickDamage);
			if (bDead)
			{
				return;
			}
		}

		if (Dot.RemainingDuration <= 0.f)
		{
			ActiveDots.RemoveAtSwap(Index);
		}
	}

	if (StunRemaining > 0.f)
	{
		StunRemaining = FMath::Max(0.f, StunRemaining - DeltaSeconds);
		return;
	}

	if (AT66MiniPlayerPawn* PlayerPawn = FindBestTargetPawn())
	{
		const FVector CurrentLocation = GetActorLocation();
		const FVector TargetLocation = PlayerPawn->GetActorLocation();
		const FVector ToPlayer = TargetLocation - CurrentLocation;
		const float Distance = ToPlayer.Size2D();
		float EffectiveMoveSpeed = MoveSpeed;
		float EffectiveTouchDamage = TouchDamage;
		FVector MovementDirection = ToPlayer.GetSafeNormal2D();
		ProjectileCooldownRemaining = FMath::Max(0.f, ProjectileCooldownRemaining - DeltaSeconds);
		const bool bCanCharge = EnemyFamily == ET66MiniEnemyFamily::Rushing || EnemyFamily == ET66MiniEnemyFamily::Boss;
		const bool bCanShoot = EnemyFamily == ET66MiniEnemyFamily::Ranged || EnemyFamily == ET66MiniEnemyFamily::Boss;

		if (EnemyFamily == ET66MiniEnemyFamily::Ranged)
		{
			const FVector StrafeDirection = FVector(-MovementDirection.Y, MovementDirection.X, 0.f);
			if (Distance > PreferredRange * 1.12f)
			{
				EffectiveMoveSpeed *= 1.10f;
			}
			else if (Distance < PreferredRange * 0.72f)
			{
				MovementDirection *= -1.f;
				EffectiveMoveSpeed *= 1.05f;
			}
			else
			{
				MovementDirection = (MovementDirection * 0.34f) + (StrafeDirection * FMath::Sin(GetWorld()->GetTimeSeconds() * 2.2f));
				MovementDirection.Normalize();
				EffectiveMoveSpeed *= 0.88f;
			}
		}
		else if (BehaviorProfile == ET66MiniEnemyBehaviorProfile::CowBruiser || BehaviorProfile == ET66MiniEnemyBehaviorProfile::Juggernaut)
		{
			EffectiveMoveSpeed *= Distance < 420.f ? 0.82f : 1.04f;
			EffectiveTouchDamage *= BehaviorProfile == ET66MiniEnemyBehaviorProfile::Juggernaut ? 1.35f : 1.15f;
		}
		else if (bCanCharge)
		{
			if (ChargeDurationRemaining > 0.f)
			{
				ChargeDurationRemaining -= DeltaSeconds;
				EffectiveMoveSpeed *= (EnemyFamily == ET66MiniEnemyFamily::Boss || BehaviorProfile == ET66MiniEnemyBehaviorProfile::Duelist) ? 2.25f : 1.95f;
				EffectiveTouchDamage *= EnemyFamily == ET66MiniEnemyFamily::Boss ? 1.35f : 1.2f;
				MovementDirection = ChargeDirection;
			}
			else
			{
				ChargeCooldownRemaining -= DeltaSeconds;
				EffectiveMoveSpeed *= (EnemyFamily == ET66MiniEnemyFamily::Boss || BehaviorProfile == ET66MiniEnemyBehaviorProfile::Duelist) ? 1.08f : 0.95f;
				if (ChargeCooldownRemaining <= 0.f && Distance >= 260.f && Distance <= 1200.f)
				{
					ChargeDirection = MovementDirection;
					ChargeDurationRemaining = (EnemyFamily == ET66MiniEnemyFamily::Boss || BehaviorProfile == ET66MiniEnemyBehaviorProfile::Duelist) ? 0.50f : 0.42f;
					ChargeCooldownRemaining = (EnemyFamily == ET66MiniEnemyFamily::Boss || BehaviorProfile == ET66MiniEnemyBehaviorProfile::Duelist)
						? FMath::FRandRange(1.8f, 2.6f)
						: FMath::FRandRange(2.4f, 3.6f);
				}
			}
		}
		else if ((BehaviorProfile == ET66MiniEnemyBehaviorProfile::PigEnrage || BehaviorProfile == ET66MiniEnemyBehaviorProfile::Balanced) && MaxHealth > 0.f && (CurrentHealth / MaxHealth) <= 0.5f)
		{
			EffectiveMoveSpeed *= 1.32f;
			EffectiveTouchDamage *= 1.25f;
		}

		if (EnemyFamily == ET66MiniEnemyFamily::Boss && Distance > PreferredRange * 0.85f && Distance < PreferredRange * 1.25f && ChargeDurationRemaining <= 0.f)
		{
			const FVector StrafeDirection = FVector(-MovementDirection.Y, MovementDirection.X, 0.f);
			MovementDirection = (MovementDirection * 0.55f) + (StrafeDirection * FMath::Sin(GetWorld()->GetTimeSeconds() * 1.5f));
			MovementDirection.Normalize();
		}

		if (bCanShoot && ProjectileCooldownRemaining <= 0.f && Distance <= PreferredRange * 1.6f)
		{
			FireProjectileAtPlayer(TargetLocation);
			ProjectileCooldownRemaining = FireIntervalSeconds;
		}

		if (Distance > KINDA_SMALL_NUMBER)
		{
			SetActorLocation(CurrentLocation + (MovementDirection * EffectiveMoveSpeed * DeltaSeconds));
		}

		if (Distance <= (CollisionComponent->GetScaledSphereRadius() + 75.f) && GetWorld()->GetTimeSeconds() >= NextTouchDamageTime)
		{
			PlayerPawn->ApplyDamage(EffectiveTouchDamage);
			NextTouchDamageTime = GetWorld()->GetTimeSeconds() + 0.8f;
		}
	}
}

void AT66MiniEnemyBase::ApplyDamage(const float InDamage)
{
	if (!HasAuthority() || bDead || InDamage <= 0.f)
	{
		return;
	}

	CurrentHealth -= InDamage;
	if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
	{
		MiniGameMode->AddCombatText(GetActorLocation() + FVector(0.f, 0.f, bIsBoss ? 110.f : 80.f), InDamage, FLinearColor::White);
	}
	if (HitFlashComponent)
	{
		HitFlashComponent->TriggerHitFlash(bIsBoss ? FLinearColor(1.0f, 0.52f, 0.26f, 0.50f) : FLinearColor(0.98f, 0.34f, 0.20f, 0.38f), bIsBoss ? 0.18f : 0.10f);
	}
	if (GetWorld() && GetWorld()->GetTimeSeconds() >= NextDamageFeedbackTime)
	{
		if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
		{
			VfxSubsystem->SpawnPulse(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 6.f), bIsBoss ? FVector(0.34f, 0.34f, 1.f) : FVector(0.22f, 0.22f, 1.f), bIsBoss ? 0.16f : 0.10f, FLinearColor(1.0f, 0.52f, 0.24f, bIsBoss ? 0.34f : 0.26f), 0.45f);
			VfxSubsystem->PlayHitSfx(this, bIsBoss ? 0.14f : 0.08f, bIsBoss ? 0.86f : 1.18f);
		}
		NextDamageFeedbackTime = GetWorld()->GetTimeSeconds() + 0.05f;
	}
	if (CurrentHealth <= 0.f)
	{
		HandleDeath();
	}
}

void AT66MiniEnemyBase::ApplyDot(const float TickDamage, const float TickInterval, const float Duration)
{
	if (bDead || TickDamage <= 0.f || TickInterval <= 0.f || Duration <= 0.f)
	{
		return;
	}

	FActiveDot& Dot = ActiveDots.AddDefaulted_GetRef();
	Dot.TickDamage = TickDamage;
	Dot.TickInterval = TickInterval;
	Dot.RemainingDuration = Duration;
	Dot.TickAccumulator = 0.f;
}

void AT66MiniEnemyBase::ApplyStun(const float DurationSeconds)
{
	if (bDead || DurationSeconds <= 0.f)
	{
		return;
	}

	StunRemaining = FMath::Max(StunRemaining, DurationSeconds);
	if (HitFlashComponent)
	{
		HitFlashComponent->TriggerHitFlash(FLinearColor(0.64f, 0.88f, 1.0f, 0.34f), 0.14f);
	}
}

AT66MiniPlayerPawn* AT66MiniEnemyBase::FindBestTargetPawn() const
{
	if (const AT66MiniGameMode* MiniGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AT66MiniGameMode>() : nullptr)
	{
		return MiniGameMode->FindClosestPlayerPawn(GetActorLocation(), true);
	}
	return nullptr;
}

void AT66MiniEnemyBase::HandleDeath()
{
	if (!HasAuthority() || bDead)
	{
		return;
	}

	bDead = true;

	UWorld* World = GetWorld();
	UGameInstance* GameInstance = GetGameInstance();
	UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniVisualSubsystem* VisualSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
	UT66MiniVFXSubsystem* VfxSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr;
	if (World)
	{
		if (VfxSubsystem)
		{
			VfxSubsystem->SpawnPulse(World, GetActorLocation() + FVector(0.f, 0.f, 8.f), bIsBoss ? FVector(0.55f, 0.55f, 1.f) : FVector(0.28f, 0.28f, 1.f), bIsBoss ? 0.20f : 0.12f, bIsBoss ? FLinearColor(1.0f, 0.58f, 0.22f, 0.36f) : FLinearColor(0.98f, 0.42f, 0.24f, 0.24f), bIsBoss ? 0.95f : 0.60f);
			VfxSubsystem->PlayHitSfx(this, bIsBoss ? 0.18f : 0.10f, bIsBoss ? 0.72f : 1.06f);
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (AT66MiniPickup* Pickup = World->SpawnActor<AT66MiniPickup>(AT66MiniPickup::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, SpawnParams))
		{
			const FString BagVisual = bIsBoss ? TEXT("LootBag_Yellow") : (FMath::FRand() > 0.5f ? TEXT("LootBag_Red") : TEXT("LootBag_Black"));
			Pickup->InitializePickup(MaterialDrop, ExperienceDrop, 0.f, VisualSubsystem ? VisualSubsystem->LoadInteractableTexture(BagVisual) : nullptr, BagVisual);
		}

		const bool bDropItemBag = bIsBoss || FMath::FRand() < 0.06f;
		if (bDropItemBag)
		{
			if (const FT66MiniItemDefinition* DroppedItem = T66MiniChooseRandomItemDefinition(DataSubsystem))
			{
				const FString ItemBagVisual = bIsBoss ? TEXT("LootBag_Yellow") : (FMath::FRand() > 0.55f ? TEXT("LootBag_Red") : TEXT("LootBag_Black"));
				const FVector ItemDropLocation = GetActorLocation() + FVector(FMath::FRandRange(-36.f, 36.f), FMath::FRandRange(-36.f, 36.f), 0.f);
				if (AT66MiniPickup* ItemPickup = World->SpawnActor<AT66MiniPickup>(AT66MiniPickup::StaticClass(), ItemDropLocation, FRotator::ZeroRotator, SpawnParams))
				{
					ItemPickup->InitializePickup(0, 0.f, 0.f, VisualSubsystem ? VisualSubsystem->LoadInteractableTexture(ItemBagVisual) : nullptr, ItemBagVisual, 28.f, DroppedItem->ItemID);
				}
			}
		}
	}

	if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
	{
		MiniGameMode->HandleEnemyKilled(this);
	}

	Destroy();
}

void AT66MiniEnemyBase::FireProjectileAtPlayer(const FVector& PlayerLocation)
{
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World)
	{
		return;
	}

	const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, bIsBoss ? 82.f : 48.f);
	const FVector FireDirection = (PlayerLocation - SpawnLocation).GetSafeNormal();
	UT66MiniVisualSubsystem* VisualSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
	UTexture2D* ProjectileTexture = VisualSubsystem
		? VisualSubsystem->LoadEffectTexture(bIsBoss ? TEXT("EnemyProjectile_Boss") : TEXT("EnemyProjectile_Ranged"))
		: nullptr;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66MiniEnemyProjectile* Projectile = World->SpawnActor<AT66MiniEnemyProjectile>(AT66MiniEnemyProjectile::StaticClass(), SpawnLocation, FireDirection.Rotation(), SpawnParams))
	{
		Projectile->InitializeProjectile(
			SpawnLocation,
			FireDirection,
			ProjectileSpeed,
			ProjectileDamage,
			ProjectileTexture,
			bIsBoss ? 5.2f : 4.0f);
	}

	if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
	{
		VfxSubsystem->SpawnPulse(World, SpawnLocation, bIsBoss ? FVector(0.26f, 0.26f, 1.f) : FVector(0.16f, 0.16f, 1.f), 0.10f, FLinearColor(1.0f, 0.62f, 0.30f, 0.22f), 0.45f);
		VfxSubsystem->PlayShotSfx(this, bIsBoss ? 0.18f : 0.10f, bIsBoss ? 0.78f : 1.02f);
	}
}

void AT66MiniEnemyBase::RefreshPresentationFromState()
{
	CollisionComponent->SetSphereRadius(bIsBoss ? 104.f : 70.f);
	SpriteComponent->SetRelativeLocation(FVector(0.f, 0.f, bIsBoss ? 82.f : 54.f));
	SpriteComponent->SetRelativeScale3D(bIsBoss ? FVector(4.35f, 4.35f, 4.35f) : FVector(2.85f, 2.85f, 2.85f));
	if (SpritePresentationComponent)
	{
		SpritePresentationComponent->SetBaseScale(SpriteComponent->GetRelativeScale3D());
	}
	IndicatorMesh->SetRelativeScale3D(bIsBoss ? FVector(0.36f, 0.36f, 1.0f) : FVector(0.23f, 0.23f, 1.0f));
	T66MiniEnemyConfigureIndicator(IndicatorMesh, this, bIsBoss ? FLinearColor(0.96f, 0.28f, 0.16f, 0.42f) : FLinearColor(0.84f, 0.18f, 0.18f, 0.30f));
	if (ShadowComponent)
	{
		ShadowComponent->InitializeShadow(
			SceneRoot,
			FVector(0.f, 0.f, 1.0f),
			bIsBoss ? FVector(0.35f, 0.35f, 1.f) : FVector(0.24f, 0.24f, 1.f),
			FLinearColor(0.f, 0.f, 0.f, bIsBoss ? 0.24f : 0.18f));
	}
	if (HitFlashComponent)
	{
		HitFlashComponent->InitializeFlash(
			SceneRoot,
			FVector(0.f, 0.f, 5.f),
			bIsBoss ? FVector(0.36f, 0.36f, 1.f) : FVector(0.24f, 0.24f, 1.f),
			bIsBoss ? FLinearColor(1.0f, 0.46f, 0.22f, 0.48f) : FLinearColor(0.98f, 0.34f, 0.20f, 0.40f));
	}

	UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniVisualSubsystem* VisualSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
	if (!DataSubsystem || !VisualSubsystem || EnemyID.IsNone())
	{
		return;
	}

	FString VisualID = EnemyID.ToString();
	if (bIsBoss)
	{
		if (const FT66MiniBossDefinition* BossDefinition = DataSubsystem->FindBoss(EnemyID))
		{
			VisualID = BossDefinition->VisualID;
		}
		SpriteComponent->SetSprite(VisualSubsystem->LoadBossTexture(VisualID));
	}
	else
	{
		if (const FT66MiniEnemyDefinition* EnemyDefinition = DataSubsystem->FindEnemy(EnemyID))
		{
			VisualID = EnemyDefinition->VisualID;
		}
		SpriteComponent->SetSprite(VisualSubsystem->LoadEnemyTexture(VisualID));
	}
}

void AT66MiniEnemyBase::OnRep_EnemyPresentationState()
{
	RefreshPresentationFromState();
}
