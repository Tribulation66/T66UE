// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66LocalizationSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Internationalization/Internationalization.h"
#include "Misc/ConfigCacheIni.h"

#define LOCTEXT_NAMESPACE "T66Localization"

static FString T66CultureForLanguage(ET66Language L)
{
	switch (L)
	{
	case ET66Language::English: return TEXT("en");
	case ET66Language::ChineseSimplified: return TEXT("zh-Hans");
	case ET66Language::ChineseTraditional: return TEXT("zh-Hant");
	case ET66Language::Japanese: return TEXT("ja");
	case ET66Language::Korean: return TEXT("ko");
	case ET66Language::Russian: return TEXT("ru");
	case ET66Language::Polish: return TEXT("pl");
	case ET66Language::German: return TEXT("de");
	case ET66Language::French: return TEXT("fr");
	case ET66Language::SpanishSpain: return TEXT("es-ES");
	case ET66Language::SpanishLatAm: return TEXT("es-419");
	case ET66Language::BrazilianPortuguese: return TEXT("pt-BR");
	case ET66Language::PortuguesePortugal: return TEXT("pt-PT");
	case ET66Language::Italian: return TEXT("it");
	case ET66Language::Turkish: return TEXT("tr");
	case ET66Language::Ukrainian: return TEXT("uk");
	case ET66Language::Czech: return TEXT("cs");
	case ET66Language::Hungarian: return TEXT("hu");
	case ET66Language::Thai: return TEXT("th");
	case ET66Language::Vietnamese: return TEXT("vi");
	case ET66Language::Indonesian: return TEXT("id");
	case ET66Language::Arabic: return TEXT("ar");
	default: return TEXT("en");
	}
}

static ET66Language T66LanguageForCulture(const FString& Culture)
{
	const FString C = Culture.ToLower();
	if (C.StartsWith(TEXT("zh-hans")) || C.StartsWith(TEXT("zh-cn")) || C.StartsWith(TEXT("zh-sg"))) return ET66Language::ChineseSimplified;
	if (C.StartsWith(TEXT("zh-hant")) || C.StartsWith(TEXT("zh-hk")) || C.StartsWith(TEXT("zh-tw")) || C.StartsWith(TEXT("zh-mo"))) return ET66Language::ChineseTraditional;
	if (C.StartsWith(TEXT("ja"))) return ET66Language::Japanese;
	if (C.StartsWith(TEXT("ko"))) return ET66Language::Korean;
	if (C.StartsWith(TEXT("ru"))) return ET66Language::Russian;
	if (C.StartsWith(TEXT("pl"))) return ET66Language::Polish;
	if (C.StartsWith(TEXT("de"))) return ET66Language::German;
	if (C.StartsWith(TEXT("fr"))) return ET66Language::French;
	if (C.StartsWith(TEXT("es-419")) || C.StartsWith(TEXT("es-mx")) || C.StartsWith(TEXT("es-ar")) || C.StartsWith(TEXT("es-cl")) || C.StartsWith(TEXT("es-co")) || C.StartsWith(TEXT("es-pe"))) return ET66Language::SpanishLatAm;
	if (C.StartsWith(TEXT("es"))) return ET66Language::SpanishSpain;
	if (C.StartsWith(TEXT("pt-br"))) return ET66Language::BrazilianPortuguese;
	if (C.StartsWith(TEXT("pt"))) return ET66Language::PortuguesePortugal;
	if (C.StartsWith(TEXT("it"))) return ET66Language::Italian;
	if (C.StartsWith(TEXT("tr"))) return ET66Language::Turkish;
	if (C.StartsWith(TEXT("uk"))) return ET66Language::Ukrainian;
	if (C.StartsWith(TEXT("cs"))) return ET66Language::Czech;
	if (C.StartsWith(TEXT("hu"))) return ET66Language::Hungarian;
	if (C.StartsWith(TEXT("th"))) return ET66Language::Thai;
	if (C.StartsWith(TEXT("vi"))) return ET66Language::Vietnamese;
	if (C.StartsWith(TEXT("id"))) return ET66Language::Indonesian;
	if (C.StartsWith(TEXT("ar"))) return ET66Language::Arabic;
	return ET66Language::English;
}

void UT66LocalizationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadSavedLanguage();
}

void UT66LocalizationSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UT66LocalizationSubsystem::SetLanguage(ET66Language NewLanguage)
{
	if (CurrentLanguage != NewLanguage)
	{
		CurrentLanguage = NewLanguage;
		const FString Culture = T66CultureForLanguage(CurrentLanguage);
		FInternationalization::Get().SetCurrentCulture(Culture);
		SaveLanguagePreference();
		OnLanguageChanged.Broadcast(NewLanguage);
	}
}

FText UT66LocalizationSubsystem::GetLanguageDisplayName(ET66Language Language) const
{
	switch (Language)
	{
	// Important: keep language names culture-invariant so the selector always shows each language in its own native name.
	// (These should NOT change when the game's current culture changes.)
	case ET66Language::English: return FText::AsCultureInvariant(TEXT("English"));
	case ET66Language::BrazilianPortuguese: return FText::AsCultureInvariant(TEXT("Português (Brasil)"));
	case ET66Language::PortuguesePortugal: return FText::AsCultureInvariant(TEXT("Português (Portugal)"));
	case ET66Language::ChineseSimplified: return FText::AsCultureInvariant(TEXT("简体中文"));
	case ET66Language::ChineseTraditional: return FText::AsCultureInvariant(TEXT("繁體中文"));
	case ET66Language::Japanese: return FText::AsCultureInvariant(TEXT("日本語"));
	case ET66Language::Korean: return FText::AsCultureInvariant(TEXT("한국어"));
	case ET66Language::Russian: return FText::AsCultureInvariant(TEXT("Русский"));
	case ET66Language::Polish: return FText::AsCultureInvariant(TEXT("Polski"));
	case ET66Language::German: return FText::AsCultureInvariant(TEXT("Deutsch"));
	case ET66Language::French: return FText::AsCultureInvariant(TEXT("Français"));
	case ET66Language::SpanishSpain: return FText::AsCultureInvariant(TEXT("Español (España)"));
	case ET66Language::SpanishLatAm: return FText::AsCultureInvariant(TEXT("Español (Latinoamérica)"));
	case ET66Language::Italian: return FText::AsCultureInvariant(TEXT("Italiano"));
	case ET66Language::Turkish: return FText::AsCultureInvariant(TEXT("Türkçe"));
	case ET66Language::Ukrainian: return FText::AsCultureInvariant(TEXT("Українська"));
	case ET66Language::Czech: return FText::AsCultureInvariant(TEXT("Čeština"));
	case ET66Language::Hungarian: return FText::AsCultureInvariant(TEXT("Magyar"));
	case ET66Language::Thai: return FText::AsCultureInvariant(TEXT("ไทย"));
	case ET66Language::Vietnamese: return FText::AsCultureInvariant(TEXT("Tiếng Việt"));
	case ET66Language::Indonesian: return FText::AsCultureInvariant(TEXT("Bahasa Indonesia"));
	case ET66Language::Arabic: return FText::AsCultureInvariant(TEXT("العربية"));
	default: return FText::AsCultureInvariant(TEXT("Unknown"));
	}
}

TArray<ET66Language> UT66LocalizationSubsystem::GetAvailableLanguages() const
{
	TArray<ET66Language> Languages;
	Languages.Add(ET66Language::English);
	Languages.Add(ET66Language::ChineseSimplified);
	Languages.Add(ET66Language::ChineseTraditional);
	Languages.Add(ET66Language::Japanese);
	Languages.Add(ET66Language::Korean);
	Languages.Add(ET66Language::Russian);
	Languages.Add(ET66Language::Polish);
	Languages.Add(ET66Language::German);
	Languages.Add(ET66Language::French);
	Languages.Add(ET66Language::SpanishSpain);
	Languages.Add(ET66Language::SpanishLatAm);
	Languages.Add(ET66Language::BrazilianPortuguese);
	Languages.Add(ET66Language::PortuguesePortugal);
	Languages.Add(ET66Language::Italian);
	Languages.Add(ET66Language::Turkish);
	Languages.Add(ET66Language::Ukrainian);
	Languages.Add(ET66Language::Czech);
	Languages.Add(ET66Language::Hungarian);
	Languages.Add(ET66Language::Thai);
	Languages.Add(ET66Language::Vietnamese);
	Languages.Add(ET66Language::Indonesian);
	Languages.Add(ET66Language::Arabic);
	return Languages;
}

void UT66LocalizationSubsystem::LoadSavedLanguage()
{
	CurrentLanguage = ET66Language::English;
	FString SavedCulture;
	if (GConfig && GConfig->GetString(TEXT("T66.Localization"), TEXT("Culture"), SavedCulture, GGameUserSettingsIni))
	{
		CurrentLanguage = T66LanguageForCulture(SavedCulture);
	}
	FInternationalization::Get().SetCurrentCulture(T66CultureForLanguage(CurrentLanguage));
}

