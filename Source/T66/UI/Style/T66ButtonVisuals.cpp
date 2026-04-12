// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Style/T66ButtonVisuals.h"
#include "UI/Style/T66LegacyUITextureAccess.h"
#include "UI/Style/T66Style.h"

#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66ButtonVisuals, Log, All);

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
	static TObjectPtr<UTexture2D> GMainMenuBlueTrimHorizontalNormal = nullptr;
	static TObjectPtr<UTexture2D> GMainMenuBlueTrimHorizontalHovered = nullptr;
	static TObjectPtr<UTexture2D> GMainMenuBlueTrimHorizontalPressed = nullptr;
	static TObjectPtr<UTexture2D> GMainMenuBlueTrimVerticalNormal = nullptr;
	static TObjectPtr<UTexture2D> GMainMenuBlueTrimVerticalHovered = nullptr;
	static TObjectPtr<UTexture2D> GMainMenuBlueTrimVerticalPressed = nullptr;
	static bool bCheckedMainMenuBlueTrimTextures = false;
	static TStrongObjectPtr<UTexture2D> GMainMenuBlueTrimFileHorizontalNormal;
	static TStrongObjectPtr<UTexture2D> GMainMenuBlueTrimFileHorizontalHovered;
	static TStrongObjectPtr<UTexture2D> GMainMenuBlueTrimFileHorizontalPressed;
	static TStrongObjectPtr<UTexture2D> GMainMenuBlueTrimFileVerticalNormal;
	static TStrongObjectPtr<UTexture2D> GMainMenuBlueTrimFileVerticalHovered;
	static TStrongObjectPtr<UTexture2D> GMainMenuBlueTrimFileVerticalPressed;
	static TObjectPtr<UTexture2D> GMainMenuArcaneFillNormal = nullptr;
	static TObjectPtr<UTexture2D> GMainMenuArcaneFillHovered = nullptr;
	static TObjectPtr<UTexture2D> GMainMenuArcaneFillPressed = nullptr;
	static bool bCheckedMainMenuArcaneFillTextures = false;
	static TStrongObjectPtr<UTexture2D> GMainMenuArcaneFillFileNormal;
	static TStrongObjectPtr<UTexture2D> GMainMenuArcaneFillFileHovered;
	static TStrongObjectPtr<UTexture2D> GMainMenuArcaneFillFilePressed;
	static TObjectPtr<UTexture2D> GMainMenuBlueWoodFillNormal = nullptr;
	static TObjectPtr<UTexture2D> GMainMenuBlueWoodFillHovered = nullptr;
	static TObjectPtr<UTexture2D> GMainMenuBlueWoodFillPressed = nullptr;
	static bool bCheckedMainMenuBlueWoodFillTextures = false;
	static TStrongObjectPtr<UTexture2D> GMainMenuBlueWoodFillFileNormal;
	static TStrongObjectPtr<UTexture2D> GMainMenuBlueWoodFillFileHovered;
	static TStrongObjectPtr<UTexture2D> GMainMenuBlueWoodFillFilePressed;

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

	static UTexture2D* LoadFallbackTexture(const FString& FilePath, TStrongObjectPtr<UTexture2D>& CachedTexture, const TCHAR* DebugVisualName)
	{
		if (CachedTexture.IsValid())
		{
			return CachedTexture.Get();
		}

		UTexture2D* Texture = T66LegacyUITextureAccess::ImportFileTexture(
			FilePath,
			TextureFilter::TF_Nearest,
			false,
			DebugVisualName);
		UE_LOG(LogT66ButtonVisuals, Log, TEXT("[BUTTONVISUAL] LoadFallbackTexture(%s): %s -> %s"), DebugVisualName, *FilePath, Texture ? TEXT("OK") : TEXT("MISS"));
		if (!Texture)
		{
			return nullptr;
		}

		CachedTexture.Reset(Texture);
		return CachedTexture.Get();
	}

	static FString GetMainMenuArcaneFillFilePath(const TCHAR* FileName)
	{
		return T66LegacyUITextureAccess::MakeProjectRuntimeDependencyPath(TEXT("RuntimeDependencies/T66/UI/MainMenuArcaneFill")) / FileName;
	}

	static FString GetMainMenuBlueTrimFilePath(const TCHAR* FileName)
	{
		return T66LegacyUITextureAccess::MakeProjectRuntimeDependencyPath(TEXT("RuntimeDependencies/T66/UI/MainMenuBlueTrim")) / FileName;
	}

	static FString GetMainMenuBlueWoodFillFilePath(const TCHAR* FileName)
	{
		return T66LegacyUITextureAccess::MakeProjectRuntimeDependencyPath(TEXT("RuntimeDependencies/T66/UI/MainMenuBlueWoodFill")) / FileName;
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
			UE_LOG(LogT66ButtonVisuals, Log, TEXT("[BORDER] LoadRetroWoodTexture: %s -> %s"), Path, Texture ? TEXT("OK") : TEXT("MISS"));
			return Texture;
		};

		GRetroWoodHorizontalNormal = LoadTexture(TEXT("/Game/UI/Assets/T_UI_RetroWoodTrimH_V2_N.T_UI_RetroWoodTrimH_V2_N"));
		GRetroWoodHorizontalHovered = LoadTexture(TEXT("/Game/UI/Assets/T_UI_RetroWoodTrimH_V2_H.T_UI_RetroWoodTrimH_V2_H"));
		GRetroWoodHorizontalPressed = LoadTexture(TEXT("/Game/UI/Assets/T_UI_RetroWoodTrimH_V2_P.T_UI_RetroWoodTrimH_V2_P"));
		GRetroWoodVerticalNormal = LoadTexture(TEXT("/Game/UI/Assets/T_UI_RetroWoodTrimV_V2_N.T_UI_RetroWoodTrimV_V2_N"));
		GRetroWoodVerticalHovered = LoadTexture(TEXT("/Game/UI/Assets/T_UI_RetroWoodTrimV_V2_H.T_UI_RetroWoodTrimV_V2_H"));
		GRetroWoodVerticalPressed = LoadTexture(TEXT("/Game/UI/Assets/T_UI_RetroWoodTrimV_V2_P.T_UI_RetroWoodTrimV_V2_P"));
	}

	static void LoadMainMenuBlueTrimTexturesOnce()
	{
		if (bCheckedMainMenuBlueTrimTextures)
		{
			return;
		}

		bCheckedMainMenuBlueTrimTextures = true;

		auto LoadTexture = [](const TCHAR* Path) -> UTexture2D*
		{
			UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, Path);
			UE_LOG(LogT66ButtonVisuals, Log, TEXT("[BORDER] LoadMainMenuBlueTrimTexture: %s -> %s"), Path, Texture ? TEXT("OK") : TEXT("MISS"));
			return Texture;
		};

		GMainMenuBlueTrimHorizontalNormal = LoadTexture(TEXT("/Game/UI/Assets/T_UI_MainMenuBlueTrimH_N.T_UI_MainMenuBlueTrimH_N"));
		GMainMenuBlueTrimHorizontalHovered = LoadTexture(TEXT("/Game/UI/Assets/T_UI_MainMenuBlueTrimH_H.T_UI_MainMenuBlueTrimH_H"));
		GMainMenuBlueTrimHorizontalPressed = LoadTexture(TEXT("/Game/UI/Assets/T_UI_MainMenuBlueTrimH_P.T_UI_MainMenuBlueTrimH_P"));
		GMainMenuBlueTrimVerticalNormal = LoadTexture(TEXT("/Game/UI/Assets/T_UI_MainMenuBlueTrimV_N.T_UI_MainMenuBlueTrimV_N"));
		GMainMenuBlueTrimVerticalHovered = LoadTexture(TEXT("/Game/UI/Assets/T_UI_MainMenuBlueTrimV_H.T_UI_MainMenuBlueTrimV_H"));
		GMainMenuBlueTrimVerticalPressed = LoadTexture(TEXT("/Game/UI/Assets/T_UI_MainMenuBlueTrimV_P.T_UI_MainMenuBlueTrimV_P"));
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
			UE_LOG(LogT66ButtonVisuals, Log, TEXT("[BUTTONFILL] LoadMainMenuArcaneFillTexture: %s -> %s"), Path, Texture ? TEXT("OK") : TEXT("MISS"));
			return Texture;
		};

		GMainMenuArcaneFillNormal = LoadTexture(TEXT("/Game/UI/Assets/T_UI_MainMenuArcaneFill_N.T_UI_MainMenuArcaneFill_N"));
		GMainMenuArcaneFillHovered = LoadTexture(TEXT("/Game/UI/Assets/T_UI_MainMenuArcaneFill_H.T_UI_MainMenuArcaneFill_H"));
		GMainMenuArcaneFillPressed = LoadTexture(TEXT("/Game/UI/Assets/T_UI_MainMenuArcaneFill_P.T_UI_MainMenuArcaneFill_P"));
	}

	static void LoadMainMenuBlueWoodFillTexturesOnce()
	{
		if (bCheckedMainMenuBlueWoodFillTextures)
		{
			return;
		}

		bCheckedMainMenuBlueWoodFillTextures = true;

		auto LoadTexture = [](const TCHAR* Path) -> UTexture2D*
		{
			UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, Path);
			UE_LOG(LogT66ButtonVisuals, Log, TEXT("[BUTTONFILL] LoadMainMenuBlueWoodFillTexture: %s -> %s"), Path, Texture ? TEXT("OK") : TEXT("MISS"));
			return Texture;
		};

		GMainMenuBlueWoodFillNormal = LoadTexture(TEXT("/Game/UI/Assets/T_UI_MainMenuBlueWoodFill_N.T_UI_MainMenuBlueWoodFill_N"));
		GMainMenuBlueWoodFillHovered = LoadTexture(TEXT("/Game/UI/Assets/T_UI_MainMenuBlueWoodFill_H.T_UI_MainMenuBlueWoodFill_H"));
		GMainMenuBlueWoodFillPressed = LoadTexture(TEXT("/Game/UI/Assets/T_UI_MainMenuBlueWoodFill_P.T_UI_MainMenuBlueWoodFill_P"));
	}

	static TSharedPtr<FT66ButtonBorderBrushSet> CreateBorderBrushSetFromResources(
		UTexture2D* HorizontalNormalTexture,
		UTexture2D* HorizontalHoveredTexture,
		UTexture2D* HorizontalPressedTexture,
		UTexture2D* VerticalNormalTexture,
		UTexture2D* VerticalHoveredTexture,
		UTexture2D* VerticalPressedTexture,
		const FString& HorizontalNormalFilePath,
		const FString& HorizontalHoveredFilePath,
		const FString& HorizontalPressedFilePath,
		const FString& VerticalNormalFilePath,
		const FString& VerticalHoveredFilePath,
		const FString& VerticalPressedFilePath,
		TStrongObjectPtr<UTexture2D>& FileHorizontalNormalTexture,
		TStrongObjectPtr<UTexture2D>& FileHorizontalHoveredTexture,
		TStrongObjectPtr<UTexture2D>& FileHorizontalPressedTexture,
		TStrongObjectPtr<UTexture2D>& FileVerticalNormalTexture,
		TStrongObjectPtr<UTexture2D>& FileVerticalHoveredTexture,
		TStrongObjectPtr<UTexture2D>& FileVerticalPressedTexture,
		const TCHAR* DebugVisualName)
	{
		if (!HorizontalNormalTexture || !HorizontalHoveredTexture || !HorizontalPressedTexture
			|| !VerticalNormalTexture || !VerticalHoveredTexture || !VerticalPressedTexture)
		{
			UTexture2D* LoadedHorizontalNormal = LoadFallbackTexture(HorizontalNormalFilePath, FileHorizontalNormalTexture, DebugVisualName);
			UTexture2D* LoadedHorizontalHovered = LoadFallbackTexture(HorizontalHoveredFilePath, FileHorizontalHoveredTexture, DebugVisualName);
			UTexture2D* LoadedHorizontalPressed = LoadFallbackTexture(HorizontalPressedFilePath, FileHorizontalPressedTexture, DebugVisualName);
			UTexture2D* LoadedVerticalNormal = LoadFallbackTexture(VerticalNormalFilePath, FileVerticalNormalTexture, DebugVisualName);
			UTexture2D* LoadedVerticalHovered = LoadFallbackTexture(VerticalHoveredFilePath, FileVerticalHoveredTexture, DebugVisualName);
			UTexture2D* LoadedVerticalPressed = LoadFallbackTexture(VerticalPressedFilePath, FileVerticalPressedTexture, DebugVisualName);

			if (!LoadedHorizontalNormal || !LoadedHorizontalHovered || !LoadedHorizontalPressed
				|| !LoadedVerticalNormal || !LoadedVerticalHovered || !LoadedVerticalPressed)
			{
				UE_LOG(
					LogT66ButtonVisuals,
					Warning,
					TEXT("[BORDER] CreateBorderBrushSet(%s): missing cooked trim textures and staged runtime fallback, returning null brush set"),
					DebugVisualName);
				return nullptr;
			}

			HorizontalNormalTexture = LoadedHorizontalNormal;
			HorizontalHoveredTexture = LoadedHorizontalHovered;
			HorizontalPressedTexture = LoadedHorizontalPressed;
			VerticalNormalTexture = LoadedVerticalNormal;
			VerticalHoveredTexture = LoadedVerticalHovered;
			VerticalPressedTexture = LoadedVerticalPressed;
			UE_LOG(LogT66ButtonVisuals, Log, TEXT("[BORDER] CreateBorderBrushSet(%s): using staged runtime trim textures"), DebugVisualName);
		}

		TSharedPtr<FT66ButtonBorderBrushSet> Set = MakeShared<FT66ButtonBorderBrushSet>();
		Set->Normal = MakeShared<FSlateBrush>(MakeTextureBrush(HorizontalNormalTexture, FVector2D(256.f, 24.f)));
		Set->Hovered = MakeShared<FSlateBrush>(MakeTextureBrush(HorizontalHoveredTexture, FVector2D(256.f, 24.f)));
		Set->Pressed = MakeShared<FSlateBrush>(MakeTextureBrush(HorizontalPressedTexture, FVector2D(256.f, 24.f)));
		Set->HorizontalNormal = Set->Normal;
		Set->HorizontalHovered = Set->Hovered;
		Set->HorizontalPressed = Set->Pressed;
		Set->VerticalNormal = MakeShared<FSlateBrush>(MakeTextureBrush(VerticalNormalTexture, FVector2D(24.f, 256.f)));
		Set->VerticalHovered = MakeShared<FSlateBrush>(MakeTextureBrush(VerticalHoveredTexture, FVector2D(24.f, 256.f)));
		Set->VerticalPressed = MakeShared<FSlateBrush>(MakeTextureBrush(VerticalPressedTexture, FVector2D(24.f, 256.f)));
		Set->Thickness = 12.f;
		return Set;
	}

	static TSharedPtr<FT66ButtonFillBrushSet> CreateFillBrushSetFromResources(
		UTexture2D* NormalTexture,
		UTexture2D* HoveredTexture,
		UTexture2D* PressedTexture,
		const FString& NormalFilePath,
		const FString& HoveredFilePath,
		const FString& PressedFilePath,
		TStrongObjectPtr<UTexture2D>& FileNormalTexture,
		TStrongObjectPtr<UTexture2D>& FileHoveredTexture,
		TStrongObjectPtr<UTexture2D>& FilePressedTexture,
		const TCHAR* DebugVisualName)
	{
		if (!NormalTexture || !HoveredTexture || !PressedTexture)
		{
			UTexture2D* LoadedNormal = LoadFallbackTexture(NormalFilePath, FileNormalTexture, DebugVisualName);
			UTexture2D* LoadedHovered = LoadFallbackTexture(HoveredFilePath, FileHoveredTexture, DebugVisualName);
			UTexture2D* LoadedPressed = LoadFallbackTexture(PressedFilePath, FilePressedTexture, DebugVisualName);

			if (!LoadedNormal || !LoadedHovered || !LoadedPressed)
			{
				UE_LOG(
					LogT66ButtonVisuals,
					Warning,
					TEXT("[BUTTONFILL] CreateFillBrushSet(%s): missing cooked fill textures and staged runtime fallback, returning null brush set"),
					DebugVisualName);
				return nullptr;
			}

			NormalTexture = LoadedNormal;
			HoveredTexture = LoadedHovered;
			PressedTexture = LoadedPressed;
			UE_LOG(LogT66ButtonVisuals, Log, TEXT("[BUTTONFILL] CreateFillBrushSet(%s): using staged runtime fill textures"), DebugVisualName);
		}

		TSharedPtr<FT66ButtonFillBrushSet> Set = MakeShared<FT66ButtonFillBrushSet>();
		Set->Normal = MakeShared<FSlateBrush>(MakeFillTextureBrush(NormalTexture));
		Set->Hovered = MakeShared<FSlateBrush>(MakeFillTextureBrush(HoveredTexture));
		Set->Pressed = MakeShared<FSlateBrush>(MakeFillTextureBrush(PressedTexture));
		return Set;
	}
}

