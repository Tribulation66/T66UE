// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66CombatTargetTypes.h"
#include "T66CombatComponent.generated.h"

class AT66EnemyBase;
class AT66MobBase;
class AT66BossBase;
class AT66HeroProjectile;
class UT66RunStateSubsystem;
class UT66FloatingCombatTextSubsystem;
class UT66IdolManagerSubsystem;
class UT66WeaponManagerSubsystem;
class UNiagaraSystem;
struct FStreamableHandle;

DECLARE_LOG_CATEGORY_EXTERN(LogT66Combat, Log, All);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class T66_API UT66CombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UT66CombatComponent();

	/** Manual attack lock target (affects auto attacks only). */
	void SetLockedTarget(const FT66CombatTargetHandle& InTarget);
	void SetLockedTarget(AActor* InTarget);
	void ClearLockedTarget();
	AActor* GetLockedTarget() const { return LockedTarget.Actor.Get(); }
	const FT66CombatTargetHandle& GetLockedTargetHandle() const { return LockedTarget; }
	void SetAutoAttackSuppressed(bool bSuppressed) { bSuppressAutoAttack = bSuppressed; }
	bool IsAutoAttackSuppressed() const { return bSuppressAutoAttack; }
	void PerformScopedPiercingShot(const FVector& Start, const FVector& End);
#if !UE_BUILD_SHIPPING
	void PerformAutomationAutoAttackNow();
	bool DebugApplyHeadshotStunForAutomation(AActor* Target, bool bForce = false);
