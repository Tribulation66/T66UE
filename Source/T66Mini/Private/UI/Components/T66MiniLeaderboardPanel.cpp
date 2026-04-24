// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Components/T66MiniLeaderboardPanel.h"

#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "UI/T66MiniUIStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

ST66MiniLeaderboardPanel::~ST66MiniLeaderboardPanel()
{
	if (LeaderboardSubsystem && LeaderboardUpdatedHandle.IsValid())
	{
		LeaderboardSubsystem->OnLeaderboardUpdated().Remove(LeaderboardUpdatedHandle);
	}
}

void ST66MiniLeaderboardPanel::Construct(const FArguments& InArgs)
{
	DataSubsystem = InArgs._DataSubsystem;
	LeaderboardSubsystem = InArgs._LeaderboardSubsystem;
	PartySubsystem = InArgs._PartySubsystem;
	CurrentPartySize = PartySubsystem ? PartySubsystem->GetCurrentPartySizeEnum() : ET66PartySize::Solo;

	PartySizeOptions = {
		MakeShared<FString>(TEXT("Solo")),
		MakeShared<FString>(TEXT("Duo")),
		MakeShared<FString>(TEXT("Trio")),
		MakeShared<FString>(TEXT("Quad"))
	};

	for (const TSharedPtr<FString>& Option : PartySizeOptions)
	{
		if (Option.IsValid() && PartySizeFromLabel(*Option) == CurrentPartySize)
		{
			SelectedPartySizeOption = Option;
			break;
		}
	}
	if (!SelectedPartySizeOption.IsValid())
	{
		SelectedPartySizeOption = PartySizeOptions[0];
	}

	BuildDifficultyOptions();

	auto MakeFilterButton = [this](const FText& Label, const ET66MiniLeaderboardFilter Filter) -> TSharedRef<SWidget>
	{
		const ET66ButtonType Type = CurrentFilter == Filter ? ET66ButtonType::Success : ET66ButtonType::Neutral;
		return T66MiniUI::MakeButton(
			Label,
			Filter == ET66MiniLeaderboardFilter::Global
				? FOnClicked::CreateSP(this, &ST66MiniLeaderboardPanel::HandleGlobalClicked)
				: FOnClicked::CreateSP(this, &ST66MiniLeaderboardPanel::HandleFriendsClicked),
			Type,
			124.f,
			42.f,
			10);
	};

	TSharedRef<SVerticalBox> Content = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 8.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("LEADERBOARD")))
			.Font(T66MiniUI::TitleFont(24))
			.ColorAndOpacity(FLinearColor::White)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Steam all-time score based on materials banked.")))
			.Font(T66MiniUI::BodyFont(12))
			.ColorAndOpacity(T66MiniUI::MutedText())
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 12.f, 0.f, 0.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
			[
				MakeFilterButton(FText::FromString(TEXT("GLOBAL")), ET66MiniLeaderboardFilter::Global)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeFilterButton(FText::FromString(TEXT("FRIENDS")), ET66MiniLeaderboardFilter::Friends)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 12.f, 0.f, 0.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 8.f, 0.f)
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&PartySizeOptions)
				.InitiallySelectedItem(SelectedPartySizeOption)
				.OnSelectionChanged(SComboBox<TSharedPtr<FString>>::FOnSelectionChanged::CreateSP(this, &ST66MiniLeaderboardPanel::OnPartySizeChanged))
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
				{
					return SNew(STextBlock)
						.Text(FText::FromString(Item.IsValid() ? *Item : TEXT("")))
						.Font(T66MiniUI::BodyFont(13))
						.ColorAndOpacity(FLinearColor::White);
				})
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						return FText::FromString(SelectedPartySizeOption.IsValid() ? *SelectedPartySizeOption : TEXT(""));
					})
					.Font(T66MiniUI::BodyFont(13))
					.ColorAndOpacity(FLinearColor::White)
				]
			]
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&DifficultyOptions)
				.InitiallySelectedItem(SelectedDifficultyOption)
				.OnSelectionChanged(SComboBox<TSharedPtr<FString>>::FOnSelectionChanged::CreateSP(this, &ST66MiniLeaderboardPanel::OnDifficultyChanged))
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
				{
					return SNew(STextBlock)
						.Text(FText::FromString(Item.IsValid() ? *Item : TEXT("")))
						.Font(T66MiniUI::BodyFont(13))
						.ColorAndOpacity(FLinearColor::White);
				})
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						return FText::FromString(SelectedDifficultyOption.IsValid() ? *SelectedDifficultyOption : TEXT(""));
					})
					.Font(T66MiniUI::BodyFont(13))
					.ColorAndOpacity(FLinearColor::White)
				]
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.Padding(0.f, 12.f, 0.f, 0.f)
		[
			SNew(SBorder)
			.BorderImage(T66MiniUI::RowShellBrush() ? T66MiniUI::RowShellBrush() : T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(T66MiniUI::RowShellBrush() ? FLinearColor::White : T66MiniUI::PanelFill())
			.Padding(FMargin(12.f))
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew(EntryListBox, SVerticalBox)
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 10.f, 0.f, 0.f)
		[
			SAssignNew(StatusTextBlock, STextBlock)
			.Text(FText::GetEmpty())
			.Font(T66MiniUI::BodyFont(12))
			.ColorAndOpacity(T66MiniUI::MutedText())
			.AutoWrapText(true)
		];

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(T66MiniUI::ContentShellBrush() ? T66MiniUI::ContentShellBrush() : T66MiniUI::WhiteBrush())
		.BorderBackgroundColor(T66MiniUI::ContentShellBrush() ? FLinearColor::White : T66MiniUI::ShellFill())
		.Padding(FMargin(18.f))
		[
			Content
		]
	];

	if (LeaderboardSubsystem)
	{
		LeaderboardUpdatedHandle = LeaderboardSubsystem->OnLeaderboardUpdated().AddSP(this, &ST66MiniLeaderboardPanel::OnMiniLeaderboardUpdated);
	}

	RefreshLeaderboard();
}

