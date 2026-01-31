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

// ========== Hero Descriptions ==========

FText UT66LocalizationSubsystem::GetText_HeroDescription(FName HeroID) const
{
	auto Ret = [this](const TCHAR* EN, const TCHAR* PT, const TCHAR* ZH) -> FText {
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(PT);
		case ET66Language::ChineseTraditional: return FText::FromString(ZH);
		default: return FText::FromString(EN);
		}
	};
	if (HeroID == FName(TEXT("Hero_AliceInWonderlandRabbit"))) return Ret(TEXT("A swift trickster who bends reality. Favors evasion and misdirection."), TEXT("Um trapaceiro ágil que distorce a realidade. Prefere evasão e desorientação."), TEXT("扭曲現實的敏捷騙徒，擅長閃避與誤導。"));
	if (HeroID == FName(TEXT("Hero_LuBu"))) return Ret(TEXT("Unmatched warrior of the Three Kingdoms. Overwhelming strength and ferocity."), TEXT("Guerreiro incomparável dos Três Reinos. Força e ferocidade avassaladoras."), TEXT("三國無雙猛將，壓倒性的力量與兇猛。"));
	if (HeroID == FName(TEXT("Hero_LeonardoDaVinci"))) return Ret(TEXT("Renaissance genius who turns invention into combat. Ingenious and versatile."), TEXT("Gênio renascentista que transforma invenção em combate. Engenhoso e versátil."), TEXT("將發明化為戰鬥的文藝復興天才，機智多才。"));
	if (HeroID == FName(TEXT("Hero_Yakub"))) return Ret(TEXT("A figure of myth and controversy. Cunning and resilient in equal measure."), TEXT("Uma figura de mito e controvérsia. Astuto e resiliente na mesma medida."), TEXT("神話與爭議交織的人物，狡黠而堅韌。"));
	if (HeroID == FName(TEXT("Hero_KingArthur"))) return Ret(TEXT("The once and future king. Leads with honor and strikes with Excalibur."), TEXT("O rei do passado e do futuro. Lidera com honra e golpeia com Excalibur."), TEXT("昔在與將來的王者，以榮譽統御，以石中劍斬敵。"));
	if (HeroID == FName(TEXT("Hero_MiyamotoMusashi"))) return Ret(TEXT("Dual-blade master of the way of the sword. Discipline and precision."), TEXT("Mestre das duas lâminas do caminho da espada. Disciplina e precisão."), TEXT("二刀流劍道宗師，紀律與精準並重。"));
	if (HeroID == FName(TEXT("Hero_CaptainJackSparrow"))) return Ret(TEXT("Pirate legend who fights with flair and luck. Unpredictable and daring."), TEXT("Lenda pirata que luta com estilo e sorte. Imprevisível e ousado."), TEXT("以風采與運氣作戰的海盜傳說，難以捉摸且大膽。"));
	if (HeroID == FName(TEXT("Hero_SoloLeveler"))) return Ret(TEXT("Rises alone through trials. Growth through combat and sheer determination."), TEXT("Sobe sozinho pelas provações. Crescimento pelo combate e pura determinação."), TEXT("獨自穿越試煉而崛起，以戰鬥與意志成長。"));
	if (HeroID == FName(TEXT("Hero_Saitama"))) return Ret(TEXT("One punch is enough. Overwhelming power hidden behind a casual demeanor."), TEXT("Um soco basta. Poder avassalador escondido atrás de uma atitude casual."), TEXT("一拳足矣。輕鬆外表下藏著壓倒性的力量。"));
	if (HeroID == FName(TEXT("Hero_RoboGoon"))) return Ret(TEXT("Mechanical enforcer. Reliable, durable, and brutally efficient."), TEXT("Executor mecânico. Confiável, durável e brutalmente eficiente."), TEXT("機械執法者，可靠、耐打、冷酷高效。"));
	if (HeroID == FName(TEXT("Hero_GeorgeWashington"))) return Ret(TEXT("Founding commander. Strategic mind and unyielding resolve."), TEXT("Comandante fundador. Mente estratégica e resolução inquebrantável."), TEXT("開國統帥，戰略頭腦與堅定決心。"));
	if (HeroID == FName(TEXT("Hero_Cain"))) return Ret(TEXT("Marked by fate. Cursed to wander but gifted with survival."), TEXT("Marcado pelo destino. Amaldiçoado a vagar mas dotado de sobrevivência."), TEXT("被命運標記，詛咒漂泊卻賦予生存之能。"));
	if (HeroID == FName(TEXT("Hero_BillyTheKid"))) return Ret(TEXT("Young gunslinger of the frontier. Fast draw and quicker wit."), TEXT("Jovem pistoleiro da fronteira. Gato rápido e astúcia mais rápida."), TEXT("邊疆少年槍手，快槍與更快的機智。"));
	if (HeroID == FName(TEXT("Hero_RoachKing"))) return Ret(TEXT("Lord of the underbelly. Thrives where others dare not go."), TEXT("Senhor do submundo. Prospera onde outros não ousam ir."), TEXT("地下王者，在他人不敢涉足之處興盛。"));
	if (HeroID == FName(TEXT("Hero_Goblino"))) return Ret(TEXT("Cunning and scrappy. Uses wit and numbers to overcome."), TEXT("Astuto e brigão. Usa astúcia e números para superar."), TEXT("狡詐好鬥，以機智與數量取勝。"));
	if (HeroID == FName(TEXT("Hero_BulkBite"))) return Ret(TEXT("Raw power and appetite. Devastating when unleashed."), TEXT("Poder e apetite brutos. Devastador quando desencadeado."), TEXT("蠻力與貪婪，一旦釋放即毀滅一切。"));
	if (HeroID == FName(TEXT("Hero_HoboWanderer"))) return Ret(TEXT("Wanderer of the margins. Survives by wit and resilience."), TEXT("Andarilho das margens. Sobrevive por astúcia e resiliência."), TEXT("邊緣的流浪者，以機智與韌性求生。"));
	if (HeroID == FName(TEXT("Hero_DryHumor"))) return Ret(TEXT("Deadpan and deadly. Strikes when least expected."), TEXT("Cara de pau e mortal. Golpeia quando menos se espera."), TEXT("冷面而致命，在最不經意時出手。"));
	if (HeroID == FName(TEXT("Hero_LyricVoi"))) return Ret(TEXT("Voice and verse as weapons. Charms and cuts in equal measure."), TEXT("Voz e verso como armas. Encanta e corta na mesma medida."), TEXT("以聲與詩為刃，魅惑與斬擊並重。"));
	if (HeroID == FName(TEXT("Hero_Jesterma"))) return Ret(TEXT("Court jester with a lethal edge. Laughs last."), TEXT("Bobo da corte com um toque letal. Quem ri por último."), TEXT("帶殺機的宮廷小丑，笑到最後。"));
	if (HeroID == FName(TEXT("Hero_NorthKing"))) return Ret(TEXT("Sovereign of the frozen realm. Cold and commanding."), TEXT("Soberano do reino congelado. Frio e comandante."), TEXT("冰封國度的君主，冷酷而威嚴。"));
	if (HeroID == FName(TEXT("Hero_Peakwarden"))) return Ret(TEXT("Guardian of the summit. Unshakeable and vigilant."), TEXT("Guardião do cume. Inabalável e vigilante."), TEXT("巔峰守護者，堅定而警覺。"));
	if (HeroID == FName(TEXT("Hero_QuinnHex"))) return Ret(TEXT("Weaver of curses. Where she points, fate follows."), TEXT("Tecelã de maldições. Para onde aponta, o destino segue."), TEXT("詛咒編織者，所指之處命運相隨。"));
	if (HeroID == FName(TEXT("Hero_SandSultan"))) return Ret(TEXT("Ruler of the dunes. Shifting sands and shifting fortunes."), TEXT("Governante das dunas. Areias movediças e fortunas movediças."), TEXT("沙丘之主，流沙與命運皆可操弄。"));
	if (HeroID == FName(TEXT("Hero_CharNut"))) return Ret(TEXT("Burned but unbroken. Turns ruin into resolve."), TEXT("Queimado mas não quebrado. Transforma ruína em resolução."), TEXT("燒灼卻未碎，將毀滅化為決心。"));
	if (HeroID == FName(TEXT("Hero_Wraithveil"))) return Ret(TEXT("Between worlds. Strikes from the veil and fades back."), TEXT("Entre mundos. Golpeia do véu e desaparece."), TEXT("遊走兩界，自帷幕中出手再隱沒。"));
	return FText::FromString(TEXT("Select a hero to view their description."));
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

