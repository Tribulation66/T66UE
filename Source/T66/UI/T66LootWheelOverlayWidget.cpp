// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66LootWheelOverlayWidget.h"

#include "Core/T66Rarity.h"
#include "UI/T66GameplayHUDWidget.h"
#include "UI/Style/T66FlatStyle.h"

#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Types/WidgetActiveTimerDelegate.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	static const FName LootWheelLandingMarker(TEXT("LootWheel.Landing"));
	static const FName LootWheelCommitMarker(TEXT("LootWheel.Commit"));
	static const FName LootWheelRevealMarker(TEXT("LootWheel.Reveal"));
	static constexpr float LootWheelDialDesiredSize = 500.f;
	static constexpr int32 LootWheelSegmentCount = 24;

	static FLinearColor LootWheelColorForType(const ET66LootWheelRewardVisualType Type)
	{
		switch (Type)
		{
		case ET66LootWheelRewardVisualType::Gold:
			return FLinearColor(0.98f, 0.72f, 0.16f, 1.f);
		case ET66LootWheelRewardVisualType::Item:
			return FLinearColor(0.36f, 0.78f, 1.00f, 1.f);
		case ET66LootWheelRewardVisualType::Boost:
			return FLinearColor(0.46f, 0.92f, 0.46f, 1.f);
		default:
			return FLinearColor::White;
		}
	}

	static FLinearColor LootWheelPrizeColorAtIndex(const int32 Index)
	{
		static const FLinearColor Palette[] =
		{
			FLinearColor(0.95f, 0.20f, 0.47f, 1.f),
			FLinearColor(0.16f, 0.78f, 1.00f, 1.f),
			FLinearColor(0.94f, 0.25f, 0.06f, 1.f),
			FLinearColor(0.05f, 0.68f, 0.35f, 1.f),
			FLinearColor(0.88f, 0.92f, 0.05f, 1.f),
			FLinearColor(0.96f, 0.86f, 1.00f, 1.f),
			FLinearColor(0.04f, 0.035f, 0.04f, 1.f),
			FLinearColor(0.70f, 0.42f, 0.96f, 1.f),
		};
		return Palette[FMath::Abs(Index) % UE_ARRAY_COUNT(Palette)];
	}

	static FText LootWheelLabelForType(const ET66LootWheelRewardVisualType Type)
	{
		switch (Type)
		{
		case ET66LootWheelRewardVisualType::Gold:
			return NSLOCTEXT("T66.LootWheel", "GoldSegment", "GOLD");
		case ET66LootWheelRewardVisualType::Item:
			return NSLOCTEXT("T66.LootWheel", "ItemSegment", "ITEM");
		case ET66LootWheelRewardVisualType::Boost:
			return NSLOCTEXT("T66.LootWheel", "BoostSegment", "BOOST");
		default:
			return FText::GetEmpty();
		}
	}

	static FText LootWheelBoostStatLabel(const ET66HeroStatType StatType)
	{
		switch (StatType)
		{
		case ET66HeroStatType::Damage:
			return NSLOCTEXT("T66.LootWheel", "BoostDamage", "Damage");
		case ET66HeroStatType::AttackSpeed:
			return NSLOCTEXT("T66.LootWheel", "BoostAttackSpeed", "Attack Speed");
		case ET66HeroStatType::AttackScale:
			return NSLOCTEXT("T66.LootWheel", "BoostAttackScale", "Attack Scale");
		case ET66HeroStatType::Armor:
			return NSLOCTEXT("T66.LootWheel", "BoostArmor", "Armor");
		case ET66HeroStatType::Evasion:
			return NSLOCTEXT("T66.LootWheel", "BoostEvasion", "Evasion");
		case ET66HeroStatType::Luck:
			return NSLOCTEXT("T66.LootWheel", "BoostLuck", "Luck");
		case ET66HeroStatType::Speed:
			return NSLOCTEXT("T66.LootWheel", "BoostSpeed", "Speed");
		case ET66HeroStatType::Accuracy:
			return NSLOCTEXT("T66.LootWheel", "BoostAccuracy", "Accuracy");
		default:
			return NSLOCTEXT("T66.LootWheel", "BoostFallback", "Stat");
		}
	}

	static FText LootWheelBoostStatLabel(const ET66StatType StatType)
	{
		switch (StatType)
		{
		case ET66StatType::FirePower:
			return NSLOCTEXT("T66.LootWheel", "BoostFirePower", "Fire Power");
		case ET66StatType::IcePower:
			return NSLOCTEXT("T66.LootWheel", "BoostIcePower", "Ice Power");
		case ET66StatType::ElectricityPower:
			return NSLOCTEXT("T66.LootWheel", "BoostElectricityPower", "Electricity Power");
		case ET66StatType::NaturePower:
			return NSLOCTEXT("T66.LootWheel", "BoostNaturePower", "Nature Power");
		case ET66StatType::WindPower:
			return NSLOCTEXT("T66.LootWheel", "BoostWindPower", "Wind Power");
		default:
			return NSLOCTEXT("T66.LootWheel", "BoostSecondaryFallback", "Stat");
		}
	}

	static TArray<FVector2D> MakeLootWheelCirclePoints(const FVector2D& Center, const float Radius, const int32 Steps, const float StartRadians = 0.f, const float EndRadians = PI * 2.f)
	{
		TArray<FVector2D> Points;
		const int32 SafeSteps = FMath::Max(3, Steps);
		Points.Reserve(SafeSteps + 1);
		for (int32 Index = 0; Index <= SafeSteps; ++Index)
		{
			const float Alpha = static_cast<float>(Index) / static_cast<float>(SafeSteps);
			const float Angle = FMath::Lerp(StartRadians, EndRadians, Alpha);
			Points.Add(Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius);
		}
		return Points;
	}

	static ET66LootWheelRewardVisualType LootWheelSegmentTypeAtIndex(const int32 Index)
	{
		static const ET66LootWheelRewardVisualType Pattern[] =
		{
			ET66LootWheelRewardVisualType::Gold,
			ET66LootWheelRewardVisualType::Item,
			ET66LootWheelRewardVisualType::Boost,
			ET66LootWheelRewardVisualType::Gold,
			ET66LootWheelRewardVisualType::Item,
			ET66LootWheelRewardVisualType::Gold,
			ET66LootWheelRewardVisualType::Boost,
			ET66LootWheelRewardVisualType::Item,
			ET66LootWheelRewardVisualType::Gold,
			ET66LootWheelRewardVisualType::Boost,
			ET66LootWheelRewardVisualType::Item,
			ET66LootWheelRewardVisualType::Gold,
			ET66LootWheelRewardVisualType::Gold,
			ET66LootWheelRewardVisualType::Item,
			ET66LootWheelRewardVisualType::Boost,
			ET66LootWheelRewardVisualType::Gold,
			ET66LootWheelRewardVisualType::Item,
			ET66LootWheelRewardVisualType::Gold,
			ET66LootWheelRewardVisualType::Boost,
			ET66LootWheelRewardVisualType::Item,
			ET66LootWheelRewardVisualType::Gold,
			ET66LootWheelRewardVisualType::Boost,
			ET66LootWheelRewardVisualType::Item,
			ET66LootWheelRewardVisualType::Gold,
		};
		return Pattern[FMath::Clamp(Index, 0, UE_ARRAY_COUNT(Pattern) - 1)];
	}

	class ST66LootWheelDialWidget final : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66LootWheelDialWidget) {}
			SLATE_ATTRIBUTE(int32, WinningSegmentIndex)
			SLATE_ATTRIBUTE(float, RevealAlpha)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			WinningSegmentIndex = InArgs._WinningSegmentIndex;
			RevealAlpha = InArgs._RevealAlpha;
		}

		virtual FVector2D ComputeDesiredSize(float) const override
		{
			return FVector2D(LootWheelDialDesiredSize, LootWheelDialDesiredSize);
		}

		virtual int32 OnPaint(
			const FPaintArgs& Args,
			const FGeometry& AllottedGeometry,
			const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements,
			int32 LayerId,
			const FWidgetStyle& InWidgetStyle,
			bool bParentEnabled) const override
		{
			(void)Args;
			(void)MyCullingRect;
			(void)bParentEnabled;

			const FVector2D Size = AllottedGeometry.GetLocalSize();
			const FVector2D Center = Size * 0.5f;
			const float Radius = FMath::Min(Size.X, Size.Y) * 0.41f;
			const float SegmentInnerRadius = Radius * 0.38f;
			const float SegmentOuterRadius = Radius + 24.f;
			const float SegmentBandRadius = (SegmentInnerRadius + SegmentOuterRadius) * 0.5f;
			const float SegmentBandThickness = SegmentOuterRadius - SegmentInnerRadius;
			const float MedallionOuterRadius = SegmentInnerRadius - 8.f;
			const float SegmentRadians = (PI * 2.f) / static_cast<float>(LootWheelSegmentCount);
			const float CurrentRevealAlpha = FMath::Clamp(RevealAlpha.Get(0.f), 0.f, 1.f);
			const int32 CurrentWinningSegment = WinningSegmentIndex.Get(INDEX_NONE);
			const FLinearColor OuterLine = InWidgetStyle.GetColorAndOpacityTint() * FLinearColor(0.045f, 0.043f, 0.040f, 0.98f);
			const FPaintGeometry PaintGeometry = AllottedGeometry.ToPaintGeometry();

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				PaintGeometry,
				MakeLootWheelCirclePoints(Center, Radius + 33.f, 96),
				ESlateDrawEffect::None,
				OuterLine,
				true,
				22.f);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				PaintGeometry,
				MakeLootWheelCirclePoints(Center, Radius + 45.f, 112),
				ESlateDrawEffect::None,
				FLinearColor(0.88f, 0.82f, 0.64f, 0.60f),
				true,
				2.f);

			for (int32 SegmentIndex = 0; SegmentIndex < LootWheelSegmentCount; ++SegmentIndex)
			{
				const float SegmentStart = -HALF_PI + (static_cast<float>(SegmentIndex) - 0.505f) * SegmentRadians;
				const float SegmentEnd = -HALF_PI + (static_cast<float>(SegmentIndex) + 0.505f) * SegmentRadians;
				const bool bWinner = SegmentIndex == CurrentWinningSegment && CurrentRevealAlpha > 0.f;
				FLinearColor SegmentColor = LootWheelPrizeColorAtIndex(SegmentIndex);
				if (bWinner)
				{
					SegmentColor = FMath::Lerp(SegmentColor, FLinearColor(1.f, 0.96f, 0.42f, 1.f), 0.40f * CurrentRevealAlpha);
				}
				SegmentColor.A = 0.90f;

				FSlateDrawElement::MakeLines(
					OutDrawElements,
					LayerId + 2,
					PaintGeometry,
					MakeLootWheelCirclePoints(Center, SegmentBandRadius, 5, SegmentStart, SegmentEnd),
					ESlateDrawEffect::None,
					SegmentColor,
					true,
					SegmentBandThickness);

				if (bWinner)
				{
					FSlateDrawElement::MakeLines(
						OutDrawElements,
						LayerId + 5,
						PaintGeometry,
						MakeLootWheelCirclePoints(Center, SegmentOuterRadius - 4.f, 5, SegmentStart, SegmentEnd),
						ESlateDrawEffect::None,
						FLinearColor(1.f, 0.95f, 0.42f, 0.92f),
						true,
						5.f);
				}

			}

			for (int32 StudIndex = 0; StudIndex < LootWheelSegmentCount; ++StudIndex)
			{
				const float Angle = -HALF_PI + (static_cast<float>(StudIndex) + 0.5f) * SegmentRadians;
				const FVector2D StudCenter = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * (Radius + 34.f);
				TArray<FVector2D> StudA;
				StudA.Add(StudCenter + FVector2D(-4.f, 0.f));
				StudA.Add(StudCenter + FVector2D(4.f, 0.f));
				TArray<FVector2D> StudB;
				StudB.Add(StudCenter + FVector2D(0.f, -4.f));
				StudB.Add(StudCenter + FVector2D(0.f, 4.f));
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 6, PaintGeometry, StudA, ESlateDrawEffect::None, FLinearColor(1.f, 0.92f, 0.68f, 0.90f), true, 2.f);
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 6, PaintGeometry, StudB, ESlateDrawEffect::None, FLinearColor(1.f, 0.92f, 0.68f, 0.90f), true, 2.f);
			}

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 7,
				PaintGeometry,
				MakeLootWheelCirclePoints(Center, MedallionOuterRadius - 3.f, 72),
				ESlateDrawEffect::None,
				FLinearColor(0.42f, 0.47f, 0.02f, 1.f),
				true,
				12.f);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 8,
				PaintGeometry,
				MakeLootWheelCirclePoints(Center, MedallionOuterRadius * 0.54f, 64),
				ESlateDrawEffect::None,
				FLinearColor(0.94f, 0.90f, 0.18f, 1.f),
				true,
				MedallionOuterRadius * 0.78f);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 9,
				PaintGeometry,
				MakeLootWheelCirclePoints(Center, 28.f, 48),
				ESlateDrawEffect::None,
				FLinearColor(0.86f, 0.84f, 0.05f, 1.f),
				true,
				56.f);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 10,
				PaintGeometry,
				MakeLootWheelCirclePoints(Center, MedallionOuterRadius + 3.f, 80),
				ESlateDrawEffect::None,
				FLinearColor(0.96f, 0.92f, 0.24f, 0.95f),
				true,
				3.f);

			return LayerId + 11;
		}

	private:
		TAttribute<int32> WinningSegmentIndex;
		TAttribute<float> RevealAlpha;
	};

	class ST66LootWheelFixedSelectorWidget : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66LootWheelFixedSelectorWidget) {}
		SLATE_END_ARGS()

		void Construct(const FArguments&)
		{
		}

		virtual FVector2D ComputeDesiredSize(float) const override
		{
			return FVector2D(LootWheelDialDesiredSize, LootWheelDialDesiredSize);
		}

		virtual int32 OnPaint(
			const FPaintArgs& Args,
			const FGeometry& AllottedGeometry,
			const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements,
			int32 LayerId,
			const FWidgetStyle& InWidgetStyle,
			bool bParentEnabled) const override
		{
			(void)Args;
			(void)MyCullingRect;
			(void)bParentEnabled;
			(void)InWidgetStyle;

			const FVector2D Size = AllottedGeometry.GetLocalSize();
			const FVector2D Center = Size * 0.5f;
			const float Radius = FMath::Min(Size.X, Size.Y) * 0.41f;
			const FPaintGeometry PaintGeometry = AllottedGeometry.ToPaintGeometry();
			const FVector2D SelectorOuter = Center + FVector2D(0.f, -Radius - 18.f);
			const FVector2D SelectorInner = Center + FVector2D(0.f, -Radius * 0.43f);
			const float MedallionOuterRadius = Radius * 0.38f - 8.f;

			TArray<FVector2D> ShadowLine;
			ShadowLine.Add(SelectorOuter + FVector2D(2.f, 0.f));
			ShadowLine.Add(SelectorInner + FVector2D(2.f, 0.f));
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				PaintGeometry,
				ShadowLine,
				ESlateDrawEffect::None,
				FLinearColor(0.f, 0.f, 0.f, 0.55f),
				true,
				5.f);

			TArray<FVector2D> SelectorLine;
			SelectorLine.Add(SelectorOuter);
			SelectorLine.Add(SelectorInner);
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				PaintGeometry,
				SelectorLine,
				ESlateDrawEffect::None,
				FLinearColor(0.015f, 0.012f, 0.009f, 0.96f),
				true,
				3.f);

			TArray<FVector2D> GoldAccent;
			GoldAccent.Add(SelectorOuter + FVector2D(-3.f, 0.f));
			GoldAccent.Add(SelectorInner + FVector2D(-3.f, 0.f));
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 2,
				PaintGeometry,
				GoldAccent,
				ESlateDrawEffect::None,
				FLinearColor(1.f, 0.88f, 0.28f, 0.68f),
				true,
				1.f);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 3,
				PaintGeometry,
				MakeLootWheelCirclePoints(Center, MedallionOuterRadius - 3.f, 72),
				ESlateDrawEffect::None,
				FLinearColor(0.42f, 0.47f, 0.02f, 1.f),
				true,
				12.f);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 4,
				PaintGeometry,
				MakeLootWheelCirclePoints(Center, MedallionOuterRadius * 0.54f, 64),
				ESlateDrawEffect::None,
				FLinearColor(0.94f, 0.90f, 0.18f, 1.f),
				true,
				MedallionOuterRadius * 0.78f);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 5,
				PaintGeometry,
				MakeLootWheelCirclePoints(Center, 28.f, 48),
				ESlateDrawEffect::None,
				FLinearColor(0.86f, 0.84f, 0.05f, 1.f),
				true,
				56.f);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 6,
				PaintGeometry,
				MakeLootWheelCirclePoints(Center, MedallionOuterRadius + 3.f, 80),
				ESlateDrawEffect::None,
				FLinearColor(0.96f, 0.92f, 0.24f, 0.95f),
				true,
				3.f);

			return LayerId + 7;
		}
	};
}

