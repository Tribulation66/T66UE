// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Components/T66LeaderboardFilterButton.h"
#include "UI/Style/T66Style.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Widgets/Layout/SBox.h"

void UT66LeaderboardFilterButton::EnsureButtonAndImageCreated()
{
	if (FilterButton && IconImage) return;

	UPanelWidget* Root = Cast<UPanelWidget>(GetRootWidget());
	if (!Root) return;

	if (!FilterButton)
	{
		FilterButton = NewObject<UButton>(this, UButton::StaticClass(), FName("FilterButton"));
		if (!FilterButton) return;
		FilterButton->SetVisibility(ESlateVisibility::Visible);
		FilterButton->SetColorAndOpacity(FLinearColor::White);
		FilterButton->SetBackgroundColor(FLinearColor::Transparent);
		const FButtonStyle& BtnStyle = FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral");
		FilterButton->SetStyle(BtnStyle);
		Root->AddChild(FilterButton);
	}
	if (!IconImage)
	{
		IconImage = NewObject<UImage>(this, UImage::StaticClass(), FName("IconImage"));
		if (!IconImage) return;
		IconImage->SetVisibility(ESlateVisibility::Visible);
		IconImage->SetColorAndOpacity(FLinearColor::White);
		FilterButton->AddChild(IconImage);
		// Make the image fill the button so the icon is visible
		if (UButtonSlot* BtnSlot = Cast<UButtonSlot>(IconImage->Slot))
		{
			BtnSlot->SetPadding(FMargin(0.0f));
			BtnSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
			BtnSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
		}
	}
}

void UT66LeaderboardFilterButton::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureButtonAndImageCreated();
	if (FilterButton)
	{
		FilterButton->OnClicked.AddDynamic(this, &UT66LeaderboardFilterButton::NotifyClicked);
	}
	SyncBrush();
	if (SpriteSheetMaterial && FrameCount > 1)
	{
		SpriteDynamicMaterial = UMaterialInstanceDynamic::Create(SpriteSheetMaterial, this);
		SpriteTime = 0.0f;
		if (IconImage && SpriteDynamicMaterial)
		{
			FSlateBrush Brush = IconImage->GetBrush();
			Brush.SetResourceObject(SpriteDynamicMaterial);
			IconImage->SetBrush(Brush);
		}
	}
}

void UT66LeaderboardFilterButton::NativeDestruct()
{
	SpriteDynamicMaterial = nullptr;
	Super::NativeDestruct();
}

void UT66LeaderboardFilterButton::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	// Apply icon once when we have texture/material and IconImage (handles late binding)
	if (IconImage && (StaticTexture || SpriteSheetMaterial) && !bSyncedBrushOnce)
	{
		SyncBrush();
		bSyncedBrushOnce = true;
	}
	if (SpriteDynamicMaterial && FrameCount > 1 && FramesPerSecond > 0.0f)
	{
		AdvanceSpriteFrame(InDeltaTime);
	}
}

void UT66LeaderboardFilterButton::RefreshIcon()
{
	bSyncedBrushOnce = false;
	SyncBrush();
	if (IconImage && (StaticTexture || SpriteSheetMaterial))
	{
		bSyncedBrushOnce = true;
	}
}

void UT66LeaderboardFilterButton::SyncBrush()
{
	if (!IconImage) return;

	if (StaticTexture)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(StaticTexture);
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.Tiling = ESlateBrushTileType::NoTile;
		IconImage->SetBrush(Brush);
		return;
	}
	if (SpriteSheetMaterial && FrameCount > 1 && SpriteDynamicMaterial)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(SpriteDynamicMaterial);
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.Tiling = ESlateBrushTileType::NoTile;
		IconImage->SetBrush(Brush);
	}
	else if (SpriteSheetMaterial)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(SpriteSheetMaterial);
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.Tiling = ESlateBrushTileType::NoTile;
		IconImage->SetBrush(Brush);
	}
}

void UT66LeaderboardFilterButton::AdvanceSpriteFrame(float DeltaTime)
{
	if (!SpriteDynamicMaterial || FrameCount <= 1) return;
	SpriteTime += DeltaTime;
	const float Period = 1.0f / FramesPerSecond;
	while (SpriteTime >= Period)
	{
		SpriteTime -= Period;
	}
	const int32 FrameIndex = FMath::Clamp(
		FMath::FloorToInt(SpriteTime * FramesPerSecond) % FrameCount,
		0, FrameCount - 1);
	SpriteDynamicMaterial->SetScalarParameterValue(FName("Frame"), static_cast<float>(FrameIndex));
}

void UT66LeaderboardFilterButton::NotifyClicked()
{
	OnFilterButtonClicked.Broadcast(Filter);
}
