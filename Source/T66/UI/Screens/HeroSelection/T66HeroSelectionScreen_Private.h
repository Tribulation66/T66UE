// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/Screens/HeroSelection/T66HeroSelectionPreviewController.h"
#include "UI/Screens/T66SelectionScreenUtils.h"
#include "UI/T66UIManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66SkinSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66SteamHelper.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/T66StatsPanelSlate.h"
#include "UI/T66TemporaryBuffUIUtils.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66CompanionBase.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Gameplay/T66CompanionPreviewStage.h"
#include "Gameplay/T66FrontendGameMode.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SToolTip.h"
#include "Brushes/SlateImageBrush.h"
#include "Styling/CoreStyle.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/Events.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "FileMediaSource.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

DECLARE_LOG_CATEGORY_EXTERN(LogT66HeroSelection, Log, All);

namespace T66HeroSelectionPrivate
{
	inline constexpr int32 HeroSelectionCarouselVisibleSlots = 7;
	inline constexpr int32 HeroSelectionCarouselCenterIndex = HeroSelectionCarouselVisibleSlots / 2;

	inline AT66PlayerController* T66GetLocalFrontendHeroPlayerController(UObject* ContextObject)
	{
		return ContextObject ? Cast<AT66PlayerController>(UGameplayStatics::GetPlayerController(ContextObject, 0)) : nullptr;
	}

	inline void T66PositionHeroPreviewCamera(UObject* ContextObject)
	{
		if (!ContextObject)
		{
			return;
		}

		if (UWorld* World = ContextObject->GetWorld())
		{
			if (AT66FrontendGameMode* GM = Cast<AT66FrontendGameMode>(World->GetAuthGameMode()))
			{
				GM->PositionCameraForHeroPreview();
				return;
			}
		}

		if (AT66PlayerController* PC = T66GetLocalFrontendHeroPlayerController(ContextObject))
		{
			PC->PositionLocalFrontendCameraForHeroPreview();
		}
	}

	inline float GetHeroSelectionCarouselBoxSize(const int32 Offset)
	{
		switch (FMath::Abs(Offset))
		{
		case 0: return 48.f;
		case 1: return 40.f;
		case 2: return 36.f;
		default: return 32.f;
		}
	}

	inline float GetHeroSelectionCarouselOpacity(const int32 Offset)
	{
		switch (FMath::Abs(Offset))
		{
		case 0: return 1.0f;
		case 1: return 0.82f;
		case 2: return 0.66f;
		default: return 0.52f;
		}
	}

	inline TSoftObjectPtr<UTexture2D> GetHeroSelectionBalanceIcon()
	{
		return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Assets/TopBar/frontend_topbar_power_coupons_icon.frontend_topbar_power_coupons_icon")));
	}

	inline FString GetHeroSelectionMedalImagePath(const ET66AccountMedalTier Tier)
	{
		switch (Tier)
		{
		case ET66AccountMedalTier::Bronze:
			return FPaths::ProjectContentDir() / TEXT("UI/Assets/Medals/medal_bronze.png");
		case ET66AccountMedalTier::Silver:
			return FPaths::ProjectContentDir() / TEXT("UI/Assets/Medals/medal_silver.png");
		case ET66AccountMedalTier::Gold:
			return FPaths::ProjectContentDir() / TEXT("UI/Assets/Medals/medal_gold.png");
		case ET66AccountMedalTier::Platinum:
			return FPaths::ProjectContentDir() / TEXT("UI/Assets/Medals/medal_platinum.png");
		case ET66AccountMedalTier::Diamond:
			return FPaths::ProjectContentDir() / TEXT("UI/Assets/Medals/medal_diamond.png");
		case ET66AccountMedalTier::None:
		default:
			return FPaths::ProjectContentDir() / TEXT("UI/Assets/Medals/medal_unproven.png");
		}
	}

	inline FString GetHeroSelectionChallengesIconPath()
	{
		return FPaths::ConvertRelativePathToFull(
			FPaths::ProjectDir() / TEXT("RuntimeDependencies/T66/UI/HeroSelection/challenges_crossed_swords.png"));
	}

	inline FString ResolveArthurPreviewMoviePath()
	{
		const TArray<FString> CandidatePaths = {
			FPaths::ProjectContentDir() / TEXT("Movies/ArthurPreview.mp4"),
			FPaths::ProjectDir() / TEXT("SourceAssets/Preview Videos/Arthur Preview.mp4"),
			FPaths::ProjectContentDir() / TEXT("Movies/KnightClip.mp4"),
			FPaths::ProjectDir() / TEXT("SourceAssets/Movies/KnightClip.mp4")
		};

		for (const FString& CandidatePath : CandidatePaths)
		{
			if (IFileManager::Get().FileExists(*CandidatePath))
			{
				return FPaths::ConvertRelativePathToFull(CandidatePath);
			}
		}

		return FString();
	}

