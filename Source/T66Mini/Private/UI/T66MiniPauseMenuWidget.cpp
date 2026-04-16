// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66MiniPauseMenuWidget.h"

#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniRunStateSubsystem.h"
#include "Data/T66MiniDataTypes.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerInput.h"
#include "Gameplay/T66MiniGameMode.h"
#include "Gameplay/T66MiniGameState.h"
#include "Gameplay/T66MiniPlayerController.h"
#include "Gameplay/T66MiniPlayerPawn.h"
#include "Save/T66MiniRunSaveGame.h"
#include "Styling/CoreStyle.h"
#include "UI/T66MiniUIStyle.h"
#include "GenericPlatform/GenericApplication.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FName MiniPauseAction(TEXT("MiniPause"));
	const FName MiniInteractAction(TEXT("MiniInteract"));
	const FName MiniUltimateAction(TEXT("MiniUltimate"));
	constexpr float FpsCaps[] = { 30.f, 60.f, 90.f, 120.f, 0.f };

	FString T66MiniPauseDescribeItem(const FT66MiniItemDefinition& ItemDefinition)
	{
		if (!ItemDefinition.SecondaryStatType.IsEmpty())
		{
			return ItemDefinition.SecondaryStatType;
		}

		return ItemDefinition.PrimaryStatType.IsEmpty() ? ItemDefinition.ItemID.ToString() : ItemDefinition.PrimaryStatType;
	}

	FText WindowModeToText(const EWindowMode::Type Mode)
	{
		switch (Mode)
		{
		case EWindowMode::Windowed:
			return NSLOCTEXT("T66Mini.Pause", "WindowModeWindowed", "Windowed");
		case EWindowMode::Fullscreen:
			return NSLOCTEXT("T66Mini.Pause", "WindowModeFullscreen", "Fullscreen");
		case EWindowMode::WindowedFullscreen:
		default:
			return NSLOCTEXT("T66Mini.Pause", "WindowModeBorderless", "Borderless");
		}
	}

	FText QualityToText(const int32 QualityNotch)
	{
		switch (FMath::Clamp(QualityNotch, 0, 3))
		{
		case 0:
			return NSLOCTEXT("T66Mini.Pause", "QualityLow", "Low");
		case 1:
			return NSLOCTEXT("T66Mini.Pause", "QualityMedium", "Medium");
		case 2:
			return NSLOCTEXT("T66Mini.Pause", "QualityHigh", "High");
		case 3:
		default:
			return NSLOCTEXT("T66Mini.Pause", "QualityEpic", "Epic");
		}
	}

	FText FpsCapToText(const int32 FpsCapIndex)
	{
		if (FMath::Clamp(FpsCapIndex, 0, 4) == 4)
		{
			return NSLOCTEXT("T66Mini.Pause", "FpsUnlimited", "Unlimited");
		}

		return FText::AsNumber(static_cast<int32>(FpsCaps[FMath::Clamp(FpsCapIndex, 0, 4)]));
	}

	FText DisplayModeToText(const int32 DisplayModeIndex)
	{
		return DisplayModeIndex == 1
			? NSLOCTEXT("T66Mini.Pause", "DisplayModeWidescreen", "Widescreen")
			: NSLOCTEXT("T66Mini.Pause", "DisplayModeStandard", "Standard");
	}

	FText MonitorIndexToText(const int32 MonitorIndex)
	{
		return MonitorIndex == 0
			? NSLOCTEXT("T66Mini.Pause", "PrimaryMonitor", "Primary Monitor")
			: FText::Format(NSLOCTEXT("T66Mini.Pause", "MonitorN", "Monitor {0}"), FText::AsNumber(MonitorIndex + 1));
	}

	int32 GetMonitorCount()
	{
		FDisplayMetrics Metrics;
		FSlateApplication::Get().GetCachedDisplayMetrics(Metrics);
		return FMath::Max(1, Metrics.MonitorInfo.Num());
	}

	FText BoolToText(const bool bEnabled)
	{
		return bEnabled
			? NSLOCTEXT("T66Mini.Pause", "Enabled", "ON")
			: NSLOCTEXT("T66Mini.Pause", "Disabled", "OFF");
	}
}

UT66MiniPauseMenuWidget::UT66MiniPauseMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

