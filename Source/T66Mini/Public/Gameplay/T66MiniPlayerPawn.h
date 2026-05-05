// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
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
enum class ET66MiniProjectileBehavior : uint8;

UCLASS()
class T66MINI_API AT66MiniPlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	AT66MiniPlayerPawn();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
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

	UFUNCTION(BlueprintCallable, Category = "Mini")
	bool TryActivateUltimate(const FVector& TargetLocation);

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
	FName GetSelectedCompanionID() const { return SelectedCompanionID; }

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

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	float GetUltimateCooldownRemaining() const { return UltimateCooldownRemaining; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	float GetUltimateCooldownDuration() const { return UltimateCooldownDuration; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	bool IsUltimateReady() const { return UltimateCooldownRemaining <= 0.f; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	bool HasEnduranceCheatUsedThisWave() const { return bEnduranceCheatUsedThisWave; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	bool HasQuickReviveReady() const { return bQuickReviveReady; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	ET66UltimateType GetUltimateType() const { return UltimateType; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	ET66PassiveType GetPassiveType() const { return PassiveType; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	FString GetUltimateLabel() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	FString GetPassiveLabel() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	const TArray<FName>& GetOwnedItemIDs() const { return OwnedItemIDs; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	const TArray<FName>& GetEquippedIdolIDs() const { return EquippedIdolIDs; }

	void HandleBasicAttackImpact(class AT66MiniEnemyBase* ImpactEnemy, const FVector& ImpactLocation);
	void HandleEnemyKilled(const class AT66MiniEnemyBase* Enemy);
	void HandleWaveStarted(int32 WaveIndex);
	void GrantQuickRevive();

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

	struct FQueuedBurst
	{
		FVector Location = FVector::ZeroVector;
		float Damage = 0.f;
		float Radius = 0.f;
		float DelayRemaining = 0.f;
		float DotTickDamage = 0.f;
		float DotDuration = 0.f;
		float StunDuration = 0.f;
		bool bSpawnTelegraph = false;
		FLinearColor Tint = FLinearColor::White;
	};

	struct FActiveAreaEffect
	{
		FVector Location = FVector::ZeroVector;
		float Radius = 0.f;
		float TickDamage = 0.f;
		float TickInterval = 0.5f;
		float RemainingDuration = 0.f;
		float TickAccumulator = 0.f;
		float StunDuration = 0.f;
		FLinearColor Tint = FLinearColor::White;
	};

	void ApplyItemDefinition(const struct FT66MiniItemDefinition& ItemDefinition);
	void ApplyLevelUpBonuses(int32 LevelsToApply);
	void InitializeFromMiniRun();
	void RefreshHeroPresentation();
	void RefreshEquippedIdolRuntime();
	void RefreshPickupMagnetProfile();
	void UpdateCameraAnchor();
	void UpdateCombat(float DeltaSeconds);
	void UpdateQueuedBursts(float DeltaSeconds);
	void UpdateAreaEffects(float DeltaSeconds);
	void FireBasicAttack();
	void FireProjectileTowardLocation(const FVector& TargetLocation, float Damage, ET66MiniProjectileBehavior Behavior, float Speed = 2400.f, float Radius = 180.f, int32 RemainingHits = 1, int32 RemainingBounces = 0, float DotTickDamage = 0.f, float DotTickInterval = 0.5f, float DotDuration = 0.f, float StunDuration = 0.f);
	void ApplyAreaDamage(const FVector& Center, float Radius, float Damage, float StunDuration = 0.f, float DotTickDamage = 0.f, float DotDuration = 0.f);
	void QueueBurst(const FVector& Location, float Damage, float Radius, float Delay, const FLinearColor& Tint, float DotTickDamage = 0.f, float DotDuration = 0.f, float StunDuration = 0.f, bool bSpawnTelegraph = true);
	void AddAreaEffect(const FVector& Location, float Radius, float TickDamage, float TickInterval, float Duration, const FLinearColor& Tint, float StunDuration = 0.f);
	float GetPassiveDamageMultiplierAgainst(const class AT66MiniEnemyBase* Enemy) const;
	float ConsumeOutgoingDamageScalar(const class AT66MiniEnemyBase* Enemy);
	void HandlePostDamageTaken(float DamageTaken);
	void HandlePassiveOnBasicHit(class AT66MiniEnemyBase* ImpactEnemy, const FVector& ImpactLocation, float DamageDealt);
	float GetUltimateBaseDamage() const;
	void TriggerIdolFollowUp(FEquippedIdolRuntime& IdolRuntime, class AT66MiniEnemyBase* ImpactEnemy, const FVector& ImpactLocation);
	void SpawnIdolImpactVfx(const FEquippedIdolRuntime& IdolRuntime, const FVector& ImpactLocation, float ScaleMultiplier = 1.f) const;
	void ApplyAoeIdolBurst(const FVector& ImpactLocation, float Damage, float Radius);
	class AT66MiniEnemyBase* FindClosestEnemyFromLocation(const FVector& Origin, const AActor* IgnoreActor, float MaxRange) const;
	class AT66MiniEnemyBase* FindBestTarget(float MaxRange) const;
	float GetNextLevelThreshold() const;
	float GetRuntimeTuningValue(const TCHAR* Key, float DefaultValue = 0.f) const;
	int32 GetRuntimeTuningInt(const TCHAR* Key, int32 DefaultValue = 0) const;

	UFUNCTION(Server, Unreliable)
	void ServerSetDesiredMoveLocation(const FVector& InDesiredLocation);

	UFUNCTION(Server, Unreliable)
	void ServerSetAimLocation(const FVector& InAimLocation);

	UFUNCTION(Server, Reliable)
	void ServerTryActivateUltimate(const FVector& TargetLocation);

	UFUNCTION()
	void OnRep_HeroConfiguration();

	UFUNCTION()
	void OnRep_RuntimeInventory();

	FVector DesiredMoveLocation = FVector::ZeroVector;
	TArray<FEquippedIdolRuntime> EquippedIdols;

	UPROPERTY(ReplicatedUsing = OnRep_HeroConfiguration)
	FName HeroID = NAME_None;

	UPROPERTY(Replicated)
	FName SelectedCompanionID = NAME_None;

	FString HeroDisplayName;

	UPROPERTY(Replicated)
	float BaseDamageStat = 4.f;

	UPROPERTY(Replicated)
	float BaseAttackSpeedStat = 2.f;

	UPROPERTY(Replicated)
	float BaseArmorStat = 2.f;

	float BaseLuckStat = 2.f;

	UPROPERTY(Replicated)
	float AttackRange = 1400.f;

	float MaterialGainMultiplier = 1.0f;
	float GoldGainMultiplier = 1.0f;

	UPROPERTY(Replicated)
	float MaxHealth = 100.f;

	UPROPERTY(Replicated)
	float CurrentHealth = 100.f;

	UPROPERTY(Replicated)
	float CritChance = 0.05f;

	float CritDamageMultiplier = 1.5f;

	UPROPERTY(Replicated)
	float PassiveRegenPerSecond = 0.f;

	UPROPERTY(Replicated)
	float EvasionChance = 0.f;

	UPROPERTY(Replicated)
	float LifeStealChance = 0.f;

	float DotDamageBonus = 0.f;
	float AoeRadiusBonus = 0.f;
	float BonusDamageMultiplier = 1.f;
	float TemporaryDamageMultiplier = 1.f;
	int32 BonusPierceCount = 0;
	int32 BonusBounceCount = 0;

	UPROPERTY(Replicated)
	int32 Materials = 0;

	UPROPERTY(Replicated)
	int32 Gold = 0;

	UPROPERTY(Replicated)
	float Experience = 0.f;

	UPROPERTY(Replicated)
	int32 HeroLevel = 1;

	int32 NextIdolProcIndex = 0;
	float AutoAttackCooldownRemaining = 0.f;
	float NextShotSoundTime = 0.f;

	UPROPERTY(Replicated)
	float UltimateCooldownRemaining = 0.f;

	UPROPERTY(Replicated)
	float UltimateCooldownDuration = 18.f;

	float PassiveBuffRemaining = 0.f;
	float PassiveSecondaryBuffRemaining = 0.f;
	int32 PassiveStacks = 0;
	int32 PassiveShotCounter = 0;

	UPROPERTY(Replicated)
	int32 CurrentWaveIndex = 1;

	UPROPERTY(Replicated)
	bool bEnduranceCheatUsedThisWave = false;

	UPROPERTY(Replicated)
	bool bQuickReviveReady = false;

	bool bChaosNextAttackBounces = false;
	bool bEvasiveNextAttackAppliesDot = false;
	TWeakObjectPtr<class AT66MiniEnemyBase> FocusTarget;

	UPROPERTY(Replicated)
	ET66UltimateType UltimateType = ET66UltimateType::None;

	UPROPERTY(Replicated)
	ET66PassiveType PassiveType = ET66PassiveType::None;

	TArray<FQueuedBurst> QueuedBursts;
	TArray<FActiveAreaEffect> ActiveAreaEffects;
	bool bInitializedFromRun = false;

	UPROPERTY(ReplicatedUsing = OnRep_RuntimeInventory)
	TArray<FName> EquippedIdolIDs;

	UPROPERTY(ReplicatedUsing = OnRep_RuntimeInventory)
	TArray<FName> OwnedItemIDs;

	UPROPERTY()
	TObjectPtr<UTexture2D> HeroProjectileTexture;

	static constexpr float HeartHealthUnit = 20.f;
};
