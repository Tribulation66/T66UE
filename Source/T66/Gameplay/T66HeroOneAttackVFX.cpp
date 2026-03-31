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
	const TCHAR* HeroOneStreakMaterialPath = TEXT("/Game/VFX/Hero1/MI_Hero1_Attack_Streak.MI_Hero1_Attack_Streak");
	const TCHAR* HeroOneImpactMaterialPath = TEXT("/Game/VFX/Hero1/MI_Hero1_Attack_Impact.MI_Hero1_Attack_Impact");
	const TCHAR* HeroOneSwordMeshPath = TEXT("/Game/VFX/Projectiles/Hero1/Arthur_Sword.Arthur_Sword");
	const FName HeroOneID(TEXT("Hero_1"));
	static TAutoConsoleVariable<int32> CVarT66VFXHero1SwordShape(
		TEXT("T66.VFX.Hero1SwordShape"),
		1,
		TEXT("Use the stage-10 approximate sword silhouette for Hero_1 Example pierce."));

	void BuildExamplePierceRuntimePalette(
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

	GuardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GuardMesh"));
	GuardMesh->SetupAttachment(SceneRoot);
	GuardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GuardMesh->SetCastShadow(false);
	GuardMesh->SetReceivesDecals(false);
	GuardMesh->SetCanEverAffectNavigation(false);
	GuardMesh->SetMobility(EComponentMobility::Movable);
	GuardMesh->TranslucencySortPriority = 4;
	GuardMesh->SetHiddenInGame(true);

	GripMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GripMesh"));
	GripMesh->SetupAttachment(SceneRoot);
	GripMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GripMesh->SetCastShadow(false);
	GripMesh->SetReceivesDecals(false);
	GripMesh->SetCanEverAffectNavigation(false);
	GripMesh->SetMobility(EComponentMobility::Movable);
	GripMesh->TranslucencySortPriority = 2;
	GripMesh->SetHiddenInGame(true);

	PommelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PommelMesh"));
	PommelMesh->SetupAttachment(SceneRoot);
	PommelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PommelMesh->SetCastShadow(false);
	PommelMesh->SetReceivesDecals(false);
	PommelMesh->SetCanEverAffectNavigation(false);
	PommelMesh->SetMobility(EComponentMobility::Movable);
	PommelMesh->TranslucencySortPriority = 3;
	PommelMesh->SetHiddenInGame(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		StreakMesh->SetStaticMesh(PlaneMesh.Object);
		ImpactMesh->SetStaticMesh(PlaneMesh.Object);
		GuardMesh->SetStaticMesh(PlaneMesh.Object);
		GripMesh->SetStaticMesh(PlaneMesh.Object);
		PommelMesh->SetStaticMesh(PlaneMesh.Object);
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
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][ExamplePierce] Init Req=%d Source=%s Actor=%s Start=(%.1f,%.1f,%.1f) End=(%.1f,%.1f,%.1f) Impact=(%.1f,%.1f,%.1f) Tint=(%.2f,%.2f,%.2f,%.2f)"),
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

	if (!StreakBaseMaterial)
	{
		StreakBaseMaterial = LoadObject<UMaterialInterface>(
			nullptr,
			HeroOneStreakMaterialPath);
	}

	if (StreakBaseMaterial)
	{
		StreakMID = UMaterialInstanceDynamic::Create(StreakBaseMaterial, this, TEXT("Hero1AttackStreakMID"));
		if (StreakMID)
		{
			StreakMesh->SetMaterial(0, StreakMID);
			GuardMesh->SetMaterial(0, StreakMID);
			GripMesh->SetMaterial(0, StreakMID);
			PommelMesh->SetMaterial(0, StreakMID);
		}
	}

	if (!ImpactBaseMaterial)
	{
		ImpactBaseMaterial = LoadObject<UMaterialInterface>(
			nullptr,
			HeroOneImpactMaterialPath);
	}

	if (ImpactBaseMaterial)
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
			LogTemp,
			Warning,
			TEXT("[ATTACK VFX][ExamplePierce] MissingMaterials Req=%d Source=%s Actor=%s StreakMID=%s ImpactMID=%s"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			StreakMID ? TEXT("OK") : TEXT("MISSING"),
			ImpactMID ? TEXT("OK") : TEXT("MISSING"));
		Destroy();
		return;
	}

	if (!ArthurSwordStaticMesh)
	{
		ArthurSwordStaticMesh = LoadObject<UStaticMesh>(nullptr, HeroOneSwordMeshPath);
	}
	if (SwordMesh && ArthurSwordStaticMesh)
	{
		SwordMesh->SetStaticMesh(ArthurSwordStaticMesh);
	}
	else if (UseArthurSwordSilhouette())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ATTACK VFX][ExamplePierce] ImportedSwordMissing Req=%d Source=%s Actor=%s MeshPath=%s Fallback=PlaneSword"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			HeroOneSwordMeshPath);
	}

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
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][ExamplePierce] Active Req=%d Source=%s Actor=%s Trace=%.1f ImpactDistance=%.1f"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			TraceLength,
			ImpactDistance);
	}

	const float Pulse = 1.f + FMath::Sin(Age01 * PI) * 0.24f;
	if (UseArthurSwordSilhouette())
	{
		const bool bUseImportedSwordMesh = SwordMesh && SwordMesh->GetStaticMesh() != nullptr;
		const float GuardOffset = FMath::Clamp(TraceLength * 0.04f, 12.f, 18.f);
		const float BladeLength = FMath::Max(56.f, TraceLength - GuardOffset - 8.f);
		const float SwordTravelDistance = FMath::Lerp(GuardOffset + 12.f, ImpactDistance, FMath::Clamp(Age01 * 1.08f, 0.f, 1.f));

		if (bUseImportedSwordMesh)
		{
			SwordMesh->SetHiddenInGame(false);
			SwordMesh->SetRelativeLocation(FVector(SwordTravelDistance, 0.f, 2.f + (Pulse * 1.4f)));
			SwordMesh->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
			SwordMesh->SetRelativeScale3D(FVector(0.60f, 0.60f, 0.60f));

			StreakMesh->SetRelativeLocation(FVector(FMath::Max(18.f, SwordTravelDistance - (BladeLength * 0.32f)), 0.f, 0.f));
			StreakMesh->SetRelativeRotation(FRotator::ZeroRotator);
			StreakMesh->SetRelativeScale3D(FVector(FMath::Max(0.58f, (BladeLength * 0.34f) / 100.f), FMath::Lerp(0.07f, 0.11f, Age01) * Pulse, 1.f));

			ImpactMesh->SetRelativeScale3D(FVector(FMath::Lerp(0.11f, 0.15f, Age01) * Pulse, FMath::Lerp(0.14f, 0.18f, Age01) * Pulse, 1.f));
		}
		else
		{
			const float BladeWidth = FMath::Lerp(0.11f, 0.15f, Age01) * Pulse;
			const float TipScale = FMath::Lerp(0.12f, 0.16f, Age01) * Pulse;
			const float GuardWidth = FMath::Lerp(0.30f, 0.36f, Age01) * Pulse;
			const float GripLength = FMath::Lerp(0.22f, 0.26f, Age01);
			const float GripWidth = FMath::Lerp(0.060f, 0.072f, Age01) * Pulse;
			const float PommelScaleX = FMath::Lerp(0.075f, 0.090f, Age01) * Pulse;
			const float PommelScaleY = FMath::Lerp(0.100f, 0.115f, Age01) * Pulse;

			StreakMesh->SetRelativeScale3D(FVector(BladeLength / 100.f, BladeWidth, 1.f));
			ImpactMesh->SetRelativeScale3D(FVector(TipScale, TipScale * 1.28f, 1.f));
			GuardMesh->SetRelativeScale3D(FVector(0.075f, GuardWidth, 1.f));
			GripMesh->SetRelativeScale3D(FVector(GripLength, GripWidth, 1.f));
			PommelMesh->SetRelativeScale3D(FVector(PommelScaleX, PommelScaleY, 1.f));
		}
	}
	else
	{
		SwordMesh->SetHiddenInGame(true);
		const float LengthScale = FMath::Lerp(1.08f, 0.88f, Age01);
		const float WidthScale = FMath::Lerp(BaseStreakWidthScale * 0.96f, BaseStreakWidthScale * 1.8f, Age01);
		const float HeightScale = 1.f;
		StreakMesh->SetRelativeScale3D(FVector((TraceLength * LengthScale) / 100.f, WidthScale, HeightScale));

		const float ImpactScale = BaseImpactScale * FMath::Lerp(1.08f, 0.92f, Age01) * Pulse;
		ImpactMesh->SetRelativeScale3D(FVector(ImpactScale, ImpactScale, 1.f));
	}

	if (Age01 >= 1.f)
	{
		if (IsHeroOneStageVerbose())
		{
			UE_LOG(LogTemp, Log, TEXT("[ATTACK VFX][ExamplePierce] Complete Req=%d Source=%s Actor=%s Age01=%.2f Elapsed=%.3f"), DebugRequestId, DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(), *GetName(), Age01, ElapsedSeconds);
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

	if (UseArthurSwordSilhouette())
	{
		const bool bUseImportedSwordMesh = SwordMesh && SwordMesh->GetStaticMesh() != nullptr;
		const float GuardOffset = FMath::Clamp(TraceLength * 0.04f, 12.f, 18.f);
		const float BladeLength = FMath::Max(56.f, TraceLength - GuardOffset - 8.f);

		SwordMesh->SetHiddenInGame(!bUseImportedSwordMesh);
		GuardMesh->SetHiddenInGame(bUseImportedSwordMesh);
		GripMesh->SetHiddenInGame(bUseImportedSwordMesh);
		PommelMesh->SetHiddenInGame(bUseImportedSwordMesh);

		if (bUseImportedSwordMesh)
		{
			SwordMesh->SetRelativeLocation(FVector(GuardOffset + 12.f, 0.f, 2.f));
			SwordMesh->SetRelativeRotation(FRotator::ZeroRotator);
			SwordMesh->SetRelativeScale3D(FVector(0.60f, 0.60f, 0.60f));

			StreakMesh->SetRelativeLocation(FVector(FMath::Max(18.f, GuardOffset + (BladeLength * 0.18f)), 0.f, 0.f));
			StreakMesh->SetRelativeRotation(FRotator::ZeroRotator);
			StreakMesh->SetRelativeScale3D(FVector(FMath::Max(0.58f, (BladeLength * 0.34f) / 100.f), 0.08f, 1.f));

			ImpactMesh->SetRelativeLocation(FVector(ImpactDistance, 0.f, 2.f));
			ImpactMesh->SetRelativeRotation(FRotator(0.f, 45.f, 0.f));
			ImpactMesh->SetRelativeScale3D(FVector(0.13f, 0.17f, 1.f));
		}
		else
		{
			StreakMesh->SetRelativeLocation(FVector(GuardOffset + (BladeLength * 0.5f), 0.f, 0.f));
			StreakMesh->SetRelativeRotation(FRotator::ZeroRotator);
			StreakMesh->SetRelativeScale3D(FVector(BladeLength / 100.f, 0.12f, 1.f));

			ImpactMesh->SetRelativeLocation(FVector(GuardOffset + BladeLength + 2.f, 0.f, 1.f));
			ImpactMesh->SetRelativeRotation(FRotator(0.f, 45.f, 0.f));
			ImpactMesh->SetRelativeScale3D(FVector(0.13f, 0.17f, 1.f));

			GuardMesh->SetRelativeLocation(FVector(GuardOffset, 0.f, 0.f));
			GuardMesh->SetRelativeRotation(FRotator::ZeroRotator);
			GuardMesh->SetRelativeScale3D(FVector(0.075f, 0.32f, 1.f));

			GripMesh->SetRelativeLocation(FVector(-12.f, 0.f, 0.f));
			GripMesh->SetRelativeRotation(FRotator::ZeroRotator);
			GripMesh->SetRelativeScale3D(FVector(0.22f, 0.065f, 1.f));

			PommelMesh->SetRelativeLocation(FVector(-28.f, 0.f, 0.f));
			PommelMesh->SetRelativeRotation(FRotator(0.f, 45.f, 0.f));
			PommelMesh->SetRelativeScale3D(FVector(0.08f, 0.11f, 1.f));
		}
	}
	else
	{
		SwordMesh->SetHiddenInGame(true);
		GuardMesh->SetHiddenInGame(true);
		GripMesh->SetHiddenInGame(true);
		PommelMesh->SetHiddenInGame(true);

		StreakMesh->SetRelativeLocation(FVector(TraceLength * 0.5f, 0.f, 0.f));
		StreakMesh->SetRelativeRotation(FRotator::ZeroRotator);
		StreakMesh->SetRelativeScale3D(FVector(TraceLength / 100.f, BaseStreakWidthScale, 1.f));

		ImpactMesh->SetRelativeLocation(FVector(ImpactDistance, 0.f, 2.f));
		ImpactMesh->SetRelativeRotation(FRotator::ZeroRotator);
		ImpactMesh->SetRelativeScale3D(FVector(BaseImpactScale, BaseImpactScale, 1.f));
	}

	if (IsHeroOneStageVerbose())
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][ExamplePierce] Transforms Req=%d Source=%s Actor=%s ShapeMode=%s ImportedSword=%d Yaw=%.1f Trace=%.1f ImpactDistance=%.1f"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			UseArthurSwordSilhouette() ? TEXT("ArthurSword") : TEXT("DefaultPierce"),
			(SwordMesh && SwordMesh->GetStaticMesh()) ? 1 : 0,
			GetActorRotation().Yaw,
			TraceLength,
			ImpactDistance);
	}
}

