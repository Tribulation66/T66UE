// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HeroAOEAttackVFX.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const TCHAR* HeroAOEMaterialPath = TEXT("/Game/VFX/Hero1/MI_Hero1_Attack_Impact.MI_Hero1_Attack_Impact");
	const TCHAR* FireAOEMaterialPath = TEXT("/Game/VFX/Idols/Fire/MI_IdolFire_Explosion_Flipbook_Additive.MI_IdolFire_Explosion_Flipbook_Additive");

	void BuildExampleAOERuntimePalette(
		const FLinearColor& SeedColor,
		FLinearColor& OutTintColor,
		FLinearColor& OutPrimaryColor,
		FLinearColor& OutSecondaryColor,
		FLinearColor& OutOutlineColor,
		float& OutGlowStrength)
	{
		const FLinearColor ClampedColor(
			FMath::Clamp(SeedColor.R, 0.0f, 1.0f),
			FMath::Clamp(SeedColor.G, 0.0f, 1.0f),
			FMath::Clamp(SeedColor.B, 0.0f, 1.0f),
			1.0f);

		FLinearColor VividHSV = ClampedColor.LinearRGBToHSV();
		VividHSV.G = FMath::Clamp(FMath::Max(VividHSV.G * 1.8f, 0.72f), 0.0f, 1.0f);
		VividHSV.B = FMath::Clamp(FMath::Max(VividHSV.B * 1.10f, 0.88f), 0.0f, 1.0f);

		const FLinearColor VividPrimary = VividHSV.HSVToLinearRGB();
		const FLinearColor BrightTint = FLinearColor::LerpUsingHSV(VividPrimary, FLinearColor::White, 0.10f);
		const FLinearColor DeepSecondary = FLinearColor::LerpUsingHSV(VividPrimary, FLinearColor::Black, 0.58f);
		const FLinearColor DeepOutline = FLinearColor::LerpUsingHSV(DeepSecondary, FLinearColor::Black, 0.68f);

		OutTintColor = BrightTint;
		OutPrimaryColor = VividPrimary;
		OutSecondaryColor = DeepSecondary;
		OutOutlineColor = DeepOutline;
		OutGlowStrength = 1.75f;
	}

	bool IsHeroAOEStageVerbose()
	{
		if (const IConsoleVariable* HeroAOEVerbose = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.VFX.HeroAOEVerbose")))
		{
			if (HeroAOEVerbose->GetInt() != 0)
			{
				return true;
			}
		}

		if (const IConsoleVariable* IdolAOEVerbose = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.VFX.IdolAOEVerbose")))
		{
			return IdolAOEVerbose->GetInt() != 0;
		}

		return false;
	}
}

AT66HeroAOEAttackVFX::AT66HeroAOEAttackVFX()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	InitialLifeSpan = 1.0f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	CoreMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreMesh"));
	CoreMesh->SetupAttachment(SceneRoot);
	CoreMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoreMesh->SetCastShadow(false);
	CoreMesh->SetReceivesDecals(false);
	CoreMesh->SetCanEverAffectNavigation(false);
	CoreMesh->SetMobility(EComponentMobility::Movable);
	CoreMesh->TranslucencySortPriority = 3;

	RingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingMesh"));
	RingMesh->SetupAttachment(SceneRoot);
	RingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RingMesh->SetCastShadow(false);
	RingMesh->SetReceivesDecals(false);
	RingMesh->SetCanEverAffectNavigation(false);
	RingMesh->SetMobility(EComponentMobility::Movable);
	RingMesh->TranslucencySortPriority = 4;

	EchoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EchoMesh"));
	EchoMesh->SetupAttachment(SceneRoot);
	EchoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EchoMesh->SetCastShadow(false);
	EchoMesh->SetReceivesDecals(false);
	EchoMesh->SetCanEverAffectNavigation(false);
	EchoMesh->SetMobility(EComponentMobility::Movable);
	EchoMesh->TranslucencySortPriority = 5;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		CoreMesh->SetStaticMesh(PlaneMesh.Object);
		RingMesh->SetStaticMesh(PlaneMesh.Object);
		EchoMesh->SetStaticMesh(PlaneMesh.Object);
	}
}