void ST66MiniLeaderboardPanel::BuildDifficultyOptions()
{
	DifficultyOptions.Reset();
	DifficultyIdsByDisplayName.Reset();
	SelectedDifficultyOption.Reset();
	CurrentDifficultyId = NAME_None;

	if (DataSubsystem)
	{
		for (const FT66MiniDifficultyDefinition& DifficultyDefinition : DataSubsystem->GetDifficulties())
		{
			const FString DisplayName = DifficultyDefinition.DisplayName.IsEmpty()
				? DifficultyDefinition.DifficultyID.ToString()
				: DifficultyDefinition.DisplayName;
			TSharedPtr<FString> Option = MakeShared<FString>(DisplayName);
			DifficultyOptions.Add(Option);
			DifficultyIdsByDisplayName.Add(DisplayName, DifficultyDefinition.DifficultyID);

			if (CurrentDifficultyId.IsNone())
			{
				CurrentDifficultyId = DifficultyDefinition.DifficultyID;
				SelectedDifficultyOption = Option;
			}
		}
	}

	if (!SelectedDifficultyOption.IsValid())
	{
		TSharedPtr<FString> Fallback = MakeShared<FString>(TEXT("Easy"));
		DifficultyOptions.Add(Fallback);
		DifficultyIdsByDisplayName.Add(*Fallback, FName(TEXT("Easy")));
		SelectedDifficultyOption = Fallback;
		CurrentDifficultyId = FName(TEXT("Easy"));
	}
}

void ST66MiniLeaderboardPanel::RefreshLeaderboard()
{
	if (!LeaderboardSubsystem || CurrentDifficultyId.IsNone())
	{
		CachedEntries.Reset();
		CachedStatusMessage = TEXT("Mini leaderboard data is unavailable.");
		RebuildEntryList();
		return;
	}

	if (!LeaderboardSubsystem->HasCachedLeaderboard(CurrentDifficultyId, CurrentPartySize, CurrentFilter))
	{
		LeaderboardSubsystem->RequestLeaderboard(CurrentDifficultyId, CurrentPartySize, CurrentFilter);
	}

	CurrentCacheKey = UT66MiniLeaderboardSubsystem::MakeCacheKey(CurrentDifficultyId, CurrentPartySize, CurrentFilter);
	CachedEntries = LeaderboardSubsystem->GetCachedLeaderboard(CurrentDifficultyId, CurrentPartySize, CurrentFilter);
	CachedStatusMessage = LeaderboardSubsystem->GetLeaderboardStatusMessage(CurrentDifficultyId, CurrentPartySize, CurrentFilter);
	RebuildEntryList();
}