TSharedPtr<FT66ButtonBorderBrushSet> FT66ButtonVisuals::CreateBorderBrushSet(ET66ButtonBorderVisual Visual)
{
	if (Visual == ET66ButtonBorderVisual::MainMenuBlueTrim)
	{
		LoadMainMenuBlueTrimTexturesOnce();
		return CreateBorderBrushSetFromResources(
			GMainMenuBlueTrimHorizontalNormal,
			GMainMenuBlueTrimHorizontalHovered,
			GMainMenuBlueTrimHorizontalPressed,
			GMainMenuBlueTrimVerticalNormal,
			GMainMenuBlueTrimVerticalHovered,
			GMainMenuBlueTrimVerticalPressed,
			GetMainMenuBlueTrimFilePath(TEXT("T_UI_MainMenuBlueTrimH_N.png")),
			GetMainMenuBlueTrimFilePath(TEXT("T_UI_MainMenuBlueTrimH_H.png")),
			GetMainMenuBlueTrimFilePath(TEXT("T_UI_MainMenuBlueTrimH_P.png")),
			GetMainMenuBlueTrimFilePath(TEXT("T_UI_MainMenuBlueTrimV_N.png")),
			GetMainMenuBlueTrimFilePath(TEXT("T_UI_MainMenuBlueTrimV_H.png")),
			GetMainMenuBlueTrimFilePath(TEXT("T_UI_MainMenuBlueTrimV_P.png")),
			GMainMenuBlueTrimFileHorizontalNormal,
			GMainMenuBlueTrimFileHorizontalHovered,
			GMainMenuBlueTrimFileHorizontalPressed,
			GMainMenuBlueTrimFileVerticalNormal,
			GMainMenuBlueTrimFileVerticalHovered,
			GMainMenuBlueTrimFileVerticalPressed,
			TEXT("MainMenuBlueTrim"));
	}

	if (Visual == ET66ButtonBorderVisual::RetroWood)
	{
		LoadRetroWoodTexturesOnce();

		if (!GRetroWoodHorizontalNormal || !GRetroWoodHorizontalHovered || !GRetroWoodHorizontalPressed
			|| !GRetroWoodVerticalNormal || !GRetroWoodVerticalHovered || !GRetroWoodVerticalPressed)
		{
			UE_LOG(LogT66ButtonVisuals, Warning, TEXT("[BORDER] CreateBorderBrushSet(RetroWood): missing border texture(s), returning null brush set"));
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
		UE_LOG(LogT66ButtonVisuals, Log, TEXT("[BORDER] CreateBorderBrushSet(RetroWood): texture brush set ready, thickness=%.1f"), Set->Thickness);
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
	if (Visual == ET66ButtonBackgroundVisual::MainMenuArcane)
	{
		LoadMainMenuArcaneFillTexturesOnce();
		return CreateFillBrushSetFromResources(
			GMainMenuArcaneFillNormal,
			GMainMenuArcaneFillHovered,
			GMainMenuArcaneFillPressed,
			GetMainMenuArcaneFillFilePath(TEXT("T_UI_MainMenuArcaneFill_N.png")),
			GetMainMenuArcaneFillFilePath(TEXT("T_UI_MainMenuArcaneFill_H.png")),
			GetMainMenuArcaneFillFilePath(TEXT("T_UI_MainMenuArcaneFill_P.png")),
			GMainMenuArcaneFillFileNormal,
			GMainMenuArcaneFillFileHovered,
			GMainMenuArcaneFillFilePressed,
			TEXT("MainMenuArcane"));
	}

	if (Visual == ET66ButtonBackgroundVisual::MainMenuBlueWood)
	{
		LoadMainMenuBlueWoodFillTexturesOnce();
		return CreateFillBrushSetFromResources(
			GMainMenuBlueWoodFillNormal,
			GMainMenuBlueWoodFillHovered,
			GMainMenuBlueWoodFillPressed,
			GetMainMenuBlueWoodFillFilePath(TEXT("T_UI_MainMenuBlueWoodFill_N.png")),
			GetMainMenuBlueWoodFillFilePath(TEXT("T_UI_MainMenuBlueWoodFill_H.png")),
			GetMainMenuBlueWoodFillFilePath(TEXT("T_UI_MainMenuBlueWoodFill_P.png")),
			GMainMenuBlueWoodFillFileNormal,
			GMainMenuBlueWoodFillFileHovered,
			GMainMenuBlueWoodFillFilePressed,
			TEXT("MainMenuBlueWood"));
	}

	return nullptr;
}
