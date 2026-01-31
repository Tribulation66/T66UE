// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66EnemyHealthBarWidget.h"

#include "Widgets/SOverlay.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/CoreStyle.h"
#include "Rendering/DrawElements.h"

namespace
{
	class ST66LockDotWidget : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66LockDotWidget) {}
			SLATE_ARGUMENT(FLinearColor, Color)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			Color = InArgs._Color;
		}

		virtual FVector2D ComputeDesiredSize(float) const override
		{
			return FVector2D(14.f, 14.f);
		}

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			const FVector2D Size = AllottedGeometry.GetLocalSize();
			const float MinDim = FMath::Max(1.f, FMath::Min(Size.X, Size.Y));
			const FVector2D Center(Size.X * 0.5f, Size.Y * 0.5f);
			const float Radius = (MinDim * 0.5f) - 0.75f;

			static constexpr int32 NumSeg = 24;
			TArray<FVector2D> Pts;
			Pts.Reserve(NumSeg + 1);
			for (int32 i = 0; i <= NumSeg; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(NumSeg);
				const float A = 2.f * PI * T;
				Pts.Add(Center + FVector2D(FMath::Cos(A) * Radius, FMath::Sin(A) * Radius));
			}

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				Pts,
				ESlateDrawEffect::None,
				Color,
				true,
				MinDim
			);

			// Outer black ring so it reads clearly.
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				Pts,
				ESlateDrawEffect::None,
				FLinearColor(0.f, 0.f, 0.f, 0.75f),
				true,
				2.f
			);

			return LayerId + 2;
		}

	private:
		FLinearColor Color = FLinearColor(1.f, 0.1f, 0.1f, 1.f);
	};
}

void UT66EnemyHealthBarWidget::SetHealthPercent(float InPct)
{
	Pct = FMath::Clamp(InPct, 0.f, 1.f);
	if (FillWidthBox)
	{
		FillWidthBox->SetWidthOverride(BarWidth * Pct);
	}
}

void UT66EnemyHealthBarWidget::SetLocked(bool bInLocked)
{
	bLocked = bInLocked;
	if (LockRingBox)
	{
		LockRingBox->SetVisibility(bLocked ? EVisibility::Visible : EVisibility::Collapsed);
	}
}

TSharedRef<SWidget> UT66EnemyHealthBarWidget::RebuildWidget()
{
	// Cache slate widget refs so we can update without ticking.
	FillWidthBox.Reset();
	LockRingBox.Reset();

	const TSharedRef<SWidget> Built =
		SNew(SBox)
		.WidthOverride(BarWidth)
		.HeightOverride(BarHeight + 14.f)
		[
			SNew(SOverlay)

			// Lock ring (red circle) above the bar.
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			[
				SAssignNew(LockRingBox, SBox)
				.WidthOverride(14.f)
				.HeightOverride(14.f)
				.Visibility(bLocked ? EVisibility::Visible : EVisibility::Collapsed)
				[
					SNew(ST66LockDotWidget)
					.Color(FLinearColor(1.f, 0.1f, 0.1f, 1.f))
				]
			]

			// Health bar.
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Bottom)
			[
				SNew(SBorder)
				.Padding(FMargin(1.f))
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.65f))
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SAssignNew(FillWidthBox, SBox)
						.WidthOverride(BarWidth * Pct)
						.HeightOverride(BarHeight)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.85f, 0.15f, 0.15f, 1.f))
						]
					]
				]
			]
		];

	RootSlate = Built;
	return Built;
}

