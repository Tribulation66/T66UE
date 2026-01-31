// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66ScreenBase.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"
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
	return SNew(SBorder)
		.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Bg"))
		.Padding(FT66Style::Tokens::Space4)
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.UI", "ScreenNotImplemented", "Screen Not Implemented"))
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
			]
		];
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
	// Tear down any existing Slate resources so the next TakeWidget() rebuilds the tree.
	ReleaseSlateResources(true);

	// Ensure layout/paint invalidation (helps the new tree apply immediately).
	InvalidateLayoutAndVolatility();

	// Rebuild now (or at least ensure the new tree exists).
	TakeWidget();
}

// ========== Slate UI Building Helpers ==========

template<typename T>
TSharedRef<SWidget> UT66ScreenBase::MakeButton(const FText& Text, T* Object, FReply (T::*Func)(), const FLinearColor& BgColor)
{
	// For UObject-based classes, we need to use CreateUObject for the delegate
	return SNew(SButton)
		.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
		.ButtonColorAndOpacity(BgColor)
		.ContentPadding(FMargin(18.0f, 10.0f))
		.OnClicked(FOnClicked::CreateUObject(Object, Func))
		[
			SNew(STextBlock)
			.Text(Text)
			.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
			.Justification(ETextJustify::Center)
		];
}

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

// Explicit template instantiations for the button helper
// Add more as needed for each screen class
template TSharedRef<SWidget> UT66ScreenBase::MakeButton<UT66ScreenBase>(const FText&, UT66ScreenBase*, FReply (UT66ScreenBase::*)(), const FLinearColor&);
