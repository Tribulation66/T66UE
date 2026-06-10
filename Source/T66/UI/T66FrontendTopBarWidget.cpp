// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66FrontendTopBarWidget.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66ShelvedFeatureGate.h"
#include "UI/T66DemoModeUIUtils.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66FriendslopStyle.h"
#include "UI/Style/T66ReferenceLayout.h"
#include "UI/Style/T66RuntimeUIFontAccess.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"

#include "Engine/Texture2D.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/FileManager.h"
#include "Misc/Parse.h"
#include "Rendering/SlateRenderer.h"

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

	int32 FitTopBarLabelFontSize(const FText& Label, const int32 PreferredSize, const int32 MinSize, const float AvailableWidth)
	{
		const FString LabelString = Label.ToString();
		if (LabelString.IsEmpty())
		{
			return PreferredSize;
		}

		const float ClampedAvailableWidth = FMath::Max(1.f, AvailableWidth);
		for (int32 FontSize = PreferredSize; FontSize >= MinSize; --FontSize)
		{
			if (FSlateApplication::IsInitialized())
			{
				FSlateRenderer* Renderer = FSlateApplication::Get().GetRenderer();
				if (Renderer)
				{
					const FVector2D MeasuredSize = Renderer->GetFontMeasureService()->Measure(LabelString, FT66FlatStyle::MakeBoldFont(FontSize));
					if (MeasuredSize.X <= ClampedAvailableWidth)
					{
						return FontSize;
					}
					continue;
				}
			}

			const float EstimatedWidth = static_cast<float>(LabelString.Len()) * static_cast<float>(FontSize) * 0.78f;
			if (EstimatedWidth <= ClampedAvailableWidth)
			{
				return FontSize;
			}
		}

		return MinSize;
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
		if (!UIManager || !UIManager->RequestFrontendRootLayerRefresh(this))
		{
			FT66Style::DeferRebuild(this, GTopBarViewportZOrder);
		}
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
		if (!UIManager || !UIManager->RequestFrontendRootLayerRefresh(this))
		{
			FT66Style::DeferRebuild(this, GTopBarViewportZOrder);
		}
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
	case ET66ScreenType::MainMenu:
	default:
		return EFrontendSection::Home;
	}
}

FText UT66FrontendTopBarWidget::GetChadCouponsValueText() const
{
	if (FParse::Param(FCommandLine::Get(), TEXT("T66FriendslopReferenceFixture")))
	{
		return FText::AsNumber(53);
	}

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
		{
			TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/topbar_coupon_ticket_icon_round09.png"),
			TEXT("RuntimeDependencies/T66/UI/Reference/Screens/MainMenu/BloodyRetro/Elements/coupon_ticket_icon.png"),
			TEXT("RuntimeDependencies/T66/UI/Icons/Flat/ticket.png")
		},
		CurrencyIconBrush,
		FVector2D(46.f, 46.f),
		TextureFilter::TF_Nearest);
	LoadLooseBrushFromCandidatePaths(
		{ TEXT("RuntimeDependencies/T66/UI/Icons/Flat/x_mark.png") },
		QuitIconBrush,
		FVector2D(54.f, 54.f),
		TextureFilter::TF_Nearest);

	TrackTopBarBrushTexture(HomeIconBrush);
	TrackTopBarBrushTexture(SettingsIconBrush);
	TrackTopBarBrushTexture(SocialIconBrush);
	TrackTopBarBrushTexture(CurrencyIconBrush);
	TrackTopBarBrushTexture(QuitIconBrush);
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

	ReleaseBrush(HomeIconBrush);
	ReleaseBrush(SettingsIconBrush);
	ReleaseBrush(SocialIconBrush);
	ReleaseBrush(CurrencyIconBrush);
	ReleaseBrush(QuitIconBrush);
	ReleaseRootedTopBarTextures();
}

void UT66FrontendTopBarWidget::TrackTopBarBrushTexture(const TSharedPtr<FSlateBrush>& Brush)
{
	if (!Brush.IsValid())
	{
		return;
	}

	UTexture2D* Texture = Cast<UTexture2D>(Brush->GetResourceObject());
	if (!Texture || !IsValid(Texture))
	{
		return;
	}

	for (const TWeakObjectPtr<UTexture2D>& RootedTexture : RootedTopBarTextures)
	{
		if (RootedTexture.Get() == Texture)
		{
			return;
		}
	}

	if (!Texture->IsRooted())
	{
		Texture->AddToRoot();
		RootedTopBarTextures.Add(Texture);
	}
}