TSharedRef<SWidget> UT66MiniPauseMenuWidget::RebuildWidget()
{
	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	const UT66MiniRunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PlayerSettings = GameInstance ? GameInstance->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	const UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	const AT66MiniPlayerPawn* PlayerPawn = Cast<AT66MiniPlayerPawn>(GetOwningPlayer() ? GetOwningPlayer()->GetPawn() : nullptr);
	const AT66MiniGameState* MiniGameState = GetWorld() ? GetWorld()->GetGameState<AT66MiniGameState>() : nullptr;

	InitializeGraphicsState();

	TSharedRef<SVerticalBox> OwnedItemsPanel = SNew(SVerticalBox);
	OwnedItemsPanel->AddSlot()
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(NSLOCTEXT("T66Mini.Pause", "InventoryPlaceholder", "Inventory"))
		.Font(T66MiniUI::BodyFont(12))
		.ColorAndOpacity(T66MiniUI::MutedText())
	];

	if (ActiveRun && DataSubsystem)
	{
		TArray<TPair<FName, int32>> OwnedItems;
		TMap<FName, int32> OwnedItemLookup;
		for (const FName ItemID : ActiveRun->OwnedItemIDs)
		{
			if (ItemID.IsNone())
			{
				continue;
			}

			if (const int32* ExistingIndex = OwnedItemLookup.Find(ItemID))
			{
				++OwnedItems[*ExistingIndex].Value;
				continue;
			}

			OwnedItemLookup.Add(ItemID, OwnedItems.Num());
			OwnedItems.Emplace(ItemID, 1);
		}

		OwnedItemsPanel = SNew(SVerticalBox);
		const int32 MaxVisibleOwnedItems = 12;
		for (int32 Index = 0; Index < OwnedItems.Num() && Index < MaxVisibleOwnedItems; ++Index)
		{
			const TPair<FName, int32>& Entry = OwnedItems[Index];
			const FT66MiniItemDefinition* ItemDefinition = DataSubsystem->FindItem(Entry.Key);
			const FString ItemLabel = ItemDefinition
				? FString::Printf(TEXT("%s  x%d   |   %s"), *Entry.Key.ToString(), Entry.Value, *T66MiniPauseDescribeItem(*ItemDefinition))
				: FString::Printf(TEXT("%s  x%d"), *Entry.Key.ToString(), Entry.Value);

			OwnedItemsPanel->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(ItemLabel))
				.Font(T66MiniUI::BodyFont(14))
				.ColorAndOpacity(FLinearColor::White)
				.AutoWrapText(true)
			];
		}

		if (OwnedItems.Num() == 0)
		{
			OwnedItemsPanel->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66Mini.Pause", "NoItems", "No item stacks yet. Loot bags and crates will fill this list during the run."))
				.Font(T66MiniUI::BodyFont(14))
				.ColorAndOpacity(T66MiniUI::MutedText())
				.AutoWrapText(true)
			];
		}
		else if (OwnedItems.Num() > MaxVisibleOwnedItems)
		{
			OwnedItemsPanel->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("+%d more stacks"), OwnedItems.Num() - MaxVisibleOwnedItems)))
				.Font(T66MiniUI::BodyFont(13))
				.ColorAndOpacity(T66MiniUI::MutedText())
			];
		}
	}

	const FString HeroLabel = PlayerPawn
		? (PlayerPawn->GetHeroDisplayName().IsEmpty() ? PlayerPawn->GetHeroID().ToString() : PlayerPawn->GetHeroDisplayName())
		: FString(TEXT("Unknown"));
	const FString DifficultyLabel = ActiveRun ? ActiveRun->DifficultyID.ToString() : FString(TEXT("Unknown"));
	const FString WaveLabel = MiniGameState ? FString::Printf(TEXT("Wave %d / 5"), MiniGameState->WaveIndex) : FString(TEXT("Wave ?"));
	const FString HealthLabel = PlayerPawn ? FString::Printf(TEXT("%.0f / %.0f"), PlayerPawn->GetCurrentHealth(), PlayerPawn->GetMaxHealth()) : FString(TEXT("Unavailable"));
	const FString EconomyLabel = PlayerPawn ? FString::Printf(TEXT("%d Gold   %d Materials   Lv %d"), PlayerPawn->GetGold(), PlayerPawn->GetMaterials(), PlayerPawn->GetHeroLevel()) : FString(TEXT("Unavailable"));
	const FString TimerLabel = MiniGameState
		? FString::Printf(TEXT("%d:%02d"), FMath::Max(0, FMath::CeilToInt(MiniGameState->WaveSecondsRemaining)) / 60, FMath::Max(0, FMath::CeilToInt(MiniGameState->WaveSecondsRemaining)) % 60)
		: FString(TEXT("0:00"));
	const FString CombatLineOne = PlayerPawn ? FString::Printf(TEXT("DMG %.1f   ATK %.1f"), PlayerPawn->GetDamageStat(), PlayerPawn->GetAttackSpeedStat()) : FString(TEXT("DMG ?   ATK ?"));
	const FString CombatLineTwo = PlayerPawn ? FString::Printf(TEXT("ARM %.1f   CRIT %.0f%%"), PlayerPawn->GetArmorStat(), PlayerPawn->GetCritChance() * 100.f) : FString(TEXT("ARM ?   CRIT ?"));
	const FString CombatLineThree = PlayerPawn ? FString::Printf(TEXT("REGEN %.1f   EVADE %.0f%%"), PlayerPawn->GetPassiveRegenPerSecond(), PlayerPawn->GetEvasionChance() * 100.f) : FString(TEXT("REGEN ?   EVADE ?"));
	const FString CombatLineFour = PlayerPawn ? FString::Printf(TEXT("RANGE %.0f   LIFE STEAL %.0f%%"), PlayerPawn->GetAttackRange(), PlayerPawn->GetLifeStealChance() * 100.f) : FString(TEXT("RANGE ?   LIFE STEAL ?"));

	const auto MakeSurface = [](const TSharedRef<SWidget>& Content, const FLinearColor& Accent, const FMargin SurfacePadding = FMargin(16.f)) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.44f))
			.Padding(1.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBorder)
					.BorderImage(T66MiniUI::WhiteBrush())
					.BorderBackgroundColor(Accent)
					.Padding(FMargin(0.f))
					[
						SNew(SBox).HeightOverride(4.f)
					]
				]
				+ SVerticalBox::Slot().FillHeight(1.f)
				[
					SNew(SBorder)
					.BorderImage(T66MiniUI::WhiteBrush())
					.BorderBackgroundColor(T66MiniUI::CardFill())
					.Padding(SurfacePadding)
					[
						Content
					]
				]
			];
	};

	const auto MakeSectionHeading = [](const FText& Title, const FText& Subtitle = FText::GetEmpty()) -> TSharedRef<SWidget>
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock).Text(Title).Font(T66MiniUI::BoldFont(18)).ColorAndOpacity(FLinearColor::White)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Visibility(Subtitle.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
				.Text(Subtitle)
				.Font(T66MiniUI::BodyFont(13))
				.ColorAndOpacity(T66MiniUI::MutedText())
				.AutoWrapText(true)
			];
	};

	const auto MakeMetricLine = [](const FText& Label, const FString& Value, const FLinearColor& ValueColor = FLinearColor::White) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(STextBlock).Text(Label).Font(T66MiniUI::BodyFont(13)).ColorAndOpacity(T66MiniUI::MutedText())
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(STextBlock).Text(FText::FromString(Value)).Font(T66MiniUI::BoldFont(14)).ColorAndOpacity(ValueColor)
			];
	};

	const auto MakeActionButton = [](const FText& Label, const FOnClicked& OnClicked, const FLinearColor& Fill, const FLinearColor& TextColor, const float Height = 54.f) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.HeightOverride(Height)
			[
				SNew(SButton)
				.OnClicked(OnClicked)
				.ButtonColorAndOpacity(Fill)
				.ContentPadding(FMargin(10.f, 8.f))
				[
					SNew(STextBlock).Text(Label).Font(T66MiniUI::BoldFont(18)).ColorAndOpacity(TextColor).Justification(ETextJustify::Center)
				]
			];
	};

	const auto MakeSliderRow = [this](const FText& Label, const FText& Description, TFunction<float()> GetValue, TFunction<void(float)> SetValue) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(T66MiniUI::RaisedFill())
			.Padding(FMargin(12.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(STextBlock).Text(Label).Font(T66MiniUI::BoldFont(14)).ColorAndOpacity(FLinearColor::White)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock)
						.Text_Lambda([GetValue]() { return FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(FMath::Clamp(GetValue(), 0.f, 1.f) * 100.f))); })
						.Font(T66MiniUI::BodyFont(13))
						.ColorAndOpacity(T66MiniUI::MutedText())
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 8.f)
				[
					SNew(STextBlock).Text(Description).Font(T66MiniUI::BodyFont(12)).ColorAndOpacity(T66MiniUI::MutedText()).AutoWrapText(true)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SSlider)
					.Value_Lambda([GetValue]() { return FMath::Clamp(GetValue(), 0.f, 1.f); })
					.OnValueChanged_Lambda([SetValue](const float NewValue) { SetValue(FMath::Clamp(NewValue, 0.f, 1.f)); })
				]
			];
	};

	const auto MakeToggleRow = [this](const FText& Label, const FText& Description, TFunction<bool()> GetValue, TFunction<void(bool)> SetValue) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(T66MiniUI::RaisedFill())
			.Padding(FMargin(12.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock).Text(Label).Font(T66MiniUI::BoldFont(14)).ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 14.f, 0.f)
					[
						SNew(STextBlock).Text(Description).Font(T66MiniUI::BodyFont(12)).ColorAndOpacity(T66MiniUI::MutedText()).AutoWrapText(true)
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SButton)
					.OnClicked_Lambda([GetValue, SetValue]()
					{
						SetValue(!GetValue());
						return FReply::Handled();
					})
					.ButtonColorAndOpacity_Lambda([GetValue]() { return GetValue() ? T66MiniUI::AccentGreen() : T66MiniUI::AccentBlue(); })
					.ContentPadding(FMargin(14.f, 8.f))
					[
						SNew(STextBlock)
						.Text_Lambda([GetValue]() { return BoolToText(GetValue()); })
						.Font(T66MiniUI::BoldFont(13))
						.ColorAndOpacity_Lambda([GetValue]() { return GetValue() ? T66MiniUI::ButtonTextDark() : FLinearColor::White; })
					]
				]
			];
	};

	const auto MakeCycleRow = [this](const FText& Label, const FText& Description, TFunction<FText()> GetValueText, TFunction<void(int32)> AdjustValue) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(T66MiniUI::RaisedFill())
			.Padding(FMargin(12.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock).Text(Label).Font(T66MiniUI::BoldFont(14)).ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 12.f, 0.f)
					[
						SNew(STextBlock).Text(Description).Font(T66MiniUI::BodyFont(12)).ColorAndOpacity(T66MiniUI::MutedText()).AutoWrapText(true)
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SButton).OnClicked_Lambda([AdjustValue]() { AdjustValue(-1); return FReply::Handled(); }).ButtonColorAndOpacity(T66MiniUI::AccentBlue()).ContentPadding(FMargin(10.f, 6.f))[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Pause", "PrevValue", "<")).Font(T66MiniUI::BoldFont(16)).ColorAndOpacity(FLinearColor::White)]
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f)
					[
						SNew(SBorder).BorderImage(T66MiniUI::WhiteBrush()).BorderBackgroundColor(FLinearColor(0.02f, 0.03f, 0.05f, 0.90f)).Padding(FMargin(14.f, 8.f))
						[
							SNew(STextBlock).Text_Lambda([GetValueText]() { return GetValueText(); }).Font(T66MiniUI::BoldFont(13)).ColorAndOpacity(FLinearColor::White)
						]
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SButton).OnClicked_Lambda([AdjustValue]() { AdjustValue(1); return FReply::Handled(); }).ButtonColorAndOpacity(T66MiniUI::AccentBlue()).ContentPadding(FMargin(10.f, 6.f))[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Pause", "NextValue", ">")).Font(T66MiniUI::BoldFont(16)).ColorAndOpacity(FLinearColor::White)]
					]
				]
			];
	};

	const auto MakeRebindRow = [this](const FText& Label, const FText& Description, const FName ActionName, const bool bAllowMouseButtons) -> TSharedRef<SWidget>
	{
		TSharedPtr<STextBlock> KeyText;

		TSharedRef<SWidget> Row =
			SNew(SBorder)
			.BorderImage(T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(T66MiniUI::RaisedFill())
			.Padding(FMargin(12.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(Label).Font(T66MiniUI::BoldFont(14)).ColorAndOpacity(FLinearColor::White)]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 14.f, 0.f)[SNew(STextBlock).Text(Description).Font(T66MiniUI::BodyFont(12)).ColorAndOpacity(T66MiniUI::MutedText()).AutoWrapText(true)]
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
					[
						SNew(SBorder).BorderImage(T66MiniUI::WhiteBrush()).BorderBackgroundColor(FLinearColor(0.02f, 0.03f, 0.05f, 0.90f)).Padding(FMargin(14.f, 8.f))
						[
							SAssignNew(KeyText, STextBlock).Text(KeyToText(FindActionKey(ActionName))).Font(T66MiniUI::BoldFont(13)).ColorAndOpacity(FLinearColor::White)
						]
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SButton).OnClicked(FOnClicked::CreateUObject(this, &UT66MiniPauseMenuWidget::HandleStartRebind, ActionName, bAllowMouseButtons)).ButtonColorAndOpacity(T66MiniUI::AccentGold()).ContentPadding(FMargin(12.f, 8.f))[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Pause", "Rebind", "REBIND")).Font(T66MiniUI::BoldFont(13)).ColorAndOpacity(T66MiniUI::ButtonTextDark())]
					]
				]
			];

		BindingTextMap.Add(ActionName, KeyText);
		return Row;
	};

	TSharedRef<SWidget> AudioTab = PlayerSettings
		? StaticCastSharedRef<SWidget>(SNew(SScrollBox) + SScrollBox::Slot()[SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeSliderRow(NSLOCTEXT("T66Mini.Pause", "MasterVolume", "Master Volume"), NSLOCTEXT("T66Mini.Pause", "MasterVolumeBody", "Global volume for the Mini run and the frontend shell."), [PlayerSettings]() { return PlayerSettings->GetMasterVolume(); }, [this, PlayerSettings](const float Value) { PlayerSettings->SetMasterVolume(Value); if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr) { MiniGameMode->RefreshAudioMix(); } })]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeSliderRow(NSLOCTEXT("T66Mini.Pause", "MusicVolume", "Music Volume"), NSLOCTEXT("T66Mini.Pause", "MusicVolumeBody", "Controls the battle music and intermission soundtrack."), [PlayerSettings]() { return PlayerSettings->GetMusicVolume(); }, [this, PlayerSettings](const float Value) { PlayerSettings->SetMusicVolume(Value); if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr) { MiniGameMode->RefreshAudioMix(); } })]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeSliderRow(NSLOCTEXT("T66Mini.Pause", "SfxVolume", "SFX Volume"), NSLOCTEXT("T66Mini.Pause", "SfxVolumeBody", "Controls combat hits, pickups, traps, and UI sound effects."), [PlayerSettings]() { return PlayerSettings->GetSfxVolume(); }, [this, PlayerSettings](const float Value) { PlayerSettings->SetSfxVolume(Value); if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr) { MiniGameMode->RefreshAudioMix(); } })]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeToggleRow(NSLOCTEXT("T66Mini.Pause", "MuteWhenUnfocused", "Mute When Unfocused"), NSLOCTEXT("T66Mini.Pause", "MuteWhenUnfocusedBody", "Drops game audio when the window loses focus."), [PlayerSettings]() { return PlayerSettings->GetMuteWhenUnfocused(); }, [PlayerSettings](const bool bEnabled) { PlayerSettings->SetMuteWhenUnfocused(bEnabled); })]
			+ SVerticalBox::Slot().AutoHeight()[MakeToggleRow(NSLOCTEXT("T66Mini.Pause", "Subtitles", "Subtitles"), NSLOCTEXT("T66Mini.Pause", "SubtitlesBody", "Keeps subtitle support enabled whenever a captioned event is available."), [PlayerSettings]() { return PlayerSettings->GetSubtitlesAlwaysOn(); }, [PlayerSettings](const bool bEnabled) { PlayerSettings->SetSubtitlesAlwaysOn(bEnabled); })]])
		: StaticCastSharedRef<SWidget>(SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Pause", "AudioUnavailable", "Audio settings are unavailable in this session.")).Font(T66MiniUI::BodyFont(14)).ColorAndOpacity(T66MiniUI::MutedText()));

	TSharedRef<SWidget> KeyBindingTab = SNew(SScrollBox) + SScrollBox::Slot()[SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Pause", "KeyBindingIntro", "Mini uses mouse-follow movement, so only the core battle bindings are exposed here.")).Font(T66MiniUI::BodyFont(13)).ColorAndOpacity(T66MiniUI::MutedText()).AutoWrapText(true)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeRebindRow(NSLOCTEXT("T66Mini.Pause", "PauseAction", "Pause / Back"), NSLOCTEXT("T66Mini.Pause", "PauseActionBody", "Used to open the pause menu and close the settings modal."), MiniPauseAction, false)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeRebindRow(NSLOCTEXT("T66Mini.Pause", "InteractAction", "Interact"), NSLOCTEXT("T66Mini.Pause", "InteractActionBody", "Triggers manual interactions like the quick revive machine when you are in range."), MiniInteractAction, true)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[MakeRebindRow(NSLOCTEXT("T66Mini.Pause", "UltimateAction", "Ultimate"), NSLOCTEXT("T66Mini.Pause", "UltimateActionBody", "Activates the equipped hero ultimate toward the current cursor target."), MiniUltimateAction, true)]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)[MakeActionButton(NSLOCTEXT("T66Mini.Pause", "RestoreMiniDefaults", "RESTORE MINI DEFAULTS"), FOnClicked::CreateUObject(this, &UT66MiniPauseMenuWidget::HandleResetBindingsClicked), T66MiniUI::AccentBlue(), FLinearColor::White, 42.f)]];

	TSharedRef<SWidget> GraphicsTab = PlayerSettings
		? StaticCastSharedRef<SWidget>(SNew(SScrollBox) + SScrollBox::Slot()[SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeCycleRow(NSLOCTEXT("T66Mini.Pause", "GraphicsWindowMode", "Window Mode"), NSLOCTEXT("T66Mini.Pause", "GraphicsWindowModeBody", "Switch between windowed, fullscreen, and borderless presentation."), [this]() { return WindowModeToText(PendingGraphics.WindowMode); }, [this](const int32 Step) { const TArray<EWindowMode::Type> Modes = { EWindowMode::Windowed, EWindowMode::Fullscreen, EWindowMode::WindowedFullscreen }; int32 Index = Modes.IndexOfByKey(PendingGraphics.WindowMode); PendingGraphics.WindowMode = Modes[(Index + Step + Modes.Num()) % Modes.Num()]; PendingGraphics.bDirty = true; })]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeCycleRow(NSLOCTEXT("T66Mini.Pause", "GraphicsQuality", "Overall Quality"), NSLOCTEXT("T66Mini.Pause", "GraphicsQualityBody", "Changes the global scalability notch used by the engine."), [this]() { return QualityToText(PendingGraphics.QualityNotch); }, [this](const int32 Step) { PendingGraphics.QualityNotch = FMath::Clamp(PendingGraphics.QualityNotch + Step, 0, 3); PendingGraphics.bDirty = true; })]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeCycleRow(NSLOCTEXT("T66Mini.Pause", "GraphicsFpsCap", "FPS Cap"), NSLOCTEXT("T66Mini.Pause", "GraphicsFpsCapBody", "Caps the render framerate for the current game window."), [this]() { return FpsCapToText(PendingGraphics.FpsCapIndex); }, [this](const int32 Step) { PendingGraphics.FpsCapIndex = (PendingGraphics.FpsCapIndex + Step + 5) % 5; PendingGraphics.bDirty = true; })]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeCycleRow(NSLOCTEXT("T66Mini.Pause", "GraphicsDisplayMode", "Display Mode"), NSLOCTEXT("T66Mini.Pause", "GraphicsDisplayModeBody", "Stores the standard or widescreen preference used by Mini and frontend UI."), [this]() { return DisplayModeToText(PendingGraphics.DisplayModeIndex); }, [this](const int32 Step) { PendingGraphics.DisplayModeIndex = (PendingGraphics.DisplayModeIndex + Step + 2) % 2; PendingGraphics.bDirty = true; })]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[MakeCycleRow(NSLOCTEXT("T66Mini.Pause", "GraphicsMonitor", "Monitor"), NSLOCTEXT("T66Mini.Pause", "GraphicsMonitorBody", "Moves the game window to the selected monitor when more than one display is available."), [this]() { return MonitorIndexToText(PendingGraphics.MonitorIndex); }, [this](const int32 Step) { const int32 MonitorCount = GetMonitorCount(); PendingGraphics.MonitorIndex = (PendingGraphics.MonitorIndex + Step + MonitorCount) % MonitorCount; PendingGraphics.bDirty = true; })]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[MakeToggleRow(NSLOCTEXT("T66Mini.Pause", "GraphicsFog", "Native Fog"), NSLOCTEXT("T66Mini.Pause", "GraphicsFogBody", "Keeps the existing gameplay fog toggle exposed from shared settings."), [PlayerSettings]() { return PlayerSettings->GetFogEnabled(); }, [PlayerSettings](const bool bEnabled) { PlayerSettings->SetFogEnabled(bEnabled); })]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)[MakeActionButton(NSLOCTEXT("T66Mini.Pause", "ApplyGraphics", "APPLY GRAPHICS"), FOnClicked::CreateUObject(this, &UT66MiniPauseMenuWidget::HandleApplyGraphicsClicked), T66MiniUI::AccentGreen(), T66MiniUI::ButtonTextDark(), 42.f)]])
		: StaticCastSharedRef<SWidget>(SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Pause", "GraphicsUnavailable", "Graphics settings are unavailable in this session.")).Font(T66MiniUI::BodyFont(14)).ColorAndOpacity(T66MiniUI::MutedText()));

	TSharedRef<SWidget> GameplayTab = PlayerSettings
		? StaticCastSharedRef<SWidget>(SNew(SScrollBox) + SScrollBox::Slot()[SNew(SVerticalBox)+SVerticalBox::Slot().AutoHeight()[MakeToggleRow(NSLOCTEXT("T66Mini.Pause", "ShowDamageNumbers", "Show Damage Numbers"), NSLOCTEXT("T66Mini.Pause", "ShowDamageNumbersBody", "Controls whether floating damage numbers render over heroes and enemies during combat."), [PlayerSettings]() { return PlayerSettings->GetShowDamageNumbers(); }, [PlayerSettings](const bool bEnabled) { PlayerSettings->SetShowDamageNumbers(bEnabled); })]])
		: StaticCastSharedRef<SWidget>(SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Pause", "GameplayUnavailable", "Gameplay settings are unavailable in this session.")).Font(T66MiniUI::BodyFont(14)).ColorAndOpacity(T66MiniUI::MutedText()));
	const auto MakeSettingsTabButton = [this](const FText& Label, const ET66MiniPauseSettingsTab Tab) -> TSharedRef<SWidget>
	{
		return SNew(SButton)
			.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniPauseMenuWidget::HandleSettingsTabClicked, Tab))
			.ButtonColorAndOpacity_Lambda([this, Tab]() { return CurrentSettingsTab == Tab ? T66MiniUI::AccentGold() : T66MiniUI::RaisedFill(); })
			.ContentPadding(FMargin(12.f, 10.f))
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(T66MiniUI::BoldFont(13))
				.ColorAndOpacity_Lambda([this, Tab]() { return CurrentSettingsTab == Tab ? T66MiniUI::ButtonTextDark() : FLinearColor::White; })
			];
	};

	TSharedRef<SWidget> SettingsModal =
		SNew(SOverlay)
		.Visibility_Lambda([this]() { return bSettingsModalOpen ? EVisibility::Visible : EVisibility::Collapsed; })
		+ SOverlay::Slot()
		[
			SNew(SBorder).BorderImage(T66MiniUI::WhiteBrush()).BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.78f))
		]
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(980.f)
			.HeightOverride(640.f)
			[
				MakeSurface(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f)
						[
							MakeSectionHeading(NSLOCTEXT("T66Mini.Pause", "SettingsTitle", "SETTINGS"), NSLOCTEXT("T66Mini.Pause", "SettingsSubtitle", "Audio, key binding, graphics, and gameplay controls for the Mini run."))
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)
						[
							MakeActionButton(NSLOCTEXT("T66Mini.Pause", "CloseSettings", "X"), FOnClicked::CreateUObject(this, &UT66MiniPauseMenuWidget::HandleCloseSettingsClicked), FLinearColor(0.76f, 0.28f, 0.30f, 1.f), T66MiniUI::ButtonTextDark(), 38.f)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 8.f, 0.f)[MakeSettingsTabButton(NSLOCTEXT("T66Mini.Pause", "AudioTab", "AUDIO"), ET66MiniPauseSettingsTab::Audio)]
						+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 8.f, 0.f)[MakeSettingsTabButton(NSLOCTEXT("T66Mini.Pause", "KeyBindingTab", "KEY BINDING"), ET66MiniPauseSettingsTab::KeyBinding)]
						+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 8.f, 0.f)[MakeSettingsTabButton(NSLOCTEXT("T66Mini.Pause", "GraphicsTab", "GRAPHICS"), ET66MiniPauseSettingsTab::Graphics)]
						+ SHorizontalBox::Slot().FillWidth(1.f)[MakeSettingsTabButton(NSLOCTEXT("T66Mini.Pause", "GameplayTab", "GAMEPLAY"), ET66MiniPauseSettingsTab::Gameplay)]
					]
					+ SVerticalBox::Slot().FillHeight(1.f)
					[
						SAssignNew(SettingsTabSwitcher, SWidgetSwitcher)
						.WidgetIndex(static_cast<int32>(CurrentSettingsTab))
						+ SWidgetSwitcher::Slot()[AudioTab]
						+ SWidgetSwitcher::Slot()[KeyBindingTab]
						+ SWidgetSwitcher::Slot()[GraphicsTab]
						+ SWidgetSwitcher::Slot()[GameplayTab]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
					[
						SNew(SBorder)
						.BorderImage(T66MiniUI::WhiteBrush())
						.BorderBackgroundColor(FLinearColor(0.02f, 0.03f, 0.05f, 0.90f))
						.Padding(FMargin(12.f))
						[
							SAssignNew(SettingsStatusText, STextBlock)
							.Text(NSLOCTEXT("T66Mini.Pause", "SettingsHint", "Press the current pause binding again to close this modal."))
							.Font(T66MiniUI::BodyFont(13))
							.ColorAndOpacity(T66MiniUI::MutedText())
							.AutoWrapText(true)
						]
					],
					T66MiniUI::AccentGold(),
					FMargin(20.f))
			]
		];

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBorder).BorderImage(T66MiniUI::WhiteBrush()).BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.76f))
		]
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(1180.f)
			.HeightOverride(720.f)
			[
				SNew(SBorder)
				.BorderImage(T66MiniUI::WhiteBrush())
				.BorderBackgroundColor(T66MiniUI::ShellFill())
				.Padding(FMargin(22.f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 18.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(270.f)
						[
							MakeSurface(
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 4.f)[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Pause", "PausedTitle", "PAUSED")).Font(T66MiniUI::TitleFont(34)).ColorAndOpacity(FLinearColor::White)]
								+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 18.f)[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Pause", "PausedSubtitle", "Mini run pause menu")).Font(T66MiniUI::BodyFont(15)).ColorAndOpacity(T66MiniUI::MutedText())]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeActionButton(NSLOCTEXT("T66Mini.Pause", "Resume", "RESUME"), FOnClicked::CreateUObject(this, &UT66MiniPauseMenuWidget::HandleResumeClicked), T66MiniUI::AccentGreen(), T66MiniUI::ButtonTextDark())]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeActionButton(NSLOCTEXT("T66Mini.Pause", "Settings", "SETTINGS"), FOnClicked::CreateUObject(this, &UT66MiniPauseMenuWidget::HandleSettingsClicked), T66MiniUI::AccentBlue(), FLinearColor::White)]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 18.f)[MakeActionButton(NSLOCTEXT("T66Mini.Pause", "SaveQuit", "SAVE AND QUIT"), FOnClicked::CreateUObject(this, &UT66MiniPauseMenuWidget::HandleSaveAndQuitClicked), FLinearColor(0.78f, 0.34f, 0.30f, 1.f), FLinearColor::White)]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)[MakeSectionHeading(NSLOCTEXT("T66Mini.Pause", "PauseHintTitle", "PAUSE FLOW"), NSLOCTEXT("T66Mini.Pause", "PauseHintBody", "Save and Quit stores the run snapshot, returns to the Mini main menu, and lets you continue later from the Mini save slots."))]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeMetricLine(NSLOCTEXT("T66Mini.Pause", "PauseHintCurrentWave", "Current Wave"), WaveLabel, T66MiniUI::AccentGold())]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeMetricLine(NSLOCTEXT("T66Mini.Pause", "PauseHintCurrentTimer", "Current Timer"), TimerLabel, T66MiniUI::AccentGold())]
								+ SVerticalBox::Slot().FillHeight(1.f).VAlign(VAlign_Bottom)[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Pause", "PauseFooter", "The Mini run auto-saves during battle, but this screen also forces a fresh save snapshot before leaving.")).Font(T66MiniUI::BodyFont(12)).ColorAndOpacity(T66MiniUI::MutedText()).AutoWrapText(true)],
								T66MiniUI::AccentGold())
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f)
						[
							SNew(SUniformGridPanel)
							.SlotPadding(FMargin(0.f, 0.f, 14.f, 0.f))
							+ SUniformGridPanel::Slot(0, 0)[MakeSurface(SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[MakeSectionHeading(NSLOCTEXT("T66Mini.Pause", "RunSnapshot", "RUN SNAPSHOT"), NSLOCTEXT("T66Mini.Pause", "RunSnapshotBody", "Current hero state, economy, and wave timing."))]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeMetricLine(NSLOCTEXT("T66Mini.Pause", "RunHero", "Hero"), HeroLabel)]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeMetricLine(NSLOCTEXT("T66Mini.Pause", "RunDifficulty", "Difficulty"), DifficultyLabel)]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeMetricLine(NSLOCTEXT("T66Mini.Pause", "RunWave", "Wave"), WaveLabel, T66MiniUI::AccentGold())]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeMetricLine(NSLOCTEXT("T66Mini.Pause", "RunTimer", "Time Remaining"), TimerLabel, T66MiniUI::AccentGold())]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeMetricLine(NSLOCTEXT("T66Mini.Pause", "RunHealth", "Health"), HealthLabel, T66MiniUI::AccentGreen())]
								+ SVerticalBox::Slot().AutoHeight()[MakeMetricLine(NSLOCTEXT("T66Mini.Pause", "RunEconomy", "Economy"), EconomyLabel)], T66MiniUI::AccentGold())]
							+ SUniformGridPanel::Slot(1, 0)[MakeSurface(SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[MakeSectionHeading(NSLOCTEXT("T66Mini.Pause", "CombatProfile", "COMBAT PROFILE"), NSLOCTEXT("T66Mini.Pause", "CombatProfileBody", "Live combat stats from the current hero build."))]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[SNew(STextBlock).Text(FText::FromString(CombatLineOne)).Font(T66MiniUI::BodyFont(14)).ColorAndOpacity(FLinearColor::White)]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[SNew(STextBlock).Text(FText::FromString(CombatLineTwo)).Font(T66MiniUI::BodyFont(14)).ColorAndOpacity(FLinearColor::White)]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[SNew(STextBlock).Text(FText::FromString(CombatLineThree)).Font(T66MiniUI::BodyFont(14)).ColorAndOpacity(FLinearColor::White)]
								+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(CombatLineFour)).Font(T66MiniUI::BodyFont(14)).ColorAndOpacity(FLinearColor::White)], T66MiniUI::AccentBlue())]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f)
						[
							MakeSurface(SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[MakeSectionHeading(NSLOCTEXT("T66Mini.Pause", "Systems", "SYSTEMS"), NSLOCTEXT("T66Mini.Pause", "SystemsBody", "Quick state for the hero passive, ultimate, and emergency recovery."))]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeMetricLine(NSLOCTEXT("T66Mini.Pause", "SystemsUltimate", "Ultimate"), PlayerPawn ? PlayerPawn->GetUltimateLabel() : FString(TEXT("Unavailable")), T66MiniUI::AccentGold())]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeMetricLine(NSLOCTEXT("T66Mini.Pause", "SystemsPassive", "Passive"), PlayerPawn ? PlayerPawn->GetPassiveLabel() : FString(TEXT("Unavailable")), T66MiniUI::AccentBlue())]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeMetricLine(NSLOCTEXT("T66Mini.Pause", "SystemsUltState", "Ultimate State"), PlayerPawn ? (PlayerPawn->IsUltimateReady() ? FString(TEXT("Ready")) : FString::Printf(TEXT("%.1fs"), PlayerPawn->GetUltimateCooldownRemaining())) : FString(TEXT("Unavailable")), PlayerPawn && PlayerPawn->IsUltimateReady() ? T66MiniUI::AccentGreen() : FLinearColor::White)]
								+ SVerticalBox::Slot().AutoHeight()[MakeMetricLine(NSLOCTEXT("T66Mini.Pause", "SystemsQuickRevive", "Quick Revive"), PlayerPawn && PlayerPawn->HasQuickReviveReady() ? FString(TEXT("Ready")) : FString(TEXT("Empty")), PlayerPawn && PlayerPawn->HasQuickReviveReady() ? T66MiniUI::AccentGreen() : T66MiniUI::MutedText())], T66MiniUI::AccentGreen())
						]
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							MakeSurface(SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeSectionHeading(NSLOCTEXT("T66Mini.Pause", "LootBag", "LOOT BAG"), NSLOCTEXT("T66Mini.Pause", "LootBagBody", "Item stacks currently carried by the active Mini run."))]
								+ SVerticalBox::Slot().FillHeight(1.f)[SNew(SScrollBox) + SScrollBox::Slot()[OwnedItemsPanel]], FLinearColor(0.68f, 0.52f, 0.84f, 1.f))
						]
					]
				]
			]
		]
		+ SOverlay::Slot()
		[
			SettingsModal
		];
}

