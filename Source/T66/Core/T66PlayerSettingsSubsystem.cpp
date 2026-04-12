// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66PlayerSettingsSaveGame.h"
#include "Gameplay/T66PlayerController.h"
#include "UI/Style/T66Style.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/App.h"
#include "Misc/PackageName.h"
#include "Sound/SoundClass.h"
#include "Core/T66MediaViewerSubsystem.h"

const FString UT66PlayerSettingsSubsystem::SlotName(TEXT("T66_PlayerSettings"));

namespace
{
	ET66MediaViewerSource SanitizeMediaViewerSourceIndex(int32 RawValue)
	{
		switch (static_cast<ET66MediaViewerSource>(RawValue))
		{
		case ET66MediaViewerSource::TikTok:
		case ET66MediaViewerSource::Shorts:
		case ET66MediaViewerSource::Reels:
			return static_cast<ET66MediaViewerSource>(RawValue);
		default:
			return ET66MediaViewerSource::TikTok;
		}
	}

	ET66BeatTargetSelectionMode SanitizeBeatTargetSelectionMode(const ET66BeatTargetSelectionMode RawValue)
	{
		switch (RawValue)
		{
		case ET66BeatTargetSelectionMode::PersonalBest:
		case ET66BeatTargetSelectionMode::FriendsTop:
		case ET66BeatTargetSelectionMode::StreamersTop:
		case ET66BeatTargetSelectionMode::GlobalTop:
		case ET66BeatTargetSelectionMode::FavoriteRun:
			return RawValue;
		default:
			return ET66BeatTargetSelectionMode::GlobalTop;
		}
	}

	void SanitizeBeatTargetSelection(FT66BeatTargetSelection& Selection)
	{
		Selection.Mode = SanitizeBeatTargetSelectionMode(Selection.Mode);
		if (Selection.Mode != ET66BeatTargetSelectionMode::FavoriteRun)
		{
			Selection.FavoriteEntryId.Reset();
		}
	}

	int32 FindFavoriteLeaderboardRunIndexByEntryId(const TArray<FT66FavoriteLeaderboardRun>& Favorites, const FString& EntryId)
	{
		if (EntryId.IsEmpty())
		{
			return INDEX_NONE;
		}

		for (int32 Index = 0; Index < Favorites.Num(); ++Index)
		{
			if (Favorites[Index].EntryId.Equals(EntryId, ESearchCase::CaseSensitive))
			{
				return Index;
			}
		}

		return INDEX_NONE;
	}

	int32 FindFavoriteAchievementIndexById(const TArray<FName>& Favorites, const FName AchievementID)
	{
		if (AchievementID.IsNone())
		{
			return INDEX_NONE;
		}

		for (int32 Index = 0; Index < Favorites.Num(); ++Index)
		{
			if (Favorites[Index] == AchievementID)
			{
				return Index;
			}
		}

		return INDEX_NONE;
	}
}

void UT66PlayerSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadOrCreate();
	ApplyUIScale();

	ApplyAudioToEngine();
	ApplyUnfocusedAudioToEngine();
}

