// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66GameplayHUDWidget.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66MediaViewerSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66StageGate.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66MiasmaBoundary.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66Style.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Rendering/DrawElements.h"
#include "Math/TransformCalculus2D.h"
#include "Rendering/SlateRenderTransform.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SToolTip.h"

namespace
{
	// Media viewer panel: original phone-panel size with tab row above.
	static constexpr float GT66MediaPanelW = 330.f;
	// Video area + tab row height (~30px tabs + 570px video = 600 total, matching old panel).
	static constexpr float GT66MediaVideoH = 570.f;
	static constexpr float GT66MediaTabRowH = 30.f;
	static constexpr float GT66MediaPanelH = GT66MediaVideoH + GT66MediaTabRowH;

	// Distance from viewport bottom to top of hearts row (matches bottom-left HUD: slot padding 24 + portrait 250 + padding 6 + hearts 48).
	static constexpr float GT66HeartsTopOffsetFromBottom = 24.f + 250.f + 6.f + 48.f;

	/** Creates a custom tooltip widget (background + text) for HUD item/idol hover. Returns null if InText is empty. */
	static TSharedPtr<IToolTip> CreateCustomTooltip(const FText& InText)
	{
		if (InText.IsEmpty()) return nullptr;
		const ISlateStyle& Style = FT66Style::Get();
		const FTextBlockStyle& TextBody = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Body");
		return SNew(SToolTip)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
				.BorderBackgroundColor(FT66Style::Tokens::Bg)
				.Padding(FMargin(8.f, 6.f))
				[
					SNew(STextBlock)
					.Text(InText)
					.TextStyle(&TextBody)
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.AutoWrapText(true)
					.WrapTextAt(280.f)
				]
			];
	}
}

class ST66RingWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(ST66RingWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		Percent01 = 0.f;
		Thickness = 4.f;
	}

	void SetPercent(float InPercent01)
	{
		Percent01 = FMath::Clamp(InPercent01, 0.f, 1.f);
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(44.f, 44.f);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const FVector2D Size = AllottedGeometry.GetLocalSize();
		const float MinDim = FMath::Max(1.f, FMath::Min(Size.X, Size.Y));
		const FVector2D Center(Size.X * 0.5f, Size.Y * 0.5f);
		const float Radius = (MinDim * 0.5f) - Thickness;

		const int32 NumSeg = 64;
		const float StartAngle = -PI * 0.5f;

		auto MakeCirclePoints = [&](int32 SegCount) -> TArray<FVector2D>
		{
			TArray<FVector2D> Pts;
			Pts.Reserve(SegCount + 1);
			for (int32 i = 0; i <= SegCount; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(NumSeg);
				const float A = StartAngle + (2.f * PI * T);
				Pts.Add(Center + FVector2D(FMath::Cos(A) * Radius, FMath::Sin(A) * Radius));
			}
			return Pts;
		};

		// Solid black filled circle background (darker and more solid).
		{
			const float FillRadius = Radius * 0.5f;
			TArray<FVector2D> FillPts;
			FillPts.Reserve(NumSeg + 1);
			for (int32 i = 0; i <= NumSeg; ++i)
			{
				const float A = (2.f * PI * static_cast<float>(i)) / static_cast<float>(NumSeg);
				FillPts.Add(Center + FVector2D(FMath::Cos(A) * FillRadius, FMath::Sin(A) * FillRadius));
			}
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				FillPts,
				ESlateDrawEffect::None,
				FLinearColor(0.02f, 0.02f, 0.02f, 1.f),
				true,
				Radius
			);
		}

		// Background ring (dark solid outline).
		{
			const TArray<FVector2D> Pts = MakeCirclePoints(NumSeg);
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				Pts,
				ESlateDrawEffect::None,
				FLinearColor(0.05f, 0.05f, 0.05f, 1.f),
				true,
				Thickness
			);
		}

		// Progress ring (gold / amber).
		const int32 ProgSeg = FMath::Clamp(FMath::RoundToInt(Percent01 * static_cast<float>(NumSeg)), 0, NumSeg);
		if (ProgSeg > 0)
		{
			TArray<FVector2D> Pts;
			Pts.Reserve(ProgSeg + 1);
			for (int32 i = 0; i <= ProgSeg; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(NumSeg);
				const float A = StartAngle + (2.f * PI * T);
				Pts.Add(Center + FVector2D(FMath::Cos(A) * Radius, FMath::Sin(A) * Radius));
			}
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 2,
				AllottedGeometry.ToPaintGeometry(),
				Pts,
				ESlateDrawEffect::None,
				FLinearColor(0.85f, 0.65f, 0.10f, 1.f),
				false,
				Thickness
			);
		}

		return LayerId + 3;
	}

private:
	float Percent01 = 0.f;
	float Thickness = 3.f;
};

class ST66DotWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(ST66DotWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		DotColor = FLinearColor::White;
	}

	void SetDotColor(const FLinearColor& InColor)
	{
		DotColor = InColor;
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(12.f, 12.f);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const FVector2D Size = AllottedGeometry.GetLocalSize();
		const float MinDim = FMath::Max(1.f, FMath::Min(Size.X, Size.Y));
		const FVector2D Center(Size.X * 0.5f, Size.Y * 0.5f);
		const float Radius = (MinDim * 0.5f) - 0.5f;

		static constexpr int32 NumSeg = 24;
		TArray<FVector2D> Pts;
		Pts.Reserve(NumSeg + 1);
		for (int32 i = 0; i <= NumSeg; ++i)
		{
			const float T = static_cast<float>(i) / static_cast<float>(NumSeg);
			const float A = 2.f * PI * T;
			Pts.Add(Center + FVector2D(FMath::Cos(A) * Radius, FMath::Sin(A) * Radius));
		}

		// UE5.7 Slate doesn't expose a convex fill helper, so we draw an extremely thick stroked circle.
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			Pts,
			ESlateDrawEffect::None,
			DotColor,
			true,
			MinDim
		);

		return LayerId + 1;
	}

private:
	FLinearColor DotColor = FLinearColor::White;
};

class ST66CrosshairWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(ST66CrosshairWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		CrosshairColor = FLinearColor(0.95f, 0.95f, 1.f, 0.85f);
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(28.f, 28.f);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const FVector2D Size = AllottedGeometry.GetLocalSize();
		const FVector2D Center(Size.X * 0.5f, Size.Y * 0.5f);

		const float Gap = 4.f;
		const float Len = 8.f;
		const float Thick = 2.f;

		auto Line = [&](const FVector2D& A, const FVector2D& B)
		{
			TArray<FVector2D> Pts;
			Pts.Reserve(2);
			Pts.Add(A);
			Pts.Add(B);
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				Pts,
				ESlateDrawEffect::None,
				CrosshairColor,
				false,
				Thick
			);
		};

		// Left / Right / Up / Down ticks
		Line(Center + FVector2D(-(Gap + Len), 0.f), Center + FVector2D(-Gap, 0.f));
		Line(Center + FVector2D(Gap, 0.f), Center + FVector2D(Gap + Len, 0.f));
		Line(Center + FVector2D(0.f, -(Gap + Len)), Center + FVector2D(0.f, -Gap));
		Line(Center + FVector2D(0.f, Gap), Center + FVector2D(0.f, Gap + Len));

		// Tiny center dot.
		{
			static constexpr int32 NumSeg = 16;
			const float R = 1.5f;
			TArray<FVector2D> Pts;
			Pts.Reserve(NumSeg + 1);
			for (int32 i = 0; i <= NumSeg; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(NumSeg);
				const float A = 2.f * PI * T;
				Pts.Add(Center + FVector2D(FMath::Cos(A) * R, FMath::Sin(A) * R));
			}
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				Pts,
				ESlateDrawEffect::None,
				CrosshairColor,
				true,
				4.f
			);
		}

		return LayerId + 1;
	}

private:
	FLinearColor CrosshairColor = FLinearColor(1.f, 1.f, 1.f, 0.85f);
};

struct FT66MapMarker
{
	FVector2D WorldXY = FVector2D::ZeroVector;
	FLinearColor Color = FLinearColor::White;
	FText Label = FText::GetEmpty();
};

class ST66WorldMapWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(ST66WorldMapWidget) {}
		SLATE_ARGUMENT(bool, bMinimap)
		SLATE_ARGUMENT(bool, bShowLabels)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		bMinimap = InArgs._bMinimap;
		bShowLabels = InArgs._bShowLabels;

		// 100k map: half-extent 50000
		FullWorldMin = FVector2D(-50000.f, -50000.f);
		FullWorldMax = FVector2D(50000.f, 50000.f);
		MinimapHalfExtent = 2500.f;

		// Safety: never draw markers outside the widget bounds.
		SetClipping(EWidgetClipping::ClipToBounds);
	}

	void SetSnapshot(const FVector2D& InPlayerWorldXY, const TArray<FT66MapMarker>& InMarkers)
	{
		PlayerWorldXY = InPlayerWorldXY;
		Markers = InMarkers;
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	void SetFullWorldBounds(const FVector2D& InMin, const FVector2D& InMax)
	{
		FullWorldMin = InMin;
		FullWorldMax = InMax;
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	void SetMinimapHalfExtent(float InHalfExtent)
	{
		MinimapHalfExtent = FMath::Max(250.f, InHalfExtent);
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return bMinimap ? FVector2D(240.f, 180.f) : FVector2D(1024.f, 640.f);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const FVector2D Size = AllottedGeometry.GetLocalSize();
		if (Size.X <= 1.f || Size.Y <= 1.f)
		{
			return LayerId;
		}

		// World bounds for projection.
		FVector2D WorldMin = FullWorldMin;
		FVector2D WorldMax = FullWorldMax;
		if (bMinimap)
		{
			WorldMin = PlayerWorldXY - FVector2D(MinimapHalfExtent, MinimapHalfExtent);
			WorldMax = PlayerWorldXY + FVector2D(MinimapHalfExtent, MinimapHalfExtent);
		}
		else
		{
			// Full map: keep full-world scale, but center the view on the player.
			const FVector2D FullSpan = FullWorldMax - FullWorldMin;
			const FVector2D Half = FullSpan * 0.5f;
			WorldMin = PlayerWorldXY - Half;
			WorldMax = PlayerWorldXY + Half;
		}
		const FVector2D WorldSpan = WorldMax - WorldMin;
		if (WorldSpan.X <= 1.f || WorldSpan.Y <= 1.f)
		{
			return LayerId;
		}

		auto WorldToLocal = [&](const FVector2D& W) -> FVector2D
		{
			const float NX = (W.X - WorldMin.X) / WorldSpan.X;
			const float NY = (W.Y - WorldMin.Y) / WorldSpan.Y;
			// Y up in world -> map up on screen (invert Y because Slate Y grows down).
			return FVector2D(NX * Size.X, (1.f - NY) * Size.Y);
		};

		auto ToPaintGeo = [&](const FVector2D& Pos, const FVector2D& LocalSize) -> FPaintGeometry
		{
			return AllottedGeometry.ToPaintGeometry(
				FVector2f(static_cast<float>(LocalSize.X), static_cast<float>(LocalSize.Y)),
				FSlateLayoutTransform(FVector2f(static_cast<float>(Pos.X), static_cast<float>(Pos.Y))));
		};

		// Solid dark background fill.
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			FCoreStyle::Get().GetBrush("WhiteBrush"),
			ESlateDrawEffect::None,
			FLinearColor(0.04f, 0.06f, 0.08f, 0.92f)
		);

		// Background grid (subtle).
		{
			static constexpr int32 GridLines = 6;
			const FLinearColor GridC(1.f, 1.f, 1.f, 0.08f);
			for (int32 i = 1; i < GridLines; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(GridLines);
				const float X = Size.X * T;
				const float Y = Size.Y * T;
				const TArray<FVector2D> V = { FVector2D(X, 0.f), FVector2D(X, Size.Y) };
				const TArray<FVector2D> H = { FVector2D(0.f, Y), FVector2D(Size.X, Y) };
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), V, ESlateDrawEffect::None, GridC, true, 1.f);
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), H, ESlateDrawEffect::None, GridC, true, 1.f);
			}
		}

		// Border outline.
		{
			const TArray<FVector2D> Border = {
				FVector2D(0.f, 0.f), FVector2D(Size.X, 0.f),
				FVector2D(Size.X, Size.Y), FVector2D(0.f, Size.Y),
				FVector2D(0.f, 0.f) };
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(),
				Border, ESlateDrawEffect::None, FLinearColor(0.35f, 0.40f, 0.50f, 0.6f), true, 1.5f);
		}

		// Areas (full map only): Start / Main / Boss.
		if (!bMinimap)
		{
			struct FArea
			{
				FText Label;
				FVector2D Center;
				FVector2D HalfExtents;
				FLinearColor Fill;
				FLinearColor Stroke;
			};

			const TArray<FArea> Areas = {
				{ NSLOCTEXT("T66.Map", "AreaStart", "START"), FVector2D(-10000.f, 0.f), FVector2D(3000.f, 3000.f), FLinearColor(0.20f, 0.60f, 0.95f, 0.10f), FLinearColor(0.20f, 0.60f, 0.95f, 0.30f) },
				{ NSLOCTEXT("T66.Map", "AreaMain",  "MAIN"),  FVector2D(0.f, 0.f),      FVector2D(10000.f, 10000.f), FLinearColor(0.90f, 0.90f, 0.95f, 0.06f), FLinearColor(0.90f, 0.90f, 0.95f, 0.18f) },
				{ NSLOCTEXT("T66.Map", "AreaBoss",  "BOSS"),  FVector2D(10000.f, 0.f),  FVector2D(3000.f, 3000.f), FLinearColor(0.95f, 0.15f, 0.15f, 0.08f), FLinearColor(0.95f, 0.15f, 0.15f, 0.25f) },
			};

			for (const FArea& A : Areas)
			{
				const FVector2D Min = WorldToLocal(A.Center - A.HalfExtents);
				const FVector2D Max = WorldToLocal(A.Center + A.HalfExtents);
				const FVector2D TL(FMath::Min(Min.X, Max.X), FMath::Min(Min.Y, Max.Y));
				const FVector2D BR(FMath::Max(Min.X, Max.X), FMath::Max(Min.Y, Max.Y));
				const FVector2D BoxSize = BR - TL;
				if (BoxSize.X <= 1.f || BoxSize.Y <= 1.f) continue;

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 2,
					ToPaintGeo(TL, BoxSize),
					FCoreStyle::Get().GetBrush("WhiteBrush"),
					ESlateDrawEffect::None,
					A.Fill
				);

				const TArray<FVector2D> Outline = { TL, FVector2D(BR.X, TL.Y), BR, FVector2D(TL.X, BR.Y), TL };
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 3, AllottedGeometry.ToPaintGeometry(), Outline, ESlateDrawEffect::None, A.Stroke, true, 1.0f);

				const FVector2D CenterLocal = WorldToLocal(A.Center);
				FSlateDrawElement::MakeText(
					OutDrawElements,
					LayerId + 4,
					ToPaintGeo(CenterLocal + FVector2D(-50.f, -10.f), FVector2D(100.f, 20.f)),
					A.Label,
					FT66Style::Tokens::FontBold(14),
					ESlateDrawEffect::None,
					FLinearColor(1.f, 1.f, 1.f, 0.65f)
				);
			}
		}

		auto ClampToBounds = [&](const FVector2D& P) -> FVector2D
		{
			return FVector2D(
				FMath::Clamp(P.X, 0.f, Size.X),
				FMath::Clamp(P.Y, 0.f, Size.Y));
		};

		// Player marker (diamond shape, green, always visible and centered on minimap).
		{
			const FVector2D P = ClampToBounds(WorldToLocal(PlayerWorldXY));
			const float R = bMinimap ? 5.f : 6.f;
			const TArray<FVector2D> Diamond = {
				FVector2D(P.X, P.Y - R),  // top
				FVector2D(P.X + R, P.Y),   // right
				FVector2D(P.X, P.Y + R),  // bottom
				FVector2D(P.X - R, P.Y),   // left
				FVector2D(P.X, P.Y - R)   // close
			};
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 5,
				AllottedGeometry.ToPaintGeometry(),
				Diamond,
				ESlateDrawEffect::None,
				FLinearColor(0.20f, 0.95f, 0.35f, 1.0f),
				true,
				2.5f
			);
		}

		// Entity markers (NPCs, gates, enemies, etc.).
		for (const FT66MapMarker& M : Markers)
		{
			const FVector2D P = ClampToBounds(WorldToLocal(M.WorldXY));
			const float R = bMinimap ? 3.5f : 4.5f;
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId + 6,
				ToPaintGeo(P - FVector2D(R, R), FVector2D(R * 2.f, R * 2.f)),
				FCoreStyle::Get().GetBrush("WhiteBrush"),
				ESlateDrawEffect::None,
				FLinearColor(M.Color.R, M.Color.G, M.Color.B, bMinimap ? 0.9f : 0.95f)
			);

			if (bShowLabels && !M.Label.IsEmpty())
			{
				FSlateDrawElement::MakeText(
					OutDrawElements,
					LayerId + 7,
					ToPaintGeo(ClampToBounds(P + FVector2D(6.f, -10.f)), FVector2D(260.f, 20.f)),
					M.Label,
					FT66Style::Tokens::FontBold(bMinimap ? 10 : 12),
					ESlateDrawEffect::None,
					FLinearColor(1.f, 1.f, 1.f, bMinimap ? 0.65f : 0.90f)
				);
			}
		}

		return LayerId + 8;
	}

