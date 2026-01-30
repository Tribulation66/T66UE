// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66EnemyHealthBarWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"

void UT66EnemyHealthBarWidget::SetHealthPercent(float InPct)
{
	Pct = FMath::Clamp(InPct, 0.f, 1.f);
	if (Bar)
	{
		Bar->SetPercent(Pct);
	}
}

void UT66EnemyHealthBarWidget::SetLocked(bool bInLocked)
{
	bLocked = bInLocked;
	if (LockDot)
	{
		LockDot->SetVisibility(bLocked ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UT66EnemyHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Build a tiny widget tree programmatically so it works reliably inside WidgetComponent.
	if (!WidgetTree) return;

	if (!RootSizeBox)
	{
		RootSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RootSizeBox"));
		RootSizeBox->SetWidthOverride(BarWidth);
		RootSizeBox->SetHeightOverride(BarHeight + 10.f); // space for lock dot
		WidgetTree->RootWidget = RootSizeBox;
	}

	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Overlay"));
	RootSizeBox->AddChild(Overlay);

	if (!Bar && Overlay)
	{
		Bar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
		Bar->SetPercent(Pct);
		Bar->SetFillColorAndOpacity(FLinearColor(0.85f, 0.15f, 0.15f, 1.f));
		UOverlaySlot* BarSlot = Overlay->AddChildToOverlay(Bar);
		if (BarSlot)
		{
			BarSlot->SetHorizontalAlignment(HAlign_Fill);
			BarSlot->SetVerticalAlignment(VAlign_Bottom);
		}
	}

	if (!LockDot && Overlay)
	{
		LockDot = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LockDot"));
		LockDot->SetBrushColor(FLinearColor(1.f, 0.1f, 0.1f, 1.f));
		LockDot->SetVisibility(bLocked ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

		USizeBox* DotSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("DotSize"));
		DotSize->SetWidthOverride(8.f);
		DotSize->SetHeightOverride(8.f);
		DotSize->AddChild(LockDot);

		UOverlaySlot* DotSlot = Overlay->AddChildToOverlay(DotSize);
		if (DotSlot)
		{
			DotSlot->SetHorizontalAlignment(HAlign_Center);
			DotSlot->SetVerticalAlignment(VAlign_Top);
		}
	}
}