void AT66HeroAOEAttackVFX::ConfigureFireFlipbook()
{
	bUseFireFlipbook = true;
	LifetimeSeconds = 1.0f;
	InitialLifeSpan = 1.25f;

	if (HasActorBegunPlay())
	{
		RefreshMaterials();
		ApplyEffectTransforms();
		UpdateMaterialParams(FMath::Clamp(ElapsedSeconds / FMath::Max(0.01f, LifetimeSeconds), 0.f, 1.f));
		LogFireFlipbookBinding();
	}
}

void AT66HeroAOEAttackVFX::SetPaletteOverride(const FLinearColor& InTint, const FLinearColor& InPrimary, const FLinearColor& InSecondary, const FLinearColor& InOutline, const float InGlowStrength, const FName InPaletteMode)
{
	OverrideTintColor = InTint;
	OverridePrimaryColor = InPrimary;
	OverrideSecondaryColor = InSecondary;
	OverrideOutlineColor = InOutline;
	OverrideGlowStrength = InGlowStrength;
	OverridePaletteMode = InPaletteMode;
	bUsePaletteOverride = true;
}

void AT66HeroAOEAttackVFX::InitEffect(const FVector& InLocation, const float InRadius, const FLinearColor& InTint, const int32 InDebugRequestId, const FName InDebugHeroID)
{
	EffectLocation = InLocation;
	BaseRadius = FMath::Max(70.f, InRadius);
	TintColor = InTint;
	DebugRequestId = InDebugRequestId;
	DebugHeroID = InDebugHeroID;
	bInitialized = true;

	if (IsHeroAOEStageVerbose())
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][ExampleAOE] Init Req=%d Source=%s Actor=%s Location=(%.1f,%.1f,%.1f) Radius=%.1f Tint=(%.2f,%.2f,%.2f,%.2f)"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			EffectLocation.X, EffectLocation.Y, EffectLocation.Z,
			BaseRadius,
			TintColor.R, TintColor.G, TintColor.B, TintColor.A);
	}

	ApplyEffectTransforms();
	LogConfiguredBegin();
	UpdateMaterialParams(0.f);
	LogFireFlipbookBinding();
}

void AT66HeroAOEAttackVFX::BeginPlay()
{
	Super::BeginPlay();

	RefreshMaterials();

	const bool bMissingCore = (CoreMID == nullptr);
	const bool bMissingExtraMIDs = !bUseFireFlipbook && (!RingMID || !EchoMID);
	if (bMissingCore || bMissingExtraMIDs)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ATTACK VFX][ExampleAOE] MissingMaterials Req=%d Source=%s Actor=%s Style=%s Core=%s Ring=%s Echo=%s"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			bUseFireFlipbook ? TEXT("FireFlipbook") : TEXT("DefaultPulse"),
			CoreMID ? TEXT("OK") : TEXT("MISSING"),
			bUseFireFlipbook ? TEXT("N/A") : (RingMID ? TEXT("OK") : TEXT("MISSING")),
			bUseFireFlipbook ? TEXT("N/A") : (EchoMID ? TEXT("OK") : TEXT("MISSING")));
		Destroy();
		return;
	}

	if (bInitialized)
	{
		ApplyEffectTransforms();
	}
}

