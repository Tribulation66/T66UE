// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66IdolProcVFX.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/T66VisualUtil.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "DrawDebugHelpers.h"

namespace
{
	enum class ET66IdolFlavor : uint8
	{
		Curse,
		Lava,
		Poison,
		Decay,
		Bleed,
		Acid,
		Electric,
		Ice,
		Sound,
		Shadow,
		Star,
		Rubber,
		Fire,
		Earth,
		Water,
		Sand,
		BlackHole,
		Storm,
		Light,
		Wind,
		Steel,
		Wood,
		Bone,
		Glass,
	};

	struct FT66IdolThemeSpec
	{
		ET66IdolFlavor Flavor = ET66IdolFlavor::Light;
		FLinearColor PrimaryColor = FLinearColor::White;
		FLinearColor SecondaryColor = FLinearColor(0.35f, 0.35f, 0.35f, 1.f);
		FLinearColor AccentColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.f);
		FLinearColor OutlineColor = FLinearColor::Black;
		ET66IdolVFXTexture PrimaryTexture = ET66IdolVFXTexture::Runes;
		ET66IdolVFXTexture AccentTexture = ET66IdolVFXTexture::Seals;
		float PixelGrid = 28.f;
		float DetailTiling = 2.f;
		float ScrollSpeed = 0.35f;
		float GlowStrength = 4.f;
		float BaseWidth = 0.2f;
		float ImpactScale = 0.55f;
		float RotationSpeed = 30.f;
	};

	struct FT66IdolRaritySpec
	{
		int32 ExtraLayers = 0;
		float WidthScale = 1.f;
		float GlowScale = 1.f;
		float ImpactScale = 1.f;
		float LifetimeScale = 1.f;
		float PixelGridScale = 1.f;
		float DetailTilingScale = 1.f;
		float Brightness = 0.f;
		float OpacityBoost = 0.9f;
	};

	FVector MultiplyComponents(const FVector& A, const FVector& B)
	{
		return FVector(A.X * B.X, A.Y * B.Y, A.Z * B.Z);
	}

	FLinearColor Brighten(const FLinearColor& Color, float Alpha)
	{
		return FLinearColor(
			FMath::Lerp(Color.R, 1.f, Alpha),
			FMath::Lerp(Color.G, 1.f, Alpha),
			FMath::Lerp(Color.B, 1.f, Alpha),
			Color.A);
	}

	FT66IdolThemeSpec MakeTheme(
		const ET66IdolFlavor Flavor,
		const FLinearColor& PrimaryColor,
		const FLinearColor& SecondaryColor,
		const FLinearColor& AccentColor,
		const FLinearColor& OutlineColor,
		const ET66IdolVFXTexture PrimaryTexture,
		const ET66IdolVFXTexture AccentTexture,
		const float PixelGrid,
		const float DetailTiling,
		const float ScrollSpeed,
		const float GlowStrength,
		const float BaseWidth,
		const float ImpactScale,
		const float RotationSpeed)
	{
		FT66IdolThemeSpec Theme;
		Theme.Flavor = Flavor;
		Theme.PrimaryColor = PrimaryColor;
		Theme.SecondaryColor = SecondaryColor;
		Theme.AccentColor = AccentColor;
		Theme.OutlineColor = OutlineColor;
		Theme.PrimaryTexture = PrimaryTexture;
		Theme.AccentTexture = AccentTexture;
		Theme.PixelGrid = PixelGrid;
		Theme.DetailTiling = DetailTiling;
		Theme.ScrollSpeed = ScrollSpeed;
		Theme.GlowStrength = GlowStrength;
		Theme.BaseWidth = BaseWidth;
		Theme.ImpactScale = ImpactScale;
		Theme.RotationSpeed = RotationSpeed;
		return Theme;
	}

	FT66IdolThemeSpec ResolveTheme(const FName IdolID)
	{
		if (IdolID == TEXT("Idol_Curse"))
		{
			return MakeTheme(ET66IdolFlavor::Curse, FLinearColor(0.87f, 0.48f, 1.00f, 1.f), FLinearColor(0.28f, 0.07f, 0.38f, 1.f), FLinearColor(0.98f, 0.88f, 1.00f, 1.f), FLinearColor(0.02f, 0.00f, 0.04f, 1.f), ET66IdolVFXTexture::Runes, ET66IdolVFXTexture::Seals, 30.f, 3.2f, 0.22f, 4.5f, 0.18f, 0.62f, 48.f);
		}
		if (IdolID == TEXT("Idol_Lava"))
		{
			return MakeTheme(ET66IdolFlavor::Lava, FLinearColor(1.00f, 0.45f, 0.14f, 1.f), FLinearColor(0.45f, 0.12f, 0.04f, 1.f), FLinearColor(1.00f, 0.82f, 0.25f, 1.f), FLinearColor(0.10f, 0.01f, 0.00f, 1.f), ET66IdolVFXTexture::Wall, ET66IdolVFXTexture::Seals, 27.f, 2.2f, 0.48f, 4.8f, 0.23f, 0.64f, 34.f);
		}
		if (IdolID == TEXT("Idol_Poison"))
		{
			return MakeTheme(ET66IdolFlavor::Poison, FLinearColor(0.48f, 1.00f, 0.42f, 1.f), FLinearColor(0.12f, 0.36f, 0.08f, 1.f), FLinearColor(0.88f, 1.00f, 0.74f, 1.f), FLinearColor(0.00f, 0.05f, 0.00f, 1.f), ET66IdolVFXTexture::Seals, ET66IdolVFXTexture::Runes, 28.f, 2.9f, 0.18f, 4.2f, 0.19f, 0.60f, 44.f);
		}
		if (IdolID == TEXT("Idol_Decay"))
		{
			return MakeTheme(ET66IdolFlavor::Decay, FLinearColor(0.72f, 0.68f, 0.36f, 1.f), FLinearColor(0.24f, 0.23f, 0.10f, 1.f), FLinearColor(0.58f, 0.84f, 0.34f, 1.f), FLinearColor(0.03f, 0.02f, 0.00f, 1.f), ET66IdolVFXTexture::Wall, ET66IdolVFXTexture::Runes, 24.f, 1.8f, 0.12f, 3.8f, 0.21f, 0.57f, 21.f);
		}
		if (IdolID == TEXT("Idol_Bleed"))
		{
			return MakeTheme(ET66IdolFlavor::Bleed, FLinearColor(0.96f, 0.18f, 0.18f, 1.f), FLinearColor(0.36f, 0.02f, 0.03f, 1.f), FLinearColor(1.00f, 0.78f, 0.78f, 1.f), FLinearColor(0.05f, 0.00f, 0.00f, 1.f), ET66IdolVFXTexture::Wall, ET66IdolVFXTexture::Seals, 26.f, 2.1f, 0.30f, 4.4f, 0.20f, 0.58f, 55.f);
		}
		if (IdolID == TEXT("Idol_Acid"))
		{
			return MakeTheme(ET66IdolFlavor::Acid, FLinearColor(0.86f, 1.00f, 0.24f, 1.f), FLinearColor(0.20f, 0.38f, 0.02f, 1.f), FLinearColor(0.98f, 1.00f, 0.72f, 1.f), FLinearColor(0.02f, 0.03f, 0.00f, 1.f), ET66IdolVFXTexture::Seals, ET66IdolVFXTexture::Wall, 29.f, 3.5f, 0.26f, 4.9f, 0.20f, 0.63f, 60.f);
		}
		if (IdolID == TEXT("Idol_Electric"))
		{
			return MakeTheme(ET66IdolFlavor::Electric, FLinearColor(0.82f, 0.38f, 1.00f, 1.f), FLinearColor(0.28f, 0.07f, 0.45f, 1.f), FLinearColor(0.92f, 0.90f, 1.00f, 1.f), FLinearColor(0.03f, 0.00f, 0.06f, 1.f), ET66IdolVFXTexture::Runes, ET66IdolVFXTexture::Seals, 31.f, 4.0f, 1.15f, 5.0f, 0.15f, 0.48f, 72.f);
		}
		if (IdolID == TEXT("Idol_Ice"))
		{
			return MakeTheme(ET66IdolFlavor::Ice, FLinearColor(0.58f, 0.88f, 1.00f, 1.f), FLinearColor(0.18f, 0.38f, 0.58f, 1.f), FLinearColor(0.94f, 0.98f, 1.00f, 1.f), FLinearColor(0.00f, 0.03f, 0.06f, 1.f), ET66IdolVFXTexture::Wall, ET66IdolVFXTexture::Seals, 32.f, 3.2f, 0.42f, 4.5f, 0.13f, 0.52f, 28.f);
		}
		if (IdolID == TEXT("Idol_Sound"))
		{
			return MakeTheme(ET66IdolFlavor::Sound, FLinearColor(0.94f, 0.78f, 1.00f, 1.f), FLinearColor(0.34f, 0.18f, 0.40f, 1.f), FLinearColor(1.00f, 0.98f, 1.00f, 1.f), FLinearColor(0.04f, 0.00f, 0.04f, 1.f), ET66IdolVFXTexture::Seals, ET66IdolVFXTexture::Runes, 26.f, 1.6f, 0.12f, 4.0f, 0.24f, 0.54f, 46.f);
		}
		if (IdolID == TEXT("Idol_Shadow"))
		{
			return MakeTheme(ET66IdolFlavor::Shadow, FLinearColor(0.18f, 0.16f, 0.26f, 1.f), FLinearColor(0.03f, 0.02f, 0.04f, 1.f), FLinearColor(0.65f, 0.38f, 0.90f, 1.f), FLinearColor(0.20f, 0.10f, 0.28f, 1.f), ET66IdolVFXTexture::Runes, ET66IdolVFXTexture::Wall, 25.f, 2.5f, 0.30f, 4.1f, 0.18f, 0.50f, 24.f);
		}
		if (IdolID == TEXT("Idol_Star"))
		{
			return MakeTheme(ET66IdolFlavor::Star, FLinearColor(1.00f, 0.94f, 0.48f, 1.f), FLinearColor(0.46f, 0.28f, 0.06f, 1.f), FLinearColor(1.00f, 1.00f, 0.92f, 1.f), FLinearColor(0.06f, 0.03f, 0.00f, 1.f), ET66IdolVFXTexture::Seals, ET66IdolVFXTexture::Runes, 30.f, 2.3f, 0.58f, 5.2f, 0.16f, 0.50f, 67.f);
		}
		if (IdolID == TEXT("Idol_Rubber"))
		{
			return MakeTheme(ET66IdolFlavor::Rubber, FLinearColor(0.95f, 0.58f, 0.24f, 1.f), FLinearColor(0.48f, 0.22f, 0.06f, 1.f), FLinearColor(1.00f, 0.86f, 0.60f, 1.f), FLinearColor(0.05f, 0.02f, 0.00f, 1.f), ET66IdolVFXTexture::Wall, ET66IdolVFXTexture::Seals, 23.f, 1.4f, 0.10f, 3.7f, 0.28f, 0.56f, 36.f);
		}
		if (IdolID == TEXT("Idol_Fire"))
		{
			return MakeTheme(ET66IdolFlavor::Fire, FLinearColor(1.00f, 0.38f, 0.10f, 1.f), FLinearColor(0.46f, 0.09f, 0.02f, 1.f), FLinearColor(1.00f, 0.92f, 0.32f, 1.f), FLinearColor(0.08f, 0.01f, 0.00f, 1.f), ET66IdolVFXTexture::Wall, ET66IdolVFXTexture::Seals, 26.f, 2.6f, 0.64f, 5.3f, 0.24f, 0.78f, 44.f);
		}
		if (IdolID == TEXT("Idol_Earth"))
		{
			return MakeTheme(ET66IdolFlavor::Earth, FLinearColor(0.70f, 0.52f, 0.30f, 1.f), FLinearColor(0.26f, 0.18f, 0.08f, 1.f), FLinearColor(0.88f, 0.78f, 0.58f, 1.f), FLinearColor(0.03f, 0.02f, 0.00f, 1.f), ET66IdolVFXTexture::Wall, ET66IdolVFXTexture::Runes, 22.f, 1.3f, 0.10f, 3.5f, 0.31f, 0.74f, 18.f);
		}
		if (IdolID == TEXT("Idol_Water"))
		{
			return MakeTheme(ET66IdolFlavor::Water, FLinearColor(0.34f, 0.72f, 1.00f, 1.f), FLinearColor(0.09f, 0.22f, 0.46f, 1.f), FLinearColor(0.88f, 0.98f, 1.00f, 1.f), FLinearColor(0.00f, 0.03f, 0.08f, 1.f), ET66IdolVFXTexture::Seals, ET66IdolVFXTexture::Runes, 29.f, 2.0f, 0.20f, 4.3f, 0.23f, 0.76f, 32.f);
		}
		if (IdolID == TEXT("Idol_Sand"))
		{
			return MakeTheme(ET66IdolFlavor::Sand, FLinearColor(0.96f, 0.88f, 0.58f, 1.f), FLinearColor(0.42f, 0.34f, 0.18f, 1.f), FLinearColor(1.00f, 0.96f, 0.82f, 1.f), FLinearColor(0.06f, 0.04f, 0.01f, 1.f), ET66IdolVFXTexture::Wall, ET66IdolVFXTexture::Seals, 24.f, 1.5f, 0.14f, 3.8f, 0.27f, 0.72f, 26.f);
		}
		if (IdolID == TEXT("Idol_BlackHole"))
		{
			return MakeTheme(ET66IdolFlavor::BlackHole, FLinearColor(0.14f, 0.08f, 0.22f, 1.f), FLinearColor(0.03f, 0.01f, 0.05f, 1.f), FLinearColor(0.72f, 0.30f, 0.98f, 1.f), FLinearColor(0.26f, 0.09f, 0.34f, 1.f), ET66IdolVFXTexture::Runes, ET66IdolVFXTexture::Seals, 34.f, 3.9f, 0.22f, 4.8f, 0.19f, 0.80f, 70.f);
		}
		if (IdolID == TEXT("Idol_Storm"))
		{
			return MakeTheme(ET66IdolFlavor::Storm, FLinearColor(0.56f, 0.66f, 0.86f, 1.f), FLinearColor(0.16f, 0.18f, 0.28f, 1.f), FLinearColor(0.92f, 0.96f, 1.00f, 1.f), FLinearColor(0.01f, 0.02f, 0.04f, 1.f), ET66IdolVFXTexture::Runes, ET66IdolVFXTexture::Seals, 28.f, 2.8f, 0.52f, 4.7f, 0.22f, 0.77f, 58.f);
		}
		if (IdolID == TEXT("Idol_Light"))
		{
			return MakeTheme(ET66IdolFlavor::Light, FLinearColor(0.98f, 0.98f, 1.00f, 1.f), FLinearColor(0.74f, 0.78f, 0.94f, 1.f), FLinearColor(1.00f, 0.96f, 0.82f, 1.f), FLinearColor(0.10f, 0.10f, 0.14f, 1.f), ET66IdolVFXTexture::Seals, ET66IdolVFXTexture::Runes, 34.f, 3.3f, 0.30f, 5.4f, 0.13f, 0.60f, 52.f);
		}
		if (IdolID == TEXT("Idol_Wind"))
		{
			return MakeTheme(ET66IdolFlavor::Wind, FLinearColor(0.72f, 0.96f, 0.88f, 1.f), FLinearColor(0.18f, 0.34f, 0.26f, 1.f), FLinearColor(0.94f, 1.00f, 0.96f, 1.f), FLinearColor(0.00f, 0.04f, 0.03f, 1.f), ET66IdolVFXTexture::Runes, ET66IdolVFXTexture::Seals, 30.f, 2.5f, 0.78f, 4.6f, 0.12f, 0.52f, 76.f);
		}
		if (IdolID == TEXT("Idol_Steel"))
		{
			return MakeTheme(ET66IdolFlavor::Steel, FLinearColor(0.70f, 0.74f, 0.92f, 1.f), FLinearColor(0.20f, 0.24f, 0.34f, 1.f), FLinearColor(0.94f, 0.98f, 1.00f, 1.f), FLinearColor(0.02f, 0.02f, 0.03f, 1.f), ET66IdolVFXTexture::Wall, ET66IdolVFXTexture::Seals, 27.f, 1.7f, 0.20f, 4.1f, 0.15f, 0.50f, 14.f);
		}
		if (IdolID == TEXT("Idol_Wood"))
		{
			return MakeTheme(ET66IdolFlavor::Wood, FLinearColor(0.46f, 0.78f, 0.30f, 1.f), FLinearColor(0.16f, 0.28f, 0.08f, 1.f), FLinearColor(0.86f, 0.96f, 0.72f, 1.f), FLinearColor(0.03f, 0.03f, 0.00f, 1.f), ET66IdolVFXTexture::Wall, ET66IdolVFXTexture::Runes, 25.f, 2.0f, 0.26f, 4.0f, 0.17f, 0.51f, 30.f);
		}
		if (IdolID == TEXT("Idol_Bone"))
		{
			return MakeTheme(ET66IdolFlavor::Bone, FLinearColor(0.96f, 0.93f, 0.84f, 1.f), FLinearColor(0.40f, 0.30f, 0.20f, 1.f), FLinearColor(1.00f, 0.98f, 0.94f, 1.f), FLinearColor(0.05f, 0.03f, 0.01f, 1.f), ET66IdolVFXTexture::Wall, ET66IdolVFXTexture::Seals, 24.f, 1.9f, 0.18f, 4.0f, 0.18f, 0.56f, 20.f);
		}
		if (IdolID == TEXT("Idol_Glass"))
		{
			return MakeTheme(ET66IdolFlavor::Glass, FLinearColor(0.90f, 0.98f, 1.00f, 1.f), FLinearColor(0.28f, 0.50f, 0.56f, 1.f), FLinearColor(0.86f, 1.00f, 1.00f, 1.f), FLinearColor(0.01f, 0.03f, 0.04f, 1.f), ET66IdolVFXTexture::Seals, ET66IdolVFXTexture::Wall, 33.f, 3.7f, 0.64f, 5.1f, 0.11f, 0.54f, 64.f);
		}

		return MakeTheme(ET66IdolFlavor::Light, FLinearColor::White, FLinearColor(0.3f, 0.3f, 0.35f, 1.f), FLinearColor(0.8f, 0.8f, 0.9f, 1.f), FLinearColor::Black, ET66IdolVFXTexture::Runes, ET66IdolVFXTexture::Seals, 28.f, 2.f, 0.3f, 4.f, 0.2f, 0.55f, 30.f);
	}

	FT66IdolRaritySpec ResolveRarity(const ET66ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black:
			return {0, 0.82f, 0.80f, 0.84f, 0.90f, 0.88f, 0.90f, 0.00f, 0.80f};
		case ET66ItemRarity::Red:
			return {1, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 0.05f, 0.92f};
		case ET66ItemRarity::Yellow:
			return {2, 1.12f, 1.18f, 1.16f, 1.08f, 1.10f, 1.12f, 0.12f, 1.00f};
		case ET66ItemRarity::White:
			return {3, 1.26f, 1.42f, 1.34f, 1.16f, 1.20f, 1.22f, 0.20f, 1.08f};
		default:
			return {0, 0.82f, 0.80f, 0.84f, 0.90f, 0.88f, 0.90f, 0.00f, 0.80f};
		}
	}