FText UT66LocalizationSubsystem::GetText_AchievementTierBlack() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("PRETO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("黑"));
	default: return FText::FromString(TEXT("BLACK"));
	}
}

FText UT66LocalizationSubsystem::GetText_AchievementTierRed() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("VERMELHO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("紅"));
	default: return FText::FromString(TEXT("RED"));
	}
}

FText UT66LocalizationSubsystem::GetText_AchievementTierYellow() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("AMARELO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("黃"));
	default: return FText::FromString(TEXT("YELLOW"));
	}
}

FText UT66LocalizationSubsystem::GetText_AchievementTierWhite() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("BRANCO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("白"));
	default: return FText::FromString(TEXT("WHITE"));
	}
}

FText UT66LocalizationSubsystem::GetText_AchievementCoinsFormat() const
{
	// Keep the AC abbreviation stable across languages for readability (Bible: "Achievement Coins (AC)").
	// Translators can adjust ordering/spacing per language.
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("{0} AC"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("{0} AC"));
	default: return FText::FromString(TEXT("{0} AC"));
	}
}

FText UT66LocalizationSubsystem::GetText_Claim() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("RESGATAR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("領取"));
	default: return FText::FromString(TEXT("CLAIM"));
	}
}

FText UT66LocalizationSubsystem::GetText_Claimed() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("RESGATADO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("已領取"));
	default: return FText::FromString(TEXT("CLAIMED"));
	}
}

FText UT66LocalizationSubsystem::GetText_AchievementName(FName AchievementID) const
{
	// v0: first wired achievement
	if (AchievementID == FName(TEXT("ACH_BLK_001")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("MATE 20 INIMIGOS"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("擊殺 20 名敵人"));
		default: return FText::FromString(TEXT("KILL 20 ENEMIES"));
		}
	}

	// Fallback: show the ID so missing strings are obvious (and never crash).
	return FText::FromName(AchievementID);
}

FText UT66LocalizationSubsystem::GetText_AchievementDescription(FName AchievementID) const
{
	if (AchievementID == FName(TEXT("ACH_BLK_001")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Mate 20 inimigos."));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("擊殺 20 名敵人。"));
		default: return FText::FromString(TEXT("Kill 20 enemies."));
		}
	}

	return FText::FromName(AchievementID);
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

// ========== Settings ==========

FText UT66LocalizationSubsystem::GetText_SettingsTabGameplay() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("JOGABILIDADE"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("遊戲"));
	default: return FText::FromString(TEXT("GAMEPLAY"));
	}
}

FText UT66LocalizationSubsystem::GetText_SettingsTabGraphics() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("GRÁFICOS"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("圖形"));
	default: return FText::FromString(TEXT("GRAPHICS"));
	}
}

FText UT66LocalizationSubsystem::GetText_SettingsTabControls() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("CONTROLES"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("控制"));
	default: return FText::FromString(TEXT("CONTROLS"));
	}
}

FText UT66LocalizationSubsystem::GetText_SettingsTabAudio() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("ÁUDIO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("音訊"));
	default: return FText::FromString(TEXT("AUDIO"));
	}
}

FText UT66LocalizationSubsystem::GetText_SettingsTabCrashing() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("TRAVANDO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("當機"));
	default: return FText::FromString(TEXT("CRASHING"));
	}
}

FText UT66LocalizationSubsystem::GetText_On() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("LIGADO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("開"));
	default: return FText::FromString(TEXT("ON"));
	}
}

FText UT66LocalizationSubsystem::GetText_Off() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("DESLIGADO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("關"));
	default: return FText::FromString(TEXT("OFF"));
	}
}

FText UT66LocalizationSubsystem::GetText_Apply() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("APLICAR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("套用"));
	default: return FText::FromString(TEXT("APPLY"));
	}
}

FText UT66LocalizationSubsystem::GetText_RestoreDefaults() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("RESTAURAR PADRÕES"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("還原預設"));
	default: return FText::FromString(TEXT("RESTORE DEFAULTS"));
	}
}

