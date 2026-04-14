// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Components/T66MiniSpritePresentationComponent.h"

#include "Components/BillboardComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Actor.h"

namespace
{
	constexpr float T66MiniMoveThresholdSq = 25.f;
	constexpr float T66MiniMoveBlendInterpSpeed = 8.5f;
	constexpr float T66MiniWalkCycleRate = 8.75f;
}

UT66MiniSpritePresentationComponent::UT66MiniSpritePresentationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UT66MiniSpritePresentationComponent::BindSprite(UBillboardComponent* InSprite)
{
	BoundSprite = InSprite;
	if (BoundSprite)
	{
		BaseSpriteLocation = BoundSprite->GetRelativeLocation();
		const FVector ExistingScale = BoundSprite->GetRelativeScale3D();
		BaseScale = FVector(FMath::Abs(ExistingScale.X), FMath::Abs(ExistingScale.Y), FMath::Abs(ExistingScale.Z));
	}
}

void UT66MiniSpritePresentationComponent::BindLowerBodySprite(UBillboardComponent* InSprite)
{
	BoundLowerBodySprite = InSprite;
	if (BoundLowerBodySprite)
	{
		BaseLowerBodySpriteLocation = BoundLowerBodySprite->GetRelativeLocation();
		const FVector ExistingScale = BoundLowerBodySprite->GetRelativeScale3D();
		LowerBodyBaseScale = FVector(FMath::Abs(ExistingScale.X), FMath::Abs(ExistingScale.Y), FMath::Abs(ExistingScale.Z));
	}
}

void UT66MiniSpritePresentationComponent::BindVisualRoot(USceneComponent* InVisualRoot)
{
	BoundVisualRoot = InVisualRoot;
	if (BoundVisualRoot)
	{
		BaseVisualRootLocation = BoundVisualRoot->GetRelativeLocation();
		BaseVisualRootRotation = BoundVisualRoot->GetRelativeRotation();
		BaseVisualRootScale = BoundVisualRoot->GetRelativeScale3D();
	}
}

void UT66MiniSpritePresentationComponent::SetAnimationSet(
	UTexture2D* InIdleTextureRight,
	UTexture2D* InWalkATextureRight,
	UTexture2D* InWalkBTextureRight,
	UTexture2D* InWalkCTextureRight,
	UTexture2D* InAttackTextureRight,
	UTexture2D* InIdleTextureLeft,
	UTexture2D* InWalkATextureLeft,
	UTexture2D* InWalkBTextureLeft,
	UTexture2D* InWalkCTextureLeft,
	UTexture2D* InAttackTextureLeft)
{
	IdleTextureRight = InIdleTextureRight;
	WalkATextureRight = InWalkATextureRight;
	WalkBTextureRight = InWalkBTextureRight;
	WalkCTextureRight = InWalkCTextureRight;
	AttackTextureRight = InAttackTextureRight;
	IdleTextureLeft = InIdleTextureLeft;
	WalkATextureLeft = InWalkATextureLeft;
	WalkBTextureLeft = InWalkBTextureLeft;
	WalkCTextureLeft = InWalkCTextureLeft;
	AttackTextureLeft = InAttackTextureLeft;
	UpdateDisplayedTexture(ResolveCurrentFrameTexture());
}

void UT66MiniSpritePresentationComponent::SetLowerBodyAnimationSet(
	UTexture2D* InIdleTextureRight,
	UTexture2D* InWalkATextureRight,
	UTexture2D* InWalkBTextureRight,
	UTexture2D* InWalkCTextureRight,
	UTexture2D* InIdleTextureLeft,
	UTexture2D* InWalkATextureLeft,
	UTexture2D* InWalkBTextureLeft,
	UTexture2D* InWalkCTextureLeft)
{
	LowerBodyIdleTextureRight = InIdleTextureRight;
	LowerBodyWalkATextureRight = InWalkATextureRight;
	LowerBodyWalkBTextureRight = InWalkBTextureRight;
	LowerBodyWalkCTextureRight = InWalkCTextureRight;
	LowerBodyIdleTextureLeft = InIdleTextureLeft;
	LowerBodyWalkATextureLeft = InWalkATextureLeft;
	LowerBodyWalkBTextureLeft = InWalkBTextureLeft;
	LowerBodyWalkCTextureLeft = InWalkCTextureLeft;
	UpdateLowerBodyTexture(ResolveCurrentLowerBodyFrameTexture());
}

void UT66MiniSpritePresentationComponent::TriggerAttackFrame(const float DurationSeconds)
{
	AttackFrameRemaining = FMath::Max(AttackFrameRemaining, FMath::Max(0.05f, DurationSeconds));
}

