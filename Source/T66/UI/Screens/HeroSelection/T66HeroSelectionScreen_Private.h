// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/Screens/HeroSelection/T66HeroSelectionPreviewController.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
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
#include "UI/Style/T66ButtonVisuals.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66RuntimeUIFontAccess.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Input/SComboButton.h"

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

	inline FString GetHeroSelectionBalanceIconPath()
	{
		return TEXT("SourceAssets/UI/MasterLibrary/Slices/IconsGenerated/icon_07_coupon_ticket_imagegen_20260425_v2.png");
	}

	inline FString GetHeroSelectionMedalImagePath(const ET66AccountMedalTier Tier)
	{
		const FString MedalDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("RuntimeDependencies/T66/UI/HeroSelection/Medals/"));
		switch (Tier)
		{
		case ET66AccountMedalTier::Bronze:
			return MedalDir / TEXT("medal_easy_bronze_imagegen_20260427_v1.png");
		case ET66AccountMedalTier::Silver:
			return MedalDir / TEXT("medal_medium_silver_imagegen_20260427_v1.png");
		case ET66AccountMedalTier::Gold:
			return MedalDir / TEXT("medal_hard_gold_imagegen_20260427_v1.png");
		case ET66AccountMedalTier::Platinum:
			return MedalDir / TEXT("medal_veryhard_platinum_imagegen_20260427_v1.png");
		case ET66AccountMedalTier::Diamond:
			return MedalDir / TEXT("medal_impossible_diamond_imagegen_20260427_v1.png");
		case ET66AccountMedalTier::None:
		default:
			return MedalDir / TEXT("medal_unproven_imagegen_20260427_v2.png");
		}
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
		case ET66SecondaryStatType::AoeDamage: return NSLOCTEXT("T66.HeroSelection", "DrugAoeDamage", "Blast Caps");
		case ET66SecondaryStatType::BounceDamage: return NSLOCTEXT("T66.HeroSelection", "DrugBounceDamage", "Rebound Pills");
		case ET66SecondaryStatType::PierceDamage: return NSLOCTEXT("T66.HeroSelection", "DrugPierceDamage", "Needle Rush");
		case ET66SecondaryStatType::DotDamage: return NSLOCTEXT("T66.HeroSelection", "DrugDotDamage", "Venom Tonic");
		case ET66SecondaryStatType::CritDamage: return NSLOCTEXT("T66.HeroSelection", "DrugCritDamage", "Redline Amp");
		case ET66SecondaryStatType::AoeSpeed: return NSLOCTEXT("T66.HeroSelection", "DrugAoeSpeed", "Blast Caffeine");
		case ET66SecondaryStatType::BounceSpeed: return NSLOCTEXT("T66.HeroSelection", "DrugBounceSpeed", "Ricochet Tabs");
		case ET66SecondaryStatType::PierceSpeed: return NSLOCTEXT("T66.HeroSelection", "DrugPierceSpeed", "Needle Sprint");
		case ET66SecondaryStatType::DotSpeed: return NSLOCTEXT("T66.HeroSelection", "DrugDotSpeed", "Toxin Drip");
		case ET66SecondaryStatType::CritChance: return NSLOCTEXT("T66.HeroSelection", "DrugCritChance", "Lucky Dust");
		case ET66SecondaryStatType::AoeScale: return NSLOCTEXT("T66.HeroSelection", "DrugAoeScale", "Blast Stretch");
		case ET66SecondaryStatType::BounceScale: return NSLOCTEXT("T66.HeroSelection", "DrugBounceScale", "Rebound Foam");
		case ET66SecondaryStatType::PierceScale: return NSLOCTEXT("T66.HeroSelection", "DrugPierceScale", "Needle Bloom");
		case ET66SecondaryStatType::DotScale: return NSLOCTEXT("T66.HeroSelection", "DrugDotScale", "Toxic Bloom");
		case ET66SecondaryStatType::AttackRange: return NSLOCTEXT("T66.HeroSelection", "DrugAttackRange", "Longshot Serum");
		case ET66SecondaryStatType::Taunt: return NSLOCTEXT("T66.HeroSelection", "DrugTaunt", "Loudmouth Brew");
		case ET66SecondaryStatType::DamageReduction: return NSLOCTEXT("T66.HeroSelection", "DrugDamageReduction", "Stone Skin");
		case ET66SecondaryStatType::ReflectDamage: return NSLOCTEXT("T66.HeroSelection", "DrugReflectDamage", "Mirror Dose");
		case ET66SecondaryStatType::Crush: return NSLOCTEXT("T66.HeroSelection", "DrugCrush", "Crusher Shot");
		case ET66SecondaryStatType::EvasionChance: return NSLOCTEXT("T66.HeroSelection", "DrugEvasionChance", "Ghost Step");
		case ET66SecondaryStatType::CounterAttack: return NSLOCTEXT("T66.HeroSelection", "DrugCounterAttack", "Payback Tabs");
		case ET66SecondaryStatType::Invisibility: return NSLOCTEXT("T66.HeroSelection", "DrugInvisibility", "Fade Powder");
		case ET66SecondaryStatType::Assassinate: return NSLOCTEXT("T66.HeroSelection", "DrugAssassinate", "Killer Focus");
		case ET66SecondaryStatType::TreasureChest: return NSLOCTEXT("T66.HeroSelection", "DrugTreasureChest", "Treasure Tonic");
		case ET66SecondaryStatType::Cheating: return NSLOCTEXT("T66.HeroSelection", "DrugCheating", "Loaded Dice");
		case ET66SecondaryStatType::Stealing: return NSLOCTEXT("T66.HeroSelection", "DrugStealing", "Sticky Fingers");
		case ET66SecondaryStatType::LootCrate: return NSLOCTEXT("T66.HeroSelection", "DrugLootCrate", "Crate Cracker");
		case ET66SecondaryStatType::Accuracy: return NSLOCTEXT("T66.HeroSelection", "DrugAccuracy", "True Aim");
		default: return NSLOCTEXT("T66.HeroSelection", "DrugFallback", "Combat Dose");
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

	inline FString GetHeroSelectionChallengesIconPath()
	{
		return TEXT("RuntimeDependencies/T66/UI/HeroSelection/challenges_crossed_swords.png");
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

	enum class ET66HeroSpriteFamily : uint8
	{
		CompactNeutral,
		ToggleOn,
		ToggleOff,
		ToggleInactive,
		CtaGreen,
		CtaBlue,
		CtaPurple
	};

	struct FHeroSelectionSpriteBrushEntry
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
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
		const ESlateBrushDrawType::Type DrawAs = ESlateBrushDrawType::Box)
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

		if (!Entry.Texture.IsValid())
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					TextureFilter::TF_Trilinear,
					true,
					TEXT("HeroSelectionReferenceSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		Entry.Brush->SetResourceObject(Entry.Texture.IsValid() ? Entry.Texture.Get() : nullptr);
		return Entry.Texture.IsValid() ? Entry.Brush.Get() : nullptr;
	}

	inline FHeroSelectionSpriteBrushSet& GetHeroSelectionButtonSpriteSet(ET66HeroSpriteFamily Family)
	{
		static FHeroSelectionSpriteBrushSet CompactNeutral;
		static FHeroSelectionSpriteBrushSet ToggleOn;
		static FHeroSelectionSpriteBrushSet ToggleOff;
		static FHeroSelectionSpriteBrushSet ToggleInactive;
		static FHeroSelectionSpriteBrushSet CtaGreen;
		static FHeroSelectionSpriteBrushSet CtaBlue;
		static FHeroSelectionSpriteBrushSet CtaPurple;

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
		case ET66HeroSpriteFamily::CtaPurple:
			return CtaPurple;
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

		if (Family == ET66HeroSpriteFamily::ToggleOn && State == ET66ButtonBorderState::Normal)
		{
			Suffix = TEXT("pressed");
		}

		return FString::Printf(TEXT("SourceAssets/UI/MasterLibrary/Slices/TopBar/topbar_nav_%s.png"), Suffix);
	}

	inline FVector2D GetHeroSelectionButtonSpriteSize(ET66HeroSpriteFamily /*Family*/, ET66ButtonBorderState /*State*/)
	{
		return FVector2D(270.f, 88.f);
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
			FMargin(0.093f, 0.213f, 0.093f, 0.213f));
	}

	inline const FSlateBrush* ResolveHeroSelectionDisabledButtonSpriteBrush()
	{
		FHeroSelectionSpriteBrushSet& Set = GetHeroSelectionButtonSpriteSet(ET66HeroSpriteFamily::ToggleInactive);
		return ResolveHeroSelectionSpriteBrush(
			Set.Disabled,
			TEXT("SourceAssets/UI/MasterLibrary/Slices/TopBar/topbar_nav_disabled.png"),
			FVector2D(270.f, 88.f),
			FMargin(0.093f, 0.213f, 0.093f, 0.213f));
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
			TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/panel_large_normal.png"),
			FVector2D(480.f, 800.f),
			FMargin(0.067f, 0.043f, 0.067f, 0.043f));
	}

	inline const FSlateBrush* GetHeroSelectionRightShellBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/panel_large_normal.png"),
			FVector2D(480.f, 800.f),
			FMargin(0.067f, 0.043f, 0.067f, 0.043f));
	}

	inline const FSlateBrush* GetHeroSelectionContentShellBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/panel_large_normal.png"),
			FVector2D(480.f, 800.f),
			FMargin(0.067f, 0.043f, 0.067f, 0.043f));
	}

	inline const FSlateBrush* GetHeroSelectionRowShellBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/modal_frame_normal.png"),
			FVector2D(620.f, 340.f),
			FMargin(0.052f, 0.094f, 0.052f, 0.094f));
	}

	inline const FSlateBrush* GetHeroSelectionDropdownFieldBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/Slices/Controls/dropdown_field_normal.png"),
			FVector2D(218.f, 50.f),
			FMargin(0.06f, 0.34f, 0.06f, 0.34f));
	}

	inline const FSlateBrush* GetHeroSelectionPartySlotBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/Slices/Slots/party_slot_normal.png"),
			FVector2D(112.f, 112.f),
			FMargin(0.20f, 0.18f, 0.20f, 0.18f));
	}

	class ST66HeroSelectionSpriteButton : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66HeroSelectionSpriteButton)
			: _SpriteFamily(ET66HeroSpriteFamily::CompactNeutral)
			, _MinWidth(0.f)
			, _Height(0.f)
			, _ContentPadding(FMargin(0.f))
			, _IsEnabled(true)
			, _Visibility(EVisibility::Visible)
		{}
			SLATE_ATTRIBUTE(ET66HeroSpriteFamily, SpriteFamily)
			SLATE_ARGUMENT(float, MinWidth)
			SLATE_ARGUMENT(float, Height)
			SLATE_ARGUMENT(FMargin, ContentPadding)
			SLATE_ARGUMENT(TAttribute<bool>, IsEnabled)
			SLATE_ARGUMENT(TAttribute<EVisibility>, Visibility)
			SLATE_EVENT(FOnClicked, OnClicked)
			SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			SpriteFamily = InArgs._SpriteFamily;
			ContentPadding = InArgs._ContentPadding;
			FButtonStyle ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			ButtonStyle.SetNormalPadding(FMargin(0.f));
			ButtonStyle.SetPressedPadding(FMargin(0.f));
			OwnedButtonStyle = ButtonStyle;

			ChildSlot
			[
				FT66Style::MakeBareButton(
					FT66BareButtonParams(
						InArgs._OnClicked,
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SImage)
							.Image(this, &ST66HeroSelectionSpriteButton::GetCurrentBrush)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Padding(this, &ST66HeroSelectionSpriteButton::GetContentPadding)
							[
								InArgs._Content.Widget
							]
						])
					.SetButtonStyle(&OwnedButtonStyle)
					.SetPadding(FMargin(0.f))
					.SetEnabled(InArgs._IsEnabled)
					.SetWidth(InArgs._MinWidth)
					.SetHeight(InArgs._Height)
					.SetVisibility(InArgs._Visibility),
					&Button)
			];
		}

	private:
		const FSlateBrush* GetCurrentBrush() const
		{
			if (!Button.IsValid() || !Button->IsEnabled())
			{
				return ResolveHeroSelectionDisabledButtonSpriteBrush();
			}

			const ET66HeroSpriteFamily Family = SpriteFamily.Get(ET66HeroSpriteFamily::CompactNeutral);
			if (Button->IsPressed())
			{
				return ResolveHeroSelectionButtonSpriteBrush(Family, ET66ButtonBorderState::Pressed);
			}
			if (Button->IsHovered())
			{
				return ResolveHeroSelectionButtonSpriteBrush(Family, ET66ButtonBorderState::Hovered);
			}
			return ResolveHeroSelectionButtonSpriteBrush(Family, ET66ButtonBorderState::Normal);
		}

		FMargin GetContentPadding() const
		{
			if (Button.IsValid() && Button->IsPressed())
			{
				return FMargin(ContentPadding.Left, ContentPadding.Top + 1.f, ContentPadding.Right, FMath::Max(0.f, ContentPadding.Bottom - 1.f));
			}
			return ContentPadding;
		}

		TAttribute<ET66HeroSpriteFamily> SpriteFamily;
		FMargin ContentPadding = FMargin(0.f);
		FButtonStyle OwnedButtonStyle;
		TSharedPtr<SButton> Button;
	};

	inline TSharedRef<SWidget> MakeHeroSelectionPanelShell(
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding,
		const bool bRightShell = false)
	{
		return SNew(SBorder)
			.BorderImage(bRightShell ? GetHeroSelectionRightShellBrush() : GetHeroSelectionLargeShellBrush())
			.BorderBackgroundColor(FLinearColor::White)
			.Padding(Padding)
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				Content
			];
	}

	inline TSharedRef<SWidget> MakeHeroSelectionContentShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		return SNew(SBorder)
			.BorderImage(GetHeroSelectionContentShellBrush())
			.BorderBackgroundColor(FLinearColor::White)
			.Padding(Padding)
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				Content
			];
	}

	inline TSharedRef<SWidget> MakeHeroSelectionRowShell(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(12.f, 8.f))
	{
		return SNew(SBorder)
			.BorderImage(GetHeroSelectionRowShellBrush())
			.BorderBackgroundColor(FLinearColor::White)
			.Padding(Padding)
			[
				Content
			];
	}

	inline TSharedRef<SWidget> MakeHeroSelectionSpriteButton(
		const FT66ButtonParams& Params,
		TAttribute<ET66HeroSpriteFamily> SpriteFamily)
	{
		const float ButtonHeight = Params.Height > 0.f ? Params.Height : 44.f;
		const FMargin ContentPadding = Params.Padding.Left >= 0.f ? Params.Padding : FMargin(6.f, 2.f);

		const TSharedRef<SWidget> Content = Params.CustomContent.IsValid()
			? Params.CustomContent.ToSharedRef()
			: T66ScreenSlateHelpers::MakeFilledButtonText(
				Params,
				ButtonHeight,
				TAttribute<FSlateColor>(FSlateColor(FT66Style::Text())),
				TAttribute<FLinearColor>(FLinearColor(0.f, 0.f, 0.f, 0.70f)));

		return SNew(ST66HeroSelectionSpriteButton)
			.SpriteFamily(SpriteFamily)
			.MinWidth(Params.MinWidth)
			.Height(ButtonHeight)
			.ContentPadding(ContentPadding)
			.IsEnabled(Params.IsEnabled)
			.Visibility(Params.Visibility)
			.OnClicked(Params.OnClicked)
			[
				Content
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

	inline TSharedRef<SWidget> MakeHeroSelectionDropdown(const FT66DropdownParams& Params)
	{
		static FComboButtonStyle FlatComboStyle = []()
		{
			FComboButtonStyle Style = FT66Style::GetDropdownComboButtonStyle();
			Style.ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			return Style;
		}();

		TSharedRef<SComboButton> Combo = SNew(SComboButton)
			.ComboButtonStyle(&FlatComboStyle)
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
			.MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize())
			.HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize())
			.Visibility(Params.Visibility)
			[
				SNew(SBorder)
				.BorderImage(GetHeroSelectionDropdownFieldBrush())
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(FMargin(4.f, 0.f))
				[
					Combo
				]
			];
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

	inline FText FormatCompanionHealingPerSecondText(const float HealingPerSecond)
	{
		return FText::Format(
			NSLOCTEXT("T66.HeroSelection", "CompanionHealingPerSecondFormat", "Heals / Second: {0}"),
			FText::AsNumber(FMath::RoundToInt(HealingPerSecond)));
	}
}