#endif

	/** Cooldown progress 0..1 (0 = just fired, 1 = ready). For UI cooldown bar below hero. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	float GetAutoAttackCooldownProgress() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AttackRange = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float FireIntervalSeconds = 1.f;

	/** Damage per shot (use high value for insta kill) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 DamagePerShot = 20;

	/** Base AoE splash radius for the slash attack (scales with ProjectileScaleMultiplier). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float SlashRadius = 300.f;

	/** Spawn death VFX: burst of red pixel particles at the given location. */
	void SpawnDeathVFX(const FVector& Location);

	/** Static: spawn a death burst at a world location (for enemies/bosses that don't own a CombatComponent). */
	static void SpawnDeathBurstAtLocation(UWorld* World, const FVector& Location, int32 NumParticles = 20, float BurstRadius = 80.f);

	void PerformUltimateSpearStorm(int32 UltimateDamage, const FVector& Start, const FVector& End);
	void PerformUltimateChainLightning(int32 UltimateDamage);
	void PerformUltimatePrecisionStrike(int32 UltimateDamage);
	void PerformUltimateFanTheHammer(int32 UltimateDamage);
	void PerformUltimateDeadeye(int32 UltimateDamage);
	void PerformUltimateDischarge(int32 UltimateDamage);
	void PerformUltimateJuiced();
	void PerformUltimateDeathSpiral(int32 UltimateDamage);
	void PerformUltimateShockwave(int32 UltimateDamage);
	void PerformUltimateTidalWave(int32 UltimateDamage);
	void PerformUltimateGoldRush(int32 UltimateDamage);
	void PerformUltimateMiasmaBomb(int32 UltimateDamage);
	void PerformUltimateRabidFrenzy();
	void PerformUltimateBlizzard(int32 UltimateDamage);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleInventoryChanged();

	void TryFire();
	void RecomputeFromRunState();
	bool TryApplyHeadshotStunToTargetHandle(const FT66CombatTargetHandle& TargetHandle, bool bForce = false);

	FTimerHandle FireTimerHandle;

	FT66CombatTargetHandle LockedTarget;

	// ---------------------------------------------------------------------------
	// Overlap-based target detection: a sphere component tracks enemies in range
	// so TryFire never needs to iterate all world actors.
	// ---------------------------------------------------------------------------

	/** Invisible sphere attached to the hero; generates overlap events for enemies entering/leaving attack range. */
	UPROPERTY(Transient)
	TObjectPtr<USphereComponent> RangeSphere;

	/** Live set of enemies/bosses currently overlapping the range sphere. Maintained by overlap callbacks. */
	TArray<TWeakObjectPtr<AActor>> EnemiesInRange;

	UFUNCTION()
	void OnRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Helper: find closest valid target in EnemiesInRange from a given location within MaxRangeSq.
	 *  Optionally exclude actors already in ExcludeSet (for bounce chains). */
	AActor* FindClosestEnemyInRange(const FVector& FromLocation, float MaxRangeSq,
		const TSet<AActor*>* ExcludeSet = nullptr) const;
	FT66CombatTargetHandle FindClosestTargetHandleInRange(const FVector& FromLocation, float MaxRangeSq, const TSet<FString>* ExcludeKeys = nullptr) const;

	/** Returns true if an actor is a valid auto-attack target (alive enemy/boss). */
	static bool IsValidAutoTarget(AActor* A);
	static bool IsValidTargetHandle(const FT66CombatTargetHandle& TargetHandle);
	bool IsTargetOnCompatibleTowerDamageFloor(AActor* Target, const FVector& DamageOrigin) const;
	static FString MakeTargetHandleKey(const FT66CombatTargetHandle& TargetHandle);
	FT66CombatTargetHandle MakeActorTargetHandle(AActor* Actor, ET66HitZoneType PreferredHitZone = ET66HitZoneType::Body) const;
	FT66CombatTargetHandle ResolveAutoAttackTargetHandle(AActor* Actor, bool bFavorLockedZone, class UT66RngSubsystem* RngSub) const;
	static FVector GetTargetAimPoint(const FT66CombatTargetHandle& TargetHandle);
	bool HasUnblockedAutoAttackPath(const FVector& FromLocation, const FT66CombatTargetHandle& TargetHandle) const;

	UPROPERTY()
	TObjectPtr<UT66RunStateSubsystem> CachedRunState;

	UPROPERTY(Transient)
	TObjectPtr<UT66FloatingCombatTextSubsystem> CachedFloatingCombatText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UT66IdolManagerSubsystem> CachedIdolManager = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UT66WeaponManagerSubsystem> CachedWeaponManager = nullptr;

	bool bHasCachedHeroData = false;
	FHeroData CachedHeroData;
	bool bHasCachedWeaponData = false;
	FWeaponData CachedWeaponData;

	struct FCachedIdolSlot
	{
		FName IdolID = NAME_None;
		FIdolData IdolData;
		ET66ItemRarity Rarity = ET66ItemRarity::Black;
		bool bValid = false;
	};

	TArray<FCachedIdolSlot> CachedIdolSlots;

	TSharedPtr<FStreamableHandle> CombatPresentationAssetsLoadHandle;

	/** Legacy Niagara system (round ball). Asset: /Game/VFX/VFX_Attack1 */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|VFX")
	TSoftObjectPtr<UNiagaraSystem> SlashVFXNiagara;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> CachedSlashVFXNiagara = nullptr;

	/** Retro pixel Niagara system (square particle). Asset: /Game/VFX/NS_PixelParticle */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|VFX")
	TSoftObjectPtr<UNiagaraSystem> PixelVFXNiagara;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> CachedPixelVFXNiagara = nullptr;

	void PrimeCombatPresentationAssetsAsync();
	void HandleCombatPresentationAssetsLoaded();
	void WarmupVFXSystems();

	/** Returns the active VFX system: pixel if available, otherwise legacy. */
	UNiagaraSystem* GetActiveVFXSystem() const;

	void PlayCombatAudioEvent(FName EventID, const FVector& Location) const;
	void PlayHeroAttackSfx(const FName& HeroID, ET66AttackCategory AttackCategory, const FVector& Location) const;

	/** Apply damage to a single actor. EventType for floating text. SourceID for damage log (NAME_None = AutoAttack, or IdolID for idol/DOT). */
	void ApplyDamageToTargetHandle(const FT66CombatTargetHandle& TargetHandle, int32 DamageAmount, FName EventType = NAME_None, FName SourceID = NAME_None, FName RangeEventForHero = NAME_None);
	void ApplyDamageToActor(AActor* Target, int32 DamageAmount, FName EventType = NAME_None, FName SourceID = NAME_None, FName RangeEventForHero = NAME_None);

	void SpawnSlashVFX(const FVector& Location, float Radius, const FLinearColor& Color);
	void SpawnPierceVFX(const FVector& Start, const FVector& End, const FLinearColor& Color);
	void SpawnHeroOnePierceVFX(const FVector& Start, const FVector& End, const FVector& ImpactLocation, const FLinearColor& Color);
	void SpawnArthurUltimateSwordVFX(const FVector& Start, const FVector& End);
	void SpawnBounceVFX(const TArray<FVector>& ChainPositions, const FLinearColor& Color);
	// Bounce projectile-travel presentation: stage one visible moving link at a time along the
	// resolved chain (hero attack origin -> primary, then primary -> next target), launching the
	// next link from the previous visual projectile's arrival callback. Visual-only; Bounce damage and the
	// per-link impact contexts stay authoritative in PerformBounce.
	void StageBounceProjectileChain(const TArray<FVector>& ChainPositions, const FLinearColor& Color, float ProjectileSpeed, float ScaleMultiplier, UNiagaraSystem* CarrierSystem, float CarrierVisualScale, float MinLinkTravelSeconds, float CarrierPlaybackSeconds);
	void SpawnBounceChainLinkSequential(const TArray<FVector>& ChainPositions, const FLinearColor& Color, float ProjectileSpeed, float ScaleMultiplier, int32 LinkIndex, UNiagaraSystem* CarrierSystem, float CarrierVisualScale, float MinLinkTravelSeconds, float CarrierPlaybackSeconds);
	AT66HeroProjectile* SpawnBounceLinkProjectile(const FVector& Start, const FVector& End, const FLinearColor& Color, float ProjectileSpeed, float ScaleMultiplier, int32 LinkIndex, int32 LinkCount, UNiagaraSystem* CarrierSystem, float CarrierVisualScale, float CarrierPlaybackSeconds);
	void SpawnDOTVFX(const FVector& Location, float Duration, float Radius, const FLinearColor& Color);
	// Reusable single-link visual-only mover: spawns one AT66HeroProjectile that travels
	// Start->End over TravelSeconds using the proven Bounce timed-travel seam, then fires its
	// arrival callback and self-destructs. Carries NO damage/collision authority. Returns the
	// projectile so the caller can wire an arrival callback, or nullptr if spawning failed.
	// When CarrierSystem is non-null, the authored Niagara carrier is attached to the moving
	// projectile (exactly as the Bounce link carrier seam) and IS the visible silhouette; the
	// temporary profile meshes are hidden. The carrier only transports the authored silhouette.
	AT66HeroProjectile* SpawnVisualTravelProjectile(const FVector& Start, const FVector& End, const FLinearColor& Color, FName ProfileID, float ScaleMultiplier, float TravelSeconds, UNiagaraSystem* CarrierSystem = nullptr, float CarrierVisualScale = 1.f);
	// Temporary Hero 1 DOT placeholder: spawn target-following sphere applicator markers that
	// persist for the DOT duration. Visual-only — the single authoritative DOT payload stays in
	// UT66RunStateSubsystem::ApplyDOT; markers never multiply or own DOT damage.
	void SpawnDOTApplicatorMarkers(AActor* FollowTarget, const FLinearColor& Color, float Duration, float MarkerScale);
	void SpawnIdolPierceVFX(const FName& IdolID, ET66ItemRarity Rarity, const FVector& Start, const FVector& End, const FVector& ImpactLocation, float StartDelaySeconds);
	void SpawnIdolAOEVFX(const FName& IdolID, ET66ItemRarity Rarity, const FVector& Location, float Radius, float StartDelaySeconds);
	void SpawnIdolBounceVFX(const FName& IdolID, ET66ItemRarity Rarity, const TArray<FVector>& ChainPositions, float StartDelaySeconds);
	void SpawnIdolDOTVFX(const FName& IdolID, ET66ItemRarity Rarity, AActor* FollowTarget, const FVector& Location, float Duration, float Radius, float StartDelaySeconds);

	/** Hero-specific VFX variants: spawn unique pixel patterns based on HeroID. */
	void SpawnHeroSlashVFX(const FVector& Location, float Radius, const FLinearColor& Color, const FName& HeroID);
	void SpawnHeroPierceVFX(const FVector& Start, const FVector& End, const FVector& ImpactLocation, const FLinearColor& Color, const FName& HeroID);
	void SpawnHeroBounceVFX(const TArray<FVector>& ChainPositions, const FLinearColor& Color, const FName& HeroID);
	void SpawnHeroDOTVFX(AActor* FollowTarget, const FVector& Location, float Duration, float Radius, const FLinearColor& Color, const FName& HeroID);

	struct FT66CombatImpactContext
	{
		ET66CombatVFXBindingSourceType SourceType = ET66CombatVFXBindingSourceType::WeaponBase;
		FName SourceID = NAME_None;
		FName ParentSourceID = NAME_None;
		FName HeroID = NAME_None;
		ET66AttackCategory AttackCategory = ET66AttackCategory::AOE;
		FVector AttackOrigin = FVector::ZeroVector;
		FVector DamageCenter = FVector::ZeroVector;
		FVector ImpactPoint = FVector::ZeroVector;
		FVector Forward = FVector::ForwardVector;
		float Radius = 0.f;
		float InnerRadius = 0.f;
		float HalfAngleDegrees = 0.f;
		float LineLength = 0.f;
		float TubeRadius = 0.f;
		int32 ChainIndex = INDEX_NONE;
		int32 EffectiveDamage = 0;
		bool bUsesFrontalSector = false;
		bool bDamageCenterValid = false;
		bool bImpactPointValid = false;
		FT66CombatTargetHandle PrimaryTargetHandle;
		TArray<FT66CombatTargetHandle> HitTargetHandles;
	};

	bool ResolveCombatVFXBinding(ET66CombatVFXBindingSourceType SourceType, FName SourceID, ET66AttackCategory AttackCategory, FT66CombatVFXBindingData& OutBindingData, UNiagaraSystem*& OutSystem) const;
	bool ShouldSuppressWeaponBaseProjectileVisual(ET66AttackCategory AttackCategory) const;
	bool TrySpawnBoundWeaponBaseSlashVFX(const FT66CombatImpactContext& WeaponImpactContext, int32 EffectiveDamage, FName HeroID, ET66AttackCategory AttackCategory);
	bool TrySpawnBoundIdolImpactVFX(const FT66CombatImpactContext& IdolImpactContext, FName IdolID, ET66ItemRarity Rarity, float Radius, bool& bOutBindingResolved);
	void SpawnWaterIdolImpactPlaceholderVFX(const FT66CombatImpactContext& IdolImpactContext, float Radius);
	void SpawnIdolImpactPlaceholderVFX(const FT66CombatImpactContext& IdolImpactContext, FName IdolID, ET66AttackCategory Category, float LingerSeconds);

	float BaseAttackRange = 0.f;
	float BaseFireIntervalSeconds = 0.f;
	int32 BaseDamagePerShot = 0;

	float EffectiveFireIntervalSeconds = 1.f;
	int32 EffectiveDamagePerShot = 20;
	float ProjectileScaleMultiplier = 1.f;

public:
	/** Current fire interval (seconds) after all multipliers. For UI cooldown timer text. */
	float GetEffectiveFireInterval() const { return EffectiveFireIntervalSeconds; }
protected:

	/** Time of last fire (for cooldown bar). */
	float LastFireTime = -9999.f;

	/** Marksman's Focus: consecutive hits on same target stack +8% damage (max 5 stacks). */
	TWeakObjectPtr<AActor> LastMarksmanTarget;
	int32 MarksmanStacks = 0;

	// Timed ultimate buffs
	float DeadeyeEndTime = -1.f;
	float JuicedEndTime = -1.f;
	int32 JuicedBonusBounce = 0;
	float RabidFrenzyEndTime = -1.f;
	bool bSuppressAutoAttack = false;
};
