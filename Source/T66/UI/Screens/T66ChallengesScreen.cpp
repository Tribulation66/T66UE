// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66ChallengesScreen.h"
#include "Core/T66GameInstance.h"
#include "UI/Style/T66Style.h"
#include "Styling/CoreStyle.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	enum class ETabIndex : int32
	{
		Challenges = 0,
		Mods = 1,
		Count
	};

	struct FCommittedSelectionState
	{
		ET66RunModifierKind Kind = ET66RunModifierKind::None;
		FName ID = NAME_None;
	};

	FCommittedSelectionState& GetFallbackCommittedSelection()
	{
		static FCommittedSelectionState Selection;
		return Selection;
	}

	UT66GameInstance* GetT66GameInstance(const UObject* Context)
	{
		return Context ? Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(Context)) : nullptr;
	}

	int32 GetTabIndexForModifierKind(const ET66RunModifierKind Kind)
	{
		return Kind == ET66RunModifierKind::Mod
			? static_cast<int32>(ETabIndex::Mods)
			: static_cast<int32>(ETabIndex::Challenges);
	}

	void SetConfirmedSelection(const UObject* Context, const ET66RunModifierKind Kind, const FName ID)
	{
		if (UT66GameInstance* GI = GetT66GameInstance(Context))
		{
			GI->SelectedRunModifierKind = Kind;
			GI->SelectedRunModifierID = ID;
			return;
		}

		FCommittedSelectionState& State = GetFallbackCommittedSelection();
		State.Kind = Kind;
		State.ID = ID;
	}

	ET66RunModifierKind GetConfirmedSelectionKind(const UObject* Context)
	{
		if (const UT66GameInstance* GI = GetT66GameInstance(Context))
		{
			return GI->SelectedRunModifierKind;
		}

		return GetFallbackCommittedSelection().Kind;
	}

	FName GetConfirmedSelectionID(const UObject* Context)
	{
		if (const UT66GameInstance* GI = GetT66GameInstance(Context))
		{
			return GI->SelectedRunModifierID;
		}

		return GetFallbackCommittedSelection().ID;
	}

	FLinearColor ChallengeShellFill()
	{
		return FLinearColor(0.02f, 0.05f, 0.04f, 0.98f);
	}

	FLinearColor ChallengePanelFill()
	{
		return FLinearColor(0.08f, 0.10f, 0.09f, 1.0f);
	}

	FLinearColor ChallengePanelInsetFill()
	{
		return FLinearColor(0.05f, 0.07f, 0.06f, 1.0f);
	}

	FLinearColor ChallengeHeaderFill()
	{
		return FLinearColor(0.18f, 0.34f, 0.28f, 1.0f);
	}

	FLinearColor ChallengeSelectedFill()
	{
		return FLinearColor(0.16f, 0.27f, 0.22f, 1.0f);
	}

	FLinearColor ChallengeRewardTint()
	{
		return FLinearColor(0.73f, 0.88f, 0.96f, 1.0f);
	}

	FLinearColor ChallengeDangerTint()
	{
		return FLinearColor(0.89f, 0.29f, 0.25f, 1.0f);
	}

	FLinearColor ChallengeSuccessTint()
	{
		return FLinearColor(0.78f, 0.88f, 0.50f, 1.0f);
	}
}

UT66ChallengesScreen::UT66ChallengesScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::Challenges;
	bIsModal = true;
}

