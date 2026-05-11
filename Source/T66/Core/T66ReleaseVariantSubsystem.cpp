// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66ReleaseVariantSubsystem.h"

#include "Core/T66SteamHelper.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
	TArray<ET66Difficulty> T66AllDifficulties()
	{
		return {
			ET66Difficulty::Easy,
			ET66Difficulty::Medium,
			ET66Difficulty::Hard,
			ET66Difficulty::VeryHard,
			ET66Difficulty::Impossible
		};
	}

	FString T66NormalizeReleaseToken(const FString& Value)
	{
		FString Normalized = Value.TrimStartAndEnd().ToLower();
		Normalized.ReplaceInline(TEXT(" "), TEXT(""));
		Normalized.ReplaceInline(TEXT("_"), TEXT(""));
		Normalized.ReplaceInline(TEXT("-"), TEXT(""));
		Normalized.ReplaceInline(TEXT("et66releasevariant::"), TEXT(""));
		Normalized.ReplaceInline(TEXT("et66difficulty::"), TEXT(""));
		return Normalized;
	}
}

bool UT66ReleaseVariantSubsystem::ParseReleaseVariantName(const FString& Value, ET66ReleaseVariant& OutVariant)
{
	const FString Normalized = T66NormalizeReleaseToken(Value);
	if (Normalized == TEXT("demo") || Normalized == TEXT("steamdemo"))
	{
		OutVariant = ET66ReleaseVariant::SteamDemo;
		return true;
	}

	if (Normalized == TEXT("full") || Normalized == TEXT("fullgame") || Normalized == TEXT("retail"))
	{
		OutVariant = ET66ReleaseVariant::FullGame;
		return true;
	}

	return false;
}

bool UT66ReleaseVariantSubsystem::ParseDifficultyName(FName DifficultyID, ET66Difficulty& OutDifficulty)
{
	const FString Normalized = T66NormalizeReleaseToken(DifficultyID.ToString());
	if (Normalized == TEXT("easy"))
	{
		OutDifficulty = ET66Difficulty::Easy;
		return true;
	}
	if (Normalized == TEXT("medium"))
	{
		OutDifficulty = ET66Difficulty::Medium;
		return true;
	}
	if (Normalized == TEXT("hard"))
	{
		OutDifficulty = ET66Difficulty::Hard;
		return true;
	}
	if (Normalized == TEXT("veryhard"))
	{
		OutDifficulty = ET66Difficulty::VeryHard;
		return true;
	}
	if (Normalized == TEXT("impossible"))
	{
		OutDifficulty = ET66Difficulty::Impossible;
		return true;
	}

	return false;
}

ET66ReleaseVariant UT66ReleaseVariantSubsystem::GetEffectiveReleaseVariant() const
{
	const TCHAR* CommandLine = FCommandLine::Get();

	FString VariantValue;
	if (FParse::Value(CommandLine, TEXT("T66ReleaseVariant="), VariantValue))
	{
		ET66ReleaseVariant ParsedVariant = ET66ReleaseVariant::FullGame;
		if (ParseReleaseVariantName(VariantValue, ParsedVariant))
		{
			return ParsedVariant;
		}
	}

	if (FParse::Param(CommandLine, TEXT("T66FullGame")))
	{
		return ET66ReleaseVariant::FullGame;
	}

	if (FParse::Param(CommandLine, TEXT("T66Demo")) || FParse::Param(CommandLine, TEXT("T66SteamDemo")))
	{
		return ET66ReleaseVariant::SteamDemo;
	}

	const UT66ReleaseVariantSettings* Settings = GetDefault<UT66ReleaseVariantSettings>();
	if (Settings && Settings->ReleaseVariant == ET66ReleaseVariant::SteamDemo)
	{
		return ET66ReleaseVariant::SteamDemo;
	}

	if (Settings && Settings->bDetectSteamDemoAppId && Settings->DemoSteamAppId > 0)
	{
		if (const UGameInstance* OwningGameInstance = GetGameInstance())
		{
			if (const UT66SteamHelper* SteamHelper = OwningGameInstance->GetSubsystem<UT66SteamHelper>())
			{
				const FString ActiveAppId = SteamHelper->GetActiveSteamAppId();
				int32 ActiveAppIdValue = 0;
				if (LexTryParseString(ActiveAppIdValue, *ActiveAppId) && ActiveAppIdValue == Settings->DemoSteamAppId)
				{
					return ET66ReleaseVariant::SteamDemo;
				}
			}
		}
	}

	return ET66ReleaseVariant::FullGame;
}

