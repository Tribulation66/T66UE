// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66FrontendTopBarWidget.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "UI/ST66PulsingIcon.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66ReferenceLayout.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"

#include "Engine/Texture2D.h"
#include "HAL/FileManager.h"
#include "Misc/Parse.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66FrontendTopBar, Log, All);
#include "Kismet/GameplayStatics.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/GarbageCollection.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr int32 GTopBarViewportZOrder = 50;
	TMap<FString, TStrongObjectPtr<UTexture2D>> GTopBarFileTextureCache;

	float SnapPixel(float Value)
	{
		return FMath::RoundToFloat(Value);
	}

	FT66ReferenceTransform GetTopBarViewportTransform()
	{
		const FVector2D SafeFrameLogicalSize = FT66Style::GetSafeFrameSize();
		const FVector2D SafeViewportSize = SafeFrameLogicalSize.IsNearlyZero()
			? FVector2D(T66MainMenuReferenceLayout::CanvasWidth, T66MainMenuReferenceLayout::CanvasHeight)
			: SafeFrameLogicalSize;
		return FT66ReferenceTransform(
			FVector2D(T66MainMenuReferenceLayout::CanvasWidth, T66MainMenuReferenceLayout::CanvasHeight),
			SafeViewportSize);
	}

	float GetTopBarReferenceReservedHeight()
	{
		float MaxBottom = 0.0f;
		auto IncludeRect = [&MaxBottom](const FT66ReferenceRect& Rect)
		{
			MaxBottom = FMath::Max(MaxBottom, Rect.Bottom());
		};

		IncludeRect(T66MainMenuReferenceLayout::TopBar::TopbarStripFull);
		IncludeRect(T66MainMenuReferenceLayout::TopBar::ButtonSettings);
		IncludeRect(T66MainMenuReferenceLayout::TopBar::ButtonChat);
		IncludeRect(T66MainMenuReferenceLayout::TopBar::TabAccount);
		IncludeRect(T66MainMenuReferenceLayout::TopBar::BadgeProfile);
		IncludeRect(T66MainMenuReferenceLayout::TopBar::TabPowerUp);
		IncludeRect(T66MainMenuReferenceLayout::TopBar::TabAchievements);
		IncludeRect(T66MainMenuReferenceLayout::TopBar::TabMinigames);
		IncludeRect(T66MainMenuReferenceLayout::TopBar::CurrencySlot);
		IncludeRect(T66MainMenuReferenceLayout::TopBar::ButtonPower);

		return FMath::Max(
			T66MainMenuReferenceLayout::TopBarReservedHeight,
			FMath::CeilToFloat(MaxBottom));
	}

	float GetTopBarReferenceSurfaceHeight()
	{
		return FMath::Max(
			T66MainMenuReferenceLayout::TopBarSurfaceHeight,
			GetTopBarReferenceReservedHeight());
	}

	bool IsBloodyRetroTopBarPreset()
	{
		return T66ScreenSlateHelpers::GetReferenceChromePreset() == T66ScreenSlateHelpers::ET66ReferenceChromePreset::BloodyRetro;
	}

	FLinearColor GetTopBarSurfaceFill()
	{
		return FLinearColor(0.f, 0.f, 0.f, 1.0f);
	}

	FLinearColor GetTopBarFrameAccent()
	{
		return IsBloodyRetroTopBarPreset()
			? FLinearColor(0.64f, 0.035f, 0.030f, 1.0f)
			: FLinearColor(0.82f, 0.10f, 1.0f, 1.0f);
	}

	TArray<FVector2D> MakeCirclePoints(const FVector2D& Center, float Radius, int32 Segments, float StartAngle = 0.f, float EndAngle = 2.f * PI)
	{
		TArray<FVector2D> Points;
		Points.Reserve(Segments + 1);
		for (int32 Index = 0; Index <= Segments; ++Index)
		{
			const float Alpha = static_cast<float>(Index) / static_cast<float>(Segments);
			const float Angle = FMath::Lerp(StartAngle, EndAngle, Alpha);
			Points.Add(Center + FVector2D(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius));
		}
		return Points;
	}

	class ST66TopBarGearGlyph : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66TopBarGearGlyph) {}
			SLATE_ARGUMENT(FVector2D, DesiredSize)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			DesiredSize = InArgs._DesiredSize.IsNearlyZero() ? FVector2D(48.f, 48.f) : InArgs._DesiredSize;
		}

		virtual FVector2D ComputeDesiredSize(float) const override
		{
			return DesiredSize;
		}

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			const FVector2D Size = AllottedGeometry.GetLocalSize();
			const FVector2D Center(Size * 0.5f);
			const float MinDim = FMath::Max(1.f, FMath::Min(Size.X, Size.Y));
			const float ToothStart = MinDim * 0.34f;
			const float ToothEnd = MinDim * 0.47f;
			const float ToothThickness = FMath::Max(3.f, MinDim * 0.10f);
			const float OuterRadius = MinDim * 0.27f;
			const float InnerRadius = MinDim * 0.17f;
			const float CoreRadius = MinDim * 0.08f;

			for (int32 AngleIndex = 0; AngleIndex < 8; ++AngleIndex)
			{
				const float Angle = FMath::DegreesToRadians(static_cast<float>(AngleIndex) * 45.f);
				const FVector2D Dir(FMath::Cos(Angle), FMath::Sin(Angle));
				const TArray<FVector2D> ToothLine = {
					Center + (Dir * ToothStart),
					Center + (Dir * ToothEnd)
				};
				FSlateDrawElement::MakeLines(
					OutDrawElements,
					LayerId,
					AllottedGeometry.ToPaintGeometry(),
					ToothLine,
					ESlateDrawEffect::None,
					FLinearColor(0.40f, 0.45f, 0.51f, 1.f),
					true,
					ToothThickness);
			}

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				MakeCirclePoints(Center, OuterRadius, 40),
				ESlateDrawEffect::None,
				FLinearColor(0.86f, 0.89f, 0.93f, 1.f),
				true,
				FMath::Max(4.f, MinDim * 0.11f));

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 2,
				AllottedGeometry.ToPaintGeometry(),
				MakeCirclePoints(Center, InnerRadius, 32),
				ESlateDrawEffect::None,
				FLinearColor(0.19f, 0.22f, 0.27f, 1.f),
				true,
				FMath::Max(5.f, MinDim * 0.16f));

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 3,
				AllottedGeometry.ToPaintGeometry(),
				MakeCirclePoints(Center, InnerRadius, 32),
				ESlateDrawEffect::None,
				FLinearColor(0.86f, 0.89f, 0.93f, 1.f),
				true,
				FMath::Max(2.f, MinDim * 0.05f));

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 4,
				AllottedGeometry.ToPaintGeometry(),
				MakeCirclePoints(Center, CoreRadius, 24),
				ESlateDrawEffect::None,
				FLinearColor(0.17f, 0.20f, 0.24f, 1.f),
				true,
				CoreRadius * 2.f);

			return LayerId + 5;
		}

	private:
		FVector2D DesiredSize = FVector2D(48.f, 48.f);
	};

	class ST66TopBarPowerGlyph : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66TopBarPowerGlyph) {}
			SLATE_ARGUMENT(FVector2D, DesiredSize)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			DesiredSize = InArgs._DesiredSize.IsNearlyZero() ? FVector2D(48.f, 48.f) : InArgs._DesiredSize;
		}

		virtual FVector2D ComputeDesiredSize(float) const override
		{
			return DesiredSize;
		}

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			const FVector2D Size = AllottedGeometry.GetLocalSize();
			const FVector2D Center(Size * 0.5f);
			const float MinDim = FMath::Max(1.f, FMath::Min(Size.X, Size.Y));
			const float Radius = MinDim * 0.36f;
			const float OuterThickness = FMath::Max(5.f, MinDim * 0.12f);
			const float InnerThickness = FMath::Max(2.f, MinDim * 0.05f);
			const float StartAngle = -0.35f * PI;
			const float EndAngle = 1.35f * PI;
			const TArray<FVector2D> ArcPoints = MakeCirclePoints(Center, Radius, 44, StartAngle, EndAngle);
			const TArray<FVector2D> StemPoints = {
				FVector2D(Center.X, MinDim * 0.12f),
				FVector2D(Center.X, Center.Y + (MinDim * 0.04f))
			};

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				ArcPoints,
				ESlateDrawEffect::None,
				FLinearColor(0.28f, 0.02f, 0.035f, 1.f),
				true,
				OuterThickness);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				ArcPoints,
				ESlateDrawEffect::None,
				FLinearColor(1.0f, 0.17f, 0.22f, 1.f),
				true,
				FMath::Max(3.f, OuterThickness - 3.f));

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 2,
				AllottedGeometry.ToPaintGeometry(),
				ArcPoints,
				ESlateDrawEffect::None,
				FLinearColor(1.f, 0.55f, 0.58f, 1.f),
				true,
				InnerThickness);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 3,
				AllottedGeometry.ToPaintGeometry(),
				StemPoints,
				ESlateDrawEffect::None,
				FLinearColor(0.28f, 0.02f, 0.035f, 1.f),
				true,
				FMath::Max(8.f, MinDim * 0.18f));

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 4,
				AllottedGeometry.ToPaintGeometry(),
				StemPoints,
				ESlateDrawEffect::None,
				FLinearColor(1.0f, 0.17f, 0.22f, 1.f),
				true,
				FMath::Max(4.f, MinDim * 0.09f));

			return LayerId + 5;
		}

	private:
		FVector2D DesiredSize = FVector2D(48.f, 48.f);
	};

	class ST66TopBarGlobeGlyph : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66TopBarGlobeGlyph) {}
			SLATE_ARGUMENT(FVector2D, DesiredSize)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			DesiredSize = InArgs._DesiredSize.IsNearlyZero() ? FVector2D(48.f, 48.f) : InArgs._DesiredSize;
		}

		virtual FVector2D ComputeDesiredSize(float) const override
		{
			return DesiredSize;
		}

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			const FVector2D Size = AllottedGeometry.GetLocalSize();
			const FVector2D Center(Size * 0.5f);
			const float MinDim = FMath::Max(1.f, FMath::Min(Size.X, Size.Y));
			const float Radius = MinDim * 0.40f;
			const float Thickness = FMath::Max(2.f, MinDim * 0.055f);
			const FLinearColor MainColor(0.90f, 0.92f, 0.95f, 1.f);

			auto MakeEllipsePoints = [](const FVector2D& InCenter, const float RadiusX, const float RadiusY, const int32 Segments)
			{
				TArray<FVector2D> Points;
				Points.Reserve(Segments + 1);
				for (int32 Index = 0; Index <= Segments; ++Index)
				{
					const float Angle = 2.f * PI * (static_cast<float>(Index) / static_cast<float>(Segments));
					Points.Add(InCenter + FVector2D(FMath::Cos(Angle) * RadiusX, FMath::Sin(Angle) * RadiusY));
				}
				return Points;
			};

			FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), MakeCirclePoints(Center, Radius, 48), ESlateDrawEffect::None, MainColor, true, Thickness);
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), MakeEllipsePoints(Center, Radius * 0.42f, Radius, 48), ESlateDrawEffect::None, MainColor, true, Thickness * 0.75f);
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(), MakeEllipsePoints(Center, Radius, Radius * 0.42f, 48), ESlateDrawEffect::None, MainColor, true, Thickness * 0.75f);

			const TArray<FVector2D> Equator = {
				FVector2D(Center.X - Radius, Center.Y),
				FVector2D(Center.X + Radius, Center.Y)
			};
			const TArray<FVector2D> Meridian = {
				FVector2D(Center.X, Center.Y - Radius),
				FVector2D(Center.X, Center.Y + Radius)
			};
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 3, AllottedGeometry.ToPaintGeometry(), Equator, ESlateDrawEffect::None, MainColor, true, Thickness * 0.75f);
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 4, AllottedGeometry.ToPaintGeometry(), Meridian, ESlateDrawEffect::None, MainColor, true, Thickness * 0.75f);
			return LayerId + 5;
		}

	private:
		FVector2D DesiredSize = FVector2D(48.f, 48.f);
	};

	class ST66TopBarProfileGlyph : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66TopBarProfileGlyph) {}
			SLATE_ARGUMENT(FVector2D, DesiredSize)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			DesiredSize = InArgs._DesiredSize.IsNearlyZero() ? FVector2D(48.f, 48.f) : InArgs._DesiredSize;
		}

		virtual FVector2D ComputeDesiredSize(float) const override
		{
			return DesiredSize;
		}

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			const FVector2D Size = AllottedGeometry.GetLocalSize();
			const FVector2D Center(Size * 0.5f);
			const float MinDim = FMath::Max(1.f, FMath::Min(Size.X, Size.Y));
			const float Thickness = FMath::Max(4.f, MinDim * 0.11f);
			const FLinearColor MainColor(0.90f, 0.92f, 0.95f, 1.f);
			const FVector2D HeadCenter(Center.X, MinDim * 0.31f);
			const float HeadRadius = MinDim * 0.17f;
			const FVector2D ShoulderCenter(Center.X, MinDim * 0.85f);
			const float ShoulderRadius = MinDim * 0.34f;

			FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), MakeCirclePoints(HeadCenter, HeadRadius, 36), ESlateDrawEffect::None, MainColor, true, Thickness);
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), MakeCirclePoints(ShoulderCenter, ShoulderRadius, 36, PI * 1.15f, PI * 1.85f), ESlateDrawEffect::None, MainColor, true, Thickness);
			const TArray<FVector2D> BodyLines = {
				FVector2D(Center.X - MinDim * 0.17f, MinDim * 0.54f),
				FVector2D(Center.X - MinDim * 0.25f, MinDim * 0.76f),
				FVector2D(Center.X + MinDim * 0.25f, MinDim * 0.76f),
				FVector2D(Center.X + MinDim * 0.17f, MinDim * 0.54f)
			};
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(), BodyLines, ESlateDrawEffect::None, MainColor, true, Thickness);
			return LayerId + 3;
		}

	private:
		FVector2D DesiredSize = FVector2D(48.f, 48.f);
	};

	class ST66TopBarBadgeBackground : public SLeafWidget
	{
	public:
		enum class EKind : uint8
		{
			Coin,
			Coupon
		};

		SLATE_BEGIN_ARGS(ST66TopBarBadgeBackground) {}
			SLATE_ARGUMENT(EKind, Kind)
			SLATE_ARGUMENT(FVector2D, DesiredSize)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			Kind = InArgs._Kind;
			DesiredSize = InArgs._DesiredSize.IsNearlyZero() ? FVector2D(56.f, 56.f) : InArgs._DesiredSize;
		}

		virtual FVector2D ComputeDesiredSize(float) const override
		{
			return DesiredSize;
		}

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			const FVector2D Size = AllottedGeometry.GetLocalSize();
			const float MinDim = FMath::Max(1.f, FMath::Min(Size.X, Size.Y));
			const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

			if (Kind == EKind::Coin)
			{
				const FVector2D Center(Size * 0.5f);
				const float OuterRadius = MinDim * 0.45f;
				const float InnerRadius = MinDim * 0.33f;
				FSlateDrawElement::MakeLines(
					OutDrawElements,
					LayerId,
					AllottedGeometry.ToPaintGeometry(),
					MakeCirclePoints(Center, OuterRadius, 40),
					ESlateDrawEffect::None,
					FLinearColor(0.62f, 0.39f, 0.04f, 1.f),
					true,
					OuterRadius * 2.f);

				FSlateDrawElement::MakeLines(
					OutDrawElements,
					LayerId + 1,
					AllottedGeometry.ToPaintGeometry(),
					MakeCirclePoints(Center, OuterRadius, 40),
					ESlateDrawEffect::None,
					FLinearColor(0.98f, 0.79f, 0.17f, 1.f),
					true,
					FMath::Max(4.f, MinDim * 0.10f));

				FSlateDrawElement::MakeLines(
					OutDrawElements,
					LayerId + 2,
					AllottedGeometry.ToPaintGeometry(),
					MakeCirclePoints(Center, InnerRadius, 36),
					ESlateDrawEffect::None,
					FLinearColor(1.f, 0.95f, 0.70f, 1.f),
					true,
					FMath::Max(2.f, MinDim * 0.04f));
				return LayerId + 3;
			}

			const FVector2D OuterInset(1.f, 4.f);
			const FVector2D OuterSize(Size.X - 2.f, Size.Y - 8.f);
			const FVector2D InnerInset(4.f, 7.f);
			const FVector2D InnerSize(Size.X - 8.f, Size.Y - 14.f);

			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(FVector2f(OuterSize), FSlateLayoutTransform(FVector2f(OuterInset))),
				WhiteBrush,
				ESlateDrawEffect::None,
				FLinearColor(0.55f, 0.37f, 0.08f, 1.f));

			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(FVector2f(InnerSize), FSlateLayoutTransform(FVector2f(InnerInset))),
				WhiteBrush,
				ESlateDrawEffect::None,
				FLinearColor(0.74f, 0.12f, 0.14f, 1.f));

			const TArray<FVector2D> OuterOutline = {
				OuterInset,
				FVector2D(OuterInset.X + OuterSize.X, OuterInset.Y),
				FVector2D(OuterInset.X + OuterSize.X, OuterInset.Y + OuterSize.Y),
				FVector2D(OuterInset.X, OuterInset.Y + OuterSize.Y),
				OuterInset
			};
			const TArray<FVector2D> InnerOutline = {
				InnerInset,
				FVector2D(InnerInset.X + InnerSize.X, InnerInset.Y),
				FVector2D(InnerInset.X + InnerSize.X, InnerInset.Y + InnerSize.Y),
				FVector2D(InnerInset.X, InnerInset.Y + InnerSize.Y),
				InnerInset
			};
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(), OuterOutline, ESlateDrawEffect::None, FLinearColor(0.94f, 0.79f, 0.29f, 1.f), true, 1.5f);
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 3, AllottedGeometry.ToPaintGeometry(), InnerOutline, ESlateDrawEffect::None, FLinearColor(0.98f, 0.85f, 0.48f, 1.f), true, 1.0f);
			return LayerId + 4;
		}

	private:
		EKind Kind = EKind::Coin;
		FVector2D DesiredSize = FVector2D(56.f, 56.f);
	};

	class ST66TopBarSlicedBrushImage : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66TopBarSlicedBrushImage)
			: _Brush(nullptr)
			, _DesiredSize(FVector2D(1.f, 1.f))
			, _SourceCapFraction(0.245f)
		{}
			SLATE_ATTRIBUTE(const FSlateBrush*, Brush)
			SLATE_ARGUMENT(FVector2D, DesiredSize)
			SLATE_ARGUMENT(float, SourceCapFraction)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			Brush = InArgs._Brush;
			DesiredSize = InArgs._DesiredSize;
			SourceCapFraction = InArgs._SourceCapFraction;
		}

		virtual FVector2D ComputeDesiredSize(float) const override
		{
			return DesiredSize;
		}

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			const FSlateBrush* SourceBrush = Brush.Get();
			if (!SourceBrush || SourceBrush == FCoreStyle::Get().GetBrush("NoBrush"))
			{
				return LayerId;
			}

			const FVector2D Size = AllottedGeometry.GetLocalSize();
			const FVector2D SourceSize(
				FMath::Max(1.f, SourceBrush->ImageSize.X),
				FMath::Max(1.f, SourceBrush->ImageSize.Y));
			if (Size.X <= 1.f || Size.Y <= 1.f || SourceSize.X <= 1.f || SourceSize.Y <= 1.f)
			{
				return LayerId;
			}

			const float CapU = FMath::Clamp(SourceCapFraction, 0.02f, 0.45f);
			const float HeightScale = Size.Y / SourceSize.Y;
			const float SourceCapWidth = SourceSize.X * CapU;
			const float DestCapWidth = FMath::Clamp(SourceCapWidth * HeightScale, 1.f, Size.X * 0.42f);
			const float DestCenterWidth = FMath::Max(0.f, Size.X - (DestCapWidth * 2.f));

			auto DrawSlice = [&](const FVector2D& Pos, const FVector2D& SliceSize, float U0, float U1)
			{
				if (SliceSize.X <= 0.5f || SliceSize.Y <= 0.5f || U1 <= U0)
				{
					return;
				}

				FSlateBrush LocalBrush = *SourceBrush;
				LocalBrush.DrawAs = ESlateBrushDrawType::Image;
				LocalBrush.Tiling = ESlateBrushTileType::NoTile;
				LocalBrush.Margin = FMargin(0.f);
				LocalBrush.SetUVRegion(FBox2f(FVector2f(U0, 0.f), FVector2f(U1, 1.f)));

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId,
					AllottedGeometry.ToPaintGeometry(
						FVector2f(SliceSize),
						FSlateLayoutTransform(FVector2f(Pos))),
					&LocalBrush,
					ESlateDrawEffect::None,
					InWidgetStyle.GetColorAndOpacityTint());
			};

			DrawSlice(FVector2D(0.f, 0.f), FVector2D(DestCapWidth, Size.Y), 0.f, CapU);
			DrawSlice(FVector2D(DestCapWidth, 0.f), FVector2D(DestCenterWidth, Size.Y), CapU, 1.f - CapU);
			DrawSlice(FVector2D(Size.X - DestCapWidth, 0.f), FVector2D(DestCapWidth, Size.Y), 1.f - CapU, 1.f);

			return LayerId + 1;
		}

	private:
		TAttribute<const FSlateBrush*> Brush;
		FVector2D DesiredSize = FVector2D(1.f, 1.f);
		float SourceCapFraction = 0.245f;
	};

	// The shared transparent button styles do not surface hover or pressed states.
	// This wrapper keeps the rendered plate visible and drives subtle state changes itself.
	class ST66TopBarStatefulButton : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66TopBarStatefulButton)
			: _NormalBrush(nullptr)
			, _HoverBrush(nullptr)
			, _PressedBrush(nullptr)
			, _DisabledBrush(nullptr)
			, _SelectedBrush(nullptr)
			, _ButtonStyle(nullptr)
			, _ContentPadding(FMargin(0.f))
			, _IsSelected(false)
			, _FallbackOuterColor(FLinearColor::White)
			, _FallbackMidColor(FLinearColor::White)
			, _FallbackInnerColor(FLinearColor::White)
		{}
			SLATE_ARGUMENT(const FSlateBrush*, NormalBrush)
			SLATE_ARGUMENT(const FSlateBrush*, HoverBrush)
			SLATE_ARGUMENT(const FSlateBrush*, PressedBrush)
			SLATE_ARGUMENT(const FSlateBrush*, DisabledBrush)
			SLATE_ARGUMENT(const FSlateBrush*, SelectedBrush)
			SLATE_ARGUMENT(const FButtonStyle*, ButtonStyle)
			SLATE_ARGUMENT(FMargin, ContentPadding)
			SLATE_ARGUMENT(bool, IsSelected)
			SLATE_ARGUMENT(FLinearColor, FallbackOuterColor)
			SLATE_ARGUMENT(FLinearColor, FallbackMidColor)
			SLATE_ARGUMENT(FLinearColor, FallbackInnerColor)
			SLATE_ARGUMENT(FText, ToolTipText)
			SLATE_EVENT(FOnClicked, OnClicked)
			SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			NormalBrush = InArgs._NormalBrush;
			HoverBrush = InArgs._HoverBrush;
			PressedBrush = InArgs._PressedBrush;
			DisabledBrush = InArgs._DisabledBrush;
			SelectedBrush = InArgs._SelectedBrush;
			ContentPadding = InArgs._ContentPadding;
			bIsSelected = InArgs._IsSelected;
			FallbackOuterColor = InArgs._FallbackOuterColor;
			FallbackMidColor = InArgs._FallbackMidColor;
			FallbackInnerColor = InArgs._FallbackInnerColor;
			ButtonStyle = InArgs._ButtonStyle
				? *InArgs._ButtonStyle
				: FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			ButtonStyle
				.SetNormalPadding(FMargin(0.f))
				.SetPressedPadding(FMargin(0.f));

			ChildSlot
			[
				FT66Style::MakeBareButton(
					FT66BareButtonParams(
						InArgs._OnClicked,
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor::Transparent)
					.Padding(0.f)
					.Clipping(EWidgetClipping::ClipToBounds)
					.RenderTransform(this, &ST66TopBarStatefulButton::GetVisualTransform)
					.RenderTransformPivot(FVector2D(0.5f, 0.5f))
					[
						SNew(SOverlay)
						.Clipping(EWidgetClipping::ClipToBounds)
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							BuildPlateWidget()
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor::Transparent)
							.ColorAndOpacity(this, &ST66TopBarStatefulButton::GetContentTint)
							.Padding(ContentPadding)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								InArgs._Content.Widget
							]
						]
					])
					.SetButtonStyle(&ButtonStyle)
					.SetPadding(FMargin(0.f)),
					&Button)
			];
			if (Button.IsValid())
			{
				Button->SetClipping(EWidgetClipping::ClipToBounds);
				Button->SetToolTipText(InArgs._ToolTipText);
			}
		}

	private:
		static FLinearColor ScaleFallbackColor(const FLinearColor& Color, float Scalar)
		{
			return FLinearColor(
				FMath::Clamp(Color.R * Scalar, 0.f, 1.f),
				FMath::Clamp(Color.G * Scalar, 0.f, 1.f),
				FMath::Clamp(Color.B * Scalar, 0.f, 1.f),
				Color.A);
		}

		bool IsButtonHovered() const
		{
			return Button.IsValid() && Button->IsHovered();
		}

		bool IsButtonPressed() const
		{
			return Button.IsValid() && Button->IsPressed();
		}

		float GetPlateScalar() const
		{
			if (IsButtonPressed())
			{
				return 0.92f;
			}

			return IsButtonHovered() ? 1.05f : 1.f;
		}

		float GetContentScalar() const
		{
			if (IsButtonPressed())
			{
				return 0.95f;
			}

			return IsButtonHovered() ? 1.03f : 1.f;
		}

		FSlateColor GetPlateTint() const
		{
			const float Scalar = GetPlateScalar();
			return FSlateColor(FLinearColor(Scalar, Scalar, Scalar, 1.f));
		}

		FLinearColor GetContentTint() const
		{
			const float Scalar = GetContentScalar();
			return FLinearColor(Scalar, Scalar, Scalar, 1.f);
		}

		TOptional<FSlateRenderTransform> GetVisualTransform() const
		{
			if (IsButtonPressed())
			{
				return TOptional<FSlateRenderTransform>(FSlateRenderTransform(FVector2D(0.f, 2.f)));
			}

			if (IsButtonHovered())
			{
				return TOptional<FSlateRenderTransform>(FSlateRenderTransform(FVector2D(0.f, -1.f)));
			}

			return TOptional<FSlateRenderTransform>();
		}

		FSlateColor GetFallbackOuterColor() const
		{
			return FSlateColor(ScaleFallbackColor(FallbackOuterColor, GetPlateScalar()));
		}

		FSlateColor GetFallbackMidColor() const
		{
			return FSlateColor(ScaleFallbackColor(FallbackMidColor, GetPlateScalar()));
		}

		FSlateColor GetFallbackInnerColor() const
		{
			return FSlateColor(ScaleFallbackColor(FallbackInnerColor, GetPlateScalar()));
		}

		const FSlateBrush* GetCurrentBrush() const
		{
			if (!Button.IsValid() || !Button->IsEnabled())
			{
				return DisabledBrush ? DisabledBrush : (NormalBrush ? NormalBrush : nullptr);
			}

			if (Button->IsPressed() && PressedBrush)
			{
				return PressedBrush;
			}

			if (bIsSelected && SelectedBrush)
			{
				return SelectedBrush;
			}

			if (Button->IsHovered() && HoverBrush)
			{
				return HoverBrush;
			}

			return NormalBrush;
		}

		TSharedRef<SWidget> BuildPlateWidget()
		{
			if (NormalBrush || HoverBrush || PressedBrush || DisabledBrush || SelectedBrush)
			{
				return SNew(ST66TopBarSlicedBrushImage)
					.Brush(this, &ST66TopBarStatefulButton::GetCurrentBrush)
					.DesiredSize(FVector2D(1.f, 1.f))
					.SourceCapFraction(0.245f);
			}

			return SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(this, &ST66TopBarStatefulButton::GetFallbackOuterColor)
				.Padding(2.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(this, &ST66TopBarStatefulButton::GetFallbackMidColor)
					.Padding(1.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(this, &ST66TopBarStatefulButton::GetFallbackInnerColor)
					]
				];
		}

		const FSlateBrush* NormalBrush = nullptr;
		const FSlateBrush* HoverBrush = nullptr;
		const FSlateBrush* PressedBrush = nullptr;
		const FSlateBrush* DisabledBrush = nullptr;
		const FSlateBrush* SelectedBrush = nullptr;
		FButtonStyle ButtonStyle;
		FMargin ContentPadding = FMargin(0.f);
		bool bIsSelected = false;
		FLinearColor FallbackOuterColor = FLinearColor::White;
		FLinearColor FallbackMidColor = FLinearColor::White;
		FLinearColor FallbackInnerColor = FLinearColor::White;
		TSharedPtr<SButton> Button;
	};

	FVector2D GetEffectiveFrontendViewportSize()
	{
		int32 AutomationViewportWidth = 0;
		int32 AutomationViewportHeight = 0;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66AutomationResX="), AutomationViewportWidth)
			&& FParse::Value(FCommandLine::Get(), TEXT("T66AutomationResY="), AutomationViewportHeight)
			&& AutomationViewportWidth > 0
			&& AutomationViewportHeight > 0)
		{
			return FVector2D(static_cast<float>(AutomationViewportWidth), static_cast<float>(AutomationViewportHeight));
		}

		return FT66Style::GetViewportSize();
	}

	float GetTopBarWidthResponsiveScale()
	{
		const FVector2D EffectiveViewportSize = GetEffectiveFrontendViewportSize();
		return FMath::Max(0.01f, EffectiveViewportSize.X / T66MainMenuReferenceLayout::CanvasWidth);
	}

	UTexture2D* LoadTopBarFileTexture(const FString& FilePath, TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		const FDateTime FileTimestamp = IFileManager::Get().GetTimeStamp(*FilePath);
		const FString CacheKey = FString::Printf(TEXT("%s|%d|%lld"), *FilePath, static_cast<int32>(Filter), FileTimestamp.GetTicks());
		if (const TStrongObjectPtr<UTexture2D>* CachedTexture = GTopBarFileTextureCache.Find(CacheKey))
		{
			return CachedTexture->Get();
		}

		if (!IFileManager::Get().FileExists(*FilePath))
		{
			return nullptr;
		}

		UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
			FilePath,
			Filter,
			TEXT("FrontendTopBar"));
		if (!Texture)
		{
			return nullptr;
		}

		GTopBarFileTextureCache.Add(CacheKey, TStrongObjectPtr<UTexture2D>(Texture));
		return Texture;
	}

	FString ResolveTopBarAssetPath(const TCHAR* ImportedAssetPath, const FString& RelativePath)
	{
		if (ImportedAssetPath && *ImportedAssetPath)
		{
			return FString(ImportedAssetPath);
		}

		if (!RelativePath.StartsWith(TEXT("SourceAssets/"), ESearchCase::CaseSensitive)
			|| !RelativePath.EndsWith(TEXT(".png"), ESearchCase::IgnoreCase))
		{
			return FString();
		}

		const FString NormalizedRelativePath = RelativePath.Replace(TEXT("\\"), TEXT("/"));
		const FString PackagePath = FString::Printf(TEXT("/Game/%s"), *NormalizedRelativePath.LeftChop(4));
		const FString AssetName = FPaths::GetBaseFilename(RelativePath);
		return FString::Printf(TEXT("%s.%s"), *PackagePath, *AssetName);
	}

	void LoadBrushFromRelativePath(
		const TCHAR* ImportedAssetPath,
		const FString& RelativePath,
		TSharedPtr<FSlateBrush>& Brush,
		const FVector2D& DesiredSize = FVector2D::ZeroVector,
		TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		if (Brush.IsValid())
		{
			return;
		}

		UTexture2D* Texture = nullptr;
		const FString FullPath = T66RuntimeUITextureAccess::MakeProjectContentPath(RelativePath);
		const bool bFileExists = IFileManager::Get().FileExists(*FullPath);
		const FString AssetPath = ResolveTopBarAssetPath(ImportedAssetPath, RelativePath);
		UE_LOG(
			LogT66FrontendTopBar,
			Verbose,
			TEXT("[TopBarBrushLoad] Begin RelativePath='%s' FullPath='%s' FileExists=%s AssetPath='%s' DesiredSize=(%.1f, %.1f)"),
			*RelativePath,
			*FullPath,
			bFileExists ? TEXT("true") : TEXT("false"),
			AssetPath.IsEmpty() ? TEXT("<null>") : *AssetPath,
			DesiredSize.X,
			DesiredSize.Y);

		if (!AssetPath.IsEmpty())
		{
			Texture = T66RuntimeUITextureAccess::LoadAssetTexture(*AssetPath, Filter, TEXT("FrontendTopBar"));
		}

		FString LoadedFrom = TEXT("none");
		if (Texture)
		{
			LoadedFrom = TEXT("asset");
		}

		if (!Texture && bFileExists)
		{
			Texture = LoadTopBarFileTexture(FullPath, Filter);
			if (Texture)
			{
				LoadedFrom = TEXT("file");
			}
		}

		UE_LOG(
			LogT66FrontendTopBar,
			Verbose,
			TEXT("[TopBarBrushLoad] Result RelativePath='%s' LoadedFrom=%s Texture=%s Resolution=%dx%d"),
			*RelativePath,
			*LoadedFrom,
			Texture ? *Texture->GetName() : TEXT("<null>"),
			Texture ? Texture->GetSizeX() : 0,
			Texture ? Texture->GetSizeY() : 0);

		if (Texture)
		{
			Brush = MakeShared<FSlateBrush>();
			Brush->DrawAs = ESlateBrushDrawType::Image;
			Brush->Tiling = ESlateBrushTileType::NoTile;
			Brush->ImageSize = DesiredSize.IsNearlyZero()
				? FVector2D(static_cast<float>(Texture->GetSizeX()), static_cast<float>(Texture->GetSizeY()))
				: DesiredSize;
			Brush->SetResourceObject(Texture);
			Brush->TintColor = FSlateColor(FLinearColor::White);
		}
	}

	void LoadLooseBrushFromRelativePath(
		const FString& RelativePath,
		TSharedPtr<FSlateBrush>& Brush,
		const FVector2D& DesiredSize = FVector2D::ZeroVector,
		TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		if (Brush.IsValid())
		{
			return;
		}

		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
		{
			if (!IFileManager::Get().FileExists(*CandidatePath))
			{
				continue;
			}

			if (UTexture2D* Texture = LoadTopBarFileTexture(CandidatePath, Filter))
			{
				Brush = MakeShared<FSlateBrush>();
				Brush->DrawAs = ESlateBrushDrawType::Image;
				Brush->Tiling = ESlateBrushTileType::NoTile;
				Brush->ImageSize = DesiredSize.IsNearlyZero()
					? FVector2D(static_cast<float>(Texture->GetSizeX()), static_cast<float>(Texture->GetSizeY()))
					: DesiredSize;
				Brush->SetResourceObject(Texture);
				Brush->TintColor = FSlateColor(FLinearColor::White);
				return;
			}
		}
	}

	void LoadBrushFromCandidatePaths(
		const TArray<FString>& RelativePaths,
		TSharedPtr<FSlateBrush>& Brush,
		const FVector2D& DesiredSize = FVector2D::ZeroVector,
		TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		for (const FString& RelativePath : RelativePaths)
		{
			LoadBrushFromRelativePath(nullptr, RelativePath, Brush, DesiredSize, Filter);
			if (Brush.IsValid())
			{
				return;
			}
		}
	}

	void LoadLooseBrushFromCandidatePaths(
		const TArray<FString>& RelativePaths,
		TSharedPtr<FSlateBrush>& Brush,
		const FVector2D& DesiredSize = FVector2D::ZeroVector,
		TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		for (const FString& RelativePath : RelativePaths)
		{
			LoadLooseBrushFromRelativePath(RelativePath, Brush, DesiredSize, Filter);
			if (Brush.IsValid())
			{
				return;
			}
		}

		for (const FString& RelativePath : RelativePaths)
		{
			if (T66RuntimeUIBrushAccess::ShouldUseSimpleReferenceFallback(RelativePath))
			{
				Brush = MakeShared<FSlateBrush>();
				T66RuntimeUIBrushAccess::ConfigureSimpleReferenceFallbackBrush(
					*Brush,
					RelativePath,
					DesiredSize.IsNearlyZero() ? FVector2D(1.f, 1.f) : DesiredSize,
					FMargin(0.f),
					ESlateBrushDrawType::Image);
				return;
			}
		}
	}

	void CopyBrushAsButtonStateSet(
		const TSharedPtr<FSlateBrush>& SourceBrush,
		UT66FrontendTopBarWidget::FPlateBrushSet& OutSet)
	{
		OutSet.NormalBrush = SourceBrush;
		OutSet.HoverBrush = SourceBrush;
		OutSet.PressedBrush = SourceBrush;
		OutSet.DisabledBrush = SourceBrush;
		OutSet.SelectedBrush = SourceBrush;
	}

	void LoadButtonStateSetFromPaths(
		const TArray<FString>& NormalPaths,
		const TArray<FString>& HoverPaths,
		const TArray<FString>& PressedPaths,
		const TArray<FString>& DisabledPaths,
		const TArray<FString>& SelectedPaths,
		UT66FrontendTopBarWidget::FPlateBrushSet& OutSet,
		const FVector2D& DesiredSize = FVector2D::ZeroVector,
		const TextureFilter Filter = TextureFilter::TF_Nearest)
	{
		LoadLooseBrushFromCandidatePaths(NormalPaths, OutSet.NormalBrush, DesiredSize, Filter);
		LoadLooseBrushFromCandidatePaths(HoverPaths, OutSet.HoverBrush, DesiredSize, Filter);
		LoadLooseBrushFromCandidatePaths(PressedPaths, OutSet.PressedBrush, DesiredSize, Filter);
		LoadLooseBrushFromCandidatePaths(DisabledPaths, OutSet.DisabledBrush, DesiredSize, Filter);
		LoadLooseBrushFromCandidatePaths(SelectedPaths, OutSet.SelectedBrush, DesiredSize, Filter);

		if (!OutSet.HoverBrush.IsValid())
		{
			OutSet.HoverBrush = OutSet.NormalBrush;
		}

		if (!OutSet.PressedBrush.IsValid())
		{
			OutSet.PressedBrush = OutSet.HoverBrush.IsValid() ? OutSet.HoverBrush : OutSet.NormalBrush;
		}

		if (!OutSet.DisabledBrush.IsValid())
		{
			OutSet.DisabledBrush = OutSet.NormalBrush;
		}

		if (!OutSet.SelectedBrush.IsValid())
		{
			OutSet.SelectedBrush = OutSet.PressedBrush.IsValid() ? OutSet.PressedBrush : OutSet.HoverBrush;
		}
	}

	void ConfigureBoxBrush(const TSharedPtr<FSlateBrush>& Brush, const FMargin& Margin)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		const bool bUseImageDraw =
			FMath::IsNearlyZero(Margin.Left)
			&& FMath::IsNearlyZero(Margin.Top)
			&& FMath::IsNearlyZero(Margin.Right)
			&& FMath::IsNearlyZero(Margin.Bottom);
		Brush->DrawAs = bUseImageDraw ? ESlateBrushDrawType::Image : ESlateBrushDrawType::Box;
		Brush->Margin = Margin;
	}

	void ConfigureBoxBrushSet(UT66FrontendTopBarWidget::FPlateBrushSet& BrushSet, const FMargin& Margin)
	{
		ConfigureBoxBrush(BrushSet.NormalBrush, Margin);
		ConfigureBoxBrush(BrushSet.HoverBrush, Margin);
		ConfigureBoxBrush(BrushSet.PressedBrush, Margin);
		ConfigureBoxBrush(BrushSet.DisabledBrush, Margin);
		ConfigureBoxBrush(BrushSet.SelectedBrush, Margin);
	}

	void ResetBrushSet(UT66FrontendTopBarWidget::FPlateBrushSet& BrushSet)
	{
		BrushSet.NormalBrush.Reset();
		BrushSet.HoverBrush.Reset();
		BrushSet.PressedBrush.Reset();
		BrushSet.DisabledBrush.Reset();
		BrushSet.SelectedBrush.Reset();
	}

	TSharedRef<SWidget> MakeWarmFallbackGlyph(const FText& Text, int32 FontSize)
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(FT66Style::MakeFont(TEXT("Bold"), FontSize))
			.ColorAndOpacity(FLinearColor(0.98f, 0.96f, 1.0f, 1.0f))
			.ShadowOffset(FVector2D(1.f, 1.f))
			.ShadowColorAndOpacity(FLinearColor(0.02f, 0.0f, 0.04f, 0.95f));
	}
}