void UT66LocalizationSubsystem::SaveLanguagePreference()
{
	if (!GConfig) return;
	const FString Culture = T66CultureForLanguage(CurrentLanguage);
	GConfig->SetString(TEXT("T66.Localization"), TEXT("Culture"), *Culture, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}

#undef LOCTEXT_NAMESPACE

// ========== Hero Names ==========
// All hero names are centralized here for localization

FText UT66LocalizationSubsystem::GetText_HeroName(FName HeroID) const
{
	if (HeroID == FName(TEXT("Hero_1"))) return NSLOCTEXT("T66.HeroNames", "Hero_1", "Knight Chad");
	if (HeroID == FName(TEXT("Hero_2"))) return NSLOCTEXT("T66.HeroNames", "Hero_2", "Ninja Chad");
	if (HeroID == FName(TEXT("Hero_3"))) return NSLOCTEXT("T66.HeroNames", "Hero_3", "Gunslinger Chad");
	if (HeroID == FName(TEXT("Hero_4"))) return NSLOCTEXT("T66.HeroNames", "Hero_4", "Magician Chad");
	return FText::FromName(HeroID);
}

// ========== Hero Descriptions ==========

FText UT66LocalizationSubsystem::GetText_HeroDescription(FName HeroID) const
{
	if (HeroID == FName(TEXT("Hero_1"))) return NSLOCTEXT("T66.HeroDescriptions", "Hero_1", "Sturdy frontline. Favors armor.");
	if (HeroID == FName(TEXT("Hero_2"))) return NSLOCTEXT("T66.HeroDescriptions", "Hero_2", "Swift and elusive. Favors evasion.");
	if (HeroID == FName(TEXT("Hero_3"))) return NSLOCTEXT("T66.HeroDescriptions", "Hero_3", "Quick on the draw. Favors attack speed.");
	if (HeroID == FName(TEXT("Hero_4"))) return NSLOCTEXT("T66.HeroDescriptions", "Hero_4", "Bends fortune. Favors luck.");
	return NSLOCTEXT("T66.HeroDescriptions", "SelectHeroPrompt", "Select a hero to view their description.");
}

FText UT66LocalizationSubsystem::GetText_HeroQuote(FName HeroID) const
{
	if (HeroID == FName(TEXT("Hero_1"))) return NSLOCTEXT("T66.HeroQuotes", "Hero_1", "\"Hold the line.\"");
	if (HeroID == FName(TEXT("Hero_2"))) return NSLOCTEXT("T66.HeroQuotes", "Hero_2", "\"You won't see me coming.\"");
	if (HeroID == FName(TEXT("Hero_3"))) return NSLOCTEXT("T66.HeroQuotes", "Hero_3", "\"Quick draw.\"");
	if (HeroID == FName(TEXT("Hero_4"))) return NSLOCTEXT("T66.HeroQuotes", "Hero_4", "\"Luck is with me.\"");
	return FText::GetEmpty();
}

// ========== Stat Labels (shared) ==========

FText UT66LocalizationSubsystem::GetText_Stat_Damage() const
{
	return NSLOCTEXT("T66.Stats", "Damage", "Damage");
}

FText UT66LocalizationSubsystem::GetText_Stat_AttackSpeed() const
{
	return NSLOCTEXT("T66.Stats", "AttackSpeed", "Attack Speed");
}

FText UT66LocalizationSubsystem::GetText_Stat_AttackSize() const
{
	return NSLOCTEXT("T66.Stats", "AttackSize", "Attack Size");
}

FText UT66LocalizationSubsystem::GetText_Stat_Armor() const
{
	return NSLOCTEXT("T66.Stats", "Armor", "Armor");
}

FText UT66LocalizationSubsystem::GetText_Stat_Evasion() const
{
	return NSLOCTEXT("T66.Stats", "Evasion", "Evasion");
}

FText UT66LocalizationSubsystem::GetText_Stat_Luck() const
{
	return NSLOCTEXT("T66.Stats", "Luck", "Luck");
}

FText UT66LocalizationSubsystem::GetText_Stat_Speed() const
{
	return NSLOCTEXT("T66.Stats", "Speed", "Speed");
}

FText UT66LocalizationSubsystem::GetText_StatLineFormat() const
{
	return NSLOCTEXT("T66.Stats", "StatLineFormat", "{0}: {1}");
}

// ========== Items (names derived from ItemID) ==========

FText UT66LocalizationSubsystem::GetText_ItemRarityName(ET66ItemRarity Rarity) const
{
	switch (Rarity)
	{
		case ET66ItemRarity::Black: return NSLOCTEXT("T66.Items", "RarityBlack", "Black");
		case ET66ItemRarity::Red: return NSLOCTEXT("T66.Items", "RarityRed", "Red");
		case ET66ItemRarity::Yellow: return NSLOCTEXT("T66.Items", "RarityYellow", "Yellow");
		case ET66ItemRarity::White: return NSLOCTEXT("T66.Items", "RarityWhite", "White");
		case ET66ItemRarity::Cursed: return NSLOCTEXT("T66.Items", "RarityCursed", "Cursed");
		default: break;
	}
	return FText::GetEmpty();
}

FText UT66LocalizationSubsystem::GetText_ItemNameFormat() const
{
	return NSLOCTEXT("T66.Items", "ItemNameFormat", "{0} {1}");
}

FText UT66LocalizationSubsystem::GetText_ItemDisplayName(FName ItemID) const
{
	const FString S = ItemID.ToString();

	// Canonical IDs: Item_<Color>_<Number>
	FString ColorStr;
	FString NumStr;
	if (S.Split(TEXT("_"), nullptr, &ColorStr))
	{
		// After first split, ColorStr holds the remainder; split again.
		// Example: "Black_01"
		FString Tail = ColorStr;
		if (Tail.Split(TEXT("_"), &ColorStr, &NumStr))
		{
			ET66ItemRarity R = ET66ItemRarity::Black;
			if (ColorStr.Equals(TEXT("Black"), ESearchCase::IgnoreCase)) R = ET66ItemRarity::Black;
			else if (ColorStr.Equals(TEXT("Red"), ESearchCase::IgnoreCase)) R = ET66ItemRarity::Red;
			else if (ColorStr.Equals(TEXT("Yellow"), ESearchCase::IgnoreCase)) R = ET66ItemRarity::Yellow;
			else if (ColorStr.Equals(TEXT("White"), ESearchCase::IgnoreCase)) R = ET66ItemRarity::White;
			else if (ColorStr.Equals(TEXT("Cursed"), ESearchCase::IgnoreCase)) R = ET66ItemRarity::Cursed;

			const int32 Index = FMath::Max(0, FCString::Atoi(*NumStr));
			if (Index > 0)
			{
				return FText::Format(GetText_ItemNameFormat(), GetText_ItemRarityName(R), FText::AsNumber(Index));
			}
		}
	}

	// Fallback: show raw ID.
	return FText::FromName(ItemID);
}

// ========== Companion Names ==========

FText UT66LocalizationSubsystem::GetText_CompanionName(FName CompanionID) const
{
	// Fallback: return the CompanionID as text
	return FText::FromName(CompanionID);
}

FText UT66LocalizationSubsystem::GetText_CompanionLore(FName CompanionID) const
{
	if (CompanionID.IsNone())                             return NSLOCTEXT("T66.CompanionLore", "NoCompanion", "Loner Style");
	if (CompanionID == FName(TEXT("Companion_01")))       return NSLOCTEXT("T66.CompanionLore", "Companion_01", "A steadfast guardian who never leaves your side.");
	if (CompanionID == FName(TEXT("Companion_02")))       return NSLOCTEXT("T66.CompanionLore", "Companion_02", "Born in flames, she heals through sheer willpower.");
	if (CompanionID == FName(TEXT("Companion_03")))       return NSLOCTEXT("T66.CompanionLore", "Companion_03", "Once a wanderer, now sworn to protect the fallen.");
	if (CompanionID == FName(TEXT("Companion_04")))       return NSLOCTEXT("T66.CompanionLore", "Companion_04", "Whispers ancient prayers that mend broken bones.");
	if (CompanionID == FName(TEXT("Companion_05")))       return NSLOCTEXT("T66.CompanionLore", "Companion_05", "A cunning trickster with a heart of gold.");
	if (CompanionID == FName(TEXT("Companion_06")))       return NSLOCTEXT("T66.CompanionLore", "Companion_06", "Draws strength from moonlight to shield allies.");
	if (CompanionID == FName(TEXT("Companion_07")))       return NSLOCTEXT("T66.CompanionLore", "Companion_07", "Forged in the depths, tempered by tribulation.");
	if (CompanionID == FName(TEXT("Companion_08")))       return NSLOCTEXT("T66.CompanionLore", "Companion_08", "The last of her kind, fighting to survive.");
	return NSLOCTEXT("T66.CompanionLore", "Unknown", "A mysterious companion.");
}

// ========== Helper functions that use the name lookups ==========

FText UT66LocalizationSubsystem::GetHeroDisplayName(const FHeroData& HeroData) const
{
	FText LocalizedName = GetText_HeroName(HeroData.HeroID);
	// If we got a valid localized name (not just the ID), use it
	if (!LocalizedName.IsEmpty() && !LocalizedName.EqualTo(FText::FromName(HeroData.HeroID)))
	{
		return LocalizedName;
	}
	// Fallback to DisplayName from DataTable if set
	if (!HeroData.DisplayName.IsEmpty())
	{
		return HeroData.DisplayName;
	}
	// Last resort: HeroID as text
	return FText::FromName(HeroData.HeroID);
}

FText UT66LocalizationSubsystem::GetCompanionDisplayName(const FCompanionData& CompanionData) const
{
	FText LocalizedName = GetText_CompanionName(CompanionData.CompanionID);
	// If we got a valid localized name (not just the ID), use it
	if (!LocalizedName.IsEmpty() && !LocalizedName.EqualTo(FText::FromName(CompanionData.CompanionID)))
	{
		return LocalizedName;
	}
	// Fallback to DisplayName from DataTable if set
	if (!CompanionData.DisplayName.IsEmpty())
	{
		return CompanionData.DisplayName;
	}
	// Last resort: CompanionID as text
	return FText::FromName(CompanionData.CompanionID);
}

// ========== Main Menu ==========

FText UT66LocalizationSubsystem::GetText_GameTitle() const
{
	// English: "Chadpocalypse"; other languages: "{Apocalypse} Chad" (Chad untranslated).
	if (GetCurrentLanguage() == ET66Language::English)
	{
		return NSLOCTEXT("T66.MainMenu", "Title", "CHADPOCALYPSE");
	}
	return FText::Format(
		NSLOCTEXT("T66.MainMenu", "GameTitleFormat", "{0} Chad"),
		GetText_Apocalypse()
	);
}

FText UT66LocalizationSubsystem::GetText_Apocalypse() const
{
	return NSLOCTEXT("T66.MainMenu", "Apocalypse", "Apocalypse");
}

FText UT66LocalizationSubsystem::GetText_NewGame() const
{
	return NSLOCTEXT("T66.MainMenu", "NewGame", "NEW GAME");
}

FText UT66LocalizationSubsystem::GetText_LoadGame() const
{
	return NSLOCTEXT("T66.MainMenu", "LoadGame", "LOAD GAME");
}

FText UT66LocalizationSubsystem::GetText_Settings() const
{
	return NSLOCTEXT("T66.MainMenu", "Settings", "SETTINGS");
}

FText UT66LocalizationSubsystem::GetText_Achievements() const
{
	return NSLOCTEXT("T66.Achievements", "Title", "ACHIEVEMENTS");
}

FText UT66LocalizationSubsystem::GetText_AchievementTierBlack() const
{
	return NSLOCTEXT("T66.Achievements", "TierBlack", "BLACK");
}

FText UT66LocalizationSubsystem::GetText_AchievementTierRed() const
{
	return NSLOCTEXT("T66.Achievements", "TierRed", "RED");
}

FText UT66LocalizationSubsystem::GetText_AchievementTierYellow() const
{
	return NSLOCTEXT("T66.Achievements", "TierYellow", "YELLOW");
}

FText UT66LocalizationSubsystem::GetText_AchievementTierWhite() const
{
	return NSLOCTEXT("T66.Achievements", "TierWhite", "WHITE");
}

FText UT66LocalizationSubsystem::GetText_AchievementCoinsFormat() const
{
	// Keep the AC abbreviation stable across languages for readability (Bible: "Achievement Coins (AC)").
	// Translators can adjust ordering/spacing per language.
	return NSLOCTEXT("T66.Achievements", "CoinsFormat", "{0} AC");
}

FText UT66LocalizationSubsystem::GetText_Claim() const
{
	return NSLOCTEXT("T66.Achievements", "Claim", "CLAIM");
}

FText UT66LocalizationSubsystem::GetText_Claimed() const
{
	return NSLOCTEXT("T66.Achievements", "Claimed", "CLAIMED");
}

FText UT66LocalizationSubsystem::GetText_AchievementName(FName AchievementID) const
{
	// v0: first wired achievement
	if (AchievementID == FName(TEXT("ACH_BLK_001")))
	{
		return NSLOCTEXT("T66.Achievements", "ACH_BLK_001_Name", "KILL 20 ENEMIES");
	}

	// Fallback: show the ID so missing strings are obvious (and never crash).
	return FText::FromName(AchievementID);
}

FText UT66LocalizationSubsystem::GetText_AchievementDescription(FName AchievementID) const
{
	if (AchievementID == FName(TEXT("ACH_BLK_001")))
	{
		return NSLOCTEXT("T66.Achievements", "ACH_BLK_001_Desc", "Kill 20 enemies.");
	}

	return FText::FromName(AchievementID);
}

FText UT66LocalizationSubsystem::GetText_Quit() const
{
	return NSLOCTEXT("T66.MainMenu", "Quit", "QUIT");
}

// ========== Theme ==========

FText UT66LocalizationSubsystem::GetText_ThemeDark() const
{
	return NSLOCTEXT("T66.Theme", "Dark", "DARK");
}

FText UT66LocalizationSubsystem::GetText_ThemeLight() const
{
	return NSLOCTEXT("T66.Theme", "Light", "LIGHT");
}

FText UT66LocalizationSubsystem::GetText_SettingsTheme() const
{
	return NSLOCTEXT("T66.Settings", "Theme", "Theme:");
}

FText UT66LocalizationSubsystem::GetText_MakeThemeDay() const
{
	return NSLOCTEXT("T66.Theme", "MakeThemeDay", "MAKE THEME DAY?");
}

FText UT66LocalizationSubsystem::GetText_MakeThemeNight() const
{
	return NSLOCTEXT("T66.Theme", "MakeThemeNight", "MAKE THEME NIGHT?");
}

// ========== Settings ==========

FText UT66LocalizationSubsystem::GetText_SettingsTabGameplay() const
{
	return NSLOCTEXT("T66.Settings", "TabGameplay", "GAMEPLAY");
}

FText UT66LocalizationSubsystem::GetText_SettingsTabGraphics() const
{
	return NSLOCTEXT("T66.Settings", "TabGraphics", "GRAPHICS");
}

FText UT66LocalizationSubsystem::GetText_SettingsTabControls() const
{
	return NSLOCTEXT("T66.Settings", "TabControls", "CONTROLS");
}

FText UT66LocalizationSubsystem::GetText_SettingsTabAudio() const
{
	return NSLOCTEXT("T66.Settings", "TabAudio", "AUDIO");
}

FText UT66LocalizationSubsystem::GetText_SettingsTabCrashing() const
{
	return NSLOCTEXT("T66.Settings", "TabCrashing", "CRASHING");
}

FText UT66LocalizationSubsystem::GetText_On() const
{
	return NSLOCTEXT("T66.Settings", "On", "ON");
}

FText UT66LocalizationSubsystem::GetText_Off() const
{
	return NSLOCTEXT("T66.Settings", "Off", "OFF");
}

FText UT66LocalizationSubsystem::GetText_Apply() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "APPLY", "APPLY");
}

