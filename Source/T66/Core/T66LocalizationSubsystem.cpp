// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66LocalizationSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Kismet/GameplayStatics.h"

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
		SaveLanguagePreference();
		OnLanguageChanged.Broadcast(NewLanguage);
	}
}

FText UT66LocalizationSubsystem::GetLanguageDisplayName(ET66Language Language) const
{
	switch (Language)
	{
	case ET66Language::English: return FText::FromString(TEXT("English"));
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Português (Brasil)"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("繁體中文"));
	default: return FText::FromString(TEXT("Unknown"));
	}
}

TArray<ET66Language> UT66LocalizationSubsystem::GetAvailableLanguages() const
{
	TArray<ET66Language> Languages;
	Languages.Add(ET66Language::English);
	Languages.Add(ET66Language::BrazilianPortuguese);
	Languages.Add(ET66Language::ChineseTraditional);
	return Languages;
}

void UT66LocalizationSubsystem::LoadSavedLanguage()
{
	// TODO: Load from SaveGame when save system is implemented
	CurrentLanguage = ET66Language::English;
}

void UT66LocalizationSubsystem::SaveLanguagePreference()
{
	// TODO: Save to SaveGame when save system is implemented
}

// ========== Hero Names ==========
// All hero names are centralized here for localization

FText UT66LocalizationSubsystem::GetText_HeroName(FName HeroID) const
{
	// Hero name lookup table - add new heroes here
	// Format: HeroID -> { English, Portuguese, Chinese Traditional }
	
	if (HeroID == FName(TEXT("Hero_AliceInWonderlandRabbit")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Coelho do País das Maravilhas"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("愛麗絲夢遊仙境兔"));
		default: return FText::FromString(TEXT("Alice in Wonderland Rabbit"));
		}
	}
	if (HeroID == FName(TEXT("Hero_LuBu")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Lu Bu"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("呂布"));
		default: return FText::FromString(TEXT("Lu Bu"));
		}
	}
	if (HeroID == FName(TEXT("Hero_LeonardoDaVinci")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Leonardo da Vinci"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("達文西"));
		default: return FText::FromString(TEXT("Leonardo da Vinci"));
		}
	}
	if (HeroID == FName(TEXT("Hero_Yakub")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Yakub"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("雅各布"));
		default: return FText::FromString(TEXT("Yakub"));
		}
	}
	if (HeroID == FName(TEXT("Hero_KingArthur")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Rei Arthur"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("亞瑟王"));
		default: return FText::FromString(TEXT("King Arthur"));
		}
	}
	if (HeroID == FName(TEXT("Hero_MiyamotoMusashi")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Miyamoto Musashi"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("宮本武藏"));
		default: return FText::FromString(TEXT("Miyamoto Musashi"));
		}
	}
	if (HeroID == FName(TEXT("Hero_CaptainJackSparrow")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Capitão Jack Sparrow"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("傑克船長"));
		default: return FText::FromString(TEXT("Captain Jack Sparrow"));
		}
	}
	if (HeroID == FName(TEXT("Hero_SoloLeveler")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Solo Leveler"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("我獨自升級"));
		default: return FText::FromString(TEXT("Solo Leveler"));
		}
	}
	if (HeroID == FName(TEXT("Hero_Saitama")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Saitama"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("埼玉"));
		default: return FText::FromString(TEXT("Saitama"));
		}
	}
	if (HeroID == FName(TEXT("Hero_RoboGoon")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("RoboGoon"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("機械狂徒"));
		default: return FText::FromString(TEXT("RoboGoon"));
		}
	}
	if (HeroID == FName(TEXT("Hero_GeorgeWashington")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("George Washington"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("喬治·華盛頓"));
		default: return FText::FromString(TEXT("George Washington"));
		}
	}
	if (HeroID == FName(TEXT("Hero_Cain")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Caim"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("該隱"));
		default: return FText::FromString(TEXT("Cain"));
		}
	}
	if (HeroID == FName(TEXT("Hero_BillyTheKid")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Billy the Kid"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("乖仔比利"));
		default: return FText::FromString(TEXT("Billy the Kid"));
		}
	}
	if (HeroID == FName(TEXT("Hero_RoachKing")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Rei Barata"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("蟑螂王"));
		default: return FText::FromString(TEXT("Roach King"));
		}
	}
	if (HeroID == FName(TEXT("Hero_Goblino")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Goblino"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("哥布林"));
		default: return FText::FromString(TEXT("Goblino"));
		}
	}
	if (HeroID == FName(TEXT("Hero_BulkBite")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("BulkBite"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("巨咬"));
		default: return FText::FromString(TEXT("BulkBite"));
		}
	}
	if (HeroID == FName(TEXT("Hero_HoboWanderer")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Andarilho"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("流浪漢"));
		default: return FText::FromString(TEXT("HoboWanderer"));
		}
	}
	if (HeroID == FName(TEXT("Hero_DryHumor")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Humor Seco"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("乾笑"));
		default: return FText::FromString(TEXT("DryHumor"));
		}
	}
	if (HeroID == FName(TEXT("Hero_LyricVoi")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("LyricVoi"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("詩聲"));
		default: return FText::FromString(TEXT("LyricVoi"));
		}
	}
	if (HeroID == FName(TEXT("Hero_Jesterma")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Jesterma"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("小丑大師"));
		default: return FText::FromString(TEXT("Jesterma"));
		}
	}
	if (HeroID == FName(TEXT("Hero_NorthKing")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Rei do Norte"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("北方之王"));
		default: return FText::FromString(TEXT("NorthKing"));
		}
	}
	if (HeroID == FName(TEXT("Hero_Peakwarden")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Guardião do Pico"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("巔峰守護者"));
		default: return FText::FromString(TEXT("Peakwarden"));
		}
	}
	if (HeroID == FName(TEXT("Hero_QuinnHex")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("QuinnHex"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("奎恩咒"));
		default: return FText::FromString(TEXT("QuinnHex"));
		}
	}
	if (HeroID == FName(TEXT("Hero_SandSultan")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Sultão das Areias"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("沙漠蘇丹"));
		default: return FText::FromString(TEXT("SandSultan"));
		}
	}
	if (HeroID == FName(TEXT("Hero_CharNut")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Amendoim Queimado"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("焦果仁"));
		default: return FText::FromString(TEXT("CharNut"));
		}
	}
	if (HeroID == FName(TEXT("Hero_Wraithveil")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Véu Espectral"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("幽靈面紗"));
		default: return FText::FromString(TEXT("Wraithveil"));
		}
	}

	// Fallback: return the HeroID as text
	return FText::FromName(HeroID);
}