float GetBaseLifetime(const ET66AttackCategory Category)
{
	switch (Category)
	{
	case ET66AttackCategory::Pierce:
		return 0.18f;
	case ET66AttackCategory::AOE:
		return 0.32f;
	case ET66AttackCategory::Bounce:
		return 0.24f;
	case ET66AttackCategory::DOT:
		return 2.5f;
	default:
		return 0.20f;
	}
}

const TCHAR* GetIdolVFXCategoryName(const ET66AttackCategory Category)
{
	switch (Category)
	{
	case ET66AttackCategory::Pierce: return TEXT("Pierce");
	case ET66AttackCategory::AOE:    return TEXT("AOE");
	case ET66AttackCategory::Bounce: return TEXT("Bounce");
	case ET66AttackCategory::DOT:    return TEXT("DOT");
	default:                         return TEXT("Unknown");
	}
}
}

AT66IdolProcVFX::AT66IdolProcVFX()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	InitialLifeSpan = 5.f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
}

void AT66IdolProcVFX::InitEffect(const FT66IdolProcVFXRequest& InRequest)
{
	Request = InRequest;
	bInitialized = true;

	if (HasActorBegunPlay() && !bBuilt)
	{
		BuildEffect();
	}
}

void AT66IdolProcVFX::BeginPlay()
{
	Super::BeginPlay();

	if (bInitialized && !bBuilt)
	{
		BuildEffect();
	}
}