void ST66MiniLeaderboardPanel::RebuildEntryList()
{
	if (!EntryListBox.IsValid())
	{
		return;
	}

	EntryListBox->ClearChildren();

	if (CachedEntries.Num() == 0)
	{
		EntryListBox->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(CachedStatusMessage))
			.Font(T66MiniUI::BodyFont(14))
			.ColorAndOpacity(T66MiniUI::MutedText())
			.AutoWrapText(true)
		];
	}
	else
	{
		for (const FT66MiniLeaderboardEntry& Entry : CachedEntries)
		{
			const FLinearColor RowAccent = Entry.bIsLocalPlayer ? T66MiniUI::AccentGreen() : T66MiniUI::RaisedFill();
			const FString DisplayLabel = Entry.bIsLocalPlayer
				? FString::Printf(TEXT("%s  •  YOU"), *Entry.DisplayName)
				: Entry.DisplayName;

			EntryListBox->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SBorder)
				.BorderImage(T66MiniUI::WhiteBrush())
				.BorderBackgroundColor(RowAccent)
				.Padding(FMargin(1.f))
				[
					SNew(SBorder)
					.BorderImage(T66MiniUI::WhiteBrush())
					.BorderBackgroundColor(T66MiniUI::CardFill())
					.Padding(FMargin(10.f, 8.f))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(FString::Printf(TEXT("#%d"), Entry.Rank)))
							.Font(T66MiniUI::BoldFont(15))
							.ColorAndOpacity(FLinearColor::White)
						]
						+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(12.f, 0.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(DisplayLabel))
							.Font(T66MiniUI::BodyFont(14))
							.ColorAndOpacity(Entry.bIsLocalPlayer ? T66MiniUI::AccentGreen() : FLinearColor::White)
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::AsNumber(Entry.Score))
							.Font(T66MiniUI::BoldFont(16))
							.ColorAndOpacity(T66MiniUI::AccentGold())
						]
					]
				]
			];
		}
	}

	if (StatusTextBlock.IsValid())
	{
		StatusTextBlock->SetText(FText::FromString(
			CachedStatusMessage.IsEmpty()
				? TEXT("All-time score is based on materials banked.")
				: CachedStatusMessage));
	}
}

void ST66MiniLeaderboardPanel::OnMiniLeaderboardUpdated(const FString& CacheKey)
{
	if (CacheKey == CurrentCacheKey)
	{
		RefreshLeaderboard();
	}
}

FReply ST66MiniLeaderboardPanel::HandleGlobalClicked()
{
	CurrentFilter = ET66MiniLeaderboardFilter::Global;
	Invalidate(EInvalidateWidgetReason::Paint);
	RefreshLeaderboard();
	return FReply::Handled();
}

FReply ST66MiniLeaderboardPanel::HandleFriendsClicked()
{
	CurrentFilter = ET66MiniLeaderboardFilter::Friends;
	Invalidate(EInvalidateWidgetReason::Paint);
	RefreshLeaderboard();
	return FReply::Handled();
}

void ST66MiniLeaderboardPanel::OnPartySizeChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (!NewSelection.IsValid())
	{
		return;
	}

	SelectedPartySizeOption = NewSelection;
	CurrentPartySize = PartySizeFromLabel(*NewSelection);
	Invalidate(EInvalidateWidgetReason::Paint);
	RefreshLeaderboard();
}

void ST66MiniLeaderboardPanel::OnDifficultyChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (!NewSelection.IsValid())
	{
		return;
	}

	SelectedDifficultyOption = NewSelection;
	CurrentDifficultyId = DifficultyIdsByDisplayName.FindRef(*NewSelection);
	Invalidate(EInvalidateWidgetReason::Paint);
	RefreshLeaderboard();
}

FString ST66MiniLeaderboardPanel::PartySizeLabel(const ET66PartySize PartySize)
{
	switch (PartySize)
	{
	case ET66PartySize::Duo:
		return TEXT("Duo");
	case ET66PartySize::Trio:
		return TEXT("Trio");
	case ET66PartySize::Quad:
		return TEXT("Quad");
	case ET66PartySize::Solo:
	default:
		return TEXT("Solo");
	}
}

ET66PartySize ST66MiniLeaderboardPanel::PartySizeFromLabel(const FString& Label)
{
	if (Label.Equals(TEXT("Duo"), ESearchCase::IgnoreCase))
	{
		return ET66PartySize::Duo;
	}
	if (Label.Equals(TEXT("Trio"), ESearchCase::IgnoreCase))
	{
		return ET66PartySize::Trio;
	}
	if (Label.Equals(TEXT("Quad"), ESearchCase::IgnoreCase))
	{
		return ET66PartySize::Quad;
	}
	return ET66PartySize::Solo;
}