UT66LootWheelOverlayWidget::UT66LootWheelOverlayWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT66LootWheelOverlayWidget::SetPresentationHost(UT66GameplayHUDWidget* InPresentationHost)
{
	PresentationHost = InPresentationHost;
}

void UT66LootWheelOverlayWidget::Configure(FT66LootWheelPresentationParams InParams)
{
	Params = MoveTemp(InParams);
}

void UT66LootWheelOverlayWidget::NativeDestruct()
{
	StopAnimationActiveTimer();
	bAnimationActive = false;
	MarkerDispatcher.ClearHandlers();

	if (!bCompletionSignaled)
	{
		CommitRewardIfNeeded();
		if (UT66GameplayHUDWidget* Host = PresentationHost.Get())
		{
			Host->ClearActiveLootWheelPresentation(this);
		}
		if (Params.OnFinished)
		{
			Params.OnFinished();
			Params.OnFinished = nullptr;
		}
	}

	Super::NativeDestruct();
}

TSharedRef<SWidget> UT66LootWheelOverlayWidget::RebuildWidget()
{
	BuildSegments();
	ResetVisualState();

	const float Center = WheelDialSize * 0.5f;
	TSharedPtr<SCanvas> WheelCanvas;
	SAssignNew(WheelCanvas, SCanvas);
	WheelCanvas->AddSlot()
	.Position(FVector2D::ZeroVector)
	.Size(FVector2D(WheelDialSize, WheelDialSize))
	[
		SNew(ST66LootWheelDialWidget)
		.WinningSegmentIndex_Lambda([this]() { return WinningSegmentIndex; })
		.RevealAlpha_Lambda([this]() { return RevealAlpha; })
	];

	for (int32 SegmentIndex = 0; SegmentIndex < Segments.Num(); ++SegmentIndex)
	{
		FLootWheelSegmentEntry& Segment = Segments[SegmentIndex];
		const float AngleDegrees = -90.f + static_cast<float>(SegmentIndex) * SegmentAngleDegrees;
		const float AngleRadians = FMath::DegreesToRadians(AngleDegrees);
		const FVector2D SegmentCenter(
			Center + FMath::Cos(AngleRadians) * SegmentLabelRadius,
			Center + FMath::Sin(AngleRadians) * SegmentLabelRadius);
		const FVector2D SegmentPosition = SegmentCenter - FVector2D(SegmentLabelWidth * 0.5f, SegmentLabelHeight * 0.5f);

		WheelCanvas->AddSlot()
		.Position(SegmentPosition)
		.Size(FVector2D(SegmentLabelWidth, SegmentLabelHeight))
		[
			SNew(SBox)
			.RenderTransformPivot(FVector2D(0.5f, 0.5f))
			.RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(AngleDegrees + 90.f))))
			[
				SAssignNew(Segment.SegmentText, STextBlock)
				.Text(Segment.Label)
				.Font(FT66FlatStyle::MakeFont(8))
				.ColorAndOpacity(FLinearColor(0.03f, 0.025f, 0.02f, 1.f))
				.Justification(ETextJustify::Center)
			]
		];
	}

	TSharedRef<SWidget> Root = SNew(SOverlay)
	+ SOverlay::Slot()
	.HAlign(HAlign_Right)
	.VAlign(VAlign_Bottom)
	.Padding(FMargin(
		0.f,
		0.f,
		UT66GameplayHUDWidget::BottomRightRewardLaneRightPadding,
		UT66GameplayHUDWidget::BottomRightRewardLaneBottomPadding))
	[
		SNew(SBox)
		.WidthOverride(UT66GameplayHUDWidget::BottomRightRewardLaneWidth)
		.HeightOverride(UT66GameplayHUDWidget::BottomRightRewardLaneHeight)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::DownOnly)
			[
				SAssignNew(RootAnimationBox, SBox)
				.WidthOverride(WheelPanelSize)
				.HeightOverride(WheelPanelSize + 88.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SAssignNew(WheelRotationBox, SBox)
							.WidthOverride(WheelDialSize)
							.HeightOverride(WheelDialSize)
							.RenderTransformPivot(FVector2D(0.5f, 0.5f))
							[
								WheelCanvas.ToSharedRef()
							]
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(WheelDialSize)
							.HeightOverride(WheelDialSize)
							[
								SNew(ST66LootWheelFixedSelectorWidget)
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 10.f, 0.f, 0.f)
					.HAlign(HAlign_Center)
					[
						SAssignNew(ResultPanelBorder, SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.035f, 0.030f, 0.024f, 0.92f))
						.Padding(FMargin(18.f, 8.f))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
							[
								SAssignNew(ResultTitleText, STextBlock)
								.Text(BuildResultTitleText())
								.Font(FT66FlatStyle::MakeFont(18))
								.ColorAndOpacity(FLinearColor::White)
								.Justification(ETextJustify::Center)
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
							[
								SAssignNew(ResultDetailText, STextBlock)
								.Text(BuildResultDetailText())
								.Font(FT66FlatStyle::MakeFont(10))
								.ColorAndOpacity(FT66FlatStyle::SecondaryText())
								.Justification(ETextJustify::Center)
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 6.f, 0.f, 0.f)
					.HAlign(HAlign_Center)
					[
						SAssignNew(SkipText, STextBlock)
						.Text(NSLOCTEXT("T66.LootWheel", "SpinCaption", "Spinning"))
						.Font(FT66FlatStyle::MakeFont(8))
						.ColorAndOpacity(FT66FlatStyle::SecondaryText())
						.Justification(ETextJustify::Center)
					]
				]
			]
		]
	];

	BuildFullAnimationSequence();
	if (RootAnimationBox.IsValid())
	{
		StartAnimationActiveTimer(RootAnimationBox.ToSharedRef());
	}
	ApplyVisuals();

	return FT66FlatStyle::MakeResponsiveRoot(Root);
}

