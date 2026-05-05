// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66LanguageSelectScreen.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

namespace
{
	FLinearColor T66LanguageShellFill()
	{
		return FT66Style::Background();
	}

	FLinearColor T66LanguagePanelFill()
	{
		return FT66Style::PanelOuter();
	}

	FLinearColor T66LanguageRowFill()
	{
		return FT66Style::PanelInner();
	}

	FLinearColor T66LanguageNeutralButtonFill()
	{
		return FT66Style::ButtonNeutral();
	}

	struct FLanguageReferenceButtonBrushSet
	{
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Normal;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Hovered;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Pressed;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Disabled;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Selected;
	};

	struct FLanguageReferenceButtonStyleEntry
	{
		FButtonStyle Style;
		bool bInitialized = false;
	};

	const FSlateBrush* ResolveLanguageReferenceBrush(
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

	const FSlateBrush* ResolveLanguageReferenceRegionBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const FString& RelativePath,
		const FMargin& Margin,
		const FBox2f& UVRegion,
		const FVector2D& ImageSize,
		const ESlateBrushDrawType::Type DrawAs,
		const FLinearColor& Tint,
		const TCHAR* DebugLabel,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		UTexture2D* Texture = T66RuntimeUIBrushAccess::LoadOptionalTexture(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(RelativePath),
			Margin,
			DebugLabel,
			Filter);
		if (!Texture || !Entry.Brush.IsValid())
		{
			return nullptr;
		}

		Entry.Brush->DrawAs = DrawAs;
		Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
		Entry.Brush->ImageSize = ImageSize;
		Entry.Brush->Margin = Margin;
		Entry.Brush->TintColor = FSlateColor(Tint);
		Entry.Brush->SetUVRegion(UVRegion);
		Entry.Brush->SetResourceObject(Texture);
		return Entry.Brush.Get();
	}

