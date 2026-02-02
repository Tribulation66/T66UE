// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66CompanionUnlockSubsystem.generated.h"

class UT66CompanionUnlockSaveGame;

/**
 * Companion unlocks (profile-wide).
 * - Owns persistence (separate save slot)
 * - Provides fast lookups for UI + gameplay
 */
UCLASS()
class T66_API UT66CompanionUnlockSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static const FString SlotName;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** True if the companion is unlocked (NAME_None is always treated as unlocked). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Companions|Unlocks")
	bool IsCompanionUnlocked(FName CompanionID) const;

	/**
	 * Unlocks the companion and saves.
	 * @return true if it was newly unlocked this call.
	 */
	UFUNCTION(BlueprintCallable, Category = "Companions|Unlocks")
	bool UnlockCompanion(FName CompanionID);

	/** Debug helper (kept callable for editor testing). */
	UFUNCTION(BlueprintCallable, Category = "Companions|Unlocks")
	void ResetAllUnlocks();

private:
	UPROPERTY()
	TObjectPtr<UT66CompanionUnlockSaveGame> SaveObj;

	void LoadOrCreate();
	void Save();
};