private:
	bool bMinimap = false;
	bool bShowLabels = false;

	FVector2D FullWorldMin = FVector2D::ZeroVector;
	FVector2D FullWorldMax = FVector2D::ZeroVector;

	FVector2D PlayerWorldXY = FVector2D::ZeroVector;
	TArray<FT66MapMarker> Markers;

	float MinimapHalfExtent = 2500.f;
};

TSharedRef<SWidget> UT66GameplayHUDWidget::RebuildWidget()
{
	return BuildSlateUI();
}

UT66RunStateSubsystem* UT66GameplayHUDWidget::GetRunState() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
}

void UT66GameplayHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshCooldownBar();
	RefreshSpeedRunTimers();
}

void UT66GameplayHUDWidget::RefreshCooldownBar()
{
	if (!CooldownBarFillBox.IsValid()) return;
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;
	APawn* Pawn = PC->GetPawn();
	AT66HeroBase* Hero = Cast<AT66HeroBase>(Pawn);
	if (!Hero || !Hero->CombatComponent)
	{
		CooldownBarFillBox->SetWidthOverride(0.f);
		if (CooldownTimeText.IsValid()) CooldownTimeText->SetText(FText::GetEmpty());
		return;
	}
	float Pct = Hero->CombatComponent->GetAutoAttackCooldownProgress();
	Pct = FMath::Clamp(Pct, 0.f, 1.f);
	CooldownBarFillBox->SetWidthOverride(CooldownBarWidth * Pct);

	// Show remaining time above the bar (e.g. "0.42s").
	if (CooldownTimeText.IsValid())
	{
		const float Interval = Hero->CombatComponent->GetEffectiveFireInterval();
		const float Remaining = FMath::Max(0.f, (1.f - Pct) * Interval);
		if (Remaining > 0.01f)
		{
			CooldownTimeText->SetText(FText::FromString(FString::Printf(TEXT("%.2fs"), Remaining)));
		}
		else
		{
			CooldownTimeText->SetText(NSLOCTEXT("T66.GameplayHUD", "CooldownReady", "READY"));
		}
	}
}

void UT66GameplayHUDWidget::RefreshHeroStats()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	if (StatLevelText.IsValid())
		StatLevelText->SetText(FText::Format(NSLOCTEXT("T66.GameplayHUD", "StatLevel", "Lv.{0}"), FText::AsNumber(RunState->GetHeroLevel())));
	if (StatDamageText.IsValid())
		StatDamageText->SetText(FText::Format(NSLOCTEXT("T66.GameplayHUD", "StatDmg", "Dmg: {0}"), FText::AsNumber(RunState->GetDamageStat())));
	if (StatAttackSpeedText.IsValid())
		StatAttackSpeedText->SetText(FText::Format(NSLOCTEXT("T66.GameplayHUD", "StatAS", "AS: {0}"), FText::AsNumber(RunState->GetAttackSpeedStat())));
	if (StatAttackScaleText.IsValid())
		StatAttackScaleText->SetText(FText::Format(NSLOCTEXT("T66.GameplayHUD", "StatScale", "Scale: {0}"), FText::AsNumber(RunState->GetScaleStat())));
	if (StatArmorText.IsValid())
		StatArmorText->SetText(FText::Format(NSLOCTEXT("T66.GameplayHUD", "StatArmor", "Armor: {0}"), FText::AsNumber(RunState->GetArmorStat())));
	if (StatSpeedText.IsValid())
		StatSpeedText->SetText(FText::FromString(FString::Printf(TEXT("Speed: %.1f"), RunState->GetHeroMoveSpeedMultiplier())));
	if (StatEvasionText.IsValid())
		StatEvasionText->SetText(FText::Format(NSLOCTEXT("T66.GameplayHUD", "StatEva", "Eva: {0}"), FText::AsNumber(RunState->GetEvasionStat())));
	if (StatLuckText.IsValid())
		StatLuckText->SetText(FText::Format(NSLOCTEXT("T66.GameplayHUD", "StatLuck", "Luck: {0}"), FText::AsNumber(RunState->GetLuckStat())));
}

void UT66GameplayHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	RunState->HeartsChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHearts);
	RunState->GoldChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
	RunState->DebtChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
	RunState->InventoryChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->PanelVisibilityChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->ScoreChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
	RunState->StageChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->StageTimerChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshStageAndTimer);
	RunState->SpeedRunTimerChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshSpeedRunTimers);
	RunState->BossChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshBossBar);
	RunState->DifficultyChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->CowardiceGatesTakenChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->IdolsChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->HeroProgressChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->UltimateChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->SurvivalChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->StatusEffectsChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshStatusEffects);
	RunState->TutorialHintChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshTutorialHint);
	RunState->DevCheatsChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		}
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			MV->OnMediaViewerOpenChanged.AddDynamic(this, &UT66GameplayHUDWidget::HandleMediaViewerOpenChanged);
		}
		if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Ach->AchievementsUnlocked.AddDynamic(this, &UT66GameplayHUDWidget::HandleAchievementsUnlocked);
		}
	}

	// Loot prompt should update immediately on overlap changes (no stage-timer polling).
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->NearbyLootBagChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshLootPrompt);
	}

	// Map/minimap refresh (lightweight, throttled timer; no per-frame UI thinking).
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(MapRefreshTimerHandle, this, &UT66GameplayHUDWidget::RefreshMapData, 0.5f, true);
		World->GetTimerManager().SetTimer(FPSTimerHandle, this, &UT66GameplayHUDWidget::RefreshFPS, 0.25f, true);
	}

	// Bottom-left HUD scale 0.8 (anchor bottom-left)
	if (BottomLeftHUDBox.IsValid())
	{
		BottomLeftHUDBox->SetRenderTransformPivot(FVector2D(0.f, 1.f));
		BottomLeftHUDBox->SetRenderTransform(FSlateRenderTransform(FTransform2D(0.8f)));
	}
	RefreshHUD();
}

void UT66GameplayHUDWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MapRefreshTimerHandle);
		World->GetTimerManager().ClearTimer(FPSTimerHandle);
		World->GetTimerManager().ClearTimer(TikTokOverlaySyncHandle);
		World->GetTimerManager().ClearTimer(WheelSpinTickHandle);
		World->GetTimerManager().ClearTimer(WheelResolveHandle);
		World->GetTimerManager().ClearTimer(WheelCloseHandle);
		World->GetTimerManager().ClearTimer(AchievementNotificationTimerHandle);
	}

	UT66RunStateSubsystem* RunState = GetRunState();
	if (RunState)
	{
		RunState->HeartsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHearts);
		RunState->GoldChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
		RunState->DebtChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
		RunState->InventoryChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->PanelVisibilityChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->ScoreChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
		RunState->StageChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->StageTimerChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshStageAndTimer);
		RunState->SpeedRunTimerChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshSpeedRunTimers);
		RunState->BossChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshBossBar);
		RunState->DifficultyChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->CowardiceGatesTakenChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->IdolsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->HeroProgressChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->UltimateChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->SurvivalChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->TutorialHintChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshTutorialHint);
		RunState->DevCheatsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->StatusEffectsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshStatusEffects);
	}
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		}
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			MV->OnMediaViewerOpenChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::HandleMediaViewerOpenChanged);
		}
		if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Ach->AchievementsUnlocked.RemoveDynamic(this, &UT66GameplayHUDWidget::HandleAchievementsUnlocked);
		}
	}

	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->NearbyLootBagChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshLootPrompt);
	}
	Super::NativeDestruct();
}

bool UT66GameplayHUDWidget::IsTikTokPlaceholderVisible() const
{
	return IsMediaViewerOpen();
}

void UT66GameplayHUDWidget::ToggleTikTokPlaceholder()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			MV->ToggleMediaViewer();
			return;
		}
	}
}

bool UT66GameplayHUDWidget::IsMediaViewerOpen() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (const UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			return MV->IsMediaViewerOpen();
		}
	}
	return false;
}

void UT66GameplayHUDWidget::HandleMediaViewerOpenChanged(bool /*bIsOpen*/)
{
	UpdateTikTokVisibility();
}

void UT66GameplayHUDWidget::HandleAchievementsUnlocked(const TArray<FName>& NewlyUnlockedIDs)
{
	AchievementNotificationQueue.Append(NewlyUnlockedIDs);
	ShowNextAchievementNotification();
}

void UT66GameplayHUDWidget::ShowNextAchievementNotification()
{
	if (AchievementNotificationQueue.Num() == 0)
	{
		if (AchievementNotificationBox.IsValid())
		{
			AchievementNotificationBox->SetVisibility(EVisibility::Collapsed);
		}
		return;
	}
	UGameInstance* GI = GetGameInstance();
	UT66AchievementsSubsystem* Ach = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	if (!Ach || !AchievementNotificationBorder.IsValid() || !AchievementNotificationTitleText.IsValid() || !AchievementNotificationBox.IsValid())
	{
		return;
	}
	const FName AchievementID = AchievementNotificationQueue[0];
	const TArray<FAchievementData> All = Ach->GetAllAchievements();
	const FAchievementData* Data = All.FindByPredicate([&AchievementID](const FAchievementData& A) { return A.AchievementID == AchievementID; });
	if (!Data)
	{
		AchievementNotificationQueue.RemoveAt(0);
		ShowNextAchievementNotification();
		return;
	}
	auto GetTierBorderColor = [](ET66AchievementTier Tier) -> FLinearColor
	{
		switch (Tier)
		{
			case ET66AchievementTier::Black: return FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
			case ET66AchievementTier::Red:   return FLinearColor(0.6f, 0.15f, 0.15f, 1.0f);
			case ET66AchievementTier::Yellow: return FLinearColor(0.6f, 0.5f, 0.1f, 1.0f);
			case ET66AchievementTier::White: return FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
			default: return FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
		}
	};
	AchievementNotificationBorder->SetBorderBackgroundColor(GetTierBorderColor(Data->Tier));
	AchievementNotificationTitleText->SetText(Data->DisplayName);
	AchievementNotificationBox->SetVisibility(EVisibility::Visible);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AchievementNotificationTimerHandle, this, &UT66GameplayHUDWidget::HideAchievementNotificationAndShowNext, AchievementNotificationDisplaySeconds, false);
	}
}

void UT66GameplayHUDWidget::HideAchievementNotificationAndShowNext()
{
	if (AchievementNotificationQueue.Num() > 0)
	{
		AchievementNotificationQueue.RemoveAt(0);
	}
	ShowNextAchievementNotification();
}

void UT66GameplayHUDWidget::RequestTikTokWebView2OverlaySync()
{
#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
	// Defer one tick so Slate has a valid cached geometry for the panel.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TikTokOverlaySyncHandle);
		World->GetTimerManager().SetTimerForNextTick(this, &UT66GameplayHUDWidget::SyncTikTokWebView2OverlayToPlaceholder);
	}
#endif
}