FText UT66LocalizationSubsystem::GetText_Rebind() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("REMAPEAR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("重新綁定"));
	default: return FText::FromString(TEXT("REBIND"));
	}
}

FText UT66LocalizationSubsystem::GetText_Clear() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("LIMPAR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("清除"));
	default: return FText::FromString(TEXT("CLEAR"));
	}
}

FText UT66LocalizationSubsystem::GetText_Primary() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("PRIMÁRIO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("主要"));
	default: return FText::FromString(TEXT("PRIMARY"));
	}
}

FText UT66LocalizationSubsystem::GetText_Secondary() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("SECUNDÁRIO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("次要"));
	default: return FText::FromString(TEXT("SECONDARY"));
	}
}

FText UT66LocalizationSubsystem::GetText_KeyboardAndMouse() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("TECLADO E MOUSE"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("鍵盤與滑鼠"));
	default: return FText::FromString(TEXT("KEYBOARD & MOUSE"));
	}
}

FText UT66LocalizationSubsystem::GetText_Controller() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("CONTROLE"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("控制器"));
	default: return FText::FromString(TEXT("CONTROLLER"));
	}
}

FText UT66LocalizationSubsystem::GetText_RebindInstructions() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Clique em REMAPEAR e pressione uma tecla/botão (Esc cancela)."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("點擊重新綁定，然後按下按鍵/按鈕（Esc 取消）。"));
	default: return FText::FromString(TEXT("Click REBIND, then press a key/button (Esc cancels)."));
	}
}

FText UT66LocalizationSubsystem::GetText_RebindPressKeyFormat() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Pressione uma tecla para {0} (Esc cancela)."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("為 {0} 按下按鍵（Esc 取消）。"));
	default: return FText::FromString(TEXT("Press a key for {0} (Esc cancels)."));
	}
}

FText UT66LocalizationSubsystem::GetText_RebindCancelled() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Remapeamento cancelado."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("已取消重新綁定。"));
	default: return FText::FromString(TEXT("Rebind cancelled."));
	}
}

FText UT66LocalizationSubsystem::GetText_RebindSaved() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Remapeamento salvo."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("已儲存綁定。"));
	default: return FText::FromString(TEXT("Rebind saved."));
	}
}

FText UT66LocalizationSubsystem::GetText_ControlMoveForward() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Mover para frente"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("前進"));
	default: return FText::FromString(TEXT("Move Forward"));
	}
}

FText UT66LocalizationSubsystem::GetText_ControlMoveBack() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Mover para trás"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("後退"));
	default: return FText::FromString(TEXT("Move Back"));
	}
}

FText UT66LocalizationSubsystem::GetText_ControlMoveLeft() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Mover para a esquerda"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("向左移動"));
	default: return FText::FromString(TEXT("Move Left"));
	}
}

FText UT66LocalizationSubsystem::GetText_ControlMoveRight() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Mover para a direita"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("向右移動"));
	default: return FText::FromString(TEXT("Move Right"));
	}
}

FText UT66LocalizationSubsystem::GetText_ControlJump() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Pular"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("跳躍"));
	default: return FText::FromString(TEXT("Jump"));
	}
}

FText UT66LocalizationSubsystem::GetText_ControlInteract() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Interagir"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("互動"));
	default: return FText::FromString(TEXT("Interact"));
	}
}

FText UT66LocalizationSubsystem::GetText_ControlPauseMenuPrimary() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Menu de pausa (primário)"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("暫停選單（主要）"));
	default: return FText::FromString(TEXT("Pause Menu (primary)"));
	}
}

FText UT66LocalizationSubsystem::GetText_ControlPauseMenuSecondary() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Menu de pausa (secundário)"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("暫停選單（次要）"));
	default: return FText::FromString(TEXT("Pause Menu (secondary)"));
	}
}

FText UT66LocalizationSubsystem::GetText_ControlToggleHUD() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Alternar HUD"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("切換 HUD"));
	default: return FText::FromString(TEXT("Toggle HUD"));
	}
}

FText UT66LocalizationSubsystem::GetText_ControlDash() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Dash"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("衝刺"));
	default: return FText::FromString(TEXT("Dash"));
	}
}

FText UT66LocalizationSubsystem::GetText_ControlUltimate() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Ultimate"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("終極技能"));
	default: return FText::FromString(TEXT("Ultimate"));
	}
}

FText UT66LocalizationSubsystem::GetText_ControlOpenFullMap() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Abrir mapa completo"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("開啟全地圖"));
	default: return FText::FromString(TEXT("Open Full Map"));
	}
}

FText UT66LocalizationSubsystem::GetText_ControlToggleMediaViewer() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Alternar visualizador de mídia"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("切換媒體檢視器"));
	default: return FText::FromString(TEXT("Toggle Media Viewer"));
	}
}

FText UT66LocalizationSubsystem::GetText_ControlToggleGamerMode() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Alternar modo gamer (hitboxes)"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("切換玩家模式（碰撞盒）"));
	default: return FText::FromString(TEXT("Toggle Gamer Mode (Hitboxes)"));
	}
}

FText UT66LocalizationSubsystem::GetText_ControlRestartRun() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Reiniciar corrida"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("重新開始"));
	default: return FText::FromString(TEXT("Restart Run"));
	}
}

FText UT66LocalizationSubsystem::GetText_ControlAttackLock() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Travar alvo"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("鎖定目標"));
	default: return FText::FromString(TEXT("Attack Lock"));
	}
}

FText UT66LocalizationSubsystem::GetText_ControlAttackUnlock() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Destravar alvo"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("解除鎖定"));
	default: return FText::FromString(TEXT("Attack Unlock"));
	}
}

FText UT66LocalizationSubsystem::GetText_PracticeMode() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Modo de Prática"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("練習模式"));
	default: return FText::FromString(TEXT("Practice Mode"));
	}
}

FText UT66LocalizationSubsystem::GetText_SubmitLeaderboardAnonymous() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Enviar ao Ranking como Anônimo"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("匿名提交排行榜"));
	default: return FText::FromString(TEXT("Submit Leaderboard as Anonymous"));
	}
}

FText UT66LocalizationSubsystem::GetText_SpeedRunMode() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Modo Speedrun"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("競速模式"));
	default: return FText::FromString(TEXT("Speed Run Mode"));
	}
}

