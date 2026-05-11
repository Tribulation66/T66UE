// Copyright Tribulation 66. All Rights Reserved.

class SBorder;
class SBox;
class STextBlock;

#include "UI/Screens/T66CompanionSelectionScreen.h"
#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"
#include "Engine/GameInstance.h"
#include "Engine/TextureDefines.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/Screens/T66SelectionScreenUtils.h"
#include "UI/Screens/T66ChallengesScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66SkinSubsystem.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66CompanionBase.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66CompanionPreviewStage.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Gameplay/T66FrontendGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Notifications/SProgressBar.h"
// Render target removed — in-world preview uses main viewport camera with full Lumen GI.

using namespace T66HeroSelectionPrivate;

namespace
{
	FText FormatCompanionRecordRankText(const int32 Rank)
	{
		if (Rank <= 0)
		{
			return NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankUnranked", "Unranked");
		}

		if (Rank <= 10000)
		{
			return FText::Format(
				NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankExactFormat", "#{0}"),
				FText::AsNumber(Rank));
		}

		if (Rank <= 25000)
		{
			return NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankTop25K", "Top 25K");
		}

		if (Rank <= 50000)
		{
			return NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankTop50K", "Top 50K");
		}

		if (Rank <= 100000)
		{
			return NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankTop100K", "Top 100K");
		}

		return NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankUnranked", "Unranked");
	}

	FText FormatCompanionPassiveHealText(const ET66Difficulty Difficulty)
	{
		const float Amount = AT66CompanionBase::GetHealingAmountForDifficulty(Difficulty);
		const float Interval = AT66CompanionBase::GetHealingIntervalSecondsForDifficulty(Difficulty);
		return FText::Format(
			NSLOCTEXT("T66.CompanionSelection", "PassiveHealFormat", "PASSIVE: Heals the hero {0} health every {1} seconds."),
			FText::AsNumber(FMath::RoundToInt(Amount)),
			FText::AsNumber(Interval));
	}

	FText ResolveCompanionLoreText(const UT66LocalizationSubsystem* Loc, const FCompanionData& CompanionData)
	{
		if (!CompanionData.LoreText.IsEmpty())
		{
			return CompanionData.LoreText;
		}
		return Loc
			? Loc->GetText_CompanionLore(CompanionData.CompanionID)
			: NSLOCTEXT("T66.CompanionSelection", "FallbackCompanionLore", "A mysterious companion.");
	}

	AT66PlayerController* T66GetLocalFrontendCompanionPlayerController(UObject* ContextObject)
	{
		return ContextObject ? Cast<AT66PlayerController>(UGameplayStatics::GetPlayerController(ContextObject, 0)) : nullptr;
	}

	void T66PositionCompanionPreviewCamera(UObject* ContextObject)
	{
		if (!ContextObject)
		{
			return;
		}

		if (UWorld* World = ContextObject->GetWorld())
		{
			if (AT66FrontendGameMode* GM = Cast<AT66FrontendGameMode>(World->GetAuthGameMode()))
			{
				GM->PositionCameraForCompanionPreview();
				return;
			}
		}

		if (AT66PlayerController* PC = T66GetLocalFrontendCompanionPlayerController(ContextObject))
		{
			PC->PositionLocalFrontendCameraForCompanionPreview();
		}
	}

	enum class ET66CompanionReferenceButtonFamily : uint8
	{
		CompactNeutral,
		ToggleOn,
		ToggleOff,
		CtaPrimary
	};

	enum class ET66CompanionReferenceButtonState : uint8
	{
		Normal,
		Hovered,
		Pressed,
		Disabled
	};

	struct FCompanionReferenceButtonBrushSet
	{
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Normal;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Hovered;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Pressed;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Disabled;
	};