	const TCHAR* GetLanguageReferenceButtonPrefix(ET66ButtonType Type)
	{
		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return TEXT("Buttons/basic_button");
		case ET66ButtonType::Danger:
			return TEXT("Buttons/basic_button");
		default:
			return TEXT("Buttons/basic_button");
		}
	}

	FLanguageReferenceButtonBrushSet& GetLanguageReferenceButtonBrushSet(ET66ButtonType Type)
	{
		static FLanguageReferenceButtonBrushSet Neutral;
		static FLanguageReferenceButtonBrushSet Success;
		static FLanguageReferenceButtonBrushSet Danger;

		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return Success;
		case ET66ButtonType::Danger:
			return Danger;
		default:
			return Neutral;
		}
	}

	FLanguageReferenceButtonStyleEntry& GetLanguageReferenceButtonStyleEntry(ET66ButtonType Type)
	{
		static FLanguageReferenceButtonStyleEntry Neutral;
		static FLanguageReferenceButtonStyleEntry Success;
		static FLanguageReferenceButtonStyleEntry Danger;

		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return Success;
		case ET66ButtonType::Danger:
			return Danger;
		default:
			return Neutral;
		}
	}

	const FSlateBrush* ResolveLanguageReferenceButtonBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const TCHAR* Prefix,
		const TCHAR* State,
		const TCHAR* DebugLabel)
	{
		(void)Prefix;
		return ResolveLanguageReferenceBrush(
			Entry,
			FString::Printf(
				TEXT("SourceAssets/UI/Reference/Screens/LanguageSelect/Buttons/languageselect_buttons_pill_%s.png"),
				State ? State : TEXT("normal")),
			FMargin(0.f),
			DebugLabel,
			TextureFilter::TF_Nearest);
	}

	const FButtonStyle& GetLanguageReferenceButtonStyle(ET66ButtonType Type)
	{
		FLanguageReferenceButtonStyleEntry& StyleEntry = GetLanguageReferenceButtonStyleEntry(Type);
		if (!StyleEntry.bInitialized)
		{
			StyleEntry.bInitialized = true;
			StyleEntry.Style = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			StyleEntry.Style.SetNormalPadding(FMargin(0.f));
			StyleEntry.Style.SetPressedPadding(FMargin(0.f));

			FLanguageReferenceButtonBrushSet& BrushSet = GetLanguageReferenceButtonBrushSet(Type);
			const TCHAR* Prefix = GetLanguageReferenceButtonPrefix(Type);
			if (const FSlateBrush* Brush = ResolveLanguageReferenceButtonBrush(BrushSet.Normal, Prefix, TEXT("normal"), TEXT("LanguageButtonNormal")))
			{
				StyleEntry.Style.SetNormal(*Brush);
			}
			if (const FSlateBrush* Brush = ResolveLanguageReferenceButtonBrush(BrushSet.Hovered, Prefix, TEXT("hover"), TEXT("LanguageButtonHover")))
			{
				StyleEntry.Style.SetHovered(*Brush);
			}
			if (const FSlateBrush* Brush = ResolveLanguageReferenceButtonBrush(BrushSet.Pressed, Prefix, TEXT("pressed"), TEXT("LanguageButtonPressed")))
			{
				StyleEntry.Style.SetPressed(*Brush);
			}
			if (const FSlateBrush* Brush = ResolveLanguageReferenceButtonBrush(BrushSet.Disabled, TEXT("Buttons/basic_button"), TEXT("disabled"), TEXT("LanguageButtonDisabled")))
			{
				StyleEntry.Style.SetDisabled(*Brush);
			}
		}

		return StyleEntry.Style;
	}

	const FSlateBrush* GetLanguageReferenceSelectedButtonBrush(ET66ButtonType Type)
	{
		FLanguageReferenceButtonBrushSet& BrushSet = GetLanguageReferenceButtonBrushSet(Type);
		return ResolveLanguageReferenceButtonBrush(
			BrushSet.Selected,
			GetLanguageReferenceButtonPrefix(Type),
			TEXT("selected"),
			TEXT("LanguageButtonSelected"));
	}

	const FSlateBrush* GetLanguageContentShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveLanguageReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/LanguageSelect/Panels/languageselect_panels_reference_scroll_paper_frame.png"),
			FMargin(0.080f, 0.120f, 0.080f, 0.120f),
			TEXT("LanguageContentShellParchment"),
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetLanguageBackdropBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveLanguageReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/LanguageSelect/Panels/languageselect_panels_fullscreen_fullscreen_panel_wide.png"),
			FMargin(0.060f, 0.115f, 0.060f, 0.115f),
			TEXT("LanguageBackdropWoodFrame"),
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetLanguageRowShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveLanguageReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/LanguageSelect/Panels/languageselect_panels_fullscreen_row_shell_quiet.png"),
			FMargin(0.070f, 0.155f, 0.070f, 0.155f),
			TEXT("LanguageRowShellV16"),
			TextureFilter::TF_Nearest);
	}

	const FScrollBarStyle* GetLanguageReferenceScrollBarStyle()
	{
		static FScrollBarStyle Style = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar");
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush TrackEntry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush ThumbEntry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush HoverEntry;

		const FString ControlsPath = TEXT("SourceAssets/UI/Reference/Screens/LanguageSelect/Controls/languageselect_controls_controls_sheet.png");
		const FBox2f VerticalBarUV(
			FVector2f(4.f / 1350.f, 4.f / 926.f),
			FVector2f(90.f / 1350.f, 644.f / 926.f));

		const FSlateBrush* TrackBrush = ResolveLanguageReferenceRegionBrush(
			TrackEntry,
			ControlsPath,
			FMargin(0.42f, 0.085f, 0.42f, 0.085f),
			VerticalBarUV,
			FVector2D(14.f, 120.f),
			ESlateBrushDrawType::Box,
			FLinearColor(0.35f, 0.34f, 0.30f, 0.70f),
			TEXT("LanguageScrollbarTrackV16"),
			TextureFilter::TF_Nearest);
		const FSlateBrush* ThumbBrush = ResolveLanguageReferenceRegionBrush(
			ThumbEntry,
			ControlsPath,
			FMargin(0.38f, 0.115f, 0.38f, 0.115f),
			VerticalBarUV,
			FVector2D(16.f, 96.f),
			ESlateBrushDrawType::Box,
			FLinearColor(0.93f, 0.82f, 0.52f, 1.0f),
			TEXT("LanguageScrollbarThumbV16"),
			TextureFilter::TF_Nearest);
		const FSlateBrush* HoverBrush = ResolveLanguageReferenceRegionBrush(
			HoverEntry,
			ControlsPath,
			FMargin(0.38f, 0.115f, 0.38f, 0.115f),
			VerticalBarUV,
			FVector2D(16.f, 96.f),
			ESlateBrushDrawType::Box,
			FLinearColor(1.0f, 0.90f, 0.62f, 1.0f),
			TEXT("LanguageScrollbarThumbHoverV16"),
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

	const FSlateBrush* GetLanguageSceneBackgroundBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveLanguageReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/LanguageSelect/ScreenArt/languageselect_screen_art_mainmenu_main_menu_scene_plate_v1.png"),
			FMargin(0.f),
			TEXT("LanguageSceneBackground"));
	}

	TSharedRef<SWidget> MakeLanguageButton(const FT66ButtonParams& Params)
	{
		const int32 FontSize = Params.FontSize > 0 ? Params.FontSize : 28;
		const TAttribute<FText> ButtonText = Params.DynamicLabel.IsBound()
			? Params.DynamicLabel
			: TAttribute<FText>(Params.Label);
		const TAttribute<FSlateColor> TextColor = Params.bHasTextColorOverride
			? Params.TextColorOverride
			: TAttribute<FSlateColor>(FSlateColor(FLinearColor(0.96f, 0.84f, 0.58f, 1.0f)));
		const FMargin ContentPadding = Params.Padding.Left >= 0.f ? Params.Padding : FMargin(26.f, 10.f, 26.f, 9.f);

		FSlateFontInfo ButtonFont = FT66Style::MakeFont(*Params.FontWeight, FontSize);
		ButtonFont.LetterSpacing = 0;

		const TSharedRef<SWidget> Content = Params.CustomContent.IsValid()
			? Params.CustomContent.ToSharedRef()
			: StaticCastSharedRef<SWidget>(
				SNew(STextBlock)
				.Text(ButtonText)
				.Font(ButtonFont)
				.ColorAndOpacity(TextColor)
				.Justification(ETextJustify::Center)
				.ShadowOffset(FVector2D(0.f, 1.f))
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.70f))
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis));

		const FButtonStyle& ButtonStyle = GetLanguageReferenceButtonStyle(Params.Type);
		return T66ScreenSlateHelpers::MakeReferenceSlicedPlateButton(
			Params.OnClicked,
			Content,
			&ButtonStyle.Normal,
			&ButtonStyle.Hovered,
			&ButtonStyle.Pressed,
			&ButtonStyle.Disabled,
			Params.MinWidth,
			Params.Height > 0.f ? Params.Height : 50.f,
			ContentPadding,
			Params.IsEnabled,
			Params.Visibility);
	}

	TSharedRef<SWidget> MakeLanguageRowButton(const FText& Label, FOnClicked OnClicked, const TAttribute<bool>& IsSelected)
	{
		const FButtonStyle& ButtonStyle = GetLanguageReferenceButtonStyle(ET66ButtonType::Neutral);
		const FSlateBrush* SelectedBrush = GetLanguageReferenceSelectedButtonBrush(ET66ButtonType::Neutral);
		const TAttribute<EVisibility> MarkerVisibility = TAttribute<EVisibility>::Create(
			TAttribute<EVisibility>::FGetter::CreateLambda([IsSelected]()
			{
				return IsSelected.Get(false) ? EVisibility::Visible : EVisibility::Collapsed;
			}));

		FSlateFontInfo MarkerFont = FT66Style::MakeFont(TEXT("Bold"), 24);
		MarkerFont.LetterSpacing = 0;
		FSlateFontInfo LabelFont = FT66Style::MakeFont(TEXT("Bold"), 24);
		LabelFont.LetterSpacing = 0;

		return T66ScreenSlateHelpers::MakeReferenceSlicedPlateButton(
			MoveTemp(OnClicked),
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(27.f, 0.f, 0.f, 0.f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("*")))
				.Font(MarkerFont)
				.ColorAndOpacity(FLinearColor(1.0f, 0.88f, 0.36f, 1.0f))
				.ShadowOffset(FVector2D(0.f, 1.f))
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.75f))
				.Visibility(MarkerVisibility)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(LabelFont)
				.ColorAndOpacity(FLinearColor(0.98f, 0.86f, 0.62f, 1.0f))
				.Justification(ETextJustify::Center)
				.ShadowOffset(FVector2D(0.f, 2.f))
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.82f))
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			],
			&ButtonStyle.Normal,
			&ButtonStyle.Hovered,
			&ButtonStyle.Pressed,
			&ButtonStyle.Disabled,
			760.f,
			54.f,
			FMargin(0.f),
			TAttribute<bool>(true),
			TAttribute<EVisibility>(EVisibility::Visible),
			0.105f,
			SelectedBrush,
			IsSelected);
	}

	TSharedRef<SWidget> MakeLanguagePanel(const TSharedRef<SWidget>& Content, ET66PanelType Type, const FLinearColor& FillColor, const FMargin& Padding)
	{
		if (const FSlateBrush* ReferenceBrush = Type == ET66PanelType::Panel ? GetLanguageContentShellBrush() : GetLanguageRowShellBrush())
		{
			return SNew(SBorder)
				.BorderImage(ReferenceBrush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(Type)
				.SetBorderVisual(ET66ButtonBorderVisual::None)
				.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
				.SetColor(FillColor)
				.SetPadding(Padding));
	}

	FSlateFontInfo T66LanguageNameFont()
	{
		FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Bold"), 24);
		Font.LetterSpacing = 0;
		return Font;
	}

	FSlateFontInfo T66LanguageTitleFont()
	{
		FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Bold"), 35);
		Font.LetterSpacing = 0;
		return Font;
	}

	FText GetLanguageReferenceDisplayName(ET66Language Language, const UT66LocalizationSubsystem* Loc)
	{
		switch (Language)
		{
		case ET66Language::English:
			return FText::AsCultureInvariant(TEXT("ENGLISH"));
		case ET66Language::ChineseSimplified:
			return FText::AsCultureInvariant(TEXT("SIMPLIFIED CHINESE"));
		case ET66Language::ChineseTraditional:
			return FText::AsCultureInvariant(TEXT("TRADITIONAL CHINESE"));
		case ET66Language::Japanese:
			return FText::AsCultureInvariant(TEXT("JAPANESE"));
		case ET66Language::Korean:
			return FText::AsCultureInvariant(TEXT("KOREAN"));
		case ET66Language::Russian:
			return FText::AsCultureInvariant(TEXT("RUSSIAN"));
		default:
			return FText::FromString((Loc ? Loc->GetLanguageDisplayName(Language).ToString() : FString(TEXT(""))).ToUpper());
		}
	}
}

