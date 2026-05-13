// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66QuitConfirmationModal.h"
#include "UI/T66UIManager.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Engine/GameInstance.h"
#include "UI/Style/T66FlatStyle.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"

namespace
{
	FName QuitConfirmationTag(const TCHAR* Name)
	{
		return FName(Name);
	}

	TSharedRef<SWidget> MakeQuitConfirmationScrim(const FName Tag)
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.56f))
			.Padding(0.f),
			Tag,
			TEXT("Scrim"),
			ET66FlatState::Default);
	}

	TSharedRef<SWidget> MakeQuitConfirmationButton(
		const FText& Text,
		FOnClicked OnClicked,
		const ET66FlatState State,
		const FName Tag)
	{
		return FT66FlatStyle::MakeFlatButton(
			State,
			Text,
			MoveTemp(OnClicked),
			nullptr,
			nullptr,
			FMargin(18.f, 8.f),
			441.f,
			84.f,
			true,
			20,
			Tag);
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
	const TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
	auto AddSlot = [Canvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		Canvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X, Y, W, H))
			[
				Widget
			];
	};

	AddSlot(0.f, 0.f, 1920.f, 1080.f, MakeQuitConfirmationScrim(QuitConfirmationTag(TEXT("QuitConfirmation.Scrim"))));
	AddSlot(413.f, 352.f, 1094.f, 376.f,
		FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(0.f),
			SNullWidget::NullWidget,
			nullptr,
			QuitConfirmationTag(TEXT("QuitConfirmation.ModalPanel"))));
	AddSlot(824.f, 404.f, 272.f, 78.f,
		FT66FlatStyle::MakeFlatLabel(
			TitleText,
			ET66FlatLabelRole::Title,
			ETextJustify::Center,
			QuitConfirmationTag(TEXT("QuitConfirmation.Title"))));
	AddSlot(535.f, 508.f, 850.f, 39.f,
		FT66FlatStyle::MakeFlatLabel(
			MessageText,
			ET66FlatLabelRole::Body,
			ETextJustify::Center,
			QuitConfirmationTag(TEXT("QuitConfirmation.Message"))));
	AddSlot(505.f, 587.f, 441.f, 84.f,
		MakeQuitConfirmationButton(
			StayText,
			FOnClicked::CreateUObject(this, &UT66QuitConfirmationModal::HandleStayClicked),
			ET66FlatState::Default,
			QuitConfirmationTag(TEXT("QuitConfirmation.StayButton"))));
	AddSlot(974.f, 587.f, 441.f, 84.f,
		MakeQuitConfirmationButton(
			QuitText,
			FOnClicked::CreateUObject(this, &UT66QuitConfirmationModal::HandleQuitClicked),
			ET66FlatState::Selected,
			QuitConfirmationTag(TEXT("QuitConfirmation.QuitButton"))));

	const TSharedRef<SWidget> RootContent = SNew(SBox)
		.WidthOverride(1920.f)
		.HeightOverride(1080.f)
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
		QuitConfirmationTag(TEXT("QuitConfirmation.Root")),
		TEXT("Root"),
		ET66FlatState::Default);
}

FReply UT66QuitConfirmationModal::HandleStayClicked() { OnStayClicked(); return FReply::Handled(); }
FReply UT66QuitConfirmationModal::HandleQuitClicked() { OnQuitClicked(); return FReply::Handled(); }

void UT66QuitConfirmationModal::OnStayClicked() { CloseModal(); }
void UT66QuitConfirmationModal::OnQuitClicked() { UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false); }