FText UT66LocalizationSubsystem::GetText_GoonerMode() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Modo Gooner"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("Gooner 模式"));
	default: return FText::FromString(TEXT("Gooner Mode"));
	}
}

FText UT66LocalizationSubsystem::GetText_Monitor() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Monitor"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("螢幕"));
	default: return FText::FromString(TEXT("Monitor"));
	}
}

FText UT66LocalizationSubsystem::GetText_PrimaryMonitor() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Monitor principal"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("主要螢幕"));
	default: return FText::FromString(TEXT("Primary Monitor"));
	}
}

FText UT66LocalizationSubsystem::GetText_Resolution() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Resolução"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("解析度"));
	default: return FText::FromString(TEXT("Resolution"));
	}
}

FText UT66LocalizationSubsystem::GetText_WindowMode() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Modo de Janela"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("視窗模式"));
	default: return FText::FromString(TEXT("Window Mode"));
	}
}

FText UT66LocalizationSubsystem::GetText_Windowed() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Janela"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("視窗"));
	default: return FText::FromString(TEXT("Windowed"));
	}
}

FText UT66LocalizationSubsystem::GetText_Fullscreen() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Tela Cheia"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("全螢幕"));
	default: return FText::FromString(TEXT("Fullscreen"));
	}
}

FText UT66LocalizationSubsystem::GetText_BorderlessWindowed() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Janela sem Bordas"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("無邊框視窗"));
	default: return FText::FromString(TEXT("Borderless Windowed"));
	}
}

FText UT66LocalizationSubsystem::GetText_DisplayMode() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Modo de Exibição"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("顯示模式"));
	default: return FText::FromString(TEXT("Display Mode"));
	}
}

FText UT66LocalizationSubsystem::GetText_DisplayModeStandard() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Padrão"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("標準"));
	default: return FText::FromString(TEXT("Standard"));
	}
}

FText UT66LocalizationSubsystem::GetText_DisplayModeWidescreen() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Widescreen"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("寬螢幕"));
	default: return FText::FromString(TEXT("Widescreen"));
	}
}

FText UT66LocalizationSubsystem::GetText_Quality() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Qualidade"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("品質"));
	default: return FText::FromString(TEXT("Quality"));
	}
}

FText UT66LocalizationSubsystem::GetText_BestPerformance() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Melhor Desempenho"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("最佳效能"));
	default: return FText::FromString(TEXT("Best Performance"));
	}
}

FText UT66LocalizationSubsystem::GetText_BestQuality() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Melhor Qualidade"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("最佳品質"));
	default: return FText::FromString(TEXT("Best Quality"));
	}
}

FText UT66LocalizationSubsystem::GetText_FpsCap() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Limite de FPS"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("FPS 上限"));
	default: return FText::FromString(TEXT("FPS Cap"));
	}
}

FText UT66LocalizationSubsystem::GetText_Unlimited() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Ilimitado"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("無上限"));
	default: return FText::FromString(TEXT("Unlimited"));
	}
}

FText UT66LocalizationSubsystem::GetText_KeepTheseSettingsTitle() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Manter estas configurações?"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("保留這些設定？"));
	default: return FText::FromString(TEXT("Keep these settings?"));
	}
}

FText UT66LocalizationSubsystem::GetText_KeepTheseSettingsBodyFormat() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Revertendo em {0}s."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("{0} 秒後自動還原。"));
	default: return FText::FromString(TEXT("Reverting in {0}s."));
	}
}

FText UT66LocalizationSubsystem::GetText_Keep() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("MANTER"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("保留"));
	default: return FText::FromString(TEXT("KEEP"));
	}
}

FText UT66LocalizationSubsystem::GetText_Revert() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("REVERTER"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("還原"));
	default: return FText::FromString(TEXT("REVERT"));
	}
}

FText UT66LocalizationSubsystem::GetText_MasterVolume() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Volume Mestre"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("主音量"));
	default: return FText::FromString(TEXT("Master Volume"));
	}
}

FText UT66LocalizationSubsystem::GetText_MusicVolume() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Volume da Música"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("音樂音量"));
	default: return FText::FromString(TEXT("Music Volume"));
	}
}

FText UT66LocalizationSubsystem::GetText_SfxVolume() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Volume de Efeitos"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("音效音量"));
	default: return FText::FromString(TEXT("SFX Volume"));
	}
}

FText UT66LocalizationSubsystem::GetText_MuteWhenUnfocused() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Silenciar ao Desfocar"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("失去焦點時靜音"));
	default: return FText::FromString(TEXT("Mute when unfocused"));
	}
}

FText UT66LocalizationSubsystem::GetText_OutputDevice() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Dispositivo de Saída"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("輸出裝置"));
	default: return FText::FromString(TEXT("Output Device"));
	}
}

FText UT66LocalizationSubsystem::GetText_SubtitlesAlwaysOn() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Legendas: sempre ativadas"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("字幕：永遠開啟"));
	default: return FText::FromString(TEXT("Subtitles: always on"));
	}
}

FText UT66LocalizationSubsystem::GetText_CrashingChecklistHeader() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Se o jogo estiver travando, tente:"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("如果遊戲當機，請嘗試："));
	default: return FText::FromString(TEXT("If your game is crashing, try these steps:"));
	}
}

FText UT66LocalizationSubsystem::GetText_CrashingChecklistBody() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese:
		return FText::FromString(TEXT("1. Abaixe a Qualidade para \"Melhor Desempenho\" na aba Gráficos.\n2. Desative \"Visuais Intensos\" na aba Jogabilidade.\n3. Reduza o Limite de FPS para 60 na aba Gráficos.\n4. Experimente o modo Janela em vez de Tela Cheia."));
	case ET66Language::ChineseTraditional:
		return FText::FromString(TEXT("1. 在「圖形」把品質調到「最佳效能」。\n2. 在「遊戲」關閉「強烈特效」。\n3. 在「圖形」把 FPS 上限調到 60。\n4. 嘗試使用視窗模式而不是全螢幕。"));
	default:
		return FText::FromString(TEXT("1. Lower Quality to \"Best Performance\" in the Graphics tab.\n2. Disable \"Intense Visuals\" in the Gameplay tab.\n3. Reduce FPS Cap to 60 in the Graphics tab.\n4. Try Windowed mode instead of Fullscreen."));
	}
}

