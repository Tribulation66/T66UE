// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66GameplayHUDWidget.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66MediaViewerSubsystem.h"
#include "Core/T66PlayerExperienceSubSystem.h"
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
#include "UI/T66ItemCardTextUtils.h"
#include "UI/Style/T66Style.h"
#include "UI/T66CrateOverlayWidget.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/InputSettings.h"
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
#include "Engine/DataTable.h"
#include "ImageUtils.h"
// [GOLD] EngineUtils.h removed — TActorIterator replaced by ActorRegistry.
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SToolTip.h"

namespace
{
	static FLinearColor WithAlpha(const FLinearColor& Color, float Alpha)
	{
		FLinearColor Out = Color;
		Out.A = Alpha;
		return Out;
	}

	enum class ET66MapMarkerVisual : uint8
	{
		Dot,
		Icon,
	};

	static TObjectPtr<UTexture2D> GMinimapIconAtlas = nullptr;
	static bool bTriedMinimapIconAtlasLoad = false;

	static UTexture2D* GetMinimapIconAtlas()
	{
		if (!bTriedMinimapIconAtlasLoad)
		{
			bTriedMinimapIconAtlasLoad = true;
			const FString AtlasPath = FPaths::ConvertRelativePathToFull(
				FPaths::ProjectDir() / TEXT("SourceAssets/Shikashi's Fantasy Icons Pack v2/Shikashi's Fantasy Icons Pack v2/#1 - Transparent Icons.png"));
			if (FPaths::FileExists(AtlasPath))
			{
			GMinimapIconAtlas = FImageUtils::ImportFileAsTexture2D(AtlasPath);
			if (GMinimapIconAtlas)
			{
				GMinimapIconAtlas->SRGB = true;
				GMinimapIconAtlas->Filter = TextureFilter::TF_Trilinear;
				GMinimapIconAtlas->LODGroup = TextureGroup::TEXTUREGROUP_UI;
				GMinimapIconAtlas->CompressionSettings = TC_EditorIcon;
				GMinimapIconAtlas->NeverStream = true;
				GMinimapIconAtlas->UpdateResource();
				GMinimapIconAtlas->AddToRoot();
			}
			}
		}

		return GMinimapIconAtlas.Get();
	}

	static TSharedPtr<FSlateBrush> MakeAtlasBrushFromPixels(float X, float Y, float W, float H, const FVector2D& DrawSize)
	{
		UTexture2D* Atlas = GetMinimapIconAtlas();
		if (!Atlas || Atlas->GetSizeX() <= 0 || Atlas->GetSizeY() <= 0)
		{
			return nullptr;
		}

		const float AtlasW = static_cast<float>(Atlas->GetSizeX());
		const float AtlasH = static_cast<float>(Atlas->GetSizeY());

		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = DrawSize;
		Brush->TintColor = FSlateColor(FLinearColor::White);
		Brush->SetResourceObject(Atlas);
		Brush->SetUVRegion(FBox2f(
			FVector2f(X / AtlasW, Y / AtlasH),
			FVector2f((X + W) / AtlasW, (Y + H) / AtlasH)));
		return Brush;
	}

	static const FSlateBrush* GetMinimapSymbolBrush(FName Key)
	{
		static bool bInitialized = false;
		static TMap<FName, TSharedPtr<FSlateBrush>> Brushes;

		if (!bInitialized)
		{
			bInitialized = true;
			Brushes.Add(FName(TEXT("NPC")), MakeAtlasBrushFromPixels(95.f, 6.f, 30.f, 24.f, FVector2D(22.f, 18.f)));
			Brushes.Add(FName(TEXT("Gate")), MakeAtlasBrushFromPixels(1.f, 41.f, 31.f, 31.f, FVector2D(20.f, 20.f)));
			Brushes.Add(FName(TEXT("Miasma")), MakeAtlasBrushFromPixels(432.f, 57.f, 31.f, 31.f, FVector2D(20.f, 20.f)));
		}

		if (const TSharedPtr<FSlateBrush>* Found = Brushes.Find(Key))
		{
			return Found->IsValid() ? Found->Get() : nullptr;
		}

		if (const TSharedPtr<FSlateBrush>* Fallback = Brushes.Find(FName(TEXT("NPC"))))
		{
			return Fallback->IsValid() ? Fallback->Get() : nullptr;
		}

		return nullptr;
	}

	static bool HasAnyNamedEntries(const TArray<FName>& IDs)
	{
		for (const FName& ID : IDs)
		{
			if (!ID.IsNone())
			{
				return true;
			}
		}

		return false;
	}

	static ET66ItemRarity GetPreviewRarityByIndex(const int32 Index)
	{
		switch (Index % 4)
		{
		case 1:
			return ET66ItemRarity::Red;
		case 2:
			return ET66ItemRarity::Yellow;
		case 3:
			return ET66ItemRarity::White;
		case 0:
		default:
			return ET66ItemRarity::Black;
		}
	}

	static void BuildPreviewInventoryDisplay(UT66GameInstance* GI, TArray<FName>& OutIDs, TArray<FT66InventorySlot>& OutSlots)
	{
		OutIDs.Init(NAME_None, UT66RunStateSubsystem::MaxInventorySlots);
		OutSlots.SetNum(UT66RunStateSubsystem::MaxInventorySlots);

		if (!GI)
		{
			return;
		}

		TArray<FName> CandidateIDs;
		if (UDataTable* ItemsDT = GI->GetItemsDataTable())
		{
			for (const FName RowName : ItemsDT->GetRowNames())
			{
				FItemData ItemData;
				if (GI->GetItemData(RowName, ItemData))
				{
					CandidateIDs.Add(RowName);
				}
			}
		}

		if (CandidateIDs.Num() == 0)
		{
			CandidateIDs = {
				FName(TEXT("Item_AoeDamage")),
				FName(TEXT("Item_CritDamage")),
				FName(TEXT("Item_LifeSteal")),
				FName(TEXT("Item_MovementSpeed"))
			};
		}

		if (CandidateIDs.Num() == 0)
		{
			return;
		}

		for (int32 SlotIndex = 0; SlotIndex < UT66RunStateSubsystem::MaxInventorySlots; ++SlotIndex)
		{
			const FName ItemID = CandidateIDs[SlotIndex % CandidateIDs.Num()];
			const ET66ItemRarity Rarity = GetPreviewRarityByIndex(SlotIndex);
			OutIDs[SlotIndex] = ItemID;
			OutSlots[SlotIndex] = FT66InventorySlot(ItemID, Rarity, 8 + (SlotIndex * 3));
		}
	}

	static void BuildPreviewIdolDisplay(UT66GameInstance* GI, TArray<FName>& OutIDs, TArray<ET66ItemRarity>& OutRarities)
	{
		OutIDs.Init(NAME_None, UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
		OutRarities.Init(ET66ItemRarity::Black, UT66IdolManagerSubsystem::MaxEquippedIdolSlots);

		if (!GI)
		{
			return;
		}

		const TArray<FName>& AllIdolIDs = UT66IdolManagerSubsystem::GetAllIdolIDs();
		if (AllIdolIDs.Num() == 0)
		{
			return;
		}

		for (int32 SlotIndex = 0; SlotIndex < UT66IdolManagerSubsystem::MaxEquippedIdolSlots; ++SlotIndex)
		{
			const FName IdolID = AllIdolIDs[SlotIndex % AllIdolIDs.Num()];
			FIdolData IdolData;
			if (!GI->GetIdolData(IdolID, IdolData))
			{
				continue;
			}

			OutIDs[SlotIndex] = IdolID;
			OutRarities[SlotIndex] = GetPreviewRarityByIndex(SlotIndex);
		}
	}

	// Media viewer panel: original phone-panel size with tab row above.
	static constexpr float GT66MediaPanelW = 330.f;
	// Video area + tab row height (~30px tabs + 570px video = 600 total, matching old panel).
	static constexpr float GT66MediaVideoH = 570.f;
	static constexpr float GT66MediaTabRowH = 30.f;
	static constexpr float GT66MediaPanelH = GT66MediaVideoH + GT66MediaTabRowH;

	// Distance from viewport bottom to top of hearts row (matches bottom-left HUD: slot padding 24 + portrait 250 + padding 6 + hearts 48).
	static constexpr float GT66HeartsTopOffsetFromBottom = 24.f + 250.f + 6.f + 48.f;

	// DPS meter reuses the same tier ladder as hearts/skulls, with broad thresholds so the color
	// changes are meaningful across early and late runs.
	static int32 GetDPSTierIndex(int32 DPS)
	{
		if (DPS <= 0) return -1;
		if (DPS >= 2000) return 5;
		if (DPS >= 1000) return 4;
		if (DPS >= 500) return 3;
		if (DPS >= 250) return 2;
		if (DPS >= 100) return 1;
		return 0;
	}

	static FLinearColor GetDPSColor(int32 DPS)
	{
		const int32 Tier = GetDPSTierIndex(DPS);
		return Tier >= 0 ? FT66RarityUtil::GetTierColor(Tier) : FT66Style::Tokens::TextMuted;
	}

	static const TCHAR* GetStatGradeLabel(int32 Value)
	{
		const int32 ClampedValue = FMath::Max(0, Value);
		if (ClampedValue <= 3)  return TEXT("F");
		if (ClampedValue <= 7)  return TEXT("D-");
		if (ClampedValue <= 11) return TEXT("D");
		if (ClampedValue <= 15) return TEXT("D+");
		if (ClampedValue <= 19) return TEXT("C-");
		if (ClampedValue <= 23) return TEXT("C");
		if (ClampedValue <= 27) return TEXT("C+");
		if (ClampedValue <= 31) return TEXT("B-");
		if (ClampedValue <= 35) return TEXT("B");
		if (ClampedValue <= 39) return TEXT("B+");
		if (ClampedValue <= 43) return TEXT("A-");
		if (ClampedValue <= 47) return TEXT("A");
		if (ClampedValue <= 51) return TEXT("A+");
		if (ClampedValue <= 55) return TEXT("S");
		if (ClampedValue <= 59) return TEXT("S+");
		if (ClampedValue <= 63) return TEXT("S++");
		return TEXT("S+++");
	}

	static FText MakeGradeStatText(const TCHAR* Label, int32 Value)
	{
		return FText::FromString(FString::Printf(TEXT("%s: %s"), Label, GetStatGradeLabel(Value)));
	}

	static bool IsKeyboardMouseKey(const FKey& Key)
	{
		if (!Key.IsValid())
		{
			return false;
		}

		if (Key.IsMouseButton())
		{
			return true;
		}

		return !Key.IsGamepadKey() && !Key.IsTouch();
	}

	static FText GetActionKeyText(FName ActionName)
	{
		if (const UInputSettings* Settings = UInputSettings::GetInputSettings())
		{
			for (const FInputActionKeyMapping& Mapping : Settings->GetActionMappings())
			{
				if (Mapping.ActionName == ActionName && IsKeyboardMouseKey(Mapping.Key))
				{
					return Mapping.Key.GetDisplayName();
				}
			}

			for (const FInputActionKeyMapping& Mapping : Settings->GetActionMappings())
			{
				if (Mapping.ActionName == ActionName)
				{
					return Mapping.Key.GetDisplayName();
				}
			}
		}

		return FText::GetEmpty();
	}

	static FText BuildSkipCountdownText(float RemainingSeconds, FName ActionName = NAME_None)
	{
		const FText SecondsText = FText::FromString(FString::Printf(TEXT("%.1f"), FMath::Max(0.f, RemainingSeconds)));
		if (!ActionName.IsNone())
		{
			const FText KeyText = GetActionKeyText(ActionName);
			if (!KeyText.IsEmpty())
			{
				return FText::Format(
					NSLOCTEXT("T66.Presentation", "SkipKeyCountdown", "Skip: {0} ({1}s)"),
					KeyText,
					SecondsText);
			}
		}

		return FText::Format(
			NSLOCTEXT("T66.Presentation", "SkipCountdown", "Skip {0}s"),
			SecondsText);
	}

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

	/** Creates a rich tooltip (title + description) for stat/ability hover. */
	static TSharedPtr<IToolTip> CreateRichTooltip(const FText& Title, const FText& Description)
	{
		if (Title.IsEmpty() && Description.IsEmpty()) return nullptr;
		return SNew(SToolTip)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
				.BorderBackgroundColor(FT66Style::Tokens::Bg)
				.Padding(FMargin(10.f, 8.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[
						SNew(STextBlock)
						.Text(Title)
						.Font(FT66Style::Tokens::FontBold(14))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(Description)
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
						.WrapTextAt(280.f)
					]
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
		return FVector2D(52.f, 52.f);
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

class ST66ScopedSniperWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(ST66ScopedSniperWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(1920.f, 1080.f);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const FVector2D Size = AllottedGeometry.GetLocalSize();
		const FVector2D Center(Size.X * 0.5f, Size.Y * 0.5f);
		const float ScopeDiameter = FMath::Min(Size.Y * 0.82f, Size.X * 0.62f);
		const float ScopeRadius = ScopeDiameter * 0.5f;
		const FVector2D ScopeTopLeft = Center - FVector2D(ScopeRadius, ScopeRadius);
		const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

		auto DrawSolidBox = [&](const FVector2D& Pos, const FVector2D& BoxSize, const FLinearColor& Color)
		{
			if (BoxSize.X <= 0.f || BoxSize.Y <= 0.f) return;
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(FVector2f(BoxSize), FSlateLayoutTransform(FVector2f(Pos))),
				WhiteBrush,
				ESlateDrawEffect::None,
				Color);
		};

		// Hard matte around the scope window. This keeps the focused, CS-style tunneled view
		// without requiring a dedicated scope texture asset.
		DrawSolidBox(FVector2D(0.f, 0.f), FVector2D(FMath::Max(0.f, ScopeTopLeft.X), Size.Y), FLinearColor::Black);
		DrawSolidBox(FVector2D(ScopeTopLeft.X + ScopeDiameter, 0.f), FVector2D(FMath::Max(0.f, Size.X - (ScopeTopLeft.X + ScopeDiameter)), Size.Y), FLinearColor::Black);
		DrawSolidBox(FVector2D(ScopeTopLeft.X, 0.f), FVector2D(ScopeDiameter, FMath::Max(0.f, ScopeTopLeft.Y)), FLinearColor::Black);
		DrawSolidBox(FVector2D(ScopeTopLeft.X, ScopeTopLeft.Y + ScopeDiameter), FVector2D(ScopeDiameter, FMath::Max(0.f, Size.Y - (ScopeTopLeft.Y + ScopeDiameter))), FLinearColor::Black);

		auto DrawLine = [&](const FVector2D& A, const FVector2D& B, const FLinearColor& Color, const float Thickness)
		{
			TArray<FVector2D> Points;
			Points.Add(A);
			Points.Add(B);
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				Points,
				ESlateDrawEffect::None,
				Color,
				false,
				Thickness);
		};

		// Scope ring.
		{
			static constexpr int32 NumSeg = 72;
			TArray<FVector2D> RingPoints;
			RingPoints.Reserve(NumSeg + 1);
			for (int32 Index = 0; Index <= NumSeg; ++Index)
			{
				const float T = static_cast<float>(Index) / static_cast<float>(NumSeg);
				const float Angle = 2.f * PI * T;
				RingPoints.Add(Center + FVector2D(FMath::Cos(Angle) * ScopeRadius, FMath::Sin(Angle) * ScopeRadius));
			}

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				RingPoints,
				ESlateDrawEffect::None,
				FLinearColor(0.95f, 0.95f, 1.f, 0.95f),
				true,
				2.f);
		}

		// Thin reticle lines like a CS sniper scope.
		const float LineExtent = ScopeRadius - 18.f;
		DrawLine(Center + FVector2D(-LineExtent, 0.f), Center + FVector2D(LineExtent, 0.f), FLinearColor(0.95f, 0.95f, 1.f, 0.95f), 1.5f);
		DrawLine(Center + FVector2D(0.f, -LineExtent), Center + FVector2D(0.f, LineExtent), FLinearColor(0.95f, 0.95f, 1.f, 0.95f), 1.5f);

		return LayerId + 2;
	}
};

struct FT66MapMarker
{
	FVector2D WorldXY = FVector2D::ZeroVector;
	FLinearColor Color = FLinearColor::White;
	FText Label = FText::GetEmpty();
	ET66MapMarkerVisual Visual = ET66MapMarkerVisual::Dot;
	const FSlateBrush* IconBrush = nullptr;
	FVector2D DrawSize = FVector2D(10.f, 10.f);
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

