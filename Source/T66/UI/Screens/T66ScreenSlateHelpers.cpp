// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66ScreenSlateHelpers.h"

#include "Engine/Texture2D.h"
#include "HAL/IConsoleManager.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66FriendslopStyle.h"
#include "UI/Style/T66RuntimeUIFontAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace T66ScreenSlateHelpers
{
	namespace
	{
		const TCHAR* ReferenceProgressSheetPath = TEXT("SourceAssets/UI/Reference/Shared/Progress/reference_progress_meter_sheet.png");
		const TCHAR* ReferenceUltrakillElementDir = TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements");
		const TCHAR* ReferenceUltrakillSquareElementDir = TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant");
		const TCHAR* ReferenceBloodyRetroElementDir = TEXT("SourceAssets/UI/Reference/Screens/MainMenu/BloodyRetro/Elements");
		const TCHAR* ReferenceBloodyRetroSquareElementDir = TEXT("SourceAssets/UI/Reference/Screens/MainMenu/BloodyRetro/Elements/SquareVariant");
		const FBox2f ReferenceProgressTrackUV(FVector2f(0.0530f, 0.2950f), FVector2f(0.9550f, 0.4440f));
		const FBox2f ReferenceProgressFillUV(FVector2f(0.0670f, 0.6320f), FVector2f(0.9320f, 0.6960f));

		TAutoConsoleVariable<int32> CVarT66UIChromePreset(
			TEXT("T66.UI.ChromePreset"),
			0,
			TEXT("UI chrome preset. 0=SquareVariant, 1=BloodyRetro."));

		ET66ReferenceChromePreset ClampReferenceChromePreset(const int32 Value)
		{
			return Value == 1
				? ET66ReferenceChromePreset::BloodyRetro
				: ET66ReferenceChromePreset::SquareVariant;
		}

		const TCHAR* GetReferenceChromeSquareElementDir(const ET66ReferenceChromePreset Preset)
		{
			switch (Preset)
			{
			case ET66ReferenceChromePreset::BloodyRetro:
				return ReferenceBloodyRetroSquareElementDir;
			case ET66ReferenceChromePreset::SquareVariant:
			default:
				return ReferenceUltrakillSquareElementDir;
			}
		}

		const TCHAR* GetReferenceMainMenuElementDir(const ET66ReferenceChromePreset Preset)
		{
			switch (Preset)
			{
			case ET66ReferenceChromePreset::BloodyRetro:
				return ReferenceBloodyRetroElementDir;
			case ET66ReferenceChromePreset::SquareVariant:
			default:
				return ReferenceUltrakillElementDir;
			}
		}

		class ST66ReferenceHorizontalSlicedImage : public SLeafWidget
		{
		public:
			SLATE_BEGIN_ARGS(ST66ReferenceHorizontalSlicedImage)
				: _Brush(nullptr)
				, _DesiredSize(FVector2D(1.0f, 1.0f))
				, _SourceCapFraction(0.105f)
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

			virtual int32 OnPaint(
				const FPaintArgs& Args,
				const FGeometry& AllottedGeometry,
				const FSlateRect& MyCullingRect,
				FSlateWindowElementList& OutDrawElements,
				int32 LayerId,
				const FWidgetStyle& InWidgetStyle,
				bool bParentEnabled) const override
			{
				const FSlateBrush* SourceBrush = Brush.Get();
				if (!SourceBrush || SourceBrush == FCoreStyle::Get().GetBrush("NoBrush"))
				{
					return LayerId;
				}

				const FVector2D Size = AllottedGeometry.GetLocalSize();
				FVector2D SourceSize(
					FMath::Max(1.0f, SourceBrush->ImageSize.X),
					FMath::Max(1.0f, SourceBrush->ImageSize.Y));
				if (const UTexture2D* SourceTexture = Cast<UTexture2D>(SourceBrush->GetResourceObject()))
				{
					SourceSize = FVector2D(
						FMath::Max(1, SourceTexture->GetSizeX()),
						FMath::Max(1, SourceTexture->GetSizeY()));
				}

				if (Size.X <= 1.0f || Size.Y <= 1.0f || SourceSize.X <= 1.0f || SourceSize.Y <= 1.0f)
				{
					return LayerId;
				}

				const float CapU = FMath::Clamp(SourceCapFraction, 0.02f, 0.45f);
				const float HeightScale = Size.Y / SourceSize.Y;
				const float SourceCapWidth = SourceSize.X * CapU;
				const float DestCapWidth = FMath::Clamp(SourceCapWidth * HeightScale, 1.0f, Size.X * 0.42f);
				const float DestCenterWidth = FMath::Max(0.0f, Size.X - (DestCapWidth * 2.0f));

				auto DrawSlice = [&](const FVector2D& Pos, const FVector2D& SliceSize, const float U0, const float U1)
				{
					if (SliceSize.X <= 0.5f || SliceSize.Y <= 0.5f || U1 <= U0)
					{
						return;
					}

					FSlateBrush LocalBrush = *SourceBrush;
					LocalBrush.DrawAs = ESlateBrushDrawType::Image;
					LocalBrush.Tiling = ESlateBrushTileType::NoTile;
					LocalBrush.Margin = FMargin(0.0f);
					LocalBrush.SetUVRegion(FBox2f(FVector2f(U0, 0.0f), FVector2f(U1, 1.0f)));

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

				DrawSlice(FVector2D(0.0f, 0.0f), FVector2D(DestCapWidth, Size.Y), 0.0f, CapU);
				DrawSlice(FVector2D(DestCapWidth, 0.0f), FVector2D(DestCenterWidth, Size.Y), CapU, 1.0f - CapU);
				DrawSlice(FVector2D(Size.X - DestCapWidth, 0.0f), FVector2D(DestCapWidth, Size.Y), 1.0f - CapU, 1.0f);

				return LayerId + 1;
			}

		private:
			TAttribute<const FSlateBrush*> Brush;
			FVector2D DesiredSize = FVector2D(1.0f, 1.0f);
			float SourceCapFraction = 0.105f;
		};

		FVector2D GetBrushSourceSize(const FSlateBrush* SourceBrush)
		{
			if (!SourceBrush)
			{
				return FVector2D(1.0f, 1.0f);
			}

			if (const UTexture2D* SourceTexture = Cast<UTexture2D>(SourceBrush->GetResourceObject()))
			{
				return FVector2D(
					FMath::Max(1, SourceTexture->GetSizeX()),
					FMath::Max(1, SourceTexture->GetSizeY()));
			}

			return FVector2D(
				FMath::Max(1.0f, SourceBrush->ImageSize.X),
				FMath::Max(1.0f, SourceBrush->ImageSize.Y));
		}

		void PaintImageRegion(
			const FSlateBrush* SourceBrush,
			const FBox2f& SourceUV,
			const FVector2D& Position,
			const FVector2D& Size,
			const FGeometry& AllottedGeometry,
			FSlateWindowElementList& OutDrawElements,
			const int32 LayerId,
			const FWidgetStyle& InWidgetStyle,
			const FLinearColor& Tint)
		{
			if (!SourceBrush || Size.X <= 0.5f || Size.Y <= 0.5f)
			{
				return;
			}

			FSlateBrush LocalBrush = *SourceBrush;
			LocalBrush.DrawAs = ESlateBrushDrawType::Image;
			LocalBrush.Tiling = ESlateBrushTileType::NoTile;
			LocalBrush.Margin = FMargin(0.0f);
			LocalBrush.SetUVRegion(SourceUV);

			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(
					FVector2f(Size),
					FSlateLayoutTransform(FVector2f(Position))),
				&LocalBrush,
				ESlateDrawEffect::None,
				InWidgetStyle.GetColorAndOpacityTint() * Tint);
		}

		void PaintHorizontalSlicedRegion(
			const FSlateBrush* SourceBrush,
			const FBox2f& SourceUV,
			const FVector2D& Position,
			const FVector2D& Size,
			const float SourceCapFraction,
			const FGeometry& AllottedGeometry,
			FSlateWindowElementList& OutDrawElements,
			const int32 LayerId,
			const FWidgetStyle& InWidgetStyle,
			const FLinearColor& Tint)
		{
			if (!SourceBrush || Size.X <= 1.0f || Size.Y <= 1.0f)
			{
				return;
			}

			const FVector2D FullSourceSize = GetBrushSourceSize(SourceBrush);
			const FVector2D RegionSourceSize(
				FMath::Max(1.0f, static_cast<float>(SourceUV.GetSize().X) * FullSourceSize.X),
				FMath::Max(1.0f, static_cast<float>(SourceUV.GetSize().Y) * FullSourceSize.Y));

			const float CapU = FMath::Clamp(SourceCapFraction, 0.02f, 0.45f);
			const float HeightScale = Size.Y / RegionSourceSize.Y;
			const float SourceCapWidth = RegionSourceSize.X * CapU;
			const float DestCapWidth = FMath::Clamp(SourceCapWidth * HeightScale, 1.0f, Size.X * 0.42f);
			const float DestCenterWidth = FMath::Max(0.0f, Size.X - (DestCapWidth * 2.0f));

			auto DrawSlice = [&](const FVector2D& SlicePos, const FVector2D& SliceSize, const float LocalU0, const float LocalU1)
			{
				if (SliceSize.X <= 0.5f || SliceSize.Y <= 0.5f || LocalU1 <= LocalU0)
				{
					return;
				}

				const float U0 = FMath::Lerp(SourceUV.Min.X, SourceUV.Max.X, LocalU0);
				const float U1 = FMath::Lerp(SourceUV.Min.X, SourceUV.Max.X, LocalU1);
				PaintImageRegion(
					SourceBrush,
					FBox2f(FVector2f(U0, SourceUV.Min.Y), FVector2f(U1, SourceUV.Max.Y)),
					Position + SlicePos,
					SliceSize,
					AllottedGeometry,
					OutDrawElements,
					LayerId,
					InWidgetStyle,
					Tint);
			};

			DrawSlice(FVector2D(0.0f, 0.0f), FVector2D(DestCapWidth, Size.Y), 0.0f, CapU);
			DrawSlice(FVector2D(DestCapWidth, 0.0f), FVector2D(DestCenterWidth, Size.Y), CapU, 1.0f - CapU);
			DrawSlice(FVector2D(Size.X - DestCapWidth, 0.0f), FVector2D(DestCapWidth, Size.Y), 1.0f - CapU, 1.0f);
		}

		const FSlateBrush* GetSimpleReferenceButtonFallbackBrush();

		class ST66ReferenceSlicedPlateButton : public SCompoundWidget
		{
		public:
			SLATE_BEGIN_ARGS(ST66ReferenceSlicedPlateButton)
				: _NormalBrush(nullptr)
				, _HoveredBrush(nullptr)
				, _PressedBrush(nullptr)
				, _DisabledBrush(nullptr)
				, _SelectedBrush(nullptr)
				, _MinWidth(0.0f)
				, _Height(0.0f)
				, _ContentPadding(FMargin(0.0f))
				, _IsEnabled(true)
				, _IsSelected(false)
				, _Visibility(EVisibility::Visible)
				, _SourceCapFraction(0.105f)
			{}
				SLATE_EVENT(FOnClicked, OnClicked)
				SLATE_ARGUMENT(TSharedPtr<SWidget>, Content)
				SLATE_ARGUMENT(const FSlateBrush*, NormalBrush)
				SLATE_ARGUMENT(const FSlateBrush*, HoveredBrush)
				SLATE_ARGUMENT(const FSlateBrush*, PressedBrush)
				SLATE_ARGUMENT(const FSlateBrush*, DisabledBrush)
				SLATE_ARGUMENT(const FSlateBrush*, SelectedBrush)
				SLATE_ARGUMENT(float, MinWidth)
				SLATE_ARGUMENT(float, Height)
				SLATE_ARGUMENT(FMargin, ContentPadding)
				SLATE_ATTRIBUTE(bool, IsEnabled)
				SLATE_ATTRIBUTE(bool, IsSelected)
				SLATE_ATTRIBUTE(EVisibility, Visibility)
				SLATE_ARGUMENT(float, SourceCapFraction)
			SLATE_END_ARGS()

			void Construct(const FArguments& InArgs)
			{
				NormalBrush = InArgs._NormalBrush;
				HoveredBrush = InArgs._HoveredBrush;
				PressedBrush = InArgs._PressedBrush;
				DisabledBrush = InArgs._DisabledBrush;
				SelectedBrush = InArgs._SelectedBrush;
				IsSelected = InArgs._IsSelected;
				SourceCapFraction = InArgs._SourceCapFraction;
				ContentPadding = InArgs._ContentPadding;

				TSharedRef<SWidget> InnerContent = InArgs._Content.IsValid()
					? InArgs._Content.ToSharedRef()
					: StaticCastSharedRef<SWidget>(SNew(SBox));

				const TAttribute<const FSlateBrush*> BrushAttribute =
					TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateSP(this, &ST66ReferenceSlicedPlateButton::GetCurrentBrush));
				const TAttribute<FMargin> PaddingAttribute =
					TAttribute<FMargin>::Create(TAttribute<FMargin>::FGetter::CreateSP(this, &ST66ReferenceSlicedPlateButton::GetContentPadding));

				TSharedRef<SOverlay> ButtonContent =
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						MakeReferenceHorizontalSlicedImage(
							BrushAttribute,
							FVector2D(1.0f, 1.0f),
							SourceCapFraction)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.Padding(PaddingAttribute)
					[
						InnerContent
					];

				FOnClicked ClickDelegate = InArgs._OnClicked;
				ChildSlot
				[
					FT66Style::MakeBareButton(
						FT66BareButtonParams(MoveTemp(ClickDelegate), ButtonContent)
							.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
							.SetPadding(FMargin(0.0f))
							.SetHAlign(HAlign_Fill)
							.SetVAlign(VAlign_Fill)
							.SetMinWidth(NormalizeReferenceSlicedButtonMinWidth(InArgs._MinWidth, InArgs._Height))
							.SetHeight(InArgs._Height)
							.SetEnabled(InArgs._IsEnabled)
							.SetVisibility(InArgs._Visibility),
						&ButtonWidget)
				];
			}

		private:
			const FSlateBrush* GetCurrentBrush() const
			{
				const bool bEnabled = ButtonWidget.IsValid() ? ButtonWidget->IsEnabled() : true;
				if (!bEnabled && DisabledBrush)
				{
					return DisabledBrush;
				}
				if (ButtonWidget.IsValid() && ButtonWidget->IsPressed() && PressedBrush)
				{
					return PressedBrush;
				}
				if (IsSelected.Get(false) && SelectedBrush)
				{
					return SelectedBrush;
				}
				if (ButtonWidget.IsValid() && ButtonWidget->IsHovered() && HoveredBrush)
				{
					return HoveredBrush;
				}
				return NormalBrush ? NormalBrush : GetSimpleReferenceButtonFallbackBrush();
			}

			FMargin GetContentPadding() const
			{
				const FMargin PressedNudge = (ButtonWidget.IsValid() && ButtonWidget->IsPressed())
					? FMargin(0.0f, 1.0f, 0.0f, 0.0f)
					: FMargin(0.0f);
				return ContentPadding + PressedNudge;
			}

			TSharedPtr<SButton> ButtonWidget;
			const FSlateBrush* NormalBrush = nullptr;
			const FSlateBrush* HoveredBrush = nullptr;
			const FSlateBrush* PressedBrush = nullptr;
			const FSlateBrush* DisabledBrush = nullptr;
			const FSlateBrush* SelectedBrush = nullptr;
			TAttribute<bool> IsSelected;
			FMargin ContentPadding = FMargin(0.0f);
			float SourceCapFraction = 0.105f;
		};

		const FSlateBrush* GetReferenceProgressSheetBrush()
		{
			static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
			return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
				Entry,
				nullptr,
				T66RuntimeUITextureAccess::MakeProjectDirPath(ReferenceProgressSheetPath),
				FMargin(0.0f),
				TEXT("ReferenceProgressSheet"),
				TextureFilter::TF_Nearest);
		}

		const FSlateBrush* GetSimpleReferenceButtonFallbackBrush()
		{
			static FSlateBrush Brush;
			static bool bInitialized = false;
			if (!bInitialized)
			{
				bInitialized = true;
				T66RuntimeUIBrushAccess::ConfigureSimpleReferenceFallbackBrush(
					Brush,
					TEXT("SourceAssets/UI/Reference/Shared/Buttons/Pill/normal.png"),
					FVector2D(1.f, 1.f),
					FMargin(0.18f, 0.24f, 0.18f, 0.24f),
					ESlateBrushDrawType::Box);
			}
			return &Brush;
		}

		class ST66ReferenceProgressBar : public SLeafWidget
		{
		public:
			SLATE_BEGIN_ARGS(ST66ReferenceProgressBar)
				: _Percent(TOptional<float>(0.0f))
				, _DesiredSize(FVector2D(180.0f, 18.0f))
				, _FallbackFill(FLinearColor(0.10f, 0.64f, 0.96f, 1.0f))
				, _Padding(FMargin(4.0f, 3.0f))
			{}
				SLATE_ATTRIBUTE(TOptional<float>, Percent)
				SLATE_ARGUMENT(FVector2D, DesiredSize)
				SLATE_ARGUMENT(FLinearColor, FallbackFill)
				SLATE_ARGUMENT(FMargin, Padding)
			SLATE_END_ARGS()

			void Construct(const FArguments& InArgs)
			{
				Percent = InArgs._Percent;
				DesiredSize = InArgs._DesiredSize;
				FallbackFill = InArgs._FallbackFill;
				Padding = InArgs._Padding;
			}

			virtual FVector2D ComputeDesiredSize(float) const override
			{
				return DesiredSize;
			}

			virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
				FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
			{
				const FVector2D Size = AllottedGeometry.GetLocalSize();
				if (Size.X <= 1.0f || Size.Y <= 1.0f)
				{
					return LayerId;
				}

				const FSlateBrush* ProgressSheetBrush = GetReferenceProgressSheetBrush();
				const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

				if (ProgressSheetBrush)
				{
					PaintHorizontalSlicedRegion(
						ProgressSheetBrush,
						ReferenceProgressTrackUV,
						FVector2D::ZeroVector,
						Size,
						0.058f,
						AllottedGeometry,
						OutDrawElements,
						LayerId,
						InWidgetStyle,
						FLinearColor::White);
				}
				else
				{
					FSlateDrawElement::MakeBox(
						OutDrawElements,
						LayerId,
						AllottedGeometry.ToPaintGeometry(FVector2f(Size), FSlateLayoutTransform()),
						WhiteBrush,
						ESlateDrawEffect::None,
						InWidgetStyle.GetColorAndOpacityTint() * FLinearColor(0.02f, 0.02f, 0.04f, 1.0f));
				}

				const float Pct = FMath::Clamp(Percent.Get(TOptional<float>(0.0f)).Get(0.0f), 0.0f, 1.0f);
				if (Pct <= 0.001f)
				{
					return LayerId + 1;
				}

				const FVector2D InnerPos(Padding.Left, Padding.Top);
				const FVector2D InnerSize(
					FMath::Max(0.0f, Size.X - Padding.Left - Padding.Right),
					FMath::Max(0.0f, Size.Y - Padding.Top - Padding.Bottom));
				const FVector2D FillSize(FMath::Max(1.0f, InnerSize.X * Pct), InnerSize.Y);
				if (FillSize.X <= 0.0f || FillSize.Y <= 0.0f)
				{
					return LayerId + 1;
				}

				if (ProgressSheetBrush)
				{
					const float FillUMax = FMath::Lerp(ReferenceProgressFillUV.Min.X, ReferenceProgressFillUV.Max.X, Pct);
					PaintImageRegion(
						ProgressSheetBrush,
						FBox2f(ReferenceProgressFillUV.Min, FVector2f(FillUMax, ReferenceProgressFillUV.Max.Y)),
						InnerPos,
						FillSize,
						AllottedGeometry,
						OutDrawElements,
						LayerId + 1,
						InWidgetStyle,
						FLinearColor::White);
				}
				else
				{
					FSlateDrawElement::MakeBox(
						OutDrawElements,
						LayerId + 1,
						AllottedGeometry.ToPaintGeometry(FVector2f(FillSize), FSlateLayoutTransform(FVector2f(InnerPos))),
						WhiteBrush,
						ESlateDrawEffect::None,
						InWidgetStyle.GetColorAndOpacityTint() * FallbackFill);
				}

				return LayerId + 2;
			}

		private:
			TAttribute<TOptional<float>> Percent;
			FVector2D DesiredSize = FVector2D(180.0f, 18.0f);
			FLinearColor FallbackFill = FLinearColor(0.10f, 0.64f, 0.96f, 1.0f);
			FMargin Padding = FMargin(4.0f, 3.0f);
		};
	}

	const FFrontendChromeMetrics& GetFrontendChromeMetrics()
	{
		static const FFrontendChromeMetrics Metrics;
		return Metrics;
	}

	float GetFrontendChromeTopInset(const UT66UIManager* UIManager)
	{
		const FFrontendChromeMetrics& Metrics = GetFrontendChromeMetrics();
		const float ResponsiveScale = FMath::Max(FT66Style::GetViewportResponsiveScale(), KINDA_SMALL_NUMBER);
		return UIManager
			? FMath::Max(0.0f, (UIManager->GetFrontendTopBarContentHeight() - Metrics.TopBarOverlapPx) / ResponsiveScale)
			: 0.0f;
	}

	FTopBarScreenLayoutMetrics MakeTopBarScreenLayoutMetrics(
		const UT66UIManager* UIManager,
		const FMargin& ExtraPadding)
	{
		FTopBarScreenLayoutMetrics Metrics;
		Metrics.ViewportSize = FT66Style::GetViewportLogicalSize();

		const float GlobalScale = FMath::Max(FT66Style::GetGlobalUIScale(), KINDA_SMALL_NUMBER);
		Metrics.TopBarReservedHeight = UIManager && UIManager->IsFrontendTopBarVisible()
			? FMath::Max(0.0f, (UIManager->GetFrontendTopBarReservedHeight() - GetFrontendChromeMetrics().TopBarOverlapPx) / GlobalScale)
			: 0.0f;

		const float TopGap = FMath::Clamp(Metrics.ViewportSize.Y * 0.002f, 0.0f, 4.0f);
		const float BottomPadding = FMath::Clamp(Metrics.ViewportSize.Y * 0.008f, 6.0f, 14.0f);

		Metrics.Padding = FMargin(
			ExtraPadding.Left,
			Metrics.TopBarReservedHeight + TopGap + ExtraPadding.Top,
			ExtraPadding.Right,
			BottomPadding + ExtraPadding.Bottom);

		Metrics.ContentSize = FVector2D(
			FMath::Max(1.0f, Metrics.ViewportSize.X - Metrics.Padding.Left - Metrics.Padding.Right),
			FMath::Max(1.0f, Metrics.ViewportSize.Y - Metrics.Padding.Top - Metrics.Padding.Bottom));
		Metrics.bCompact = Metrics.ContentSize.X < 1600.0f || Metrics.ContentSize.Y < 760.0f;
		Metrics.bStacked = Metrics.ContentSize.X < 1420.0f || Metrics.ContentSize.Y < 700.0f;
		return Metrics;
	}

	TSharedRef<SWidget> MakeTopBarScreenRoot(
		const UT66UIManager* UIManager,
		const TSharedRef<SWidget>& Content,
		const TSharedRef<SWidget>& BackgroundContent,
		const FLinearColor& OverlayTint,
		const FMargin& ExtraPadding)
	{
		const TAttribute<FMargin> PaddingAttr = TAttribute<FMargin>::CreateLambda([UIManager, ExtraPadding]() -> FMargin
		{
			return MakeTopBarScreenLayoutMetrics(UIManager, ExtraPadding).Padding;
		});

		const EVisibility OverlayVisibility = OverlayTint.A > 0.0f
			? EVisibility::SelfHitTestInvisible
			: EVisibility::Collapsed;

		return SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				BackgroundContent
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.Visibility(OverlayVisibility)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(OverlayTint)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
				.Padding(PaddingAttr)
				[
					Content
				]
			];
	}

	FSlateFontInfo MakeFrontendChromeTitleFont()
	{
		return FT66Style::Tokens::FontBold(GetFrontendChromeMetrics().TitleFontSize);
	}

	FSlateFontInfo MakeFrontendChromeTabFont()
	{
		return FT66Style::Tokens::FontBold(GetFrontendChromeMetrics().TabFontSize);
	}

	TSharedPtr<FSlateBrush> MakeSlateBrush(const FVector2D& ImageSize, ESlateBrushDrawType::Type DrawAs)
	{
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = DrawAs;
		Brush->ImageSize = ImageSize;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->SetResourceObject(nullptr);
		return Brush;
	}

	FResponsiveGridModalMetrics MakeResponsiveGridModalMetrics(int32 ItemCount, const FVector2D& SafeFrameSize)
	{
		FResponsiveGridModalMetrics Metrics;
		const int32 SafeItemCount = FMath::Max(ItemCount, 1);
		Metrics.Columns = FMath::Max(1, FMath::CeilToInt(FMath::Sqrt(static_cast<float>(SafeItemCount))));
		Metrics.Rows = FMath::Max(1, FMath::CeilToInt(static_cast<float>(SafeItemCount) / static_cast<float>(Metrics.Columns)));
		Metrics.ModalWidth = FMath::Min(SafeFrameSize.X * 0.92f, 1180.0f);
		Metrics.ModalHeight = FMath::Min(SafeFrameSize.Y * 0.94f, 700.0f);
		const float GridWidthBudget = FMath::Max(1.0f, Metrics.ModalWidth - (Metrics.ModalPaddingX * 2.0f));
		const float GridHeightBudget = FMath::Max(
			1.0f,
			Metrics.ModalHeight - (Metrics.ModalPaddingY * 2.0f) - Metrics.TitleSectionHeight - Metrics.FooterSectionHeight);

		Metrics.TileSize = FMath::Clamp(
			FMath::FloorToFloat(FMath::Min(
				(GridWidthBudget - Metrics.TileGap * static_cast<float>(Metrics.Columns)) / static_cast<float>(Metrics.Columns),
				(GridHeightBudget - Metrics.TileGap * static_cast<float>(Metrics.Rows)) / static_cast<float>(Metrics.Rows))),
			64.0f,
			256.0f);
		Metrics.GridWidth = Metrics.Columns * Metrics.TileSize + Metrics.Columns * Metrics.TileGap;
		Metrics.GridHeight = Metrics.Rows * Metrics.TileSize + Metrics.Rows * Metrics.TileGap;
		return Metrics;
	}

	void AddUniformGridPaddingSlots(SGridPanel& GridPanel, int32 FilledSlotCount, const FResponsiveGridModalMetrics& Metrics)
	{
		for (int32 Index = FilledSlotCount; Index < Metrics.Rows * Metrics.Columns; ++Index)
		{
			GridPanel.AddSlot(Index % Metrics.Columns, Index / Metrics.Columns)
				.Padding(Metrics.TileGap * 0.5f)
				[
					SNew(SBox)
					.WidthOverride(Metrics.TileSize)
					.HeightOverride(Metrics.TileSize)
					[
						SNew(SSpacer)
					]
				];
		}
	}

	TSharedRef<SWidget> MakeFilledButtonText(
		const FT66ButtonParams& Params,
		float ButtonHeight,
		const TAttribute<FSlateColor>& DefaultTextColor,
		const TAttribute<FLinearColor>& DefaultShadowColor)
	{
		const TAttribute<FText> Text = Params.DynamicLabel.IsBound()
			? Params.DynamicLabel
			: TAttribute<FText>(Params.Label);
		const int32 HeightFallbackFontSize = ButtonHeight > 0.f
			? FMath::RoundToInt(ButtonHeight * 0.42f)
			: 16;
		const int32 RequestedFontSize = Params.FontSize > 0 ? Params.FontSize : HeightFallbackFontSize;
		const int32 HeightCapFontSize = ButtonHeight > 0.f
			? FMath::FloorToInt(ButtonHeight * 0.52f)
			: 64;
		const int32 FontSize = FMath::Clamp(FMath::Min(RequestedFontSize, HeightCapFontSize), 12, 64);

		FSlateFontInfo Font = FT66Style::MakeFont(*Params.FontWeight, FontSize);
		Font.LetterSpacing = 1;

		TAttribute<FSlateColor> TextColor;
		if (Params.bHasStateTextColors)
		{
			TextColor = TAttribute<FSlateColor>(Params.NormalStateTextColor);
		}
		else if (Params.bHasTextColorOverride)
		{
			TextColor = Params.TextColorOverride;
		}
		else
		{
			TextColor = DefaultTextColor;
		}

		const TAttribute<FVector2D> ShadowOffset = Params.bHasTextShadowOffset
			? TAttribute<FVector2D>(Params.TextShadowOffset)
			: TAttribute<FVector2D>(FVector2D(1.f, 1.f));
		const TAttribute<FLinearColor> ShadowColor = Params.bHasStateTextShadowColors
			? TAttribute<FLinearColor>(Params.NormalStateTextShadowColor)
			: DefaultShadowColor;

		return FT66Style::MakeRetroUIText(
			StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					.StretchDirection(EStretchDirection::DownOnly)
					[
						SNew(STextBlock)
						.Text(Text)
						.Font(Font)
						.ColorAndOpacity(TextColor)
						.ShadowOffset(ShadowOffset)
						.ShadowColorAndOpacity(ShadowColor)
						.Justification(ETextJustify::Center)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.Clipping(EWidgetClipping::ClipToBounds)
					]
				]));
	}

	TSharedRef<SWidget> MakeReferenceHorizontalSlicedImage(
		TAttribute<const FSlateBrush*> Brush,
		const FVector2D& DesiredSize,
		const float SourceCapFraction)
	{
		return FT66Style::MakeRetroUIChromeSurface(
			StaticCastSharedRef<SWidget>(
				SNew(ST66ReferenceHorizontalSlicedImage)
				.Brush(MoveTemp(Brush))
				.DesiredSize(DesiredSize)
				.SourceCapFraction(SourceCapFraction)));
	}

	float NormalizeReferenceSlicedButtonMinWidth(const float RequestedMinWidth, const float Height)
	{
		const float EffectiveHeight = Height > 0.f ? Height : 44.f;
		const float ContractMinWidth = FMath::Clamp(EffectiveHeight * 2.0f, 84.f, 132.f);
		return FMath::Max(RequestedMinWidth, ContractMinWidth);
	}

	ET66ReferenceChromePreset GetReferenceChromePreset()
	{
		return ClampReferenceChromePreset(CVarT66UIChromePreset.GetValueOnGameThread());
	}

	const TCHAR* GetReferenceChromePresetName()
	{
		switch (GetReferenceChromePreset())
		{
		case ET66ReferenceChromePreset::BloodyRetro:
			return TEXT("BloodyRetro");
		case ET66ReferenceChromePreset::SquareVariant:
		default:
			return TEXT("SquareVariant");
		}
	}

	void SetReferenceChromePresetForSession(const ET66ReferenceChromePreset Preset)
	{
		CVarT66UIChromePreset.AsVariable()->Set(
			Preset == ET66ReferenceChromePreset::BloodyRetro ? 1 : 0,
			ECVF_SetByCode);
	}

	FString MakeReferenceMainMenuElementAssetPath(const TCHAR* FileName)
	{
		return FString::Printf(
			TEXT("%s/%s"),
			GetReferenceMainMenuElementDir(GetReferenceChromePreset()),
			FileName ? FileName : TEXT(""));
	}

	FString MakeReferenceChromeElementAssetPath(const TCHAR* FileName)
	{
		return FString::Printf(
			TEXT("%s/%s"),
			GetReferenceChromeSquareElementDir(GetReferenceChromePreset()),
			FileName ? FileName : TEXT(""));
	}

	FString MakeReferenceLongPanelAssetPath(const TCHAR* State)
	{
		FString NormalizedState(State ? State : TEXT("normal"));
		NormalizedState.ToLowerInline();
		if (NormalizedState.Equals(TEXT("hovered"), ESearchCase::IgnoreCase)
			|| NormalizedState.Equals(TEXT("pressed"), ESearchCase::IgnoreCase)
			|| NormalizedState.Equals(TEXT("selected"), ESearchCase::IgnoreCase)
			|| NormalizedState.Equals(TEXT("focused"), ESearchCase::IgnoreCase))
		{
			NormalizedState = TEXT("hover");
		}
		else if (!NormalizedState.Equals(TEXT("disabled"), ESearchCase::IgnoreCase))
		{
			NormalizedState = TEXT("normal");
		}

		if (GetReferenceChromePreset() == ET66ReferenceChromePreset::BloodyRetro)
		{
			return MakeReferenceMainMenuElementAssetPath(*FString::Printf(TEXT("long_panel_%s.png"), *NormalizedState));
		}

		const TCHAR* SquareState = NormalizedState.Equals(TEXT("hover"), ESearchCase::IgnoreCase)
			? TEXT("hover")
			: TEXT("normal");
		return MakeReferenceChromeElementAssetPath(*FString::Printf(TEXT("player_row_panel_%s_square_variant.png"), SquareState));
	}

	FString MakeReferenceRedSquareButtonAssetPath(const TCHAR* State)
	{
		FString NormalizedState(State ? State : TEXT("normal"));
		NormalizedState.ToLowerInline();
		if (NormalizedState.Equals(TEXT("hovered"), ESearchCase::IgnoreCase)
			|| NormalizedState.Equals(TEXT("focused"), ESearchCase::IgnoreCase))
		{
			NormalizedState = TEXT("hover");
		}
		else if (NormalizedState.Equals(TEXT("selected"), ESearchCase::IgnoreCase)
			|| NormalizedState.Equals(TEXT("active"), ESearchCase::IgnoreCase))
		{
			NormalizedState = TEXT("normal");
		}
		else if (!NormalizedState.Equals(TEXT("hover"), ESearchCase::IgnoreCase)
			&& !NormalizedState.Equals(TEXT("pressed"), ESearchCase::IgnoreCase)
			&& !NormalizedState.Equals(TEXT("disabled"), ESearchCase::IgnoreCase))
		{
			NormalizedState = TEXT("normal");
		}

		return FString::Printf(
			TEXT("%s/cta_new_game_button_%s_red_square_variant.png"),
			ReferenceUltrakillSquareElementDir,
			*NormalizedState);
	}

	namespace
	{
		const TCHAR* NormalizeReferenceChromeButtonFamily(const TCHAR* Family)
		{
			if (!Family
				|| FCString::Stricmp(Family, TEXT("Pill")) == 0
				|| FCString::Stricmp(Family, TEXT("reference_pill_button")) == 0)
			{
				return TEXT("Pill");
			}
			if (FCString::Stricmp(Family, TEXT("CTA")) == 0
				|| FCString::Stricmp(Family, TEXT("reference_cta_button")) == 0)
			{
				return TEXT("CTA");
			}
			if (FCString::Stricmp(Family, TEXT("SquareIcon")) == 0
				|| FCString::Stricmp(Family, TEXT("reference_square_button")) == 0)
			{
				return TEXT("SquareIcon");
			}
			return TEXT("Pill");
		}
	}

	FString MakeReferenceChromeButtonAssetPath(
		const TCHAR* Family,
		const TCHAR* State)
	{
		const TCHAR* SafeFamily = NormalizeReferenceChromeButtonFamily(Family);
		const TCHAR* SafeState = State ? State : TEXT("normal");
		const FString NormalizedState = FString(SafeState).Equals(TEXT("hovered"), ESearchCase::IgnoreCase)
			? FString(TEXT("hover"))
			: FString(SafeState).ToLower();
		const TCHAR* FamilyFileStem = TEXT("leaderboard_tab_button");
		if (FCString::Stricmp(SafeFamily, TEXT("CTA")) == 0)
		{
			return MakeReferenceRedSquareButtonAssetPath(*NormalizedState);
		}
		else if (FCString::Stricmp(SafeFamily, TEXT("SquareIcon")) == 0)
		{
			FamilyFileStem = TEXT("topbar_icon_button");
			if (NormalizedState.Equals(TEXT("selected"), ESearchCase::IgnoreCase))
			{
				return MakeReferenceChromeElementAssetPath(*FString::Printf(TEXT("%s_selected_square_variant.png"), FamilyFileStem));
			}
		}

		if (FCString::Stricmp(SafeFamily, TEXT("Pill")) == 0)
		{
			return MakeReferenceRedSquareButtonAssetPath(*NormalizedState);
		}

		return MakeReferenceChromeElementAssetPath(*FString::Printf(
			TEXT("%s_%s_square_variant.png"),
			FamilyFileStem,
			*NormalizedState));
	}

	FString MakeReferenceButtonAssetPath(
		const TCHAR* FamilyStem,
		const TCHAR* State)
	{
		return MakeReferenceChromeButtonAssetPath(FamilyStem, State);
	}

	FString MakeReferenceSharedAssetPath(const TCHAR* RelativeAssetPath)
	{
		const TCHAR* SafeRelativeAssetPath = RelativeAssetPath ? RelativeAssetPath : TEXT("");
		const FString Relative(SafeRelativeAssetPath);
		const FString LowerRelative = Relative.ToLower();
		if (Relative.StartsWith(TEXT("Panels/Modal/"), ESearchCase::IgnoreCase)
			|| Relative.StartsWith(TEXT("Panels/"), ESearchCase::IgnoreCase))
		{
			if (LowerRelative.Contains(TEXT("row"))
				|| LowerRelative.Contains(TEXT("strip"))
				|| LowerRelative.Contains(TEXT("table")))
			{
				return MakeReferenceLongPanelAssetPath(TEXT("normal"));
			}
			return MakeReferenceChromeElementAssetPath(TEXT("main_panel_normal_square_variant.png"));
		}
		if (Relative.StartsWith(TEXT("Buttons/DangerCTA/"), ESearchCase::IgnoreCase))
		{
			const FString State = FPaths::GetBaseFilename(Relative).ToLower();
			return MakeReferenceRedSquareButtonAssetPath(*State);
		}
		if (Relative.StartsWith(TEXT("Buttons/CTA/"), ESearchCase::IgnoreCase))
		{
			const FString State = FPaths::GetBaseFilename(Relative).ToLower();
			return MakeReferenceRedSquareButtonAssetPath(*State);
		}
		if (Relative.StartsWith(TEXT("Buttons/SquareIcon/"), ESearchCase::IgnoreCase))
		{
			const FString State = FPaths::GetBaseFilename(Relative).ToLower();
			const TCHAR* ResolvedState = State.Equals(TEXT("selected"), ESearchCase::IgnoreCase) ? TEXT("selected") : *State;
			return MakeReferenceChromeElementAssetPath(*FString::Printf(
				TEXT("topbar_icon_button_%s_square_variant.png"),
				ResolvedState));
		}
		if (Relative.StartsWith(TEXT("Buttons/basic_button/"), ESearchCase::IgnoreCase)
			|| Relative.StartsWith(TEXT("Buttons/Pill/"), ESearchCase::IgnoreCase))
		{
			const FString State = FPaths::GetBaseFilename(Relative).ToLower();
			return MakeReferenceRedSquareButtonAssetPath(*State);
		}
		if (Relative.StartsWith(TEXT("Slots/"), ESearchCase::IgnoreCase))
		{
			const FString State = FPaths::GetBaseFilename(Relative).ToLower();
			const TCHAR* SlotState = TEXT("normal");
			if (State.Contains(TEXT("disabled")))
			{
				SlotState = TEXT("disabled");
			}
			else if (State.Contains(TEXT("selected")))
			{
				SlotState = TEXT("selected");
			}
			else if (State.Contains(TEXT("hover")))
			{
				SlotState = TEXT("hover");
			}
			return MakeReferenceChromeElementAssetPath(*FString::Printf(TEXT("profile_slot_%s_square_variant.png"), SlotState));
		}
		if (Relative.StartsWith(TEXT("Controls/dropdown_field"), ESearchCase::IgnoreCase)
			|| Relative.StartsWith(TEXT("Controls/dropdown_field_"), ESearchCase::IgnoreCase))
		{
			FString State = FPaths::GetBaseFilename(Relative).ToLower();
			if (State.StartsWith(TEXT("dropdown_field_"), ESearchCase::IgnoreCase))
			{
				State.RightChopInline(15, EAllowShrinking::No);
			}
			if (State.IsEmpty() || State.Equals(TEXT("dropdown_field"), ESearchCase::IgnoreCase))
			{
				State = TEXT("normal");
			}
			if (State.Equals(TEXT("selected"), ESearchCase::IgnoreCase)
				|| State.Equals(TEXT("focused"), ESearchCase::IgnoreCase))
			{
				State = TEXT("hover");
			}
			return MakeReferenceRedSquareButtonAssetPath(*State);
		}

		return FString::Printf(
			TEXT("SourceAssets/UI/Reference/Shared/%s"),
			SafeRelativeAssetPath);
	}

	namespace
	{
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& GetReferenceSharedBrushEntry(const FString& AssetPath)
		{
			static TMap<FString, T66RuntimeUIBrushAccess::FOptionalTextureBrush> Entries;
			return Entries.FindOrAdd(AssetPath);
		}
	}

	const FSlateBrush* GetReferenceSharedBrush(
		const TCHAR* RelativeAssetPath,
		const FMargin& Margin,
		const TCHAR* DebugLabel)
	{
		const FString AssetPath = MakeReferenceSharedAssetPath(RelativeAssetPath);
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			GetReferenceSharedBrushEntry(AssetPath),
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(AssetPath),
			Margin,
			DebugLabel,
			TextureFilter::TF_Nearest);
	}

	bool IsReferenceChromeButtonAssetPath(const FString& SourceRelativePath)
	{
		return SourceRelativePath.Contains(TEXT("SourceAssets/UI/Reference/"))
			&& SourceRelativePath.Contains(TEXT("/Buttons/"));
	}

	bool IsReferenceChromePillButtonAssetPath(const FString& SourceRelativePath)
	{
		return SourceRelativePath.Contains(TEXT("SourceAssets/UI/Reference/"))
			&& SourceRelativePath.Contains(TEXT("/Buttons/Pill/"));
	}

	bool IsReferenceChromeCTAButtonAssetPath(const FString& SourceRelativePath)
	{
		return SourceRelativePath.Contains(TEXT("SourceAssets/UI/Reference/"))
			&& SourceRelativePath.Contains(TEXT("/Buttons/CTA/"));
	}

	TSharedRef<SWidget> MakeReferenceSharedBorder(
		const TCHAR* RelativeAssetPath,
		const TSharedRef<SWidget>& Content,
		const FMargin& BrushMargin,
		const FMargin& Padding,
		const TCHAR* DebugLabel,
		const FLinearColor& FallbackColor)
	{
		auto MakeLayeredReferenceBorder =
			[&Content, Padding](const FSlateBrush* BorderBrush, const FLinearColor& BorderColor) -> TSharedRef<SWidget>
		{
			TSharedRef<SBorder> ChromeBorder =
				SNew(SBorder)
				.BorderImage(BorderBrush)
				.BorderBackgroundColor(BorderColor)
				.Padding(FMargin(0.0f))
				.Visibility(EVisibility::HitTestInvisible)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					SNew(SBox)
				];

			return SNew(SOverlay)
				.Clipping(EWidgetClipping::ClipToBounds)
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					FT66Style::MakeRetroUIChromeSurface(StaticCastSharedRef<SWidget>(ChromeBorder))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
					.Padding(Padding)
					.Clipping(EWidgetClipping::ClipToBounds)
					[
						Content
					]
				];
		};

		if (const FSlateBrush* Brush = GetReferenceSharedBrush(RelativeAssetPath, BrushMargin, DebugLabel))
		{
			return MakeLayeredReferenceBorder(Brush, FLinearColor::White);
		}

		return MakeLayeredReferenceBorder(FCoreStyle::Get().GetBrush("WhiteBrush"), FallbackColor);
	}

	TSharedRef<SWidget> MakeReferenceSlicedPlateButton(
		FOnClicked OnClicked,
		const TSharedRef<SWidget>& Content,
		const FSlateBrush* NormalBrush,
		const FSlateBrush* HoveredBrush,
		const FSlateBrush* PressedBrush,
		const FSlateBrush* DisabledBrush,
		float MinWidth,
		float Height,
		const FMargin& ContentPadding,
		const TAttribute<bool>& IsEnabled,
		const TAttribute<EVisibility>& Visibility,
		float SourceCapFraction,
		const FSlateBrush* SelectedBrush,
		const TAttribute<bool>& IsSelected)
	{
		return SNew(ST66ReferenceSlicedPlateButton)
			.OnClicked(MoveTemp(OnClicked))
			.Content(Content)
			.NormalBrush(NormalBrush)
			.HoveredBrush(HoveredBrush)
			.PressedBrush(PressedBrush)
			.DisabledBrush(DisabledBrush)
			.SelectedBrush(SelectedBrush)
			.MinWidth(MinWidth)
			.Height(Height)
			.ContentPadding(ContentPadding)
			.IsEnabled(IsEnabled)
			.IsSelected(IsSelected)
			.Visibility(Visibility)
			.SourceCapFraction(SourceCapFraction);
	}

	TSharedRef<SWidget> MakeReferenceProgressBar(
		TAttribute<TOptional<float>> Percent,
		const FVector2D& DesiredSize,
		const FLinearColor& FallbackFill,
		const FMargin& Padding)
	{
		return FT66Style::MakeRetroUIChromeSurface(
			StaticCastSharedRef<SWidget>(
				SNew(ST66ReferenceProgressBar)
				.Percent(Percent)
				.DesiredSize(DesiredSize)
				.FallbackFill(FallbackFill)
				.Padding(Padding)));
	}

	TSharedRef<SWidget> MakeReferenceProgressBar(
		float Percent,
		const FVector2D& DesiredSize,
		const FLinearColor& FallbackFill,
		const FMargin& Padding)
	{
		return MakeReferenceProgressBar(TAttribute<TOptional<float>>(TOptional<float>(Percent)), DesiredSize, FallbackFill, Padding);
	}

	TSharedRef<SWidget> MakeResponsiveGridTile(
		const FT66ButtonParams& ButtonParams,
		const FLinearColor& BackgroundColor,
		const TSharedRef<SWidget>& Content,
		const FResponsiveGridModalMetrics& Metrics)
	{
		FT66ButtonParams TileParams = ButtonParams;
		TileParams
			.SetMinWidth(0.0f)
			.SetPadding(FMargin(0.0f))
			.SetColor(FLinearColor::Transparent)
			.SetContent(
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BackgroundColor)
				]
				+ SOverlay::Slot()
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					[
						Content
					]
				]);

		return SNew(SBox)
			.WidthOverride(Metrics.TileSize)
			.HeightOverride(Metrics.TileSize)
			[
				FT66Style::MakeButton(TileParams)
			];
	}

	TSharedRef<SWidget> MakeResponsiveGridModal(
		const FText& TitleText,
		const TSharedRef<SWidget>& GridContent,
		const TSharedRef<SWidget>& FooterContent,
		const FResponsiveGridModalMetrics& Metrics)
	{
		return MakeCenteredScrimModal(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FT66Style::Panel())
			.Padding(FMargin(Metrics.ModalPaddingX, Metrics.ModalPaddingY))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 0.0f, 0.0f, 20.0f)
				[
					SNew(STextBlock)
					.Text(TitleText)
					.Font(FT66Style::Tokens::FontBold(28))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(Metrics.GridWidth)
					.HeightOverride(Metrics.GridHeight)
					[
						GridContent
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 20.0f, 0.0f, 0.0f)
				[
					FooterContent
				]
			],
			FMargin(0.0f),
			Metrics.ModalWidth,
			Metrics.ModalHeight,
			true);
	}

	TSharedRef<SWidget> MakeCenteredScrimModal(
		const TSharedRef<SWidget>& Content,
		const FMargin& OuterPadding,
		float WidthOverride,
		float HeightOverride,
		bool bUseWhiteBrush)
	{
		TSharedRef<SWidget> SizedContent = Content;
		if (WidthOverride > 0.0f || HeightOverride > 0.0f)
		{
			TSharedRef<SBox> SizedBox = SNew(SBox)[Content];
			if (WidthOverride > 0.0f) { SizedBox->SetWidthOverride(WidthOverride); }
			if (HeightOverride > 0.0f) { SizedBox->SetHeightOverride(HeightOverride); }
			SizedContent = SizedBox;
		}

		TSharedRef<SBorder> Scrim = SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Scrim())
			.Padding(OuterPadding);
		if (bUseWhiteBrush)
		{
			Scrim->SetBorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"));
		}

		Scrim->SetContent(
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SizedContent
			]);
		return Scrim;
	}

	ET66FlatState ToFlatState(const EFriendslopStandardModalButtonState State)
	{
		switch (State)
		{
		case EFriendslopStandardModalButtonState::Selected:
			return ET66FlatState::Selected;
		case EFriendslopStandardModalButtonState::Ready:
			return ET66FlatState::Ready;
		case EFriendslopStandardModalButtonState::Disabled:
			return ET66FlatState::Disabled;
		case EFriendslopStandardModalButtonState::Default:
		default:
			return ET66FlatState::Default;
		}
	}

	FName MakeFriendslopStandardModalButtonChildTag(const FName Tag, const TCHAR* Suffix)
	{
		if (Tag.IsNone())
		{
			return NAME_None;
		}

		return FName(*(Tag.ToString() + TEXT(".") + Suffix));
	}

	FName MakeFriendslopStandardModalCheckboxChildTag(const FName Tag, const TCHAR* Suffix)
	{
		if (Tag.IsNone())
		{
			return NAME_None;
		}

		return FName(*(Tag.ToString() + TEXT(".") + Suffix));
	}

	const TCHAR* GetFriendslopStandardModalButtonAssetPath(const EFriendslopStandardModalButtonChrome Chrome)
	{
		switch (Chrome)
		{
		case EFriendslopStandardModalButtonChrome::Green:
			return TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/SharedPrimitives/standard_modal_button_action_green.png");
		case EFriendslopStandardModalButtonChrome::Dark:
			return TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/SharedPrimitives/standard_modal_button_long_dark.png");
		case EFriendslopStandardModalButtonChrome::Red:
		default:
			return TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/SharedPrimitives/standard_modal_button_primary_red.png");
		}
	}

	const TCHAR* GetFriendslopStandardModalCheckboxAssetPath(const bool bChecked)
	{
		return bChecked
			? TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/SharedPrimitives/standard_modal_checkbox_checked.png")
			: TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/SharedPrimitives/standard_modal_checkbox_unchecked.png");
	}

	T66RuntimeUIBrushAccess::FOptionalTextureBrush& GetFriendslopStandardModalCheckboxBrushEntry(const bool bChecked)
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush CheckedBrush;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush UncheckedBrush;
		return bChecked ? CheckedBrush : UncheckedBrush;
	}

	const FSlateBrush* ResolveFriendslopStandardModalCheckboxBrush(const bool bChecked)
	{
		const FString FallbackFilePath = FPaths::ProjectDir() / GetFriendslopStandardModalCheckboxAssetPath(bChecked);
		const FSlateBrush* Brush = T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			GetFriendslopStandardModalCheckboxBrushEntry(bChecked),
			nullptr,
			FallbackFilePath,
			FMargin(0.0f),
			bChecked ? TEXT("FriendslopStandardModalCheckboxChecked") : TEXT("FriendslopStandardModalCheckboxUnchecked"));

		return Brush ? Brush : FCoreStyle::Get().GetBrush(TEXT("NoBrush"));
	}

	FVector2D GetFriendslopStandardModalButtonFallbackSize(const EFriendslopStandardModalButtonChrome Chrome)
	{
		switch (Chrome)
		{
		case EFriendslopStandardModalButtonChrome::Dark:
			return FVector2D(300.0f, 58.0f);
		case EFriendslopStandardModalButtonChrome::Green:
		case EFriendslopStandardModalButtonChrome::Red:
		default:
			return FVector2D(300.0f, 58.0f);
		}
	}

	FLinearColor GetFriendslopStandardModalButtonFallbackTint(const EFriendslopStandardModalButtonChrome Chrome)
	{
		switch (Chrome)
		{
		case EFriendslopStandardModalButtonChrome::Green:
			return FLinearColor(0.05f, 0.46f, 0.12f, 1.0f);
		case EFriendslopStandardModalButtonChrome::Dark:
			return FLinearColor(0.015f, 0.018f, 0.024f, 1.0f);
		case EFriendslopStandardModalButtonChrome::Red:
		default:
			return FLinearColor(0.62f, 0.04f, 0.075f, 1.0f);
		}
	}

	TSharedRef<SWidget> MakeFriendslopModalText(
		const FText& Text,
		const int32 FontSize,
		const bool bBold,
		const FLinearColor& Color,
		const float WrapTextAt,
		const FName Tag,
		const TCHAR* Role,
		const bool bScaleDown,
		const bool bAutoWrap = true)
	{
		TSharedRef<STextBlock> TextBlock =
			SNew(STextBlock)
			.Text(Text)
			.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(FontSize, bBold))
			.ColorAndOpacity(Color)
			.ShadowOffset(FVector2D(2.0f, 2.0f))
			.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.58f))
			.Justification(ETextJustify::Center)
			.AutoWrapText(bAutoWrap)
			.WrapTextAt(bAutoWrap ? WrapTextAt : 0.0f)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds);

		TSharedRef<SWidget> TextWidget = TextBlock;
		if (bScaleDown)
		{
			TextWidget =
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::DownOnly)
				[
					TextBlock
				];
		}
		else
		{
			TextWidget =
				SNew(SBox)
				.WidthOverride(WrapTextAt)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					TextBlock
				];
		}

		return FT66FlatStyle::AttachMetadata(
			TextWidget,
			Tag,
			Role,
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true);
	}

	TSharedRef<SWidget> MakeFriendslopStandardModalButton(const FFriendslopStandardModalButton& Button)
	{
		const ET66FlatState FlatState = ToFlatState(Button.State);
		const float ButtonWidth = Button.MinWidth > 0.0f ? Button.MinWidth : 300.0f;
		const float ButtonHeight = Button.Height > 0.0f ? Button.Height : 58.0f;
		const float LabelWidth = FMath::Max(96.0f, ButtonWidth - 74.0f);
		const float LabelHeight = FMath::Max(24.0f, ButtonHeight - 22.0f);

		const TSharedRef<SWidget> Label =
			FT66FlatStyle::AttachMetadata(
				SNew(STextBlock)
				.Text(Button.Label)
				.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(Button.FontSize, true))
				.ColorAndOpacity(FT66FriendslopStyle::TextColorForState(FlatState))
				.Justification(ETextJustify::Center)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.ShadowOffset(FVector2D(2.0f, 2.0f))
				.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.55f)),
				MakeFriendslopStandardModalButtonChildTag(Button.Tag, TEXT("Label")),
				TEXT("Label.Button"),
				FlatState,
				TOptional<FLinearColor>(),
				false,
				NAME_None,
				true);

		return FT66FriendslopStyle::MakeCustomToggleGroupButton(
			GetFriendslopStandardModalButtonAssetPath(Button.Chrome),
			FMargin(0.0f),
			GetFriendslopStandardModalButtonFallbackSize(Button.Chrome),
			FlatState,
			SNew(SBox)
			.WidthOverride(LabelWidth)
			.HeightOverride(LabelHeight)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::DownOnly)
				[
					Label
				]
			],
			Button.OnClicked,
			FMargin(34.0f, 11.0f),
			ButtonWidth,
			ButtonHeight,
			Button.IsEnabled,
			Button.Tag,
			NAME_None,
			GetFriendslopStandardModalButtonFallbackTint(Button.Chrome),
			ESlateBrushDrawType::Image);
	}

	TSharedRef<SWidget> MakeFriendslopStandardModalCheckboxRow(const FFriendslopStandardModalCheckboxRow& Checkbox)
	{
		const float CheckboxSize = Checkbox.CheckboxSize > 0.0f ? Checkbox.CheckboxSize : 44.0f;
		const TAttribute<bool> IsChecked = Checkbox.IsChecked;
		const FName GlyphTag = Checkbox.CheckboxTag.IsNone()
			? MakeFriendslopStandardModalCheckboxChildTag(Checkbox.RowTag, TEXT("Checkbox"))
			: Checkbox.CheckboxTag;
		const FName LabelTag = Checkbox.LabelTag.IsNone()
			? MakeFriendslopStandardModalCheckboxChildTag(Checkbox.RowTag, TEXT("Label"))
			: Checkbox.LabelTag;

		const TSharedRef<SWidget> Glyph =
			FT66FlatStyle::AttachMetadata(
				SNew(SBox)
				.WidthOverride(CheckboxSize)
				.HeightOverride(CheckboxSize)
				[
					SNew(SImage)
					.Image_Lambda([IsChecked]() -> const FSlateBrush*
					{
						return ResolveFriendslopStandardModalCheckboxBrush(IsChecked.Get(false));
					})
				],
				GlyphTag,
				TEXT("Checkbox.Glyph"),
				ET66FlatState::Default);

		const TSharedRef<SWidget> Label =
			FT66FlatStyle::AttachMetadata(
				SNew(SBox)
				.WidthOverride(360.0f)
				.HeightOverride(38.0f)
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					.StretchDirection(EStretchDirection::DownOnly)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(Checkbox.Label)
						.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(Checkbox.FontSize, true))
						.ColorAndOpacity(FLinearColor(0.90f, 0.92f, 0.96f, 1.0f))
						.ShadowOffset(FVector2D(2.0f, 2.0f))
						.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.55f))
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.Clipping(EWidgetClipping::ClipToBounds)
					]
				],
				LabelTag,
				TEXT("Label.Checkbox"),
				ET66FlatState::Default,
				TOptional<FLinearColor>(),
				false,
				NAME_None,
				true);

		TSharedRef<SWidget> Row =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				Glyph
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(14.0f, 0.0f, 0.0f, 0.0f)
			[
				Label
			];

		TSharedRef<SButton> Button =
			SNew(SButton)
			.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>(TEXT("NoBorder")))
			.ContentPadding(FMargin(0.0f))
			.ClickMethod(EButtonClickMethod::MouseDown)
			.IsEnabled(Checkbox.IsEnabled)
			.OnClicked(Checkbox.OnClicked)
			[
				Row
			];

		return FT66FlatStyle::AttachMetadata(
			Button,
			Checkbox.RowTag,
			TEXT("CheckboxButton"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			Checkbox.OnClicked.IsBound(),
			NAME_None,
			false,
			Checkbox.OnClicked.IsBound());
	}

	TSharedRef<SWidget> MakeFriendslopStandardModal(const FFriendslopStandardModalParams& Params)
	{
		constexpr float PanelWidth = 1120.0f;
		const bool bHasCheckboxRow = Params.bShowCheckboxRow;
		const float PanelHeight = bHasCheckboxRow ? 490.0f : 400.0f;
		const float ButtonY = bHasCheckboxRow ? 360.0f : 274.0f;
		const float StatusY = bHasCheckboxRow ? 250.0f : 244.0f;
		const float CheckboxY = bHasCheckboxRow ? 296.0f : 252.0f;
		const FString ModalAssetPath(TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/SharedPrimitives/standard_modal_panel_textless.png"));

		TSharedRef<SConstraintCanvas> ContentCanvas = SNew(SConstraintCanvas);
		auto AddSlot = [&ContentCanvas](
			const float X,
			const float Y,
			const float W,
			const float H,
			const TSharedRef<SWidget>& Widget)
		{
			ContentCanvas->AddSlot()
				.Anchors(FAnchors(0.0f, 0.0f))
				.Alignment(FVector2D(0.0f, 0.0f))
				.Offset(FMargin(X, Y, W, H))
				[
					Widget
				];
		};

		AddSlot(
			0.0f,
			0.0f,
			PanelWidth,
			PanelHeight,
			FT66FriendslopStyle::MakeCustomPanel(
				ModalAssetPath,
				FMargin(0.14f, 0.20f, 0.14f, 0.16f),
				FVector2D(PanelWidth, PanelHeight),
				ET66FlatState::Default,
				FMargin(0.0f),
				SNullWidget::NullWidget,
				nullptr,
				Params.PanelTag,
				FLinearColor(0.035f, 0.038f, 0.047f, 1.0f)));

		AddSlot(
			190.0f,
			59.0f,
			740.0f,
			60.0f,
			MakeFriendslopModalText(
				Params.TitleText,
				30,
				true,
				FLinearColor::White,
				700.0f,
				Params.TitleTag,
				TEXT("Label.Title"),
				true,
				false));

		AddSlot(
			200.0f,
			156.0f,
			720.0f,
			88.0f,
			MakeFriendslopModalText(
				Params.BodyText,
				16,
				false,
				FLinearColor(0.90f, 0.92f, 0.96f, 1.0f),
				650.0f,
				Params.BodyTag,
				TEXT("Label.Body"),
				false));

		AddSlot(
			170.0f,
			StatusY,
			780.0f,
			32.0f,
			SNew(SBox)
			.Visibility(Params.StatusText.IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible)
			[
				MakeFriendslopModalText(
					Params.StatusText,
					16,
					true,
					FLinearColor(1.0f, 0.78f, 0.32f, 1.0f),
					730.0f,
					Params.StatusTag,
					TEXT("Label.Status"),
					true)
			]);

		if (bHasCheckboxRow)
		{
			AddSlot(
				340.0f,
				CheckboxY,
				440.0f,
				52.0f,
				MakeFriendslopStandardModalCheckboxRow(Params.CheckboxRow));
		}

		AddSlot(
			230.0f,
			ButtonY,
			300.0f,
			58.0f,
			MakeFriendslopStandardModalButton(Params.LeftButton));

		AddSlot(
			590.0f,
			ButtonY,
			300.0f,
			58.0f,
			MakeFriendslopStandardModalButton(Params.RightButton));

		const TSharedRef<SWidget> Root =
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				FT66FlatStyle::AttachMetadata(
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
					.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.62f))
					.Padding(0.0f),
					Params.ScrimTag,
					TEXT("Scrim"),
					ET66FlatState::Default)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(PanelWidth)
				.HeightOverride(PanelHeight)
				[
					ContentCanvas
				]
			];

		return FT66FlatStyle::AttachMetadata(
			Root,
			Params.RootTag,
			TEXT("Root"),
			ET66FlatState::Default);
	}

	TSharedRef<SWidget> MakeTwoButtonRow(
		const TSharedRef<SWidget>& LeftButton,
		const TSharedRef<SWidget>& RightButton,
		const FMargin& LeftPadding,
		const FMargin& RightPadding,
		EVisibility Visibility)
	{
		return SNew(SHorizontalBox)
			.Visibility(Visibility)
			+ SHorizontalBox::Slot().AutoWidth().Padding(LeftPadding)[LeftButton]
			+ SHorizontalBox::Slot().AutoWidth().Padding(RightPadding)[RightButton];
	}
}
