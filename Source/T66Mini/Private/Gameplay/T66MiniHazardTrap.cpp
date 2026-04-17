// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiniHazardTrap.h"

#include "Components/BillboardComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/T66MiniVisualSubsystem.h"
#include "Core/T66MiniVFXSubsystem.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Gameplay/T66MiniEnemyBase.h"
#include "Gameplay/T66MiniGameMode.h"
#include "Gameplay/T66MiniPlayerPawn.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"

namespace
{
	const TCHAR* T66MiniTrapCoreEffectName(const int32 TrapVariant)
	{
		switch (TrapVariant % 3)
		{
		case 1:
			return TEXT("Trap_Core_Spike");

		case 2:
			return TEXT("Trap_Core_Arcane");

		default:
			return TEXT("Trap_Core_Fire");
		}
	}

	UStaticMesh* T66MiniTrapLoadPlaneMesh()
	{
		static TWeakObjectPtr<UStaticMesh> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
		}

		return Cached.Get();
	}

	UMaterialInterface* T66MiniTrapLoadArenaMaterial()
	{
		static TWeakObjectPtr<UMaterialInterface> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
		}

		return Cached.Get();
	}
}

AT66MiniHazardTrap::AT66MiniHazardTrap()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	IndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Indicator"));
	IndicatorMesh->SetupAttachment(SceneRoot);
	IndicatorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	IndicatorMesh->SetCastShadow(false);
	IndicatorMesh->SetStaticMesh(T66MiniTrapLoadPlaneMesh());
	IndicatorMesh->SetRelativeLocation(FVector(0.f, 0.f, 3.f));
	IndicatorMesh->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	IndicatorMesh->SetRelativeScale3D(FVector(0.4f, 0.4f, 1.f));

	SpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	SpriteComponent->SetupAttachment(SceneRoot);
	SpriteComponent->SetRelativeLocation(FVector(0.f, 0.f, 40.f));
	SpriteComponent->SetRelativeScale3D(FVector(1.2f, 1.2f, 1.2f));
	SpriteComponent->SetHiddenInGame(false, true);
	SpriteComponent->SetVisibility(true, true);
}

void AT66MiniHazardTrap::BeginPlay()
{
	Super::BeginPlay();
	if (AT66MiniGameMode* MiniGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AT66MiniGameMode>() : nullptr)
	{
		MiniGameMode->RegisterLiveTrap(this);
	}

	UT66MiniVisualSubsystem* VisualSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
	UTexture2D* TelegraphTexture = VisualSubsystem ? VisualSubsystem->LoadEffectTexture(TEXT("Trap_Telegraph_Ring")) : nullptr;

	if (UMaterialInterface* BaseMaterial = T66MiniTrapLoadArenaMaterial())
	{
		IndicatorMaterial = IndicatorMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0, BaseMaterial);
		if (!IndicatorMaterial)
		{
			IndicatorMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			IndicatorMesh->SetMaterial(0, IndicatorMaterial);
		}

		if (IndicatorMaterial)
		{
			if (UTexture2D* RingTexture = TelegraphTexture ? TelegraphTexture : LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture")))
			{
				IndicatorMaterial->SetTextureParameterValue(TEXT("DiffuseColorMap"), RingTexture);
				IndicatorMaterial->SetTextureParameterValue(TEXT("BaseColorTexture"), RingTexture);
			}
		}
	}

	if (UTexture2D* TrapSprite = VisualSubsystem ? VisualSubsystem->LoadEffectTexture(T66MiniTrapCoreEffectName(TrapVariant)) : nullptr)
	{
		SpriteComponent->SetSprite(TrapSprite);
	}
	else if (UTexture2D* WhiteTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture")))
	{
		SpriteComponent->SetSprite(WhiteTexture);
	}

	UpdatePresentation();
}

void AT66MiniHazardTrap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AT66MiniGameMode* MiniGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AT66MiniGameMode>() : nullptr)
	{
		MiniGameMode->UnregisterLiveTrap(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AT66MiniHazardTrap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT66MiniHazardTrap, Radius);
	DOREPLIFETIME(AT66MiniHazardTrap, DamagePerPulse);
	DOREPLIFETIME(AT66MiniHazardTrap, PulseInterval);
	DOREPLIFETIME(AT66MiniHazardTrap, TrapVariant);
}

void AT66MiniHazardTrap::InitializeTrap(const FVector& SpawnLocation, float InRadius, float InDamagePerPulse, float InPulseInterval, float InWarmupSeconds, float InActiveSeconds, int32 InTrapVariant)
{
	SetActorLocation(SpawnLocation);
	Radius = FMath::Clamp(InRadius, 120.f, 620.f);
	DamagePerPulse = FMath::Max(1.f, InDamagePerPulse);
	PulseInterval = FMath::Clamp(InPulseInterval, 0.20f, 2.0f);
	WarmupRemaining = FMath::Max(0.f, InWarmupSeconds);
	ActiveRemaining = FMath::Max(0.3f, InActiveSeconds);
	LifetimeRemaining = WarmupRemaining + ActiveRemaining;
	TrapVariant = InTrapVariant;
	PulseAccumulator = 0.f;

	const float IndicatorScale = FMath::Clamp(Radius / 640.f, 0.26f, 0.98f);
	IndicatorMesh->SetRelativeScale3D(FVector(IndicatorScale, IndicatorScale, 1.f));
	if (UT66MiniVisualSubsystem* VisualSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr)
	{
		if (UTexture2D* TrapSprite = VisualSubsystem->LoadEffectTexture(T66MiniTrapCoreEffectName(TrapVariant)))
		{
			SpriteComponent->SetSprite(TrapSprite);
		}
	}
	UpdatePresentation();
}