void UT66FrontendTopBarWidget::ReleaseRootedTopBarTextures()
{
	for (const TWeakObjectPtr<UTexture2D>& RootedTexture : RootedTopBarTextures)
	{
		if (UTexture2D* Texture = RootedTexture.Get())
		{
			if (IsValid(Texture) && Texture->IsRooted())
			{
				Texture->RemoveFromRoot();
			}
		}
	}
	RootedTopBarTextures.Reset();
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

	if (!UIManager || !UIManager->RequestFrontendRootLayerRefresh(this))
	{
		FT66Style::DeferRebuild(this, GTopBarViewportZOrder);
	}
}

TSharedRef<SWidget> UT66FrontendTopBarWidget::BuildSlateUI()
{
	{
		CachedViewportSize = GetEffectiveFrontendViewportSize();
		bViewportResponsiveRebuildQueued = false;
		RequestTopBarAssets();

		UT66LocalizationSubsystem* Loc = GetLocSubsystem();
		const FText SettingsText = Loc ? Loc->GetText_Settings() : NSLOCTEXT("T66.MainMenu", "Settings", "SETTINGS");
		const FText LanguageText = Loc ? Loc->GetText_LangButton() : NSLOCTEXT("T66.LanguageSelect", "LangButton", "LANG");
		const FText AccountText = Loc ? Loc->GetText_AccountStatus() : NSLOCTEXT("T66.AccountStatus", "Title", "ACCOUNT");
		const FText HomeText = NSLOCTEXT("T66.MainMenu", "TopBarHome", "HOME");
		const FText PowerUpText = NSLOCTEXT("T66.MainMenu", "PowerUp", "POWER UP");
		const FText AchievementsText = Loc ? Loc->GetText_Achievements() : NSLOCTEXT("T66.MainMenu", "Achievements", "ACHIEVEMENTS");
		const FText BackToMainMenuText = NSLOCTEXT("T66.MainMenu", "BackToMainMenu", "BACK TO MAIN MENU");
		const FText QuitTooltipText = Loc ? Loc->GetText_Quit() : NSLOCTEXT("T66.MainMenu", "Quit", "QUIT");
		const ETopBarSection ActiveSection = GetActiveSection();

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

		// UI Reimagine pass 5: rects from the color-mask instrument readings of the
		// approved v7 reference at 1920x1080 (measure_geometry2.py / pass5_plan.md).
		const FNormalizedTopBarRect OuterRect{ 0.000f, 0.000f, 1.000f, 0.089f };
		const float TopBarControlY = 0.009f;
		const float TopBarControlH = 0.070f;
		const FNormalizedTopBarRect SettingsRect{ 0.014f, 0.012f, 0.033f, 0.065f };
		const FNormalizedTopBarRect LanguageRect{ 0.053f, 0.012f, 0.033f, 0.065f };
		const FNormalizedTopBarRect AccountRect{ 0.085f, 0.000f, 0.086f, 0.118f };
		const FNormalizedTopBarRect ProfileRect{ 0.163f, TopBarControlY, 0.150f, TopBarControlH };
		const FNormalizedTopBarRect PowerUpRect{ 0.319f, TopBarControlY, 0.102f, TopBarControlH };
		const FNormalizedTopBarRect AchievementsRect{ 0.441f, TopBarControlY, 0.117f, TopBarControlH };
		const FNormalizedTopBarRect TicketRect{ 0.586f, 0.018f, 0.067f, 0.069f };
		const FNormalizedTopBarRect QuitRect{ 0.922f, TopBarControlY, 0.068f, TopBarControlH };

		const float IconSize = 42.f;
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

		auto MakePairedIconButtonContent = [&MakeTaggedIconWidget](const FName Tag) -> TSharedRef<SWidget>
		{
			return MakeTaggedIconWidget(SNew(SBox), FVector2D(1.f, 1.f), Tag);
		};

		auto MakeIconActionButton = [this](const ET66FlatState State, const TSharedRef<SWidget>& Icon, FReply (UT66FrontendTopBarWidget::*ClickFunc)(), const FName Tag, const FNormalizedTopBarRect& Rect, const ET66FriendslopChrome Chrome) -> TSharedRef<SWidget>
		{
			return FT66FriendslopStyle::MakeToggleGroupButton(
				State,
				Icon,
				FOnClicked::CreateUObject(this, ClickFunc),
				FMargin(6.f),
				Rect.ReferenceWidth(),
				Rect.ReferenceHeight(),
				true,
				Tag,
				NAME_None,
				Chrome);
		};

		const bool bDailyDescentTopBar = UIManager && UIManager->GetCurrentScreenType() == ET66ScreenType::DailyDescent;
		if (bDailyDescentTopBar)
		{
			const FNormalizedTopBarRect DailyOuterRect{ 0.010f, 0.000f, 0.981f, 0.105f };
			const FNormalizedTopBarRect DailySettingsRect{ 0.023f, 0.016f, 0.044f, 0.073f };
			const FNormalizedTopBarRect DailyLanguageRect{ 0.083f, 0.016f, 0.045f, 0.074f };
			const FNormalizedTopBarRect DailyBackRect{ 0.322f, 0.016f, 0.347f, 0.076f };
			const FNormalizedTopBarRect DailyQuitRect{ 0.919f, 0.016f, 0.050f, 0.076f };

			const TSharedRef<SWidget> SettingsButtonWidget = MakeIconActionButton(
				ET66FlatState::Default,
				MakePairedIconButtonContent(TEXT("FrontendTopBar.SettingsButton.Icon")),
				&UT66FrontendTopBarWidget::HandleSettingsClicked,
				TEXT("FrontendTopBar.SettingsButton"),
				DailySettingsRect,
				ET66FriendslopChrome::TopbarSettingsIconButtonRound06);
			const TSharedRef<SWidget> LanguageButtonWidget = MakeIconActionButton(
				ET66FlatState::Default,
				MakeTaggedIconWidget(SNew(ST66TopBarGlobeGlyph).DesiredSize(FVector2D(IconSize, IconSize)), FVector2D(IconSize, IconSize), TEXT("FrontendTopBar.GlobeButton.Icon")),
				&UT66FrontendTopBarWidget::HandleLanguageClicked,
				TEXT("FrontendTopBar.GlobeButton"),
				DailyLanguageRect,
				ET66FriendslopChrome::TopbarIconDarkRound06);
			const TSharedRef<SWidget> BackToMainMenuButtonWidget = FT66FriendslopStyle::MakeToggleGroupButton(
				ET66FlatState::Default,
				FT66FlatStyle::MakeFlatLabel(BackToMainMenuText, ET66FlatLabelRole::Button, ETextJustify::Center, TEXT("FrontendTopBar.BackToMainMenuButton.Label")),
				FOnClicked::CreateUObject(this, &UT66FrontendTopBarWidget::HandleHomeClicked),
				FMargin(12.f, 8.f),
				DailyBackRect.ReferenceWidth(),
				DailyBackRect.ReferenceHeight(),
				true,
				TEXT("FrontendTopBar.BackToMainMenuButton"),
				NAME_None,
				ET66FriendslopChrome::TopbarTabDarkRound06);
			const TSharedRef<SWidget> PowerButtonWidget = MakeIconActionButton(
				ET66FlatState::Selected,
				MakePairedIconButtonContent(TEXT("FrontendTopBar.PowerButton.Icon")),
				&UT66FrontendTopBarWidget::HandleQuitClicked,
				TEXT("FrontendTopBar.PowerButton"),
				DailyQuitRect,
				ET66FriendslopChrome::TopbarPowerIconButtonRound06);
			PowerButtonWidget->SetToolTipText(QuitTooltipText);

			const TSharedRef<SConstraintCanvas> DailyTopBarCanvas = SNew(SConstraintCanvas);
			DailyTopBarCanvas->AddSlot()
				.Alignment(FVector2D(0.f, 0.f))
				.Offset(DailyOuterRect.ToReferenceOffset())
				[
					FT66FriendslopStyle::MakePanel(
						ET66FlatState::Default,
						FMargin(0.f),
						SNew(SBox),
						nullptr,
						TEXT("FrontendTopBar.OuterContainer"),
						ET66FriendslopChrome::TopbarStripRound06)
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
								.VAlign(VAlign_Top)
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

		// Pass 5: reference tab labels measure ~50-60px cap height — fonts scale to match.
		const FVector2D TopBarViewportSize = GetEffectiveFrontendViewportSize();
		const int32 CategoryTabFontSize = TopBarViewportSize.X <= 1366.f
			? 38
			: (TopBarViewportSize.X <= 1600.f ? 42 : 46);
		const int32 ProfileTabFontSize = TopBarViewportSize.X <= 1366.f
			? 19
			: (TopBarViewportSize.X <= 1600.f ? 21 : 23);

		auto MakeCategoryItem = [this, CategoryTabFontSize](const FText& Label, FReply (UT66FrontendTopBarWidget::*ClickFunc)(), const FName Tag, const FNormalizedTopBarRect& Rect, const bool bSelected, const bool bEnabled = true) -> FT66FlatToggleGroupItem
		{
			FT66FlatToggleGroupItem Item;
			Item.State = bEnabled ? ET66FlatState::Default : ET66FlatState::Disabled;
			Item.bIsSelected = bEnabled && bSelected;
			Item.Label = Label;
			Item.OnClicked = bEnabled ? FOnClicked::CreateUObject(this, ClickFunc) : FOnClicked();
			Item.Padding = FMargin(10.f, 7.f);
			Item.MinWidth = Rect.ReferenceWidth();
			Item.Height = Rect.ReferenceHeight();
			Item.IsEnabled = bEnabled;
			Item.FontSize = FitTopBarLabelFontSize(Label, CategoryTabFontSize, 30, Rect.ReferenceWidth() - 24.f);
			Item.Tag = Tag;
			return Item;
		};

		// UI Reimagine 2026-06-10 (approved v7): ACCOUNT tab removed (account opens via the
		// main menu profile row). Bar order: logo-home, ACHIEVEMENTS, POWER UP, MINI GAMES.
		CategoryGroup.Items.Add(MakeCategoryItem(
			AchievementsText,
			&UT66FrontendTopBarWidget::HandleAchievementsClicked,
			TEXT("FrontendTopBar.AchievementsButton"),
			ProfileRect,
			ActiveSection == ETopBarSection::Achievements));
		CategoryGroup.Items.Add(MakeCategoryItem(
			PowerUpText,
			&UT66FrontendTopBarWidget::HandlePowerUpClicked,
			TEXT("FrontendTopBar.PowerUpButton"),
			PowerUpRect,
			ActiveSection == ETopBarSection::PowerUp));
		// MINI GAMES renders as a normal pill with dimmed text (the disabled film hides
		// the plate entirely); click is a no-op until the hub ships.
		CategoryGroup.Items.Add(MakeCategoryItem(
			NSLOCTEXT("T66.MainMenu", "TopBarMiniGames", "MINI GAMES"),
			&UT66FrontendTopBarWidget::HandleMiniGamesPlaceholderClicked,
			TEXT("FrontendTopBar.MiniGamesButton"),
			AchievementsRect,
			false,
			true));
		TArray<TSharedRef<SWidget>> CategoryButtons;
		CategoryButtons.Reserve(CategoryGroup.Items.Num());
		// Per-width plates (Law 1: image-drawn plates must match the slot aspect).
		static const TCHAR* TabPlateWidths[] = { TEXT("288"), TEXT("196"), TEXT("224") };
		for (int32 ItemIndex = 0; ItemIndex < CategoryGroup.Items.Num(); ++ItemIndex)
		{
			const FT66FlatToggleGroupItem& Item = CategoryGroup.Items[ItemIndex];
			const TCHAR* PlateWidth = TabPlateWidths[FMath::Min<int32>(ItemIndex, 2)];
			const bool bSelected = Item.bIsSelected.Get(false);
			const ET66FlatState RenderState = bSelected ? ET66FlatState::Selected : Item.State;
			CategoryButtons.Add(FT66FriendslopStyle::MakeCustomToggleGroupButton(
				FString::Printf(TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/MainMenu/tab_%s_%s.png"),
					RenderState == ET66FlatState::Selected ? TEXT("selected") : TEXT("idle"),
					PlateWidth),
				FMargin(0.f),
				FVector2D(560.f, 152.f),
				RenderState,
				// Sized box forces the plate border to fill the slot (a bare text block
				// makes the border hug the label).
				SNew(SBox)
				.WidthOverride(Item.MinWidth - 20.f)
				.HeightOverride(Item.Height - 14.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Item.Label)
					.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(Item.FontSize, true))
					.ColorAndOpacity(Item.Tag == FName(TEXT("FrontendTopBar.MiniGamesButton"))
						? FSlateColor(FLinearColor(0.52f, 0.46f, 0.38f, 1.f))
						: FSlateColor(FT66FriendslopStyle::TextColorForState(RenderState)))
					.Justification(ETextJustify::Center)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				],
				Item.OnClicked,
				Item.Padding,
				Item.MinWidth,
				Item.Height,
				Item.IsEnabled,
				Item.Tag,
				CategoryGroup.GroupName));
		}

		auto MakeHellfireWellButton = [&](const ET66FlatState WellState, const TSharedRef<SWidget>& IconContent, FReply (UT66FrontendTopBarWidget::*ClickFunc)(), const FName WellTag, const FNormalizedTopBarRect& Rect) -> TSharedRef<SWidget>
		{
			return FT66FriendslopStyle::MakeCustomToggleGroupButton(
				TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/MainMenu/well_icon_round.png"),
				FMargin(0.f),
				FVector2D(120.f, 115.f),
				WellState,
				SNew(SBox)
				.WidthOverride(Rect.ReferenceWidth() - 12.f)
				.HeightOverride(Rect.ReferenceHeight() - 10.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					IconContent
				],
				FOnClicked::CreateUObject(this, ClickFunc),
				FMargin(6.f),
				Rect.ReferenceWidth(),
				Rect.ReferenceHeight(),
				true,
				WellTag,
				NAME_None,
				FLinearColor(0.10f, 0.085f, 0.12f, 1.f),
				ESlateBrushDrawType::Image);
		};
		const TSharedRef<SWidget> SettingsButtonWidget = MakeHellfireWellButton(
			ActiveSection == ETopBarSection::Settings ? ET66FlatState::Selected : ET66FlatState::Default,
			MakePairedIconButtonContent(TEXT("FrontendTopBar.SettingsButton.Icon")),
			&UT66FrontendTopBarWidget::HandleSettingsClicked,
			TEXT("FrontendTopBar.SettingsButton"),
			SettingsRect);
		const TSharedRef<SWidget> LanguageButtonWidget = MakeHellfireWellButton(
			ActiveSection == ETopBarSection::Language ? ET66FlatState::Selected : ET66FlatState::Default,
			MakeTaggedIconWidget(SNew(ST66TopBarGlobeGlyph).DesiredSize(FVector2D(IconSize, IconSize)), FVector2D(IconSize, IconSize), TEXT("FrontendTopBar.GlobeButton.Icon")),
			&UT66FrontendTopBarWidget::HandleLanguageClicked,
			TEXT("FrontendTopBar.GlobeButton"),
			LanguageRect);
		const ET66FlatState ProfileState = ActiveSection == ETopBarSection::Home ? ET66FlatState::Selected : ET66FlatState::Default;
		// Logo-as-home badge (overflows the strip like the reference).
		const float LogoBadgeSize = AccountRect.ReferenceHeight();
		const TSharedRef<SWidget> ProfileButtonWidget = FT66FriendslopStyle::MakeCustomToggleGroupButton(
			TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/MainMenu/badge_logo_home.png"),
			FMargin(0.f),
			FVector2D(166.f, 144.f),
			ProfileState,
			SNew(SBox)
			.WidthOverride(AccountRect.ReferenceWidth() - 14.f)
			.HeightOverride(AccountRect.ReferenceHeight() - 10.f),
			FOnClicked::CreateUObject(this, &UT66FrontendTopBarWidget::HandleHomeClicked),
			FMargin(0.f),
			LogoBadgeSize,
			LogoBadgeSize,
			true,
			TEXT("FrontendTopBar.ProfileButton"),
			NAME_None,
			FLinearColor(0.62f, 0.04f, 0.075f, 1.f),
			ESlateBrushDrawType::Image);
		const TSharedRef<SWidget> PowerButtonWidget = FT66FriendslopStyle::MakeCustomToggleGroupButton(
			TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/MainMenu/power_well.png"),
			FMargin(0.f),
			FVector2D(340.f, 140.f),
			ET66FlatState::Selected,
			SNew(SBox)
			.WidthOverride(QuitRect.ReferenceWidth() - 16.f)
			.HeightOverride(QuitRect.ReferenceHeight() - 12.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				MakePairedIconButtonContent(TEXT("FrontendTopBar.PowerButton.Icon"))
			],
			FOnClicked::CreateUObject(this, &UT66FrontendTopBarWidget::HandleQuitClicked),
			FMargin(8.f, 6.f),
			QuitRect.ReferenceWidth(),
			QuitRect.ReferenceHeight(),
			true,
			TEXT("FrontendTopBar.PowerButton"),
			NAME_None,
			FLinearColor(0.62f, 0.04f, 0.075f, 1.f),
			ESlateBrushDrawType::Image);
		PowerButtonWidget->SetToolTipText(QuitTooltipText);

		const TSharedRef<SWidget> TicketValue = FT66FlatStyle::AttachMetadata(
			SNew(STextBlock)
			.Text(TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateUObject(this, &UT66FrontendTopBarWidget::GetChadCouponsValueText)))
			.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(28, true))
			.ColorAndOpacity(FT66FlatStyle::PrimaryText())
			.Justification(ETextJustify::Center),
			TEXT("FrontendTopBar.TicketBadge.Value"),
			TEXT("Label.Button"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true);
		const TSharedRef<SWidget> TicketContent =
			SNew(SBox)
			.WidthOverride(FMath::Max(1.f, TicketRect.ReferenceWidth() - 16.f))
			.HeightOverride(FMath::Max(1.f, TicketRect.ReferenceHeight() - 12.f))
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			.Padding(FMargin(0.f, 0.f, 14.f, 0.f))
			[
				TicketValue
			];
		// Naked coupon indicator (approved v7): gold ticket icon + value on the bar's black,
		// no plate, not a button.
		const TSharedRef<SWidget> TicketBadgeWidget = FT66FlatStyle::AttachMetadata(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 10.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(48.f)
				.HeightOverride(48.f)
				[
					SNew(SImage)
					.Image(FT66FriendslopStyle::GetCustomBrush(
						TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/MainMenu/ic_ticket.png"),
						FMargin(0.f),
						ESlateBrushDrawType::Image,
						FVector2D(40.f, 40.f)))
					.ColorAndOpacity(FLinearColor::White)
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				TicketValue
			],
			TEXT("FrontendTopBar.TicketBadge"),
			TEXT("CouponIndicator"),
			ET66FlatState::Default);

		const TSharedRef<SConstraintCanvas> TopBarCanvas = SNew(SConstraintCanvas);
		TopBarCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(OuterRect.ToReferenceOffset())
			[
				FT66FriendslopStyle::MakeCustomSurface(
					TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/MainMenu/bar_strip.png"),
					FMargin(0.f),
					ESlateBrushDrawType::Image,
					FVector2D(1920.f, 76.f),
					ET66FlatState::Default,
					FMargin(0.f),
					SNew(SBox),
					nullptr,
					TEXT("FrontendTopBar.OuterContainer"),
					TEXT("Panel"))
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
		AddControl(AccountRect, ProfileButtonWidget);
		AddControl(ProfileRect, CategoryButtons[0]);
		AddControl(PowerUpRect, CategoryButtons[1]);
		AddControl(AchievementsRect, CategoryButtons[2]);
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
							.VAlign(VAlign_Top)
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
		UT66UIManager* RuntimeUIManager = UIManager.Get();
		if (!RuntimeUIManager || !RuntimeUIManager->RequestFrontendRootLayerRefresh(this))
		{
			FT66Style::DeferRebuild(this, GTopBarViewportZOrder);
		}
	}
}

void UT66FrontendTopBarWidget::NativeDestruct()
{
	ReleaseTopBarBrushes();
	Super::NativeDestruct();
}

void UT66FrontendTopBarWidget::ReleaseSlateResources(bool bReleaseChildren)
{
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

FReply UT66FrontendTopBarWidget::HandleAchievementsClicked()
{
	FLagScopedScope LagScope(GetWorld(), TEXT("FE-03 FrontendTopBar::Achievements"));
	NavigateWithTopBar(ET66ScreenType::Achievements);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleMiniGamesPlaceholderClicked()
{
	// Mini games hub not built yet (UI Reimagine punch list) — pill is visible per the
	// approved reference but clicking does nothing.
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
