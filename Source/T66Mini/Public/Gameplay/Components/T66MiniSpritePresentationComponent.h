// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "T66MiniSpritePresentationComponent.generated.h"

class UBillboardComponent;
class USceneComponent;
class UTexture2D;

UCLASS(ClassGroup=(Mini), meta=(BlueprintSpawnableComponent))
class T66MINI_API UT66MiniSpritePresentationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UT66MiniSpritePresentationComponent();

	void BindSprite(UBillboardComponent* InSprite);
	void BindLowerBodySprite(UBillboardComponent* InSprite);
	void BindVisualRoot(USceneComponent* InVisualRoot);
	void SetBaseScale(const FVector& InScale);
	void SetAnimationSet(
		UTexture2D* InIdleTextureRight,
		UTexture2D* InWalkATextureRight = nullptr,
		UTexture2D* InWalkBTextureRight = nullptr,
		UTexture2D* InWalkCTextureRight = nullptr,
		UTexture2D* InAttackTextureRight = nullptr,
		UTexture2D* InIdleTextureLeft = nullptr,
		UTexture2D* InWalkATextureLeft = nullptr,
		UTexture2D* InWalkBTextureLeft = nullptr,
		UTexture2D* InWalkCTextureLeft = nullptr,
		UTexture2D* InAttackTextureLeft = nullptr);
	void SetLowerBodyAnimationSet(
		UTexture2D* InIdleTextureRight,
		UTexture2D* InWalkATextureRight = nullptr,
		UTexture2D* InWalkBTextureRight = nullptr,
		UTexture2D* InWalkCTextureRight = nullptr,
		UTexture2D* InIdleTextureLeft = nullptr,
		UTexture2D* InWalkATextureLeft = nullptr,
		UTexture2D* InWalkBTextureLeft = nullptr,
		UTexture2D* InWalkCTextureLeft = nullptr);
	void TriggerAttackFrame(float DurationSeconds = 0.10f);
	const FVector& GetBaseScale() const { return BaseScale; }
	UBillboardComponent* GetBoundSprite() const { return BoundSprite.Get(); }
	void ApplyFacingSign(float InFacingSign);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UTexture2D* ResolveDirectionalTexture(UTexture2D* RightTexture, UTexture2D* LeftTexture) const;
	UTexture2D* ResolveCurrentFrameTexture() const;
	UTexture2D* ResolveCurrentLowerBodyFrameTexture() const;
	bool HasDirectionalTextures() const;
	bool HasDirectionalLowerBodyTextures() const;
	void UpdateDisplayedTexture(UTexture2D* DesiredTexture);
	void UpdateLowerBodyTexture(UTexture2D* DesiredTexture);
	void ApplyProceduralLocomotion(float DeltaTime, const FVector& FrameDelta);

	UPROPERTY()
	TObjectPtr<UBillboardComponent> BoundSprite;

	UPROPERTY()
	TObjectPtr<UBillboardComponent> BoundLowerBodySprite;

	UPROPERTY()
	TObjectPtr<USceneComponent> BoundVisualRoot;

	UPROPERTY()
	TObjectPtr<UTexture2D> DisplayedTexture;

	UPROPERTY()
	TObjectPtr<UTexture2D> DisplayedLowerBodyTexture;

	UPROPERTY()
	TObjectPtr<UTexture2D> IdleTextureRight;

	UPROPERTY()
	TObjectPtr<UTexture2D> WalkATextureRight;

	UPROPERTY()
	TObjectPtr<UTexture2D> WalkBTextureRight;

	UPROPERTY()
	TObjectPtr<UTexture2D> WalkCTextureRight;

	UPROPERTY()
	TObjectPtr<UTexture2D> AttackTextureRight;

	UPROPERTY()
	TObjectPtr<UTexture2D> IdleTextureLeft;

	UPROPERTY()
	TObjectPtr<UTexture2D> WalkATextureLeft;

	UPROPERTY()
	TObjectPtr<UTexture2D> WalkBTextureLeft;

	UPROPERTY()
	TObjectPtr<UTexture2D> WalkCTextureLeft;

	UPROPERTY()
	TObjectPtr<UTexture2D> AttackTextureLeft;

	UPROPERTY()
	TObjectPtr<UTexture2D> LowerBodyIdleTextureRight;

	UPROPERTY()
	TObjectPtr<UTexture2D> LowerBodyWalkATextureRight;

	UPROPERTY()
	TObjectPtr<UTexture2D> LowerBodyWalkBTextureRight;

	UPROPERTY()
	TObjectPtr<UTexture2D> LowerBodyWalkCTextureRight;

	UPROPERTY()
	TObjectPtr<UTexture2D> LowerBodyIdleTextureLeft;

	UPROPERTY()
	TObjectPtr<UTexture2D> LowerBodyWalkATextureLeft;

	UPROPERTY()
	TObjectPtr<UTexture2D> LowerBodyWalkBTextureLeft;

	UPROPERTY()
	TObjectPtr<UTexture2D> LowerBodyWalkCTextureLeft;

	FVector BaseScale = FVector::OneVector;
	FVector BaseSpriteLocation = FVector::ZeroVector;
	FVector LowerBodyBaseScale = FVector::OneVector;
	FVector BaseLowerBodySpriteLocation = FVector::ZeroVector;
	FVector BaseVisualRootLocation = FVector::ZeroVector;
	FVector BaseVisualRootScale = FVector::OneVector;
	FRotator BaseVisualRootRotation = FRotator::ZeroRotator;
	FVector LastOwnerLocation = FVector::ZeroVector;
	float AttackFrameRemaining = 0.f;
	float WalkCyclePhase = 0.f;
	float MoveBlend = 0.f;
	float CurrentFacingSign = 1.f;
	bool bHasLastOwnerLocation = false;
};
