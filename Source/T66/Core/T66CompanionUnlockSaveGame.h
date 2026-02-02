// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "T66CompanionUnlockSaveGame.generated.h"

/**
 * Persistent companion unlock state (profile-wide).
 * Intentionally separate from Achievements/Profile save.
 */
UCLASS()
class T66_API UT66CompanionUnlockSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Bump when adding/changing fields in a breaking way.
	UPROPERTY(SaveGame)
	int32 SchemaVersion = 1;

	/** Unlocked companion IDs (e.g. Companion_01..Companion_48). */
	UPROPERTY(SaveGame)
	TSet<FName> UnlockedCompanionIDs;
};