	void SetSnapshot(const FVector2D& InPlayerWorldXY, const TArray<FT66MapMarker>& InMarkers, const FSlateBrush* InPlayerBrush = nullptr)
	{
		PlayerWorldXY = InPlayerWorldXY;
		Markers = InMarkers;
		PlayerBrush = InPlayerBrush;
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
		return bMinimap ? FVector2D(228.f, 228.f) : FVector2D(1024.f, 640.f);
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

		const FLinearColor MapBackgroundColor = bMinimap ? FT66Style::MinimapBackground() : FT66Style::Background();
		const FLinearColor MapTerrainColor = WithAlpha(FT66Style::MinimapTerrain(), bMinimap ? 0.42f : 0.22f);
		const FLinearColor GridColor = FT66Style::MinimapGrid();
		const FLinearColor OutlineColor = WithAlpha(FT66Style::Border(), 0.88f);

		// Solid dark background fill.
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			FCoreStyle::Get().GetBrush("WhiteBrush"),
			ESlateDrawEffect::None,
			MapBackgroundColor
		);

		// Terrain tint and diagonal lane strokes so the minimap reads more like a finished surface than a debug grid.
		{
			const FVector2D Inset(6.f, 6.f);
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId + 1,
				ToPaintGeo(Inset, Size - (Inset * 2.f)),
				FCoreStyle::Get().GetBrush("WhiteBrush"),
				ESlateDrawEffect::None,
				MapTerrainColor
			);

			const FLinearColor LaneColor = WithAlpha(FT66Style::MinimapTerrain() * FLinearColor(0.7f, 0.7f, 0.7f, 1.f), 0.75f);
			const TArray<FVector2D> LaneA = {
				FVector2D(Size.X * 0.08f, Size.Y * 0.78f),
				FVector2D(Size.X * 0.24f, Size.Y * 0.62f),
				FVector2D(Size.X * 0.40f, Size.Y * 0.46f),
				FVector2D(Size.X * 0.58f, Size.Y * 0.30f),
				FVector2D(Size.X * 0.84f, Size.Y * 0.12f)
			};
			const TArray<FVector2D> LaneB = {
				FVector2D(Size.X * 0.16f, Size.Y * 0.16f),
				FVector2D(Size.X * 0.34f, Size.Y * 0.30f),
				FVector2D(Size.X * 0.52f, Size.Y * 0.48f),
				FVector2D(Size.X * 0.72f, Size.Y * 0.68f),
				FVector2D(Size.X * 0.90f, Size.Y * 0.84f)
			};
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(), LaneA, ESlateDrawEffect::None, LaneColor, true, bMinimap ? 10.f : 16.f);
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(), LaneB, ESlateDrawEffect::None, WithAlpha(LaneColor, 0.50f), true, bMinimap ? 8.f : 12.f);
		}

		// Background grid (subtle).
		{
			static constexpr int32 GridLines = 6;
			for (int32 i = 1; i < GridLines; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(GridLines);
				const float X = Size.X * T;
				const float Y = Size.Y * T;
				const TArray<FVector2D> V = { FVector2D(X, 0.f), FVector2D(X, Size.Y) };
				const TArray<FVector2D> H = { FVector2D(0.f, Y), FVector2D(Size.X, Y) };
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 3, AllottedGeometry.ToPaintGeometry(), V, ESlateDrawEffect::None, GridColor, true, 1.f);
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 3, AllottedGeometry.ToPaintGeometry(), H, ESlateDrawEffect::None, GridColor, true, 1.f);
			}
		}

		// Border outline.
		{
			const TArray<FVector2D> Border = {
				FVector2D(0.f, 0.f), FVector2D(Size.X, 0.f),
				FVector2D(Size.X, Size.Y), FVector2D(0.f, Size.Y),
				FVector2D(0.f, 0.f) };
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 4, AllottedGeometry.ToPaintGeometry(),
				Border, ESlateDrawEffect::None, OutlineColor, true, 2.f);
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
				{ NSLOCTEXT("T66.Map", "AreaStart", "START"), FVector2D(-10000.f, 0.f), FVector2D(3000.f, 3000.f), WithAlpha(FT66Style::MinimapFriendly(), 0.10f), WithAlpha(FT66Style::MinimapFriendly(), 0.28f) },
				{ NSLOCTEXT("T66.Map", "AreaMain",  "MAIN"),  FVector2D(0.f, 0.f),      FVector2D(10000.f, 10000.f), WithAlpha(FT66Style::MinimapNeutral(), 0.06f), WithAlpha(FT66Style::MinimapNeutral(), 0.18f) },
				{ NSLOCTEXT("T66.Map", "AreaBoss",  "BOSS"),  FVector2D(10000.f, 0.f),  FVector2D(3000.f, 3000.f), WithAlpha(FT66Style::MinimapEnemy(), 0.08f), WithAlpha(FT66Style::MinimapEnemy(), 0.25f) },
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
					WithAlpha(FT66Style::TextMuted(), 0.80f)
				);
			}
		}

		auto IsInsideBounds = [&](const FVector2D& P) -> bool
		{
			return P.X >= 0.f && P.X <= Size.X && P.Y >= 0.f && P.Y <= Size.Y;
		};

		auto ClampToBounds = [&](const FVector2D& P) -> FVector2D
		{
			return FVector2D(
				FMath::Clamp(P.X, 0.f, Size.X),
				FMath::Clamp(P.Y, 0.f, Size.Y));
		};

		// Player marker (portrait when available, else the old green diamond fallback).
		{
			const FVector2D P = ClampToBounds(WorldToLocal(PlayerWorldXY));
			if (PlayerBrush && PlayerBrush->GetResourceObject())
			{
				const float MarkerSize = bMinimap ? 18.f : 24.f;
				const FVector2D TL = ClampToBounds(P - FVector2D(MarkerSize * 0.5f, MarkerSize * 0.5f));
				const FVector2D InnerTL = TL + FVector2D(2.f, 2.f);
				const FVector2D InnerSize = FVector2D(MarkerSize - 4.f, MarkerSize - 4.f);

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 5,
					ToPaintGeo(TL - FVector2D(1.f, 1.f), FVector2D(MarkerSize + 2.f, MarkerSize + 2.f)),
					FCoreStyle::Get().GetBrush("WhiteBrush"),
					ESlateDrawEffect::None,
					FT66Style::PanelOuter());

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 6,
					ToPaintGeo(TL, FVector2D(MarkerSize, MarkerSize)),
					FCoreStyle::Get().GetBrush("WhiteBrush"),
					ESlateDrawEffect::None,
					FT66Style::MinimapFriendly());

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 7,
					ToPaintGeo(InnerTL, InnerSize),
					PlayerBrush,
					ESlateDrawEffect::None,
					FLinearColor::White);
			}
			else
			{
				const float R = bMinimap ? 5.f : 6.f;
				const TArray<FVector2D> Diamond = {
					FVector2D(P.X, P.Y - R),
					FVector2D(P.X + R, P.Y),
					FVector2D(P.X, P.Y + R),
					FVector2D(P.X - R, P.Y),
					FVector2D(P.X, P.Y - R)
				};
				FSlateDrawElement::MakeLines(
					OutDrawElements,
					LayerId + 5,
					AllottedGeometry.ToPaintGeometry(),
					Diamond,
					ESlateDrawEffect::None,
					FT66Style::MinimapFriendly(),
					true,
					2.5f
				);
			}
		}

		// Entity markers (NPCs, gates, enemies, etc.).
		for (const FT66MapMarker& M : Markers)
		{
			const FVector2D RawP = WorldToLocal(M.WorldXY);
			if (bMinimap && !IsInsideBounds(RawP))
			{
				continue;
			}

			const FVector2D P = ClampToBounds(RawP);

			if (M.Visual == ET66MapMarkerVisual::Icon && M.IconBrush && M.IconBrush->GetResourceObject())
			{
				const FVector2D IconSize = bMinimap ? M.DrawSize : (M.DrawSize + FVector2D(4.f, 4.f));
				const FVector2D TL = P - (IconSize * 0.5f);

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 8,
					ToPaintGeo(TL - FVector2D(1.f, 1.f), IconSize + FVector2D(2.f, 2.f)),
					FCoreStyle::Get().GetBrush("WhiteBrush"),
					ESlateDrawEffect::None,
					FT66Style::PanelOuter());

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 9,
					ToPaintGeo(TL, IconSize),
					M.IconBrush,
					ESlateDrawEffect::None,
					FLinearColor(M.Color.R, M.Color.G, M.Color.B, bMinimap ? 0.95f : 0.98f));
			}
			else
			{
				const float R = bMinimap ? 3.0f : 4.0f;
				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 8,
					ToPaintGeo(P - FVector2D(R + 1.f, R + 1.f), FVector2D((R + 1.f) * 2.f, (R + 1.f) * 2.f)),
					FCoreStyle::Get().GetBrush("WhiteBrush"),
					ESlateDrawEffect::None,
					FT66Style::PanelOuter()
				);

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 9,
					ToPaintGeo(P - FVector2D(R, R), FVector2D(R * 2.f, R * 2.f)),
					FCoreStyle::Get().GetBrush("WhiteBrush"),
					ESlateDrawEffect::None,
					FLinearColor(M.Color.R, M.Color.G, M.Color.B, bMinimap ? 0.96f : 0.98f)
				);
			}

			if (bShowLabels && !M.Label.IsEmpty())
			{
				FSlateDrawElement::MakeText(
					OutDrawElements,
					LayerId + 10,
					ToPaintGeo(ClampToBounds(P + FVector2D(6.f, -10.f)), FVector2D(260.f, 20.f)),
					M.Label,
					FT66Style::Tokens::FontBold(bMinimap ? 10 : 12),
					ESlateDrawEffect::None,
					WithAlpha(FT66Style::Text(), bMinimap ? 0.70f : 0.92f)
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
	const FSlateBrush* PlayerBrush = nullptr;

	float MinimapHalfExtent = 2500.f;
};

TSharedRef<SWidget> UT66GameplayHUDWidget::RebuildWidget()
{
	return FT66Style::MakeResponsiveRoot(BuildSlateUI());
}

UT66RunStateSubsystem* UT66GameplayHUDWidget::GetRunState() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
}