FText UT66LocalizationSubsystem::GetText_RestoreDefaults() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Restore Defaults", "RESTORE DEFAULTS");
}

FText UT66LocalizationSubsystem::GetText_Rebind() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "REBIND", "REBIND");
}

FText UT66LocalizationSubsystem::GetText_Clear() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "CLEAR", "CLEAR");
}

FText UT66LocalizationSubsystem::GetText_Primary() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "PRIMARY", "PRIMARY");
}

FText UT66LocalizationSubsystem::GetText_Secondary() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "SECONDARY", "SECONDARY");
}

FText UT66LocalizationSubsystem::GetText_KeyboardAndMouse() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "KEYBOARD & MOUSE", "KEYBOARD & MOUSE");
}

FText UT66LocalizationSubsystem::GetText_Controller() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "CONTROLLER", "CONTROLLER");
}

FText UT66LocalizationSubsystem::GetText_RebindInstructions() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "RebindInstructions", "Click REBIND, then press a key/button (Esc cancels).");
}

FText UT66LocalizationSubsystem::GetText_RebindPressKeyFormat() const
{
	return NSLOCTEXT("T66.Settings", "RebindPressKeyFormat", "Press a key for {0} (Esc cancels).");
}

FText UT66LocalizationSubsystem::GetText_RebindCancelled() const
{
	return NSLOCTEXT("T66.Settings", "RebindCancelled", "Rebind cancelled.");
}

FText UT66LocalizationSubsystem::GetText_RebindSaved() const
{
	return NSLOCTEXT("T66.Settings", "RebindSaved", "Rebind saved.");
}

FText UT66LocalizationSubsystem::GetText_ControlMoveForward() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Move Forward", "Move Forward");
}

FText UT66LocalizationSubsystem::GetText_ControlMoveBack() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Move Back", "Move Back");
}

FText UT66LocalizationSubsystem::GetText_ControlMoveLeft() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Move Left", "Move Left");
}

FText UT66LocalizationSubsystem::GetText_ControlMoveRight() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Move Right", "Move Right");
}

FText UT66LocalizationSubsystem::GetText_ControlJump() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Jump", "Jump");
}

FText UT66LocalizationSubsystem::GetText_ControlInteract() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Interact", "Interact");
}

FText UT66LocalizationSubsystem::GetText_ControlPauseMenuPrimary() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Pause Menu (primary)", "Pause Menu (primary)");
}

FText UT66LocalizationSubsystem::GetText_ControlPauseMenuSecondary() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Pause Menu (secondary)", "Pause Menu (secondary)");
}

