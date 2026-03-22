// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Style/T66ButtonVisuals.h"
#include "UI/Style/T66Style.h"

#include "Brushes/SlateDynamicImageBrush.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	static TObjectPtr<UMaterialInterface> GRetroSkyNormal = nullptr;
	static TObjectPtr<UMaterialInterface> GRetroSkyHovered = nullptr;
	static TObjectPtr<UMaterialInterface> GRetroSkyPressed = nullptr;
	static bool bCheckedRetroSkyMaterials = false;
	static TObjectPtr<UTexture2D> GRetroWoodHorizontalNormal = nullptr;
	static TObjectPtr<UTexture2D> GRetroWoodHorizontalHovered = nullptr;
	static TObjectPtr<UTexture2D> GRetroWoodHorizontalPressed = nullptr;
	static TObjectPtr<UTexture2D> GRetroWoodVerticalNormal = nullptr;
	static TObjectPtr<UTexture2D> GRetroWoodVerticalHovered = nullptr;
	static TObjectPtr<UTexture2D> GRetroWoodVerticalPressed = nullptr;
	static bool bCheckedRetroWoodTextures = false;
	static TObjectPtr<UTexture2D> GMainMenuArcaneFillNormal = nullptr;
	static TObjectPtr<UTexture2D> GMainMenuArcaneFillHovered = nullptr;
	static TObjectPtr<UTexture2D> GMainMenuArcaneFillPressed = nullptr;
	static bool bCheckedMainMenuArcaneFillTextures = false;

	static FSlateBrush MakeMaterialBrush(UMaterialInterface* Material)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.Tiling = ESlateBrushTileType::NoTile;
		Brush.SetResourceObject(Material);
		Brush.TintColor = FSlateColor(FLinearColor::White);
		Brush.ImageSize = FVector2D(1.f, 1.f);
		return Brush;
	}

	static FSlateBrush MakeTextureBrush(UTexture2D* Texture, const FVector2D& Size)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.Tiling = ESlateBrushTileType::NoTile;
		Brush.SetResourceObject(Texture);
		Brush.TintColor = FSlateColor(FLinearColor::White);
		// Keep decorative border brushes layout-neutral so they don't change button desired size.
		// The surrounding SBox edge strips control the actual border thickness.
		Brush.ImageSize = FVector2D(1.f, 1.f);
		return Brush;
	}

	static FSlateBrush MakeFillTextureBrush(UTexture2D* Texture)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::Box;
		Brush.Tiling = ESlateBrushTileType::NoTile;
		Brush.SetResourceObject(Texture);
		Brush.TintColor = FSlateColor(FLinearColor::White);
		Brush.ImageSize = FVector2D(1.f, 1.f);
		Brush.Margin = FMargin(0.f);
		return Brush;
	}

	static TSharedPtr<FSlateBrush> MakeFillFileBrush(const FString& FilePath)
	{
		if (!FPaths::FileExists(FilePath))
		{
			return nullptr;
		}

		return MakeShared<FSlateDynamicImageBrush>(FName(*FilePath), FVector2D(1.f, 1.f));
	}

	static FString GetMainMenuArcaneFillFilePath(const TCHAR* FileName)
	{
		return FPaths::ProjectDir() / TEXT("SourceAssets/UI/MainMenuArcaneFill") / FileName;
	}

	static void LoadRetroSkyMaterialsOnce()
	{
		if (bCheckedRetroSkyMaterials)
		{
			return;
		}

		bCheckedRetroSkyMaterials = true;
		GRetroSkyNormal = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/UI/Materials/MI_UI_RetroSkyBorderOverlay_N.MI_UI_RetroSkyBorderOverlay_N"));
		GRetroSkyHovered = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/UI/Materials/MI_UI_RetroSkyBorderOverlay_H.MI_UI_RetroSkyBorderOverlay_H"));
		GRetroSkyPressed = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/UI/Materials/MI_UI_RetroSkyBorderOverlay_P.MI_UI_RetroSkyBorderOverlay_P"));
	}

	static void LoadRetroWoodTexturesOnce()
	{
		if (bCheckedRetroWoodTextures)
		{
			return;
		}

		bCheckedRetroWoodTextures = true;

		auto LoadTexture = [](const TCHAR* Path) -> UTexture2D*
		{
			UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, Path);
			UE_LOG(LogTemp, Log, TEXT("[BORDER] LoadRetroWoodTexture: %s -> %s"), Path, Texture ? TEXT("OK") : TEXT("MISS"));
			return Texture;
		};

		GRetroWoodHorizontalNormal = LoadTexture(TEXT("/Game/UI/Assets/T_UI_RetroWoodTrimH_V2_N.T_UI_RetroWoodTrimH_V2_N"));
		GRetroWoodHorizontalHovered = LoadTexture(TEXT("/Game/UI/Assets/T_UI_RetroWoodTrimH_V2_H.T_UI_RetroWoodTrimH_V2_H"));
		GRetroWoodHorizontalPressed = LoadTexture(TEXT("/Game/UI/Assets/T_UI_RetroWoodTrimH_V2_P.T_UI_RetroWoodTrimH_V2_P"));
		GRetroWoodVerticalNormal = LoadTexture(TEXT("/Game/UI/Assets/T_UI_RetroWoodTrimV_V2_N.T_UI_RetroWoodTrimV_V2_N"));
		GRetroWoodVerticalHovered = LoadTexture(TEXT("/Game/UI/Assets/T_UI_RetroWoodTrimV_V2_H.T_UI_RetroWoodTrimV_V2_H"));
		GRetroWoodVerticalPressed = LoadTexture(TEXT("/Game/UI/Assets/T_UI_RetroWoodTrimV_V2_P.T_UI_RetroWoodTrimV_V2_P"));
	}

	static void LoadMainMenuArcaneFillTexturesOnce()
	{
		if (bCheckedMainMenuArcaneFillTextures)
		{
			return;
		}

		bCheckedMainMenuArcaneFillTextures = true;

		auto LoadTexture = [](const TCHAR* Path) -> UTexture2D*
		{
			UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, Path);
			UE_LOG(LogTemp, Log, TEXT("[BUTTONFILL] LoadMainMenuArcaneFillTexture: %s -> %s"), Path, Texture ? TEXT("OK") : TEXT("MISS"));
			return Texture;
		};

		GMainMenuArcaneFillNormal = LoadTexture(TEXT("/Game/UI/Assets/T_UI_MainMenuArcaneFill_N.T_UI_MainMenuArcaneFill_N"));
		GMainMenuArcaneFillHovered = LoadTexture(TEXT("/Game/UI/Assets/T_UI_MainMenuArcaneFill_H.T_UI_MainMenuArcaneFill_H"));
		GMainMenuArcaneFillPressed = LoadTexture(TEXT("/Game/UI/Assets/T_UI_MainMenuArcaneFill_P.T_UI_MainMenuArcaneFill_P"));
	}
}