const TArray<UT66ChallengesScreen::FChallengeEntry>& UT66ChallengesScreen::GetPlaceholderChallenges() const
{
	static const TArray<FChallengeEntry> PlaceholderChallenges = {
		{
			NSLOCTEXT("T66.Challenges", "Fragile3Title", "Fragile 3"),
			NSLOCTEXT("T66.Challenges", "Fragile3Reward", "1.6x Chad Coupons"),
			NSLOCTEXT("T66.Challenges", "Fragile3Desc", "One hit ends the run. Every floor has to be played as a clean execution check."),
			{
				NSLOCTEXT("T66.Challenges", "Fragile3Constraint1", "Die upon taking any damage."),
				NSLOCTEXT("T66.Challenges", "Fragile3Constraint2", "Bosses still have to be cleared to finish the run.")
			},
			NSLOCTEXT("T66.Challenges", "Fragile3RewardDetail", "Reward: 160 Chad Coupons (placeholder)"),
			NSLOCTEXT("T66.Challenges", "Fragile3Completion", "Not completed"),
			ET66RunModifierKind::Challenge,
			FName(TEXT("Fragile3"))
		},
		{
			NSLOCTEXT("T66.Challenges", "Speedrunner3Title", "Speedrunner 3"),
			NSLOCTEXT("T66.Challenges", "Speedrunner3Reward", "1.5x Chad Coupons"),
			NSLOCTEXT("T66.Challenges", "Speedrunner3Desc", "Push to the boss immediately and clear the stage before the route fully collapses."),
			{
				NSLOCTEXT("T66.Challenges", "Speedrunner3Constraint1", "Beat the stage before the stage timer expires."),
				NSLOCTEXT("T66.Challenges", "Speedrunner3Constraint2", "Skipping floors is allowed, but the boss still has to die.")
			},
			NSLOCTEXT("T66.Challenges", "Speedrunner3RewardDetail", "Reward: 150 Chad Coupons (placeholder)"),
			NSLOCTEXT("T66.Challenges", "Speedrunner3Completion", "Not completed"),
			ET66RunModifierKind::Challenge,
			FName(TEXT("Speedrunner3"))
		},
		{
			NSLOCTEXT("T66.Challenges", "SpeedrunnerPlus3Title", "Speedrunner+ 3"),
			NSLOCTEXT("T66.Challenges", "SpeedrunnerPlus3Reward", "2.0x Chad Coupons"),
			NSLOCTEXT("T66.Challenges", "SpeedrunnerPlus3Desc", "A harsher timed push with almost no recovery space between floors."),
			{
				NSLOCTEXT("T66.Challenges", "SpeedrunnerPlus3Constraint1", "Beat the stage with less than 45 seconds left."),
				NSLOCTEXT("T66.Challenges", "SpeedrunnerPlus3Constraint2", "No revives or support saves.")
			},
			NSLOCTEXT("T66.Challenges", "SpeedrunnerPlus3RewardDetail", "Reward: 200 Chad Coupons (placeholder)"),
			NSLOCTEXT("T66.Challenges", "SpeedrunnerPlus3Completion", "Not completed"),
			ET66RunModifierKind::Challenge,
			FName(TEXT("SpeedrunnerPlus3"))
		},
		{
			NSLOCTEXT("T66.Challenges", "OhNo3Title", "Oh No 3"),
			NSLOCTEXT("T66.Challenges", "OhNo3Reward", "1.5x Chad Coupons"),
			NSLOCTEXT("T66.Challenges", "OhNo3Desc", "Resource denial. Gold matters, but greed kills the run if you overcommit."),
			{
				NSLOCTEXT("T66.Challenges", "OhNo3Constraint1", "Do not spend more than 250 gold in the stage."),
				NSLOCTEXT("T66.Challenges", "OhNo3Constraint2", "No alchemy rerolls.")
			},
			NSLOCTEXT("T66.Challenges", "OhNo3RewardDetail", "Reward: 150 Chad Coupons (placeholder)"),
			NSLOCTEXT("T66.Challenges", "OhNo3Completion", "Not completed"),
			ET66RunModifierKind::Challenge,
			FName(TEXT("OhNo3"))
		},
		{
			NSLOCTEXT("T66.Challenges", "OhShit3Title", "Oh Shit 3"),
			NSLOCTEXT("T66.Challenges", "OhShit3Reward", "1.75x Chad Coupons"),
			NSLOCTEXT("T66.Challenges", "OhShit3Desc", "Flood pressure stays high. If you lose the route, the run gets taken away from you."),
			{
				NSLOCTEXT("T66.Challenges", "OhShit3Constraint1", "Stay above the flooded floor for the whole stage."),
				NSLOCTEXT("T66.Challenges", "OhShit3Constraint2", "Do not use quick revive.")
			},
			NSLOCTEXT("T66.Challenges", "OhShit3RewardDetail", "Reward: 175 Chad Coupons (placeholder)"),
			NSLOCTEXT("T66.Challenges", "OhShit3Completion", "Not completed"),
			ET66RunModifierKind::Challenge,
			FName(TEXT("OhShit3"))
		},
		{
			NSLOCTEXT("T66.Challenges", "WhatTheHell3Title", "What The Hell 3"),
			NSLOCTEXT("T66.Challenges", "WhatTheHell3Reward", "2.0x Chad Coupons"),
			NSLOCTEXT("T66.Challenges", "WhatTheHell3Desc", "The stage is played under stacked restrictions and is meant to feel unfair in a controlled way."),
			{
				NSLOCTEXT("T66.Challenges", "WhatTheHell3Constraint1", "No damage, no revive, and no chest opens."),
				NSLOCTEXT("T66.Challenges", "WhatTheHell3Constraint2", "Kill the boss to validate the challenge.")
			},
			NSLOCTEXT("T66.Challenges", "WhatTheHell3RewardDetail", "Reward: 200 Chad Coupons (placeholder)"),
			NSLOCTEXT("T66.Challenges", "WhatTheHell3Completion", "Not completed"),
			ET66RunModifierKind::Challenge,
			FName(TEXT("WhatTheHell3"))
		}
	};

	return PlaceholderChallenges;
}

