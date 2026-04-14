// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66MiniCompanionBase.generated.h"

class UBillboardComponent;
class USceneComponent;
class UTexture2D;
class UT66MiniDirectionResolverComponent;
class UT66MiniShadowComponent;
class UT66MiniSpritePresentationComponent;
struct FT66MiniCompanionDefinition;

UCLASS()
class T66MINI_API AT66MiniCompanionBase : public AActor
{
	GENERATED_BODY()

public:
	AT66MiniCompanionBase();

	virtual void Tick(float DeltaSeconds) override;

	void InitializeCompanion(const FT66MiniCompanionDefinition& InDefinition, class AT66MiniPlayerPawn* InFollowTarget);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	FName GetCompanionID() const { return CompanionID; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	FString GetCompanionDisplayName() const { return CompanionDisplayName; }

protected:
	virtual void BeginPlay() override;

private:
	void RefreshVisuals();

	UPROPERTY(VisibleAnywhere, Category = "Mini")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Mini")
	TObjectPtr<USceneComponent> VisualPivotComponent;

	UPROPERTY(VisibleAnywhere, Category = "Mini")
	TObjectPtr<UBillboardComponent> SpriteComponent;

	UPROPERTY(VisibleAnywhere, Category = "Mini")
	TObjectPtr<UT66MiniSpritePresentationComponent> SpritePresentationComponent;

	UPROPERTY(VisibleAnywhere, Category = "Mini")
	TObjectPtr<UT66MiniDirectionResolverComponent> DirectionResolverComponent;

	UPROPERTY(VisibleAnywhere, Category = "Mini")
	TObjectPtr<UT66MiniShadowComponent> ShadowComponent;

	UPROPERTY()
	TObjectPtr<UTexture2D> StaticSpriteTexture;

	TWeakObjectPtr<class AT66MiniPlayerPawn> FollowTarget;
	FName CompanionID = NAME_None;
	FString CompanionDisplayName;
	FString CompanionVisualID;
	FLinearColor PlaceholderColor = FLinearColor(0.48f, 0.38f, 0.22f, 1.0f);
	float BaseHealingPerSecond = 5.0f;
	float HealingPerSecond = 5.0f;
	float FollowOffsetX = -145.0f;
	float FollowOffsetY = 110.0f;
	float FollowMoveSpeed = 5.8f;
	float HealPulseCooldownRemaining = 0.0f;
	int32 UnityStagesCleared = 0;
};
