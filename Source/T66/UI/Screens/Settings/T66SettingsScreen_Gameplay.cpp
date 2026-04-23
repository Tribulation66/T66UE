// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

using namespace T66SettingsScreenPrivate;

namespace
{
	FText GetBeatTargetBuiltinLabel(UT66LocalizationSubsystem* Loc, const ET66BeatTargetSelectionMode Mode)
	{
		switch (Mode)
		{
		case ET66BeatTargetSelectionMode::PersonalBest:
			return NSLOCTEXT("T66.Settings", "BeatTargetPersonalBest", "Personal Best");
		case ET66BeatTargetSelectionMode::FriendsTop:
			return Loc ? Loc->GetText_Friends() : NSLOCTEXT("T66.Settings", "BeatTargetFriends", "Friends");
		case ET66BeatTargetSelectionMode::StreamersTop:
			return Loc ? Loc->GetText_Streamers() : NSLOCTEXT("T66.Settings", "BeatTargetStreamers", "Streamers");
		case ET66BeatTargetSelectionMode::GlobalTop:
		default:
			return Loc ? Loc->GetText_Global() : NSLOCTEXT("T66.Settings", "BeatTargetGlobal", "Global");
		}
	}

	FText GetBeatTargetDifficultyLabel(UT66LocalizationSubsystem* Loc, const ET66Difficulty Difficulty)
	{
		if (!Loc)
		{
			switch (Difficulty)
			{
			case ET66Difficulty::Medium: return NSLOCTEXT("T66.Settings", "BeatTargetDifficultyMedium", "Medium");
			case ET66Difficulty::Hard: return NSLOCTEXT("T66.Settings", "BeatTargetDifficultyHard", "Hard");
			case ET66Difficulty::VeryHard: return NSLOCTEXT("T66.Settings", "BeatTargetDifficultyVeryHard", "Very Hard");
			case ET66Difficulty::Impossible: return NSLOCTEXT("T66.Settings", "BeatTargetDifficultyImpossible", "Impossible");
			case ET66Difficulty::Easy:
			default:
				return NSLOCTEXT("T66.Settings", "BeatTargetDifficultyEasy", "Easy");
			}
		}

		switch (Difficulty)
		{
		case ET66Difficulty::Medium: return Loc->GetText_Medium();
		case ET66Difficulty::Hard: return Loc->GetText_Hard();
		case ET66Difficulty::VeryHard: return Loc->GetText_VeryHard();
		case ET66Difficulty::Impossible: return Loc->GetText_Impossible();
		case ET66Difficulty::Easy:
		default:
			return Loc->GetText_Easy();
		}
	}

	FText GetBeatTargetPartyLabel(UT66LocalizationSubsystem* Loc, const ET66PartySize PartySize)
	{
		if (!Loc)
		{
			switch (PartySize)
			{
			case ET66PartySize::Duo: return NSLOCTEXT("T66.Settings", "BeatTargetPartyDuo", "Duo");
			case ET66PartySize::Trio: return NSLOCTEXT("T66.Settings", "BeatTargetPartyTrio", "Trio");
			case ET66PartySize::Quad: return NSLOCTEXT("T66.Settings", "BeatTargetPartyQuad", "Quad");
			case ET66PartySize::Solo:
			default:
				return NSLOCTEXT("T66.Settings", "BeatTargetPartySolo", "Solo");
			}
		}

		switch (PartySize)
		{
		case ET66PartySize::Duo: return Loc->GetText_Duo();
		case ET66PartySize::Trio: return Loc->GetText_Trio();
		case ET66PartySize::Quad: return Loc->GetText_Quad();
		case ET66PartySize::Solo:
		default:
			return Loc->GetText_Solo();
		}
	}

	FText GetBeatTargetTimeScopeLabel(UT66LocalizationSubsystem* Loc, const ET66LeaderboardTime TimeScope)
	{
		if (TimeScope == ET66LeaderboardTime::Current)
		{
			return Loc ? Loc->GetText_Weekly() : NSLOCTEXT("T66.Settings", "BeatTargetWeekly", "Weekly");
		}

		return NSLOCTEXT("T66.Settings", "BeatTargetAllTime", "All Time");
	}

	FString GetBeatTargetFilterInitial(const ET66LeaderboardFilter Filter)
	{
		switch (Filter)
		{
		case ET66LeaderboardFilter::Friends:
			return TEXT("F");
		case ET66LeaderboardFilter::Streamers:
			return TEXT("S");
		case ET66LeaderboardFilter::Global:
		default:
			return TEXT("G");
		}
	}