	FString MakeCompanionUltrakillElementPath(const TCHAR* FileName)
	{
		return FString::Printf(TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/%s"), FileName);
	}

	const FSlateBrush* ResolveCompanionReferenceBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const FString& RelativePath,
		const FMargin& Margin,
		const TCHAR* DebugLabel,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(RelativePath),
			Margin,
			DebugLabel,
			Filter);
	}

	const FSlateBrush* ResolveCompanionReferenceRegionBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const FString& RelativePath,
		const FVector2D& ImageSize,
		const FMargin& Margin,
		const FBox2f& UVRegion,
		const FLinearColor& Tint,
		const TCHAR* DebugLabel,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		const FSlateBrush* Brush = ResolveCompanionReferenceBrush(Entry, RelativePath, Margin, DebugLabel, Filter);
		if (Brush && Entry.Brush.IsValid())
		{
			Entry.Brush->DrawAs = ESlateBrushDrawType::Box;
			Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
			Entry.Brush->TintColor = FSlateColor(Tint);
			Entry.Brush->ImageSize = ImageSize;
			Entry.Brush->Margin = Margin;
			Entry.Brush->SetUVRegion(UVRegion);
			return Entry.Brush.Get();
		}
		return nullptr;
	}

	FCompanionReferenceButtonBrushSet& GetCompanionReferenceButtonBrushSet(const ET66CompanionReferenceButtonFamily Family)
	{
		static FCompanionReferenceButtonBrushSet CompactNeutral;
		static FCompanionReferenceButtonBrushSet ToggleOn;
		static FCompanionReferenceButtonBrushSet ToggleOff;
		static FCompanionReferenceButtonBrushSet CtaPrimary;

		switch (Family)
		{
		case ET66CompanionReferenceButtonFamily::ToggleOn:
			return ToggleOn;
		case ET66CompanionReferenceButtonFamily::ToggleOff:
			return ToggleOff;
		case ET66CompanionReferenceButtonFamily::CtaPrimary:
			return CtaPrimary;
		case ET66CompanionReferenceButtonFamily::CompactNeutral:
		default:
			return CompactNeutral;
		}
	}

	FString GetCompanionReferenceButtonPath(
		const ET66CompanionReferenceButtonFamily Family,
		const ET66CompanionReferenceButtonState State)
	{
		const TCHAR* StateSuffix = TEXT("normal");
		switch (State)
		{
		case ET66CompanionReferenceButtonState::Hovered:
			StateSuffix = TEXT("hover");
			break;
		case ET66CompanionReferenceButtonState::Pressed:
			StateSuffix = TEXT("pressed");
			break;
		case ET66CompanionReferenceButtonState::Disabled:
			StateSuffix = TEXT("disabled");
			break;
		case ET66CompanionReferenceButtonState::Normal:
		default:
			break;
		}

		if (Family == ET66CompanionReferenceButtonFamily::ToggleOn && State == ET66CompanionReferenceButtonState::Normal)
		{
			StateSuffix = TEXT("selected");
		}

		if (Family == ET66CompanionReferenceButtonFamily::CtaPrimary)
		{
			const FString CtaState = FString(StateSuffix).Equals(TEXT("selected"), ESearchCase::IgnoreCase)
				? FString(TEXT("normal"))
				: FString(StateSuffix);
			return T66ScreenSlateHelpers::MakeReferenceRedSquareButtonAssetPath(*CtaState);
		}

		return MakeCompanionUltrakillElementPath(*FString::Printf(TEXT("SquareVariant/leaderboard_tab_button_%s_square_variant.png"), StateSuffix));
	}

	FMargin GetCompanionReferenceButtonMargin(const ET66CompanionReferenceButtonFamily /*Family*/)
	{
		return FMargin(0.f);
	}

	const FSlateBrush* ResolveCompanionReferenceButtonBrush(
		const ET66CompanionReferenceButtonFamily Family,
		const ET66CompanionReferenceButtonState State)
	{
		FCompanionReferenceButtonBrushSet& Set = GetCompanionReferenceButtonBrushSet(Family);
		T66RuntimeUIBrushAccess::FOptionalTextureBrush* Entry = &Set.Normal;
		if (State == ET66CompanionReferenceButtonState::Hovered)
		{
			Entry = &Set.Hovered;
		}
		else if (State == ET66CompanionReferenceButtonState::Pressed)
		{
			Entry = &Set.Pressed;
		}
		else if (State == ET66CompanionReferenceButtonState::Disabled)
		{
			Entry = &Set.Disabled;
		}

		return ResolveCompanionReferenceBrush(
			*Entry,
			GetCompanionReferenceButtonPath(Family, State),
			GetCompanionReferenceButtonMargin(Family),
			TEXT("CompanionReferenceButton"),
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetCompanionLeftPanelShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveCompanionReferenceBrush(
			Entry,
			MakeCompanionUltrakillElementPath(TEXT("SquareVariant/main_panel_normal_square_variant.png")),
			FMargin(0.035f, 0.105f, 0.035f, 0.105f),
			TEXT("CompanionLeftShellReference12"),
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetCompanionRightPanelShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveCompanionReferenceBrush(
			Entry,
			MakeCompanionUltrakillElementPath(TEXT("SquareVariant/main_panel_normal_square_variant.png")),
			FMargin(0.035f, 0.105f, 0.035f, 0.105f),
			TEXT("CompanionRightShellReference12"),
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetCompanionPaperFrameBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveCompanionReferenceBrush(
			Entry,
			MakeCompanionUltrakillElementPath(TEXT("SquareVariant/main_panel_normal_square_variant.png")),
			FMargin(0.095f, 0.13f, 0.095f, 0.13f),
			TEXT("CompanionPaperFrameReference12"),
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetCompanionRowShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveCompanionReferenceBrush(
			Entry,
			MakeCompanionUltrakillElementPath(TEXT("SquareVariant/player_row_panel_normal_square_variant.png")),
			FMargin(0.070f, 0.155f, 0.070f, 0.155f),
			TEXT("CompanionRowShellV16"),
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetCompanionFieldShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveCompanionReferenceBrush(
			Entry,
			MakeCompanionUltrakillElementPath(TEXT("SquareVariant/dropdown_field_normal_square_variant.png")),
			FMargin(0.06f, 0.34f, 0.06f, 0.34f),
			TEXT("CompanionFieldShell"));
	}

	const FSlateBrush* GetCompanionAvatarFrameBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveCompanionReferenceBrush(
			Entry,
			MakeCompanionUltrakillElementPath(TEXT("SquareVariant/profile_slot_normal_square_variant.png")),
			FMargin(0.20f, 0.20f, 0.20f, 0.20f),
			TEXT("CompanionAvatarFrame"));
	}

	const FSlateBrush* GetCompanionSceneBackgroundBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveCompanionReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/CompanionSelection/ScreenArt/companionselection_screen_art_center_scene_v1.png"),
			FMargin(0.f),
			TEXT("CompanionSceneBackground"));
	}

	const FScrollBarStyle* GetCompanionReferenceScrollBarStyle()
	{
		static FScrollBarStyle Style = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar");
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush TrackEntry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush ThumbEntry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush HoverEntry;

		const FString ControlsPath = TEXT("SourceAssets/UI/Reference/Screens/CompanionSelection/Controls/companionselection_controls_controls_sheet.png");
		const FBox2f VerticalBarUV(
			FVector2f(4.f / 1350.f, 4.f / 926.f),
			FVector2f(90.f / 1350.f, 644.f / 926.f));

		const FSlateBrush* TrackBrush = ResolveCompanionReferenceRegionBrush(
			TrackEntry,
			ControlsPath,
			FVector2D(14.f, 120.f),
			FMargin(0.42f, 0.085f, 0.42f, 0.085f),
			VerticalBarUV,
			FLinearColor(0.35f, 0.34f, 0.30f, 0.70f),
			TEXT("CompanionScrollbarTrackV16"),
			TextureFilter::TF_Nearest);
		const FSlateBrush* ThumbBrush = ResolveCompanionReferenceRegionBrush(
			ThumbEntry,
			ControlsPath,
			FVector2D(16.f, 96.f),
			FMargin(0.38f, 0.115f, 0.38f, 0.115f),
			VerticalBarUV,
			FLinearColor(0.93f, 0.82f, 0.52f, 1.0f),
			TEXT("CompanionScrollbarThumbV16"),
			TextureFilter::TF_Nearest);
		const FSlateBrush* HoverBrush = ResolveCompanionReferenceRegionBrush(
			HoverEntry,
			ControlsPath,
			FVector2D(16.f, 96.f),
			FMargin(0.38f, 0.115f, 0.38f, 0.115f),
			VerticalBarUV,
			FLinearColor(1.0f, 0.90f, 0.62f, 1.0f),
			TEXT("CompanionScrollbarHoverV16"),
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

	TSharedRef<SWidget> MakeCompanionReferencePanel(
		const TSharedRef<SWidget>& Content,
		const FSlateBrush* Brush,
		const FMargin& Padding,
		const FSlateColor& FallbackFill)
	{
		if (Brush)
		{
			return SNew(SBorder)
				.BorderImage(Brush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(FallbackFill)
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeCompanionReferencePaperPanel(
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding,
		const FSlateColor& FallbackFill)
	{
		if (const FSlateBrush* Brush = GetCompanionPaperFrameBrush())
		{
			return SNew(SBorder)
				.BorderImage(Brush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(FallbackFill)
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeCompanionReferenceRow(
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding,
		const FSlateColor& FallbackFill)
	{
		if (const FSlateBrush* Brush = GetCompanionRowShellBrush())
		{
			return SNew(SBorder)
				.BorderImage(Brush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel2)
				.SetColor(FallbackFill)
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeCompanionReferenceField(
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding,
		const FSlateColor& FallbackFill)
	{
		if (const FSlateBrush* Brush = GetCompanionFieldShellBrush())
		{
			return SNew(SBorder)
				.BorderImage(Brush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(FallbackFill)
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeCompanionAvatarSocket(
		const TSharedRef<SWidget>& Content,
		const FLinearColor& FallbackFill,
		const float Opacity,
		const bool bSelected)
	{
		if (const FSlateBrush* FrameBrush = GetCompanionAvatarFrameBrush())
		{
			const FMargin ContentInset = bSelected ? FMargin(5.f) : FMargin(4.f);
			return SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FallbackFill * FMath::Clamp(Opacity, 0.0f, 1.0f))
				]
				+ SOverlay::Slot()
				.Padding(ContentInset)
				[
					Content
				]
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image(FrameBrush)
					.ColorAndOpacity(bSelected
						? FLinearColor(1.12f, 1.04f, 0.82f, 1.0f)
						: FLinearColor(0.78f, 0.88f, 0.78f, 0.88f))
				];
		}

		return FT66Style::MakeSlotFrame(Content, FallbackFill * Opacity, FMargin(2.f));
	}

	class ST66CompanionReferenceButton : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66CompanionReferenceButton)
			: _ButtonFamily(ET66CompanionReferenceButtonFamily::CompactNeutral)
			, _MinWidth(0.f)
			, _Height(0.f)
			, _ContentPadding(FMargin(0.f))
			, _IsEnabled(true)
			, _Visibility(EVisibility::Visible)
		{
		}
			SLATE_ATTRIBUTE(ET66CompanionReferenceButtonFamily, ButtonFamily)
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
			ButtonFamily = InArgs._ButtonFamily;
			ContentPadding = InArgs._ContentPadding;
			OwnedButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			OwnedButtonStyle.SetNormalPadding(FMargin(0.f));
			OwnedButtonStyle.SetPressedPadding(FMargin(0.f));

			ChildSlot
			[
				FT66Style::MakeBareButton(
					FT66BareButtonParams(
						InArgs._OnClicked,
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							T66ScreenSlateHelpers::MakeReferenceHorizontalSlicedImage(
								TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateSP(this, &ST66CompanionReferenceButton::GetCurrentBrush)),
								FVector2D(1.0f, 1.0f),
								0.105f)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Padding(this, &ST66CompanionReferenceButton::GetContentPadding)
							[
								InArgs._Content.Widget
							]
						])
					.SetButtonStyle(&OwnedButtonStyle)
					.SetPadding(FMargin(0.f))
					.SetEnabled(InArgs._IsEnabled)
					.SetMinWidth(T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth(InArgs._MinWidth, InArgs._Height))
					.SetHeight(InArgs._Height)
					.SetVisibility(InArgs._Visibility),
					&Button)
			];
		}

	private:
		const FSlateBrush* GetCurrentBrush() const
		{
			const ET66CompanionReferenceButtonFamily Family = ButtonFamily.Get(ET66CompanionReferenceButtonFamily::CompactNeutral);
			if (!Button.IsValid() || !Button->IsEnabled())
			{
				return ResolveCompanionReferenceButtonBrush(Family, ET66CompanionReferenceButtonState::Disabled);
			}
			if (Button->IsPressed())
			{
				return ResolveCompanionReferenceButtonBrush(Family, ET66CompanionReferenceButtonState::Pressed);
			}
			if (Button->IsHovered())
			{
				return ResolveCompanionReferenceButtonBrush(Family, ET66CompanionReferenceButtonState::Hovered);
			}
			return ResolveCompanionReferenceButtonBrush(Family, ET66CompanionReferenceButtonState::Normal);
		}

		FMargin GetContentPadding() const
		{
			if (Button.IsValid() && Button->IsPressed())
			{
				return FMargin(
					ContentPadding.Left,
					ContentPadding.Top + 1.f,
					ContentPadding.Right,
					FMath::Max(0.f, ContentPadding.Bottom - 1.f));
			}
			return ContentPadding;
		}

		TAttribute<ET66CompanionReferenceButtonFamily> ButtonFamily;
		FMargin ContentPadding = FMargin(0.f);
		FButtonStyle OwnedButtonStyle;
		TSharedPtr<SButton> Button;
	};

	TSharedRef<SWidget> MakeCompanionReferenceButton(
		const FT66ButtonParams& Params,
		TAttribute<ET66CompanionReferenceButtonFamily> ButtonFamily)
	{
		const ET66CompanionReferenceButtonFamily InitialFamily = ButtonFamily.Get(ET66CompanionReferenceButtonFamily::CompactNeutral);
		if (!ResolveCompanionReferenceButtonBrush(InitialFamily, ET66CompanionReferenceButtonState::Normal))
		{
			return FT66Style::MakeButton(Params);
		}

		const float ButtonHeight = Params.Height > 0.f ? Params.Height : 44.f;
		const FMargin ContentPadding = Params.Padding.Left >= 0.f ? Params.Padding : FMargin(6.f, 2.f);

		const TSharedRef<SWidget> Content = Params.CustomContent.IsValid()
			? Params.CustomContent.ToSharedRef()
			: T66ScreenSlateHelpers::MakeFilledButtonText(
				Params,
				ButtonHeight,
				TAttribute<FSlateColor>(FSlateColor(FT66Style::Tokens::Text)),
				TAttribute<FLinearColor>(FLinearColor(0.f, 0.f, 0.f, 0.68f)));

		return SNew(ST66CompanionReferenceButton)
			.ButtonFamily(ButtonFamily)
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

	TSharedRef<SWidget> MakeCompanionReferenceButton(
		const FT66ButtonParams& Params,
		const ET66CompanionReferenceButtonFamily ButtonFamily)
	{
		return MakeCompanionReferenceButton(Params, TAttribute<ET66CompanionReferenceButtonFamily>(ButtonFamily));
	}
}

UT66CompanionSelectionScreen::UT66CompanionSelectionScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::CompanionSelection;
	bIsModal = false;
}

UT66LocalizationSubsystem* UT66CompanionSelectionScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

bool UT66CompanionSelectionScreen::IsCompanionUnlocked(FName CompanionID) const
{
	if (CompanionID.IsNone())
	{
		return true;
	}

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66CompanionUnlockSubsystem* Unlocks = GI->GetSubsystem<UT66CompanionUnlockSubsystem>())
		{
			return Unlocks->IsCompanionUnlocked(CompanionID);
		}
	}
	return true; // fail-open so we don't hard-lock the UI if subsystem is missing
}

void UT66CompanionSelectionScreen::GeneratePlaceholderSkins()
{
	PlaceholderSkins.Empty();
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66SkinSubsystem* Skin = GI ? GI->GetSubsystem<UT66SkinSubsystem>() : nullptr;
	FName CompanionForSkins = PreviewedCompanionID.IsNone() && AllCompanionIDs.Num() > 0 ? AllCompanionIDs[0] : PreviewedCompanionID;
	if (Skin && !CompanionForSkins.IsNone())
	{
		PlaceholderSkins = Skin->GetSkinsForEntity(ET66SkinEntityType::Companion, CompanionForSkins);
	}
	else
	{
		T66SelectionScreenUtils::PopulateDefaultOwnedSkins(PlaceholderSkins);
	}
}

void UT66CompanionSelectionScreen::RefreshSkinsList()
{
	GeneratePlaceholderSkins();
	if (SkinsListBoxWidget.IsValid())
	{
		SkinsListBoxWidget->ClearChildren();
		AddSkinRowsToBox(SkinsListBoxWidget);
	}
	if (ACBalanceTextBlock.IsValid())
	{
		ACBalanceTextBlock->SetText(T66SelectionScreenUtils::FormatAchievementCoinBalance(
			GetLocSubsystem(),
			T66SelectionScreenUtils::GetAchievementCoinBalance(this)));
	}
	UpdateCompanionDisplay();
}

void UT66CompanionSelectionScreen::AddSkinRowsToBox(const TSharedPtr<SVerticalBox>& Box)
{
	if (!Box.IsValid()) return;
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const FText EquipText = Loc ? Loc->GetText_Equip() : NSLOCTEXT("T66.Common", "Equip", "EQUIP");
	const FText EquippedText = NSLOCTEXT("T66.HeroSelection", "Equipped", "EQUIPPED");
	const FText PreviewText = Loc ? Loc->GetText_Preview() : NSLOCTEXT("T66.Common", "Preview", "PREVIEW");
	const FText BuyText = Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY");
	static constexpr int32 BeachgoerPriceAC = UT66SkinSubsystem::DefaultSkinPriceAC;
	const float ActionMinHeight = 36.f;
	const float ActionMinWidth = 96.f;
	const float EquippedMinWidth = 104.f;
	const float BuyButtonMinWidth = 108.f;
	const float BuyButtonHeight = 44.f;
	const FText BeachgoerPriceText = T66SelectionScreenUtils::FormatAchievementCoinBalance(Loc, BeachgoerPriceAC);
	const FSlateColor SkinRowFill = FT66Style::IsDotaTheme()
		? FSlateColor(FLinearColor(0.028f, 0.028f, 0.031f, 1.0f))
		: FT66Style::Tokens::Panel2;
	const FSlateColor SkinFieldFill = FT66Style::IsDotaTheme()
		? FSlateColor(FLinearColor(0.075f, 0.075f, 0.08f, 1.0f))
		: FT66Style::Tokens::Accent2;

	for (const FSkinData& Skin : PlaceholderSkins)
	{
		FName SkinIDCopy = Skin.SkinID;
		bool bIsDefault = Skin.bIsDefault;
		bool bIsOwned = Skin.bIsOwned;
		bool bIsEquipped = Skin.bIsEquipped;
		FName CID = PreviewedCompanionID.IsNone() && AllCompanionIDs.Num() > 0 ? AllCompanionIDs[0] : PreviewedCompanionID;

		const FLinearColor SkinSwatchFill = bIsDefault
			? FLinearColor(0.11f, 0.065f, 0.035f, 1.0f)
			: FLinearColor(0.025f, 0.21f, 0.28f, 1.0f);

		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 14.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(42.f)
				.HeightOverride(42.f)
				[
					MakeCompanionAvatarSocket(
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(SkinSwatchFill)
						[
							SNew(SBox)
						],
						SkinSwatchFill,
						1.0f,
						bIsEquipped)
				]
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Loc ? Loc->GetText_SkinName(SkinIDCopy) : FText::FromName(SkinIDCopy))
				.Font(FT66Style::Tokens::FontRegular(16))
				.ColorAndOpacity(FLinearColor(0.075f, 0.055f, 0.025f, 1.0f))
			];

		if (bIsDefault)
		{
			Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(5.0f, 0.0f)
				[
					SNew(SBox).MinDesiredWidth(EquippedMinWidth).MinDesiredHeight(ActionMinHeight)
					[
						SNew(SWidgetSwitcher)
						.WidgetIndex(bIsEquipped ? 1 : 0)
						+ SWidgetSwitcher::Slot()
						[
						MakeCompanionReferenceButton(FT66ButtonParams(EquipText,
							FOnClicked::CreateLambda([this, CID]()
							{
								if (CID.IsNone()) return FReply::Handled();
								if (UT66SkinSubsystem* SkinSub = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UT66SkinSubsystem>())
								{
									SkinSub->SetEquippedCompanionSkinID(CID, UT66SkinSubsystem::DefaultSkinID);
									PreviewedCompanionSkinIDOverride = NAME_None;
									RefreshSkinsList();
								}
								return FReply::Handled();
							}),
							ET66ButtonType::Primary).SetMinWidth(ActionMinWidth).SetHeight(ActionMinHeight).SetFontSize(16),
							ET66CompanionReferenceButtonFamily::ToggleOn)
						]
						+ SWidgetSwitcher::Slot()
						[
							SNew(SBox).MinDesiredWidth(EquippedMinWidth).HeightOverride(ActionMinHeight)
							[
								MakeCompanionReferenceField(
									SNew(STextBlock)
									.Text(EquippedText)
									.Font(FT66Style::Tokens::FontBold(16))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.Justification(ETextJustify::Center),
									FMargin(10.0f, 4.0f),
									SkinFieldFill)
							]
						]
					]
				];
		}
		else
		{
			Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(5.0f, 0.0f)
				[
					MakeCompanionReferenceButton(FT66ButtonParams(PreviewText,
					FOnClicked::CreateLambda([this, SkinIDCopy]()
					{
						PreviewedCompanionSkinIDOverride = (PreviewedCompanionSkinIDOverride == SkinIDCopy) ? NAME_None : SkinIDCopy;
						UpdateCompanionDisplay();
						return FReply::Handled();
					}),
					ET66ButtonType::Neutral).SetMinWidth(ActionMinWidth).SetHeight(ActionMinHeight).SetFontSize(16),
					ET66CompanionReferenceButtonFamily::CompactNeutral)
				];
			Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(4.0f, 0.0f)
				[
					SNew(SBox).MinDesiredWidth(EquippedMinWidth).MinDesiredHeight(BuyButtonHeight)
					[
						SNew(SWidgetSwitcher)
						.WidgetIndex(!bIsOwned ? 0 : (bIsEquipped ? 2 : 1))
						+ SWidgetSwitcher::Slot()
						[
						MakeCompanionReferenceButton(FT66ButtonParams(BuyText,
							FOnClicked::CreateLambda([this, CID, SkinIDCopy]()
							{
								if (CID.IsNone()) return FReply::Handled();
								UT66SkinSubsystem* SkinSub = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UT66SkinSubsystem>();
								if (!SkinSub || !SkinSub->PurchaseCompanionSkin(CID, SkinIDCopy, BeachgoerPriceAC)) return FReply::Handled();
								SkinSub->SetEquippedCompanionSkinID(CID, SkinIDCopy);
								PreviewedCompanionSkinIDOverride = NAME_None;
								RefreshSkinsList();
								return FReply::Handled();
							}),
							ET66ButtonType::Primary)
							.SetMinWidth(BuyButtonMinWidth)
							.SetHeight(BuyButtonHeight)
							.SetColor(HeroSelectionChromeTokenAccent())
							.SetPadding(FMargin(6.f, 3.f))
							.SetContent(
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[ SNew(STextBlock).Text(BuyText).Font(FT66Style::Tokens::FontBold(15)).ColorAndOpacity(FT66Style::Tokens::Text) ]
								+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[ SNew(STextBlock).Text(BeachgoerPriceText).Font(FT66Style::Tokens::FontRegular(13)).ColorAndOpacity(FT66Style::Tokens::Text) ]
							),
							ET66CompanionReferenceButtonFamily::ToggleOn)
						]
						+ SWidgetSwitcher::Slot()
						[
						MakeCompanionReferenceButton(FT66ButtonParams(EquipText,
							FOnClicked::CreateLambda([this, CID, SkinIDCopy]()
							{
								if (CID.IsNone()) return FReply::Handled();
								if (UT66SkinSubsystem* SkinSub = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UT66SkinSubsystem>())
								{
									SkinSub->SetEquippedCompanionSkinID(CID, SkinIDCopy);
									PreviewedCompanionSkinIDOverride = NAME_None;
									RefreshSkinsList();
								}
								return FReply::Handled();
							}),
							ET66ButtonType::Primary).SetMinWidth(ActionMinWidth).SetHeight(ActionMinHeight).SetFontSize(16),
							ET66CompanionReferenceButtonFamily::ToggleOn)
						]
						+ SWidgetSwitcher::Slot()
						[
							SNew(SBox).MinDesiredWidth(EquippedMinWidth).HeightOverride(ActionMinHeight)
							[
								MakeCompanionReferenceField(
									SNew(STextBlock)
									.Text(EquippedText)
									.Font(FT66Style::Tokens::FontBold(16))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.Justification(ETextJustify::Center),
									FMargin(10.0f, 4.0f),
									SkinFieldFill)
							]
						]
					]
				];
		}
		Box->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 5.0f)
			[
				SNew(SBox)
				.HeightOverride(86.f)
				[
					MakeCompanionReferencePaperPanel(
						Row,
						FMargin(18.f, 10.f),
						SkinRowFill)
				]
			];
	}
}

TSharedRef<SWidget> UT66CompanionSelectionScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66PartySubsystem* PartySubsystem = T66GI ? T66GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = T66GI ? T66GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	RefreshCompanionList();

	if (T66GI)
	{
		SelectedDifficulty = T66GI->ResolvePlayableDifficulty(SessionSubsystem ? SessionSubsystem->GetSharedLobbyDifficulty() : T66GI->SelectedDifficulty);
		T66GI->SelectedDifficulty = SelectedDifficulty;
	}

	if ((PreviewedCompanionID.IsNone() || !AllCompanionIDs.Contains(PreviewedCompanionID)) && T66GI)
	{
		if (!T66GI->SelectedCompanionID.IsNone() && AllCompanionIDs.Contains(T66GI->SelectedCompanionID))
		{
			PreviewedCompanionID = T66GI->SelectedCompanionID;
		}
		else if (AllCompanionIDs.Num() > 0)
		{
			PreviewedCompanionID = AllCompanionIDs[0];
		}
	}
	CurrentCompanionIndex = AllCompanionIDs.IndexOfByKey(PreviewedCompanionID);
	if (CurrentCompanionIndex == INDEX_NONE && AllCompanionIDs.Num() > 0)
	{
		CurrentCompanionIndex = 0;
		PreviewedCompanionID = AllCompanionIDs[0];
	}

	GeneratePlaceholderSkins();
	SAssignNew(SkinsListBoxWidget, SVerticalBox);
	AddSkinRowsToBox(SkinsListBoxWidget);

	const FText SkinsText = Loc ? Loc->GetText_Skins() : NSLOCTEXT("T66.CompanionSelection", "Skins", "SKINS");
	const FText LoreText = Loc ? Loc->GetText_Lore() : NSLOCTEXT("T66.CompanionSelection", "Lore", "LORE");
	const FText ConfirmText = Loc ? Loc->GetText_ConfirmCompanion() : NSLOCTEXT("T66.CompanionSelection", "ConfirmCompanion", "CONFIRM COMPANION");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const FText EnterText = NSLOCTEXT("T66.CompanionSelection", "EnterShort", "ENTER");
	const FText ReadyText = NSLOCTEXT("T66.CompanionSelection", "Ready", "READY");
	const FText UnreadyText = NSLOCTEXT("T66.CompanionSelection", "Unready", "UNREADY");
	const FText WaitingForPartyText = NSLOCTEXT("T66.CompanionSelection", "WaitingForParty", "WAITING FOR PARTY");
	const FText ACBalanceText = FText::AsNumber(T66SelectionScreenUtils::GetAchievementCoinBalance(this));

	DifficultyOptions.Empty();
	const TArray<ET66Difficulty> Difficulties = T66GI ? T66GI->GetPlayableDifficulties() : TArray<ET66Difficulty>{
		ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard, ET66Difficulty::VeryHard, ET66Difficulty::Impossible
	};
	for (const ET66Difficulty Difficulty : Difficulties)
	{
		DifficultyOptions.Add(MakeShared<FString>((Loc ? Loc->GetText_Difficulty(Difficulty) : FText::FromString(TEXT("?"))).ToString()));
	}
	const int32 CurrentDifficultyIndex = Difficulties.IndexOfByKey(SelectedDifficulty);
	if (DifficultyOptions.IsValidIndex(CurrentDifficultyIndex))
	{
		CurrentDifficultyOption = DifficultyOptions[CurrentDifficultyIndex];
	}
	else if (DifficultyOptions.Num() > 0)
	{
		CurrentDifficultyOption = DifficultyOptions[0];
		SelectedDifficulty = Difficulties.IsValidIndex(0) ? Difficulties[0] : ET66Difficulty::Easy;
	}

	FText CurrentCompanionName = Loc ? Loc->GetText_NoCompanion() : NSLOCTEXT("T66.CompanionSelection", "NoCompanion", "NO COMPANION");
	FText CurrentCompanionLore = NSLOCTEXT("T66.CompanionSelection", "NoCompanionLore", "Select a companion to learn their story.");
	FLinearColor PreviewColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);
	if (!PreviewedCompanionID.IsNone())
	{
		FCompanionData Data;
		if (GetPreviewedCompanionData(Data))
		{
			CurrentCompanionName = Loc ? Loc->GetCompanionDisplayName(Data) : Data.DisplayName;
			CurrentCompanionLore = ResolveCompanionLoreText(Loc, Data);
			PreviewColor = Data.PlaceholderColor;
		}
	}
	if (!PreviewedCompanionID.IsNone() && !IsCompanionUnlocked(PreviewedCompanionID))
	{
		PreviewColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);
	}

	CompanionCarouselPortraitBrushes.SetNum(HeroSelectionCarouselVisibleSlots);
	for (int32 i = 0; i < CompanionCarouselPortraitBrushes.Num(); ++i)
	{
		if (!CompanionCarouselPortraitBrushes[i].IsValid())
		{
			CompanionCarouselPortraitBrushes[i] = MakeShared<FSlateBrush>();
			CompanionCarouselPortraitBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
			CompanionCarouselPortraitBrushes[i]->ImageSize = FVector2D(128.f, 128.f);
		}
	}
	RefreshCompanionCarouselPortraits();

	TSharedRef<SHorizontalBox> CompanionCarousel = SNew(SHorizontalBox);
	for (int32 Offset = -HeroSelectionCarouselCenterIndex; Offset <= HeroSelectionCarouselCenterIndex; ++Offset)
	{
		if (AllCompanionIDs.Num() == 0)
		{
			break;
		}
		const int32 SlotIdx = Offset + HeroSelectionCarouselCenterIndex;
		const int32 Idx = (CurrentCompanionIndex + Offset + AllCompanionIDs.Num() * 2) % AllCompanionIDs.Num();
		const FName CompanionID = AllCompanionIDs.IsValidIndex(Idx) ? AllCompanionIDs[Idx] : NAME_None;
		const bool bCenterSlot = Offset == 0;
		const bool bUnlocked = IsCompanionUnlocked(CompanionID);
		const float BoxSize = GetHeroSelectionCarouselBoxSize(Offset);
		const float Opacity = GetHeroSelectionCarouselOpacity(Offset) * (bUnlocked ? 1.0f : 0.45f);

		CompanionCarousel->AddSlot()
			.AutoWidth()
			.Padding(3.0f, 0.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(BoxSize)
				.HeightOverride(BoxSize)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SImage)
						.Image(GetHeroSelectionCarouselSlotBrush(bCenterSlot))
					]
					+ SOverlay::Slot()
					.Padding(FMargin(bCenterSlot ? 5.f : 6.f))
					[
						SNew(SImage)
						.Image_Lambda([this, SlotIdx]() -> const FSlateBrush*
						{
							return CompanionCarouselPortraitBrushes.IsValidIndex(SlotIdx) && CompanionCarouselPortraitBrushes[SlotIdx].IsValid()
								? CompanionCarouselPortraitBrushes[SlotIdx].Get()
								: nullptr;
						})
						.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Opacity))
					]
				]
			];
	}

	const FHeroSelectionSharedLayoutMetrics Layout = MakeHeroSelectionSharedLayoutMetrics();
	const FVector2D LayoutViewportSize = Layout.LayoutViewportSize;
	const bool bShortViewport = Layout.bShortViewport;
	const float ReferenceLayoutWidth = Layout.ReferenceLayoutWidth;
	const float ReferenceLayoutHeight = Layout.ReferenceLayoutHeight;
	const float LeftPanelWidth = Layout.LeftPanelWidth;
	const float RightPanelWidth = Layout.RightPanelWidth;
	const float CenterPanelX = Layout.CenterPanelX;
	const float CenterPreviewWidth = Layout.CenterPreviewWidth;
	const float PartyFooterWidth = Layout.PartyFooterWidth;
	const float CompanionFooterWidth = Layout.CompanionFooterWidth;
	const float CompanionFooterX = Layout.CompanionFooterX;
	const float RunFooterX = Layout.RunFooterX;
	const float RunFooterWidth = Layout.RunFooterWidth;
	const float CompanionFooterContentWidth = Layout.CompanionFooterContentWidth;
	const float RunFooterContentWidth = Layout.RunFooterContentWidth;
	const float UpperPanelY = Layout.UpperPanelY;
	const float FooterPanelMinHeight = Layout.FooterPanelMinHeight;
	const float FooterPanelY = Layout.FooterPanelY;
	const float UpperSidePanelHeight = Layout.UpperSidePanelHeight;
	const float OuterPanelBleed = Layout.OuterPanelBleed;
	const float LayoutCompactScale = Layout.LayoutCompactScale;
	const float FooterActionHeight = Layout.FooterActionHeight;
	const float BalanceBadgeIconWidth = Layout.BalanceBadgeIconWidth;
	const float BalanceBadgeIconHeight = Layout.BalanceBadgeIconHeight;
	const float RightPreviewPanelHeight = Layout.RightPreviewPanelHeight;
	const int32 ScreenHeaderFontSize = Layout.ScreenHeaderFontSize;
	const int32 SecondaryButtonFontSize = Layout.SecondaryButtonFontSize;
	const int32 BodyTextFontSize = Layout.BodyTextFontSize;
	const int32 PrimaryCtaFontSize = Layout.PrimaryCtaFontSize;
	const int32 DifficultyMenuFontSize = Layout.DifficultyMenuFontSize;
	const int32 HeroArrowFontSize = Layout.HeroArrowFontSize;
	const int32 ACBalanceFontSize = Layout.ACBalanceFontSize;
	const float HeroArrowButtonWidth = Layout.HeroArrowButtonWidth;
	const float HeroArrowButtonHeight = Layout.HeroArrowButtonHeight;
	const float TopStripBackButtonWidth = Layout.TopStripBackButtonWidth;
	const float TopStripBackButtonHeight = Layout.TopStripBackButtonHeight;
	ResolveHeroSelectionLooseIconBrush(
		GetHeroSelectionBalanceIconPath(),
		FVector2D(BalanceBadgeIconWidth, BalanceBadgeIconHeight),
		ACBalanceIconBrush,
		ACBalanceIconTexture,
		TEXT("CompanionSelectionBalanceIcon"));

	const bool bIsLocalPartyHost = !SessionSubsystem || SessionSubsystem->IsLocalPlayerPartyHost();
	const bool bPartyLobbyContextActive = SessionSubsystem && SessionSubsystem->IsPartyLobbyContextActive();
	const int32 LobbyPlayerCount = SessionSubsystem ? SessionSubsystem->GetCurrentLobbyPlayerCount() : 0;
	const int32 ActivePartySlots = bPartyLobbyContextActive
		? FMath::Clamp(LobbyPlayerCount, 1, 4)
		: (PartySubsystem ? FMath::Clamp(PartySubsystem->GetPartyMemberCount(), 1, 4) : 1);
	const bool bHasRemotePartyMembers = bPartyLobbyContextActive
		? LobbyPlayerCount > 1
		: (PartySubsystem && PartySubsystem->HasRemotePartyMembers());
	const bool bUsePartyReadyFlow = bPartyLobbyContextActive && (!bIsLocalPartyHost || bHasRemotePartyMembers);
	const bool bCanEditDifficulty = !bUsePartyReadyFlow || bIsLocalPartyHost;
	const bool bLocalReady = SessionSubsystem && SessionSubsystem->IsLocalLobbyReady();
	const bool bCanStartPartyRun = !bUsePartyReadyFlow || !SessionSubsystem || SessionSubsystem->AreAllPartyMembersReadyForGameplay();
	const FText PrimaryActionText = bUsePartyReadyFlow && !bIsLocalPartyHost
		? (bLocalReady ? UnreadyText : ReadyText)
		: (bCanStartPartyRun ? EnterText : WaitingForPartyText);
	auto MakePreviewFocusMask = []() -> TSharedRef<SWidget>
	{
		return SNew(SBox).Visibility(EVisibility::Collapsed);
	};

	auto MakeWorldScrim = []() -> TSharedRef<SWidget>
	{
		return SNew(SBox).Visibility(EVisibility::Collapsed);
	};
	auto MakeSelectionBar = [](TSharedRef<SWidget> Content) -> TSharedRef<SWidget>
	{
		return MakeHeroSelectionContentShell(Content, FMargin(0.f));
	};

	auto MakeBalanceBadge = [this,
		BalanceBadgeIconWidth,
		BalanceBadgeIconHeight,
		ACBalanceText,
		ACBalanceFontSize,
		SecondaryButtonFontSize]() -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(BalanceBadgeIconWidth)
				.HeightOverride(BalanceBadgeIconHeight)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					[
						FT66Style::MakeRetroUIIcon(StaticCastSharedRef<SWidget>(
							SNew(SImage)
							.Image_Lambda([this]() -> const FSlateBrush*
							{
								return ACBalanceIconBrush.IsValid() && ::IsValid(ACBalanceIconBrush->GetResourceObject())
									? ACBalanceIconBrush.Get()
									: nullptr;
							})))
					]
				]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.HeroSelection", "CurrencyBadgeFallback", "CC"))
						.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Visibility_Lambda([this]() -> EVisibility
						{
							return ACBalanceIconBrush.IsValid() && ::IsValid(ACBalanceIconBrush->GetResourceObject())
								? EVisibility::Collapsed
								: EVisibility::Visible;
						})
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.f, 0.f, 0.f, 0.f)
			[
				SAssignNew(ACBalanceTextBlock, STextBlock)
				.Text(ACBalanceText)
				.Font(FT66Style::Tokens::FontBold(ACBalanceFontSize))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			];
	};

	auto MakeTopStripBackButton = [this, BackText, SecondaryButtonFontSize, TopStripBackButtonWidth, TopStripBackButtonHeight]() -> TSharedRef<SWidget>
	{
		return MakeHeroSelectionButton(
			FT66ButtonParams(
				BackText,
				FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleBackClicked),
				ET66ButtonType::Neutral)
			.SetMinWidth(TopStripBackButtonWidth)
			.SetHeight(TopStripBackButtonHeight)
			.SetFontSize(SecondaryButtonFontSize)
			.SetPadding(FMargin(12.f, 6.f, 12.f, 4.f)));
	};

	auto MakeRecordInfoButton = [SecondaryButtonFontSize](const FText& Title, const FText& Body) -> TSharedRef<SWidget>
	{
		static FButtonStyle ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
		return SNew(SBox)
			.WidthOverride(24.f)
			.HeightOverride(24.f)
			.ToolTip(MakeHeroSelectionAbilityTooltip(Title, Body, -1))
			[
				SNew(SButton)
				.ButtonStyle(&ButtonStyle)
				.ContentPadding(0.f)
				.OnClicked(FOnClicked::CreateLambda([]() -> FReply { return FReply::Handled(); }))
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(HeroSelectionChromeAccent())
					.Padding(1.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(HeroSelectionChromeInnerFillAlt())
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.CompanionSelection", "CompanionRecordInfoButton", "?"))
							.Font(FT66Style::Tokens::FontBold(FMath::Max(SecondaryButtonFontSize - 5, 12)))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
					]
				]
			];
	};

	const TSharedRef<SWidget> TopBarWidget = MakeSelectionBar(
		SNew(SBox)
		.WidthOverride(FMath::Max(1.f, CenterPreviewWidth))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				MakeHeroSelectionButton(
					FT66ButtonParams(
						NSLOCTEXT("T66.Common", "Prev", "<"),
						FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandlePrevClicked),
						ET66ButtonType::Neutral)
					.SetMinWidth(HeroArrowButtonWidth)
					.SetHeight(HeroArrowButtonHeight)
					.SetFontSize(HeroArrowFontSize))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFitX)
				.StretchDirection(EStretchDirection::DownOnly)
				[
					CompanionCarousel
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(6.0f, 0.0f, 0.0f, 0.0f)
			[
				MakeHeroSelectionButton(
					FT66ButtonParams(
						NSLOCTEXT("T66.Common", "Next", ">"),
						FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleNextClicked),
						ET66ButtonType::Neutral)
					.SetMinWidth(HeroArrowButtonWidth)
					.SetHeight(HeroArrowButtonHeight)
					.SetFontSize(HeroArrowFontSize))
			]
		]);

	const TSharedRef<SWidget> LeftPanelWidget = MakeHeroSelectionPanelShell(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				MakeTopStripBackButton()
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			[
				MakeBalanceBadge()
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 12.0f)
		[
			SNew(SBox)
			.HeightOverride(36.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(SkinsText)
				.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize + 2))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SScrollBox)
			.ScrollBarStyle(GetCompanionReferenceScrollBarStyle())
			.ScrollBarThickness(FVector2D(14.f, 14.f))
			.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
			+ SScrollBox::Slot()
			[
				SkinsListBoxWidget.ToSharedRef()
			]
		],
		FMargin(FT66Style::Tokens::Space3 + OuterPanelBleed, FT66Style::Tokens::Space3 + OuterPanelBleed, FT66Style::Tokens::Space3, FT66Style::Tokens::Space3));

	const TSharedRef<SWidget> RightPanelWidget = MakeHeroSelectionPanelShell(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(SBox)
			.HeightOverride(48.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(CompanionNameWidget, STextBlock)
				.Text(CurrentCompanionName)
				.Font(FT66Style::Tokens::FontBold(31))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 6.0f)
		[
			MakeHeroSelectionParchmentPanelShell(
				SNew(SBox)
				.HeightOverride(RightPreviewPanelHeight)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor::Black)
				],
				FMargin(5.0f))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 6.0f)
		[
			MakeHeroSelectionParchmentRowShell(
				SNew(SBox)
				.MinDesiredHeight(56.f)
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 6.f, 0.f)
					[
						MakeRecordInfoButton(
							NSLOCTEXT("T66.CompanionSelection", "CompanionRankTooltipTitle", "Rank"),
							NSLOCTEXT("T66.CompanionSelection", "CompanionRankTooltipBody", "All-time score placement for the selected difficulty, party size, and companion. N/A means no eligible score has been submitted yet."))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankLabel", "RANK"))
						.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize + 3))
						.ColorAndOpacity(GetHeroSelectionParchmentMutedText())
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					[
						SAssignNew(CompanionRecordRankWidget, STextBlock)
						.Text(NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankDefault", "..."))
						.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize + 3))
						.ColorAndOpacity(GetHeroSelectionParchmentText())
					]
				],
				FMargin(20.f, 9.f))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SAssignNew(CompanionUnionBox, SBox)
			[
				MakeHeroSelectionParchmentRowShell(
					SNew(SBox)
					.MinDesiredHeight(62.f)
					.VAlign(VAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0.f, 0.f, 6.f, 0.f)
						[
							MakeRecordInfoButton(
								NSLOCTEXT("T66.CompanionSelection", "CompanionUnityTooltipTitle", "Unity"),
								NSLOCTEXT("T66.CompanionSelection", "CompanionUnityTooltipBody", "Companion progression earned by clearing stages with this companion. Healing is now fixed by difficulty."))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0.f, 0.f, 12.f, 0.f)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.CompanionSelection", "CompanionUnityLabel", "UNITY"))
							.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize + 3))
							.ColorAndOpacity(GetHeroSelectionParchmentMutedText())
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.VAlign(VAlign_Center)
						.Padding(0.f, 0.f, 12.f, 0.f)
						[
							SNew(SBox)
							.HeightOverride(16.f)
							[
								T66ScreenSlateHelpers::MakeReferenceProgressBar(
									TAttribute<TOptional<float>>::Create(TAttribute<TOptional<float>>::FGetter::CreateLambda([this]() -> TOptional<float>
									{
										return FMath::Clamp(CompanionUnionProgress01, 0.f, 1.f);
									})),
									FVector2D(240.f, 16.f),
									FLinearColor(0.92f, 0.05f, 0.12f, 1.0f),
									FMargin(4.f, 2.f))
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SAssignNew(CompanionUnionText, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(FMath::Max(SecondaryButtonFontSize, 14)))
							.ColorAndOpacity(GetHeroSelectionParchmentText())
						]
					],
					FMargin(20.f, 9.f))
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SBox)
			.HeightOverride(36.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LoreText)
				.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize + 6))
				.ColorAndOpacity(GetHeroSelectionParchmentMutedText())
				.Justification(ETextJustify::Center)
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeHeroSelectionParchmentPanelShell(
				SNew(SScrollBox)
				.ScrollBarStyle(GetCompanionReferenceScrollBarStyle())
				.ScrollBarThickness(FVector2D(14.f, 14.f))
				.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
				+ SScrollBox::Slot()
				[
					SAssignNew(CompanionLoreWidget, STextBlock)
					.Text(CurrentCompanionLore)
					.Font(FT66Style::Tokens::FontRegular(BodyTextFontSize + 2))
					.ColorAndOpacity(GetHeroSelectionParchmentText())
					.AutoWrapText(true)
				],
				FMargin(18.f, 14.f))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeHeroSelectionParchmentRowShell(
				SAssignNew(CompanionUnionHealingText, STextBlock)
				.Text(FormatCompanionPassiveHealText(SelectedDifficulty))
				.Font(FT66Style::Tokens::FontBold(BodyTextFontSize + 2))
				.ColorAndOpacity(GetHeroSelectionParchmentText())
				.AutoWrapText(true),
				FMargin(18.f, 12.f))
		],
		FMargin(FT66Style::Tokens::Space4, FT66Style::Tokens::Space4 + OuterPanelBleed, FT66Style::Tokens::Space4, FT66Style::Tokens::Space4),
		true);

	const TSharedRef<SWidget> PreviewWidget =
		SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			CreateCompanionPreviewWidget(PreviewColor)
		];

	auto MakePartyBox = [this, ActivePartySlots, PartySubsystem, T66GI, bUsePartyReadyFlow, LayoutCompactScale, PartyFooterWidth, FooterPanelMinHeight, OuterPanelBleed]() -> TSharedRef<SWidget>
	{
		const float PartyScale = FMath::Clamp(LayoutCompactScale, 0.82f, 1.12f);
		const float PartyTileSide = FMath::RoundToFloat(84.f * PartyScale);
		const float PartyMemberGap = FMath::RoundToFloat(10.f * PartyScale);
		const FVector2D PartyProfileSize(PartyTileSide, PartyTileSide);
		const FVector2D PartyAvatarImageSize(FMath::RoundToFloat(68.f * PartyScale), FMath::RoundToFloat(68.f * PartyScale));
		const FVector2D PartyHeroSize(PartyTileSide, PartyTileSide);
		const FVector2D PartyHeroImageSize(FMath::RoundToFloat(68.f * PartyScale), FMath::RoundToFloat(68.f * PartyScale));
		const float PartyReadyHeight = FMath::RoundToFloat(22.f * PartyScale);
		const float PartyMemberWidth = PartyProfileSize.X + PartyHeroSize.X;
		const float PartyMemberHeight = PartyReadyHeight + PartyProfileSize.Y;
		TArray<FT66PartyMemberEntry> PartyMembers = PartySubsystem ? PartySubsystem->GetPartyMembers() : TArray<FT66PartyMemberEntry>();
		UT66SteamHelper* SteamHelper = T66GI ? T66GI->GetSubsystem<UT66SteamHelper>() : nullptr;
		UT66UITexturePoolSubsystem* TexPool = T66GI ? T66GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (PartyMembers.Num() == 0)
		{
			FT66PartyMemberEntry& LocalMember = PartyMembers.AddDefaulted_GetRef();
			LocalMember.DisplayName = SteamHelper ? SteamHelper->GetLocalDisplayName() : FString(TEXT("Player"));
			LocalMember.bIsLocal = true;
			LocalMember.bOnline = true;
			LocalMember.bReady = true;
			LocalMember.bIsPartyHost = true;
		}
		PartyAvatarBrushes.SetNum(4);
		PartyHeroPortraitBrushes.SetNum(4);
		PartyAvatarImageWidgets.SetNum(4);
		PartyHeroPortraitImageWidgets.SetNum(4);
		const bool bTreatPartyAsReadyByDefault = !bUsePartyReadyFlow || PartyMembers.Num() <= 1;

		auto MakeReadyBanner = [&](const bool bReady, const bool bOccupied) -> TSharedRef<SWidget>
		{
			return SNew(SBox)
				.WidthOverride(PartyProfileSize.X)
				.HeightOverride(PartyReadyHeight)
				.Visibility(bOccupied ? EVisibility::Visible : EVisibility::Hidden)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(bReady ? FLinearColor(0.55f, 0.84f, 0.60f, 1.0f) : FLinearColor(0.92f, 0.48f, 0.48f, 1.0f))
					.Padding(1.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(bReady ? FLinearColor(0.16f, 0.44f, 0.21f, 1.0f) : FLinearColor(0.48f, 0.14f, 0.14f, 1.0f))
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(bReady ? NSLOCTEXT("T66.CompanionSelection", "PartyReadySmall", "READY") : NSLOCTEXT("T66.CompanionSelection", "PartyWaitingSmall", "WAIT"))
							.Font(FT66Style::Tokens::FontBold(bReady ? 10 : 9))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
					]
				];
		};

		TSharedRef<SHorizontalBox> PartySlots = SNew(SHorizontalBox);
		for (int32 SlotIndex = 0; SlotIndex < 4; ++SlotIndex)
		{
			const FT66PartyMemberEntry* PartyMember = PartyMembers.IsValidIndex(SlotIndex) ? &PartyMembers[SlotIndex] : nullptr;
			UTexture2D* AvatarTexture = nullptr;
			if (PartyMember && SteamHelper)
			{
				AvatarTexture = SteamHelper->GetAvatarTextureForSteamId(PartyMember->PlayerId);
				if (!AvatarTexture && PartyMember->bIsLocal)
				{
					AvatarTexture = SteamHelper->GetLocalAvatarTexture();
				}
			}
			if (!PartyAvatarBrushes[SlotIndex].IsValid())
			{
				PartyAvatarBrushes[SlotIndex] = MakeShared<FSlateBrush>();
				PartyAvatarBrushes[SlotIndex]->DrawAs = ESlateBrushDrawType::Image;
			}
			PartyAvatarBrushes[SlotIndex]->ImageSize = PartyAvatarImageSize;
			PartyAvatarBrushes[SlotIndex]->SetResourceObject(AvatarTexture);
			if (!PartyHeroPortraitBrushes[SlotIndex].IsValid())
			{
				PartyHeroPortraitBrushes[SlotIndex] = MakeShared<FSlateBrush>();
				PartyHeroPortraitBrushes[SlotIndex]->DrawAs = ESlateBrushDrawType::Image;
			}
			PartyHeroPortraitBrushes[SlotIndex]->ImageSize = PartyHeroImageSize;
			PartyHeroPortraitBrushes[SlotIndex]->SetResourceObject(nullptr);

			const bool bOccupiedSlot = PartyMember != nullptr;
			const bool bPartyEnabledSlot = SlotIndex < ActivePartySlots;
			const FName SlotHeroID = bOccupiedSlot && PartyMember->bIsLocal && T66GI ? T66GI->SelectedHeroID : NAME_None;
			if (!SlotHeroID.IsNone() && T66GI && TexPool)
			{
				FHeroData SlotHeroData;
				if (T66GI->GetHeroData(SlotHeroID, SlotHeroData))
				{
					const TSoftObjectPtr<UTexture2D> PortraitSoft = T66GI->ResolveHeroPortrait(SlotHeroData, T66GI->SelectedHeroBodyType, ET66HeroPortraitVariant::Half);
					if (!PortraitSoft.IsNull())
					{
						T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, PartyHeroPortraitBrushes[SlotIndex], FName(TEXT("CompanionSelectionPartyHero"), SlotIndex + 1), true);
					}
				}
			}

			const float PlaceholderOpacity = bPartyEnabledSlot ? 0.55f : 0.28f;
			const TSharedRef<SWidget> ProfileSlot =
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image(GetHeroSelectionPartySlotBrush())
					.ColorAndOpacity(bOccupiedSlot ? FLinearColor::White : FLinearColor(0.08f, 0.09f, 0.10f, 1.0f))
				]
				+ SOverlay::Slot()
				.Padding(FMargin(9.f))
				[
					AvatarTexture
					? StaticCastSharedRef<SWidget>(SNew(SScaleBox).Stretch(EStretch::ScaleToFit)[SAssignNew(PartyAvatarImageWidgets[SlotIndex], SImage).Image(PartyAvatarBrushes[SlotIndex].Get())])
					: StaticCastSharedRef<SWidget>(
						SNew(SOverlay)
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Top)
						.Padding(0.f, 10.f, 0.f, 0.f)
						[
							SNew(SBox).WidthOverride(12.f).HeightOverride(12.f)
							[
								SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(0.20f, 0.22f, 0.24f, PlaceholderOpacity))
							]
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Bottom)
						.Padding(0.f, 0.f, 0.f, 10.f)
						[
							SNew(SBox).WidthOverride(20.f).HeightOverride(14.f)
							[
								SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(0.20f, 0.22f, 0.24f, PlaceholderOpacity))
							]
						])
				];

			const TSharedRef<SWidget> HeroSlot =
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(bOccupiedSlot ? HeroSelectionChromeAccent(0.95f) : HeroSelectionChromeAccentInactive(0.75f))
				.Padding(1.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(HeroSelectionChromeInnerFill())
					.Padding(3.f)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SScaleBox)
							.Stretch(EStretch::ScaleToFit)
							[
								SAssignNew(PartyHeroPortraitImageWidgets[SlotIndex], SImage)
								.Image(PartyHeroPortraitBrushes[SlotIndex].Get())
								.Visibility(SlotHeroID.IsNone() ? EVisibility::Collapsed : EVisibility::Visible)
							]
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Visibility(SlotHeroID.IsNone() ? EVisibility::Visible : EVisibility::Collapsed)
							.Text(bOccupiedSlot ? NSLOCTEXT("T66.CompanionSelection", "PartyHeroUnknown", "?") : NSLOCTEXT("T66.CompanionSelection", "PartyHeroEmpty", "+"))
							.Font(FT66Style::Tokens::FontBold(20))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
					]
				];

			PartySlots->AddSlot()
				.AutoWidth()
				.Padding(SlotIndex > 0 ? FMargin(PartyMemberGap, 0.f, 0.f, 0.f) : FMargin(0.f))
				[
					SNew(SBox)
					.WidthOverride(PartyMemberWidth)
					.HeightOverride(PartyMemberHeight)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()[MakeReadyBanner(bOccupiedSlot && (bTreatPartyAsReadyByDefault || PartyMember->bReady), bOccupiedSlot)]
							+ SVerticalBox::Slot().AutoHeight()
							[
								SNew(SBox).WidthOverride(PartyProfileSize.X).HeightOverride(PartyProfileSize.Y)[ProfileSlot]
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Bottom)
						[
							SNew(SBox).WidthOverride(PartyHeroSize.X).HeightOverride(PartyHeroSize.Y)[HeroSlot]
						]
					]
				];
		}

		return SNew(SBox)
			.WidthOverride(PartyFooterWidth)
			.HeightOverride(FooterPanelMinHeight)
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				MakeHeroSelectionContentShell(
					SNew(SBox)
					.WidthOverride(FMath::Max(1.f, PartyFooterWidth - 20.f))
					.HeightOverride(FMath::Max(1.f, FooterPanelMinHeight - 20.f))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Fill)
					.Clipping(EWidgetClipping::ClipToBounds)
					[
						SNew(SScaleBox)
						.Stretch(EStretch::ScaleToFitX)
						.StretchDirection(EStretchDirection::DownOnly)
						[
							PartySlots
						]
					],
					FMargin(10.f + OuterPanelBleed, 10.f, 10.f, 10.f))
			];
	};

	const TSharedRef<SWidget> ConfirmFooterPanel =
		SNew(SBox)
		.WidthOverride(CompanionFooterWidth)
		.HeightOverride(FooterPanelMinHeight)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Clipping(EWidgetClipping::ClipToBounds)
		[
			MakeHeroSelectionContentShell(
				SNew(SBox)
				.WidthOverride(FMath::Max(1.f, CompanionFooterContentWidth))
				.HeightOverride(FMath::Max(1.f, FooterPanelMinHeight - 24.f))
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					MakeHeroSelectionButton(
						FT66ButtonParams(
							ConfirmText,
							FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleConfirmClicked),
							ET66ButtonType::Primary)
						.SetMinWidth(CompanionFooterContentWidth)
						.SetHeight(126.f)
						.SetFontSize(PrimaryCtaFontSize)
						.SetPadding(FMargin(12.f, 12.f))
						.SetEnabled(TAttribute<bool>::CreateLambda([this]() -> bool
						{
							return !PreviewedCompanionID.IsNone() && IsCompanionUnlocked(PreviewedCompanionID);
						})))
				],
				FMargin(12.f))
		];

	auto MakeRunControls = [this, bUsePartyReadyFlow, bIsLocalPartyHost, bCanEditDifficulty, bCanStartPartyRun, PrimaryActionText, PrimaryCtaFontSize, DifficultyMenuFontSize, SecondaryButtonFontSize, FooterActionHeight, Loc, RunFooterContentWidth, FooterPanelMinHeight]() -> TSharedRef<SWidget>
	{
		auto WrapRunControls = [RunFooterContentWidth, FooterPanelMinHeight](const TSharedRef<SWidget>& Content) -> TSharedRef<SWidget>
		{
			return SNew(SBox)
				.HeightOverride(FooterPanelMinHeight)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					MakeHeroSelectionContentShell(
						SNew(SBox)
						.WidthOverride(FMath::Max(1.f, RunFooterContentWidth))
						.HeightOverride(FMath::Max(1.f, FooterPanelMinHeight - 24.f))
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Center)
						.Clipping(EWidgetClipping::ClipToBounds)
						[
							SNew(SScaleBox)
							.Stretch(EStretch::ScaleToFitX)
							.StretchDirection(EStretchDirection::DownOnly)
							[
								Content
							]
						],
						FMargin(12.f))
				];
		};
		auto MakeCommunityContentButtons = [this, FooterActionHeight, SecondaryButtonFontSize]() -> TSharedRef<SWidget>
		{
			const float TextButtonWidth = 108.f;
			const float ButtonGap = 8.f;
			const int32 TextFontSize = FMath::Max(SecondaryButtonFontSize - 5, 12);
			return SNew(SBox)
				.WidthOverride((TextButtonWidth * 2.f) + ButtonGap)
				.HeightOverride(FooterActionHeight)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						MakeHeroSelectionButton(
							FT66ButtonParams(
								NSLOCTEXT("T66.CompanionSelection", "ChallengesButtonText", "CHALLENGES"),
								FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleChallengesClicked),
								ET66ButtonType::Neutral)
							.SetMinWidth(TextButtonWidth)
							.SetHeight(FooterActionHeight)
							.SetPadding(FMargin(4.f, 8.f))
							.SetFontSize(TextFontSize))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.Padding(ButtonGap, 0.f, 0.f, 0.f)
					[
						MakeHeroSelectionButton(
							FT66ButtonParams(
								NSLOCTEXT("T66.CompanionSelection", "ModsButtonText", "MODS"),
								FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleModsClicked),
								ET66ButtonType::Neutral)
							.SetMinWidth(TextButtonWidth)
							.SetHeight(FooterActionHeight)
							.SetPadding(FMargin(4.f, 8.f))
							.SetFontSize(TextFontSize))
					]
				];
		};

		if (bUsePartyReadyFlow && !bIsLocalPartyHost)
		{
			return WrapRunControls(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					MakeHeroSelectionButton(
						FT66ButtonParams(
							PrimaryActionText,
							FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleEnterClicked),
							ET66ButtonType::Primary)
						.SetMinWidth(0.f)
						.SetHeight(FooterActionHeight)
						.SetPadding(FMargin(12.f, 8.f))
						.SetFontSize(PrimaryCtaFontSize))
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)[MakeCommunityContentButtons()]);
		}

		return WrapRunControls(
			SNew(SHorizontalBox)
			.Clipping(EWidgetClipping::ClipToBounds)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Fill)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(230.f)
				.HeightOverride(FooterActionHeight)
				.IsEnabled(bCanEditDifficulty)
				[
					MakeHeroSelectionDropdown(
						FT66DropdownParams(
							SAssignNew(DifficultyDropdownText, STextBlock)
							.Text(CurrentDifficultyOption.IsValid()
								? FText::FromString(*CurrentDifficultyOption)
								: (Loc ? Loc->GetText_Easy() : NSLOCTEXT("T66.Difficulty", "Easy", "Easy")))
							.Font(FT66Style::Tokens::FontBold(DifficultyMenuFontSize))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds),
							[this, FooterActionHeight, DifficultyMenuFontSize]()
							{
								TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
								for (const TSharedPtr<FString>& Opt : DifficultyOptions)
								{
									if (!Opt.IsValid())
									{
										continue;
									}
									TSharedPtr<FString> Captured = Opt;
									Box->AddSlot().AutoHeight()
									[
										FT66Style::MakeDropdownOptionButton(
											FText::FromString(*Opt),
											FOnClicked::CreateLambda([this, Captured]()
											{
												OnDifficultyChanged(Captured, ESelectInfo::Direct);
												FSlateApplication::Get().DismissAllMenus();
												return FReply::Handled();
											}),
											CurrentDifficultyOption.IsValid() && *CurrentDifficultyOption == *Opt,
											0.f,
											FooterActionHeight,
											DifficultyMenuFontSize,
											FMargin(10.f, 8.f, 10.f, 6.f))
									];
								}
								return Box;
							})
						.SetMinWidth(230.f)
						.SetHeight(FooterActionHeight)
						.SetPadding(FMargin(10.f, 8.f)))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.WidthOverride(250.f)
				.HeightOverride(FooterActionHeight)
				.IsEnabled(bCanStartPartyRun)
				[
					MakeHeroSelectionSpriteButton(
						FT66ButtonParams(
							PrimaryActionText,
							FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleEnterClicked),
							bCanStartPartyRun ? ET66ButtonType::Primary : ET66ButtonType::Neutral)
						.SetMinWidth(250.f)
						.SetHeight(FooterActionHeight)
						.SetPadding(FMargin(12.f, 8.f))
						.SetFontSize(PrimaryCtaFontSize),
						TAttribute<ET66HeroSpriteFamily>(bCanStartPartyRun ? ET66HeroSpriteFamily::ToggleOn : ET66HeroSpriteFamily::CompactNeutral))
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)[MakeCommunityContentButtons()]);
	};

	const TSharedRef<SWidget> LeftFooterPanel =
		SNew(SBox)
		.WidthOverride(PartyFooterWidth)
		.HeightOverride(FooterPanelMinHeight)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Clipping(EWidgetClipping::ClipToBounds)
		[
			MakePartyBox()
		];

	const TSharedRef<SWidget> RightFooterPanel =
		SNew(SBox)
		.WidthOverride(RunFooterWidth)
		.HeightOverride(FooterPanelMinHeight)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Clipping(EWidgetClipping::ClipToBounds)
		[
			MakeRunControls()
		];

	const TSharedRef<SWidget> CenterColumnWidget =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			TopBarWidget
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			PreviewWidget
		];

	const TSharedRef<SWidget> ReferenceCanvas = SNew(SBox)
		.WidthOverride(ReferenceLayoutWidth)
		.HeightOverride(ReferenceLayoutHeight)
		[
			SNew(SConstraintCanvas)
			+ SConstraintCanvas::Slot()
			.Offset(FMargin(-OuterPanelBleed, UpperPanelY - OuterPanelBleed, LeftPanelWidth + OuterPanelBleed, UpperSidePanelHeight + OuterPanelBleed))
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D::ZeroVector)
			[
				LeftPanelWidget
			]
			+ SConstraintCanvas::Slot()
			.Offset(FMargin(CenterPanelX, UpperPanelY, CenterPreviewWidth, UpperSidePanelHeight))
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D::ZeroVector)
			[
				CenterColumnWidget
			]
			+ SConstraintCanvas::Slot()
			.Offset(FMargin(ReferenceLayoutWidth - RightPanelWidth, UpperPanelY - OuterPanelBleed, RightPanelWidth + OuterPanelBleed, UpperSidePanelHeight + OuterPanelBleed))
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D::ZeroVector)
			[
				RightPanelWidget
			]
			+ SConstraintCanvas::Slot()
			.Offset(FMargin(-OuterPanelBleed, FooterPanelY, PartyFooterWidth + OuterPanelBleed, FooterPanelMinHeight + OuterPanelBleed))
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D::ZeroVector)
			[
				LeftFooterPanel
			]
			+ SConstraintCanvas::Slot()
			.Offset(FMargin(CompanionFooterX, FooterPanelY, CompanionFooterWidth, FooterPanelMinHeight + OuterPanelBleed))
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D::ZeroVector)
			[
				ConfirmFooterPanel
			]
			+ SConstraintCanvas::Slot()
			.Offset(FMargin(RunFooterX, FooterPanelY, RunFooterWidth + OuterPanelBleed, FooterPanelMinHeight + OuterPanelBleed))
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D::ZeroVector)
			[
				RightFooterPanel
			]
		];

	const TSharedRef<SWidget> Root = SNew(SBox)
		.WidthOverride(LayoutViewportSize.X)
		.HeightOverride(LayoutViewportSize.Y)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()[MakePreviewFocusMask()]
			+ SOverlay::Slot()[MakeWorldScrim()]
			+ SOverlay::Slot()
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::Both)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					ReferenceCanvas
				]
			]
		];
	if (const FSlateBrush* SceneBackgroundBrush = GetCompanionSceneBackgroundBrush())
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SImage)
				.Image(SceneBackgroundBrush)
				.ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.02f, 0.025f, 0.035f, 0.08f))
			]
			+ SOverlay::Slot()
			[
				Root
			];
	}
	return Root;
}
FReply UT66CompanionSelectionScreen::HandlePrevClicked() { PreviewPreviousCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleNextClicked() { PreviewNextCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleCompanionGridClicked() { OnCompanionGridClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleNoCompanionClicked() { SelectNoCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleLoreClicked() { bShowingLore = !bShowingLore; UpdateCompanionDisplay(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleConfirmClicked() { OnConfirmCompanionClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleEnterClicked() { OnEnterTribulationClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleChallengesClicked() { OnChallengesClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleModsClicked() { OnModsClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleBackClicked() { OnBackClicked(); return FReply::Handled(); }

void UT66CompanionSelectionScreen::OnDifficultyChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type /*SelectInfo*/)
{
	if (!NewValue.IsValid())
	{
		return;
	}

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	const TArray<ET66Difficulty> Difficulties = GI ? GI->GetPlayableDifficulties() : TArray<ET66Difficulty>{
		ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard, ET66Difficulty::VeryHard, ET66Difficulty::Impossible
	};
	const int32 Index = DifficultyOptions.IndexOfByKey(NewValue);
	if (!Difficulties.IsValidIndex(Index))
	{
		return;
	}

	SelectedDifficulty = GI ? GI->ResolvePlayableDifficulty(Difficulties[Index]) : Difficulties[Index];
	CurrentDifficultyOption = NewValue;
	if (GI)
	{
		GI->SelectedDifficulty = SelectedDifficulty;
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->SetLocalLobbyReady(false);
		}
	}
	RefreshDifficultyDropdownText();
	UpdateCompanionDisplay();
	RefreshCompanionRecordRank();
	if (AT66CompanionPreviewStage* Stage = GetCompanionPreviewStage())
	{
		Stage->SetPreviewDifficulty(SelectedDifficulty);
	}
	if (AT66HeroPreviewStage* HeroStage = GetHeroPreviewStage())
	{
		HeroStage->SetPreviewDifficulty(SelectedDifficulty);
	}
}

void UT66CompanionSelectionScreen::RefreshDifficultyDropdownText()
{
	if (DifficultyOptions.Num() > 0)
	{
		UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
		const TArray<ET66Difficulty> Difficulties = GI ? GI->GetPlayableDifficulties() : TArray<ET66Difficulty>{
			ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard, ET66Difficulty::VeryHard, ET66Difficulty::Impossible
		};
		const int32 CurrentDiffIndex = Difficulties.IndexOfByKey(SelectedDifficulty);
		if (DifficultyOptions.IsValidIndex(CurrentDiffIndex))
		{
			CurrentDifficultyOption = DifficultyOptions[CurrentDiffIndex];
		}
	}

	if (DifficultyDropdownText.IsValid())
	{
		UT66LocalizationSubsystem* Loc = GetLocSubsystem();
		DifficultyDropdownText->SetText(
			CurrentDifficultyOption.IsValid()
				? FText::FromString(*CurrentDifficultyOption)
				: (Loc ? Loc->GetText_Easy() : NSLOCTEXT("T66.Difficulty", "Easy", "Easy")));
	}
}

void UT66CompanionSelectionScreen::RefreshCompanionRecordRank()
{
	if (!CompanionRecordRankWidget.IsValid())
	{
		return;
	}

	CompanionRecordRankRequestKey.Reset();
	if (PreviewedCompanionID.IsNone())
	{
		CompanionRecordRankWidget->SetText(NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankUnavailable", "--"));
		return;
	}

	UGameInstance* GIBase = UGameplayStatics::GetGameInstance(this);
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GIBase);
	UT66BackendSubsystem* Backend = GIBase ? GIBase->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (!Backend)
	{
		CompanionRecordRankWidget->SetText(NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankUnavailableNoBackend", "--"));
		return;
	}

	int32 PartySize = 1;
	if (T66GI)
	{
		if (UT66SessionSubsystem* SessionSubsystem = T66GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (SessionSubsystem->IsPartyLobbyContextActive())
			{
				PartySize = FMath::Max(1, SessionSubsystem->GetCurrentLobbyPlayerCount());
			}
		}
		if (PartySize <= 1)
		{
			if (UT66PartySubsystem* PartySubsystem = T66GI->GetSubsystem<UT66PartySubsystem>())
			{
				PartySize = FMath::Max(1, PartySubsystem->GetPartyMemberCount());
			}
		}
	}

	const FString CompanionID = PreviewedCompanionID.ToString();
	const FString DifficultyKey = HeroSelectionDifficultyToApiString(SelectedDifficulty);
	const FString PartyKey = HeroSelectionPartySizeToApiString(PartySize);
	const FString RankKey = UT66BackendSubsystem::MakeMyRankCacheKey(
		TEXT("score"),
		TEXT("alltime"),
		PartyKey,
		DifficultyKey,
		TEXT("companion"),
		CompanionID);
	CompanionRecordRankRequestKey = RankKey;

	bool bRankSuccess = false;
	int32 Rank = 0;
	int32 TotalEntries = 0;
	if (Backend->GetCachedMyRank(RankKey, bRankSuccess, Rank, TotalEntries))
	{
		static_cast<void>(TotalEntries);
		CompanionRecordRankWidget->SetText(bRankSuccess && Rank > 0 ? FormatCompanionRecordRankText(Rank) : FormatCompanionRecordRankText(0));
		return;
	}

	if (!Backend->IsBackendConfigured() || !Backend->HasSteamTicket())
	{
		CompanionRecordRankWidget->SetText(NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankOffline", "--"));
		return;
	}

	CompanionRecordRankWidget->SetText(NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankPending", "..."));
	Backend->FetchMyRankFiltered(
		TEXT("score"),
		TEXT("alltime"),
		PartyKey,
		DifficultyKey,
		TEXT("companion"),
		CompanionID);
}

void UT66CompanionSelectionScreen::HandleBackendMyRankDataReady(const FString& Key, bool bSuccess, int32 Rank, int32 TotalEntries)
{
	static_cast<void>(bSuccess);
	static_cast<void>(Rank);
	static_cast<void>(TotalEntries);

	if (!CompanionRecordRankRequestKey.Equals(Key) || !HasBuiltSlateUI() || !IsVisible())
	{
		return;
	}

	RefreshCompanionRecordRank();
}

void UT66CompanionSelectionScreen::HandlePartyStateChanged()
{
	FT66Style::DeferRebuild(this);
}

void UT66CompanionSelectionScreen::HandleSessionStateChanged()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SelectedDifficulty = GI->ResolvePlayableDifficulty(SessionSubsystem->GetSharedLobbyDifficulty());
			GI->SelectedDifficulty = SelectedDifficulty;
		}
	}
	RefreshDifficultyDropdownText();
	RefreshCompanionRecordRank();
	FT66Style::DeferRebuild(this);
}

AT66CompanionPreviewStage* UT66CompanionSelectionScreen::GetCompanionPreviewStage() const
{
	if (AT66CompanionPreviewStage* CachedStage = CachedCompanionPreviewStage.Get())
	{
		return CachedStage;
	}

	UWorld* World = GetWorld();
	if (!World) return nullptr;
	if (AT66PlayerController* PC = T66GetLocalFrontendCompanionPlayerController(const_cast<UT66CompanionSelectionScreen*>(this)))
	{
		PC->EnsureLocalFrontendPreviewScene();
	}

	if (AT66CompanionPreviewStage* CachedStage = CachedCompanionPreviewStage.Get())
	{
		return CachedStage;
	}

	// UI setup fallback: the frontend preview scene is resolved once and then
	// cached so carousel refreshes do not rescan the world.
	for (TActorIterator<AT66CompanionPreviewStage> It(World); It; ++It)
	{
		CachedCompanionPreviewStage = *It;
		return CachedCompanionPreviewStage.Get();
	}
	return nullptr;
}

AT66HeroPreviewStage* UT66CompanionSelectionScreen::GetHeroPreviewStage() const
{
	if (AT66HeroPreviewStage* CachedStage = CachedHeroPreviewStage.Get())
	{
		return CachedStage;
	}

	UWorld* World = GetWorld();
	if (!World) return nullptr;
	if (AT66PlayerController* PC = T66GetLocalFrontendCompanionPlayerController(const_cast<UT66CompanionSelectionScreen*>(this)))
	{
		PC->EnsureLocalFrontendPreviewScene();
	}

	if (AT66HeroPreviewStage* CachedStage = CachedHeroPreviewStage.Get())
	{
		return CachedStage;
	}

	// UI setup fallback: cached after first resolve for this screen.
	for (TActorIterator<AT66HeroPreviewStage> It(World); It; ++It)
	{
		CachedHeroPreviewStage = *It;
		return CachedHeroPreviewStage.Get();
	}
	return nullptr;
}

TSharedRef<SWidget> UT66CompanionSelectionScreen::CreateCompanionPreviewWidget(const FLinearColor& FallbackColor)
{
	AT66CompanionPreviewStage* Stage = GetCompanionPreviewStage();

	if (Stage)
	{
		const TWeakObjectPtr<AT66CompanionPreviewStage> WeakStage(Stage);
		// In-world preview: transparent overlay for drag-rotate/zoom.
		// The main viewport renders the companion with full Lumen GI behind the UI.
		return SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(ST66DragRotateStagePreview)
				.DegreesPerPixel(0.28f)
				.OnRotateYaw(FT66DragPreviewDeltaDelegate::CreateLambda([WeakStage](const float DeltaYaw)
				{
					if (AT66CompanionPreviewStage* PreviewStage = WeakStage.Get())
					{
						PreviewStage->AddPreviewYaw(DeltaYaw);
					}
				}))
				.OnZoom(FT66DragPreviewDeltaDelegate::CreateLambda([WeakStage](const float ZoomDelta)
				{
					if (AT66CompanionPreviewStage* PreviewStage = WeakStage.Get())
					{
						PreviewStage->AddPreviewZoom(ZoomDelta);
						T66PositionCompanionPreviewCamera(PreviewStage);
					}
				}))
			];
	}
	return SAssignNew(CompanionPreviewColorBox, SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66Style::IsDotaTheme() ? FLinearColor::Transparent : FallbackColor)
		[
			SNew(SBox)
		];
}

void UT66CompanionSelectionScreen::UpdateCompanionDisplay()
{
	FName EffectiveSkin = PreviewedCompanionSkinIDOverride;
	if (EffectiveSkin.IsNone() && !PreviewedCompanionID.IsNone())
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
			{
				EffectiveSkin = SkinSub->GetEquippedCompanionSkinID(PreviewedCompanionID);
			}
		}
	}
	if (EffectiveSkin.IsNone()) EffectiveSkin = FName(TEXT("Default"));

	if (AT66CompanionPreviewStage* Stage = GetCompanionPreviewStage())
	{
		Stage->SetPreviewStageMode(ET66PreviewStageMode::Selection);
		if (const UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			Stage->SetPreviewDifficulty(GI->SelectedDifficulty);
		}
		Stage->SetPreviewCompanion(PreviewedCompanionID, EffectiveSkin);
		T66PositionCompanionPreviewCamera(this);
	}
	else if (CompanionPreviewColorBox.IsValid())
	{
		if (FT66Style::IsDotaTheme())
		{
			CompanionPreviewColorBox->SetBorderBackgroundColor(FLinearColor::Transparent);
		}
		else
		{
			FCompanionData Data;
			if (GetPreviewedCompanionData(Data))
			{
				CompanionPreviewColorBox->SetBorderBackgroundColor(Data.PlaceholderColor);
			}
			else
			{
				CompanionPreviewColorBox->SetBorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.4f, 1.0f));
			}

			if (!PreviewedCompanionID.IsNone() && !IsCompanionUnlocked(PreviewedCompanionID))
			{
				CompanionPreviewColorBox->SetBorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.02f, 1.0f));
			}
		}
	}

	if (CompanionNameWidget.IsValid())
	{
		if (PreviewedCompanionID.IsNone())
		{
			UT66LocalizationSubsystem* Loc = GetLocSubsystem();
			CompanionNameWidget->SetText(Loc ? Loc->GetText_NoCompanion() : NSLOCTEXT("T66.CompanionSelection", "NoCompanionTitle", "No Companion"));
		}
		else
		{
			FCompanionData Data;
			if (GetPreviewedCompanionData(Data))
			{
				UT66LocalizationSubsystem* Loc = GetLocSubsystem();
				CompanionNameWidget->SetText(Loc ? Loc->GetCompanionDisplayName(Data) : Data.DisplayName);
			}
		}
	}

	if (CompanionLoreWidget.IsValid())
	{
		if (PreviewedCompanionID.IsNone())
		{
			CompanionLoreWidget->SetText(NSLOCTEXT("T66.CompanionSelection", "NoCompanionLore", "Selecting no companion means you face the tribulation alone."));
		}
		else
		{
			FCompanionData Data;
			if (GetPreviewedCompanionData(Data))
			{
				CompanionLoreWidget->SetText(ResolveCompanionLoreText(GetLocSubsystem(), Data));
			}
		}
	}

	// Unity is profile progression only; healing strength is fixed by difficulty.
	if (CompanionUnionBox.IsValid())
	{
		const bool bShowUnion = !PreviewedCompanionID.IsNone() && IsCompanionUnlocked(PreviewedCompanionID);
		CompanionUnionBox->SetVisibility(bShowUnion ? EVisibility::Visible : EVisibility::Collapsed);
	}

	CompanionUnionProgress01 = 0.f;
	CompanionUnionStagesCleared = 0;
	if (!PreviewedCompanionID.IsNone())
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
			{
				CompanionUnionStagesCleared = FMath::Max(0, Ach->GetCompanionUnionStagesCleared(PreviewedCompanionID));
				CompanionUnionProgress01 = FMath::Clamp(Ach->GetCompanionUnionProgress01(PreviewedCompanionID), 0.f, 1.f);

				const int32 Needed = UT66AchievementsSubsystem::UnionTier_HyperStages;
				if (CompanionUnionText.IsValid())
				{
					CompanionUnionText->SetText(FText::Format(
						NSLOCTEXT("T66.CompanionSelection", "UnityStagesFormat", "{0} / {1}"),
						FText::AsNumber(CompanionUnionStagesCleared),
						FText::AsNumber(Needed)));
				}
			}
		}
	}
	if (CompanionUnionHealingText.IsValid())
	{
		CompanionUnionHealingText->SetText(FormatCompanionPassiveHealText(SelectedDifficulty));
	}
	RefreshCompanionRecordRank();

	RefreshCompanionCarouselPortraits();
}