UT66FrontendTopBarWidget::UT66FrontendTopBarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::None;
	bIsModal = false;
}

float UT66FrontendTopBarWidget::GetReservedHeight()
{
	return GetTopBarReferenceReservedHeight() * GetTopBarWidthResponsiveScale();
}

float UT66FrontendTopBarWidget::GetVisibleContentHeight()
{
	return GetTopBarReferenceSurfaceHeight() * GetTopBarWidthResponsiveScale();
}

UT66FrontendTopBarWidget::EFrontendSection UT66FrontendTopBarWidget::ResolveFrontendSectionForScreen(const ET66ScreenType ScreenType)
{
	switch (ScreenType)
	{
	case ET66ScreenType::AccountStatus:
		return EFrontendSection::AccountStatus;
	case ET66ScreenType::Settings:
		return EFrontendSection::Settings;
	case ET66ScreenType::LanguageSelect:
		return EFrontendSection::Language;
	case ET66ScreenType::PowerUp:
		return EFrontendSection::PowerUp;
	case ET66ScreenType::Achievements:
		return EFrontendSection::Achievements;
	case ET66ScreenType::Minigames:
		return EFrontendSection::MiniGames;
	case ET66ScreenType::MainMenu:
		return EFrontendSection::Home;
	default:
		return EFrontendSection::None;
	}
}