FText UT66LocalizationSubsystem::GetText_ApplySafeModeSettings() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Aplicar Modo Seguro"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("套用安全模式"));
	default: return FText::FromString(TEXT("Apply Safe Mode Settings"));
	}
}

FText UT66LocalizationSubsystem::GetText_StillHavingIssues() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Ainda com problemas?"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("還有問題？"));
	default: return FText::FromString(TEXT("Still having issues?"));
	}
}

FText UT66LocalizationSubsystem::GetText_ReportBugDescription() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Reporte um bug para nos ajudar. Seu relatório incluirá informações básicas do sistema."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("回報問題以協助我們修復。你的回報會包含基本系統資訊。"));
	default: return FText::FromString(TEXT("Report a bug to help us fix it. Your report will include basic system info."));
	}
}

FText UT66LocalizationSubsystem::GetText_DescribeTheBugHint() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Descreva o bug..."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("描述問題..."));
	default: return FText::FromString(TEXT("Describe the bug..."));
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
	if (SkinID == FName(TEXT("Frost")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Gelo Eterno"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("永霜"));
		default: return FText::FromString(TEXT("Eternal Frost"));
		}
	}
	if (SkinID == FName(TEXT("Void")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Abismo"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("虛空"));
		default: return FText::FromString(TEXT("Void Walker"));
		}
	}
	if (SkinID == FName(TEXT("Phoenix")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Fênix"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("鳳凰"));
		default: return FText::FromString(TEXT("Phoenix Rising"));
		}
	}
	if (SkinID == FName(TEXT("Cosmic")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Cósmico"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("宇宙"));
		default: return FText::FromString(TEXT("Cosmic"));
		}
	}
	if (SkinID == FName(TEXT("Neon")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Neon"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("霓虹"));
		default: return FText::FromString(TEXT("Neon"));
		}
	}
	if (SkinID == FName(TEXT("Primal")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Primal"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("原始"));
		default: return FText::FromString(TEXT("Primal"));
		}
	}
	if (SkinID == FName(TEXT("Celestial")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Celestial"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("天界"));
		default: return FText::FromString(TEXT("Celestial"));
		}
	}
	if (SkinID == FName(TEXT("Obsidian")))
	{
		switch (CurrentLanguage)
		{
		case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Obsidiana"));
		case ET66Language::ChineseTraditional: return FText::FromString(TEXT("黑曜石"));
		default: return FText::FromString(TEXT("Obsidian"));
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

// ========== Gameplay Overlays ==========

FText UT66LocalizationSubsystem::GetText_WheelSpinTitle() const
{
	static const FText En = FText::FromString(TEXT("WHEEL SPIN"));
	static const FText Pt = FText::FromString(TEXT("RODA DA SORTE"));
	static const FText Zh = FText::FromString(TEXT("轉盤"));
	switch (CurrentLanguage)
	{
		case ET66Language::BrazilianPortuguese: return Pt;
		case ET66Language::ChineseTraditional: return Zh;
		default: return En;
	}
}

FText UT66LocalizationSubsystem::GetText_Spin() const
{
	static const FText En = FText::FromString(TEXT("SPIN"));
	static const FText Pt = FText::FromString(TEXT("GIRAR"));
	static const FText Zh = FText::FromString(TEXT("旋轉"));
	switch (CurrentLanguage)
	{
		case ET66Language::BrazilianPortuguese: return Pt;
		case ET66Language::ChineseTraditional: return Zh;
		default: return En;
	}
}

FText UT66LocalizationSubsystem::GetText_PressSpinToRollGold() const
{
	static const FText En = FText::FromString(TEXT("Press SPIN to roll gold."));
	static const FText Pt = FText::FromString(TEXT("Pressione GIRAR para ganhar ouro."));
	static const FText Zh = FText::FromString(TEXT("按下旋轉來獲得金幣。"));
	switch (CurrentLanguage)
	{
		case ET66Language::BrazilianPortuguese: return Pt;
		case ET66Language::ChineseTraditional: return Zh;
		default: return En;
	}
}

FText UT66LocalizationSubsystem::GetText_Spinning() const
{
	static const FText En = FText::FromString(TEXT("Spinning..."));
	static const FText Pt = FText::FromString(TEXT("Girando..."));
	static const FText Zh = FText::FromString(TEXT("旋轉中..."));
	switch (CurrentLanguage)
	{
		case ET66Language::BrazilianPortuguese: return Pt;
		case ET66Language::ChineseTraditional: return Zh;
		default: return En;
	}
}

FText UT66LocalizationSubsystem::GetText_YouWonGoldFormat() const
{
	static const FText En = FText::FromString(TEXT("You won {0} gold."));
	static const FText Pt = FText::FromString(TEXT("Você ganhou {0} ouro."));
	static const FText Zh = FText::FromString(TEXT("你獲得了 {0} 金幣。"));
	switch (CurrentLanguage)
	{
		case ET66Language::BrazilianPortuguese: return Pt;
		case ET66Language::ChineseTraditional: return Zh;
		default: return En;
	}
}

FText UT66LocalizationSubsystem::GetText_IdolAltarTitle() const
{
	static const FText En = FText::FromString(TEXT("IDOL ALTAR"));
	static const FText Pt = FText::FromString(TEXT("ALTAR DE ÍDOLOS"));
	static const FText Zh = FText::FromString(TEXT("偶像祭壇"));
	switch (CurrentLanguage)
	{
		case ET66Language::BrazilianPortuguese: return Pt;
		case ET66Language::ChineseTraditional: return Zh;
		default: return En;
	}
}

FText UT66LocalizationSubsystem::GetText_IdolAltarHoverHint() const
{
	static const FText En = FText::FromString(TEXT("Hover an idol to see its effect."));
	static const FText Pt = FText::FromString(TEXT("Passe o mouse sobre um ídolo para ver o efeito."));
	static const FText Zh = FText::FromString(TEXT("將滑鼠移到偶像上查看效果。"));
	switch (CurrentLanguage)
	{
		case ET66Language::BrazilianPortuguese: return Pt;
		case ET66Language::ChineseTraditional: return Zh;
		default: return En;
	}
}

FText UT66LocalizationSubsystem::GetText_IdolAltarDropHere() const
{
	static const FText En = FText::FromString(TEXT("DROP\nHERE"));
	static const FText Pt = FText::FromString(TEXT("SOLTE\nAQUI"));
	static const FText Zh = FText::FromString(TEXT("放到\n這裡"));
	switch (CurrentLanguage)
	{
		case ET66Language::BrazilianPortuguese: return Pt;
		case ET66Language::ChineseTraditional: return Zh;
		default: return En;
	}
}

FText UT66LocalizationSubsystem::GetText_IdolAltarDropAnIdolHere() const
{
	static const FText En = FText::FromString(TEXT("Drop an Idol here."));
	static const FText Pt = FText::FromString(TEXT("Solte um Ídolo aqui."));
	static const FText Zh = FText::FromString(TEXT("將偶像放到這裡。"));
	switch (CurrentLanguage)
	{
		case ET66Language::BrazilianPortuguese: return Pt;
		case ET66Language::ChineseTraditional: return Zh;
		default: return En;
	}
}

FText UT66LocalizationSubsystem::GetText_IdolAltarAlreadySelectedStage1() const
{
	// Repurposed: v0 now allows selecting idols multiple times; lock only when all slots are full.
	static const FText En = FText::FromString(TEXT("All idol slots are full."));
	static const FText Pt = FText::FromString(TEXT("Todos os slots de ídolo estão cheios."));
	static const FText Zh = FText::FromString(TEXT("所有偶像插槽已滿。"));
	switch (CurrentLanguage)
	{
		case ET66Language::BrazilianPortuguese: return Pt;
		case ET66Language::ChineseTraditional: return Zh;
		default: return En;
	}
}

FText UT66LocalizationSubsystem::GetText_IdolAltarAlreadyEquipped() const
{
	static const FText En = FText::FromString(TEXT("Already equipped."));
	static const FText Pt = FText::FromString(TEXT("Já equipado."));
	static const FText Zh = FText::FromString(TEXT("已裝備。"));
	switch (CurrentLanguage)
	{
		case ET66Language::BrazilianPortuguese: return Pt;
		case ET66Language::ChineseTraditional: return Zh;
		default: return En;
	}
}

FText UT66LocalizationSubsystem::GetText_IdolAltarDragFirst() const
{
	static const FText En = FText::FromString(TEXT("Drag an idol into the center slot first."));
	static const FText Pt = FText::FromString(TEXT("Arraste um ídolo para o slot central primeiro."));
	static const FText Zh = FText::FromString(TEXT("先把偶像拖到中央插槽。"));
	switch (CurrentLanguage)
	{
		case ET66Language::BrazilianPortuguese: return Pt;
		case ET66Language::ChineseTraditional: return Zh;
		default: return En;
	}
}

FText UT66LocalizationSubsystem::GetText_IdolAltarEquipped() const
{
	static const FText En = FText::FromString(TEXT("Equipped."));
	static const FText Pt = FText::FromString(TEXT("Equipado."));
	static const FText Zh = FText::FromString(TEXT("已裝備。"));
	switch (CurrentLanguage)
	{
		case ET66Language::BrazilianPortuguese: return Pt;
		case ET66Language::ChineseTraditional: return Zh;
		default: return En;
	}
}

FText UT66LocalizationSubsystem::GetText_IdolTooltip(FName IdolID) const
{
	// v0: use identical text across languages (can be translated later).
	static const FText Unknown = FText::FromString(TEXT("IDOL\nUnknown."));
	static const FText Frost = FText::FromString(TEXT("FROST\nAuto attacks freeze enemies for 0.5s."));
	static const FText Shock = FText::FromString(TEXT("SHOCK\nAuto attacks chain to 1 nearby enemy."));
	static const FText Glue = FText::FromString(TEXT("GLUE\nAuto attacks slow enemies briefly."));
	static const FText Silence = FText::FromString(TEXT("SILENCE\nAuto attacks prevent enemy attacks briefly."));
	static const FText Mark = FText::FromString(TEXT("MARK\nAuto attacks mark enemies (take extra auto-attack damage)."));
	static const FText Pierce = FText::FromString(TEXT("PIERCE\nAuto attacks pierce +1 enemy."));
	static const FText Split = FText::FromString(TEXT("SPLIT\nAuto attacks split into 2 weaker projectiles on hit."));
	static const FText Knock = FText::FromString(TEXT("KNOCKBACK\nAuto attacks knock enemies back."));
	static const FText Rico = FText::FromString(TEXT("RICOCHET\nAuto attacks bounce once to another enemy."));
	static const FText Hex = FText::FromString(TEXT("HEX\nAuto attacks briefly stun enemies."));
	static const FText Fire = FText::FromString(TEXT("FIRE\nAuto attacks apply a burn: 10 damage per second."));
	static const FText Life = FText::FromString(TEXT("LIFESTEAL\nAuto attacks heal you by 1 heart per hit."));

	if (IdolID == FName(TEXT("Idol_Frost"))) return Frost;
	if (IdolID == FName(TEXT("Idol_Shock"))) return Shock;
	if (IdolID == FName(TEXT("Idol_Glue"))) return Glue;
	if (IdolID == FName(TEXT("Idol_Silence"))) return Silence;
	if (IdolID == FName(TEXT("Idol_Mark"))) return Mark;
	if (IdolID == FName(TEXT("Idol_Pierce"))) return Pierce;
	if (IdolID == FName(TEXT("Idol_Split"))) return Split;
	if (IdolID == FName(TEXT("Idol_Knockback"))) return Knock;
	if (IdolID == FName(TEXT("Idol_Ricochet"))) return Rico;
	if (IdolID == FName(TEXT("Idol_Hex"))) return Hex;
	if (IdolID == FName(TEXT("Idol_Fire"))) return Fire;
	if (IdolID == FName(TEXT("Idol_Lifesteal"))) return Life;
	return Unknown;
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

// ========== Pause Menu ==========

FText UT66LocalizationSubsystem::GetText_Resume() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("CONTINUAR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("繼續遊戲"));
	default: return FText::FromString(TEXT("RESUME GAME"));
	}
}

FText UT66LocalizationSubsystem::GetText_SaveAndQuit() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("SALVAR E SAIR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("儲存並離開"));
	default: return FText::FromString(TEXT("SAVE AND QUIT GAME"));
	}
}

FText UT66LocalizationSubsystem::GetText_Restart() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("REINICIAR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("重新開始"));
	default: return FText::FromString(TEXT("RESTART"));
	}
}