void UT66LootWheelOverlayWidget::BuildSegments()
{
	Segments.Reset();
	for (int32 SegmentIndex = 0; SegmentIndex < WheelSegmentCount; ++SegmentIndex)
	{
		FLootWheelSegmentEntry& Segment = Segments.AddDefaulted_GetRef();
		Segment.Type = LootWheelSegmentTypeAtIndex(SegmentIndex);
		Segment.Label = LootWheelLabelForType(Segment.Type);
		Segment.Color = LootWheelPrizeColorAtIndex(SegmentIndex);
	}
	SegmentAngleDegrees = Segments.Num() > 0 ? 360.f / static_cast<float>(Segments.Num()) : 15.f;
	WinningSegmentIndex = ResolveWinningSegmentIndex();
	FinalRotationDegrees = 360.f * 6.f - static_cast<float>(FMath::Max(0, WinningSegmentIndex)) * SegmentAngleDegrees;
	UE_LOG(LogTemp, Display, TEXT("[LootWheelUI] configured reward=%s winningSegment=%d finalRotation=%.1f"),
		*BuildResultTitleText().ToString(),
		WinningSegmentIndex,
		FinalRotationDegrees);
}

int32 UT66LootWheelOverlayWidget::ResolveWinningSegmentIndex() const
{
	for (int32 SegmentIndex = 0; SegmentIndex < Segments.Num(); ++SegmentIndex)
	{
		if (Segments[SegmentIndex].Type == Params.RewardType)
		{
			return SegmentIndex;
		}
	}
	return 0;
}