void AT66IdolProcVFX::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ElapsedSeconds += DeltaSeconds;
	UpdateFollowTarget();

	const float ActiveElapsed = ElapsedSeconds - Request.StartDelay;
	if (ActiveElapsed < 0.f)
	{
		for (const FT66IdolVFXLayerAnimState& Layer : LayerStates)
		{
			if (Layer.Mesh)
			{
				Layer.Mesh->SetVisibility(false, true);
			}
		}
		return;
	}

	const float ActiveAge01 = FMath::Clamp(ActiveElapsed / FMath::Max(0.01f, LifetimeSeconds), 0.f, 1.f);
	UpdateLayers(ActiveElapsed, ActiveAge01);

	if (ActiveAge01 >= 1.f)
	{
		Destroy();
	}
}

void AT66IdolProcVFX::ResolveAssets()
{
	if (!PlaneMesh)
	{
		PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	}

	if (!BaseMaterial)
	{
		BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/VFX/Idols/Shared/MI_IdolPixelPortal_Base.MI_IdolPixelPortal_Base"));
		if (!BaseMaterial)
		{
			BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/VFX/Hero1/M_Hero1_PixelAttack.M_Hero1_PixelAttack"));
		}
	}

	if (!RunesTexture)
	{
		RunesTexture = LoadObject<UTexture>(nullptr, TEXT("/Game/VFX/Idols/Shared/T_IdolVFX_Runes.T_IdolVFX_Runes"));
		if (!RunesTexture)
		{
			RunesTexture = LoadObject<UTexture>(nullptr, TEXT("/Game/VFX/Hero1/T_Hero1_Attack_Runes.T_Hero1_Attack_Runes"));
		}
	}

	if (!SealsTexture)
	{
		SealsTexture = LoadObject<UTexture>(nullptr, TEXT("/Game/VFX/Idols/Shared/T_IdolVFX_Seals.T_IdolVFX_Seals"));
		if (!SealsTexture)
		{
			SealsTexture = LoadObject<UTexture>(nullptr, TEXT("/Game/VFX/Hero1/T_Hero1_Attack_Seals.T_Hero1_Attack_Seals"));
		}
	}

	if (!WallTexture)
	{
		WallTexture = LoadObject<UTexture>(nullptr, TEXT("/Game/VFX/Idols/Shared/T_IdolVFX_Wall.T_IdolVFX_Wall"));
		if (!WallTexture)
		{
			WallTexture = LoadObject<UTexture>(nullptr, TEXT("/Game/VFX/Hero1/T_Hero1_Attack_Wall.T_Hero1_Attack_Wall"));
		}
	}

	if (!PlaneMesh || !BaseMaterial || !RunesTexture || !SealsTexture || !WallTexture)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[IDOL VFX ACTOR] ResolveAssets failed actor=%s plane=%d material=%d runes=%d seals=%d wall=%d"),
			*GetName(),
			PlaneMesh != nullptr,
			BaseMaterial != nullptr,
			RunesTexture != nullptr,
			SealsTexture != nullptr,
			WallTexture != nullptr);
	}
}

UTexture* AT66IdolProcVFX::ResolveTexture(const ET66IdolVFXTexture Texture) const
{
	switch (Texture)
	{
	case ET66IdolVFXTexture::Runes:
		return RunesTexture;
	case ET66IdolVFXTexture::Seals:
		return SealsTexture;
	case ET66IdolVFXTexture::Wall:
		return WallTexture;
	default:
		return RunesTexture;
	}
}

void AT66IdolProcVFX::AddLayer(
	const FName& LayerName,
	const FVector& RelativeLocation,
	const FRotator& RelativeRotation,
	const FVector& RelativeScale,
	const FT66IdolVFXLayerMaterial& MaterialParams,
	const FT66IdolVFXLayerAnimState& AnimTemplate,
	const int32 SortPriority)
{
	if (!PlaneMesh || !BaseMaterial)
	{
		return;
	}

	UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(this, LayerName);
	if (!Mesh)
	{
		return;
	}

	AddInstanceComponent(Mesh);
	Mesh->SetupAttachment(SceneRoot);
	Mesh->SetStaticMesh(PlaneMesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCastShadow(false);
	Mesh->SetReceivesDecals(false);
	Mesh->SetCanEverAffectNavigation(false);
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetHiddenInGame(false, true);
	Mesh->SetVisibility(true, true);
	Mesh->SetRenderInMainPass(true);
	Mesh->SetRenderInDepthPass(true);
	Mesh->TranslucencySortPriority = 20 + SortPriority;
	Mesh->SetRelativeLocation(RelativeLocation);
	Mesh->SetRelativeRotation(RelativeRotation);
	Mesh->SetRelativeScale3D(RelativeScale);
	Mesh->RegisterComponent();

	const FName MIDName(*FString::Printf(TEXT("%s_MID"), *LayerName.ToString()));
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, this, MIDName);
	if (MID)
	{
		if (UTexture* DetailTexture = ResolveTexture(MaterialParams.Texture))
		{
			MID->SetTextureParameterValue(TEXT("DetailTexture"), DetailTexture);
		}
		MID->SetVectorParameterValue(TEXT("PrimaryColor"), MaterialParams.PrimaryColor);
		MID->SetVectorParameterValue(TEXT("SecondaryColor"), MaterialParams.SecondaryColor);
		MID->SetVectorParameterValue(TEXT("OutlineColor"), MaterialParams.OutlineColor);
		MID->SetVectorParameterValue(TEXT("TintColor"), MaterialParams.TintColor);
		MID->SetScalarParameterValue(TEXT("PixelGrid"), MaterialParams.PixelGrid);
		MID->SetScalarParameterValue(TEXT("DetailTiling"), MaterialParams.DetailTiling);
		MID->SetScalarParameterValue(TEXT("ScrollSpeed"), MaterialParams.ScrollSpeed);
		MID->SetScalarParameterValue(TEXT("InnerRadius"), MaterialParams.InnerRadius);
		MID->SetScalarParameterValue(TEXT("OuterRadius"), MaterialParams.OuterRadius);
		MID->SetScalarParameterValue(TEXT("GlowStrength"), MaterialParams.GlowStrength);
		MID->SetScalarParameterValue(TEXT("OpacityBoost"), MaterialParams.OpacityBoost);
		MID->SetScalarParameterValue(TEXT("Age01"), 0.f);
		Mesh->SetMaterial(0, MID);
	}
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[IDOL VFX ACTOR] Layer actor=%s name=%s relLoc=%s relScale=%s mid=%d"),
		*GetName(),
		*LayerName.ToString(),
		*RelativeLocation.ToCompactString(),
		*RelativeScale.ToCompactString(),
		MID != nullptr);

	SpawnedMeshes.Add(Mesh);
	SpawnedMIDs.Add(MID);

	FT66IdolVFXLayerAnimState LayerState = AnimTemplate;
	LayerState.Mesh = Mesh;
	LayerState.MID = MID;
	LayerState.BaseLocation = RelativeLocation;
	LayerState.BaseRotation = RelativeRotation;
	LayerState.BaseScale = RelativeScale;
	LayerStates.Add(LayerState);

	const FString LayerNameString = LayerName.ToString();
	const bool bAddSolidSilhouette =
		LayerNameString.Contains(TEXT("Core")) ||
		LayerNameString.Contains(TEXT("Impact")) ||
		LayerNameString.Contains(TEXT("Pulse"));
	if (bAddSolidSilhouette)
	{
		const FName SolidName(*FString::Printf(TEXT("%s_Solid"), *LayerNameString));
		UStaticMeshComponent* SolidMesh = NewObject<UStaticMeshComponent>(this, SolidName);
		if (SolidMesh)
		{
			AddInstanceComponent(SolidMesh);
			SolidMesh->SetupAttachment(SceneRoot);
			SolidMesh->SetStaticMesh(PlaneMesh);
			SolidMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SolidMesh->SetCastShadow(false);
			SolidMesh->SetReceivesDecals(false);
			SolidMesh->SetCanEverAffectNavigation(false);
			SolidMesh->SetMobility(EComponentMobility::Movable);
			SolidMesh->SetHiddenInGame(false, true);
			SolidMesh->SetVisibility(true, true);
			SolidMesh->SetRenderInMainPass(true);
			SolidMesh->SetRenderInDepthPass(true);
			SolidMesh->TranslucencySortPriority = 10 + SortPriority;
			const FVector SolidScale = FVector(
				FMath::Max(0.04f, RelativeScale.X * 0.86f),
				FMath::Max(0.04f, RelativeScale.Y * 0.86f),
				FMath::Max(0.04f, RelativeScale.Z));
			SolidMesh->SetRelativeLocation(RelativeLocation + FVector(0.f, 0.f, -1.5f));
			SolidMesh->SetRelativeRotation(RelativeRotation);
			SolidMesh->SetRelativeScale3D(SolidScale);
			SolidMesh->RegisterComponent();
			FT66VisualUtil::ApplyT66Color(SolidMesh, this, MaterialParams.PrimaryColor);

			SpawnedMeshes.Add(SolidMesh);

			FT66IdolVFXLayerAnimState SolidState = AnimTemplate;
			SolidState.Mesh = SolidMesh;
			SolidState.MID = nullptr;
			SolidState.BaseLocation = RelativeLocation + FVector(0.f, 0.f, -1.5f);
			SolidState.BaseRotation = RelativeRotation;
			SolidState.BaseScale = SolidScale;
			LayerStates.Add(SolidState);

			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[IDOL VFX ACTOR] SolidLayer actor=%s name=%s relLoc=%s relScale=%s"),
				*GetName(),
				*SolidName.ToString(),
				*SolidState.BaseLocation.ToCompactString(),
				*SolidScale.ToCompactString());
		}
	}
}

