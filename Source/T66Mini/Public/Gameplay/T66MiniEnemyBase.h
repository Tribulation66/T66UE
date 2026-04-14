// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66MiniDataTypes.h"
#include "GameFramework/Actor.h"
#include "T66MiniEnemyBase.generated.h"

class UBillboardComponent;
class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;
class UTexture2D;
class UT66MiniDirectionResolverComponent;
class UT66MiniHitFlashComponent;
class UT66MiniShadowComponent;
class UT66MiniSpritePresentationComponent;

UCLASS()
class T66MINI_API AT66MiniEnemyBase : public AActor
{
	GENERATED_BODY()

public:
	AT66MiniEnemyBase();

	virtual void Tick(float DeltaSeconds) override;

	void InitializeEnemy(
		FName InEnemyID,
		bool bInBoss,
		UTexture2D* InTexture,
		float InMaxHealth,
		float InMoveSpeed,
		float InTouchDamage,
		int32 InMaterialDrop,
		float InExperienceDrop,
		float InCurrentHealth = -1.f,
		ET66MiniEnemyBehaviorProfile InBehaviorProfile = ET66MiniEnemyBehaviorProfile::Balanced);

	void ApplyDamage(float InDamage);
	void ApplyDot(float TickDamage, float TickInterval, float Duration);
	void ApplyStun(float DurationSeconds);

	bool IsEnemyDead() const { return bDead; }
	bool IsBossEnemy() const { return bIsBoss; }
	FName GetEnemyID() const { return EnemyID; }
	float GetCurrentHealth() const { return CurrentHealth; }
	float GetMaxHealth() const { return MaxHealth; }
	float GetMoveSpeed() const { return MoveSpeed; }
	float GetTouchDamage() const { return TouchDamage; }
	int32 GetMaterialDrop() const { return MaterialDrop; }
	float GetExperienceDrop() const { return ExperienceDrop; }

protected:
	virtual void BeginPlay() override;

private:
	struct FActiveDot
	{
		float TickDamage = 0.f;
		float TickInterval = 0.5f;
		float RemainingDuration = 0.f;
		float TickAccumulator = 0.f;
	};

	class AT66MiniPlayerPawn* GetMiniPlayerPawn() const;
	void HandleDeath();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBillboardComponent> SpriteComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> IndicatorMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UT66MiniSpritePresentationComponent> SpritePresentationComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UT66MiniDirectionResolverComponent> DirectionResolverComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UT66MiniShadowComponent> ShadowComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UT66MiniHitFlashComponent> HitFlashComponent;

	FName EnemyID = NAME_None;
	bool bIsBoss = false;
	bool bDead = false;
	ET66MiniEnemyBehaviorProfile BehaviorProfile = ET66MiniEnemyBehaviorProfile::Balanced;
	float MaxHealth = 30.f;
	float CurrentHealth = 30.f;
	float MoveSpeed = 280.f;
	float TouchDamage = 10.f;
	int32 MaterialDrop = 4;
	float ExperienceDrop = 4.f;
	float NextTouchDamageTime = 0.f;
	float NextDamageFeedbackTime = 0.f;
	float ChargeCooldownRemaining = 0.f;
	float ChargeDurationRemaining = 0.f;
	float StunRemaining = 0.f;
	FVector ChargeDirection = FVector::ForwardVector;
	TArray<FActiveDot> ActiveDots;
};