void UT66PlayerSettingsSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UT66PlayerSettingsSubsystem::LoadOrCreate()
{
	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
	SettingsObj = Cast<UT66PlayerSettingsSaveGame>(Loaded);
	if (!SettingsObj)
	{
		SettingsObj = Cast<UT66PlayerSettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(UT66PlayerSettingsSaveGame::StaticClass()));
		Save();
		return;
	}

	bool bNeedsSave = false;

	if (SettingsObj->SchemaVersion < 5)
	{
		SettingsObj->SchemaVersion = 5;
		SettingsObj->bFogEnabled = true;
		bNeedsSave = true;
	}

	if (SettingsObj->SchemaVersion < 6)
	{
		SettingsObj->SchemaVersion = 6;
		SettingsObj->bFogEnabled = true;
		SettingsObj->FogIntensityPercent = 55.0f;
		bNeedsSave = true;
	}

	if (SettingsObj->SchemaVersion < 10)
	{
		SettingsObj->SchemaVersion = 10;
		SettingsObj->UIScale = 1.0f;
		bNeedsSave = true;
	}

	if (SettingsObj->SchemaVersion < 11)
	{
		SettingsObj->SchemaVersion = 11;
		SettingsObj->FavoriteFriendSteamIds.Reset();
		bNeedsSave = true;
	}

	if (SettingsObj->SchemaVersion < 12)
	{
		SettingsObj->SchemaVersion = 12;
		SettingsObj->bSpeedRunMode = true;
		SettingsObj->MasterVolume = 0.0f;
		bNeedsSave = true;
	}

	if (SettingsObj->SchemaVersion < 14)
	{
		SettingsObj->SchemaVersion = 14;
		SettingsObj->MediaViewerSourceIndex = static_cast<int32>(ET66MediaViewerSource::TikTok);
		bNeedsSave = true;
	}

	if (SettingsObj->SchemaVersion < 15)
	{
		SettingsObj->SchemaVersion = 15;
		SettingsObj->bShowTimeToBeat = true;
		SettingsObj->bShowScoreToBeat = true;
		SettingsObj->bShowTimePacing = false;
		SettingsObj->bShowScorePacing = false;
		SettingsObj->TimeToBeatSelection = FT66BeatTargetSelection{};
		SettingsObj->ScoreToBeatSelection = FT66BeatTargetSelection{};
		SettingsObj->FavoriteLeaderboardRuns.Reset();
		bNeedsSave = true;
	}

	if (SettingsObj->SchemaVersion < 16)
	{
		SettingsObj->SchemaVersion = 16;
		SettingsObj->FavoriteAchievementIds.Reset();
		bNeedsSave = true;
	}

	if (SettingsObj->SchemaVersion < 17)
	{
		SettingsObj->SchemaVersion = 17;
		bNeedsSave = true;
	}

	const ET66MediaViewerSource SanitizedMediaViewerSource = SanitizeMediaViewerSourceIndex(SettingsObj->MediaViewerSourceIndex);
	if (SettingsObj->MediaViewerSourceIndex != static_cast<int32>(SanitizedMediaViewerSource))
	{
		SettingsObj->MediaViewerSourceIndex = static_cast<int32>(SanitizedMediaViewerSource);
		bNeedsSave = true;
	}

	{
		TSet<FName> SeenAchievementIds;
		for (int32 Index = SettingsObj->FavoriteAchievementIds.Num() - 1; Index >= 0; --Index)
		{
			const FName AchievementID = SettingsObj->FavoriteAchievementIds[Index];
			if (AchievementID.IsNone() || SeenAchievementIds.Contains(AchievementID))
			{
				SettingsObj->FavoriteAchievementIds.RemoveAt(Index);
				bNeedsSave = true;
				continue;
			}

			SeenAchievementIds.Add(AchievementID);
		}
	}

	FT66BeatTargetSelection SanitizedTimeSelection = SettingsObj->TimeToBeatSelection;
	SanitizeBeatTargetSelection(SanitizedTimeSelection);
	if (SanitizedTimeSelection.Mode != SettingsObj->TimeToBeatSelection.Mode
		|| SanitizedTimeSelection.FavoriteEntryId != SettingsObj->TimeToBeatSelection.FavoriteEntryId)
	{
		SettingsObj->TimeToBeatSelection = SanitizedTimeSelection;
		bNeedsSave = true;
	}

	FT66BeatTargetSelection SanitizedScoreSelection = SettingsObj->ScoreToBeatSelection;
	SanitizeBeatTargetSelection(SanitizedScoreSelection);
	if (SanitizedScoreSelection.Mode != SettingsObj->ScoreToBeatSelection.Mode
		|| SanitizedScoreSelection.FavoriteEntryId != SettingsObj->ScoreToBeatSelection.FavoriteEntryId)
	{
		SettingsObj->ScoreToBeatSelection = SanitizedScoreSelection;
		bNeedsSave = true;
	}

	if (bNeedsSave)
	{
		Save();
	}
}

