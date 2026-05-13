// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66LanguageSelectScreen.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66UIManager.h"
#include "Engine/GameInstance.h"
#include "UI/Style/T66FlatStyle.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

namespace
{
	FName LanguageSelectTag(const TCHAR* Name)
	{
		return FName(Name);
	}

	FName LanguageSelectRowTag(const int32 Index)
	{
		return FName(*FString::Printf(TEXT("LanguageSelect.Row%02d"), Index + 1));
	}

	TSharedRef<SWidget> MakeLanguageSelectButtonLabel(const FText& Label, const TAttribute<bool>& IsSelected)
	{
		const TAttribute<EVisibility> MarkerVisibility = TAttribute<EVisibility>::Create(
			TAttribute<EVisibility>::FGetter::CreateLambda([IsSelected]()
			{
				return IsSelected.Get(false) ? EVisibility::Visible : EVisibility::Collapsed;
			}));

		return SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(692.f)
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					.StretchDirection(EStretchDirection::DownOnly)
					[
						SNew(STextBlock)
						.Text(Label)
						.Font(FT66FlatStyle::MakeBoldFont(24))
						.ColorAndOpacity(FT66FlatStyle::PrimaryText())
						.Justification(ETextJustify::Center)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.Clipping(EWidgetClipping::ClipToBounds)
					]
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(27.f, 0.f, 0.f, 0.f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("*")))
				.Font(FT66FlatStyle::MakeBoldFont(22))
				.ColorAndOpacity(FT66FlatStyle::SelectedText())
				.Visibility(MarkerVisibility)
			]
			;
	}

	FText GetLanguageFlatDisplayName(ET66Language Language, const UT66LocalizationSubsystem* Loc)
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

	TSharedRef<SVerticalBox> LanguageButtons = SNew(SVerticalBox);
	
	if (Loc)
	{
		TArray<ET66Language> Languages = Loc->GetAvailableLanguages();
		int32 RowIndex = 0;
		for (ET66Language Lang : Languages)
		{
			FText LangName = GetLanguageFlatDisplayName(Lang, Loc);
			const TAttribute<bool> IsSelected = TAttribute<bool>::Create(
				TAttribute<bool>::FGetter::CreateLambda([this, Lang]()
				{
					return Lang == PreviewedLanguage;
				}));
			const ET66FlatState RowState = (Lang == PreviewedLanguage) ? ET66FlatState::Selected : ET66FlatState::Default;

			LanguageButtons->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.0f, 4.0f, 0.0f, 4.0f)
			[
				SNew(SBox)
				.WidthOverride(792.0f)
				.HeightOverride(54.0f)
				[
					FT66FlatStyle::MakeFlatToggleGroupButton(
						RowState,
						MakeLanguageSelectButtonLabel(LangName, IsSelected),
						FOnClicked::CreateUObject(this, &UT66LanguageSelectScreen::HandleLanguageClicked, Lang),
						FMargin(0.f),
						792.f,
						54.f,
						true,
						LanguageSelectRowTag(RowIndex),
						LanguageSelectTag(TEXT("LanguageSelect.LanguageSelection")))
				]
			];
			++RowIndex;
		}
	}

	FText TitleText = NSLOCTEXT("T66.LanguageSelect.Flat", "Title", "SELECT LANGUAGE");
	FText ConfirmText = NSLOCTEXT("T66.LanguageSelect.Flat", "Confirm", "CONFIRM");

	const float ScreenPadding = 60.0f;
	const bool bModalPresentation = (UIManager && UIManager->GetCurrentModalType() == ScreenType) || (!UIManager && GetOwningPlayer() && GetOwningPlayer()->IsPaused());

	const TSharedRef<SWidget> LanguageList =
		FT66FlatStyle::AttachMetadata(
			SNew(SBox)
			.WidthOverride(836.f)
			.HeightOverride(384.f)
			[
				SNew(SScrollBox)
				.Orientation(Orient_Vertical)
				.ScrollBarVisibility(EVisibility::Visible)
				.ScrollBarThickness(FVector2D(14.f, 14.f))
				.ScrollBarPadding(FMargin(24.f, 0.f, 0.f, 0.f))
				.ConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible)
				+ SScrollBox::Slot()
				.Padding(FMargin(0.0f, 0.0f, 18.0f, 0.0f))
				[
					LanguageButtons
				]
			],
			LanguageSelectTag(TEXT("LanguageSelect.LanguageList")),
			TEXT("List"),
			ET66FlatState::Default);

	const TSharedRef<SWidget> PageContent =
		SNew(SConstraintCanvas)
		+ SConstraintCanvas::Slot()
		.Anchors(FAnchors(0.f, 0.f))
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(373.f, 48.f, 273.f, 51.f))
		[
			FT66FlatStyle::MakeFlatLabel(
				TitleText,
				ET66FlatLabelRole::Header,
				ETextJustify::Center,
				LanguageSelectTag(TEXT("LanguageSelect.Title")))
		]
		+ SConstraintCanvas::Slot()
		.Anchors(FAnchors(0.f, 0.f))
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(91.f, 117.f, 836.f, 384.f))
		[
			LanguageList
		]
		+ SConstraintCanvas::Slot()
		.Anchors(FAnchors(0.f, 0.f))
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(382.f, 522.f, 253.f, 58.f))
		[
			FT66FlatStyle::MakeFlatButton(
				ET66FlatState::Selected,
				ConfirmText,
				FOnClicked::CreateUObject(this, &UT66LanguageSelectScreen::HandleConfirmClicked),
				nullptr,
				nullptr,
				FMargin(24.f, 9.f, 24.f, 8.f),
				253.f,
				58.f,
				true,
				22,
				LanguageSelectTag(TEXT("LanguageSelect.ConfirmButton")))
		];

	if (bModalPresentation)
	{
		return FT66FlatStyle::AttachMetadata(
			T66ScreenSlateHelpers::MakeCenteredScrimModal(
				FT66FlatStyle::MakeFlatPanel(
					ET66FlatState::Default,
					FMargin(0.f),
					SNew(SBox)
					.WidthOverride(960.f)
					.HeightOverride(604.f)
					[
						PageContent
					],
					nullptr,
					LanguageSelectTag(TEXT("LanguageSelect.Panel"))),
				FMargin(ScreenPadding),
				0.0f,
				0.0f,
				true),
			LanguageSelectTag(TEXT("LanguageSelect.Root")),
			TEXT("Root"),
			ET66FlatState::Default);
	}

	const TSharedRef<SWidget> LanguageSurface =
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			[
				SNew(SBox)
				.WidthOverride(1018.f)
				.HeightOverride(604.f)
				[
					FT66FlatStyle::MakeFlatPanel(
						ET66FlatState::Default,
						FMargin(0.f),
						PageContent,
						nullptr,
						LanguageSelectTag(TEXT("LanguageSelect.Panel")))
				]
			]
		];

	return FT66FlatStyle::AttachMetadata(
		T66ScreenSlateHelpers::MakeTopBarScreenRoot(
			UIManager,
			LanguageSurface,
			SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor::Black),
			FLinearColor::Transparent,
			FMargin(0.f)),
		LanguageSelectTag(TEXT("LanguageSelect.Root")),
		TEXT("Root"),
		ET66FlatState::Default);
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
	
	ForceRebuildSlate();
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