const TArray<UT66ChallengesScreen::FChallengeEntry>& UT66ChallengesScreen::GetPlaceholderMods() const
{
	static const TArray<FChallengeEntry> PlaceholderMods = {
		{
			NSLOCTEXT("T66.Challenges", "MaxPowerTitle", "Max Power"),
			NSLOCTEXT("T66.Challenges", "MaxPowerReward", "No Chad Coupons"),
			NSLOCTEXT("T66.Challenges", "MaxPowerDesc", "A full-power modded run that starts at level 99 with all stats maxed."),
			{
				NSLOCTEXT("T66.Challenges", "MaxPowerConstraint1", "Start at level 99."),
				NSLOCTEXT("T66.Challenges", "MaxPowerConstraint2", "All stats begin maxed for the run."),
				NSLOCTEXT("T66.Challenges", "MaxPowerConstraint3", "This is a placeholder mod entry.")
			},
			NSLOCTEXT("T66.Challenges", "MaxPowerRewardDetail", "Reward: none (placeholder)"),
			NSLOCTEXT("T66.Challenges", "MaxPowerCompletion", "Not completed"),
			ET66RunModifierKind::Mod,
			FName(TEXT("Mod_MaxHeroStats"))
		}
	};

	return PlaceholderMods;
}

const TArray<UT66ChallengesScreen::FChallengeEntry>& UT66ChallengesScreen::GetEntriesForTab(const int32 TabIndex) const
{
	return TabIndex == static_cast<int32>(ETabIndex::Mods) ? GetPlaceholderMods() : GetPlaceholderChallenges();
}

void UT66ChallengesScreen::InitializeSelectionState()
{
	if (bSelectionStateInitialized)
	{
		return;
	}

	bSelectionStateInitialized = true;
	ActiveTabIndex = static_cast<int32>(ETabIndex::Challenges);
	PendingSelections[static_cast<int32>(ETabIndex::Challenges)] = { ET66RunModifierKind::Challenge, NAME_None };
	PendingSelections[static_cast<int32>(ETabIndex::Mods)] = { ET66RunModifierKind::Mod, NAME_None };

	for (int32 TabIndex = 0; TabIndex < static_cast<int32>(ETabIndex::Count); ++TabIndex)
	{
		const TArray<FChallengeEntry>& Entries = GetEntriesForTab(TabIndex);
		if (Entries.Num() == 0)
		{
			continue;
		}

		PendingSelections[TabIndex].Kind = Entries[0].RunModifierKind;
		PendingSelections[TabIndex].ID = Entries[0].RunModifierID;
	}

	const ET66RunModifierKind ConfirmedKind = GetConfirmedSelectionKind(this);
	const FName ConfirmedID = GetConfirmedSelectionID(this);
	ActiveTabIndex = FMath::Clamp(GetTabIndexForModifierKind(ConfirmedKind), 0, static_cast<int32>(ETabIndex::Count) - 1);

	if (ConfirmedKind == ET66RunModifierKind::None || ConfirmedID.IsNone())
	{
		return;
	}

	for (int32 TabIndex = 0; TabIndex < static_cast<int32>(ETabIndex::Count); ++TabIndex)
	{
		const TArray<FChallengeEntry>& Entries = GetEntriesForTab(TabIndex);
		for (const FChallengeEntry& Entry : Entries)
		{
			if (Entry.RunModifierKind == ConfirmedKind && Entry.RunModifierID == ConfirmedID)
			{
				PendingSelections[TabIndex].Kind = Entry.RunModifierKind;
				PendingSelections[TabIndex].ID = Entry.RunModifierID;
				return;
			}
		}
	}
}

