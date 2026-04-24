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
		const TCHAR* Prefix = TEXT("settings_compact_neutral");
		const TCHAR* BasePath = TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center");
		switch (Family)
		{
		case ET66HeroSpriteFamily::ToggleOn:
			Prefix = TEXT("settings_toggle_on");
			break;
		case ET66HeroSpriteFamily::ToggleOff:
			Prefix = TEXT("settings_toggle_off");
			break;
		case ET66HeroSpriteFamily::ToggleInactive:
			Prefix = TEXT("settings_toggle_inactive");
			break;
		case ET66HeroSpriteFamily::CtaGreen:
			BasePath = TEXT("SourceAssets/UI/MainMenuReference/Center");
			Prefix = TEXT("cta_button_new_game_plate");
			break;
		case ET66HeroSpriteFamily::CtaBlue:
			BasePath = TEXT("SourceAssets/UI/MainMenuReference/Center");
			Prefix = TEXT("cta_button_load_game_plate");
			break;
		case ET66HeroSpriteFamily::CtaPurple:
			BasePath = TEXT("SourceAssets/UI/MainMenuReference/Center");
			Prefix = TEXT("cta_button_daily_challenge_plate");
			break;
		case ET66HeroSpriteFamily::CompactNeutral:
		default:
			break;
		}

		if (Family == ET66HeroSpriteFamily::CtaGreen || Family == ET66HeroSpriteFamily::CtaBlue || Family == ET66HeroSpriteFamily::CtaPurple)
		{
			switch (State)
			{
			case ET66ButtonBorderState::Hovered:
				return FString::Printf(TEXT("%s/%s_hover.png"), BasePath, Prefix);
			case ET66ButtonBorderState::Pressed:
				return FString::Printf(TEXT("%s/%s_pressed.png"), BasePath, Prefix);
			case ET66ButtonBorderState::Normal:
			default:
				return FString::Printf(TEXT("%s/%s.png"), BasePath, Prefix);
			}
		}

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

		return FString::Printf(TEXT("%s/%s_%s.png"), BasePath, Prefix, Suffix);
	}

	inline FVector2D GetHeroSelectionButtonSpriteSize(ET66HeroSpriteFamily Family, ET66ButtonBorderState State)
	{
		switch (Family)
		{
		case ET66HeroSpriteFamily::CtaGreen:
			return FVector2D(388.f, 100.f);
		case ET66HeroSpriteFamily::CtaBlue:
			return FVector2D(388.f, 97.f);
		case ET66HeroSpriteFamily::CtaPurple:
			return FVector2D(388.f, 92.f);
		case ET66HeroSpriteFamily::ToggleOn:
			return State == ET66ButtonBorderState::Pressed ? FVector2D(187.f, 67.f) : FVector2D(180.f, 68.f);
		case ET66HeroSpriteFamily::ToggleOff:
			return State == ET66ButtonBorderState::Pressed ? FVector2D(186.f, 68.f) : FVector2D(180.f, 68.f);
		case ET66HeroSpriteFamily::ToggleInactive:
			return State == ET66ButtonBorderState::Hovered ? FVector2D(186.f, 69.f) : FVector2D(180.f, 68.f);
		case ET66HeroSpriteFamily::CompactNeutral:
		default:
			return State == ET66ButtonBorderState::Pressed ? FVector2D(186.f, 68.f) : FVector2D(180.f, 68.f);
		}
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

		const FMargin ButtonMargin = (Family == ET66HeroSpriteFamily::CtaGreen || Family == ET66HeroSpriteFamily::CtaBlue || Family == ET66HeroSpriteFamily::CtaPurple)
			? FMargin(0.16f, 0.30f, 0.16f, 0.30f)
			: FMargin(0.14f, 0.30f, 0.14f, 0.30f);
		return ResolveHeroSelectionSpriteBrush(
			*Entry,
			GetHeroSelectionButtonSpritePath(Family, State),
			GetHeroSelectionButtonSpriteSize(Family, State),
			ButtonMargin);
	}

	inline const FSlateBrush* ResolveHeroSelectionDisabledButtonSpriteBrush()
	{
		FHeroSelectionSpriteBrushSet& Set = GetHeroSelectionButtonSpriteSet(ET66HeroSpriteFamily::ToggleInactive);
		return ResolveHeroSelectionSpriteBrush(
			Set.Disabled,
			TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_toggle_inactive_normal.png"),
			FVector2D(180.f, 69.f));
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
			TEXT("SourceAssets/UI/MainMenuReference/LeftPanel/shell_clean.png"),
			FVector2D(487.f, 726.f),
			FMargin(0.12f, 0.08f, 0.12f, 0.08f));
	}

	inline const FSlateBrush* GetHeroSelectionRightShellBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/MainMenuReference/RightPanel/shell_clean.png"),
			FVector2D(458.f, 709.f),
			FMargin(0.12f, 0.08f, 0.12f, 0.08f));
	}

	inline const FSlateBrush* GetHeroSelectionContentShellBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_content_shell_frame.png"),
			FVector2D(1521.f, 463.f),
			FMargin(0.035f, 0.12f, 0.035f, 0.12f));
	}

	inline const FSlateBrush* GetHeroSelectionRowShellBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_row_shell_full.png"),
			FVector2D(861.f, 74.f),
			FMargin(0.055f, 0.32f, 0.055f, 0.32f));
	}

	inline const FSlateBrush* GetHeroSelectionDropdownFieldBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_dropdown_field.png"),
			FVector2D(862.f, 77.f),
			FMargin(0.06f, 0.34f, 0.06f, 0.34f));
	}

	inline const FSlateBrush* GetHeroSelectionPartySlotBrush()
	{
		static FHeroSelectionSpriteBrushEntry Entry;
		return ResolveHeroSelectionSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/MainMenuReference/LeftPanel/party_slot_frame.png"),
			FVector2D(93.f, 103.f),
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

			TSharedRef<SButton> ButtonRef =
				SAssignNew(Button, SButton)
				.ButtonStyle(&OwnedButtonStyle)
				.ContentPadding(FMargin(0.f))
				.IsEnabled(InArgs._IsEnabled)
				.OnClicked(InArgs._OnClicked)
				[
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
					]
				];

			ChildSlot
			[
				SNew(SBox)
				.WidthOverride(InArgs._MinWidth > 0.f ? InArgs._MinWidth : FOptionalSize())
				.HeightOverride(InArgs._Height > 0.f ? InArgs._Height : FOptionalSize())
				.Visibility(InArgs._Visibility)
				[
					ButtonRef
				]
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
		const int32 FontSize = Params.FontSize > 0 ? Params.FontSize : 16;
		const FSlateFontInfo Font = T66RuntimeUIFontAccess::IsBoldWeight(*Params.FontWeight)
			? FT66Style::Tokens::FontBold(FontSize)
			: FT66Style::MakeFont(*Params.FontWeight, FontSize);
		const TAttribute<FText> Text = Params.DynamicLabel.IsBound()
			? Params.DynamicLabel
			: TAttribute<FText>(Params.Label);
		const TAttribute<FSlateColor> TextColor = Params.bHasTextColorOverride
			? Params.TextColorOverride
			: TAttribute<FSlateColor>(FSlateColor(FT66Style::Text()));
		const FMargin ContentPadding = Params.Padding.Left >= 0.f ? Params.Padding : FMargin(12.f, 7.f);

		const TSharedRef<SWidget> Content = Params.CustomContent.IsValid()
			? Params.CustomContent.ToSharedRef()
			: StaticCastSharedRef<SWidget>(
				SNew(STextBlock)
				.Text(Text)
				.Font(Font)
				.ColorAndOpacity(TextColor)
				.ShadowOffset(FVector2D(1.f, 1.f))
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.70f))
				.Justification(ETextJustify::Center));

		return SNew(ST66HeroSelectionSpriteButton)
			.SpriteFamily(SpriteFamily)
			.MinWidth(Params.MinWidth)
			.Height(Params.Height > 0.f ? Params.Height : 44.f)
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
				Params.Content
			];

		return SNew(SBox)
			.MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize())
			.HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize())
			.Visibility(Params.Visibility)
			[
				SNew(SBorder)
				.BorderImage(GetHeroSelectionDropdownFieldBrush())
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(FMargin(4.f, 2.f))
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
