// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66QuitConfirmationModal.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66UIManager.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float QuitModalButtonHeight = 58.0f;

	struct FMasterLibraryButtonBrushSet
	{
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Normal;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Hovered;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Pressed;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Disabled;
	};

	struct FMasterLibraryButtonStyleEntry
	{
		FButtonStyle Style;
		bool bInitialized = false;
	};

	const TCHAR* GetMasterLibraryButtonPrefix(ET66ButtonType Type)
	{
		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return TEXT("Buttons/basic_button");
		case ET66ButtonType::Danger:
			return TEXT("Buttons/DangerCTA");
		default:
			return TEXT("Buttons/basic_button");
		}
	}

	FMasterLibraryButtonBrushSet& GetMasterLibraryButtonBrushSet(ET66ButtonType Type)
	{
		static FMasterLibraryButtonBrushSet Neutral;
		static FMasterLibraryButtonBrushSet Success;
		static FMasterLibraryButtonBrushSet Danger;

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

	FMasterLibraryButtonStyleEntry& GetMasterLibraryButtonStyleEntry(ET66ButtonType Type)
	{
		static FMasterLibraryButtonStyleEntry Neutral;
		static FMasterLibraryButtonStyleEntry Success;
		static FMasterLibraryButtonStyleEntry Danger;

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

	const FSlateBrush* ResolveMasterLibraryBrush(
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

	const FSlateBrush* ResolveMasterLibraryButtonBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const TCHAR* Prefix,
		const TCHAR* State,
		const TCHAR* DebugLabel)
	{
		if (FCString::Strcmp(Prefix, TEXT("Buttons/DangerCTA")) == 0)
		{
			const FString RelativePath = FString::Printf(TEXT("Buttons/DangerCTA/%s.png"), State);
			return T66ScreenSlateHelpers::GetReferenceSharedBrush(*RelativePath, FMargin(0.f), DebugLabel);
		}

		return ResolveMasterLibraryBrush(
			Entry,
			T66ScreenSlateHelpers::MakeReferenceChromeButtonAssetPath(TEXT("Pill"), State),
			FMargin(0.f),
			DebugLabel,
			TextureFilter::TF_Nearest);
	}

	const FButtonStyle& GetMasterLibraryButtonStyle(ET66ButtonType Type)
	{
		FMasterLibraryButtonStyleEntry& StyleEntry = GetMasterLibraryButtonStyleEntry(Type);
		if (!StyleEntry.bInitialized)
		{
			StyleEntry.bInitialized = true;
			StyleEntry.Style = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			StyleEntry.Style.SetNormalPadding(FMargin(0.0f));
			StyleEntry.Style.SetPressedPadding(FMargin(0.0f));

			FMasterLibraryButtonBrushSet& BrushSet = GetMasterLibraryButtonBrushSet(Type);
			const TCHAR* Prefix = GetMasterLibraryButtonPrefix(Type);
			if (const FSlateBrush* Brush = ResolveMasterLibraryButtonBrush(BrushSet.Normal, Prefix, TEXT("normal"), TEXT("QuitModalButtonNormal")))
			{
				StyleEntry.Style.SetNormal(*Brush);
			}
			if (const FSlateBrush* Brush = ResolveMasterLibraryButtonBrush(BrushSet.Hovered, Prefix, TEXT("hover"), TEXT("QuitModalButtonHover")))
			{
				StyleEntry.Style.SetHovered(*Brush);
			}
			if (const FSlateBrush* Brush = ResolveMasterLibraryButtonBrush(BrushSet.Pressed, Prefix, TEXT("pressed"), TEXT("QuitModalButtonPressed")))
			{
				StyleEntry.Style.SetPressed(*Brush);
			}
			if (const FSlateBrush* Brush = ResolveMasterLibraryButtonBrush(BrushSet.Disabled, Prefix, TEXT("disabled"), TEXT("QuitModalButtonDisabled")))
			{
				StyleEntry.Style.SetDisabled(*Brush);
			}
		}

		return StyleEntry.Style;
	}

	const FSlateBrush* GetMasterLibraryModalShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush ShellBrush;
		return ResolveMasterLibraryBrush(
			ShellBrush,
			TEXT("SourceAssets/UI/Reference/Modals/QuitConfirmation/Panels/quitconfirmation_panels_inner_panel_normal.png"),
			FMargin(0.067f, 0.043f, 0.067f, 0.043f),
			TEXT("QuitModalShell"));
	}

	TSharedRef<SWidget> MakeMasterLibraryModalShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		return T66ScreenSlateHelpers::MakeReferenceSharedBorder(
			TEXT("Panels/Modal/modal_shell_wide.png"),
			Content,
			FMargin(0.075f, 0.105f, 0.075f, 0.105f),
			Padding,
			TEXT("QuitModalShellV14"),
			FT66Style::Panel());
	}

	TSharedRef<SWidget> MakeMasterLibraryButton(const FT66ButtonParams& Params)
	{
		const int32 FontSize = Params.FontSize > 0 ? Params.FontSize : 19;
		FSlateFontInfo ButtonFont = FT66Style::MakeFont(*Params.FontWeight, FontSize);
		ButtonFont.LetterSpacing = 0;

		const TAttribute<FText> ButtonText = Params.DynamicLabel.IsBound()
			? Params.DynamicLabel
			: TAttribute<FText>(Params.Label);
		const TAttribute<FSlateColor> TextColor = Params.bHasTextColorOverride
			? Params.TextColorOverride
			: TAttribute<FSlateColor>(FSlateColor(FT66Style::Text()));
		const FMargin ContentPadding = Params.Padding.Left >= 0.0f
			? Params.Padding
			: FMargin(18.0f, 8.0f, 18.0f, 7.0f);

		const TSharedRef<SWidget> ButtonContent = Params.CustomContent.IsValid()
			? Params.CustomContent.ToSharedRef()
			: StaticCastSharedRef<SWidget>(
				SNew(STextBlock)
				.Text(ButtonText)
				.Font(ButtonFont)
				.ColorAndOpacity(TextColor)
				.Justification(ETextJustify::Center)
				.ShadowOffset(FVector2D(1.0f, 1.0f))
				.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.68f))
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds));

		const FButtonStyle& ButtonStyle = GetMasterLibraryButtonStyle(Params.Type);
		return T66ScreenSlateHelpers::MakeReferenceSlicedPlateButton(
			Params.OnClicked,
			ButtonContent,
			&ButtonStyle.Normal,
			&ButtonStyle.Hovered,
			&ButtonStyle.Pressed,
			&ButtonStyle.Disabled,
			Params.MinWidth,
			Params.Height > 0.0f ? Params.Height : QuitModalButtonHeight,
			ContentPadding,
			Params.IsEnabled,
			Params.Visibility);
	}
}

