// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66SavePreviewScreen.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UIManager.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float T66SavePreviewButtonHeight = 48.f;

	FLinearColor T66SavePreviewBrightText()
	{
		return FLinearColor(0.97f, 0.94f, 0.86f, 1.0f);
	}

	FLinearColor T66SavePreviewGoldText()
	{
		return FLinearColor(0.94f, 0.76f, 0.34f, 1.0f);
	}

	FLinearColor T66SavePreviewMutedText()
	{
		return FLinearColor(0.72f, 0.67f, 0.56f, 1.0f);
	}

	const FSlateBrush* ResolveSavePreviewBrush(
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

	struct FSavePreviewReferenceButtonBrushSet
	{
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Normal;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Hovered;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Pressed;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Disabled;
	};

	struct FSavePreviewReferenceButtonStyleEntry
	{
		FButtonStyle Style;
		bool bInitialized = false;
	};

	const TCHAR* GetSavePreviewButtonPrefix(const ET66ButtonType Type)
	{
		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return TEXT("settings_toggle_on");
		case ET66ButtonType::Danger:
			return TEXT("settings_toggle_off");
		default:
			return TEXT("settings_compact_neutral");
		}
	}

	FSavePreviewReferenceButtonBrushSet& GetSavePreviewButtonBrushSet(const ET66ButtonType Type)
	{
		static FSavePreviewReferenceButtonBrushSet Neutral;
		static FSavePreviewReferenceButtonBrushSet Primary;
		static FSavePreviewReferenceButtonBrushSet Danger;

		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return Primary;
		case ET66ButtonType::Danger:
			return Danger;
		default:
			return Neutral;
		}
	}

	FSavePreviewReferenceButtonStyleEntry& GetSavePreviewButtonStyleEntry(const ET66ButtonType Type)
	{
		static FSavePreviewReferenceButtonStyleEntry Neutral;
		static FSavePreviewReferenceButtonStyleEntry Primary;
		static FSavePreviewReferenceButtonStyleEntry Danger;

		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return Primary;
		case ET66ButtonType::Danger:
			return Danger;
		default:
			return Neutral;
		}
	}

	const FSlateBrush* ResolveSavePreviewButtonBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const TCHAR* Prefix,
		const TCHAR* State,
		const TCHAR* DebugLabel)
	{
		return ResolveSavePreviewBrush(
			Entry,
			FString::Printf(TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/%s_%s.png"), Prefix, State),
			FMargin(0.16f, 0.28f, 0.16f, 0.28f),
			DebugLabel);
	}

	const FButtonStyle& GetSavePreviewButtonStyle(const ET66ButtonType Type)
	{
		FSavePreviewReferenceButtonStyleEntry& StyleEntry = GetSavePreviewButtonStyleEntry(Type);
		if (!StyleEntry.bInitialized)
		{
			StyleEntry.bInitialized = true;
			StyleEntry.Style = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			StyleEntry.Style.SetNormalPadding(FMargin(0.f));
			StyleEntry.Style.SetPressedPadding(FMargin(0.f));

			FSavePreviewReferenceButtonBrushSet& BrushSet = GetSavePreviewButtonBrushSet(Type);
			const TCHAR* Prefix = GetSavePreviewButtonPrefix(Type);
			if (const FSlateBrush* Brush = ResolveSavePreviewButtonBrush(BrushSet.Normal, Prefix, TEXT("normal"), TEXT("SavePreviewButtonNormal")))
			{
				StyleEntry.Style.SetNormal(*Brush);
			}
			if (const FSlateBrush* Brush = ResolveSavePreviewButtonBrush(BrushSet.Hovered, Prefix, TEXT("hover"), TEXT("SavePreviewButtonHover")))
			{
				StyleEntry.Style.SetHovered(*Brush);
			}
			if (const FSlateBrush* Brush = ResolveSavePreviewButtonBrush(BrushSet.Pressed, Prefix, TEXT("pressed"), TEXT("SavePreviewButtonPressed")))
			{
				StyleEntry.Style.SetPressed(*Brush);
			}
			if (const FSlateBrush* Brush = ResolveSavePreviewButtonBrush(BrushSet.Disabled, TEXT("settings_toggle_inactive"), TEXT("normal"), TEXT("SavePreviewButtonDisabled")))
			{
				StyleEntry.Style.SetDisabled(*Brush);
			}
		}

		return StyleEntry.Style;
	}

	const FSlateBrush* GetSavePreviewShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveSavePreviewBrush(
			Entry,
			TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_content_shell_frame.png"),
			FMargin(0.035f, 0.12f, 0.035f, 0.12f),
			TEXT("SavePreviewShell"));
	}

	TSharedRef<SWidget> MakeSavePreviewShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		if (const FSlateBrush* ShellBrush = GetSavePreviewShellBrush())
		{
			return SNew(SBorder)
				.BorderImage(ShellBrush)
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
				.SetColor(FLinearColor(0.f, 0.f, 0.f, 0.92f))
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeSavePreviewButton(FT66ButtonParams Params)
	{
		const int32 FontSize = Params.FontSize > 0 ? Params.FontSize : 14;
		FSlateFontInfo ButtonFont = FT66Style::MakeFont(*Params.FontWeight, FontSize);
		ButtonFont.LetterSpacing = 0;

		const TAttribute<FText> ButtonText = Params.DynamicLabel.IsBound()
			? Params.DynamicLabel
			: TAttribute<FText>(Params.Label);
		const TAttribute<FSlateColor> TextColor = Params.bHasTextColorOverride
			? Params.TextColorOverride
			: TAttribute<FSlateColor>(FSlateColor(T66SavePreviewBrightText()));
		const FMargin ContentPadding = Params.Padding.Left >= 0.f ? Params.Padding : FMargin(16.f, 8.f, 16.f, 7.f);

		const TSharedRef<SWidget> Content = Params.CustomContent.IsValid()
			? Params.CustomContent.ToSharedRef()
			: StaticCastSharedRef<SWidget>(
				SNew(STextBlock)
				.Text(ButtonText)
				.Font(ButtonFont)
				.ColorAndOpacity(TextColor)
				.Justification(ETextJustify::Center)
				.ShadowOffset(FVector2D(1.f, 1.f))
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.78f))
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis));

		return SNew(SBox)
			.MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize())
			.HeightOverride(Params.Height > 0.f ? Params.Height : T66SavePreviewButtonHeight)
			.Visibility(Params.Visibility)
			[
				SNew(SButton)
				.ButtonStyle(&GetSavePreviewButtonStyle(Params.Type))
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
}

