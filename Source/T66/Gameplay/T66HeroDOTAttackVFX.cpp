// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HeroDOTAttackVFX.h"

#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const TCHAR* HeroDOTMaterialPath = TEXT("/Game/VFX/Hero1/MI_Hero1_Attack_Impact.MI_Hero1_Attack_Impact");

	void BuildExampleDOTRuntimePalette(
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
		VividHSV.G = FMath::Clamp(FMath::Max(VividHSV.G * 1.55f, 0.66f), 0.0f, 1.0f);
		VividHSV.B = FMath::Clamp(FMath::Max(VividHSV.B * 1.08f, 0.82f), 0.0f, 1.0f);

		const FLinearColor VividPrimary = VividHSV.HSVToLinearRGB();
		const FLinearColor BrightTint = FLinearColor::LerpUsingHSV(VividPrimary, FLinearColor::White, 0.12f);
		const FLinearColor DeepSecondary = FLinearColor::LerpUsingHSV(VividPrimary, FLinearColor::Black, 0.62f);
		const FLinearColor DeepOutline = FLinearColor::LerpUsingHSV(DeepSecondary, FLinearColor::Black, 0.72f);

		OutTintColor = BrightTint;
		OutPrimaryColor = VividPrimary;
		OutSecondaryColor = DeepSecondary;
		OutOutlineColor = DeepOutline;
		OutGlowStrength = 1.82f;
	}

	bool IsHeroDOTStageVerbose()
	{
		if (const IConsoleVariable* HeroDOTVerbose = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.VFX.HeroDOTVerbose")))
		{
			return HeroDOTVerbose->GetInt() != 0;
		}

		return false;
	}

	FVector ResolveActorOverheadAnchor(AActor* Actor)
	{
		if (!Actor)
		{
			return FVector::ZeroVector;
		}

		FVector Anchor = Actor->GetActorLocation();
		if (const UCapsuleComponent* Capsule = Actor->FindComponentByClass<UCapsuleComponent>())
		{
			Anchor.Z += Capsule->GetScaledCapsuleHalfHeight() + 26.f;
		}
		else
		{
			Anchor.Z += 96.f;
		}

		return Anchor;
	}
}

AT66HeroDOTAttackVFX::AT66HeroDOTAttackVFX()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	InitialLifeSpan = 6.0f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	auto ConfigureMesh = [&](const TCHAR* Name, const int32 SortPriority)
	{
		UStaticMeshComponent* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Mesh->SetupAttachment(SceneRoot);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetCastShadow(false);
		Mesh->SetReceivesDecals(false);
		Mesh->SetCanEverAffectNavigation(false);
		Mesh->SetMobility(EComponentMobility::Movable);
		Mesh->TranslucencySortPriority = SortPriority;
		return Mesh;
	};

	GroundMesh = ConfigureMesh(TEXT("GroundMesh"), 3);
	AuraMesh = ConfigureMesh(TEXT("AuraMesh"), 4);
	SealMesh = ConfigureMesh(TEXT("SealMesh"), 5);
	PulseMesh = ConfigureMesh(TEXT("PulseMesh"), 6);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		GroundMesh->SetStaticMesh(PlaneMesh.Object);
		AuraMesh->SetStaticMesh(PlaneMesh.Object);
		SealMesh->SetStaticMesh(PlaneMesh.Object);
		PulseMesh->SetStaticMesh(PlaneMesh.Object);
	}
}

void AT66HeroDOTAttackVFX::SetPaletteOverride(const FLinearColor& InTint, const FLinearColor& InPrimary, const FLinearColor& InSecondary, const FLinearColor& InOutline, const float InGlowStrength, const FName InPaletteMode)
{
	OverrideTintColor = InTint;
	OverridePrimaryColor = InPrimary;
	OverrideSecondaryColor = InSecondary;
	OverrideOutlineColor = InOutline;
	OverrideGlowStrength = InGlowStrength;
	OverridePaletteMode = InPaletteMode;
	bUsePaletteOverride = true;
}

