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

