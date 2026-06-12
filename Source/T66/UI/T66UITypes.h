// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66UITypes.generated.h"

/**
 * Enum for all screen types in the game's UI flow
 */
UENUM(BlueprintType)
enum class ET66ScreenType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	MainMenu = 1 UMETA(DisplayName = "Main Menu"),
	// Removed compatibility slots are retained so serialized enum values do not shift.
	RemovedScreenSlot02 = 2 UMETA(Hidden),
	RemovedScreenSlot03 = 3 UMETA(Hidden),
	SaveSlots = 4 UMETA(DisplayName = "Save Slots"),
	HeroSelection = 5 UMETA(DisplayName = "Hero Selection"),
	CompanionSelection = 6 UMETA(DisplayName = "Girlfriend Selection"),
	Settings = 7 UMETA(DisplayName = "Settings"),
	Achievements = 8 UMETA(DisplayName = "Achievements"),
	RemovedScreenSlot09 = 9 UMETA(Hidden),
	RemovedLeaderboardSlot = 10 UMETA(Hidden),
	PauseMenu = 11 UMETA(DisplayName = "Pause Menu Modal"),
	ReportBug = 12 UMETA(DisplayName = "Report Bug Modal"),
	RunSummary = 13 UMETA(DisplayName = "Run Summary Modal"),
	// Legacy enum key retained for compatibility with existing assets and Blueprint references.
	PowerUp = 14 UMETA(DisplayName = "Power Up"),
	// Modals (overlays on top of other screens)
	HeroGrid = 15 UMETA(DisplayName = "Hero Grid Modal"),
	HeroLore = 16 UMETA(Hidden),
	CompanionGrid = 17 UMETA(DisplayName = "Girlfriend Grid Modal"),
	CompanionLore = 18 UMETA(Hidden),
	LanguageSelect = 22 UMETA(DisplayName = "Language Select Modal"),
	QuitConfirmation = 23 UMETA(DisplayName = "Quit Confirmation Modal"),
	PartyInvite = 24 UMETA(DisplayName = "Party Invite Modal"),
	RemovedUtilitySlot = 25 UMETA(Hidden),
	RemovedScreenSlot26 = 26 UMETA(Hidden),
	RemovedScreenSlot27 = 27 UMETA(Hidden),
	AccountStatus = 28 UMETA(DisplayName = "Account Status Panel"),
	PlayerSummaryPicker = 29 UMETA(DisplayName = "Player Summary Picker"),
	SavePreview = 30 UMETA(DisplayName = "Save Preview Modal"),
	RemovedScreenSlot31 = 31 UMETA(Hidden),
	RemovedScreenSlot32 = 32 UMETA(Hidden),
	RemovedScreenSlot33 = 33 UMETA(Hidden),
	RemovedScreenSlot34 = 34 UMETA(Hidden),
	RemovedScreenSlot35 = 35 UMETA(Hidden),
	RemovedScreenSlot36 = 36 UMETA(Hidden),
	RemovedScreenSlot37 = 37 UMETA(Hidden),
	RemovedScreenSlot38 = 38 UMETA(Hidden),
	Challenges = 39 UMETA(DisplayName = "Challenges Modal"),
	DailyDescent = 40 UMETA(DisplayName = "Daily Descent"),
	RemovedScreenSlot41 = 41 UMETA(Hidden),
	RemovedScreenSlot42 = 42 UMETA(Hidden),
	RemovedScreenSlot43 = 43 UMETA(Hidden),
	RemovedScreenSlot44 = 44 UMETA(Hidden),
	RemovedScreenSlot45 = 45 UMETA(Hidden),
	RemovedScreenSlot46 = 46 UMETA(Hidden),
	RemovedScreenSlot47 = 47 UMETA(Hidden),
	RemovedScreenSlot48 = 48 UMETA(Hidden),
	RemovedScreenSlot49 = 49 UMETA(Hidden),
	GameOver = 50 UMETA(DisplayName = "Game Over Modal"),
	PetSelection = 51 UMETA(DisplayName = "Pet Selection"),
	CustomHeroBuilder = 52 UMETA(DisplayName = "Custom Hero Builder")
};

UENUM(BlueprintType)
enum class ET66UITheme : uint8
{
	Classic UMETA(DisplayName = "Classic")
};
