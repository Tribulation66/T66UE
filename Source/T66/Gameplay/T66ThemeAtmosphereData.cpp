// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ThemeAtmosphereData.h"

namespace
{
	static FT66ThemeAtmosphereSpec T66MakeNeutralAtmosphereSpec()
	{
		FT66ThemeAtmosphereSpec Spec;
		Spec.SkyLightColor = FLinearColor::White;
		Spec.SkyLightIntensity = 0.5f;
		Spec.FogDensity = 0.0f;
		Spec.FogHeightFalloff = 0.2f;
		Spec.FogInscatteringColor = FLinearColor::White;
		Spec.FogStartDistance = 400.0f;
		Spec.FogCutoffDistance = 20000.0f;
		Spec.ColorGradeShadowsTint = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
		Spec.ColorGradeMidtonesTint = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
		Spec.ColorGradeHighlightsTint = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
		Spec.ColorGradeSaturation = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
		Spec.ColorGradeContrast = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
		Spec.ColorGradeGain = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
		return Spec;
	}

	static FT66ThemeCelAtmosphere T66MakeDungeonCelAtmosphere()
	{
		FT66ThemeCelAtmosphere Cel;
		Cel.LightDirection = FVector(-0.4f, 0.6f, -0.7f).GetSafeNormal();
		Cel.LightColor = FLinearColor::White;
		Cel.RampStep1 = 0.0f;
		Cel.RampStep2 = 0.5f;
		Cel.ShadeColor = FLinearColor(0.35f, 0.38f, 0.50f, 1.0f);
		Cel.MidtoneColor = FLinearColor(0.7f, 0.7f, 0.72f, 1.0f);
		Cel.LitColor = FLinearColor::White;
		Cel.RimColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
		Cel.RimPower = 4.0f;
		Cel.RimStrength = 0.3f;
		Cel.OutlineColor = FLinearColor::Black;
		Cel.OutlineWidth = 1.5f;
		Cel.EnvShadeColor = FLinearColor(0.3f, 0.32f, 0.42f, 1.0f);
		Cel.EnvMidtoneColor = FLinearColor(0.55f, 0.58f, 0.62f, 1.0f);
		Cel.EnvLitColor = FLinearColor(0.85f, 0.85f, 0.90f, 1.0f);
		return Cel;
	}

	static FT66ThemeAtmosphereSpec T66MakeDungeonAtmosphereSpec()
	{
		FT66ThemeAtmosphereSpec Spec;
		Spec.SkyLightColor = FLinearColor::White;
		Spec.SkyLightIntensity = 0.0f;
		Spec.AmbientCubemap = TSoftObjectPtr<UTextureCube>(FSoftObjectPath(TEXT("/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap")));
		Spec.AmbientCubemapIntensity = 2.5f;
		Spec.AmbientCubemapTint = FLinearColor(0.45f, 0.6f, 0.9f, 1.0f);
		Spec.FogDensity = 0.018f;
		Spec.FogHeightFalloff = 0.2f;
		Spec.FogInscatteringColor = FLinearColor(0.18f, 0.26f, 0.45f, 1.0f);
		Spec.FogStartDistance = 400.0f;
		Spec.FogCutoffDistance = 20000.0f;
		Spec.ColorGradeShadowsTint = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
		Spec.ColorGradeMidtonesTint = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
		Spec.ColorGradeHighlightsTint = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
		Spec.ColorGradeSaturation = FVector4(0.95f, 0.95f, 0.95f, 1.0f);
		Spec.ColorGradeContrast = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
		Spec.ColorGradeGain = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
		Spec.TorchIntensity = 500.0f;
		Spec.TorchAttenuationRadius = 600.0f;
		Spec.TorchColor = FLinearColor(1.0f, 0.30f, 0.05f);
		Spec.TorchFalloffExponent = 2.0f;
		Spec.TorchVerticalOffset = 450.0f;
		Spec.TorchSpacingAlongWall = 1800.0f;
		Spec.TorchMinSeparation = 1400.0f;
		Spec.TorchMaxPerFloor = 200;
		Spec.CarryLightIntensity = 0.0f;
		Spec.CarryLightAttenuationRadius = 350.0f;
		Spec.CarryLightColor = FLinearColor(1.0f, 0.7f, 0.4f);
		Spec.CarryLightFalloffExponent = 1.5f;
		Spec.CarryLightVerticalOffset = 60.0f;
		Spec.CelAtmosphere = T66MakeDungeonCelAtmosphere();
		return Spec;
	}

	static FT66ThemeAtmosphereSpec T66MakeHellAtmosphereSpec()
	{
		FT66ThemeAtmosphereSpec Spec = T66MakeNeutralAtmosphereSpec();
		Spec.CelAtmosphere = T66MakeDungeonCelAtmosphere();
		Spec.CelAtmosphere.LightDirection = FVector(-0.25f, 0.45f, -0.86f).GetSafeNormal();
		Spec.CelAtmosphere.ShadeColor = FLinearColor(0.28f, 0.05f, 0.04f, 1.0f);
		Spec.CelAtmosphere.MidtoneColor = FLinearColor(0.65f, 0.22f, 0.12f, 1.0f);
		Spec.CelAtmosphere.LitColor = FLinearColor(1.0f, 0.68f, 0.35f, 1.0f);
		Spec.CelAtmosphere.RimColor = FLinearColor(1.0f, 0.38f, 0.05f, 1.0f);
		Spec.CelAtmosphere.OutlineColor = FLinearColor(0.04f, 0.0f, 0.0f, 1.0f);
		Spec.CelAtmosphere.EnvShadeColor = FLinearColor(0.18f, 0.04f, 0.04f, 1.0f);
		Spec.CelAtmosphere.EnvMidtoneColor = FLinearColor(0.45f, 0.13f, 0.09f, 1.0f);
		Spec.CelAtmosphere.EnvLitColor = FLinearColor(0.85f, 0.38f, 0.18f, 1.0f);
		return Spec;
	}

