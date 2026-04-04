// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Dota/T66DotaSlate.h"

#include "UI/Dota/T66DotaTheme.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FSlateBrush* GetWhiteBrush()
	{
		return FCoreStyle::Get().GetBrush("WhiteBrush");
	}

	struct FT66OptionalTextureBrush
	{
		TStrongObjectPtr<UTexture2D> ImportedTexture;
		TStrongObjectPtr<UTexture2D> FileTexture;
		TSharedPtr<FSlateBrush> Brush;
		bool bChecked = false;

		UTexture2D* GetTexture() const
		{
			if (ImportedTexture.IsValid())
			{
				return ImportedTexture.Get();
			}

			return FileTexture.IsValid() ? FileTexture.Get() : nullptr;
		}
	};

	FSlateBrush MakeNineSliceBrush(UTexture2D* Texture, const FMargin& Margin)
	{
		FSlateBrush Brush;
		const bool bUseImageDraw =
			FMath::IsNearlyZero(Margin.Left) &&
			FMath::IsNearlyZero(Margin.Top) &&
			FMath::IsNearlyZero(Margin.Right) &&
			FMath::IsNearlyZero(Margin.Bottom);
		Brush.DrawAs = bUseImageDraw ? ESlateBrushDrawType::Image : ESlateBrushDrawType::Box;
		Brush.Tiling = ESlateBrushTileType::NoTile;
		Brush.SetResourceObject(Texture);
		Brush.TintColor = FSlateColor(FLinearColor::White);
		Brush.ImageSize = FVector2D(1.f, 1.f);
		Brush.Margin = Margin;
		return Brush;
	}

	UTexture2D* LoadOptionalTexture(
		FT66OptionalTextureBrush& Entry,
		const TCHAR* ImportedAssetPath,
		const FString& FallbackFilePath,
		const FMargin& Margin)
	{
		UTexture2D* Texture = Entry.GetTexture();
		if (Entry.Brush.IsValid())
		{
			if (Texture)
			{
				if (Entry.Brush->GetResourceObject() != Texture)
				{
					Entry.Brush->SetResourceObject(Texture);
				}
				return Texture;
			}

			Entry.Brush.Reset();
		}

		if (!Entry.bChecked)
		{
			Entry.bChecked = true;
			if (ImportedAssetPath && *ImportedAssetPath)
			{
				Entry.ImportedTexture.Reset(LoadObject<UTexture2D>(nullptr, ImportedAssetPath));
				if (Entry.ImportedTexture.IsValid())
				{
					Entry.ImportedTexture->Filter = TextureFilter::TF_Trilinear;
					Entry.ImportedTexture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
					Entry.ImportedTexture->NeverStream = true;
				}
			}

			Texture = Entry.GetTexture();
			if (!Texture && !FallbackFilePath.IsEmpty() && FPaths::FileExists(FallbackFilePath))
			{
				Texture = FImageUtils::ImportFileAsTexture2D(FallbackFilePath);
				if (Texture)
				{
					Texture->SRGB = true;
					Texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
					Texture->NeverStream = true;
					Texture->Filter = TextureFilter::TF_Trilinear;
					Texture->CompressionSettings = TC_EditorIcon;
					Texture->UpdateResource();
					Entry.FileTexture.Reset(Texture);
				}
			}

			Texture = Entry.GetTexture();
			if (Texture)
			{
				Entry.Brush = MakeShared<FSlateBrush>(MakeNineSliceBrush(Texture, Margin));
			}
		}

		return Entry.GetTexture();
	}

	const FSlateBrush* ResolveOptionalTextureBrush(
		FT66OptionalTextureBrush& Entry,
		const TCHAR* ImportedAssetPath,
		const FString& FallbackFilePath,
		const FMargin& Margin)
	{
		LoadOptionalTexture(Entry, ImportedAssetPath, FallbackFilePath, Margin);
		return Entry.Brush.IsValid() ? Entry.Brush.Get() : nullptr;
	}

	const FString GDotaGeneratedSourceDir = FPaths::ProjectDir() / TEXT("SourceAssets/UI/Dota/Generated");

	FT66OptionalTextureBrush& GetButtonPlateEntry(ET66DotaButtonPlateType Type)
	{
		static FT66OptionalTextureBrush Neutral;
		static FT66OptionalTextureBrush Primary;
		static FT66OptionalTextureBrush Danger;

		switch (Type)
		{
		case ET66DotaButtonPlateType::Primary:
			return Primary;
		case ET66DotaButtonPlateType::Danger:
			return Danger;
		case ET66DotaButtonPlateType::Neutral:
		default:
			return Neutral;
		}
	}

	const TCHAR* GetButtonPlateAssetPath(ET66DotaButtonPlateType Type)
	{
		switch (Type)
		{
		case ET66DotaButtonPlateType::Primary:
			return TEXT("/Game/UI/Assets/T_UI_DotaPrimaryButtonPlate.T_UI_DotaPrimaryButtonPlate");
		case ET66DotaButtonPlateType::Danger:
			return TEXT("/Game/UI/Assets/T_UI_DotaDangerButtonPlate.T_UI_DotaDangerButtonPlate");
		case ET66DotaButtonPlateType::Neutral:
		default:
			return TEXT("/Game/UI/Assets/T_UI_DotaNeutralButtonPlate.T_UI_DotaNeutralButtonPlate");
		}
	}

	FString GetButtonPlateFallbackPath(ET66DotaButtonPlateType Type)
	{
		switch (Type)
		{
		case ET66DotaButtonPlateType::Primary:
			return GDotaGeneratedSourceDir / TEXT("dota_primary_button_plate.png");
		case ET66DotaButtonPlateType::Danger:
			return GDotaGeneratedSourceDir / TEXT("dota_danger_button_plate.png");
		case ET66DotaButtonPlateType::Neutral:
		default:
			return GDotaGeneratedSourceDir / TEXT("dota_neutral_button_plate.png");
		}
	}

	FT66OptionalTextureBrush& GetInventorySlotEntry()
	{
		static FT66OptionalTextureBrush InventorySlot;
		return InventorySlot;
	}

	TSharedRef<SWidget> MakeLayeredSurface(
		const TSharedRef<SWidget>& Content,
		const FLinearColor& Outer,
		const FLinearColor& Middle,
		const FLinearColor& Fill,
		const FLinearColor& TopHighlight,
		const FMargin& Padding)
	{
		return SNew(SBorder)
			.BorderImage(GetWhiteBrush())
			.BorderBackgroundColor(Outer)
			.Padding(1.f)
			[
				SNew(SBorder)
				.BorderImage(GetWhiteBrush())
				.BorderBackgroundColor(Middle)
				.Padding(1.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SBorder)
						.BorderImage(GetWhiteBrush())
						.BorderBackgroundColor(Fill)
						.Padding(Padding)
						[
							Content
						]
					]
					+ SOverlay::Slot()
					.VAlign(VAlign_Top)
					.Padding(1.f, 1.f, 1.f, 0.f)
					[
						SNew(SBox)
						.HeightOverride(2.f)
						[
							SNew(SBorder)
							.BorderImage(GetWhiteBrush())
							.BorderBackgroundColor(TopHighlight)
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
							.BorderImage(GetWhiteBrush())
							.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.42f))
						]
					]
				]
			];
	}
}

