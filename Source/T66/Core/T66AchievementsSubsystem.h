// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66ProfileSaveGame.h"
#include "T66AchievementsSubsystem.generated.h"

class UT66LocalizationSubsystem;
class UT66ProfileSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAchievementCoinsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAchievementsStateChanged);

/**
 * Persistent achievements + Achievement Coins (AC) wallet.
 * Lives at GameInstance scope (lifetime progression, not tied to a single run slot).
 */
UCLASS()
class T66_API UT66AchievementsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static const FString ProfileSaveSlotName;
	static constexpr int32 ProfileSaveUserIndex = 0;

	// ============================================
	// Companion Union (profile progression)
	// ============================================

	/** Union tier thresholds expressed as "stages cleared with this companion". */
	static constexpr int32 UnionTier_GoodStages = 5;
	static constexpr int32 UnionTier_MediumStages = 10;
	static constexpr int32 UnionTier_HyperStages = 20;

	/** Union healing intervals per tier (seconds per 1 heart). */
	static constexpr float UnionHealInterval_BasicSeconds = 10.f;
	static constexpr float UnionHealInterval_GoodSeconds = 5.f;
	static constexpr float UnionHealInterval_MediumSeconds = 3.f;
	static constexpr float UnionHealInterval_HyperSeconds = 1.f;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "Achievements")
	FOnAchievementCoinsChanged AchievementCoinsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Achievements")
	FOnAchievementsStateChanged AchievementsStateChanged;

	/** Current AC wallet balance. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Achievements")
	int32 GetAchievementCoinsBalance() const;

	/** Spend AC. Returns true if balance was sufficient and amount was deducted. Persists profile. */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	bool SpendAchievementCoins(int32 Amount);

	/** Hero skins: per-hero ownership. Default is always considered owned. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Achievements|Skins")
	bool IsHeroSkinOwned(FName HeroID, FName SkinID) const;

	/** Purchase a skin for a specific hero with AC. Returns true if purchased (deducts AC, adds to that hero's owned). */
	UFUNCTION(BlueprintCallable, Category = "Achievements|Skins")
	bool PurchaseHeroSkin(FName HeroID, FName SkinID, int32 CostAC);

	/** Currently equipped skin for a hero (Default if none set). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Achievements|Skins")
	FName GetEquippedHeroSkinID(FName HeroID) const;

	/** Set equipped skin for a hero (must be owned for that hero). */
	UFUNCTION(BlueprintCallable, Category = "Achievements|Skins")
	void SetEquippedHeroSkinID(FName HeroID, FName SkinID);

	/** Debug/dev: Reset all hero skin ownership (clears all purchased skins, everyone back to Default). */
	UFUNCTION(BlueprintCallable, Category = "Achievements|Skins")
	void ResetAllHeroSkinOwnership();

	/** Companion skins: per-companion ownership. Default is always considered owned. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Achievements|Skins")
	bool IsCompanionSkinOwned(FName CompanionID, FName SkinID) const;

	/** Purchase a skin for a specific companion with AC. Returns true if purchased. */
	UFUNCTION(BlueprintCallable, Category = "Achievements|Skins")
	bool PurchaseCompanionSkin(FName CompanionID, FName SkinID, int32 CostAC);

	/** Currently equipped skin for a companion (Default if none set). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Achievements|Skins")
	FName GetEquippedCompanionSkinID(FName CompanionID) const;

	/** Set equipped skin for a companion (must be owned for that companion). */
	UFUNCTION(BlueprintCallable, Category = "Achievements|Skins")
	void SetEquippedCompanionSkinID(FName CompanionID, FName SkinID);

	/** Get all achievements (definitions + current runtime state). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Achievements")
	TArray<FAchievementData> GetAllAchievements() const;

	/** Get achievements filtered by tier. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Achievements")
	TArray<FAchievementData> GetAchievementsForTier(ET66AchievementTier Tier) const;

	/** True if an achievement has been claimed already. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Achievements")
	bool IsAchievementClaimed(FName AchievementID) const;

	/**
	 * Gameplay event: an enemy died.
	 * This increments progress on any achievements that track enemy kills.
	 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void NotifyEnemyKilled(int32 Count = 1);

	// ============================================
	// Lab unlocks (items/enemies ever obtained or killed)
	// ============================================

	/** Mark an item as unlocked for The Lab (call when player obtains it in any run). Returns true if newly added. */
	UFUNCTION(BlueprintCallable, Category = "Lab")
	bool AddLabUnlockedItem(FName ItemID);

	/** Mark an enemy/boss as unlocked for The Lab (call when player kills it in any run). Returns true if newly added. */
	UFUNCTION(BlueprintCallable, Category = "Lab")
	bool AddLabUnlockedEnemy(FName EnemyOrBossID);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lab")
	bool IsLabUnlockedItem(FName ItemID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lab")
	bool IsLabUnlockedEnemy(FName EnemyOrBossID) const;

	/**
	 * Claim an unlocked achievement reward (adds AC to wallet).
	 * Returns true if claimed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	bool TryClaimAchievement(FName AchievementID);

	/** Debug/dev: reset all achievements and AC to zero. */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void ResetProfileProgress();

	// ============================================
	// Tutorial (first-time onboarding)
	// ============================================

	/** True if the player has completed the tutorial area at least once (profile). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tutorial")
	bool HasCompletedTutorial() const;

	/** Mark tutorial completed in the profile save (persists immediately). */
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void MarkTutorialCompleted();

	/** Union: stages cleared with a specific companion (lifetime / profile). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Union")
	int32 GetCompanionUnionStagesCleared(FName CompanionID) const;

	/** Union: increment stage-clears for a companion (e.g., when the player clears a stage with them). */
	UFUNCTION(BlueprintCallable, Category = "Union")
	void AddCompanionUnionStagesCleared(FName CompanionID, int32 DeltaStagesCleared = 1);

	/** Union: healing interval in seconds per 1 heart (Basic/Good/Medium/Hyper). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Union")
	float GetCompanionUnionHealingIntervalSeconds(FName CompanionID) const;

	/** Union: progress 0..1 toward Hyper tier (for UI bars). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Union")
	float GetCompanionUnionProgress01(FName CompanionID) const;

	/** For T66SkinSubsystem: access profile to read/write skin data. Returns null if not loaded. */
	UT66ProfileSaveGame* GetProfile() { return Profile; }
	const UT66ProfileSaveGame* GetProfile() const { return Profile; }

	/** For T66SkinSubsystem: mark profile dirty, save, and optionally broadcast AchievementCoinsChanged. */
	void MarkProfileDirtyAndSave(bool bBroadcastCoinsChanged = false);

private:
	UPROPERTY()
	TObjectPtr<UT66ProfileSaveGame> Profile;

	UPROPERTY()
	TArray<FAchievementData> CachedDefinitions;

	/** Accumulate saves to avoid disk IO per kill. */
	bool bProfileDirty = false;
	float LastProfileSaveWorldSeconds = -9999.f;
	float SaveThrottleSeconds = 2.0f;

	UT66LocalizationSubsystem* GetLocSubsystem() const;

	void LoadOrCreateProfile();
	void SaveProfileIfNeeded(bool bForce);
	void MarkDirtyAndMaybeSave(bool bForce);

	void RebuildDefinitions();
	void ApplyRuntimeStateToCachedDefinitions(TArray<FAchievementData>& InOut) const;

	FT66AchievementState* FindOrAddState(FName AchievementID);
	const FT66AchievementState* FindState(FName AchievementID) const;

	UFUNCTION()
	void HandleLanguageChanged(ET66Language NewLanguage);
};