	inline TSoftObjectPtr<UTexture2D> ResolveHeroSelectionUltimateIcon(const FName HeroID, const ET66UltimateType UltimateType)
	{
		if (HeroID == FName(TEXT("Hero_1")) && UltimateType == ET66UltimateType::SpearStorm)
		{
			return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/Abilities/Hero_1/T_Hero_1_Ultimate.T_Hero_1_Ultimate")));
		}

		return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/ULTS/KnightULT.KnightULT")));
	}

	inline TSoftObjectPtr<UTexture2D> ResolveHeroSelectionPassiveIcon(const FName HeroID, const ET66PassiveType PassiveType)
	{
		if (HeroID == FName(TEXT("Hero_1")) && PassiveType == ET66PassiveType::IronWill)
		{
			return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/Abilities/Hero_1/T_Hero_1_Passive.T_Hero_1_Passive")));
		}

		return ResolveHeroSelectionUltimateIcon(HeroID, ET66UltimateType::None);
	}

	inline TSharedRef<SToolTip> MakeHeroSelectionAbilityTooltip(const FText& Title, const FText& Description, const int32 FontSizeAdjustment = 0)
	{
		return SNew(SToolTip)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 4.f)
					[
						SNew(STextBlock)
						.Text(Title)
						.Font(FT66Style::Tokens::FontBold(FMath::Max(14 + FontSizeAdjustment, 10)))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(Description)
						.Font(FT66Style::Tokens::FontRegular(FMath::Max(12 + FontSizeAdjustment, 9)))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
						.WrapTextAt(280.f)
					],
					FT66PanelParams(ET66PanelType::Panel)
						.SetColor(FT66Style::Tokens::Panel2)
						.SetPadding(FMargin(12.f, 10.f)))
			];
	}

	inline FText HeroRecordMedalText(ET66AccountMedalTier Tier)
	{
		switch (Tier)
		{
		case ET66AccountMedalTier::Bronze:
			return NSLOCTEXT("T66.HeroSelection", "MedalBronze", "Bronze");
		case ET66AccountMedalTier::Silver:
			return NSLOCTEXT("T66.HeroSelection", "MedalSilver", "Silver");
		case ET66AccountMedalTier::Gold:
			return NSLOCTEXT("T66.HeroSelection", "MedalGold", "Gold");
		case ET66AccountMedalTier::Platinum:
			return NSLOCTEXT("T66.HeroSelection", "MedalPlatinum", "Platinum");
		case ET66AccountMedalTier::Diamond:
			return NSLOCTEXT("T66.HeroSelection", "MedalDiamond", "Diamond");
		case ET66AccountMedalTier::None:
		default:
			return NSLOCTEXT("T66.HeroSelection", "MedalNone", "Unproven");
		}
	}

	inline FLinearColor HeroRecordMedalColor(ET66AccountMedalTier Tier)
	{
		switch (Tier)
		{
		case ET66AccountMedalTier::Bronze:
			return FLinearColor(0.67f, 0.43f, 0.26f, 1.0f);
		case ET66AccountMedalTier::Silver:
			return FLinearColor(0.76f, 0.79f, 0.84f, 1.0f);
		case ET66AccountMedalTier::Gold:
			return FLinearColor(0.89f, 0.74f, 0.27f, 1.0f);
		case ET66AccountMedalTier::Platinum:
			return FLinearColor(0.56f, 0.77f, 0.88f, 1.0f);
		case ET66AccountMedalTier::Diamond:
			return FLinearColor(0.45f, 0.86f, 0.99f, 1.0f);
		case ET66AccountMedalTier::None:
		default:
			return FLinearColor(0.74f, 0.74f, 0.74f, 1.0f);
		}
	}

	inline FText HeroRecordThirdMetricLabel(const bool bShowingCompanionInfo)
	{
		return bShowingCompanionInfo
			? NSLOCTEXT("T66.HeroSelection", "CompanionRecordHealingLabel", "Total Healing")
			: NSLOCTEXT("T66.HeroSelection", "HeroRecordScoreLabel", "Cumulative Score");
	}

	inline FText FormatCompanionHealingPerSecondText(const float HealingPerSecond)
	{
		return FText::Format(
			NSLOCTEXT("T66.HeroSelection", "CompanionHealingPerSecondFormat", "Heals / Second: {0}"),
			FText::AsNumber(FMath::RoundToInt(HealingPerSecond)));
	}
}
