// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66RuntimePlatformSubsystem.h"

#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

THIRD_PARTY_INCLUDES_START
#include "steam/steam_api.h"
THIRD_PARTY_INCLUDES_END

namespace
{
	const UT66RuntimePlatformSettings* T66RuntimePlatformSettings()
	{
		return GetDefault<UT66RuntimePlatformSettings>();
	}
}

bool UT66RuntimePlatformSubsystem::ParseRuntimePlatformName(const FString& Value, ET66RuntimePlatformProfile& OutProfile)
{
	if (Value.Equals(TEXT("SteamDeck"), ESearchCase::IgnoreCase)
		|| Value.Equals(TEXT("Deck"), ESearchCase::IgnoreCase)
		|| Value.Equals(TEXT("SteamOS"), ESearchCase::IgnoreCase))
	{
		OutProfile = ET66RuntimePlatformProfile::SteamDeck;
		return true;
	}

	if (Value.Equals(TEXT("Desktop"), ESearchCase::IgnoreCase)
		|| Value.Equals(TEXT("Windows"), ESearchCase::IgnoreCase)
		|| Value.Equals(TEXT("Full"), ESearchCase::IgnoreCase))
	{
		OutProfile = ET66RuntimePlatformProfile::Desktop;
		return true;
	}

	return false;
}

bool UT66RuntimePlatformSubsystem::IsRunningOnSteamDeckViaSteamworks()
{
	if (ISteamUtils* SteamUtilsApi = SteamUtils())
	{
		return SteamUtilsApi->IsSteamRunningOnSteamDeck();
	}

	return false;
}

ET66RuntimePlatformProfile UT66RuntimePlatformSubsystem::GetEffectiveRuntimePlatformProfile() const
{
	FString PlatformOverride;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66RuntimePlatform="), PlatformOverride))
	{
		ET66RuntimePlatformProfile ParsedProfile = ET66RuntimePlatformProfile::Desktop;
		if (ParseRuntimePlatformName(PlatformOverride, ParsedProfile))
		{
			return ParsedProfile;
		}
	}

	if (FParse::Param(FCommandLine::Get(), TEXT("T66SteamDeck")))
	{
		return ET66RuntimePlatformProfile::SteamDeck;
	}

	if (FParse::Param(FCommandLine::Get(), TEXT("T66Desktop")))
	{
		return ET66RuntimePlatformProfile::Desktop;
	}

	const UT66RuntimePlatformSettings* Settings = T66RuntimePlatformSettings();
	if (Settings && Settings->bDetectSteamDeck && IsRunningOnSteamDeckViaSteamworks())
	{
		return ET66RuntimePlatformProfile::SteamDeck;
	}

	return ET66RuntimePlatformProfile::Desktop;
}

bool UT66RuntimePlatformSubsystem::IsSteamDeckLikeDevice() const
{
	return GetEffectiveRuntimePlatformProfile() == ET66RuntimePlatformProfile::SteamDeck;
}

bool UT66RuntimePlatformSubsystem::ShouldShowMediaViewer() const
{
	const UT66RuntimePlatformSettings* Settings = T66RuntimePlatformSettings();
	if (IsSteamDeckLikeDevice())
	{
		return Settings ? Settings->bAllowMediaViewerOnSteamDeck : false;
	}

	return true;
}

float UT66RuntimePlatformSubsystem::GetDefaultUIScale() const
{
	const UT66RuntimePlatformSettings* Settings = T66RuntimePlatformSettings();
	if (IsSteamDeckLikeDevice() && Settings)
	{
		return Settings->SteamDeckDefaultUIScale;
	}

	return 1.0f;
}

int32 UT66RuntimePlatformSubsystem::GetDefaultScalabilityLevel() const
{
	const UT66RuntimePlatformSettings* Settings = T66RuntimePlatformSettings();
	if (IsSteamDeckLikeDevice() && Settings)
	{
		return FMath::Clamp(Settings->SteamDeckDefaultScalabilityLevel, 0, 3);
	}

	return 3;
}

float UT66RuntimePlatformSubsystem::GetDefaultFrameRateLimit() const
{
	const UT66RuntimePlatformSettings* Settings = T66RuntimePlatformSettings();
	if (IsSteamDeckLikeDevice() && Settings)
	{
		return FMath::Max(0.0f, Settings->SteamDeckDefaultFrameRateLimit);
	}

	return 60.0f;
}
