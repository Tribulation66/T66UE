// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66ChallengesScreen.h"
#include "Core/T66CommunityContentSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66WebImageCache.h"
#include "Data/T66DataTypes.h"
#include "Engine/Texture2D.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UIManager.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	enum class ETabIndex : int32
	{
		Challenges = 0,
		Mods = 1,
		Count,
	};

	enum class ESourceTabIndex : int32
	{
		Official = 0,
		Community = 1,
		Count,
	};

	enum class ET66ChallengeButtonFamily : uint8
	{
		CompactNeutral,
		ToggleOn,
		ToggleOff,
		ToggleInactive
	};

	enum class ET66ChallengeButtonState : uint8
	{
		Normal,
		Hovered,
		Pressed,
		Disabled
	};

	struct FT66ChallengeSpriteBrushEntry
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
		bool bSimpleFallback = false;
	};

	struct FT66ChallengeButtonBrushSet
	{
		FT66ChallengeSpriteBrushEntry Normal;
		FT66ChallengeSpriteBrushEntry Hover;
		FT66ChallengeSpriteBrushEntry Pressed;
		FT66ChallengeSpriteBrushEntry Disabled;
	};

	struct FFlatChallengeCardData
	{
		const TCHAR* Title;
		int32 SkullCount = 1;
		int32 RewardCoupons = 0;
		const TCHAR* Description;
		const TCHAR* RuleA;
		const TCHAR* RuleB;
	};

	UT66GameInstance* GetT66GameInstance(const UObject* Context)
	{
		return Context ? Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(Context)) : nullptr;
	}

	FLinearColor ChallengeShellFill()
	{
		return FLinearColor(0.055f, 0.027f, 0.012f, 1.0f);
	}

	FLinearColor ChallengePanelFill()
	{
		return FLinearColor(0.095f, 0.047f, 0.020f, 1.0f);
	}

	FLinearColor ChallengePanelInsetFill()
	{
		return FLinearColor(0.72f, 0.50f, 0.23f, 1.0f);
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
		return FLinearColor(0.36f, 0.20f, 0.07f, 1.0f);
	}

	FLinearColor ChallengeDangerTint()
	{
		return FLinearColor(0.89f, 0.29f, 0.25f, 1.0f);
	}

	FLinearColor ChallengeSuccessTint()
	{
		return FLinearColor(0.86f, 0.67f, 0.34f, 1.0f);
	}

	FLinearColor ChallengeMutedBadgeTint()
	{
		return FLinearColor(0.37f, 0.22f, 0.09f, 1.0f);
	}

	const FLinearColor ChallengeFantasyText(0.953f, 0.925f, 0.835f, 1.0f);
	const FLinearColor ChallengeFantasyMuted(0.738f, 0.708f, 0.648f, 1.0f);
	const FLinearColor ChallengePaperText(0.98f, 0.91f, 0.70f, 1.0f);
	const FLinearColor ChallengePaperMuted(0.74f, 0.66f, 0.48f, 1.0f);
	const FLinearColor ChallengeGoldText(0.92f, 0.74f, 0.42f, 1.0f);

	const FSlateBrush* ResolveChallengeSpriteBrush(
		FT66ChallengeSpriteBrushEntry& Entry,
		const FString& RelativePath,
		const FVector2D& ImageSize,
		const FMargin& Margin,
		const ESlateBrushDrawType::Type DrawAs,
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
					TEXT("ChallengesReferenceSprite")))
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

	const FSlateBrush* ResolveChallengeSpriteRegionBrush(
		FT66ChallengeSpriteBrushEntry& Entry,
		const FString& RelativePath,
		const FVector2D& ImageSize,
		const FMargin& Margin,
		const FBox2f& UVRegion,
		const ESlateBrushDrawType::Type DrawAs,
		const FLinearColor& Tint,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		if (!Entry.Brush.IsValid())
		{
			Entry.Brush = MakeShared<FSlateBrush>();
			Entry.Brush->DrawAs = DrawAs;
			Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
			Entry.Brush->TintColor = FSlateColor(Tint);
			Entry.Brush->ImageSize = ImageSize;
			Entry.Brush->Margin = Margin;
			Entry.Brush->SetUVRegion(UVRegion);
		}

		if (!Entry.Texture.IsValid() && !Entry.bSimpleFallback)
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					Filter,
					true,
					TEXT("ChallengesReferenceSprite")))
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

	const FSlateBrush* GetChallengeContentShellBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/main_panel_normal_square_variant.png"),
			FVector2D(1588.f, 653.f),
			FMargin(0.060f, 0.090f, 0.060f, 0.105f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeRowShellBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			FT66FlatStyle::GetFlatLongPanelAssetPath(TEXT("normal")),
			FVector2D(1632.f, 209.f),
			FMargin(0.055f, 0.210f, 0.055f, 0.210f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeListPanelBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/main_panel_normal_square_variant.png"),
			FVector2D(777.f, 380.f),
			FMargin(0.065f, 0.090f, 0.065f, 0.105f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeDetailFrameBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/main_panel_normal_square_variant.png"),
			FVector2D(405.f, 388.f),
			FMargin(0.090f, 0.090f, 0.090f, 0.105f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeBrowserRowBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			FT66FlatStyle::GetFlatLongPanelAssetPath(TEXT("normal")),
			FVector2D(563.f, 107.f),
			FMargin(0.055f, 0.210f, 0.055f, 0.210f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeDetailPaperBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/main_panel_normal_square_variant.png"),
			FVector2D(451.f, 148.f),
			FMargin(0.095f, 0.185f, 0.095f, 0.185f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeRulesPaperBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/main_panel_normal_square_variant.png"),
			FVector2D(457.f, 181.f),
			FMargin(0.085f, 0.165f, 0.085f, 0.165f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeStatusBarBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/progress_bar_track.png"),
			FVector2D(693.f, 39.f),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeTagPillBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_normal_red_square_variant.png"),
			FVector2D(181.f, 60.f),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeStateSocketBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/check_square_normal.png"),
			FVector2D(123.f, 116.f),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeHeaderDividerBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/progress_bar_fill_cyan.png"),
			FVector2D(144.f, 6.f),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	const FScrollBarStyle* GetChallengeReferenceScrollBarStyle()
	{
		static FScrollBarStyle Style = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar");
		static FT66ChallengeSpriteBrushEntry TrackEntry;
		static FT66ChallengeSpriteBrushEntry ThumbEntry;
		static FT66ChallengeSpriteBrushEntry HoverEntry;

		const FSlateBrush* TrackBrush = ResolveChallengeSpriteBrush(
			TrackEntry,
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/progress_bar_track.png"),
			FVector2D(14.f, 120.f),
			FMargin(0.42f, 0.085f, 0.42f, 0.085f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
		const FSlateBrush* ThumbBrush = ResolveChallengeSpriteBrush(
			ThumbEntry,
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/progress_bar_fill_cyan.png"),
			FVector2D(16.f, 96.f),
			FMargin(0.38f, 0.115f, 0.38f, 0.115f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
		const FSlateBrush* HoverBrush = ResolveChallengeSpriteBrush(
			HoverEntry,
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/progress_bar_fill_cyan.png"),
			FVector2D(16.f, 96.f),
			FMargin(0.38f, 0.115f, 0.38f, 0.115f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);

		if (TrackBrush && ThumbBrush && HoverBrush)
		{
			Style
				.SetVerticalBackgroundImage(*TrackBrush)
				.SetVerticalTopSlotImage(*TrackBrush)
				.SetVerticalBottomSlotImage(*TrackBrush)
				.SetNormalThumbImage(*ThumbBrush)
				.SetHoveredThumbImage(*HoverBrush)
				.SetDraggedThumbImage(*HoverBrush)
				.SetThickness(14.f);
		}

		return &Style;
	}

	FString GetChallengeButtonPath(const ET66ChallengeButtonFamily Family, const ET66ChallengeButtonState State)
	{
		const TCHAR* Suffix = TEXT("normal");
		if (Family == ET66ChallengeButtonFamily::ToggleOn && State == ET66ChallengeButtonState::Normal)
		{
			Suffix = TEXT("selected");
		}
		else if (State == ET66ChallengeButtonState::Hovered)
		{
			Suffix = TEXT("hover");
		}
		else if (State == ET66ChallengeButtonState::Pressed)
		{
			Suffix = TEXT("pressed");
		}
		else if (State == ET66ChallengeButtonState::Disabled)
		{
			Suffix = TEXT("disabled");
		}

		const FString ButtonState = FCString::Stricmp(Suffix, TEXT("selected")) == 0
			? FString(TEXT("normal"))
			: FString(Suffix);
		return FString::Printf(
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_%s_red_square_variant.png"),
			*ButtonState);
	}

	FVector2D GetChallengeButtonSize(const ET66ChallengeButtonFamily Family, const ET66ChallengeButtonState State)
	{
		if (Family == ET66ChallengeButtonFamily::ToggleOn)
		{
			return State == ET66ChallengeButtonState::Pressed ? FVector2D(187.f, 67.f) : FVector2D(180.f, 68.f);
		}
		if (Family == ET66ChallengeButtonFamily::ToggleOff)
		{
			return State == ET66ChallengeButtonState::Pressed ? FVector2D(186.f, 68.f) : FVector2D(180.f, 68.f);
		}
		if (Family == ET66ChallengeButtonFamily::ToggleInactive)
		{
			return State == ET66ChallengeButtonState::Hovered ? FVector2D(186.f, 69.f) : FVector2D(180.f, 68.f);
		}
		return State == ET66ChallengeButtonState::Pressed ? FVector2D(186.f, 68.f) : FVector2D(180.f, 68.f);
	}

	FT66ChallengeButtonBrushSet& GetChallengeButtonBrushSet(const ET66ChallengeButtonFamily Family)
	{
		static FT66ChallengeButtonBrushSet CompactNeutral;
		static FT66ChallengeButtonBrushSet ToggleOn;
		static FT66ChallengeButtonBrushSet ToggleOff;
		static FT66ChallengeButtonBrushSet ToggleInactive;

		if (Family == ET66ChallengeButtonFamily::ToggleOn)
		{
			return ToggleOn;
		}
		if (Family == ET66ChallengeButtonFamily::ToggleOff)
		{
			return ToggleOff;
		}
		if (Family == ET66ChallengeButtonFamily::ToggleInactive)
		{
			return ToggleInactive;
		}
		return CompactNeutral;
	}

	const FSlateBrush* GetChallengeButtonBrush(const ET66ChallengeButtonFamily Family, const ET66ChallengeButtonState State)
	{
		FT66ChallengeButtonBrushSet& Set = GetChallengeButtonBrushSet(Family);
		FT66ChallengeSpriteBrushEntry* Entry = &Set.Normal;
		if (State == ET66ChallengeButtonState::Hovered)
		{
			Entry = &Set.Hover;
		}
		else if (State == ET66ChallengeButtonState::Pressed)
		{
			Entry = &Set.Pressed;
		}
		else if (State == ET66ChallengeButtonState::Disabled)
		{
			Entry = &Set.Disabled;
		}

		return ResolveChallengeSpriteBrush(
			*Entry,
			GetChallengeButtonPath(Family, State),
			GetChallengeButtonSize(Family, State),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	TSharedRef<SWidget> MakeChallengeSpritePanel(
		const TSharedRef<SWidget>& Content,
		const FSlateBrush* Brush,
		const FMargin& Padding,
		const FLinearColor& FallbackColor)
	{
		return SNew(SBorder)
			.BorderImage(Brush ? Brush : FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(Brush ? FLinearColor::White : FallbackColor)
			.Padding(Padding)
			[
				Content
			];
	}

	TSharedRef<SWidget> MakeChallengeHorizontalSlicedPanel(
		const TSharedRef<SWidget>& Content,
		const FSlateBrush* Brush,
		const float Height,
		const FMargin& Padding,
		const FLinearColor& FallbackColor,
		const float SourceCapFraction = 0.105f)
	{
		if (!Brush)
		{
			return SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FallbackColor)
				.Padding(Padding)
				[
					Content
				];
		}

		return SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				FT66FlatStyle::BuildFlatHorizontalSlicedImage(
					Brush,
					FVector2D(1.f, Height),
					SourceCapFraction)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(Padding)
			[
				Content
			];
	}

	TSharedRef<SWidget> MakeChallengeTagPill(const FText& Label)
	{
		return SNew(SBox)
			.WidthOverride(92.f)
			.HeightOverride(28.f)
			[
				MakeChallengeHorizontalSlicedPanel(
					SNew(STextBlock)
					.Text(Label)
					.Font(FT66FlatStyle::Tokens::FontBold(10))
					.ColorAndOpacity(ChallengeGoldText)
					.Justification(ETextJustify::Center)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds),
					GetChallengeTagPillBrush(),
					28.f,
					FMargin(8.f, 4.f, 8.f, 3.f),
					ChallengeMutedBadgeTint(),
					0.180f)
			];
	}

	TSharedRef<SWidget> MakeChallengeSpriteButtonContent(
		const TSharedRef<SWidget>& Content,
		const FOnClicked& OnClicked,
		const ET66ChallengeButtonFamily Family,
		const float MinWidth,
		const float Height,
		const FMargin& ContentPadding)
	{
		const FSlateBrush* NormalBrush = GetChallengeButtonBrush(Family, ET66ChallengeButtonState::Normal);
		const FSlateBrush* HoverBrush = GetChallengeButtonBrush(Family, ET66ChallengeButtonState::Hovered);
		const FSlateBrush* PressedBrush = GetChallengeButtonBrush(Family, ET66ChallengeButtonState::Pressed);
		const FSlateBrush* DisabledBrush = GetChallengeButtonBrush(Family, ET66ChallengeButtonState::Disabled);
		if (!NormalBrush)
		{
			return FT66FlatStyle::MakeButton(
				FT66ButtonParams(FText::GetEmpty(), OnClicked, Family == ET66ChallengeButtonFamily::ToggleOn ? ET66ButtonType::Primary : ET66ButtonType::Neutral)
				.SetMinWidth(MinWidth)
				.SetHeight(Height)
				.SetPadding(ContentPadding)
				.SetContent(Content));
		}

		return FT66FlatStyle::BuildFlatSlicedPlateButton(
			OnClicked,
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				Content
			],
			NormalBrush,
			HoverBrush,
			PressedBrush,
			DisabledBrush,
			MinWidth,
			Height,
			ContentPadding);
	}

	TSharedRef<SWidget> MakeChallengeSpriteButton(
		const FText& Label,
		const FOnClicked& OnClicked,
		const ET66ChallengeButtonFamily Family,
		const float MinWidth,
		const float Height,
		const int32 FontSize,
		const FMargin& ContentPadding = FMargin(12.f, 7.f, 12.f, 6.f))
	{
		return MakeChallengeSpriteButtonContent(
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::DownOnly)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(FT66FlatStyle::Tokens::FontBold(FontSize))
				.ColorAndOpacity(Family == ET66ChallengeButtonFamily::ToggleInactive ? ChallengeFantasyMuted : ChallengeFantasyText)
				.Justification(ETextJustify::Center)
				.AutoWrapText(false)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
			],
			OnClicked,
			Family,
			MinWidth,
			Height,
			ContentPadding);
	}

	ET66CommunityContentKind TabIndexToKind(const int32 TabIndex)
	{
		return TabIndex == static_cast<int32>(ETabIndex::Mods)
			? ET66CommunityContentKind::Mod
			: ET66CommunityContentKind::Challenge;
	}
}

UT66ChallengesScreen::UT66ChallengesScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::Challenges;
	bIsModal = false;
}

void UT66ChallengesScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		if (!bCommunityDelegateBound)
		{
			Community->OnContentChanged().AddUObject(this, &UT66ChallengesScreen::HandleCommunityContentChanged);
			bCommunityDelegateBound = true;
		}
	}
}

void UT66ChallengesScreen::OnScreenDeactivated_Implementation()
{
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		Community->OnContentChanged().RemoveAll(this);
	}

	bCommunityDelegateBound = false;
	Super::OnScreenDeactivated_Implementation();
}

UT66CommunityContentSubsystem* UT66ChallengesScreen::GetCommunitySubsystem() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UT66CommunityContentSubsystem>();
	}

	return nullptr;
}

ET66CommunityContentKind UT66ChallengesScreen::GetActiveKind() const
{
	return TabIndexToKind(ActiveTabIndex);
}

void UT66ChallengesScreen::OpenContentKind(const ET66CommunityContentKind ContentKind)
{
	InitializeSelectionState();
	ActiveTabIndex = ContentKind == ET66CommunityContentKind::Mod
		? static_cast<int32>(ETabIndex::Mods)
		: static_cast<int32>(ETabIndex::Challenges);
	EndDraftEditor();
	RequestDeferredSlateRebuild();
}

TArray<FT66CommunityContentEntry> UT66ChallengesScreen::GetEntriesForView(const int32 TabIndex, const int32 SourceTabIndex) const
{
	const UT66CommunityContentSubsystem* Community = GetCommunitySubsystem();
	if (!Community)
	{
		return {};
	}

	const ET66CommunityContentKind Kind = TabIndexToKind(TabIndex);
	return SourceTabIndex == static_cast<int32>(ESourceTabIndex::Official)
		? TArray<FT66CommunityContentEntry>(Community->GetOfficialEntries(Kind))
		: Community->GetCommunityBrowserEntries(Kind);
}

bool UT66ChallengesScreen::FindSelectedEntryForView(const int32 TabIndex, const int32 SourceTabIndex, FT66CommunityContentEntry& OutEntry)
{
	const TArray<FT66CommunityContentEntry> Entries = GetEntriesForView(TabIndex, SourceTabIndex);
	if (Entries.Num() <= 0)
	{
		return false;
	}

	const FName SelectedId = GetSelectedEntryIdForView(TabIndex, SourceTabIndex);
	const FT66CommunityContentEntry* Found = Entries.FindByPredicate([SelectedId](const FT66CommunityContentEntry& Entry)
	{
		return Entry.LocalId == SelectedId;
	});

	OutEntry = Found ? *Found : Entries[0];
	return true;
}

bool UT66ChallengesScreen::FindCurrentSelectedEntry(FT66CommunityContentEntry& OutEntry)
{
	return FindSelectedEntryForView(
		ActiveTabIndex,
		ActiveSourceTabIndex[ActiveTabIndex],
		OutEntry);
}

bool UT66ChallengesScreen::FindConfirmedEntry(FT66CommunityContentEntry& OutEntry) const
{
	if (const UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		return Community->GetActiveEntry(OutEntry);
	}

	return false;
}

FName UT66ChallengesScreen::GetSelectedEntryIdForView(const int32 TabIndex, const int32 SourceTabIndex)
{
	InitializeSelectionState();

	const int32 SafeTabIndex = FMath::Clamp(TabIndex, 0, static_cast<int32>(ETabIndex::Count) - 1);
	const int32 SafeSourceIndex = FMath::Clamp(SourceTabIndex, 0, static_cast<int32>(ESourceTabIndex::Count) - 1);
	FName& SelectedId = PendingSelections[SafeTabIndex][SafeSourceIndex];
	if (!SelectedId.IsNone())
	{
		return SelectedId;
	}

	const TArray<FT66CommunityContentEntry> Entries = GetEntriesForView(SafeTabIndex, SafeSourceIndex);
	if (Entries.Num() > 0)
	{
		SelectedId = Entries[0].LocalId;
	}

	return SelectedId;
}

FString UT66ChallengesScreen::GetOriginLabel(const FT66CommunityContentEntry& Entry) const
{
	switch (Entry.Origin)
	{
	case ET66CommunityContentOrigin::Draft:
		return TEXT("Draft");
	case ET66CommunityContentOrigin::Community:
		return TEXT("Community");
	case ET66CommunityContentOrigin::Official:
	default:
		return TEXT("Official");
	}
}

FString UT66ChallengesScreen::GetDraftSubmissionLabel(const FT66CommunityContentEntry& Entry) const
{
	if (!Entry.SubmissionStatus.IsEmpty())
	{
		return Entry.SubmissionStatus;
	}

	if (!Entry.ReviewNote.IsEmpty())
	{
		return Entry.ReviewNote;
	}

	if (!Entry.ModerationStatus.IsEmpty())
	{
		return Entry.ModerationStatus;
	}

	return TEXT("Not submitted");
}

FString UT66ChallengesScreen::GetPassiveLabel(const ET66PassiveType PassiveType) const
{
	if (const UEnum* Enum = StaticEnum<ET66PassiveType>())
	{
		return Enum->GetDisplayNameTextByValue(static_cast<int64>(PassiveType)).ToString();
	}

	return TEXT("None");
}

FString UT66ChallengesScreen::GetUltimateLabel(const ET66UltimateType UltimateType) const
{
	if (const UEnum* Enum = StaticEnum<ET66UltimateType>())
	{
		return Enum->GetDisplayNameTextByValue(static_cast<int64>(UltimateType)).ToString();
	}

	return TEXT("None");
}

FString UT66ChallengesScreen::GetItemLabel(const FName ItemId) const
{
	return ItemId.IsNone() ? TEXT("None") : ItemId.ToString();
}

TArray<FName> UT66ChallengesScreen::GetSelectableItemIds() const
{
	TArray<FName> Result;
	Result.Add(NAME_None);

	if (UT66GameInstance* T66GI = GetT66GameInstance(this))
	{
		if (UDataTable* ItemsTable = T66GI->GetItemsDataTable())
		{
			TArray<FName> RowNames = ItemsTable->GetRowNames();
			RowNames.Sort([](const FName& A, const FName& B)
			{
				return A.LexicalLess(B);
			});
			Result.Append(RowNames);
		}
	}

	return Result;
}

const FSlateBrush* UT66ChallengesScreen::GetOrCreateAvatarBrush(const FString& AvatarUrl)
{
	if (!DefaultAvatarBrush.IsValid())
	{
		DefaultAvatarBrush = MakeShared<FSlateBrush>();
		DefaultAvatarBrush->DrawAs = ESlateBrushDrawType::Image;
		DefaultAvatarBrush->ImageSize = FVector2D(52.0f, 52.0f);
		DefaultAvatarBrush->TintColor = FSlateColor(FLinearColor(0.14f, 0.15f, 0.17f, 0.32f));
	}

	if (AvatarUrl.IsEmpty())
	{
		return DefaultAvatarBrush.Get();
	}

	if (TSharedPtr<FSlateBrush>* Found = AvatarBrushes.Find(AvatarUrl))
	{
		return Found->Get();
	}

	const UGameInstance* GI = GetGameInstance();
	UT66WebImageCache* ImageCache = GI ? GI->GetSubsystem<UT66WebImageCache>() : nullptr;
	if (!ImageCache)
	{
		return DefaultAvatarBrush.Get();
	}

	if (UTexture2D* CachedTexture = ImageCache->GetCachedImage(AvatarUrl))
	{
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->SetResourceObject(CachedTexture);
		Brush->ImageSize = FVector2D(52.0f, 52.0f);
		AvatarBrushes.Add(AvatarUrl, Brush);
		return Brush.Get();
	}

	TWeakObjectPtr<UT66ChallengesScreen> WeakScreen(this);
	ImageCache->RequestImage(AvatarUrl, [WeakScreen](UTexture2D* Texture)
	{
		if (Texture)
		{
			if (UT66ChallengesScreen* Screen = WeakScreen.Get())
			{
				Screen->RequestDeferredSlateRebuild();
			}
		}
	});

	return DefaultAvatarBrush.Get();
}

void UT66ChallengesScreen::InitializeSelectionState()
{
	if (bSelectionStateInitialized)
	{
		return;
	}

	bSelectionStateInitialized = true;
	ActiveTabIndex = static_cast<int32>(ETabIndex::Challenges);
	ActiveSourceTabIndex[static_cast<int32>(ETabIndex::Challenges)] = static_cast<int32>(ESourceTabIndex::Official);
	ActiveSourceTabIndex[static_cast<int32>(ETabIndex::Mods)] = static_cast<int32>(ESourceTabIndex::Official);

	for (int32 TabIndex = 0; TabIndex < static_cast<int32>(ETabIndex::Count); ++TabIndex)
	{
		for (int32 SourceTabIndex = 0; SourceTabIndex < static_cast<int32>(ESourceTabIndex::Count); ++SourceTabIndex)
		{
			const TArray<FT66CommunityContentEntry> Entries = GetEntriesForView(TabIndex, SourceTabIndex);
			PendingSelections[TabIndex][SourceTabIndex] = Entries.Num() > 0 ? Entries[0].LocalId : NAME_None;
		}
	}

	if (const UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		FT66CommunityContentEntry ActiveEntry;
		if (Community->GetActiveEntry(ActiveEntry))
		{
			ActiveTabIndex = ActiveEntry.Kind == ET66CommunityContentKind::Mod
				? static_cast<int32>(ETabIndex::Mods)
				: static_cast<int32>(ETabIndex::Challenges);
			ActiveSourceTabIndex[ActiveTabIndex] = ActiveEntry.Origin == ET66CommunityContentOrigin::Official
				? static_cast<int32>(ESourceTabIndex::Official)
				: static_cast<int32>(ESourceTabIndex::Community);
			PendingSelections[ActiveTabIndex][ActiveSourceTabIndex[ActiveTabIndex]] = ActiveEntry.LocalId;
			return;
		}
	}

	if (const UT66GameInstance* T66GI = GetT66GameInstance(this))
	{
		if (T66GI->SelectedRunModifierKind == ET66RunModifierKind::Mod)
		{
			ActiveTabIndex = static_cast<int32>(ETabIndex::Mods);
		}
	}
}

void UT66ChallengesScreen::BeginDraftEditor(const FT66CommunityContentEntry& DraftEntry)
{
	bDraftEditorActive = true;
	DraftEditorEntry = DraftEntry;
	ActiveTabIndex = DraftEntry.Kind == ET66CommunityContentKind::Mod
		? static_cast<int32>(ETabIndex::Mods)
		: static_cast<int32>(ETabIndex::Challenges);
	ActiveSourceTabIndex[ActiveTabIndex] = static_cast<int32>(ESourceTabIndex::Community);
	PendingSelections[ActiveTabIndex][ActiveSourceTabIndex[ActiveTabIndex]] = DraftEntry.LocalId;
}

void UT66ChallengesScreen::EndDraftEditor()
{
	bDraftEditorActive = false;
	DraftEditorEntry = FT66CommunityContentEntry{};
}

void UT66ChallengesScreen::CycleDraftPassive(const int32 Direction)
{
	const UEnum* Enum = StaticEnum<ET66PassiveType>();
	if (!Enum)
	{
		return;
	}

	TArray<ET66PassiveType> Values;
	for (int32 EnumIndex = 0; EnumIndex < Enum->NumEnums() - 1; ++EnumIndex)
	{
		Values.Add(static_cast<ET66PassiveType>(Enum->GetValueByIndex(EnumIndex)));
	}

	const int32 CurrentIndex = Values.IndexOfByKey(DraftEditorEntry.Rules.PassiveOverride);
	const int32 NextIndex = Values.IsValidIndex(CurrentIndex)
		? (CurrentIndex + Direction + Values.Num()) % Values.Num()
		: 0;
	DraftEditorEntry.Rules.PassiveOverride = Values.IsValidIndex(NextIndex) ? Values[NextIndex] : ET66PassiveType::None;
}

void UT66ChallengesScreen::CycleDraftUltimate(const int32 Direction)
{
	const UEnum* Enum = StaticEnum<ET66UltimateType>();
	if (!Enum)
	{
		return;
	}

	TArray<ET66UltimateType> Values;
	for (int32 EnumIndex = 0; EnumIndex < Enum->NumEnums() - 1; ++EnumIndex)
	{
		Values.Add(static_cast<ET66UltimateType>(Enum->GetValueByIndex(EnumIndex)));
	}

	const int32 CurrentIndex = Values.IndexOfByKey(DraftEditorEntry.Rules.UltimateOverride);
	const int32 NextIndex = Values.IsValidIndex(CurrentIndex)
		? (CurrentIndex + Direction + Values.Num()) % Values.Num()
		: 0;
	DraftEditorEntry.Rules.UltimateOverride = Values.IsValidIndex(NextIndex) ? Values[NextIndex] : ET66UltimateType::None;
}

void UT66ChallengesScreen::CycleDraftStartingItem(const int32 Direction)
{
	const TArray<FName> ItemIds = GetSelectableItemIds();
	if (ItemIds.Num() <= 0)
	{
		DraftEditorEntry.Rules.StartingItemId = NAME_None;
		return;
	}

	const int32 CurrentIndex = ItemIds.IndexOfByKey(DraftEditorEntry.Rules.StartingItemId);
	const int32 SafeCurrentIndex = CurrentIndex != INDEX_NONE ? CurrentIndex : 0;
	const int32 NextIndex = (SafeCurrentIndex + Direction + ItemIds.Num()) % ItemIds.Num();
	DraftEditorEntry.Rules.StartingItemId = ItemIds[NextIndex];
}

void UT66ChallengesScreen::AdjustDraftStat(const EDraftStatField Field, const int32 Delta)
{
	auto Clamp = [](int32 Value)
	{
		return T66CommunityContentLimits::ClampStatBonus(Value);
	};

	switch (Field)
	{
	case EDraftStatField::Damage:
		DraftEditorEntry.Rules.BonusStats.Damage = Clamp(DraftEditorEntry.Rules.BonusStats.Damage + Delta);
		break;
	case EDraftStatField::AttackSpeed:
		DraftEditorEntry.Rules.BonusStats.AttackSpeed = Clamp(DraftEditorEntry.Rules.BonusStats.AttackSpeed + Delta);
		break;
	case EDraftStatField::AttackScale:
		DraftEditorEntry.Rules.BonusStats.AttackScale = Clamp(DraftEditorEntry.Rules.BonusStats.AttackScale + Delta);
		break;
	case EDraftStatField::Accuracy:
		DraftEditorEntry.Rules.BonusStats.Accuracy = Clamp(DraftEditorEntry.Rules.BonusStats.Accuracy + Delta);
		break;
	case EDraftStatField::Armor:
		DraftEditorEntry.Rules.BonusStats.Armor = Clamp(DraftEditorEntry.Rules.BonusStats.Armor + Delta);
		break;
	case EDraftStatField::Evasion:
		DraftEditorEntry.Rules.BonusStats.Evasion = Clamp(DraftEditorEntry.Rules.BonusStats.Evasion + Delta);
		break;
	case EDraftStatField::Luck:
		DraftEditorEntry.Rules.BonusStats.Luck = Clamp(DraftEditorEntry.Rules.BonusStats.Luck + Delta);
		break;
	case EDraftStatField::Speed:
		DraftEditorEntry.Rules.BonusStats.Speed = Clamp(DraftEditorEntry.Rules.BonusStats.Speed + Delta);
		break;
	default:
		break;
	}
}

TSharedRef<SWidget> UT66ChallengesScreen::BuildSlateUI()
{
	InitializeSelectionState();

	UT66CommunityContentSubsystem* Community = GetCommunitySubsystem();
	if (Community && !bRequestedCommunityRefresh)
	{
		bRequestedCommunityRefresh = true;
		Community->RefreshCommunityCatalog(false);
		Community->RefreshMySubmissionStates(false);
	}

	const float ReferenceCanvasWidth = 1920.0f;
	const float ReferenceCanvasHeight = 941.0f;
	const float ListPanelWidth = 672.0f;
	const float DetailPanelWidth = 900.0f;
	const float BodyPanelHeight = 548.0f;
	const float DetailColumnWidth = DetailPanelWidth - 46.0f;
	const float ListColumnWidth = ListPanelWidth - 44.0f;
	const int32 HeaderTabFontSize = 18;
	const int32 SourceTabFontSize = 20;
	const int32 ActionButtonFontSize = 19;
	const int32 CurrentSourceTabIndex = ActiveSourceTabIndex[ActiveTabIndex];
	const ET66CommunityContentKind ActiveKind = GetActiveKind();
	const TArray<FT66CommunityContentEntry> Entries = GetEntriesForView(ActiveTabIndex, CurrentSourceTabIndex);

	FT66CommunityContentEntry SelectedEntry;
	const bool bHasSelectedEntry = !bDraftEditorActive && FindCurrentSelectedEntry(SelectedEntry);

	FT66CommunityContentEntry ConfirmedEntry;
	const bool bHasConfirmedEntry = FindConfirmedEntry(ConfirmedEntry);
	const bool bSelectedEntryConfirmed = bHasSelectedEntry && bHasConfirmedEntry && SelectedEntry.LocalId == ConfirmedEntry.LocalId;

	const FString HeaderTitle = ActiveKind == ET66CommunityContentKind::Mod ? TEXT("Mods") : TEXT("Challenges");
	const FString DetailListHeader = ActiveKind == ET66CommunityContentKind::Mod ? TEXT("Rules") : TEXT("Rules And Requirements");
	const FString StatusMessage = Community ? Community->GetLastStatusMessage() : FString();
	const bool bCanConfirmSelectedEntry = bHasSelectedEntry && !SelectedEntry.IsDraft();
	const FText FooterConfirmLabel = FText::FromString(bSelectedEntryConfirmed ? TEXT("SELECTED") : TEXT("CONFIRM"));
	const ET66ChallengeButtonFamily FooterConfirmFamily = bCanConfirmSelectedEntry
		? ET66ChallengeButtonFamily::ToggleOn
		: ET66ChallengeButtonFamily::ToggleInactive;

	if (ActiveKind == ET66CommunityContentKind::Challenge && !bDraftEditorActive)
	{
		FlatSelectedChallengeCardIndex = FMath::Clamp(FlatSelectedChallengeCardIndex, 0, 3);
		FlatChallengePageIndex = FMath::Clamp(FlatChallengePageIndex, 0, 3);

		constexpr float CanvasW = 1920.f;
		constexpr float CanvasH = 1080.f;
		const FName ChallengeTabsGroup(TEXT("ChallengeTabs"));
		const FName ChallengeSelectionGroup(TEXT("ChallengeSelection"));
		const FName ChallengePaginationGroup(TEXT("ChallengePagination"));
		const FLinearColor Purple = FT66FlatStyle::PurpleAccent();
		const FLinearColor Red = FT66FlatStyle::SelectedText();
		const FLinearColor White = FT66FlatStyle::PrimaryText();

		const FFlatChallengeCardData FlatCards[4] =
		{
			{ TEXT("GLASS ROUTE"), 1, 40, TEXT("Clear the run without taking a single hit."), TEXT("Challenge only completes on a full clear."), TEXT("Take no damage for the run.") },
			{ TEXT("PRESSURE RUN"), 2, 30, TEXT("Finish a full clear before the timer budget expires."), TEXT("Challenge only completes on a full clear."), TEXT("Finish before the pressure timer expires.") },
			{ TEXT("LAST STAND"), 3, 60, TEXT("Survive the final route with no second chances."), TEXT("Challenge only completes on a full clear."), TEXT("No revival or safety net is allowed.") },
			{ TEXT("APOCALYPSE PROTOCOL"), 4, 80, TEXT("Clear the run under maximum pressure protocol."), TEXT("Challenge only completes on a full clear."), TEXT("All route pressure modifiers are active.") },
		};
		const FFlatChallengeCardData& SelectedFlatCard = FlatCards[FlatSelectedChallengeCardIndex];

		TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
		auto DTag = [](const TCHAR* Name) -> FName
		{
			return FName(Name);
		};
		auto AddN = [&Canvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
		{
			Canvas->AddSlot()
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X * CanvasW, Y * CanvasH, W * CanvasW, H * CanvasH))
			[
				Widget
			];
		};
		auto MakeLabel = [](
			const FName Tag,
			const FText& Text,
			const int32 FontSize,
			const FLinearColor& Color,
			const bool bBold = true,
			const ETextJustify::Type Justification = ETextJustify::Left) -> TSharedRef<SWidget>
		{
			TSharedRef<STextBlock> Label = SNew(STextBlock)
				.Text(Text)
				.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
				.ColorAndOpacity(Color)
				.Justification(Justification)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
				.Visibility(EVisibility::HitTestInvisible);
			return FT66FlatStyle::AttachMetadata(
				Label,
				Tag,
				TEXT("Label"),
				ET66FlatState::Default,
				TOptional<FLinearColor>(),
				false,
				NAME_None,
				true);
		};
		auto MakeRect = [](const FLinearColor& Color, const FName Tag, const FString& Role = TEXT("Rect")) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
				.BorderBackgroundColor(Color)
				.Visibility(EVisibility::HitTestInvisible),
				Tag,
				Role,
				ET66FlatState::Default);
		};
		auto MakePanel = [](const ET66FlatState State, const FName Tag) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatPanel(State, FMargin(0.f), SNullWidget::NullWidget, nullptr, Tag);
		};
		auto MakeButtonShell = [](
			const ET66FlatState State,
			FOnClicked OnClicked,
			const FName Tag,
			const FName ToggleGroup = NAME_None) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatToggleGroupButton(
				State,
				SNullWidget::NullWidget,
				MoveTemp(OnClicked),
				FMargin(0.f),
				0.f,
				0.f,
				true,
				Tag,
				ToggleGroup);
		};
		auto MakeIcon = [&MakeLabel](const FName Tag, const TCHAR* Path, const FVector2D& Size, const FLinearColor& Tint, const FText& Fallback) -> TSharedRef<SWidget>
		{
			static TMap<FString, FT66ChallengeSpriteBrushEntry> FlatIconEntries;
			FT66ChallengeSpriteBrushEntry& Entry = FlatIconEntries.FindOrAdd(FString(Path));
			const FSlateBrush* Brush = ResolveChallengeSpriteBrush(
				Entry,
				FString(Path),
				Size,
				FMargin(0.f),
				ESlateBrushDrawType::Image,
				TextureFilter::TF_Trilinear);
			if (!Brush)
			{
				return MakeLabel(Tag, Fallback, 24, Tint, true, ETextJustify::Center);
			}
			TSharedRef<SImage> Image = SNew(SImage)
				.Image(Brush)
				.ColorAndOpacity(Tint)
				.Visibility(EVisibility::HitTestInvisible);
			return FT66FlatStyle::AttachMetadata(Image, Tag, TEXT("Icon"), ET66FlatState::Default);
		};
		auto AddSectionHeader = [&](const float Y, const TCHAR* HeaderTag, const FText& Text)
		{
			AddN(0.533f, Y + 0.012f, 0.132f, 0.003f, MakeRect(FT66FlatStyle::SelectedBorder(), NAME_None, TEXT("Divider")));
			AddN(0.808f, Y + 0.012f, 0.140f, 0.003f, MakeRect(FT66FlatStyle::SelectedBorder(), NAME_None, TEXT("Divider")));
			AddN(0.662f, Y, 0.160f, 0.032f, MakeLabel(DTag(HeaderTag), Text, 22, Red, true, ETextJustify::Center));
		};

		AddN(0.f, 0.f, 1.f, 1.f, MakeRect(FT66FlatStyle::BackgroundColor(), DTag(TEXT("Challenges.Background")), TEXT("Background")));
		AddN(0.f, 0.f, 1.f, 1.f, FT66FlatStyle::AttachMetadata(SNew(SBox), DTag(TEXT("Challenges.Root")), TEXT("ScreenRoot"), ET66FlatState::Default));

		AddN(0.012f, 0.021f, 0.132f, 0.071f, MakeButtonShell(ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleBackClicked), DTag(TEXT("Challenges.TopRow.BackButton"))));
		AddN(0.033f, 0.044f, 0.022f, 0.034f, MakeIcon(DTag(TEXT("Challenges.TopRow.BackButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/back_chevron.png"), FVector2D(48.f, 48.f), Purple, FText::FromString(TEXT("<"))));
		AddN(0.071f, 0.039f, 0.054f, 0.044f, MakeLabel(DTag(TEXT("Challenges.TopRow.BackButton.Label")), NSLOCTEXT("T66.Challenges", "FlatBack", "BACK"), 28, Purple, true, ETextJustify::Left));

		AddN(0.374f, 0.033f, 0.254f, 0.066f, MakeLabel(DTag(TEXT("Challenges.Title")), NSLOCTEXT("T66.Challenges", "FlatTitle", "CHALLENGES"), 60, White, true, ETextJustify::Center));

		AddN(0.846f, 0.021f, 0.141f, 0.071f, MakeButtonShell(ET66FlatState::Selected, FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleConfirmClicked), DTag(TEXT("Challenges.TopRow.ConfirmButton"))));
		AddN(0.870f, 0.039f, 0.070f, 0.044f, MakeLabel(DTag(TEXT("Challenges.TopRow.ConfirmButton.Label")), NSLOCTEXT("T66.Challenges", "FlatConfirm", "CONFIRM"), 27, Red, true, ETextJustify::Left));
		AddN(0.947f, 0.044f, 0.025f, 0.034f, MakeIcon(DTag(TEXT("Challenges.TopRow.ConfirmButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/forward_chevron.png"), FVector2D(48.f, 48.f), Red, FText::FromString(TEXT(">"))));

		const bool bOfficialSelected = CurrentSourceTabIndex == static_cast<int32>(ESourceTabIndex::Official);
		const bool bCommunitySelected = CurrentSourceTabIndex == static_cast<int32>(ESourceTabIndex::Community);
		AddN(0.149f, 0.129f, 0.206f, 0.077f, MakeButtonShell(bOfficialSelected ? ET66FlatState::Selected : ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleSourceTabSelected, static_cast<int32>(ESourceTabIndex::Official)), DTag(TEXT("Challenges.Tabs.OfficialButton")), ChallengeTabsGroup));
		AddN(0.181f, 0.145f, 0.032f, 0.048f, MakeIcon(DTag(TEXT("Challenges.Tabs.OfficialButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/target_crosshair.png"), FVector2D(56.f, 56.f), bOfficialSelected ? Red : Purple, FText::FromString(TEXT("+"))));
		AddN(0.222f, 0.148f, 0.076f, 0.044f, MakeLabel(DTag(TEXT("Challenges.Tabs.OfficialButton.Label")), NSLOCTEXT("T66.Challenges", "FlatOfficial", "OFFICIAL"), 26, bOfficialSelected ? Red : White, true, ETextJustify::Center));
		AddN(0.314f, 0.146f, 0.025f, 0.044f, MakeIcon(DTag(TEXT("Challenges.Tabs.OfficialButton.InfoIcon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/info.png"), FVector2D(44.f, 44.f), bOfficialSelected ? Red : Purple, FText::FromString(TEXT("i"))));

		AddN(0.376f, 0.129f, 0.217f, 0.074f, MakeButtonShell(bCommunitySelected ? ET66FlatState::Selected : ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleSourceTabSelected, static_cast<int32>(ESourceTabIndex::Community)), DTag(TEXT("Challenges.Tabs.CommunityButton")), ChallengeTabsGroup));
		AddN(0.406f, 0.145f, 0.032f, 0.048f, MakeIcon(DTag(TEXT("Challenges.Tabs.CommunityButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/people.png"), FVector2D(56.f, 56.f), bCommunitySelected ? Red : Purple, FText::FromString(TEXT("**"))));
		AddN(0.447f, 0.148f, 0.096f, 0.044f, MakeLabel(DTag(TEXT("Challenges.Tabs.CommunityButton.Label")), NSLOCTEXT("T66.Challenges", "FlatCommunity", "COMMUNITY"), 25, bCommunitySelected ? Red : White, true, ETextJustify::Center));
		AddN(0.552f, 0.146f, 0.025f, 0.044f, MakeIcon(DTag(TEXT("Challenges.Tabs.CommunityButton.InfoIcon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/info.png"), FVector2D(44.f, 44.f), bCommunitySelected ? Red : Purple, FText::FromString(TEXT("i"))));

		AddN(0.615f, 0.129f, 0.236f, 0.074f, MakeButtonShell(ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCreateDraftClicked), DTag(TEXT("Challenges.Tabs.CreateButton")), ChallengeTabsGroup));
		AddN(0.642f, 0.145f, 0.032f, 0.048f, MakeIcon(DTag(TEXT("Challenges.Tabs.CreateButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/pencil_edit.png"), FVector2D(56.f, 56.f), Purple, FText::FromString(TEXT("/"))));
		AddN(0.683f, 0.148f, 0.130f, 0.044f, MakeLabel(DTag(TEXT("Challenges.Tabs.CreateButton.Label")), NSLOCTEXT("T66.Challenges", "FlatCreateChallenge", "CREATE CHALLENGE"), 23, White, true, ETextJustify::Center));
		AddN(0.819f, 0.146f, 0.025f, 0.044f, MakeIcon(DTag(TEXT("Challenges.Tabs.CreateButton.InfoIcon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/info.png"), FVector2D(44.f, 44.f), Purple, FText::FromString(TEXT("i"))));

		AddN(0.027f, 0.230f, 0.466f, 0.704f, MakePanel(ET66FlatState::Default, DTag(TEXT("Challenges.LeftPanel"))));
		const float CardY[4] = { 0.252f, 0.418f, 0.586f, 0.753f };
		const TCHAR* CardTags[4] =
		{
			TEXT("Challenges.LeftPanel.Card01"),
			TEXT("Challenges.LeftPanel.Card02"),
			TEXT("Challenges.LeftPanel.Card03"),
			TEXT("Challenges.LeftPanel.Card04"),
		};
		for (int32 CardIndex = 0; CardIndex < 4; ++CardIndex)
		{
			const FFlatChallengeCardData& Card = FlatCards[CardIndex];
			const FString Prefix(CardTags[CardIndex]);
			const ET66FlatState CardState = CardIndex == FlatSelectedChallengeCardIndex ? ET66FlatState::Selected : ET66FlatState::Default;
			const FLinearColor CardAccent = CardState == ET66FlatState::Selected ? Red : Purple;
			AddN(0.038f, CardY[CardIndex], 0.445f, 0.150f, MakeButtonShell(CardState, FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleFlatChallengeCardSelected, CardIndex), DTag(CardTags[CardIndex]), ChallengeSelectionGroup));
			AddN(0.142f, CardY[CardIndex] + 0.030f, CardIndex == 3 ? 0.210f : 0.150f, 0.042f, MakeLabel(FName(*(Prefix + TEXT(".Title"))), FText::FromString(Card.Title), 32, White, true, ETextJustify::Left));
			AddN(0.172f, CardY[CardIndex] + 0.087f, 0.126f, 0.038f, MakeLabel(FName(*(Prefix + TEXT(".Reward"))), FText::Format(NSLOCTEXT("T66.Challenges", "FlatRewardFormat", "{0} CHAD COUPONS"), FText::AsNumber(Card.RewardCoupons)), 23, CardAccent, true, ETextJustify::Left));
			AddN(0.351f, CardY[CardIndex] + 0.087f, 0.132f, 0.038f, MakeLabel(FName(*(Prefix + TEXT(".Author"))), NSLOCTEXT("T66.Challenges", "FlatAuthor", "TRIBULATION 66"), 23, CardAccent, true, ETextJustify::Left));
			AddN(0.143f, CardY[CardIndex] + 0.088f, 0.024f, 0.036f, MakeIcon(NAME_None, TEXT("RuntimeDependencies/T66/UI/Icons/Flat/ticket.png"), FVector2D(44.f, 44.f), CardAccent, FText::FromString(TEXT("#"))));
			AddN(0.322f, CardY[CardIndex] + 0.088f, 0.024f, 0.036f, MakeIcon(NAME_None, TEXT("RuntimeDependencies/T66/UI/Icons/Flat/people.png"), FVector2D(44.f, 44.f), CardAccent, FText::FromString(TEXT("@"))));
			for (int32 SkullIndex = 0; SkullIndex < Card.SkullCount; ++SkullIndex)
			{
				AddN(0.057f + SkullIndex * 0.022f, CardY[CardIndex] + 0.049f, 0.025f, 0.047f, MakeIcon(NAME_None, TEXT("RuntimeDependencies/T66/UI/Icons/Flat/skull.png"), FVector2D(48.f, 48.f), CardAccent, FText::FromString(TEXT("S"))));
			}
		}

		AddN(0.221f, 0.912f, 0.050f, 0.016f, FT66FlatStyle::AttachMetadata(SNew(SBox), DTag(TEXT("Challenges.Pagination")), TEXT("Pagination"), ET66FlatState::Default));
		for (int32 DotIndex = 0; DotIndex < 4; ++DotIndex)
		{
			const FString DotTag = FString::Printf(TEXT("Challenges.Pagination.Dot%02d"), DotIndex + 1);
			AddN(0.221f + DotIndex * 0.014f, 0.912f, 0.008f, 0.014f, MakeButtonShell(DotIndex == FlatChallengePageIndex ? ET66FlatState::Selected : ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleFlatPaginationSelected, DotIndex), FName(*DotTag), ChallengePaginationGroup));
		}

		AddN(0.507f, 0.231f, 0.465f, 0.704f, MakePanel(ET66FlatState::Selected, DTag(TEXT("Challenges.RightPanel"))));
		AddN(0.533f, 0.270f, 0.210f, 0.048f, MakeLabel(DTag(TEXT("Challenges.RightPanel.Title")), FText::FromString(SelectedFlatCard.Title), 38, White, true, ETextJustify::Left));
		AddN(0.895f, 0.279f, 0.062f, 0.036f, MakeLabel(DTag(TEXT("Challenges.RightPanel.OriginLabel")), NSLOCTEXT("T66.Challenges", "FlatOfficialOrigin", "Official"), 26, Red, true, ETextJustify::Right));
		AddSectionHeader(0.342f, TEXT("Challenges.RightPanel.DescriptionHeader"), NSLOCTEXT("T66.Challenges", "FlatDescriptionHeader", "DESCRIPTION"));
		AddN(0.533f, 0.397f, 0.330f, 0.038f, MakeLabel(DTag(TEXT("Challenges.RightPanel.DescriptionText")), FText::FromString(SelectedFlatCard.Description), 23, White, false, ETextJustify::Left));
		AddSectionHeader(0.471f, TEXT("Challenges.RightPanel.SkullRatingHeader"), NSLOCTEXT("T66.Challenges", "FlatSkullRatingHeader", "SKULL RATING"));
		AddN(0.718f, 0.522f, 0.030f, 0.061f, MakeIcon(DTag(TEXT("Challenges.RightPanel.SkullRatingIcon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/skull.png"), FVector2D(60.f, 60.f), Red, FText::FromString(TEXT("S"))));
		AddSectionHeader(0.615f, TEXT("Challenges.RightPanel.RulesHeader"), NSLOCTEXT("T66.Challenges", "FlatRulesHeader", "RULES AND REQUIREMENTS"));
		AddN(0.535f, 0.674f, 0.009f, 0.016f, MakeRect(FT66FlatStyle::SelectedBorder(), DTag(TEXT("Challenges.RightPanel.Rule01.Bullet")), TEXT("Bullet")));
		AddN(0.553f, 0.663f, 0.345f, 0.045f, MakeLabel(DTag(TEXT("Challenges.RightPanel.Rule01")), FText::FromString(SelectedFlatCard.RuleA), 22, White, false, ETextJustify::Left));
		AddN(0.535f, 0.730f, 0.009f, 0.016f, MakeRect(FT66FlatStyle::SelectedBorder(), DTag(TEXT("Challenges.RightPanel.Rule02.Bullet")), TEXT("Bullet")));
		AddN(0.553f, 0.719f, 0.300f, 0.045f, MakeLabel(DTag(TEXT("Challenges.RightPanel.Rule02")), FText::FromString(SelectedFlatCard.RuleB), 22, White, false, ETextJustify::Left));
		AddSectionHeader(0.811f, TEXT("Challenges.RightPanel.RewardHeader"), NSLOCTEXT("T66.Challenges", "FlatRewardHeader", "REWARD"));
		AddN(0.645f, 0.864f, 0.164f, 0.046f, FT66FlatStyle::AttachMetadata(SNew(SBox), DTag(TEXT("Challenges.RightPanel.Reward")), TEXT("RewardCluster"), ET66FlatState::Default));
		AddN(0.644f, 0.850f, 0.031f, 0.055f, MakeIcon(DTag(TEXT("Challenges.RightPanel.Reward.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/ticket.png"), FVector2D(58.f, 58.f), Red, FText::FromString(TEXT("#"))));
		AddN(0.685f, 0.858f, 0.140f, 0.046f, MakeLabel(DTag(TEXT("Challenges.RightPanel.Reward.Label")), FText::Format(NSLOCTEXT("T66.Challenges", "FlatDetailRewardFormat", "{0} CHAD COUPONS"), FText::AsNumber(SelectedFlatCard.RewardCoupons)), 24, Red, true, ETextJustify::Left));

		return FT66FlatStyle::WrapWithoutRetainer(
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
				.BorderBackgroundColor(FT66FlatStyle::BackgroundColor())
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::Both)
				[
					SNew(SBox)
					.WidthOverride(CanvasW)
					.HeightOverride(CanvasH)
					[
						Canvas
					]
				]
			],
			DTag(TEXT("Challenges.ViewportRoot")));
	}

	auto MakeConstraintRow = [DetailColumnWidth](const FString& ConstraintText) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 1.f, 8.f, 0.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Challenges", "ConstraintBullet", "\u25C6"))
				.Font(FT66FlatStyle::Tokens::FontBold(15))
				.ColorAndOpacity(ChallengePaperMuted)
			]
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(ConstraintText))
				.Font(FT66FlatStyle::Tokens::FontBold(15))
				.ColorAndOpacity(ChallengePaperText)
				.AutoWrapText(true)
				.WrapTextAt(FMath::Max(260.f, DetailColumnWidth - 104.f))
				.Clipping(EWidgetClipping::ClipToBounds)
			];
	};

	auto MakeTopTabButton = [this, HeaderTabFontSize](const int32 TabIndex, const FText& Label) -> TSharedRef<SWidget>
	{
		const bool bActive = ActiveTabIndex == TabIndex;
		const float ButtonWidth = TabIndex == static_cast<int32>(ETabIndex::Challenges) ? 246.f : 178.f;
		return MakeChallengeSpriteButton(
			Label,
			FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleTabSelected, TabIndex),
			bActive ? ET66ChallengeButtonFamily::ToggleOn : ET66ChallengeButtonFamily::CompactNeutral,
			ButtonWidth,
			52.f,
			HeaderTabFontSize,
			FMargin(16.f, 7.f));
	};

	auto MakeSourceTabButton = [this, CurrentSourceTabIndex, SourceTabFontSize](const int32 SourceTabIndex, const FText& Label) -> TSharedRef<SWidget>
	{
		const bool bActive = CurrentSourceTabIndex == SourceTabIndex;
		return MakeChallengeSpriteButton(
			Label,
			FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleSourceTabSelected, SourceTabIndex),
			bActive ? ET66ChallengeButtonFamily::ToggleOn : ET66ChallengeButtonFamily::CompactNeutral,
			296.f,
			44.f,
			SourceTabFontSize,
			FMargin(16.f, 8.f));
	};

	auto MakeEntryRow = [this, CurrentSourceTabIndex, Community, ListColumnWidth](const FT66CommunityContentEntry& Entry, const int32 EntryIndex) -> TSharedRef<SWidget>
	{
		const FName SelectedId = GetSelectedEntryIdForView(ActiveTabIndex, CurrentSourceTabIndex);
		const bool bSelected = Entry.LocalId == SelectedId;
		const FT66CommunityContentEntry ActiveEntry = [this]()
		{
			FT66CommunityContentEntry Result;
			FindConfirmedEntry(Result);
			return Result;
		}();
		const bool bConfirmed = !ActiveEntry.LocalId.IsNone() && ActiveEntry.LocalId == Entry.LocalId;

		const TSharedRef<SWidget> StateSocket =
			SNew(SBox)
			.WidthOverride(52.f)
			.HeightOverride(52.f)
			[
				MakeChallengeSpritePanel(
					SNew(STextBlock)
					.Text(bConfirmed ? NSLOCTEXT("T66.Challenges", "ConfirmedMarker", "X") : FText::GetEmpty())
					.Font(FT66FlatStyle::Tokens::FontBold(14))
					.ColorAndOpacity(bConfirmed ? ChallengeGoldText : FLinearColor::Transparent)
					.Justification(ETextJustify::Center),
					GetChallengeStateSocketBrush(),
					FMargin(0.f),
					ChallengeMutedBadgeTint())
			];

		const TSharedRef<SWidget> RowContent =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 18.f, 0.f)
			[
				StateSocket
			]
			+ SHorizontalBox::Slot().FillWidth(0.44f).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(Entry.Title))
					.Font(FT66FlatStyle::Tokens::FontBold(21))
					.ColorAndOpacity(bSelected ? ChallengePaperText : ChallengePaperMuted)
					.WrapTextAt(FMath::Max(210.f, ListColumnWidth * 0.34f))
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
				[
					MakeChallengeTagPill(FText::FromString(GetOriginLabel(Entry).ToUpper()))
				]
			]
			+ SHorizontalBox::Slot().FillWidth(0.26f).VAlign(VAlign_Center).Padding(20.f, 0.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Community ? Community->BuildRewardSummary(Entry) : TEXT("No reward data")))
				.Font(FT66FlatStyle::Tokens::FontBold(14))
				.ColorAndOpacity(ChallengeRewardTint())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
			]
			+ SHorizontalBox::Slot().FillWidth(0.24f).VAlign(VAlign_Center).Padding(20.f, 0.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Entry.IsDraft() ? GetDraftSubmissionLabel(Entry) : Entry.AuthorDisplayName))
				.Font(FT66FlatStyle::Tokens::FontBold(13))
				.ColorAndOpacity(ChallengePaperMuted)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
			];

		return FT66FlatStyle::MakeBareButton(
			FT66BareButtonParams(
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleEntrySelected, EntryIndex),
				SNew(SBox)
				.HeightOverride(112.f)
				[
					MakeChallengeSpritePanel(
						RowContent,
						GetChallengeBrowserRowBrush(),
						FMargin(28.f, 18.f, 28.f, 16.f),
						ChallengePanelInsetFill())
				])
				.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
				.SetPadding(FMargin(0.f))
				.SetHAlign(HAlign_Fill)
				.SetVAlign(VAlign_Fill)
				.SetHeight(112.f));
	};

	auto MakeDraftStepRow = [this](const FString& Label, const int32 Value, const FOnClicked& OnMinus, const FOnClicked& OnPlus) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Label))
				.Font(FT66FlatStyle::Tokens::FontBold(12))
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
			[
				MakeChallengeSpriteButton(FText::FromString(TEXT("-")), OnMinus, ET66ChallengeButtonFamily::CompactNeutral, 28.f, 24.f, 12, FMargin(0.f))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(Value))
				.Font(FT66FlatStyle::Tokens::FontBold(12))
				.ColorAndOpacity(ChallengeRewardTint())
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeChallengeSpriteButton(FText::FromString(TEXT("+")), OnPlus, ET66ChallengeButtonFamily::CompactNeutral, 28.f, 24.f, 12, FMargin(0.f))
			];
	};

	auto MakeCycleRow = [this](const FString& Label, const FString& Value, const FOnClicked& OnPrev, const FOnClicked& OnNext) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Label))
				.Font(FT66FlatStyle::Tokens::FontBold(12))
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
			[
				MakeChallengeSpriteButton(FText::FromString(TEXT("<")), OnPrev, ET66ChallengeButtonFamily::CompactNeutral, 28.f, 24.f, 12, FMargin(0.f))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Value))
				.Font(FT66FlatStyle::Tokens::FontBold(12))
				.ColorAndOpacity(ChallengeRewardTint())
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
			[
				MakeChallengeSpriteButton(FText::FromString(TEXT(">")), OnNext, ET66ChallengeButtonFamily::CompactNeutral, 28.f, 24.f, 12, FMargin(0.f))
			];
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

	const TSharedRef<SWidget> ListPanelContent = Entries.Num() > 0
		? StaticCastSharedRef<SWidget>(
			SNew(SScrollBox)
			.ScrollBarStyle(GetChallengeReferenceScrollBarStyle())
			.ScrollBarVisibility(EVisibility::Visible)
			.ScrollBarThickness(FVector2D(14.f, 14.f))
			.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
			+ SScrollBox::Slot()
			[
				EntryList
			])
		: StaticCastSharedRef<SWidget>(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(CurrentSourceTabIndex == static_cast<int32>(ESourceTabIndex::Community)
					? TEXT("No community entries yet. Create the first one.")
					: TEXT("No official entries were found.")))
				.Font(FT66FlatStyle::Tokens::FontBold(16))
				.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
			]);

	const TSharedRef<SWidget> CreateButtonContent = MakeChallengeSpriteButton(
		FText::FromString(ActiveKind == ET66CommunityContentKind::Mod ? TEXT("CREATE MOD") : TEXT("CREATE CHALLENGE")),
		FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCreateDraftClicked),
		ET66ChallengeButtonFamily::ToggleOn,
		264.f,
		44.f,
		ActionButtonFontSize,
		FMargin(18.f, 8.f));

	TSharedRef<SWidget> DetailPanelContent = SNew(STextBlock)
		.Text(NSLOCTEXT("T66.Challenges", "NoSelection", "Select an entry or create a new draft."))
		.Font(FT66FlatStyle::Tokens::FontRegular(13))
		.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
		.AutoWrapText(true);

	if (bDraftEditorActive)
	{
		TSharedRef<SVerticalBox> EditorRows = SNew(SVerticalBox);

		auto AddEditorSpacer = [&EditorRows](float Height)
		{
			EditorRows->AddSlot().AutoHeight().Padding(0.f, Height, 0.f, 0.f)
			[
				SNew(SSpacer)
				.Size(FVector2D(1.f, 1.f))
			];
		};

		EditorRows->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(DraftEditorEntry.Kind == ET66CommunityContentKind::Mod ? TEXT("Create Mod") : TEXT("Create Challenge")))
			.Font(FT66FlatStyle::Tokens::FontBold(28))
			.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
		];
		AddEditorSpacer(10.f);
		EditorRows->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Title")))
			.Font(FT66FlatStyle::Tokens::FontBold(12))
			.ColorAndOpacity(ChallengeSuccessTint())
		];
		EditorRows->AddSlot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
		[
			MakeChallengeSpritePanel(
				SNew(SEditableTextBox)
				.Text(FText::FromString(DraftEditorEntry.Title))
				.ForegroundColor(ChallengeFantasyText)
				.BackgroundColor(FLinearColor::Transparent)
				.OnTextChanged_UObject(this, &UT66ChallengesScreen::HandleDraftTitleChanged),
				GetChallengeRowShellBrush(),
				FMargin(10.f, 6.f),
				ChallengePanelInsetFill())
		];
		AddEditorSpacer(10.f);
		EditorRows->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Description")))
			.Font(FT66FlatStyle::Tokens::FontBold(12))
			.ColorAndOpacity(ChallengeSuccessTint())
		];
		EditorRows->AddSlot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
		[
			MakeChallengeSpritePanel(
				SNew(SMultiLineEditableTextBox)
				.Text(FText::FromString(DraftEditorEntry.Description))
				.ForegroundColor(ChallengeFantasyText)
				.OnTextChanged_UObject(this, &UT66ChallengesScreen::HandleDraftDescriptionChanged),
				GetChallengeRowShellBrush(),
				FMargin(10.f, 6.f),
				ChallengePanelInsetFill())
		];

		if (DraftEditorEntry.Kind == ET66CommunityContentKind::Challenge)
		{
			AddEditorSpacer(12.f);
			EditorRows->AddSlot().AutoHeight()
			[
				MakeDraftStepRow(
					TEXT("Suggested Chad Coupons"),
					DraftEditorEntry.SuggestedRewardChadCoupons,
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftReward, -5),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftReward, +5))
			];
		}

		AddEditorSpacer(12.f);
		EditorRows->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Gameplay Rules")))
			.Font(FT66FlatStyle::Tokens::FontBold(12))
			.ColorAndOpacity(ChallengeSuccessTint())
		];
		EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			MakeDraftStepRow(
				TEXT("Start Level"),
				DraftEditorEntry.Rules.StartLevelOverride,
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftStartLevel, -1),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftStartLevel, +1))
		];
		EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Max Hero Stats")))
				.Font(FT66FlatStyle::Tokens::FontBold(12))
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeChallengeSpriteButton(
					FText::FromString(DraftEditorEntry.Rules.bSetMaxHeroStats ? TEXT("Enabled") : TEXT("Disabled")),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleToggleDraftMaxStats),
					DraftEditorEntry.Rules.bSetMaxHeroStats ? ET66ChallengeButtonFamily::ToggleOn : ET66ChallengeButtonFamily::CompactNeutral,
					100.f,
					24.f,
					10,
					FMargin(8.f, 2.f))
			]
		];

		const TArray<TPair<FString, EDraftStatField>> StatFields = {
			TPair<FString, EDraftStatField>(TEXT("Damage"), EDraftStatField::Damage),
			TPair<FString, EDraftStatField>(TEXT("Attack Speed"), EDraftStatField::AttackSpeed),
			TPair<FString, EDraftStatField>(TEXT("Attack Scale"), EDraftStatField::AttackScale),
			TPair<FString, EDraftStatField>(TEXT("Accuracy"), EDraftStatField::Accuracy),
			TPair<FString, EDraftStatField>(TEXT("Armor"), EDraftStatField::Armor),
			TPair<FString, EDraftStatField>(TEXT("Evasion"), EDraftStatField::Evasion),
			TPair<FString, EDraftStatField>(TEXT("Luck"), EDraftStatField::Luck),
			TPair<FString, EDraftStatField>(TEXT("Speed"), EDraftStatField::Speed),
		};

		auto GetDraftStatValue = [this](const EDraftStatField Field)
		{
			switch (Field)
			{
			case EDraftStatField::Damage: return DraftEditorEntry.Rules.BonusStats.Damage;
			case EDraftStatField::AttackSpeed: return DraftEditorEntry.Rules.BonusStats.AttackSpeed;
			case EDraftStatField::AttackScale: return DraftEditorEntry.Rules.BonusStats.AttackScale;
			case EDraftStatField::Accuracy: return DraftEditorEntry.Rules.BonusStats.Accuracy;
			case EDraftStatField::Armor: return DraftEditorEntry.Rules.BonusStats.Armor;
			case EDraftStatField::Evasion: return DraftEditorEntry.Rules.BonusStats.Evasion;
			case EDraftStatField::Luck: return DraftEditorEntry.Rules.BonusStats.Luck;
			case EDraftStatField::Speed: return DraftEditorEntry.Rules.BonusStats.Speed;
			default: return 0;
			}
		};

		for (const TPair<FString, EDraftStatField>& StatField : StatFields)
		{
			EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
			[
				MakeDraftStepRow(
					StatField.Key,
					GetDraftStatValue(StatField.Value),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftStatClicked, StatField.Value, -5),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftStatClicked, StatField.Value, +5))
			];
		}

		EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			MakeCycleRow(
				TEXT("Starting Item"),
				GetItemLabel(DraftEditorEntry.Rules.StartingItemId),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftStartingItemClicked, -1),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftStartingItemClicked, +1))
		];
		EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			MakeCycleRow(
				TEXT("Passive Override"),
				GetPassiveLabel(DraftEditorEntry.Rules.PassiveOverride),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftPassiveClicked, -1),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftPassiveClicked, +1))
		];
		EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			MakeCycleRow(
				TEXT("Ultimate Override"),
				GetUltimateLabel(DraftEditorEntry.Rules.UltimateOverride),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftUltimateClicked, -1),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftUltimateClicked, +1))
		];

		if (DraftEditorEntry.Kind == ET66CommunityContentKind::Challenge)
		{
			AddEditorSpacer(12.f);
			EditorRows->AddSlot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Completion Requirements")))
				.Font(FT66FlatStyle::Tokens::FontBold(12))
				.ColorAndOpacity(ChallengeSuccessTint())
			];
			EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
			[
				SNew(SCheckBox)
				.IsChecked(DraftEditorEntry.Rules.bRequireFullClear ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_UObject(this, &UT66ChallengesScreen::HandleDraftFullClearChanged)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Require full clear")))
					.Font(FT66FlatStyle::Tokens::FontRegular(12))
					.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
				]
			];
			EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
			[
				SNew(SCheckBox)
				.IsChecked(DraftEditorEntry.Rules.bRequireNoDamage ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_UObject(this, &UT66ChallengesScreen::HandleDraftNoDamageChanged)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Require no damage")))
					.Font(FT66FlatStyle::Tokens::FontRegular(12))
					.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
				]
			];
			EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
			[
				MakeDraftStepRow(
					TEXT("Minimum Stage Reached"),
					DraftEditorEntry.Rules.RequiredStageReached,
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftRequiredStage, -1),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftRequiredStage, +1))
			];
			EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
			[
				MakeDraftStepRow(
					TEXT("Max Run Time (Seconds)"),
					DraftEditorEntry.Rules.MaxRunTimeSeconds,
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftMaxRunTime, -30),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftMaxRunTime, +30))
			];
		}

		AddEditorSpacer(16.f);
		EditorRows->AddSlot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
			[
				MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "SaveDraft", "SAVE DRAFT"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleSaveDraftClicked), ET66ChallengeButtonFamily::ToggleOn, 136.f, 34.f, 12)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
			[
				MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "SubmitDraft", "SUBMIT"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleSubmitDraftClicked), ET66ChallengeButtonFamily::ToggleOn, 120.f, 34.f, 12)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
			[
				MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "PlayDraft", "PLAY DRAFT"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandlePlayDraftClicked), ET66ChallengeButtonFamily::CompactNeutral, 120.f, 34.f, 12)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "CancelDraft", "CANCEL"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCancelDraftEditorClicked), ET66ChallengeButtonFamily::ToggleOff, 112.f, 34.f, 12)
			]
		];

		DetailPanelContent =
			SNew(SScrollBox)
			.ScrollBarStyle(GetChallengeReferenceScrollBarStyle())
			.ScrollBarVisibility(EVisibility::Visible)
			.ScrollBarThickness(FVector2D(14.f, 14.f))
			.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
			+ SScrollBox::Slot()
			[
				EditorRows
			];
	}
	else if (bHasSelectedEntry)
	{
		const TArray<FString> RuleLines = Community ? Community->BuildRuleSummaryLines(SelectedEntry) : TArray<FString>{};
		TSharedRef<SVerticalBox> RuleList = SNew(SVerticalBox);
		for (const FString& RuleLine : RuleLines)
		{
			RuleList->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				MakeConstraintRow(RuleLine)
			];
		}

		TSharedRef<SVerticalBox> DetailLayout = SNew(SVerticalBox);
		DetailLayout->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(SBox)
			.WidthOverride(DetailColumnWidth - 18.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(SelectedEntry.Title))
				.Font(FT66FlatStyle::Tokens::FontBold(28))
				.ColorAndOpacity(ChallengeFantasyText)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				.WrapTextAt(DetailColumnWidth - 24.f)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
			]
		];
		DetailLayout->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
			[
				SelectedEntry.AuthorAvatarUrl.IsEmpty()
				? StaticCastSharedRef<SWidget>(SNew(SSpacer).Size(FVector2D(0.f, 0.f)))
				: StaticCastSharedRef<SWidget>(
					SNew(SBox)
					.WidthOverride(52.f)
					.HeightOverride(52.f)
					[
						SNew(SImage)
						.Image(GetOrCreateAvatarBrush(SelectedEntry.AuthorAvatarUrl))
					])
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%s%s%s"),
					*GetOriginLabel(SelectedEntry),
					SelectedEntry.AuthorDisplayName.IsEmpty() ? TEXT("") : TEXT(" by "),
					*SelectedEntry.AuthorDisplayName)))
				.Font(FT66FlatStyle::Tokens::FontBold(16))
				.ColorAndOpacity(ChallengeGoldText)
				.AutoWrapText(true)
				.WrapTextAt(DetailColumnWidth - 84.f)
				.Clipping(EWidgetClipping::ClipToBounds)
			]
		];
		DetailLayout->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
		[
			SNew(SBox)
			.HeightOverride(132.f)
			[
				MakeChallengeSpritePanel(
					SNew(STextBlock)
					.Text(FText::FromString(SelectedEntry.Description))
					.Font(FT66FlatStyle::Tokens::FontBold(16))
					.ColorAndOpacity(ChallengePaperText)
					.AutoWrapText(true)
					.WrapTextAt(DetailColumnWidth - 86.f)
					.Clipping(EWidgetClipping::ClipToBounds),
					GetChallengeDetailPaperBrush(),
					FMargin(26.f, 24.f),
					ChallengePanelInsetFill())
			]
		];
		DetailLayout->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(DetailListHeader))
			.Font(FT66FlatStyle::Tokens::FontBold(22))
			.ColorAndOpacity(ChallengeFantasyText)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds)
		];
		DetailLayout->AddSlot().AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(248.f)
			[
				MakeChallengeSpritePanel(
					SNew(SScrollBox)
					.ScrollBarStyle(GetChallengeReferenceScrollBarStyle())
					.ScrollBarVisibility(EVisibility::Visible)
					.ScrollBarThickness(FVector2D(14.f, 14.f))
					.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
					+ SScrollBox::Slot()
					[
						RuleList
					],
					GetChallengeRulesPaperBrush(),
					FMargin(28.f, 22.f),
					ChallengePanelInsetFill())
			]
		];
		DetailLayout->AddSlot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Community ? Community->BuildRewardSummary(SelectedEntry) : TEXT("No reward data")))
			.Font(FT66FlatStyle::Tokens::FontBold(18))
			.ColorAndOpacity(ChallengeGoldText)
			.AutoWrapText(true)
			.WrapTextAt(DetailColumnWidth - 12.f)
			.Clipping(EWidgetClipping::ClipToBounds)
		];
		DetailLayout->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(STextBlock)
				.Text(FText::FromString(Community ? Community->BuildSelectionSummary(SelectedEntry) : TEXT("No selection summary available.")))
				.Font(FT66FlatStyle::Tokens::FontRegular(11))
				.ColorAndOpacity(ChallengeFantasyMuted)
				.AutoWrapText(true)
				.WrapTextAt(DetailColumnWidth - 12.f)
				.Clipping(EWidgetClipping::ClipToBounds)
		];

		if (SelectedEntry.IsDraft())
		{
			DetailLayout->AddSlot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(GetDraftSubmissionLabel(SelectedEntry)))
				.Font(FT66FlatStyle::Tokens::FontBold(14))
				.ColorAndOpacity(ChallengeDangerTint())
				.AutoWrapText(true)
				.WrapTextAt(DetailColumnWidth - 12.f)
				.Clipping(EWidgetClipping::ClipToBounds)
			];
			DetailLayout->AddSlot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
				[
					MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "EditDraft", "EDIT"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleEditDraftClicked), ET66ChallengeButtonFamily::CompactNeutral, 100.f, 34.f, 12)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
				[
					MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "SubmitSelectedDraft", "SUBMIT"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleSubmitDraftClicked), ET66ChallengeButtonFamily::ToggleOn, 112.f, 34.f, 12)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
				[
					MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "PlaySelectedDraft", "PLAY"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandlePlayDraftClicked), ET66ChallengeButtonFamily::ToggleOn, 112.f, 34.f, 12)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "DeleteSelectedDraft", "DELETE"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleDeleteDraftClicked), ET66ChallengeButtonFamily::ToggleOff, 112.f, 34.f, 12)
				]
			];
		}
		else
		{
			DetailLayout->AddSlot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(bSelectedEntryConfirmed ? TEXT("Selected for the next run.") : TEXT("Confirm this entry to arm it for the next run.")))
				.Font(FT66FlatStyle::Tokens::FontRegular(11))
				.ColorAndOpacity(ChallengeFantasyMuted)
				.AutoWrapText(true)
				.WrapTextAt(DetailColumnWidth - 12.f)
				.Clipping(EWidgetClipping::ClipToBounds)
			];
			DetailLayout->AddSlot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 16.f, 0.f, 0.f)
			[
				MakeChallengeSpriteButton(
					FText::FromString(bSelectedEntryConfirmed ? TEXT("SELECTED") : TEXT("CONFIRM")),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleConfirmClicked),
					ET66ChallengeButtonFamily::ToggleOn,
					270.f,
					62.f,
					20,
					FMargin(22.f, 10.f, 22.f, 8.f))
			];
		}

		DetailPanelContent =
			SNew(SScrollBox)
			.ScrollBarStyle(GetChallengeReferenceScrollBarStyle())
			.ScrollBarVisibility(EVisibility::Visible)
			.ScrollBarThickness(FVector2D(14.f, 14.f))
			.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
			+ SScrollBox::Slot()
			[
				DetailLayout
			];
	}

	const TSharedRef<SWidget> ChallengeCanvas = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black)
		[
			SNew(SBox)
			.WidthOverride(ReferenceCanvasWidth)
			.HeightOverride(ReferenceCanvasHeight)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(0.f, 26.f, 0.f, 0.f))
				[
					SNew(SBox)
					.WidthOverride(ReferenceCanvasWidth)
					.HeightOverride(887.f)
					[
						MakeChallengeSpritePanel(
							SNew(SSpacer),
							GetChallengeContentShellBrush(),
							FMargin(0.f),
							ChallengeShellFill())
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				.Padding(FMargin(0.f, 70.f, 0.f, 0.f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(HeaderTitle.ToUpper()))
					.Font(FT66FlatStyle::MakeFont(TEXT("Black"), 62))
					.ColorAndOpacity(ChallengeFantasyText)
					.ShadowOffset(FVector2D(0.f, 2.f))
					.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(254.f, 164.f, 0.f, 0.f))
				[
					MakeSourceTabButton(static_cast<int32>(ESourceTabIndex::Official), NSLOCTEXT("T66.Challenges", "OfficialTab", "OFFICIAL"))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(516.f, 164.f, 0.f, 0.f))
				[
					MakeSourceTabButton(static_cast<int32>(ESourceTabIndex::Community), NSLOCTEXT("T66.Challenges", "CommunityTab", "COMMUNITY"))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(810.f, 164.f, 0.f, 0.f))
				[
					CreateButtonContent
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(36.f, 209.f, 0.f, 0.f))
				[
					SNew(SBox)
					.WidthOverride(ReferenceCanvasWidth - 72.f)
					.HeightOverride(48.f)
					[
						MakeChallengeHorizontalSlicedPanel(
							SNew(STextBlock)
							.Text(FText::FromString(StatusMessage))
							.Font(FT66FlatStyle::Tokens::FontBold(17))
							.ColorAndOpacity(ChallengeFantasyMuted)
							.Justification(ETextJustify::Center)
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds),
							GetChallengeStatusBarBrush(),
							48.f,
							FMargin(28.f, 12.f, 28.f, 10.f),
							ChallengePanelFill(),
							0.125f)
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(0.f, 266.f, 0.f, 0.f))
				[
					SNew(SBox)
					.WidthOverride(ListPanelWidth)
					.HeightOverride(BodyPanelHeight)
					[
						MakeChallengeSpritePanel(
							SNew(SBox)
							.WidthOverride(ListColumnWidth)
							[
								ListPanelContent
							],
							GetChallengeListPanelBrush(),
							FMargin(22.f, 28.f),
							ChallengePanelFill())
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(716.f, 266.f, 0.f, 0.f))
				[
					SNew(SBox)
					.WidthOverride(DetailPanelWidth)
					.HeightOverride(BodyPanelHeight)
					[
						MakeChallengeSpritePanel(
							SNew(SBox)
							.WidthOverride(DetailColumnWidth)
							[
								DetailPanelContent
							],
							GetChallengeDetailFrameBrush(),
							FMargin(34.f, 28.f, 32.f, 28.f),
							ChallengePanelFill())
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				.Padding(FMargin(0.f, 834.f, 96.f, 0.f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.f, 0.f, 16.f, 0.f)
					[
						MakeChallengeSpriteButton(
							NSLOCTEXT("T66.Challenges", "FooterBack", "BACK"),
							FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleBackClicked),
							ET66ChallengeButtonFamily::CompactNeutral,
							214.f,
							62.f,
							20,
							FMargin(22.f, 10.f, 22.f, 8.f))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						MakeChallengeSpriteButton(
							FooterConfirmLabel,
							FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleConfirmClicked),
							FooterConfirmFamily,
							270.f,
							62.f,
							20,
							FMargin(22.f, 10.f, 22.f, 8.f))
					]
			]
		]
		];

	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Black)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Padding(FMargin(-14.f, 0.f, -14.f, 0.f))
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			[
				ChallengeCanvas
			]
		];
}

