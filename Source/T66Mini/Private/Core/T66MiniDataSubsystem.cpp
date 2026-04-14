// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MiniDataSubsystem.h"

#include "Engine/Texture2D.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
	FString T66MiniTrimCell(const FString& InValue)
	{
		FString Result = InValue;
		Result.TrimStartAndEndInline();
		return Result;
	}

	TArray<TArray<FString>> T66MiniParseCsv(const FString& RawText)
	{
		FString Text = RawText;
		Text.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
		Text.ReplaceInline(TEXT("\r"), TEXT("\n"));

		TArray<TArray<FString>> Rows;
		TArray<FString> CurrentRow;
		FString CurrentCell;
		bool bInQuotes = false;

		for (int32 Index = 0; Index < Text.Len(); ++Index)
		{
			const TCHAR Character = Text[Index];
			if (Character == TEXT('"'))
			{
				const bool bEscapedQuote = bInQuotes && Index + 1 < Text.Len() && Text[Index + 1] == TEXT('"');
				if (bEscapedQuote)
				{
					CurrentCell.AppendChar(TEXT('"'));
					++Index;
				}
				else
				{
					bInQuotes = !bInQuotes;
				}
				continue;
			}

			if (!bInQuotes && Character == TEXT(','))
			{
				CurrentRow.Add(T66MiniTrimCell(CurrentCell));
				CurrentCell.Reset();
				continue;
			}

			if (!bInQuotes && Character == TEXT('\n'))
			{
				CurrentRow.Add(T66MiniTrimCell(CurrentCell));
				CurrentCell.Reset();

				bool bHasValue = false;
				for (const FString& Cell : CurrentRow)
				{
					if (!Cell.IsEmpty())
					{
						bHasValue = true;
						break;
					}
				}

				if (bHasValue)
				{
					Rows.Add(CurrentRow);
				}

				CurrentRow.Reset();
				continue;
			}

			CurrentCell.AppendChar(Character);
		}

		if (!CurrentCell.IsEmpty() || CurrentRow.Num() > 0)
		{
			CurrentRow.Add(T66MiniTrimCell(CurrentCell));
			Rows.Add(CurrentRow);
		}

		return Rows;
	}

	TArray<TMap<FString, FString>> T66MiniLoadCsvRows(const FString& AbsolutePath)
	{
		FString RawText;
		if (!FFileHelper::LoadFileToString(RawText, *AbsolutePath))
		{
			return {};
		}

		const TArray<TArray<FString>> ParsedRows = T66MiniParseCsv(RawText);
		if (ParsedRows.Num() < 2)
		{
			return {};
		}

		const TArray<FString>& Headers = ParsedRows[0];
		TArray<TMap<FString, FString>> OutputRows;
		for (int32 RowIndex = 1; RowIndex < ParsedRows.Num(); ++RowIndex)
		{
			const TArray<FString>& Row = ParsedRows[RowIndex];
			TMap<FString, FString> RowMap;
			for (int32 ColumnIndex = 0; ColumnIndex < Headers.Num(); ++ColumnIndex)
			{
				RowMap.Add(Headers[ColumnIndex], Row.IsValidIndex(ColumnIndex) ? Row[ColumnIndex] : FString());
			}
			OutputRows.Add(MoveTemp(RowMap));
		}

		return OutputRows;
	}

	FString T66MiniGetValue(const TMap<FString, FString>& Row, const TCHAR* Key)
	{
		if (const FString* Found = Row.Find(Key))
		{
			return *Found;
		}
		return FString();
	}

	float T66MiniToFloat(const FString& Value, const float DefaultValue = 0.f)
	{
		float Parsed = DefaultValue;
		return LexTryParseString(Parsed, *Value) ? Parsed : DefaultValue;
	}

	int32 T66MiniToInt(const FString& Value, const int32 DefaultValue = 0)
	{
		int32 Parsed = DefaultValue;
		return LexTryParseString(Parsed, *Value) ? Parsed : DefaultValue;
	}

	bool T66MiniToBool(const FString& Value, const bool bDefaultValue = false)
	{
		if (Value.IsEmpty())
		{
			return bDefaultValue;
		}

		return Value.Equals(TEXT("true"), ESearchCase::IgnoreCase)
			|| Value.Equals(TEXT("1"), ESearchCase::IgnoreCase)
			|| Value.Equals(TEXT("yes"), ESearchCase::IgnoreCase);
	}

	FLinearColor T66MiniToColor(const FString& Value, const FLinearColor& DefaultColor)
	{
		FString Sanitized = Value;
		Sanitized.ReplaceInline(TEXT("("), TEXT(""));
		Sanitized.ReplaceInline(TEXT(")"), TEXT(""));
		FLinearColor ParsedColor;
		return ParsedColor.InitFromString(Sanitized) ? ParsedColor : DefaultColor;
	}

	FName T66MiniToName(const TMap<FString, FString>& Row, const TCHAR* Key)
	{
		return FName(*T66MiniGetValue(Row, Key));
	}

	TArray<FName> T66MiniSplitNameList(const FString& Value)
	{
		TArray<FString> Parts;
		Value.ParseIntoArray(Parts, TEXT("|"), true);

		TArray<FName> Names;
		Names.Reserve(Parts.Num());
		for (const FString& Part : Parts)
		{
			const FString Trimmed = T66MiniTrimCell(Part);
			if (!Trimmed.IsEmpty())
			{
				Names.Add(FName(*Trimmed));
			}
		}

		return Names;
	}

	ET66MiniEnemyBehaviorProfile T66MiniToBehaviorProfile(const FString& Value)
	{
		if (Value.Equals(TEXT("RoostPress"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyBehaviorProfile::RoostPress;
		}
		if (Value.Equals(TEXT("CowBruiser"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyBehaviorProfile::CowBruiser;
		}
		if (Value.Equals(TEXT("GoatCharger"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyBehaviorProfile::GoatCharger;
		}
		if (Value.Equals(TEXT("PigEnrage"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyBehaviorProfile::PigEnrage;
		}
		if (Value.Equals(TEXT("Sharpshooter"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyBehaviorProfile::Sharpshooter;
		}
		if (Value.Equals(TEXT("Juggernaut"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyBehaviorProfile::Juggernaut;
		}
		if (Value.Equals(TEXT("Duelist"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyBehaviorProfile::Duelist;
		}

		return ET66MiniEnemyBehaviorProfile::Balanced;
	}

	ET66MiniInteractableType T66MiniToInteractableType(const FString& Value)
	{
		if (Value.Equals(TEXT("Fountain"), ESearchCase::IgnoreCase))
		{
			return ET66MiniInteractableType::Fountain;
		}
		if (Value.Equals(TEXT("LootCrate"), ESearchCase::IgnoreCase))
		{
			return ET66MiniInteractableType::LootCrate;
		}
		if (Value.Equals(TEXT("QuickReviveMachine"), ESearchCase::IgnoreCase))
		{
			return ET66MiniInteractableType::QuickReviveMachine;
		}

		return ET66MiniInteractableType::TreasureChest;
	}

	FString T66MiniBossProfileToVisualID(const FString& BossProfile)
	{
		if (BossProfile.Equals(TEXT("Sharpshooter"), ESearchCase::IgnoreCase))
		{
			return TEXT("Roost");
		}
		if (BossProfile.Equals(TEXT("Juggernaut"), ESearchCase::IgnoreCase))
		{
			return TEXT("Cow");
		}
		if (BossProfile.Equals(TEXT("Duelist"), ESearchCase::IgnoreCase))
		{
			return TEXT("Goat");
		}

		return TEXT("Pig");
	}

	FString T66MiniMiniDataPath(const TCHAR* FileName)
	{
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("Mini/Data") / FileName);
	}

	void T66MiniBuildFallbackIdols(TArray<FT66MiniIdolDefinition>& OutIdols)
	{
		struct FFallbackIdol
		{
			const TCHAR* IdolID;
			const TCHAR* Category;
			const TCHAR* IconPath;
			float BaseProperty;
			float PropertyPerLevel;
		};

		static const FFallbackIdol FallbackIdols[] = {
			{ TEXT("Idol_Curse"), TEXT("DOT"), TEXT("/Game/Idols/Sprites/Black/Idol_Curse_black.Idol_Curse_black"), 1.f, 1.f },
			{ TEXT("Idol_Lava"), TEXT("DOT"), TEXT("/Game/Idols/Sprites/Black/Idol_Lava_black.Idol_Lava_black"), 1.f, 1.f },
			{ TEXT("Idol_Poison"), TEXT("DOT"), TEXT("/Game/Idols/Sprites/Black/Idol_Poison_black.Idol_Poison_black"), 1.f, 1.f },
			{ TEXT("Idol_Bleed"), TEXT("DOT"), TEXT("/Game/Idols/Sprites/Black/Idol_Bleed_black.Idol_Bleed_black"), 1.f, 1.f },
			{ TEXT("Idol_Electric"), TEXT("Bounce"), TEXT("/Game/Idols/Sprites/Black/Idol_Electric_black.Idol_Electric_black"), 1.f, 1.f },
			{ TEXT("Idol_Ice"), TEXT("Bounce"), TEXT("/Game/Idols/Sprites/Black/Idol_Ice_black.Idol_Ice_black"), 1.f, 1.f },
			{ TEXT("Idol_Shadow"), TEXT("Bounce"), TEXT("/Game/Idols/Sprites/Black/Idol_Shadow_black.Idol_Shadow_black"), 1.f, 1.f },
			{ TEXT("Idol_Star"), TEXT("Bounce"), TEXT("/Game/Idols/Sprites/Black/Idol_Star_black.Idol_Star_black"), 1.f, 1.f },
			{ TEXT("Idol_Earth"), TEXT("AOE"), TEXT("/Game/Idols/Sprites/Black/Idol_Earth_black.Idol_Earth_black"), 200.f, 20.f },
			{ TEXT("Idol_Water"), TEXT("AOE"), TEXT("/Game/Idols/Sprites/Black/Idol_Water_black.Idol_Water_black"), 200.f, 20.f },
			{ TEXT("Idol_BlackHole"), TEXT("AOE"), TEXT("/Game/Idols/Sprites/Black/Idol_BlackHole_black.Idol_BlackHole_black"), 200.f, 20.f },
			{ TEXT("Idol_Storm"), TEXT("AOE"), TEXT("/Game/Idols/Sprites/Black/Idol_Storm_black.Idol_Storm_black"), 200.f, 20.f },
			{ TEXT("Idol_Light"), TEXT("Pierce"), TEXT("/Game/Idols/Sprites/Black/Idol_Light_black.Idol_Light_black"), 1.f, 1.f },
			{ TEXT("Idol_Steel"), TEXT("Pierce"), TEXT("/Game/Idols/Sprites/Black/Idol_Steel_black.Idol_Steel_black"), 1.f, 1.f },
			{ TEXT("Idol_Wood"), TEXT("Pierce"), TEXT("/Game/Idols/Sprites/Black/Idol_Wood_black.Idol_Wood_black"), 1.f, 1.f },
			{ TEXT("Idol_Bone"), TEXT("Pierce"), TEXT("/Game/Idols/Sprites/Black/Idol_Bone_black.Idol_Bone_black"), 1.f, 1.f }
		};

		for (const FFallbackIdol& FallbackIdol : FallbackIdols)
		{
			FT66MiniIdolDefinition& Definition = OutIdols.AddDefaulted_GetRef();
			Definition.IdolID = FName(FallbackIdol.IdolID);
			Definition.Category = FallbackIdol.Category;
			Definition.IconPath = FallbackIdol.IconPath;
			Definition.MaxLevel = 10;
			Definition.BaseDamage = 8.f;
			Definition.DamagePerLevel = 4.f;
			Definition.BaseProperty = FallbackIdol.BaseProperty;
			Definition.PropertyPerLevel = FallbackIdol.PropertyPerLevel;
		}
	}
}

void UT66MiniDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ReloadData();
}

void UT66MiniDataSubsystem::ReloadData()
{
	LoadHeroes();
	LoadIdols();
	LoadCompanions();
	LoadDifficulties();
	LoadEnemies();
	LoadBosses();
	LoadWaves();
	LoadInteractables();
	LoadItems();
}

const FT66MiniHeroDefinition* UT66MiniDataSubsystem::FindHero(const FName HeroID) const
{
	return Heroes.FindByPredicate([HeroID](const FT66MiniHeroDefinition& Definition)
	{
		return Definition.HeroID == HeroID;
	});
}

const FT66MiniIdolDefinition* UT66MiniDataSubsystem::FindIdol(const FName IdolID) const
{
	return Idols.FindByPredicate([IdolID](const FT66MiniIdolDefinition& Definition)
	{
		return Definition.IdolID == IdolID;
	});
}

const FT66MiniCompanionDefinition* UT66MiniDataSubsystem::FindCompanion(const FName CompanionID) const
{
	return Companions.FindByPredicate([CompanionID](const FT66MiniCompanionDefinition& Definition)
	{
		return Definition.CompanionID == CompanionID;
	});
}

const FT66MiniDifficultyDefinition* UT66MiniDataSubsystem::FindDifficulty(const FName DifficultyID) const
{
	return Difficulties.FindByPredicate([DifficultyID](const FT66MiniDifficultyDefinition& Definition)
	{
		return Definition.DifficultyID == DifficultyID;
	});
}

const FT66MiniEnemyDefinition* UT66MiniDataSubsystem::FindEnemy(const FName EnemyID) const
{
	return Enemies.FindByPredicate([EnemyID](const FT66MiniEnemyDefinition& Definition)
	{
		return Definition.EnemyID == EnemyID;
	});
}

const FT66MiniBossDefinition* UT66MiniDataSubsystem::FindBoss(const FName BossID) const
{
	return Bosses.FindByPredicate([BossID](const FT66MiniBossDefinition& Definition)
	{
		return Definition.BossID == BossID;
	});
}

const FT66MiniWaveDefinition* UT66MiniDataSubsystem::FindWave(const FName DifficultyID, const int32 WaveIndex) const
{
	return Waves.FindByPredicate([DifficultyID, WaveIndex](const FT66MiniWaveDefinition& Definition)
	{
		return Definition.DifficultyID == DifficultyID && Definition.WaveIndex == WaveIndex;
	});
}

const FT66MiniInteractableDefinition* UT66MiniDataSubsystem::FindInteractable(const FName InteractableID) const
{
	return Interactables.FindByPredicate([InteractableID](const FT66MiniInteractableDefinition& Definition)
	{
		return Definition.InteractableID == InteractableID;
	});
}

const FT66MiniItemDefinition* UT66MiniDataSubsystem::FindItem(const FName ItemID) const
{
	return Items.FindByPredicate([ItemID](const FT66MiniItemDefinition& Definition)
	{
		return Definition.ItemID == ItemID;
	});
}

void UT66MiniDataSubsystem::LoadHeroes()
{
	Heroes.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_Heroes.csv"))))
	{
		FT66MiniHeroDefinition Definition;
		Definition.HeroID = FName(*T66MiniGetValue(Row, TEXT("HeroID")));
		Definition.DisplayName = T66MiniGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66MiniGetValue(Row, TEXT("Description"));
		Definition.PrimaryCategory = T66MiniGetValue(Row, TEXT("PrimaryCategory"));
		Definition.PlaceholderColor = T66MiniToColor(T66MiniGetValue(Row, TEXT("PlaceholderColor")), FLinearColor(0.24f, 0.28f, 0.36f, 1.0f));
		Definition.PortraitPath = T66MiniGetValue(Row, TEXT("Portrait"));
		Definition.PortraitFullPath = T66MiniGetValue(Row, TEXT("PortraitFull"));
		Definition.BaseDamage = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseDamage")));
		Definition.BaseAttackSpeed = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseAttackSpeed")));
		Definition.BaseArmor = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseArmor")));
		Definition.BaseLuck = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseLuck")));
		Definition.BaseSpeed = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseSpeed")));
		Definition.bUnlockedByDefault = T66MiniToBool(T66MiniGetValue(Row, TEXT("bUnlockedByDefault")), true);

		if (Definition.HeroID != NAME_None)
		{
			Heroes.Add(MoveTemp(Definition));
		}
	}
}

void UT66MiniDataSubsystem::LoadIdols()
{
	Idols.Reset();
	const FString CsvPath = T66MiniMiniDataPath(TEXT("T66Mini_Idols.csv"));

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(CsvPath))
	{
		FT66MiniIdolDefinition Definition;
		Definition.IdolID = FName(*T66MiniGetValue(Row, TEXT("IdolID")));
		Definition.Category = T66MiniGetValue(Row, TEXT("Category"));
		Definition.IconPath = T66MiniGetValue(Row, TEXT("Icon"));
		Definition.MaxLevel = T66MiniToInt(T66MiniGetValue(Row, TEXT("MaxLevel")), 1);
		Definition.BaseDamage = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseDamage")));
		Definition.DamagePerLevel = T66MiniToFloat(T66MiniGetValue(Row, TEXT("DamagePerLevel")));
		Definition.BaseProperty = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseProperty")));
		Definition.PropertyPerLevel = T66MiniToFloat(T66MiniGetValue(Row, TEXT("PropertyPerLevel")));

		if (Definition.IdolID != NAME_None)
		{
			Idols.Add(MoveTemp(Definition));
		}
	}

	if (Idols.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("T66MiniDataSubsystem: failed to load mini idol rows from '%s'. Falling back to built-in idol definitions."), *CsvPath);
		T66MiniBuildFallbackIdols(Idols);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("T66MiniDataSubsystem: loaded %d mini idols from '%s'."), Idols.Num(), *CsvPath);
	}
}

void UT66MiniDataSubsystem::LoadCompanions()
{
	Companions.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_Companions.csv"))))
	{
		FT66MiniCompanionDefinition Definition;
		Definition.CompanionID = T66MiniToName(Row, TEXT("CompanionID"));
		Definition.DisplayName = T66MiniGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66MiniGetValue(Row, TEXT("Description"));
		Definition.VisualID = T66MiniGetValue(Row, TEXT("VisualID"));
		Definition.PlaceholderColor = T66MiniToColor(T66MiniGetValue(Row, TEXT("PlaceholderColor")), FLinearColor(0.48f, 0.38f, 0.22f, 1.0f));
		Definition.BaseHealingPerSecond = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseHealingPerSecond")), 5.0f);
		Definition.UnlockStageRequirement = T66MiniToInt(T66MiniGetValue(Row, TEXT("UnlockStageRequirement")), 0);
		Definition.FollowOffsetX = T66MiniToFloat(T66MiniGetValue(Row, TEXT("FollowOffsetX")), -145.0f);
		Definition.FollowOffsetY = T66MiniToFloat(T66MiniGetValue(Row, TEXT("FollowOffsetY")), 110.0f);
		Definition.bUnlockedByDefault = T66MiniToBool(T66MiniGetValue(Row, TEXT("bUnlockedByDefault")), false);

		if (Definition.VisualID.IsEmpty())
		{
			Definition.VisualID = Definition.DisplayName;
		}

		if (Definition.CompanionID != NAME_None)
		{
			Companions.Add(MoveTemp(Definition));
		}
	}
}

void UT66MiniDataSubsystem::LoadDifficulties()
{
	Difficulties.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_Difficulties.csv"))))
	{
		FT66MiniDifficultyDefinition Definition;
		Definition.DifficultyID = T66MiniToName(Row, TEXT("DifficultyID"));
		Definition.DisplayName = T66MiniGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66MiniGetValue(Row, TEXT("Description"));
		Definition.AccentColor = T66MiniToColor(T66MiniGetValue(Row, TEXT("AccentColor")), FLinearColor(0.34f, 0.44f, 0.60f, 1.0f));
		Definition.HealthScalar = T66MiniToFloat(T66MiniGetValue(Row, TEXT("HealthScalar")), 1.0f);
		Definition.DamageScalar = T66MiniToFloat(T66MiniGetValue(Row, TEXT("DamageScalar")), 1.0f);
		Definition.SpeedScalar = T66MiniToFloat(T66MiniGetValue(Row, TEXT("SpeedScalar")), 1.0f);
		Definition.SpawnRateScalar = T66MiniToFloat(T66MiniGetValue(Row, TEXT("SpawnRateScalar")), 1.0f);
		Definition.BossScalar = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BossScalar")), 1.0f);
		Definition.InteractableInterval = T66MiniToFloat(T66MiniGetValue(Row, TEXT("InteractableInterval")), 18.0f);

		if (Definition.DifficultyID != NAME_None)
		{
			Difficulties.Add(MoveTemp(Definition));
		}
	}
}

void UT66MiniDataSubsystem::LoadEnemies()
{
	Enemies.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_Enemies.csv"))))
	{
		FT66MiniEnemyDefinition Definition;
		Definition.EnemyID = T66MiniToName(Row, TEXT("EnemyID"));
		Definition.DisplayName = T66MiniGetValue(Row, TEXT("DisplayName"));
		Definition.VisualID = T66MiniGetValue(Row, TEXT("VisualID"));
		Definition.BehaviorProfile = T66MiniToBehaviorProfile(T66MiniGetValue(Row, TEXT("BehaviorProfile")));
		Definition.BaseHealth = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseHealth")), 30.0f);
		Definition.BaseSpeed = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseSpeed")), 260.0f);
		Definition.BaseTouchDamage = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseTouchDamage")), 8.0f);
		Definition.BaseMaterials = T66MiniToInt(T66MiniGetValue(Row, TEXT("BaseMaterials")), 4);
		Definition.BaseExperience = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseExperience")), 5.0f);
		Definition.SpawnWeight = T66MiniToFloat(T66MiniGetValue(Row, TEXT("SpawnWeight")), 1.0f);

		if (Definition.EnemyID != NAME_None)
		{
			Enemies.Add(MoveTemp(Definition));
		}
	}
}

void UT66MiniDataSubsystem::LoadBosses()
{
	Bosses.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_Bosses.csv"))))
	{
		FT66MiniBossDefinition Definition;
		const FString BossProfile = T66MiniGetValue(Row, TEXT("BossPartProfile"));
		Definition.BossID = T66MiniToName(Row, TEXT("BossID"));
		Definition.DisplayName = Definition.BossID.ToString().Replace(TEXT("_"), TEXT(" "));
		Definition.VisualID = T66MiniBossProfileToVisualID(BossProfile);
		Definition.BehaviorProfile = T66MiniToBehaviorProfile(BossProfile);
		Definition.MaxHealth = T66MiniToFloat(T66MiniGetValue(Row, TEXT("MaxHP")), 1500.0f);
		Definition.MoveSpeed = T66MiniToFloat(T66MiniGetValue(Row, TEXT("MoveSpeed")), 360.0f);
		Definition.TouchDamage = FMath::Max(16.0f, T66MiniToFloat(T66MiniGetValue(Row, TEXT("ProjectileDamageHearts")), 1.0f) * 14.0f);
		Definition.MaterialReward = FMath::Max(20, T66MiniToInt(T66MiniGetValue(Row, TEXT("PointValue")), 500) / 40);
		Definition.ExperienceReward = FMath::Max(18.0f, T66MiniToFloat(T66MiniGetValue(Row, TEXT("PointValue")), 500.0f) / 55.0f);
		Definition.TelegraphSeconds = 1.15f;

		if (Definition.BossID != NAME_None)
		{
			Bosses.Add(MoveTemp(Definition));
		}
	}
}

void UT66MiniDataSubsystem::LoadWaves()
{
	Waves.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_Waves.csv"))))
	{
		FT66MiniWaveDefinition Definition;
		Definition.DifficultyID = T66MiniToName(Row, TEXT("DifficultyID"));
		Definition.WaveIndex = T66MiniToInt(T66MiniGetValue(Row, TEXT("WaveIndex")), 1);
		Definition.DurationSeconds = T66MiniToFloat(T66MiniGetValue(Row, TEXT("DurationSeconds")), 180.0f);
		Definition.BossID = T66MiniToName(Row, TEXT("BossID"));
		Definition.EnemyIDs = T66MiniSplitNameList(T66MiniGetValue(Row, TEXT("EnemyIDs")));
		Definition.SpawnInterval = T66MiniToFloat(T66MiniGetValue(Row, TEXT("SpawnInterval")), 1.2f);
		Definition.InteractableInterval = T66MiniToFloat(T66MiniGetValue(Row, TEXT("InteractableInterval")), 18.0f);
		Definition.EnemyHealthScalar = T66MiniToFloat(T66MiniGetValue(Row, TEXT("EnemyHealthScalar")), 1.0f);
		Definition.EnemyDamageScalar = T66MiniToFloat(T66MiniGetValue(Row, TEXT("EnemyDamageScalar")), 1.0f);
		Definition.EnemySpeedScalar = T66MiniToFloat(T66MiniGetValue(Row, TEXT("EnemySpeedScalar")), 1.0f);

		if (Definition.DifficultyID != NAME_None && Definition.WaveIndex > 0)
		{
			Waves.Add(MoveTemp(Definition));
		}
	}
}

void UT66MiniDataSubsystem::LoadInteractables()
{
	Interactables.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_Interactables.csv"))))
	{
		FT66MiniInteractableDefinition Definition;
		Definition.InteractableID = T66MiniToName(Row, TEXT("InteractableID"));
		Definition.DisplayName = T66MiniGetValue(Row, TEXT("DisplayName"));
		Definition.Type = T66MiniToInteractableType(T66MiniGetValue(Row, TEXT("Type")));
		Definition.VisualID = T66MiniGetValue(Row, TEXT("VisualID"));
		Definition.LifetimeSeconds = T66MiniToFloat(T66MiniGetValue(Row, TEXT("LifetimeSeconds")), 12.0f);
		Definition.SpawnWeight = T66MiniToFloat(T66MiniGetValue(Row, TEXT("SpawnWeight")), 1.0f);
		Definition.MaterialReward = T66MiniToInt(T66MiniGetValue(Row, TEXT("MaterialReward")), 0);
		Definition.GoldReward = T66MiniToInt(T66MiniGetValue(Row, TEXT("GoldReward")), 0);
		Definition.ExperienceReward = T66MiniToFloat(T66MiniGetValue(Row, TEXT("ExperienceReward")), 0.0f);
		Definition.HealAmount = T66MiniToFloat(T66MiniGetValue(Row, TEXT("HealAmount")), 0.0f);

		if (Definition.InteractableID != NAME_None)
		{
			Interactables.Add(MoveTemp(Definition));
		}
	}
}

void UT66MiniDataSubsystem::LoadItems()
{
	Items.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_Items.csv"))))
	{
		FT66MiniItemDefinition Definition;
		Definition.ItemID = FName(*T66MiniGetValue(Row, TEXT("ItemID")));
		Definition.IconPath = T66MiniGetValue(Row, TEXT("Icon"));
		Definition.PrimaryStatType = T66MiniGetValue(Row, TEXT("PrimaryStatType"));
		Definition.SecondaryStatType = T66MiniGetValue(Row, TEXT("SecondaryStatType"));
		Definition.BaseBuyGold = T66MiniToInt(T66MiniGetValue(Row, TEXT("BaseBuyGold")));
		Definition.BaseSellGold = T66MiniToInt(T66MiniGetValue(Row, TEXT("BaseSellGold")));

		if (Definition.ItemID != NAME_None)
		{
			Items.Add(MoveTemp(Definition));
		}
	}
}