void UT66LootWheelOverlayWidget::ResetVisualState()
{
	WheelRotationDegrees = 0.f;
	RevealAlpha = 0.f;
	RootOpacity = 1.f;
	ActiveSequenceDuration = 0.f;
	bAnimationActive = false;
	bCompletionSignaled = false;
	bCommitAttempted = false;
	bSkipSequenceActive = false;
	ActivePhase = ELootWheelAnimationPhase::Idle;
}

FT66AnimationTimeline UT66LootWheelOverlayWidget::MakeTimeline(
	const FName TimelineName,
	const float Duration,
	const FT66AnimationCurveSpec& Curve,
	TFunction<void(float)> ProgressCallback) const
{
	FT66AnimationTimeline Timeline(TimelineName);
	Timeline.SetDuration(Duration);
	Timeline.SetCurve(Curve);
	Timeline.SetMaxMarkerDispatchesPerTick(12);
	Timeline.SetProgressCallback(MoveTemp(ProgressCallback));
	return Timeline;
}

void UT66LootWheelOverlayWidget::BuildFullAnimationSequence()
{
	RegisterMarkerHandlers();
	AnimationSequence = FT66AnimationSequence();
	bSkipSequenceActive = false;
	ActivePhase = ELootWheelAnimationPhase::Anticipation;

	const float SpinEndRotation = FMath::Max(0.f, FinalRotationDegrees - 720.f);
	TWeakObjectPtr<UT66LootWheelOverlayWidget> WeakThis(this);

	FT66AnimationTimeline Anticipation = MakeTimeline(
		FName(TEXT("LootWheel.Anticipation")),
		AnticipationDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::Anticipation, 0.75f),
		[WeakThis](float CurveValue)
		{
			if (UT66LootWheelOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ELootWheelAnimationPhase::Anticipation;
				Self->ApplyWheelRotation(FMath::Sin(CurveValue * PI * 2.f) * 3.5f);
			}
		});
	AnimationSequence.AddTimeline(MoveTemp(Anticipation));

	FT66AnimationTimeline Spin = MakeTimeline(
		FName(TEXT("LootWheel.Spin")),
		SpinDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
		[WeakThis, SpinEndRotation](float CurveValue)
		{
			if (UT66LootWheelOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ELootWheelAnimationPhase::Spin;
				Self->ApplyWheelRotation(FMath::Lerp(0.f, SpinEndRotation, FMath::Clamp(CurveValue, 0.f, 1.f)));
			}
		});
	AnimationSequence.AddTimeline(MoveTemp(Spin));

	FT66AnimationTimeline Deceleration = MakeTimeline(
		FName(TEXT("LootWheel.Deceleration")),
		DecelerationDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::EaseOutCubic),
		[WeakThis, SpinEndRotation](float CurveValue)
		{
			if (UT66LootWheelOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ELootWheelAnimationPhase::Deceleration;
				const float Clamped = FMath::Clamp(CurveValue, 0.f, 1.f);
				Self->ApplyWheelRotation(FMath::Lerp(SpinEndRotation, Self->FinalRotationDegrees, Clamped));
			}
		});
	Deceleration.AddMarker({ LootWheelLandingMarker, ET66AnimationMarkerType::ProgressBased, 1.f, 0.f, ET66AnimationMarkerFirePolicy::Once, LootWheelLandingMarker });
	Deceleration.AddMarker({ LootWheelCommitMarker, ET66AnimationMarkerType::ProgressBased, 1.f, 0.f, ET66AnimationMarkerFirePolicy::Once, LootWheelCommitMarker });
	AnimationSequence.AddTimeline(MoveTemp(Deceleration));

	FT66AnimationTimeline Reveal = MakeTimeline(
		FName(TEXT("LootWheel.Reveal")),
		RevealDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::EaseOutCubic),
		[WeakThis](float CurveValue)
		{
			if (UT66LootWheelOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ELootWheelAnimationPhase::Reveal;
				Self->RevealAlpha = FMath::Clamp(CurveValue, 0.f, 1.f);
				Self->ApplyWheelRotation(Self->FinalRotationDegrees);
			}
		});
	Reveal.AddMarker({ LootWheelRevealMarker, ET66AnimationMarkerType::ProgressBased, 0.05f, 0.f, ET66AnimationMarkerFirePolicy::Once, LootWheelRevealMarker });
	AnimationSequence.AddTimeline(MoveTemp(Reveal));

	FT66AnimationTimeline Hold = MakeTimeline(
		FName(TEXT("LootWheel.Hold")),
		HoldDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
		[WeakThis](float)
		{
			if (UT66LootWheelOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ELootWheelAnimationPhase::Hold;
				Self->RevealAlpha = 1.f;
				Self->ApplyWheelRotation(Self->FinalRotationDegrees);
			}
		});
	AnimationSequence.AddTimeline(MoveTemp(Hold));

	FT66AnimationTimeline Dismiss = MakeTimeline(
		FName(TEXT("LootWheel.Dismiss")),
		DismissDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::EaseInCubic),
		[WeakThis](float CurveValue)
		{
			if (UT66LootWheelOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ELootWheelAnimationPhase::Dismiss;
				const float Clamped = FMath::Clamp(CurveValue, 0.f, 1.f);
				Self->RootOpacity = 1.f - Clamped;
				Self->RevealAlpha = 1.f - Clamped;
			}
		});
	AnimationSequence.AddTimeline(MoveTemp(Dismiss));

	ActiveSequenceDuration = AnimationSequence.GetDuration();
	bAnimationActive = true;
	AnimationSequence.Play();
}