FText UT66LocalizationSubsystem::GetText_ReportBug() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("REPORTAR BUG"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("回報問題"));
	default: return FText::FromString(TEXT("REPORT BUG"));
	}
}

// ========== Report Bug ==========

FText UT66LocalizationSubsystem::GetText_ReportBugTitle() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("REPORTAR BUG"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("回報問題"));
	default: return FText::FromString(TEXT("REPORT BUG"));
	}
}

FText UT66LocalizationSubsystem::GetText_ReportBugSubmit() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("ENVIAR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("送出"));
	default: return FText::FromString(TEXT("SUBMIT"));
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

// ========== Gameplay HUD ==========

FText UT66LocalizationSubsystem::GetText_GoldFormat() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Ouro: {0}"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("金幣：{0}"));
	default: return FText::FromString(TEXT("Gold: {0}"));
	}
}

FText UT66LocalizationSubsystem::GetText_OweFormat() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Deve: {0}"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("欠款：{0}"));
	default: return FText::FromString(TEXT("Owe: {0}"));
	}
}

FText UT66LocalizationSubsystem::GetText_StageNumberFormat() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Fase número: {0}"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("關卡：{0}"));
	default: return FText::FromString(TEXT("Stage number: {0}"));
	}
}

FText UT66LocalizationSubsystem::GetText_BountyLabel() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Recompensa:"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("懸賞："));
	default: return FText::FromString(TEXT("Bounty:"));
	}
}