void UT66GameplayHUDWidget::SyncTikTokWebView2OverlayToPlaceholder()
{
#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
	if (!IsMediaViewerOpen()) return;
	// Use the inner video box (below tabs) so the WebView2 overlay doesn't cover the tab buttons.
	TSharedPtr<SBox> SyncBox = MediaViewerVideoBox.IsValid() ? MediaViewerVideoBox : TikTokPlaceholderBox;
	if (!SyncBox.IsValid()) return;
	if (!FSlateApplication::IsInitialized()) return;

	// If the widget is collapsed, geometry may be invalid. Guard against 0-size.
	const FGeometry Geo = SyncBox->GetCachedGeometry();
	const FVector2D LocalSize = Geo.GetLocalSize();
	if (LocalSize.X < 4.f || LocalSize.Y < 4.f)
	{
		return;
	}

	const TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(SyncBox.ToSharedRef());
	if (!Window.IsValid())
	{
		return;
	}

	// Compute rect in window client coordinates (pixels). In practice, Slate's "screen space" aligns with window screen space.
	const FVector2D AbsTL = Geo.LocalToAbsolute(FVector2D::ZeroVector);
	const FVector2D AbsBR = Geo.LocalToAbsolute(LocalSize);

	// Treat Slate absolute space as desktop screen coordinates, then let Win32 do ScreenToClient against the real HWND.
	const int32 X0 = FMath::RoundToInt(AbsTL.X);
	const int32 Y0 = FMath::RoundToInt(AbsTL.Y);
	const int32 X1 = FMath::RoundToInt(AbsBR.X);
	const int32 Y1 = FMath::RoundToInt(AbsBR.Y);
	const FIntRect Rect(FIntPoint(X0, Y0), FIntPoint(X1, Y1));

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			MV->SetTikTokWebView2ScreenRect(Rect);
		}
	}
#endif
}

void UT66GameplayHUDWidget::UpdateTikTokVisibility()
{
	const bool bOpen = IsMediaViewerOpen();

#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
	// On Windows, TikTok UI is rendered by a native WebView2 overlay, but we keep the Slate panel visible
	// as a layout anchor so we can position the overlay correctly.
	const EVisibility Vis = bOpen ? EVisibility::Visible : EVisibility::Collapsed;
#else
	const EVisibility Vis = bOpen ? EVisibility::Visible : EVisibility::Collapsed;
#endif
	if (TikTokPlaceholderBox.IsValid())
	{
		TikTokPlaceholderBox->SetVisibility(Vis);
	}
	UE_LOG(LogTemp, Log, TEXT("[TIKTOK] Viewer %s"), bOpen ? TEXT("OPEN") : TEXT("CLOSED"));
	if (bOpen)
	{
#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
		// Align native WebView2 overlay to the phone panel location (and keep it aligned if window DPI/position changes).
		RequestTikTokWebView2OverlaySync();
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TikTokOverlaySyncHandle);
			World->GetTimerManager().SetTimer(TikTokOverlaySyncHandle, this, &UT66GameplayHUDWidget::SyncTikTokWebView2OverlayToPlaceholder, 0.50f, true);
		}
#endif
	}
	else
	{
#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TikTokOverlaySyncHandle);
		}
#endif
	}
}

void UT66GameplayHUDWidget::StartWheelSpin(ET66Rarity WheelRarity)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Ensure panels exist.
	if (!WheelSpinBox.IsValid() || !WheelSpinDisk.IsValid() || !WheelSpinText.IsValid())
	{
		return;
	}

	// Reset any previous state.
	World->GetTimerManager().ClearTimer(WheelSpinTickHandle);
	World->GetTimerManager().ClearTimer(WheelResolveHandle);
	World->GetTimerManager().ClearTimer(WheelCloseHandle);

	ActiveWheelRarity = WheelRarity;
	bWheelPanelOpen = true;
	bWheelSpinning = true;
	WheelSpinElapsed = 0.f;
	WheelSpinDuration = 2.0f;
	WheelStartAngleDeg = 0.f;
	WheelTotalAngleDeg = 0.f;
	WheelLastTickTimeSeconds = static_cast<float>(World->GetTimeSeconds());

	// Roll pending gold immediately; award on resolve.
	FRandomStream SpinRng(static_cast<int32>(FPlatformTime::Cycles())); // visual-only randomness (not luck-affected)

	int32 PendingGold = 50;
	int32 MinGold = 0;
	int32 MaxGold = 0;
	bool bHasGoldRange = false;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RngSubsystem* RngSub = GI->GetSubsystem<UT66RngSubsystem>())
		{
			UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
			if (RunState)
			{
				RngSub->UpdateLuckStat(RunState->GetLuckStat());
			}

			const UT66RngTuningConfig* Tuning = RngSub->GetTuning();
			if (Tuning)
			{
				const FT66FloatRange Range =
					(WheelRarity == ET66Rarity::Black) ? Tuning->WheelGoldRange_Black :
					(WheelRarity == ET66Rarity::Red) ? Tuning->WheelGoldRange_Red :
					(WheelRarity == ET66Rarity::Yellow) ? Tuning->WheelGoldRange_Yellow :
					(WheelRarity == ET66Rarity::White) ? Tuning->WheelGoldRange_White :
					Tuning->WheelGoldRange_Black;

				MinGold = FMath::FloorToInt(FMath::Min(Range.Min, Range.Max));
				MaxGold = FMath::CeilToInt(FMath::Max(Range.Min, Range.Max));
				bHasGoldRange = true;

				FRandomStream& Stream = RngSub->GetRunStream();
				PendingGold = FMath::Max(0, FMath::RoundToInt(RngSub->RollFloatRangeBiased(Range, Stream)));
			}

			if (RunState && bHasGoldRange)
			{
				RunState->RecordLuckQuantityRoll(FName(TEXT("WheelGold")), PendingGold, MinGold, MaxGold);
			}
		}
	}
	WheelPendingGold = PendingGold;

	// Tint the wheel texture by rarity color.
	WheelSpinDisk->SetColorAndOpacity(FT66RarityUtil::GetRarityColor(WheelRarity));
	WheelSpinText->SetText(NSLOCTEXT("T66.Wheel", "Spinning", "Spinning..."));

	WheelSpinBox->SetVisibility(EVisibility::Visible);

	// Big spin: multiple rotations + random offset.
	WheelTotalAngleDeg = static_cast<float>(SpinRng.RandRange(5, 9)) * 360.f + static_cast<float>(SpinRng.RandRange(0, 359));

	// 30Hz is plenty for a simple HUD spin and reduces timer overhead on low-end CPUs.
	World->GetTimerManager().SetTimer(WheelSpinTickHandle, this, &UT66GameplayHUDWidget::TickWheelSpin, 0.033f, true);
}

void UT66GameplayHUDWidget::TickWheelSpin()
{
	if (!bWheelSpinning || !WheelSpinDisk.IsValid()) return;
	UWorld* World = GetWorld();
	if (!World) return;

	const float Now = static_cast<float>(World->GetTimeSeconds());
	float Delta = Now - WheelLastTickTimeSeconds;
	WheelLastTickTimeSeconds = Now;
	Delta = FMath::Clamp(Delta, 0.f, 0.05f);

	WheelSpinElapsed += Delta;
	const float Alpha = FMath::Clamp(WheelSpinElapsed / FMath::Max(0.01f, WheelSpinDuration), 0.f, 1.f);
	const float Ease = FMath::InterpEaseOut(0.f, 1.f, Alpha, 3.f);
	const float Angle = WheelStartAngleDeg + (WheelTotalAngleDeg * Ease);

	WheelSpinDisk->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	WheelSpinDisk->SetRenderTransform(FSlateRenderTransform(FTransform2D(FQuat2D(FMath::DegreesToRadians(Angle)))));

	if (Alpha >= 1.f)
	{
		ResolveWheelSpin();
	}
}

void UT66GameplayHUDWidget::ResolveWheelSpin()
{
	if (!bWheelSpinning) return;
	bWheelSpinning = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WheelSpinTickHandle);
	}

	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		if (WheelPendingGold > 0)
		{
			RunState->AddGold(WheelPendingGold);
		}
	}

	if (WheelSpinText.IsValid())
	{
		WheelSpinText->SetText(FText::Format(
			NSLOCTEXT("T66.Wheel", "YouWonGoldFormat", "You won {0} gold."),
			FText::AsNumber(WheelPendingGold)));
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(WheelCloseHandle, this, &UT66GameplayHUDWidget::CloseWheelSpin, 1.0f, false);
	}
}

void UT66GameplayHUDWidget::CloseWheelSpin()
{
	bWheelPanelOpen = false;
	if (WheelSpinBox.IsValid())
	{
		WheelSpinBox->SetVisibility(EVisibility::Collapsed);
	}
}

FReply UT66GameplayHUDWidget::OnToggleImmortality()
{
	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		RunState->ToggleDevImmortality();
	}
	return FReply::Handled();
}

FReply UT66GameplayHUDWidget::OnTogglePower()
{
	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		RunState->ToggleDevPower();
	}
	return FReply::Handled();
}

void UT66GameplayHUDWidget::SetFullMapOpen(bool bOpen)
{
	bFullMapOpen = bOpen;
	if (FullMapOverlayBorder.IsValid())
	{
		FullMapOverlayBorder->SetVisibility(bFullMapOpen ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (bFullMapOpen)
	{
		RefreshMapData();
	}
}

void UT66GameplayHUDWidget::ToggleFullMap()
{
	SetFullMapOpen(!bFullMapOpen);
}

void UT66GameplayHUDWidget::RefreshFPS()
{
	if (!FPSText.IsValid()) return;
	UWorld* World = GetWorld();
	const float Delta = World ? World->GetDeltaSeconds() : 0.f;
	const int32 FPS = (Delta > 0.f) ? FMath::RoundToInt(1.f / Delta) : 0;
	FPSText->SetText(FText::FromString(FString::Printf(TEXT("FPS: %d"), FPS)));
}

void UT66GameplayHUDWidget::RefreshMapData()
{
	if (!MinimapWidget.IsValid() && !FullMapWidget.IsValid())
	{
		return;
	}

	// If neither minimap nor full map is visible, skip all work.
	const bool bMinimapVisible = MinimapPanelBox.IsValid() && (MinimapPanelBox->GetVisibility() != EVisibility::Collapsed);
	if (!bMinimapVisible && !bFullMapOpen)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FLagScopedScope LagScope(World, TEXT("RefreshMapData (4x TActorIterator)"));

	// Try multiple paths to find the player pawn — GetOwningPlayerPawn can return null
	// if the pawn hasn't been possessed yet when the HUD widget was first created.
	const APawn* P = GetOwningPlayerPawn();
	if (!P)
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			P = PC->GetPawn();
		}
	}
	if (!P)
	{
		P = UGameplayStatics::GetPlayerPawn(World, 0);
	}
	const FVector PL = P ? P->GetActorLocation() : FVector::ZeroVector;
	const FVector2D PlayerXY(PL.X, PL.Y);

	TArray<FT66MapMarker> Markers;
	Markers.Reserve(64);

	// NPCs (colored by NPCColor)
	for (TActorIterator<AT66HouseNPCBase> It(World); It; ++It)
	{
		const AT66HouseNPCBase* NPC = *It;
		if (!NPC) continue;
		const FVector L = NPC->GetActorLocation();
		FT66MapMarker M;
		M.WorldXY = FVector2D(L.X, L.Y);
		M.Color = NPC->NPCColor;
		M.Label = NPC->NPCName;
		Markers.Add(M);
	}

	// Stage gate (bright blue)
	for (TActorIterator<AT66StageGate> It(World); It; ++It)
	{
		const AActor* A = *It;
		if (!A) continue;
		const FVector L = A->GetActorLocation();
		FT66MapMarker M;
		M.WorldXY = FVector2D(L.X, L.Y);
		M.Color = FLinearColor(0.25f, 0.55f, 1.0f, 1.f);
		M.Label = NSLOCTEXT("T66.Map", "Gate", "GATE");
		Markers.Add(M);
	}

	// Enemies (red dots, no labels on minimap to avoid clutter)
	for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
	{
		const AT66EnemyBase* Enemy = *It;
		if (!IsValid(Enemy)) continue;
		const FVector L = Enemy->GetActorLocation();
		FT66MapMarker M;
		M.WorldXY = FVector2D(L.X, L.Y);
		M.Color = FLinearColor(0.95f, 0.20f, 0.15f, 0.85f);
		Markers.Add(M);
	}

	// Miasma boundary (purple marker)
	for (TActorIterator<AT66MiasmaBoundary> It(World); It; ++It)
	{
		const AActor* A = *It;
		if (!A) continue;
		const FVector L = A->GetActorLocation();
		FT66MapMarker M;
		M.WorldXY = FVector2D(L.X, L.Y);
		M.Color = FLinearColor(0.65f, 0.15f, 0.85f, 0.6f);
		Markers.Add(M);
	}

	if (MinimapWidget.IsValid())
	{
		MinimapWidget->SetSnapshot(PlayerXY, Markers);
	}
	if (bFullMapOpen && FullMapWidget.IsValid())
	{
		FullMapWidget->SetSnapshot(PlayerXY, Markers);
	}
}

void UT66GameplayHUDWidget::RefreshEconomy()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	// Gold
	if (GoldText.IsValid())
	{
		const FText Fmt = Loc ? Loc->GetText_GoldFormat() : NSLOCTEXT("T66.GameplayHUD", "GoldFormat", "Gold: {0}");
		GoldText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentGold())));
	}

	// Owe (Debt) in red
	if (DebtText.IsValid())
	{
		const FText Fmt = Loc ? Loc->GetText_OweFormat() : NSLOCTEXT("T66.GameplayHUD", "OweFormat", "Owe: {0}");
		DebtText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentDebt())));
	}

	// Score (Bounty)
	if (ScoreText.IsValid())
	{
		ScoreText->SetText(FText::AsNumber(RunState->GetCurrentScore()));
	}
	if (ScoreMultiplierText.IsValid())
	{
		FNumberFormattingOptions Opt;
		Opt.MinimumFractionalDigits = 1;
		Opt.MaximumFractionalDigits = 1;
		const FText ScalarText = FText::AsNumber(RunState->GetDifficultyScalar(), &Opt);
		ScoreMultiplierText->SetText(FText::Format(
			NSLOCTEXT("T66.GameplayHUD", "ScoreMultiplierFormat", "x{0}"),
			ScalarText));
	}
}

void UT66GameplayHUDWidget::RefreshTutorialHint()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	// Tutorial hint (above crosshair)
	if (TutorialHintBorder.IsValid() && TutorialHintLine1Text.IsValid() && TutorialHintLine2Text.IsValid())
	{
		const bool bShow = RunState->IsTutorialHintVisible() && (!RunState->GetTutorialHintLine1().IsEmpty() || !RunState->GetTutorialHintLine2().IsEmpty());
		TutorialHintBorder->SetVisibility(bShow ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
		if (bShow)
		{
			const FText L1 = RunState->GetTutorialHintLine1();
			const FText L2 = RunState->GetTutorialHintLine2();
			TutorialHintLine1Text->SetText(L1);
			TutorialHintLine2Text->SetText(L2);
			TutorialHintLine2Text->SetVisibility(L2.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible);
		}
	}
}

