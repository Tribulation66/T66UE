// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66HeroDOTAttackVFX.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class USceneComponent;
class UStaticMeshComponent;

UCLASS(NotBlueprintable)
class T66_API AT66HeroDOTAttackVFX : public AActor
{
	GENERATED_BODY()

public:
	AT66HeroDOTAttackVFX();

	void SetPaletteOverride(const FLinearColor& InTint, const FLinearColor& InPrimary, const FLinearColor& InSecondary, const FLinearColor& InOutline, float InGlowStrength, FName InPaletteMode = NAME_None);
	void InitEffect(AActor* InFollowTarget, const FVector& InLocation, float InDuration, float InRadius, const FLinearColor& InTint, int32 InDebugRequestId = INDEX_NONE, FName InDebugHeroID = NAME_None);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void ApplyEffectTransforms();
	void UpdateMaterialParams(float Age01) const;
	void LogConfiguredBegin() const;
	void ResolveRuntimePalette(FLinearColor& OutTintColor, FLinearColor& OutPrimaryColor, FLinearColor& OutSecondaryColor, FLinearColor& OutOutlineColor, float& OutGlowStrength, FString& OutPaletteMode) const;
	FVector ResolveAnchorLocation() const;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> GroundMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> AuraMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> SealMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> PulseMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> BaseMaterial = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> GroundMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> AuraMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> SealMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PulseMID = nullptr;

	TWeakObjectPtr<AActor> FollowTarget;
	FVector EffectLocation = FVector::ZeroVector;
	FLinearColor TintColor = FLinearColor::White;
	float BaseRadius = 80.f;
	float ElapsedSeconds = 0.f;
	float LifetimeSeconds = 1.2f;
	int32 DebugRequestId = INDEX_NONE;
	FName DebugHeroID = NAME_None;
	FLinearColor OverrideTintColor = FLinearColor::White;
	FLinearColor OverridePrimaryColor = FLinearColor::White;
	FLinearColor OverrideSecondaryColor = FLinearColor(0.35f, 0.35f, 0.35f, 1.f);
	FLinearColor OverrideOutlineColor = FLinearColor::Black;
	float OverrideGlowStrength = 0.f;
	FName OverridePaletteMode = NAME_None;
	bool bUsePaletteOverride = false;
	bool bInitialized = false;
};