// ========== Companion Names ==========

FText UT66LocalizationSubsystem::GetText_CompanionName(FName CompanionID) const
{
	if (CompanionID == FName(TEXT("Companion_Luna")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Luna"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("月娜"));
		default: return FText::FromString(TEXT("Luna"));
		}
	}
	if (CompanionID == FName(TEXT("Companion_Ember")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Brasa"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("燼火"));
		default: return FText::FromString(TEXT("Ember"));
		}
	}
	if (CompanionID == FName(TEXT("Companion_Shade")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Sombra"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("暗影"));
		default: return FText::FromString(TEXT("Shade"));
		}
	}

	// Fallback: return the CompanionID as text
	return FText::FromName(CompanionID);
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
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("TRIBULAÇÃO 66"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("苦難 66"));
	default: return FText::FromString(TEXT("TRIBULATION 66"));
	}
}

FText UT66LocalizationSubsystem::GetText_NewGame() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("NOVO JOGO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("新遊戲"));
	default: return FText::FromString(TEXT("NEW GAME"));
	}
}

FText UT66LocalizationSubsystem::GetText_LoadGame() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("CARREGAR JOGO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("載入遊戲"));
	default: return FText::FromString(TEXT("LOAD GAME"));
	}
}

FText UT66LocalizationSubsystem::GetText_Settings() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("CONFIGURAÇÕES"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("設定"));
	default: return FText::FromString(TEXT("SETTINGS"));
	}
}

FText UT66LocalizationSubsystem::GetText_Achievements() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("CONQUISTAS"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("成就"));
	default: return FText::FromString(TEXT("ACHIEVEMENTS"));
	}
}

FText UT66LocalizationSubsystem::GetText_Quit() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("SAIR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("離開"));
	default: return FText::FromString(TEXT("QUIT"));
	}
}

// ========== Leaderboard ==========

FText UT66LocalizationSubsystem::GetText_Leaderboard() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("CLASSIFICAÇÃO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("排行榜"));
	default: return FText::FromString(TEXT("LEADERBOARD"));
	}
}

FText UT66LocalizationSubsystem::GetText_Global() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("GLOBAL"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("全球"));
	default: return FText::FromString(TEXT("GLOBAL"));
	}
}

FText UT66LocalizationSubsystem::GetText_Friends() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("AMIGOS"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("好友"));
	default: return FText::FromString(TEXT("FRIENDS"));
	}
}

FText UT66LocalizationSubsystem::GetText_Streamers() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("STREAMERS"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("實況主"));
	default: return FText::FromString(TEXT("STREAMERS"));
	}
}

FText UT66LocalizationSubsystem::GetText_SoloRuns() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("SOLO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("單人"));
	default: return FText::FromString(TEXT("SOLO"));
	}
}

FText UT66LocalizationSubsystem::GetText_DuoRuns() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("DUPLA"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("雙人"));
	default: return FText::FromString(TEXT("DUO"));
	}
}