UT66SavePreviewScreen::UT66SavePreviewScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::SavePreview;
	bIsModal = true;
}

UT66LocalizationSubsystem* UT66SavePreviewScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	return nullptr;
}

TSharedRef<SWidget> UT66SavePreviewScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const FText PreviewTitle = Loc ? Loc->GetText_Preview() : NSLOCTEXT("T66.SavePreview", "PreviewFallback", "PREVIEW");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const FText LoadText = NSLOCTEXT("T66.SavePreview", "Load", "LOAD");
	const FText SubtitleText = NSLOCTEXT("T66.SavePreview", "Subtitle", "The run is paused for inspection. Back returns to Save Slots, Load resumes normally.");
	const TSharedRef<SWidget> BackButton =
		MakeSavePreviewButton(
			FT66ButtonParams(BackText, FOnClicked::CreateUObject(this, &UT66SavePreviewScreen::HandleBackClicked), ET66ButtonType::Neutral)
			.SetMinWidth(160.f)
			.SetHeight(T66SavePreviewButtonHeight)
			.SetFontSize(12));
	const TSharedRef<SWidget> LoadButton =
		MakeSavePreviewButton(
			FT66ButtonParams(LoadText, FOnClicked::CreateUObject(this, &UT66SavePreviewScreen::HandleLoadClicked), ET66ButtonType::Primary)
			.SetMinWidth(160.f)
			.SetHeight(T66SavePreviewButtonHeight)
			.SetFontSize(12));

	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.58f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(32.f, 32.f, 32.f, 28.f))
		[
			SNew(SBox)
			.WidthOverride(760.f)
			[
				MakeSavePreviewShell(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(PreviewTitle)
						.Font(FT66Style::Tokens::FontBold(24))
						.ColorAndOpacity(T66SavePreviewGoldText())
						.Justification(ETextJustify::Center)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 6.f, 0.f, 18.f)
					[
						SNew(STextBlock)
						.Text(SubtitleText)
						.Font(FT66Style::Tokens::FontRegular(13))
						.ColorAndOpacity(T66SavePreviewMutedText())
						.AutoWrapText(true)
						.Justification(ETextJustify::Center)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						T66ScreenSlateHelpers::MakeTwoButtonRow(
							BackButton,
							LoadButton,
							FMargin(0.f, 0.f, 12.f, 0.f),
							FMargin(0.f))
					],
					FMargin(42.f, 30.f, 42.f, 34.f))
			]
		];
}

FReply UT66SavePreviewScreen::HandleBackClicked()
{
	OnBackClicked();
	return FReply::Handled();
}

FReply UT66SavePreviewScreen::HandleLoadClicked()
{
	OnLoadClicked();
	return FReply::Handled();
}

void UT66SavePreviewScreen::OnBackClicked()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	AT66PlayerController* PC = GetOwningPlayer<AT66PlayerController>();
	if (PC)
	{
		PC->SetPause(false);
	}

	if (GI)
	{
		GI->bSaveSlotPreviewMode = false;
		GI->PendingFrontendScreen = ET66ScreenType::SaveSlots;
	}

	if (UIManager)
	{
		UIManager->HideAllUI();
	}

	UGameplayStatics::OpenLevel(this, UT66GameInstance::GetFrontendLevelName());
}

void UT66SavePreviewScreen::OnLoadClicked()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	AT66PlayerController* PC = GetOwningPlayer<AT66PlayerController>();
	if (GI)
	{
		GI->bSaveSlotPreviewMode = false;
		GI->bRestoreSaveSlotsState = false;
		GI->PendingSaveSlotsPage = 0;
	}

	if (UIManager)
	{
		UIManager->CloseModal();
	}

	if (PC)
	{
		PC->SetPause(false);
		PC->RestoreGameplayInputMode();
	}
}