void UT66LootWheelOverlayWidget::BuildSkipToLandingSequence()
{
	RegisterMarkerHandlers();
	AnimationSequence = FT66AnimationSequence();
	bSkipSequenceActive = true;

	const float StartRotation = WheelRotationDegrees;
	TWeakObjectPtr<UT66LootWheelOverlayWidget> WeakThis(this);
	FT66AnimationTimeline Skip = MakeTimeline(
		FName(TEXT("LootWheel.SkipLanding")),
		SkipLandingDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::EaseOutCubic),
		[WeakThis, StartRotation](float CurveValue)
		{
			if (UT66LootWheelOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ELootWheelAnimationPhase::Deceleration;
				const float Clamped = FMath::Clamp(CurveValue, 0.f, 1.f);
				Self->ApplyWheelRotation(FMath::Lerp(StartRotation, Self->FinalRotationDegrees, Clamped));
			}
		});
	Skip.AddMarker({ LootWheelLandingMarker, ET66AnimationMarkerType::ProgressBased, 1.f, 0.f, ET66AnimationMarkerFirePolicy::Once, LootWheelLandingMarker });
	Skip.AddMarker({ LootWheelCommitMarker, ET66AnimationMarkerType::ProgressBased, 1.f, 0.f, ET66AnimationMarkerFirePolicy::Once, LootWheelCommitMarker });
	AnimationSequence.AddTimeline(MoveTemp(Skip));

	FT66AnimationTimeline Reveal = MakeTimeline(
		FName(TEXT("LootWheel.SkipReveal")),
		0.28f,
		FT66AnimationCurveSpec(ET66AnimationCurve::EaseOutCubic),
		[WeakThis](float CurveValue)
		{
			if (UT66LootWheelOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ELootWheelAnimationPhase::Reveal;
				Self->RevealAlpha = FMath::Clamp(CurveValue, 0.f, 1.f);
				Self->ApplyWheelRotation(Self->FinalRotationDegrees);
			}
		});
	Reveal.AddMarker({ LootWheelRevealMarker, ET66AnimationMarkerType::ProgressBased, 0.05f, 0.f, ET66AnimationMarkerFirePolicy::Once, LootWheelRevealMarker });
	AnimationSequence.AddTimeline(MoveTemp(Reveal));

	FT66AnimationTimeline Hold = MakeTimeline(
		FName(TEXT("LootWheel.SkipHold")),
		0.25f,
		FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
		[WeakThis](float)
		{
			if (UT66LootWheelOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ELootWheelAnimationPhase::Hold;
				Self->RevealAlpha = 1.f;
			}
		});
	AnimationSequence.AddTimeline(MoveTemp(Hold));

	BuildDismissSequence();
}