void UT66CompanionSelectionScreen::RefreshCompanionCarouselPortraits()
{
	const int32 NumCarousel = AllCompanionIDs.Num();
	const int32 CarouselIndex = NumCarousel > 0 ? FMath::Clamp(CurrentCompanionIndex, 0, NumCarousel - 1) : 0;

	CompanionCarouselPortraitBrushes.SetNum(HeroSelectionCarouselVisibleSlots);
	for (int32 i = 0; i < CompanionCarouselPortraitBrushes.Num(); ++i)
	{
		if (!CompanionCarouselPortraitBrushes[i].IsValid())
		{
			CompanionCarouselPortraitBrushes[i] = MakeShared<FSlateBrush>();
			CompanionCarouselPortraitBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
			CompanionCarouselPortraitBrushes[i]->ImageSize = FVector2D(128.f, 128.f);
		}
	}

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		UT66UITexturePoolSubsystem* TexPool = GI->GetSubsystem<UT66UITexturePoolSubsystem>();
		for (int32 Offset = -HeroSelectionCarouselCenterIndex; Offset <= HeroSelectionCarouselCenterIndex; ++Offset)
		{
			const int32 SlotIdx = Offset + HeroSelectionCarouselCenterIndex;
			if (!CompanionCarouselPortraitBrushes.IsValidIndex(SlotIdx) || !CompanionCarouselPortraitBrushes[SlotIdx].IsValid())
			{
				continue;
			}
			const int32 Idx = NumCarousel > 0 ? (CarouselIndex + Offset + NumCarousel * 2) % NumCarousel : 0;
			const FName CompanionID = NumCarousel > 0 && AllCompanionIDs.IsValidIndex(Idx) ? AllCompanionIDs[Idx] : NAME_None;
			TSoftObjectPtr<UTexture2D> PortraitSoft;
			if (!CompanionID.IsNone())
			{
				FCompanionData D;
				if (GI->GetCompanionData(CompanionID, D))
				{
					PortraitSoft = !D.SelectionPortrait.IsNull() ? D.SelectionPortrait : D.Portrait;
				}
			}
			const float BoxSize = GetHeroSelectionCarouselBoxSize(Offset);
			if (PortraitSoft.IsNull() || !TexPool)
			{
				CompanionCarouselPortraitBrushes[SlotIdx]->SetResourceObject(nullptr);
			}
			else
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, CompanionCarouselPortraitBrushes[SlotIdx], FName(TEXT("CompanionCarousel"), SlotIdx + 1), /*bClearWhileLoading*/ false);
			}
			CompanionCarouselPortraitBrushes[SlotIdx]->ImageSize = FVector2D(BoxSize, BoxSize);
		}
	}
}