FText UT66LocalizationSubsystem::GetText_ControlToggleHUD() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Toggle HUD", "Toggle HUD");
}

FText UT66LocalizationSubsystem::GetText_ControlToggleTikTok() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Toggle TikTok", "Toggle TikTok");
}

FText UT66LocalizationSubsystem::GetText_ControlDash() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Dash", "Dash");
}

FText UT66LocalizationSubsystem::GetText_ControlUltimate() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Ultimate", "Ultimate");
}

FText UT66LocalizationSubsystem::GetText_ControlOpenFullMap() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Open Full Map", "Open Full Map");
}

FText UT66LocalizationSubsystem::GetText_ControlToggleMediaViewer() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Toggle Media Viewer", "Toggle Media Viewer");
}

FText UT66LocalizationSubsystem::GetText_ControlToggleGamerMode() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Toggle Gamer Mode (Hitboxes)", "Toggle Gamer Mode (Hitboxes)");
}

FText UT66LocalizationSubsystem::GetText_ControlRestartRun() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Restart Run", "Restart Run");
}

FText UT66LocalizationSubsystem::GetText_ControlAttackLock() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Attack Lock", "Attack Lock");
}

FText UT66LocalizationSubsystem::GetText_ControlAttackUnlock() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Attack Unlock", "Attack Unlock");
}

FText UT66LocalizationSubsystem::GetText_PracticeMode() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Practice Mode", "Practice Mode");
}

FText UT66LocalizationSubsystem::GetText_SubmitLeaderboardAnonymous() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Submit Leaderboard as Anonymous", "Submit Leaderboard as Anonymous");
}

FText UT66LocalizationSubsystem::GetText_SpeedRunMode() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Speed Run Mode", "Speed Run Mode");
}

FText UT66LocalizationSubsystem::GetText_GoonerMode() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Gooner Mode", "Gooner Mode");
}

FText UT66LocalizationSubsystem::GetText_Monitor() const
{
	return NSLOCTEXT("T66.Settings", "Monitor", "Monitor");
}

FText UT66LocalizationSubsystem::GetText_PrimaryMonitor() const
{
	return NSLOCTEXT("T66.Settings", "PrimaryMonitor", "Primary Monitor");
}

FText UT66LocalizationSubsystem::GetText_Resolution() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Resolution", "Resolution");
}

FText UT66LocalizationSubsystem::GetText_WindowMode() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Window Mode", "Window Mode");
}

FText UT66LocalizationSubsystem::GetText_Windowed() const
{
	return NSLOCTEXT("T66.Settings", "Windowed", "Windowed");
}

FText UT66LocalizationSubsystem::GetText_Fullscreen() const
{
	return NSLOCTEXT("T66.Settings", "Fullscreen", "Fullscreen");
}

FText UT66LocalizationSubsystem::GetText_BorderlessWindowed() const
{
	return NSLOCTEXT("T66.Settings", "BorderlessWindowed", "Borderless Windowed");
}

FText UT66LocalizationSubsystem::GetText_DisplayMode() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Display Mode", "Display Mode");
}

FText UT66LocalizationSubsystem::GetText_DisplayModeStandard() const
{
	return NSLOCTEXT("T66.Settings", "DisplayModeStandard", "Standard");
}

FText UT66LocalizationSubsystem::GetText_DisplayModeWidescreen() const
{
	return NSLOCTEXT("T66.Settings", "DisplayModeWidescreen", "Widescreen");
}

FText UT66LocalizationSubsystem::GetText_Quality() const
{
	return NSLOCTEXT("T66.Settings", "Quality", "Quality");
}

FText UT66LocalizationSubsystem::GetText_BestPerformance() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Best Performance", "Best Performance");
}

FText UT66LocalizationSubsystem::GetText_BestQuality() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Best Quality", "Best Quality");
}

FText UT66LocalizationSubsystem::GetText_FpsCap() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "FPS Cap", "FPS Cap");
}

FText UT66LocalizationSubsystem::GetText_Unlimited() const
{
	return NSLOCTEXT("T66.Settings", "Unlimited", "Unlimited");
}

FText UT66LocalizationSubsystem::GetText_KeepTheseSettingsTitle() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Keep these settings?", "Keep these settings?");
}

FText UT66LocalizationSubsystem::GetText_KeepTheseSettingsBodyFormat() const
{
	return NSLOCTEXT("T66.Settings", "KeepTheseSettingsBodyFormat", "Reverting in {0}s.");
}

FText UT66LocalizationSubsystem::GetText_Keep() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "KEEP", "KEEP");
}

FText UT66LocalizationSubsystem::GetText_Revert() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "REVERT", "REVERT");
}

FText UT66LocalizationSubsystem::GetText_MasterVolume() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Master Volume", "Master Volume");
}

FText UT66LocalizationSubsystem::GetText_MusicVolume() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Music Volume", "Music Volume");
}

FText UT66LocalizationSubsystem::GetText_SfxVolume() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "SFX Volume", "SFX Volume");
}

FText UT66LocalizationSubsystem::GetText_MuteWhenUnfocused() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Mute when unfocused", "Mute when unfocused");
}

FText UT66LocalizationSubsystem::GetText_OutputDevice() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Output Device", "Output Device");
}

FText UT66LocalizationSubsystem::GetText_SubtitlesAlwaysOn() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Subtitles: always on", "Subtitles: always on");
}

FText UT66LocalizationSubsystem::GetText_CrashingChecklistHeader() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "CrashingChecklistHeader", "If your game is crashing, try these steps:");
}

FText UT66LocalizationSubsystem::GetText_CrashingChecklistBody() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "CrashingChecklistBody",
		"1. Lower Quality to \"Best Performance\" in the Graphics tab.\n"
		"2. Disable \"Intense Visuals\" in the Gameplay tab.\n"
		"3. Reduce FPS Cap to 60 in the Graphics tab.\n"
		"4. Try Windowed mode instead of Fullscreen.");
}

FText UT66LocalizationSubsystem::GetText_ApplySafeModeSettings() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Apply Safe Mode Settings", "Apply Safe Mode Settings");
}

FText UT66LocalizationSubsystem::GetText_StillHavingIssues() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "Still having issues?", "Still having issues?");
}

FText UT66LocalizationSubsystem::GetText_ReportBugDescription() const
{
	return NSLOCTEXT("T66.Settings.Fallback", "ReportBugDescription", "Report a bug to help us fix it. Your report will include basic system info.");
}

FText UT66LocalizationSubsystem::GetText_DescribeTheBugHint() const
{
	return NSLOCTEXT("T66.ReportBug", "Hint", "Describe the bug...");
}

// ========== Leaderboard ==========

FText UT66LocalizationSubsystem::GetText_Leaderboard() const
{
	return NSLOCTEXT("T66.Leaderboard", "Title", "LEADERBOARD");
}

FText UT66LocalizationSubsystem::GetText_Global() const
{
	return NSLOCTEXT("T66.Leaderboard", "Global", "GLOBAL");
}

FText UT66LocalizationSubsystem::GetText_Friends() const
{
	return NSLOCTEXT("T66.Leaderboard", "Friends", "FRIENDS");
}

FText UT66LocalizationSubsystem::GetText_Streamers() const
{
	return NSLOCTEXT("T66.Leaderboard", "Streamers", "STREAMERS");
}

FText UT66LocalizationSubsystem::GetText_Weekly() const
{
	return NSLOCTEXT("T66.Leaderboard", "Weekly", "WEEKLY");
}

FText UT66LocalizationSubsystem::GetText_SoloRuns() const
{
	return NSLOCTEXT("T66.Leaderboard", "SoloRuns", "SOLO");
}

FText UT66LocalizationSubsystem::GetText_DuoRuns() const
{
	return NSLOCTEXT("T66.Leaderboard", "DuoRuns", "DUO");
}

FText UT66LocalizationSubsystem::GetText_TrioRuns() const
{
	return NSLOCTEXT("T66.Leaderboard", "TrioRuns", "TRIO");
}

// ========== Party Size Picker ==========

FText UT66LocalizationSubsystem::GetText_SelectPartySize() const
{
	return NSLOCTEXT("T66.PartySize", "Title", "SELECT PARTY SIZE");
}

FText UT66LocalizationSubsystem::GetText_Solo() const
{
	return NSLOCTEXT("T66.PartySize", "Solo", "SOLO");
}

FText UT66LocalizationSubsystem::GetText_Duo() const
{
	return NSLOCTEXT("T66.PartySize", "Duo", "DUO");
}

FText UT66LocalizationSubsystem::GetText_Trio() const
{
	return NSLOCTEXT("T66.PartySize", "Trio", "TRIO");
}

FText UT66LocalizationSubsystem::GetText_Coop() const
{
	return NSLOCTEXT("T66.PartySize", "Coop", "CO-OP");
}

FText UT66LocalizationSubsystem::GetText_LobbyTitle() const
{
	return NSLOCTEXT("T66.Lobby", "Title", "LOBBY");
}

FText UT66LocalizationSubsystem::GetText_LobbyYou() const
{
	return NSLOCTEXT("T66.Lobby", "You", "You");
}

