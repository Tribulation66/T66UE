// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66FrontendTopBarWidget.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "UI/ST66PulsingIcon.h"
#include "UI/T66UIManager.h"
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
#include "Widgets/SLeafWidget.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float GTopBarReservedHeight = 146.f;
	constexpr float GTopBarButtonWidth = 84.f;
	constexpr float GTopBarButtonHeight = 78.f;
	constexpr float GTopBarSurfaceHeight = 118.f;
	constexpr float GTopBarPlateVisibleHeightRatio = 98.f / 108.f;
	constexpr float GTopBarInactiveNavHeight = 70.f;
	constexpr float GTopBarActiveNavHeight = 76.f;
	constexpr float GTopBarInactiveNavMinWidth = 156.f;
	constexpr float GTopBarActiveNavMinWidth = 168.f;
	constexpr float GTopBarIconOnlyNavWidth = 72.f;
	constexpr float GTopBarNavSeparatorHorizontalPadding = 8.f;
	constexpr float GTopBarRightClusterGap = 10.f;
	constexpr int32 GTopBarViewportZOrder = 50;
	TMap<FString, TStrongObjectPtr<UTexture2D>> GTopBarFileTextureCache;

	float SnapPixel(float Value)
	{
		return FMath::RoundToFloat(Value);
	}

	FVector2D SnapPixelSize(const FVector2D& Value)
	{
		return FVector2D(SnapPixel(Value.X), SnapPixel(Value.Y));
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
}

UT66FrontendTopBarWidget::UT66FrontendTopBarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::None;
	bIsModal = false;
}

float UT66FrontendTopBarWidget::GetReservedHeight()
{
	return GTopBarReservedHeight;
}

float UT66FrontendTopBarWidget::GetVisibleContentHeight()
{
	return GTopBarSurfaceHeight;
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
		UIManager->ShowScreenWithoutHistory(ET66ScreenType::MainMenu);
		return;
	}

	UIManager->ShowScreen(TargetScreen);
}

