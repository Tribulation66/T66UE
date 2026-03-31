// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66FrontendTopBarWidget.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PowerUpSubsystem.h"
#include "UI/ST66PulsingIcon.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"

#include "Engine/Texture2D.h"
#include "GameFramework/GameUserSettings.h"
#include "HAL/FileManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "ImageUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "TextureResource.h"
#include "UObject/StrongObjectPtr.h"
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
	constexpr float GTopBarButtonWidth = 92.f;
	constexpr float GTopBarButtonHeight = 78.f;
	constexpr float GTopBarSurfaceHeight = 118.f;
	constexpr float GTopBarPlateVisibleHeightRatio = 98.f / 108.f;
	constexpr float GTopBarInactiveNavHeight = 72.f;
	constexpr float GTopBarActiveNavHeight = 80.f;
	constexpr float GTopBarInactiveNavMinWidth = 176.f;
	constexpr float GTopBarActiveNavMinWidth = 188.f;
	constexpr float GTopBarIconOnlyNavWidth = 80.f;
	constexpr float GTopBarNavSeparatorHorizontalPadding = 10.f;
	constexpr float GTopBarRightClusterGap = 12.f;
	constexpr int32 GTopBarViewportZOrder = 50;
	TMap<FString, TStrongObjectPtr<UTexture2D>> GTopBarFileTextureCache;

	bool DecodeImageFileToBGRA(const FString& FilePath, TArray<FColor>& OutPixels, int32& OutWidth, int32& OutHeight)
	{
		OutPixels.Reset();
		OutWidth = 0;
		OutHeight = 0;

		TArray64<uint8> CompressedBytes;
		if (!FFileHelper::LoadFileToArray(CompressedBytes, *FilePath) || CompressedBytes.Num() == 0)
		{
			return false;
		}

		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
		const EImageFormat Format = ImageWrapperModule.DetectImageFormat(CompressedBytes.GetData(), CompressedBytes.Num());
		if (Format == EImageFormat::Invalid)
		{
			return false;
		}

		const TSharedPtr<IImageWrapper> Wrapper = ImageWrapperModule.CreateImageWrapper(Format);
		if (!Wrapper.IsValid() || !Wrapper->SetCompressed(CompressedBytes.GetData(), CompressedBytes.Num()))
		{
			return false;
		}

		TArray64<uint8> RawBytes;
		if (!Wrapper->GetRaw(ERGBFormat::BGRA, 8, RawBytes))
		{
			return false;
		}

		OutWidth = Wrapper->GetWidth();
		OutHeight = Wrapper->GetHeight();
		if (OutWidth <= 0 || OutHeight <= 0)
		{
			return false;
		}

		const int64 PixelCount = static_cast<int64>(OutWidth) * static_cast<int64>(OutHeight);
		if (RawBytes.Num() != PixelCount * static_cast<int64>(sizeof(FColor)))
		{
			return false;
		}

		OutPixels.SetNumUninitialized(static_cast<int32>(PixelCount));
		FMemory::Memcpy(OutPixels.GetData(), RawBytes.GetData(), RawBytes.Num());
		return true;
	}

	void CopyPixelsIntoMip(FTexture2DMipMap& Mip, const TArray<FColor>& Pixels)
	{
		const int64 ExpectedBytes = static_cast<int64>(Pixels.Num()) * static_cast<int64>(sizeof(FColor));
		void* DestPixels = Mip.BulkData.Lock(LOCK_READ_WRITE);
		DestPixels = Mip.BulkData.Realloc(ExpectedBytes);
		FMemory::Memcpy(DestPixels, Pixels.GetData(), ExpectedBytes);
		Mip.BulkData.Unlock();
	}

	UTexture2D* CreateTopBarTextureWithMips(const FString& SourcePath, const TArray<FColor>& BasePixels, int32 Width, int32 Height, TextureFilter Filter)
	{
		if (BasePixels.Num() != Width * Height || Width <= 0 || Height <= 0)
		{
			return nullptr;
		}

		const FName TextureName = MakeUniqueObjectName(
			GetTransientPackage(),
			UTexture2D::StaticClass(),
			FName(*FPaths::GetBaseFilename(SourcePath)));

		UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8, TextureName);
		if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
		{
			return nullptr;
		}

		Texture->bNotOfflineProcessed = true;
		Texture->SRGB = true;
		Texture->Filter = Filter;
		Texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
		Texture->CompressionSettings = TC_EditorIcon;
		Texture->NeverStream = true;
		Texture->AddressX = TextureAddress::TA_Clamp;
		Texture->AddressY = TextureAddress::TA_Clamp;

		CopyPixelsIntoMip(Texture->GetPlatformData()->Mips[0], BasePixels);

		TArray<FColor> SourcePixels = BasePixels;
		int32 SourceWidth = Width;
		int32 SourceHeight = Height;

		while (SourceWidth > 1 || SourceHeight > 1)
		{
			const int32 NextWidth = FMath::Max(1, SourceWidth / 2);
			const int32 NextHeight = FMath::Max(1, SourceHeight / 2);

			TArray<FColor> NextPixels;
			FImageUtils::ImageResize(
				SourceWidth,
				SourceHeight,
				SourcePixels,
				NextWidth,
				NextHeight,
				NextPixels,
				true,
				false);

			FTexture2DMipMap* NextMip = new FTexture2DMipMap(NextWidth, NextHeight, 1);
			Texture->GetPlatformData()->Mips.Add(NextMip);
			CopyPixelsIntoMip(*NextMip, NextPixels);

			SourcePixels = MoveTemp(NextPixels);
			SourceWidth = NextWidth;
			SourceHeight = NextHeight;
		}

		Texture->UpdateResource();
		return Texture;
	}

	UTexture2D* LoadTopBarAssetTexture(const TCHAR* AssetPath, TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		if (!AssetPath || !*AssetPath)
		{
			return nullptr;
		}

		if (UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, AssetPath))
		{
			Texture->SRGB = true;
			Texture->Filter = Filter;
			Texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
			Texture->CompressionSettings = TC_EditorIcon;
			Texture->NeverStream = true;
			Texture->UpdateResource();
			return Texture;
		}

		return nullptr;
	}

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
		FVector2D EffectiveSize = FT66Style::GetViewportSize();

		if (GEngine)
		{
			if (UGameUserSettings* GameUserSettings = GEngine->GetGameUserSettings())
			{
				const FIntPoint ScreenResolution = GameUserSettings->GetScreenResolution();
				if (ScreenResolution.X > 0 && ScreenResolution.Y > 0
					&& GameUserSettings->GetFullscreenMode() != EWindowMode::Windowed)
				{
					EffectiveSize.X = FMath::Max(EffectiveSize.X, static_cast<float>(ScreenResolution.X));
					EffectiveSize.Y = FMath::Max(EffectiveSize.Y, static_cast<float>(ScreenResolution.Y));
				}
			}
		}

		return EffectiveSize;
	}

	float GetTopBarLayoutScale()
	{
		const FVector2D ReferenceViewportSize(1920.f, 1080.f);
		const FVector2D EffectiveViewportSize = GetEffectiveFrontendViewportSize();
		const float WidthScale = EffectiveViewportSize.X / FMath::Max(ReferenceViewportSize.X, 1.f);
		const float HeightScale = EffectiveViewportSize.Y / FMath::Max(ReferenceViewportSize.Y, 1.f);
		const float Scale = FMath::Clamp(FMath::Sqrt(WidthScale * HeightScale), 0.70f, 3.0f);
		if (EffectiveViewportSize.X >= 1600.f && EffectiveViewportSize.Y >= 900.f && Scale <= 1.f)
		{
			return 1.f;
		}

		return Scale;
	}

	UTexture2D* LoadTopBarFileTexture(const FString& FilePath, TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		const FDateTime FileTimestamp = IFileManager::Get().GetTimeStamp(*FilePath);
		const FString CacheKey = FString::Printf(TEXT("%s|%d|%lld"), *FilePath, static_cast<int32>(Filter), FileTimestamp.GetTicks());
		if (const TStrongObjectPtr<UTexture2D>* CachedTexture = GTopBarFileTextureCache.Find(CacheKey))
		{
			return CachedTexture->Get();
		}

		if (!FPaths::FileExists(FilePath))
		{
			return nullptr;
		}

		TArray<FColor> DecodedPixels;
		int32 Width = 0;
		int32 Height = 0;
		UTexture2D* Texture = nullptr;
		if (DecodeImageFileToBGRA(FilePath, DecodedPixels, Width, Height))
		{
			Texture = CreateTopBarTextureWithMips(FilePath, DecodedPixels, Width, Height, Filter);
		}

		if (!Texture)
		{
			Texture = FImageUtils::ImportFileAsTexture2D(FilePath);
		}

		if (!Texture)
		{
			return nullptr;
		}

		Texture->SRGB = true;
		Texture->Filter = Filter;
		Texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
		Texture->CompressionSettings = TC_EditorIcon;
		Texture->NeverStream = true;
		Texture->UpdateResource();

		GTopBarFileTextureCache.Add(CacheKey, TStrongObjectPtr<UTexture2D>(Texture));
		return Texture;
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
		const FString FullPath = FPaths::ProjectDir() / RelativePath;
		const bool bFileExists = FPaths::FileExists(FullPath);
		const FString AssetPath = ImportedAssetPath ? FString(ImportedAssetPath) : FString(TEXT("<null>"));
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[TopBarBrushLoad] Begin RelativePath='%s' FullPath='%s' FileExists=%s AssetPath='%s' DesiredSize=(%.1f, %.1f)"),
			*RelativePath,
			*FullPath,
			bFileExists ? TEXT("true") : TEXT("false"),
			*AssetPath,
			DesiredSize.X,
			DesiredSize.Y);

		if (bFileExists)
		{
			Texture = LoadTopBarFileTexture(FullPath, Filter);
		}

		FString LoadedFrom = TEXT("none");
		if (Texture)
		{
			LoadedFrom = TEXT("file");
		}

		if (!Texture)
		{
			Texture = LoadTopBarAssetTexture(ImportedAssetPath, Filter);
			if (Texture)
			{
				LoadedFrom = TEXT("asset");
			}
		}

		UE_LOG(
			LogTemp,
			Log,
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
	return GTopBarReservedHeight * GetTopBarLayoutScale();
}

