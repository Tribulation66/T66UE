// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66FrontendTopBarWidget.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "UI/ST66PulsingIcon.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66ReferenceLayout.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"

#include "Engine/Texture2D.h"
#include "HAL/FileManager.h"

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
			, _ButtonStyle(nullptr)
			, _ContentPadding(FMargin(0.f))
			, _FallbackOuterColor(FLinearColor::White)
			, _FallbackMidColor(FLinearColor::White)
			, _FallbackInnerColor(FLinearColor::White)
		{}
			SLATE_ARGUMENT(const FSlateBrush*, NormalBrush)
			SLATE_ARGUMENT(const FSlateBrush*, HoverBrush)
			SLATE_ARGUMENT(const FSlateBrush*, PressedBrush)
			SLATE_ARGUMENT(const FSlateBrush*, DisabledBrush)
			SLATE_ARGUMENT(const FButtonStyle*, ButtonStyle)
			SLATE_ARGUMENT(FMargin, ContentPadding)
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
			ContentPadding = InArgs._ContentPadding;
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
				SAssignNew(Button, SButton)
				.ButtonStyle(&ButtonStyle)
				.ContentPadding(FMargin(0.f))
				.ToolTipText(InArgs._ToolTipText)
				.OnClicked(InArgs._OnClicked)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor::Transparent)
					.Padding(0.f)
					.RenderTransform(this, &ST66TopBarStatefulButton::GetVisualTransform)
					.RenderTransformPivot(FVector2D(0.5f, 0.5f))
					[
						SNew(SOverlay)
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
							[
								InArgs._Content.Widget
							]
						]
					]
				]
			];
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

			if (Button->IsHovered() && HoverBrush)
			{
				return HoverBrush;
			}

			return NormalBrush;
		}

		TSharedRef<SWidget> BuildPlateWidget()
		{
			if (NormalBrush || HoverBrush || PressedBrush || DisabledBrush)
			{
				return SNew(SImage)
					.Image(this, &ST66TopBarStatefulButton::GetCurrentBrush);
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
		FButtonStyle ButtonStyle;
		FMargin ContentPadding = FMargin(0.f);
		FLinearColor FallbackOuterColor = FLinearColor::White;
		FLinearColor FallbackMidColor = FLinearColor::White;
		FLinearColor FallbackInnerColor = FLinearColor::White;
		TSharedPtr<SButton> Button;
	};

	FVector2D GetEffectiveFrontendViewportSize()
	{
		return FT66Style::GetViewportSize();
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
	}

	void CopyBrushAsButtonStateSet(
		const TSharedPtr<FSlateBrush>& SourceBrush,
		UT66FrontendTopBarWidget::FPlateBrushSet& OutSet)
	{
		OutSet.NormalBrush = SourceBrush;
		OutSet.HoverBrush = SourceBrush;
		OutSet.PressedBrush = SourceBrush;
		OutSet.DisabledBrush = SourceBrush;
	}

	void LoadButtonStateSetFromPaths(
		const TArray<FString>& NormalPaths,
		const TArray<FString>& HoverPaths,
		const TArray<FString>& PressedPaths,
		const TArray<FString>& DisabledPaths,
		UT66FrontendTopBarWidget::FPlateBrushSet& OutSet,
		const FVector2D& DesiredSize = FVector2D::ZeroVector)
	{
		LoadLooseBrushFromCandidatePaths(NormalPaths, OutSet.NormalBrush, DesiredSize);
		LoadLooseBrushFromCandidatePaths(HoverPaths, OutSet.HoverBrush, DesiredSize);
		LoadLooseBrushFromCandidatePaths(PressedPaths, OutSet.PressedBrush, DesiredSize);
		LoadLooseBrushFromCandidatePaths(DisabledPaths, OutSet.DisabledBrush, DesiredSize);

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
	}

	void ConfigureBoxBrush(const TSharedPtr<FSlateBrush>& Brush, const FMargin& Margin)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		Brush->DrawAs = ESlateBrushDrawType::Box;
		Brush->Margin = Margin;
	}

	TSharedRef<SWidget> MakeWarmFallbackGlyph(const FText& Text, int32 FontSize)
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(FT66Style::MakeFont(TEXT("Bold"), FontSize))
			.ColorAndOpacity(FLinearColor(0.96f, 0.89f, 0.73f, 1.0f))
			.ShadowOffset(FVector2D(1.f, 1.f))
			.ShadowColorAndOpacity(FLinearColor(0.11f, 0.06f, 0.03f, 0.95f));
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
	return SnapPixel(GetTopBarViewportTransform().MapY(T66MainMenuReferenceLayout::TopBarReservedHeight));
}

