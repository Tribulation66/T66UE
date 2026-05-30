// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/Screens/HeroSelection/T66HeroSelectionPreviewController.h"
#include "UI/Screens/T66SelectionScreenUtils.h"
#include "UI/T66UIManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66SkinSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66SteamHelper.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/T66DemoModeUIUtils.h"
#include "UI/T66StatsPanelSlate.h"
#include "UI/T66TemporaryBuffUIUtils.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66Style.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "Gameplay/T66CompanionBase.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SConstraintCanvas.h"
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
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UI/Style/T66ButtonVisuals.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66RuntimeUIFontAccess.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Input/SComboButton.h"

DECLARE_LOG_CATEGORY_EXTERN(LogT66HeroSelection, Log, All);

namespace T66HeroSelectionPrivate
{
	inline constexpr int32 HeroSelectionCarouselVisibleSlots = 5;
	inline constexpr int32 HeroSelectionCarouselCenterIndex = HeroSelectionCarouselVisibleSlots / 2;
	inline constexpr int32 HeroSelectionHeroCarouselVisibleSlots = 7;
	inline constexpr int32 HeroSelectionHeroCarouselCenterIndex = HeroSelectionHeroCarouselVisibleSlots / 2;

	inline FLinearColor HeroSelectionChromeAccent(float Alpha = 1.0f)
	{
		FLinearColor Color = FT66FlatStyle::SelectedBorder();
		Color.A = Alpha;
		return Color;
	}

	inline FLinearColor HeroSelectionChromeAccentInactive(float Alpha = 0.75f)
	{
		FLinearColor Color = FT66FlatStyle::DefaultBorder();
		Color.A = Alpha;
		return Color;
	}

	inline FLinearColor HeroSelectionChromeInnerFill(float Alpha = 0.96f)
	{
		FLinearColor Color = FT66FlatStyle::DefaultFill();
		Color.A = Alpha;
		return Color;
	}

	inline FLinearColor HeroSelectionChromeInnerFillAlt(float Alpha = 0.96f)
	{
		FLinearColor Color = FT66FlatStyle::SelectedFill();
		Color.A = Alpha;
		return Color;
	}

	inline FLinearColor HeroSelectionChromeTokenAccent()
	{
		return FT66FlatStyle::PurpleAccent();
	}

	struct FHeroSelectionSharedLayoutMetrics
	{
		FVector2D LayoutViewportSize = FVector2D(1920.f, 1080.f);
		bool bShortViewport = false;
		float ReferenceLayoutWidth = 1920.f;
		float ReferenceLayoutHeight = 1080.f;
		float PanelTouchOverlap = 0.f;
		float LeftPanelWidth = 548.f;
		float RightPanelWidth = 592.f;
		float CenterPanelX = 580.f;
		float CenterPreviewWidth = 714.f;
		float PartyFooterWidth = 664.f;
		float CompanionFooterWidth = 508.f;
		float CompanionFooterX = 708.f;
		float RunFooterX = 1252.f;
		float RunFooterWidth = 652.f;
		float CompanionFooterContentWidth = 486.f;
		float RunFooterContentWidth = 694.f;
		float UpperPanelY = 16.f;
		float FooterPanelMinHeight = 218.f;
		float FooterPanelY = 804.f;
		float UpperSidePanelHeight = 752.f;
		float RightStatsCardHeight = 200.f;
		float RightUltRowHeight = 136.f;
		float PanelGap = FT66FlatStyle::FlatGap;
		float OuterPanelBleed = 0.f;
		float TopBarBottomGap = 8.f;
		float LayoutCompactScale = 1.f;
		float FooterToggleWidth = 239.f;
		float FooterToggleHeight = 76.f;
		float FooterActionHeight = 136.f;
		float BalanceBadgeIconWidth = 56.f;
		float BalanceBadgeIconHeight = 34.f;
		float LeftSkinsCardHeight = 428.f;
		float RightPreviewPanelHeight = 72.f;
		float RightAbilityIconButtonSize = 90.f;
		float RightAbilityIconSize = 70.f;
		int32 ScreenHeaderFontSize = 21;
		int32 BodyToggleFontSize = 21;
		int32 PrimaryCtaFontSize = 26;
		int32 HeroArrowFontSize = 24;
		int32 ACBalanceFontSize = 23;
		int32 HeroNameFontSize = 31;
		int32 SecondaryButtonFontSize = 20;
		int32 EntityDropdownFontSize = 20;
		int32 BodyTextFontSize = 16;
		int32 DifficultyMenuFontSize = 23;
		float HeroArrowButtonWidth = 64.f;
		float HeroArrowButtonHeight = 48.f;
		float TopStripBackButtonWidth = 112.f;
		float TopStripBackButtonHeight = 34.f;
	};

	inline FVector2D GetHeroSelectionLayoutViewportSize()
	{
		FVector2D LayoutViewportSize = FT66Style::GetViewportLogicalSize();
		LayoutViewportSize.X = FMath::Max(LayoutViewportSize.X, 1.f);
		LayoutViewportSize.Y = FMath::Max(LayoutViewportSize.Y, 1.f);

		int32 AutomationViewportWidth = 0;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66AutomationResX="), AutomationViewportWidth) && AutomationViewportWidth > 0)
		{
			LayoutViewportSize.X = static_cast<float>(AutomationViewportWidth) / FMath::Max(0.01f, FT66Style::GetGlobalUIScale());
		}
		int32 AutomationViewportHeight = 0;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66AutomationResY="), AutomationViewportHeight) && AutomationViewportHeight > 0)
		{
			LayoutViewportSize.Y = static_cast<float>(AutomationViewportHeight) / FMath::Max(0.01f, FT66Style::GetGlobalUIScale());
		}

		return LayoutViewportSize;
	}