void AT66HeroDOTAttackVFX::InitEffect(AActor* InFollowTarget, const FVector& InLocation, const float InDuration, const float InRadius, const FLinearColor& InTint, const int32 InDebugRequestId, const FName InDebugHeroID)
{
	FollowTarget = InFollowTarget;
	EffectLocation = InLocation;
	BaseRadius = FMath::Max(60.f, InRadius);
	LifetimeSeconds = FMath::Clamp(InDuration, 0.7f, 3.5f);
	TintColor = InTint;
	DebugRequestId = InDebugRequestId;
	DebugHeroID = InDebugHeroID;
	bInitialized = true;

	if (IsHeroDOTStageVerbose())
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][ExampleDOT] Init Req=%d Source=%s Actor=%s Follow=%d Location=(%.1f,%.1f,%.1f) Radius=%.1f Duration=%.2f Tint=(%.2f,%.2f,%.2f,%.2f)"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			FollowTarget.IsValid() ? 1 : 0,
			EffectLocation.X, EffectLocation.Y, EffectLocation.Z,
			BaseRadius,
			LifetimeSeconds,
			TintColor.R, TintColor.G, TintColor.B, TintColor.A);
	}

	if (HasActorBegunPlay())
	{
		ApplyEffectTransforms();
		UpdateMaterialParams(0.f);
		LogConfiguredBegin();
	}
}

void AT66HeroDOTAttackVFX::BeginPlay()
{
	Super::BeginPlay();

	if (!BaseMaterial)
	{
		BaseMaterial = LoadObject<UMaterialInterface>(nullptr, HeroDOTMaterialPath);
	}

	if (BaseMaterial)
	{
		GroundMID = UMaterialInstanceDynamic::Create(BaseMaterial, this, TEXT("HeroDOTGroundMID"));
		AuraMID = UMaterialInstanceDynamic::Create(BaseMaterial, this, TEXT("HeroDOTAuraMID"));
		SealMID = UMaterialInstanceDynamic::Create(BaseMaterial, this, TEXT("HeroDOTSealMID"));
		PulseMID = UMaterialInstanceDynamic::Create(BaseMaterial, this, TEXT("HeroDOTPulseMID"));

		if (GroundMID)
		{
			GroundMesh->SetMaterial(0, GroundMID);
		}
		if (AuraMID)
		{
			AuraMesh->SetMaterial(0, AuraMID);
		}
		if (SealMID)
		{
			SealMesh->SetMaterial(0, SealMID);
		}
		if (PulseMID)
		{
			PulseMesh->SetMaterial(0, PulseMID);
		}
	}

	if (!GroundMID || !AuraMID || !SealMID || !PulseMID)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ATTACK VFX][ExampleDOT] MissingMaterials Req=%d Source=%s Actor=%s Ground=%s Aura=%s Seal=%s Pulse=%s"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			GroundMID ? TEXT("OK") : TEXT("MISSING"),
			AuraMID ? TEXT("OK") : TEXT("MISSING"),
			SealMID ? TEXT("OK") : TEXT("MISSING"),
			PulseMID ? TEXT("OK") : TEXT("MISSING"));
		Destroy();
		return;
	}

	if (bInitialized)
	{
		ApplyEffectTransforms();
		UpdateMaterialParams(0.f);
		LogConfiguredBegin();
	}
}

