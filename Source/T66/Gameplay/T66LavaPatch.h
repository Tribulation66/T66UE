// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66LavaPatch.generated.h"

class AT66HeroBase;
class UBoxComponent;
class UMaterialInstanceDynamic;
class UPrimitiveComponent;
class UStaticMeshComponent;
class UTexture2D;
struct FHitResult;

/**
 * Thin ground patch that renders animated pixel lava without requiring a custom .uasset material.
 * The look is generated in C++ and fed into the project's existing unlit environment master.
 */
UCLASS(Blueprintable)
class T66_API AT66LavaPatch : public AActor
{
	GENERATED_BODY()

public:
	AT66LavaPatch();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> DamageBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Shape")
	float PatchSize = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Shape")
	float HoverHeight = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Shape")
	float CollisionHeight = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Shape")
	bool bSnapToGroundOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Animation", meta = (ClampMin = "16", ClampMax = "256"))
	int32 TextureResolution = 64;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Animation", meta = (ClampMin = "4", ClampMax = "64"))
	int32 AnimationFrames = 18;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Animation", meta = (ClampMin = "1.0", ClampMax = "60.0"))
	float AnimationFPS = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Animation", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float WarpSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Animation", meta = (ClampMin = "0.0", ClampMax = "1.5"))
	float WarpIntensity = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Animation", meta = (ClampMin = "0.01", ClampMax = "10.0"))
	float WarpCloseness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Animation")
	FVector2D FlowDir = FVector2D(1.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Animation", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float FlowSpeed = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Animation", meta = (ClampMin = "0.1", ClampMax = "30.0"))
	float UVScale = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Look", meta = (ClampMin = "1.0", ClampMax = "24.0"))
	float CellDensity = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Look", meta = (ClampMin = "1.0", ClampMax = "18.0"))
	float EdgeContrast = 7.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Look", meta = (ClampMin = "0.1", ClampMax = "6.0"))
	float Brightness = 2.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Look")
	FVector2D PatternOffset = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Look")
	FLinearColor CoreColor = FLinearColor(0.09f, 0.01f, 0.00f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Look")
	FLinearColor MidColor = FLinearColor(0.78f, 0.11f, 0.02f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Look")
	FLinearColor GlowColor = FLinearColor(1.00f, 0.47f, 0.08f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Gameplay")
	bool bDamageHero = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Gameplay", meta = (ClampMin = "1"))
	int32 DamagePerTick = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava|Gameplay", meta = (ClampMin = "0.05", ClampMax = "5.0"))
	float DamageIntervalSeconds = 0.5f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	UFUNCTION()
	void OnDamageBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnDamageBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void RefreshPatchLayout();
	void SnapOriginToGround();
	void EnsureVisualMaterial();
	void GenerateAnimationFrames();
	UTexture2D* BuildFrameTexture(int32 FrameIndex, int32 ClampedFrames, int32 Resolution, const FVector2D& EffectivePatternOffset) const;
	FLinearColor SampleLavaColor(const FVector2D& BaseUV, float Phase, const FVector2D& EffectivePatternOffset) const;
	void ApplyAnimationFrame(int32 FrameIndex);
	void ApplyDamageTick();
	void UpdateAnimationTickState();

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> LavaMID = nullptr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTexture2D>> GeneratedFrames;

	TWeakObjectPtr<AT66HeroBase> OverlappingHero;
	FTimerHandle DamageTimerHandle;
	float AnimationStartTimeSeconds = 0.f;
	int32 StartFrameOffset = 0;
	int32 CurrentFrameIndex = INDEX_NONE;
};