int32 UT66ChallengesScreen::GetActiveTabIndex()
{
	InitializeSelectionState();
	return FMath::Clamp(ActiveTabIndex, 0, static_cast<int32>(ETabIndex::Count) - 1);
}

int32 UT66ChallengesScreen::GetSelectedEntryIndex(const int32 TabIndex)
{
	InitializeSelectionState();

	const TArray<FChallengeEntry>& Entries = GetEntriesForTab(TabIndex);
	if (Entries.Num() == 0)
	{
		return INDEX_NONE;
	}

	const int32 SafeTabIndex = FMath::Clamp(TabIndex, 0, static_cast<int32>(ETabIndex::Count) - 1);
	const FSelectionState& PendingSelection = PendingSelections[SafeTabIndex];

	for (int32 EntryIndex = 0; EntryIndex < Entries.Num(); ++EntryIndex)
	{
		const FChallengeEntry& Entry = Entries[EntryIndex];
		if (Entry.RunModifierKind == PendingSelection.Kind && Entry.RunModifierID == PendingSelection.ID)
		{
			return EntryIndex;
		}
	}

	return 0;
}

int32 UT66ChallengesScreen::GetConfirmedEntryIndex(const int32 TabIndex) const
{
	const TArray<FChallengeEntry>& Entries = GetEntriesForTab(TabIndex);
	if (Entries.Num() == 0)
	{
		return INDEX_NONE;
	}

	const ET66RunModifierKind ConfirmedKind = GetConfirmedSelectionKind(this);
	const FName ConfirmedID = GetConfirmedSelectionID(this);
	if (ConfirmedKind == ET66RunModifierKind::None || ConfirmedID.IsNone())
	{
		return INDEX_NONE;
	}

	for (int32 EntryIndex = 0; EntryIndex < Entries.Num(); ++EntryIndex)
	{
		const FChallengeEntry& Entry = Entries[EntryIndex];
		if (Entry.RunModifierKind == ConfirmedKind && Entry.RunModifierID == ConfirmedID)
		{
			return EntryIndex;
		}
	}

	return INDEX_NONE;
}