void UT66FrontendTopBarWidget::RequestTopBarAssets()
{
	LoadBrushFromRelativePath(TEXT("/Game/SourceAssets/UI/Dota/Generated/frontend_topbar_plate.frontend_topbar_plate"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_plate.png"), TopBarPlateBrush);
	LoadBrushFromRelativePath(TEXT("/Game/SourceAssets/UI/Dota/Generated/frontend_topbar_nav_tab.frontend_topbar_nav_tab"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_nav_tab.png"), InactiveTabBrush);
	LoadBrushFromRelativePath(TEXT("/Game/SourceAssets/UI/Dota/Generated/frontend_topbar_active_tab.frontend_topbar_active_tab"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_active_tab.png"), ActiveTabBrush);
	LoadBrushFromRelativePath(TEXT("/Game/SourceAssets/UI/Dota/Generated/frontend_topbar_home_tab.frontend_topbar_home_tab"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_home_tab.png"), HomeInactiveTabBrush);
	LoadBrushFromRelativePath(TEXT("/Game/SourceAssets/UI/Dota/Generated/frontend_topbar_home_active_tab.frontend_topbar_home_active_tab"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_home_active_tab.png"), HomeActiveTabBrush);
	LoadBrushFromRelativePath(TEXT("/Game/UI/Assets/TopBar/frontend_topbar_home_icon.frontend_topbar_home_icon"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_home_icon.png"), HomeIconBrush, FVector2D(112.f, 112.f), TextureFilter::TF_Trilinear);
	LoadBrushFromRelativePath(TEXT("/Game/SourceAssets/UI/Dota/Generated/frontend_topbar_separator.frontend_topbar_separator"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_separator.png"), NavSeparatorBrush);
	LoadBrushFromRelativePath(TEXT("/Game/SourceAssets/UI/Dota/Generated/frontend_topbar_settings_slot.frontend_topbar_settings_slot"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_settings_slot.png"), SettingsSlotBrush, FVector2D(78.f, 78.f));
	LoadBrushFromRelativePath(TEXT("/Game/SourceAssets/UI/Dota/Generated/frontend_topbar_utility_slot.frontend_topbar_utility_slot"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_utility_slot.png"), UtilitySlotBrush, FVector2D(78.f, 78.f));
	LoadBrushFromRelativePath(TEXT("/Game/SourceAssets/UI/Dota/Generated/frontend_topbar_quit_slot.frontend_topbar_quit_slot"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_quit_slot.png"), QuitSlotBrush, FVector2D(78.f, 78.f));
	LoadBrushFromRelativePath(TEXT("/Game/UI/Assets/TopBar/frontend_topbar_settings_icon.frontend_topbar_settings_icon"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_settings_icon.png"), SettingsIconBrush, FVector2D(44.f, 44.f), TextureFilter::TF_Trilinear);
	LoadBrushFromRelativePath(TEXT("/Game/UI/Assets/TopBar/frontend_topbar_language_icon.frontend_topbar_language_icon"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_language_icon.png"), LanguageIconBrush, FVector2D(48.f, 48.f), TextureFilter::TF_Trilinear);
	LoadBrushFromRelativePath(TEXT("/Game/UI/Assets/TopBar/frontend_topbar_achievement_coins_icon.frontend_topbar_achievement_coins_icon"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_achievement_coins_icon.png"), AchievementCoinsIconBrush, FVector2D(48.f, 48.f), TextureFilter::TF_Trilinear);
	LoadBrushFromRelativePath(TEXT("/Game/UI/Assets/TopBar/frontend_topbar_power_coupons_icon.frontend_topbar_power_coupons_icon"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_power_coupons_icon.png"), PowerCouponsIconBrush, FVector2D(66.f, 44.f), TextureFilter::TF_Trilinear);
	LoadBrushFromRelativePath(TEXT("/Game/UI/Assets/TopBar/frontend_topbar_quit_icon.frontend_topbar_quit_icon"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_quit_icon.png"), QuitIconBrush, FVector2D(44.f, 44.f), TextureFilter::TF_Trilinear);
	LoadBrushFromRelativePath(TEXT("/Game/SourceAssets/UI/Dota/Generated/frontend_topbar_quit_glow.frontend_topbar_quit_glow"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_quit_glow.png"), QuitGlowBrush, FVector2D(62.f, 62.f));

	if (TopBarPlateBrush.IsValid())
	{
		TopBarPlateBrush->DrawAs = ESlateBrushDrawType::Box;
		TopBarPlateBrush->Margin = FMargin(0.10f, 0.18f, 0.10f, 0.22f);
	}

	if (InactiveTabBrush.IsValid())
	{
		InactiveTabBrush->DrawAs = ESlateBrushDrawType::Box;
		InactiveTabBrush->Margin = FMargin(0.16f, 0.18f, 0.16f, 0.28f);
	}

	if (ActiveTabBrush.IsValid())
	{
		ActiveTabBrush->DrawAs = ESlateBrushDrawType::Box;
		ActiveTabBrush->Margin = FMargin(0.16f, 0.16f, 0.16f, 0.28f);
	}

	if (HomeInactiveTabBrush.IsValid())
	{
		HomeInactiveTabBrush->DrawAs = ESlateBrushDrawType::Box;
		HomeInactiveTabBrush->Margin = FMargin(0.14f, 0.16f, 0.14f, 0.24f);
	}

	if (HomeActiveTabBrush.IsValid())
	{
		HomeActiveTabBrush->DrawAs = ESlateBrushDrawType::Box;
		HomeActiveTabBrush->Margin = FMargin(0.14f, 0.14f, 0.14f, 0.24f);
	}
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

	ReleaseBrush(TopBarPlateBrush);
	ReleaseBrush(InactiveTabBrush);
	ReleaseBrush(ActiveTabBrush);
	ReleaseBrush(HomeInactiveTabBrush);
	ReleaseBrush(HomeActiveTabBrush);
	ReleaseBrush(HomeIconBrush);
	ReleaseBrush(NavSeparatorBrush);
	ReleaseBrush(SettingsSlotBrush);
	ReleaseBrush(UtilitySlotBrush);
	ReleaseBrush(QuitSlotBrush);
	ReleaseBrush(SettingsIconBrush);
	ReleaseBrush(LanguageIconBrush);
	ReleaseBrush(AchievementCoinsIconBrush);
	ReleaseBrush(PowerCouponsIconBrush);
	ReleaseBrush(QuitIconBrush);
	ReleaseBrush(QuitGlowBrush);
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
	const FVector2D ViewportLogicalSize = FT66Style::GetViewportLogicalSize();
	const float LayoutWidthAlpha = FMath::Clamp(
		(ViewportLogicalSize.X - FT66Style::Tokens::ReferenceLayoutWidth) / 640.f,
		0.f,
		1.f);
	auto LerpDimension = [LayoutWidthAlpha](float CompactValue, float FullValue) -> float
	{
		return FMath::Lerp(CompactValue, FullValue, LayoutWidthAlpha);
	};
	auto LerpInt = [LayoutWidthAlpha](int32 CompactValue, int32 FullValue) -> int32
	{
		return FMath::RoundToInt(FMath::Lerp(static_cast<float>(CompactValue), static_cast<float>(FullValue), LayoutWidthAlpha));
	};

	const FButtonStyle& FlatButtonStyle = FT66Style::Get().GetWidgetStyle<FButtonStyle>(TEXT("T66.Button.FlatTransparent"));
	const float TopBarButtonWidth = LerpDimension(78.f, 88.f);
	const float TopBarButtonHeight = LerpDimension(76.f, 82.f);
	const float TopBarSurfaceHeight = GTopBarSurfaceHeight;
	const float InactiveNavHeight = LerpDimension(64.f, GTopBarInactiveNavHeight);
	const float ActiveNavHeight = LerpDimension(70.f, GTopBarActiveNavHeight);
	const float InactiveNavMinWidth = LerpDimension(144.f, GTopBarInactiveNavMinWidth);
	const float ActiveNavMinWidth = LerpDimension(156.f, GTopBarActiveNavMinWidth);
	const float UtilityClusterGap = LerpDimension(12.f, 14.f);
	const float ClusterGap = LerpDimension(10.f, 14.f);
	const float NavButtonGap = LerpDimension(6.f, 8.f);
	const float RowHorizontalPadding = LerpDimension(10.f, 14.f);
	const float RowTopPadding = LerpDimension(4.f, 6.f);
	const FVector2D UtilitySlotSize(LerpDimension(70.f, 76.f), LerpDimension(70.f, 76.f));
	const FVector2D SettingsIconSize(LerpDimension(46.f, 50.f), LerpDimension(46.f, 50.f));
	const FVector2D LanguageIconSize(LerpDimension(48.f, 52.f), LerpDimension(48.f, 52.f));
	const FVector2D PowerCouponIconSize(LerpDimension(76.f, 88.f), LerpDimension(46.f, 54.f));
	const FVector2D QuitIconSize(LerpDimension(32.f, 38.f), LerpDimension(32.f, 38.f));
	const FVector2D QuitGlowSize(LerpDimension(56.f, 62.f), LerpDimension(56.f, 62.f));
	const FVector2D HomeLogoIconSize(LerpDimension(78.f, 88.f), LerpDimension(78.f, 88.f));
	const float HomeNavButtonSize = LerpDimension(92.f, 100.f);
	const float CouponButtonWidth = LerpDimension(148.f, 164.f);
	const float CouponButtonHeight = UtilitySlotSize.Y;
	const int32 TopBarTitleFontSize = LerpInt(13, 15);
	const float AccountTabWidth = LerpDimension(158.f, 172.f);
	const float PowerUpTabWidth = LerpDimension(170.f, 186.f);
	const float AchievementsTabWidth = LerpDimension(194.f, 208.f);
	const float MiniGamesTabWidth = LerpDimension(176.f, 188.f);
	const FLinearColor TopBarSurfaceColor(0.15f, 0.20f, 0.27f, 1.0f);
	const FLinearColor NavBorderColor(0.56f, 0.60f, 0.67f, 0.70f);
	const FLinearColor NavBorderActiveColor(0.86f, 0.90f, 0.78f, 0.95f);
	const FLinearColor NavInnerBorderColor(0.07f, 0.09f, 0.12f, 1.0f);
	const FLinearColor NavInnerBorderActiveColor(0.21f, 0.26f, 0.18f, 1.0f);
	const FLinearColor NavFillColor(0.21f, 0.24f, 0.31f, 0.99f);
	const FLinearColor NavFillActiveColor(0.40f, 0.46f, 0.38f, 0.99f);
	const FLinearColor CouponBorderColor(0.81f, 0.64f, 0.24f, 0.98f);
	const FLinearColor CouponInnerBorderColor(0.16f, 0.04f, 0.04f, 1.0f);
	const FLinearColor CouponFillColor(0.27f, 0.08f, 0.09f, 0.98f);
	const FLinearColor CouponTextColor(0.98f, 0.95f, 0.87f, 1.0f);
	const FLinearColor CouponTextShadowColor(0.12f, 0.03f, 0.03f, 0.92f);

	FSlateFontInfo NavFont = FT66Style::MakeFont(TEXT("Bold"), TopBarTitleFontSize);
	NavFont.LetterSpacing = LerpInt(0, 2);

	FSlateFontInfo ActiveNavFont = FT66Style::MakeFont(TEXT("Bold"), TopBarTitleFontSize + 1);
	ActiveNavFont.LetterSpacing = LerpInt(0, 3);

	FSlateFontInfo BalanceFont = FT66Style::MakeFont(TEXT("Bold"), LerpInt(14, 18));
	BalanceFont.LetterSpacing = LerpInt(0, 2);

	auto MakeFallbackIconLabel = [TopBarTitleFontSize](const FText& FallbackText) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(FallbackText)
			.Font(FT66Style::Tokens::FontBold(TopBarTitleFontSize))
			.ColorAndOpacity(FLinearColor(0.92f, 0.90f, 0.84f, 1.0f));
	};

	auto MakeBrushIconWidget = [&MakeFallbackIconLabel](const TSharedPtr<FSlateBrush>& IconBrush, const FVector2D& Size, const FText& FallbackText) -> TSharedRef<SWidget>
	{
		if (!IconBrush.IsValid())
		{
			return MakeFallbackIconLabel(FallbackText);
		}

		return SNew(SBox)
			.WidthOverride(Size.X)
			.HeightOverride(Size.Y)
			[
				SNew(SImage)
				.Image(IconBrush.Get())
				.ColorAndOpacity(FLinearColor::White)
			];
	};

	auto MakeOffsetWidget = [](const TSharedRef<SWidget>& Widget, const FVector2D& Offset) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.RenderTransform(FSlateRenderTransform(Offset))
			[
				Widget
			];
	};

	auto MakeNavLabel = [&NavFont, &ActiveNavFont](const FText& Text, bool bActive) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(bActive ? ActiveNavFont : NavFont)
			.ColorAndOpacity(bActive ? FT66Style::Text() : FLinearColor(0.80f, 0.82f, 0.84f, 0.94f))
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds)
			.Justification(ETextJustify::Center);
	};

	auto MakeNavContent =
		[&MakeBrushIconWidget, &MakeNavLabel](
			const FText& Text,
			bool bActive,
			const TSharedPtr<FSlateBrush>& IconBrush,
			const FVector2D& IconSize,
			const FText& IconFallbackText,
			bool bIconOnly,
			float WidthBudget) -> TSharedRef<SWidget>
	{
		if (bIconOnly)
		{
			return MakeBrushIconWidget(IconBrush, IconSize, IconFallbackText);
		}

		const float MaxContentWidth = FMath::Max(1.f, WidthBudget);

		if (!IconBrush.IsValid())
		{
			return SNew(SBox)
				.MaxDesiredWidth(MaxContentWidth)
				[
					MakeNavLabel(Text, bActive)
				];
		}

		return SNew(SBox)
			.MaxDesiredWidth(MaxContentWidth)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.f, 0.f, 6.f, 0.f)
				[
					MakeBrushIconWidget(IconBrush, IconSize, IconFallbackText)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				[
					MakeNavLabel(Text, bActive)
				]
			];
	};

	auto MakeNavButton =
		[this, &FlatButtonStyle, &MakeNavContent, ActiveNavMinWidth = ActiveNavMinWidth, ActiveNavHeight = ActiveNavHeight, InactiveNavMinWidth = InactiveNavMinWidth, InactiveNavHeight = InactiveNavHeight, NavBorderColor, NavBorderActiveColor, NavInnerBorderColor, NavInnerBorderActiveColor, NavFillColor, NavFillActiveColor](
			const FText& Text,
			FReply (UT66FrontendTopBarWidget::*ClickFunc)(),
			bool bActive,
			const TSharedPtr<FSlateBrush>& IconBrush,
			const FVector2D& IconSize,
			const FText& IconFallbackText,
			bool bIconOnly = false,
			float CustomButtonWidth = 0.f,
			float CustomButtonHeight = 0.f) -> TSharedRef<SWidget>
	{
		const float ButtonWidth = (CustomButtonWidth > 0.f)
			? CustomButtonWidth
			: (bIconOnly
				? GTopBarIconOnlyNavWidth
				: (bActive ? ActiveNavMinWidth : InactiveNavMinWidth));
		const float ButtonHeight = CustomButtonHeight > 0.f
			? CustomButtonHeight
			: (bActive ? ActiveNavHeight : InactiveNavHeight);
		const float HorizontalContentPadding = bIconOnly ? 9.f : 12.f;
		const float ContentWidthBudget = ButtonWidth - (HorizontalContentPadding * 2.f);
		return SNew(SBox)
			.WidthOverride(ButtonWidth)
			.HeightOverride(ButtonHeight)
			[
				SNew(SButton)
				.ButtonStyle(&FlatButtonStyle)
				.OnClicked(FOnClicked::CreateUObject(this, ClickFunc))
				.ContentPadding(FMargin(0.f))
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(bActive ? NavBorderActiveColor : NavBorderColor)
					.Padding(1.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(bActive ? NavInnerBorderActiveColor : NavInnerBorderColor)
						.Padding(1.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(bActive ? NavFillActiveColor : NavFillColor)
							.Padding(bIconOnly ? FMargin(8.f) : FMargin(HorizontalContentPadding, 8.f, HorizontalContentPadding, 8.f))
							[
								SNew(SBox)
								.Clipping(EWidgetClipping::ClipToBounds)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									MakeNavContent(Text, bActive, IconBrush, IconSize, IconFallbackText, bIconOnly, ContentWidthBudget)
								]
							]
						]
					]
				]
			];
	};

	auto MakeUtilityButton =
		[this, &FlatButtonStyle, &MakeOffsetWidget, TopBarButtonWidth, TopBarButtonHeight](
			const FText& TooltipText,
			const TSharedRef<SWidget>& ContentWidget,
			FReply (UT66FrontendTopBarWidget::*ClickFunc)(),
			const TSharedPtr<FSlateBrush>& SlotBrush = TSharedPtr<FSlateBrush>(),
			const FVector2D& SlotSize = FVector2D::ZeroVector,
			const FVector2D& ContentOffset = FVector2D::ZeroVector) -> TSharedRef<SWidget>
	{
		const TSharedRef<SWidget> DisplayContent =
			ContentOffset.IsNearlyZero()
			? ContentWidget
			: MakeOffsetWidget(ContentWidget, ContentOffset);

		return SNew(SBox)
			.WidthOverride(TopBarButtonWidth)
			.HeightOverride(TopBarButtonHeight)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(SlotSize.X)
					.HeightOverride(SlotSize.Y)
					.Visibility(SlotBrush.IsValid() ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
					[
						SNew(SImage)
						.Image(SlotBrush.Get())
						.ColorAndOpacity(FLinearColor::White)
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SButton)
					.ButtonStyle(&FlatButtonStyle)
					.ToolTipText(TooltipText)
					.OnClicked(FOnClicked::CreateUObject(this, ClickFunc))
					.ContentPadding(FMargin(0.f))
					[
						SNew(SBox)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							DisplayContent
						]
					]
				]
			];
	};

	auto MakeBalanceNavButton =
		[this, &FlatButtonStyle, &BalanceFont, &MakeBrushIconWidget, PowerCouponIconSize, CouponButtonWidth, CouponButtonHeight, CouponBorderColor, CouponInnerBorderColor, CouponFillColor, CouponTextColor, CouponTextShadowColor](
			const FText& TooltipText,
			FReply (UT66FrontendTopBarWidget::*ClickFunc)(),
			FText (UT66FrontendTopBarWidget::*ValueGetter)() const) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(CouponButtonWidth)
			.HeightOverride(CouponButtonHeight)
			[
				SNew(SButton)
				.ButtonStyle(&FlatButtonStyle)
				.ToolTipText(TooltipText)
				.OnClicked(FOnClicked::CreateUObject(this, ClickFunc))
				.ContentPadding(FMargin(0.f))
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(CouponBorderColor)
					.Padding(1.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(CouponInnerBorderColor)
						.Padding(1.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(CouponFillColor)
							.Padding(FMargin(10.f, 6.f, 12.f, 6.f))
							[
								SNew(SBox)
								.Clipping(EWidgetClipping::ClipToBounds)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.VAlign(VAlign_Center)
									[
										MakeBrushIconWidget(
											PowerCouponsIconBrush,
											PowerCouponIconSize,
											NSLOCTEXT("T66.PowerUp", "PowerCouponsIconFallback", "PC"))
									]
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.VAlign(VAlign_Center)
									.Padding(10.f, 0.f, 0.f, 0.f)
									[
										SNew(SBox)
										.MinDesiredWidth(24.f)
										.VAlign(VAlign_Center)
										[
											SNew(STextBlock)
											.Text_Lambda([this, ValueGetter]() -> FText
											{
												return (this->*ValueGetter)();
											})
											.Font(BalanceFont)
											.ColorAndOpacity(CouponTextColor)
											.ShadowColorAndOpacity(CouponTextShadowColor)
											.ShadowOffset(FVector2D(1.f, 1.f))
										]
									]
								]
							]
						]
					]
				]
			];
	};

	const TSharedRef<SWidget> SettingsIconWidget = MakeBrushIconWidget(SettingsIconBrush, SettingsIconSize, NSLOCTEXT("T66.Settings", "SettingsIconFallback", "S"));
	const TSharedRef<SWidget> LanguageIconWidget = MakeBrushIconWidget(LanguageIconBrush, LanguageIconSize, NSLOCTEXT("T66.LanguageSelect", "LanguageIconFallback", "L"));
	const TSharedRef<SWidget> SettingsButtonWidget =
		MakeUtilityButton(
			SettingsText,
			SettingsIconWidget,
			&UT66FrontendTopBarWidget::HandleSettingsClicked);
	const TSharedRef<SWidget> LanguageButtonWidget =
		MakeUtilityButton(
			LanguageText,
			LanguageIconWidget,
			&UT66FrontendTopBarWidget::HandleLanguageClicked);
	const TSharedRef<SWidget> ChadCouponsWidget =
		MakeBalanceNavButton(
			NSLOCTEXT("T66.Shop", "ChadCouponsBalanceTooltip", "Chad Coupons"),
			&UT66FrontendTopBarWidget::HandleShopClicked,
			&UT66FrontendTopBarWidget::GetChadCouponsValueText);
	const TSharedRef<SWidget> QuitIconWidget = MakeBrushIconWidget(QuitIconBrush, QuitIconSize, NSLOCTEXT("T66.MainMenu", "QuitIconFallback", "Q"));

	const TSharedRef<SWidget> LeftUtilityCluster =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SettingsButtonWidget
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(UtilityClusterGap, 0.f, 0.f, 0.f)
		[
			LanguageButtonWidget
		];

	const TSharedRef<SWidget> CenterNavCluster =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[
			MakeNavButton(AccountText, &UT66FrontendTopBarWidget::HandleAccountStatusClicked, ActiveSection == ETopBarSection::AccountStatus, TSharedPtr<FSlateBrush>(), FVector2D::ZeroVector, FText::GetEmpty(), false, AccountTabWidth)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(NavButtonGap, 0.f, 0.f, 0.f)
		[
			MakeNavButton(
				HomeText,
				&UT66FrontendTopBarWidget::HandleHomeClicked,
				ActiveSection == ETopBarSection::Home,
				HomeIconBrush,
				HomeLogoIconSize,
				NSLOCTEXT("T66.MainMenu", "HomeIconFallback", "C"),
				true,
				HomeNavButtonSize,
				HomeNavButtonSize)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(NavButtonGap, 0.f, 0.f, 0.f)
		[
			MakeNavButton(PowerUpText, &UT66FrontendTopBarWidget::HandleShopClicked, ActiveSection == ETopBarSection::PowerUp, TSharedPtr<FSlateBrush>(), FVector2D::ZeroVector, FText::GetEmpty(), false, PowerUpTabWidth)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(NavButtonGap, 0.f, 0.f, 0.f)
		[
			MakeNavButton(AchievementsText, &UT66FrontendTopBarWidget::HandleAchievementsClicked, ActiveSection == ETopBarSection::Achievements, TSharedPtr<FSlateBrush>(), FVector2D::ZeroVector, FText::GetEmpty(), false, AchievementsTabWidth)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(NavButtonGap, 0.f, 0.f, 0.f)
		[
			MakeNavButton(MiniGamesText, &UT66FrontendTopBarWidget::HandleMiniGamesClicked, ActiveSection == ETopBarSection::MiniGames, TSharedPtr<FSlateBrush>(), FVector2D::ZeroVector, FText::GetEmpty(), false, MiniGamesTabWidth)
		];

	const TSharedRef<SWidget> RightUtilityCluster =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[
			ChadCouponsWidget
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(UtilityClusterGap, 0.f, 0.f, 0.f)
		[
			MakeUtilityButton(QuitTooltipText, QuitIconWidget, &UT66FrontendTopBarWidget::HandleQuitClicked)
		];

	const TSharedRef<SWidget> TopBarRow =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			LeftUtilityCluster
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(ClusterGap, 0.f)
		[
			CenterNavCluster
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		[
			RightUtilityCluster
		];

	const TSharedRef<SWidget> Surface =
		SNew(SBox)
		.HeightOverride(TopBarSurfaceHeight)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(TopBarSurfaceColor)
			.Padding(FMargin(RowHorizontalPadding, RowTopPadding, RowHorizontalPadding, 0.f))
			[
				TopBarRow
			]
		];

	return SNew(SVerticalBox)
		.Visibility(EVisibility::SelfHitTestInvisible)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(GetReservedHeight())
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Top)
				[
					Surface
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