bool UT66ReleaseVariantSubsystem::IsSteamDemoBuild() const
{
	return GetEffectiveReleaseVariant() == ET66ReleaseVariant::SteamDemo;
}

bool UT66ReleaseVariantSubsystem::IsHeroAllowed(FName HeroID) const
{
	if (!IsSteamDemoBuild() || HeroID.IsNone())
	{
		return true;
	}

	const UT66ReleaseVariantSettings* Settings = GetDefault<UT66ReleaseVariantSettings>();
	if (!Settings || Settings->DemoAllowedHeroIDs.Num() == 0)
	{
		return true;
	}

	return Settings->DemoAllowedHeroIDs.Contains(HeroID);
}

bool UT66ReleaseVariantSubsystem::IsDifficultyAllowed(ET66Difficulty Difficulty) const
{
	if (!IsSteamDemoBuild())
	{
		return true;
	}

	const TArray<ET66Difficulty> PlayableDifficulties = GetPlayableDifficulties();
	return PlayableDifficulties.Contains(Difficulty);
}

TArray<FName> UT66ReleaseVariantSubsystem::FilterHeroIDs(const TArray<FName>& HeroIDs) const
{
	if (!IsSteamDemoBuild())
	{
		return HeroIDs;
	}

	const UT66ReleaseVariantSettings* Settings = GetDefault<UT66ReleaseVariantSettings>();
	if (!Settings || Settings->DemoAllowedHeroIDs.Num() == 0)
	{
		return HeroIDs;
	}

	TArray<FName> FilteredHeroIDs;
	FilteredHeroIDs.Reserve(FMath::Min(HeroIDs.Num(), Settings->DemoAllowedHeroIDs.Num()));
	for (const FName& HeroID : HeroIDs)
	{
		if (Settings->DemoAllowedHeroIDs.Contains(HeroID))
		{
			FilteredHeroIDs.Add(HeroID);
		}
	}
	return FilteredHeroIDs;
}

TArray<ET66Difficulty> UT66ReleaseVariantSubsystem::GetPlayableDifficulties() const
{
	if (!IsSteamDemoBuild())
	{
		return T66AllDifficulties();
	}

	const UT66ReleaseVariantSettings* Settings = GetDefault<UT66ReleaseVariantSettings>();
	if (!Settings || Settings->DemoAllowedDifficultyIDs.Num() == 0)
	{
		return T66AllDifficulties();
	}

	TArray<ET66Difficulty> ParsedDifficulties;
	ParsedDifficulties.Reserve(Settings->DemoAllowedDifficultyIDs.Num());
	for (const FName& DifficultyID : Settings->DemoAllowedDifficultyIDs)
	{
		ET66Difficulty ParsedDifficulty = ET66Difficulty::Easy;
		if (ParseDifficultyName(DifficultyID, ParsedDifficulty))
		{
			ParsedDifficulties.AddUnique(ParsedDifficulty);
		}
	}

	return ParsedDifficulties.Num() > 0 ? ParsedDifficulties : T66AllDifficulties();
}

ET66Difficulty UT66ReleaseVariantSubsystem::ResolvePlayableDifficulty(ET66Difficulty Difficulty) const
{
	const TArray<ET66Difficulty> PlayableDifficulties = GetPlayableDifficulties();
	if (PlayableDifficulties.Contains(Difficulty))
	{
		return Difficulty;
	}

	return PlayableDifficulties.Num() > 0 ? PlayableDifficulties[0] : ET66Difficulty::Easy;
}
