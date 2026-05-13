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
	CompanionSelection = 6 UMETA(DisplayName = "Companion Selection"),
	Settings = 7 UMETA(DisplayName = "Settings"),
	Achievements = 8 UMETA(DisplayName = "Achievements"),
	Minigames = 9 UMETA(DisplayName = "Minigames"),
	RemovedLeaderboardSlot = 10 UMETA(Hidden),
	PauseMenu = 11 UMETA(DisplayName = "Pause Menu Modal"),
	ReportBug = 12 UMETA(DisplayName = "Report Bug Modal"),
	RunSummary = 13 UMETA(DisplayName = "Run Summary Modal"),
	// Legacy enum key retained for compatibility with existing assets and Blueprint references.
	PowerUp = 14 UMETA(DisplayName = "Power Up"),
	// Modals (overlays on top of other screens)
	HeroGrid = 15 UMETA(DisplayName = "Hero Grid Modal"),
	HeroLore = 16 UMETA(Hidden),
	CompanionGrid = 17 UMETA(DisplayName = "Companion Grid Modal"),
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
	MiniMainMenu = 31 UMETA(DisplayName = "Mini Chadpocalypse Main Menu"),
	MiniSaveSlots = 32 UMETA(DisplayName = "Mini Chadpocalypse Save Slots"),
	MiniCharacterSelect = 33 UMETA(DisplayName = "Mini Chadpocalypse Character Select"),
	MiniDifficultySelect = 34 UMETA(DisplayName = "Mini Chadpocalypse Difficulty Select"),
	MiniIdolSelect = 35 UMETA(DisplayName = "Mini Chadpocalypse Idol Select"),
	MiniShop = 36 UMETA(DisplayName = "Mini Chadpocalypse Shop"),
	MiniRunSummary = 37 UMETA(DisplayName = "Mini Chadpocalypse Run Summary"),
	MiniCompanionSelect = 38 UMETA(DisplayName = "Mini Chadpocalypse Companion Select"),
	Challenges = 39 UMETA(DisplayName = "Challenges Modal"),
	DailyDescent = 40 UMETA(DisplayName = "Daily Descent"),
	TDMainMenu = 41 UMETA(DisplayName = "Chadpocalypse Tower Defense Main Menu"),
	TDDifficultySelect = 42 UMETA(DisplayName = "Chadpocalypse Tower Defense Difficulty Select"),
	TDBattle = 43 UMETA(DisplayName = "Chadpocalypse Tower Defense Battle"),
	RemovedScreenSlot44 = 44 UMETA(Hidden),
	RemovedScreenSlot45 = 45 UMETA(Hidden),
	IdleMainMenu = 46 UMETA(DisplayName = "Chadpocalypse Idle Main Menu"),
	DeckMainMenu = 47 UMETA(DisplayName = "Chadpocalypse Deck Builder Main Menu"),
	VersusMainMenu = 48 UMETA(DisplayName = "Versus Main Menu")
};

UENUM(BlueprintType)
enum class ET66UITheme : uint8
{
	Classic UMETA(DisplayName = "Classic")
};
