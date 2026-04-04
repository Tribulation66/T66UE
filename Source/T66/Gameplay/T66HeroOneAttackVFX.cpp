// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HeroOneAttackVFX.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	DEFINE_LOG_CATEGORY_STATIC(LogT66HeroAttackVFX, Log, All);

	const TCHAR* HeroOneStreakMaterialPath = TEXT("/Game/VFX/Hero1/MI_Hero1_Attack_Streak.MI_Hero1_Attack_Streak");
	const TCHAR* HeroOneImpactMaterialPath = TEXT("/Game/VFX/Hero1/MI_Hero1_Attack_Impact.MI_Hero1_Attack_Impact");
	const TCHAR* HeroOneSwordMeshPath = TEXT("/Game/VFX/Projectiles/Hero1/Arthur_Sword.Arthur_Sword");
	const FName HeroOneID(TEXT("Hero_1"));

	UMaterialInterface* GetHeroOneStreakBaseMaterial()
	{
		return LoadObject<UMaterialInterface>(nullptr, HeroOneStreakMaterialPath);
	}

	UMaterialInterface* GetHeroOneImpactBaseMaterial()
	{
		return LoadObject<UMaterialInterface>(nullptr, HeroOneImpactMaterialPath);
	}

	UStaticMesh* GetArthurSwordMesh()
	{
		return LoadObject<UStaticMesh>(nullptr, HeroOneSwordMeshPath);
	}

	void BuildHeroPierceRuntimePalette(
		const FName HeroID,
		const FLinearColor& SeedColor,
		FLinearColor& OutTintColor,
		FLinearColor& OutPrimaryColor,
		FLinearColor& OutSecondaryColor,
		FLinearColor& OutOutlineColor,
		float& OutGlowStrength)
	{
		if (HeroID.IsNone() || HeroID == HeroOneID)
		{
			OutTintColor = FLinearColor(1.00f, 0.83f, 0.30f, 1.0f);
			OutPrimaryColor = FLinearColor(1.00f, 0.72f, 0.16f, 1.0f);
			OutSecondaryColor = FLinearColor(0.46f, 0.21f, 0.03f, 1.0f);
			OutOutlineColor = FLinearColor(0.09f, 0.02f, 0.00f, 1.0f);
			OutGlowStrength = 1.85f;
			return;
		}

		const FLinearColor ClampedSeed(
			FMath::Clamp(SeedColor.R, 0.0f, 1.0f),
			FMath::Clamp(SeedColor.G, 0.0f, 1.0f),
			FMath::Clamp(SeedColor.B, 0.0f, 1.0f),
			1.0f);

		FLinearColor VividHSV = ClampedSeed.LinearRGBToHSV();
		VividHSV.G = FMath::Clamp(FMath::Max(VividHSV.G * 2.2f, 0.82f), 0.0f, 1.0f);
		VividHSV.B = 1.0f;

		const FLinearColor VividPrimary = VividHSV.HSVToLinearRGB();
		const FLinearColor DeepAccent = FLinearColor::LerpUsingHSV(VividPrimary, FLinearColor::Black, 0.60f);

		OutTintColor = VividPrimary;
		OutPrimaryColor = VividPrimary;
		OutSecondaryColor = DeepAccent;
		OutOutlineColor = FLinearColor(0.02f, 0.02f, 0.03f, 1.0f);
		OutGlowStrength = 1.95f;
	}

	bool IsHeroOneStageVerbose()
	{
		const IConsoleVariable* HeroOneVerbose = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.VFX.Hero1Verbose"));
		if (HeroOneVerbose && HeroOneVerbose->GetInt() != 0)
		{
			return true;
		}

		if (const IConsoleVariable* HeroPierceVerbose = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.VFX.HeroPierceVerbose")))
		{
			return HeroPierceVerbose->GetInt() != 0;
		}

		return false;
	}
}