void UT66FrontendTopBarWidget::SetActiveSection(const EFrontendSection InActiveSection)
{
	if (bHasActiveSectionOverride && ActiveSectionOverride == InActiveSection)
	{
		return;
	}

	ActiveSectionOverride = InActiveSection;
	bHasActiveSectionOverride = true;
	if (GetCachedWidget().IsValid())
	{
		FT66Style::DeferRebuild(this, GTopBarViewportZOrder);
	}
}

void UT66FrontendTopBarWidget::ClearActiveSectionOverride()
{
	if (!bHasActiveSectionOverride)
	{
		return;
	}

	bHasActiveSectionOverride = false;
	ActiveSectionOverride = EFrontendSection::None;
	if (GetCachedWidget().IsValid())
	{
		FT66Style::DeferRebuild(this, GTopBarViewportZOrder);
	}
}

UT66FrontendTopBarWidget::EFrontendSection UT66FrontendTopBarWidget::GetRenderedActiveSection() const
{
	return GetActiveSection();
}

TSharedRef<SWidget> UT66FrontendTopBarWidget::RebuildWidget()
{
	// The top bar already sizes itself against the live safe-frame width.
	// Applying the shared responsive root here compresses it twice.
	return BuildSlateUI();
}

UT66LocalizationSubsystem* UT66FrontendTopBarWidget::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

