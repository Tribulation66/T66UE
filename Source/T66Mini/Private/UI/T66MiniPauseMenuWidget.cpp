// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66MiniPauseMenuWidget.h"

#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniRunStateSubsystem.h"
#include "Data/T66MiniDataTypes.h"
#include "Gameplay/T66MiniGameMode.h"
#include "Gameplay/T66MiniPlayerPawn.h"
#include "Save/T66MiniRunSaveGame.h"
#include "Gameplay/T66MiniPlayerController.h"
#include "Styling/CoreStyle.h"
#include "UI/T66MiniUIStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	FString T66MiniPauseDescribeItem(const FT66MiniItemDefinition& ItemDefinition)
	{
		if (!ItemDefinition.SecondaryStatType.IsEmpty())
		{
			return ItemDefinition.SecondaryStatType;
		}

		return ItemDefinition.PrimaryStatType.IsEmpty() ? ItemDefinition.ItemID.ToString() : ItemDefinition.PrimaryStatType;
	}
}

TSharedRef<SWidget> UT66MiniPauseMenuWidget::RebuildWidget()
{
	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	const UT66MiniRunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PlayerSettings = GameInstance ? GameInstance->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	const UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	const AT66MiniPlayerPawn* PlayerPawn = Cast<AT66MiniPlayerPawn>(GetOwningPlayer() ? GetOwningPlayer()->GetPawn() : nullptr);

	TSharedRef<SVerticalBox> OwnedItemsPanel = SNew(SVerticalBox);
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

		const int32 MaxVisibleOwnedItems = 10;
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
				.Font(T66MiniUI::BodyFont(15))
				.ColorAndOpacity(FLinearColor::White)
				.AutoWrapText(true)
			];
		}

		if (OwnedItems.Num() > MaxVisibleOwnedItems)
		{
			OwnedItemsPanel->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("+%d more item stacks"), OwnedItems.Num() - MaxVisibleOwnedItems)))
				.Font(T66MiniUI::BodyFont(15))
				.ColorAndOpacity(T66MiniUI::MutedText())
			];
		}

		if (OwnedItems.Num() == 0)
		{
			OwnedItemsPanel->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66Mini.Pause", "NoItems", "No owned items yet. Loot bags and crates will show up here once collected."))
				.Font(T66MiniUI::BodyFont(15))
				.ColorAndOpacity(T66MiniUI::MutedText())
				.AutoWrapText(true)
			];
		}
	}
	else
	{
		OwnedItemsPanel->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66Mini.Pause", "MissingInventory", "Mini inventory data is unavailable for this pause snapshot."))
			.Font(T66MiniUI::BodyFont(15))
			.ColorAndOpacity(T66MiniUI::MutedText())
			.AutoWrapText(true)
		];
	}

	const auto MakeSectionTitle = [](const FText& Label) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(Label)
			.Font(T66MiniUI::BoldFont(18))
			.ColorAndOpacity(FLinearColor::White);
	};

	const auto MakeBodyLine = [](const FString& Label) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(FText::FromString(Label))
			.Font(T66MiniUI::BodyFont(16))
			.ColorAndOpacity(FLinearColor(0.90f, 0.92f, 0.96f))
			.AutoWrapText(true);
	};

	const auto MakeSettingsSliderRow = [](const FText& Label, TFunction<float()> GetValue, TFunction<void(float)> SetValue) -> TSharedRef<SWidget>
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(T66MiniUI::BodyFont(14))
					.ColorAndOpacity(FLinearColor(0.90f, 0.92f, 0.96f))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Lambda([GetValue]()
					{
						const int32 Percent = FMath::RoundToInt(FMath::Clamp(GetValue(), 0.f, 1.f) * 100.f);
						return FText::FromString(FString::Printf(TEXT("%d%%"), Percent));
					})
					.Font(T66MiniUI::BodyFont(13))
					.ColorAndOpacity(T66MiniUI::MutedText())
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSlider)
				.Value_Lambda([GetValue]()
				{
					return FMath::Clamp(GetValue(), 0.f, 1.f);
				})
				.OnValueChanged_Lambda([SetValue](const float NewValue)
				{
					SetValue(FMath::Clamp(NewValue, 0.f, 1.f));
				})
			];
	};

	TSharedRef<SWidget> SettingsPanel = SNullWidget::NullWidget;
	if (PlayerSettings)
	{
		SettingsPanel =
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				MakeSectionTitle(NSLOCTEXT("T66Mini.Pause", "SettingsTitle", "SETTINGS"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66Mini.Pause", "SettingsSubtitle", "These sliders use the shared player audio profile."))
				.Font(T66MiniUI::BodyFont(13))
				.ColorAndOpacity(T66MiniUI::MutedText())
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
			[
				MakeSettingsSliderRow(
					NSLOCTEXT("T66Mini.Pause", "MasterVolume", "Master Volume"),
					[PlayerSettings]() { return PlayerSettings ? PlayerSettings->GetMasterVolume() : 0.8f; },
					[this, PlayerSettings](const float Value)
					{
						if (PlayerSettings)
						{
							PlayerSettings->SetMasterVolume(Value);
						}

						if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
						{
							MiniGameMode->RefreshAudioMix();
						}
					})
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
			[
				MakeSettingsSliderRow(
					NSLOCTEXT("T66Mini.Pause", "MusicVolume", "Music Volume"),
					[PlayerSettings]() { return PlayerSettings ? PlayerSettings->GetMusicVolume() : 0.6f; },
					[this, PlayerSettings](const float Value)
					{
						if (PlayerSettings)
						{
							PlayerSettings->SetMusicVolume(Value);
						}

						if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
						{
							MiniGameMode->RefreshAudioMix();
						}
					})
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				MakeSettingsSliderRow(
					NSLOCTEXT("T66Mini.Pause", "SfxVolume", "SFX Volume"),
					[PlayerSettings]() { return PlayerSettings ? PlayerSettings->GetSfxVolume() : 0.8f; },
					[this, PlayerSettings](const float Value)
					{
						if (PlayerSettings)
						{
							PlayerSettings->SetSfxVolume(Value);
						}

						if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
						{
							MiniGameMode->RefreshAudioMix();
						}
					})
			];
	}
	else
	{
		SettingsPanel =
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				MakeSectionTitle(NSLOCTEXT("T66Mini.Pause", "SettingsTitleUnavailable", "SETTINGS"))
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66Mini.Pause", "SettingsUnavailable", "Audio settings are unavailable in this session."))
				.Font(T66MiniUI::BodyFont(13))
				.ColorAndOpacity(T66MiniUI::MutedText())
				.AutoWrapText(true)
			];
	}

	const FString HeroLabel = PlayerPawn
		? (PlayerPawn->GetHeroDisplayName().IsEmpty() ? PlayerPawn->GetHeroID().ToString() : PlayerPawn->GetHeroDisplayName())
		: FString(TEXT("Unknown"));
	const FString DifficultyLabel = ActiveRun ? ActiveRun->DifficultyID.ToString() : FString(TEXT("Unknown"));
	const FString WaveLabel = ActiveRun ? FString::Printf(TEXT("Wave %d"), ActiveRun->WaveIndex) : FString(TEXT("Wave ?"));
	const FString HealthLabel = PlayerPawn
		? FString::Printf(TEXT("Hearts: %d   HP %.0f / %.0f"), PlayerPawn->GetMaxHearts(), PlayerPawn->GetCurrentHealth(), PlayerPawn->GetMaxHealth())
		: FString(TEXT("Hearts: unavailable"));
	const FString EconomyLabel = PlayerPawn
		? FString::Printf(TEXT("Gold: %d   Materials: %d   Level: %d"), PlayerPawn->GetGold(), PlayerPawn->GetMaterials(), PlayerPawn->GetHeroLevel())
		: FString(TEXT("Economy: unavailable"));
	const FString CombatLineOne = PlayerPawn
		? FString::Printf(TEXT("DMG %.1f   ATK %.1f"), PlayerPawn->GetDamageStat(), PlayerPawn->GetAttackSpeedStat())
		: FString(TEXT("DMG ?   ATK ?"));
	const FString CombatLineTwo = PlayerPawn
		? FString::Printf(TEXT("ARM %.1f   CRIT %.0f%%"), PlayerPawn->GetArmorStat(), PlayerPawn->GetCritChance() * 100.f)
		: FString(TEXT("ARM ?   CRIT ?"));
	const FString CombatLineThree = PlayerPawn
		? FString::Printf(TEXT("REGEN %.1f   EVADE %.0f%%"), PlayerPawn->GetPassiveRegenPerSecond(), PlayerPawn->GetEvasionChance() * 100.f)
		: FString(TEXT("REGEN ?   EVADE ?"));
	const FString CombatLineFour = PlayerPawn
		? FString::Printf(TEXT("RANGE %.0f   LS %.0f%%"), PlayerPawn->GetAttackRange(), PlayerPawn->GetLifeStealChance() * 100.f)
		: FString(TEXT("RANGE ?   LS ?"));

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.72f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(1040.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.07f, 0.98f))
				.Padding(FMargin(28.f, 24.f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.f, 0.f, 24.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(260.f)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.f, 0.f, 0.f, 8.f)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66Mini.Pause", "Title", "PAUSED"))
								.Font(T66MiniUI::BoldFont(32))
								.ColorAndOpacity(FLinearColor::White)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.f, 0.f, 0.f, 18.f)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66Mini.Pause", "Subtitle", "Mini run snapshot"))
								.Font(T66MiniUI::BodyFont(16))
								.ColorAndOpacity(T66MiniUI::MutedText())
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.f, 0.f, 0.f, 12.f)
							[
								SNew(SBox)
								.WidthOverride(240.f)
								.HeightOverride(56.f)
								[
									SNew(SButton)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniPauseMenuWidget::HandleResumeClicked))
									.ButtonColorAndOpacity(T66MiniUI::AccentGreen())
									[
										SNew(STextBlock)
										.Text(NSLOCTEXT("T66Mini.Pause", "Resume", "RESUME"))
										.Font(T66MiniUI::BoldFont(22))
										.ColorAndOpacity(T66MiniUI::ButtonTextDark())
										.Justification(ETextJustify::Center)
									]
								]
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(240.f)
								.HeightOverride(56.f)
								[
									SNew(SButton)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniPauseMenuWidget::HandleMainMenuClicked))
									.ButtonColorAndOpacity(T66MiniUI::RaisedFill())
									[
										SNew(STextBlock)
										.Text(NSLOCTEXT("T66Mini.Pause", "MainMenu", "BACK TO MAIN MENU"))
										.Font(T66MiniUI::BoldFont(20))
										.ColorAndOpacity(FLinearColor::White)
										.Justification(ETextJustify::Center)
									]
								]
							]
							+ SVerticalBox::Slot()
							.FillHeight(1.f)
							.Padding(0.f, 24.f, 0.f, 0.f)
							.VAlign(VAlign_Bottom)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.09f, 0.10f, 0.13f, 0.94f))
								.Padding(FMargin(14.f))
								[
									SettingsPanel
								]
							]
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 0.f, 0.f, 12.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.09f, 0.10f, 0.13f, 0.94f))
							.Padding(FMargin(16.f))
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeSectionTitle(NSLOCTEXT("T66Mini.Pause", "RunStatusTitle", "RUN STATUS"))]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[MakeBodyLine(FString::Printf(TEXT("%s   |   %s   |   %s"), *HeroLabel, *DifficultyLabel, *WaveLabel))]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[MakeBodyLine(HealthLabel)]
								+ SVerticalBox::Slot().AutoHeight()[MakeBodyLine(EconomyLabel)]
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 0.f, 0.f, 12.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.09f, 0.10f, 0.13f, 0.94f))
							.Padding(FMargin(16.f))
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeSectionTitle(NSLOCTEXT("T66Mini.Pause", "CombatTitle", "COMBAT STATS"))]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[MakeBodyLine(CombatLineOne)]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[MakeBodyLine(CombatLineTwo)]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[MakeBodyLine(CombatLineThree)]
								+ SVerticalBox::Slot().AutoHeight()[MakeBodyLine(CombatLineFour)]
							]
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.09f, 0.10f, 0.13f, 0.94f))
							.Padding(FMargin(16.f))
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeSectionTitle(NSLOCTEXT("T66Mini.Pause", "InventoryTitle", "OWNED ITEMS"))]
								+ SVerticalBox::Slot()
								.FillHeight(1.f)
								[
									SNew(SScrollBox)
									+ SScrollBox::Slot()
									[
										OwnedItemsPanel
									]
								]
							]
						]
					]
				]
			]
		];
}

FReply UT66MiniPauseMenuWidget::HandleResumeClicked()
{
	if (AT66MiniPlayerController* MiniPlayerController = Cast<AT66MiniPlayerController>(GetOwningPlayer()))
	{
		MiniPlayerController->ResumeFromPauseMenu();
	}

	return FReply::Handled();
}

FReply UT66MiniPauseMenuWidget::HandleMainMenuClicked()
{
	if (AT66MiniPlayerController* MiniPlayerController = Cast<AT66MiniPlayerController>(GetOwningPlayer()))
	{
		MiniPlayerController->ReturnToMainMenuFromPause();
	}

	return FReply::Handled();
}