void AT66IdolProcVFX::BuildEffect()
{
	if (!bInitialized || bBuilt)
	{
		return;
	}

	ResolveAssets();
	if (!PlaneMesh || !BaseMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("[IDOL VFX ACTOR] BuildEffect aborted actor=%s idol=%s category=%s plane=%d material=%d"), *GetName(), *Request.IdolID.ToString(), GetIdolVFXCategoryName(Request.Category), PlaneMesh != nullptr, BaseMaterial != nullptr);
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("[IDOL VFX ACTOR] BuildEffect start actor=%s idol=%s category=%s delay=%.3f"), *GetName(), *Request.IdolID.ToString(), GetIdolVFXCategoryName(Request.Category), Request.StartDelay);

	const FT66IdolRaritySpec Tier = ResolveRarity(Request.Rarity);
	if (Request.Category == ET66AttackCategory::DOT)
	{
		LifetimeSeconds = FMath::Max(0.75f, Request.Duration) * Tier.LifetimeScale;
	}
	else
	{
		LifetimeSeconds = GetBaseLifetime(Request.Category) * Tier.LifetimeScale;
	}
	InitialLifeSpan = Request.StartDelay + LifetimeSeconds + 0.4f;

	switch (Request.Category)
	{
	case ET66AttackCategory::Pierce:
		{
			const FVector FlatDir = FVector(Request.End.X - Request.Start.X, Request.End.Y - Request.Start.Y, 0.f).GetSafeNormal();
			SetActorLocation(Request.Start + FVector(0.f, 0.f, 8.f));
			SetActorRotation(FlatDir.IsNearlyZero() ? FRotator::ZeroRotator : FlatDir.Rotation());
		}
		BuildPierceEffect();
		break;
	case ET66AttackCategory::AOE:
		SetActorLocation(Request.Center + FVector(0.f, 0.f, 8.f));
		SetActorRotation(FRotator::ZeroRotator);
		BuildAOEEffect();
		break;
	case ET66AttackCategory::Bounce:
		SetActorLocation((Request.ChainPositions.Num() > 0 ? Request.ChainPositions[0] : Request.Start) + FVector(0.f, 0.f, 10.f));
		SetActorRotation(FRotator::ZeroRotator);
		BuildBounceEffect();
		break;
	case ET66AttackCategory::DOT:
		SetActorLocation((Request.FollowTarget.IsValid() ? Request.FollowTarget->GetActorLocation() : Request.Center) + FVector(0.f, 0.f, 34.f));
		SetActorRotation(FRotator::ZeroRotator);
		BuildDOTEffect();
		break;
	default:
		SetActorLocation(Request.Center + FVector(0.f, 0.f, 28.f));
		SetActorRotation(FRotator::ZeroRotator);
		BuildFallbackEffect();
		break;
	}

	if (LayerStates.Num() == 0)
	{
		BuildFallbackEffect();
	}
	const FT66IdolThemeSpec DebugTheme = ResolveTheme(Request.IdolID);
	const FColor DebugColor = DebugTheme.PrimaryColor.ToFColor(true);
	const float DebugRadius = Request.Category == ET66AttackCategory::AOE ? FMath::Max(40.f, Request.Radius * 0.34f) : 28.f;
	DrawDebugSphere(GetWorld(), GetActorLocation(), DebugRadius, 16, DebugColor, false, 2.0f, 0, 2.0f);
	UE_LOG(LogTemp, Warning, TEXT("[IDOL VFX ACTOR] BuildEffect done actor=%s idol=%s category=%s layers=%d lifetime=%.2f actorLoc=%s"), *GetName(), *Request.IdolID.ToString(), GetIdolVFXCategoryName(Request.Category), LayerStates.Num(), LifetimeSeconds, *GetActorLocation().ToCompactString());

	bBuilt = true;
}

