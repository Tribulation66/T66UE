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
	None UMETA(DisplayName = "None"),
	MainMenu UMETA(DisplayName = "Main Menu"),
	PartySizePicker UMETA(DisplayName = "Party Size Picker"),
	Lobby UMETA(DisplayName = "Lobby"),
	SaveSlots UMETA(DisplayName = "Save Slots"),
	HeroSelection UMETA(DisplayName = "Hero Selection"),
	CompanionSelection UMETA(DisplayName = "Companion Selection"),
	Settings UMETA(DisplayName = "Settings"),
	Achievements UMETA(DisplayName = "Achievements"),
	PauseMenu UMETA(DisplayName = "Pause Menu Modal"),
	ReportBug UMETA(DisplayName = "Report Bug Modal"),
	RunSummary UMETA(DisplayName = "Run Summary Modal"),
	// Modals (overlays on top of other screens)
	HeroGrid UMETA(DisplayName = "Hero Grid Modal"),
	HeroLore UMETA(DisplayName = "Hero Lore Modal"),
	CompanionGrid UMETA(DisplayName = "Companion Grid Modal"),
	CompanionLore UMETA(DisplayName = "Companion Lore Modal"),
	LanguageSelect UMETA(DisplayName = "Language Select Modal"),
	QuitConfirmation UMETA(DisplayName = "Quit Confirmation Modal"),
	LobbyReadyCheck UMETA(DisplayName = "Lobby Ready Check Modal"),
	LobbyBackConfirm UMETA(DisplayName = "Lobby Back Confirm Modal"),
	AccountStatus UMETA(DisplayName = "Account Status Panel"),
	PlayerSummaryPicker UMETA(DisplayName = "Player Summary Picker")
};
