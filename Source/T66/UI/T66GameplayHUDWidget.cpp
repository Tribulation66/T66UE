// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66GameplayHUDWidget.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66MediaViewerSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66Rarity.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "UI/Style/T66Style.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "Rendering/DrawElements.h"
#include "Math/TransformCalculus2D.h"
#include "Rendering/SlateRenderTransform.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"

namespace
{
	// TikTok web has a minimum layout width. Keep the original height to avoid overlapping the bottom-left HUD,
	// but widen the panel so TikTok's layout centers properly.
	static constexpr float GT66TikTokPanelW = 420.f;
	static constexpr float GT66TikTokPanelH = 600.f;
}

class ST66RingWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(ST66RingWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		Percent01 = 0.f;
		Thickness = 5.f;
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

		// Background ring.
		{
			const TArray<FVector2D> Pts = MakeCirclePoints(NumSeg);
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				Pts,
				ESlateDrawEffect::None,
				FLinearColor(0.18f, 0.18f, 0.22f, 1.f),
				true,
				Thickness
			);
		}

		// Progress ring.
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
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				Pts,
				ESlateDrawEffect::None,
				FLinearColor(0.20f, 0.90f, 0.35f, 1.f),
				false,
				Thickness
			);
		}

		return LayerId + 2;
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

		FullWorldMin = FVector2D(-13000.f, -7000.f);
		FullWorldMax = FVector2D(13000.f, 7000.f);
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

		// Background grid (cheap, readable).
		{
			static constexpr int32 GridLines = 6;
			const FLinearColor GridC(1.f, 1.f, 1.f, 0.06f);
			for (int32 i = 1; i < GridLines; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(GridLines);
				const float X = Size.X * T;
				const float Y = Size.Y * T;
				const TArray<FVector2D> V = { FVector2D(X, 0.f), FVector2D(X, Size.Y) };
				const TArray<FVector2D> H = { FVector2D(0.f, Y), FVector2D(Size.X, Y) };
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), V, ESlateDrawEffect::None, GridC, true, 1.f);
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), H, ESlateDrawEffect::None, GridC, true, 1.f);
			}
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
				{ NSLOCTEXT("T66.Map", "AreaMain",  "MAIN"),  FVector2D(0.f, 0.f),      FVector2D(7000.f, 7000.f), FLinearColor(0.90f, 0.90f, 0.95f, 0.06f), FLinearColor(0.90f, 0.90f, 0.95f, 0.18f) },
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
					LayerId + 1,
					ToPaintGeo(TL, BoxSize),
					FCoreStyle::Get().GetBrush("WhiteBrush"),
					ESlateDrawEffect::None,
					A.Fill
				);

				const TArray<FVector2D> Outline = { TL, FVector2D(BR.X, TL.Y), BR, FVector2D(TL.X, BR.Y), TL };
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(), Outline, ESlateDrawEffect::None, A.Stroke, true, 1.0f);

				const FVector2D CenterLocal = WorldToLocal(A.Center);
				FSlateDrawElement::MakeText(
					OutDrawElements,
					LayerId + 3,
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

		// Player marker (always) - clamp so it can never render outside the minimap.
		{
			const FVector2D P = ClampToBounds(WorldToLocal(PlayerWorldXY));
			const float R = bMinimap ? 4.f : 5.f;
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId + 4,
				ToPaintGeo(P - FVector2D(R, R), FVector2D(R * 2.f, R * 2.f)),
				FCoreStyle::Get().GetBrush("WhiteBrush"),
				ESlateDrawEffect::None,
				FLinearColor(0.20f, 0.95f, 0.35f, 0.95f)
			);
		}

		// NPC markers.
		for (const FT66MapMarker& M : Markers)
		{
			const FVector2D P = ClampToBounds(WorldToLocal(M.WorldXY));
			const float R = bMinimap ? 3.f : 4.f;
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId + 5,
				ToPaintGeo(P - FVector2D(R, R), FVector2D(R * 2.f, R * 2.f)),
				FCoreStyle::Get().GetBrush("WhiteBrush"),
				ESlateDrawEffect::None,
				FLinearColor(M.Color.R, M.Color.G, M.Color.B, bMinimap ? 0.9f : 0.95f)
			);

			if (bShowLabels && !M.Label.IsEmpty())
			{
				FSlateDrawElement::MakeText(
					OutDrawElements,
					LayerId + 6,
					ToPaintGeo(ClampToBounds(P + FVector2D(6.f, -10.f)), FVector2D(260.f, 20.f)),
					M.Label,
					FT66Style::Tokens::FontBold(bMinimap ? 10 : 12),
					ESlateDrawEffect::None,
					FLinearColor(1.f, 1.f, 1.f, bMinimap ? 0.65f : 0.90f)
				);
			}
		}

		return LayerId + 7;
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