UT66LanguageSelectScreen::UT66LanguageSelectScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::LanguageSelect;
	bIsModal = false;
}

UT66LocalizationSubsystem* UT66LanguageSelectScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

TSharedRef<SWidget> UT66LanguageSelectScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	// Build language buttons
	TSharedRef<SVerticalBox> LanguageButtons = SNew(SVerticalBox);
	
	if (Loc)
	{
		TArray<ET66Language> Languages = Loc->GetAvailableLanguages();
		for (ET66Language Lang : Languages)
		{
			FText LangName = GetLanguageReferenceDisplayName(Lang, Loc);
			const TAttribute<bool> IsSelected = TAttribute<bool>::Create(
				TAttribute<bool>::FGetter::CreateLambda([this, Lang]()
				{
					return Lang == PreviewedLanguage;
				}));

			LanguageButtons->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(30.0f, 5.0f, 0.0f, 5.0f)
			[
				SNew(SBox)
				.WidthOverride(760.0f)
				.HeightOverride(54.0f)
				[
					MakeLanguageRowButton(
						LangName,
						FOnClicked::CreateUObject(this, &UT66LanguageSelectScreen::HandleLanguageClicked, Lang),
						IsSelected)
				]
			];
		}
	}

	FText TitleText = NSLOCTEXT("T66.LanguageSelect.Reference", "Title", "SELECT LANGUAGE");
	FText ConfirmText = NSLOCTEXT("T66.LanguageSelect.Reference", "Confirm", "CONFIRM");

	const float ScreenPadding = 60.0f;
	const bool bModalPresentation = (UIManager && UIManager->GetCurrentModalType() == ScreenType) || (!UIManager && GetOwningPlayer() && GetOwningPlayer()->IsPaused());
	const float ResponsiveScale = FMath::Max(FT66Style::GetViewportResponsiveScale(), KINDA_SMALL_NUMBER);
	const float TopBarOverlapPx = 18.f;
	const float TopInset = bModalPresentation
		? 0.f
		: FMath::Max(0.f, ((UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f) - TopBarOverlapPx) / ResponsiveScale);

	const TSharedRef<SWidget> LanguageList =
		SNew(SBox)
		.WidthOverride(836.f)
		.HeightOverride(384.f)
		[
			SNew(SScrollBox)
			.Orientation(Orient_Vertical)
			.ScrollBarStyle(GetLanguageReferenceScrollBarStyle())
			.ScrollBarVisibility(EVisibility::Visible)
			.ScrollBarThickness(FVector2D(14.f, 14.f))
			.ScrollBarPadding(FMargin(24.f, 0.f, 0.f, 0.f))
			.ConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible)
			+ SScrollBox::Slot()
			.Padding(FMargin(0.0f, 0.0f, 18.0f, 0.0f))
			[
				LanguageButtons
			]
		];

	const TSharedRef<SWidget> PageContent =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0.0f, 48.0f, 0.0f, 18.0f)
		[
			SNew(STextBlock)
			.Text(TitleText)
			.Font(T66LanguageTitleFont())
			.ColorAndOpacity(FLinearColor(0.08f, 0.045f, 0.018f, 1.0f))
			.ShadowOffset(FVector2D(0.f, 1.f))
			.ShadowColorAndOpacity(FLinearColor(0.90f, 0.68f, 0.35f, 0.35f))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			LanguageList
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0.0f, 22.0f, 0.0f, 0.0f)
		[
			SNew(SBox)
			.WidthOverride(253.f)
			.HeightOverride(58.f)
			[
				MakeLanguageButton(
					FT66ButtonParams(ConfirmText, FOnClicked::CreateUObject(this, &UT66LanguageSelectScreen::HandleConfirmClicked), ET66ButtonType::Success)
					.SetMinWidth(253.f)
					.SetHeight(58.f)
					.SetFontSize(22)
					.SetPadding(FMargin(24.f, 9.f, 24.f, 8.f)))
			]
		];

	if (bModalPresentation)
	{
		return T66ScreenSlateHelpers::MakeCenteredScrimModal(
			MakeLanguagePanel(
				SNew(SBox)
				.WidthOverride(960.f)
				[
					PageContent
				],
				ET66PanelType::Panel,
				T66LanguageShellFill(),
				FMargin(24.f)),
			FMargin(ScreenPadding),
			0.0f,
			0.0f,
			true);
	}

	const TSharedRef<SWidget> LanguageSurface =
		SNew(SBox)
		.Padding(FMargin(0.f, TopInset, 0.f, 0.f))
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		[
			SNew(SBox)
			.WidthOverride(1018.f)
			.HeightOverride(604.f)
			[
				MakeLanguagePanel(
					PageContent,
					ET66PanelType::Panel,
					T66LanguageShellFill(),
					FMargin(0.f))
			]
		];

	if (const FSlateBrush* SceneBackgroundBrush = GetLanguageSceneBackgroundBrush())
	{
		const FSlateBrush* BackdropBrush = GetLanguageBackdropBrush();
		const FSlateBrush* BackgroundBrush = BackdropBrush ? BackdropBrush : SceneBackgroundBrush;
		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(BackgroundBrush)
				.BorderBackgroundColor(FLinearColor::White)
			]
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.030f, 0.018f, 0.010f, BackdropBrush ? 0.18f : 0.82f))
			]
			+ SOverlay::Slot()
			[
				LanguageSurface
			];
	}

	return LanguageSurface;
}