void UT66GameplayHUDWidget::RefreshStageAndTimer()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	// Stage number
	if (StageText.IsValid())
	{
		if (RunState->IsInStageBoost())
		{
			StageText->SetText(NSLOCTEXT("T66.GameplayHUD", "StageBoost", "Stage: Boost"));
		}
		else
		{
			const FText Fmt = Loc ? Loc->GetText_StageNumberFormat() : NSLOCTEXT("T66.GameplayHUD", "StageNumberFormat", "Stage number: {0}");
			StageText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentStage())));
		}
	}

	// (Central countdown timer removed)
}

void UT66GameplayHUDWidget::RefreshSpeedRunTimers()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	UT66PlayerSettingsSubsystem* PS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LB = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66GameInstance* GIAsT66 = Cast<UT66GameInstance>(GetGameInstance());

	// Time (speedrun timer): counts up after leaving the start area (visibility toggled by player setting)
	if (SpeedRunText.IsValid())
	{
		const bool bShow = PS ? PS->GetSpeedRunMode() : false;
		SpeedRunText->SetVisibility(bShow ? EVisibility::Visible : EVisibility::Collapsed);
		if (bShow)
		{
			const float Secs = FMath::Max(0.f, RunState->GetSpeedRunElapsedSeconds());
			const int32 M = FMath::FloorToInt(Secs / 60.f);
			const int32 S = FMath::FloorToInt(FMath::Fmod(Secs, 60.f));
			const int32 Cs = FMath::FloorToInt(FMath::Fmod(Secs * 100.f, 100.f)); // centiseconds
			FNumberFormattingOptions TwoDigits;
			TwoDigits.MinimumIntegralDigits = 2;
			SpeedRunText->SetText(FText::Format(
				NSLOCTEXT("T66.GameplayHUD", "SpeedRunTimerFormat", "Time {0}:{1}.{2}"),
				FText::AsNumber(M),
				FText::AsNumber(S, &TwoDigits),
				FText::AsNumber(Cs, &TwoDigits)));
		}
	}

	// Speedrun target time (10th place) for the current stage.
	if (SpeedRunTargetText.IsValid())
	{
		const bool bShow = PS ? PS->GetSpeedRunMode() : false;
		if (!bShow || !LB || !GIAsT66 || RunState->IsInStageBoost())
		{
			SpeedRunTargetText->SetVisibility(EVisibility::Collapsed);
		}
		else
		{
			float TargetSeconds = 0.f;
			const int32 Stage = RunState->GetCurrentStage();
			const bool bHasTarget = (Stage >= 1 && Stage <= 10) && LB->GetSpeedRunTarget10Seconds(GIAsT66->SelectedDifficulty, GIAsT66->SelectedPartySize, Stage, TargetSeconds);
			if (!bHasTarget)
			{
				SpeedRunTargetText->SetVisibility(EVisibility::Collapsed);
			}
		else
		{
			const float TSecs = FMath::Max(0.f, TargetSeconds);
			const int32 M = FMath::FloorToInt(TSecs / 60.f);
			const int32 S = FMath::FloorToInt(FMath::Fmod(TSecs, 60.f));
			const int32 Cs = FMath::FloorToInt(FMath::Fmod(TSecs * 100.f, 100.f));
			FNumberFormattingOptions TwoDigits;
			TwoDigits.MinimumIntegralDigits = 2;
			SpeedRunTargetText->SetText(FText::Format(
				NSLOCTEXT("T66.GameplayHUD", "SpeedRunTargetFormat", "TO BEAT {0}:{1}.{2}"),
				FText::AsNumber(M),
				FText::AsNumber(S, &TwoDigits),
				FText::AsNumber(Cs, &TwoDigits)));
			SpeedRunTargetText->SetVisibility(EVisibility::Visible);
		}
		}
	}
}

void UT66GameplayHUDWidget::RefreshBossBar()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	// Boss health bar: visible only when boss awakened
	const bool bBossActive = RunState->GetBossActive();
	if (BossBarContainerBox.IsValid())
	{
		BossBarContainerBox->SetVisibility(bBossActive ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (bBossActive)
	{
		const int32 BossHP = RunState->GetBossCurrentHP();
		const int32 BossMax = FMath::Max(1, RunState->GetBossMaxHP());
		const float Pct = static_cast<float>(BossHP) / static_cast<float>(BossMax);
		const float BarWidth = 600.f;
		if (BossBarFillBox.IsValid())
		{
			BossBarFillBox->SetWidthOverride(FMath::Clamp(BarWidth * Pct, 0.f, BarWidth));
		}
		if (BossBarText.IsValid())
		{
			BossBarText->SetText(FText::Format(
				NSLOCTEXT("T66.Common", "Fraction", "{0}/{1}"),
				FText::AsNumber(BossHP),
				FText::AsNumber(BossMax)));
		}
	}
}

void UT66GameplayHUDWidget::RefreshLootPrompt()
{
	// Loot prompt: top-of-HUD, non-blocking. Accept with F, Reject with RMB.
	if (!LootPromptBox.IsValid())
	{
		return;
	}

	AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer());
	AT66LootBagPickup* Bag = PC ? PC->GetNearbyLootBag() : nullptr;
	if (!Bag)
	{
		LootPromptBox->SetVisibility(EVisibility::Collapsed);
		return;
	}

	FItemData D;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	const bool bHasData = T66GI && T66GI->GetItemData(Bag->GetItemID(), D);
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	const FText ItemNameText = Loc ? Loc->GetText_ItemDisplayName(Bag->GetItemID()) : FText::FromName(Bag->GetItemID());

	if (LootPromptText.IsValid())
	{
		const FText RarityText =
			(Bag->GetLootRarity() == ET66Rarity::Black) ? NSLOCTEXT("T66.Rarity", "Black", "BLACK") :
			(Bag->GetLootRarity() == ET66Rarity::Red) ? NSLOCTEXT("T66.Rarity", "Red", "RED") :
			(Bag->GetLootRarity() == ET66Rarity::Yellow) ? NSLOCTEXT("T66.Rarity", "Yellow", "YELLOW") :
			NSLOCTEXT("T66.Rarity", "White", "WHITE");

		LootPromptText->SetText(FText::Format(
			NSLOCTEXT("T66.GameplayHUD", "LootPromptFormat", "LOOT BAG ({0}): {1}   [F] {2}   [RMB] {3}"),
			RarityText,
			ItemNameText,
			NSLOCTEXT("T66.GameplayHUD", "Accept", "Accept"),
			NSLOCTEXT("T66.GameplayHUD", "Reject", "Reject")));
	}
	if (LootPromptIconBrush.IsValid())
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (bHasData && !D.Icon.IsNull() && TexPool)
		{
			T66SlateTexture::BindSharedBrushAsync(TexPool, D.Icon, this, LootPromptIconBrush, FName(TEXT("HUDLootPrompt")), /*bClearWhileLoading*/ true);
		}
		else
		{
			LootPromptIconBrush->SetResourceObject(nullptr);
		}
	}
	if (LootPromptIconImage.IsValid())
	{
		const bool bShow = bHasData && !D.Icon.IsNull();
		LootPromptIconImage->SetVisibility(bShow ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (LootPromptBorder.IsValid())
	{
		LootPromptBorder->SetBorderBackgroundColor(FT66RarityUtil::GetRarityColor(Bag->GetLootRarity()) * 0.35f + FLinearColor(0.02f, 0.02f, 0.03f, 0.65f));
	}
	LootPromptBox->SetVisibility(EVisibility::Visible);
}

void UT66GameplayHUDWidget::RefreshHearts()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	// Hearts: 5-slot compression with tier colors (red -> blue -> green -> ...)
	const int32 HeartsNow = RunState->GetCurrentHearts();
	int32 Tier = 0;
	int32 Count = 0;
	FT66RarityUtil::ComputeTierAndCount5(HeartsNow, Tier, Count);
	const FLinearColor TierC = FT66RarityUtil::GetTierColor(Tier);
	const FLinearColor EmptyC(0.25f, 0.25f, 0.28f, 0.35f);
	for (int32 i = 0; i < HeartImages.Num(); ++i)
	{
		if (!HeartImages[i].IsValid()) continue;
		const bool bFilled = (i < Count);
		HeartImages[i]->SetColorAndOpacity(bFilled ? TierC : EmptyC);
	}
}

void UT66GameplayHUDWidget::RefreshStatusEffects()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	// Status effect dots (above hearts)
	const bool bBurn = RunState->HasStatusBurn();
	const bool bChill = RunState->HasStatusChill();
	const bool bCurse = RunState->HasStatusCurse();

	auto SetDot = [&](int32 Index, bool bActive, const FLinearColor& C)
	{
		if (StatusEffectDotBoxes.Num() <= Index || StatusEffectDots.Num() <= Index) return;
		if (StatusEffectDotBoxes[Index].IsValid())
		{
			StatusEffectDotBoxes[Index]->SetVisibility(bActive ? EVisibility::Visible : EVisibility::Collapsed);
		}
		if (bActive && StatusEffectDots[Index].IsValid())
		{
			StatusEffectDots[Index]->SetDotColor(C);
		}
	};

	SetDot(0, bBurn, FLinearColor(0.95f, 0.25f, 0.10f, 1.f));
	SetDot(1, bChill, FLinearColor(0.20f, 0.60f, 0.95f, 1.f));
	SetDot(2, bCurse, FLinearColor(0.65f, 0.20f, 0.90f, 1.f));

	if (CurseOverlayBorder.IsValid())
	{
		CurseOverlayBorder->SetVisibility(bCurse ? EVisibility::Visible : EVisibility::Collapsed);
	}
}