void AT66HeroAOEAttackVFX::RefreshMaterials()
{
	CoreBaseMaterial = LoadObject<UMaterialInterface>(nullptr, bUseFireFlipbook ? FireAOEMaterialPath : HeroAOEMaterialPath);
	CoreMID = nullptr;
	RingMID = nullptr;
	EchoMID = nullptr;

	if (CoreBaseMaterial)
	{
		CoreMID = UMaterialInstanceDynamic::Create(CoreBaseMaterial, this, TEXT("HeroAOECoreMID"));
		if (CoreMID)
		{
			CoreMesh->SetMaterial(0, CoreMID);
		}
	}

	if (bUseFireFlipbook)
	{
		RingMesh->SetHiddenInGame(true);
		RingMesh->SetVisibility(false, true);
		EchoMesh->SetHiddenInGame(true);
		EchoMesh->SetVisibility(false, true);
		return;
	}

	RingMesh->SetHiddenInGame(false);
	RingMesh->SetVisibility(true, true);
	EchoMesh->SetHiddenInGame(false);
	EchoMesh->SetVisibility(true, true);

	if (CoreBaseMaterial)
	{
		RingMID = UMaterialInstanceDynamic::Create(CoreBaseMaterial, this, TEXT("HeroAOERingMID"));
		EchoMID = UMaterialInstanceDynamic::Create(CoreBaseMaterial, this, TEXT("HeroAOEEchoMID"));

		if (RingMID)
		{
			RingMesh->SetMaterial(0, RingMID);
		}
		if (EchoMID)
		{
			EchoMesh->SetMaterial(0, EchoMID);
		}
	}
}

void AT66HeroAOEAttackVFX::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ElapsedSeconds += DeltaSeconds;
	const float Age01 = FMath::Clamp(ElapsedSeconds / FMath::Max(0.01f, LifetimeSeconds), 0.f, 1.f);
	UpdateMaterialParams(Age01);

	const float RadiusScale = BaseRadius / 100.f;
	if (bUseFireFlipbook)
	{
		const float Growth = FMath::Lerp(0.84f, 1.02f, Age01);
		CoreMesh->SetRelativeScale3D(FVector(RadiusScale * Growth, RadiusScale * Growth, 1.f));
		CoreMesh->SetRelativeLocation(FVector(0.f, 0.f, FMath::Lerp(0.f, 10.f, Age01)));
	}
	else
	{
		const float Pulse = 1.f + FMath::Sin(Age01 * PI) * 0.10f;

		CoreMesh->SetRelativeScale3D(FVector(RadiusScale * FMath::Lerp(0.24f, 0.72f, Age01) * Pulse, RadiusScale * FMath::Lerp(0.24f, 0.72f, Age01) * Pulse, 1.f));
		RingMesh->SetRelativeScale3D(FVector(RadiusScale * FMath::Lerp(0.36f, 0.98f, Age01), RadiusScale * FMath::Lerp(0.36f, 0.98f, Age01), 1.f));
		EchoMesh->SetRelativeScale3D(FVector(RadiusScale * FMath::Lerp(0.52f, 1.18f, Age01), RadiusScale * FMath::Lerp(0.52f, 1.18f, Age01), 1.f));

		RingMesh->SetRelativeRotation(FRotator(90.f, Age01 * 48.f, 0.f));
		EchoMesh->SetRelativeRotation(FRotator(90.f, -Age01 * 62.f, 0.f));
	}

	if (IsHeroAOEStageVerbose() && ElapsedSeconds <= DeltaSeconds + KINDA_SMALL_NUMBER)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][ExampleAOE] Active Req=%d Source=%s Actor=%s Radius=%.1f"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			BaseRadius);
	}

	if (Age01 >= 1.f)
	{
		if (IsHeroAOEStageVerbose())
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("[ATTACK VFX][ExampleAOE] Complete Req=%d Source=%s Actor=%s Age01=%.2f Elapsed=%.3f"),
				DebugRequestId,
				DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
				*GetName(),
				Age01,
				ElapsedSeconds);
		}
		Destroy();
	}
}