UT66FrontendTopBarWidget::ETopBarSection UT66FrontendTopBarWidget::GetActiveSection() const
{
	if (bHasActiveSectionOverride)
	{
		return ActiveSectionOverride;
	}

	if (!UIManager)
	{
		return EFrontendSection::Home;
	}

	ET66ScreenType FocusedScreenType = UIManager->GetCurrentScreenType();
	if (UIManager->IsModalActive() && UIManager->GetCurrentModalType() == ET66ScreenType::AccountStatus)
	{
		FocusedScreenType = ET66ScreenType::AccountStatus;
	}

	switch (FocusedScreenType)
	{
	case ET66ScreenType::AccountStatus:
		return EFrontendSection::AccountStatus;
	case ET66ScreenType::Settings:
		return EFrontendSection::Settings;
	case ET66ScreenType::LanguageSelect:
		return EFrontendSection::Language;
	case ET66ScreenType::PowerUp:
		return EFrontendSection::PowerUp;
	case ET66ScreenType::Achievements:
		return EFrontendSection::Achievements;
	case ET66ScreenType::Minigames:
		return EFrontendSection::MiniGames;
	case ET66ScreenType::MainMenu:
	default:
		return EFrontendSection::Home;
	}
}

FText UT66FrontendTopBarWidget::GetChadCouponsValueText() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66AchievementsSubsystem* Achievements = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			return FText::AsNumber(Achievements->GetChadCouponBalance());
		}
	}

	return FText::AsNumber(0);
}

void UT66FrontendTopBarWidget::NavigateWithTopBar(const ET66ScreenType TargetScreen)
{
	if (!UIManager)
	{
		NavigateTo(TargetScreen);
		return;
	}

	if (UIManager->IsModalActive())
	{
		if (UIManager->GetCurrentScreenType() == TargetScreen)
		{
			UIManager->CloseModal();
			return;
		}

		UIManager->CloseModal();
	}

	if (TargetScreen != ET66ScreenType::MainMenu && UIManager->GetCurrentScreenType() == TargetScreen)
	{
		return;
	}

	UIManager->ShowScreen(TargetScreen);
}