void UT66CompanionSelectionScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66CompanionSelectionScreen::OnLanguageChanged);
	}
	RefreshCompanionList();
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->PrimeHeroSelectionAssetsAsync();
		GI->PrimeHeroSelectionPreviewVisualsAsync();
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartyStateChangedHandle = PartySubsystem->OnPartyStateChanged().AddUObject(this, &UT66CompanionSelectionScreen::HandlePartyStateChanged);
		}
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionStateChangedHandle = SessionSubsystem->OnSessionStateChanged().AddUObject(this, &UT66CompanionSelectionScreen::HandleSessionStateChanged);
			SessionSubsystem->SetLocalFrontendScreen(ET66ScreenType::CompanionSelection);
			SelectedDifficulty = GI->ResolvePlayableDifficulty(SessionSubsystem->GetSharedLobbyDifficulty());
		}
		else
		{
			SelectedDifficulty = GI->ResolvePlayableDifficulty(GI->SelectedDifficulty);
		}
		GI->SelectedDifficulty = SelectedDifficulty;
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			if (BackendMyRankReadyHandle.IsValid())
			{
				Backend->OnMyRankDataReady.Remove(BackendMyRankReadyHandle);
				BackendMyRankReadyHandle.Reset();
			}
			BackendMyRankReadyHandle = Backend->OnMyRankDataReady.AddUObject(this, &UT66CompanionSelectionScreen::HandleBackendMyRankDataReady);
		}
		if (!GI->SelectedCompanionID.IsNone() && AllCompanionIDs.Contains(GI->SelectedCompanionID))
		{
			PreviewCompanion(GI->SelectedCompanionID);
		}
		else if (AllCompanionIDs.Num() > 0)
		{
			PreviewCompanion(AllCompanionIDs[0]);
		}
	}
	T66PositionCompanionPreviewCamera(this);
}