void UT66LootWheelOverlayWidget::BuildDismissSequence()
{
	if (!bSkipSequenceActive)
	{
		RegisterMarkerHandlers();
		AnimationSequence = FT66AnimationSequence();
	}

	TWeakObjectPtr<UT66LootWheelOverlayWidget> WeakThis(this);
	FT66AnimationTimeline Dismiss = MakeTimeline(
		FName(TEXT("LootWheel.DismissOnly")),
		DismissDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::EaseInCubic),
		[WeakThis](float CurveValue)
		{
			if (UT66LootWheelOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ELootWheelAnimationPhase::Dismiss;
				const float Clamped = FMath::Clamp(CurveValue, 0.f, 1.f);
				Self->RootOpacity = 1.f - Clamped;
				Self->RevealAlpha = 1.f - Clamped;
			}
		});
	AnimationSequence.AddTimeline(MoveTemp(Dismiss));

	ActiveSequenceDuration = AnimationSequence.GetDuration();
	bAnimationActive = true;
	AnimationSequence.Play();
}

void UT66LootWheelOverlayWidget::RegisterMarkerHandlers()
{
	MarkerDispatcher.ClearHandlers();
	TWeakObjectPtr<UT66LootWheelOverlayWidget> WeakThis(this);
	MarkerDispatcher.RegisterHandler(LootWheelLandingMarker, [WeakThis](const FT66AnimationMarkerEvent&)
	{
		if (UT66LootWheelOverlayWidget* Self = WeakThis.Get())
		{
			Self->ApplyWheelRotation(Self->FinalRotationDegrees);
			UE_LOG(LogTemp, Display, TEXT("[LootWheelUI] landed reward=%s segment=%d rotation=%.1f"),
				*Self->BuildResultTitleText().ToString(),
				Self->WinningSegmentIndex,
				Self->FinalRotationDegrees);
		}
	});
	MarkerDispatcher.RegisterHandler(LootWheelCommitMarker, [WeakThis](const FT66AnimationMarkerEvent&)
	{
		if (UT66LootWheelOverlayWidget* Self = WeakThis.Get())
		{
			Self->CommitRewardIfNeeded();
		}
	});
	MarkerDispatcher.RegisterHandler(LootWheelRevealMarker, [WeakThis](const FT66AnimationMarkerEvent&)
	{
		if (UT66LootWheelOverlayWidget* Self = WeakThis.Get())
		{
			Self->RevealAlpha = FMath::Max(Self->RevealAlpha, 0.35f);
		}
	});
}

