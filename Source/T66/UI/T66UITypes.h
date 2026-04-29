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
	Lobby = 3 UMETA(Hidden),
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
	TemporaryBuffSelection = 19 UMETA(DisplayName = "Temporary Buff Selection Modal"),
	// Reserved compatibility slot: screen class and flow are deleted, but the value stays hidden so older assets do not shift.
	TemporaryBuffPresetCreate = 20 UMETA(Hidden),
	TemporaryBuffShop = 21 UMETA(DisplayName = "Temporary Buff Shop Modal"),
	LanguageSelect = 22 UMETA(DisplayName = "Language Select Modal"),
	QuitConfirmation = 23 UMETA(DisplayName = "Quit Confirmation Modal"),
	PartyInvite = 24 UMETA(DisplayName = "Party Invite Modal"),
	RemovedUtilitySlot = 25 UMETA(Hidden),
	LobbyReadyCheck = 26 UMETA(Hidden),
	LobbyBackConfirm = 27 UMETA(Hidden),
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
	DailyClimb = 40 UMETA(DisplayName = "Daily Climb"),
	TDMainMenu = 41 UMETA(DisplayName = "Chadpocalypse TD Main Menu"),
	TDDifficultySelect = 42 UMETA(DisplayName = "Chadpocalypse TD Difficulty Select"),
	TDBattle = 43 UMETA(DisplayName = "Chadpocalypse TD Battle"),
	RemovedScreenSlot44 = 44 UMETA(Hidden),
	// Legacy serialized value retained so old Blueprint defaults can be loaded and repaired.
	PartySizePicker = 45 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ET66UITheme : uint8
{
	Classic UMETA(DisplayName = "Classic"),
	Dota UMETA(DisplayName = "Dota")
};