void UT66CompanionSelectionScreen::OnScreenDeactivated_Implementation()
{
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.RemoveDynamic(this, &UT66CompanionSelectionScreen::OnLanguageChanged);
	}

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->OnPartyStateChanged().Remove(PartyStateChangedHandle);
			PartyStateChangedHandle.Reset();
		}
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->OnSessionStateChanged().Remove(SessionStateChangedHandle);
			SessionStateChangedHandle.Reset();
		}
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnMyRankDataReady.Remove(BackendMyRankReadyHandle);
			BackendMyRankReadyHandle.Reset();
		}
	}

	Super::OnScreenDeactivated_Implementation();
}

void UT66CompanionSelectionScreen::RefreshScreen_Implementation()
{
	FCompanionData Data;
	bool bIsNoCompanion = PreviewedCompanionID.IsNone();
	if (!bIsNoCompanion) GetPreviewedCompanionData(Data);
	OnPreviewedCompanionChanged(Data, bIsNoCompanion);
	UpdateCompanionDisplay();
}

void UT66CompanionSelectionScreen::RefreshCompanionList()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		AllCompanionIDs = GI->GetAllCompanionIDs();
}

TArray<FCompanionData> UT66CompanionSelectionScreen::GetAllCompanions()
{
	TArray<FCompanionData> Companions;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		for (const FName& ID : AllCompanionIDs)
		{
			FCompanionData Data;
			if (GI->GetCompanionData(ID, Data)) Companions.Add(Data);
		}
	}
	return Companions;
}

