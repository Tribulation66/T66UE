// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "T66LocalizationSubsystem.generated.h"

/**
 * Supported languages enum - extensible for future additions
 */
UENUM(BlueprintType)
enum class ET66Language : uint8
{
	English UMETA(DisplayName = "English"),
	BrazilianPortuguese UMETA(DisplayName = "Português (Brasil)"),
	ChineseTraditional UMETA(DisplayName = "繁體中文"),
	// Add new languages above this line
	MAX UMETA(Hidden)
};

/**
 * Delegate broadcast when language changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLanguageChanged, ET66Language, NewLanguage);

/**
 * Localization subsystem - manages game language and provides localized text
 * Uses a code-based string table approach for compile-time safety
 */
UCLASS()
class T66_API UT66LocalizationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Get the current language */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization")
	ET66Language GetCurrentLanguage() const { return CurrentLanguage; }

	/** Set the current language (broadcasts OnLanguageChanged) */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	void SetLanguage(ET66Language NewLanguage);

	/** Get display name for a language (in that language) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization")
	FText GetLanguageDisplayName(ET66Language Language) const;

	/** Get all available languages */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization")
	TArray<ET66Language> GetAvailableLanguages() const;

	/** Delegate broadcast when language changes */
	UPROPERTY(BlueprintAssignable, Category = "Localization")
	FOnLanguageChanged OnLanguageChanged;

	// ========== Localized Text Getters ==========
	// All UI text should be retrieved through these functions

	// Main Menu
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|MainMenu")
	FText GetText_GameTitle() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|MainMenu")
	FText GetText_NewGame() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|MainMenu")
	FText GetText_LoadGame() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|MainMenu")
	FText GetText_Settings() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|MainMenu")
	FText GetText_Achievements() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|MainMenu")
	FText GetText_Quit() const;
	
	// Leaderboard
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Leaderboard")
	FText GetText_Leaderboard() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Leaderboard")
	FText GetText_Global() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Leaderboard")
	FText GetText_Friends() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Leaderboard")
	FText GetText_Streamers() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Leaderboard")
	FText GetText_SoloRuns() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Leaderboard")
	FText GetText_DuoRuns() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Leaderboard")
	FText GetText_TrioRuns() const;
	
	// Party Size Picker
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|PartySizePicker")
	FText GetText_SelectPartySize() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|PartySizePicker")
	FText GetText_Solo() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|PartySizePicker")
	FText GetText_Duo() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|PartySizePicker")
	FText GetText_Trio() const;
	
	// Hero Selection
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_SelectYourHero() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_HeroGrid() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_ChooseCompanion() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_EnterTheTribulation() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_TheLab() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_BodyTypeA() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_BodyTypeB() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_Skins() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_HeroInfo() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_Lore() const;
	
	// Skin names
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Skins")
	FText GetText_SkinName(FName SkinID) const;
	
	// Companion Selection
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|CompanionSelection")
	FText GetText_SelectCompanion() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|CompanionSelection")
	FText GetText_CompanionGrid() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|CompanionSelection")
	FText GetText_NoCompanion() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|CompanionSelection")
	FText GetText_ConfirmCompanion() const;
	
	// Data - Hero and Companion Names
	// These functions return localized names for heroes/companions by ID
	// All localized hero/companion names are defined here, not in DataTables
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Data")
	FText GetText_HeroName(FName HeroID) const;
	
	/** Localized hero description blurb (placeholder text per hero) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Data")
	FText GetText_HeroDescription(FName HeroID) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Data")
	FText GetText_CompanionName(FName CompanionID) const;
	
	/** Helper that calls GetText_HeroName with the HeroID from FHeroData */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Data")
	FText GetHeroDisplayName(const FHeroData& HeroData) const;
	
	/** Helper that calls GetText_CompanionName with the CompanionID from FCompanionData */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Data")
	FText GetCompanionDisplayName(const FCompanionData& CompanionData) const;
	
	// Common
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_Back() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_Confirm() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_Cancel() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_Yes() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_No() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_Buy() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_Sell() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_Equip() const;
	
	// Language Select
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|LanguageSelect")
	FText GetText_SelectLanguage() const;
	
	// Quit Confirmation
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|QuitConfirmation")
	FText GetText_QuitConfirmTitle() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|QuitConfirmation")
	FText GetText_QuitConfirmMessage() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|QuitConfirmation")
	FText GetText_YesQuit() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|QuitConfirmation")
	FText GetText_NoStay() const;
	
	// Difficulties
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Difficulty")
	FText GetText_Difficulty(ET66Difficulty Difficulty) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Difficulty")
	FText GetText_Easy() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Difficulty")
	FText GetText_Medium() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Difficulty")
	FText GetText_Hard() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Difficulty")
	FText GetText_VeryHard() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Difficulty")
	FText GetText_Impossible() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Difficulty")
	FText GetText_Perdition() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Difficulty")
	FText GetText_Final() const;
	
	// Save Slots
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|SaveSlots")
	FText GetText_SaveSlots() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|SaveSlots")
	FText GetText_EmptySlot() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|SaveSlots")
	FText GetText_Stage() const;
	
	// Pause Menu
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|PauseMenu")
	FText GetText_Resume() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|PauseMenu")
	FText GetText_SaveAndQuit() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|PauseMenu")
	FText GetText_Restart() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|PauseMenu")
	FText GetText_ReportBug() const;
	
	// Report Bug modal
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|ReportBug")
	FText GetText_ReportBugTitle() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|ReportBug")
	FText GetText_ReportBugSubmit() const;
	
	// Account Status
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|AccountStatus")
	FText GetText_AccountStatus() const;

private:
	ET66Language CurrentLanguage = ET66Language::English;
	
	void LoadSavedLanguage();
	void SaveLanguagePreference();
};