FText UT66LocalizationSubsystem::GetText_LobbyWaitingForPlayer() const
{
	return NSLOCTEXT("T66.Lobby", "WaitingForPlayer", "Waiting for player...");
}

FText UT66LocalizationSubsystem::GetText_LobbyInviteFriend() const
{
	return NSLOCTEXT("T66.Lobby", "InviteFriend", "INVITE FRIEND");
}

FText UT66LocalizationSubsystem::GetText_LobbyContinue() const
{
	return NSLOCTEXT("T66.Lobby", "Continue", "CONTINUE");
}

FText UT66LocalizationSubsystem::GetText_LobbySelectHero() const
{
	return NSLOCTEXT("T66.Lobby", "SelectHero", "SELECT HERO");
}

FText UT66LocalizationSubsystem::GetText_LobbyReadyCheck() const
{
	return NSLOCTEXT("T66.Lobby", "ReadyCheck", "READY CHECK");
}

FText UT66LocalizationSubsystem::GetText_LobbyReadyToStart() const
{
	return NSLOCTEXT("T66.Lobby", "ReadyToStart", "Ready to start?");
}

FText UT66LocalizationSubsystem::GetText_LobbyFriends() const
{
	return NSLOCTEXT("T66.Lobby", "Friends", "FRIENDS");
}

FText UT66LocalizationSubsystem::GetText_LobbyReady() const
{
	return NSLOCTEXT("T66.Lobby", "Ready", "READY");
}

FText UT66LocalizationSubsystem::GetText_LobbyNotReady() const
{
	return NSLOCTEXT("T66.Lobby", "NotReady", "Not Ready");
}

FText UT66LocalizationSubsystem::GetText_LobbyLeaveConfirmTitle() const
{
	return NSLOCTEXT("T66.Lobby", "LeaveConfirmTitle", "Leave lobby?");
}

FText UT66LocalizationSubsystem::GetText_LobbyLeaveConfirmMessage() const
{
	return NSLOCTEXT("T66.Lobby", "LeaveConfirmMessage", "Are you sure you want to leave the lobby?");
}

FText UT66LocalizationSubsystem::GetText_LobbyLeaveStay() const
{
	return NSLOCTEXT("T66.Lobby", "LeaveStay", "STAY");
}

FText UT66LocalizationSubsystem::GetText_LobbyLeaveLeave() const
{
	return NSLOCTEXT("T66.Lobby", "LeaveLeave", "LEAVE");
}

FText UT66LocalizationSubsystem::GetText_BackToLobby() const
{
	return NSLOCTEXT("T66.Lobby", "BackToLobby", "BACK TO LOBBY");
}

// ========== Hero Selection ==========

FText UT66LocalizationSubsystem::GetText_SelectYourHero() const
{
	return NSLOCTEXT("T66.HeroSelection", "SelectHero", "Select a Hero");
}

FText UT66LocalizationSubsystem::GetText_HeroGrid() const
{
	return NSLOCTEXT("T66.HeroSelection", "HeroGrid", "HERO GRID");
}

FText UT66LocalizationSubsystem::GetText_ChooseCompanion() const
{
	return NSLOCTEXT("T66.HeroSelection", "ChooseCompanion", "CHOOSE COMPANION");
}

FText UT66LocalizationSubsystem::GetText_EnterTheTribulation() const
{
	return NSLOCTEXT("T66.HeroSelection", "EnterTheTribulation", "ENTER THE TRIBULATION");
}

FText UT66LocalizationSubsystem::GetText_TheLab() const
{
	return NSLOCTEXT("T66.HeroSelection", "TheLab", "THE LAB");
}

FText UT66LocalizationSubsystem::GetText_BodyTypeA() const
{
	return NSLOCTEXT("T66.HeroSelection", "BodyTypeA", "TYPE A");
}

FText UT66LocalizationSubsystem::GetText_BodyTypeB() const
{
	return NSLOCTEXT("T66.HeroSelection", "BodyTypeB", "TYPE B");
}

FText UT66LocalizationSubsystem::GetText_Skins() const
{
	return NSLOCTEXT("T66.HeroSelection", "Skins", "SKINS");
}

FText UT66LocalizationSubsystem::GetText_HeroInfo() const
{
	return NSLOCTEXT("T66.HeroSelection", "HeroInfo", "HERO INFO");
}

FText UT66LocalizationSubsystem::GetText_Lore() const
{
	return NSLOCTEXT("T66.HeroSelection", "Lore", "LORE");
}

FText UT66LocalizationSubsystem::GetText_BaseStatsHeader() const
{
	return NSLOCTEXT("T66.HeroSelection", "BaseStatsHeader", "Base Stats");
}

// ========== Skin Names ==========

FText UT66LocalizationSubsystem::GetText_SkinName(FName SkinID) const
{
	if (SkinID == FName(TEXT("Default")))
	{
		return NSLOCTEXT("T66.Skins", "Default", "Default");
	}
	if (SkinID == FName(TEXT("Beachgoer")))
	{
		return NSLOCTEXT("T66.Skins", "Beachgoer", "Beachgoer");
	}
	if (SkinID == FName(TEXT("Golden")))
	{
		return NSLOCTEXT("T66.Skins", "Golden", "Golden Champion");
	}
	if (SkinID == FName(TEXT("Shadow")))
	{
		return NSLOCTEXT("T66.Skins", "Shadow", "Shadow Walker");
	}
	if (SkinID == FName(TEXT("Infernal")))
	{
		return NSLOCTEXT("T66.Skins", "Infernal", "Infernal Wrath");
	}
	if (SkinID == FName(TEXT("Frost")))
	{
		return NSLOCTEXT("T66.Skins", "Frost", "Eternal Frost");
	}
	if (SkinID == FName(TEXT("Void")))
	{
		return NSLOCTEXT("T66.Skins", "Void", "Void Walker");
	}
	if (SkinID == FName(TEXT("Phoenix")))
	{
		return NSLOCTEXT("T66.Skins", "Phoenix", "Phoenix Rising");
	}
	if (SkinID == FName(TEXT("Cosmic")))
	{
		return NSLOCTEXT("T66.Skins", "Cosmic", "Cosmic");
	}
	if (SkinID == FName(TEXT("Neon")))
	{
		return NSLOCTEXT("T66.Skins", "Neon", "Neon");
	}
	if (SkinID == FName(TEXT("Primal")))
	{
		return NSLOCTEXT("T66.Skins", "Primal", "Primal");
	}
	if (SkinID == FName(TEXT("Celestial")))
	{
		return NSLOCTEXT("T66.Skins", "Celestial", "Celestial");
	}
	if (SkinID == FName(TEXT("Obsidian")))
	{
		return NSLOCTEXT("T66.Skins", "Obsidian", "Obsidian");
	}
	// Fallback
	return FText::FromName(SkinID);
}

// ========== Companion Selection ==========

FText UT66LocalizationSubsystem::GetText_SelectCompanion() const
{
	return NSLOCTEXT("T66.CompanionSelection", "Title", "SELECT COMPANION");
}

FText UT66LocalizationSubsystem::GetText_CompanionGrid() const
{
	return NSLOCTEXT("T66.CompanionSelection", "Grid", "COMPANION GRID");
}

FText UT66LocalizationSubsystem::GetText_NoCompanion() const
{
	return NSLOCTEXT("T66.CompanionSelection", "NoCompanion", "NO COMPANION");
}

FText UT66LocalizationSubsystem::GetText_ConfirmCompanion() const
{
	return NSLOCTEXT("T66.CompanionSelection", "ConfirmCompanion", "CONFIRM COMPANION");
}

FText UT66LocalizationSubsystem::GetText_Union() const
{
	return NSLOCTEXT("T66.CompanionSelection", "Union", "UNION");
}

FText UT66LocalizationSubsystem::GetText_UnionProgressFormat() const
{
	return NSLOCTEXT("T66.CompanionSelection", "UnionProgressFormat", "Union: {0}/{1}");
}

FText UT66LocalizationSubsystem::GetText_BasicHealing() const
{
	return NSLOCTEXT("T66.CompanionSelection", "BasicHealing", "Basic Healing");
}

FText UT66LocalizationSubsystem::GetText_GoodHealing() const
{
	return NSLOCTEXT("T66.CompanionSelection", "GoodHealing", "Good Healing");
}

FText UT66LocalizationSubsystem::GetText_MediumHealing() const
{
	return NSLOCTEXT("T66.CompanionSelection", "MediumHealing", "Medium Healing");
}

FText UT66LocalizationSubsystem::GetText_HyperHealing() const
{
	return NSLOCTEXT("T66.CompanionSelection", "HyperHealing", "Hyper Healing");
}

// ========== Common ==========

FText UT66LocalizationSubsystem::GetText_Back() const
{
	return NSLOCTEXT("T66.Common", "Back", "BACK");
}

FText UT66LocalizationSubsystem::GetText_FollowMe() const
{
	return NSLOCTEXT("T66.Common", "FollowMe", "Follow me");
}

FText UT66LocalizationSubsystem::GetText_Leave() const
{
	return NSLOCTEXT("T66.Common", "Leave", "Leave");
}

FText UT66LocalizationSubsystem::GetText_Level() const
{
	return NSLOCTEXT("T66.Common", "Level", "LEVEL");
}