UT66DamageLogSubsystem* UT66GameplayHUDWidget::GetDamageLog() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66DamageLogSubsystem>() : nullptr;
}

void UT66GameplayHUDWidget::MarkHUDDirty()
{
	bHUDDirty = true;
}

void UT66GameplayHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bHUDDirty)
	{
		RefreshHUD();
	}

	RefreshCooldownBar();
	RefreshSpeedRunTimers();
	DPSRefreshAccumSeconds += InDeltaTime;
	if (DPSRefreshAccumSeconds >= DPSRefreshIntervalSeconds)
	{
		DPSRefreshAccumSeconds = 0.f;
		RefreshDPS();
	}

	if (bPickupCardVisible && PickupCardBox.IsValid())
	{
		PickupCardRemainingSeconds = FMath::Max(0.f, PickupCardRemainingSeconds - InDeltaTime);
		if (PickupCardSkipText.IsValid())
		{
			PickupCardSkipText->SetText(BuildSkipCountdownText(PickupCardRemainingSeconds, FName(TEXT("Interact"))));
		}

		const float FadeAlpha = (PickupCardRemainingSeconds > PickupCardFadeOutSeconds)
			? 1.f
			: FMath::Clamp(PickupCardRemainingSeconds / PickupCardFadeOutSeconds, 0.f, 1.f);
		PickupCardBox->SetRenderOpacity(FadeAlpha);

		if (PickupCardRemainingSeconds <= 0.f)
		{
			HidePickupCard();
		}
	}

	if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		const bool bScoped = T66PC->IsHeroOneScopeViewEnabled();
		if (CenterCrosshairBox.IsValid())
		{
			CenterCrosshairBox->SetVisibility(bScoped ? EVisibility::Collapsed : EVisibility::HitTestInvisible);
		}
		if (ScopedSniperOverlayBorder.IsValid())
		{
			ScopedSniperOverlayBorder->SetVisibility(bScoped ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
		}
		if (bScoped)
		{
			if (ScopedUltTimerText.IsValid())
			{
				ScopedUltTimerText->SetText(FText::FromString(FString::Printf(TEXT("ULT %.1fs"), T66PC->GetHeroOneScopedUltRemainingSeconds())));
			}
			if (ScopedShotCooldownText.IsValid())
			{
				const float ShotCooldown = T66PC->GetHeroOneScopedShotCooldownRemainingSeconds();
				ScopedShotCooldownText->SetText(
					ShotCooldown > 0.f
						? FText::FromString(FString::Printf(TEXT("SHOT %.2fs"), ShotCooldown))
						: NSLOCTEXT("T66.HUD", "ScopedShotReady", "SHOT READY"));
			}
		}
	}
	else
	{
		if (CenterCrosshairBox.IsValid())
		{
			CenterCrosshairBox->SetVisibility(EVisibility::HitTestInvisible);
		}
		if (ScopedSniperOverlayBorder.IsValid())
		{
			ScopedSniperOverlayBorder->SetVisibility(EVisibility::Collapsed);
		}
	}
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
		if (LastDisplayedCooldownBarWidthPx != 0)
		{
			LastDisplayedCooldownBarWidthPx = 0;
			CooldownBarFillBox->SetWidthOverride(0.f);
		}
		if (CooldownTimeText.IsValid() && LastDisplayedCooldownRemainingCs != -1)
		{
			LastDisplayedCooldownRemainingCs = -1;
			CooldownTimeText->SetText(FText::GetEmpty());
		}
		return;
	}
	float Pct = Hero->CombatComponent->GetAutoAttackCooldownProgress();
	Pct = FMath::Clamp(Pct, 0.f, 1.f);
	const int32 WidthPx = FMath::RoundToInt(CooldownBarWidth * Pct);
	if (WidthPx != LastDisplayedCooldownBarWidthPx)
	{
		LastDisplayedCooldownBarWidthPx = WidthPx;
		CooldownBarFillBox->SetWidthOverride(static_cast<float>(WidthPx));
	}

	// Show remaining time above the bar (e.g. "0.42s").
	// Perf: only reformat the text when the displayed centisecond value changes.
	if (CooldownTimeText.IsValid())
	{
		const float Interval = Hero->CombatComponent->GetEffectiveFireInterval();
		const float Remaining = FMath::Max(0.f, (1.f - Pct) * Interval);
		const int32 RemainingCs = FMath::RoundToInt(Remaining * 100.f); // centiseconds
		if (RemainingCs != LastDisplayedCooldownRemainingCs)
		{
			LastDisplayedCooldownRemainingCs = RemainingCs;
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
}

void UT66GameplayHUDWidget::RefreshHeroStats()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	if (StatDamageText.IsValid())
		StatDamageText->SetText(MakeGradeStatText(TEXT("Dmg"), RunState->GetDamageStat()));
	if (StatAttackSpeedText.IsValid())
		StatAttackSpeedText->SetText(MakeGradeStatText(TEXT("AS"), RunState->GetAttackSpeedStat()));
	if (StatAttackScaleText.IsValid())
		StatAttackScaleText->SetText(MakeGradeStatText(TEXT("Scale"), RunState->GetScaleStat()));
	if (StatArmorText.IsValid())
		StatArmorText->SetText(MakeGradeStatText(TEXT("Armor"), RunState->GetArmorStat()));
	if (StatEvasionText.IsValid())
		StatEvasionText->SetText(MakeGradeStatText(TEXT("Eva"), RunState->GetEvasionStat()));
	if (StatLuckText.IsValid())
		StatLuckText->SetText(MakeGradeStatText(TEXT("Luck"), RunState->GetLuckStat()));
	if (StatSpeedText.IsValid())
		StatSpeedText->SetText(MakeGradeStatText(TEXT("Speed"), FMath::RoundToInt(RunState->GetHeroMoveSpeedMultiplier() * 10.f)));
}

void UT66GameplayHUDWidget::RefreshDPS()
{
	if (!DPSText.IsValid()) return;

	UT66DamageLogSubsystem* DamageLog = GetDamageLog();
	const int32 DisplayDPS = DamageLog ? FMath::RoundToInt(FMath::Max(0.f, DamageLog->GetRollingDPS())) : 0;
	const FLinearColor DPSColor = GetDPSColor(DisplayDPS);
	if (DisplayDPS != LastDisplayedDPS || !LastDisplayedDPSColor.Equals(DPSColor))
	{
		LastDisplayedDPS = DisplayDPS;
		LastDisplayedDPSColor = DPSColor;
		DPSText->SetText(FText::Format(
			NSLOCTEXT("T66.GameplayHUD", "DPSFormat", "DPS {0}"),
			FText::AsNumber(DisplayDPS)));
		DPSText->SetColorAndOpacity(FSlateColor(DPSColor));
	}
}

void UT66GameplayHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	RunState->HeartsChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHearts);
	RunState->GoldChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
	RunState->DebtChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
	RunState->InventoryChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
	RunState->InventoryChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->PanelVisibilityChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->ScoreChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
	RunState->StageChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->StageTimerChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshStageAndTimer);
	RunState->SpeedRunTimerChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshSpeedRunTimers);
	RunState->BossChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshBossBar);
	RunState->DifficultyChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->CowardiceGatesTakenChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66IdolManagerSubsystem* IdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>())
		{
			IdolManager->IdolStateChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		}
	}
	RunState->HeroProgressChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->UltimateChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->SurvivalChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->QuickReviveChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->StatusEffectsChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshStatusEffects);
	RunState->TutorialHintChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshTutorialHint);
	RunState->DevCheatsChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
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

	// Passive and Ultimate ability tooltips (hero-specific)
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	FHeroData HD;
	if (T66GI && T66GI->GetSelectedHeroData(HD) && Loc)
	{
		if (PassiveBorder.IsValid())
		{
			PassiveBorder->SetToolTip(CreateRichTooltip(Loc->GetText_PassiveName(HD.PassiveType), Loc->GetText_PassiveDescription(HD.PassiveType)));
		}
		if (UltimateBorder.IsValid())
		{
			UltimateBorder->SetToolTip(CreateRichTooltip(Loc->GetText_UltimateName(HD.UltimateType), Loc->GetText_UltimateDescription(HD.UltimateType)));
		}
	}

	MarkHUDDirty();
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

	HidePickupCard();
	if (UT66CrateOverlayWidget* Overlay = ActiveCrateOverlay.Get())
	{
		Overlay->RemoveFromParent();
		ActiveCrateOverlay.Reset();
	}

	UT66RunStateSubsystem* RunState = GetRunState();
	if (RunState)
	{
		RunState->HeartsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHearts);
		RunState->GoldChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
		RunState->DebtChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
		RunState->InventoryChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
		RunState->InventoryChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->PanelVisibilityChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->ScoreChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
		RunState->StageChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->StageTimerChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshStageAndTimer);
		RunState->SpeedRunTimerChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshSpeedRunTimers);
		RunState->BossChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshBossBar);
		RunState->DifficultyChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->CowardiceGatesTakenChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66IdolManagerSubsystem* IdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>())
			{
				IdolManager->IdolStateChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
			}
		}
		RunState->HeroProgressChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->UltimateChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->SurvivalChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->QuickReviveChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->TutorialHintChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshTutorialHint);
		RunState->DevCheatsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->StatusEffectsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshStatusEffects);
	}
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
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

			if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
			{
				if (UT66PlayerExperienceSubSystem* PlayerExperience = GI->GetSubsystem<UT66PlayerExperienceSubSystem>())
				{
					const FT66FloatRange Range = PlayerExperience->GetDifficultyWheelGoldRange(T66GI->SelectedDifficulty, WheelRarity);

					MinGold = FMath::FloorToInt(FMath::Min(Range.Min, Range.Max));
					MaxGold = FMath::CeilToInt(FMath::Max(Range.Min, Range.Max));
					bHasGoldRange = true;

					FRandomStream& Stream = RngSub->GetRunStream();
					PendingGold = FMath::Max(0, FMath::RoundToInt(RngSub->RollFloatRangeBiased(Range, Stream)));
				}
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

void UT66GameplayHUDWidget::StartCrateOpen()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	HidePickupCard();
	if (ActiveCrateOverlay.IsValid())
	{
		return;
	}

	UT66CrateOverlayWidget* Overlay = CreateWidget<UT66CrateOverlayWidget>(PC, UT66CrateOverlayWidget::StaticClass());
	if (Overlay)
	{
		Overlay->SetPresentationHost(this);
		Overlay->SetVisibility(ESlateVisibility::HitTestInvisible);
		Overlay->AddToViewport(100);
		ActiveCrateOverlay = Overlay;
	}
}