float UT66FrontendTopBarWidget::GetVisibleContentHeight()
{
	return (GTopBarSurfaceHeight * GTopBarPlateVisibleHeightRatio) * GetTopBarLayoutScale();
}

TSharedRef<SWidget> UT66FrontendTopBarWidget::RebuildWidget()
{
	// The top bar computes its own resolution-aware scale and reserved height.
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
	case ET66ScreenType::MainMenu:
	default:
		return ETopBarSection::Home;
	}
}

FText UT66FrontendTopBarWidget::GetAchievementCoinsValueText() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66AchievementsSubsystem* Achievements = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			return FText::AsNumber(Achievements->GetAchievementCoinsBalance());
		}
	}

	return FText::AsNumber(0);
}

FText UT66FrontendTopBarWidget::GetPowerCouponsValueText() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66PowerUpSubsystem* PowerUp = GI->GetSubsystem<UT66PowerUpSubsystem>())
		{
			return FText::AsNumber(PowerUp->GetPowerCrystalBalance());
		}
	}

	return FText::AsNumber(0);
}

void UT66FrontendTopBarWidget::RequestTopBarAssets()
{
#if WITH_EDITOR
	TopBarPlateBrush.Reset();
	InactiveTabBrush.Reset();
	ActiveTabBrush.Reset();
	HomeInactiveTabBrush.Reset();
	HomeActiveTabBrush.Reset();
	NavSeparatorBrush.Reset();
	SettingsSlotBrush.Reset();
	UtilitySlotBrush.Reset();
	QuitSlotBrush.Reset();
	SettingsIconBrush.Reset();
	LanguageIconBrush.Reset();
	AchievementCoinsIconBrush.Reset();
	PowerCouponsIconBrush.Reset();
	QuitIconBrush.Reset();
	QuitGlowBrush.Reset();
	GTopBarFileTextureCache.Empty();
#endif

	LoadBrushFromRelativePath(nullptr, TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_plate.png"), TopBarPlateBrush);
	LoadBrushFromRelativePath(nullptr, TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_nav_tab.png"), InactiveTabBrush);
	LoadBrushFromRelativePath(nullptr, TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_active_tab.png"), ActiveTabBrush);
	LoadBrushFromRelativePath(nullptr, TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_home_tab.png"), HomeInactiveTabBrush);
	LoadBrushFromRelativePath(nullptr, TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_home_active_tab.png"), HomeActiveTabBrush);
	LoadBrushFromRelativePath(nullptr, TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_separator.png"), NavSeparatorBrush);
	LoadBrushFromRelativePath(nullptr, TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_settings_slot.png"), SettingsSlotBrush, FVector2D(78.f, 78.f));
	LoadBrushFromRelativePath(nullptr, TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_utility_slot.png"), UtilitySlotBrush, FVector2D(78.f, 78.f));
	LoadBrushFromRelativePath(nullptr, TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_quit_slot.png"), QuitSlotBrush, FVector2D(78.f, 78.f));
	LoadBrushFromRelativePath(TEXT("/Game/UI/Assets/TopBar/frontend_topbar_settings_icon.frontend_topbar_settings_icon"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_settings_icon.png"), SettingsIconBrush, FVector2D(44.f, 44.f), TextureFilter::TF_Trilinear);
	LoadBrushFromRelativePath(TEXT("/Game/UI/Assets/TopBar/frontend_topbar_language_icon.frontend_topbar_language_icon"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_language_icon.png"), LanguageIconBrush, FVector2D(48.f, 48.f), TextureFilter::TF_Trilinear);
	LoadBrushFromRelativePath(TEXT("/Game/UI/Assets/TopBar/frontend_topbar_achievement_coins_icon.frontend_topbar_achievement_coins_icon"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_achievement_coins_icon.png"), AchievementCoinsIconBrush, FVector2D(48.f, 48.f), TextureFilter::TF_Trilinear);
	LoadBrushFromRelativePath(TEXT("/Game/UI/Assets/TopBar/frontend_topbar_power_coupons_icon.frontend_topbar_power_coupons_icon"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_power_coupons_icon.png"), PowerCouponsIconBrush, FVector2D(58.f, 38.f), TextureFilter::TF_Trilinear);
	LoadBrushFromRelativePath(TEXT("/Game/UI/Assets/TopBar/frontend_topbar_quit_icon.frontend_topbar_quit_icon"), TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_quit_icon.png"), QuitIconBrush, FVector2D(44.f, 44.f), TextureFilter::TF_Trilinear);
	LoadBrushFromRelativePath(nullptr, TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_quit_glow.png"), QuitGlowBrush, FVector2D(62.f, 62.f));

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

void UT66FrontendTopBarWidget::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	FT66Style::DeferRebuild(this, GTopBarViewportZOrder);
}

TSharedRef<SWidget> UT66FrontendTopBarWidget::BuildSlateUI()
{
	CachedViewportSize = GetEffectiveFrontendViewportSize();
	bViewportResponsiveRebuildQueued = false;
	RequestTopBarAssets();

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66LeaderboardSubsystem* Leaderboard = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const ETopBarSection ActiveSection = GetActiveSection();
	const bool bAccountGood = Leaderboard ? Leaderboard->IsAccountEligibleForLeaderboard() : true;

	const FText SettingsText = Loc ? Loc->GetText_Settings() : NSLOCTEXT("T66.MainMenu", "Settings", "SETTINGS");
	const FText LanguageText = Loc ? Loc->GetText_LangButton() : NSLOCTEXT("T66.LanguageSelect", "LangButton", "LANG");
	const FText HomeText = NSLOCTEXT("T66.MainMenu", "Home", "CHADPOCALYPSE");
	const FText PowerUpText = NSLOCTEXT("T66.MainMenu", "PowerUp", "POWER UP");
	const FText AchievementsText = Loc ? Loc->GetText_Achievements() : NSLOCTEXT("T66.MainMenu", "Achievements", "ACHIEVEMENTS");
	const FText AccountStatusText = NSLOCTEXT("T66.AccountStatus", "TopBarTitle", "ACCOUNT STATUS");
	const FText QuitTooltipText = Loc ? Loc->GetText_Quit() : NSLOCTEXT("T66.MainMenu", "Quit", "QUIT");

	const FButtonStyle& FlatButtonStyle = FT66Style::Get().GetWidgetStyle<FButtonStyle>(TEXT("T66.Button.FlatTransparent"));
	const float LayoutScale = GetTopBarLayoutScale();
	const float TopBarButtonWidth = SnapPixel(GTopBarButtonWidth * LayoutScale);
	const float TopBarButtonHeight = SnapPixel(GTopBarButtonHeight * LayoutScale);
	const float TopBarSurfaceHeight = SnapPixel(GTopBarSurfaceHeight * LayoutScale);
	const float InactiveNavHeight = SnapPixel(GTopBarInactiveNavHeight * LayoutScale);
	const float ActiveNavHeight = SnapPixel(GTopBarActiveNavHeight * LayoutScale);
	const float InactiveNavMinWidth = SnapPixel(GTopBarInactiveNavMinWidth * LayoutScale);
	const float ActiveNavMinWidth = SnapPixel(GTopBarActiveNavMinWidth * LayoutScale);
	const float UtilityClusterGap = SnapPixel(12.f * LayoutScale);
	const float ClusterGap = SnapPixel(18.f * LayoutScale);
	const float NavButtonGap = SnapPixel(8.f * LayoutScale);
	const float ReadoutGap = SnapPixel(10.f * LayoutScale);
	const float QuitGap = SnapPixel(12.f * LayoutScale);
	const float RowHorizontalPadding = SnapPixel(18.f * LayoutScale);
	const float RowTopPadding = SnapPixel(8.f * LayoutScale);
	const FVector2D UtilitySlotSize = SnapPixelSize(FVector2D(78.f, 78.f) * LayoutScale);
	const FVector2D SettingsIconSize = SnapPixelSize(FVector2D(44.f, 44.f) * LayoutScale);
	const FVector2D LanguageIconSize = SnapPixelSize(FVector2D(48.f, 48.f) * LayoutScale);
	const FVector2D AchievementCoinIconSize = SnapPixelSize(FVector2D(48.f, 48.f) * LayoutScale);
	const FVector2D PowerCouponIconSize = SnapPixelSize(FVector2D(58.f, 38.f) * LayoutScale);
	const FVector2D QuitIconSize = SnapPixelSize(FVector2D(44.f, 44.f) * LayoutScale);
	const FVector2D QuitGlowSize = SnapPixelSize(FVector2D(70.f, 70.f) * LayoutScale);
	constexpr int32 TopBarTitleFontSize = 16;

	FSlateFontInfo NavFont = FT66Style::MakeFont(TEXT("Bold"), TopBarTitleFontSize);
	NavFont.LetterSpacing = 14;

	FSlateFontInfo ActiveNavFont = FT66Style::MakeFont(TEXT("Bold"), TopBarTitleFontSize + 1);
	ActiveNavFont.LetterSpacing = 16;

	FSlateFontInfo BalanceFont = FT66Style::MakeFont(TEXT("Bold"), TopBarTitleFontSize);
	BalanceFont.LetterSpacing = 8;

	auto MakeFallbackIconLabel = [](const FText& FallbackText) -> TSharedRef<SWidget>
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
			.Justification(ETextJustify::Center);
	};

	auto MakeNavContent =
		[&MakeBrushIconWidget, &MakeNavLabel](
			const FText& Text,
			bool bActive,
			const TSharedPtr<FSlateBrush>& IconBrush,
			const FVector2D& IconSize,
			const FText& IconFallbackText,
			bool bIconOnly) -> TSharedRef<SWidget>
	{
		if (bIconOnly)
		{
			return MakeBrushIconWidget(IconBrush, IconSize, IconFallbackText);
		}

		if (!IconBrush.IsValid())
		{
			return MakeNavLabel(Text, bActive);
		}

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 10.f, 0.f)
			[
				MakeBrushIconWidget(IconBrush, IconSize, IconFallbackText)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				MakeNavLabel(Text, bActive)
			];
	};

	auto MakeNavButton =
		[this, &FlatButtonStyle, &MakeNavContent, LayoutScale, TopBarButtonWidth = TopBarButtonWidth, LanguageIconSize = LanguageIconSize, ActiveNavMinWidth = ActiveNavMinWidth, ActiveNavHeight = ActiveNavHeight, InactiveNavMinWidth = InactiveNavMinWidth, InactiveNavHeight = InactiveNavHeight](
			const FText& Text,
			FReply (UT66FrontendTopBarWidget::*ClickFunc)(),
			bool bActive,
			const TSharedPtr<FSlateBrush>& IconBrush,
			const FVector2D& IconSize,
			const FText& IconFallbackText,
			const FLinearColor& BackgroundTint = FLinearColor::White,
			bool bIconOnly = false,
			const FSlateBrush* InactiveBackgroundBrush = nullptr,
			const FSlateBrush* ActiveBackgroundBrush = nullptr) -> TSharedRef<SWidget>
	{
		const float IconOnlyNavWidth = FMath::Max(TopBarButtonWidth, LanguageIconSize.X + (28.f * LayoutScale));
		const FSlateBrush* BackgroundBrush = nullptr;
		if (bActive)
		{
			BackgroundBrush = ActiveBackgroundBrush
				? ActiveBackgroundBrush
				: (ActiveTabBrush.IsValid() ? ActiveTabBrush.Get() : InactiveTabBrush.Get());
		}
		else
		{
			BackgroundBrush = InactiveBackgroundBrush
				? InactiveBackgroundBrush
				: (InactiveTabBrush.IsValid() ? InactiveTabBrush.Get() : ActiveTabBrush.Get());
		}
		const float ButtonMinWidth = bActive ? ActiveNavMinWidth : InactiveNavMinWidth;
		const float ButtonHeight = bActive ? ActiveNavHeight : InactiveNavHeight;
		return SNew(SBox)
			.MinDesiredWidth(bIconOnly ? IconOnlyNavWidth : ButtonMinWidth)
			.HeightOverride(ButtonHeight)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SImage)
					.Image(BackgroundBrush)
					.ColorAndOpacity(BackgroundTint)
					.Visibility(BackgroundBrush ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SButton)
					.ButtonStyle(&FlatButtonStyle)
					.OnClicked(FOnClicked::CreateUObject(this, ClickFunc))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(bIconOnly ? FMargin(18.f * LayoutScale) : FMargin(24.f * LayoutScale, 12.f * LayoutScale, 24.f * LayoutScale, 12.f * LayoutScale))
					[
						MakeNavContent(Text, bActive, IconBrush, IconSize, IconFallbackText, bIconOnly)
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

	auto MakeBalanceReadout =
		[this, &BalanceFont, LayoutScale, InactiveNavHeight = InactiveNavHeight](
			const TSharedRef<SWidget>& IconWidget,
			const FText& TooltipText,
			FText (UT66FrontendTopBarWidget::*ValueGetter)() const) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.HeightOverride(InactiveNavHeight)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.ToolTipText(TooltipText)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					IconWidget
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(8.f * LayoutScale, 0.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text_Lambda([this, ValueGetter]() -> FText
					{
						return (this->*ValueGetter)();
					})
					.Font(BalanceFont)
					.ColorAndOpacity(FT66Style::Text())
				]
			];
	};

	const TSharedRef<SWidget> SettingsIconWidget = MakeBrushIconWidget(SettingsIconBrush, SettingsIconSize, NSLOCTEXT("T66.Settings", "SettingsIconFallback", "S"));
	const TSharedRef<SWidget> LanguageIconWidget = MakeBrushIconWidget(LanguageIconBrush, LanguageIconSize, NSLOCTEXT("T66.LanguageSelect", "LanguageIconFallback", "L"));
	const TSharedRef<SWidget> AchievementCoinsIconWidget = MakeBrushIconWidget(AchievementCoinsIconBrush, AchievementCoinIconSize, NSLOCTEXT("T66.Achievements", "AchievementCoinsIconFallback", "AC"));
	const TSharedRef<SWidget> PowerCouponsIconWidget = MakeBrushIconWidget(PowerCouponsIconBrush, PowerCouponIconSize, NSLOCTEXT("T66.PowerUp", "PowerCouponsIconFallback", "PC"));
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
	const TSharedRef<SWidget> AchievementCoinsWidget =
		MakeBalanceReadout(
			AchievementCoinsIconWidget,
			NSLOCTEXT("T66.Achievements", "AchievementCoinsBalanceTooltip", "Achievement Coins"),
			&UT66FrontendTopBarWidget::GetAchievementCoinsValueText);
	const TSharedRef<SWidget> PowerCouponsWidget =
		MakeBalanceReadout(
			PowerCouponsIconWidget,
			NSLOCTEXT("T66.PowerUp", "PowerCouponsBalanceTooltip", "Power Coupons"),
			&UT66FrontendTopBarWidget::GetPowerCouponsValueText);
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
			MakeNavButton(
				AccountStatusText,
				&UT66FrontendTopBarWidget::HandleAccountStatusClicked,
				ActiveSection == ETopBarSection::AccountStatus,
				TSharedPtr<FSlateBrush>(),
				FVector2D::ZeroVector,
				FText::GetEmpty(),
				bAccountGood
					? (ActiveSection == ETopBarSection::AccountStatus
						? FLinearColor(0.84f, 0.94f, 0.72f, 1.0f)
						: FLinearColor(0.76f, 0.89f, 0.66f, 1.0f))
					: FLinearColor::White,
				false)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(NavButtonGap, 0.f, 0.f, 0.f)
		[
			MakeNavButton(
				HomeText,
				&UT66FrontendTopBarWidget::HandleHomeClicked,
				ActiveSection == ETopBarSection::Home,
				TSharedPtr<FSlateBrush>(),
				FVector2D::ZeroVector,
				FText::GetEmpty(),
				FLinearColor::White,
				false,
				HomeInactiveTabBrush.Get(),
				HomeActiveTabBrush.Get())
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(NavButtonGap, 0.f, 0.f, 0.f)
		[
			MakeNavButton(PowerUpText, &UT66FrontendTopBarWidget::HandlePowerUpClicked, ActiveSection == ETopBarSection::PowerUp, TSharedPtr<FSlateBrush>(), FVector2D::ZeroVector, FText::GetEmpty(), FLinearColor::White, false)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(NavButtonGap, 0.f, 0.f, 0.f)
		[
			MakeNavButton(AchievementsText, &UT66FrontendTopBarWidget::HandleAchievementsClicked, ActiveSection == ETopBarSection::Achievements, TSharedPtr<FSlateBrush>(), FVector2D::ZeroVector, FText::GetEmpty(), FLinearColor::White, false)
		];

	const TSharedRef<SWidget> RightUtilityCluster =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[
			PowerCouponsWidget
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(ReadoutGap, 0.f, 0.f, 0.f)
		[
			AchievementCoinsWidget
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(QuitGap, 0.f, 0.f, 0.f)
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
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			[
				SNew(SImage)
				.Image(TopBarPlateBrush.Get())
				.ColorAndOpacity(FLinearColor::White)
				.Visibility(TopBarPlateBrush.IsValid() ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			.Padding(RowHorizontalPadding, RowTopPadding, RowHorizontalPadding, 0.f)
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

	if (bViewportResponsiveRebuildQueued)
	{
		return;
	}

	const FVector2D CurrentViewportSize = GetEffectiveFrontendViewportSize();
	if (!CurrentViewportSize.Equals(CachedViewportSize, 1.0f))
	{
		CachedViewportSize = CurrentViewportSize;
		bViewportResponsiveRebuildQueued = true;
		ForceRebuildSlate();
	}
}

FReply UT66FrontendTopBarWidget::HandleSettingsClicked()
{
	NavigateTo(ET66ScreenType::Settings);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleLanguageClicked()
{
	NavigateTo(ET66ScreenType::LanguageSelect);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleHomeClicked()
{
	NavigateTo(ET66ScreenType::MainMenu);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandlePowerUpClicked()
{
	NavigateTo(ET66ScreenType::PowerUp);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleUnlocksClicked()
{
	NavigateTo(ET66ScreenType::Unlocks);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleAchievementsClicked()
{
	NavigateTo(ET66ScreenType::Achievements);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleAccountStatusClicked()
{
	NavigateTo(ET66ScreenType::AccountStatus);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleQuitClicked()
{
	ShowModal(ET66ScreenType::QuitConfirmation);
	return FReply::Handled();
}