TSharedRef<SWidget> FT66DotaSlate::MakeScreenSurface(const TSharedRef<SWidget>& Content, const FMargin& Padding)
{
	return MakeLayeredSurface(
		Content,
		FT66DotaTheme::PanelOuter(),
		FT66DotaTheme::Border(),
		FT66DotaTheme::PanelInner(),
		FT66DotaTheme::Stroke() * FLinearColor(1.f, 1.f, 1.f, 0.55f),
		Padding);
}

TSharedRef<SWidget> FT66DotaSlate::MakeViewportFrame(const TSharedRef<SWidget>& Content, const FMargin& Padding)
{
	return MakeLayeredSurface(
		Content,
		FT66DotaTheme::PanelOuter(),
		FT66DotaTheme::Border(),
		FT66DotaTheme::Background(),
		FT66DotaTheme::Stroke() * FLinearColor(1.f, 1.f, 1.f, 0.70f),
		Padding);
}

TSharedRef<SWidget> FT66DotaSlate::MakeViewportCutoutFrame(const TSharedRef<SWidget>& Content, const FMargin& Padding)
{
	return MakeLayeredSurface(
		Content,
		FT66DotaTheme::PanelOuter(),
		FT66DotaTheme::Border(),
		FLinearColor(0.f, 0.f, 0.f, 0.f),
		FT66DotaTheme::Stroke() * FLinearColor(1.f, 1.f, 1.f, 0.70f),
		Padding);
}

