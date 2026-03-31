// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "GameFramework/Actor.h"
#include "T66IdolVolumeAttackVFX.generated.h"

class AZibraVDBActor;
class USceneComponent;

UCLASS(NotBlueprintable)
class T66_API AT66IdolVolumeAttackVFX : public AActor
{
	GENERATED_BODY()

public:
	AT66IdolVolumeAttackVFX();

	void InitEffect(
		FName InIdolID,
		ET66AttackCategory InCategory,
		AActor* InFollowTarget,
		const FVector& InLocation,
		float InRadius,
		float InDuration,
		const FLinearColor& InTint,
		int32 InDebugRequestId = INDEX_NONE);

	static bool ShouldUseVolume(FName IdolID, ET66AttackCategory Category);
	static bool Preflight(FName IdolID, ET66AttackCategory Category, FString& OutBindingSummary, FString& OutFailureReason);
	static FString DescribeProfile(FName IdolID, ET66AttackCategory Category);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool SetupVolume(FString& OutFailureReason);
	FVector ResolveAnchorLocation() const;
	void ApplyBaseTransform();
	void DestroySpawnedZibraActor();
	void LogPlayResult(const TCHAR* EventName, bool bSuccess, const FString& Details, const TCHAR* FailurePoint = TEXT("None"), const TCHAR* FallbackPath = TEXT("None")) const;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	FName IdolID = NAME_None;
	ET66AttackCategory Category = ET66AttackCategory::AOE;
	TWeakObjectPtr<AActor> FollowTarget;
	TWeakObjectPtr<AZibraVDBActor> SpawnedZibraActor;

	FVector EffectLocation = FVector::ZeroVector;
	FLinearColor TintColor = FLinearColor::White;
	float BaseRadius = 120.f;
	float LifetimeSeconds = 1.0f;
	float ElapsedSeconds = 0.f;
	int32 DebugRequestId = INDEX_NONE;
	bool bInitialized = false;
	bool bSetupSucceeded = false;
	bool bLoggedActive = false;
};