bool UT66CompanionSelectionScreen::GetPreviewedCompanionData(FCompanionData& OutData)
{
	if (PreviewedCompanionID.IsNone()) return false;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		return GI->GetCompanionData(PreviewedCompanionID, OutData);
	return false;
}

void UT66CompanionSelectionScreen::PreviewCompanion(FName ID)
{
	PreviewedCompanionID = ID;
	PreviewedCompanionSkinIDOverride = NAME_None;
	CurrentCompanionIndex = ID.IsNone() ? -1 : AllCompanionIDs.IndexOfByKey(ID);
	if (CurrentCompanionIndex == INDEX_NONE) CurrentCompanionIndex = -1;
	FCompanionData Data;
	bool bIsNoCompanion = ID.IsNone();
	if (!bIsNoCompanion) GetPreviewedCompanionData(Data);
	OnPreviewedCompanionChanged(Data, bIsNoCompanion);
	RefreshSkinsList();
}

void UT66CompanionSelectionScreen::SelectNoCompanion() { PreviewCompanion(NAME_None); }

void UT66CompanionSelectionScreen::PreviewNextCompanion()
{
	if (AllCompanionIDs.Num() == 0) return;
	CurrentCompanionIndex = (FMath::Max(CurrentCompanionIndex, 0) + 1) % AllCompanionIDs.Num();
	PreviewCompanion(AllCompanionIDs[CurrentCompanionIndex]);
}