void AT66HeroAOEAttackVFX::ApplyEffectTransforms()
{
	SetActorLocation(EffectLocation + FVector(0.f, 0.f, 8.f));
	SetActorRotation(FRotator::ZeroRotator);

	const float RadiusScale = BaseRadius / 100.f;
	CoreMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	CoreMesh->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	CoreMesh->SetRelativeScale3D(FVector(RadiusScale * (bUseFireFlipbook ? 0.84f : 0.28f), RadiusScale * (bUseFireFlipbook ? 0.84f : 0.28f), 1.f));

	if (!bUseFireFlipbook)
	{
		RingMesh->SetRelativeLocation(FVector(0.f, 0.f, 2.f));
		RingMesh->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
		RingMesh->SetRelativeScale3D(FVector(RadiusScale * 0.42f, RadiusScale * 0.42f, 1.f));

		EchoMesh->SetRelativeLocation(FVector(0.f, 0.f, 4.f));
		EchoMesh->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
		EchoMesh->SetRelativeScale3D(FVector(RadiusScale * 0.56f, RadiusScale * 0.56f, 1.f));
	}

	if (IsHeroAOEStageVerbose())
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][ExampleAOE] Transforms Req=%d Source=%s Actor=%s Style=%s Radius=%.1f"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			bUseFireFlipbook ? TEXT("FireFlipbook") : TEXT("DefaultPulse"),
			BaseRadius);
	}
}

void AT66HeroAOEAttackVFX::ResolveRuntimePalette(FLinearColor& OutTintColor, FLinearColor& OutPrimaryColor, FLinearColor& OutSecondaryColor, FLinearColor& OutOutlineColor, float& OutGlowStrength, FString& OutPaletteMode) const
{
	if (bUsePaletteOverride)
	{
		OutTintColor = OverrideTintColor;
		OutPrimaryColor = OverridePrimaryColor;
		OutSecondaryColor = OverrideSecondaryColor;
		OutOutlineColor = OverrideOutlineColor;
		OutGlowStrength = OverrideGlowStrength;
		OutPaletteMode = OverridePaletteMode.IsNone() ? TEXT("ExplicitOverride") : OverridePaletteMode.ToString();
		return;
	}

	BuildExampleAOERuntimePalette(TintColor, OutTintColor, OutPrimaryColor, OutSecondaryColor, OutOutlineColor, OutGlowStrength);
	OutPaletteMode = TEXT("HeroMono");
}

void AT66HeroAOEAttackVFX::LogConfiguredBegin() const
{
	if (!IsHeroAOEStageVerbose() || !CoreMID || !bInitialized)
	{
		return;
	}

	FLinearColor RuntimeTintColor;
	FLinearColor RuntimePrimaryColor;
	FLinearColor RuntimeSecondaryColor;
	FLinearColor RuntimeOutlineColor;
	float RuntimeGlowStrength = 0.f;
	FString PaletteMode;
	ResolveRuntimePalette(RuntimeTintColor, RuntimePrimaryColor, RuntimeSecondaryColor, RuntimeOutlineColor, RuntimeGlowStrength, PaletteMode);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[ATTACK VFX][ExampleAOE] Begin Req=%d Source=%s Actor=%s Style=%s PaletteMode=%s Lifetime=%.2f Radius=%.1f Tint=(%.2f,%.2f,%.2f,%.2f) Primary=(%.2f,%.2f,%.2f,%.2f) Secondary=(%.2f,%.2f,%.2f,%.2f) Outline=(%.2f,%.2f,%.2f,%.2f) Glow=%.2f"),
		DebugRequestId,
		DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
		*GetName(),
		bUseFireFlipbook ? TEXT("FireFlipbook") : TEXT("DefaultPulse"),
		*PaletteMode,
		LifetimeSeconds,
		BaseRadius,
		RuntimeTintColor.R, RuntimeTintColor.G, RuntimeTintColor.B, RuntimeTintColor.A,
		RuntimePrimaryColor.R, RuntimePrimaryColor.G, RuntimePrimaryColor.B, RuntimePrimaryColor.A,
		RuntimeSecondaryColor.R, RuntimeSecondaryColor.G, RuntimeSecondaryColor.B, RuntimeSecondaryColor.A,
		RuntimeOutlineColor.R, RuntimeOutlineColor.G, RuntimeOutlineColor.B, RuntimeOutlineColor.A,
		RuntimeGlowStrength);
}