FText UT66LocalizationSubsystem::GetText_Confirm() const
{
	return NSLOCTEXT("T66.Common", "Confirm", "CONFIRM");
}

FText UT66LocalizationSubsystem::GetText_Cancel() const
{
	return NSLOCTEXT("T66.Common", "Cancel", "CANCEL");
}

FText UT66LocalizationSubsystem::GetText_Yes() const
{
	return NSLOCTEXT("T66.Common", "Yes", "YES");
}

FText UT66LocalizationSubsystem::GetText_No() const
{
	return NSLOCTEXT("T66.Common", "No", "NO");
}

FText UT66LocalizationSubsystem::GetText_Buy() const
{
	return NSLOCTEXT("T66.Common", "Buy", "BUY");
}

FText UT66LocalizationSubsystem::GetText_Sell() const
{
	return NSLOCTEXT("T66.Common", "Sell", "SELL");
}

FText UT66LocalizationSubsystem::GetText_Equip() const
{
	return NSLOCTEXT("T66.Common", "Equip", "EQUIP");
}

FText UT66LocalizationSubsystem::GetText_Preview() const
{
	return NSLOCTEXT("T66.Common", "Preview", "PREVIEW");
}

FText UT66LocalizationSubsystem::GetText_Vendor() const
{
	return NSLOCTEXT("T66.Vendor", "VendorTitle", "VENDOR");
}

FText UT66LocalizationSubsystem::GetText_VendorDialoguePrompt() const
{
	return NSLOCTEXT("T66.Vendor", "VendorDialoguePrompt", "What do you want?");
}

FText UT66LocalizationSubsystem::GetText_IWantToShop() const
{
	return NSLOCTEXT("T66.Vendor", "IWantToShop", "I want to shop");
}

FText UT66LocalizationSubsystem::GetText_Shop() const
{
	return NSLOCTEXT("T66.Vendor", "ShopTitle", "SHOP");
}

FText UT66LocalizationSubsystem::GetText_YourItems() const
{
	return NSLOCTEXT("T66.Vendor", "InventoryTitle", "INVENTORY");
}

FText UT66LocalizationSubsystem::GetText_Upgrade() const
{
	return NSLOCTEXT("T66.Vendor", "UpgradeTag", "UPGRADE");
}

FText UT66LocalizationSubsystem::GetText_Steal() const
{
	return NSLOCTEXT("T66.Vendor", "Steal", "STEAL");
}

FText UT66LocalizationSubsystem::GetText_Reroll() const
{
	return NSLOCTEXT("T66.Vendor", "Reroll", "REROLL");
}

// ========== Gameplay Overlays ==========

FText UT66LocalizationSubsystem::GetText_WheelSpinTitle() const
{
	return NSLOCTEXT("T66.Wheel", "Title", "WHEEL SPIN");
}

FText UT66LocalizationSubsystem::GetText_Spin() const
{
	return NSLOCTEXT("T66.Wheel", "Spin", "SPIN");
}

FText UT66LocalizationSubsystem::GetText_PressSpinToRollGold() const
{
	return NSLOCTEXT("T66.Wheel", "PressSpinToRollGold", "Press SPIN to roll gold.");
}

FText UT66LocalizationSubsystem::GetText_Spinning() const
{
	return NSLOCTEXT("T66.Wheel", "Spinning", "Spinning...");
}

FText UT66LocalizationSubsystem::GetText_YouWonGoldFormat() const
{
	return NSLOCTEXT("T66.Wheel", "YouWonGoldFormat", "You won {0} gold.");
}

FText UT66LocalizationSubsystem::GetText_IdolAltarTitle() const
{
	return NSLOCTEXT("T66.IdolAltar", "Title", "IDOL ALTAR");
}

FText UT66LocalizationSubsystem::GetText_IdolAltarHoverHint() const
{
	return NSLOCTEXT("T66.IdolAltar", "HoverHint", "Hover an idol to see its effect.");
}

FText UT66LocalizationSubsystem::GetText_IdolAltarDropHere() const
{
	return NSLOCTEXT("T66.IdolAltar", "DropHere", "DROP\nHERE");
}

FText UT66LocalizationSubsystem::GetText_IdolAltarDropAnIdolHere() const
{
	return NSLOCTEXT("T66.IdolAltar", "DropAnIdolHere", "Drop an Idol here.");
}

FText UT66LocalizationSubsystem::GetText_IdolAltarAlreadySelectedStage1() const
{
	// Repurposed: v0 now allows selecting idols multiple times; lock only when all slots are full.
	return NSLOCTEXT("T66.IdolAltar", "AllSlotsFull", "All idol slots are full.");
}

FText UT66LocalizationSubsystem::GetText_IdolAltarAlreadyEquipped() const
{
	return NSLOCTEXT("T66.IdolAltar", "AlreadyEquipped", "Already equipped.");
}

FText UT66LocalizationSubsystem::GetText_IdolAltarDragFirst() const
{
	return NSLOCTEXT("T66.IdolAltar", "DragFirst", "Drag an idol into the center slot first.");
}

FText UT66LocalizationSubsystem::GetText_IdolAltarEquipped() const
{
	return NSLOCTEXT("T66.IdolAltar", "Equipped", "Equipped.");
}

FText UT66LocalizationSubsystem::GetText_IdolTooltip(FName IdolID) const
{
	// Back-compat aliases.
	if (IdolID == FName(TEXT("Idol_Shock"))) IdolID = FName(TEXT("Idol_Lightning"));
	if (IdolID == FName(TEXT("Idol_Silence"))) IdolID = FName(TEXT("Idol_Darkness"));
	if (IdolID == FName(TEXT("Idol_Mark"))) IdolID = FName(TEXT("Idol_Light"));
	if (IdolID == FName(TEXT("Idol_Pierce"))) IdolID = FName(TEXT("Idol_Metal"));
	if (IdolID == FName(TEXT("Idol_Split"))) IdolID = FName(TEXT("Idol_Wind"));
	if (IdolID == FName(TEXT("Idol_Knockback"))) IdolID = FName(TEXT("Idol_Wood"));
	if (IdolID == FName(TEXT("Idol_Ricochet"))) IdolID = FName(TEXT("Idol_Water"));
	if (IdolID == FName(TEXT("Idol_Hex"))) IdolID = FName(TEXT("Idol_Spectral"));
	if (IdolID == FName(TEXT("Idol_Lifesteal"))) IdolID = FName(TEXT("Idol_Poison"));

	if (IdolID == FName(TEXT("Idol_Darkness"))) return NSLOCTEXT("T66.IdolAltar", "Idol_Darkness_Tooltip", "DARKNESS\nAn idol of shadowed power.");
	if (IdolID == FName(TEXT("Idol_Light"))) return NSLOCTEXT("T66.IdolAltar", "Idol_Light_Tooltip", "LIGHT\nAn idol of radiant power.");
	if (IdolID == FName(TEXT("Idol_Lightning"))) return NSLOCTEXT("T66.IdolAltar", "Idol_Lightning_Tooltip", "LIGHTNING\nAn idol of crackling power.");
	if (IdolID == FName(TEXT("Idol_Metal"))) return NSLOCTEXT("T66.IdolAltar", "Idol_Metal_Tooltip", "METAL\nAn idol of unyielding power.");
	if (IdolID == FName(TEXT("Idol_Poison"))) return NSLOCTEXT("T66.IdolAltar", "Idol_Poison_Tooltip", "POISON\nAn idol of venomous power.");
	if (IdolID == FName(TEXT("Idol_Spectral"))) return NSLOCTEXT("T66.IdolAltar", "Idol_Spectral_Tooltip", "SPECTRAL\nAn idol of unseen power.");
	if (IdolID == FName(TEXT("Idol_Water"))) return NSLOCTEXT("T66.IdolAltar", "Idol_Water_Tooltip", "WATER\nAn idol of flowing power.");
	if (IdolID == FName(TEXT("Idol_Wind"))) return NSLOCTEXT("T66.IdolAltar", "Idol_Wind_Tooltip", "WIND\nAn idol of cutting power.");
	if (IdolID == FName(TEXT("Idol_Wood"))) return NSLOCTEXT("T66.IdolAltar", "Idol_Wood_Tooltip", "WOOD\nAn idol of growing power.");

	if (IdolID == FName(TEXT("Idol_Frost"))) return NSLOCTEXT("T66.IdolAltar", "Idol_Frost_Tooltip", "FROST\nAn idol of frozen power.");
	if (IdolID == FName(TEXT("Idol_Glue"))) return NSLOCTEXT("T66.IdolAltar", "Idol_Glue_Tooltip", "GLUE\nAn idol of binding power.");
	if (IdolID == FName(TEXT("Idol_Fire"))) return NSLOCTEXT("T66.IdolAltar", "Idol_Fire_Tooltip", "FIRE\nAn idol of burning power.");
	return NSLOCTEXT("T66.IdolAltar", "IdolTooltipUnknown", "IDOL\nUnknown.");
}

// ========== Language Select ==========

FText UT66LocalizationSubsystem::GetText_SelectLanguage() const
{
	return NSLOCTEXT("T66.LanguageSelect", "Title", "Select Language");
}

