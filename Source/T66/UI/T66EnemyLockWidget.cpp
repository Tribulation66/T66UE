// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66EnemyLockWidget.h"

#include "Rendering/DrawElements.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Layout/SBox.h"

namespace
{
	class ST66EnemyBullseyeWidget : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66EnemyBullseyeWidget) {}
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
		}

		virtual FVector2D ComputeDesiredSize(float) const override
		{
			return FVector2D(44.f, 44.f);
		}

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			const FVector2D Size = AllottedGeometry.GetLocalSize();
			const FVector2D Center(Size.X * 0.5f, Size.Y * 0.5f);

			auto DrawRing = [&](const float Radius, const float Thickness, const FLinearColor& Color, const int32 DrawLayer)
			{
				static constexpr int32 NumSeg = 28;
				TArray<FVector2D> Points;
				Points.Reserve(NumSeg + 1);
				for (int32 SegmentIndex = 0; SegmentIndex <= NumSeg; ++SegmentIndex)
				{
					const float T = static_cast<float>(SegmentIndex) / static_cast<float>(NumSeg);
					const float Angle = 2.f * PI * T;
					Points.Add(Center + FVector2D(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius));
				}

				FSlateDrawElement::MakeLines(
					OutDrawElements,
					DrawLayer,
					AllottedGeometry.ToPaintGeometry(),
					Points,
					ESlateDrawEffect::None,
					Color,
					true,
					Thickness);
			};

			const float TimeSeconds = static_cast<float>(FPlatformTime::Seconds());
			const float Pulse = 1.f + (0.05f * (0.5f + (0.5f * FMath::Sin(TimeSeconds * 7.f))));

			DrawRing(15.5f * Pulse, 6.f, FLinearColor(0.f, 0.f, 0.f, 0.38f), LayerId);
			DrawRing(15.f * Pulse, 5.f, FLinearColor(0.84f, 0.08f, 0.08f, 0.98f), LayerId + 1);
			DrawRing(10.f, 4.5f, FLinearColor(0.98f, 0.95f, 0.92f, 0.98f), LayerId + 2);
			DrawRing(5.5f, 4.f, FLinearColor(0.84f, 0.08f, 0.08f, 0.98f), LayerId + 3);

			auto DrawLine = [&](const FVector2D& Start, const FVector2D& End, const FLinearColor& Color, const float Thickness, const int32 DrawLayer)
			{
				TArray<FVector2D> Points;
				Points.Add(Start);
				Points.Add(End);
				FSlateDrawElement::MakeLines(
					OutDrawElements,
					DrawLayer,
					AllottedGeometry.ToPaintGeometry(),
					Points,
					ESlateDrawEffect::None,
					Color,
					false,
					Thickness);
			};

			DrawLine(Center + FVector2D(-18.f, 0.f), Center + FVector2D(-10.f, 0.f), FLinearColor(1.f, 0.95f, 0.92f, 0.92f), 1.8f, LayerId + 4);
			DrawLine(Center + FVector2D(10.f, 0.f), Center + FVector2D(18.f, 0.f), FLinearColor(1.f, 0.95f, 0.92f, 0.92f), 1.8f, LayerId + 4);
			DrawLine(Center + FVector2D(0.f, -18.f), Center + FVector2D(0.f, -10.f), FLinearColor(1.f, 0.95f, 0.92f, 0.92f), 1.8f, LayerId + 4);
			DrawLine(Center + FVector2D(0.f, 10.f), Center + FVector2D(0.f, 18.f), FLinearColor(1.f, 0.95f, 0.92f, 0.92f), 1.8f, LayerId + 4);

			DrawLine(Center + FVector2D(-3.2f, 0.f), Center + FVector2D(3.2f, 0.f), FLinearColor(1.f, 1.f, 1.f, 1.f), 2.f, LayerId + 5);
			DrawLine(Center + FVector2D(0.f, -3.2f), Center + FVector2D(0.f, 3.2f), FLinearColor(1.f, 1.f, 1.f, 1.f), 2.f, LayerId + 5);
			DrawRing(1.7f, 4.f, FLinearColor(1.f, 1.f, 1.f, 1.f), LayerId + 6);

			return LayerId + 7;
		}
	};
}

TSharedRef<SWidget> UT66EnemyLockWidget::RebuildWidget()
{
	return SNew(SBox)
		.WidthOverride(44.f)
		.HeightOverride(44.f)
		[
			SNew(ST66EnemyBullseyeWidget)
		];
}