UT66QuitConfirmationModal::UT66QuitConfirmationModal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::QuitConfirmation;
	bIsModal = true;
}

TSharedRef<SWidget> UT66QuitConfirmationModal::BuildSlateUI()
{
	// Get localization subsystem
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	FText TitleText = Loc ? Loc->GetText_QuitConfirmTitle() : NSLOCTEXT("T66.QuitConfirm", "Title", "QUIT GAME?");
	FText MessageText = Loc ? Loc->GetText_QuitConfirmMessage() : NSLOCTEXT("T66.QuitConfirm", "Message", "Are you sure you want to quit?");
	FText StayText = Loc ? Loc->GetText_NoStay() : NSLOCTEXT("T66.QuitConfirm", "Stay", "NO, I WANT TO STAY");
	FText QuitText = Loc ? Loc->GetText_YesQuit() : NSLOCTEXT("T66.QuitConfirm", "Quit", "YES, I WANT TO QUIT");
	const TSharedRef<SWidget> StayButton =
		MakeMasterLibraryButton(FT66ButtonParams(StayText, FOnClicked::CreateUObject(this, &UT66QuitConfirmationModal::HandleStayClicked), ET66ButtonType::Success)
			.SetMinWidth(306.f)
			.SetHeight(QuitModalButtonHeight)
			.SetFontSize(18)
			.SetPadding(FMargin(18.f, 8.f, 18.f, 7.f)));
	const TSharedRef<SWidget> QuitButton =
		MakeMasterLibraryButton(FT66ButtonParams(QuitText, FOnClicked::CreateUObject(this, &UT66QuitConfirmationModal::HandleQuitClicked), ET66ButtonType::Danger)
			.SetMinWidth(306.f)
			.SetHeight(QuitModalButtonHeight)
			.SetFontSize(18)
			.SetPadding(FMargin(18.f, 8.f, 18.f, 7.f)));

	FSlateFontInfo TitleFont = FT66Style::Tokens::FontBold(38);
	TitleFont.LetterSpacing = 130;

	return T66ScreenSlateHelpers::MakeCenteredScrimModal(
		SNew(SBox)
		.WidthOverride(760.0f)
		[
		MakeMasterLibraryModalShell(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.0f, 0.0f, 0.0f, 18.0f)
			[
				SNew(STextBlock)
				.Text(TitleText)
				.Font(TitleFont)
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.0f, 0.0f, 0.0f, 28.0f)
			[
				SNew(SBox)
				.WidthOverride(590.0f)
				[
					SNew(STextBlock)
					.Text(MessageText)
					.Font(FT66Style::Tokens::FontRegular(19))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					.AutoWrapText(true)
					.Justification(ETextJustify::Center)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				T66ScreenSlateHelpers::MakeTwoButtonRow(StayButton, QuitButton)
			],
			FMargin(48.0f, 36.0f, 48.0f, 40.0f))
		],
		FMargin(0.0f),
		0.0f,
		0.0f,
		true);
}

FReply UT66QuitConfirmationModal::HandleStayClicked() { OnStayClicked(); return FReply::Handled(); }
FReply UT66QuitConfirmationModal::HandleQuitClicked() { OnQuitClicked(); return FReply::Handled(); }

void UT66QuitConfirmationModal::OnStayClicked() { CloseModal(); }
void UT66QuitConfirmationModal::OnQuitClicked() { UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false); }

