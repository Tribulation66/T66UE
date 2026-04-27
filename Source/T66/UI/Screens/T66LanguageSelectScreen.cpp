// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66LanguageSelectScreen.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
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
		const TCHAR* DebugLabel)
	{
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(RelativePath),
			Margin,
			DebugLabel);
	}

	const TCHAR* GetLanguageReferenceButtonPrefix(ET66ButtonType Type)
	{
		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return TEXT("TopBar/topbar_nav");
		case ET66ButtonType::Danger:
			return TEXT("TopBar/topbar_nav");
		default:
			return TEXT("TopBar/topbar_nav");
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
		return ResolveLanguageReferenceBrush(
			Entry,
			FString::Printf(TEXT("SourceAssets/UI/MasterLibrary/Slices/%s_%s.png"), Prefix, State),
			FMargin(0.093f, 0.213f, 0.093f, 0.213f),
			DebugLabel);
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
			if (const FSlateBrush* Brush = ResolveLanguageReferenceButtonBrush(BrushSet.Disabled, TEXT("TopBar/topbar_nav"), TEXT("disabled"), TEXT("LanguageButtonDisabled")))
			{
				StyleEntry.Style.SetDisabled(*Brush);
			}
		}

		return StyleEntry.Style;
	}

	const FSlateBrush* GetLanguageContentShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveLanguageReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/panel_large_normal.png"),
			FMargin(0.067f, 0.043f, 0.067f, 0.043f),
			TEXT("LanguageContentShell"));
	}

	const FSlateBrush* GetLanguageRowShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveLanguageReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/modal_frame_normal.png"),
			FMargin(0.052f, 0.094f, 0.052f, 0.094f),
			TEXT("LanguageRowShell"));
	}

	const FSlateBrush* GetLanguageSceneBackgroundBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveLanguageReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/ScreenArt/MainMenu/main_menu_scene_plate_imagegen_20260425_v1.png"),
			FMargin(0.f),
			TEXT("LanguageSceneBackground"));
	}

	TSharedRef<SWidget> MakeLanguageButton(const FT66ButtonParams& Params)
	{
		const int32 FontSize = Params.FontSize > 0 ? Params.FontSize : 18;
		const TAttribute<FText> ButtonText = Params.DynamicLabel.IsBound()
			? Params.DynamicLabel
			: TAttribute<FText>(Params.Label);
		const TAttribute<FSlateColor> TextColor = Params.bHasTextColorOverride
			? Params.TextColorOverride
			: TAttribute<FSlateColor>(FSlateColor(FT66Style::Tokens::Text));
		const FMargin ContentPadding = Params.Padding.Left >= 0.f ? Params.Padding : FMargin(18.f, 8.f, 18.f, 7.f);

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

		return SNew(SBox)
			.WidthOverride(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize())
			.HeightOverride(Params.Height > 0.f ? Params.Height : 50.f)
			.Visibility(Params.Visibility)
			[
				SNew(SButton)
				.ButtonStyle(&GetLanguageReferenceButtonStyle(Params.Type))
				.ContentPadding(ContentPadding)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.IsEnabled(Params.IsEnabled)
				.OnClicked(FT66Style::DebounceClick(Params.OnClicked))
				[
					Content
				]
			];
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
		return FT66Style::MakeFont(TEXT("Regular"), 22);
	}

	FSlateFontInfo T66LanguageTitleFont()
	{
		return FT66Style::MakeFont(TEXT("Bold"), 42);
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
			FText LangName = Loc->GetLanguageDisplayName(Lang);
			// Make selection highlight persistent by binding the row color to PreviewedLanguage.
			// This avoids having to rebuild the whole Slate tree each click (and keeps scroll position).
			const auto GetRowBgColor = [this, Lang]()
			{
				const bool bIsSelected = (Lang == PreviewedLanguage);
				return bIsSelected ? FLinearColor(1.10f, 1.02f, 0.74f, 1.0f) : FLinearColor::White;
			};

			const auto GetRowTextColor = [this, Lang]()
			{
				const bool bIsSelected = (Lang == PreviewedLanguage);
				return bIsSelected ? FLinearColor::White : FLinearColor(0.92f, 0.92f, 0.95f, 1.0f);
			};

			LanguageButtons->AddSlot()
			.AutoHeight()
			// Revert list spacing closer to the original.
			.HAlign(HAlign_Center)
			.Padding(10.0f, 5.0f)
			[
				SNew(SBox)
				.WidthOverride(560.0f)
				.HeightOverride(60.0f)
				[
					SNew(SButton)
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Center)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66LanguageSelectScreen::HandleLanguageClicked, Lang))
					[
						SNew(SBorder)
						.BorderImage(GetLanguageRowShellBrush())
						.BorderBackgroundColor_Lambda(GetRowBgColor)
						.Padding(FMargin(14.0f, 8.0f))
						[
							SNew(STextBlock)
							.Text(LangName)
							.Font(T66LanguageNameFont())
							.ColorAndOpacity_Lambda(GetRowTextColor)
							.Justification(ETextJustify::Center)
						]
					]
				]
			];
		}
	}

	FText TitleText = Loc ? Loc->GetText_SelectLanguage() : NSLOCTEXT("T66.LanguageSelect", "Title", "Select Language");
	FText ConfirmText = Loc ? Loc->GetText_Confirm() : NSLOCTEXT("T66.LanguageSelect", "Confirm", "Confirm");

	const float ScreenPadding = 60.0f;
	const bool bModalPresentation = (UIManager && UIManager->GetCurrentModalType() == ScreenType) || (!UIManager && GetOwningPlayer() && GetOwningPlayer()->IsPaused());
	const float ResponsiveScale = FMath::Max(FT66Style::GetViewportResponsiveScale(), KINDA_SMALL_NUMBER);
	const float TopBarOverlapPx = 8.f;
	const float TopInset = bModalPresentation
		? 0.f
		: FMath::Max(0.f, ((UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f) - TopBarOverlapPx) / ResponsiveScale);

	const TSharedRef<SWidget> LanguageList =
		MakeLanguagePanel(
			SNew(SScrollBox)
			.Orientation(Orient_Vertical)
			.ScrollBarVisibility(EVisibility::Visible)
			.ConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible)
			+ SScrollBox::Slot()
			.Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))
			[
				LanguageButtons
			],
			ET66PanelType::Panel2,
			T66LanguagePanelFill(),
			FMargin(18.f, 16.f, 14.f, 16.f));

	const TSharedRef<SWidget> PageContent =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0.0f, 0.0f, 0.0f, 26.0f)
		[
			SNew(STextBlock)
			.Text(TitleText)
			.Font(T66LanguageTitleFont())
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.HAlign(HAlign_Fill)
		[
			LanguageList
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0.0f, 30.0f, 0.0f, 0.0f)
	[
			SNew(SBox)
			.Visibility(bModalPresentation ? EVisibility::Visible : EVisibility::Collapsed)
			[
				MakeLanguageButton(
					FT66ButtonParams(ConfirmText, FOnClicked::CreateUObject(this, &UT66LanguageSelectScreen::HandleConfirmClicked), ET66ButtonType::Success)
					.SetMinWidth(180.f)
					.SetColor(FT66Style::Tokens::Success))
			]
		];

	if (bModalPresentation)
	{
		return T66ScreenSlateHelpers::MakeCenteredScrimModal(
			MakeLanguagePanel(
				SNew(SBox)
				.WidthOverride(960.f)
				.Padding(FMargin(40.0f, 30.0f))
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
		.Padding(FMargin(14.f, TopInset, 14.f, 0.f))
		[
			MakeLanguagePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					PageContent
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 24.0f, 0.0f, 0.0f)
				[
					MakeLanguageButton(
						FT66ButtonParams(ConfirmText, FOnClicked::CreateUObject(this, &UT66LanguageSelectScreen::HandleConfirmClicked), ET66ButtonType::Success)
						.SetMinWidth(180.f)
						.SetColor(FT66Style::Tokens::Success))
				],
				ET66PanelType::Panel,
				T66LanguageShellFill(),
				FMargin(ScreenPadding, 0.f, ScreenPadding, 28.f))
		];

	if (const FSlateBrush* SceneBackgroundBrush = GetLanguageSceneBackgroundBrush())
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(SceneBackgroundBrush)
			]
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.02f, 0.025f, 0.035f, 0.48f))
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