void AT66IdolProcVFX::BuildPierceEffect()
{
	const FT66IdolThemeSpec Theme = ResolveTheme(Request.IdolID);
	const FT66IdolRaritySpec Tier = ResolveRarity(Request.Rarity);

	const FVector RawDir = Request.End - Request.Start;
	const FVector FlatDir = FVector(RawDir.X, RawDir.Y, 0.f);
	const float TraceLength = FMath::Max(70.f, FVector::Dist2D(Request.Start, Request.End));
	const float Yaw = 0.f;
	const FVector ImpactLocal = Request.Impact.IsNearlyZero()
		? FVector(TraceLength * 0.3f, 0.f, 12.f)
		: (Request.Impact - Request.Start) + FVector(0.f, 0.f, 12.f);

	const FLinearColor Primary = Brighten(Theme.PrimaryColor, Tier.Brightness);
	const FLinearColor Secondary = Brighten(Theme.SecondaryColor, Tier.Brightness * 0.5f);
	const FLinearColor Accent = Brighten(Theme.AccentColor, Tier.Brightness);

	auto MakeMaterial = [&](const ET66IdolVFXTexture Texture, const FLinearColor& LocalPrimary, const FLinearColor& LocalSecondary, const float InnerRadius, const float OuterRadius, const float GlowScale, const float ScrollScale)
	{
		FT66IdolVFXLayerMaterial Material;
		Material.Texture = Texture;
		Material.PrimaryColor = LocalPrimary;
		Material.SecondaryColor = LocalSecondary;
		Material.OutlineColor = Theme.OutlineColor;
		Material.TintColor = FLinearColor::White;
		Material.PixelGrid = Theme.PixelGrid * Tier.PixelGridScale;
		Material.DetailTiling = Theme.DetailTiling * Tier.DetailTilingScale;
		Material.ScrollSpeed = Theme.ScrollSpeed * ScrollScale;
		Material.InnerRadius = InnerRadius;
		Material.OuterRadius = OuterRadius;
		Material.GlowStrength = Theme.GlowStrength * Tier.GlowScale * GlowScale;
		Material.OpacityBoost = Tier.OpacityBoost;
		return Material;
	};

	auto MakeAnim = [&](const FVector& Wave, const FVector& ScaleAmp, const float RotationSpeed, const float Phase, const float AgeOffset)
	{
		FT66IdolVFXLayerAnimState Anim;
		Anim.WaveAmplitude = Wave;
		Anim.ScaleAmplitude = ScaleAmp;
		Anim.RotationSpeed = RotationSpeed;
		Anim.Phase = Phase;
		Anim.AgeOffset = AgeOffset;
		return Anim;
	};

	AddLayer(
		TEXT("PierceCore"),
		FVector(TraceLength * 0.5f, 0.f, 10.f),
		FRotator(0.f, Yaw, 0.f),
		FVector(TraceLength / 100.f, Theme.BaseWidth * Tier.WidthScale, 1.f),
		MakeMaterial(Theme.PrimaryTexture, Primary, Secondary, 0.34f, 0.49f, 1.f, 1.f),
		MakeAnim(FVector::ZeroVector, FVector(0.05f, 0.20f, 0.f), 0.f, 0.f, 0.f),
		3);

	switch (Theme.Flavor)
	{
	case ET66IdolFlavor::Light:
		AddLayer(TEXT("PierceLightHalo"), FVector(TraceLength * 0.5f, 0.f, 14.f), FRotator(0.f, Yaw, 0.f), FVector((TraceLength * 0.92f) / 100.f, Theme.BaseWidth * 0.55f * Tier.WidthScale, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.22f, 0.48f, 1.20f, 0.65f), MakeAnim(FVector::ZeroVector, FVector(0.10f, 0.12f, 0.f), Theme.RotationSpeed * 0.08f, 0.4f, -0.04f), 4);
		break;
	case ET66IdolFlavor::Wind:
		AddLayer(TEXT("PierceWindLeft"), FVector(TraceLength * 0.48f, -18.f, 12.f), FRotator(0.f, Yaw, 0.f), FVector((TraceLength * 0.90f) / 100.f, Theme.BaseWidth * 0.50f * Tier.WidthScale, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Secondary, 0.18f, 0.44f, 0.95f, 1.45f), MakeAnim(FVector(0.f, 12.f, 4.f), FVector(0.04f, 0.16f, 0.f), Theme.RotationSpeed * 0.15f, 0.1f, 0.02f), 4);
		AddLayer(TEXT("PierceWindRight"), FVector(TraceLength * 0.48f, 18.f, 8.f), FRotator(0.f, Yaw, 0.f), FVector((TraceLength * 0.84f) / 100.f, Theme.BaseWidth * 0.40f * Tier.WidthScale, 1.f), MakeMaterial(Theme.AccentTexture, Primary, Accent, 0.18f, 0.44f, 0.85f, 1.25f), MakeAnim(FVector(0.f, -12.f, 3.f), FVector(0.03f, 0.12f, 0.f), -Theme.RotationSpeed * 0.12f, 1.3f, -0.03f), 4);
		break;
	case ET66IdolFlavor::Steel:
		AddLayer(TEXT("PierceSteelRailLeft"), FVector(TraceLength * 0.50f, -10.f, 12.f), FRotator(0.f, Yaw, 0.f), FVector((TraceLength * 0.95f) / 100.f, Theme.BaseWidth * 0.28f * Tier.WidthScale, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Secondary, 0.30f, 0.48f, 0.90f, 0.55f), MakeAnim(FVector::ZeroVector, FVector(0.02f, 0.04f, 0.f), 0.f, 0.7f, 0.01f), 4);
		AddLayer(TEXT("PierceSteelRailRight"), FVector(TraceLength * 0.50f, 10.f, 12.f), FRotator(0.f, Yaw, 0.f), FVector((TraceLength * 0.95f) / 100.f, Theme.BaseWidth * 0.28f * Tier.WidthScale, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Secondary, 0.30f, 0.48f, 0.90f, 0.55f), MakeAnim(FVector::ZeroVector, FVector(0.02f, 0.04f, 0.f), 0.f, 1.8f, -0.01f), 4);
		break;
	case ET66IdolFlavor::Wood:
		AddLayer(TEXT("PierceWoodThorn"), FVector(TraceLength * 0.45f, 0.f, 16.f), FRotator(0.f, Yaw + 7.f, 0.f), FVector((TraceLength * 0.75f) / 100.f, Theme.BaseWidth * 0.42f * Tier.WidthScale, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Secondary, 0.28f, 0.49f, 1.05f, 0.70f), MakeAnim(FVector(0.f, 8.f, 5.f), FVector(0.05f, 0.10f, 0.f), Theme.RotationSpeed * 0.10f, 0.6f, 0.03f), 4);
		break;
	case ET66IdolFlavor::Bone:
		for (int32 SegmentIndex = 0; SegmentIndex < 3; ++SegmentIndex)
		{
			const float T = 0.22f + (0.24f * static_cast<float>(SegmentIndex));
			AddLayer(*FString::Printf(TEXT("PierceBoneSegment_%d"), SegmentIndex), FVector(TraceLength * T, 0.f, 14.f + (2.f * SegmentIndex)), FRotator(0.f, Yaw + (SegmentIndex % 2 == 0 ? 8.f : -8.f), 0.f), FVector((TraceLength * 0.18f) / 100.f, Theme.BaseWidth * 0.58f * Tier.WidthScale, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Secondary, 0.26f, 0.47f, 0.90f, 0.40f), MakeAnim(FVector::ZeroVector, FVector(0.04f, 0.08f, 0.f), 0.f, 0.4f * SegmentIndex, 0.03f * SegmentIndex), 4);
		}
		break;
	case ET66IdolFlavor::Glass:
		AddLayer(TEXT("PierceGlassEchoLeft"), FVector(TraceLength * 0.50f, -12.f, 12.f), FRotator(0.f, Yaw - 5.f, 0.f), FVector((TraceLength * 0.88f) / 100.f, Theme.BaseWidth * 0.22f * Tier.WidthScale, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Secondary, 0.26f, 0.46f, 1.15f, 1.25f), MakeAnim(FVector(0.f, 5.f, 3.f), FVector(0.04f, 0.10f, 0.f), Theme.RotationSpeed * 0.18f, 1.2f, -0.02f), 4);
		AddLayer(TEXT("PierceGlassEchoRight"), FVector(TraceLength * 0.50f, 12.f, 12.f), FRotator(0.f, Yaw + 5.f, 0.f), FVector((TraceLength * 0.88f) / 100.f, Theme.BaseWidth * 0.22f * Tier.WidthScale, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Secondary, 0.26f, 0.46f, 1.15f, 1.35f), MakeAnim(FVector(0.f, -5.f, 3.f), FVector(0.04f, 0.10f, 0.f), -Theme.RotationSpeed * 0.18f, 2.1f, 0.02f), 4);
		break;
	default:
		break;
	}

	for (int32 EchoIndex = 0; EchoIndex < Tier.ExtraLayers; ++EchoIndex)
	{
		const float EchoSign = (EchoIndex % 2 == 0) ? -1.f : 1.f;
		const float EchoOffset = 7.f + (EchoIndex * 5.f);
		AddLayer(
			*FString::Printf(TEXT("PierceEcho_%d"), EchoIndex),
			FVector(TraceLength * 0.48f, EchoOffset * EchoSign, 11.f + EchoIndex),
			FRotator(0.f, Yaw + (EchoSign * 4.f), 0.f),
			FVector((TraceLength * (0.82f - (EchoIndex * 0.06f))) / 100.f, Theme.BaseWidth * (0.40f - (EchoIndex * 0.05f)) * Tier.WidthScale, 1.f),
			MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.22f, 0.46f, 0.75f, 0.85f + (EchoIndex * 0.15f)),
			MakeAnim(FVector(0.f, 4.f * EchoSign, 2.f), FVector(0.04f, 0.09f, 0.f), Theme.RotationSpeed * 0.08f * EchoSign, 0.8f * EchoIndex, 0.04f * EchoIndex),
			4);
	}

	AddLayer(
		TEXT("PierceImpact"),
		ImpactLocal,
		FRotator::ZeroRotator,
		FVector(Theme.ImpactScale * Tier.ImpactScale, Theme.ImpactScale * Tier.ImpactScale, 1.f),
		MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.24f, 0.48f, 1.25f, 0.30f),
		MakeAnim(FVector::ZeroVector, FVector(0.20f, 0.20f, 0.f), Theme.RotationSpeed * 0.30f, 0.2f, -0.06f),
		5);
}

