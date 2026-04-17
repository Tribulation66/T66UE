// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66FrontendBackButtonWidget.h"

#include "Core/T66LocalizationSubsystem.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"

#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"

namespace
{
	constexpr int32 GFrontendBackButtonViewportZOrder = 40;
}

UT66FrontendBackButtonWidget::UT66FrontendBackButtonWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::None;
	bIsModal = false;
}

TSharedRef<SWidget> UT66FrontendBackButtonWidget::RebuildWidget()
{
	// This overlay positions itself directly against the live viewport and safe frame.
	return BuildSlateUI();
}

UT66LocalizationSubsystem* UT66FrontendBackButtonWidget::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	return nullptr;
}

TSharedRef<SWidget> UT66FrontendBackButtonWidget::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const float TopPadding = (UIManager ? UIManager->GetFrontendTopBarReservedHeight() : 0.f) + 12.f;
	const FMargin BackButtonPadding = FT66Style::GetSafePadding(FMargin(24.f, TopPadding, 0.f, 0.f));

	return SNew(SOverlay)
		.Visibility(EVisibility::SelfHitTestInvisible)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(BackButtonPadding)
		[
			SNew(SBox)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(
						BackText,
						FOnClicked::CreateUObject(this, &UT66FrontendBackButtonWidget::HandleBackClicked),
						ET66ButtonType::Neutral)
					.SetBorderVisual(ET66ButtonBorderVisual::None)
					.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
					.SetUseGlow(false)
					.SetMinWidth(120.f)
					.SetHeight(38.f)
					.SetFontSize(18)
					.SetColor(FT66Style::ButtonNeutral()))
			]
		];
}

void UT66FrontendBackButtonWidget::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	FT66Style::DeferRebuild(this, GFrontendBackButtonViewportZOrder);
}

FReply UT66FrontendBackButtonWidget::HandleBackClicked()
{
	if (!UIManager)
	{
		return FReply::Handled();
	}

	if (UIManager->CanGoBack())
	{
		UIManager->GoBack();
	}
	else if (UIManager->GetCurrentScreenType() != ET66ScreenType::MainMenu)
	{
		UIManager->ShowScreenWithoutHistory(ET66ScreenType::MainMenu);
	}

	return FReply::Handled();
}