void UT66GameplayHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	RunState->HeartsChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->GoldChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->DebtChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->InventoryChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->PanelVisibilityChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->ScoreChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->StageChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->StageTimerChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->SpeedRunTimerChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->BossChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->DifficultyChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->IdolsChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->HeroProgressChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->UltimateChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->SurvivalChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->StatusEffectsChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);

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
	}

	// Map/minimap refresh (lightweight, throttled timer; no per-frame UI thinking).
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(MapRefreshTimerHandle, this, &UT66GameplayHUDWidget::RefreshMapData, 0.15f, true);
	}
	RefreshHUD();
}

void UT66GameplayHUDWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MapRefreshTimerHandle);
		World->GetTimerManager().ClearTimer(TikTokOverlaySyncHandle);
		World->GetTimerManager().ClearTimer(WheelSpinTickHandle);
		World->GetTimerManager().ClearTimer(WheelResolveHandle);
		World->GetTimerManager().ClearTimer(WheelCloseHandle);
	}

	UT66RunStateSubsystem* RunState = GetRunState();
	if (RunState)
	{
		RunState->HeartsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->GoldChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->DebtChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->InventoryChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->PanelVisibilityChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->ScoreChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->StageChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->StageTimerChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->SpeedRunTimerChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->BossChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->DifficultyChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->IdolsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->HeroProgressChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->UltimateChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->SurvivalChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->StatusEffectsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
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
	if (!TikTokPlaceholderBox.IsValid()) return;
	if (!FSlateApplication::IsInitialized()) return;

	// If the widget is collapsed, geometry may be invalid. Guard against 0-size.
	const FGeometry Geo = TikTokPlaceholderBox->GetCachedGeometry();
	const FVector2D LocalSize = Geo.GetLocalSize();
	if (LocalSize.X < 4.f || LocalSize.Y < 4.f)
	{
		return;
	}

	const TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(TikTokPlaceholderBox.ToSharedRef());
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
			World->GetTimerManager().SetTimer(TikTokOverlaySyncHandle, this, &UT66GameplayHUDWidget::SyncTikTokWebView2OverlayToPlaceholder, 0.25f, true);
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
	FRandomStream Rng(static_cast<int32>(FPlatformTime::Cycles()));
	auto RollWheelGold = [&](ET66Rarity R) -> int32
	{
		switch (R)
		{
		case ET66Rarity::Black:  { static const int32 V[] = { 25, 50, 75, 100 }; return V[Rng.RandRange(0, 3)]; }
		case ET66Rarity::Red:    { static const int32 V[] = { 100, 150, 200, 300 }; return V[Rng.RandRange(0, 3)]; }
		case ET66Rarity::Yellow: { static const int32 V[] = { 200, 300, 400, 600 }; return V[Rng.RandRange(0, 3)]; }
		case ET66Rarity::White:  { static const int32 V[] = { 500, 750, 1000, 1500 }; return V[Rng.RandRange(0, 3)]; }
		default: return 50;
		}
	};
	WheelPendingGold = RollWheelGold(WheelRarity);

	WheelSpinDisk->SetBorderBackgroundColor(FT66RarityUtil::GetRarityColor(WheelRarity));
	WheelSpinText->SetText(NSLOCTEXT("T66.Wheel", "Spinning", "Spinning..."));

	WheelSpinBox->SetVisibility(EVisibility::Visible);

	// Big spin: multiple rotations + random offset.
	WheelTotalAngleDeg = static_cast<float>(Rng.RandRange(5, 9)) * 360.f + static_cast<float>(Rng.RandRange(0, 359));

	World->GetTimerManager().SetTimer(WheelSpinTickHandle, this, &UT66GameplayHUDWidget::TickWheelSpin, 0.016f, true);
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

void UT66GameplayHUDWidget::RefreshMapData()
{
	if (!MinimapWidget.IsValid() && !FullMapWidget.IsValid())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const APawn* P = GetOwningPlayerPawn();
	const FVector PL = P ? P->GetActorLocation() : FVector::ZeroVector;
	const FVector2D PlayerXY(PL.X, PL.Y);

	TArray<FT66MapMarker> Markers;
	Markers.Reserve(16);
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

	if (MinimapWidget.IsValid())
	{
		MinimapWidget->SetSnapshot(PlayerXY, Markers);
	}
	if (bFullMapOpen && FullMapWidget.IsValid())
	{
		FullMapWidget->SetSnapshot(PlayerXY, Markers);
	}
}

void UT66GameplayHUDWidget::RefreshHUD()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LB = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66GameInstance* GIAsT66 = Cast<UT66GameInstance>(GetGameInstance());

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
	// Bounty (Score)
	if (ScoreText.IsValid())
	{
		ScoreText->SetText(FText::AsNumber(RunState->GetCurrentScore()));
	}

	// Stage number: x
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

	// Stage timer: frozen at full until start gate, then countdown (e.g. 6:00, 0:45)
	if (TimerText.IsValid())
	{
		const float Secs = RunState->GetStageTimerSecondsRemaining();
		const int32 Total = FMath::CeilToInt(FMath::Max(0.f, Secs));
		const int32 M = FMath::FloorToInt(Total / 60.f);
		const int32 S = FMath::FloorToInt(FMath::Fmod(static_cast<float>(Total), 60.f));
		FNumberFormattingOptions TwoDigits;
		TwoDigits.MinimumIntegralDigits = 2;
		TimerText->SetText(FText::Format(
			NSLOCTEXT("T66.Common", "MinutesSeconds", "{0}:{1}"),
			FText::AsNumber(M),
			FText::AsNumber(S, &TwoDigits)));
	}

	// Speedrun timer: counts up after leaving the start area (visibility toggled by player setting)
	if (SpeedRunText.IsValid())
	{
		const bool bShow = PS ? PS->GetSpeedRunMode() : false;
		SpeedRunText->SetVisibility(bShow ? EVisibility::Visible : EVisibility::Collapsed);
		if (bShow)
		{
			const float Secs = RunState->GetSpeedRunElapsedSeconds();
			const int32 Total = FMath::FloorToInt(FMath::Max(0.f, Secs));
			const int32 M = FMath::FloorToInt(Total / 60.f);
			const int32 S = FMath::FloorToInt(FMath::Fmod(static_cast<float>(Total), 60.f));
			FNumberFormattingOptions TwoDigits;
			TwoDigits.MinimumIntegralDigits = 2;
			SpeedRunText->SetText(FText::Format(
				NSLOCTEXT("T66.GameplayHUD", "SpeedRunTimerFormat", "SR {0}:{1}"),
				FText::AsNumber(M),
				FText::AsNumber(S, &TwoDigits)));
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
				const int32 Total = FMath::CeilToInt(FMath::Max(0.f, TargetSeconds));
				const int32 M = FMath::FloorToInt(Total / 60.f);
				const int32 S = FMath::FloorToInt(FMath::Fmod(static_cast<float>(Total), 60.f));
				FNumberFormattingOptions TwoDigits;
				TwoDigits.MinimumIntegralDigits = 2;
				SpeedRunTargetText->SetText(FText::Format(
					NSLOCTEXT("T66.GameplayHUD", "SpeedRunTargetFormat", "TO BEAT {0}:{1}"),
					FText::AsNumber(M),
					FText::AsNumber(S, &TwoDigits)));
				SpeedRunTargetText->SetVisibility(EVisibility::Visible);
			}
		}
	}

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

	// Loot prompt: top-of-HUD, non-blocking. Accept with F, Reject with RMB.
	if (LootPromptBox.IsValid())
	{
		AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer());
		AT66LootBagPickup* Bag = PC ? PC->GetNearbyLootBag() : nullptr;
		if (Bag)
		{
			FItemData D;
			UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
			const bool bHasData = T66GI && T66GI->GetItemData(Bag->GetItemID(), D);
			const FString ItemName = bHasData ? D.ItemID.ToString() : Bag->GetItemID().ToString();

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
					FText::FromString(ItemName),
					NSLOCTEXT("T66.GameplayHUD", "Accept", "Accept"),
					NSLOCTEXT("T66.GameplayHUD", "Reject", "Reject")));
			}
			if (LootPromptBorder.IsValid())
			{
				LootPromptBorder->SetBorderBackgroundColor(FT66RarityUtil::GetRarityColor(Bag->GetLootRarity()) * 0.35f + FLinearColor(0.02f, 0.02f, 0.03f, 0.65f));
			}
			LootPromptBox->SetVisibility(EVisibility::Visible);
		}
		else
		{
			LootPromptBox->SetVisibility(EVisibility::Collapsed);
		}
	}

	// Hearts: 5-slot compression with tier colors (red -> blue -> green -> ...)
	{
		const int32 HeartsNow = RunState->GetCurrentHearts();
		int32 Tier = 0;
		int32 Count = 0;
		FT66RarityUtil::ComputeTierAndCount5(HeartsNow, Tier, Count);
		const FLinearColor TierC = FT66RarityUtil::GetTierColor(Tier);
		const FLinearColor EmptyC(0.25f, 0.25f, 0.28f, 1.f);
		for (int32 i = 0; i < HeartBorders.Num(); ++i)
		{
			if (!HeartBorders[i].IsValid()) continue;
			const bool bFilled = (i < Count);
			HeartBorders[i]->SetBorderBackgroundColor(bFilled ? TierC : EmptyC);
		}
	}

	// Status effect dots (above hearts)
	{
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

	// Portrait: tier color based on MAX heart quantity (not current health).
	if (PortraitBorder.IsValid())
	{
		int32 Tier = 0;
		int32 Count = 0;
		FT66RarityUtil::ComputeTierAndCount5(RunState->GetMaxHearts(), Tier, Count);
		PortraitBorder->SetBorderBackgroundColor(Count > 0 ? FT66RarityUtil::GetTierColor(Tier) : FLinearColor(0.12f, 0.12f, 0.14f, 1.f));
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

	// Ultimate (R)
	if (UltimateText.IsValid())
	{
		if (RunState->IsUltimateReady())
		{
			UltimateText->SetText(NSLOCTEXT("T66.GameplayHUD", "Ready", "READY"));
		}
		else
		{
			const int32 Sec = FMath::CeilToInt(RunState->GetUltimateCooldownRemainingSeconds());
			UltimateText->SetText(FText::AsNumber(FMath::Max(0, Sec)));
		}
	}
	if (UltimateBorder.IsValid())
	{
		UltimateBorder->SetBorderBackgroundColor(RunState->IsUltimateReady() ? FLinearColor(0.15f, 0.85f, 0.25f, 0.8f) : FLinearColor(0.10f, 0.10f, 0.12f, 0.8f));
	}

	// Stats panel next to idols (always visible)
	if (StatLineTexts.Num() >= 6)
	{
		// Show raw foundational stat numbers (base + level-ups + items). Speed is not shown here.
		const int32 Dmg = RunState->GetDamageStat();
		const int32 AtkSpd = RunState->GetAttackSpeedStat();
		const int32 Size = RunState->GetScaleStat(); // Attack Size
		const int32 Arm = RunState->GetArmorStat();
		const int32 Eva = RunState->GetEvasionStat();
		const int32 Luck = RunState->GetLuckStat();

		if (StatLineTexts[0].IsValid()) StatLineTexts[0]->SetText(FText::Format(NSLOCTEXT("T66.GameplayHUD", "Stat_Dmg", "DMG: {0}"), FText::AsNumber(Dmg)));
		if (StatLineTexts[1].IsValid()) StatLineTexts[1]->SetText(FText::Format(NSLOCTEXT("T66.GameplayHUD", "Stat_As", "AS:  {0}"), FText::AsNumber(AtkSpd)));
		if (StatLineTexts[2].IsValid()) StatLineTexts[2]->SetText(FText::Format(NSLOCTEXT("T66.GameplayHUD", "Stat_Scl", "SCL: {0}"), FText::AsNumber(Size)));
		if (StatLineTexts[3].IsValid()) StatLineTexts[3]->SetText(FText::Format(NSLOCTEXT("T66.GameplayHUD", "Stat_Arm", "ARM: {0}"), FText::AsNumber(Arm)));
		if (StatLineTexts[4].IsValid()) StatLineTexts[4]->SetText(FText::Format(NSLOCTEXT("T66.GameplayHUD", "Stat_Eva", "EVA: {0}"), FText::AsNumber(Eva)));
		if (StatLineTexts[5].IsValid()) StatLineTexts[5]->SetText(FText::Format(NSLOCTEXT("T66.GameplayHUD", "Stat_Lck", "LCK: {0}"), FText::AsNumber(Luck)));
	}

	// Difficulty (Skulls): 5-slot compression with tier colors + half-step support
	{
		const float Skulls = RunState->GetDifficultySkulls();
		const int32 Tier = FMath::Max(0, FMath::FloorToInt(Skulls / 5.f));
		const float Within = FMath::Clamp(Skulls - (static_cast<float>(Tier) * 5.f), 0.f, 5.f);
		const FLinearColor TierC = FT66RarityUtil::GetTierColor(Tier);
		const FLinearColor EmptyC(0.18f, 0.18f, 0.22f, 1.f);
		for (int32 i = 0; i < DifficultyBorders.Num(); ++i)
		{
			if (!DifficultyBorders[i].IsValid()) continue;
			const float NeedFull = static_cast<float>(i + 1);
			const float NeedHalf = static_cast<float>(i) + 0.5f;
			if (Within >= NeedFull)
			{
				DifficultyBorders[i]->SetBorderBackgroundColor(TierC);
			}
			else if (Within >= NeedHalf)
			{
				DifficultyBorders[i]->SetBorderBackgroundColor(TierC * 0.55f);
			}
			else
			{
				DifficultyBorders[i]->SetBorderBackgroundColor(EmptyC);
			}
		}
	}

	// Idol slots (3): colored if equipped, grey if empty
	const TArray<FName>& Idols = RunState->GetEquippedIdols();
	for (int32 i = 0; i < IdolSlotBorders.Num(); ++i)
	{
		if (!IdolSlotBorders[i].IsValid()) continue;
		FLinearColor C = FLinearColor(0.18f, 0.18f, 0.22f, 1.f);
		if (i < Idols.Num() && !Idols[i].IsNone())
		{
			C = UT66RunStateSubsystem::GetIdolColor(Idols[i]);
		}
		IdolSlotBorders[i]->SetBorderBackgroundColor(C);
	}

	// Inventory slots: item color + hover tooltip, grey when empty
	const TArray<FName>& Inv = RunState->GetInventory();
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	for (int32 i = 0; i < InventorySlotBorders.Num(); ++i)
	{
		if (!InventorySlotBorders[i].IsValid()) continue;

		FLinearColor SlotColor = FLinearColor(0.2f, 0.2f, 0.22f, 1.f);
		FText Tooltip = FText::GetEmpty();
		if (i < Inv.Num() && !Inv[i].IsNone())
		{
			const FName ItemID = Inv[i];
			FItemData D;
			if (T66GI && T66GI->GetItemData(ItemID, D))
			{
				SlotColor = D.PlaceholderColor;
				TArray<FText> TipLines;
				TipLines.Reserve(8);
				TipLines.Add(FText::FromName(ItemID));

				// Main stat line (v1): one foundational stat (excluding Speed), flat numeric bonus.
				auto StatLabel = [&](ET66HeroStatType Type) -> FText
				{
					if (Loc)
					{
						switch (Type)
						{
							case ET66HeroStatType::Damage: return Loc->GetText_Stat_Damage();
							case ET66HeroStatType::AttackSpeed: return Loc->GetText_Stat_AttackSpeed();
							case ET66HeroStatType::AttackSize: return Loc->GetText_Stat_AttackSize();
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
						case ET66HeroStatType::AttackSize: return NSLOCTEXT("T66.Stats", "AttackSize", "Attack Size");
						case ET66HeroStatType::Armor: return NSLOCTEXT("T66.Stats", "Armor", "Armor");
						case ET66HeroStatType::Evasion: return NSLOCTEXT("T66.Stats", "Evasion", "Evasion");
						case ET66HeroStatType::Luck: return NSLOCTEXT("T66.Stats", "Luck", "Luck");
						default: return FText::GetEmpty();
					}
				};

				ET66HeroStatType MainType = D.MainStatType;
				int32 MainValue = D.MainStatValue;
				if (MainValue == 0)
				{
					// Derive from legacy v0 fields until DT_Items is updated.
					switch (D.EffectType)
					{
						case ET66ItemEffectType::BonusDamagePct: MainType = ET66HeroStatType::Damage; MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 10.f); break;
						case ET66ItemEffectType::BonusAttackSpeedPct: MainType = ET66HeroStatType::AttackSpeed; MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 10.f); break;
						case ET66ItemEffectType::BonusArmorPctPoints: MainType = ET66HeroStatType::Armor; MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 4.f); break;
						case ET66ItemEffectType::BonusEvasionPctPoints: MainType = ET66HeroStatType::Evasion; MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 4.f); break;
						case ET66ItemEffectType::BonusLuckFlat: MainType = ET66HeroStatType::Luck; MainValue = FMath::RoundToInt(FMath::Max(0.f, D.EffectMagnitude)); break;
						case ET66ItemEffectType::BonusMoveSpeedPct:
						case ET66ItemEffectType::DashCooldownReductionPct:
							// Speed is not an item stat; map mobility effects to Evasion for now.
							MainType = ET66HeroStatType::Evasion;
							MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 10.f);
							break;
						default: break;
					}
				}

				if (MainValue > 0)
				{
					TipLines.Add(FText::Format(
						NSLOCTEXT("T66.ItemTooltip", "MainStatLineFormat", "{0}: +{1}"),
						StatLabel(MainType),
						FText::AsNumber(MainValue)));
				}
				if (D.SellValueGold > 0)
				{
					TipLines.Add(FText::Format(
						NSLOCTEXT("T66.ItemTooltip", "SellValueGold", "Sell: {0} gold"),
						FText::AsNumber(D.SellValueGold)));
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
		InventorySlotBorders[i]->SetToolTipText(Tooltip);
	}

	// Panel visibility
	const EVisibility PanelVis = RunState->GetHUDPanelsVisible() ? EVisibility::Visible : EVisibility::Collapsed;
	if (InventoryPanelBox.IsValid()) InventoryPanelBox->SetVisibility(PanelVis);
	if (MinimapPanelBox.IsValid()) MinimapPanelBox->SetVisibility(PanelVis);
	if (StatsPanelBox.IsValid()) StatsPanelBox->SetVisibility(PanelVis);

	UpdateTikTokVisibility();
	if (WheelSpinBox.IsValid())
	{
		if (PanelVis == EVisibility::Collapsed)
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
	const FText BountyLabel = Loc ? Loc->GetText_BountyLabel() : NSLOCTEXT("T66.GameplayHUD", "BountyLabel", "Bounty:");
	const FText PortraitLabel = Loc ? Loc->GetText_PortraitPlaceholder() : NSLOCTEXT("T66.GameplayHUD", "PortraitLabel", "PORTRAIT");

	HeartBorders.SetNum(UT66RunStateSubsystem::DefaultMaxHearts);
	DifficultyBorders.SetNum(5);
	IdolSlotBorders.SetNum(UT66RunStateSubsystem::MaxEquippedIdolSlots);
	InventorySlotBorders.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	StatLineTexts.SetNum(6);
	StatusEffectDots.SetNum(3);
	StatusEffectDotBoxes.SetNum(3);
	static constexpr float BossBarWidth = 600.f;

	// Difficulty row (5-slot compression squares) sized to match minimap width.
	static constexpr float MinimapWidth = 240.f;
	static constexpr float DiffGap = 6.f;
	const float DiffSize = FMath::Max(18.f, (MinimapWidth - (DiffGap * 4.f)) / 5.f);
	TSharedRef<SHorizontalBox> DifficultyRowRef = SNew(SHorizontalBox);
	for (int32 i = 0; i < DifficultyBorders.Num(); ++i)
	{
		TSharedPtr<SBorder> DiffBorder;
		DifficultyRowRef->AddSlot()
			.AutoWidth()
			.Padding(i == 0 ? 0.f : DiffGap, 0.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(DiffSize)
				.HeightOverride(DiffSize)
				[
					SAssignNew(DiffBorder, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.18f, 0.18f, 0.22f, 1.f))
				]
			];
		DifficultyBorders[i] = DiffBorder;
	}

	// Build hearts row first (5 icons). Portrait width should match this row width.
	static constexpr float HeartSize = 38.f;
	static constexpr float HeartPad = 3.f;
	const float HeartsRowWidth = (HeartSize + (HeartPad * 2.f)) * static_cast<float>(UT66RunStateSubsystem::DefaultMaxHearts);
	TSharedRef<SHorizontalBox> HeartsRowRef = SNew(SHorizontalBox);
	for (int32 i = 0; i < UT66RunStateSubsystem::DefaultMaxHearts; ++i)
	{
		TSharedPtr<SBorder> HeartBorder;
		HeartsRowRef->AddSlot()
			.AutoWidth()
			.Padding(HeartPad, 0.f)
			[
				SNew(SBox)
				.WidthOverride(HeartSize)
				.HeightOverride(HeartSize)
				[
					SAssignNew(HeartBorder, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.9f, 0.2f, 0.2f, 1.f))
				]
			];
		HeartBorders[i] = HeartBorder;
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

	// Idol slots: 2x3 grid (placeholder for equipped idols).
	TSharedRef<SGridPanel> IdolSlotsRef = SNew(SGridPanel);
	static constexpr float IdolSlotPad = 4.f;
	const float IdolSlotSize = FMath::Max(18.f, (HeartsRowWidth - (IdolSlotPad * 6.f)) / 3.f); // 3 rows tall == portrait height
	for (int32 i = 0; i < UT66RunStateSubsystem::MaxEquippedIdolSlots; ++i)
	{
		TSharedPtr<SBorder> IdolBorder;
		const int32 Row = i / 2;
		const int32 Col = i % 2;
		IdolSlotsRef->AddSlot(Col, Row)
			.Padding(IdolSlotPad)
			[
				SNew(SBox)
				.WidthOverride(IdolSlotSize)
				.HeightOverride(IdolSlotSize)
				[
					SAssignNew(IdolBorder, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.18f, 0.18f, 0.22f, 1.f))
				]
			];
		IdolSlotBorders[i] = IdolBorder;
	}

	// Stats panel (always visible) next to idol slots
	TSharedRef<SVerticalBox> StatsPanelRef = SNew(SVerticalBox);
	auto MakeStatLine = [&](int32 Index) -> TSharedRef<SWidget>
	{
		return SAssignNew(StatLineTexts[Index], STextBlock)
			.Text(FText::GetEmpty())
			.Font(FT66Style::Tokens::FontBold(11))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted);
	};
	StatsPanelRef->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)[ MakeStatLine(0) ];
	StatsPanelRef->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)[ MakeStatLine(1) ];
	StatsPanelRef->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)[ MakeStatLine(2) ];
	StatsPanelRef->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)[ MakeStatLine(3) ];
	StatsPanelRef->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)[ MakeStatLine(4) ];
	StatsPanelRef->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 0.f)[ MakeStatLine(5) ];

	// Wrap stats so we can hide/show them with HUD panels (and so Ultimate shifts left when collapsed).
	TSharedRef<SWidget> StatsPanelWrapped =
		SAssignNew(StatsPanelBox, SBox)
		[
			StatsPanelRef
		];

	// Inventory: 2 rows x 10 columns (20 slots total).
	static constexpr int32 InvCols = 10;
	static constexpr int32 InvRows = 2;
	static constexpr float InvSlotSize = 42.f;
	static constexpr float InvSlotPad = 4.f;
	TSharedRef<SVerticalBox> InvGridRef = SNew(SVerticalBox);
	int32 SlotIndex = 0;
	for (int32 Row = 0; Row < InvRows; ++Row)
	{
		TSharedRef<SHorizontalBox> RowBox = SNew(SHorizontalBox);
		for (int32 Col = 0; Col < InvCols; ++Col)
		{
			TSharedPtr<SBorder> SlotBorder;
			RowBox->AddSlot()
				.AutoWidth()
				.Padding(InvSlotPad, InvSlotPad)
				[
					SNew(SBox)
					.WidthOverride(InvSlotSize)
					.HeightOverride(InvSlotSize)
					[
						SAssignNew(SlotBorder, SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.22f, 1.f))
					]
				];

			if (InventorySlotBorders.IsValidIndex(SlotIndex))
			{
				InventorySlotBorders[SlotIndex] = SlotBorder;
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
					SAssignNew(LootPromptText, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FT66Style::Tokens::FontBold(14))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
		]
		// Top-left stats (no overlap with bottom-left portrait stack)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(24.f, 24.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(BountyLabel)
					.Font(FT66Style::Tokens::FontBold(16))
					.ColorAndOpacity(FLinearColor(0.85f, 0.75f, 0.2f, 1.f))
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SAssignNew(ScoreText, STextBlock)
					.Text(FText::AsNumber(0))
					.Font(FT66Style::Tokens::FontBold(16))
					.ColorAndOpacity(FLinearColor(0.95f, 0.85f, 0.3f, 1.f))
				]
			]
			// Speedrun info below bounty (per request)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
			[
				SAssignNew(SpeedRunText, STextBlock)
				.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunDefault", "SR 0:00"))
				.Font(FT66Style::Tokens::FontBold(16))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Visibility(EVisibility::Collapsed)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
			[
				SAssignNew(SpeedRunTargetText, STextBlock)
				.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunTargetDefault", "TO BEAT --:--"))
				.Font(FT66Style::Tokens::FontBold(12))
				.ColorAndOpacity(FLinearColor(0.75f, 0.75f, 0.82f, 1.f))
				.Visibility(EVisibility::Collapsed)
			]
			// TikTok placeholder (toggle with O). Sized like a phone and placed just under speedrun.
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
			[
				SAssignNew(TikTokPlaceholderBox, SBox)
				.WidthOverride(GT66TikTokPanelW)
				.HeightOverride(GT66TikTokPanelH)
				[
					SNew(SBorder)
					.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
					.Padding(6.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 0.55f))
						.Padding(2.f)
						[
							// Important: no scaling here. Scaling can break browser hit-testing (click/scroll),
							// so we anchor any native overlay to match this box's size/position.
							SAssignNew(TikTokContentBox, SBox)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 1.f))
							]
						]
					]
				]
			]
		]

		// Bottom-left portrait stack: difficulty squares (placeholder skulls) -> hearts -> portrait
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(24.f, 0.f, 0.f, 24.f)
		[
			SNew(SVerticalBox)
			// Wheel spin HUD animation (no overlay; triggered by wheel interact)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				SAssignNew(WheelSpinBox, SBox)
				.Visibility(EVisibility::Collapsed)
				[
					SNew(SBorder)
					.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
					.Padding(10.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(70.f)
							.HeightOverride(70.f)
							[
								SAssignNew(WheelSpinDisk, SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.20f, 0.20f, 0.22f, 1.f))
							]
						]
						+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(12.f, 0.f, 0.f, 0.f)
						[
							SAssignNew(WheelSpinText, STextBlock)
							.Text(NSLOCTEXT("T66.Wheel", "Spinning", "Spinning..."))
							.Font(FT66Style::Tokens::FontBold(14))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
					]
				]
			]
			// Core bottom-left HUD block
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				// 2x3 idol slots (left of portrait)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom)
				[
					IdolSlotsRef
				]
				// Portrait + hearts moved right
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom).Padding(16.f, 0.f, 0.f, 0.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
					[
						StatusDotsRowRef
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
							SAssignNew(PortraitBorder, SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.14f, 1.f))
							[
								SNew(STextBlock)
								.Text(PortraitLabel)
								.Font(FT66Style::Tokens::FontBold(12))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
								.Justification(ETextJustify::Center)
							]
						]
					]
				]
				// Level element above stats box
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom).Padding(18.f, 0.f, 0.f, 0.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(SBox)
						.WidthOverride(52.f)
						.HeightOverride(52.f)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SAssignNew(LevelRingWidget, ST66RingWidget)
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(34.f)
								.HeightOverride(34.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.08f, 0.85f))
									[
										SAssignNew(LevelText, STextBlock)
										.Text(FText::AsNumber(1))
										.Font(FT66Style::Tokens::FontBold(14))
										.ColorAndOpacity(FT66Style::Tokens::Text)
										.Justification(ETextJustify::Center)
									]
								]
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						StatsPanelWrapped
					]
				]
				// Ultimate to the right of stat box
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom).Padding(16.f, 0.f, 0.f, 0.f)
				[
					SNew(SBox)
					.WidthOverride(84.f)
					.HeightOverride(56.f)
					[
						SAssignNew(UltimateBorder, SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.10f, 0.10f, 0.12f, 0.8f))
						[
							SAssignNew(UltimateText, STextBlock)
							.Text(NSLOCTEXT("T66.GameplayHUD", "Ready", "READY"))
							.Font(FT66Style::Tokens::FontBold(14))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
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
							SNew(SBorder)
							.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
							.Padding(8.f)
							[
								SAssignNew(MinimapWidget, ST66WorldMapWidget)
								.bMinimap(true)
								.bShowLabels(false)
							]
						]
						// Timer centered at the top of the minimap
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Top)
						.Padding(0.f, 6.f)
						[
							SAssignNew(TimerText, STextBlock)
							.Text(NSLOCTEXT("T66.GameplayHUD", "StageTimerDefault", "6:00"))
							.Font(FT66Style::Tokens::FontBold(20))
							.ColorAndOpacity(FT66Style::Tokens::Success)
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
						DifficultyRowRef
					]
				]
			]
		]
		// Inventory panel moved to bottom-right; larger, with Gold/Owe above it
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(24.f, 0.f, 24.f, 24.f)
		[
			SAssignNew(InventoryPanelBox, SBox)
			[
				SNew(SBorder)
				.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
				.Padding(12.f)
				[
					SNew(SVerticalBox)
					// Gold / Owe above items (Gold above Owe), same font size
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SAssignNew(GoldText, STextBlock)
							.Text(GoldInit)
							.Font(FT66Style::Tokens::FontBold(20))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
						[
							SAssignNew(DebtText, STextBlock)
							.Text(OweInit)
							.Font(FT66Style::Tokens::FontBold(20))
							.ColorAndOpacity(FT66Style::Tokens::Danger)
						]
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SBorder)
						.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
						.Padding(10.f)
						[
							InvGridRef
						]
					]
				]
			]
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
					SNew(SBorder)
					.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
					.Padding(16.f)
					[
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
								SNew(SBorder)
								.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
								.Padding(10.f)
								[
									SAssignNew(FullMapWidget, ST66WorldMapWidget)
									.bMinimap(false)
									.bShowLabels(true)
								]
							]
						]
					]
				]
			]
		];

	return Root;
}