void UT66GameplayHUDWidget::RefreshHUD()
{
	UWorld* World = GetWorld();
	if (World)
	{
		const float Now = static_cast<float>(World->GetTimeSeconds());
		if (LastRefreshHUDTime >= 0.f && (Now - LastRefreshHUDTime) < RefreshHUDThrottleSeconds)
		{
			return;
		}
		LastRefreshHUDTime = Now;
	}
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LB = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66GameInstance* GIAsT66 = Cast<UT66GameInstance>(GetGameInstance());

	RefreshEconomy();
	RefreshTutorialHint();

	RefreshStageAndTimer();
	RefreshSpeedRunTimers();
	RefreshBossBar();
	RefreshLootPrompt();

	RefreshHearts();
	RefreshStatusEffects();
	RefreshHeroStats();

	// Portrait: tier color based on MAX heart quantity (not current health).
	if (PortraitBorder.IsValid())
	{
		int32 Tier = 0;
		int32 Count = 0;
		FT66RarityUtil::ComputeTierAndCount5(RunState->GetMaxHearts(), Tier, Count);
		PortraitBorder->SetBorderBackgroundColor(Count > 0 ? FT66RarityUtil::GetTierColor(Tier) : FLinearColor(0.12f, 0.12f, 0.14f, 1.f));
	}
	TSoftObjectPtr<UTexture2D> PortraitSoft;
	if (GIAsT66 && !GIAsT66->SelectedHeroID.IsNone())
	{
		FHeroData HeroData;
		if (GIAsT66->GetHeroData(GIAsT66->SelectedHeroID, HeroData))
		{
			const bool bUseTypeB = (GIAsT66->SelectedHeroBodyType == ET66BodyType::TypeB);
			if (bUseTypeB && !HeroData.PortraitTypeB.IsNull())
			{
				PortraitSoft = HeroData.PortraitTypeB;
			}
			else if (!HeroData.Portrait.IsNull())
			{
				PortraitSoft = HeroData.Portrait;
			}
		}
	}
	const bool bHasPortraitRef = !PortraitSoft.IsNull();

	if (PortraitBrush.IsValid())
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (PortraitSoft.IsNull() || !TexPool)
		{
			PortraitBrush->SetResourceObject(nullptr);
		}
		else
		{
			T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, PortraitBrush, FName(TEXT("HUDPortrait")), /*bClearWhileLoading*/ true);
		}
	}
	if (PortraitImage.IsValid())
	{
		PortraitImage->SetVisibility(bHasPortraitRef ? EVisibility::Visible : EVisibility::Collapsed);
	}

	// Hero level + XP progress ring
	if (LevelRingWidget.IsValid())
	{
		LevelRingWidget->SetPercent(RunState->GetHeroXP01());
	}
	if (LevelText.IsValid())
	{
		LevelText->SetText(FText::AsNumber(RunState->GetHeroLevel()));
	}

	// Ultimate (R) — show cooldown overlay with countdown when on cooldown, hide when ready
	{
		const bool bReady = RunState->IsUltimateReady();
		if (UltimateCooldownOverlay.IsValid())
		{
			UltimateCooldownOverlay->SetVisibility(bReady ? EVisibility::Collapsed : EVisibility::Visible);
		}
		if (UltimateText.IsValid() && !bReady)
		{
			const int32 Sec = FMath::CeilToInt(RunState->GetUltimateCooldownRemainingSeconds());
			UltimateText->SetText(FText::AsNumber(FMath::Max(0, Sec)));
		}
		if (UltimateBorder.IsValid())
		{
			// Subtle glow tint when ready, neutral border otherwise
			UltimateBorder->SetBorderBackgroundColor(bReady ? FLinearColor(0.08f, 0.08f, 0.10f, 1.f) : FLinearColor(0.08f, 0.08f, 0.10f, 1.f));
		}
	}

	// Difficulty (Skulls): 5-slot compression with tier colors (no half-skulls).
	{
		const int32 Skulls = FMath::Max(0, RunState->GetDifficultySkulls());

		// Color tier changes every 5 skulls, but filling within a tier is 1..5.
		// Skull 1-5 => Tier 0, Within 1..5; Skull 6 => Tier 1, Within 1, etc.
		int32 Tier = 0;
		int32 Within = 0;
		if (Skulls > 0)
		{
			Tier = (Skulls - 1) / 5;
			Within = ((Skulls - 1) % 5) + 1;
		}
		const FLinearColor TierC = FT66RarityUtil::GetTierColor(Tier);
		for (int32 i = 0; i < DifficultyImages.Num(); ++i)
		{
			if (!DifficultyImages[i].IsValid()) continue;
			const bool bFilled = (i < Within);
			// Show skull only if slot should be filled; hide empty slots so skulls appear one-by-one.
			DifficultyImages[i]->SetVisibility(bFilled ? EVisibility::Visible : EVisibility::Collapsed);
			if (bFilled)
			{
				// Tier 0 = white skulls; higher tiers get the tier color.
				DifficultyImages[i]->SetColorAndOpacity(Tier == 0 ? FLinearColor::White : TierC);
			}
		}
	}

	// Cowardice (clowns): show N clowns for gates taken this segment (resets after Coliseum).
	{
		const int32 Clowns = FMath::Max(0, RunState->GetCowardiceGatesTaken());
		for (int32 i = 0; i < ClownImages.Num(); ++i)
		{
			if (!ClownImages[i].IsValid()) continue;
			const bool bFilled = (i < Clowns);
			ClownImages[i]->SetVisibility(bFilled ? EVisibility::Visible : EVisibility::Collapsed);
			if (bFilled)
			{
				ClownImages[i]->SetColorAndOpacity(FLinearColor::White);
			}
		}
	}

	// Score multiplier color: theme for initial/tier 0, tier color for higher tiers
	if (ScoreMultiplierText.IsValid())
	{
		const int32 Skulls = FMath::Max(0, RunState->GetDifficultySkulls());
		if (Skulls > 0)
		{
			const int32 Tier = (Skulls - 1) / 5;
			const FLinearColor TierC = FT66RarityUtil::GetTierColor(Tier);
			ScoreMultiplierText->SetColorAndOpacity(Tier == 0 ? FT66Style::Tokens::Text : TierC);
		}
		else
		{
			ScoreMultiplierText->SetColorAndOpacity(FT66Style::Tokens::Text);
		}
	}

	// Dev toggles (immortality / power)
	if (ImmortalityButtonText.IsValid())
	{
		const bool bOn = RunState->GetDevImmortalityEnabled();
		ImmortalityButtonText->SetText(bOn
			? NSLOCTEXT("T66.Dev", "ImmortalityOn", "IMMORTALITY: ON")
			: NSLOCTEXT("T66.Dev", "ImmortalityOff", "IMMORTALITY: OFF"));
		ImmortalityButtonText->SetColorAndOpacity(bOn ? FLinearColor(0.20f, 0.85f, 0.35f, 1.f) : FT66Style::Tokens::Text);
	}
	if (PowerButtonText.IsValid())
	{
		const bool bOn = RunState->GetDevPowerEnabled();
		PowerButtonText->SetText(bOn
			? NSLOCTEXT("T66.Dev", "PowerOn", "POWER: ON")
			: NSLOCTEXT("T66.Dev", "PowerOff", "POWER: OFF"));
		PowerButtonText->SetColorAndOpacity(bOn ? FLinearColor(0.95f, 0.80f, 0.20f, 1.f) : FT66Style::Tokens::Text);
	}

	// Idol slots (3): colored if equipped, dark teal if empty (matches inventory style)
	const TArray<FName>& Idols = RunState->GetEquippedIdols();
	for (int32 i = 0; i < IdolSlotBorders.Num(); ++i)
	{
		if (!IdolSlotBorders[i].IsValid()) continue;
		FLinearColor C = FLinearColor(0.08f, 0.14f, 0.12f, 0.92f);
		TSoftObjectPtr<UTexture2D> IdolIconSoft;
		FText IdolTooltipText = FText::GetEmpty();
		if (i < Idols.Num() && !Idols[i].IsNone())
		{
			C = UT66RunStateSubsystem::GetIdolColor(Idols[i]);
			if (GIAsT66)
			{
				FIdolData IdolData;
				if (GIAsT66->GetIdolData(Idols[i], IdolData))
				{
					if (!IdolData.Icon.IsNull())
					{
						IdolIconSoft = IdolData.Icon;
					}
					// Tooltip: same as altar (category + effect description)
					if (Loc)
					{
						const FText CatName = Loc->GetText_IdolCategoryName(IdolData.Category);
						const FText Tooltip = Loc->GetText_IdolTooltip(Idols[i]);
						IdolTooltipText = FText::Format(
							NSLOCTEXT("T66.IdolAltar", "IdolCardDescFormat", "{0}\n{1}"),
							CatName, Tooltip);
					}
					else
					{
						IdolTooltipText = FText::FromName(Idols[i]);
					}
				}
			}
		}
		IdolSlotBorders[i]->SetBorderBackgroundColor(C);
		IdolSlotBorders[i]->SetToolTip(CreateCustomTooltip(IdolTooltipText));

		if (IdolSlotBrushes.IsValidIndex(i) && IdolSlotBrushes[i].IsValid())
		{
			UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
			if (IdolIconSoft.IsNull() || !TexPool)
			{
				IdolSlotBrushes[i]->SetResourceObject(nullptr);
			}
			else
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, IdolIconSoft, this, IdolSlotBrushes[i], FName(TEXT("HUDIdol"), i + 1), /*bClearWhileLoading*/ true);
			}
		}
		if (IdolSlotImages.IsValidIndex(i) && IdolSlotImages[i].IsValid())
		{
			IdolSlotImages[i]->SetVisibility(!IdolIconSoft.IsNull() ? EVisibility::Visible : EVisibility::Collapsed);
		}

		// Idol level dots (10 max): show one yellow dot per level.
		const int32 Level = RunState->GetEquippedIdolLevelInSlot(i);
		for (int32 d = 0; d < UT66RunStateSubsystem::MaxIdolLevel; ++d)
		{
			const int32 FlatIdx = (i * UT66RunStateSubsystem::MaxIdolLevel) + d;
			if (!IdolLevelDotBorders.IsValidIndex(FlatIdx) || !IdolLevelDotBorders[FlatIdx].IsValid()) continue;
			const bool bShow = (Level > 0) && (d < Level);
			IdolLevelDotBorders[FlatIdx]->SetVisibility(bShow ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}

	// Inventory slots: item color + hover tooltip, grey when empty
	const TArray<FName>& Inv = RunState->GetInventory();
	const TArray<FT66InventorySlot>& InvSlots = RunState->GetInventorySlots();
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	for (int32 i = 0; i < InventorySlotBorders.Num(); ++i)
	{
		if (!InventorySlotBorders[i].IsValid()) continue;

		FLinearColor SlotColor = FLinearColor(0.f, 0.f, 0.f, 0.25f);
		FText Tooltip = FText::GetEmpty();
		TSoftObjectPtr<UTexture2D> SlotIconSoft;
		if (i < Inv.Num() && !Inv[i].IsNone())
		{
			const FName ItemID = Inv[i];
			FItemData D;
			if (T66GI && T66GI->GetItemData(ItemID, D))
			{
				SlotColor = InvSlots.IsValidIndex(i) ? FItemData::GetItemRarityColor(InvSlots[i].Rarity) : FT66Style::Tokens::Panel2;
				TArray<FText> TipLines;
				TipLines.Reserve(8);
				TipLines.Add(Loc ? Loc->GetText_ItemDisplayName(ItemID) : FText::FromName(ItemID));

				// Icon (optional). Do NOT sync-load in gameplay UI; request via the UI texture pool.
				if (!D.Icon.IsNull())
				{
					SlotIconSoft = D.Icon;
				}

				// Main stat line (v1): one foundational stat (excluding Speed), flat numeric bonus.
				auto StatLabel = [&](ET66HeroStatType Type) -> FText
				{
					if (Loc)
					{
						switch (Type)
						{
							case ET66HeroStatType::Damage: return Loc->GetText_Stat_Damage();
							case ET66HeroStatType::AttackSpeed: return Loc->GetText_Stat_AttackSpeed();
							case ET66HeroStatType::AttackScale: return Loc->GetText_Stat_AttackScale();
							case ET66HeroStatType::Armor: return Loc->GetText_Stat_Armor();
							case ET66HeroStatType::Evasion: return Loc->GetText_Stat_Evasion();
							case ET66HeroStatType::Luck: return Loc->GetText_Stat_Luck();
							default: break;
						}
					}
					// Fallback (safe)
					switch (Type)
					{
						case ET66HeroStatType::Damage: return NSLOCTEXT("T66.Stats", "Damage", "Damage");
						case ET66HeroStatType::AttackSpeed: return NSLOCTEXT("T66.Stats", "AttackSpeed", "Attack Speed");
						case ET66HeroStatType::AttackScale: return NSLOCTEXT("T66.Stats", "AttackScale", "Attack Scale");
						case ET66HeroStatType::Armor: return NSLOCTEXT("T66.Stats", "Armor", "Armor");
						case ET66HeroStatType::Evasion: return NSLOCTEXT("T66.Stats", "Evasion", "Evasion");
						case ET66HeroStatType::Luck: return NSLOCTEXT("T66.Stats", "Luck", "Luck");
						default: return FText::GetEmpty();
					}
				};

				const ET66HeroStatType MainType = D.PrimaryStatType;
				int32 MainValue = 0;
				if (RunState)
				{
					const TArray<FT66InventorySlot>& Slots = RunState->GetInventorySlots();
					if (i >= 0 && i < Slots.Num())
					{
						MainValue = Slots[i].Line1RolledValue;
					}
				}

				if (MainValue > 0)
				{
					if (MainType == ET66HeroStatType::AttackScale && RunState)
					{
						const float ScaleMult = RunState->GetHeroScaleMultiplier();
						TipLines.Add(FText::Format(
							NSLOCTEXT("T66.ItemTooltip", "AttackScaleLineFormat", "{0}: +{1} ({2})"),
							StatLabel(MainType), FText::AsNumber(MainValue), FText::FromString(FString::Printf(TEXT("%.1f"), ScaleMult))));
					}
					else
					{
						TipLines.Add(FText::Format(
							NSLOCTEXT("T66.ItemTooltip", "MainStatLineFormat", "{0}: +{1}"),
							StatLabel(MainType),
							FText::AsNumber(MainValue)));
					}
				}
				// Line 2: secondary stat name (match vendor description)
				if (Loc && D.SecondaryStatType != ET66SecondaryStatType::None)
				{
					TipLines.Add(Loc->GetText_SecondaryStatName(D.SecondaryStatType));
				}
				{
					int32 SellValue = D.BaseSellGold;
					if (RunState)
					{
						const TArray<FT66InventorySlot>& Slots = RunState->GetInventorySlots();
						if (i >= 0 && i < Slots.Num())
						{
							SellValue = D.GetSellGoldForRarity(Slots[i].Rarity);
						}
					}
					if (SellValue > 0)
					{
						TipLines.Add(FText::Format(
							NSLOCTEXT("T66.ItemTooltip", "SellValueGold", "Sell: {0} gold"),
							FText::AsNumber(SellValue)));
					}
				}

				Tooltip = TipLines.Num() > 0 ? FText::Join(NSLOCTEXT("T66.Common", "NewLine", "\n"), TipLines) : FText::GetEmpty();
			}
			else
			{
				SlotColor = FLinearColor(0.95f, 0.15f, 0.15f, 1.f);
				Tooltip = FText::FromName(ItemID);
			}
		}
		InventorySlotBorders[i]->SetBorderBackgroundColor(SlotColor);
		InventorySlotBorders[i]->SetToolTip(CreateCustomTooltip(Tooltip));

		if (InventorySlotBrushes.IsValidIndex(i) && InventorySlotBrushes[i].IsValid())
		{
			UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
			if (SlotIconSoft.IsNull() || !TexPool)
			{
				InventorySlotBrushes[i]->SetResourceObject(nullptr);
			}
			else
			{
					T66SlateTexture::BindSharedBrushAsync(TexPool, SlotIconSoft, this, InventorySlotBrushes[i], FName(TEXT("HUDInv"), i + 1), /*bClearWhileLoading*/ true);
			}
		}
		if (InventorySlotImages.IsValidIndex(i) && InventorySlotImages[i].IsValid())
		{
			InventorySlotImages[i]->SetVisibility(!SlotIconSoft.IsNull() ? EVisibility::Visible : EVisibility::Hidden);
		}
	}

	// Panel visibility: each element follows HUD toggle only if enabled in Settings (HUD tab).
	UGameInstance* GIHud = GetGameInstance();
	UT66PlayerSettingsSubsystem* HUDPS = GIHud ? GIHud->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	const bool bPanelsVisible = RunState->GetHUDPanelsVisible();
	auto ElemVis = [HUDPS, bPanelsVisible](bool bAffects) -> EVisibility
	{
		// If this element is not in the toggle set, always visible. Otherwise follow global panels state.
		if (!HUDPS || !bAffects) return EVisibility::Visible;
		return bPanelsVisible ? EVisibility::Visible : EVisibility::Collapsed;
	};
	if (InventoryPanelBox.IsValid()) InventoryPanelBox->SetVisibility(ElemVis(HUDPS ? HUDPS->GetHudToggleAffectsInventory() : true));
	if (MinimapPanelBox.IsValid()) MinimapPanelBox->SetVisibility(ElemVis(HUDPS ? HUDPS->GetHudToggleAffectsMinimap() : true));
	if (IdolSlotsPanelBox.IsValid()) IdolSlotsPanelBox->SetVisibility(ElemVis(HUDPS ? HUDPS->GetHudToggleAffectsIdolSlots() : true));
	if (PortraitStatPanelBox.IsValid()) PortraitStatPanelBox->SetVisibility(ElemVis(HUDPS ? HUDPS->GetHudToggleAffectsPortraitStats() : true));

	// Wheel spin panel: hide when all toggled panels would be collapsed (any one visible is enough to show wheel in its slot).
	const bool bAnyPanelVisible = (!HUDPS || HUDPS->GetHudToggleAffectsInventory() || HUDPS->GetHudToggleAffectsMinimap() || HUDPS->GetHudToggleAffectsIdolSlots() || HUDPS->GetHudToggleAffectsPortraitStats())
		? bPanelsVisible
		: true;
	UpdateTikTokVisibility();
	if (WheelSpinBox.IsValid())
	{
		if (!bAnyPanelVisible)
		{
			WheelSpinBox->SetVisibility(EVisibility::Collapsed);
		}
		else
		{
			WheelSpinBox->SetVisibility(bWheelPanelOpen ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}
}

TSharedRef<SWidget> UT66GameplayHUDWidget::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	const FText StageInit = Loc ? FText::Format(Loc->GetText_StageNumberFormat(), FText::AsNumber(1)) : NSLOCTEXT("T66.GameplayHUD", "StageNumberInit", "Stage number: 1");
	const FText GoldInit = Loc ? FText::Format(Loc->GetText_GoldFormat(), FText::AsNumber(0)) : NSLOCTEXT("T66.GameplayHUD", "GoldInit", "Gold: 0");
	const FText OweInit = Loc ? FText::Format(Loc->GetText_OweFormat(), FText::AsNumber(0)) : NSLOCTEXT("T66.GameplayHUD", "OweInit", "Owe: 0");
	const FText HighScoreLabel = Loc ? Loc->GetText_BountyLabel() : NSLOCTEXT("T66.GameplayHUD", "BountyLabel", "High Score:");
	const FText PortraitLabel = Loc ? Loc->GetText_PortraitPlaceholder() : NSLOCTEXT("T66.GameplayHUD", "PortraitLabel", "PORTRAIT");

	HeartBorders.SetNum(UT66RunStateSubsystem::DefaultMaxHearts);
	HeartImages.SetNum(UT66RunStateSubsystem::DefaultMaxHearts);
	DifficultyBorders.SetNum(5);
	DifficultyImages.SetNum(5);
	ClownImages.SetNum(5);
	IdolSlotBorders.SetNum(UT66RunStateSubsystem::MaxEquippedIdolSlots);
	IdolSlotImages.SetNum(UT66RunStateSubsystem::MaxEquippedIdolSlots);
	IdolSlotBrushes.SetNum(UT66RunStateSubsystem::MaxEquippedIdolSlots);
	IdolLevelDotBorders.SetNum(UT66RunStateSubsystem::MaxEquippedIdolSlots * UT66RunStateSubsystem::MaxIdolLevel);
	InventorySlotBorders.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotImages.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotBrushes.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	StatusEffectDots.SetNum(3);
	StatusEffectDotBoxes.SetNum(3);
	WorldDialogueOptionBorders.SetNum(3);
	WorldDialogueOptionTexts.SetNum(3);
	static constexpr float BossBarWidth = 600.f;

	// Brushes for icons (kept alive by shared pointers).
	if (!LootPromptIconBrush.IsValid())
	{
		LootPromptIconBrush = MakeShared<FSlateBrush>();
		LootPromptIconBrush->DrawAs = ESlateBrushDrawType::Image;
		LootPromptIconBrush->ImageSize = FVector2D(28.f, 28.f);
	}
	if (!PortraitBrush.IsValid())
	{
		PortraitBrush = MakeShared<FSlateBrush>();
		PortraitBrush->DrawAs = ESlateBrushDrawType::Image;
		PortraitBrush->ImageSize = FVector2D(1.f, 1.f);
	}
	if (!UltimateBrush.IsValid())
	{
		UltimateBrush = MakeShared<FSlateBrush>();
		UltimateBrush->DrawAs = ESlateBrushDrawType::Image;
		UltimateBrush->ImageSize = FVector2D(84.f, 84.f);
		UltimateBrush->Tiling = ESlateBrushTileType::NoTile;
		UltimateBrush->SetResourceObject(nullptr);
	}
	// Load KnightULT texture via texture pool
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> UltSoft(FSoftObjectPath(TEXT("/Game/ULTS/KnightULT.KnightULT")));
			T66SlateTexture::BindSharedBrushAsync(TexPool, UltSoft, this, UltimateBrush, FName(TEXT("HUDUltimate")), false);
		}
	}
	// Wheel spin texture
	{
		WheelTextureBrush = FSlateBrush();
		WheelTextureBrush.ImageSize = FVector2D(120.f, 120.f);
		WheelTextureBrush.DrawAs = ESlateBrushDrawType::Image;
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> WheelSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Wheel/Firefly_Gemini_Flash_Remove_background_286654.Firefly_Gemini_Flash_Remove_background_286654")));
			T66SlateTexture::BindBrushAsync(TexPool, WheelSoft, this, WheelTextureBrush, FName(TEXT("HUDWheel")), /*bClearWhileLoading*/ true);
		}
	}
	// Heart sprite brush
	if (!HeartBrush.IsValid())
	{
		HeartBrush = MakeShared<FSlateBrush>();
		HeartBrush->DrawAs = ESlateBrushDrawType::Image;
		HeartBrush->ImageSize = FVector2D(38.f, 38.f);
		HeartBrush->Tiling = ESlateBrushTileType::NoTile;
		HeartBrush->SetResourceObject(nullptr);
	}
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> HeartSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/UI/HEARTS.HEARTS")));
			T66SlateTexture::BindSharedBrushAsync(TexPool, HeartSoft, this, HeartBrush, FName(TEXT("HUDHeart")), false);
		}
	}
	// Skull sprite brush
	if (!SkullBrush.IsValid())
	{
		SkullBrush = MakeShared<FSlateBrush>();
		SkullBrush->DrawAs = ESlateBrushDrawType::Image;
		SkullBrush->ImageSize = FVector2D(38.f, 38.f);
		SkullBrush->Tiling = ESlateBrushTileType::NoTile;
		SkullBrush->SetResourceObject(nullptr);
	}
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> SkullSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/UI/SKULL.SKULL")));
			T66SlateTexture::BindSharedBrushAsync(TexPool, SkullSoft, this, SkullBrush, FName(TEXT("HUDSkull")), false);
		}
	}
	// Clown sprite brush (cowardice gates taken; same size as skull).
	if (!ClownBrush.IsValid())
	{
		ClownBrush = MakeShared<FSlateBrush>();
		ClownBrush->DrawAs = ESlateBrushDrawType::Image;
		ClownBrush->ImageSize = FVector2D(38.f, 38.f);
		ClownBrush->Tiling = ESlateBrushTileType::NoTile;
		ClownBrush->SetResourceObject(nullptr);
	}
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> ClownSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/UI/CLOWN.CLOWN")));
			T66SlateTexture::BindSharedBrushAsync(TexPool, ClownSoft, this, ClownBrush, FName(TEXT("HUDClown")), false);
		}
	}
	for (int32 i = 0; i < IdolSlotBrushes.Num(); ++i)
	{
		IdolSlotBrushes[i] = MakeShared<FSlateBrush>();
		IdolSlotBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
		IdolSlotBrushes[i]->ImageSize = FVector2D(1.f, 1.f);
	}
	for (int32 i = 0; i < InventorySlotBrushes.Num(); ++i)
	{
		InventorySlotBrushes[i] = MakeShared<FSlateBrush>();
		InventorySlotBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
		// Size is assigned below where InvSlotSize is known; keep a safe default now.
		InventorySlotBrushes[i]->ImageSize = FVector2D(42.f, 42.f);
	}

	// Difficulty row (5-slot skull sprites).
	static constexpr float MinimapWidth = 240.f;
	static constexpr float DiffGap = 2.f;
	static constexpr float DiffSize = 44.f;
	TSharedRef<SHorizontalBox> DifficultyRowRef = SNew(SHorizontalBox);
	for (int32 i = 0; i < DifficultyBorders.Num(); ++i)
	{
		TSharedPtr<SImage> DiffImg;
		DifficultyRowRef->AddSlot()
			.AutoWidth()
			.Padding(i == 0 ? 0.f : DiffGap, 0.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(DiffSize)
				.HeightOverride(DiffSize)
				[
					SAssignNew(DiffImg, SImage)
					.Image(SkullBrush.Get())
					.ColorAndOpacity(FLinearColor::White)
					.Visibility(EVisibility::Collapsed) // Start hidden; skulls appear one-by-one
				]
			];
		DifficultyImages[i] = DiffImg;
	}

	// Cowardice row (5-slot clown sprites, below skulls; one per cowardice gate taken).
	TSharedRef<SHorizontalBox> CowardiceRowRef = SNew(SHorizontalBox);
	for (int32 i = 0; i < ClownImages.Num(); ++i)
	{
		TSharedPtr<SImage> ClownImg;
		CowardiceRowRef->AddSlot()
			.AutoWidth()
			.Padding(i == 0 ? 0.f : DiffGap, 0.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(DiffSize)
				.HeightOverride(DiffSize)
				[
					SAssignNew(ClownImg, SImage)
					.Image(ClownBrush.Get())
					.ColorAndOpacity(FLinearColor::White)
					.Visibility(EVisibility::Collapsed)
				]
			];
		ClownImages[i] = ClownImg;
	}

	// Build hearts row first (5 sprite icons). Portrait width should match this row width.
	static constexpr float HeartSize = 48.f;
	static constexpr float HeartPad = 1.f;
	const float HeartsRowWidth = (HeartSize + (HeartPad * 2.f)) * static_cast<float>(UT66RunStateSubsystem::DefaultMaxHearts);
	TSharedRef<SHorizontalBox> HeartsRowRef = SNew(SHorizontalBox);
	for (int32 i = 0; i < UT66RunStateSubsystem::DefaultMaxHearts; ++i)
	{
		TSharedPtr<SImage> HeartImg;
		HeartsRowRef->AddSlot()
			.AutoWidth()
			.Padding(HeartPad, 0.f)
			[
				SNew(SBox)
				.WidthOverride(HeartSize)
				.HeightOverride(HeartSize)
				[
					SAssignNew(HeartImg, SImage)
					.Image(HeartBrush.Get())
					.ColorAndOpacity(FLinearColor(0.9f, 0.2f, 0.2f, 1.f))
				]
			];
		HeartImages[i] = HeartImg;
	}

	// Status effect dots row (above hearts): burn / chill / curse
	TSharedRef<SHorizontalBox> StatusDotsRowRef = SNew(SHorizontalBox);
	for (int32 i = 0; i < 3; ++i)
	{
		TSharedPtr<SBox> DotBox;
		TSharedPtr<ST66DotWidget> Dot;
		StatusDotsRowRef->AddSlot()
			.AutoWidth()
			.Padding(2.f, 0.f)
			[
				SAssignNew(DotBox, SBox)
				.WidthOverride(10.f)
				.HeightOverride(10.f)
				.Visibility(EVisibility::Collapsed)
				[
					SAssignNew(Dot, ST66DotWidget)
				]
			];
		StatusEffectDotBoxes[i] = DotBox;
		StatusEffectDots[i] = Dot;
	}

	// Idol slots: 2x3 grid; size so title + separator + grid fit inside Idols panel (HeartsRowWidth).
	TSharedRef<SGridPanel> IdolSlotsRef = SNew(SGridPanel);
	static constexpr float IdolSlotPad = 4.f;
	static constexpr float IdolPanelOverhead = 50.f; // title + separator + paddings
	const float IdolGridHeight = FMath::Max(0.f, HeartsRowWidth - IdolPanelOverhead);
	const float IdolSlotSize = FMath::Max(18.f, (IdolGridHeight / 3.f) - (IdolSlotPad * 2.f));
	for (int32 i = 0; i < UT66RunStateSubsystem::MaxEquippedIdolSlots; ++i)
	{
		TSharedPtr<SBorder> IdolBorder;
		TSharedPtr<SBorder> Dots[UT66RunStateSubsystem::MaxIdolLevel];
		const int32 Row = i / 2;
		const int32 Col = i % 2;
		IdolSlotsRef->AddSlot(Col, Row)
			.Padding(IdolSlotPad)
			[
				SNew(SBox)
				.WidthOverride(IdolSlotSize)
				.HeightOverride(IdolSlotSize)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
					SAssignNew(IdolBorder, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.25f))
					.Padding(1.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.45f, 0.55f, 0.50f, 0.5f))
						.Padding(0.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.f))
						]
					]
					]
					+ SOverlay::Slot()
					[
						SAssignNew(IdolSlotImages[i], SImage)
						.Image(IdolSlotBrushes.IsValidIndex(i) && IdolSlotBrushes[i].IsValid() ? IdolSlotBrushes[i].Get() : nullptr)
						.ColorAndOpacity(FLinearColor::White)
						.Visibility(EVisibility::Collapsed)
					]
					// Idol level dots (2 rows x 5 cols, bottom-centered)
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Bottom)
					.Padding(0.f, 0.f, 0.f, 4.f)
					[
						SNew(SUniformGridPanel)
						.SlotPadding(FMargin(1.f, 1.f))
						+ SUniformGridPanel::Slot(0, 0)
						[
							SNew(SBox).WidthOverride(6.f).HeightOverride(6.f)
							[
								SAssignNew(Dots[0], SBorder)
								.Visibility(EVisibility::Collapsed)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.95f, 0.80f, 0.20f, 1.f))
							]
						]
						+ SUniformGridPanel::Slot(1, 0)
						[
							SNew(SBox).WidthOverride(6.f).HeightOverride(6.f)
							[
								SAssignNew(Dots[1], SBorder)
								.Visibility(EVisibility::Collapsed)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.95f, 0.80f, 0.20f, 1.f))
							]
						]
						+ SUniformGridPanel::Slot(2, 0)
						[
							SNew(SBox).WidthOverride(6.f).HeightOverride(6.f)
							[
								SAssignNew(Dots[2], SBorder)
								.Visibility(EVisibility::Collapsed)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.95f, 0.80f, 0.20f, 1.f))
							]
						]
						+ SUniformGridPanel::Slot(3, 0)
						[
							SNew(SBox).WidthOverride(6.f).HeightOverride(6.f)
							[
								SAssignNew(Dots[3], SBorder)
								.Visibility(EVisibility::Collapsed)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.95f, 0.80f, 0.20f, 1.f))
							]
						]
						+ SUniformGridPanel::Slot(4, 0)
						[
							SNew(SBox).WidthOverride(6.f).HeightOverride(6.f)
							[
								SAssignNew(Dots[4], SBorder)
								.Visibility(EVisibility::Collapsed)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.95f, 0.80f, 0.20f, 1.f))
							]
						]
						+ SUniformGridPanel::Slot(0, 1)
						[
							SNew(SBox).WidthOverride(6.f).HeightOverride(6.f)
							[
								SAssignNew(Dots[5], SBorder)
								.Visibility(EVisibility::Collapsed)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.95f, 0.80f, 0.20f, 1.f))
							]
						]
						+ SUniformGridPanel::Slot(1, 1)
						[
							SNew(SBox).WidthOverride(6.f).HeightOverride(6.f)
							[
								SAssignNew(Dots[6], SBorder)
								.Visibility(EVisibility::Collapsed)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.95f, 0.80f, 0.20f, 1.f))
							]
						]
						+ SUniformGridPanel::Slot(2, 1)
						[
							SNew(SBox).WidthOverride(6.f).HeightOverride(6.f)
							[
								SAssignNew(Dots[7], SBorder)
								.Visibility(EVisibility::Collapsed)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.95f, 0.80f, 0.20f, 1.f))
							]
						]
						+ SUniformGridPanel::Slot(3, 1)
						[
							SNew(SBox).WidthOverride(6.f).HeightOverride(6.f)
							[
								SAssignNew(Dots[8], SBorder)
								.Visibility(EVisibility::Collapsed)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.95f, 0.80f, 0.20f, 1.f))
							]
						]
						+ SUniformGridPanel::Slot(4, 1)
						[
							SNew(SBox).WidthOverride(6.f).HeightOverride(6.f)
							[
								SAssignNew(Dots[9], SBorder)
								.Visibility(EVisibility::Collapsed)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.95f, 0.80f, 0.20f, 1.f))
							]
						]
					]
				]
			];
		IdolSlotBorders[i] = IdolBorder;
		for (int32 d = 0; d < UT66RunStateSubsystem::MaxIdolLevel; ++d)
		{
			IdolLevelDotBorders[(i * UT66RunStateSubsystem::MaxIdolLevel) + d] = Dots[d];
		}
	}

	// Inventory: 2 rows x 10 columns (20 slots total). RPG-style with transparent slots and thin borders.
	static constexpr int32 InvCols = 10;
	static constexpr int32 InvRows = 2;
	static constexpr float InvSlotSize = 42.f;
	static constexpr float InvSlotPad = 2.f;
	static const FLinearColor InvSlotBorderColor(0.45f, 0.55f, 0.50f, 0.5f);
	for (int32 i = 0; i < InventorySlotBrushes.Num(); ++i)
	{
		if (InventorySlotBrushes[i].IsValid())
		{
			InventorySlotBrushes[i]->ImageSize = FVector2D(InvSlotSize, InvSlotSize);
		}
	}
	TSharedRef<SVerticalBox> InvGridRef = SNew(SVerticalBox);
	int32 SlotIndex = 0;
	for (int32 Row = 0; Row < InvRows; ++Row)
	{
		TSharedRef<SHorizontalBox> RowBox = SNew(SHorizontalBox);
		for (int32 Col = 0; Col < InvCols; ++Col)
		{
			TSharedPtr<SBorder> SlotBorder;
			TSharedPtr<SImage> SlotImage;
			const int32 ThisSlotIndex = SlotIndex;
			RowBox->AddSlot()
				.AutoWidth()
				.Padding(InvSlotPad, InvSlotPad)
				[
					SNew(SBox)
					.WidthOverride(InvSlotSize)
					.HeightOverride(InvSlotSize)
					[
						SNew(SOverlay)
						// Transparent slot bg with thin border outline
						+ SOverlay::Slot()
						[
							SAssignNew(SlotBorder, SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.25f))
							.Padding(1.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(InvSlotBorderColor)
								.Padding(0.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.f))
								]
							]
						]
						// Item icon on top
						+ SOverlay::Slot()
						.Padding(2.f)
						[
							SAssignNew(SlotImage, SImage)
							.Image(InventorySlotBrushes.IsValidIndex(ThisSlotIndex) && InventorySlotBrushes[ThisSlotIndex].IsValid()
								? InventorySlotBrushes[ThisSlotIndex].Get()
								: nullptr)
							.ColorAndOpacity(FLinearColor::White)
						]
					]
				];

			if (InventorySlotBorders.IsValidIndex(ThisSlotIndex))
			{
				InventorySlotBorders[ThisSlotIndex] = SlotBorder;
			}
			if (InventorySlotImages.IsValidIndex(ThisSlotIndex))
			{
				InventorySlotImages[ThisSlotIndex] = SlotImage;
			}
			SlotIndex++;
		}
		InvGridRef->AddSlot().AutoHeight()[ RowBox ];
	}

	TSharedRef<SOverlay> Root = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(0.f, 12.f)
		[
			SAssignNew(BossBarContainerBox, SBox)
			.WidthOverride(BossBarWidth)
			.HeightOverride(28.f)
			.Visibility(EVisibility::Collapsed)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.08f, 0.9f))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Fill)
				[
					SAssignNew(BossBarFillBox, SBox)
					.WidthOverride(BossBarWidth)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.9f, 0.1f, 0.1f, 0.95f))
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SAssignNew(BossBarText, STextBlock)
					.Text(FText::Format(
						NSLOCTEXT("T66.Common", "Fraction", "{0}/{1}"),
						FText::AsNumber(100),
						FText::AsNumber(100)))
					.Font(FT66Style::Tokens::FontBold(16))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
		]
		// Top-center loot prompt (non-blocking)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(0.f, 48.f, 0.f, 0.f)
		[
			SAssignNew(LootPromptBox, SBox)
			.WidthOverride(760.f)
			.HeightOverride(40.f)
			.Visibility(EVisibility::Collapsed)
			[
				SAssignNew(LootPromptBorder, SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 0.65f))
				.Padding(10.f, 6.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(28.f)
						.HeightOverride(28.f)
						[
							SAssignNew(LootPromptIconImage, SImage)
							.Image(LootPromptIconBrush.Get())
							.ColorAndOpacity(FLinearColor::White)
							.Visibility(EVisibility::Collapsed)
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						SAssignNew(LootPromptText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(14))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
				]
			]
		]
		// (Central countdown timer removed — stage timer info available in top-left stats)
		// In-world NPC dialogue (positioned via RenderTransform; hidden by default)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		[
			SAssignNew(WorldDialogueBox, SBox)
			.Visibility(EVisibility::Collapsed)
			.RenderTransform(FSlateRenderTransform(FVector2D(0.f, 0.f)))
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SAssignNew(WorldDialogueOptionBorders[0], SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.08f, 0.85f))
						.Padding(10.f, 6.f)
						[
							SAssignNew(WorldDialogueOptionTexts[0], STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
					[
						SAssignNew(WorldDialogueOptionBorders[1], SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.08f, 0.85f))
						.Padding(10.f, 6.f)
						[
							SAssignNew(WorldDialogueOptionTexts[1], STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
					[
						SAssignNew(WorldDialogueOptionBorders[2], SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.08f, 0.85f))
						.Padding(10.f, 6.f)
						[
							SAssignNew(WorldDialogueOptionTexts[2], STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
					]
				,
				FT66PanelParams(ET66PanelType::Panel).SetPadding(12.f)
			)
			]
		]
		// Media viewer panel (TikTok / Reels / Shorts) with tab buttons above the video area.
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(16.f, 0.f, 0.f, GT66HeartsTopOffsetFromBottom)
		[
			SAssignNew(TikTokPlaceholderBox, SBox)
			.WidthOverride(GT66MediaPanelW)
			.HeightOverride(GT66MediaPanelH)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					// Tab row (TikTok | Shorts)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 2.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.Padding(2.f, 0.f)
						[
							FT66Style::MakeButton(
								FT66ButtonParams(
									NSLOCTEXT("T66.MediaViewer", "Tab_TikTok", "TikTok"),
									FOnClicked::CreateLambda([this]() -> FReply
									{
										if (UGameInstance* GI = GetGameInstance())
										{
											if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
											{
												MV->SetMediaViewerSource(ET66MediaViewerSource::TikTok);
											}
										}
										return FReply::Handled();
									}),
									ET66ButtonType::Neutral
								).SetFontSize(13).SetMinWidth(80.f)
							)
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.Padding(2.f, 0.f)
						[
							FT66Style::MakeButton(
								FT66ButtonParams(
									NSLOCTEXT("T66.MediaViewer", "Tab_Shorts", "Shorts"),
									FOnClicked::CreateLambda([this]() -> FReply
									{
										if (UGameInstance* GI = GetGameInstance())
										{
											if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
											{
												MV->SetMediaViewerSource(ET66MediaViewerSource::Shorts);
											}
										}
										return FReply::Handled();
									}),
									ET66ButtonType::Neutral
								).SetFontSize(13).SetMinWidth(80.f)
							)
						]
					]
					// Video area (WebView2 overlay syncs to this box's geometry)
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 0.55f))
						.Padding(2.f)
						[
							SAssignNew(MediaViewerVideoBox, SBox)
							[
								SAssignNew(TikTokContentBox, SBox)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 1.f))
								]
							]
						]
					]
				,
				FT66PanelParams(ET66PanelType::Panel).SetPadding(6.f)
				)
			]
		]
		// Top-left stats (score + speedrun time) — themed panel and text
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(24.f, 24.f)
		[
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(HighScoreLabel)
						.Font(FT66Style::Tokens::FontBold(20))
						.ColorAndOpacity(FSlateColor(FT66Style::Tokens::Text))
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SAssignNew(ScoreText, STextBlock)
						.Text(FText::AsNumber(0))
						.Font(FT66Style::Tokens::FontBold(20))
						.ColorAndOpacity(FSlateColor(FT66Style::Tokens::Text))
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
					[
						SAssignNew(ScoreMultiplierText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "ScoreMultiplierDefault", "x1.0"))
						.Font(FT66Style::Tokens::FontBold(20))
						.ColorAndOpacity(FSlateColor(FT66Style::Tokens::Text))
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
				[
					SAssignNew(SpeedRunText, STextBlock)
					.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunDefault", "Time 0:00.00"))
					.Font(FT66Style::Tokens::FontBold(20))
					.ColorAndOpacity(FSlateColor(FT66Style::Tokens::Text))
					.Visibility(EVisibility::Collapsed)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
				[
					SAssignNew(SpeedRunTargetText, STextBlock)
					.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunTargetDefault", "TO BEAT --:--.--"))
					.Font(FT66Style::Tokens::FontBold(20))
					.ColorAndOpacity(FSlateColor(FT66Style::Tokens::Text))
					.Visibility(EVisibility::Collapsed)
				],
				FT66PanelParams(ET66PanelType::Panel).SetPadding(12.f)
			)
		]

		// Wheel spin HUD animation (right side)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(24.f, 0.f, 24.f, 0.f)
		[
			SAssignNew(WheelSpinBox, SBox)
			.WidthOverride(160.f)
			.HeightOverride(160.f)
			.Visibility(EVisibility::Collapsed)
			[
				FT66Style::MakePanel(
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
					SNew(SBox)
					.WidthOverride(120.f)
					.HeightOverride(120.f)
					[
						SAssignNew(WheelSpinDisk, SImage)
						.Image(&WheelTextureBrush)
					]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
						.WidthOverride(140.f)
						[
							SAssignNew(WheelSpinText, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(14))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
							.AutoWrapText(true)
						]
					]
				,
					FT66PanelParams(ET66PanelType::Panel).SetPadding(10.f))
			]
		]

		// Bottom-left portrait stack: 20% smaller (scale 0.8); idol slots in Inventory-style panel
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(24.f, 0.f, 0.f, 24.f)
		[
			SAssignNew(BottomLeftHUDBox, SBox)
			[
				SNew(SVerticalBox)
				// Core bottom-left HUD block
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					// Idol slots: bordered panel with "Idols" title; top aligned with portrait (shorter panel)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom).Padding(0.f, 92.f, 0.f, 0.f)
					[
						SAssignNew(IdolSlotsPanelBox, SBox)
						.HeightOverride(HeartsRowWidth)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.30f, 0.38f, 0.35f, 0.85f))
								.Padding(3.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.08f, 0.14f, 0.12f, 0.92f))
									.Padding(FMargin(10.f, 4.f))
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 2.f)
										[
											SNew(STextBlock)
											.Text(NSLOCTEXT("T66.GameplayHUD", "IdolsTitle", "Idols"))
											.Font(FT66Style::Tokens::FontBold(16))
											.ColorAndOpacity(FLinearColor(0.75f, 0.82f, 0.78f, 1.f))
											.Justification(ETextJustify::Center)
										]
										+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(4.f, 0.f, 4.f, 4.f)
										[
											SNew(SBox)
											.HeightOverride(1.f)
											[
												SNew(SBorder)
												.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
												.BorderBackgroundColor(FLinearColor(0.45f, 0.55f, 0.50f, 0.5f))
											]
										]
										+ SVerticalBox::Slot().AutoHeight()
										[
											IdolSlotsRef
										]
									]
								]
							]
						]
					]
				// Portrait + hearts moved right
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom).Padding(16.f, 0.f, 0.f, 0.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
					[
						StatusDotsRowRef
					]
					// Cooldown time text (above cooldown bar)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 2.f)
					[
						SAssignNew(CooldownTimeText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.80f, 0.80f, 0.85f, 0.90f)))
					]
					// Auto-attack cooldown bar (above hearts)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[
						SNew(SBox)
						.WidthOverride(CooldownBarWidth)
						.HeightOverride(CooldownBarHeight)
						[
							SNew(SOverlay)
							// Background track
							+ SOverlay::Slot()
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.10f, 0.85f))
							]
							// Fill
							+ SOverlay::Slot().HAlign(HAlign_Left)
							[
								SAssignNew(CooldownBarFillBox, SBox)
								.WidthOverride(0.f)
								.HeightOverride(CooldownBarHeight)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.85f, 0.85f, 0.90f, 0.95f))
								]
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
					[
						HeartsRowRef
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(HeartsRowWidth)
						.HeightOverride(HeartsRowWidth)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SAssignNew(PortraitBorder, SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.14f, 1.f))
							]
							+ SOverlay::Slot()
							[
								SAssignNew(PortraitImage, SImage)
								.Image(PortraitBrush.Get())
								.ColorAndOpacity(FLinearColor::White)
								.Visibility(EVisibility::Collapsed)
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(PortraitLabel)
								.Font(FT66Style::Tokens::FontBold(12))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
								.Justification(ETextJustify::Center)
								.Visibility_Lambda([this]() -> EVisibility
								{
									return (PortraitBrush.IsValid() && PortraitBrush->GetResourceObject()) ? EVisibility::Collapsed : EVisibility::Visible;
								})
							]
							// EXP ring + level number (Dota 2 style, top-right corner of portrait, fully inside)
							+ SOverlay::Slot()
							.HAlign(HAlign_Right)
							.VAlign(VAlign_Top)
							.Padding(0.f, 2.f, 2.f, 0.f)
							[
								SNew(SBox)
								.WidthOverride(42.f)
								.HeightOverride(42.f)
								[
									SNew(SOverlay)
									// Ring widget draws filled black circle + progress arc
									+ SOverlay::Slot()
									[
										SAssignNew(LevelRingWidget, ST66RingWidget)
									]
									// Level number centered
									+ SOverlay::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									[
										SAssignNew(LevelText, STextBlock)
										.Text(FText::AsNumber(1))
										.Font(FT66Style::Tokens::FontBold(14))
										.ColorAndOpacity(FLinearColor(0.90f, 0.75f, 0.20f, 1.f))
										.Justification(ETextJustify::Center)
									]
								]
							]
						]
					]
				]
				// Hero stats panel (right of portrait): primary stats only, same height as portrait; toggles with T
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom).Padding(12.f, 0.f, 0.f, 0.f)
				[
					SAssignNew(PortraitStatPanelBox, SBox)
					.WidthOverride(120.f)
					.HeightOverride(HeartsRowWidth)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.30f, 0.38f, 0.35f, 0.85f))
							.Padding(3.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.08f, 0.14f, 0.12f, 0.92f))
								.Padding(FMargin(10.f, 4.f))
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 2.f)
									[
										SNew(STextBlock)
										.Text(NSLOCTEXT("T66.GameplayHUD", "StatsTitle", "Stats"))
										.Font(FT66Style::Tokens::FontBold(19))
										.ColorAndOpacity(FLinearColor(0.75f, 0.82f, 0.78f, 1.f))
										.Justification(ETextJustify::Center)
									]
									+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(4.f, 0.f, 4.f, 4.f)
									[
										SNew(SBox)
										.HeightOverride(1.f)
										[
											SNew(SBorder)
											.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
											.BorderBackgroundColor(FLinearColor(0.45f, 0.55f, 0.50f, 0.5f))
										]
									]
									+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 2.f)
									[
										SAssignNew(StatLevelText, STextBlock)
										.Text(FText::FromString(TEXT("Lv.1")))
										.Font(FT66Style::Tokens::FontBold(15))
										.ColorAndOpacity(FLinearColor(0.9f, 0.85f, 0.6f, 1.f))
									]
									+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 2.f)
									[
										SAssignNew(StatDamageText, STextBlock)
										.Text(FText::FromString(TEXT("Dmg: 0")))
										.Font(FT66Style::Tokens::FontBold(14))
										.ColorAndOpacity(FT66Style::Tokens::Text)
									]
									+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 2.f)
									[
										SAssignNew(StatAttackSpeedText, STextBlock)
										.Text(FText::FromString(TEXT("AS: 0")))
										.Font(FT66Style::Tokens::FontBold(14))
										.ColorAndOpacity(FT66Style::Tokens::Text)
									]
									+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 2.f)
									[
										SAssignNew(StatAttackScaleText, STextBlock)
										.Text(FText::FromString(TEXT("Scale: 0")))
										.Font(FT66Style::Tokens::FontBold(14))
										.ColorAndOpacity(FT66Style::Tokens::Text)
									]
									+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 2.f)
									[
										SAssignNew(StatArmorText, STextBlock)
										.Text(FText::FromString(TEXT("Armor: 0")))
										.Font(FT66Style::Tokens::FontBold(14))
										.ColorAndOpacity(FT66Style::Tokens::Text)
									]
									+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 2.f)
									[
										SAssignNew(StatSpeedText, STextBlock)
										.Text(FText::FromString(TEXT("Speed: 1.0")))
										.Font(FT66Style::Tokens::FontBold(14))
										.ColorAndOpacity(FT66Style::Tokens::Text)
									]
									+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 2.f)
									[
										SAssignNew(StatEvasionText, STextBlock)
										.Text(FText::FromString(TEXT("Eva: 0")))
										.Font(FT66Style::Tokens::FontBold(14))
										.ColorAndOpacity(FT66Style::Tokens::Text)
									]
									+ SVerticalBox::Slot().AutoHeight()
									[
										SAssignNew(StatLuckText, STextBlock)
										.Text(FText::FromString(TEXT("Luck: 0")))
										.Font(FT66Style::Tokens::FontBold(14))
										.ColorAndOpacity(FT66Style::Tokens::Text)
									]
								]
							]
						]
					]
				]
				// Ultimate ability icon (to the right of portrait area)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom).Padding(16.f, 0.f, 0.f, 0.f)
				[
					SNew(SBox)
					.WidthOverride(84.f)
					.HeightOverride(84.f)
					[
						SAssignNew(UltimateBorder, SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.10f, 1.f))
						.Padding(0.f)
						[
							SNew(SOverlay)
							// KnightULT texture as background
							+ SOverlay::Slot()
							[
								SAssignNew(UltimateImage, SImage)
								.Image(UltimateBrush.Get())
								.ColorAndOpacity(FLinearColor::White)
							]
							// Cooldown overlay: semi-transparent black + white countdown text
							+ SOverlay::Slot()
							[
								SAssignNew(UltimateCooldownOverlay, SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.65f))
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.Visibility(EVisibility::Collapsed)
								[
									SAssignNew(UltimateText, STextBlock)
									.Text(FText::GetEmpty())
									.Font(FT66Style::Tokens::FontBold(22))
									.ColorAndOpacity(FLinearColor::White)
									.Justification(ETextJustify::Center)
								]
							]
							// Keybind badge (top-left corner, like Dota 2's "Q" badge)
							+ SOverlay::Slot()
							.HAlign(HAlign_Left)
							.VAlign(VAlign_Top)
							[
								SNew(SBox)
								.WidthOverride(22.f)
								.HeightOverride(22.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.75f))
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									[
										SNew(STextBlock)
										.Text(NSLOCTEXT("T66.GameplayHUD", "UltKeybind", "R"))
										.Font(FT66Style::Tokens::FontBold(11))
										.ColorAndOpacity(FLinearColor::White)
										.Justification(ETextJustify::Center)
									]
								]
							]
						]
					]
				]
			]
		]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(24.f, 24.f)
		[
			SAssignNew(MinimapPanelBox, SBox)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBox)
					.WidthOverride(240.f)
					.HeightOverride(180.f)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							FT66Style::MakePanel(
								SAssignNew(MinimapWidget, ST66WorldMapWidget)
								.bMinimap(true)
								.bShowLabels(false),
								FT66PanelParams(ET66PanelType::Panel2).SetPadding(8.f)
							)
						]
					]
				]
				// Stage number beneath minimap, above skulls
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f)
				[
					SAssignNew(StageText, STextBlock)
					.Text(StageInit)
					.Font(FT66Style::Tokens::FontBold(16))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				// Difficulty beneath minimap (no outer box), sized to minimap width
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
				[
					SNew(SBox)
					.WidthOverride(MinimapWidth)
					[
						SNew(SVerticalBox)
						// Difficulty skulls (immediately beneath minimap)
						+ SVerticalBox::Slot().AutoHeight()
						[
							DifficultyRowRef
						]
						// Cowardice clowns (below skulls; one per gate taken this segment)
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
						[
							CowardiceRowRef
						]
						// Immortality toggle (below skulls)
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
						[
							SAssignNew(ImmortalityButton, SButton)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66GameplayHUDWidget::OnToggleImmortality))
							.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
							.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
							.ContentPadding(FMargin(8.f, 4.f))
							[
								SAssignNew(ImmortalityButtonText, STextBlock)
								.Text(NSLOCTEXT("T66.Dev", "ImmortalityOff", "IMMORTALITY: OFF"))
								.Font(FT66Style::Tokens::FontBold(10))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
						]
						// Power toggle (below immortality)
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
						[
							SAssignNew(PowerButton, SButton)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66GameplayHUDWidget::OnTogglePower))
							.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
							.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
							.ContentPadding(FMargin(8.f, 4.f))
							[
								SAssignNew(PowerButtonText, STextBlock)
								.Text(NSLOCTEXT("T66.Dev", "PowerOff", "POWER: OFF"))
								.Font(FT66Style::Tokens::FontBold(10))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
						]
					]
				]
			]
		]
		// Inventory panel moved to bottom-right; FPS above it, then Gold/Owe and grid
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(24.f, 0.f, 24.f, 24.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SAssignNew(FPSText, STextBlock)
				.Text(NSLOCTEXT("T66.GameplayHUD", "FPSDefault", "FPS: 0"))
				.Font(FT66Style::Tokens::FontBold(14))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Right)
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SAssignNew(InventoryPanelBox, SBox)
				[
					// RPG-style inventory panel: dark teal background, ornamental border, transparent slots
					SNew(SOverlay)
				// Outer border (lighter accent)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.30f, 0.38f, 0.35f, 0.85f))
					.Padding(3.f)
					[
						// Inner panel (dark teal)
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.08f, 0.14f, 0.12f, 0.92f))
						.Padding(FMargin(10.f, 8.f))
						[
							SNew(SVerticalBox)
							// "Inventory" title
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 4.f)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.GameplayHUD", "InventoryTitle", "Inventory"))
								.Font(FT66Style::Tokens::FontBold(18))
								.ColorAndOpacity(FLinearColor(0.75f, 0.82f, 0.78f, 1.f))
								.Justification(ETextJustify::Center)
							]
							// Decorative divider line
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(4.f, 0.f, 4.f, 6.f)
							[
								SNew(SBox)
								.HeightOverride(1.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.45f, 0.55f, 0.50f, 0.5f))
								]
							]
							// Gold / Owe row
							+ SVerticalBox::Slot().AutoHeight().Padding(4.f, 0.f, 4.f, 8.f)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								[
									SAssignNew(GoldText, STextBlock)
									.Text(GoldInit)
									.Font(FT66Style::Tokens::FontBold(18))
									.ColorAndOpacity(FLinearColor(0.85f, 0.75f, 0.20f, 1.f))
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
								[
									SAssignNew(DebtText, STextBlock)
									.Text(OweInit)
									.Font(FT66Style::Tokens::FontBold(18))
									.ColorAndOpacity(FT66Style::Tokens::Danger)
								]
							]
							// Item grid
							+ SVerticalBox::Slot().AutoHeight()
							[
								InvGridRef
							]
						]
					]
				]
				// Corner accents (4 small ornamental squares at each corner)
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.55f, 0.65f, 0.58f, 0.9f))
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				[
					SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.55f, 0.65f, 0.58f, 0.9f))
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Bottom)
				[
					SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.55f, 0.65f, 0.58f, 0.9f))
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				[
					SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.55f, 0.65f, 0.58f, 0.9f))
					]
				]
			]
		]
		]
		// Achievement unlock notification (above inventory, tier-colored border)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(24.f, 0.f, 24.f, 280.f)
		[
			SAssignNew(AchievementNotificationBox, SBox)
			.Visibility(EVisibility::Collapsed)
			.WidthOverride(280.f)
			[
				SAssignNew(AchievementNotificationBorder, SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
				.Padding(3.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FT66Style::Tokens::Panel)
					.Padding(FMargin(10.f, 8.f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SAssignNew(AchievementNotificationTitleText, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(16))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.AutoWrapText(true)
							.WrapTextAt(256.f)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.GameplayHUD", "AchievementUnlocked", "Unlocked!"))
							.Font(FT66Style::Tokens::FontRegular(14))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
					]
				]
			]
		]
		// Tutorial hint (above crosshair)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(0.f, -220.f, 0.f, 0.f)
		[
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SAssignNew(TutorialHintLine1Text, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FT66Style::Tokens::FontBold(18))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 0.f)
				[
					SAssignNew(TutorialHintLine2Text, STextBlock)
					.Visibility(EVisibility::Collapsed)
					.Text(FText::GetEmpty())
					.Font(FT66Style::Tokens::FontRegular(14))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					.Justification(ETextJustify::Center)
				]
			,
			FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(12.f, 8.f)).SetVisibility(EVisibility::Collapsed),
			&TutorialHintBorder
		)
		]
		// Center crosshair (screen center; camera unchanged)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(0.f, -140.f, 0.f, 0.f)
		[
			SNew(SBox)
			.WidthOverride(28.f)
			.HeightOverride(28.f)
			[
				SNew(ST66CrosshairWidget)
			]
		]
		// Curse (visibility) overlay (always on top)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(CurseOverlayBorder, SBorder)
			.Visibility(EVisibility::Collapsed)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.05f, 0.0f, 0.08f, 0.40f))
		]
		// Full-screen map overlay (OpenFullMap / M)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(FullMapOverlayBorder, SBorder)
			.Visibility(EVisibility::Collapsed)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.78f))
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					FT66Style::MakePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.Map", "Title", "MAP"))
								.Font(FT66Style::Tokens::FontBold(20))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.Map", "CloseHint", "[M] Close"))
								.Font(FT66Style::Tokens::FontBold(12))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBox)
							.WidthOverride(1100.f)
							.HeightOverride(680.f)
							[
								FT66Style::MakePanel(
									SAssignNew(FullMapWidget, ST66WorldMapWidget)
									.bMinimap(false)
									.bShowLabels(true),
									FT66PanelParams(ET66PanelType::Panel2).SetPadding(10.f)
								)
							]
						]
					,
					FT66PanelParams(ET66PanelType::Panel)
				)
				]
			]
		];

	return Root;
}