AT66HeroOneAttackVFX::AT66HeroOneAttackVFX()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	InitialLifeSpan = 1.0f;
	LifetimeSeconds = 0.18f;
	BaseStreakWidthScale = 0.26f;
	BaseImpactScale = 0.65f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	StreakMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StreakMesh"));
	StreakMesh->SetupAttachment(SceneRoot);
	StreakMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StreakMesh->SetCastShadow(false);
	StreakMesh->SetReceivesDecals(false);
	StreakMesh->SetCanEverAffectNavigation(false);
	StreakMesh->SetMobility(EComponentMobility::Movable);
	StreakMesh->TranslucencySortPriority = 3;

	ImpactMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ImpactMesh"));
	ImpactMesh->SetupAttachment(SceneRoot);
	ImpactMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ImpactMesh->SetCastShadow(false);
	ImpactMesh->SetReceivesDecals(false);
	ImpactMesh->SetCanEverAffectNavigation(false);
	ImpactMesh->SetMobility(EComponentMobility::Movable);
	ImpactMesh->TranslucencySortPriority = 4;

	SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordMesh"));
	SwordMesh->SetupAttachment(SceneRoot);
	SwordMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SwordMesh->SetCastShadow(false);
	SwordMesh->SetReceivesDecals(false);
	SwordMesh->SetCanEverAffectNavigation(false);
	SwordMesh->SetMobility(EComponentMobility::Movable);
	SwordMesh->SetHiddenInGame(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		StreakMesh->SetStaticMesh(PlaneMesh.Object);
		ImpactMesh->SetStaticMesh(PlaneMesh.Object);
	}
}

void AT66HeroOneAttackVFX::SetPaletteOverride(const FLinearColor& InTint, const FLinearColor& InPrimary, const FLinearColor& InSecondary, const FLinearColor& InOutline, const float InGlowStrength, const FName InPaletteMode)
{
	OverrideTintColor = InTint;
	OverridePrimaryColor = InPrimary;
	OverrideSecondaryColor = InSecondary;
	OverrideOutlineColor = InOutline;
	OverrideGlowStrength = InGlowStrength;
	OverridePaletteMode = InPaletteMode;
	bUsePaletteOverride = true;
}

