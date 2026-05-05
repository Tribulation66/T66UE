// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66ScreenSlateHelpers.h"

#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace T66ScreenSlateHelpers
{
	namespace
	{
		const TCHAR* ReferenceProgressSheetPath = TEXT("SourceAssets/UI/Reference/Shared/Progress/reference_progress_meter_sheet.png");
		const FBox2f ReferenceProgressTrackUV(FVector2f(0.0530f, 0.2950f), FVector2f(0.9550f, 0.4440f));
		const FBox2f ReferenceProgressFillUV(FVector2f(0.0670f, 0.6320f), FVector2f(0.9320f, 0.6960f));

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
				return NormalBrush ? NormalBrush : FCoreStyle::Get().GetBrush("WhiteBrush");
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

		return SNew(SBox)
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
			];
	}

	TSharedRef<SWidget> MakeReferenceHorizontalSlicedImage(
		TAttribute<const FSlateBrush*> Brush,
		const FVector2D& DesiredSize,
		const float SourceCapFraction)
	{
		return SNew(ST66ReferenceHorizontalSlicedImage)
			.Brush(MoveTemp(Brush))
			.DesiredSize(DesiredSize)
			.SourceCapFraction(SourceCapFraction);
	}

	float NormalizeReferenceSlicedButtonMinWidth(const float RequestedMinWidth, const float Height)
	{
		const float EffectiveHeight = Height > 0.f ? Height : 44.f;
		const float ContractMinWidth = FMath::Clamp(EffectiveHeight * 2.0f, 84.f, 132.f);
		return FMath::Max(RequestedMinWidth, ContractMinWidth);
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
		return FString::Printf(
			TEXT("SourceAssets/UI/Reference/Shared/Buttons/%s/%s.png"),
			SafeFamily,
			SafeState);
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
		if (const FSlateBrush* Brush = GetReferenceSharedBrush(RelativeAssetPath, BrushMargin, DebugLabel))
		{
			return SNew(SBorder)
				.BorderImage(Brush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				];
		}

		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FallbackColor)
			.Padding(Padding)
			[
				Content
			];
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
		return SNew(ST66ReferenceProgressBar)
			.Percent(Percent)
			.DesiredSize(DesiredSize)
			.FallbackFill(FallbackFill)
			.Padding(Padding);
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
