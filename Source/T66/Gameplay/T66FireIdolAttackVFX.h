// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66FireIdolAttackVFX.generated.h"

class AZibraVDBActor;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class USceneComponent;
class UStaticMeshComponent;
class UZibraVDBVolume4;

enum class ET66FireIdolTestCandidate : uint8
{
	Blender3D = 0,
	ZibraShockwave = 1,
};

UCLASS(NotBlueprintable)
class T66_API AT66FireIdolAttackVFX : public AActor
{
	GENERATED_BODY()

public:
	AT66FireIdolAttackVFX();

	void ConfigureCandidate(ET66FireIdolTestCandidate InCandidate);
	void InitEffect(const FVector& InLocation, float InRadius, const FLinearColor& InTint, int32 InDebugRequestId = INDEX_NONE, FName InDebugSourceID = NAME_None);

	static const TCHAR* GetCandidateName(ET66FireIdolTestCandidate Candidate);
	static ET66FireIdolTestCandidate SanitizeCandidate(int32 RawValue, bool& bOutWasClamped);
	static bool PreflightCandidate(ET66FireIdolTestCandidate Candidate, FString& OutBindingSummary, FString& OutFailureReason);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool SetupCandidate(FString& OutFailureReason);
	bool SetupBlenderProxy(FString& OutFailureReason);
	bool SetupZibraShockwave(FString& OutFailureReason);
	void HideAllCandidateVisuals();
	void ApplyBaseTransform();
	void UpdateBlenderProxy(float Age01);
	void UpdateProxyMaterial(UMaterialInstanceDynamic* Material, const FLinearColor& Tint, const FLinearColor& Primary, const FLinearColor& Secondary, const FLinearColor& Outline, float GlowStrength, float Age01) const;
	void DestroySpawnedZibraActor();
	void LogPlayResult(const TCHAR* EventName, bool bSuccess, const FString& Details, const TCHAR* FailurePoint = TEXT("None"), const TCHAR* FallbackPath = TEXT("None")) const;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> FireCoreMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> FireShellMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> FireLobeAMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> FireLobeBMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> SmokeMainMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> SmokeCapMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> BlenderProxyBaseMaterial = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FireCoreMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FireShellMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FireLobeAMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FireLobeBMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> SmokeMainMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> SmokeCapMID = nullptr;

	UPROPERTY()
	TObjectPtr<UZibraVDBVolume4> ZibraVolumeAsset = nullptr;

	TWeakObjectPtr<AZibraVDBActor> SpawnedZibraActor;

	FVector EffectLocation = FVector::ZeroVector;
	FLinearColor TintColor = FLinearColor::White;
	float BaseRadius = 120.f;
	float ElapsedSeconds = 0.f;
	float LifetimeSeconds = 1.0f;
	int32 DebugRequestId = INDEX_NONE;
	FName DebugSourceID = NAME_None;
	ET66FireIdolTestCandidate Candidate = ET66FireIdolTestCandidate::ZibraShockwave;
	bool bInitialized = false;
	bool bSetupSucceeded = false;
	bool bLoggedActive = false;
};