void AT66HeroOneAttackVFX::InitEffect(const FVector& InStart, const FVector& InEnd, const FVector& InImpactLocation, const FLinearColor& InTint, const int32 InDebugRequestId, const FName InDebugHeroID)
{
	StartLocation = InStart;
	EndLocation = InEnd;
	ImpactLocation = InImpactLocation;
	TintColor = InTint;
	DebugRequestId = InDebugRequestId;
	DebugHeroID = InDebugHeroID;
	bInitialized = true;
	if (IsHeroOneStageVerbose())
	{
		UE_LOG(
			LogT66HeroAttackVFX,
			Log,
			TEXT("[ATTACK VFX][HeroPierce] Init Req=%d Source=%s Actor=%s Start=(%.1f,%.1f,%.1f) End=(%.1f,%.1f,%.1f) Impact=(%.1f,%.1f,%.1f) Tint=(%.2f,%.2f,%.2f,%.2f)"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			StartLocation.X, StartLocation.Y, StartLocation.Z,
			EndLocation.X, EndLocation.Y, EndLocation.Z,
			ImpactLocation.X, ImpactLocation.Y, ImpactLocation.Z,
			TintColor.R, TintColor.G, TintColor.B, TintColor.A);
	}
	ApplyEffectTransforms();
	LogConfiguredBegin();
}

void AT66HeroOneAttackVFX::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValid(StreakMesh) || !IsValid(ImpactMesh) || !IsValid(SwordMesh))
	{
		UE_LOG(
			LogT66HeroAttackVFX,
			Warning,
			TEXT("[ATTACK VFX][HeroPierce] MissingComponents Req=%d Source=%s Actor=%s StreakMesh=%s ImpactMesh=%s SwordMesh=%s"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			IsValid(StreakMesh) ? TEXT("OK") : TEXT("MISSING"),
			IsValid(ImpactMesh) ? TEXT("OK") : TEXT("MISSING"),
			IsValid(SwordMesh) ? TEXT("OK") : TEXT("MISSING"));
		Destroy();
		return;
	}

	if (!IsValid(StreakBaseMaterial))
	{
		StreakBaseMaterial = GetHeroOneStreakBaseMaterial();
	}

	if (IsValid(StreakBaseMaterial))
	{
		StreakMID = UMaterialInstanceDynamic::Create(StreakBaseMaterial, this, TEXT("Hero1AttackStreakMID"));
		if (StreakMID)
		{
			StreakMesh->SetMaterial(0, StreakMID);
		}
	}

	if (!IsValid(ImpactBaseMaterial))
	{
		ImpactBaseMaterial = GetHeroOneImpactBaseMaterial();
	}

	if (IsValid(ImpactBaseMaterial))
	{
		ImpactMID = UMaterialInstanceDynamic::Create(ImpactBaseMaterial, this, TEXT("Hero1AttackImpactMID"));
		if (ImpactMID)
		{
			ImpactMesh->SetMaterial(0, ImpactMID);
		}
	}

	if (!StreakMID || !ImpactMID)
	{
		UE_LOG(
			LogT66HeroAttackVFX,
			Warning,
			TEXT("[ATTACK VFX][HeroPierce] MissingMaterials Req=%d Source=%s Actor=%s StreakMID=%s ImpactMID=%s"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			StreakMID ? TEXT("OK") : TEXT("MISSING"),
			ImpactMID ? TEXT("OK") : TEXT("MISSING"));
		Destroy();
		return;
	}

	if (!IsValid(ArthurSwordStaticMesh))
	{
		ArthurSwordStaticMesh = GetArthurSwordMesh();
	}
	if (!IsValid(ArthurSwordStaticMesh))
	{
		UE_LOG(
			LogT66HeroAttackVFX,
			Warning,
			TEXT("[ATTACK VFX][HeroPierce] ImportedSwordMissing Req=%d Source=%s Actor=%s MeshPath=%s"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			HeroOneSwordMeshPath);
		Destroy();
		return;
	}
	SwordMesh->SetStaticMesh(ArthurSwordStaticMesh);

	if (bInitialized)
	{
		ApplyEffectTransforms();
		LogConfiguredBegin();
		UpdateMaterialParams(0.f);
	}
}

void AT66HeroOneAttackVFX::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ElapsedSeconds += DeltaSeconds;
	const float Age01 = FMath::Clamp(ElapsedSeconds / FMath::Max(0.01f, LifetimeSeconds), 0.f, 1.f);
	UpdateMaterialParams(Age01);

	if (IsHeroOneStageVerbose() && ElapsedSeconds <= DeltaSeconds + KINDA_SMALL_NUMBER)
	{
		UE_LOG(
			LogT66HeroAttackVFX,
			Log,
			TEXT("[ATTACK VFX][HeroPierce] Active Req=%d Source=%s Actor=%s Trace=%.1f ImpactDistance=%.1f"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			TraceLength,
			ImpactDistance);
	}

	const float Pulse = 1.f + FMath::Sin(Age01 * PI) * 0.24f;
	const float GuardOffset = FMath::Clamp(TraceLength * 0.04f, 12.f, 18.f);
	const float BladeLength = FMath::Max(56.f, TraceLength - GuardOffset - 8.f);
	const float SwordTravelDistance = FMath::Lerp(GuardOffset + 12.f, ImpactDistance, FMath::Clamp(Age01 * 1.08f, 0.f, 1.f));

	SwordMesh->SetHiddenInGame(false);
	SwordMesh->SetRelativeLocation(FVector(SwordTravelDistance, 0.f, 2.f + (Pulse * 1.4f)));
	SwordMesh->SetRelativeRotation(FRotator::ZeroRotator);
	SwordMesh->SetRelativeScale3D(FVector(0.60f, 0.60f, 0.60f));

	StreakMesh->SetRelativeLocation(FVector(FMath::Max(18.f, SwordTravelDistance - (BladeLength * 0.32f)), 0.f, 0.f));
	StreakMesh->SetRelativeRotation(FRotator::ZeroRotator);
	StreakMesh->SetRelativeScale3D(FVector(FMath::Max(0.58f, (BladeLength * 0.34f) / 100.f), FMath::Lerp(0.07f, 0.11f, Age01) * Pulse, 1.f));

	ImpactMesh->SetRelativeScale3D(FVector(FMath::Lerp(0.11f, 0.15f, Age01) * Pulse, FMath::Lerp(0.14f, 0.18f, Age01) * Pulse, 1.f));

	if (Age01 >= 1.f)
	{
		if (IsHeroOneStageVerbose())
		{
			UE_LOG(LogT66HeroAttackVFX, Log, TEXT("[ATTACK VFX][HeroPierce] Complete Req=%d Source=%s Actor=%s Age01=%.2f Elapsed=%.3f"), DebugRequestId, DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(), *GetName(), Age01, ElapsedSeconds);
		}
		Destroy();
	}
}

void AT66HeroOneAttackVFX::ApplyEffectTransforms()
{
	const FVector RawDir = EndLocation - StartLocation;
	const FVector FlatDir = FVector(RawDir.X, RawDir.Y, 0.f).GetSafeNormal();
	const FVector Dir = FlatDir.IsNearlyZero() ? RawDir.GetSafeNormal() : FlatDir;
	const FRotator EffectRotation = Dir.IsNearlyZero() ? FRotator::ZeroRotator : Dir.Rotation();

	TraceLength = FMath::Max(60.f, FVector::Dist2D(StartLocation, EndLocation));
	SetActorLocation(FVector(StartLocation.X, StartLocation.Y, StartLocation.Z + 10.f));
	SetActorRotation(FRotator(0.f, EffectRotation.Yaw, 0.f));

	const FVector LocalImpact = FRotationMatrix(GetActorRotation()).InverseTransformVector(ImpactLocation - StartLocation);
	ImpactDistance = FMath::Clamp(LocalImpact.X, 12.f, TraceLength);

	const float GuardOffset = FMath::Clamp(TraceLength * 0.04f, 12.f, 18.f);
	const float BladeLength = FMath::Max(56.f, TraceLength - GuardOffset - 8.f);

	SwordMesh->SetHiddenInGame(false);
	SwordMesh->SetRelativeLocation(FVector(GuardOffset + 12.f, 0.f, 2.f));
	SwordMesh->SetRelativeRotation(FRotator::ZeroRotator);
	SwordMesh->SetRelativeScale3D(FVector(0.60f, 0.60f, 0.60f));

	StreakMesh->SetRelativeLocation(FVector(FMath::Max(18.f, GuardOffset + (BladeLength * 0.18f)), 0.f, 0.f));
	StreakMesh->SetRelativeRotation(FRotator::ZeroRotator);
	StreakMesh->SetRelativeScale3D(FVector(FMath::Max(0.58f, (BladeLength * 0.34f) / 100.f), 0.08f, 1.f));

	ImpactMesh->SetRelativeLocation(FVector(ImpactDistance, 0.f, 2.f));
	ImpactMesh->SetRelativeRotation(FRotator(0.f, 45.f, 0.f));
	ImpactMesh->SetRelativeScale3D(FVector(0.13f, 0.17f, 1.f));

	if (IsHeroOneStageVerbose())
	{
		UE_LOG(
			LogT66HeroAttackVFX,
			Log,
			TEXT("[ATTACK VFX][HeroPierce] Transforms Req=%d Source=%s Actor=%s ImportedSword=%d Yaw=%.1f Trace=%.1f ImpactDistance=%.1f"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			(SwordMesh && SwordMesh->GetStaticMesh()) ? 1 : 0,
			GetActorRotation().Yaw,
			TraceLength,
			ImpactDistance);
	}
}

void AT66HeroOneAttackVFX::ResolveRuntimePalette(FLinearColor& OutTintColor, FLinearColor& OutPrimaryColor, FLinearColor& OutSecondaryColor, FLinearColor& OutOutlineColor, float& OutGlowStrength, FString& OutPaletteMode) const
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

	BuildHeroPierceRuntimePalette(DebugHeroID, TintColor, OutTintColor, OutPrimaryColor, OutSecondaryColor, OutOutlineColor, OutGlowStrength);
	OutPaletteMode = (DebugHeroID.IsNone() || DebugHeroID == HeroOneID) ? TEXT("Hero1Warm") : TEXT("DerivedFromHero");
}

void AT66HeroOneAttackVFX::LogConfiguredBegin() const
{
	if (!IsHeroOneStageVerbose() || !StreakMID || !ImpactMID || !bInitialized)
	{
		return;
	}

	FLinearColor RuntimeTintColor;
	FLinearColor RuntimePrimaryColor;
	FLinearColor RuntimeSecondaryColor;
	FLinearColor RuntimeOutlineColor;
	float RuntimeGlowStrength = 0.0f;
	FString PaletteMode;
	ResolveRuntimePalette(RuntimeTintColor, RuntimePrimaryColor, RuntimeSecondaryColor, RuntimeOutlineColor, RuntimeGlowStrength, PaletteMode);

	UE_LOG(
		LogT66HeroAttackVFX,
		Log,
		TEXT("[ATTACK VFX][HeroPierce] Begin Req=%d Source=%s Actor=%s PaletteMode=%s Lifetime=%.2f BaseWidth=%.2f BaseImpact=%.2f StreakMesh=%s ImpactMesh=%s SwordMesh=%s PaletteTint=(%.2f,%.2f,%.2f,%.2f) Primary=(%.2f,%.2f,%.2f,%.2f) Secondary=(%.2f,%.2f,%.2f,%.2f) Outline=(%.2f,%.2f,%.2f,%.2f) Glow=%.2f"),
		DebugRequestId,
		DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
		*GetName(),
		*PaletteMode,
		LifetimeSeconds,
		BaseStreakWidthScale,
		BaseImpactScale,
		(StreakMesh && StreakMesh->GetStaticMesh()) ? *StreakMesh->GetStaticMesh()->GetName() : TEXT("None"),
		(ImpactMesh && ImpactMesh->GetStaticMesh()) ? *ImpactMesh->GetStaticMesh()->GetName() : TEXT("None"),
		(SwordMesh && SwordMesh->GetStaticMesh()) ? *SwordMesh->GetStaticMesh()->GetName() : TEXT("None"),
		RuntimeTintColor.R, RuntimeTintColor.G, RuntimeTintColor.B, RuntimeTintColor.A,
		RuntimePrimaryColor.R, RuntimePrimaryColor.G, RuntimePrimaryColor.B, RuntimePrimaryColor.A,
		RuntimeSecondaryColor.R, RuntimeSecondaryColor.G, RuntimeSecondaryColor.B, RuntimeSecondaryColor.A,
		RuntimeOutlineColor.R, RuntimeOutlineColor.G, RuntimeOutlineColor.B, RuntimeOutlineColor.A,
		RuntimeGlowStrength);
}

void AT66HeroOneAttackVFX::UpdateMaterialParams(float Age01) const
{
	FLinearColor RuntimeTintColor;
	FLinearColor RuntimePrimaryColor;
	FLinearColor RuntimeSecondaryColor;
	FLinearColor RuntimeOutlineColor;
	float RuntimeGlowStrength = 0.0f;
	FString PaletteMode;
	ResolveRuntimePalette(RuntimeTintColor, RuntimePrimaryColor, RuntimeSecondaryColor, RuntimeOutlineColor, RuntimeGlowStrength, PaletteMode);

	if (StreakMID)
	{
		StreakMID->SetScalarParameterValue(TEXT("Age01"), Age01);
		StreakMID->SetVectorParameterValue(TEXT("TintColor"), RuntimeTintColor);
		StreakMID->SetVectorParameterValue(TEXT("PrimaryColor"), RuntimePrimaryColor);
		StreakMID->SetVectorParameterValue(TEXT("SecondaryColor"), RuntimeSecondaryColor);
		StreakMID->SetVectorParameterValue(TEXT("OutlineColor"), RuntimeOutlineColor);
		StreakMID->SetScalarParameterValue(TEXT("GlowStrength"), RuntimeGlowStrength);
	}

	if (ImpactMID)
	{
		ImpactMID->SetScalarParameterValue(TEXT("Age01"), Age01);
		ImpactMID->SetVectorParameterValue(TEXT("TintColor"), RuntimeTintColor);
		ImpactMID->SetVectorParameterValue(TEXT("PrimaryColor"), RuntimePrimaryColor);
		ImpactMID->SetVectorParameterValue(TEXT("SecondaryColor"), RuntimeSecondaryColor);
		ImpactMID->SetVectorParameterValue(TEXT("OutlineColor"), RuntimeOutlineColor);
		ImpactMID->SetScalarParameterValue(TEXT("GlowStrength"), RuntimeGlowStrength + 0.10f);
	}
}