void UT66MiniSpritePresentationComponent::SetBaseScale(const FVector& InScale)
{
	BaseScale = FVector(FMath::Abs(InScale.X), FMath::Abs(InScale.Y), FMath::Abs(InScale.Z));
	LowerBodyBaseScale = BaseScale;

	if (BoundSprite)
	{
		const float FacingSign = HasDirectionalTextures() ? 1.f : CurrentFacingSign;
		BoundSprite->SetRelativeScale3D(FVector(BaseScale.X * FacingSign, BaseScale.Y, BaseScale.Z));
	}

	if (BoundLowerBodySprite)
	{
		const float FacingSign = HasDirectionalLowerBodyTextures() ? 1.f : CurrentFacingSign;
		BoundLowerBodySprite->SetRelativeScale3D(FVector(LowerBodyBaseScale.X * FacingSign, LowerBodyBaseScale.Y, LowerBodyBaseScale.Z));
	}
}

void UT66MiniSpritePresentationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AActor* Owner = GetOwner())
	{
		LastOwnerLocation = Owner->GetActorLocation();
		bHasLastOwnerLocation = true;
	}
}

void UT66MiniSpritePresentationComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!BoundSprite)
	{
		return;
	}

	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const FVector CurrentLocation = Owner->GetActorLocation();
	if (!bHasLastOwnerLocation)
	{
		LastOwnerLocation = CurrentLocation;
		bHasLastOwnerLocation = true;
	}

	const FVector Delta = CurrentLocation - LastOwnerLocation;
	AttackFrameRemaining = FMath::Max(0.f, AttackFrameRemaining - DeltaTime);
	ApplyProceduralLocomotion(DeltaTime, Delta);
	UpdateDisplayedTexture(ResolveCurrentFrameTexture());
	UpdateLowerBodyTexture(ResolveCurrentLowerBodyFrameTexture());
	LastOwnerLocation = CurrentLocation;
}

void UT66MiniSpritePresentationComponent::ApplyFacingSign(const float InFacingSign)
{
	CurrentFacingSign = InFacingSign < 0.f ? -1.f : 1.f;

	if (BoundSprite)
	{
		const float AppliedScaleX = HasDirectionalTextures() ? BaseScale.X : (BaseScale.X * CurrentFacingSign);
		BoundSprite->SetRelativeScale3D(FVector(AppliedScaleX, BaseScale.Y, BaseScale.Z));
		UpdateDisplayedTexture(ResolveCurrentFrameTexture());
	}

	if (BoundLowerBodySprite)
	{
		const float AppliedScaleX = HasDirectionalLowerBodyTextures() ? LowerBodyBaseScale.X : (LowerBodyBaseScale.X * CurrentFacingSign);
		BoundLowerBodySprite->SetRelativeScale3D(FVector(AppliedScaleX, LowerBodyBaseScale.Y, LowerBodyBaseScale.Z));
		UpdateLowerBodyTexture(ResolveCurrentLowerBodyFrameTexture());
	}
}

UTexture2D* UT66MiniSpritePresentationComponent::ResolveDirectionalTexture(UTexture2D* RightTexture, UTexture2D* LeftTexture) const
{
	if (CurrentFacingSign < 0.f)
	{
		return LeftTexture ? LeftTexture : RightTexture;
	}

	return RightTexture ? RightTexture : LeftTexture;
}

UTexture2D* UT66MiniSpritePresentationComponent::ResolveCurrentFrameTexture() const
{
	if (AttackFrameRemaining > KINDA_SMALL_NUMBER)
	{
		if (UTexture2D* AttackTexture = ResolveDirectionalTexture(AttackTextureRight, AttackTextureLeft))
		{
			return AttackTexture;
		}
	}

	const bool bHasWalkCycle = WalkATextureRight || WalkATextureLeft || WalkBTextureRight || WalkBTextureLeft || WalkCTextureRight || WalkCTextureLeft;
	if (MoveBlend > 0.15f && bHasWalkCycle)
	{
		const int32 WalkFrameIndex = FMath::FloorToInt(WalkCyclePhase) % 4;
		switch (WalkFrameIndex)
		{
		case 0:
			if (UTexture2D* Frame = ResolveDirectionalTexture(WalkATextureRight, WalkATextureLeft))
			{
				return Frame;
			}
			break;
		case 1:
			if (UTexture2D* Frame = ResolveDirectionalTexture(WalkBTextureRight, WalkBTextureLeft))
			{
				return Frame;
			}
			break;
		case 2:
			if (UTexture2D* Frame = ResolveDirectionalTexture(WalkCTextureRight, WalkCTextureLeft))
			{
				return Frame;
			}
			break;
		default:
			if (UTexture2D* Frame = ResolveDirectionalTexture(WalkBTextureRight, WalkBTextureLeft))
			{
				return Frame;
			}
			break;
		}
	}

	if (UTexture2D* IdleTexture = ResolveDirectionalTexture(IdleTextureRight, IdleTextureLeft))
	{
		return IdleTexture;
	}

	if (UTexture2D* WalkTexture = ResolveDirectionalTexture(WalkATextureRight, WalkATextureLeft))
	{
		return WalkTexture;
	}

	if (UTexture2D* WalkTexture = ResolveDirectionalTexture(WalkBTextureRight, WalkBTextureLeft))
	{
		return WalkTexture;
	}

	return ResolveDirectionalTexture(WalkCTextureRight, WalkCTextureLeft);
}