void AT66HeroDOTAttackVFX::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ElapsedSeconds += DeltaSeconds;
	const float Age01 = FMath::Clamp(ElapsedSeconds / FMath::Max(0.01f, LifetimeSeconds), 0.f, 1.f);

	ApplyEffectTransforms();
	UpdateMaterialParams(Age01);

	const float RadiusScale = BaseRadius / 100.f;
	const float SlowPulse = 1.f + (FMath::Sin((Age01 * 2.f * PI) + 0.35f) * 0.08f);
	const float FastPulse = 1.f + (FMath::Sin((Age01 * 4.f * PI) + 0.85f) * 0.14f);

	GroundMesh->SetRelativeScale3D(FVector(RadiusScale * FMath::Lerp(0.74f, 0.96f, Age01) * SlowPulse, RadiusScale * FMath::Lerp(0.74f, 0.96f, Age01) * SlowPulse, 1.f));
	AuraMesh->SetRelativeScale3D(FVector(RadiusScale * FMath::Lerp(0.42f, 0.54f, Age01) * FastPulse, RadiusScale * FMath::Lerp(0.42f, 0.54f, Age01) * FastPulse, 1.f));
	SealMesh->SetRelativeScale3D(FVector(RadiusScale * FMath::Lerp(0.56f, 0.70f, Age01), RadiusScale * FMath::Lerp(0.56f, 0.70f, Age01), 1.f));
	PulseMesh->SetRelativeScale3D(FVector(RadiusScale * FMath::Lerp(0.20f, 0.38f, Age01) * FastPulse, RadiusScale * FMath::Lerp(0.20f, 0.38f, Age01) * FastPulse, 1.f));

	AuraMesh->SetRelativeRotation(FRotator(90.f, Age01 * 32.f, 0.f));
	SealMesh->SetRelativeRotation(FRotator(90.f, -Age01 * 58.f, 0.f));
	PulseMesh->SetRelativeRotation(FRotator(90.f, Age01 * 92.f, 0.f));

	if (IsHeroDOTStageVerbose() && ElapsedSeconds <= DeltaSeconds + KINDA_SMALL_NUMBER)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][ExampleDOT] Active Req=%d Source=%s Actor=%s Follow=%d Radius=%.1f Duration=%.2f"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			FollowTarget.IsValid() ? 1 : 0,
			BaseRadius,
			LifetimeSeconds);
	}

	if (Age01 >= 1.f)
	{
		if (IsHeroDOTStageVerbose())
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("[ATTACK VFX][ExampleDOT] Complete Req=%d Source=%s Actor=%s Age01=%.2f Elapsed=%.3f"),
				DebugRequestId,
				DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
				*GetName(),
				Age01,
				ElapsedSeconds);
		}
		Destroy();
	}
}

void AT66HeroDOTAttackVFX::ApplyEffectTransforms()
{
	SetActorLocation(ResolveAnchorLocation());
	SetActorRotation(FRotator::ZeroRotator);

	const float RadiusScale = BaseRadius / 100.f;
	GroundMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	GroundMesh->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	GroundMesh->SetRelativeScale3D(FVector(RadiusScale * 0.78f, RadiusScale * 0.78f, 1.f));

	AuraMesh->SetRelativeLocation(FVector(0.f, 0.f, 18.f));
	AuraMesh->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	AuraMesh->SetRelativeScale3D(FVector(RadiusScale * 0.44f, RadiusScale * 0.44f, 1.f));

	SealMesh->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	SealMesh->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	SealMesh->SetRelativeScale3D(FVector(RadiusScale * 0.60f, RadiusScale * 0.60f, 1.f));

	PulseMesh->SetRelativeLocation(FVector(0.f, 0.f, 24.f));
	PulseMesh->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	PulseMesh->SetRelativeScale3D(FVector(RadiusScale * 0.24f, RadiusScale * 0.24f, 1.f));

	if (IsHeroDOTStageVerbose())
	{
		const FVector Anchor = ResolveAnchorLocation();
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][ExampleDOT] Transforms Req=%d Source=%s Actor=%s Anchor=(%.1f,%.1f,%.1f) Radius=%.1f"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			Anchor.X, Anchor.Y, Anchor.Z,
			BaseRadius);
	}
}