void UT66FrontendTopBarWidget::RequestTopBarAssets()
{
	LoadLooseBrushFromCandidatePaths(
		{ TEXT("RuntimeDependencies/T66/UI/Icons/Flat/people.png") },
		HomeIconBrush,
		FVector2D(58.f, 58.f),
		TextureFilter::TF_Nearest);
	LoadLooseBrushFromCandidatePaths(
		{ TEXT("RuntimeDependencies/T66/UI/Icons/Flat/gear.png") },
		SettingsIconBrush,
		FVector2D(54.f, 54.f),
		TextureFilter::TF_Nearest);
	LoadLooseBrushFromCandidatePaths(
		{ TEXT("RuntimeDependencies/T66/UI/Icons/Flat/broadcast_antenna.png") },
		SocialIconBrush,
		FVector2D(54.f, 54.f),
		TextureFilter::TF_Nearest);
	LoadLooseBrushFromCandidatePaths(
		{ TEXT("RuntimeDependencies/T66/UI/Icons/Flat/ticket.png") },
		CurrencyIconBrush,
		FVector2D(46.f, 46.f),
		TextureFilter::TF_Nearest);
	LoadLooseBrushFromCandidatePaths(
		{ TEXT("RuntimeDependencies/T66/UI/Icons/Flat/x_mark.png") },
		QuitIconBrush,
		FVector2D(54.f, 54.f),
		TextureFilter::TF_Nearest);
}

void UT66FrontendTopBarWidget::ReleaseTopBarBrushes()
{
	auto ReleaseBrush = [](TSharedPtr<FSlateBrush>& Brush)
	{
		if (Brush.IsValid())
		{
			Brush->SetResourceObject(nullptr);
			Brush.Reset();
		}
	};
	auto ReleaseBrushSet = [&ReleaseBrush](UT66FrontendTopBarWidget::FPlateBrushSet& BrushSet)
	{
		ReleaseBrush(BrushSet.NormalBrush);
		ReleaseBrush(BrushSet.HoverBrush);
		ReleaseBrush(BrushSet.PressedBrush);
		ReleaseBrush(BrushSet.DisabledBrush);
		ReleaseBrush(BrushSet.SelectedBrush);
	};

	ReleaseBrush(TopBarBackdropBrush);
	ReleaseBrush(TopBarFoliageLeftBrush);
	ReleaseBrush(TopBarFoliageRightBrush);
	ReleaseBrushSet(SettingsButtonBrushes);
	ReleaseBrushSet(LanguageButtonBrushes);
	ReleaseBrushSet(AccountButtonBrushes);
	ReleaseBrushSet(HomeButtonBrushes);
	ReleaseBrushSet(NavButtonBrushes);
	ReleaseBrushSet(PowerUpButtonBrushes);
	ReleaseBrushSet(AchievementsButtonBrushes);
	ReleaseBrushSet(MiniGamesButtonBrushes);
	ReleaseBrushSet(PortraitButtonBrushes);
	ReleaseBrushSet(CouponButtonBrushes);
	ReleaseBrushSet(QuitButtonBrushes);
	ReleaseBrush(UtilityButtonBrush);
	ReleaseBrush(AccountButtonBrush);
	ReleaseBrush(AccountButtonActiveBrush);
	ReleaseBrush(NavButtonBrush);
	ReleaseBrush(NavButtonActiveBrush);
	ReleaseBrush(HomeButtonBrush);
	ReleaseBrush(HomeButtonActiveBrush);
	ReleaseBrush(CurrencyButtonBrush);
	ReleaseBrush(HomeIconBrush);
	ReleaseBrush(SettingsIconBrush);
	ReleaseBrush(SocialIconBrush);
	ReleaseBrush(CurrencyIconBrush);
	ReleaseBrush(QuitIconBrush);
}

void UT66FrontendTopBarWidget::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();

	if (UWorld* World = GetWorld())
	{
		if (World->bIsTearingDown || GExitPurge || IsGarbageCollecting())
		{
			return;
		}
	}

	FT66Style::DeferRebuild(this, GTopBarViewportZOrder);
}