void AT66HeroAOEAttackVFX::LogFireFlipbookBinding() const
{
	if (!IsHeroAOEStageVerbose() || !bUseFireFlipbook || !CoreMID)
	{
		return;
	}

	UTexture* FlipbookTexture = CoreMID->K2_GetTextureParameterValue(TEXT("FlipbookTexture"));
	const float Rows = CoreMID->K2_GetScalarParameterValue(TEXT("Rows"));
	const float Columns = CoreMID->K2_GetScalarParameterValue(TEXT("Columns"));
	const float Age = CoreMID->K2_GetScalarParameterValue(TEXT("Age01"));
	const float Glow = CoreMID->K2_GetScalarParameterValue(TEXT("GlowStrength"));

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[ATTACK VFX][ExampleAOE][Fire] MaterialBound Req=%d Source=%s Actor=%s Material=%s Texture=%s Rows=%.1f Columns=%.1f Age01=%.2f Glow=%.2f"),
		DebugRequestId,
		DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
		*GetName(),
		CoreBaseMaterial ? *CoreBaseMaterial->GetPathName() : TEXT("None"),
		FlipbookTexture ? *FlipbookTexture->GetPathName() : TEXT("None"),
		Rows,
		Columns,
		Age,
		Glow);

	if (!FlipbookTexture)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ATTACK VFX][ExampleAOE][Fire] Missing FlipbookTexture parameter at runtime Req=%d Source=%s Actor=%s Material=%s"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			CoreBaseMaterial ? *CoreBaseMaterial->GetPathName() : TEXT("None"));
	}
}

void AT66HeroAOEAttackVFX::UpdateMaterialParams(float Age01) const
{
	FLinearColor RuntimeTintColor;
	FLinearColor RuntimePrimaryColor;
	FLinearColor RuntimeSecondaryColor;
	FLinearColor RuntimeOutlineColor;
	float RuntimeGlowStrength = 0.f;
	FString PaletteMode;
	ResolveRuntimePalette(RuntimeTintColor, RuntimePrimaryColor, RuntimeSecondaryColor, RuntimeOutlineColor, RuntimeGlowStrength, PaletteMode);

	if (CoreMID)
	{
		CoreMID->SetScalarParameterValue(TEXT("Age01"), Age01);
		CoreMID->SetScalarParameterValue(TEXT("GlowStrength"), RuntimeGlowStrength + 0.05f);
	}

	if (bUseFireFlipbook)
	{
		return;
	}

	if (CoreMID)
	{
		CoreMID->SetVectorParameterValue(TEXT("TintColor"), RuntimeTintColor);
		CoreMID->SetVectorParameterValue(TEXT("PrimaryColor"), RuntimePrimaryColor);
		CoreMID->SetVectorParameterValue(TEXT("SecondaryColor"), RuntimeSecondaryColor);
		CoreMID->SetVectorParameterValue(TEXT("OutlineColor"), RuntimeOutlineColor);
	}

	if (RingMID)
	{
		RingMID->SetScalarParameterValue(TEXT("Age01"), FMath::Clamp(Age01 * 0.92f, 0.f, 1.f));
		RingMID->SetVectorParameterValue(TEXT("TintColor"), RuntimeTintColor);
		RingMID->SetVectorParameterValue(TEXT("PrimaryColor"), RuntimeTintColor);
		RingMID->SetVectorParameterValue(TEXT("SecondaryColor"), RuntimePrimaryColor);
		RingMID->SetVectorParameterValue(TEXT("OutlineColor"), RuntimeOutlineColor);
		RingMID->SetScalarParameterValue(TEXT("GlowStrength"), RuntimeGlowStrength);
	}

	if (EchoMID)
	{
		EchoMID->SetScalarParameterValue(TEXT("Age01"), FMath::Clamp(Age01 * 0.82f, 0.f, 1.f));
		EchoMID->SetVectorParameterValue(TEXT("TintColor"), RuntimeTintColor);
		EchoMID->SetVectorParameterValue(TEXT("PrimaryColor"), RuntimeSecondaryColor);
		EchoMID->SetVectorParameterValue(TEXT("SecondaryColor"), RuntimePrimaryColor);
		EchoMID->SetVectorParameterValue(TEXT("OutlineColor"), RuntimeOutlineColor);
		EchoMID->SetScalarParameterValue(TEXT("GlowStrength"), RuntimeGlowStrength - 0.10f);
	}
}