TSharedPtr<FT66ButtonBorderBrushSet> FT66ButtonVisuals::CreateBorderBrushSet(ET66ButtonBorderVisual Visual)
{
	if (Visual == ET66ButtonBorderVisual::RetroWood)
	{
		LoadRetroWoodTexturesOnce();

		if (!GRetroWoodHorizontalNormal || !GRetroWoodHorizontalHovered || !GRetroWoodHorizontalPressed
			|| !GRetroWoodVerticalNormal || !GRetroWoodVerticalHovered || !GRetroWoodVerticalPressed)
		{
			UE_LOG(LogTemp, Warning, TEXT("[BORDER] CreateBorderBrushSet(RetroWood): missing border texture(s), returning null brush set"));
			return nullptr;
		}

		TSharedPtr<FT66ButtonBorderBrushSet> Set = MakeShared<FT66ButtonBorderBrushSet>();
		Set->Normal = MakeShared<FSlateBrush>(MakeTextureBrush(GRetroWoodHorizontalNormal, FVector2D(256.f, 24.f)));
		Set->Hovered = MakeShared<FSlateBrush>(MakeTextureBrush(GRetroWoodHorizontalHovered, FVector2D(256.f, 24.f)));
		Set->Pressed = MakeShared<FSlateBrush>(MakeTextureBrush(GRetroWoodHorizontalPressed, FVector2D(256.f, 24.f)));
		Set->HorizontalNormal = Set->Normal;
		Set->HorizontalHovered = Set->Hovered;
		Set->HorizontalPressed = Set->Pressed;
		Set->VerticalNormal = MakeShared<FSlateBrush>(MakeTextureBrush(GRetroWoodVerticalNormal, FVector2D(24.f, 256.f)));
		Set->VerticalHovered = MakeShared<FSlateBrush>(MakeTextureBrush(GRetroWoodVerticalHovered, FVector2D(24.f, 256.f)));
		Set->VerticalPressed = MakeShared<FSlateBrush>(MakeTextureBrush(GRetroWoodVerticalPressed, FVector2D(24.f, 256.f)));
		Set->Thickness = 12.f;
		UE_LOG(LogTemp, Log, TEXT("[BORDER] CreateBorderBrushSet(RetroWood): texture brush set ready, thickness=%.1f"), Set->Thickness);
		return Set;
	}

	if (Visual != ET66ButtonBorderVisual::RetroSky)
	{
		return nullptr;
	}

	LoadRetroSkyMaterialsOnce();

	if (!GRetroSkyNormal || !GRetroSkyHovered || !GRetroSkyPressed)
	{
		return nullptr;
	}

	TSharedPtr<FT66ButtonBorderBrushSet> Set = MakeShared<FT66ButtonBorderBrushSet>();
	Set->Normal = MakeShared<FSlateBrush>(MakeMaterialBrush(GRetroSkyNormal));
	Set->Hovered = MakeShared<FSlateBrush>(MakeMaterialBrush(GRetroSkyHovered));
	Set->Pressed = MakeShared<FSlateBrush>(MakeMaterialBrush(GRetroSkyPressed));
	Set->Thickness = 12.f;
	return Set;
}

