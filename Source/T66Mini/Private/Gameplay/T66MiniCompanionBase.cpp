// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiniCompanionBase.h"

#include "Components/BillboardComponent.h"
#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniVisualSubsystem.h"
#include "Core/T66MiniVFXSubsystem.h"
#include "Data/T66MiniDataTypes.h"
#include "Engine/Texture2D.h"
#include "Gameplay/Components/T66MiniDirectionResolverComponent.h"
#include "Gameplay/Components/T66MiniShadowComponent.h"
#include "Gameplay/Components/T66MiniSpritePresentationComponent.h"
#include "Gameplay/T66MiniGameMode.h"
#include "Gameplay/T66MiniPlayerPawn.h"
#include "Save/T66MiniSaveSubsystem.h"

AT66MiniCompanionBase::AT66MiniCompanionBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	VisualPivotComponent = CreateDefaultSubobject<USceneComponent>(TEXT("VisualPivot"));
	VisualPivotComponent->SetupAttachment(SceneRoot);

	SpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	SpriteComponent->SetupAttachment(VisualPivotComponent);
	SpriteComponent->SetRelativeLocation(FVector(0.f, 0.f, 52.f));
	SpriteComponent->SetRelativeScale3D(FVector(2.15f, 2.15f, 2.15f));
	SpriteComponent->SetHiddenInGame(false, true);
	SpriteComponent->SetVisibility(true, true);

	SpritePresentationComponent = CreateDefaultSubobject<UT66MiniSpritePresentationComponent>(TEXT("SpritePresentation"));
	SpritePresentationComponent->BindSprite(SpriteComponent);
	SpritePresentationComponent->BindVisualRoot(VisualPivotComponent);

	DirectionResolverComponent = CreateDefaultSubobject<UT66MiniDirectionResolverComponent>(TEXT("DirectionResolver"));
	DirectionResolverComponent->BindPresentation(SpritePresentationComponent);

	ShadowComponent = CreateDefaultSubobject<UT66MiniShadowComponent>(TEXT("Shadow"));
	ShadowComponent->InitializeShadow(SceneRoot, FVector(0.f, 0.f, 1.2f), FVector(0.24f, 0.24f, 1.f), FLinearColor(0.f, 0.f, 0.f, 0.18f));
}

void AT66MiniCompanionBase::BeginPlay()
{
	Super::BeginPlay();

	if (!FollowTarget.IsValid())
	{
		FollowTarget = GetWorld() ? Cast<AT66MiniPlayerPawn>(GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr) : nullptr;
	}
}

void AT66MiniCompanionBase::InitializeCompanion(const FT66MiniCompanionDefinition& InDefinition, AT66MiniPlayerPawn* InFollowTarget)
{
	CompanionID = InDefinition.CompanionID;
	CompanionDisplayName = InDefinition.DisplayName;
	CompanionVisualID = InDefinition.VisualID.IsEmpty() ? InDefinition.DisplayName : InDefinition.VisualID;
	PlaceholderColor = InDefinition.PlaceholderColor;
	BaseHealingPerSecond = FMath::Max(1.0f, InDefinition.BaseHealingPerSecond);
	FollowOffsetX = InDefinition.FollowOffsetX;
	FollowOffsetY = InDefinition.FollowOffsetY;
	FollowTarget = InFollowTarget;
	UnityStagesCleared = 0;
	HealingPerSecond = BaseHealingPerSecond;
	HealPulseCooldownRemaining = FMath::FRandRange(0.08f, 0.28f);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UT66MiniSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UT66MiniSaveSubsystem>())
		{
			if (const UT66MiniDataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UT66MiniDataSubsystem>())
			{
				UnityStagesCleared = SaveSubsystem->GetCompanionUnityStagesCleared(CompanionID, DataSubsystem);
				HealingPerSecond = FMath::Max(BaseHealingPerSecond, SaveSubsystem->GetCompanionHealingPerSecond(CompanionID, DataSubsystem));
			}
		}
	}

	RefreshVisuals();
}