void UT66CompanionSelectionScreen::PreviewPreviousCompanion()
{
	if (AllCompanionIDs.Num() == 0) return;
	CurrentCompanionIndex = (FMath::Max(CurrentCompanionIndex, 0) - 1 + AllCompanionIDs.Num()) % AllCompanionIDs.Num();
	PreviewCompanion(AllCompanionIDs[CurrentCompanionIndex]);
}

void UT66CompanionSelectionScreen::OnCompanionGridClicked() { ShowModal(ET66ScreenType::CompanionGrid); }
void UT66CompanionSelectionScreen::OnCompanionLoreClicked() { if (!PreviewedCompanionID.IsNone()) ShowModal(ET66ScreenType::CompanionLore); }
void UT66CompanionSelectionScreen::OnConfirmCompanionClicked()
{
	// Locked companions cannot be confirmed/selected.
	if (!PreviewedCompanionID.IsNone() && !IsCompanionUnlocked(PreviewedCompanionID))
	{
		return;
	}
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedCompanionID = PreviewedCompanionID;
		GI->PersistRememberedSelectionDefaults();
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->SetLocalLobbyReady(false);
		}

		if (AT66HeroPreviewStage* HeroStage = GetHeroPreviewStage())
		{
			FName EffectiveHeroSkinID = GI->SelectedHeroSkinID;
			if (EffectiveHeroSkinID.IsNone())
			{
				EffectiveHeroSkinID = FName(TEXT("Default"));
			}

			HeroStage->SetPreviewStageMode(ET66PreviewStageMode::Selection);
			HeroStage->SetPreviewDifficulty(GI->SelectedDifficulty);
			HeroStage->SetPreviewHero(GI->SelectedHeroID, GI->SelectedHeroBodyType, EffectiveHeroSkinID, GI->SelectedCompanionID);
		}
	}
	NavigateBack();
}
void UT66CompanionSelectionScreen::OpenCommunityContent(const bool bOpenMods)
{
	const ET66CommunityContentKind ContentKind = bOpenMods
		? ET66CommunityContentKind::Mod
		: ET66CommunityContentKind::Challenge;

	ShowModal(ET66ScreenType::Challenges);

	UT66ChallengesScreen* ChallengesScreen = UIManager
		? Cast<UT66ChallengesScreen>(UIManager->GetCurrentScreen())
		: nullptr;
	if (!ChallengesScreen && UIManager)
	{
		ChallengesScreen = Cast<UT66ChallengesScreen>(UIManager->GetCurrentModal());
	}

	if (ChallengesScreen)
	{
		ChallengesScreen->OpenContentKind(ContentKind);
	}
}