FText UT66LocalizationSubsystem::GetText_TrioRuns() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("TRIO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("三人"));
	default: return FText::FromString(TEXT("TRIO"));
	}
}

// ========== Party Size Picker ==========

FText UT66LocalizationSubsystem::GetText_SelectPartySize() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("SELECIONE O TAMANHO DO GRUPO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("選擇隊伍人數"));
	default: return FText::FromString(TEXT("SELECT PARTY SIZE"));
	}
}

FText UT66LocalizationSubsystem::GetText_Solo() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("SOLO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("單人"));
	default: return FText::FromString(TEXT("SOLO"));
	}
}

FText UT66LocalizationSubsystem::GetText_Duo() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("DUPLA"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("雙人"));
	default: return FText::FromString(TEXT("DUO"));
	}
}

FText UT66LocalizationSubsystem::GetText_Trio() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("TRIO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("三人"));
	default: return FText::FromString(TEXT("TRIO"));
	}
}

// ========== Hero Selection ==========

FText UT66LocalizationSubsystem::GetText_SelectYourHero() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("SELECIONE SEU HERÓI"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("選擇你的英雄"));
	default: return FText::FromString(TEXT("SELECT YOUR HERO"));
	}
}

FText UT66LocalizationSubsystem::GetText_HeroGrid() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("GRADE DE HERÓIS"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("英雄列表"));
	default: return FText::FromString(TEXT("HERO GRID"));
	}
}

FText UT66LocalizationSubsystem::GetText_ChooseCompanion() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("ESCOLHER COMPANHEIRO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("選擇同伴"));
	default: return FText::FromString(TEXT("CHOOSE COMPANION"));
	}
}

FText UT66LocalizationSubsystem::GetText_EnterTheTribulation() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("ENTRAR NA TRIBULAÇÃO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("進入苦難"));
	default: return FText::FromString(TEXT("ENTER THE TRIBULATION"));
	}
}

FText UT66LocalizationSubsystem::GetText_TheLab() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("O LABORATÓRIO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("實驗室"));
	default: return FText::FromString(TEXT("THE LAB"));
	}
}

FText UT66LocalizationSubsystem::GetText_BodyTypeA() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("TIPO A"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("類型 A"));
	default: return FText::FromString(TEXT("TYPE A"));
	}
}

FText UT66LocalizationSubsystem::GetText_BodyTypeB() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("TIPO B"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("類型 B"));
	default: return FText::FromString(TEXT("TYPE B"));
	}
}

FText UT66LocalizationSubsystem::GetText_Skins() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("SKINS"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("外觀"));
	default: return FText::FromString(TEXT("SKINS"));
	}
}

FText UT66LocalizationSubsystem::GetText_HeroInfo() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("INFO DO HERÓI"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("英雄資訊"));
	default: return FText::FromString(TEXT("HERO INFO"));
	}
}

FText UT66LocalizationSubsystem::GetText_Lore() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("HISTÓRIA"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("背景故事"));
	default: return FText::FromString(TEXT("LORE"));
	}
}

// ========== Skin Names ==========

FText UT66LocalizationSubsystem::GetText_SkinName(FName SkinID) const
{
	if (SkinID == FName(TEXT("Default")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Padrão"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("預設"));
		default: return FText::FromString(TEXT("Default"));
		}
	}
	if (SkinID == FName(TEXT("Golden")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Campeão Dourado"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("黃金冠軍"));
		default: return FText::FromString(TEXT("Golden Champion"));
		}
	}
	if (SkinID == FName(TEXT("Shadow")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Andarilho das Sombras"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("暗影行者"));
		default: return FText::FromString(TEXT("Shadow Walker"));
		}
	}
	if (SkinID == FName(TEXT("Infernal")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Fúria Infernal"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("煉獄之怒"));
		default: return FText::FromString(TEXT("Infernal Wrath"));
		}
	}
	// Fallback
	return FText::FromName(SkinID);
}

// ========== Companion Selection ==========

FText UT66LocalizationSubsystem::GetText_SelectCompanion() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("SELECIONE COMPANHEIRO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("選擇同伴"));
	default: return FText::FromString(TEXT("SELECT COMPANION"));
	}
}

FText UT66LocalizationSubsystem::GetText_CompanionGrid() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("GRADE DE COMPANHEIROS"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("同伴列表"));
	default: return FText::FromString(TEXT("COMPANION GRID"));
	}
}

FText UT66LocalizationSubsystem::GetText_NoCompanion() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("SEM COMPANHEIRO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("無同伴"));
	default: return FText::FromString(TEXT("NO COMPANION"));
	}
}

FText UT66LocalizationSubsystem::GetText_ConfirmCompanion() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("CONFIRMAR COMPANHEIRO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("確認同伴"));
	default: return FText::FromString(TEXT("CONFIRM COMPANION"));
	}
}