	FText MakeFavoriteBeatTargetLabel(UT66LocalizationSubsystem* Loc, const FT66FavoriteLeaderboardRun& Favorite)
	{
		const FString DisplayName = Favorite.DisplayName.TrimStartAndEnd().IsEmpty()
			? NSLOCTEXT("T66.Settings", "FavoriteRunFallbackName", "Saved Run").ToString()
			: Favorite.DisplayName.TrimStartAndEnd();

		return FText::Format(
			NSLOCTEXT("T66.Settings", "FavoriteBeatTargetDisplay", "{0} #{1} {2} | {3} | {4} {5}"),
			FText::FromString(GetBeatTargetFilterInitial(Favorite.Filter)),
			FText::AsNumber(FMath::Max(1, Favorite.Rank)),
			FText::FromString(DisplayName),
			GetBeatTargetTimeScopeLabel(Loc, Favorite.TimeScope),
			GetBeatTargetDifficultyLabel(Loc, Favorite.Difficulty),
			GetBeatTargetPartyLabel(Loc, Favorite.PartySize));
	}

	FText GetBeatTargetSelectionLabel(UT66LocalizationSubsystem* Loc, const FT66BeatTargetSelection& Selection, const TArray<FT66FavoriteLeaderboardRun>& Favorites)
	{
		if (Selection.Mode != ET66BeatTargetSelectionMode::FavoriteRun)
		{
			return GetBeatTargetBuiltinLabel(Loc, Selection.Mode);
		}

		for (const FT66FavoriteLeaderboardRun& Favorite : Favorites)
		{
			if (Favorite.EntryId == Selection.FavoriteEntryId)
			{
				return MakeFavoriteBeatTargetLabel(Loc, Favorite);
			}
		}

		return NSLOCTEXT("T66.Settings", "FavoriteRunMissing", "Missing Favorite Run");
	}
}
TSharedRef<SWidget> UT66SettingsScreen::BuildGameplayTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();

	auto MakeBeatTargetSourceMenu = [this, Loc, PS](TFunction<void(const FT66BeatTargetSelection&)> SetSelection, TFunction<FText()> GetCurrentValue) -> TFunction<TSharedRef<SWidget>(const TSharedPtr<STextBlock>&)>
	{
		return [this, Loc, PS, SetSelection, GetCurrentValue](const TSharedPtr<STextBlock>& CurrentValueText) -> TSharedRef<SWidget>
		{
			TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);

			auto AddOptionButton = [this, &Box, SetSelection, GetCurrentValue, CurrentValueText](const FT66BeatTargetSelection& OptionSelection, const FText& Label)
			{
				Box->AddSlot().AutoHeight()
				[
					MakeSettingsButton(
						FT66ButtonParams(Label, FOnClicked::CreateLambda([this, SetSelection, OptionSelection, GetCurrentValue, CurrentValueText]()
						{
							SetSelection(OptionSelection);
							if (CurrentValueText.IsValid())
							{
								CurrentValueText->SetText(GetCurrentValue());
							}
							FSlateApplication::Get().DismissAllMenus();
							return FReply::Handled();
						}), ET66ButtonType::Neutral)
						.SetMinWidth(0.f)
						.SetFontWeight(TEXT("Regular")))
				];
			};

			FT66BeatTargetSelection PersonalBestSelection;
			PersonalBestSelection.Mode = ET66BeatTargetSelectionMode::PersonalBest;
			AddOptionButton(PersonalBestSelection, GetBeatTargetBuiltinLabel(Loc, ET66BeatTargetSelectionMode::PersonalBest));

			FT66BeatTargetSelection FriendsSelection;
			FriendsSelection.Mode = ET66BeatTargetSelectionMode::FriendsTop;
			AddOptionButton(FriendsSelection, GetBeatTargetBuiltinLabel(Loc, ET66BeatTargetSelectionMode::FriendsTop));

			FT66BeatTargetSelection StreamersSelection;
			StreamersSelection.Mode = ET66BeatTargetSelectionMode::StreamersTop;
			AddOptionButton(StreamersSelection, GetBeatTargetBuiltinLabel(Loc, ET66BeatTargetSelectionMode::StreamersTop));

			FT66BeatTargetSelection GlobalSelection;
			GlobalSelection.Mode = ET66BeatTargetSelectionMode::GlobalTop;
			AddOptionButton(GlobalSelection, GetBeatTargetBuiltinLabel(Loc, ET66BeatTargetSelectionMode::GlobalTop));

			if (PS)
			{
				const TArray<FT66FavoriteLeaderboardRun> Favorites = PS->GetFavoriteLeaderboardRuns();
				for (const FT66FavoriteLeaderboardRun& Favorite : Favorites)
				{
					if (Favorite.EntryId.IsEmpty())
					{
						continue;
					}

					FT66BeatTargetSelection FavoriteSelection;
					FavoriteSelection.Mode = ET66BeatTargetSelectionMode::FavoriteRun;
					FavoriteSelection.FavoriteEntryId = Favorite.EntryId;

					AddOptionButton(FavoriteSelection, MakeFavoriteBeatTargetLabel(Loc, Favorite));
				}
			}

			return Box;
		};
	};

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_PracticeMode() : NSLOCTEXT("T66.Settings.Fallback", "Practice Mode", "Practice Mode"),
					[PS]() { return PS ? PS->GetPracticeMode() : false; },
					[PS](bool b) { if (PS) PS->SetPracticeMode(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_SubmitLeaderboardAnonymous() : NSLOCTEXT("T66.Settings.Fallback", "Submit Leaderboard as Anonymous", "Submit Leaderboard as Anonymous"),
					[PS]() { return PS ? PS->GetSubmitLeaderboardAnonymous() : false; },
					[PS](bool b) { if (PS) PS->SetSubmitLeaderboardAnonymous(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_SpeedRunMode() : NSLOCTEXT("T66.Settings.Fallback", "Speed Run Mode", "Speed Run Mode"),
					[PS]() { return PS ? PS->GetSpeedRunMode() : false; },
					[PS](bool b) { if (PS) PS->SetSpeedRunMode(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					NSLOCTEXT("T66.Settings", "ShowTimeToBeatLabel", "Show Time to Beat"),
					[PS]() { return PS ? PS->GetShowTimeToBeat() : true; },
					[PS](bool bValue) { if (PS) PS->SetShowTimeToBeat(bValue); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsDropdownRow(
					NSLOCTEXT("T66.Settings", "TimeToBeatSourceLabel", "Time to Beat Source"),
					[Loc, PS]()
					{
						return GetBeatTargetSelectionLabel(
							Loc,
							PS ? PS->GetTimeToBeatSelection() : FT66BeatTargetSelection{},
							PS ? PS->GetFavoriteLeaderboardRuns() : TArray<FT66FavoriteLeaderboardRun>{});
					},
					MakeBeatTargetSourceMenu(
						[PS](const FT66BeatTargetSelection& Selection)
						{
							if (PS)
							{
								PS->SetTimeToBeatSelection(Selection);
							}
						},
						[Loc, PS]()
						{
							return GetBeatTargetSelectionLabel(
								Loc,
								PS ? PS->GetTimeToBeatSelection() : FT66BeatTargetSelection{},
								PS ? PS->GetFavoriteLeaderboardRuns() : TArray<FT66FavoriteLeaderboardRun>{});
						}),
					0.4f,
					0.6f,
					true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					NSLOCTEXT("T66.Settings", "ShowTimePacingLabel", "Show Time Pacing (Only for Global)"),
					[PS]() { return PS ? PS->GetShowTimePacing() : false; },
					[PS](bool bValue) { if (PS) PS->SetShowTimePacing(bValue); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					NSLOCTEXT("T66.Settings", "ShowScoreToBeatLabel", "Show Score to Beat"),
					[PS]() { return PS ? PS->GetShowScoreToBeat() : true; },
					[PS](bool bValue) { if (PS) PS->SetShowScoreToBeat(bValue); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsDropdownRow(
					NSLOCTEXT("T66.Settings", "ScoreToBeatSourceLabel", "Score to Beat Source"),
					[Loc, PS]()
					{
						return GetBeatTargetSelectionLabel(
							Loc,
							PS ? PS->GetScoreToBeatSelection() : FT66BeatTargetSelection{},
							PS ? PS->GetFavoriteLeaderboardRuns() : TArray<FT66FavoriteLeaderboardRun>{});
					},
					MakeBeatTargetSourceMenu(
						[PS](const FT66BeatTargetSelection& Selection)
						{
							if (PS)
							{
								PS->SetScoreToBeatSelection(Selection);
							}
						},
						[Loc, PS]()
						{
							return GetBeatTargetSelectionLabel(
								Loc,
								PS ? PS->GetScoreToBeatSelection() : FT66BeatTargetSelection{},
								PS ? PS->GetFavoriteLeaderboardRuns() : TArray<FT66FavoriteLeaderboardRun>{});
						}),
					0.4f,
					0.6f,
					true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					NSLOCTEXT("T66.Settings", "ShowScorePacingLabel", "Show Score Pacing (Only for Global)"),
					[PS]() { return PS ? PS->GetShowScorePacing() : false; },
					[PS](bool bValue) { if (PS) PS->SetShowScorePacing(bValue); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_GoonerMode() : NSLOCTEXT("T66.Settings.Fallback", "Gooner Mode", "Gooner Mode"),
					[PS]() { return PS ? PS->GetGoonerMode() : false; },
					[PS](bool b) { if (PS) PS->SetGoonerMode(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsPercentEntryRow(
					NSLOCTEXT("T66.Settings", "NativeFogIntensityLabel", "Native Fog Intensity"),
					NSLOCTEXT("T66.Settings", "NativeFogIntensityBody", "Controls the strength of the gameplay haze from 0 to 100. 0 disables native fog entirely, 100 is intentionally very heavy."),
					[PS]() { return PS ? PS->GetFogIntensityPercent() : 55.0f; },
					[PS](float Value) { if (PS) PS->SetFogIntensityPercent(Value); },
					NSLOCTEXT("T66.Settings", "GameplayNumericHint", "0-100"),
					NSLOCTEXT("T66.Settings", "GameplayNumericHelp", "Enter a value from 0 to 100.")
				)
			]
		];
}