bool UT66GameplayHUDWidget::TrySkipActivePresentation()
{
	if (UT66CrateOverlayWidget* Overlay = ActiveCrateOverlay.Get())
	{
		Overlay->RequestSkip();
		return true;
	}

	if (bPickupCardVisible)
	{
		HidePickupCard();
		return true;
	}

	return false;
}

void UT66GameplayHUDWidget::ClearActiveCratePresentation(UT66CrateOverlayWidget* Overlay)
{
	if (!Overlay || ActiveCrateOverlay.Get() == Overlay)
	{
		ActiveCrateOverlay.Reset();
	}
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
	if (!FPSText.IsValid() && !ElevationText.IsValid()) return;
	UWorld* World = GetWorld();
	const float Delta = World ? World->GetDeltaSeconds() : 0.f;
	const int32 FPS = (Delta > 0.f) ? FMath::RoundToInt(1.f / Delta) : 0;
	if (FPSText.IsValid())
	{
		FPSText->SetText(FText::FromString(FString::Printf(TEXT("FPS: %d"), FPS)));
	}

	if (ElevationText.IsValid())
	{
		const APawn* Pawn = GetOwningPlayerPawn();
		if (!Pawn)
		{
			if (const APlayerController* PC = GetOwningPlayer())
			{
				Pawn = PC->GetPawn();
			}
		}

		float GroundElevation = Pawn ? Pawn->GetActorLocation().Z : 0.f;
		if (const AT66HeroBase* Hero = Cast<AT66HeroBase>(Pawn))
		{
			if (const UCapsuleComponent* Capsule = Hero->GetCapsuleComponent())
			{
				GroundElevation -= Capsule->GetScaledCapsuleHalfHeight();
			}
		}

		ElevationText->SetText(FText::FromString(FString::Printf(TEXT("ELV: %d"), FMath::RoundToInt(GroundElevation))));
	}
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

	const float Now = static_cast<float>(World->GetTimeSeconds());
	const bool bWorldChanged = (MapCacheWorld.Get() != World);
	const bool bNeedsFullRefresh = bWorldChanged || (MapCacheLastRefreshTime < 0.f) || ((Now - MapCacheLastRefreshTime) >= MapCacheRefreshIntervalSeconds);

	TArray<FT66MapMarker> Markers;
	Markers.Reserve(64);

	if (bNeedsFullRefresh)
	{
		MapCacheWorld = World;
		MapCacheLastRefreshTime = Now;
		MapCache.Reset();

		// [GOLD] Use the actor registry instead of 4x TActorIterator (O(1) list lookup).
		UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>();
		if (Registry)
		{
			// NPCs use icon markers on the minimap.
			for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : Registry->GetNPCs())
			{
				AT66HouseNPCBase* NPC = WeakNPC.Get();
				if (!NPC) continue;
				MapCache.Add({ NPC, EMapCacheMarkerType::NPC, NPC->NPCColor, NPC->NPCName, NPC->NPCID });
			}
			// Stage gate / interactable.
			for (const TWeakObjectPtr<AT66StageGate>& WeakGate : Registry->GetStageGates())
			{
				AT66StageGate* A = WeakGate.Get();
				if (!A) continue;
				MapCache.Add({ A, EMapCacheMarkerType::Gate, FT66Style::Accent2(), NSLOCTEXT("T66.Map", "Gate", "GATE"), FName(TEXT("Gate")) });
			}
			// Enemies stay as close-range red pips.
			for (const TWeakObjectPtr<AT66EnemyBase>& WeakEnemy : Registry->GetEnemies())
			{
				AT66EnemyBase* Enemy = WeakEnemy.Get();
				if (!IsValid(Enemy)) continue;
				MapCache.Add({ Enemy, EMapCacheMarkerType::Enemy, FT66Style::MinimapEnemy(), FText::GetEmpty(), NAME_None });
			}
			// Miasma boundary uses a dedicated icon marker on the full map.
			for (const TWeakObjectPtr<AT66MiasmaBoundary>& WeakMiasma : Registry->GetMiasmaBoundaries())
			{
				AT66MiasmaBoundary* A = WeakMiasma.Get();
				if (!A) continue;
				MapCache.Add({ A, EMapCacheMarkerType::Miasma, FLinearColor(0.65f, 0.15f, 0.85f, 0.78f), FText::GetEmpty(), FName(TEXT("Miasma")) });
			}

			UE_LOG(LogTemp, Verbose, TEXT("[GOLD] RefreshMapData: used ActorRegistry (NPCs=%d, Gates=%d, Enemies=%d, Miasma=%d)"),
				Registry->GetNPCs().Num(), Registry->GetStageGates().Num(),
				Registry->GetEnemies().Num(), Registry->GetMiasmaBoundaries().Num());
		}
	}

	// Build markers from cache (positions only; cache has static Color/Label).
	for (const FMapCacheEntry& E : MapCache)
	{
		AActor* A = E.Actor.Get();
		if (!IsValid(A)) continue;
		FT66MapMarker M;
		const FVector L = A->GetActorLocation();
		M.WorldXY = FVector2D(L.X, L.Y);
		M.Color = E.Color;
		M.Label = E.Label;

		switch (E.Type)
		{
		case EMapCacheMarkerType::NPC:
			M.Visual = ET66MapMarkerVisual::Icon;
			M.IconBrush = GetMinimapSymbolBrush(FName(TEXT("NPC")));
			M.DrawSize = FVector2D(12.f, 12.f);
			break;
		case EMapCacheMarkerType::Gate:
			M.Visual = ET66MapMarkerVisual::Icon;
			M.IconBrush = GetMinimapSymbolBrush(FName(TEXT("Gate")));
			M.DrawSize = FVector2D(12.f, 12.f);
			break;
		case EMapCacheMarkerType::Miasma:
			M.Visual = ET66MapMarkerVisual::Icon;
			M.IconBrush = GetMinimapSymbolBrush(FName(TEXT("Miasma")));
			M.DrawSize = FVector2D(12.f, 12.f);
			break;
		case EMapCacheMarkerType::Enemy:
		default:
			M.Visual = ET66MapMarkerVisual::Dot;
			M.DrawSize = FVector2D(6.f, 6.f);
			break;
		}

		Markers.Add(M);
	}

	const FSlateBrush* PlayerMarkerBrush = (PortraitBrush.IsValid() && PortraitBrush->GetResourceObject())
		? PortraitBrush.Get()
		: nullptr;

	if (MinimapWidget.IsValid())
	{
		MinimapWidget->SetSnapshot(PlayerXY, Markers, PlayerMarkerBrush);
	}
	if (bFullMapOpen && FullMapWidget.IsValid())
	{
		FullMapWidget->SetSnapshot(PlayerXY, Markers, PlayerMarkerBrush);
	}
}