void AT66HeroDOTAttackVFX::UpdateMaterialParams(const float Age01) const
{
	FLinearColor RuntimeTintColor;
	FLinearColor RuntimePrimaryColor;
	FLinearColor RuntimeSecondaryColor;
	FLinearColor RuntimeOutlineColor;
	float RuntimeGlowStrength = 0.f;
	FString PaletteMode;
	ResolveRuntimePalette(RuntimeTintColor, RuntimePrimaryColor, RuntimeSecondaryColor, RuntimeOutlineColor, RuntimeGlowStrength, PaletteMode);

	auto ApplyMID = [&](UMaterialInstanceDynamic* MID, const float LocalAge, const FLinearColor& Primary, const FLinearColor& Secondary, const float Glow)
	{
		if (!MID)
		{
			return;
		}

		MID->SetScalarParameterValue(TEXT("Age01"), LocalAge);
		MID->SetVectorParameterValue(TEXT("TintColor"), RuntimeTintColor);
		MID->SetVectorParameterValue(TEXT("PrimaryColor"), Primary);
		MID->SetVectorParameterValue(TEXT("SecondaryColor"), Secondary);
		MID->SetVectorParameterValue(TEXT("OutlineColor"), RuntimeOutlineColor);
		MID->SetScalarParameterValue(TEXT("GlowStrength"), Glow);
	};

	ApplyMID(GroundMID, Age01, RuntimePrimaryColor, RuntimeSecondaryColor, RuntimeGlowStrength - 0.08f);
	ApplyMID(AuraMID, FMath::Clamp(Age01 + 0.08f, 0.f, 1.f), RuntimeTintColor, RuntimePrimaryColor, RuntimeGlowStrength + 0.10f);
	ApplyMID(SealMID, FMath::Clamp(Age01 + 0.16f, 0.f, 1.f), RuntimePrimaryColor, RuntimeSecondaryColor, RuntimeGlowStrength);
	ApplyMID(PulseMID, FMath::Clamp(Age01 + 0.24f, 0.f, 1.f), RuntimeTintColor, RuntimePrimaryColor, RuntimeGlowStrength + 0.18f);
}

void AT66HeroDOTAttackVFX::LogConfiguredBegin() const
{
	if (!IsHeroDOTStageVerbose() || !GroundMID || !AuraMID || !SealMID || !PulseMID)
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
		TEXT("[ATTACK VFX][ExampleDOT] Begin Req=%d Source=%s Actor=%s PaletteMode=%s Follow=%d Radius=%.1f Duration=%.2f Tint=(%.2f,%.2f,%.2f,%.2f) Primary=(%.2f,%.2f,%.2f,%.2f) Secondary=(%.2f,%.2f,%.2f,%.2f) Outline=(%.2f,%.2f,%.2f,%.2f) Glow=%.2f"),
		DebugRequestId,
		DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
		*GetName(),
		*PaletteMode,
		FollowTarget.IsValid() ? 1 : 0,
		BaseRadius,
		LifetimeSeconds,
		RuntimeTintColor.R, RuntimeTintColor.G, RuntimeTintColor.B, RuntimeTintColor.A,
		RuntimePrimaryColor.R, RuntimePrimaryColor.G, RuntimePrimaryColor.B, RuntimePrimaryColor.A,
		RuntimeSecondaryColor.R, RuntimeSecondaryColor.G, RuntimeSecondaryColor.B, RuntimeSecondaryColor.A,
		RuntimeOutlineColor.R, RuntimeOutlineColor.G, RuntimeOutlineColor.B, RuntimeOutlineColor.A,
		RuntimeGlowStrength);
}

void AT66HeroDOTAttackVFX::ResolveRuntimePalette(FLinearColor& OutTintColor, FLinearColor& OutPrimaryColor, FLinearColor& OutSecondaryColor, FLinearColor& OutOutlineColor, float& OutGlowStrength, FString& OutPaletteMode) const
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

	BuildExampleDOTRuntimePalette(TintColor, OutTintColor, OutPrimaryColor, OutSecondaryColor, OutOutlineColor, OutGlowStrength);
	OutPaletteMode = TEXT("HeroMono");
}

FVector AT66HeroDOTAttackVFX::ResolveAnchorLocation() const
{
	if (AActor* FollowActor = FollowTarget.Get())
	{
		return ResolveActorOverheadAnchor(FollowActor);
	}

	return EffectLocation + FVector(0.f, 0.f, 96.f);
}