FText UT66LocalizationSubsystem::GetText_PortraitPlaceholder() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("RETRATO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("肖像"));
	default: return FText::FromString(TEXT("PORTRAIT"));
	}
}

FText UT66LocalizationSubsystem::GetText_MinimapPlaceholder() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("MINIMAPA"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("小地圖"));
	default: return FText::FromString(TEXT("MINIMAP"));
	}
}

// ========== Run Summary ==========

FText UT66LocalizationSubsystem::GetText_RunSummaryTitle() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("RESUMO DA RUN"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("回合總結"));
	default: return FText::FromString(TEXT("RUN SUMMARY"));
	}
}

FText UT66LocalizationSubsystem::GetText_RunSummaryStageReachedBountyFormat() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Fase alcançada: {0}  |  Recompensa: {1}"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("到達關卡：{0}  |  懸賞：{1}"));
	default: return FText::FromString(TEXT("Stage Reached: {0}  |  Bounty: {1}"));
	}
}

FText UT66LocalizationSubsystem::GetText_RunSummaryPreviewPlaceholder() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Pré-visualização 3D"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("3D 預覽"));
	default: return FText::FromString(TEXT("3D Preview"));
	}
}

FText UT66LocalizationSubsystem::GetText_MainMenu() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("MENU PRINCIPAL"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("主選單"));
	default: return FText::FromString(TEXT("MAIN MENU"));
	}
}

// ========== Run Log ==========

FText UT66LocalizationSubsystem::GetText_RunLog_StageFormat() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Fase {0}"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("關卡 {0}"));
	default: return FText::FromString(TEXT("Stage {0}"));
	}
}

FText UT66LocalizationSubsystem::GetText_RunLog_PickedUpFormat() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Pegou {0}"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("拾取 {0}"));
	default: return FText::FromString(TEXT("Picked up {0}"));
	}
}

FText UT66LocalizationSubsystem::GetText_RunLog_GoldFormat() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Ouro: {0}"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("金幣：{0}"));
	default: return FText::FromString(TEXT("Gold: {0}"));
	}
}

FText UT66LocalizationSubsystem::GetText_RunLog_KillFormat() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Abate +{0}"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("擊殺 +{0}"));
	default: return FText::FromString(TEXT("Kill +{0}"));
	}
}

// ========== Gambler ==========

FText UT66LocalizationSubsystem::GetText_Gambler() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("APOSTADOR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("賭徒"));
	default: return FText::FromString(TEXT("GAMBLER"));
	}
}

FText UT66LocalizationSubsystem::GetText_Casino() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("CASSINO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("賭場"));
	default: return FText::FromString(TEXT("CASINO"));
	}
}

FText UT66LocalizationSubsystem::GetText_Bank() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("BANCO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("銀行"));
	default: return FText::FromString(TEXT("BANK"));
	}
}

FText UT66LocalizationSubsystem::GetText_Games() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("JOGOS"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("遊戲"));
	default: return FText::FromString(TEXT("GAMES"));
	}
}

FText UT66LocalizationSubsystem::GetText_Anger() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("RAIVA"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("怒氣"));
	default: return FText::FromString(TEXT("ANGER"));
	}
}

FText UT66LocalizationSubsystem::GetText_GamblerDialoguePrompt() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("O que você quer?"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("你想要什麼？"));
	default: return FText::FromString(TEXT("What do you want?"));
	}
}

FText UT66LocalizationSubsystem::GetText_LetMeGamble() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("DEIXA EU APOSTAR"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("讓我賭一把"));
	default: return FText::FromString(TEXT("LET ME GAMBLE"));
	}
}

FText UT66LocalizationSubsystem::GetText_TeleportMeToYourBrother() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("TELEPORTE-ME ATÉ SEU IRMÃO"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("傳送我去你兄弟那裡"));
	default: return FText::FromString(TEXT("TELEPORT ME TO YOUR BROTHER"));
	}
}

FText UT66LocalizationSubsystem::GetText_TeleportDisabledBossActive() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Teleporte desativado (encontro com chefe iniciado)."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("無法傳送（首領戰已開始）。"));
	default: return FText::FromString(TEXT("Teleport disabled (boss encounter started)."));
	}
}

FText UT66LocalizationSubsystem::GetText_Bet() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Aposta"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("下注"));
	default: return FText::FromString(TEXT("Bet"));
	}
}

FText UT66LocalizationSubsystem::GetText_Borrow() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Emprestar"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("借款"));
	default: return FText::FromString(TEXT("Borrow"));
	}
}

FText UT66LocalizationSubsystem::GetText_Payback() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Pagar"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("還款"));
	default: return FText::FromString(TEXT("Payback"));
	}
}

FText UT66LocalizationSubsystem::GetText_Max() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("MÁX"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("最大"));
	default: return FText::FromString(TEXT("MAX"));
	}
}

FText UT66LocalizationSubsystem::GetText_CoinFlip() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("CARA OU COROA"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("掷硬幣"));
	default: return FText::FromString(TEXT("COIN FLIP"));
	}
}