	inline FHeroSelectionSharedLayoutMetrics MakeHeroSelectionSharedLayoutMetrics()
	{
		FHeroSelectionSharedLayoutMetrics Metrics;
		Metrics.LayoutViewportSize = GetHeroSelectionLayoutViewportSize();
		Metrics.bShortViewport = Metrics.LayoutViewportSize.Y < 960.f;
		Metrics.ReferenceLayoutWidth = 1920.f;
		constexpr float ReferenceLayoutBaselineHeight = 1080.f;
		Metrics.ReferenceLayoutHeight = FMath::Max(
			ReferenceLayoutBaselineHeight,
			FMath::CeilToFloat(Metrics.ReferenceLayoutWidth * Metrics.LayoutViewportSize.Y / Metrics.LayoutViewportSize.X));
		Metrics.PanelTouchOverlap = 0.f;
		Metrics.LeftPanelWidth = 548.f;
		Metrics.RightPanelWidth = 592.f;
		Metrics.CenterPanelX = 580.f;
		Metrics.CenterPreviewWidth = 714.f;
		Metrics.PartyFooterWidth = 664.f;
		Metrics.CompanionFooterWidth = 508.f;
		Metrics.CompanionFooterX = 708.f;
		Metrics.RunFooterX = 1252.f;
		Metrics.RunFooterWidth = Metrics.ReferenceLayoutWidth - Metrics.RunFooterX - 16.f;
		Metrics.CompanionFooterContentWidth = Metrics.CompanionFooterWidth - 20.f;
		Metrics.RunFooterContentWidth = Metrics.RunFooterWidth - 24.f;
		Metrics.UpperPanelY = 16.f;
		Metrics.FooterPanelMinHeight = 218.f;
		Metrics.FooterPanelY = Metrics.ReferenceLayoutHeight - Metrics.FooterPanelMinHeight - 58.f;
		Metrics.UpperSidePanelHeight = Metrics.FooterPanelY - Metrics.UpperPanelY - 36.f;
		Metrics.RightStatsCardHeight = 200.f;
		Metrics.RightUltRowHeight = 136.f;
		Metrics.PanelGap = FT66FlatStyle::FlatGap;
		Metrics.OuterPanelBleed = 0.f;
		Metrics.TopBarBottomGap = 8.f;
		Metrics.LayoutCompactScale = 1.f;
		Metrics.FooterToggleWidth = FMath::RoundToFloat((Metrics.CompanionFooterContentWidth - 8.f) * 0.5f);
		Metrics.FooterToggleHeight = Metrics.bShortViewport ? 68.f : 76.f;
		Metrics.FooterActionHeight = Metrics.bShortViewport ? 112.f : 136.f;
		Metrics.BalanceBadgeIconWidth = FMath::RoundToFloat(56.f * Metrics.LayoutCompactScale);
		Metrics.BalanceBadgeIconHeight = FMath::RoundToFloat(34.f * Metrics.LayoutCompactScale);
		Metrics.LeftSkinsCardHeight = Metrics.bShortViewport ? 340.f : 428.f;
		Metrics.RightPreviewPanelHeight = 72.f;
		Metrics.RightAbilityIconButtonSize = Metrics.bShortViewport ? 78.f : 90.f;
		Metrics.RightAbilityIconSize = Metrics.bShortViewport ? 58.f : 70.f;
		Metrics.ScreenHeaderFontSize = FMath::RoundToInt(21.f * Metrics.LayoutCompactScale);
		Metrics.BodyToggleFontSize = FMath::RoundToInt(21.f * Metrics.LayoutCompactScale);
		Metrics.PrimaryCtaFontSize = FMath::RoundToInt((Metrics.bShortViewport ? 22.f : 26.f) * Metrics.LayoutCompactScale);
		Metrics.HeroArrowFontSize = FMath::RoundToInt(24.f * Metrics.LayoutCompactScale);
		Metrics.ACBalanceFontSize = Metrics.ScreenHeaderFontSize + 2;
		Metrics.HeroNameFontSize = FMath::RoundToInt(31.f * Metrics.LayoutCompactScale);
		Metrics.SecondaryButtonFontSize = FMath::RoundToInt(20.f * Metrics.LayoutCompactScale);
		Metrics.EntityDropdownFontSize = FMath::RoundToInt(20.f * Metrics.LayoutCompactScale);
		Metrics.BodyTextFontSize = FMath::RoundToInt(16.f * Metrics.LayoutCompactScale);
		Metrics.DifficultyMenuFontSize = FMath::RoundToInt(23.f * Metrics.LayoutCompactScale);
		Metrics.HeroArrowButtonWidth = 64.f;
		Metrics.HeroArrowButtonHeight = Metrics.bShortViewport ? 42.f : 48.f;
		Metrics.TopStripBackButtonWidth = FMath::RoundToFloat(112.f * Metrics.LayoutCompactScale);
		Metrics.TopStripBackButtonHeight = Metrics.bShortViewport ? 32.f : 34.f;
		return Metrics;
	}