void UT66LootWheelOverlayWidget::HandleMarkerEvents(const TArray<FT66AnimationMarkerEvent>& MarkerEvents)
{
	if (MarkerEvents.Num() > 0)
	{
		MarkerDispatcher.Dispatch(MarkerEvents);
	}
}

void UT66LootWheelOverlayWidget::StartAnimationActiveTimer(const TSharedRef<SWidget>& OwningWidget)
{
	StopAnimationActiveTimer();
	AnimationActiveTimerWidget = OwningWidget;
	AnimationActiveTimerHandle = OwningWidget->RegisterActiveTimer(
		0.f,
		FWidgetActiveTimerDelegate::CreateUObject(this, &UT66LootWheelOverlayWidget::HandleAnimationActiveTimer));
}

void UT66LootWheelOverlayWidget::StopAnimationActiveTimer()
{
	TSharedPtr<SWidget> Widget = AnimationActiveTimerWidget.Pin();
	if (Widget.IsValid() && AnimationActiveTimerHandle.IsValid())
	{
		Widget->UnRegisterActiveTimer(AnimationActiveTimerHandle.ToSharedRef());
	}

	AnimationActiveTimerHandle.Reset();
	AnimationActiveTimerWidget.Reset();
}

EActiveTimerReturnType UT66LootWheelOverlayWidget::HandleAnimationActiveTimer(double CurrentTime, const float DeltaTime)
{
	(void)CurrentTime;
	if (!bAnimationActive || !AnimationActiveTimerWidget.IsValid())
	{
		StopAnimationActiveTimer();
		return EActiveTimerReturnType::Stop;
	}

	TickAnimation(DeltaTime);
	if (TSharedPtr<SWidget> Widget = AnimationActiveTimerWidget.Pin())
	{
		Widget->Invalidate(EInvalidateWidgetReason::PaintAndVolatility);
	}

	return bAnimationActive ? EActiveTimerReturnType::Continue : EActiveTimerReturnType::Stop;
}

void UT66LootWheelOverlayWidget::TickAnimation(const float DeltaSeconds)
{
	if (!bAnimationActive)
	{
		return;
	}

	TArray<FT66AnimationMarkerEvent> MarkerEvents;
	AnimationSequence.Tick(FMath::Clamp(DeltaSeconds, 0.f, 0.05f), MarkerEvents);
	HandleMarkerEvents(MarkerEvents);
	ApplyVisuals();

	if (IsAnimationTerminal())
	{
		bAnimationActive = false;
		ActivePhase = ELootWheelAnimationPhase::Complete;
		SignalAnimationComplete();
	}
}

void UT66LootWheelOverlayWidget::ApplyWheelRotation(const float NewRotationDegrees)
{
	WheelRotationDegrees = NewRotationDegrees;
	if (WheelRotationBox.IsValid())
	{
		WheelRotationBox->SetRenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(WheelRotationDegrees))));
	}
}

