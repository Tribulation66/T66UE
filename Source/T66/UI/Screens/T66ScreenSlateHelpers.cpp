// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66ScreenSlateHelpers.h"

#include "Styling/CoreStyle.h"
#include "UI/T66UIManager.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace T66ScreenSlateHelpers
{
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
