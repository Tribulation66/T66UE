// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66SavePreviewScreen.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/T66UIManager.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	FName SavePreviewTag(const TCHAR* Tag)
	{
		return FName(Tag);
	}

	TSharedRef<SWidget> MakeSavePreviewRect(const FLinearColor& Color, const FName Tag, const FString& Role)
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
			.BorderBackgroundColor(Color)
			.Visibility(EVisibility::HitTestInvisible),
			Tag,
			Role,
			ET66FlatState::Default);
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

	constexpr float CanvasW = 1920.f;
	constexpr float CanvasH = 1080.f;
	const TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
	auto AddSlot = [&Canvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		Canvas->AddSlot()
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X, Y, W, H))
			[
				Widget
			];
	};
	auto MakeLabel = [](
		const FName Tag,
		const FText& Text,
		const int32 FontSize,
		const FLinearColor& Color,
		const bool bBold,
		const ETextJustify::Type Justification = ETextJustify::Center) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(STextBlock)
			.Text(Text)
			.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
			.ColorAndOpacity(Color)
			.Justification(Justification)
			.AutoWrapText(true)
			.WrapTextAt(650.f)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds)
			.Visibility(EVisibility::HitTestInvisible),
			Tag,
			TEXT("Label"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true);
	};

	AddSlot(0.f, 0.f, CanvasW, CanvasH,
		MakeSavePreviewRect(FT66FlatStyle::BackgroundColor(), SavePreviewTag(TEXT("SavePreview.Background")), TEXT("Background")));
	AddSlot(0.f, 0.f, CanvasW, CanvasH,
		MakeSavePreviewRect(FLinearColor(0.f, 0.f, 0.f, 0.58f), SavePreviewTag(TEXT("SavePreview.Scrim")), TEXT("Scrim")));
	AddSlot(412.8f, 697.5f, 1094.4f, 342.3f,
		FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(0.f),
			SNullWidget::NullWidget,
			nullptr,
			SavePreviewTag(TEXT("SavePreview.ModalPanel"))));
	AddSlot(473.3f, 740.7f, 973.4f, 250.1f,
		FT66FlatStyle::AttachMetadata(SNew(SBox), SavePreviewTag(TEXT("SavePreview.Content")), TEXT("Content"), ET66FlatState::Default));
	AddSlot(473.3f, 740.7f, 973.4f, 62.f,
		MakeLabel(
			SavePreviewTag(TEXT("SavePreview.Title")),
			PreviewTitle,
			30,
			FT66FlatStyle::SelectedText(),
			true));
	AddSlot(473.3f, 811.3f, 973.4f, 70.f,
		MakeLabel(
			SavePreviewTag(TEXT("SavePreview.Subtitle")),
			SubtitleText,
			17,
			FT66FlatStyle::SecondaryText(),
			false));
	AddSlot(649.f, 907.2f, 622.1f, 83.5f,
		FT66FlatStyle::AttachMetadata(SNew(SBox), SavePreviewTag(TEXT("SavePreview.ButtonRow")), TEXT("ActionRow"), ET66FlatState::Default));
	AddSlot(649.f, 907.2f, 302.4f, 83.5f,
		FT66FlatStyle::MakeFlatButton(
			ET66FlatState::Default,
			BackText,
			FOnClicked::CreateUObject(this, &UT66SavePreviewScreen::HandleBackClicked),
			nullptr,
			nullptr,
			FMargin(20.f, 8.f),
			0.f,
			0.f,
			true,
			22,
			SavePreviewTag(TEXT("SavePreview.BackButton"))));
	AddSlot(968.6f, 907.2f, 302.4f, 83.5f,
		FT66FlatStyle::MakeFlatButton(
			ET66FlatState::Selected,
			LoadText,
			FOnClicked::CreateUObject(this, &UT66SavePreviewScreen::HandleLoadClicked),
			nullptr,
			nullptr,
			FMargin(20.f, 8.f),
			0.f,
			0.f,
			true,
			22,
			SavePreviewTag(TEXT("SavePreview.LoadButton"))));

	const TSharedRef<SWidget> RootContent = SNew(SBox)
		.WidthOverride(CanvasW)
		.HeightOverride(CanvasH)
		[
			Canvas
		];

	return FT66FlatStyle::AttachMetadata(
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				RootContent
			]
		],
		SavePreviewTag(TEXT("SavePreview.Root")),
		TEXT("Root"),
		ET66FlatState::Default);
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