static void T66_ApplyWorldDialogueSelection(
	const TArray<TSharedPtr<SBorder>>& OptionBorders,
	const TArray<TSharedPtr<STextBlock>>& OptionTexts,
	int32 SelectedIndex)
{
	for (int32 i = 0; i < OptionBorders.Num(); ++i)
	{
		const bool bSelected = (i == SelectedIndex);
		if (OptionBorders[i].IsValid())
		{
			OptionBorders[i]->SetBorderBackgroundColor(bSelected
				? FLinearColor(0.18f, 0.18f, 0.26f, 0.95f)
				: FLinearColor(0.06f, 0.06f, 0.08f, 0.85f));
		}
		if (OptionTexts.IsValidIndex(i) && OptionTexts[i].IsValid())
		{
			OptionTexts[i]->SetColorAndOpacity(bSelected ? FT66Style::Tokens::Text : FT66Style::Tokens::TextMuted);
		}
	}
}

void UT66GameplayHUDWidget::ShowWorldDialogue(const TArray<FText>& Options, int32 SelectedIndex)
{
	if (!WorldDialogueBox.IsValid()) return;
	if (Options.Num() < 2) return;
	if (WorldDialogueOptionTexts.Num() < 3) return;

	for (int32 i = 0; i < 3; ++i)
	{
		const bool bHasOption = Options.IsValidIndex(i);
		if (WorldDialogueOptionTexts[i].IsValid())
		{
			WorldDialogueOptionTexts[i]->SetText(bHasOption ? Options[i] : FText::GetEmpty());
		}
		if (WorldDialogueOptionBorders.IsValidIndex(i) && WorldDialogueOptionBorders[i].IsValid())
		{
			WorldDialogueOptionBorders[i]->SetVisibility(bHasOption ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}
	T66_ApplyWorldDialogueSelection(WorldDialogueOptionBorders, WorldDialogueOptionTexts, SelectedIndex);
	WorldDialogueBox->SetVisibility(EVisibility::Visible);
}

void UT66GameplayHUDWidget::HideWorldDialogue()
{
	if (!WorldDialogueBox.IsValid()) return;
	WorldDialogueBox->SetVisibility(EVisibility::Collapsed);
}

void UT66GameplayHUDWidget::SetWorldDialogueSelection(int32 SelectedIndex)
{
	T66_ApplyWorldDialogueSelection(WorldDialogueOptionBorders, WorldDialogueOptionTexts, SelectedIndex);
}

void UT66GameplayHUDWidget::SetWorldDialogueScreenPosition(const FVector2D& ScreenPos)
{
	if (!WorldDialogueBox.IsValid()) return;
	WorldDialogueBox->SetRenderTransform(FSlateRenderTransform(ScreenPos));
}

bool UT66GameplayHUDWidget::IsWorldDialogueVisible() const
{
	return WorldDialogueBox.IsValid() && WorldDialogueBox->GetVisibility() == EVisibility::Visible;
}