FText UT66LocalizationSubsystem::GetText_LangButton() const
{
	return NSLOCTEXT("T66.LanguageSelect", "LangButton", "Lang");
}

// ========== Quit Confirmation ==========

FText UT66LocalizationSubsystem::GetText_QuitConfirmTitle() const
{
	return NSLOCTEXT("T66.QuitConfirm", "Title", "QUIT GAME?");
}

FText UT66LocalizationSubsystem::GetText_QuitConfirmMessage() const
{
	return NSLOCTEXT("T66.QuitConfirm", "Message", "Are you sure you want to quit?");
}

FText UT66LocalizationSubsystem::GetText_YesQuit() const
{
	return NSLOCTEXT("T66.QuitConfirm", "Quit", "YES, I WANT TO QUIT");
}

FText UT66LocalizationSubsystem::GetText_NoStay() const
{
	return NSLOCTEXT("T66.QuitConfirm", "Stay", "NO, I WANT TO STAY");
}

// ========== Difficulties ==========

FText UT66LocalizationSubsystem::GetText_Difficulty(ET66Difficulty Difficulty) const
{
	switch (Difficulty)
	{
	case ET66Difficulty::Easy: return GetText_Easy();
	case ET66Difficulty::Medium: return GetText_Medium();
	case ET66Difficulty::Hard: return GetText_Hard();
	case ET66Difficulty::VeryHard: return GetText_VeryHard();
	case ET66Difficulty::Impossible: return GetText_Impossible();
	case ET66Difficulty::Perdition: return GetText_Perdition();
	case ET66Difficulty::Final: return GetText_Final();
	default: return NSLOCTEXT("T66.Difficulty", "Unknown", "???");
	}
}

FText UT66LocalizationSubsystem::GetText_Easy() const
{
	return NSLOCTEXT("T66.Difficulty", "Easy", "Easy");
}

FText UT66LocalizationSubsystem::GetText_Medium() const
{
	return NSLOCTEXT("T66.Difficulty", "Medium", "Medium");
}

FText UT66LocalizationSubsystem::GetText_Hard() const
{
	return NSLOCTEXT("T66.Difficulty", "Hard", "Hard");
}

FText UT66LocalizationSubsystem::GetText_VeryHard() const
{
	return NSLOCTEXT("T66.Difficulty", "VeryHard", "Very Hard");
}

FText UT66LocalizationSubsystem::GetText_Impossible() const
{
	return NSLOCTEXT("T66.Difficulty", "Impossible", "Impossible");
}

FText UT66LocalizationSubsystem::GetText_Perdition() const
{
	return NSLOCTEXT("T66.Difficulty", "Perdition", "Perdition");
}

FText UT66LocalizationSubsystem::GetText_Final() const
{
	return NSLOCTEXT("T66.Difficulty", "Final", "Final");
}

// ========== Save Slots ==========

FText UT66LocalizationSubsystem::GetText_SaveSlots() const
{
	return NSLOCTEXT("T66.SaveSlots", "Title", "SAVE SLOTS");
}

FText UT66LocalizationSubsystem::GetText_EmptySlot() const
{
	return NSLOCTEXT("T66.SaveSlots", "EmptySlot", "EMPTY SLOT");
}

FText UT66LocalizationSubsystem::GetText_Stage() const
{
	return NSLOCTEXT("T66.SaveSlots", "Stage", "STAGE");
}

// ========== Pause Menu ==========

FText UT66LocalizationSubsystem::GetText_Resume() const
{
	return NSLOCTEXT("T66.PauseMenu", "Resume", "RESUME GAME");
}

FText UT66LocalizationSubsystem::GetText_SaveAndQuit() const
{
	return NSLOCTEXT("T66.PauseMenu", "SaveAndQuit", "SAVE AND QUIT GAME");
}

FText UT66LocalizationSubsystem::GetText_Restart() const
{
	return NSLOCTEXT("T66.PauseMenu", "Restart", "RESTART");
}

FText UT66LocalizationSubsystem::GetText_ReportBug() const
{
	return NSLOCTEXT("T66.PauseMenu", "ReportBug", "REPORT BUG");
}

// ========== Report Bug ==========

FText UT66LocalizationSubsystem::GetText_ReportBugTitle() const
{
	return NSLOCTEXT("T66.ReportBug", "Title", "REPORT BUG");
}

FText UT66LocalizationSubsystem::GetText_ReportBugSubmit() const
{
	return NSLOCTEXT("T66.ReportBug", "Submit", "SUBMIT");
}

// ========== Account Status ==========

FText UT66LocalizationSubsystem::GetText_AccountStatus() const
{
	return NSLOCTEXT("T66.AccountStatus", "Title", "ACCOUNT STATUS");
}

FText UT66LocalizationSubsystem::GetText_AccountStatus_SuspicionHeadline() const
{
	return NSLOCTEXT("T66.AccountStatus", "SuspicionHeadline", "RUN UNDER SUSPICION OF CHEATING");
}

FText UT66LocalizationSubsystem::GetText_AccountStatus_CertaintyHeadline() const
{
	return NSLOCTEXT("T66.AccountStatus", "CertaintyHeadline", "FORBIDDEN EVENT DETECTED");
}

FText UT66LocalizationSubsystem::GetText_AccountStatus_ReasonLabel() const
{
	return NSLOCTEXT("T66.AccountStatus", "ReasonLabel", "Reason");
}

FText UT66LocalizationSubsystem::GetText_AccountStatus_RunInQuestion() const
{
	return NSLOCTEXT("T66.AccountStatus", "RunInQuestion", "RUN IN QUESTION");
}

FText UT66LocalizationSubsystem::GetText_AccountStatus_AppealTitle() const
{
	return NSLOCTEXT("T66.AccountStatus", "AppealTitle", "APPEAL");
}

FText UT66LocalizationSubsystem::GetText_AccountStatus_AppealHint() const
{
	return NSLOCTEXT("T66.AccountStatus", "AppealHint", "Write your appeal message here...");
}

FText UT66LocalizationSubsystem::GetText_AccountStatus_EvidenceHint() const
{
	return NSLOCTEXT("T66.AccountStatus", "EvidenceHint", "Evidence link (optional)...");
}

FText UT66LocalizationSubsystem::GetText_AccountStatus_SubmitAppeal() const
{
	return NSLOCTEXT("T66.AccountStatus", "SubmitAppeal", "SUBMIT APPEAL");
}

FText UT66LocalizationSubsystem::GetText_AccountStatus_NoAppealAvailable() const
{
	return NSLOCTEXT("T66.AccountStatus", "NoAppealAvailable", "NO APPEAL AVAILABLE.");
}

FText UT66LocalizationSubsystem::GetText_AccountStatus_AppealNotSubmitted() const
{
	return NSLOCTEXT("T66.AccountStatus", "AppealNotSubmitted", "Appeal not submitted");
}

FText UT66LocalizationSubsystem::GetText_AccountStatus_AppealUnderReview() const
{
	return NSLOCTEXT("T66.AccountStatus", "AppealUnderReview", "Appeal under review");
}

FText UT66LocalizationSubsystem::GetText_AccountStatus_AppealDenied() const
{
	return NSLOCTEXT("T66.AccountStatus", "AppealDenied", "Appeal denied");
}

FText UT66LocalizationSubsystem::GetText_AccountStatus_AppealApproved() const
{
	return NSLOCTEXT("T66.AccountStatus", "AppealApproved", "Appeal approved");
}

// ========== Gameplay HUD ==========

FText UT66LocalizationSubsystem::GetText_GoldFormat() const
{
	return NSLOCTEXT("T66.GameplayHUD", "GoldFormat", "Gold: {0}");
}

FText UT66LocalizationSubsystem::GetText_OweFormat() const
{
	return NSLOCTEXT("T66.GameplayHUD", "OweFormat", "Debt: {0}");
}

FText UT66LocalizationSubsystem::GetText_StageNumberFormat() const
{
	return NSLOCTEXT("T66.GameplayHUD", "StageNumberFormat", "Stage number: {0}");
}

FText UT66LocalizationSubsystem::GetText_BountyLabel() const
{
	return NSLOCTEXT("T66.GameplayHUD", "BountyLabel", "High Score:");
}

FText UT66LocalizationSubsystem::GetText_PortraitPlaceholder() const
{
	return NSLOCTEXT("T66.GameplayHUD", "PortraitLabel", "PORTRAIT");
}

FText UT66LocalizationSubsystem::GetText_MinimapPlaceholder() const
{
	return NSLOCTEXT("T66.GameplayHUD", "MinimapLabel", "MINIMAP");
}

// ========== Run Summary ==========

FText UT66LocalizationSubsystem::GetText_RunSummaryTitle() const
{
	return NSLOCTEXT("T66.RunSummary", "Title", "RUN SUMMARY");
}

FText UT66LocalizationSubsystem::GetText_RunSummaryStageReachedBountyFormat() const
{
	return NSLOCTEXT("T66.RunSummary", "StageReachedBountyFormat", "Stage Reached: {0}  |  High Score: {1}");
}

FText UT66LocalizationSubsystem::GetText_RunSummaryPreviewPlaceholder() const
{
	return NSLOCTEXT("T66.RunSummary", "PreviewPlaceholder", "3D Preview");
}