void UT66PlayerSettingsSubsystem::Save()
{
	if (!SettingsObj) return;
	UGameplayStatics::SaveGameToSlot(SettingsObj, SlotName, 0);
	OnSettingsChanged.Broadcast();
}

int32 UT66PlayerSettingsSubsystem::GetLastSettingsTabIndex() const
{
	return SettingsObj ? SettingsObj->LastSettingsTabIndex : 0;
}

void UT66PlayerSettingsSubsystem::SetLastSettingsTabIndex(int32 TabIndex)
{
	if (!SettingsObj) return;
	SettingsObj->LastSettingsTabIndex = FMath::Clamp(TabIndex, 0, 7);
	Save();
}

void UT66PlayerSettingsSubsystem::SetUITheme(ET66UITheme NewTheme)
{
	(void)NewTheme;
	if (!SettingsObj)
	{
		return;
	}

	RebuildThemeAwareUI();
}

ET66UITheme UT66PlayerSettingsSubsystem::GetUITheme() const
{
	return ET66UITheme::Dota;
}

void UT66PlayerSettingsSubsystem::SetUIScale(float NewScale)
{
	if (!SettingsObj) return;

	const float ClampedScale = FMath::Clamp(NewScale, 0.75f, 1.50f);
	if (FMath::IsNearlyEqual(SettingsObj->UIScale, ClampedScale, KINDA_SMALL_NUMBER))
	{
		return;
	}

	SettingsObj->UIScale = ClampedScale;
	ApplyUIScale();
	Save();
}

float UT66PlayerSettingsSubsystem::GetUIScale() const
{
	return SettingsObj ? FMath::Clamp(SettingsObj->UIScale, 0.75f, 1.50f) : 1.0f;
}

bool UT66PlayerSettingsSubsystem::IsFavoriteFriend(const FString& FriendSteamId) const
{
	return SettingsObj
		&& !FriendSteamId.IsEmpty()
		&& SettingsObj->FavoriteFriendSteamIds.Contains(FriendSteamId);
}

void UT66PlayerSettingsSubsystem::SetFavoriteFriend(const FString& FriendSteamId, bool bFavorite)
{
	if (!SettingsObj || FriendSteamId.IsEmpty())
	{
		return;
	}

	const bool bAlreadyFavorite = SettingsObj->FavoriteFriendSteamIds.Contains(FriendSteamId);
	if (bFavorite == bAlreadyFavorite)
	{
		return;
	}

	if (bFavorite)
	{
		SettingsObj->FavoriteFriendSteamIds.Add(FriendSteamId);
	}
	else
	{
		SettingsObj->FavoriteFriendSteamIds.Remove(FriendSteamId);
	}

	Save();
}

bool UT66PlayerSettingsSubsystem::IsFavoriteAchievement(const FName AchievementID) const
{
	return SettingsObj
		&& !AchievementID.IsNone()
		&& FindFavoriteAchievementIndexById(SettingsObj->FavoriteAchievementIds, AchievementID) != INDEX_NONE;
}

void UT66PlayerSettingsSubsystem::SetFavoriteAchievement(const FName AchievementID, const bool bFavorite)
{
	if (!SettingsObj || AchievementID.IsNone())
	{
		return;
	}

	const int32 ExistingIndex = FindFavoriteAchievementIndexById(SettingsObj->FavoriteAchievementIds, AchievementID);
	if (bFavorite)
	{
		if (ExistingIndex != INDEX_NONE)
		{
			SettingsObj->FavoriteAchievementIds.RemoveAt(ExistingIndex);
		}
		SettingsObj->FavoriteAchievementIds.Insert(AchievementID, 0);
		Save();
		return;
	}

	if (ExistingIndex == INDEX_NONE)
	{
		return;
	}

	SettingsObj->FavoriteAchievementIds.RemoveAt(ExistingIndex);
	Save();
}