void UT66LanguageSelectScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		OriginalLanguage = Loc->GetCurrentLanguage();
		PreviewedLanguage = OriginalLanguage;
	}
}

void UT66LanguageSelectScreen::SelectLanguage(ET66Language Language)
{
	PreviewedLanguage = Language;
	
	// Just update visual selection, don't apply yet
	// Language will be applied on Confirm
	InvalidateLayoutAndVolatility();
}

FReply UT66LanguageSelectScreen::HandleLanguageClicked(ET66Language Language)
{
	SelectLanguage(Language);
	return FReply::Handled();
}

FReply UT66LanguageSelectScreen::HandleConfirmClicked()
{
	OnConfirmClicked();
	return FReply::Handled();
}

FReply UT66LanguageSelectScreen::HandleBackClicked()
{
	OnBackClicked();
	return FReply::Handled();
}

void UT66LanguageSelectScreen::OnConfirmClicked()
{
	const bool bModalPresentation = UIManager && UIManager->GetCurrentModalType() == ScreenType;

	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->SetLanguage(PreviewedLanguage);
	}

	if (bModalPresentation)
	{
		CloseModal();
	}
	else if (UIManager)
	{
		UIManager->GoBack();
	}

	if (UIManager)
	{
		UIManager->RebuildAllVisibleUI();
	}
}

void UT66LanguageSelectScreen::OnBackClicked()
{
	if (UIManager && UIManager->GetCurrentModalType() == ScreenType)
	{
		CloseModal();
		return;
	}

	if (UIManager)
	{
		UIManager->GoBack();
	}
}

void UT66LanguageSelectScreen::RebuildLanguageList()
{
	// Force widget rebuild
	ForceRebuildSlate();
}