FText UT66LocalizationSubsystem::GetText_MainMenu() const
{
	return NSLOCTEXT("T66.RunSummary", "MainMenu", "MAIN MENU");
}

FText UT66LocalizationSubsystem::GetText_NewPersonalHighScore() const
{
	return NSLOCTEXT("T66.RunSummary", "NewPersonalHighScore", "New Personal High Score");
}

FText UT66LocalizationSubsystem::GetText_NewPersonalBestTime() const
{
	return NSLOCTEXT("T66.RunSummary", "NewPersonalBestTime", "New Personal Best Time");
}

// ========== Run Log ==========

FText UT66LocalizationSubsystem::GetText_RunLog_StageFormat() const
{
	return NSLOCTEXT("T66.RunLog", "StageFormat", "Stage {0}");
}

FText UT66LocalizationSubsystem::GetText_RunLog_PickedUpFormat() const
{
	return NSLOCTEXT("T66.RunLog", "PickedUpFormat", "Picked up {0}");
}

FText UT66LocalizationSubsystem::GetText_RunLog_GoldFormat() const
{
	return NSLOCTEXT("T66.RunLog", "GoldFormat", "Gold: {0}");
}

FText UT66LocalizationSubsystem::GetText_RunLog_KillFormat() const
{
	return NSLOCTEXT("T66.RunLog", "KillFormat", "Kill +{0}");
}

// ========== Gambler ==========

FText UT66LocalizationSubsystem::GetText_Gambler() const
{
	return NSLOCTEXT("T66.Gambler", "Title", "GAMBLER");
}

FText UT66LocalizationSubsystem::GetText_Casino() const
{
	return NSLOCTEXT("T66.Gambler", "Casino", "CASINO");
}

FText UT66LocalizationSubsystem::GetText_Bank() const
{
	// Shared label used in both Vendor and Gambler UI.
	return NSLOCTEXT("T66.Vendor", "BankTitle", "BANK");
}

FText UT66LocalizationSubsystem::GetText_Games() const
{
	return NSLOCTEXT("T66.Gambler", "Games", "GAMES");
}

FText UT66LocalizationSubsystem::GetText_Anger() const
{
	return NSLOCTEXT("T66.Common", "Anger", "ANGER");
}

FText UT66LocalizationSubsystem::GetText_GamblerDialoguePrompt() const
{
	return NSLOCTEXT("T66.Gambler", "DialoguePrompt", "What do you want?");
}

FText UT66LocalizationSubsystem::GetText_LetMeGamble() const
{
	return NSLOCTEXT("T66.Gambler", "LetMeGamble", "LET ME GAMBLE");
}

FText UT66LocalizationSubsystem::GetText_TeleportMeToYourBrother() const
{
	return NSLOCTEXT("T66.Gambler", "TeleportMeToYourBrother", "TELEPORT ME TO YOUR BROTHER");
}

FText UT66LocalizationSubsystem::GetText_TeleportDisabledBossActive() const
{
	return NSLOCTEXT("T66.Gambler", "TeleportDisabledBossActive", "Teleport disabled (boss encounter started).");
}

FText UT66LocalizationSubsystem::GetText_Bet() const
{
	return NSLOCTEXT("T66.Gambler", "Bet", "Bet");
}

FText UT66LocalizationSubsystem::GetText_Borrow() const
{
	return NSLOCTEXT("T66.Gambler", "Borrow", "Borrow");
}

FText UT66LocalizationSubsystem::GetText_Payback() const
{
	return NSLOCTEXT("T66.Gambler", "Payback", "Payback");
}

FText UT66LocalizationSubsystem::GetText_Max() const
{
	return NSLOCTEXT("T66.Gambler", "Max", "MAX");
}

FText UT66LocalizationSubsystem::GetText_CoinFlip() const
{
	return NSLOCTEXT("T66.Gambler", "CoinFlip", "COIN FLIP");
}

FText UT66LocalizationSubsystem::GetText_RockPaperScissors() const
{
	return NSLOCTEXT("T66.Gambler", "RockPaperScissors", "ROCK PAPER SCISSORS");
}

FText UT66LocalizationSubsystem::GetText_FindTheBall() const
{
	return NSLOCTEXT("T66.Gambler", "FindTheBall", "FIND THE BALL");
}

FText UT66LocalizationSubsystem::GetText_ChooseHeadsOrTails() const
{
	return NSLOCTEXT("T66.Gambler", "ChooseHeadsOrTails", "Choose Heads or Tails.");
}

FText UT66LocalizationSubsystem::GetText_Heads() const
{
	return NSLOCTEXT("T66.Gambler", "Heads", "HEADS");
}

FText UT66LocalizationSubsystem::GetText_Tails() const
{
	return NSLOCTEXT("T66.Gambler", "Tails", "TAILS");
}

FText UT66LocalizationSubsystem::GetText_PickOne() const
{
	return NSLOCTEXT("T66.Gambler", "PickOne", "Pick one.");
}

FText UT66LocalizationSubsystem::GetText_Rock() const
{
	return NSLOCTEXT("T66.Gambler", "Rock", "Rock");
}

FText UT66LocalizationSubsystem::GetText_Paper() const
{
	return NSLOCTEXT("T66.Gambler", "Paper", "Paper");
}

FText UT66LocalizationSubsystem::GetText_Scissors() const
{
	return NSLOCTEXT("T66.Gambler", "Scissors", "Scissors");
}

FText UT66LocalizationSubsystem::GetText_PickACup() const
{
	return NSLOCTEXT("T66.Gambler", "PickACup", "Pick a cup.");
}

FText UT66LocalizationSubsystem::GetText_Cup(int32 CupIndex1Based) const
{
	return FText::Format(NSLOCTEXT("T66.Gambler", "CupFormat", "CUP {0}"), FText::AsNumber(CupIndex1Based));
}

FText UT66LocalizationSubsystem::GetText_ResultDash() const
{
	return NSLOCTEXT("T66.Gambler", "ResultDash", "Result: -");
}

FText UT66LocalizationSubsystem::GetText_Rolling() const
{
	return NSLOCTEXT("T66.Gambler", "Rolling", "Rolling...");
}

FText UT66LocalizationSubsystem::GetText_CheatPromptTitle() const
{
	return NSLOCTEXT("T66.Gambler", "CheatPromptTitle", "Cheat?");
}

FText UT66LocalizationSubsystem::GetText_CheatPromptBody() const
{
	return NSLOCTEXT("T66.Gambler", "CheatPromptBody", "Cheating increases Anger.");
}

FText UT66LocalizationSubsystem::GetText_GambleAmountMustBePositive() const
{
	return NSLOCTEXT("T66.Gambler", "GambleAmountMustBePositive", "Gamble amount must be > 0.");
}

FText UT66LocalizationSubsystem::GetText_BorrowAmountMustBePositive() const
{
	return NSLOCTEXT("T66.Gambler", "BorrowAmountMustBePositive", "Borrow amount must be > 0.");
}

FText UT66LocalizationSubsystem::GetText_PaybackAmountMustBePositive() const
{
	return NSLOCTEXT("T66.Gambler", "PaybackAmountMustBePositive", "Payback amount must be > 0.");
}

FText UT66LocalizationSubsystem::GetText_NotEnoughGold() const
{
	return NSLOCTEXT("T66.Gambler", "NotEnoughGold", "Not enough gold.");
}

FText UT66LocalizationSubsystem::GetText_NotEnoughGoldOrNoDebt() const
{
	return NSLOCTEXT("T66.Gambler", "NotEnoughGoldOrNoDebt", "Not enough gold (or no debt).");
}

FText UT66LocalizationSubsystem::GetText_BorrowedAmountFormat() const
{
	return NSLOCTEXT("T66.Gambler", "BorrowedAmountFormat", "Borrowed {0}.");
}

FText UT66LocalizationSubsystem::GetText_PaidBackAmountFormat() const
{
	return NSLOCTEXT("T66.Gambler", "PaidBackAmountFormat", "Paid back {0}.");
}

FText UT66LocalizationSubsystem::GetText_WinPlusAmountFormat() const
{
	return NSLOCTEXT("T66.Gambler", "WinPlusAmountFormat", "WIN (+{0})");
}

FText UT66LocalizationSubsystem::GetText_Win() const
{
	return NSLOCTEXT("T66.Gambler", "Win", "WIN");
}

FText UT66LocalizationSubsystem::GetText_Lose() const
{
	return NSLOCTEXT("T66.Gambler", "Lose", "LOSE");
}

FText UT66LocalizationSubsystem::GetText_CoinFlipResultFormat() const
{
	return NSLOCTEXT("T66.Gambler", "CoinFlipResultFormat", "Result: {0} — {1}");
}

FText UT66LocalizationSubsystem::GetText_RpsResultFormat() const
{
	return NSLOCTEXT("T66.Gambler", "RpsResultFormat", "You: {0}  |  Gambler: {1}  —  {2}");
}

FText UT66LocalizationSubsystem::GetText_FindBallResultFormat() const
{
	return NSLOCTEXT("T66.Gambler", "FindBallResultFormat", "Ball was under {0} — {1}");
}
