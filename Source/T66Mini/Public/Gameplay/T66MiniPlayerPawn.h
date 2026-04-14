// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "T66MiniPlayerPawn.generated.h"

class UBillboardComponent;
class UCameraComponent;
class USceneComponent;
class USphereComponent;
class USpringArmComponent;
class UTexture2D;
class UT66MiniDirectionResolverComponent;
class UT66MiniHitFlashComponent;
class UT66MiniPickupMagnetComponent;
class UT66MiniShadowComponent;
class UT66MiniSpritePresentationComponent;

UCLASS()
class T66MINI_API AT66MiniPlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	AT66MiniPlayerPawn();

	virtual void Tick(float DeltaSeconds) override;
	bool AcquireItem(FName ItemID);

	UFUNCTION(BlueprintCallable, Category = "Mini")
	void SetDesiredMoveLocation(const FVector& InDesiredLocation);

	UFUNCTION(BlueprintCallable, Category = "Mini")
	void SetAimLocation(const FVector& InAimLocation);

	UFUNCTION(BlueprintCallable, Category = "Mini")
	void GainMaterials(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Mini")
	void GainGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Mini")
	bool SpendGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Mini")
	void GainExperience(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Mini")
	void Heal(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Mini")
	void GrantHeart(int32 HeartCount = 1, bool bHealToFull = true);

	UFUNCTION(BlueprintCallable, Category = "Mini")
	void ApplyDamage(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Mini")
	void HandleSuccessfulHit(float DamageDealt);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	int32 GetMaterials() const { return Materials; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	int32 GetGold() const { return Gold; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	float GetExperience() const { return Experience; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	int32 GetHeroLevel() const { return HeroLevel; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	bool IsHeroAlive() const { return CurrentHealth > 0.f; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	FName GetHeroID() const { return HeroID; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	FString GetHeroDisplayName() const { return HeroDisplayName; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	float GetDamageStat() const { return BaseDamageStat; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	float GetAttackSpeedStat() const { return BaseAttackSpeedStat; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	float GetArmorStat() const { return BaseArmorStat; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	float GetAttackRange() const { return AttackRange; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	float GetCritChance() const { return CritChance; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	float GetPassiveRegenPerSecond() const { return PassiveRegenPerSecond; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	float GetLifeStealChance() const { return LifeStealChance; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	float GetEvasionChance() const { return EvasionChance; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	float GetExperienceToNextLevel() const { return GetNextLevelThreshold(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	int32 GetMaxHearts() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	float GetHeartFill(int32 HeartIndex) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	float GetPickupMagnetRadius() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	float GetPickupMagnetPullSpeed() const;

	void HandleBasicAttackImpact(class AT66MiniEnemyBase* ImpactEnemy, const FVector& ImpactLocation);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float MoveSpeed = 900.f;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mini")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mini")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mini")
	TObjectPtr<USceneComponent> VisualPivotComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mini")
	TObjectPtr<UBillboardComponent> SpriteComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mini")
	TObjectPtr<UBillboardComponent> LegsSpriteComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mini")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mini")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mini")
	TObjectPtr<UT66MiniSpritePresentationComponent> SpritePresentationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mini")
	TObjectPtr<UT66MiniDirectionResolverComponent> DirectionResolverComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mini")
	TObjectPtr<UT66MiniShadowComponent> ShadowComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mini")
	TObjectPtr<UT66MiniHitFlashComponent> HitFlashComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mini")
	TObjectPtr<UT66MiniPickupMagnetComponent> PickupMagnetComponent;

private:
	struct FEquippedIdolRuntime
	{
		FName IdolID = NAME_None;
		FString Category;
		FString IconPath;
		float BaseDamage = 8.f;
		float BaseProperty = 1.f;
		float DamagePerLevel = 0.f;
		float CooldownRemaining = 0.f;
		TObjectPtr<UTexture2D> EffectTexture;
	};

	void ApplyItemDefinition(const struct FT66MiniItemDefinition& ItemDefinition);
	void ApplyLevelUpBonuses(int32 LevelsToApply);
	void InitializeFromMiniRun();
	void RefreshPickupMagnetProfile();
	void UpdateCameraAnchor();
	void UpdateCombat(float DeltaSeconds);
	void FireBasicAttack();
	void TriggerIdolFollowUp(FEquippedIdolRuntime& IdolRuntime, class AT66MiniEnemyBase* ImpactEnemy, const FVector& ImpactLocation);
	void SpawnIdolImpactVfx(const FEquippedIdolRuntime& IdolRuntime, const FVector& ImpactLocation, float ScaleMultiplier = 1.f) const;
	void ApplyAoeIdolBurst(const FVector& ImpactLocation, float Damage, float Radius);
	class AT66MiniEnemyBase* FindClosestEnemyFromLocation(const FVector& Origin, const AActor* IgnoreActor, float MaxRange) const;
	class AT66MiniEnemyBase* FindBestTarget(float MaxRange) const;
	float GetNextLevelThreshold() const;

	FVector DesiredMoveLocation = FVector::ZeroVector;
	TArray<FEquippedIdolRuntime> EquippedIdols;
	FName HeroID = NAME_None;
	FString HeroDisplayName;
	float BaseDamageStat = 4.f;
	float BaseAttackSpeedStat = 2.f;
	float BaseArmorStat = 2.f;
	float BaseLuckStat = 2.f;
	float AttackRange = 1400.f;
	float MaterialGainMultiplier = 1.0f;
	float MaxHealth = 100.f;
	float CurrentHealth = 100.f;
	float CritChance = 0.05f;
	float CritDamageMultiplier = 1.5f;
	float PassiveRegenPerSecond = 0.f;
	float EvasionChance = 0.f;
	float LifeStealChance = 0.f;
	float DotDamageBonus = 0.f;
	float AoeRadiusBonus = 0.f;
	float BonusDamageMultiplier = 1.f;
	int32 BonusPierceCount = 0;
	int32 BonusBounceCount = 0;
	int32 Materials = 0;
	int32 Gold = 0;
	float Experience = 0.f;
	int32 HeroLevel = 1;
	int32 NextIdolProcIndex = 0;
	float AutoAttackCooldownRemaining = 0.f;
	float NextShotSoundTime = 0.f;
	bool bInitializedFromRun = false;

	UPROPERTY()
	TObjectPtr<UTexture2D> HeroProjectileTexture;

	static constexpr float HeartHealthUnit = 20.f;
};
