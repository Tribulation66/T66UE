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
