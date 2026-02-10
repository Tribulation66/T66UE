// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66ScreenBase.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Styling/SlateTypes.h"

void UT66ScreenBase::NativeConstruct()
{
	Super::NativeConstruct();
}

TSharedRef<SWidget> UT66ScreenBase::RebuildWidget()
{
	// Build our custom Slate UI
	return BuildSlateUI();
}

TSharedRef<SWidget> UT66ScreenBase::BuildSlateUI()
{
	// Default implementation - just a placeholder
	// Subclasses should override this
	return FT66Style::MakePanel(
		SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.UI", "ScreenNotImplemented", "Screen Not Implemented"))
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
			],
		FT66PanelParams(ET66PanelType::Bg).SetPadding(FT66Style::Tokens::Space4));
}

void UT66ScreenBase::OnScreenActivated_Implementation()
{
	RefreshScreen();
}

void UT66ScreenBase::OnScreenDeactivated_Implementation()
{
	// Default: nothing to clean up
}

void UT66ScreenBase::RefreshScreen_Implementation()
{
	// Default: nothing to refresh
}

void UT66ScreenBase::NavigateTo(ET66ScreenType NewScreen)
{
	if (UIManager)
	{
		UIManager->ShowScreen(NewScreen);
	}
}

void UT66ScreenBase::NavigateBack()
{
	if (UIManager)
	{
		UIManager->GoBack();
	}
}

void UT66ScreenBase::ShowModal(ET66ScreenType ModalScreen)
{
	if (UIManager)
	{
		UIManager->ShowModal(ModalScreen);
	}
}

void UT66ScreenBase::CloseModal()
{
	if (UIManager)
	{
		UIManager->CloseModal();
	}
}

void UT66ScreenBase::ForceRebuildSlate()
{
	// RemoveFromParent + AddToViewport is required because ReleaseSlateResources
	// creates a new SObjectWidget, but the viewport's Slate overlay still holds
	// a shared-ref to the OLD SObjectWidget. Simply calling TakeWidget() after
	// Release leaves the viewport displaying stale widgets.
	// RemoveFromParent detaches the old SObjectWidget from the viewport overlay,
	// and AddToViewport re-attaches with the freshly built one.
	const bool bWasInViewport = IsInViewport();
	const int32 ZOrder = bIsModal ? 100 : 0;

	if (bWasInViewport)
	{
		RemoveFromParent();
	}

	ReleaseSlateResources(true);

	// Force rebuild now so the new Slate tree (with updated data) exists before we re-add.
	TakeWidget();

	if (bWasInViewport)
	{
		AddToViewport(ZOrder);
	}
}

// ========== Slate UI Building Helpers ==========

TSharedRef<SWidget> UT66ScreenBase::MakeText(const FText& Text, int32 FontSize, const FLinearColor& Color)
{
	return SNew(STextBlock)
		.Text(Text)
		.Font(FT66Style::Tokens::FontRegular(FontSize > 0 ? FontSize : 14))
		.ColorAndOpacity(Color);
}

TSharedRef<SWidget> UT66ScreenBase::MakeTitle(const FText& Text, int32 FontSize)
{
	return SNew(STextBlock)
		.Text(Text)
		.Font(FT66Style::Tokens::FontBold(FontSize > 0 ? FontSize : 24))
		.ColorAndOpacity(FT66Style::Tokens::Text)
		.Justification(ETextJustify::Center);
}

TSharedRef<SWidget> UT66ScreenBase::WrapInSizeBox(TSharedRef<SWidget> Content, float Width, float Height)
{
	return SNew(SBox)
		.WidthOverride(Width)
		.HeightOverride(Height)
		[
			Content
		];
}

TSharedRef<SWidget> UT66ScreenBase::WrapInBorder(TSharedRef<SWidget> Content, const FLinearColor& Color)
{
	return SNew(SBorder)
		.BorderBackgroundColor(Color)
		[
			Content
		];
}