	static FT66ThemeAtmosphereSpec T66MakeOceanAtmosphereSpec()
	{
		FT66ThemeAtmosphereSpec Spec = T66MakeNeutralAtmosphereSpec();
		Spec.CelAtmosphere = T66MakeDungeonCelAtmosphere();
		Spec.CelAtmosphere.LightDirection = FVector(-0.2f, 0.35f, -0.92f).GetSafeNormal();
		Spec.CelAtmosphere.ShadeColor = FLinearColor(0.05f, 0.22f, 0.28f, 1.0f);
		Spec.CelAtmosphere.MidtoneColor = FLinearColor(0.35f, 0.62f, 0.70f, 1.0f);
		Spec.CelAtmosphere.LitColor = FLinearColor(0.85f, 0.98f, 1.0f, 1.0f);
		Spec.CelAtmosphere.RimColor = FLinearColor(0.35f, 0.95f, 1.0f, 1.0f);
		Spec.CelAtmosphere.OutlineColor = FLinearColor(0.0f, 0.02f, 0.08f, 1.0f);
		Spec.CelAtmosphere.EnvShadeColor = FLinearColor(0.04f, 0.16f, 0.21f, 1.0f);
		Spec.CelAtmosphere.EnvMidtoneColor = FLinearColor(0.22f, 0.45f, 0.53f, 1.0f);
		Spec.CelAtmosphere.EnvLitColor = FLinearColor(0.62f, 0.84f, 0.9f, 1.0f);
		return Spec;
	}

	static FT66ThemeAtmosphereSpec T66MakeMartianAtmosphereSpec()
	{
		FT66ThemeAtmosphereSpec Spec = T66MakeNeutralAtmosphereSpec();
		Spec.CelAtmosphere = T66MakeDungeonCelAtmosphere();
		Spec.CelAtmosphere.LightDirection = FVector(-0.45f, 0.25f, -0.86f).GetSafeNormal();
		Spec.CelAtmosphere.ShadeColor = FLinearColor(0.34f, 0.13f, 0.07f, 1.0f);
		Spec.CelAtmosphere.MidtoneColor = FLinearColor(0.68f, 0.36f, 0.18f, 1.0f);
		Spec.CelAtmosphere.LitColor = FLinearColor(1.0f, 0.74f, 0.48f, 1.0f);
		Spec.CelAtmosphere.RimColor = FLinearColor(1.0f, 0.55f, 0.22f, 1.0f);
		Spec.CelAtmosphere.EnvShadeColor = FLinearColor(0.24f, 0.09f, 0.05f, 1.0f);
		Spec.CelAtmosphere.EnvMidtoneColor = FLinearColor(0.52f, 0.25f, 0.14f, 1.0f);
		Spec.CelAtmosphere.EnvLitColor = FLinearColor(0.86f, 0.55f, 0.35f, 1.0f);
		return Spec;
	}

	static FT66ThemeAtmosphereSpec T66MakeForestAtmosphereSpec()
	{
		FT66ThemeAtmosphereSpec Spec = T66MakeNeutralAtmosphereSpec();
		Spec.CelAtmosphere = T66MakeDungeonCelAtmosphere();
		Spec.CelAtmosphere.LightDirection = FVector(-0.35f, 0.5f, -0.79f).GetSafeNormal();
		Spec.CelAtmosphere.ShadeColor = FLinearColor(0.08f, 0.22f, 0.10f, 1.0f);
		Spec.CelAtmosphere.MidtoneColor = FLinearColor(0.42f, 0.60f, 0.34f, 1.0f);
		Spec.CelAtmosphere.LitColor = FLinearColor(0.88f, 0.96f, 0.76f, 1.0f);
		Spec.CelAtmosphere.RimColor = FLinearColor(0.78f, 1.0f, 0.50f, 1.0f);
		Spec.CelAtmosphere.EnvShadeColor = FLinearColor(0.07f, 0.18f, 0.08f, 1.0f);
		Spec.CelAtmosphere.EnvMidtoneColor = FLinearColor(0.32f, 0.48f, 0.28f, 1.0f);
		Spec.CelAtmosphere.EnvLitColor = FLinearColor(0.70f, 0.82f, 0.58f, 1.0f);
		return Spec;
	}
}

const FT66ThemeAtmosphereSpec& T66ThemeAtmosphereData::GetSpecForTheme(const T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme)
{
	static const FT66ThemeAtmosphereSpec DungeonSpec = T66MakeDungeonAtmosphereSpec();
	static const FT66ThemeAtmosphereSpec NeutralSpec = T66MakeNeutralAtmosphereSpec();
	static const FT66ThemeAtmosphereSpec HellSpec = T66MakeHellAtmosphereSpec();
	static const FT66ThemeAtmosphereSpec OceanSpec = T66MakeOceanAtmosphereSpec();
	static const FT66ThemeAtmosphereSpec MartianSpec = T66MakeMartianAtmosphereSpec();
	static const FT66ThemeAtmosphereSpec ForestSpec = T66MakeForestAtmosphereSpec();

	switch (Theme)
	{
	case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Dungeon:
		return DungeonSpec;
	case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Forest:
		return ForestSpec;
	case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Ocean:
		return OceanSpec;
	case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Martian:
		return MartianSpec;
	case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Hell:
		return HellSpec;
	default:
		return NeutralSpec;
	}
}