TSharedRef<SWidget> UT66FrontendTopBarWidget::BuildSlateUI()
{
	{
		CachedViewportSize = GetEffectiveFrontendViewportSize();
		bViewportResponsiveRebuildQueued = false;
		RequestTopBarAssets();

		UT66LocalizationSubsystem* Loc = GetLocSubsystem();
		const EFrontendSection ActiveSection = GetActiveSection();
		const FText SettingsText = Loc ? Loc->GetText_Settings() : NSLOCTEXT("T66.MainMenu", "Settings", "SETTINGS");
		const FText LanguageText = Loc ? Loc->GetText_LangButton() : NSLOCTEXT("T66.LanguageSelect", "LangButton", "LANG");
		const FText AccountText = Loc ? Loc->GetText_AccountStatus() : NSLOCTEXT("T66.AccountStatus", "Title", "ACCOUNT");
		const FText PowerUpText = NSLOCTEXT("T66.MainMenu", "PowerUp", "POWER UP");
		const FText AchievementsText = Loc ? Loc->GetText_Achievements() : NSLOCTEXT("T66.MainMenu", "Achievements", "ACHIEVEMENTS");
		const FText MiniGamesText = NSLOCTEXT("T66.MainMenu", "MiniGames", "MINIGAMES");
		const FText BackToMainMenuText = NSLOCTEXT("T66.MainMenu", "BackToMainMenu", "BACK TO MAIN MENU");
		const FText QuitTooltipText = Loc ? Loc->GetText_Quit() : NSLOCTEXT("T66.MainMenu", "Quit", "QUIT");

		struct FNormalizedTopBarRect
		{
			float X;
			float Y;
			float W;
			float H;

			FMargin ToReferenceOffset() const
			{
				return FMargin(X * T66MainMenuReferenceLayout::CanvasWidth, Y * T66MainMenuReferenceLayout::CanvasHeight, W * T66MainMenuReferenceLayout::CanvasWidth, H * T66MainMenuReferenceLayout::CanvasHeight);
			}

			float ReferenceWidth() const
			{
				return W * T66MainMenuReferenceLayout::CanvasWidth;
			}

			float ReferenceHeight() const
			{
				return H * T66MainMenuReferenceLayout::CanvasHeight;
			}
		};

		const FNormalizedTopBarRect OuterRect{ 0.012f, 0.006f, 0.976f, 0.077f };
		const FNormalizedTopBarRect SettingsRect{ 0.013f, 0.006f, 0.046f, 0.077f };
		const FNormalizedTopBarRect LanguageRect{ 0.073f, 0.006f, 0.046f, 0.077f };
		const FNormalizedTopBarRect AccountRect{ 0.133f, 0.006f, 0.156f, 0.077f };
		const FNormalizedTopBarRect ProfileRect{ 0.302f, 0.006f, 0.043f, 0.077f };
		const FNormalizedTopBarRect PowerUpRect{ 0.359f, 0.006f, 0.142f, 0.077f };
		const FNormalizedTopBarRect AchievementsRect{ 0.513f, 0.006f, 0.153f, 0.077f };
		const FNormalizedTopBarRect MiniGamesRect{ 0.679f, 0.006f, 0.126f, 0.077f };
		const FNormalizedTopBarRect TicketRect{ 0.819f, 0.006f, 0.087f, 0.077f };
		const FNormalizedTopBarRect QuitRect{ 0.923f, 0.006f, 0.063f, 0.077f };

		const float IconSize = 46.f;
		auto MakeTaggedIconWidget = [](const TSharedRef<SWidget>& IconContent, const FVector2D& Size, const FName Tag) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(
				SNew(SBox)
				.WidthOverride(Size.X)
				.HeightOverride(Size.Y)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					IconContent
				],
				Tag,
				TEXT("Icon"),
				ET66FlatState::Default);
		};

		auto MakeTaggedIcon = [&MakeTaggedIconWidget](const TSharedPtr<FSlateBrush>& Brush, const FText& FallbackText, const FVector2D& Size, const FName Tag) -> TSharedRef<SWidget>
		{
			const TSharedRef<SWidget> IconContent = Brush.IsValid()
				? StaticCastSharedRef<SWidget>(SNew(SImage).Image(Brush.Get()).ColorAndOpacity(FLinearColor::White))
				: StaticCastSharedRef<SWidget>(SNew(STextBlock)
					.Text(FallbackText)
					.Font(FT66FlatStyle::MakeBoldFont(24))
					.ColorAndOpacity(FT66FlatStyle::DefaultText())
					.Justification(ETextJustify::Center));

			return MakeTaggedIconWidget(IconContent, Size, Tag);
		};

		auto MakeIconActionButton = [this](const ET66FlatState State, const TSharedRef<SWidget>& Icon, FReply (UT66FrontendTopBarWidget::*ClickFunc)(), const FName Tag, const FNormalizedTopBarRect& Rect) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatToggleGroupButton(
				State,
				Icon,
				FOnClicked::CreateUObject(this, ClickFunc),
				FMargin(8.f),
				Rect.ReferenceWidth(),
				Rect.ReferenceHeight(),
				true,
				Tag);
		};

		const bool bDailyDescentTopBar = UIManager && UIManager->GetCurrentScreenType() == ET66ScreenType::DailyDescent;
		if (bDailyDescentTopBar)
		{
			const FNormalizedTopBarRect DailyOuterRect{ 0.010f, 0.001f, 0.981f, 0.104f };
			const FNormalizedTopBarRect DailySettingsRect{ 0.023f, 0.016f, 0.044f, 0.073f };
			const FNormalizedTopBarRect DailyLanguageRect{ 0.083f, 0.016f, 0.045f, 0.074f };
			const FNormalizedTopBarRect DailyBackRect{ 0.322f, 0.016f, 0.347f, 0.076f };
			const FNormalizedTopBarRect DailyQuitRect{ 0.919f, 0.016f, 0.050f, 0.076f };

			const TSharedRef<SWidget> SettingsButtonWidget = MakeIconActionButton(
				ET66FlatState::Default,
				MakeTaggedIconWidget(SNew(ST66TopBarGearGlyph).DesiredSize(FVector2D(IconSize, IconSize)), FVector2D(IconSize, IconSize), TEXT("FrontendTopBar.SettingsButton.Icon")),
				&UT66FrontendTopBarWidget::HandleSettingsClicked,
				TEXT("FrontendTopBar.SettingsButton"),
				DailySettingsRect);
			const TSharedRef<SWidget> LanguageButtonWidget = MakeIconActionButton(
				ET66FlatState::Default,
				MakeTaggedIconWidget(SNew(ST66TopBarGlobeGlyph).DesiredSize(FVector2D(IconSize, IconSize)), FVector2D(IconSize, IconSize), TEXT("FrontendTopBar.GlobeButton.Icon")),
				&UT66FrontendTopBarWidget::HandleLanguageClicked,
				TEXT("FrontendTopBar.GlobeButton"),
				DailyLanguageRect);
			const TSharedRef<SWidget> BackToMainMenuButtonWidget = FT66FlatStyle::MakeFlatToggleGroupButton(
				ET66FlatState::Default,
				FT66FlatStyle::MakeFlatLabel(BackToMainMenuText, ET66FlatLabelRole::Button, ETextJustify::Center, TEXT("FrontendTopBar.BackToMainMenuButton.Label")),
				FOnClicked::CreateUObject(this, &UT66FrontendTopBarWidget::HandleHomeClicked),
				FMargin(12.f, 8.f),
				DailyBackRect.ReferenceWidth(),
				DailyBackRect.ReferenceHeight(),
				true,
				TEXT("FrontendTopBar.BackToMainMenuButton"));
			const TSharedRef<SWidget> PowerButtonWidget = MakeIconActionButton(
				ET66FlatState::Selected,
				MakeTaggedIconWidget(SNew(ST66TopBarPowerGlyph).DesiredSize(FVector2D(IconSize, IconSize)), FVector2D(IconSize, IconSize), TEXT("FrontendTopBar.PowerButton.Icon")),
				&UT66FrontendTopBarWidget::HandleQuitClicked,
				TEXT("FrontendTopBar.PowerButton"),
				DailyQuitRect);
			PowerButtonWidget->SetToolTipText(QuitTooltipText);

			const TSharedRef<SConstraintCanvas> DailyTopBarCanvas = SNew(SConstraintCanvas);
			DailyTopBarCanvas->AddSlot()
				.Alignment(FVector2D(0.f, 0.f))
				.Offset(DailyOuterRect.ToReferenceOffset())
				[
					FT66FlatStyle::AttachMetadata(
						SNew(SBox),
						TEXT("FrontendTopBar.OuterContainer"),
						TEXT("TransparentRegion"),
						ET66FlatState::Default)
				];

			auto AddDailyControl = [&DailyTopBarCanvas](const FNormalizedTopBarRect& Rect, const TSharedRef<SWidget>& Widget)
			{
				DailyTopBarCanvas->AddSlot()
					.Alignment(FVector2D(0.f, 0.f))
					.Offset(Rect.ToReferenceOffset())
					[
						Widget
					];
			};

			AddDailyControl(DailySettingsRect, SettingsButtonWidget);
			AddDailyControl(DailyLanguageRect, LanguageButtonWidget);
			AddDailyControl(DailyBackRect, BackToMainMenuButtonWidget);
			AddDailyControl(DailyQuitRect, PowerButtonWidget);

			const TSharedRef<SWidget> ReservedReferenceCanvas =
				SNew(SBox)
				.WidthOverride(T66MainMenuReferenceLayout::CanvasWidth)
				.HeightOverride(GetTopBarReferenceReservedHeight())
				[
					DailyTopBarCanvas
				];

			const float ReservedHeight = GetReservedHeight();
			const FVector2D ViewportSize = GetEffectiveFrontendViewportSize();
			const TSharedRef<SWidget> Root =
				SNew(SBox)
				.WidthOverride(FMath::Max(1.f, ViewportSize.X))
				.HeightOverride(FMath::Max(1.f, ViewportSize.Y))
				[
					SNew(SVerticalBox)
					.Visibility(EVisibility::SelfHitTestInvisible)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.HeightOverride(ReservedHeight)
						[
							SNew(SDPIScaler)
							.DPIScale(TAttribute<float>::CreateLambda([]() -> float
							{
								return 1.f / FMath::Max(0.01f, FT66Style::GetEngineDPIScale());
							}))
							[
								SNew(SScaleBox)
								.Stretch(EStretch::ScaleToFitX)
								.StretchDirection(EStretchDirection::Both)
								[
									ReservedReferenceCanvas
								]
							]
						]
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SNullWidget::NullWidget
					]
				];

			return FT66FlatStyle::AttachMetadata(
				Root,
				TEXT("FrontendTopBar.Root"),
				TEXT("TopBarRoot"),
				ET66FlatState::Default);
		}

		FT66FlatToggleGroupParams CategoryGroup;
		CategoryGroup.GroupName = TEXT("FrontendCategorySelection");
		CategoryGroup.bMutuallyExclusive = true;

		auto MakeCategoryItem = [this](const FText& Label, FReply (UT66FrontendTopBarWidget::*ClickFunc)(), const FName Tag, const FNormalizedTopBarRect& Rect, const bool bSelected) -> FT66FlatToggleGroupItem
		{
			FT66FlatToggleGroupItem Item;
			Item.State = ET66FlatState::Default;
			Item.bIsSelected = bSelected;
			Item.Label = Label;
			Item.OnClicked = FOnClicked::CreateUObject(this, ClickFunc);
			Item.Padding = FMargin(12.f, 8.f);
			Item.MinWidth = Rect.ReferenceWidth();
			Item.Height = Rect.ReferenceHeight();
			Item.FontSize = 30;
			Item.Tag = Tag;
			return Item;
		};

		CategoryGroup.Items.Add(MakeCategoryItem(
			AccountText,
			&UT66FrontendTopBarWidget::HandleAccountStatusClicked,
			TEXT("FrontendTopBar.AccountButton"),
			AccountRect,
			false));
		CategoryGroup.Items.Add(MakeCategoryItem(
			PowerUpText,
			&UT66FrontendTopBarWidget::HandlePowerUpClicked,
			TEXT("FrontendTopBar.PowerUpButton"),
			PowerUpRect,
			ActiveSection == EFrontendSection::PowerUp));
		CategoryGroup.Items.Add(MakeCategoryItem(
			AchievementsText,
			&UT66FrontendTopBarWidget::HandleAchievementsClicked,
			TEXT("FrontendTopBar.AchievementsButton"),
			AchievementsRect,
			ActiveSection == EFrontendSection::Achievements));
		CategoryGroup.Items.Add(MakeCategoryItem(
			MiniGamesText,
			&UT66FrontendTopBarWidget::HandleMiniGamesClicked,
			TEXT("FrontendTopBar.MinigamesButton"),
			MiniGamesRect,
			ActiveSection == EFrontendSection::MiniGames));
		const TArray<TSharedRef<SWidget>> CategoryButtons = FT66FlatStyle::MakeFlatToggleGroup(CategoryGroup);

		const TSharedRef<SWidget> SettingsButtonWidget = MakeIconActionButton(
			ActiveSection == EFrontendSection::Settings ? ET66FlatState::Selected : ET66FlatState::Default,
			MakeTaggedIconWidget(SNew(ST66TopBarGearGlyph).DesiredSize(FVector2D(IconSize, IconSize)), FVector2D(IconSize, IconSize), TEXT("FrontendTopBar.SettingsButton.Icon")),
			&UT66FrontendTopBarWidget::HandleSettingsClicked,
			TEXT("FrontendTopBar.SettingsButton"),
			SettingsRect);
		const TSharedRef<SWidget> LanguageButtonWidget = MakeIconActionButton(
			ActiveSection == EFrontendSection::Language ? ET66FlatState::Selected : ET66FlatState::Default,
			MakeTaggedIconWidget(SNew(ST66TopBarGlobeGlyph).DesiredSize(FVector2D(IconSize, IconSize)), FVector2D(IconSize, IconSize), TEXT("FrontendTopBar.GlobeButton.Icon")),
			&UT66FrontendTopBarWidget::HandleLanguageClicked,
			TEXT("FrontendTopBar.GlobeButton"),
			LanguageRect);
		const TSharedRef<SWidget> ProfileButtonWidget = MakeIconActionButton(
			ActiveSection == EFrontendSection::Home ? ET66FlatState::Selected : ET66FlatState::Default,
			MakeTaggedIconWidget(SNew(ST66TopBarProfileGlyph).DesiredSize(FVector2D(IconSize, IconSize)), FVector2D(IconSize, IconSize), TEXT("FrontendTopBar.ProfileButton.Icon")),
			&UT66FrontendTopBarWidget::HandleHomeClicked,
			TEXT("FrontendTopBar.ProfileButton"),
			ProfileRect);
		const TSharedRef<SWidget> PowerButtonWidget = MakeIconActionButton(
			ET66FlatState::Selected,
			MakeTaggedIconWidget(SNew(ST66TopBarPowerGlyph).DesiredSize(FVector2D(IconSize, IconSize)), FVector2D(IconSize, IconSize), TEXT("FrontendTopBar.PowerButton.Icon")),
			&UT66FrontendTopBarWidget::HandleQuitClicked,
			TEXT("FrontendTopBar.PowerButton"),
			QuitRect);
		PowerButtonWidget->SetToolTipText(QuitTooltipText);

		const TSharedRef<SWidget> TicketIcon = MakeTaggedIcon(CurrencyIconBrush, FText::FromString(TEXT("T")), FVector2D(40.f, 40.f), TEXT("FrontendTopBar.TicketBadge.Icon"));
		const TSharedRef<SWidget> TicketValue = FT66FlatStyle::MakeFlatLabel(
			TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateUObject(this, &UT66FrontendTopBarWidget::GetChadCouponsValueText)),
			ET66FlatLabelRole::Button,
			ETextJustify::Center,
			TEXT("FrontendTopBar.TicketBadge.Value"));
		const TSharedRef<SConstraintCanvas> TicketCanvas = SNew(SConstraintCanvas);
		TicketCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(36.f, 15.f, 40.f, 40.f))
			[
				TicketIcon
			];
		TicketCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(96.f, 18.f, 52.f, 30.f))
			[
				TicketValue
			];
		const TSharedRef<SWidget> TicketContent =
			SNew(SBox)
			.WidthOverride(FMath::Max(1.f, TicketRect.ReferenceWidth() - 16.f))
			.HeightOverride(FMath::Max(1.f, TicketRect.ReferenceHeight() - 12.f))
			[
				TicketCanvas
			];
		const TSharedRef<SWidget> TicketBadgeWidget = FT66FlatStyle::MakeFlatToggleGroupButton(
			ET66FlatState::Default,
			TicketContent,
			FOnClicked::CreateUObject(this, &UT66FrontendTopBarWidget::HandlePowerUpClicked),
			FMargin(8.f, 6.f),
			TicketRect.ReferenceWidth(),
			TicketRect.ReferenceHeight(),
			true,
			TEXT("FrontendTopBar.TicketBadge"));

		const TSharedRef<SConstraintCanvas> TopBarCanvas = SNew(SConstraintCanvas);
		TopBarCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(OuterRect.ToReferenceOffset())
			[
				FT66FlatStyle::AttachMetadata(
					SNew(SBox),
					TEXT("FrontendTopBar.OuterContainer"),
					TEXT("TransparentRegion"),
					ET66FlatState::Default)
			];

		auto AddControl = [&TopBarCanvas](const FNormalizedTopBarRect& Rect, const TSharedRef<SWidget>& Widget)
		{
			TopBarCanvas->AddSlot()
				.Alignment(FVector2D(0.f, 0.f))
				.Offset(Rect.ToReferenceOffset())
				[
					Widget
				];
		};

		AddControl(SettingsRect, SettingsButtonWidget);
		AddControl(LanguageRect, LanguageButtonWidget);
		AddControl(AccountRect, CategoryButtons[0]);
		AddControl(ProfileRect, ProfileButtonWidget);
		AddControl(PowerUpRect, CategoryButtons[1]);
		AddControl(AchievementsRect, CategoryButtons[2]);
		AddControl(MiniGamesRect, CategoryButtons[3]);
		AddControl(TicketRect, TicketBadgeWidget);
		AddControl(QuitRect, PowerButtonWidget);

		const TSharedRef<SWidget> ReservedReferenceCanvas =
			SNew(SBox)
			.WidthOverride(T66MainMenuReferenceLayout::CanvasWidth)
			.HeightOverride(GetTopBarReferenceReservedHeight())
			[
				TopBarCanvas
			];

		const float ReservedHeight = GetReservedHeight();
		const FVector2D ViewportSize = GetEffectiveFrontendViewportSize();
		const TSharedRef<SWidget> Root =
			SNew(SBox)
			.WidthOverride(FMath::Max(1.f, ViewportSize.X))
			.HeightOverride(FMath::Max(1.f, ViewportSize.Y))
			[
				SNew(SVerticalBox)
				.Visibility(EVisibility::SelfHitTestInvisible)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox)
					.HeightOverride(ReservedHeight)
					[
						SNew(SDPIScaler)
						.DPIScale(TAttribute<float>::CreateLambda([]() -> float
						{
							return 1.f / FMath::Max(0.01f, FT66Style::GetEngineDPIScale());
						}))
						[
							SNew(SScaleBox)
							.Stretch(EStretch::ScaleToFitX)
							.StretchDirection(EStretchDirection::Both)
							[
								ReservedReferenceCanvas
							]
						]
					]
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					SNullWidget::NullWidget
				]
			];

		return FT66FlatStyle::AttachMetadata(
			Root,
			TEXT("FrontendTopBar.Root"),
			TEXT("TopBarRoot"),
			ET66FlatState::Default);
	}

	CachedViewportSize = GetEffectiveFrontendViewportSize();
	bViewportResponsiveRebuildQueued = false;
	RequestTopBarAssets();
	ensureMsgf(HomeButtonBrushes.NormalBrush.IsValid(), TEXT("Main menu top bar missing home button plate."));

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const ETopBarSection ActiveSection = GetActiveSection();

	const FText SettingsText = Loc ? Loc->GetText_Settings() : NSLOCTEXT("T66.MainMenu", "Settings", "SETTINGS");
	const FText LanguageText = Loc ? Loc->GetText_LangButton() : NSLOCTEXT("T66.LanguageSelect", "LangButton", "LANG");
	const FText AccountText = Loc ? Loc->GetText_AccountStatus() : NSLOCTEXT("T66.AccountStatus", "Title", "ACCOUNT");
	const FText HomeText = NSLOCTEXT("T66.MainMenu", "Home", "CHADPOCALYPSE");
	const FText PowerUpText = NSLOCTEXT("T66.MainMenu", "PowerUp", "POWER UP");
	const FText AchievementsText = Loc ? Loc->GetText_Achievements() : NSLOCTEXT("T66.MainMenu", "Achievements", "ACHIEVEMENTS");
	const FText MiniGamesText = NSLOCTEXT("T66.MainMenu", "MiniGames", "MINIGAMES");
	const FText BackToMainMenuText = NSLOCTEXT("T66.MainMenu", "BackToMainMenu", "BACK TO MAIN MENU");
	const FText QuitTooltipText = Loc ? Loc->GetText_Quit() : NSLOCTEXT("T66.MainMenu", "Quit", "QUIT");
	const FButtonStyle& FlatButtonStyle = FT66Style::Get().GetWidgetStyle<FButtonStyle>(TEXT("T66.Button.FlatTransparent"));
	const float SurfaceWidth = T66MainMenuReferenceLayout::CanvasWidth;
	const float SurfaceHeight = GetTopBarReferenceSurfaceHeight();
	const float ReservedReferenceHeight = GetTopBarReferenceReservedHeight();
	const float LabelShadowOffset = 1.f;
	const FT66ReferenceRect& SettingsRect = T66MainMenuReferenceLayout::TopBar::ButtonSettings;
	const FT66ReferenceRect& LanguageRect = T66MainMenuReferenceLayout::TopBar::ButtonChat;
	const FT66ReferenceRect& AccountRect = T66MainMenuReferenceLayout::TopBar::TabAccount;
	const FT66ReferenceRect& PortraitRect = T66MainMenuReferenceLayout::TopBar::BadgeProfile;
	const FT66ReferenceRect& PowerUpRect = T66MainMenuReferenceLayout::TopBar::TabPowerUp;
	const FT66ReferenceRect& AchievementsRect = T66MainMenuReferenceLayout::TopBar::TabAchievements;
	const FT66ReferenceRect& MiniGamesRect = T66MainMenuReferenceLayout::TopBar::TabMinigames;
	const FT66ReferenceRect& CouponRect = T66MainMenuReferenceLayout::TopBar::CurrencySlot;
	const FT66ReferenceRect& QuitRect = T66MainMenuReferenceLayout::TopBar::ButtonPower;
	const bool bDailyDescentTopBar = UIManager && UIManager->GetCurrentScreenType() == ET66ScreenType::DailyDescent;
	const FT66ReferenceRect DailyBackRect(
		PowerUpRect.X,
		PowerUpRect.Y,
		AchievementsRect.Right() - PowerUpRect.X,
		PowerUpRect.Height);
	FT66ReferenceRect StripRect(0.f, 0.f, SurfaceWidth, SurfaceHeight);
	const float TopBarButtonHeight = FMath::Clamp(StripRect.Height - 22.f, 86.f, 96.f);
	const float UtilityIconGlyphSize = FMath::FloorToFloat(TopBarButtonHeight * 0.62f);
	const float CurrencyIconGlyphSize = FMath::FloorToFloat(TopBarButtonHeight * 0.58f);
	const FVector2D UtilityIconSize = FVector2D(UtilityIconGlyphSize, UtilityIconGlyphSize);
	const FVector2D CurrencyIconSize = FVector2D(CurrencyIconGlyphSize, CurrencyIconGlyphSize);

	auto MakeTopBarControlOffset = [&StripRect, TopBarButtonHeight](const FT66ReferenceRect& Rect, const float Width = 0.f, const float Height = 0.f) -> FMargin
	{
		const float ResolvedWidth = Width > 0.f ? Width : Rect.Width;
		const float ResolvedHeight = Height > 0.f ? Height : TopBarButtonHeight;
		const float X = Rect.X + FMath::Max(0.f, Rect.Width - ResolvedWidth) * 0.5f;
		const float Y = StripRect.Y + FMath::Max(0.f, StripRect.Height - ResolvedHeight) * 0.5f;
		return FMargin(X, Y, ResolvedWidth, ResolvedHeight);
	};

	FSlateFontInfo NavFont = FT66Style::MakeFont(TEXT("Bold"), 34);
	NavFont.LetterSpacing = 0;
	FSlateFontInfo CurrencyFont = FT66Style::MakeFont(TEXT("Bold"), 34);
	CurrencyFont.LetterSpacing = 0;

	const FLinearColor LabelColor(0.98f, 0.96f, 1.0f, 1.0f);
	const FLinearColor LabelShadowColor(0.02f, 0.0f, 0.04f, 0.98f);
	const FLinearColor AccountOuter(0.82f, 0.10f, 1.0f, 1.0f);
	const FLinearColor AccountMid(0.05f, 0.04f, 0.075f, 1.0f);
	const FLinearColor AccountInner(0.08f, 0.06f, 0.12f, 1.0f);
	const FLinearColor NavOuter(0.82f, 0.10f, 1.0f, 1.0f);
	const FLinearColor NavMid(0.05f, 0.04f, 0.075f, 1.0f);
	const FLinearColor NavInner(0.06f, 0.055f, 0.09f, 1.0f);
	const FLinearColor HomeOuter(0.92f, 0.22f, 1.0f, 1.0f);
	const FLinearColor HomeMid(0.08f, 0.035f, 0.12f, 1.0f);
	const FLinearColor HomeInner(0.16f, 0.055f, 0.22f, 1.0f);
	const FLinearColor CurrencyOuter(0.82f, 0.10f, 1.0f, 1.0f);
	const FLinearColor CurrencyMid(0.05f, 0.04f, 0.075f, 1.0f);
	const FLinearColor CurrencyInner(0.08f, 0.06f, 0.12f, 1.0f);
	const FLinearColor TransparentPlate(0.f, 0.f, 0.f, 0.f);

	auto MakeLabelWidget = [&NavFont, LabelColor, LabelShadowColor, LabelShadowOffset](const FText& Text) -> TSharedRef<SWidget>
	{
		return FT66Style::MakeRetroUIText(StaticCastSharedRef<SWidget>(SNew(STextBlock)
			.Text(Text)
			.Font(NavFont)
			.ColorAndOpacity(LabelColor)
			.ShadowOffset(FVector2D(LabelShadowOffset, LabelShadowOffset))
			.ShadowColorAndOpacity(LabelShadowColor)
			.Justification(ETextJustify::Center)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds)));
	};

	auto MakeIconWidget = [](const TSharedPtr<FSlateBrush>& Brush, const FVector2D& Size, const TSharedRef<SWidget>& Fallback) -> TSharedRef<SWidget>
	{
		if (!Brush.IsValid())
		{
			return Fallback;
		}

		return SNew(SBox)
			.WidthOverride(Size.X)
			.HeightOverride(Size.Y)
			[
				FT66Style::MakeRetroUIIcon(StaticCastSharedRef<SWidget>(
					SNew(SImage)
					.Image(Brush.Get())
					.ColorAndOpacity(FLinearColor::White)))
			];
	};

	auto MakePlateButton =
		[this, &FlatButtonStyle](
			const FPlateBrushSet& BrushSet,
			float Width,
			float Height,
			const FText& TooltipText,
			FReply (UT66FrontendTopBarWidget::*ClickFunc)(),
			const TSharedRef<SWidget>& ContentWidget,
			const FMargin& ContentPadding,
			const bool bSelected,
			const FLinearColor& OuterColor,
			const FLinearColor& MidColor,
			const FLinearColor& InnerColor) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(Width)
			.HeightOverride(Height)
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				SNew(ST66TopBarStatefulButton)
				.ButtonStyle(&FlatButtonStyle)
				.NormalBrush(BrushSet.NormalBrush.Get())
				.HoverBrush(BrushSet.HoverBrush.Get())
				.PressedBrush(BrushSet.PressedBrush.Get())
				.DisabledBrush(BrushSet.DisabledBrush.Get())
				.SelectedBrush(BrushSet.SelectedBrush.Get())
				.IsSelected(bSelected)
				.ToolTipText(TooltipText)
				.OnClicked(FOnClicked::CreateUObject(this, ClickFunc))
				.ContentPadding(ContentPadding)
				.FallbackOuterColor(OuterColor)
				.FallbackMidColor(MidColor)
				.FallbackInnerColor(InnerColor)
				[
					ContentWidget
				]
			];
	};

	const TSharedRef<SWidget> SettingsIconWidget = MakeIconWidget(
		SettingsIconBrush,
		UtilityIconSize,
		SNullWidget::NullWidget);
	const TSharedRef<SWidget> LanguageIconWidget = MakeIconWidget(
		SocialIconBrush,
		UtilityIconSize,
		SNullWidget::NullWidget);
	const TSharedRef<SWidget> CurrencyIconWidget = MakeIconWidget(
		CurrencyIconBrush,
		CurrencyIconSize,
		SNullWidget::NullWidget);
	const TSharedRef<SWidget> QuitIconWidget = MakeIconWidget(
		QuitIconBrush,
		UtilityIconSize,
		SNullWidget::NullWidget);
	const TSharedPtr<SWidget> CurrencyNavIcon = TSharedPtr<SWidget>(CurrencyIconWidget);

	auto MakeNavContent =
		[](
			const TSharedRef<SWidget>& LabelWidget,
			const TSharedPtr<SWidget>& OptionalIconWidget,
			float ButtonWidth) -> TSharedRef<SWidget>
	{
		if (!OptionalIconWidget.IsValid())
		{
			return LabelWidget;
		}

		const float ContentGap = FMath::Clamp(ButtonWidth * 0.045f, 8.f, 14.f);

		return SNew(SBox)
			.WidthOverride(ButtonWidth)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					OptionalIconWidget.ToSharedRef()
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(FMargin(ContentGap, 0.f, 0.f, 0.f))
				[
					LabelWidget
				]
			];
	};

	const TSharedRef<SWidget> SettingsButtonWidget = MakePlateButton(
		SettingsButtonBrushes,
		SettingsRect.Width,
		TopBarButtonHeight,
		SettingsText,
		&UT66FrontendTopBarWidget::HandleSettingsClicked,
		SettingsIconWidget,
		FMargin(0.f),
		ActiveSection == ETopBarSection::Settings,
		TransparentPlate,
		TransparentPlate,
		TransparentPlate);
	const TSharedRef<SWidget> LanguageButtonWidget = MakePlateButton(
		LanguageButtonBrushes,
		LanguageRect.Width,
		TopBarButtonHeight,
		LanguageText,
		&UT66FrontendTopBarWidget::HandleLanguageClicked,
		LanguageIconWidget,
		FMargin(0.f),
		ActiveSection == ETopBarSection::Language,
		TransparentPlate,
		TransparentPlate,
		TransparentPlate);
	const TSharedRef<SWidget> AccountButtonWidget = MakePlateButton(
		AccountButtonBrushes,
		AccountRect.Width,
		TopBarButtonHeight,
		AccountText,
		&UT66FrontendTopBarWidget::HandleAccountStatusClicked,
		MakeLabelWidget(AccountText),
		FMargin(0.f),
		ActiveSection == ETopBarSection::AccountStatus,
		AccountOuter,
		AccountMid,
		AccountInner);
	const float HomeButtonSquareSize = TopBarButtonHeight;
	const float HomeImageSize = FMath::FloorToFloat(TopBarButtonHeight * 0.78f);
	const TSharedRef<SWidget> HomeImageWidget = MakeIconWidget(
		HomeIconBrush,
		FVector2D(HomeImageSize, HomeImageSize),
		SNullWidget::NullWidget);
	const TSharedRef<SWidget> HomeButtonWidget =
		SNew(SBox)
		.WidthOverride(PortraitRect.Width)
		.HeightOverride(TopBarButtonHeight)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			MakePlateButton(
				HomeButtonBrushes,
				HomeButtonSquareSize,
				HomeButtonSquareSize,
				HomeText,
				&UT66FrontendTopBarWidget::HandleHomeClicked,
				HomeImageWidget,
				FMargin(0.f),
				ActiveSection == ETopBarSection::Home,
				HomeOuter,
				HomeMid,
				HomeInner)
		];
	const TSharedRef<SWidget> PowerUpButtonWidget = MakePlateButton(
		PowerUpButtonBrushes,
		PowerUpRect.Width,
		TopBarButtonHeight,
		PowerUpText,
		&UT66FrontendTopBarWidget::HandlePowerUpClicked,
		MakeLabelWidget(PowerUpText),
		FMargin(0.f),
		ActiveSection == ETopBarSection::PowerUp,
		NavOuter,
		NavMid,
		NavInner);
	const TSharedRef<SWidget> AchievementsButtonWidget = MakePlateButton(
		AchievementsButtonBrushes,
		AchievementsRect.Width,
		TopBarButtonHeight,
		AchievementsText,
		&UT66FrontendTopBarWidget::HandleAchievementsClicked,
		MakeLabelWidget(AchievementsText),
		FMargin(0.f),
		ActiveSection == ETopBarSection::Achievements,
		NavOuter,
		NavMid,
		NavInner);
	const TSharedRef<SWidget> MiniGamesButtonWidget = MakePlateButton(
		MiniGamesButtonBrushes,
		MiniGamesRect.Width,
		TopBarButtonHeight,
		MiniGamesText,
		&UT66FrontendTopBarWidget::HandleMiniGamesClicked,
		MakeLabelWidget(MiniGamesText),
		FMargin(0.f),
		ActiveSection == ETopBarSection::MiniGames,
		NavOuter,
		NavMid,
		NavInner);
	const TSharedRef<SWidget> BackToMainMenuButtonWidget = MakePlateButton(
		NavButtonBrushes,
		DailyBackRect.Width,
		TopBarButtonHeight,
		BackToMainMenuText,
		&UT66FrontendTopBarWidget::HandleHomeClicked,
		MakeLabelWidget(BackToMainMenuText),
		FMargin(0.f),
		ActiveSection == ETopBarSection::Home,
		NavOuter,
		NavMid,
		NavInner);
	const TSharedRef<SWidget> ChadCouponsWidget = MakePlateButton(
		CouponButtonBrushes,
		CouponRect.Width,
		TopBarButtonHeight,
		NSLOCTEXT("T66.PowerUp", "ChadCouponsBalanceTooltip", "Chad Coupons"),
		&UT66FrontendTopBarWidget::HandlePowerUpClicked,
		MakeNavContent(
			FT66Style::MakeRetroUIText(StaticCastSharedRef<SWidget>(SNew(STextBlock)
			.Text_Lambda([this]() -> FText
			{
				return GetChadCouponsValueText();
			})
			.Font(CurrencyFont)
			.ColorAndOpacity(LabelColor)
			.ShadowOffset(FVector2D(LabelShadowOffset, LabelShadowOffset))
			.ShadowColorAndOpacity(LabelShadowColor))),
			CurrencyNavIcon,
			CouponRect.Width),
		FMargin(0.f),
		ActiveSection == ETopBarSection::PowerUp,
		TransparentPlate,
		TransparentPlate,
		TransparentPlate);
	const TSharedRef<SWidget> QuitButtonWidget = MakePlateButton(
		QuitButtonBrushes,
		QuitRect.Width,
		TopBarButtonHeight,
		QuitTooltipText,
		&UT66FrontendTopBarWidget::HandleQuitClicked,
		QuitIconWidget,
		FMargin(0.f),
		false,
		TransparentPlate,
		TransparentPlate,
		TransparentPlate);

	TSharedRef<SConstraintCanvas> ButtonsCanvas = SNew(SConstraintCanvas);
	ButtonsCanvas->AddSlot()
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(MakeTopBarControlOffset(SettingsRect, SettingsRect.Width, TopBarButtonHeight))
		[
			SettingsButtonWidget
		];
	ButtonsCanvas->AddSlot()
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(MakeTopBarControlOffset(LanguageRect, LanguageRect.Width, TopBarButtonHeight))
		[
			LanguageButtonWidget
		];
	if (bDailyDescentTopBar)
	{
		ButtonsCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(MakeTopBarControlOffset(DailyBackRect, DailyBackRect.Width, TopBarButtonHeight))
			[
				BackToMainMenuButtonWidget
			];
	}
	else
	{
		ButtonsCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(MakeTopBarControlOffset(AccountRect, AccountRect.Width, TopBarButtonHeight))
			[
				AccountButtonWidget
			];
		ButtonsCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(MakeTopBarControlOffset(PortraitRect, PortraitRect.Width, TopBarButtonHeight))
			[
				HomeButtonWidget
			];
		ButtonsCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(MakeTopBarControlOffset(PowerUpRect, PowerUpRect.Width, TopBarButtonHeight))
			[
				PowerUpButtonWidget
			];
		ButtonsCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(MakeTopBarControlOffset(AchievementsRect, AchievementsRect.Width, TopBarButtonHeight))
			[
				AchievementsButtonWidget
			];
		ButtonsCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(MakeTopBarControlOffset(MiniGamesRect, MiniGamesRect.Width, TopBarButtonHeight))
			[
				MiniGamesButtonWidget
			];
		ButtonsCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(MakeTopBarControlOffset(CouponRect, CouponRect.Width, TopBarButtonHeight))
			[
				ChadCouponsWidget
			];
	}
	ButtonsCanvas->AddSlot()
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(MakeTopBarControlOffset(QuitRect, QuitRect.Width, TopBarButtonHeight))
		[
			QuitButtonWidget
		];

	auto MakeSurfaceBackground = []() -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(GetTopBarFrameAccent())
			.Padding(FMargin(3.f))
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(GetTopBarSurfaceFill())
				.Padding(FMargin(0.f))
			];
	};

	const TSharedRef<SWidget> Surface =
		SNew(SBox)
		.WidthOverride(SurfaceWidth)
		.HeightOverride(SurfaceHeight)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(FMargin(StripRect.X, StripRect.Y, 0.f, 0.f))
			[
				SNew(SBox)
				.WidthOverride(StripRect.Width)
				.HeightOverride(StripRect.Height)
				[
					MakeSurfaceBackground()
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				ButtonsCanvas
			]
		];

	const TSharedRef<SWidget> ReservedReferenceCanvas =
		SNew(SBox)
		.WidthOverride(T66MainMenuReferenceLayout::CanvasWidth)
		.HeightOverride(ReservedReferenceHeight)
	[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			.Padding(FMargin(0.f, T66MainMenuReferenceLayout::TopBarSurfaceOffsetY, 0.f, 0.f))
			[
				Surface
			]
		];

	const float ReservedHeight = GetReservedHeight();
	const FVector2D ViewportSize = GetEffectiveFrontendViewportSize();

	return SNew(SBox)
		.WidthOverride(FMath::Max(1.f, ViewportSize.X))
		.HeightOverride(FMath::Max(1.f, ViewportSize.Y))
		[
			SNew(SVerticalBox)
			.Visibility(EVisibility::SelfHitTestInvisible)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(ReservedHeight)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Top)
				[
					SNew(SDPIScaler)
						.DPIScale(TAttribute<float>::CreateLambda([]() -> float
						{
							return 1.f / FMath::Max(0.01f, FT66Style::GetEngineDPIScale());
						}))
						[
							SNew(SScaleBox)
							.Stretch(EStretch::ScaleToFitX)
							.StretchDirection(EStretchDirection::Both)
							[
								ReservedReferenceCanvas
							]
						]
					]
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SNullWidget::NullWidget
			]
		];
}

