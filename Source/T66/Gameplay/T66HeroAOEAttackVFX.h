// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66HeroAOEAttackVFX.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class USceneComponent;
class UStaticMeshComponent;

UCLASS(NotBlueprintable)
class T66_API AT66HeroAOEAttackVFX : public AActor
{
	GENERATED_BODY()

public:
	AT66HeroAOEAttackVFX();

	void ConfigureFireFlipbook();
	void SetPaletteOverride(const FLinearColor& InTint, const FLinearColor& InPrimary, const FLinearColor& InSecondary, const FLinearColor& InOutline, float InGlowStrength, FName InPaletteMode = NAME_None);
	void InitEffect(const FVector& InLocation, float InRadius, const FLinearColor& InTint, int32 InDebugRequestId = INDEX_NONE, FName InDebugHeroID = NAME_None);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void ApplyEffectTransforms();
	void RefreshMaterials();
	void LogFireFlipbookBinding() const;
	void UpdateMaterialParams(float Age01) const;
	void LogConfiguredBegin() const;
	void ResolveRuntimePalette(FLinearColor& OutTintColor, FLinearColor& OutPrimaryColor, FLinearColor& OutSecondaryColor, FLinearColor& OutOutlineColor, float& OutGlowStrength, FString& OutPaletteMode) const;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> CoreMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> RingMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> EchoMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> CoreBaseMaterial = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CoreMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> RingMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> EchoMID = nullptr;

	FVector EffectLocation = FVector::ZeroVector;
	FLinearColor TintColor = FLinearColor::White;
	float BaseRadius = 120.f;
	float ElapsedSeconds = 0.f;
	float LifetimeSeconds = 0.26f;
	int32 DebugRequestId = INDEX_NONE;
	FName DebugHeroID = NAME_None;
	FLinearColor OverrideTintColor = FLinearColor::White;
	FLinearColor OverridePrimaryColor = FLinearColor::White;
	FLinearColor OverrideSecondaryColor = FLinearColor(0.35f, 0.35f, 0.35f, 1.f);
	FLinearColor OverrideOutlineColor = FLinearColor::Black;
	float OverrideGlowStrength = 0.f;
	FName OverridePaletteMode = NAME_None;
	bool bUseFireFlipbook = false;
	bool bUsePaletteOverride = false;
	bool bInitialized = false;
};