FReply UT66MiniPauseMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (bWaitingForRebind)
	{
		const FKey Key = InKeyEvent.GetKey();
		if (Key == EKeys::Escape)
		{
			return FReply::Unhandled();
		}

		if (TryApplyCapturedBinding(Key))
		{
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UT66MiniPauseMenuWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bWaitingForRebind && (FPlatformTime::Seconds() - BindingCaptureStartedAt) > 0.15)
	{
		if (TryApplyCapturedBinding(InMouseEvent.GetEffectingButton()))
		{
			return FReply::Handled();
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

bool UT66MiniPauseMenuWidget::HandlePauseBackAction()
{
	if (bWaitingForRebind)
	{
		CancelPendingRebind();
		return true;
	}

	if (bSettingsModalOpen)
	{
		bSettingsModalOpen = false;
		SetSettingsStatus(NSLOCTEXT("T66Mini.Pause", "SettingsClosed", "Settings closed."));
		return true;
	}

	return false;
}

FReply UT66MiniPauseMenuWidget::HandleResumeClicked()
{
	if (AT66MiniPlayerController* MiniPlayerController = GetMiniController())
	{
		MiniPlayerController->ResumeFromPauseMenu();
	}

	return FReply::Handled();
}

FReply UT66MiniPauseMenuWidget::HandleSaveAndQuitClicked()
{
	if (AT66MiniPlayerController* MiniPlayerController = GetMiniController())
	{
		MiniPlayerController->SaveAndQuitToMiniMenuFromPause();
	}

	return FReply::Handled();
}

FReply UT66MiniPauseMenuWidget::HandleSettingsClicked()
{
	InitializeGraphicsState();
	bSettingsModalOpen = true;
	if (SettingsTabSwitcher.IsValid())
	{
		SettingsTabSwitcher->SetActiveWidgetIndex(static_cast<int32>(CurrentSettingsTab));
	}
	SetSettingsStatus(NSLOCTEXT("T66Mini.Pause", "SettingsOpened", "Press the current pause binding again to close this modal."));
	SetKeyboardFocus();
	return FReply::Handled();
}

FReply UT66MiniPauseMenuWidget::HandleCloseSettingsClicked()
{
	bSettingsModalOpen = false;
	SetSettingsStatus(NSLOCTEXT("T66Mini.Pause", "SettingsClosed", "Settings closed."));
	return FReply::Handled();
}

FReply UT66MiniPauseMenuWidget::HandleSettingsTabClicked(const ET66MiniPauseSettingsTab NewTab)
{
	CurrentSettingsTab = NewTab;
	if (SettingsTabSwitcher.IsValid())
	{
		SettingsTabSwitcher->SetActiveWidgetIndex(static_cast<int32>(CurrentSettingsTab));
	}

	return FReply::Handled();
}

FReply UT66MiniPauseMenuWidget::HandleStartRebind(const FName ActionName, const bool bAllowMouseButtons)
{
	FPendingMiniRebind NewPending;
	NewPending.ActionName = ActionName;
	NewPending.OldKey = FindActionKey(ActionName);
	NewPending.KeyText = BindingTextMap.FindRef(ActionName);
	NewPending.bAllowMouseButtons = bAllowMouseButtons;
	PendingRebind = NewPending;
	bWaitingForRebind = true;
	BindingCaptureStartedAt = FPlatformTime::Seconds();
	SetSettingsStatus(FText::Format(NSLOCTEXT("T66Mini.Pause", "WaitingForRebind", "Press a new key for {0}."), FText::FromName(ActionName)));
	SetKeyboardFocus();
	return FReply::Handled();
}

FReply UT66MiniPauseMenuWidget::HandleResetBindingsClicked()
{
	UInputSettings* Settings = UInputSettings::GetInputSettings();
	if (!Settings)
	{
		SetSettingsStatus(NSLOCTEXT("T66Mini.Pause", "BindingsUnavailable", "Input settings are unavailable."));
		return FReply::Handled();
	}

	const TArray<FInputActionKeyMapping> ExistingMappings = Settings->GetActionMappings();
	for (const FInputActionKeyMapping& Mapping : ExistingMappings)
	{
		if ((Mapping.ActionName == MiniPauseAction || Mapping.ActionName == MiniInteractAction || Mapping.ActionName == MiniUltimateAction)
			&& IsKeyboardMouseKey(Mapping.Key))
		{
			Settings->RemoveActionMapping(Mapping);
		}
	}

	Settings->AddActionMapping(FInputActionKeyMapping(MiniPauseAction, EKeys::Escape));
	Settings->AddActionMapping(FInputActionKeyMapping(MiniInteractAction, EKeys::LeftMouseButton));
	Settings->AddActionMapping(FInputActionKeyMapping(MiniUltimateAction, EKeys::RightMouseButton));
	Settings->SaveKeyMappings();
	Settings->SaveConfig();

	if (APlayerController* OwningPlayer = GetOwningPlayer())
	{
		if (OwningPlayer->PlayerInput)
		{
			OwningPlayer->PlayerInput->ForceRebuildingKeyMaps(true);
		}
	}

	RefreshAllBindingTexts();
	SetSettingsStatus(NSLOCTEXT("T66Mini.Pause", "BindingsRestored", "Mini bindings restored to their default layout."));
	return FReply::Handled();
}

FReply UT66MiniPauseMenuWidget::HandleApplyGraphicsClicked()
{
	ApplyGraphicsSettings();
	return FReply::Handled();
}

void UT66MiniPauseMenuWidget::InitializeGraphicsState()
{
	if (bGraphicsInitialized)
	{
		return;
	}

	bGraphicsInitialized = true;

	if (UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		PendingGraphics.WindowMode = GUS->GetFullscreenMode();
		PendingGraphics.QualityNotch = FMath::Clamp(GUS->GetOverallScalabilityLevel(), 0, 3);

		const float Limit = GUS->GetFrameRateLimit();
		if (Limit <= 0.0f)
		{
			PendingGraphics.FpsCapIndex = 4;
		}
		else if (Limit <= 30.0f)
		{
			PendingGraphics.FpsCapIndex = 0;
		}
		else if (Limit <= 60.0f)
		{
			PendingGraphics.FpsCapIndex = 1;
		}
		else if (Limit <= 90.0f)
		{
			PendingGraphics.FpsCapIndex = 2;
		}
		else
		{
			PendingGraphics.FpsCapIndex = 3;
		}
	}

	if (UT66PlayerSettingsSubsystem* PlayerSettings = GetPlayerSettings())
	{
		PendingGraphics.DisplayModeIndex = FMath::Clamp(PlayerSettings->GetDisplayModeIndex(), 0, 1);
		PendingGraphics.MonitorIndex = FMath::Clamp(PlayerSettings->GetMonitorIndex(), 0, GetMonitorCount() - 1);
	}

	PendingGraphics.bDirty = false;
}

void UT66MiniPauseMenuWidget::ApplyGraphicsSettings()
{
	UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!GUS)
	{
		SetSettingsStatus(NSLOCTEXT("T66Mini.Pause", "GraphicsApplyFailed", "Unable to access engine graphics settings."));
		return;
	}

	GUS->SetFullscreenMode(PendingGraphics.WindowMode);
	GUS->SetOverallScalabilityLevel(FMath::Clamp(PendingGraphics.QualityNotch, 0, 3));
	GUS->SetFrameRateLimit(FpsCaps[FMath::Clamp(PendingGraphics.FpsCapIndex, 0, 4)]);
	GUS->ApplySettings(false);
	GUS->SaveSettings();

	if (UT66PlayerSettingsSubsystem* PlayerSettings = GetPlayerSettings())
	{
		PlayerSettings->SetDisplayModeIndex(PendingGraphics.DisplayModeIndex);
		PlayerSettings->SetMonitorIndex(PendingGraphics.MonitorIndex);
	}

	FDisplayMetrics Metrics;
	FSlateApplication::Get().GetCachedDisplayMetrics(Metrics);
	if (Metrics.MonitorInfo.IsValidIndex(PendingGraphics.MonitorIndex))
	{
		const FMonitorInfo& MonitorInfo = Metrics.MonitorInfo[PendingGraphics.MonitorIndex];
		const TArray<TSharedRef<SWindow>> TopLevelWindows = FSlateApplication::Get().GetTopLevelWindows();
		if (TopLevelWindows.Num() > 0)
		{
			TopLevelWindows[0]->MoveWindowTo(FVector2D(static_cast<float>(MonitorInfo.WorkArea.Left), static_cast<float>(MonitorInfo.WorkArea.Top)));
		}
	}

	PendingGraphics.bDirty = false;
	SetSettingsStatus(NSLOCTEXT("T66Mini.Pause", "GraphicsApplied", "Graphics settings applied."));
}

void UT66MiniPauseMenuWidget::RefreshBindingText(const FName ActionName)
{
	if (const TSharedPtr<STextBlock>* TextBlock = BindingTextMap.Find(ActionName))
	{
		if (TextBlock->IsValid())
		{
			(*TextBlock)->SetText(KeyToText(FindActionKey(ActionName)));
		}
	}
}

void UT66MiniPauseMenuWidget::RefreshAllBindingTexts()
{
	RefreshBindingText(MiniPauseAction);
	RefreshBindingText(MiniInteractAction);
	RefreshBindingText(MiniUltimateAction);
}

bool UT66MiniPauseMenuWidget::TryApplyCapturedBinding(const FKey& Key)
{
	if (!bWaitingForRebind)
	{
		return false;
	}

	if (!IsKeyboardMouseKey(Key))
	{
		SetSettingsStatus(NSLOCTEXT("T66Mini.Pause", "KeyboardMouseOnly", "Mini key binding only accepts keyboard or mouse input."));
		return true;
	}

	if (!PendingRebind.bAllowMouseButtons && Key.IsMouseButton())
	{
		SetSettingsStatus(NSLOCTEXT("T66Mini.Pause", "PauseKeyboardOnly", "Pause / Back must stay on a keyboard key."));
		return true;
	}

	UInputSettings* Settings = UInputSettings::GetInputSettings();
	if (!Settings)
	{
		SetSettingsStatus(NSLOCTEXT("T66Mini.Pause", "BindingsUnavailable", "Input settings are unavailable."));
		return true;
	}

	const TArray<FInputActionKeyMapping> ExistingMappings = Settings->GetActionMappings();
	for (const FInputActionKeyMapping& Mapping : ExistingMappings)
	{
		if (!IsKeyboardMouseKey(Mapping.Key))
		{
			continue;
		}

		if (Mapping.ActionName == PendingRebind.ActionName)
		{
			Settings->RemoveActionMapping(Mapping);
			continue;
		}

		if ((Mapping.ActionName == MiniPauseAction || Mapping.ActionName == MiniInteractAction || Mapping.ActionName == MiniUltimateAction)
			&& Mapping.Key == Key)
		{
			Settings->RemoveActionMapping(Mapping);
		}
	}

	Settings->AddActionMapping(FInputActionKeyMapping(PendingRebind.ActionName, Key));
	Settings->SaveKeyMappings();
	Settings->SaveConfig();

	if (APlayerController* OwningPlayer = GetOwningPlayer())
	{
		if (OwningPlayer->PlayerInput)
		{
			OwningPlayer->PlayerInput->ForceRebuildingKeyMaps(true);
		}
	}

	bWaitingForRebind = false;
	RefreshAllBindingTexts();
	SetSettingsStatus(FText::Format(
		NSLOCTEXT("T66Mini.Pause", "BindingSaved", "{0} is now bound to {1}."),
		FText::FromName(PendingRebind.ActionName),
		KeyToText(Key)));
	PendingRebind = FPendingMiniRebind{};
	return true;
}

void UT66MiniPauseMenuWidget::CancelPendingRebind()
{
	if (!bWaitingForRebind)
	{
		return;
	}

	bWaitingForRebind = false;
	PendingRebind = FPendingMiniRebind{};
	SetSettingsStatus(NSLOCTEXT("T66Mini.Pause", "BindingCancelled", "Key binding cancelled."));
}

void UT66MiniPauseMenuWidget::SetSettingsStatus(const FText& StatusText)
{
	if (SettingsStatusText.IsValid())
	{
		SettingsStatusText->SetText(StatusText);
	}
}

AT66MiniPlayerController* UT66MiniPauseMenuWidget::GetMiniController() const
{
	return Cast<AT66MiniPlayerController>(GetOwningPlayer());
}

UT66PlayerSettingsSubsystem* UT66MiniPauseMenuWidget::GetPlayerSettings() const
{
	if (const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		return GameInstance->GetSubsystem<UT66PlayerSettingsSubsystem>();
	}

	return nullptr;
}

FKey UT66MiniPauseMenuWidget::FindActionKey(const FName ActionName)
{
	if (const UInputSettings* Settings = UInputSettings::GetInputSettings())
	{
		for (const FInputActionKeyMapping& Mapping : Settings->GetActionMappings())
		{
			if (Mapping.ActionName == ActionName && IsKeyboardMouseKey(Mapping.Key))
			{
				return Mapping.Key;
			}
		}
	}

	if (ActionName == MiniPauseAction)
	{
		return EKeys::Escape;
	}

	if (ActionName == MiniInteractAction)
	{
		return EKeys::LeftMouseButton;
	}

	if (ActionName == MiniUltimateAction)
	{
		return EKeys::RightMouseButton;
	}

	return FKey();
}

FText UT66MiniPauseMenuWidget::KeyToText(const FKey& Key)
{
	return Key.IsValid() ? Key.GetDisplayName() : NSLOCTEXT("T66Mini.Pause", "Unbound", "Unbound");
}

bool UT66MiniPauseMenuWidget::IsKeyboardMouseKey(const FKey& Key)
{
	return Key.IsValid() && !Key.IsGamepadKey() && !Key.IsTouch();
}