TSharedRef<SWidget> UT66ChallengesScreen::BuildSlateUI()
{
	const int32 CurrentTabIndex = GetActiveTabIndex();
	const TArray<FChallengeEntry>& Entries = GetEntriesForTab(CurrentTabIndex);
	const FVector2D SafeFrameSize = FT66Style::GetSafeFrameSize();
	const float ModalWidth = FMath::Min(SafeFrameSize.X * 0.94f, 1100.0f);
	const float ModalHeight = FMath::Min(SafeFrameSize.Y * 0.92f, 760.0f);
	const float ColumnGap = 12.0f;
	const float BodyPadding = 14.0f;
	const float DetailColumnWidth = FMath::Max(340.0f, ModalWidth * 0.44f);
	const float ListColumnWidth = FMath::Max(320.0f, ModalWidth - DetailColumnWidth - 64.0f);

	if (Entries.Num() == 0)
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FT66Style::Scrim())
			[
				SNew(SBox)
				.WidthOverride(640.f)
				.HeightOverride(320.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(ChallengeShellFill())
					.Padding(18.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().FillHeight(1.f).VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.Challenges", "NoChallenges", "No challenge definitions are available."))
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 12.f, 0.f, 0.f)
						[
							FT66Style::MakeButton(
								FT66ButtonParams(NSLOCTEXT("T66.Challenges", "CloseEmpty", "CLOSE"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleBackClicked), ET66ButtonType::Neutral)
								.SetMinWidth(120.f)
								.SetHeight(34.f))
						]
					]
				]
			];
	}

	const int32 SelectedEntryIndex = GetSelectedEntryIndex(CurrentTabIndex);
	const int32 ConfirmedEntryIndex = GetConfirmedEntryIndex(CurrentTabIndex);
	const int32 DetailEntryIndex = SelectedEntryIndex != INDEX_NONE ? SelectedEntryIndex : 0;
	const FChallengeEntry& SelectedEntry = Entries[DetailEntryIndex];
	const bool bSelectedEntryConfirmed = SelectedEntryIndex != INDEX_NONE && SelectedEntryIndex == ConfirmedEntryIndex;
	const bool bChallengesTabActive = CurrentTabIndex == static_cast<int32>(ETabIndex::Challenges);
	const FText HeaderTitle = bChallengesTabActive
		? NSLOCTEXT("T66.Challenges", "ChallengesTitle", "Challenges Tier 3")
		: NSLOCTEXT("T66.Challenges", "ModsTitle", "Mods Tier 3");
	const FText DetailListHeader = bChallengesTabActive
		? NSLOCTEXT("T66.Challenges", "ConstraintsHeader", "Constraints")
		: NSLOCTEXT("T66.Challenges", "ModifiersHeader", "Modifiers");
	const FText LeaderboardNote = NSLOCTEXT("T66.Challenges", "LeaderboardNote", "Challenge and modded runs are not uploaded to online leaderboards.");
	const FText SelectNote = bChallengesTabActive
		? NSLOCTEXT("T66.Challenges", "SelectChallengeNote", "Pick a challenge and confirm it to arm the placeholder rule set.")
		: NSLOCTEXT("T66.Challenges", "SelectModNote", "Pick a mod and confirm it to arm the placeholder mod set.");

	auto MakeConstraintRow = [](const FText& ConstraintText) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 1.f, 8.f, 0.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Challenges", "ConstraintBullet", "-"))
				.Font(FT66Style::Tokens::FontBold(14))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			]
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(STextBlock)
				.Text(ConstraintText)
				.Font(FT66Style::Tokens::FontRegular(12))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.AutoWrapText(true)
			];
	};

	auto MakeTabButton = [this, CurrentTabIndex](const int32 TabIndex, const FText& Label) -> TSharedRef<SWidget>
	{
		const bool bActive = CurrentTabIndex == TabIndex;

		return FT66Style::MakeButton(
			FT66ButtonParams(Label, FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleTabSelected, TabIndex), bActive ? ET66ButtonType::ToggleActive : ET66ButtonType::Row)
			.SetBorderVisual(ET66ButtonBorderVisual::None)
			.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
			.SetMinWidth(124.f)
			.SetHeight(36.f)
			.SetPadding(FMargin(12.f, 4.f))
			.SetColor(bActive ? ChallengeSelectedFill() : FLinearColor(0.10f, 0.12f, 0.11f, 1.0f))
			.SetTextColor(FT66Style::Tokens::Text)
			.SetUseGlow(false));
	};

	auto MakeEntryRow = [this, CurrentTabIndex](const FChallengeEntry& Entry, const int32 EntryIndex) -> TSharedRef<SWidget>
	{
		const bool bSelected = EntryIndex == GetSelectedEntryIndex(CurrentTabIndex);
		const bool bConfirmed = EntryIndex == GetConfirmedEntryIndex(CurrentTabIndex);

		return FT66Style::MakeButton(
			FT66ButtonParams(FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleEntrySelected, EntryIndex), bSelected ? ET66ButtonType::ToggleActive : ET66ButtonType::Row)
			.SetBorderVisual(ET66ButtonBorderVisual::None)
			.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
			.SetMinWidth(0.f)
			.SetHeight(52.f)
			.SetPadding(FMargin(12.f, 8.f))
			.SetColor(bSelected ? ChallengeSelectedFill() : ChallengePanelInsetFill())
			.SetTextColor(FT66Style::Tokens::Text)
			.SetUseGlow(false)
			.SetContent(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
				[
					SNew(SBox)
					.WidthOverride(22.f)
					.HeightOverride(22.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(bConfirmed ? ChallengeSuccessTint() : FLinearColor(0.10f, 0.12f, 0.11f, 1.0f))
						.Padding(2.f)
						[
							SNew(SBox)
							[
								SNew(STextBlock)
								.Text(bConfirmed ? NSLOCTEXT("T66.Challenges", "ConfirmedMarker", "X") : FText::GetEmpty())
								.Font(FT66Style::Tokens::FontBold(10))
								.ColorAndOpacity(bConfirmed ? FLinearColor(0.08f, 0.09f, 0.07f, 1.0f) : FLinearColor::Transparent)
								.Justification(ETextJustify::Center)
							]
						]
					]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Entry.Title)
					.Font(FT66Style::Tokens::FontBold(15))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Entry.RewardSummary)
					.Font(FT66Style::Tokens::FontBold(13))
					.ColorAndOpacity(ChallengeRewardTint())
				]));
	};

	TSharedRef<SVerticalBox> EntryList = SNew(SVerticalBox);
	for (int32 EntryIndex = 0; EntryIndex < Entries.Num(); ++EntryIndex)
	{
		EntryList->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			MakeEntryRow(Entries[EntryIndex], EntryIndex)
		];
	}

	TSharedRef<SVerticalBox> ConstraintList = SNew(SVerticalBox);
	for (const FText& ConstraintText : SelectedEntry.Constraints)
	{
		ConstraintList->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 8.f)
		[
			MakeConstraintRow(ConstraintText)
		];
	}

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66Style::Scrim())
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SSpacer)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(FMargin(24.f, 28.f))
			[
				SNew(SBox)
				.WidthOverride(ModalWidth)
				.HeightOverride(ModalHeight)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(ChallengeShellFill())
					.Padding(3.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(ChallengeHeaderFill())
							.Padding(FMargin(16.f, 12.f))
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(HeaderTitle)
									.Font(FT66Style::Tokens::FontBold(28))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
									[
										MakeTabButton(static_cast<int32>(ETabIndex::Challenges), NSLOCTEXT("T66.Challenges", "ChallengesTab", "Challenges"))
									]
									+ SHorizontalBox::Slot().AutoWidth()
									[
										MakeTabButton(static_cast<int32>(ETabIndex::Mods), NSLOCTEXT("T66.Challenges", "ModsTab", "Mods"))
									]
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									FT66Style::MakeButton(
										FT66ButtonParams(NSLOCTEXT("T66.Challenges", "Close", "X"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleBackClicked), ET66ButtonType::Danger)
										.SetBorderVisual(ET66ButtonBorderVisual::None)
										.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
										.SetMinWidth(38.f)
										.SetHeight(38.f)
										.SetPadding(FMargin(0.f))
										.SetColor(FLinearColor(0.32f, 0.08f, 0.06f, 1.0f))
										.SetTextColor(FLinearColor(1.0f, 0.18f, 0.15f, 1.0f))
										.SetFontSize(18)
										.SetUseGlow(false))
								]
							]
						]
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.f).Padding(BodyPadding, BodyPadding, ColumnGap, BodyPadding)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(ChallengePanelFill())
								.Padding(8.f)
								[
									SNew(SBox)
									.WidthOverride(ListColumnWidth)
									[
										SNew(SScrollBox)
										+ SScrollBox::Slot()
										[
											EntryList
										]
									]
								]
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, BodyPadding, BodyPadding, BodyPadding)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(ChallengePanelFill())
								.Padding(14.f)
								[
									SNew(SBox)
									.WidthOverride(DetailColumnWidth)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
										[
											SNew(STextBlock)
											.Text(SelectedEntry.Title)
											.Font(FT66Style::Tokens::FontBold(30))
											.ColorAndOpacity(FT66Style::Tokens::Text)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
										[
											SNew(STextBlock)
											.Text(SelectedEntry.Description)
											.Font(FT66Style::Tokens::FontRegular(13))
											.ColorAndOpacity(FT66Style::Tokens::Text)
											.AutoWrapText(true)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
										[
											SNew(STextBlock)
											.Text(DetailListHeader)
											.Font(FT66Style::Tokens::FontBold(14))
											.ColorAndOpacity(ChallengeSuccessTint())
										]
										+ SVerticalBox::Slot().FillHeight(1.f)
										[
											SNew(SBorder)
											.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
											.BorderBackgroundColor(ChallengePanelInsetFill())
											.Padding(12.f)
											[
												ConstraintList
											]
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
										[
											SNew(STextBlock)
											.Text(SelectedEntry.RewardSummary)
											.Font(FT66Style::Tokens::FontBold(22))
											.ColorAndOpacity(ChallengeRewardTint())
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
										[
											SNew(STextBlock)
											.Text(SelectedEntry.RewardDetail)
											.Font(FT66Style::Tokens::FontRegular(12))
											.ColorAndOpacity(FT66Style::Tokens::Text)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
										[
											SNew(STextBlock)
											.Text(LeaderboardNote)
											.Font(FT66Style::Tokens::FontRegular(11))
											.ColorAndOpacity(FT66Style::Tokens::TextMuted)
											.AutoWrapText(true)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
										[
											SNew(STextBlock)
											.Text(SelectedEntry.CompletionPlaceholder)
											.Font(FT66Style::Tokens::FontBold(14))
											.ColorAndOpacity(ChallengeDangerTint())
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
										[
											SNew(STextBlock)
											.Text(bSelectedEntryConfirmed
												? NSLOCTEXT("T66.Challenges", "SelectedNote", "Selected for the next run.")
												: SelectNote)
											.Font(FT66Style::Tokens::FontRegular(11))
											.ColorAndOpacity(FT66Style::Tokens::TextMuted)
											.AutoWrapText(true)
										]
										+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 16.f, 0.f, 0.f)
										[
											FT66Style::MakeButton(
												FT66ButtonParams(
													bSelectedEntryConfirmed
														? NSLOCTEXT("T66.Challenges", "Selected", "SELECTED")
														: NSLOCTEXT("T66.Challenges", "Confirm", "CONFIRM"),
													FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleConfirmClicked),
													ET66ButtonType::Primary)
												.SetMinWidth(174.f)
												.SetHeight(40.f)
												.SetPadding(FMargin(16.f, 8.f, 16.f, 6.f))
												.SetColor(bSelectedEntryConfirmed ? ChallengeSuccessTint() : ChallengeRewardTint())
												.SetTextColor(FLinearColor(0.07f, 0.08f, 0.09f, 1.0f))
												.SetUseGlow(false))
										]
									]
								]
							]
						]
					]
				]
			]
		];
}

