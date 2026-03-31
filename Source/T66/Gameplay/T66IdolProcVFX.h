// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "GameFramework/Actor.h"
#include "T66IdolProcVFX.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UTexture;

enum class ET66IdolVFXTexture : uint8
{
	Runes,
	Seals,
	Wall,
};

struct FT66IdolProcVFXRequest
{
	FName IdolID = NAME_None;
	ET66ItemRarity Rarity = ET66ItemRarity::Black;
	ET66AttackCategory Category = ET66AttackCategory::Pierce;
	FVector Start = FVector::ZeroVector;
	FVector End = FVector::ForwardVector * 100.f;
	FVector Impact = FVector::ZeroVector;
	FVector Center = FVector::ZeroVector;
	float Radius = 120.f;
	float Duration = 0.18f;
	float StartDelay = 0.f;
	TArray<FVector> ChainPositions;
	TWeakObjectPtr<AActor> FollowTarget;
};

struct FT66IdolVFXLayerMaterial
{
	ET66IdolVFXTexture Texture = ET66IdolVFXTexture::Runes;
	FLinearColor PrimaryColor = FLinearColor::White;
	FLinearColor SecondaryColor = FLinearColor(0.35f, 0.35f, 0.35f, 1.f);
	FLinearColor OutlineColor = FLinearColor::Black;
	FLinearColor TintColor = FLinearColor::White;
	float PixelGrid = 28.f;
	float DetailTiling = 2.f;
	float ScrollSpeed = 0.35f;
	float InnerRadius = 0.38f;
	float OuterRadius = 0.48f;
	float GlowStrength = 4.f;
	float OpacityBoost = 0.95f;
};

struct FT66IdolVFXLayerAnimState
{
	UStaticMeshComponent* Mesh = nullptr;
	UMaterialInstanceDynamic* MID = nullptr;
	FVector BaseLocation = FVector::ZeroVector;
	FRotator BaseRotation = FRotator::ZeroRotator;
	FVector BaseScale = FVector::OneVector;
	FVector WaveAmplitude = FVector::ZeroVector;
	FVector ScaleAmplitude = FVector::ZeroVector;
	FVector TravelOffset = FVector::ZeroVector;
	float RotationSpeed = 0.f;
	float Phase = 0.f;
	float AgeOffset = 0.f;
	float TravelAlphaExponent = 1.f;
};

UCLASS(NotBlueprintable)
class T66_API AT66IdolProcVFX : public AActor
{
	GENERATED_BODY()

public:
	AT66IdolProcVFX();

	void InitEffect(const FT66IdolProcVFXRequest& InRequest);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void ResolveAssets();
	void BuildEffect();
	void BuildPierceEffect();
	void BuildAOEEffect();
	void BuildBounceEffect();
	void BuildDOTEffect();
	void BuildFallbackEffect();
	void UpdateFollowTarget();
	void UpdateLayers(float ActiveElapsed, float ActiveAge01);

	UTexture* ResolveTexture(ET66IdolVFXTexture Texture) const;
	void AddLayer(
		const FName& LayerName,
		const FVector& RelativeLocation,
		const FRotator& RelativeRotation,
		const FVector& RelativeScale,
		const FT66IdolVFXLayerMaterial& MaterialParams,
		const FT66IdolVFXLayerAnimState& AnimTemplate,
		int32 SortPriority);
	void AddMeshLayer(
		const FName& LayerName,
		UStaticMesh* MeshAsset,
		const FVector& RelativeLocation,
		const FRotator& RelativeRotation,
		const FVector& RelativeScale,
		const FT66IdolVFXLayerAnimState& AnimTemplate);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY()
	TObjectPtr<UStaticMesh> PlaneMesh = nullptr;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> BaseMaterial = nullptr;

	UPROPERTY()
	TObjectPtr<UTexture> RunesTexture = nullptr;

	UPROPERTY()
	TObjectPtr<UTexture> SealsTexture = nullptr;

	UPROPERTY()
	TObjectPtr<UTexture> WallTexture = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> SpawnedMeshes;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> SpawnedMIDs;

	TArray<FT66IdolVFXLayerAnimState> LayerStates;
	FT66IdolProcVFXRequest Request;

	float LifetimeSeconds = 0.18f;
	float ElapsedSeconds = 0.f;
	bool bInitialized = false;
	bool bBuilt = false;
};