void AT66IdolProcVFX::BuildAOEEffect()
{
	const FT66IdolThemeSpec Theme = ResolveTheme(Request.IdolID);
	const FT66IdolRaritySpec Tier = ResolveRarity(Request.Rarity);

	const float RadiusScale = FMath::Max(0.75f, Request.Radius / 100.f);
	const FLinearColor Primary = Brighten(Theme.PrimaryColor, Tier.Brightness);
	const FLinearColor Secondary = Brighten(Theme.SecondaryColor, Tier.Brightness * 0.4f);
	const FLinearColor Accent = Brighten(Theme.AccentColor, Tier.Brightness);

	auto MakeMaterial = [&](const ET66IdolVFXTexture Texture, const FLinearColor& LocalPrimary, const FLinearColor& LocalSecondary, const float InnerRadius, const float OuterRadius, const float GlowScale, const float ScrollScale)
	{
		FT66IdolVFXLayerMaterial Material;
		Material.Texture = Texture;
		Material.PrimaryColor = LocalPrimary;
		Material.SecondaryColor = LocalSecondary;
		Material.OutlineColor = Theme.OutlineColor;
		Material.PixelGrid = Theme.PixelGrid * Tier.PixelGridScale;
		Material.DetailTiling = Theme.DetailTiling * Tier.DetailTilingScale;
		Material.ScrollSpeed = Theme.ScrollSpeed * ScrollScale;
		Material.InnerRadius = InnerRadius;
		Material.OuterRadius = OuterRadius;
		Material.GlowStrength = Theme.GlowStrength * Tier.GlowScale * GlowScale;
		Material.OpacityBoost = Tier.OpacityBoost;
		return Material;
	};

	auto MakeAnim = [&](const FVector& Wave, const FVector& ScaleAmp, const float RotationSpeed, const float Phase, const float AgeOffset)
	{
		FT66IdolVFXLayerAnimState Anim;
		Anim.WaveAmplitude = Wave;
		Anim.ScaleAmplitude = ScaleAmp;
		Anim.RotationSpeed = RotationSpeed;
		Anim.Phase = Phase;
		Anim.AgeOffset = AgeOffset;
		return Anim;
	};

	AddLayer(TEXT("AOECore"), FVector(0.f, 0.f, 6.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.68f, RadiusScale * 0.68f, 1.f), MakeMaterial(Theme.PrimaryTexture, Primary, Secondary, 0.18f, 0.49f, 1.10f, 1.f), MakeAnim(FVector::ZeroVector, FVector(0.25f, 0.25f, 0.f), Theme.RotationSpeed * 0.24f, 0.f, 0.f), 3);

	switch (Theme.Flavor)
	{
	case ET66IdolFlavor::Fire:
		AddLayer(TEXT("AOEFireOuter"), FVector(0.f, 0.f, 10.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.90f, RadiusScale * 0.90f, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.28f, 0.49f, 1.30f, 1.35f), MakeAnim(FVector::ZeroVector, FVector(0.20f, 0.22f, 0.f), Theme.RotationSpeed * 0.34f, 0.7f, -0.05f), 4);
		AddLayer(TEXT("AOEFireInner"), FVector(0.f, 0.f, 14.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.42f, RadiusScale * 0.42f, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Secondary, 0.10f, 0.44f, 1.15f, 0.90f), MakeAnim(FVector(0.f, 0.f, 4.f), FVector(0.14f, 0.14f, 0.f), Theme.RotationSpeed * -0.16f, 1.2f, 0.02f), 5);
		break;
	case ET66IdolFlavor::Earth:
		AddLayer(TEXT("AOEEarthRim"), FVector(0.f, 0.f, 8.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.84f, RadiusScale * 0.84f, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Secondary, 0.34f, 0.49f, 0.95f, 0.55f), MakeAnim(FVector::ZeroVector, FVector(0.12f, 0.12f, 0.f), Theme.RotationSpeed * 0.12f, 0.4f, -0.02f), 4);
		break;
	case ET66IdolFlavor::Water:
		AddLayer(TEXT("AOEWaterRippleA"), FVector(0.f, 0.f, 9.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.84f, RadiusScale * 0.84f, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.32f, 0.49f, 0.95f, 0.45f), MakeAnim(FVector::ZeroVector, FVector(0.18f, 0.18f, 0.f), Theme.RotationSpeed * 0.24f, 0.3f, -0.03f), 4);
		AddLayer(TEXT("AOEWaterRippleB"), FVector(0.f, 0.f, 13.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.56f, RadiusScale * 0.56f, 1.f), MakeMaterial(Theme.PrimaryTexture, Primary, Accent, 0.18f, 0.44f, 1.10f, -0.30f), MakeAnim(FVector::ZeroVector, FVector(0.15f, 0.15f, 0.f), -Theme.RotationSpeed * 0.28f, 1.4f, 0.03f), 5);
		break;
	case ET66IdolFlavor::Sand:
		AddLayer(TEXT("AOESandDust"), FVector(0.f, 0.f, 10.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.92f, RadiusScale * 0.92f, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Secondary, 0.24f, 0.48f, 0.90f, 0.75f), MakeAnim(FVector(0.f, 0.f, 3.f), FVector(0.16f, 0.16f, 0.f), Theme.RotationSpeed * 0.18f, 0.9f, -0.04f), 4);
		break;
	case ET66IdolFlavor::BlackHole:
		AddLayer(TEXT("AOEBlackHoleRim"), FVector(0.f, 0.f, 11.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.82f, RadiusScale * 0.82f, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Secondary, 0.36f, 0.49f, 1.20f, 1.10f), MakeAnim(FVector::ZeroVector, FVector(-0.12f, -0.12f, 0.f), Theme.RotationSpeed * 0.38f, 0.6f, 0.04f), 5);
		AddLayer(TEXT("AOEBlackHoleCore"), FVector(0.f, 0.f, 7.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.32f, RadiusScale * 0.32f, 1.f), MakeMaterial(Theme.PrimaryTexture, Theme.AccentColor, Theme.SecondaryColor, 0.02f, 0.38f, 0.85f, -0.18f), MakeAnim(FVector::ZeroVector, FVector(0.06f, 0.06f, 0.f), -Theme.RotationSpeed * 0.22f, 1.4f, -0.03f), 4);
		break;
	case ET66IdolFlavor::Storm:
		AddLayer(TEXT("AOEStormRing"), FVector(0.f, 0.f, 10.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.88f, RadiusScale * 0.88f, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.28f, 0.49f, 1.05f, 1.20f), MakeAnim(FVector::ZeroVector, FVector(0.18f, 0.18f, 0.f), Theme.RotationSpeed * 0.30f, 0.2f, -0.03f), 4);
		AddLayer(TEXT("AOEStormEye"), FVector(0.f, 0.f, 13.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.46f, RadiusScale * 0.46f, 1.f), MakeMaterial(Theme.PrimaryTexture, Primary, Secondary, 0.14f, 0.42f, 0.95f, 0.50f), MakeAnim(FVector::ZeroVector, FVector(0.14f, 0.14f, 0.f), -Theme.RotationSpeed * 0.24f, 1.1f, 0.01f), 5);
		break;
	default:
		AddLayer(TEXT("AOEAccent"), FVector(0.f, 0.f, 10.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.84f, RadiusScale * 0.84f, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.26f, 0.48f, 1.00f, 0.80f), MakeAnim(FVector::ZeroVector, FVector(0.16f, 0.16f, 0.f), Theme.RotationSpeed * 0.18f, 0.6f, -0.02f), 4);
		break;
	}

	for (int32 LayerIndex = 0; LayerIndex < Tier.ExtraLayers; ++LayerIndex)
	{
		const float Scale = RadiusScale * (0.78f + (LayerIndex * 0.16f));
		AddLayer(*FString::Printf(TEXT("AOEEcho_%d"), LayerIndex), FVector(0.f, 0.f, 9.f + LayerIndex), FRotator::ZeroRotator, FVector(Scale, Scale, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.28f, 0.49f, 0.65f, 0.55f + (LayerIndex * 0.18f)), MakeAnim(FVector::ZeroVector, FVector(0.12f, 0.12f, 0.f), Theme.RotationSpeed * 0.10f * (LayerIndex % 2 == 0 ? 1.f : -1.f), 0.7f * LayerIndex, 0.04f * LayerIndex), 4);
	}
}