float UT66FrontendTopBarWidget::GetVisibleContentHeight()
{
	return SnapPixel(GetTopBarViewportTransform().MapY(T66MainMenuReferenceLayout::TopBarSurfaceHeight));
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
	case ET66ScreenType::PowerUp:
		return ETopBarSection::PowerUp;
	case ET66ScreenType::Achievements:
		return ETopBarSection::Achievements;
	case ET66ScreenType::Unlocks:
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
	LoadLooseBrushFromCandidatePaths(
		{
			TEXT("SourceAssets/UI/MainMenuReference/topbar_shell.png"),
			TEXT("SourceAssets/UI/MainMenuReference/topbar_backdrop.png"),
			TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_shell.png")
		},
		TopBarBackdropBrush);

	LoadButtonStateSetFromPaths(
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/button_settings.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/button_settings.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/button_settings.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/button_settings.png") },
		SettingsButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/button_chat.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/button_chat.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/button_chat.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/button_chat.png") },
		LanguageButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_account.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_account_hover.png"), TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_account.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_account_pressed.png"), TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_account.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_account.png") },
		AccountButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_home.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_home_active.png"), TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_home.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_home_active.png"), TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_home.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_home.png") },
		HomeButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ TEXT("SourceAssets/UI/MainMenuReference/SheetSlices/TopBar/nav_normal.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/SheetSlices/TopBar/nav_hover.png"), TEXT("SourceAssets/UI/MainMenuReference/SheetSlices/TopBar/nav_normal.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/SheetSlices/TopBar/nav_pressed.png"), TEXT("SourceAssets/UI/MainMenuReference/SheetSlices/TopBar/nav_normal.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/SheetSlices/TopBar/nav_disabled.png"), TEXT("SourceAssets/UI/MainMenuReference/SheetSlices/TopBar/nav_normal.png") },
		NavButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_power_up.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_power_up_hover.png"), TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_power_up.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_power_up_pressed.png"), TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_power_up.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_power_up.png") },
		PowerUpButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_achievements.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_achievements_hover.png"), TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_achievements.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_achievements_pressed.png"), TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_achievements.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_achievements.png") },
		AchievementsButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_minigames.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_minigames_hover.png"), TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_minigames.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_minigames_pressed.png"), TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_minigames.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/topbar_button_nav_minigames.png") },
		MiniGamesButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/badge_profile.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/badge_profile.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/badge_profile.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/badge_profile.png") },
		PortraitButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/currency_slot_blank.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/currency_slot_blank.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/currency_slot_blank.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/currency_slot_blank.png") },
		CouponButtonBrushes);

	LoadButtonStateSetFromPaths(
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/button_power.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/button_power.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/button_power.png") },
		{ TEXT("SourceAssets/UI/MainMenuReference/TopBar/button_power.png") },
		QuitButtonBrushes);

	ConfigureBoxBrush(NavButtonBrushes.NormalBrush, FMargin(0.11f, 0.18f, 0.11f, 0.28f));
	ConfigureBoxBrush(NavButtonBrushes.HoverBrush, FMargin(0.11f, 0.18f, 0.11f, 0.28f));
	ConfigureBoxBrush(NavButtonBrushes.PressedBrush, FMargin(0.11f, 0.18f, 0.11f, 0.28f));
	ConfigureBoxBrush(NavButtonBrushes.DisabledBrush, FMargin(0.11f, 0.18f, 0.11f, 0.28f));

	LoadLooseBrushFromCandidatePaths(
		{
			TEXT("SourceAssets/UI/MainMenuReference/icon_powerup.png"),
			TEXT("SourceAssets/UI/MainMenuReference/TopBar/icon_powerup.png")
		},
		PowerUpIconBrush,
		FVector2D(28.f, 28.f));
	LoadLooseBrushFromCandidatePaths(
		{
			TEXT("SourceAssets/UI/MainMenuReference/icon_achievements.png"),
			TEXT("SourceAssets/UI/MainMenuReference/TopBar/icon_achievements.png")
		},
		AchievementsIconBrush,
		FVector2D(30.f, 30.f));
	LoadLooseBrushFromCandidatePaths(
		{
			TEXT("SourceAssets/UI/MainMenuReference/icon_minigames.png"),
			TEXT("SourceAssets/UI/MainMenuReference/TopBar/icon_minigames.png")
		},
		MiniGamesIconBrush,
		FVector2D(30.f, 30.f));
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
	ReleaseBrush(SettingsIconBrush);
	ReleaseBrush(SocialIconBrush);
	ReleaseBrush(ProfileIconBrush);
	ReleaseBrush(PowerUpIconBrush);
	ReleaseBrush(AchievementsIconBrush);
	ReleaseBrush(MiniGamesIconBrush);
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
	const FText QuitTooltipText = Loc ? Loc->GetText_Quit() : NSLOCTEXT("T66.MainMenu", "Quit", "QUIT");
	const FButtonStyle& FlatButtonStyle = FT66Style::Get().GetWidgetStyle<FButtonStyle>(TEXT("T66.Button.FlatTransparent"));
	const float SurfaceWidth = T66MainMenuReferenceLayout::CanvasWidth;
	const float SurfaceHeight = T66MainMenuReferenceLayout::TopBarSurfaceHeight;
	const FVector2D NavIconSize = FVector2D(28.f, 28.f);
	const float LabelIconGap = 10.f;
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

	FSlateFontInfo NavFont = FT66Style::MakeFont(TEXT("Bold"), 18);
	NavFont.LetterSpacing = 2;
	FSlateFontInfo CurrencyFont = FT66Style::MakeFont(TEXT("Bold"), 19);
	CurrencyFont.LetterSpacing = 2;

	const FLinearColor LabelColor(0.96f, 0.90f, 0.75f, 1.0f);
	const FLinearColor LabelShadowColor(0.12f, 0.07f, 0.03f, 0.98f);
	const FLinearColor BarFallbackOuter(0.17f, 0.12f, 0.08f, 1.0f);
	const FLinearColor BarFallbackInner(0.22f, 0.16f, 0.11f, 1.0f);
	const FLinearColor AccountOuter(0.63f, 0.47f, 0.24f, 1.0f);
	const FLinearColor AccountMid(0.16f, 0.12f, 0.10f, 1.0f);
	const FLinearColor AccountInner(0.20f, 0.29f, 0.35f, 1.0f);
	const FLinearColor NavOuter(0.63f, 0.47f, 0.24f, 1.0f);
	const FLinearColor NavMid(0.16f, 0.12f, 0.10f, 1.0f);
	const FLinearColor NavInner(0.22f, 0.23f, 0.25f, 1.0f);
	const FLinearColor HomeOuter(0.82f, 0.62f, 0.24f, 1.0f);
	const FLinearColor HomeMid(0.31f, 0.19f, 0.08f, 1.0f);
	const FLinearColor HomeInner(0.72f, 0.54f, 0.17f, 1.0f);
	const FLinearColor CurrencyOuter(0.73f, 0.54f, 0.24f, 1.0f);
	const FLinearColor CurrencyMid(0.20f, 0.11f, 0.07f, 1.0f);
	const FLinearColor CurrencyInner(0.32f, 0.21f, 0.12f, 1.0f);

	auto MakeLabelWidget = [&NavFont, LabelColor, LabelShadowColor, LabelShadowOffset](const FText& Text) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(NavFont)
			.ColorAndOpacity(LabelColor)
			.ShadowOffset(FVector2D(LabelShadowOffset, LabelShadowOffset))
			.ShadowColorAndOpacity(LabelShadowColor)
			.Justification(ETextJustify::Center)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds);
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
				SNew(SImage)
				.Image(Brush.Get())
				.ColorAndOpacity(FLinearColor::White)
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
			const FLinearColor& OuterColor,
			const FLinearColor& MidColor,
			const FLinearColor& InnerColor) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(Width)
			.HeightOverride(Height)
			[
				SNew(ST66TopBarStatefulButton)
				.ButtonStyle(&FlatButtonStyle)
				.NormalBrush(BrushSet.NormalBrush.Get())
				.HoverBrush(BrushSet.HoverBrush.Get())
				.PressedBrush(BrushSet.PressedBrush.Get())
				.DisabledBrush(BrushSet.DisabledBrush.Get())
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

	const TSharedRef<SWidget> PowerUpIconWidget = MakeIconWidget(
		PowerUpIconBrush,
		NavIconSize,
		MakeWarmFallbackGlyph(NSLOCTEXT("T66.MainMenu", "TopBarPowerFallback", "PU"), 12));
	const TSharedRef<SWidget> AchievementsIconWidget = MakeIconWidget(
		AchievementsIconBrush,
		NavIconSize,
		MakeWarmFallbackGlyph(NSLOCTEXT("T66.MainMenu", "TopBarAchievementsFallback", "A"), 14));
	const TSharedRef<SWidget> MiniGamesIconWidget = MakeIconWidget(
		MiniGamesIconBrush,
		NavIconSize,
		MakeWarmFallbackGlyph(NSLOCTEXT("T66.MainMenu", "TopBarMiniGamesFallback", "M"), 14));
	const TSharedPtr<SWidget> PowerUpNavIcon = TSharedPtr<SWidget>(PowerUpIconWidget);
	const TSharedPtr<SWidget> AchievementsNavIcon = TSharedPtr<SWidget>(AchievementsIconWidget);
	const TSharedPtr<SWidget> MiniGamesNavIcon = TSharedPtr<SWidget>(MiniGamesIconWidget);

	auto MakeNavContent =
		[&MakeLabelWidget, LabelIconGap](
			const TSharedRef<SWidget>& LabelWidget,
			const TSharedPtr<SWidget>& OptionalIconWidget) -> TSharedRef<SWidget>
	{
		if (!OptionalIconWidget.IsValid())
		{
			return LabelWidget;
		}

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, LabelIconGap, 0.f)
			[
				OptionalIconWidget.ToSharedRef()
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				LabelWidget
			];
	};

	const TSharedRef<SWidget> SettingsButtonWidget = MakePlateButton(
		SettingsButtonBrushes,
		SettingsRect.Width,
		SettingsRect.Height,
		SettingsText,
		&UT66FrontendTopBarWidget::HandleSettingsClicked,
		SNullWidget::NullWidget,
		FMargin(0.f),
		AccountOuter,
		AccountMid,
		AccountInner);
	const TSharedRef<SWidget> LanguageButtonWidget = MakePlateButton(
		LanguageButtonBrushes,
		LanguageRect.Width,
		LanguageRect.Height,
		LanguageText,
		&UT66FrontendTopBarWidget::HandleLanguageClicked,
		SNullWidget::NullWidget,
		FMargin(0.f),
		AccountOuter,
		AccountMid,
		AccountInner);
	const TSharedRef<SWidget> AccountButtonWidget = MakePlateButton(
		AccountButtonBrushes,
		AccountRect.Width,
		AccountRect.Height,
		AccountText,
		&UT66FrontendTopBarWidget::HandleAccountStatusClicked,
		MakeLabelWidget(AccountText),
		FMargin(22.f, 22.f, 22.f, 24.f),
		AccountOuter,
		AccountMid,
		AccountInner);
	const TSharedRef<SWidget> HomeButtonWidget = MakePlateButton(
		HomeButtonBrushes,
		PortraitRect.Width,
		PortraitRect.Height,
		HomeText,
		&UT66FrontendTopBarWidget::HandleHomeClicked,
		SNullWidget::NullWidget,
		FMargin(0.f),
		HomeOuter,
		HomeMid,
		HomeInner);
	const TSharedRef<SWidget> PowerUpButtonWidget = MakePlateButton(
		PowerUpButtonBrushes,
		PowerUpRect.Width,
		PowerUpRect.Height,
		PowerUpText,
		&UT66FrontendTopBarWidget::HandleShopClicked,
		MakeNavContent(MakeLabelWidget(PowerUpText), PowerUpNavIcon),
		FMargin(22.f, 20.f, 22.f, 24.f),
		NavOuter,
		NavMid,
		NavInner);
	const TSharedRef<SWidget> AchievementsButtonWidget = MakePlateButton(
		AchievementsButtonBrushes,
		AchievementsRect.Width,
		AchievementsRect.Height,
		AchievementsText,
		&UT66FrontendTopBarWidget::HandleAchievementsClicked,
		MakeNavContent(MakeLabelWidget(AchievementsText), AchievementsNavIcon),
		FMargin(22.f, 20.f, 22.f, 24.f),
		NavOuter,
		NavMid,
		NavInner);
	const TSharedRef<SWidget> MiniGamesButtonWidget = MakePlateButton(
		MiniGamesButtonBrushes,
		MiniGamesRect.Width,
		MiniGamesRect.Height,
		MiniGamesText,
		&UT66FrontendTopBarWidget::HandleMiniGamesClicked,
		MakeNavContent(MakeLabelWidget(MiniGamesText), MiniGamesNavIcon),
		FMargin(22.f, 20.f, 22.f, 24.f),
		NavOuter,
		NavMid,
		NavInner);
	const TSharedRef<SWidget> ChadCouponsWidget = MakePlateButton(
		CouponButtonBrushes,
		CouponRect.Width,
		CouponRect.Height,
		NSLOCTEXT("T66.Shop", "ChadCouponsBalanceTooltip", "Chad Coupons"),
		&UT66FrontendTopBarWidget::HandleShopClicked,
		SNew(SBox)
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() -> FText
			{
				return GetChadCouponsValueText();
			})
			.Font(CurrencyFont)
			.ColorAndOpacity(LabelColor)
			.ShadowOffset(FVector2D(LabelShadowOffset, LabelShadowOffset))
			.ShadowColorAndOpacity(LabelShadowColor)
		],
		FMargin(84.f, 18.f, 20.f, 18.f),
		CurrencyOuter,
		CurrencyMid,
		CurrencyInner);
	const TSharedRef<SWidget> QuitButtonWidget = MakePlateButton(
		QuitButtonBrushes,
		QuitRect.Width,
		QuitRect.Height,
		QuitTooltipText,
		&UT66FrontendTopBarWidget::HandleQuitClicked,
		SNullWidget::NullWidget,
		FMargin(0.f),
		HomeOuter,
		HomeMid,
		HomeInner);

	const TSharedRef<SWidget> ButtonsCanvas =
		SNew(SConstraintCanvas)
		+ SConstraintCanvas::Slot().Offset(FMargin(SettingsRect.X, SettingsRect.Y, SettingsRect.Width, SettingsRect.Height))
		[
			SettingsButtonWidget
		]
		+ SConstraintCanvas::Slot().Offset(FMargin(LanguageRect.X, LanguageRect.Y, LanguageRect.Width, LanguageRect.Height))
		[
			LanguageButtonWidget
		]
		+ SConstraintCanvas::Slot().Offset(FMargin(AccountRect.X, AccountRect.Y, AccountRect.Width, AccountRect.Height))
		[
			AccountButtonWidget
		]
		+ SConstraintCanvas::Slot().Offset(FMargin(PortraitRect.X, PortraitRect.Y, PortraitRect.Width, PortraitRect.Height))
		[
			HomeButtonWidget
		]
		+ SConstraintCanvas::Slot().Offset(FMargin(PowerUpRect.X, PowerUpRect.Y, PowerUpRect.Width, PowerUpRect.Height))
		[
			PowerUpButtonWidget
		]
		+ SConstraintCanvas::Slot().Offset(FMargin(AchievementsRect.X, AchievementsRect.Y, AchievementsRect.Width, AchievementsRect.Height))
		[
			AchievementsButtonWidget
		]
		+ SConstraintCanvas::Slot().Offset(FMargin(MiniGamesRect.X, MiniGamesRect.Y, MiniGamesRect.Width, MiniGamesRect.Height))
		[
			MiniGamesButtonWidget
		]
		+ SConstraintCanvas::Slot().Offset(FMargin(CouponRect.X, CouponRect.Y, CouponRect.Width, CouponRect.Height))
		[
			ChadCouponsWidget
		]
		+ SConstraintCanvas::Slot().Offset(FMargin(QuitRect.X, QuitRect.Y, QuitRect.Width, QuitRect.Height))
		[
			QuitButtonWidget
		];

	auto MakeSurfaceBackground = [this, BarFallbackOuter, BarFallbackInner]() -> TSharedRef<SWidget>
	{
		if (UIManager && UIManager->GetCurrentScreenType() == ET66ScreenType::MainMenu)
		{
			return SNullWidget::NullWidget;
		}

		if (TopBarBackdropBrush.IsValid())
		{
			return SNew(SImage)
				.Image(TopBarBackdropBrush.Get())
				.ColorAndOpacity(FLinearColor::White);
		}

		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(BarFallbackOuter)
			.Padding(2.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(BarFallbackInner)
			];
	};

	const TSharedRef<SWidget> Surface =
		SNew(SBox)
		.WidthOverride(SurfaceWidth)
		.HeightOverride(SurfaceHeight)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				MakeSurfaceBackground()
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
		.HeightOverride(T66MainMenuReferenceLayout::TopBarReservedHeight)
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

	return SNew(SVerticalBox)
		.Visibility(EVisibility::SelfHitTestInvisible)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(SnapPixel(GetTopBarViewportTransform().MapY(T66MainMenuReferenceLayout::TopBarReservedHeight)))
			[
				SNew(SScaleBox)
				.Stretch(EStretch::Fill)
				[
					ReservedReferenceCanvas
				]
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNullWidget::NullWidget
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
		ForceRebuildSlate();
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

FReply UT66FrontendTopBarWidget::HandleShopClicked()
{
	FLagScopedScope LagScope(GetWorld(), TEXT("FE-01/FE-02 FrontendTopBar::PowerUp"));
	NavigateWithTopBar(ET66ScreenType::PowerUp);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleMiniGamesClicked()
{
	NavigateWithTopBar(ET66ScreenType::Unlocks);
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