void UT66LootWheelOverlayWidget::ApplyVisuals()
{
	if (RootAnimationBox.IsValid())
	{
		RootAnimationBox->SetRenderOpacity(FMath::Clamp(RootOpacity, 0.f, 1.f));
	}
	if (ResultPanelBorder.IsValid())
	{
		ResultPanelBorder->SetRenderOpacity(FMath::Clamp(RevealAlpha, 0.f, 1.f));
	}
	for (int32 SegmentIndex = 0; SegmentIndex < Segments.Num(); ++SegmentIndex)
	{
		FLootWheelSegmentEntry& Segment = Segments[SegmentIndex];
		const bool bWinner = SegmentIndex == WinningSegmentIndex && RevealAlpha > 0.f;
		if (Segment.SegmentBorder.IsValid())
		{
			FLinearColor Color = Segment.Color * (bWinner ? 0.82f : 0.55f) + FLinearColor(0.03f, 0.025f, 0.02f, bWinner ? 0.45f : 0.70f);
			Color.A = 1.f;
			Segment.SegmentBorder->SetBorderBackgroundColor(Color);
			Segment.SegmentBorder->SetRenderOpacity(bWinner ? 1.f : 0.92f);
		}
		if (Segment.SegmentText.IsValid())
		{
			const float SegmentLuminance = Segment.Color.R * 0.30f + Segment.Color.G * 0.59f + Segment.Color.B * 0.11f;
			const FLinearColor BaseTextColor = SegmentLuminance < 0.22f
				? FLinearColor(0.96f, 0.94f, 0.90f, 0.98f)
				: FLinearColor(0.02f, 0.018f, 0.014f, 0.98f);
			Segment.SegmentText->SetColorAndOpacity(bWinner
				? FLinearColor(1.f, 0.98f, 0.50f, 1.f)
				: BaseTextColor);
		}
	}
	if (SkipText.IsValid())
	{
		const FText Caption = ActivePhase == ELootWheelAnimationPhase::Reveal || ActivePhase == ELootWheelAnimationPhase::Hold
			? NSLOCTEXT("T66.LootWheel", "LandingCaption", "Locked")
			: NSLOCTEXT("T66.LootWheel", "SpinningCaption", "Spinning");
		SkipText->SetText(Caption);
	}
}

void UT66LootWheelOverlayWidget::CommitRewardIfNeeded()
{
	if (bCommitAttempted)
	{
		return;
	}

	bCommitAttempted = true;
	if (Params.OnLandingCommit)
	{
		Params.OnLandingCommit();
		Params.OnLandingCommit = nullptr;
	}
}

void UT66LootWheelOverlayWidget::SignalAnimationComplete()
{
	if (bCompletionSignaled)
	{
		return;
	}

	CommitRewardIfNeeded();
	bCompletionSignaled = true;

	if (UT66GameplayHUDWidget* Host = PresentationHost.Get())
	{
		Host->ClearActiveLootWheelPresentation(this);
	}

	if (Params.OnFinished)
	{
		Params.OnFinished();
		Params.OnFinished = nullptr;
	}

	RemoveFromParent();
}

bool UT66LootWheelOverlayWidget::IsAnimationTerminal() const
{
	return T66Animation::IsTerminalState(AnimationSequence.GetState());
}

void UT66LootWheelOverlayWidget::RequestSkip()
{
	if (bCompletionSignaled)
	{
		return;
	}

	switch (ActivePhase)
	{
	case ELootWheelAnimationPhase::Reveal:
	case ELootWheelAnimationPhase::Hold:
		CommitRewardIfNeeded();
		bSkipSequenceActive = false;
		BuildDismissSequence();
		break;
	case ELootWheelAnimationPhase::Dismiss:
		AnimationSequence.RequestSkip();
		break;
	case ELootWheelAnimationPhase::Complete:
		break;
	case ELootWheelAnimationPhase::Idle:
	case ELootWheelAnimationPhase::Anticipation:
	case ELootWheelAnimationPhase::Spin:
	case ELootWheelAnimationPhase::Deceleration:
	default:
		BuildSkipToLandingSequence();
		break;
	}
}

FText UT66LootWheelOverlayWidget::BuildResultTitleText() const
{
	return LootWheelLabelForType(Params.RewardType);
}

FText UT66LootWheelOverlayWidget::BuildResultDetailText() const
{
	switch (Params.RewardType)
	{
	case ET66LootWheelRewardVisualType::Gold:
		return FText::Format(
			NSLOCTEXT("T66.LootWheel", "GoldDetail", "+{0} gold"),
			FText::AsNumber(FMath::Max(0, Params.Gold)));
	case ET66LootWheelRewardVisualType::Item:
		return Params.ItemID.IsNone()
			? NSLOCTEXT("T66.LootWheel", "ItemDetailFallback", "Inventory item")
			: FText::FromName(Params.ItemID);
	case ET66LootWheelRewardVisualType::Boost:
		return FText::Format(
			NSLOCTEXT("T66.LootWheel", "BoostDetail", "+{0} {1} for {2}s"),
			FText::AsNumber(FMath::Max(0, Params.BoostBonusStatPoints)),
			Params.bBoostUsesStat ? LootWheelBoostStatLabel(Params.BoostStatType) : LootWheelBoostStatLabel(Params.BoostStatType),
			FText::AsNumber(FMath::RoundToInt(FMath::Max(0.f, Params.BoostDurationSeconds))));
	default:
		return FText::GetEmpty();
	}
}