void UT66GameplayHUDWidget::RefreshEconomy()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	// Net Worth
	if (NetWorthText.IsValid())
	{
		const int32 NetWorth = RunState->GetNetWorth();
		const FText Fmt = Loc ? Loc->GetText_NetWorthFormat() : NSLOCTEXT("T66.GameplayHUD", "NetWorthFormat", "Net Worth: {0}");
		NetWorthText->SetText(FText::Format(Fmt, FText::AsNumber(NetWorth)));

		const FLinearColor NetWorthColor = NetWorth > 0
			? FT66Style::Tokens::Success
			: (NetWorth < 0 ? FT66Style::Tokens::Danger : FT66Style::Tokens::Text);
		NetWorthText->SetColorAndOpacity(FSlateColor(NetWorthColor));
	}

	// Gold
	if (GoldText.IsValid())
	{
		const FText Fmt = Loc ? Loc->GetText_GoldFormat() : NSLOCTEXT("T66.GameplayHUD", "GoldFormat", "Gold: {0}");
		GoldText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentGold())));
	}

	// Owe (Debt) in red
	if (DebtText.IsValid())
	{
		const FText Fmt = Loc ? Loc->GetText_OweFormat() : NSLOCTEXT("T66.GameplayHUD", "OweFormat", "Debt: {0}");
		DebtText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentDebt())));
	}

	// Score
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
		if (RunState->IsInStageCatchUp())
		{
			StageText->SetText(NSLOCTEXT("T66.GameplayHUD", "StageCatchUp", "Stage: Catch Up"));
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
		const EVisibility DesiredVisibility = bShow ? EVisibility::Visible : EVisibility::Collapsed;
		if (SpeedRunText->GetVisibility() != DesiredVisibility)
		{
			SpeedRunText->SetVisibility(DesiredVisibility);
		}
		if (bShow)
		{
			const float Secs = FMath::Max(0.f, RunState->GetSpeedRunElapsedSeconds());
			// [GOLD] HUD text cache: only reformat when the displayed centisecond value changes (avoids FText::Format every frame).
			const int32 TotalCs = FMath::FloorToInt(Secs * 100.f);
			if (TotalCs != LastDisplayedSpeedRunTotalCs)
			{
				LastDisplayedSpeedRunTotalCs = TotalCs;
				const int32 M = FMath::FloorToInt(Secs / 60.f);
				const int32 S = FMath::FloorToInt(FMath::Fmod(Secs, 60.f));
				const int32 Cs = FMath::FloorToInt(FMath::Fmod(Secs * 100.f, 100.f));
				FNumberFormattingOptions TwoDigits;
				TwoDigits.MinimumIntegralDigits = 2;
				SpeedRunText->SetText(FText::Format(
					NSLOCTEXT("T66.GameplayHUD", "SpeedRunTimerFormat", "Time {0}:{1}.{2}"),
					FText::AsNumber(M),
					FText::AsNumber(S, &TwoDigits),
					FText::AsNumber(Cs, &TwoDigits)));
			}
		}
		else
		{
			LastDisplayedSpeedRunTotalCs = -1;
		}
	}

	// Speedrun target time (10th place) for the current stage.
	if (SpeedRunTargetText.IsValid())
	{
		const bool bShow = PS ? PS->GetSpeedRunMode() : false;
		if (!bShow || !LB || !GIAsT66 || RunState->IsInStageCatchUp())
		{
			SpeedRunTargetText->SetVisibility(EVisibility::Collapsed);
		}
		else
		{
			float TargetSeconds = 0.f;
			const int32 Stage = RunState->GetCurrentStage();
			const bool bHasTarget = (Stage >= 1 && Stage <= 5) && LB->GetSpeedRunTarget10Seconds(GIAsT66->SelectedDifficulty, GIAsT66->SelectedPartySize, Stage, TargetSeconds);
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
	// No longer show accept/reject prompt; item is added immediately on interact, then item card popup is shown.
	if (LootPromptBox.IsValid())
	{
		LootPromptBox->SetVisibility(EVisibility::Collapsed);
	}
}

void UT66GameplayHUDWidget::ShowPickupItemCard(FName ItemID, ET66ItemRarity ItemRarity)
{
	if (ItemID.IsNone() || !PickupCardBox.IsValid()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66RunStateSubsystem* RunState = GetRunState();

	FItemData D;
	const bool bHasData = GI && GI->GetItemData(ItemID, D);

	if (PickupCardNameText.IsValid())
	{
		PickupCardNameText->SetText(Loc ? Loc->GetText_ItemDisplayName(ItemID) : FText::FromName(ItemID));
	}
	if (PickupCardDescText.IsValid())
	{
		if (!bHasData)
		{
			PickupCardDescText->SetText(FText::GetEmpty());
		}
		else
		{
			int32 MainValue = 0;
			if (RunState)
			{
				const TArray<FT66InventorySlot>& Slots = RunState->GetInventorySlots();
				if (Slots.Num() > 0 && Slots.Last().ItemTemplateID == ItemID)
				{
					MainValue = Slots.Last().Line1RolledValue;
				}
				if (ItemID == FName(TEXT("Item_GamblersToken")))
				{
					MainValue = RunState->GetActiveGamblersTokenLevel();
				}
			}
			const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f;
			PickupCardDescText->SetText(T66ItemCardTextUtils::BuildItemCardDescription(Loc, D, ItemRarity, MainValue, ScaleMult));
		}
	}
	if (!PickupCardIconBrush.IsValid())
	{
		PickupCardIconBrush = MakeShared<FSlateBrush>();
		PickupCardIconBrush->DrawAs = ESlateBrushDrawType::Image;
		PickupCardIconBrush->ImageSize = FVector2D(PickupCardWidth, PickupCardWidth);
	}
	const TSoftObjectPtr<UTexture2D> PickupIconSoft = bHasData ? D.GetIconForRarity(ItemRarity) : TSoftObjectPtr<UTexture2D>();
	if (!PickupIconSoft.IsNull())
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			T66SlateTexture::BindSharedBrushAsync(TexPool, PickupIconSoft, this, PickupCardIconBrush, FName(TEXT("HUDPickupCard")), true);
		}
	}
	if (PickupCardIconImage.IsValid())
	{
		PickupCardIconImage->SetImage(PickupCardIconBrush.Get());
		PickupCardIconImage->SetVisibility(!PickupIconSoft.IsNull() ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (PickupCardTileBorder.IsValid())
	{
		PickupCardTileBorder->SetBorderBackgroundColor(bHasData ? FItemData::GetItemRarityColor(ItemRarity) : FT66Style::Tokens::Panel);
	}
	if (PickupCardIconBorder.IsValid())
	{
		PickupCardIconBorder->SetBorderBackgroundColor(bHasData ? FItemData::GetItemRarityColor(ItemRarity) : FT66Style::Tokens::Panel2);
	}
	if (PickupCardSkipText.IsValid())
	{
		PickupCardSkipText->SetText(BuildSkipCountdownText(PickupCardDisplaySeconds, FName(TEXT("Interact"))));
	}

	bPickupCardVisible = true;
	PickupCardRemainingSeconds = PickupCardDisplaySeconds;
	PickupCardBox->SetVisibility(EVisibility::Visible);
	PickupCardBox->SetRenderOpacity(1.f);
}

void UT66GameplayHUDWidget::HidePickupCard()
{
	bPickupCardVisible = false;
	PickupCardRemainingSeconds = 0.f;
	if (PickupCardBox.IsValid())
	{
		PickupCardBox->SetVisibility(EVisibility::Collapsed);
		PickupCardBox->SetRenderOpacity(1.f);
	}
	if (PickupCardSkipText.IsValid())
	{
		PickupCardSkipText->SetText(FText::GetEmpty());
	}
}

void UT66GameplayHUDWidget::RefreshHearts()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	// Hearts: 5 slots driven by numerical HP; tier from MaxHP (red/blue/green/purple/gold).
	const int32 Tier = RunState->GetHeartDisplayTier();
	const FLinearColor TierC = FT66RarityUtil::GetTierColor(Tier);
	const FLinearColor EmptyC(0.25f, 0.25f, 0.28f, 0.35f);
	for (int32 i = 0; i < HeartImages.Num(); ++i)
	{
		if (!HeartImages[i].IsValid()) continue;
		const float Fill = RunState->GetHeartSlotFill(i);
		const bool bFilled = (Fill > 0.01f);
		HeartImages[i]->SetColorAndOpacity(bFilled ? TierC : EmptyC);
	}
}

void UT66GameplayHUDWidget::RefreshQuickReviveState()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState)
	{
		return;
	}

	if (QuickReviveIconRowBox.IsValid())
	{
		QuickReviveIconRowBox->SetVisibility(RunState->HasQuickReviveCharge() ? EVisibility::Visible : EVisibility::Collapsed);
	}

	if (QuickReviveDownedOverlayBorder.IsValid())
	{
		const bool bDowned = RunState->IsInQuickReviveDownedState();
		QuickReviveDownedOverlayBorder->SetVisibility(bDowned ? EVisibility::Visible : EVisibility::Collapsed);
		if (QuickReviveDownedText.IsValid())
		{
			const int32 SecondsRemaining = FMath::Max(1, FMath::CeilToInt(RunState->GetQuickReviveDownedSecondsRemaining()));
			QuickReviveDownedText->SetText(FText::Format(
				NSLOCTEXT("T66.GameplayHUD", "QuickReviveDownedCountdown", "REVIVING IN {0}"),
				FText::AsNumber(SecondsRemaining)));
		}
	}
}

void UT66GameplayHUDWidget::RefreshStatusEffects()
{
}

