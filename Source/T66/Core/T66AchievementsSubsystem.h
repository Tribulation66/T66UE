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