TArray<FName> UT66PlayerSettingsSubsystem::GetFavoriteAchievementIds() const
{
	return SettingsObj ? SettingsObj->FavoriteAchievementIds : TArray<FName>{};
}

bool UT66PlayerSettingsSubsystem::GetPracticeMode() const
{
	return SettingsObj ? SettingsObj->bPracticeMode : false;
}

void UT66PlayerSettingsSubsystem::SetPracticeMode(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bPracticeMode = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetSubmitLeaderboardAnonymous() const
{
	return SettingsObj ? SettingsObj->bSubmitLeaderboardAnonymous : false;
}

void UT66PlayerSettingsSubsystem::SetSubmitLeaderboardAnonymous(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bSubmitLeaderboardAnonymous = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetSpeedRunMode() const
{
	return SettingsObj ? SettingsObj->bSpeedRunMode : true;
}

void UT66PlayerSettingsSubsystem::SetSpeedRunMode(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bSpeedRunMode = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetGoonerMode() const
{
	return SettingsObj ? SettingsObj->bGoonerMode : false;
}

void UT66PlayerSettingsSubsystem::SetGoonerMode(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bGoonerMode = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetShowTimeToBeat() const
{
	return SettingsObj ? SettingsObj->bShowTimeToBeat : true;
}

void UT66PlayerSettingsSubsystem::SetShowTimeToBeat(bool bEnabled)
{
	if (!SettingsObj || SettingsObj->bShowTimeToBeat == bEnabled) return;
	SettingsObj->bShowTimeToBeat = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetShowScoreToBeat() const
{
	return SettingsObj ? SettingsObj->bShowScoreToBeat : true;
}

void UT66PlayerSettingsSubsystem::SetShowScoreToBeat(bool bEnabled)
{
	if (!SettingsObj || SettingsObj->bShowScoreToBeat == bEnabled) return;
	SettingsObj->bShowScoreToBeat = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetShowTimePacing() const
{
	return SettingsObj ? SettingsObj->bShowTimePacing : false;
}

void UT66PlayerSettingsSubsystem::SetShowTimePacing(bool bEnabled)
{
	if (!SettingsObj || SettingsObj->bShowTimePacing == bEnabled) return;
	SettingsObj->bShowTimePacing = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetShowScorePacing() const
{
	return SettingsObj ? SettingsObj->bShowScorePacing : false;
}

void UT66PlayerSettingsSubsystem::SetShowScorePacing(bool bEnabled)
{
	if (!SettingsObj || SettingsObj->bShowScorePacing == bEnabled) return;
	SettingsObj->bShowScorePacing = bEnabled;
	Save();
}

FT66BeatTargetSelection UT66PlayerSettingsSubsystem::GetTimeToBeatSelection() const
{
	FT66BeatTargetSelection Selection = SettingsObj ? SettingsObj->TimeToBeatSelection : FT66BeatTargetSelection{};
	SanitizeBeatTargetSelection(Selection);
	return Selection;
}

void UT66PlayerSettingsSubsystem::SetTimeToBeatSelection(const FT66BeatTargetSelection& Selection)
{
	if (!SettingsObj) return;
	FT66BeatTargetSelection SanitizedSelection = Selection;
	SanitizeBeatTargetSelection(SanitizedSelection);
	if (SettingsObj->TimeToBeatSelection.Mode == SanitizedSelection.Mode
		&& SettingsObj->TimeToBeatSelection.FavoriteEntryId == SanitizedSelection.FavoriteEntryId)
	{
		return;
	}

	SettingsObj->TimeToBeatSelection = SanitizedSelection;
	Save();
}

FT66BeatTargetSelection UT66PlayerSettingsSubsystem::GetScoreToBeatSelection() const
{
	FT66BeatTargetSelection Selection = SettingsObj ? SettingsObj->ScoreToBeatSelection : FT66BeatTargetSelection{};
	SanitizeBeatTargetSelection(Selection);
	return Selection;
}

void UT66PlayerSettingsSubsystem::SetScoreToBeatSelection(const FT66BeatTargetSelection& Selection)
{
	if (!SettingsObj) return;
	FT66BeatTargetSelection SanitizedSelection = Selection;
	SanitizeBeatTargetSelection(SanitizedSelection);
	if (SettingsObj->ScoreToBeatSelection.Mode == SanitizedSelection.Mode
		&& SettingsObj->ScoreToBeatSelection.FavoriteEntryId == SanitizedSelection.FavoriteEntryId)
	{
		return;
	}

	SettingsObj->ScoreToBeatSelection = SanitizedSelection;
	Save();
}

bool UT66PlayerSettingsSubsystem::IsFavoriteLeaderboardRun(const FString& EntryId) const
{
	return SettingsObj && FindFavoriteLeaderboardRunIndexByEntryId(SettingsObj->FavoriteLeaderboardRuns, EntryId) != INDEX_NONE;
}

bool UT66PlayerSettingsSubsystem::FindFavoriteLeaderboardRun(const FString& EntryId, FT66FavoriteLeaderboardRun& OutFavorite) const
{
	if (!SettingsObj)
	{
		return false;
	}

	const int32 FavoriteIndex = FindFavoriteLeaderboardRunIndexByEntryId(SettingsObj->FavoriteLeaderboardRuns, EntryId);
	if (FavoriteIndex == INDEX_NONE)
	{
		return false;
	}

	OutFavorite = SettingsObj->FavoriteLeaderboardRuns[FavoriteIndex];
	return true;
}

TArray<FT66FavoriteLeaderboardRun> UT66PlayerSettingsSubsystem::GetFavoriteLeaderboardRuns() const
{
	return SettingsObj ? SettingsObj->FavoriteLeaderboardRuns : TArray<FT66FavoriteLeaderboardRun>{};
}

void UT66PlayerSettingsSubsystem::SetFavoriteLeaderboardRun(const FT66FavoriteLeaderboardRun& Favorite, bool bFavorite)
{
	if (!SettingsObj || Favorite.EntryId.IsEmpty())
	{
		return;
	}

	const int32 ExistingIndex = FindFavoriteLeaderboardRunIndexByEntryId(SettingsObj->FavoriteLeaderboardRuns, Favorite.EntryId);
	if (bFavorite)
	{
		FT66FavoriteLeaderboardRun SanitizedFavorite = Favorite;
		SanitizedFavorite.EntryId = SanitizedFavorite.EntryId.TrimStartAndEnd();
		SanitizedFavorite.DisplayName = SanitizedFavorite.DisplayName.TrimStartAndEnd();
		if (SanitizedFavorite.EntryId.IsEmpty())
		{
			return;
		}

		if (ExistingIndex != INDEX_NONE)
		{
			SettingsObj->FavoriteLeaderboardRuns.RemoveAt(ExistingIndex);
		}
		SettingsObj->FavoriteLeaderboardRuns.Insert(SanitizedFavorite, 0);
		Save();
		return;
	}

	if (ExistingIndex == INDEX_NONE)
	{
		return;
	}

	SettingsObj->FavoriteLeaderboardRuns.RemoveAt(ExistingIndex);

	auto ClearSelectionIfFavoriteMatches = [Favorite](FT66BeatTargetSelection& Selection)
	{
		if (Selection.Mode == ET66BeatTargetSelectionMode::FavoriteRun
			&& Selection.FavoriteEntryId.Equals(Favorite.EntryId, ESearchCase::CaseSensitive))
		{
			Selection.Mode = ET66BeatTargetSelectionMode::GlobalTop;
			Selection.FavoriteEntryId.Reset();
		}
	};

	ClearSelectionIfFavoriteMatches(SettingsObj->TimeToBeatSelection);
	ClearSelectionIfFavoriteMatches(SettingsObj->ScoreToBeatSelection);
	Save();
}

float UT66PlayerSettingsSubsystem::GetMasterVolume() const
{
	return SettingsObj ? SettingsObj->MasterVolume : 0.0f;
}

void UT66PlayerSettingsSubsystem::SetMasterVolume(float NewValue01)
{
	if (!SettingsObj) return;
	SettingsObj->MasterVolume = FMath::Clamp(NewValue01, 0.0f, 1.0f);
	ApplyAudioToEngine();
	Save();
}

float UT66PlayerSettingsSubsystem::GetMusicVolume() const
{
	return SettingsObj ? SettingsObj->MusicVolume : 0.6f;
}

void UT66PlayerSettingsSubsystem::SetMusicVolume(float NewValue01)
{
	if (!SettingsObj) return;
	SettingsObj->MusicVolume = FMath::Clamp(NewValue01, 0.0f, 1.0f);
	ApplyClassVolumesIfPresent();
	Save();
}

float UT66PlayerSettingsSubsystem::GetSfxVolume() const
{
	return SettingsObj ? SettingsObj->SfxVolume : 0.8f;
}

void UT66PlayerSettingsSubsystem::SetSfxVolume(float NewValue01)
{
	if (!SettingsObj) return;
	SettingsObj->SfxVolume = FMath::Clamp(NewValue01, 0.0f, 1.0f);
	ApplyClassVolumesIfPresent();
	Save();
}

bool UT66PlayerSettingsSubsystem::GetMuteWhenUnfocused() const
{
	return SettingsObj ? SettingsObj->bMuteWhenUnfocused : false;
}

void UT66PlayerSettingsSubsystem::SetMuteWhenUnfocused(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bMuteWhenUnfocused = bEnabled;
	ApplyUnfocusedAudioToEngine();
	Save();
}

void UT66PlayerSettingsSubsystem::SetOutputDeviceId(const FString& NewId)
{
	if (!SettingsObj) return;
	SettingsObj->OutputDeviceId = NewId;
	// TODO: Apply to audio device routing when implemented.
	Save();
}

FString UT66PlayerSettingsSubsystem::GetOutputDeviceId() const
{
	return SettingsObj ? SettingsObj->OutputDeviceId : FString();
}

void UT66PlayerSettingsSubsystem::SetSubtitlesAlwaysOn(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bSubtitlesAlwaysOn = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetSubtitlesAlwaysOn() const
{
	return SettingsObj ? SettingsObj->bSubtitlesAlwaysOn : false;
}

void UT66PlayerSettingsSubsystem::SetDisplayModeIndex(int32 Index)
{
	if (!SettingsObj) return;
	SettingsObj->DisplayModeIndex = FMath::Clamp(Index, 0, 1);
	Save();
}

int32 UT66PlayerSettingsSubsystem::GetDisplayModeIndex() const
{
	return SettingsObj ? SettingsObj->DisplayModeIndex : 0;
}

void UT66PlayerSettingsSubsystem::SetMonitorIndex(int32 Index)
{
	if (!SettingsObj) return;
	SettingsObj->MonitorIndex = FMath::Max(0, Index);
	Save();
}

int32 UT66PlayerSettingsSubsystem::GetMonitorIndex() const
{
	return SettingsObj ? SettingsObj->MonitorIndex : 0;
}

void UT66PlayerSettingsSubsystem::SetHudToggleAffectsInventory(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bHudToggleAffectsInventory = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetHudToggleAffectsInventory() const
{
	return SettingsObj ? SettingsObj->bHudToggleAffectsInventory : true;
}

void UT66PlayerSettingsSubsystem::SetHudToggleAffectsMinimap(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bHudToggleAffectsMinimap = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetHudToggleAffectsMinimap() const
{
	return SettingsObj ? SettingsObj->bHudToggleAffectsMinimap : true;
}

void UT66PlayerSettingsSubsystem::SetHudToggleAffectsIdolSlots(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bHudToggleAffectsIdolSlots = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetHudToggleAffectsIdolSlots() const
{
	return SettingsObj ? SettingsObj->bHudToggleAffectsIdolSlots : true;
}

void UT66PlayerSettingsSubsystem::SetHudToggleAffectsPortraitStats(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bHudToggleAffectsPortraitStats = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetHudToggleAffectsPortraitStats() const
{
	return SettingsObj ? SettingsObj->bHudToggleAffectsPortraitStats : true;
}

void UT66PlayerSettingsSubsystem::SetMediaViewerEnabled(bool bEnabled)
{
	if (!SettingsObj) return;
	if (SettingsObj->bMediaViewerEnabled == bEnabled)
	{
		return;
	}
	SettingsObj->bMediaViewerEnabled = bEnabled;

	if (!bEnabled)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
			{
				if (MV->IsMediaViewerOpen())
				{
					MV->SetMediaViewerOpen(false);
				}
			}
		}
	}

	Save();
}

bool UT66PlayerSettingsSubsystem::GetMediaViewerEnabled() const
{
	return SettingsObj ? SettingsObj->bMediaViewerEnabled : true;
}

void UT66PlayerSettingsSubsystem::SetMediaViewerSource(ET66MediaViewerSource NewSource)
{
	if (!SettingsObj) return;

	const ET66MediaViewerSource SanitizedSource = SanitizeMediaViewerSourceIndex(static_cast<int32>(NewSource));
	if (SettingsObj->MediaViewerSourceIndex == static_cast<int32>(SanitizedSource))
	{
		return;
	}

	SettingsObj->MediaViewerSourceIndex = static_cast<int32>(SanitizedSource);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			MV->SetMediaViewerSource(SanitizedSource);
		}
	}

	Save();
}

ET66MediaViewerSource UT66PlayerSettingsSubsystem::GetMediaViewerSource() const
{
	return SettingsObj
		? SanitizeMediaViewerSourceIndex(SettingsObj->MediaViewerSourceIndex)
		: ET66MediaViewerSource::TikTok;
}

void UT66PlayerSettingsSubsystem::SetFogEnabled(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bFogEnabled = bEnabled;
	if (bEnabled && SettingsObj->FogIntensityPercent <= KINDA_SMALL_NUMBER)
	{
		SettingsObj->FogIntensityPercent = 55.0f;
	}
	Save();
}

bool UT66PlayerSettingsSubsystem::GetFogEnabled() const
{
	return SettingsObj ? (SettingsObj->bFogEnabled && SettingsObj->FogIntensityPercent > KINDA_SMALL_NUMBER) : true;
}

void UT66PlayerSettingsSubsystem::SetFogIntensityPercent(float NewValue)
{
	if (!SettingsObj) return;

	SettingsObj->FogIntensityPercent = FMath::Clamp(NewValue, 0.0f, 100.0f);
	SettingsObj->bFogEnabled = SettingsObj->FogIntensityPercent > KINDA_SMALL_NUMBER;
	Save();
}

float UT66PlayerSettingsSubsystem::GetFogIntensityPercent() const
{
	return SettingsObj ? FMath::Clamp(SettingsObj->FogIntensityPercent, 0.0f, 100.0f) : 55.0f;
}

void UT66PlayerSettingsSubsystem::SetRetroFXSettings(const FT66RetroFXSettings& NewSettings)
{
	if (!SettingsObj) return;
	SettingsObj->RetroFXSettings = NewSettings;
	Save();
}

FT66RetroFXSettings UT66PlayerSettingsSubsystem::GetRetroFXSettings() const
{
	static const FT66RetroFXSettings DefaultSettings;
	return SettingsObj ? SettingsObj->RetroFXSettings : DefaultSettings;
}

void UT66PlayerSettingsSubsystem::ResetRetroFXSettingsToDefaults()
{
	if (!SettingsObj) return;
	SettingsObj->RetroFXSettings = FT66RetroFXSettings();
	Save();
}

void UT66PlayerSettingsSubsystem::ApplyUIScale()
{
	RebuildThemeAwareUI();
}

void UT66PlayerSettingsSubsystem::RebuildThemeAwareUI()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UWorld* World = GI->GetWorld())
		{
			if (AT66PlayerController* PC = Cast<AT66PlayerController>(World->GetFirstPlayerController()))
			{
				PC->RebuildThemeAwareUI();
			}
		}
	}
}

void UT66PlayerSettingsSubsystem::ApplyAudioToEngine()
{
	if (!SettingsObj) return;
	// Bible: when Media Viewer is open, all other game audio is muted.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			if (MV->IsMediaViewerOpen())
			{
				return;
			}
		}
	}
	// Global master volume multiplier.
	FApp::SetVolumeMultiplier(FMath::Clamp(SettingsObj->MasterVolume, 0.0f, 1.0f));
	ApplyClassVolumesIfPresent();
}

void UT66PlayerSettingsSubsystem::ApplyUnfocusedAudioToEngine()
{
	if (!SettingsObj) return;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			if (MV->IsMediaViewerOpen())
			{
				return;
			}
		}
	}
	// If muted when unfocused, drop unfocused volume to 0; otherwise keep at 1.
	FApp::SetUnfocusedVolumeMultiplier(SettingsObj->bMuteWhenUnfocused ? 0.0f : 1.0f);
}

void UT66PlayerSettingsSubsystem::ApplySafeModeSettings()
{
	if (!SettingsObj) return;

	// Gameplay-side stability toggles.
	SettingsObj->bIntenseVisuals = false;
	SettingsObj->RetroFXSettings = FT66RetroFXSettings();

	// Audio: keep user master, but enforce mute-unfocused off for stability/debug.
	SettingsObj->bMuteWhenUnfocused = false;
	ApplyUnfocusedAudioToEngine();

	// Graphics: conservative defaults.
	if (UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		GUS->SetOverallScalabilityLevel(0);     // Best performance
		GUS->SetFrameRateLimit(60.0f);          // Conservative cap
		GUS->SetFullscreenMode(EWindowMode::Windowed);
		GUS->ApplySettings(false);
		GUS->SaveSettings();
	}

	Save();
}

void UT66PlayerSettingsSubsystem::ApplyClassVolumesIfPresent()
{
	if (!SettingsObj) return;

	// Foundation: if/when we create SoundClass assets, we can apply per-class volume multipliers.
	// This is safe to call even if assets don't exist yet.
	static const TCHAR* MusicClassPackagePath = TEXT("/Game/Audio/SC_Music");
	static const TCHAR* MusicClassPath = TEXT("/Game/Audio/SC_Music.SC_Music");
	static const TCHAR* SfxClassPackagePath = TEXT("/Game/Audio/SC_SFX");
	static const TCHAR* SfxClassPath = TEXT("/Game/Audio/SC_SFX.SC_SFX");

	if (FPackageName::DoesPackageExist(MusicClassPackagePath))
	{
		if (USoundClass* Music = LoadObject<USoundClass>(nullptr, MusicClassPath))
		{
			Music->Properties.Volume = FMath::Clamp(SettingsObj->MusicVolume, 0.0f, 1.0f);
		}
	}
	if (FPackageName::DoesPackageExist(SfxClassPackagePath))
	{
		if (USoundClass* Sfx = LoadObject<USoundClass>(nullptr, SfxClassPath))
		{
			Sfx->Properties.Volume = FMath::Clamp(SettingsObj->SfxVolume, 0.0f, 1.0f);
		}
	}
}