bool AT66HeroOneAttackVFX::UseArthurSwordSilhouette() const
{
	return !bUsePaletteOverride
		&& DebugHeroID == HeroOneID
		&& CVarT66VFXHero1SwordShape.GetValueOnGameThread() != 0;
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

	BuildExamplePierceRuntimePalette(DebugHeroID, TintColor, OutTintColor, OutPrimaryColor, OutSecondaryColor, OutOutlineColor, OutGlowStrength);
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
		LogTemp,
		Log,
		TEXT("[ATTACK VFX][ExamplePierce] Begin Req=%d Source=%s Actor=%s ShapeMode=%s PaletteMode=%s Lifetime=%.2f BaseWidth=%.2f BaseImpact=%.2f StreakMesh=%s ImpactMesh=%s SwordMesh=%s GuardMesh=%s GripMesh=%s PommelMesh=%s PaletteTint=(%.2f,%.2f,%.2f,%.2f) Primary=(%.2f,%.2f,%.2f,%.2f) Secondary=(%.2f,%.2f,%.2f,%.2f) Outline=(%.2f,%.2f,%.2f,%.2f) Glow=%.2f"),
		DebugRequestId,
		DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
		*GetName(),
		UseArthurSwordSilhouette() ? TEXT("ArthurSword") : TEXT("DefaultPierce"),
		*PaletteMode,
		LifetimeSeconds,
		BaseStreakWidthScale,
		BaseImpactScale,
		(StreakMesh && StreakMesh->GetStaticMesh()) ? *StreakMesh->GetStaticMesh()->GetName() : TEXT("None"),
		(ImpactMesh && ImpactMesh->GetStaticMesh()) ? *ImpactMesh->GetStaticMesh()->GetName() : TEXT("None"),
		(SwordMesh && SwordMesh->GetStaticMesh()) ? *SwordMesh->GetStaticMesh()->GetName() : TEXT("None"),
		(GuardMesh && GuardMesh->GetStaticMesh()) ? *GuardMesh->GetStaticMesh()->GetName() : TEXT("None"),
		(GripMesh && GripMesh->GetStaticMesh()) ? *GripMesh->GetStaticMesh()->GetName() : TEXT("None"),
		(PommelMesh && PommelMesh->GetStaticMesh()) ? *PommelMesh->GetStaticMesh()->GetName() : TEXT("None"),
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