void AT66IdolProcVFX::BuildBounceEffect()
{
	const FT66IdolThemeSpec Theme = ResolveTheme(Request.IdolID);
	const FT66IdolRaritySpec Tier = ResolveRarity(Request.Rarity);
	if (Request.ChainPositions.Num() < 2)
	{
		return;
	}

	const FVector Root = Request.ChainPositions[0];
	const FLinearColor Primary = Brighten(Theme.PrimaryColor, Tier.Brightness);
	const FLinearColor Secondary = Brighten(Theme.SecondaryColor, Tier.Brightness * 0.4f);
	const FLinearColor Accent = Brighten(Theme.AccentColor, Tier.Brightness);

	auto MakeMaterial = [&](const ET66IdolVFXTexture Texture, const FLinearColor& LocalPrimary, const FLinearColor& LocalSecondary, const float InnerRadius, const float OuterRadius, const float GlowScale, const float ScrollScale)
	{
		FT66IdolVFXLayerMaterial Material;
		Material.Texture = Texture;
		Material.PrimaryColor = LocalPrimary;
		Material.SecondaryColor = LocalSecondary;
		Material.OutlineColor = Theme.OutlineColor;
		Material.PixelGrid = Theme.PixelGrid * Tier.PixelGridScale;
		Material.DetailTiling = Theme.DetailTiling * Tier.DetailTilingScale;
		Material.ScrollSpeed = Theme.ScrollSpeed * ScrollScale;
		Material.InnerRadius = InnerRadius;
		Material.OuterRadius = OuterRadius;
		Material.GlowStrength = Theme.GlowStrength * Tier.GlowScale * GlowScale;
		Material.OpacityBoost = Tier.OpacityBoost;
		return Material;
	};

	auto MakeAnim = [&](const FVector& Wave, const FVector& ScaleAmp, const float RotationSpeed, const float Phase, const float AgeOffset)
	{
		FT66IdolVFXLayerAnimState Anim;
		Anim.WaveAmplitude = Wave;
		Anim.ScaleAmplitude = ScaleAmp;
		Anim.RotationSpeed = RotationSpeed;
		Anim.Phase = Phase;
		Anim.AgeOffset = AgeOffset;
		return Anim;
	};

	for (int32 SegmentIndex = 0; SegmentIndex < Request.ChainPositions.Num() - 1; ++SegmentIndex)
	{
		const FVector SegmentStart = Request.ChainPositions[SegmentIndex];
		const FVector SegmentEnd = Request.ChainPositions[SegmentIndex + 1];
		const FVector SegmentDir = FVector(SegmentEnd.X - SegmentStart.X, SegmentEnd.Y - SegmentStart.Y, 0.f).GetSafeNormal();
		if (SegmentDir.IsNearlyZero())
		{
			continue;
		}

		const float Dist = FMath::Max(40.f, FVector::Dist2D(SegmentStart, SegmentEnd));
		const float Yaw = SegmentDir.Rotation().Yaw;
		const FVector MidLocal = ((SegmentStart + SegmentEnd) * 0.5f) - Root + FVector(0.f, 0.f, 10.f);
		const FVector ImpactLocal = SegmentStart - Root + FVector(0.f, 0.f, 12.f);

		FVector Wave(0.f, 0.f, 0.f);
		float WidthMultiplier = 1.f;
		float ScrollMultiplier = 1.f;
		switch (Theme.Flavor)
		{
		case ET66IdolFlavor::Electric:
			Wave = FVector(0.f, 11.f, 3.f);
			WidthMultiplier = 0.90f;
			ScrollMultiplier = 1.60f;
			break;
		case ET66IdolFlavor::Ice:
			Wave = FVector(0.f, 3.f, 1.f);
			WidthMultiplier = 0.80f;
			ScrollMultiplier = 0.75f;
			break;
		case ET66IdolFlavor::Sound:
			Wave = FVector(0.f, 7.f, 0.f);
			WidthMultiplier = 1.30f;
			ScrollMultiplier = 0.55f;
			break;
		case ET66IdolFlavor::Shadow:
			Wave = FVector(0.f, 5.f, 4.f);
			WidthMultiplier = 1.05f;
			ScrollMultiplier = 0.90f;
			break;
		case ET66IdolFlavor::Star:
			Wave = FVector(0.f, 8.f, 2.f);
			WidthMultiplier = 0.95f;
			ScrollMultiplier = 1.15f;
			break;
		case ET66IdolFlavor::Rubber:
			Wave = FVector(0.f, 4.f, 0.f);
			WidthMultiplier = 1.40f;
			ScrollMultiplier = 0.40f;
			break;
		default:
			break;
		}

		AddLayer(*FString::Printf(TEXT("BounceCore_%d"), SegmentIndex), MidLocal, FRotator(0.f, Yaw, 0.f), FVector(Dist / 100.f, Theme.BaseWidth * WidthMultiplier * Tier.WidthScale, 1.f), MakeMaterial(Theme.PrimaryTexture, Primary, Secondary, 0.30f, 0.48f, 1.00f, ScrollMultiplier), MakeAnim(Wave, FVector(0.03f, 0.14f, 0.f), Theme.RotationSpeed * 0.08f, SegmentIndex * 0.9f, SegmentIndex * 0.03f), 3);

		if (Theme.Flavor == ET66IdolFlavor::Electric || Theme.Flavor == ET66IdolFlavor::Shadow || Theme.Flavor == ET66IdolFlavor::Star)
		{
			AddLayer(*FString::Printf(TEXT("BounceAccent_%d"), SegmentIndex), MidLocal + FVector(0.f, (SegmentIndex % 2 == 0 ? 8.f : -8.f), 3.f), FRotator(0.f, Yaw + (SegmentIndex % 2 == 0 ? 6.f : -6.f), 0.f), FVector((Dist * 0.82f) / 100.f, Theme.BaseWidth * 0.42f * Tier.WidthScale, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.20f, 0.46f, 0.90f, ScrollMultiplier * 1.15f), MakeAnim(Wave * 0.5f, FVector(0.04f, 0.10f, 0.f), Theme.RotationSpeed * 0.14f, 0.6f + SegmentIndex, -0.02f), 4);
		}

		AddLayer(*FString::Printf(TEXT("BounceImpact_%d"), SegmentIndex), ImpactLocal, FRotator::ZeroRotator, FVector(Theme.ImpactScale * 0.82f * Tier.ImpactScale, Theme.ImpactScale * 0.82f * Tier.ImpactScale, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.18f, 0.46f, 1.10f, 0.35f), MakeAnim(FVector::ZeroVector, FVector(0.16f, 0.16f, 0.f), Theme.RotationSpeed * 0.28f, 0.2f + SegmentIndex, -0.04f), 5);

		if (Theme.Flavor == ET66IdolFlavor::Sound || Theme.Flavor == ET66IdolFlavor::Rubber)
		{
			AddLayer(*FString::Printf(TEXT("BouncePulse_%d"), SegmentIndex), ImpactLocal + FVector(0.f, 0.f, 2.f), FRotator::ZeroRotator, FVector(Theme.ImpactScale * 1.15f * Tier.ImpactScale, Theme.ImpactScale * 1.15f * Tier.ImpactScale, 1.f), MakeMaterial(Theme.PrimaryTexture, Primary, Secondary, 0.30f, 0.49f, 0.85f, 0.20f), MakeAnim(FVector::ZeroVector, FVector(0.22f, 0.22f, 0.f), Theme.RotationSpeed * 0.18f, 1.1f + SegmentIndex, 0.01f), 4);
		}
	}

	const FVector FinalImpactLocal = Request.ChainPositions.Last() - Request.ChainPositions[0] + FVector(0.f, 0.f, 12.f);
	AddLayer(TEXT("BounceFinalImpact"), FinalImpactLocal, FRotator::ZeroRotator, FVector(Theme.ImpactScale * Tier.ImpactScale, Theme.ImpactScale * Tier.ImpactScale, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.16f, 0.47f, 1.20f, 0.30f), MakeAnim(FVector::ZeroVector, FVector(0.24f, 0.24f, 0.f), Theme.RotationSpeed * 0.32f, 0.5f, -0.06f), 5);

	for (int32 EchoIndex = 0; EchoIndex < Tier.ExtraLayers; ++EchoIndex)
	{
		for (int32 SegmentIndex = 0; SegmentIndex < Request.ChainPositions.Num() - 1; ++SegmentIndex)
		{
			const FVector SegmentStart = Request.ChainPositions[SegmentIndex];
			const FVector SegmentEnd = Request.ChainPositions[SegmentIndex + 1];
			const FVector SegmentDir = FVector(SegmentEnd.X - SegmentStart.X, SegmentEnd.Y - SegmentStart.Y, 0.f).GetSafeNormal();
			if (SegmentDir.IsNearlyZero())
			{
				continue;
			}

			const float Dist = FMath::Max(40.f, FVector::Dist2D(SegmentStart, SegmentEnd));
			const float Yaw = SegmentDir.Rotation().Yaw;
			const float EchoSign = ((EchoIndex + SegmentIndex) % 2 == 0) ? -1.f : 1.f;
			const FVector MidLocal = ((SegmentStart + SegmentEnd) * 0.5f) - Root + FVector(0.f, 10.f * EchoSign, 12.f + EchoIndex);
			AddLayer(*FString::Printf(TEXT("BounceEcho_%d_%d"), EchoIndex, SegmentIndex), MidLocal, FRotator(0.f, Yaw + (EchoSign * 4.f), 0.f), FVector((Dist * (0.78f - (EchoIndex * 0.05f))) / 100.f, Theme.BaseWidth * 0.32f * Tier.WidthScale, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.22f, 0.44f, 0.70f, 0.85f + (EchoIndex * 0.18f)), MakeAnim(FVector(0.f, 4.f * EchoSign, 2.f), FVector(0.03f, 0.08f, 0.f), Theme.RotationSpeed * 0.12f, 0.8f * (EchoIndex + SegmentIndex), 0.04f * EchoIndex), 4);
		}
	}
}

