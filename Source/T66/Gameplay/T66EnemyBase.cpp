// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66EnemyDirector.h"
#include "Gameplay/T66EnemyAIController.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66Rarity.h"
#include "UI/T66EnemyHealthBarWidget.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EngineUtils.h"

AT66EnemyBase::AT66EnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AT66EnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	CurrentHP = MaxHP;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 350.f;
	}

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap); // so projectile overlaps and hits
	}

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Align primitive mesh to ground when capsule is grounded:
	// capsule half-height~88, cylinder half-height=50 => relative Z = 50 - 88 = -38.
	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, -38.f));
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (Cylinder)
	{
		VisualMesh->SetStaticMesh(Cylinder);
		VisualMesh->SetRelativeScale3D(FVector(0.9f, 0.9f, 1.f));
		UMaterialInstanceDynamic* Mat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (Mat)
		{
			Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.8f, 0.2f, 0.2f, 1.f));
		}
	}

	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 155.f));
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidget->SetDrawAtDesiredSize(true);
	HealthBarWidget->SetDrawSize(FVector2D(120.f, 12.f));
}

void AT66EnemyBase::BeginPlay()
{
	Super::BeginPlay();
	if (!bBaseTuningInitialized)
	{
		BaseMaxHP = MaxHP;
		BaseTouchDamageHearts = TouchDamageHearts;
		bBaseTuningInitialized = true;
	}
	// Ensure HP is valid on spawn (in case difficulty scaled before BeginPlay).
	CurrentHP = FMath::Clamp(CurrentHP, 1, MaxHP);

	// Ensure the widget exists and is the correct class.
	if (HealthBarWidget)
	{
		HealthBarWidget->SetWidgetClass(UT66EnemyHealthBarWidget::StaticClass());
		HealthBarWidget->InitWidget();
	}
	UpdateHealthBar();

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->OnComponentBeginOverlap.AddDynamic(this, &AT66EnemyBase::OnCapsuleBeginOverlap);
	}
}

void AT66EnemyBase::ApplyDifficultyTier(int32 Tier)
{
	if (!bBaseTuningInitialized)
	{
		BaseMaxHP = MaxHP;
		BaseTouchDamageHearts = TouchDamageHearts;
		bBaseTuningInitialized = true;
	}
	const int32 ClampedTier = FMath::Max(0, Tier);
	const float Mult = 1.f + static_cast<float>(ClampedTier);
	MaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseMaxHP) * Mult));
	CurrentHP = MaxHP;
	TouchDamageHearts = FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseTouchDamageHearts) * Mult));
	UpdateHealthBar();
}

void AT66EnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (CurrentHP <= 0) return;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move) return;

	// Safe zone rule: enemies cannot enter NPC safe bubbles.
	// Perf: evaluate bubble membership at a low frequency (not every Tick).
	SafeZoneCheckAccumSeconds += DeltaSeconds;
	if (SafeZoneCheckAccumSeconds >= SafeZoneCheckIntervalSeconds)
	{
		SafeZoneCheckAccumSeconds = 0.f;
		bCachedInsideSafeZone = false;
		CachedSafeZoneEscapeDir = FVector::ZeroVector;

		for (TActorIterator<AT66HouseNPCBase> It(GetWorld()); It; ++It)
		{
			AT66HouseNPCBase* NPC = *It;
			if (!NPC) continue;

			const FVector N = NPC->GetActorLocation();
			FVector ToEnemy = GetActorLocation() - N;
			ToEnemy.Z = 0.f;
			const float Dist = ToEnemy.Size();
			if (Dist < NPC->GetSafeZoneRadius())
			{
				bCachedInsideSafeZone = true;
				if (Dist > 1.f)
				{
					ToEnemy /= Dist;
					CachedSafeZoneEscapeDir = ToEnemy;
				}
				break;
			}
		}
	}

	if (bCachedInsideSafeZone)
	{
		if (!CachedSafeZoneEscapeDir.IsNearlyZero())
		{
			AddMovementInput(CachedSafeZoneEscapeDir, 1.f);
		}
		return;
	}

	FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	float Len = ToPlayer.Size();
	if (Len > 10.f)
	{
		ToPlayer /= Len;
		if (bRunAwayFromPlayer)
		{
			AddMovementInput(-ToPlayer, 1.f);
		}
		else
		{
			AddMovementInput(ToPlayer, 1.f);
		}
	}
}

void AT66EnemyBase::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero) return;
	if (Hero->IsInSafeZone()) return;

	UWorld* World = GetWorld();
	if (!World) return;
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return;
	UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
	if (!RunState) return;

	const float Now = static_cast<float>(World->GetTimeSeconds());
	if (Now - LastTouchDamageTime < TouchDamageCooldown) return;

	LastTouchDamageTime = Now;
	RunState->ApplyDamage(TouchDamageHearts);
}

bool AT66EnemyBase::TakeDamageFromHero(int32 Damage)
{
	if (Damage <= 0 || CurrentHP <= 0) return false;
	CurrentHP = FMath::Max(0, CurrentHP - Damage);
	UpdateHealthBar();
	if (CurrentHP <= 0)
	{
		OnDeath();
		return true;
	}
	return false;
}

void AT66EnemyBase::UpdateHealthBar()
{
	if (!HealthBarWidget) return;
	UT66EnemyHealthBarWidget* W = Cast<UT66EnemyHealthBarWidget>(HealthBarWidget->GetUserWidgetObject());
	if (!W) return;
	const float Pct = (MaxHP <= 0) ? 0.f : (static_cast<float>(FMath::Clamp(CurrentHP, 0, MaxHP)) / static_cast<float>(MaxHP));
	W->SetHealthPercent(Pct);
}

void AT66EnemyBase::SetLockedIndicator(bool bLocked)
{
	if (!HealthBarWidget) return;
	UT66EnemyHealthBarWidget* W = Cast<UT66EnemyHealthBarWidget>(HealthBarWidget->GetUserWidgetObject());
	if (!W) return;
	W->SetLocked(bLocked);
}

void AT66EnemyBase::OnDeath()
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState)
	{
		RunState->AddScore(PointValue);
		RunState->AddStructuredEvent(ET66RunEventType::EnemyKilled, FString::Printf(TEXT("PointValue=%d"), PointValue));
	}

	if (OwningDirector)
	{
		OwningDirector->NotifyEnemyDied(this);
	}

	if (!World) return;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance());
	if (bDropsLoot && T66GI)
	{
		// Spawn one loot bag with rarity, and roll an item from that rarity pool.
		FRandomStream Rng(FPlatformTime::Cycles());
		const ET66Rarity BagRarity = FT66RarityUtil::RollDefaultRarity(Rng);
		const FName ItemID = T66GI->GetRandomItemIDForLootRarity(BagRarity);
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66LootBagPickup* Loot = World->SpawnActor<AT66LootBagPickup>(AT66LootBagPickup::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
		if (Loot)
		{
			Loot->SetLootRarity(BagRarity);
			Loot->SetItemID(ItemID);
		}
	}

	Destroy();
}