TSharedRef<SWidget> FT66DotaSlate::MakeSlotFrame(const TSharedRef<SWidget>& Content, const TAttribute<FSlateColor>& AccentColor, const FMargin& Padding)
{
	const FSlateBrush* SlotBrush = GetInventorySlotFrameBrush();

	return SNew(SBorder)
		.BorderImage(GetWhiteBrush())
		.BorderBackgroundColor(FT66DotaTheme::SlotOuter())
		.Padding(1.f)
		[
			SNew(SBorder)
			.BorderImage(GetWhiteBrush())
			.BorderBackgroundColor(AccentColor)
			.Padding(1.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(GetWhiteBrush())
					.BorderBackgroundColor(FT66DotaTheme::SlotFill())
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SImage)
							.Visibility(SlotBrush ? EVisibility::Visible : EVisibility::Collapsed)
							.Image(SlotBrush)
							.ColorAndOpacity(FLinearColor::White)
						]
						+ SOverlay::Slot()
						.Padding(Padding)
						[
							Content
						]
					]
				]
				+ SOverlay::Slot()
				.VAlign(VAlign_Top)
				.Padding(1.f, 1.f, 1.f, 0.f)
				[
					SNew(SBox)
					.HeightOverride(2.f)
					[
						SNew(SBorder)
						.BorderImage(GetWhiteBrush())
						.BorderBackgroundColor(FLinearColor(0.95f, 0.96f, 1.0f, 0.10f))
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
						.BorderImage(GetWhiteBrush())
						.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.42f))
					]
				]
			]
		];
}

TSharedRef<SWidget> FT66DotaSlate::MakeSlotFrame(const TSharedRef<SWidget>& Content, const FLinearColor& AccentColor, const FMargin& Padding)
{
	return MakeSlotFrame(Content, TAttribute<FSlateColor>(FSlateColor(AccentColor)), Padding);
}

TSharedRef<SWidget> FT66DotaSlate::MakeHudPanel(const TSharedRef<SWidget>& Content, const FText& Title, const FMargin& Padding)
{
	TSharedRef<SWidget> PanelContent =
		Title.IsEmpty()
		? Content
		: StaticCastSharedRef<SWidget>(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(Title)
				.Font(FT66DotaTheme::MakeFont(TEXT("Bold"), 18))
				.ColorAndOpacity(FT66DotaTheme::TextMuted())
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 8.f)
			[
				MakeDivider()
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				Content
			]);

	return MakeScreenSurface(PanelContent, Padding);
}

TSharedRef<SWidget> FT66DotaSlate::MakeHudPanel(const TSharedRef<SWidget>& Content, const FMargin& Padding)
{
	return MakeHudPanel(Content, FText::GetEmpty(), Padding);
}

TSharedRef<SWidget> FT66DotaSlate::MakeDivider(float Height)
{
	return SNew(SBox)
		.HeightOverride(Height)
		[
			SNew(SBorder)
			.BorderImage(GetWhiteBrush())
			.BorderBackgroundColor(FT66DotaTheme::Stroke() * FLinearColor(1.f, 1.f, 1.f, 0.45f))
		];
}

TSharedRef<SWidget> FT66DotaSlate::MakeMinimapFrame(const TSharedRef<SWidget>& Content, const FMargin& Padding)
{
	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			MakeLayeredSurface(
				Content,
				FT66DotaTheme::PanelOuter(),
				FT66DotaTheme::Border(),
				FT66DotaTheme::Background(),
				FT66DotaTheme::Accent() * FLinearColor(1.f, 1.f, 1.f, 0.75f),
				Padding)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(5.f)
		[
			SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
			[
				SNew(SBorder)
				.BorderImage(GetWhiteBrush())
				.BorderBackgroundColor(FT66DotaTheme::Accent2())
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(5.f)
		[
			SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
			[
				SNew(SBorder)
				.BorderImage(GetWhiteBrush())
				.BorderBackgroundColor(FT66DotaTheme::Accent2())
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(5.f)
		[
			SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
			[
				SNew(SBorder)
				.BorderImage(GetWhiteBrush())
				.BorderBackgroundColor(FT66DotaTheme::Accent2())
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(5.f)
		[
			SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
			[
				SNew(SBorder)
				.BorderImage(GetWhiteBrush())
				.BorderBackgroundColor(FT66DotaTheme::Accent2())
			]
		];
}

const FSlateBrush* FT66DotaSlate::GetButtonPlateBrush(ET66DotaButtonPlateType Type)
{
	const FMargin Margin = (Type == ET66DotaButtonPlateType::Primary)
		? FMargin(0.f)
		: FMargin(0.22f, 0.32f, 0.22f, 0.32f);
	return ResolveOptionalTextureBrush(
		GetButtonPlateEntry(Type),
		GetButtonPlateAssetPath(Type),
		GetButtonPlateFallbackPath(Type),
		Margin);
}

const FSlateBrush* FT66DotaSlate::GetInventorySlotFrameBrush()
{
	return ResolveOptionalTextureBrush(
		GetInventorySlotEntry(),
		TEXT("/Game/UI/Assets/T_UI_DotaInventorySlotFrame.T_UI_DotaInventorySlotFrame"),
		GDotaGeneratedSourceDir / TEXT("dota_inventory_slot_frame.png"),
		FMargin(0.22f));
}