UTexture2D* UT66MiniSpritePresentationComponent::ResolveCurrentLowerBodyFrameTexture() const
{
	const bool bHasWalkCycle = LowerBodyWalkATextureRight || LowerBodyWalkATextureLeft
		|| LowerBodyWalkBTextureRight || LowerBodyWalkBTextureLeft
		|| LowerBodyWalkCTextureRight || LowerBodyWalkCTextureLeft;
	if (MoveBlend > 0.15f && bHasWalkCycle)
	{
		const int32 WalkFrameIndex = FMath::FloorToInt(WalkCyclePhase) % 4;
		switch (WalkFrameIndex)
		{
		case 0:
			if (UTexture2D* Frame = ResolveDirectionalTexture(LowerBodyWalkATextureRight, LowerBodyWalkATextureLeft))
			{
				return Frame;
			}
			break;
		case 1:
			if (UTexture2D* Frame = ResolveDirectionalTexture(LowerBodyWalkBTextureRight, LowerBodyWalkBTextureLeft))
			{
				return Frame;
			}
			break;
		case 2:
			if (UTexture2D* Frame = ResolveDirectionalTexture(LowerBodyWalkCTextureRight, LowerBodyWalkCTextureLeft))
			{
				return Frame;
			}
			break;
		default:
			if (UTexture2D* Frame = ResolveDirectionalTexture(LowerBodyWalkBTextureRight, LowerBodyWalkBTextureLeft))
			{
				return Frame;
			}
			break;
		}
	}

	return ResolveDirectionalTexture(LowerBodyIdleTextureRight, LowerBodyIdleTextureLeft);
}

bool UT66MiniSpritePresentationComponent::HasDirectionalTextures() const
{
	return IdleTextureLeft || WalkATextureLeft || WalkBTextureLeft || WalkCTextureLeft || AttackTextureLeft;
}

bool UT66MiniSpritePresentationComponent::HasDirectionalLowerBodyTextures() const
{
	return LowerBodyIdleTextureLeft || LowerBodyWalkATextureLeft || LowerBodyWalkBTextureLeft || LowerBodyWalkCTextureLeft;
}

void UT66MiniSpritePresentationComponent::UpdateDisplayedTexture(UTexture2D* DesiredTexture)
{
	if (!BoundSprite || !DesiredTexture || DisplayedTexture == DesiredTexture)
	{
		return;
	}

	DisplayedTexture = DesiredTexture;
	BoundSprite->SetSprite(DesiredTexture);
}

void UT66MiniSpritePresentationComponent::UpdateLowerBodyTexture(UTexture2D* DesiredTexture)
{
	if (!BoundLowerBodySprite || !DesiredTexture || DisplayedLowerBodyTexture == DesiredTexture)
	{
		return;
	}

	DisplayedLowerBodyTexture = DesiredTexture;
	BoundLowerBodySprite->SetSprite(DesiredTexture);
}

void UT66MiniSpritePresentationComponent::ApplyProceduralLocomotion(const float DeltaTime, const FVector& FrameDelta)
{
	const FVector Velocity = bHasLastOwnerLocation && DeltaTime > KINDA_SMALL_NUMBER
		? (FrameDelta / DeltaTime)
		: FVector::ZeroVector;
	const float Speed2D = Velocity.Size2D();
	const float MoveAlpha = FMath::Clamp(Speed2D / 430.f, 0.f, 1.f);
	const float TargetMoveBlend = Speed2D * Speed2D > T66MiniMoveThresholdSq ? MoveAlpha : 0.f;
	MoveBlend = FMath::FInterpTo(MoveBlend, TargetMoveBlend, DeltaTime, T66MiniMoveBlendInterpSpeed);

	if (MoveBlend > KINDA_SMALL_NUMBER)
	{
		WalkCyclePhase += DeltaTime * T66MiniWalkCycleRate * (0.75f + (MoveBlend * 0.50f));
	}
	else
	{
		WalkCyclePhase = 0.f;
	}

	if (BoundVisualRoot)
	{
		BoundVisualRoot->SetRelativeLocation(BaseVisualRootLocation);
		BoundVisualRoot->SetRelativeRotation(BaseVisualRootRotation);
		BoundVisualRoot->SetRelativeScale3D(BaseVisualRootScale);
	}

	if (BoundSprite)
	{
		BoundSprite->SetRelativeLocation(BaseSpriteLocation);
		const float AppliedScaleX = HasDirectionalTextures() ? BaseScale.X : (BaseScale.X * CurrentFacingSign);
		BoundSprite->SetRelativeScale3D(FVector(AppliedScaleX, BaseScale.Y, BaseScale.Z));
	}

	if (BoundLowerBodySprite)
	{
		BoundLowerBodySprite->SetRelativeLocation(BaseLowerBodySpriteLocation);
		const float AppliedScaleX = HasDirectionalLowerBodyTextures() ? LowerBodyBaseScale.X : (LowerBodyBaseScale.X * CurrentFacingSign);
		BoundLowerBodySprite->SetRelativeScale3D(FVector(AppliedScaleX, LowerBodyBaseScale.Y, LowerBodyBaseScale.Z));
	}
}