FReply UT66ChallengesScreen::HandleBackClicked()
{
	bSelectionStateInitialized = false;
	CloseModal();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleTabSelected(const int32 TabIndex)
{
	InitializeSelectionState();
	ActiveTabIndex = FMath::Clamp(TabIndex, 0, static_cast<int32>(ETabIndex::Count) - 1);
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleEntrySelected(const int32 EntryIndex)
{
	const int32 CurrentTabIndex = GetActiveTabIndex();
	const TArray<FChallengeEntry>& Entries = GetEntriesForTab(CurrentTabIndex);
	if (Entries.IsValidIndex(EntryIndex))
	{
		PendingSelections[CurrentTabIndex].Kind = Entries[EntryIndex].RunModifierKind;
		PendingSelections[CurrentTabIndex].ID = Entries[EntryIndex].RunModifierID;
	}
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleConfirmClicked()
{
	const int32 CurrentTabIndex = GetActiveTabIndex();
	const TArray<FChallengeEntry>& Entries = GetEntriesForTab(CurrentTabIndex);
	const int32 SelectedIndex = GetSelectedEntryIndex(CurrentTabIndex);
	const int32 EntryIndex = Entries.IsValidIndex(SelectedIndex) ? SelectedIndex : 0;
	if (Entries.IsValidIndex(EntryIndex))
	{
		SetConfirmedSelection(this, Entries[EntryIndex].RunModifierKind, Entries[EntryIndex].RunModifierID);
	}
	bSelectionStateInitialized = false;
	CloseModal();
	return FReply::Handled();
}