FText UT66LocalizationSubsystem::GetText_RockPaperScissors() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("PEDRA PAPEL TESOURA"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("剪刀石頭布"));
	default: return FText::FromString(TEXT("ROCK PAPER SCISSORS"));
	}
}

FText UT66LocalizationSubsystem::GetText_FindTheBall() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("ACHE A BOLA"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("找球"));
	default: return FText::FromString(TEXT("FIND THE BALL"));
	}
}

FText UT66LocalizationSubsystem::GetText_ChooseHeadsOrTails() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Escolha Cara ou Coroa."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("選擇正面或反面。"));
	default: return FText::FromString(TEXT("Choose Heads or Tails."));
	}
}

FText UT66LocalizationSubsystem::GetText_Heads() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("CARA"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("正面"));
	default: return FText::FromString(TEXT("HEADS"));
	}
}

FText UT66LocalizationSubsystem::GetText_Tails() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("COROA"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("反面"));
	default: return FText::FromString(TEXT("TAILS"));
	}
}

FText UT66LocalizationSubsystem::GetText_PickOne() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Escolha um."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("選一個。"));
	default: return FText::FromString(TEXT("Pick one."));
	}
}

FText UT66LocalizationSubsystem::GetText_Rock() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Pedra"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("石頭"));
	default: return FText::FromString(TEXT("Rock"));
	}
}

FText UT66LocalizationSubsystem::GetText_Paper() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Papel"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("布"));
	default: return FText::FromString(TEXT("Paper"));
	}
}

FText UT66LocalizationSubsystem::GetText_Scissors() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Tesoura"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("剪刀"));
	default: return FText::FromString(TEXT("Scissors"));
	}
}

FText UT66LocalizationSubsystem::GetText_PickACup() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Escolha um copo."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("選一個杯子。"));
	default: return FText::FromString(TEXT("Pick a cup."));
	}
}

FText UT66LocalizationSubsystem::GetText_Cup(int32 CupIndex1Based) const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(FString::Printf(TEXT("COPO %d"), CupIndex1Based));
	case ET66Language::ChineseTraditional: return FText::FromString(FString::Printf(TEXT("杯 %d"), CupIndex1Based));
	default: return FText::FromString(FString::Printf(TEXT("CUP %d"), CupIndex1Based));
	}
}

FText UT66LocalizationSubsystem::GetText_ResultDash() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Resultado: -"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("結果：-"));
	default: return FText::FromString(TEXT("Result: -"));
	}
}

FText UT66LocalizationSubsystem::GetText_Rolling() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Girando..."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("進行中…"));
	default: return FText::FromString(TEXT("Rolling..."));
	}
}

FText UT66LocalizationSubsystem::GetText_CheatPromptTitle() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Trapacear?"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("作弊？"));
	default: return FText::FromString(TEXT("Cheat?"));
	}
}

FText UT66LocalizationSubsystem::GetText_CheatPromptBody() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Trapacear aumenta a Raiva."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("作弊會增加怒氣。"));
	default: return FText::FromString(TEXT("Cheating increases Anger."));
	}
}

FText UT66LocalizationSubsystem::GetText_GambleAmountMustBePositive() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("A aposta deve ser > 0."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("下注金額必須大於 0。"));
	default: return FText::FromString(TEXT("Gamble amount must be > 0."));
	}
}

FText UT66LocalizationSubsystem::GetText_BorrowAmountMustBePositive() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("O empréstimo deve ser > 0."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("借款金額必須大於 0。"));
	default: return FText::FromString(TEXT("Borrow amount must be > 0."));
	}
}

FText UT66LocalizationSubsystem::GetText_PaybackAmountMustBePositive() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("O pagamento deve ser > 0."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("還款金額必須大於 0。"));
	default: return FText::FromString(TEXT("Payback amount must be > 0."));
	}
}

FText UT66LocalizationSubsystem::GetText_NotEnoughGold() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Ouro insuficiente."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("金幣不足。"));
	default: return FText::FromString(TEXT("Not enough gold."));
	}
}

FText UT66LocalizationSubsystem::GetText_NotEnoughGoldOrNoDebt() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Ouro insuficiente (ou sem dívida)."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("金幣不足（或沒有欠款）。"));
	default: return FText::FromString(TEXT("Not enough gold (or no debt)."));
	}
}

FText UT66LocalizationSubsystem::GetText_BorrowedAmountFormat() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Emprestou {0}."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("已借款 {0}。"));
	default: return FText::FromString(TEXT("Borrowed {0}."));
	}
}

FText UT66LocalizationSubsystem::GetText_PaidBackAmountFormat() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Pagou {0}."));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("已還款 {0}。"));
	default: return FText::FromString(TEXT("Paid back {0}."));
	}
}

FText UT66LocalizationSubsystem::GetText_WinPlusAmountFormat() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("VITÓRIA (+{0})"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("勝利（+{0}）"));
	default: return FText::FromString(TEXT("WIN (+{0})"));
	}
}

FText UT66LocalizationSubsystem::GetText_Win() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("VITÓRIA"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("勝利"));
	default: return FText::FromString(TEXT("WIN"));
	}
}

FText UT66LocalizationSubsystem::GetText_Lose() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("DERROTA"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("失敗"));
	default: return FText::FromString(TEXT("LOSE"));
	}
}

FText UT66LocalizationSubsystem::GetText_CoinFlipResultFormat() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Resultado: {0} — {1}"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("結果：{0} — {1}"));
	default: return FText::FromString(TEXT("Result: {0} — {1}"));
	}
}

FText UT66LocalizationSubsystem::GetText_RpsResultFormat() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("Você: {0}  |  Apostador: {1}  —  {2}"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("你：{0}  |  賭徒：{1}  —  {2}"));
	default: return FText::FromString(TEXT("You: {0}  |  Gambler: {1}  —  {2}"));
	}
}

FText UT66LocalizationSubsystem::GetText_FindBallResultFormat() const
{
	switch (CurrentLanguage)
	{
	case ET66Language::BrazilianPortuguese: return FText::FromString(TEXT("A bola estava em {0} — {1}"));
	case ET66Language::ChineseTraditional: return FText::FromString(TEXT("球在 {0} — {1}"));
	default: return FText::FromString(TEXT("Ball was under {0} — {1}"));
	}
}