void UT66CompanionSelectionScreen::OnChallengesClicked() { OpenCommunityContent(false); }

void UT66CompanionSelectionScreen::OnModsClicked() { OpenCommunityContent(true); }

void UT66CompanionSelectionScreen::OnEnterTribulationClicked()
{
	if (!PreviewedCompanionID.IsNone() && !IsCompanionUnlocked(PreviewedCompanionID))
	{
		return;
	}

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (GI)
	{
		SelectedDifficulty = GI->ResolvePlayableDifficulty(SelectedDifficulty);
		GI->SelectedCompanionID = PreviewedCompanionID;
		GI->SelectedDifficulty = SelectedDifficulty;
		GI->PersistRememberedSelectionDefaults();
		GI->ApplyConfiguredMainMapLayoutVariant();
		GI->bStageCatchUpPending = false;
		GI->PendingLoadedTransform = FTransform();
		GI->bApplyLoadedTransform = false;
		GI->RunSeed = FMath::Rand();
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->ApplyCurrentPartyToGameInstanceRunContext();
		}
	}

	if (SessionSubsystem && SessionSubsystem->IsPartyLobbyContextActive() && GI)
	{
		SelectedDifficulty = GI->ResolvePlayableDifficulty(SessionSubsystem->GetSharedLobbyDifficulty());
		GI->SelectedDifficulty = SelectedDifficulty;
		SessionSubsystem->SyncLocalLobbyProfile();

		if (!SessionSubsystem->IsLocalPlayerPartyHost())
		{
			SessionSubsystem->SetLocalLobbyReady(!SessionSubsystem->IsLocalLobbyReady());
			ForceRebuildSlate();
			return;
		}

		FString FailureReason;
		if (!SessionSubsystem->AreAllPartyMembersReadyForGameplay(&FailureReason))
		{
			UE_LOG(LogTemp, Log, TEXT("%s"), *FailureReason);
			ForceRebuildSlate();
			return;
		}

		if (UIManager) UIManager->HideAllUI();
		SessionSubsystem->StartGameplayTravel();
		return;
	}

	if (UIManager) UIManager->HideAllUI();
	if (GI)
	{
		GI->TransitionToGameplayLevel();
	}
	else
	{
		UGameplayStatics::OpenLevel(this, UT66GameInstance::GetTribulationEntryLevelName());
	}
}
void UT66CompanionSelectionScreen::OnBackClicked() { NavigateBack(); }

void UT66CompanionSelectionScreen::OnLanguageChanged(ET66Language NewLanguage)
{
	FT66Style::DeferRebuild(this);
}