void AT66MiniHazardTrap::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		UpdatePresentation();
		return;
	}

	LifetimeRemaining -= DeltaSeconds;
	if (LifetimeRemaining <= 0.f)
	{
		Destroy();
		return;
	}

	if (WarmupRemaining > 0.f)
	{
		WarmupRemaining = FMath::Max(0.f, WarmupRemaining - DeltaSeconds);
	}
	else
	{
		ActiveRemaining = FMath::Max(0.f, ActiveRemaining - DeltaSeconds);
		PulseAccumulator += DeltaSeconds;
		while (PulseAccumulator >= PulseInterval && ActiveRemaining > 0.f)
		{
			PulseAccumulator -= PulseInterval;
			ApplyPulse();
		}
	}

	UpdatePresentation();
}

void AT66MiniHazardTrap::ApplyPulse()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		AT66MiniPlayerPawn* PlayerPawn = PlayerController ? Cast<AT66MiniPlayerPawn>(PlayerController->GetPawn()) : nullptr;
		if (PlayerPawn && PlayerPawn->IsHeroAlive() && FVector::DistSquared2D(GetActorLocation(), PlayerPawn->GetActorLocation()) <= FMath::Square(Radius))
		{
			PlayerPawn->ApplyDamage(DamagePerPulse);
		}
	}

	const AT66MiniGameMode* MiniGameMode = World->GetAuthGameMode<AT66MiniGameMode>();
	const TArray<TObjectPtr<AT66MiniEnemyBase>>* LiveEnemies = MiniGameMode ? &MiniGameMode->GetLiveEnemies() : nullptr;
	if (LiveEnemies)
	{
		for (AT66MiniEnemyBase* Enemy : *LiveEnemies)
		{
			if (!Enemy || Enemy->IsEnemyDead())
			{
				continue;
			}

			if (FVector::DistSquared2D(GetActorLocation(), Enemy->GetActorLocation()) <= FMath::Square(Radius))
			{
				Enemy->ApplyDamage(DamagePerPulse);
			}
		}
	}

	if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
	{
		const FVector PulseScale(FMath::Clamp(Radius / 700.f, 0.24f, 1.0f), FMath::Clamp(Radius / 700.f, 0.24f, 1.0f), 1.f);
		if (UT66MiniVisualSubsystem* VisualSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr)
		{
			if (UTexture2D* PulseTexture = VisualSubsystem->LoadEffectTexture(TEXT("Trap_Pulse_Impact")))
			{
				VfxSubsystem->SpawnSpritePulse(World, GetActorLocation() + FVector(0.f, 0.f, 4.f), PulseScale, 0.14f, ResolveTrapTint(), PulseTexture, 0.60f);
			}
			else
			{
				VfxSubsystem->SpawnPulse(World, GetActorLocation() + FVector(0.f, 0.f, 4.f), PulseScale, 0.12f, ResolveTrapTint(), 0.55f);
			}
		}
		else
		{
			VfxSubsystem->SpawnPulse(World, GetActorLocation() + FVector(0.f, 0.f, 4.f), PulseScale, 0.12f, ResolveTrapTint(), 0.55f);
		}
	}
}

void AT66MiniHazardTrap::UpdatePresentation() const
{
	const FLinearColor BaseTint = ResolveTrapTint();
	const float LifeAlpha = LifetimeRemaining <= 1.8f
		? (0.30f + (0.70f * (FMath::Sin(GetWorld()->GetTimeSeconds() * 16.f) * 0.5f + 0.5f)))
		: (WarmupRemaining > 0.f ? 0.42f : 0.82f);
	const FLinearColor Tint(BaseTint.R, BaseTint.G, BaseTint.B, LifeAlpha);

	if (IndicatorMaterial)
	{
		IndicatorMaterial->SetVectorParameterValue(TEXT("Color"), Tint);
		IndicatorMaterial->SetVectorParameterValue(TEXT("BaseColor"), Tint);
		IndicatorMaterial->SetVectorParameterValue(TEXT("Tint"), Tint);
	}

	const bool bBlinkVisible = LifetimeRemaining > 1.2f || FMath::Fmod(GetWorld()->GetTimeSeconds() * 10.f, 1.f) > 0.35f;
	SpriteComponent->SetVisibility(bBlinkVisible, true);
}

FLinearColor AT66MiniHazardTrap::ResolveTrapTint() const
{
	switch (TrapVariant % 3)
	{
	case 1:
		return FLinearColor(1.0f, 0.58f, 0.18f, 0.36f);

	case 2:
		return FLinearColor(0.92f, 0.24f, 0.20f, 0.36f);

	default:
		return FLinearColor(1.0f, 0.84f, 0.26f, 0.34f);
	}
}

AT66MiniPlayerPawn* AT66MiniHazardTrap::FindClosestPlayerPawn(const bool bRequireAlive) const
{
	if (const AT66MiniGameMode* MiniGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AT66MiniGameMode>() : nullptr)
	{
		return MiniGameMode->FindClosestPlayerPawn(GetActorLocation(), bRequireAlive);
	}
	return nullptr;
}

void AT66MiniHazardTrap::OnRep_TrapPresentationState()
{
	IndicatorMesh->SetRelativeScale3D(FVector(FMath::Clamp(Radius / 640.f, 0.26f, 0.98f), FMath::Clamp(Radius / 640.f, 0.26f, 0.98f), 1.f));
	if (UT66MiniVisualSubsystem* VisualSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr)
	{
		if (UTexture2D* TrapSprite = VisualSubsystem->LoadEffectTexture(T66MiniTrapCoreEffectName(TrapVariant)))
		{
			SpriteComponent->SetSprite(TrapSprite);
		}
	}
	UpdatePresentation();
}