void UT66GameplayHUDWidget::RefreshHUD()
{
	bHUDDirty = false;
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
	RefreshDPS();

	RefreshHearts();
	RefreshQuickReviveState();
	RefreshStatusEffects();
	RefreshHeroStats();

	// Portrait: tier color from MaxHP (heart display tier).
	if (PortraitBorder.IsValid())
	{
		const int32 Tier = RunState->GetHeartDisplayTier();
		PortraitBorder->SetBorderBackgroundColor(RunState->GetMaxHP() > 0.f ? FT66RarityUtil::GetTierColor(Tier) : FLinearColor(0.12f, 0.12f, 0.14f, 1.f));
	}
	const FName DesiredPortraitHeroID = GIAsT66 ? GIAsT66->SelectedHeroID : NAME_None;
	const ET66BodyType DesiredPortraitBodyType = GIAsT66 ? GIAsT66->SelectedHeroBodyType : ET66BodyType::TypeA;
	ET66HeroPortraitVariant DesiredPortraitVariant = ET66HeroPortraitVariant::Half;
	const int32 HeartsRemaining = RunState->GetCurrentHearts();
	if (HeartsRemaining <= 1)
	{
		DesiredPortraitVariant = ET66HeroPortraitVariant::Low;
	}
	else if (HeartsRemaining >= 5)
	{
		DesiredPortraitVariant = ET66HeroPortraitVariant::Full;
	}

	const bool bPortraitStateChanged = !bPortraitStateInitialized
		|| LastPortraitHeroID != DesiredPortraitHeroID
		|| LastPortraitBodyType != DesiredPortraitBodyType
		|| LastPortraitVariant != DesiredPortraitVariant;

	if (bPortraitStateChanged)
	{
		bPortraitStateInitialized = true;
		LastPortraitHeroID = DesiredPortraitHeroID;
		LastPortraitBodyType = DesiredPortraitBodyType;
		LastPortraitVariant = DesiredPortraitVariant;

		TSoftObjectPtr<UTexture2D> PortraitSoft;
		if (GIAsT66 && !DesiredPortraitHeroID.IsNone())
		{
			FHeroData HeroData;
			if (GIAsT66->GetHeroData(DesiredPortraitHeroID, HeroData))
			{
				PortraitSoft = GIAsT66->ResolveHeroPortrait(HeroData, DesiredPortraitBodyType, DesiredPortraitVariant);
			}
		}

		bLastPortraitHasRef = !PortraitSoft.IsNull();
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
	}
	if (PortraitImage.IsValid())
	{
		const EVisibility DesiredVisibility = bLastPortraitHasRef ? EVisibility::Visible : EVisibility::Collapsed;
		if (PortraitImage->GetVisibility() != DesiredVisibility)
		{
			PortraitImage->SetVisibility(DesiredVisibility);
		}
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

	// Passive stack badge (Rallying Blow: show circle + stack count)
	{
		const bool bRallyingBlow = (RunState->GetPassiveType() == ET66PassiveType::RallyingBlow);
		const int32 Stacks = RunState->GetRallyStacks();
		if (PassiveStackBadgeBox.IsValid())
		{
			PassiveStackBadgeBox->SetVisibility(bRallyingBlow ? EVisibility::Visible : EVisibility::Collapsed);
		}
		if (PassiveStackText.IsValid())
		{
			PassiveStackText->SetText(FText::AsNumber(FMath::Max(0, Stacks)));
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

	// Idol slots (6): rarity-colored when equipped, dark teal when empty.
	UT66IdolManagerSubsystem* IdolManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr;
	const TArray<FName>& EquippedIdols = IdolManager ? IdolManager->GetEquippedIdols() : RunState->GetEquippedIdols();
	const bool bAllowPreviewLoadout = GIAsT66 && GIAsT66->bLoadAsPreview;
	TArray<FName> PreviewIdolIDs;
	TArray<ET66ItemRarity> PreviewIdolRarities;
	const TArray<FName>* DisplayIdols = &EquippedIdols;
	const TArray<ET66ItemRarity>* DisplayIdolRarities = nullptr;
	if (bAllowPreviewLoadout && !HasAnyNamedEntries(EquippedIdols))
	{
		BuildPreviewIdolDisplay(GIAsT66, PreviewIdolIDs, PreviewIdolRarities);
		if (HasAnyNamedEntries(PreviewIdolIDs))
		{
			DisplayIdols = &PreviewIdolIDs;
			DisplayIdolRarities = &PreviewIdolRarities;
		}
	}
	const TArray<FName>& Idols = *DisplayIdols;
	for (int32 i = 0; i < IdolSlotBorders.Num(); ++i)
	{
		if (!IdolSlotBorders[i].IsValid()) continue;
		FLinearColor C = FLinearColor(0.08f, 0.14f, 0.12f, 0.92f);
		TSoftObjectPtr<UTexture2D> IdolIconSoft;
		FText IdolTooltipText = FText::GetEmpty();
		if (i < Idols.Num() && !Idols[i].IsNone())
		{
			const ET66ItemRarity IdolRarity = (DisplayIdolRarities && DisplayIdolRarities->IsValidIndex(i))
				? (*DisplayIdolRarities)[i]
				: (IdolManager ? IdolManager->GetEquippedIdolRarityInSlot(i) : RunState->GetEquippedIdolRarityInSlot(i));
			C = FItemData::GetItemRarityColor(IdolRarity);
			if (GIAsT66)
			{
				FIdolData IdolData;
				if (GIAsT66->GetIdolData(Idols[i], IdolData))
				{
					IdolIconSoft = IdolData.GetIconForRarity(IdolRarity);
					// Tooltip: same as altar (category + effect description)
					if (Loc)
					{
						const UEnum* RarityEnum = StaticEnum<ET66ItemRarity>();
						const FText RarityText = RarityEnum
							? RarityEnum->GetDisplayNameTextByValue(static_cast<int64>(IdolRarity))
							: FText::GetEmpty();
						const FText CatName = Loc->GetText_IdolCategoryName(IdolData.Category);
						const FText Tooltip = Loc->GetText_IdolTooltip(Idols[i]);
						IdolTooltipText = FText::Format(
							NSLOCTEXT("T66.IdolAltar", "IdolCardDescWithRarityFormat", "{0}\n{1}\n{2}"),
							RarityText, CatName, Tooltip);
					}
					else
					{
						IdolTooltipText = FText::FromName(Idols[i]);
					}
				}
			}
		}
		IdolSlotBorders[i]->SetBorderBackgroundColor(C);
		if (IdolSlotContainers.IsValidIndex(i) && IdolSlotContainers[i].IsValid())
		{
			IdolSlotContainers[i]->SetToolTip(CreateCustomTooltip(IdolTooltipText));
		}

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
	}

	// Inventory slots: item color + hover tooltip, grey when empty
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	const TArray<FName>& EquippedInventory = RunState->GetInventory();
	const TArray<FT66InventorySlot>& EquippedInventorySlots = RunState->GetInventorySlots();
	TArray<FName> PreviewInventoryIDs;
	TArray<FT66InventorySlot> PreviewInventorySlots;
	const TArray<FName>* DisplayInventory = &EquippedInventory;
	const TArray<FT66InventorySlot>* DisplayInventorySlots = &EquippedInventorySlots;
	if (bAllowPreviewLoadout && !HasAnyNamedEntries(EquippedInventory))
	{
		BuildPreviewInventoryDisplay(T66GI, PreviewInventoryIDs, PreviewInventorySlots);
		if (HasAnyNamedEntries(PreviewInventoryIDs))
		{
			DisplayInventory = &PreviewInventoryIDs;
			DisplayInventorySlots = &PreviewInventorySlots;
		}
	}
	const TArray<FName>& Inv = *DisplayInventory;
	const TArray<FT66InventorySlot>& InvSlots = *DisplayInventorySlots;
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
				const ET66ItemRarity SlotRarity = InvSlots.IsValidIndex(i) ? InvSlots[i].Rarity : ET66ItemRarity::Black;
				SlotIconSoft = D.GetIconForRarity(SlotRarity);

				int32 MainValue = 0;
				if (InvSlots.IsValidIndex(i))
				{
					MainValue = InvSlots[i].Line1RolledValue;
				}
				const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f;
				const FText CardDesc = T66ItemCardTextUtils::BuildItemCardDescription(Loc, D, SlotRarity, MainValue, ScaleMult);
				if (!CardDesc.IsEmpty())
				{
					TipLines.Add(CardDesc);
				}
				{
					int32 SellValue = 0;
					if (RunState && DisplayInventorySlots == &EquippedInventorySlots && i >= 0 && i < InvSlots.Num())
					{
						SellValue = RunState->GetSellGoldForInventorySlot(InvSlots[i]);
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
		const FName CurrentItemID = (i < Inv.Num()) ? Inv[i] : NAME_None;
		if (!CachedInventorySlotIDs.IsValidIndex(i) || CachedInventorySlotIDs[i] != CurrentItemID)
		{
			if (CachedInventorySlotIDs.IsValidIndex(i)) CachedInventorySlotIDs[i] = CurrentItemID;
			if (InventorySlotContainers.IsValidIndex(i) && InventorySlotContainers[i].IsValid())
			{
				InventorySlotContainers[i]->SetToolTip(CreateCustomTooltip(Tooltip));
			}
		}

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
	const FText NetWorthInit = Loc ? FText::Format(Loc->GetText_NetWorthFormat(), FText::AsNumber(0)) : NSLOCTEXT("T66.GameplayHUD", "NetWorthInit", "Net Worth: 0");
	const FText GoldInit = Loc ? FText::Format(Loc->GetText_GoldFormat(), FText::AsNumber(0)) : NSLOCTEXT("T66.GameplayHUD", "GoldInit", "Gold: 0");
	const FText OweInit = Loc ? FText::Format(Loc->GetText_OweFormat(), FText::AsNumber(0)) : NSLOCTEXT("T66.GameplayHUD", "OweInit", "Debt: 0");
	const FText ScoreLabelText = Loc ? Loc->GetText_ScoreLabel() : NSLOCTEXT("T66.GameplayHUD", "ScoreLabel", "Score:");
	const FText PortraitLabel = Loc ? Loc->GetText_PortraitPlaceholder() : NSLOCTEXT("T66.GameplayHUD", "PortraitLabel", "PORTRAIT");
	const bool bDotaTheme = FT66Style::IsDotaTheme();
	const FLinearColor SlotOuterColor = bDotaTheme ? FT66Style::SlotOuter() : FLinearColor(0.f, 0.f, 0.f, 0.25f);
	const FLinearColor SlotFrameColor = bDotaTheme ? FT66Style::SlotInner() : FLinearColor(0.45f, 0.55f, 0.50f, 0.5f);
	const FLinearColor SlotFillColor = bDotaTheme ? FT66Style::SlotFill() : FLinearColor(0.f, 0.f, 0.f, 0.f);
	const FSlateBrush* DotaSlotFrameBrush = bDotaTheme ? FT66Style::GetInventorySlotFrameBrush() : nullptr;
	const FLinearColor BossBarBackgroundColor = bDotaTheme ? FT66Style::BossBarBackground() : FLinearColor(0.08f, 0.08f, 0.08f, 0.9f);
	const FLinearColor BossBarFillColor = bDotaTheme ? FT66Style::BossBarFill() : FLinearColor(0.9f, 0.1f, 0.1f, 0.95f);
	const FLinearColor PromptBackgroundColor = bDotaTheme ? FT66Style::PromptBackground() : FLinearColor(0.02f, 0.02f, 0.03f, 0.65f);
	const FLinearColor DialogueBackgroundColor = bDotaTheme ? FT66Style::PromptBackground() : FLinearColor(0.06f, 0.06f, 0.08f, 0.85f);

	HeartBorders.SetNum(UT66RunStateSubsystem::DefaultMaxHearts);
	HeartImages.SetNum(UT66RunStateSubsystem::DefaultMaxHearts);
	DifficultyBorders.SetNum(5);
	DifficultyImages.SetNum(5);
	ClownImages.SetNum(5);
	IdolSlotBorders.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
	IdolSlotContainers.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
	IdolSlotImages.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
	IdolSlotBrushes.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
	IdolLevelDotBorders.Empty();
	CachedIdolSlotIDs.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
	InventorySlotBorders.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotContainers.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotImages.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotBrushes.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	CachedInventorySlotIDs.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
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
	if (!QuickReviveBrush.IsValid())
	{
		QuickReviveBrush = MakeShared<FSlateBrush>();
		QuickReviveBrush->DrawAs = ESlateBrushDrawType::Image;
		QuickReviveBrush->ImageSize = FVector2D(34.f, 34.f);
		QuickReviveBrush->Tiling = ESlateBrushTileType::NoTile;
		QuickReviveBrush->SetResourceObject(nullptr);
	}
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> QuickReviveSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Interactables/QuickReviveIcon.QuickReviveIcon")));
			T66SlateTexture::BindSharedBrushAsync(TexPool, QuickReviveSoft, this, QuickReviveBrush, FName(TEXT("HUDQuickRevive")), false);
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
		InventorySlotBrushes[i]->ImageSize = FVector2D(36.f, 36.f);
	}

	// Difficulty row (5-slot skull sprites).
	static constexpr float MinimapWidth = 228.f;
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

	TSharedRef<SWidget> QuickReviveIconRowRef =
		SAssignNew(QuickReviveIconRowBox, SBox)
		.Visibility(EVisibility::Collapsed)
		.WidthOverride(HeartsRowWidth)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(34.f)
				.HeightOverride(34.f)
				[
					SAssignNew(QuickReviveIconImage, SImage)
					.Image(QuickReviveBrush.Get())
					.ColorAndOpacity(FLinearColor::White)
				]
			]
		];

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
	static constexpr int32 IdolColumns = 2;
	static constexpr float IdolSlotPad = 3.f;
	static constexpr float IdolSlotSize = 38.f;
	const int32 IdolRows = FMath::DivideAndRoundUp(UT66IdolManagerSubsystem::MaxEquippedIdolSlots, IdolColumns);
	const float IdolPanelMinWidth = (IdolColumns * IdolSlotSize) + ((IdolColumns * 2.f) * IdolSlotPad) + 28.f;
	const float IdolPanelMinHeight = (IdolRows * IdolSlotSize) + ((IdolRows * 2.f) * IdolSlotPad) + (bDotaTheme ? 56.f : 50.f);
	const float HUDSidePanelMinHeight = FMath::Max(IdolPanelMinHeight, 182.f);
	const float StatsPanelMinWidth = bDotaTheme ? 170.f : 156.f;
	for (int32 i = 0; i < UT66IdolManagerSubsystem::MaxEquippedIdolSlots; ++i)
	{
		TSharedPtr<SBorder> IdolBorder;
		const int32 Row = i / IdolColumns;
		const int32 Col = i % IdolColumns;
		IdolSlotsRef->AddSlot(Col, Row)
			.Padding(IdolSlotPad)
			[
				SAssignNew(IdolSlotContainers[i], SBox)
				.WidthOverride(IdolSlotSize)
				.HeightOverride(IdolSlotSize)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
					SAssignNew(IdolBorder, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(SlotOuterColor)
					.Padding(1.f)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(SlotFrameColor)
							.Padding(1.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(SlotFillColor)
							]
						]
						+ SOverlay::Slot()
						[
							SNew(SImage)
							.Visibility(DotaSlotFrameBrush ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
							.Image(DotaSlotFrameBrush)
							.ColorAndOpacity(FLinearColor::White)
						]
						+ SOverlay::Slot()
						.VAlign(VAlign_Top)
						.Padding(1.f, 1.f, 1.f, 0.f)
						[
							SNew(SBox)
							.HeightOverride(2.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.95f, 0.97f, 1.0f, bDotaTheme ? 0.12f : 0.08f))
							]
						]
						+ SOverlay::Slot()
						.VAlign(VAlign_Bottom)
						.Padding(1.f, 0.f, 1.f, 1.f)
						[
							SNew(SBox)
							.HeightOverride(2.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.42f))
							]
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
				]
			];
		IdolSlotBorders[i] = IdolBorder;
	}

	// Inventory: 2 rows x 10 columns (20 slots total). RPG-style with transparent slots and thin borders.
	static constexpr int32 InvCols = 10;
	static constexpr int32 InvRows = 2;
	static constexpr float InvSlotSize = 36.f;
	static constexpr float InvSlotPad = 2.f;
	const FLinearColor InvSlotBorderColor = SlotFrameColor;
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
					SAssignNew(InventorySlotContainers[ThisSlotIndex], SBox)
					.WidthOverride(InvSlotSize)
					.HeightOverride(InvSlotSize)
					[
						SNew(SOverlay)
						// Transparent slot bg with thin border outline
						+ SOverlay::Slot()
						[
							SAssignNew(SlotBorder, SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(SlotOuterColor)
							.Padding(1.f)
							[
								SNew(SOverlay)
								+ SOverlay::Slot()
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(InvSlotBorderColor)
									.Padding(1.f)
									[
										SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(SlotFillColor)
									]
								]
								+ SOverlay::Slot()
								[
									SNew(SImage)
									.Visibility(DotaSlotFrameBrush ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
									.Image(DotaSlotFrameBrush)
									.ColorAndOpacity(FLinearColor::White)
								]
								+ SOverlay::Slot()
								.VAlign(VAlign_Top)
								.Padding(1.f, 1.f, 1.f, 0.f)
								[
									SNew(SBox)
									.HeightOverride(2.f)
									[
										SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(FLinearColor(0.95f, 0.97f, 1.0f, bDotaTheme ? 0.12f : 0.08f))
									]
								]
								+ SOverlay::Slot()
								.VAlign(VAlign_Bottom)
								.Padding(1.f, 0.f, 1.f, 1.f)
								[
									SNew(SBox)
									.HeightOverride(2.f)
									[
										SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.42f))
									]
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
					.BorderBackgroundColor(BossBarBackgroundColor)
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
						.BorderBackgroundColor(BossBarFillColor)
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
				.BorderBackgroundColor(PromptBackgroundColor)
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
						.BorderBackgroundColor(DialogueBackgroundColor)
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
						.BorderBackgroundColor(DialogueBackgroundColor)
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
						.BorderBackgroundColor(DialogueBackgroundColor)
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
			bDotaTheme
				? FT66Style::MakeHudPanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(ScoreLabelText)
							.Font(FT66Style::Tokens::FontBold(17))
							.ColorAndOpacity(FSlateColor(FT66Style::TextMuted()))
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
						[
							SAssignNew(ScoreText, STextBlock)
							.Text(FText::AsNumber(0))
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FSlateColor(FT66Style::Text()))
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
						[
							SAssignNew(ScoreMultiplierText, STextBlock)
							.Text(NSLOCTEXT("T66.GameplayHUD", "ScoreMultiplierDefault", "x1.0"))
							.Font(FT66Style::Tokens::FontBold(16))
							.ColorAndOpacity(FSlateColor(FT66Style::Accent2()))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
					[
						SAssignNew(DPSText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "DPSDefault", "DPS 0"))
						.Font(FT66Style::Tokens::FontBold(16))
						.ColorAndOpacity(FSlateColor(FT66Style::TextMuted()))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunDefault", "Time 0:00.00"))
						.Font(FT66Style::Tokens::FontBold(16))
						.ColorAndOpacity(FSlateColor(FT66Style::Text()))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunTargetText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunTargetDefault", "TO BEAT --:--.--"))
						.Font(FT66Style::Tokens::FontBold(15))
						.ColorAndOpacity(FSlateColor(FT66Style::TextMuted()))
						.Visibility(EVisibility::Collapsed)
					],
					NSLOCTEXT("T66.GameplayHUD", "CombatTitle", "COMBAT"),
					FMargin(14.f, 10.f))
				: FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(ScoreLabelText)
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FSlateColor(FT66Style::Tokens::Text))
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SAssignNew(ScoreText, STextBlock)
							.Text(FText::AsNumber(0))
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FSlateColor(FT66Style::Tokens::Text))
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
						[
							SAssignNew(ScoreMultiplierText, STextBlock)
							.Text(NSLOCTEXT("T66.GameplayHUD", "ScoreMultiplierDefault", "x1.0"))
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FSlateColor(FT66Style::Tokens::Text))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						SAssignNew(DPSText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "DPSDefault", "DPS 0"))
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(FSlateColor(FT66Style::Tokens::TextMuted))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunDefault", "Time 0:00.00"))
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(FSlateColor(FT66Style::Tokens::Text))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunTargetText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunTargetDefault", "TO BEAT --:--.--"))
						.Font(FT66Style::Tokens::FontBold(18))
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
				// Elevation above FPS above idol panel
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 2.f)
				[
					SAssignNew(ElevationText, STextBlock)
					.Text(NSLOCTEXT("T66.GameplayHUD", "ElevationDefault", "ELV: 0"))
					.Font(FT66Style::Tokens::FontBold(14))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					.Justification(ETextJustify::Left)
				]
				// FPS above idol panel
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
				[
					SAssignNew(FPSText, STextBlock)
					.Text(NSLOCTEXT("T66.GameplayHUD", "FPSDefault", "FPS: 0"))
					.Font(FT66Style::Tokens::FontBold(14))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					.Justification(ETextJustify::Left)
				]
				// Core bottom-left HUD block
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					// Idol slots: bordered panel with "Idols" title; top aligned with portrait (shorter panel)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom).Padding(0.f, 92.f, 0.f, 0.f)
					[
						SAssignNew(IdolSlotsPanelBox, SBox)
						.MinDesiredWidth(IdolPanelMinWidth)
						.MinDesiredHeight(HUDSidePanelMinHeight)
						[
							bDotaTheme
								? FT66Style::MakeHudPanel(IdolSlotsRef, NSLOCTEXT("T66.GameplayHUD", "IdolsTitle", "IDOLS"), FMargin(10.f, 10.f))
								: StaticCastSharedRef<SWidget>(
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
									])
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
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
					[
						QuickReviveIconRowRef
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
							bDotaTheme
								? StaticCastSharedRef<SWidget>(
									SNew(SOverlay)
									+ SOverlay::Slot()
									[
										SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(FT66Style::PanelOuter())
										.Padding(1.f)
										[
											SNew(SBorder)
											.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
											.BorderBackgroundColor(FT66Style::Border())
											.Padding(1.f)
											[
												SAssignNew(PortraitBorder, SBorder)
												.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
												.BorderBackgroundColor(FT66Style::PanelInner())
											]
										]
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
										.ColorAndOpacity(FT66Style::TextMuted())
										.Justification(ETextJustify::Center)
										.Visibility_Lambda([this]() -> EVisibility
										{
											return (PortraitBrush.IsValid() && PortraitBrush->GetResourceObject()) ? EVisibility::Collapsed : EVisibility::Visible;
										})
									]
									+ SOverlay::Slot()
									.HAlign(HAlign_Right)
									.VAlign(VAlign_Top)
									.Padding(0.f, 2.f, 2.f, 0.f)
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
												SAssignNew(LevelText, STextBlock)
												.Text(FText::AsNumber(1))
												.Font(FT66Style::Tokens::FontBold(14))
												.ColorAndOpacity(FT66Style::Accent2())
												.Justification(ETextJustify::Center)
											]
										]
									])
								: StaticCastSharedRef<SWidget>(
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
									+ SOverlay::Slot()
									.HAlign(HAlign_Right)
									.VAlign(VAlign_Top)
									.Padding(0.f, 2.f, 2.f, 0.f)
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
												SAssignNew(LevelText, STextBlock)
												.Text(FText::AsNumber(1))
												.Font(FT66Style::Tokens::FontBold(14))
												.ColorAndOpacity(FLinearColor(0.90f, 0.75f, 0.20f, 1.f))
												.Justification(ETextJustify::Center)
											]
										]
									])
						]
					]
				]
				// Hero stats panel (right of portrait): primary stats only, same height as portrait; toggles with T
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom).Padding(12.f, 0.f, 0.f, 0.f)
				[
					SAssignNew(PortraitStatPanelBox, SBox)
					.MinDesiredWidth(StatsPanelMinWidth)
					.MinDesiredHeight(HUDSidePanelMinHeight)
					[
						bDotaTheme
							? FT66Style::MakeHudPanel(
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
									.Padding(0.f)
									.ToolTip(CreateRichTooltip(Loc ? Loc->GetText_Stat_Damage() : FText::GetEmpty(), Loc ? Loc->GetText_PrimaryStatDescription(1) : FText::GetEmpty()))
									[
										SAssignNew(StatDamageText, STextBlock)
										.Text(FText::FromString(TEXT("Dmg: F")))
										.Font(FT66Style::Tokens::FontBold(16))
										.ColorAndOpacity(FT66Style::Text())
									]
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
									.Padding(0.f)
									.ToolTip(CreateRichTooltip(Loc ? Loc->GetText_Stat_AttackSpeed() : FText::GetEmpty(), Loc ? Loc->GetText_PrimaryStatDescription(2) : FText::GetEmpty()))
									[
										SAssignNew(StatAttackSpeedText, STextBlock)
										.Text(FText::FromString(TEXT("AS: F")))
										.Font(FT66Style::Tokens::FontBold(16))
										.ColorAndOpacity(FT66Style::Text())
									]
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
									.Padding(0.f)
									.ToolTip(CreateRichTooltip(Loc ? Loc->GetText_Stat_AttackScale() : FText::GetEmpty(), Loc ? Loc->GetText_PrimaryStatDescription(3) : FText::GetEmpty()))
									[
										SAssignNew(StatAttackScaleText, STextBlock)
										.Text(FText::FromString(TEXT("Scale: F")))
										.Font(FT66Style::Tokens::FontBold(16))
										.ColorAndOpacity(FT66Style::Text())
									]
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
									.Padding(0.f)
									.ToolTip(CreateRichTooltip(Loc ? Loc->GetText_Stat_Armor() : FText::GetEmpty(), Loc ? Loc->GetText_PrimaryStatDescription(4) : FText::GetEmpty()))
									[
										SAssignNew(StatArmorText, STextBlock)
										.Text(FText::FromString(TEXT("Armor: F")))
										.Font(FT66Style::Tokens::FontBold(16))
										.ColorAndOpacity(FT66Style::Text())
									]
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
									.Padding(0.f)
									.ToolTip(CreateRichTooltip(Loc ? Loc->GetText_Stat_Evasion() : FText::GetEmpty(), Loc ? Loc->GetText_PrimaryStatDescription(5) : FText::GetEmpty()))
									[
										SAssignNew(StatEvasionText, STextBlock)
										.Text(FText::FromString(TEXT("Eva: F")))
										.Font(FT66Style::Tokens::FontBold(16))
										.ColorAndOpacity(FT66Style::Text())
									]
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
									.Padding(0.f)
									.ToolTip(CreateRichTooltip(Loc ? Loc->GetText_Stat_Luck() : FText::GetEmpty(), Loc ? Loc->GetText_PrimaryStatDescription(6) : FText::GetEmpty()))
									[
										SAssignNew(StatLuckText, STextBlock)
										.Text(FText::FromString(TEXT("Luck: F")))
										.Font(FT66Style::Tokens::FontBold(16))
										.ColorAndOpacity(FT66Style::Text())
									]
								]
								+ SVerticalBox::Slot().AutoHeight()
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
									.Padding(0.f)
									.ToolTip(CreateRichTooltip(Loc ? Loc->GetText_Stat_Speed() : FText::GetEmpty(), Loc ? Loc->GetText_PrimaryStatDescription(7) : FText::GetEmpty()))
									[
										SAssignNew(StatSpeedText, STextBlock)
										.Text(FText::FromString(TEXT("Speed: F")))
										.Font(FT66Style::Tokens::FontBold(16))
										.ColorAndOpacity(FT66Style::Text())
									]
								],
								NSLOCTEXT("T66.GameplayHUD", "StatsTitleUpper", "STATS"),
								FMargin(12.f, 10.f))
							: StaticCastSharedRef<SWidget>(
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
												.Font(FT66Style::Tokens::FontBold(23))
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
												SNew(SBorder)
												.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
												.Padding(0.f)
												.ToolTip(CreateRichTooltip(Loc ? Loc->GetText_Stat_Damage() : FText::GetEmpty(), Loc ? Loc->GetText_PrimaryStatDescription(1) : FText::GetEmpty()))
												[
													SAssignNew(StatDamageText, STextBlock)
													.Text(FText::FromString(TEXT("Dmg: F")))
													.Font(FT66Style::Tokens::FontBold(18))
													.ColorAndOpacity(FT66Style::Tokens::Text)
												]
											]
											+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 2.f)
											[
												SNew(SBorder)
												.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
												.Padding(0.f)
												.ToolTip(CreateRichTooltip(Loc ? Loc->GetText_Stat_AttackSpeed() : FText::GetEmpty(), Loc ? Loc->GetText_PrimaryStatDescription(2) : FText::GetEmpty()))
												[
													SAssignNew(StatAttackSpeedText, STextBlock)
													.Text(FText::FromString(TEXT("AS: F")))
													.Font(FT66Style::Tokens::FontBold(18))
													.ColorAndOpacity(FT66Style::Tokens::Text)
												]
											]
											+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 2.f)
											[
												SNew(SBorder)
												.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
												.Padding(0.f)
												.ToolTip(CreateRichTooltip(Loc ? Loc->GetText_Stat_AttackScale() : FText::GetEmpty(), Loc ? Loc->GetText_PrimaryStatDescription(3) : FText::GetEmpty()))
												[
													SAssignNew(StatAttackScaleText, STextBlock)
													.Text(FText::FromString(TEXT("Scale: F")))
													.Font(FT66Style::Tokens::FontBold(18))
													.ColorAndOpacity(FT66Style::Tokens::Text)
												]
											]
											+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 2.f)
											[
												SNew(SBorder)
												.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
												.Padding(0.f)
												.ToolTip(CreateRichTooltip(Loc ? Loc->GetText_Stat_Armor() : FText::GetEmpty(), Loc ? Loc->GetText_PrimaryStatDescription(4) : FText::GetEmpty()))
												[
													SAssignNew(StatArmorText, STextBlock)
													.Text(FText::FromString(TEXT("Armor: F")))
													.Font(FT66Style::Tokens::FontBold(18))
													.ColorAndOpacity(FT66Style::Tokens::Text)
												]
											]
											+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 2.f)
											[
												SNew(SBorder)
												.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
												.Padding(0.f)
												.ToolTip(CreateRichTooltip(Loc ? Loc->GetText_Stat_Evasion() : FText::GetEmpty(), Loc ? Loc->GetText_PrimaryStatDescription(5) : FText::GetEmpty()))
												[
													SAssignNew(StatEvasionText, STextBlock)
													.Text(FText::FromString(TEXT("Eva: F")))
													.Font(FT66Style::Tokens::FontBold(18))
													.ColorAndOpacity(FT66Style::Tokens::Text)
												]
											]
											+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 2.f)
											[
												SNew(SBorder)
												.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
												.Padding(0.f)
												.ToolTip(CreateRichTooltip(Loc ? Loc->GetText_Stat_Luck() : FText::GetEmpty(), Loc ? Loc->GetText_PrimaryStatDescription(6) : FText::GetEmpty()))
												[
													SAssignNew(StatLuckText, STextBlock)
													.Text(FText::FromString(TEXT("Luck: F")))
													.Font(FT66Style::Tokens::FontBold(18))
													.ColorAndOpacity(FT66Style::Tokens::Text)
												]
											]
											+ SVerticalBox::Slot().AutoHeight()
											[
												SNew(SBorder)
												.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
												.Padding(0.f)
												.ToolTip(CreateRichTooltip(Loc ? Loc->GetText_Stat_Speed() : FText::GetEmpty(), Loc ? Loc->GetText_PrimaryStatDescription(7) : FText::GetEmpty()))
												[
													SAssignNew(StatSpeedText, STextBlock)
													.Text(FText::FromString(TEXT("Speed: F")))
													.Font(FT66Style::Tokens::FontBold(18))
													.ColorAndOpacity(FT66Style::Tokens::Text)
												]
											]
										]
									]
								])
					]
				]
				// Passive icon (above) + Ultimate ability icon (to the right of portrait area)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom).Padding(16.f, 0.f, 0.f, 0.f)
				[
					SNew(SBox)
					.WidthOverride(84.f)
					.HeightOverride(174.f)
					[
						SNew(SVerticalBox)
						// Passive ability icon + stack badge (circle + number for Rallying Blow)
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
						[
							SNew(SBox)
							.WidthOverride(84.f)
							.HeightOverride(84.f)
							[
								SNew(SOverlay)
								// Passive icon (border + image)
								+ SOverlay::Slot()
								[
									SAssignNew(PassiveBorder, SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.10f, 1.f))
									.Padding(0.f)
									[
										SAssignNew(PassiveImage, SImage)
										.Image(UltimateBrush.Get())
										.ColorAndOpacity(FLinearColor::White)
									]
								]
								// Stack badge: circle background + number (bottom-right)
								+ SOverlay::Slot()
								.HAlign(HAlign_Right)
								.VAlign(VAlign_Bottom)
								.Padding(0.f, 0.f, 4.f, 4.f)
								[
									SAssignNew(PassiveStackBadgeBox, SBox)
									.WidthOverride(28.f)
									.HeightOverride(28.f)
									[
										SNew(SOverlay)
										+ SOverlay::Slot()
										[
											SNew(SBorder)
											.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Circle"))
											.BorderBackgroundColor(FLinearColor(0.12f, 0.10f, 0.08f, 0.95f))
											.Padding(0.f)
											.HAlign(HAlign_Center)
											.VAlign(VAlign_Center)
											[
												SAssignNew(PassiveStackText, STextBlock)
												.Text(FText::AsNumber(0))
												.Font(FT66Style::Tokens::FontBold(12))
												.ColorAndOpacity(FLinearColor(0.95f, 0.75f, 0.25f, 1.f))
												.Justification(ETextJustify::Center)
											]
										]
									]
								]
							]
						]
						// Ultimate ability icon
						+ SVerticalBox::Slot().AutoHeight()
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
					.WidthOverride(MinimapWidth)
					.HeightOverride(MinimapWidth)
					[
						bDotaTheme
							? FT66Style::MakeMinimapFrame(
								SAssignNew(MinimapWidget, ST66WorldMapWidget)
								.bMinimap(true)
								.bShowLabels(false),
								FMargin(8.f))
							: StaticCastSharedRef<SWidget>(
								SNew(SOverlay)
								+ SOverlay::Slot()
								[
									FT66Style::MakePanel(
										SAssignNew(MinimapWidget, ST66WorldMapWidget)
										.bMinimap(true)
										.bShowLabels(false),
										FT66PanelParams(ET66PanelType::Panel2).SetPadding(8.f)
									)
								])
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
		// Inventory panel bottom-right (Gold/Owe and grid); FPS is above idol panel on the left
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(24.f, 0.f, 24.f, 24.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SAssignNew(InventoryPanelBox, SBox)
				[
					bDotaTheme
						? FT66Style::MakeHudPanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().Padding(4.f, 0.f, 4.f, 8.f)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								[
									SAssignNew(NetWorthText, STextBlock)
									.Text(NetWorthInit)
									.Font(FT66Style::Tokens::FontBold(17))
									.ColorAndOpacity(FT66Style::TextMuted())
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
								[
									SAssignNew(GoldText, STextBlock)
									.Text(GoldInit)
									.Font(FT66Style::Tokens::FontBold(18))
									.ColorAndOpacity(FT66Style::Accent2())
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
								[
									SAssignNew(DebtText, STextBlock)
									.Text(OweInit)
									.Font(FT66Style::Tokens::FontBold(18))
									.ColorAndOpacity(FT66Style::Danger())
								]
							]
							+ SVerticalBox::Slot().AutoHeight()
							[
								InvGridRef
							],
							NSLOCTEXT("T66.GameplayHUD", "InventoryTitle", "INVENTORY"),
							FMargin(12.f, 10.f))
						: StaticCastSharedRef<SWidget>(
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
									.Padding(FMargin(10.f, 8.f))
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 4.f)
										[
											SNew(STextBlock)
											.Text(NSLOCTEXT("T66.GameplayHUD", "InventoryTitle", "Inventory"))
											.Font(FT66Style::Tokens::FontBold(18))
											.ColorAndOpacity(FLinearColor(0.75f, 0.82f, 0.78f, 1.f))
											.Justification(ETextJustify::Center)
										]
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
										+ SVerticalBox::Slot().AutoHeight().Padding(4.f, 0.f, 4.f, 8.f)
										[
											SNew(SVerticalBox)
											+ SVerticalBox::Slot().AutoHeight()
											[
												SAssignNew(NetWorthText, STextBlock)
												.Text(NetWorthInit)
												.Font(FT66Style::Tokens::FontBold(18))
												.ColorAndOpacity(FT66Style::Tokens::Text)
											]
											+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
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
										+ SVerticalBox::Slot().AutoHeight()
										[
											InvGridRef
										]
									]
								]
							]
							+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top)
							[
								SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.55f, 0.65f, 0.58f, 0.9f))
								]
							]
							+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top)
							[
								SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.55f, 0.65f, 0.58f, 0.9f))
								]
							]
							+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom)
							[
								SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.55f, 0.65f, 0.58f, 0.9f))
								]
							]
							+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom)
							[
								SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.55f, 0.65f, 0.58f, 0.9f))
								]
							])
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
		// Pickup item card (right side, bottom of card just above inventory)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(24.f, 0.f, 24.f, PickupCardBottomOffset)
		[
			SAssignNew(PickupCardBox, SBox)
			.Visibility(EVisibility::Collapsed)
			.WidthOverride(PickupCardWidth)
			.HeightOverride(PickupCardHeight)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SAssignNew(PickupCardNameText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(16))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.AutoWrapText(true)
						.WrapTextAt(PickupCardWidth - 24.f)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space2, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center)
						[
							FT66Style::MakePanel(
								SNew(SBox)
								.WidthOverride(PickupCardWidth)
								.HeightOverride(PickupCardWidth)
								[
									SAssignNew(PickupCardIconImage, SImage)
									.Image(PickupCardIconBrush.Get())
									.ColorAndOpacity(FLinearColor::White)
									.Visibility(EVisibility::Collapsed)
								],
								FT66PanelParams(ET66PanelType::Panel2).SetPadding(0.f),
								&PickupCardIconBorder)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space2, 0.f, 0.f)
					[
						SAssignNew(PickupCardDescText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontRegular(12))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f)
					[
						FT66Style::MakePanel(
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
							[
								SAssignNew(PickupCardSkipText, STextBlock)
								.Text(FText::GetEmpty())
								.Font(FT66Style::Tokens::FontBold(14))
								.ColorAndOpacity(FT66Style::Tokens::Text)
								.Justification(ETextJustify::Center)
							],
							FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(8.f, 6.f)))
					],
					FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4),
					&PickupCardTileBorder)
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
			SAssignNew(CenterCrosshairBox, SBox)
			.WidthOverride(28.f)
			.HeightOverride(28.f)
			[
				SNew(ST66CrosshairWidget)
			]
		]
		// Hero 1 scoped sniper overlay (first-person aim view + ult timers)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(ScopedSniperOverlayBorder, SBorder)
			.Visibility(EVisibility::Collapsed)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
			.Padding(0.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(ST66ScopedSniperWidget)
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				.Padding(0.f, 24.f, 0.f, 0.f)
				[
					FT66Style::MakePanel(
						SAssignNew(ScopedUltTimerText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(20))
						.ColorAndOpacity(FT66Style::Tokens::Text),
						FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(12.f, 8.f)))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Bottom)
				.Padding(0.f, 0.f, 0.f, 42.f)
				[
					FT66Style::MakePanel(
						SAssignNew(ScopedShotCooldownText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(FT66Style::Tokens::Text),
						FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f, 8.f)))
				]
			]
		]
		// Quick Revive downed overlay
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(QuickReviveDownedOverlayBorder, SBorder)
			.Visibility(EVisibility::Collapsed)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.42f, 0.42f, 0.42f, 0.58f))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(QuickReviveDownedText, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(28))
				.ColorAndOpacity(FLinearColor::White)
				.Justification(ETextJustify::Center)
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
								.Font(FT66Style::Tokens::FontBold(18))
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
				? (FT66Style::IsDotaTheme() ? FT66Style::SelectionFill() : FLinearColor(0.18f, 0.18f, 0.26f, 0.95f))
				: (FT66Style::IsDotaTheme() ? FT66Style::PromptBackground() : FLinearColor(0.06f, 0.06f, 0.08f, 0.85f)));
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

void UT66GameplayHUDWidget::SetInteractive(bool bInteractive)
{
	SetVisibility(bInteractive ? ESlateVisibility::Visible : ESlateVisibility::SelfHitTestInvisible);
}