TSharedPtr<FT66ButtonFillBrushSet> FT66ButtonVisuals::CreateFillBrushSet(ET66ButtonBackgroundVisual Visual)
{
	if (Visual != ET66ButtonBackgroundVisual::MainMenuArcane)
	{
		return nullptr;
	}

	LoadMainMenuArcaneFillTexturesOnce();

	if (!GMainMenuArcaneFillNormal || !GMainMenuArcaneFillHovered || !GMainMenuArcaneFillPressed)
	{
		TSharedPtr<FSlateBrush> FileNormal = MakeFillFileBrush(GetMainMenuArcaneFillFilePath(TEXT("T_UI_MainMenuArcaneFill_N.png")));
		TSharedPtr<FSlateBrush> FileHovered = MakeFillFileBrush(GetMainMenuArcaneFillFilePath(TEXT("T_UI_MainMenuArcaneFill_H.png")));
		TSharedPtr<FSlateBrush> FilePressed = MakeFillFileBrush(GetMainMenuArcaneFillFilePath(TEXT("T_UI_MainMenuArcaneFill_P.png")));

		if (!FileNormal.IsValid() || !FileHovered.IsValid() || !FilePressed.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[BUTTONFILL] CreateFillBrushSet(MainMenuArcane): missing fill textures in Content and SourceAssets, returning null brush set"));
			return nullptr;
		}

		TSharedPtr<FT66ButtonFillBrushSet> Set = MakeShared<FT66ButtonFillBrushSet>();
		Set->Normal = FileNormal;
		Set->Hovered = FileHovered;
		Set->Pressed = FilePressed;
		UE_LOG(LogTemp, Log, TEXT("[BUTTONFILL] CreateFillBrushSet(MainMenuArcane): using file-backed SourceAssets fill brushes"));
		return Set;
	}

	TSharedPtr<FT66ButtonFillBrushSet> Set = MakeShared<FT66ButtonFillBrushSet>();
	Set->Normal = MakeShared<FSlateBrush>(MakeFillTextureBrush(GMainMenuArcaneFillNormal));
	Set->Hovered = MakeShared<FSlateBrush>(MakeFillTextureBrush(GMainMenuArcaneFillHovered));
	Set->Pressed = MakeShared<FSlateBrush>(MakeFillTextureBrush(GMainMenuArcaneFillPressed));
	return Set;
}