void UT66FrontendTopBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (UWorld* World = GetWorld())
	{
		if (World->bIsTearingDown || GExitPurge || IsGarbageCollecting())
		{
			bViewportResponsiveRebuildQueued = false;
			return;
		}
	}

	if (bViewportResponsiveRebuildQueued)
	{
		return;
	}

	const FVector2D CurrentViewportSize = GetEffectiveFrontendViewportSize();
	if (CurrentViewportSize.IsNearlyZero())
	{
		return;
	}

	if (!CurrentViewportSize.Equals(CachedViewportSize, 1.0f))
	{
		CachedViewportSize = CurrentViewportSize;
		bViewportResponsiveRebuildQueued = true;
		FT66Style::DeferRebuild(this, GTopBarViewportZOrder);
	}
}

void UT66FrontendTopBarWidget::NativeDestruct()
{
	ReleaseTopBarBrushes();
	Super::NativeDestruct();
}

void UT66FrontendTopBarWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	ReleaseTopBarBrushes();
	Super::ReleaseSlateResources(bReleaseChildren);
}

FReply UT66FrontendTopBarWidget::HandleSettingsClicked()
{
	FLagScopedScope LagScope(GetWorld(), TEXT("FE-03 FrontendTopBar::Settings"));
	NavigateWithTopBar(ET66ScreenType::Settings);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleLanguageClicked()
{
	NavigateWithTopBar(ET66ScreenType::LanguageSelect);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleHomeClicked()
{
	FLagScopedScope LagScope(GetWorld(), TEXT("FE-04 FrontendTopBar::Home"));
	NavigateWithTopBar(ET66ScreenType::MainMenu);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandlePowerUpClicked()
{
	FLagScopedScope LagScope(GetWorld(), TEXT("FE-01/FE-02 FrontendTopBar::PowerUp"));
	NavigateWithTopBar(ET66ScreenType::PowerUp);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleMiniGamesClicked()
{
	NavigateWithTopBar(ET66ScreenType::Minigames);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleAchievementsClicked()
{
	FLagScopedScope LagScope(GetWorld(), TEXT("FE-03 FrontendTopBar::Achievements"));
	NavigateWithTopBar(ET66ScreenType::Achievements);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleAccountStatusClicked()
{
	FLagScopedScope LagScope(GetWorld(), TEXT("FE-03 FrontendTopBar::AccountStatus"));
	NavigateWithTopBar(ET66ScreenType::AccountStatus);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleQuitClicked()
{
	ShowModal(ET66ScreenType::QuitConfirmation);
	return FReply::Handled();
}