void AT66MiniCompanionBase::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AT66MiniPlayerPawn* PlayerPawn = FollowTarget.Get();
	if (!PlayerPawn)
	{
		PlayerPawn = GetWorld() ? Cast<AT66MiniPlayerPawn>(GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr) : nullptr;
		FollowTarget = PlayerPawn;
	}

	if (!PlayerPawn)
	{
		return;
	}

	const FVector DesiredLocation = PlayerPawn->GetActorLocation() + FVector(FollowOffsetX, FollowOffsetY, 0.f);
	const FVector NextLocation = FMath::VInterpTo(GetActorLocation(), DesiredLocation, DeltaSeconds, FollowMoveSpeed);

	if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
	{
		SetActorLocation(MiniGameMode->ClampPointToArena(NextLocation));
	}
	else
	{
		SetActorLocation(NextLocation);
	}

	HealPulseCooldownRemaining = FMath::Max(0.0f, HealPulseCooldownRemaining - DeltaSeconds);
	if (DirectionResolverComponent)
	{
		DirectionResolverComponent->ClearFacingWorldTarget();
	}

	if (PlayerPawn->GetCurrentHealth() < PlayerPawn->GetMaxHealth() && HealingPerSecond > 0.0f)
	{
		PlayerPawn->Heal(HealingPerSecond * DeltaSeconds);

		if (HealPulseCooldownRemaining <= 0.0f)
		{
			if (SpritePresentationComponent)
			{
				SpritePresentationComponent->TriggerAttackFrame(0.10f);
			}

			if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
			{
				const FLinearColor HealTint = UnityStagesCleared >= UT66MiniSaveSubsystem::CompanionUnityTierHyperStages
					? FLinearColor(0.56f, 0.96f, 0.74f, 0.32f)
					: FLinearColor(0.46f, 0.86f, 0.66f, 0.28f);
				VfxSubsystem->SpawnPulse(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 18.f), FVector(0.14f, 0.14f, 1.f), 0.08f, HealTint, 0.48f);
				VfxSubsystem->SpawnPulse(GetWorld(), PlayerPawn->GetActorLocation() + FVector(0.f, 0.f, 16.f), FVector(0.24f, 0.24f, 1.f), 0.12f, HealTint, 0.42f);
				VfxSubsystem->PlayPickupSfx(this, 0.08f, 1.18f);
			}

			HealPulseCooldownRemaining = 0.55f;
		}
	}
}

void AT66MiniCompanionBase::RefreshVisuals()
{
	UT66MiniVisualSubsystem* VisualSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
	if (!VisualSubsystem)
	{
		return;
	}

	UTexture2D* IdleTextureRight = VisualSubsystem->LoadCompanionAnimationTexture(CompanionVisualID, TEXT("Idle_R"));
	UTexture2D* WalkATextureRight = VisualSubsystem->LoadCompanionAnimationTexture(CompanionVisualID, TEXT("WalkA_R"));
	UTexture2D* WalkBTextureRight = VisualSubsystem->LoadCompanionAnimationTexture(CompanionVisualID, TEXT("WalkB_R"));
	UTexture2D* WalkCTextureRight = VisualSubsystem->LoadCompanionAnimationTexture(CompanionVisualID, TEXT("WalkC_R"));
	UTexture2D* AttackTextureRight = VisualSubsystem->LoadCompanionAnimationTexture(CompanionVisualID, TEXT("Attack_R"));
	UTexture2D* IdleTextureLeft = VisualSubsystem->LoadCompanionAnimationTexture(CompanionVisualID, TEXT("Idle_L"));
	UTexture2D* WalkATextureLeft = VisualSubsystem->LoadCompanionAnimationTexture(CompanionVisualID, TEXT("WalkA_L"));
	UTexture2D* WalkBTextureLeft = VisualSubsystem->LoadCompanionAnimationTexture(CompanionVisualID, TEXT("WalkB_L"));
	UTexture2D* WalkCTextureLeft = VisualSubsystem->LoadCompanionAnimationTexture(CompanionVisualID, TEXT("WalkC_L"));
	UTexture2D* AttackTextureLeft = VisualSubsystem->LoadCompanionAnimationTexture(CompanionVisualID, TEXT("Attack_L"));
	StaticSpriteTexture = VisualSubsystem->LoadCompanionTexture(CompanionVisualID);

	UTexture2D* FallbackTexture = IdleTextureRight ? IdleTextureRight : StaticSpriteTexture.Get();
	if (!FallbackTexture)
	{
		return;
	}

	SpriteComponent->SetSprite(FallbackTexture);
	if (SpritePresentationComponent)
	{
		SpritePresentationComponent->SetAnimationSet(
			FallbackTexture,
			WalkATextureRight,
			WalkBTextureRight,
			WalkCTextureRight,
			AttackTextureRight ? AttackTextureRight : FallbackTexture,
			IdleTextureLeft,
			WalkATextureLeft,
			WalkBTextureLeft,
			WalkCTextureLeft,
			AttackTextureLeft ? AttackTextureLeft : IdleTextureLeft);
		SpritePresentationComponent->SetBaseScale(SpriteComponent->GetRelativeScale3D());
	}
}