void AT66IdolProcVFX::BuildDOTEffect()
{
	const FT66IdolThemeSpec Theme = ResolveTheme(Request.IdolID);
	const FT66IdolRaritySpec Tier = ResolveRarity(Request.Rarity);
	const float RadiusScale = FMath::Max(0.45f, Request.Radius / 100.f);
	const FLinearColor Primary = Brighten(Theme.PrimaryColor, Tier.Brightness);
	const FLinearColor Secondary = Brighten(Theme.SecondaryColor, Tier.Brightness * 0.4f);
	const FLinearColor Accent = Brighten(Theme.AccentColor, Tier.Brightness);

	auto MakeMaterial = [&](const ET66IdolVFXTexture Texture, const FLinearColor& LocalPrimary, const FLinearColor& LocalSecondary, const float InnerRadius, const float OuterRadius, const float GlowScale, const float ScrollScale)
	{
		FT66IdolVFXLayerMaterial Material;
		Material.Texture = Texture;
		Material.PrimaryColor = LocalPrimary;
		Material.SecondaryColor = LocalSecondary;
		Material.OutlineColor = Theme.OutlineColor;
		Material.PixelGrid = Theme.PixelGrid * Tier.PixelGridScale;
		Material.DetailTiling = Theme.DetailTiling * Tier.DetailTilingScale;
		Material.ScrollSpeed = Theme.ScrollSpeed * ScrollScale;
		Material.InnerRadius = InnerRadius;
		Material.OuterRadius = OuterRadius;
		Material.GlowStrength = Theme.GlowStrength * Tier.GlowScale * GlowScale;
		Material.OpacityBoost = Tier.OpacityBoost;
		return Material;
	};

	auto MakeAnim = [&](const FVector& Wave, const FVector& ScaleAmp, const float RotationSpeed, const float Phase, const float AgeOffset)
	{
		FT66IdolVFXLayerAnimState Anim;
		Anim.WaveAmplitude = Wave;
		Anim.ScaleAmplitude = ScaleAmp;
		Anim.RotationSpeed = RotationSpeed;
		Anim.Phase = Phase;
		Anim.AgeOffset = AgeOffset;
		return Anim;
	};

	AddLayer(TEXT("DOTGround"), FVector(0.f, 0.f, 4.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.86f, RadiusScale * 0.86f, 1.f), MakeMaterial(Theme.PrimaryTexture, Primary, Secondary, 0.30f, 0.49f, 0.95f, 0.35f), MakeAnim(FVector::ZeroVector, FVector(0.08f, 0.08f, 0.f), Theme.RotationSpeed * 0.10f, 0.f, 0.f), 3);
	AddLayer(TEXT("DOTAura"), FVector(0.f, 0.f, 26.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.46f, RadiusScale * 0.46f, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.12f, 0.44f, 1.05f, 0.70f), MakeAnim(FVector(0.f, 0.f, 6.f), FVector(0.14f, 0.14f, 0.f), Theme.RotationSpeed * 0.22f, 0.6f, -0.03f), 4);

	switch (Theme.Flavor)
	{
	case ET66IdolFlavor::Curse:
		AddLayer(TEXT("DOTCurseSeal"), FVector(0.f, 0.f, 14.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.60f, RadiusScale * 0.60f, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.22f, 0.48f, 1.10f, 0.55f), MakeAnim(FVector(0.f, 0.f, 2.f), FVector(0.10f, 0.10f, 0.f), Theme.RotationSpeed * 0.18f, 1.1f, 0.02f), 5);
		break;
	case ET66IdolFlavor::Lava:
		AddLayer(TEXT("DOTLavaCore"), FVector(0.f, 0.f, 16.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.38f, RadiusScale * 0.38f, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.08f, 0.38f, 1.20f, 1.10f), MakeAnim(FVector(0.f, 0.f, 4.f), FVector(0.16f, 0.16f, 0.f), Theme.RotationSpeed * 0.14f, 0.8f, -0.01f), 5);
		break;
	case ET66IdolFlavor::Poison:
		AddLayer(TEXT("DOTPoisonCloud"), FVector(0.f, 0.f, 22.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.58f, RadiusScale * 0.58f, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.14f, 0.42f, 0.90f, 0.40f), MakeAnim(FVector(0.f, 0.f, 7.f), FVector(0.18f, 0.18f, 0.f), Theme.RotationSpeed * 0.16f, 1.3f, 0.01f), 5);
		break;
	case ET66IdolFlavor::Decay:
		AddLayer(TEXT("DOTDecayCorrode"), FVector(0.f, 0.f, 10.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.70f, RadiusScale * 0.70f, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Secondary, 0.20f, 0.47f, 0.85f, 0.20f), MakeAnim(FVector(0.f, 0.f, 3.f), FVector(0.12f, 0.12f, 0.f), Theme.RotationSpeed * 0.08f, 0.7f, 0.03f), 4);
		break;
	case ET66IdolFlavor::Bleed:
		AddLayer(TEXT("DOTBleedPulse"), FVector(0.f, 0.f, 12.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.64f, RadiusScale * 0.64f, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.18f, 0.47f, 1.00f, 0.85f), MakeAnim(FVector::ZeroVector, FVector(0.18f, 0.18f, 0.f), Theme.RotationSpeed * 0.12f, 0.5f, -0.02f), 5);
		break;
	case ET66IdolFlavor::Acid:
		AddLayer(TEXT("DOTAcidSizzle"), FVector(0.f, 0.f, 18.f), FRotator::ZeroRotator, FVector(RadiusScale * 0.52f, RadiusScale * 0.52f, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.10f, 0.42f, 1.10f, 1.25f), MakeAnim(FVector(0.f, 0.f, 5.f), FVector(0.16f, 0.16f, 0.f), Theme.RotationSpeed * 0.20f, 1.5f, -0.01f), 5);
		break;
	default:
		break;
	}

	for (int32 LayerIndex = 0; LayerIndex < Tier.ExtraLayers; ++LayerIndex)
	{
		const float Scale = RadiusScale * (0.56f + (LayerIndex * 0.14f));
		AddLayer(*FString::Printf(TEXT("DOTEcho_%d"), LayerIndex), FVector(0.f, 0.f, 16.f + (LayerIndex * 5.f)), FRotator::ZeroRotator, FVector(Scale, Scale, 1.f), MakeMaterial(Theme.AccentTexture, Accent, Primary, 0.20f, 0.46f, 0.72f, 0.50f + (LayerIndex * 0.15f)), MakeAnim(FVector(0.f, 0.f, 3.f + LayerIndex), FVector(0.10f, 0.10f, 0.f), Theme.RotationSpeed * 0.12f * (LayerIndex % 2 == 0 ? 1.f : -1.f), 0.8f * LayerIndex, 0.05f * LayerIndex), 4);
	}
}

void AT66IdolProcVFX::BuildFallbackEffect()
{
	const FT66IdolThemeSpec Theme = ResolveTheme(Request.IdolID);
	const FT66IdolRaritySpec Tier = ResolveRarity(Request.Rarity);

	FT66IdolVFXLayerMaterial Material;
	Material.Texture = Theme.PrimaryTexture;
	Material.PrimaryColor = Brighten(Theme.PrimaryColor, Tier.Brightness);
	Material.SecondaryColor = Brighten(Theme.SecondaryColor, Tier.Brightness * 0.4f);
	Material.OutlineColor = Theme.OutlineColor;
	Material.PixelGrid = Theme.PixelGrid * Tier.PixelGridScale;
	Material.DetailTiling = Theme.DetailTiling * Tier.DetailTilingScale;
	Material.ScrollSpeed = Theme.ScrollSpeed;
	Material.InnerRadius = 0.22f;
	Material.OuterRadius = 0.48f;
	Material.GlowStrength = Theme.GlowStrength * Tier.GlowScale;
	Material.OpacityBoost = Tier.OpacityBoost;

	FT66IdolVFXLayerAnimState Anim;
	Anim.ScaleAmplitude = FVector(0.14f, 0.14f, 0.f);
	Anim.RotationSpeed = Theme.RotationSpeed * 0.18f;

	AddLayer(TEXT("FallbackLayer"), FVector(0.f, 0.f, 10.f), FRotator::ZeroRotator, FVector(0.7f, 0.7f, 1.f), Material, Anim, 3);
}

void AT66IdolProcVFX::UpdateFollowTarget()
{
	if (Request.Category != ET66AttackCategory::DOT)
	{
		return;
	}

	if (AActor* FollowActor = Request.FollowTarget.Get())
	{
		SetActorLocation(FollowActor->GetActorLocation() + FVector(0.f, 0.f, 34.f));
	}
	else
	{
		SetActorLocation(Request.Center + FVector(0.f, 0.f, 34.f));
	}
}

void AT66IdolProcVFX::UpdateLayers(const float ActiveElapsed, const float ActiveAge01)
{
	for (FT66IdolVFXLayerAnimState& Layer : LayerStates)
	{
		if (!Layer.Mesh)
		{
			continue;
		}

		Layer.Mesh->SetVisibility(true, true);

		const float Osc = FMath::Sin((ActiveAge01 * 2.f * PI) + Layer.Phase);
		const FVector Offset = Layer.WaveAmplitude * Osc;
		const FVector ScaleFactor = FVector(
			FMath::Max(0.02f, 1.f + (Layer.ScaleAmplitude.X * Osc)),
			FMath::Max(0.02f, 1.f + (Layer.ScaleAmplitude.Y * Osc)),
			FMath::Max(0.02f, 1.f + (Layer.ScaleAmplitude.Z * Osc)));
		const FVector FinalScale = MultiplyComponents(Layer.BaseScale, ScaleFactor);
		const FRotator FinalRotation = Layer.BaseRotation + FRotator(0.f, Layer.RotationSpeed * ActiveElapsed, 0.f);

		Layer.Mesh->SetRelativeLocation(Layer.BaseLocation + Offset);
		Layer.Mesh->SetRelativeRotation(FinalRotation);
		Layer.Mesh->SetRelativeScale3D(FinalScale);

		if (Layer.MID)
		{
			Layer.MID->SetScalarParameterValue(TEXT("Age01"), FMath::Clamp(ActiveAge01 + Layer.AgeOffset, 0.f, 1.f));
		}
	}
}