FReply UT66ChallengesScreen::HandleBackClicked()
{
	bSelectionStateInitialized = false;
	bRequestedCommunityRefresh = false;
	EndDraftEditor();
	if (bIsModal)
	{
		CloseModal();
	}
	else
	{
		NavigateBack();
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleTabSelected(const int32 TabIndex)
{
	InitializeSelectionState();
	ActiveTabIndex = FMath::Clamp(TabIndex, 0, static_cast<int32>(ETabIndex::Count) - 1);
	EndDraftEditor();
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleSourceTabSelected(const int32 SourceTabIndex)
{
	InitializeSelectionState();
	ActiveSourceTabIndex[ActiveTabIndex] = FMath::Clamp(SourceTabIndex, 0, static_cast<int32>(ESourceTabIndex::Count) - 1);
	FlatSelectedChallengeCardIndex = 0;
	EndDraftEditor();
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleEntrySelected(const int32 EntryIndex)
{
	const int32 SourceTabIndex = ActiveSourceTabIndex[ActiveTabIndex];
	const TArray<FT66CommunityContentEntry> Entries = GetEntriesForView(ActiveTabIndex, SourceTabIndex);
	if (Entries.IsValidIndex(EntryIndex))
	{
		PendingSelections[ActiveTabIndex][SourceTabIndex] = Entries[EntryIndex].LocalId;
	}

	EndDraftEditor();
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleFlatChallengeCardSelected(const int32 CardIndex)
{
	InitializeSelectionState();
	FlatSelectedChallengeCardIndex = FMath::Clamp(CardIndex, 0, 3);

	const int32 SourceTabIndex = ActiveSourceTabIndex[ActiveTabIndex];
	const TArray<FT66CommunityContentEntry> Entries = GetEntriesForView(ActiveTabIndex, SourceTabIndex);
	if (Entries.IsValidIndex(FlatSelectedChallengeCardIndex))
	{
		PendingSelections[ActiveTabIndex][SourceTabIndex] = Entries[FlatSelectedChallengeCardIndex].LocalId;
	}

	EndDraftEditor();
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleFlatPaginationSelected(const int32 PageIndex)
{
	FlatChallengePageIndex = FMath::Clamp(PageIndex, 0, 3);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleConfirmClicked()
{
	if (GetActiveKind() == ET66CommunityContentKind::Challenge && !bDraftEditorActive)
	{
		const int32 SourceTabIndex = ActiveSourceTabIndex[ActiveTabIndex];
		if (!GetEntriesForView(ActiveTabIndex, SourceTabIndex).IsValidIndex(FlatSelectedChallengeCardIndex))
		{
			return FReply::Handled();
		}
	}

	bool bActivatedEntry = false;
	FT66CommunityContentEntry Entry;
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		if (FindCurrentSelectedEntry(Entry))
		{
			Community->ActivateEntry(Entry.LocalId);
			bActivatedEntry = true;
		}
	}

	if (!bActivatedEntry)
	{
		return FReply::Handled();
	}

	bSelectionStateInitialized = false;
	if (bIsModal)
	{
		CloseModal();
	}
	else
	{
		NavigateBack();
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleCreateDraftClicked()
{
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		BeginDraftEditor(Community->CreateDraftTemplate(GetActiveKind()));
		RequestDeferredSlateRebuild();
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleEditDraftClicked()
{
	FT66CommunityContentEntry Entry;
	if (FindCurrentSelectedEntry(Entry) && Entry.IsDraft())
	{
		BeginDraftEditor(Entry);
		RequestDeferredSlateRebuild();
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleDeleteDraftClicked()
{
	FT66CommunityContentEntry Entry;
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		if (FindCurrentSelectedEntry(Entry) && Entry.IsDraft())
		{
			Community->DeleteDraft(Entry.LocalId);
			EndDraftEditor();
			PendingSelections[ActiveTabIndex][ActiveSourceTabIndex[ActiveTabIndex]] = NAME_None;
			RequestDeferredSlateRebuild();
		}
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleSaveDraftClicked()
{
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		Community->SaveDraft(DraftEditorEntry);
		PendingSelections[ActiveTabIndex][static_cast<int32>(ESourceTabIndex::Community)] = DraftEditorEntry.LocalId;
	}
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandlePlayDraftClicked()
{
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		if (bDraftEditorActive)
		{
			Community->SaveDraft(DraftEditorEntry);
			Community->ActivateEntry(DraftEditorEntry.LocalId);
		}
		else
		{
			FT66CommunityContentEntry Entry;
			if (FindCurrentSelectedEntry(Entry) && Entry.IsDraft())
			{
				Community->ActivateEntry(Entry.LocalId);
			}
		}
	}

	bSelectionStateInitialized = false;
	if (bIsModal)
	{
		CloseModal();
	}
	else
	{
		NavigateBack();
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleSubmitDraftClicked()
{
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		const FName DraftId = bDraftEditorActive ? DraftEditorEntry.LocalId : GetSelectedEntryIdForView(ActiveTabIndex, ActiveSourceTabIndex[ActiveTabIndex]);
		if (bDraftEditorActive)
		{
			Community->SaveDraft(DraftEditorEntry);
		}
		Community->SubmitDraftForApproval(DraftId);
	}
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleCancelDraftEditorClicked()
{
	EndDraftEditor();
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleAdjustDraftReward(const int32 Delta)
{
	DraftEditorEntry.SuggestedRewardChadCoupons = T66CommunityContentLimits::ClampRewardChadCoupons(DraftEditorEntry.SuggestedRewardChadCoupons + Delta);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleAdjustDraftStartLevel(const int32 Delta)
{
	DraftEditorEntry.Rules.StartLevelOverride = T66CommunityContentLimits::ClampStartLevelOverride(DraftEditorEntry.Rules.StartLevelOverride + Delta);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleAdjustDraftRequiredStage(const int32 Delta)
{
	DraftEditorEntry.Rules.RequiredStageReached = T66CommunityContentLimits::ClampRequiredStageReached(DraftEditorEntry.Rules.RequiredStageReached + Delta);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleAdjustDraftMaxRunTime(const int32 Delta)
{
	DraftEditorEntry.Rules.MaxRunTimeSeconds = T66CommunityContentLimits::ClampRunTimeSeconds(DraftEditorEntry.Rules.MaxRunTimeSeconds + Delta);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleToggleDraftMaxStats()
{
	DraftEditorEntry.Rules.bSetMaxHeroStats = !DraftEditorEntry.Rules.bSetMaxHeroStats;
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleAdjustDraftStatClicked(const EDraftStatField Field, const int32 Delta)
{
	AdjustDraftStat(Field, Delta);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleCycleDraftPassiveClicked(const int32 Direction)
{
	CycleDraftPassive(Direction);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleCycleDraftUltimateClicked(const int32 Direction)
{
	CycleDraftUltimate(Direction);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleCycleDraftStartingItemClicked(const int32 Direction)
{
	CycleDraftStartingItem(Direction);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

void UT66ChallengesScreen::HandleDraftTitleChanged(const FText& NewText)
{
	DraftEditorEntry.Title = NewText.ToString();
}

void UT66ChallengesScreen::HandleDraftDescriptionChanged(const FText& NewText)
{
	DraftEditorEntry.Description = NewText.ToString();
}

void UT66ChallengesScreen::HandleDraftFullClearChanged(const ECheckBoxState NewState)
{
	DraftEditorEntry.Rules.bRequireFullClear = (NewState == ECheckBoxState::Checked);
}

void UT66ChallengesScreen::HandleDraftNoDamageChanged(const ECheckBoxState NewState)
{
	DraftEditorEntry.Rules.bRequireNoDamage = (NewState == ECheckBoxState::Checked);
}

void UT66ChallengesScreen::HandleCommunityContentChanged()
{
	RequestDeferredSlateRebuild();
}
