// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66FrontendTopBarWidget.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "UI/ST66PulsingIcon.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66UIManager.h"
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
			const float StartAngle = -1.10f * PI;
			const float EndAngle = 0.10f * PI;
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
				FLinearColor(0.22f, 0.12f, 0.08f, 1.f),
				true,
				OuterThickness);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				ArcPoints,
				ESlateDrawEffect::None,
				FLinearColor(0.96f, 0.57f, 0.22f, 1.f),
				true,
				FMath::Max(3.f, OuterThickness - 3.f));

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 2,
				AllottedGeometry.ToPaintGeometry(),
				ArcPoints,
				ESlateDrawEffect::None,
				FLinearColor(1.f, 0.92f, 0.74f, 1.f),
				true,
				InnerThickness);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 3,
				AllottedGeometry.ToPaintGeometry(),
				StemPoints,
				ESlateDrawEffect::None,
				FLinearColor(0.22f, 0.12f, 0.08f, 1.f),
				true,
				FMath::Max(8.f, MinDim * 0.18f));

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 4,
				AllottedGeometry.ToPaintGeometry(),
				StemPoints,
				ESlateDrawEffect::None,
				FLinearColor(1.f, 0.92f, 0.74f, 1.f),
				true,
				FMath::Max(4.f, MinDim * 0.09f));

			return LayerId + 5;
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
							FT66Style::MakeRetroUIChromeSurface(BuildPlateWidget())
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
	if (!UIManager)
	{
		return ETopBarSection::Home;
	}

	ET66ScreenType FocusedScreenType = UIManager->GetCurrentScreenType();
	if (UIManager->IsModalActive() && UIManager->GetCurrentModalType() == ET66ScreenType::AccountStatus)
	{
		FocusedScreenType = ET66ScreenType::AccountStatus;
	}

	switch (FocusedScreenType)
	{
	case ET66ScreenType::AccountStatus:
		return ETopBarSection::AccountStatus;
	case ET66ScreenType::Settings:
		return ETopBarSection::Settings;
	case ET66ScreenType::LanguageSelect:
		return ETopBarSection::Language;
	case ET66ScreenType::PowerUp:
		return ETopBarSection::PowerUp;
	case ET66ScreenType::Achievements:
		return ETopBarSection::Achievements;
	case ET66ScreenType::Minigames:
		return ETopBarSection::MiniGames;
	case ET66ScreenType::MainMenu:
	default:
		return ETopBarSection::Home;
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
	const FString IconButtonNormalPath = T66ScreenSlateHelpers::MakeReferenceChromeElementAssetPath(TEXT("topbar_icon_button_normal_square_variant.png"));
	const FString IconButtonHoverPath = T66ScreenSlateHelpers::MakeReferenceChromeElementAssetPath(TEXT("topbar_icon_button_hover_square_variant.png"));
	const FString IconButtonPressedPath = T66ScreenSlateHelpers::MakeReferenceChromeElementAssetPath(TEXT("topbar_icon_button_pressed_square_variant.png"));
	const FString IconButtonDisabledPath = T66ScreenSlateHelpers::MakeReferenceChromeElementAssetPath(TEXT("topbar_icon_button_disabled_square_variant.png"));
	const FString IconButtonSelectedPath = T66ScreenSlateHelpers::MakeReferenceChromeElementAssetPath(TEXT("topbar_icon_button_selected_square_variant.png"));
	const FString TextButtonNormalPath = T66ScreenSlateHelpers::MakeReferenceChromeElementAssetPath(TEXT("cta_new_game_button_normal_square_variant.png"));
	const FString TextButtonHoverPath = T66ScreenSlateHelpers::MakeReferenceChromeElementAssetPath(TEXT("cta_new_game_button_hover_square_variant.png"));
	const FString TextButtonPressedPath = T66ScreenSlateHelpers::MakeReferenceChromeElementAssetPath(TEXT("cta_new_game_button_pressed_square_variant.png"));
	const FString TextButtonDisabledPath = T66ScreenSlateHelpers::MakeReferenceChromeElementAssetPath(TEXT("cta_new_game_button_disabled_square_variant.png"));
	const FString TextButtonSelectedPath = TextButtonHoverPath;
	const FString CurrencyButtonNormalPath = TextButtonNormalPath;
	const FString CurrencyButtonHoverPath = TextButtonHoverPath;
	const FString CurrencyButtonPressedPath = TextButtonPressedPath;
	const FString CurrencyButtonDisabledPath = TextButtonDisabledPath;

	LoadLooseBrushFromCandidatePaths(
		{
			T66ScreenSlateHelpers::MakeReferenceChromeElementAssetPath(TEXT("main_panel_normal_square_variant.png"))
		},
		TopBarBackdropBrush,
		FVector2D(T66MainMenuReferenceLayout::TopBar::TopbarStripFull.Width, T66MainMenuReferenceLayout::TopBar::TopbarStripFull.Height));

	LoadButtonStateSetFromPaths(
		{ IconButtonNormalPath },
		{ IconButtonSelectedPath, IconButtonHoverPath },
		{ IconButtonPressedPath },
		{ IconButtonDisabledPath },
		{ IconButtonSelectedPath, IconButtonHoverPath },
		SettingsButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ IconButtonNormalPath },
		{ IconButtonSelectedPath, IconButtonHoverPath },
		{ IconButtonPressedPath },
		{ IconButtonDisabledPath },
		{ IconButtonSelectedPath, IconButtonHoverPath },
		LanguageButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ TextButtonNormalPath },
		{ TextButtonHoverPath },
		{ TextButtonPressedPath },
		{ TextButtonDisabledPath },
		{ TextButtonSelectedPath, TextButtonPressedPath },
		AccountButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ IconButtonNormalPath },
		{ IconButtonHoverPath },
		{ IconButtonPressedPath },
		{ IconButtonDisabledPath },
		{ IconButtonHoverPath },
		HomeButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ TextButtonNormalPath },
		{ TextButtonHoverPath },
		{ TextButtonPressedPath },
		{ TextButtonDisabledPath },
		{ TextButtonSelectedPath, TextButtonPressedPath },
		NavButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ TextButtonNormalPath },
		{ TextButtonHoverPath },
		{ TextButtonPressedPath },
		{ TextButtonDisabledPath },
		{ TextButtonSelectedPath, TextButtonPressedPath },
		PowerUpButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ TextButtonNormalPath },
		{ TextButtonHoverPath },
		{ TextButtonPressedPath },
		{ TextButtonDisabledPath },
		{ TextButtonSelectedPath, TextButtonPressedPath },
		AchievementsButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ TextButtonNormalPath },
		{ TextButtonHoverPath },
		{ TextButtonPressedPath },
		{ TextButtonDisabledPath },
		{ TextButtonSelectedPath, TextButtonPressedPath },
		MiniGamesButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ IconButtonNormalPath },
		{ IconButtonHoverPath },
		{ IconButtonPressedPath },
		{ IconButtonDisabledPath },
		{ IconButtonHoverPath },
		PortraitButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ CurrencyButtonNormalPath },
		{ CurrencyButtonHoverPath },
		{ TextButtonSelectedPath, CurrencyButtonPressedPath },
		{ CurrencyButtonDisabledPath },
		{ CurrencyButtonPressedPath },
		CouponButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ IconButtonNormalPath },
		{ IconButtonSelectedPath, IconButtonHoverPath },
		{ IconButtonPressedPath },
		{ IconButtonDisabledPath },
		{ IconButtonHoverPath },
		QuitButtonBrushes);

	const FMargin SquareMargin(0.105f, 0.105f, 0.105f, 0.105f);
	const FMargin NavMargin(0.060f, 0.155f, 0.060f, 0.155f);
	const FMargin CurrencyMargin(0.075f, 0.155f, 0.075f, 0.155f);
	const FMargin IconMargin(0.105f, 0.105f, 0.105f, 0.105f);
	ConfigureBoxBrushSet(SettingsButtonBrushes, IconMargin);
	ConfigureBoxBrushSet(LanguageButtonBrushes, IconMargin);
	ConfigureBoxBrushSet(AccountButtonBrushes, NavMargin);
	ConfigureBoxBrushSet(HomeButtonBrushes, SquareMargin);
	ConfigureBoxBrushSet(NavButtonBrushes, NavMargin);
	ConfigureBoxBrushSet(PowerUpButtonBrushes, NavMargin);
	ConfigureBoxBrushSet(AchievementsButtonBrushes, NavMargin);
	ConfigureBoxBrushSet(MiniGamesButtonBrushes, NavMargin);
	ConfigureBoxBrushSet(PortraitButtonBrushes, SquareMargin);
	ConfigureBoxBrushSet(CouponButtonBrushes, CurrencyMargin);
	ConfigureBoxBrushSet(QuitButtonBrushes, IconMargin);
	ConfigureBoxBrush(TopBarBackdropBrush, FMargin(0.f));
	ResetBrushSet(SettingsButtonBrushes);
	ResetBrushSet(LanguageButtonBrushes);
	ResetBrushSet(CouponButtonBrushes);
	ResetBrushSet(QuitButtonBrushes);

	LoadLooseBrushFromCandidatePaths(
			{
			T66ScreenSlateHelpers::MakeReferenceMainMenuElementAssetPath(TEXT("home_profile_icon.png"))
		},
		HomeIconBrush,
		FVector2D(88.f, 88.f));
	LoadLooseBrushFromCandidatePaths(
			{
			T66ScreenSlateHelpers::MakeReferenceMainMenuElementAssetPath(TEXT("settings_gear_icon.png"))
		},
		SettingsIconBrush,
		FVector2D(64.f, 64.f));
	LoadLooseBrushFromCandidatePaths(
			{
			T66ScreenSlateHelpers::MakeReferenceMainMenuElementAssetPath(TEXT("language_globe_icon.png"))
		},
		SocialIconBrush,
		FVector2D(64.f, 64.f));
	LoadLooseBrushFromCandidatePaths(
			{
			T66ScreenSlateHelpers::MakeReferenceMainMenuElementAssetPath(TEXT("coupon_ticket_icon.png"))
		},
		CurrencyIconBrush,
		FVector2D(58.f, 58.f));
	LoadLooseBrushFromCandidatePaths(
			{
			T66ScreenSlateHelpers::MakeReferenceMainMenuElementAssetPath(TEXT("power_off_icon.png"))
		},
		QuitIconBrush,
		FVector2D(64.f, 64.f));
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