	inline void ResolveHeroSelectionLooseIconBrush(
		const FString& RelativePath,
		const FVector2D& ImageSize,
		TSharedPtr<FSlateBrush>& Brush,
		TStrongObjectPtr<UTexture2D>& Texture,
		const TCHAR* DebugName)
	{
		if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateBrush>();
			Brush->DrawAs = ESlateBrushDrawType::Image;
			Brush->Tiling = ESlateBrushTileType::NoTile;
			Brush->TintColor = FSlateColor(FLinearColor::White);
		}

		Brush->ImageSize = ImageSize;
		if (!Texture.IsValid())
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (!FPaths::FileExists(CandidatePath))
				{
					continue;
				}

				if (UTexture2D* LoadedTexture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					TextureFilter::TF_Trilinear,
					true,
					DebugName))
				{
					Texture.Reset(LoadedTexture);
					break;
				}

				if (UTexture2D* LoadedTexture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
					CandidatePath,
					TextureFilter::TF_Trilinear,
					DebugName))
				{
					Texture.Reset(LoadedTexture);
					break;
				}
			}
		}

		Brush->SetResourceObject(Texture.IsValid() ? Texture.Get() : nullptr);
	}

	inline float GetHeroSelectionCarouselBoxSize(const int32 Offset)
	{
		switch (FMath::Abs(Offset))
		{
		case 0: return 128.f;
		case 1: return 112.f;
		case 2: return 96.f;
		default: return 84.f;
		}
	}

	inline float GetHeroSelectionCarouselOpacity(const int32 Offset)
	{
		switch (FMath::Abs(Offset))
		{
		case 0: return 1.0f;
		case 1: return 0.90f;
		case 2: return 0.78f;
		case 3: return 0.66f;
		case 4: return 0.56f;
		default: return 0.48f;
		}
	}

	inline FString GetHeroSelectionBalanceIconPath()
	{
		return TEXT("RuntimeDependencies/T66/UI/Icons/Flat/chalice_grail.png");
	}

	inline FString MakeHeroSelectionReferenceAssetPath(const TCHAR* RelativePath)
	{
		return FString::Printf(
			TEXT("SourceAssets/UI/ContentStubs/HeroSelection/%s"),
			RelativePath ? RelativePath : TEXT(""));
	}

	inline FString MakeHeroSelectionUltrakillElementPath(const TCHAR* FileName)
	{
		return FString::Printf(
			TEXT("RuntimeDependencies/T66/UI/Icons/Flat/%s"),
			FileName ? FileName : TEXT(""));
	}

	inline FString MakeHeroSelectionSquareVariantElementPath(const TCHAR* FileName)
	{
		return FString::Printf(
			TEXT("RuntimeDependencies/T66/UI/Icons/Flat/%s"),
			FileName ? FileName : TEXT(""));
	}

	inline FSlateColor GetHeroSelectionParchmentText()
	{
		return FSlateColor(FT66FlatStyle::PrimaryText());
	}

	inline FSlateColor GetHeroSelectionParchmentMutedText()
	{
		return FSlateColor(FT66FlatStyle::PurpleAccent());
	}

	inline FString GetHeroSelectionRankImagePath()
	{
		return FPaths::ConvertRelativePathToFull(
			FPaths::ProjectDir() / TEXT("RuntimeDependencies/T66/UI/HeroSelection/Medals/rank_badge_imagegen_20260427_v1.png"));
	}

	inline FText GetHeroSelectionDrugName(const ET66SecondaryStatType StatType)
	{
		switch (StatType)
		{
		case ET66SecondaryStatType::AoeDamage: return NSLOCTEXT("T66.HeroSelection", "DrugAoeDamage", "OXYMETHOLONE");
		case ET66SecondaryStatType::BounceDamage: return NSLOCTEXT("T66.HeroSelection", "DrugBounceDamage", "METHANDROSTENOLONE");
		case ET66SecondaryStatType::PierceDamage: return NSLOCTEXT("T66.HeroSelection", "DrugPierceDamage", "FLUOXYMESTERONE");
		case ET66SecondaryStatType::DotDamage: return NSLOCTEXT("T66.HeroSelection", "DrugDotDamage", "NANDROLONE DECANOATE");
		case ET66SecondaryStatType::CritDamage: return NSLOCTEXT("T66.HeroSelection", "DrugCritDamage", "TRENBOLONE ACETATE");
		case ET66SecondaryStatType::AoeSpeed: return NSLOCTEXT("T66.HeroSelection", "DrugAoeSpeed", "CAFFEINE CITRATE");
		case ET66SecondaryStatType::BounceSpeed: return NSLOCTEXT("T66.HeroSelection", "DrugBounceSpeed", "MODAFINIL");
		case ET66SecondaryStatType::PierceSpeed: return NSLOCTEXT("T66.HeroSelection", "DrugPierceSpeed", "EPHEDRINE HCL");
		case ET66SecondaryStatType::DotSpeed: return NSLOCTEXT("T66.HeroSelection", "DrugDotSpeed", "SALBUTAMOL SULFATE");
		case ET66SecondaryStatType::CritChance: return NSLOCTEXT("T66.HeroSelection", "DrugCritChance", "STANOZOLOL");
		case ET66SecondaryStatType::AoeScale: return NSLOCTEXT("T66.HeroSelection", "DrugAoeScale", "TESTOSTERONE ENANTHATE");
		case ET66SecondaryStatType::BounceScale: return NSLOCTEXT("T66.HeroSelection", "DrugBounceScale", "BOLDENONE UNDECYLENATE");
		case ET66SecondaryStatType::PierceScale: return NSLOCTEXT("T66.HeroSelection", "DrugPierceScale", "DROSTANOLONE PROPIONATE");
		case ET66SecondaryStatType::DotScale: return NSLOCTEXT("T66.HeroSelection", "DrugDotScale", "METHENOLONE ENANTHATE");
		case ET66SecondaryStatType::AttackRange: return NSLOCTEXT("T66.HeroSelection", "DrugAttackRange", "CLENBUTEROL HCL");
		case ET66SecondaryStatType::Execute: return NSLOCTEXT("T66.HeroSelection", "DrugExecute", "ATOMOXETINE HCL");
		case ET66SecondaryStatType::Taunt: return NSLOCTEXT("T66.HeroSelection", "DrugTaunt", "HYDROCORTISONE");
		case ET66SecondaryStatType::DamageReduction: return NSLOCTEXT("T66.HeroSelection", "DrugDamageReduction", "PREDNISONE");
		case ET66SecondaryStatType::ReflectDamage: return NSLOCTEXT("T66.HeroSelection", "DrugReflectDamage", "DEXAMETHASONE");
		case ET66SecondaryStatType::Crush: return NSLOCTEXT("T66.HeroSelection", "DrugCrush", "BETAMETHASONE");
		case ET66SecondaryStatType::EvasionChance: return NSLOCTEXT("T66.HeroSelection", "DrugEvasionChance", "SCOPOLAMINE HBR");
		case ET66SecondaryStatType::CounterAttack: return NSLOCTEXT("T66.HeroSelection", "DrugCounterAttack", "LIDOCAINE HCL");
		case ET66SecondaryStatType::Invisibility: return NSLOCTEXT("T66.HeroSelection", "DrugInvisibility", "DIPHENHYDRAMINE HCL");
		case ET66SecondaryStatType::Assassinate: return NSLOCTEXT("T66.HeroSelection", "DrugAssassinate", "ATROPINE SULFATE");
		case ET66SecondaryStatType::TreasureChest: return NSLOCTEXT("T66.HeroSelection", "DrugTreasureChest", "NICOTINAMIDE RIBOSIDE");
		case ET66SecondaryStatType::Cheating: return NSLOCTEXT("T66.HeroSelection", "DrugCheating", "SILDENAFIL CITRATE");
		case ET66SecondaryStatType::Stealing: return NSLOCTEXT("T66.HeroSelection", "DrugStealing", "LOPERAMIDE HCL");
		case ET66SecondaryStatType::LootCrate: return NSLOCTEXT("T66.HeroSelection", "DrugLootCrate", "METFORMIN HCL");
		case ET66SecondaryStatType::LootBag: return NSLOCTEXT("T66.HeroSelection", "DrugLootBag", "UBIQUINOL");
		case ET66SecondaryStatType::LootWheel: return NSLOCTEXT("T66.HeroSelection", "DrugLootWheel", "THEOBROMINE");
		case ET66SecondaryStatType::Accuracy: return NSLOCTEXT("T66.HeroSelection", "DrugAccuracy", "ATOMOXETINE HCL");
		case ET66SecondaryStatType::VendorToken: return NSLOCTEXT("T66.HeroSelection", "DrugVendorToken", "VENDOR TOKEN");
		default: return NSLOCTEXT("T66.HeroSelection", "DrugFallback", "COMPOUND");
		}
	}

	inline FText GetHeroSelectionDrugEffectText(const ET66SecondaryStatType StatType, const UT66LocalizationSubsystem* Loc)
	{
		const FText StatName = Loc ? Loc->GetText_SecondaryStatName(StatType) : FText::FromString(TEXT("?"));
		const int32 Percent = FMath::RoundToInt((UT66BuffSubsystem::SingleUseSecondaryBuffMultiplier - 1.f) * 100.f);
		return FText::Format(
			NSLOCTEXT("T66.HeroSelection", "DrugEffectFormat", "+{0}% {1}"),
			FText::AsNumber(Percent),
			StatName);
	}

	inline FString GetHeroSelectionChadIconPath()
	{
		return TEXT("SourceAssets/UI/HeroSelection/Companions/companion_chad_male_blue.png");
	}

	inline FString GetHeroSelectionStacyIconPath()
	{
		return TEXT("SourceAssets/UI/HeroSelection/Companions/companion_stacy_female_pink.png");
	}

	inline bool HasUnlockedHeroSelectionDrugs(const UT66AchievementsSubsystem* Achievements)
	{
		if (!Achievements)
		{
			return false;
		}

		const UT66ProfileSaveGame* Profile = Achievements->GetProfile();
		if (!Profile)
		{
			return false;
		}

		const int32 RequiredTier = static_cast<int32>(UT66AchievementsSubsystem::MedalTierForDifficulty(ET66Difficulty::Easy));
		for (const TPair<FName, ET66AccountMedalTier>& Pair : Profile->HeroHighestMedalByID)
		{
			if (static_cast<int32>(Pair.Value) >= RequiredTier)
			{
				return true;
			}
		}

		return false;
	}

	inline FString HeroSelectionDifficultyToApiString(ET66Difficulty Difficulty)
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Easy: return TEXT("easy");
		case ET66Difficulty::Medium: return TEXT("medium");
		case ET66Difficulty::Hard: return TEXT("hard");
		case ET66Difficulty::VeryHard: return TEXT("veryhard");
		case ET66Difficulty::Impossible: return TEXT("impossible");
		default: return TEXT("easy");
		}
	}

	inline FString HeroSelectionPartySizeToApiString(const int32 PartySize)
	{
		switch (FMath::Clamp(PartySize, 1, 4))
		{
		case 2: return TEXT("duo");
		case 3: return TEXT("trio");
		case 4: return TEXT("quad");
		case 1:
		default:
			return TEXT("solo");
		}
	}

	inline TSoftObjectPtr<UTexture2D> ResolveHeroSelectionUltimateIcon(const FName HeroID, const ET66UltimateType UltimateType)
	{
		static_cast<void>(HeroID);
		if (UltimateType == ET66UltimateType::SpearStorm)
		{
			return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/Abilities/Hero_1/T_Hero_1_Ultimate.T_Hero_1_Ultimate")));
		}

		return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/ULTS/KnightULT.KnightULT")));
	}

	inline TSoftObjectPtr<UTexture2D> ResolveHeroSelectionPassiveIcon(const FName HeroID, const ET66PassiveType PassiveType)
	{
		static_cast<void>(HeroID);
		if (PassiveType == ET66PassiveType::IronWill)
		{
			return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/Abilities/Hero_1/T_Hero_1_Passive.T_Hero_1_Passive")));
		}

		return ResolveHeroSelectionUltimateIcon(HeroID, ET66UltimateType::None);
	}

	enum class ET66HeroSpriteFamily : uint8
	{
		CompactNeutral,
		ToggleOn,
		ToggleOff,
		ToggleInactive,
		CtaGreen,
		CtaBlue,
		CtaRed
	};

	struct FHeroSelectionSpriteBrushEntry
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
		bool bSimpleFallback = false;
	};

	struct FHeroSelectionSpriteBrushSet
	{
		FHeroSelectionSpriteBrushEntry Normal;
		FHeroSelectionSpriteBrushEntry Hover;
		FHeroSelectionSpriteBrushEntry Pressed;
		FHeroSelectionSpriteBrushEntry Disabled;
	};

	inline const FSlateBrush* ResolveHeroSelectionSpriteBrush(
		FHeroSelectionSpriteBrushEntry& Entry,
		const FString& RelativePath,
		const FVector2D& ImageSize,
		const FMargin& Margin = FMargin(0.12f, 0.28f, 0.12f, 0.28f),
		const ESlateBrushDrawType::Type DrawAs = ESlateBrushDrawType::Box,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		if (!Entry.Brush.IsValid())
		{
			Entry.Brush = MakeShared<FSlateBrush>();
			Entry.Brush->DrawAs = DrawAs;
			Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
			Entry.Brush->TintColor = FSlateColor(FLinearColor::White);
			Entry.Brush->ImageSize = ImageSize;
			Entry.Brush->Margin = Margin;
		}

		if (!Entry.Texture.IsValid() && !Entry.bSimpleFallback)
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					Filter,
					true,
					TEXT("HeroSelectionReferenceSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		if (Entry.Texture.IsValid())
		{
			Entry.bSimpleFallback = false;
			Entry.Brush->SetResourceObject(Entry.Texture.Get());
			return Entry.Brush.Get();
		}

		if (T66RuntimeUIBrushAccess::ShouldUseSimpleReferenceFallback(RelativePath))
		{
			Entry.bSimpleFallback = true;
			T66RuntimeUIBrushAccess::ConfigureSimpleReferenceFallbackBrush(
				*Entry.Brush,
				RelativePath,
				ImageSize,
				Margin,
				DrawAs);
			return Entry.Brush.Get();
		}

		Entry.bSimpleFallback = false;
		Entry.Brush->SetResourceObject(nullptr);
		return nullptr;
	}

	inline const FSlateBrush* ResolveHeroSelectionSpriteRegionBrush(
		FHeroSelectionSpriteBrushEntry& Entry,
		const FString& RelativePath,
		const FVector2D& ImageSize,
		const FMargin& Margin,
		const FBox2f& UVRegion,
		const FLinearColor& Tint,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		if (!Entry.Brush.IsValid())
		{
			Entry.Brush = MakeShared<FSlateBrush>();
		}

		Entry.Brush->DrawAs = ESlateBrushDrawType::Box;
		Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
		Entry.Brush->TintColor = FSlateColor(Tint);
		Entry.Brush->ImageSize = ImageSize;
		Entry.Brush->Margin = Margin;
		Entry.Brush->SetUVRegion(UVRegion);

		if (!Entry.Texture.IsValid() && !Entry.bSimpleFallback)
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					Filter,
					true,
					TEXT("HeroSelectionReferenceRegionSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		if (Entry.Texture.IsValid())
		{
			Entry.bSimpleFallback = false;
			Entry.Brush->SetResourceObject(Entry.Texture.Get());
			return Entry.Brush.Get();
		}

		if (T66RuntimeUIBrushAccess::ShouldUseSimpleReferenceFallback(RelativePath))
		{
			Entry.bSimpleFallback = true;
			T66RuntimeUIBrushAccess::ConfigureSimpleReferenceFallbackBrush(
				*Entry.Brush,
				RelativePath,
				ImageSize,
				Margin,
				ESlateBrushDrawType::Box);
			return Entry.Brush.Get();
		}

		Entry.bSimpleFallback = false;
		Entry.Brush->SetResourceObject(nullptr);
		return nullptr;
	}

	inline FHeroSelectionSpriteBrushSet& GetHeroSelectionButtonSpriteSet(ET66HeroSpriteFamily Family)
	{
		static FHeroSelectionSpriteBrushSet CompactNeutral;
		static FHeroSelectionSpriteBrushSet ToggleOn;
		static FHeroSelectionSpriteBrushSet ToggleOff;
		static FHeroSelectionSpriteBrushSet ToggleInactive;
		static FHeroSelectionSpriteBrushSet CtaGreen;
		static FHeroSelectionSpriteBrushSet CtaBlue;
		static FHeroSelectionSpriteBrushSet CtaRed;

		switch (Family)
		{
		case ET66HeroSpriteFamily::ToggleOn:
			return ToggleOn;
		case ET66HeroSpriteFamily::ToggleOff:
			return ToggleOff;
		case ET66HeroSpriteFamily::ToggleInactive:
			return ToggleInactive;
		case ET66HeroSpriteFamily::CtaGreen:
			return CtaGreen;
		case ET66HeroSpriteFamily::CtaBlue:
			return CtaBlue;
		case ET66HeroSpriteFamily::CtaRed:
			return CtaRed;
		case ET66HeroSpriteFamily::CompactNeutral:
		default:
			return CompactNeutral;
		}
	}

	inline FString GetHeroSelectionButtonSpritePath(ET66HeroSpriteFamily Family, ET66ButtonBorderState State)
	{
		const TCHAR* Suffix = TEXT("normal");
		switch (State)
		{
		case ET66ButtonBorderState::Hovered:
			Suffix = TEXT("hover");
			break;
		case ET66ButtonBorderState::Pressed:
			Suffix = TEXT("pressed");
			break;
		case ET66ButtonBorderState::Normal:
		default:
			break;
		}

		if (Family == ET66HeroSpriteFamily::ToggleOn)
		{
			return MakeHeroSelectionSquareVariantElementPath(TEXT("cta_new_game_button_normal_red_square_variant.png"));
		}
		if (Family == ET66HeroSpriteFamily::ToggleOff)
		{
			return MakeHeroSelectionSquareVariantElementPath(*FString::Printf(
				TEXT("cta_new_game_button_%s_red_square_variant.png"),
				Suffix));
		}
		if (Family == ET66HeroSpriteFamily::CtaGreen
			|| Family == ET66HeroSpriteFamily::CtaBlue
			|| Family == ET66HeroSpriteFamily::CtaRed)
		{
			return MakeHeroSelectionSquareVariantElementPath(*FString::Printf(
				TEXT("cta_new_game_button_%s_red_square_variant.png"),
				Suffix));
		}

		return MakeHeroSelectionSquareVariantElementPath(*FString::Printf(
			TEXT("cta_new_game_button_%s_red_square_variant.png"),
			Suffix));
	}

	inline FVector2D GetHeroSelectionButtonSpriteSize(ET66HeroSpriteFamily /*Family*/, ET66ButtonBorderState /*State*/)
	{
		return FVector2D(160.f, 66.f);
	}

	inline const FSlateBrush* ResolveHeroSelectionButtonSpriteBrush(ET66HeroSpriteFamily Family, ET66ButtonBorderState State)
	{
		FHeroSelectionSpriteBrushSet& Set = GetHeroSelectionButtonSpriteSet(Family);
		FHeroSelectionSpriteBrushEntry* Entry = &Set.Normal;
		if (State == ET66ButtonBorderState::Hovered)
		{
			Entry = &Set.Hover;
		}
		else if (State == ET66ButtonBorderState::Pressed)
		{
			Entry = &Set.Pressed;
		}

		return ResolveHeroSelectionSpriteBrush(
			*Entry,
			GetHeroSelectionButtonSpritePath(Family, State),
			GetHeroSelectionButtonSpriteSize(Family, State),
			FMargin(0.180f, 0.240f, 0.180f, 0.240f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	inline const FSlateBrush* ResolveHeroSelectionDisabledButtonSpriteBrush()
	{
		FHeroSelectionSpriteBrushSet& Set = GetHeroSelectionButtonSpriteSet(ET66HeroSpriteFamily::ToggleInactive);
		return ResolveHeroSelectionSpriteBrush(
			Set.Disabled,
			MakeHeroSelectionSquareVariantElementPath(TEXT("cta_new_game_button_disabled_red_square_variant.png")),
			FVector2D(145.f, 66.f),
			FMargin(0.180f, 0.240f, 0.180f, 0.240f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	inline ET66HeroSpriteFamily GetDefaultHeroSelectionButtonFamily(ET66ButtonType Type)
	{
		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return ET66HeroSpriteFamily::ToggleOn;
		case ET66ButtonType::Danger:
			return ET66HeroSpriteFamily::ToggleOff;
		case ET66ButtonType::Neutral:
		case ET66ButtonType::Row:
		default:
			return ET66HeroSpriteFamily::CompactNeutral;
		}
	}

	inline const FSlateBrush* GetHeroSelectionLargeShellBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			MakeHeroSelectionSquareVariantElementPath(TEXT("main_panel_normal_square_variant.png")),
			FVector2D(300.f, 548.f),
			FMargin(0.105f, 0.055f, 0.105f, 0.055f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	inline const FSlateBrush* GetHeroSelectionRightShellBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			MakeHeroSelectionSquareVariantElementPath(TEXT("main_panel_normal_square_variant.png")),
			FVector2D(300.f, 548.f),
			FMargin(0.105f, 0.055f, 0.105f, 0.055f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	inline const FSlateBrush* GetHeroSelectionContentShellBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			MakeHeroSelectionSquareVariantElementPath(TEXT("player_row_panel_normal_square_variant.png")),
			FVector2D(1347.f, 120.f),
			FMargin(0.040f, 0.250f, 0.040f, 0.250f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	inline const FSlateBrush* GetHeroSelectionRowShellBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			MakeHeroSelectionSquareVariantElementPath(TEXT("player_row_panel_normal_square_variant.png")),
			FVector2D(569.f, 95.f),
			FMargin(0.070f, 0.240f, 0.070f, 0.240f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	inline const FSlateBrush* GetHeroSelectionParchmentPanelBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			MakeHeroSelectionSquareVariantElementPath(TEXT("main_panel_normal_square_variant.png")),
			FVector2D(568.f, 287.f),
			FMargin(0.075f, 0.125f, 0.075f, 0.125f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	inline const FSlateBrush* GetHeroSelectionParchmentRowBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			MakeHeroSelectionSquareVariantElementPath(TEXT("player_row_panel_normal_square_variant.png")),
			FVector2D(569.f, 95.f),
			FMargin(0.070f, 0.240f, 0.070f, 0.240f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	inline const FSlateBrush* GetHeroSelectionCarouselSlotBrush(const bool bSelected)
	{
		static FHeroSelectionSpriteBrushEntry NormalEntry;
		static FHeroSelectionSpriteBrushEntry SelectedEntry;
		return ResolveHeroSelectionSpriteBrush(
			bSelected ? SelectedEntry : NormalEntry,
			FString::Printf(
				TEXT("RuntimeDependencies/T66/UI/Icons/Flat/portrait_slot_%s.png"),
				bSelected ? TEXT("selected") : TEXT("normal")),
			bSelected ? FVector2D(111.f, 79.f) : FVector2D(100.f, 75.f),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	inline const FSlateBrush* GetHeroSelectionSquareButtonBrush(const ET66ButtonBorderState State, const bool bSelected = false)
	{
		static FHeroSelectionSpriteBrushEntry NormalEntry;
		static FHeroSelectionSpriteBrushEntry HoverEntry;
		static FHeroSelectionSpriteBrushEntry PressedEntry;
		static FHeroSelectionSpriteBrushEntry SelectedEntry;

		const TCHAR* Suffix = TEXT("normal");
		FHeroSelectionSpriteBrushEntry* Entry = &NormalEntry;
		if (bSelected)
		{
			Suffix = TEXT("selected");
			Entry = &SelectedEntry;
		}
		else if (State == ET66ButtonBorderState::Hovered)
		{
			Suffix = TEXT("hover");
			Entry = &HoverEntry;
		}
		else if (State == ET66ButtonBorderState::Pressed)
		{
			Suffix = TEXT("pressed");
			Entry = &PressedEntry;
		}
		return ResolveHeroSelectionSpriteBrush(
			*Entry,
			MakeHeroSelectionSquareVariantElementPath(*FString::Printf(
				TEXT("topbar_icon_button_%s_square_variant.png"),
				Suffix)),
			bSelected ? FVector2D(111.f, 79.f) : FVector2D(100.f, 75.f),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	inline const FScrollBarStyle* GetHeroSelectionFlatScrollBarStyle()
	{
		static FScrollBarStyle Style = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar");
		Style.SetThickness(10.f);
		return &Style;
	}

	inline const FSlateBrush* GetHeroSelectionDropdownFieldBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			MakeHeroSelectionSquareVariantElementPath(TEXT("dropdown_field_normal_square_variant.png")),
			FVector2D(452.f, 75.f),
			FMargin(0.130f, 0.235f, 0.130f, 0.235f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	inline const FSlateBrush* GetHeroSelectionPartySlotBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			MakeHeroSelectionSquareVariantElementPath(TEXT("profile_slot_normal_square_variant.png")),
			FVector2D(73.f, 73.f),
			FMargin(0.20f, 0.18f, 0.20f, 0.18f));
	}

	inline TSharedRef<SWidget> MakeHeroSelectionPanelShell(
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding,
		const bool bRightShell = false)
	{
		static_cast<void>(bRightShell);
		return FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			Padding,
			Content);
	}

	inline TSharedRef<SWidget> MakeHeroSelectionContentShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		return FT66FlatStyle::MakeFlatSubPanel(
			ET66FlatState::Default,
			Padding,
			Content);
	}

	inline TSharedRef<SWidget> MakeHeroSelectionParchmentPanelShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		return FT66FlatStyle::MakeFlatSubPanel(
			ET66FlatState::Default,
			Padding,
			Content);
	}

	inline TSharedRef<SWidget> MakeHeroSelectionParchmentRowShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		return FT66FlatStyle::MakeFlatSubPanel(
			ET66FlatState::Default,
			Padding,
			Content);
	}

	inline TSharedRef<SWidget> MakeHeroSelectionRowShell(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(12.f, 8.f))
	{
		return FT66FlatStyle::MakeFlatSubPanel(
			ET66FlatState::Default,
			Padding,
			Content);
	}

	inline TSharedRef<SWidget> MakeHeroSelectionSpriteButton(
		const FT66ButtonParams& Params,
		TAttribute<ET66HeroSpriteFamily> SpriteFamily)
	{
		static_cast<void>(SpriteFamily);
		const ET66FlatState FlatState = !Params.IsEnabled.Get(true)
			? ET66FlatState::Disabled
			: ((Params.Type == ET66ButtonType::Primary
				|| Params.Type == ET66ButtonType::Danger
				|| Params.Type == ET66ButtonType::Success
				|| Params.Type == ET66ButtonType::ToggleActive)
				? ET66FlatState::Selected
				: ET66FlatState::Default);
		const float ButtonHeight = Params.Height > 0.f ? Params.Height : 44.f;
		const FMargin ContentPadding = Params.Padding.Left >= 0.f ? Params.Padding : FMargin(6.f, 2.f);
		const int32 ButtonFontSize = Params.FontSize > 0 ? Params.FontSize : 20;

		if (!Params.CustomContent.IsValid())
		{
			const TAttribute<FText> LabelAttribute = Params.DynamicLabel.IsSet()
				? Params.DynamicLabel
				: TAttribute<FText>(Params.Label);
			return FT66FlatStyle::MakeFlatButton(
				FlatState,
				LabelAttribute,
				Params.OnClicked,
				nullptr,
				nullptr,
				ContentPadding,
				Params.MinWidth,
				ButtonHeight,
				Params.IsEnabled,
				ButtonFontSize);
		}

		return SNew(SBox)
			.Visibility(Params.Visibility)
			[
				FT66FlatStyle::MakeFlatToggleGroupButton(
					FlatState,
					Params.CustomContent.ToSharedRef(),
					Params.OnClicked,
					ContentPadding,
					Params.MinWidth,
					ButtonHeight,
					Params.IsEnabled)
			];
	}

	inline TSharedRef<SWidget> MakeHeroSelectionButton(const FT66ButtonParams& Params)
	{
		return MakeHeroSelectionSpriteButton(
			Params,
			TAttribute<ET66HeroSpriteFamily>(GetDefaultHeroSelectionButtonFamily(Params.Type)));
	}

	inline TSharedRef<SWidget> MakeHeroSelectionButton(
		const FText& Label,
		FOnClicked OnClicked,
		ET66ButtonType Type = ET66ButtonType::Neutral,
		float MinWidth = 120.f)
	{
		return MakeHeroSelectionButton(FT66ButtonParams(Label, MoveTemp(OnClicked), Type).SetMinWidth(MinWidth));
	}

	inline TSharedRef<SWidget> MakeHeroSelectionFittedLabel(
		const FText& Label,
		const int32 FontSize,
		const FSlateColor& Color,
		const ETextJustify::Type Justification = ETextJustify::Center,
		const EHorizontalAlignment HorizontalAlignment = HAlign_Center)
	{
		return SNew(SBox)
			.HAlign(HorizontalAlignment)
			.VAlign(VAlign_Center)
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::DownOnly)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(FT66Style::Tokens::FontBold(FontSize))
					.ColorAndOpacity(Color)
					.Justification(Justification)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
			];
	}

	inline TSharedRef<SWidget> MakeHeroSelectionDropdown(const FT66DropdownParams& Params)
	{
		static FComboButtonStyle FlatComboStyle = []()
		{
			FComboButtonStyle Style = FCoreStyle::Get().GetWidgetStyle<FComboButtonStyle>("ComboButton");
			Style.ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			return Style;
		}();

		TSharedRef<SComboButton> Combo = SNew(SComboButton)
			.ComboButtonStyle(&FlatComboStyle)
			.MenuPlacement(MenuPlacement_BelowAnchor)
			.HasDownArrow(false)
			.OnGetMenuContent_Lambda([OnGet = Params.OnGetMenuContent]()
			{
				return MakeHeroSelectionContentShell(OnGet(), FMargin(4.f));
			})
			.ContentPadding(Params.Padding)
			.ButtonContent()
			[
				SNew(SBox)
				.HeightOverride(Params.Height > 0.f ? FOptionalSize(FMath::Max(1.f, Params.Height - 4.f)) : FOptionalSize())
				.VAlign(VAlign_Center)
				[
					Params.Content
				]
			];

		return SNew(SBox)
			.MinDesiredWidth(Params.MinWidth > 0.f ? FOptionalSize(Params.MinWidth) : FOptionalSize())
			.HeightOverride(Params.Height > 0.f ? FOptionalSize(Params.Height) : FOptionalSize())
			.Visibility(Params.Visibility)
			[
				FT66FlatStyle::MakeFlatPanel(ET66FlatState::Selected, FMargin(4.f, 0.f), Combo)
			];
	}

	inline TSharedRef<SToolTip> MakeHeroSelectionAbilityTooltip(const FText& Title, const FText& Description, const int32 FontSizeAdjustment = 0)
	{
		return SNew(SToolTip)
			[
				FT66FlatStyle::MakeFlatPanel(
					ET66FlatState::Default,
					FMargin(12.f, 10.f),
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
					])
			];
	}

	inline FText FormatCompanionHealingPerSecondText(const float HealingPerSecond)
	{
		return FText::Format(
			NSLOCTEXT("T66.HeroSelection", "CompanionHealingPerSecondFormat", "Heals / Second: {0}"),
			FText::AsNumber(FMath::RoundToInt(HealingPerSecond)));
	}

	inline FText FormatCompanionDifficultyHealText(const float HealAmount, const float HealIntervalSeconds)
	{
		return FText::Format(
			NSLOCTEXT("T66.HeroSelection", "CompanionDifficultyHealFormat", "Heals: {0} HP every {1}s"),
			FText::AsNumber(FMath::RoundToInt(HealAmount)),
			FText::AsNumber(HealIntervalSeconds));
	}
}