// ========== Common ==========

FText UT66LocalizationSubsystem::GetText_Back() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("VOLTAR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("返回"));
	default: return FText::FromString(TEXT("BACK"));
	}
}

FText UT66LocalizationSubsystem::GetText_Confirm() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("CONFIRMAR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("確認"));
	default: return FText::FromString(TEXT("CONFIRM"));
	}
}

FText UT66LocalizationSubsystem::GetText_Cancel() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("CANCELAR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("取消"));
	default: return FText::FromString(TEXT("CANCEL"));
	}
}

FText UT66LocalizationSubsystem::GetText_Yes() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("SIM"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("是"));
	default: return FText::FromString(TEXT("YES"));
	}
}

FText UT66LocalizationSubsystem::GetText_No() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("NÃO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("否"));
	default: return FText::FromString(TEXT("NO"));
	}
}

FText UT66LocalizationSubsystem::GetText_Buy() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("COMPRAR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("購買"));
	default: return FText::FromString(TEXT("BUY"));
	}
}

FText UT66LocalizationSubsystem::GetText_Sell() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("VENDER"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("出售"));
	default: return FText::FromString(TEXT("SELL"));
	}
}

FText UT66LocalizationSubsystem::GetText_Equip() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("EQUIPAR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("裝備"));
	default: return FText::FromString(TEXT("EQUIP"));
	}
}

// ========== Language Select ==========

FText UT66LocalizationSubsystem::GetText_SelectLanguage() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("SELECIONAR IDIOMA"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("選擇語言"));
	default: return FText::FromString(TEXT("SELECT LANGUAGE"));
	}
}

// ========== Quit Confirmation ==========

FText UT66LocalizationSubsystem::GetText_QuitConfirmTitle() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("CONFIRMAR SAÍDA"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("確認離開"));
	default: return FText::FromString(TEXT("CONFIRM QUIT"));
	}
}

FText UT66LocalizationSubsystem::GetText_QuitConfirmMessage() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Tem certeza que deseja sair?"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("確定要離開遊戲嗎？"));
	default: return FText::FromString(TEXT("Are you sure you want to quit?"));
	}
}

FText UT66LocalizationSubsystem::GetText_YesQuit() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("SIM, QUERO SAIR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("是的，我要離開"));
	default: return FText::FromString(TEXT("YES, I WANT TO QUIT"));
	}
}

FText UT66LocalizationSubsystem::GetText_NoStay() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("NÃO, QUERO FICAR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("不，我要留下"));
	default: return FText::FromString(TEXT("NO, I WANT TO STAY"));
	}
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
	default: return FText::FromString(TEXT("Unknown"));
	}
}

FText UT66LocalizationSubsystem::GetText_Easy() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("FÁCIL"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("簡單"));
	default: return FText::FromString(TEXT("EASY"));
	}
}

FText UT66LocalizationSubsystem::GetText_Medium() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("MÉDIO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("普通"));
	default: return FText::FromString(TEXT("MEDIUM"));
	}
}

FText UT66LocalizationSubsystem::GetText_Hard() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("DIFÍCIL"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("困難"));
	default: return FText::FromString(TEXT("HARD"));
	}
}

FText UT66LocalizationSubsystem::GetText_VeryHard() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("MUITO DIFÍCIL"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("非常困難"));
	default: return FText::FromString(TEXT("VERY HARD"));
	}
}

FText UT66LocalizationSubsystem::GetText_Impossible() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("IMPOSSÍVEL"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("不可能"));
	default: return FText::FromString(TEXT("IMPOSSIBLE"));
	}
}

FText UT66LocalizationSubsystem::GetText_Perdition() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("PERDIÇÃO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("地獄"));
	default: return FText::FromString(TEXT("PERDITION"));
	}
}

FText UT66LocalizationSubsystem::GetText_Final() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("FINAL"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("終極"));
	default: return FText::FromString(TEXT("FINAL"));
	}
}

// ========== Save Slots ==========

FText UT66LocalizationSubsystem::GetText_SaveSlots() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("SLOTS DE SALVAMENTO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("存檔欄位"));
	default: return FText::FromString(TEXT("SAVE SLOTS"));
	}
}

FText UT66LocalizationSubsystem::GetText_EmptySlot() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("SLOT VAZIO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("空欄位"));
	default: return FText::FromString(TEXT("EMPTY SLOT"));
	}
}

FText UT66LocalizationSubsystem::GetText_Stage() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("FASE"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("關卡"));
	default: return FText::FromString(TEXT("STAGE"));
	}
}

// ========== Account Status ==========

FText UT66LocalizationSubsystem::GetText_AccountStatus() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("STATUS DA CONTA"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("帳號狀態"));
	default: return FText::FromString(TEXT("ACCOUNT STATUS"));
	}
}
