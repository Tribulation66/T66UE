// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HeroBounceAttackVFX.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const TCHAR* HeroBounceSegmentMaterialPath = TEXT("/Game/VFX/Hero1/MI_Hero1_Attack_Streak.MI_Hero1_Attack_Streak");
	const TCHAR* HeroBounceImpactMaterialPath = TEXT("/Game/VFX/Hero1/MI_Hero1_Attack_Impact.MI_Hero1_Attack_Impact");

	void BuildExampleBounceRuntimePalette(
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
		VividHSV.G = FMath::Clamp(FMath::Max(VividHSV.G * 1.65f, 0.74f), 0.0f, 1.0f);
		VividHSV.B = 1.0f;

		const FLinearColor VividPrimary = VividHSV.HSVToLinearRGB();
		const FLinearColor BrightTint = FLinearColor::LerpUsingHSV(VividPrimary, FLinearColor::White, 0.14f);
		const FLinearColor DeepSecondary = FLinearColor::LerpUsingHSV(VividPrimary, FLinearColor::Black, 0.60f);
		const FLinearColor DeepOutline = FLinearColor::LerpUsingHSV(DeepSecondary, FLinearColor::Black, 0.72f);

		OutTintColor = BrightTint;
		OutPrimaryColor = VividPrimary;
		OutSecondaryColor = DeepSecondary;
		OutOutlineColor = DeepOutline;
		OutGlowStrength = 1.90f;
	}

	bool IsHeroBounceStageVerbose()
	{
		if (const IConsoleVariable* HeroBounceVerbose = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.VFX.HeroBounceVerbose")))
		{
			return HeroBounceVerbose->GetInt() != 0;
		}

		return false;
	}
}

AT66HeroBounceAttackVFX::AT66HeroBounceAttackVFX()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	InitialLifeSpan = 1.0f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMeshFinder.Succeeded())
	{
		PlaneMesh = PlaneMeshFinder.Object;
	}
}

void AT66HeroBounceAttackVFX::SetPaletteOverride(const FLinearColor& InTint, const FLinearColor& InPrimary, const FLinearColor& InSecondary, const FLinearColor& InOutline, const float InGlowStrength, const FName InPaletteMode)
{
	OverrideTintColor = InTint;
	OverridePrimaryColor = InPrimary;
	OverrideSecondaryColor = InSecondary;
	OverrideOutlineColor = InOutline;
	OverrideGlowStrength = InGlowStrength;
	OverridePaletteMode = InPaletteMode;
	bUsePaletteOverride = true;
}

void AT66HeroBounceAttackVFX::InitEffect(const TArray<FVector>& InChainPositions, const FLinearColor& InTint, const int32 InDebugRequestId, const FName InDebugHeroID)
{
	ChainPositions = InChainPositions;
	TintColor = InTint;
	DebugRequestId = InDebugRequestId;
	DebugHeroID = InDebugHeroID;
	bInitialized = true;

	if (IsHeroBounceStageVerbose())
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][ExampleBounce] Init Req=%d Source=%s Actor=%s Links=%d Tint=(%.2f,%.2f,%.2f,%.2f)"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			FMath::Max(0, ChainPositions.Num() - 1),
			TintColor.R, TintColor.G, TintColor.B, TintColor.A);
	}

	if (HasActorBegunPlay())
	{
		ResolveAssets();
		BuildEffect();
		if (!bBuilt)
		{
			Destroy();
			return;
		}

		UpdatePieces(0.f);
		LogConfiguredBegin();
	}
}

void AT66HeroBounceAttackVFX::BeginPlay()
{
	Super::BeginPlay();

	ResolveAssets();
	if (bInitialized)
	{
		BuildEffect();
		if (!bBuilt)
		{
			Destroy();
			return;
		}

		UpdatePieces(0.f);
		LogConfiguredBegin();
	}
}

void AT66HeroBounceAttackVFX::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ElapsedSeconds += DeltaSeconds;
	const float Age01 = FMath::Clamp(ElapsedSeconds / FMath::Max(0.01f, LifetimeSeconds), 0.f, 1.f);
	UpdatePieces(Age01);

	if (IsHeroBounceStageVerbose() && ElapsedSeconds <= DeltaSeconds + KINDA_SMALL_NUMBER)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][ExampleBounce] Active Req=%d Source=%s Actor=%s Links=%d"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			FMath::Max(0, ChainPositions.Num() - 1));
	}

	if (Age01 >= 1.f)
	{
		if (IsHeroBounceStageVerbose())
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("[ATTACK VFX][ExampleBounce] Complete Req=%d Source=%s Actor=%s Age01=%.2f Elapsed=%.3f"),
				DebugRequestId,
				DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
				*GetName(),
				Age01,
				ElapsedSeconds);
		}
		Destroy();
	}
}

void AT66HeroBounceAttackVFX::ResolveAssets()
{
	if (!SegmentBaseMaterial)
	{
		SegmentBaseMaterial = LoadObject<UMaterialInterface>(nullptr, HeroBounceSegmentMaterialPath);
	}

	if (!ImpactBaseMaterial)
	{
		ImpactBaseMaterial = LoadObject<UMaterialInterface>(nullptr, HeroBounceImpactMaterialPath);
	}
}

void AT66HeroBounceAttackVFX::BuildEffect()
{
	if (bBuilt)
	{
		return;
	}

	if (!bInitialized || !PlaneMesh || !SegmentBaseMaterial || !ImpactBaseMaterial || ChainPositions.Num() < 2)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ATTACK VFX][ExampleBounce] BuildEffect aborted Req=%d Source=%s Actor=%s Plane=%d SegmentMat=%d ImpactMat=%d ChainPoints=%d"),
			DebugRequestId,
			DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
			*GetName(),
			PlaneMesh ? 1 : 0,
			SegmentBaseMaterial ? 1 : 0,
			ImpactBaseMaterial ? 1 : 0,
			ChainPositions.Num());
		return;
	}

	const FVector RootLocation = ChainPositions[0] + FVector(0.f, 0.f, 10.f);
	SetActorLocation(RootLocation);
	SetActorRotation(FRotator::ZeroRotator);

	for (int32 SegmentIndex = 0; SegmentIndex < ChainPositions.Num() - 1; ++SegmentIndex)
	{
		const FVector A = ChainPositions[SegmentIndex];
		const FVector B = ChainPositions[SegmentIndex + 1];
		const FVector RawDir = B - A;
		const FVector FlatDir = FVector(RawDir.X, RawDir.Y, 0.f).GetSafeNormal();
		const FVector Dir = FlatDir.IsNearlyZero() ? RawDir.GetSafeNormal() : FlatDir;
		if (Dir.IsNearlyZero())
		{
			continue;
		}

		const float Dist = FMath::Max(50.f, FVector::Dist2D(A, B));
		const FVector Midpoint = FMath::Lerp(A, B, 0.5f) + FVector(0.f, 0.f, 12.f);
		const FRotator Rotation(0.f, Dir.Rotation().Yaw, 0.f);

		auto AddPiece = [&](const FString& PieceName, UMaterialInterface* BaseMaterial, const FVector& Location, const FRotator& PieceRotation, const FVector& Scale, const int32 SortPriority, const bool bImpactPiece, const float Phase)
		{
			UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(this, *PieceName);
			Mesh->SetupAttachment(SceneRoot);
			Mesh->SetStaticMesh(PlaneMesh);
			Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Mesh->SetCastShadow(false);
			Mesh->SetReceivesDecals(false);
			Mesh->SetCanEverAffectNavigation(false);
			Mesh->SetMobility(EComponentMobility::Movable);
			Mesh->TranslucencySortPriority = SortPriority;
			Mesh->RegisterComponent();

			UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, this, *FString::Printf(TEXT("%sMID"), *PieceName));
			if (MID)
			{
				Mesh->SetMaterial(0, MID);
			}

			const FVector RelativeLocation = Location - RootLocation;
			Mesh->SetRelativeLocation(RelativeLocation);
			Mesh->SetRelativeRotation(PieceRotation);
			Mesh->SetRelativeScale3D(Scale);

			SpawnedMeshes.Add(Mesh);
			SpawnedMIDs.Add(MID);

			FT66HeroBounceVisualPiece& Piece = VisualPieces.AddDefaulted_GetRef();
			Piece.Mesh = Mesh;
			Piece.MID = MID;
			Piece.BaseLocation = RelativeLocation;
			Piece.BaseRotation = PieceRotation;
			Piece.BaseScale = Scale;
			Piece.Phase = Phase;
			Piece.bImpact = bImpactPiece;
		};

		AddPiece(
			FString::Printf(TEXT("BounceCore_%d"), SegmentIndex),
			SegmentBaseMaterial,
			Midpoint,
			Rotation,
			FVector(Dist / 100.f, 0.22f, 1.f),
			3,
			false,
			SegmentIndex * 0.34f);

		AddPiece(
			FString::Printf(TEXT("BounceEcho_%d"), SegmentIndex),
			SegmentBaseMaterial,
			Midpoint + FVector(0.f, (SegmentIndex % 2 == 0 ? 6.f : -6.f), 4.f),
			FRotator(0.f, Rotation.Yaw + (SegmentIndex % 2 == 0 ? 4.f : -4.f), 0.f),
			FVector((Dist * 0.86f) / 100.f, 0.14f, 1.f),
			4,
			false,
			0.7f + SegmentIndex * 0.28f);

		AddPiece(
			FString::Printf(TEXT("BounceImpact_%d"), SegmentIndex),
			ImpactBaseMaterial,
			B + FVector(0.f, 0.f, 14.f),
			FRotator::ZeroRotator,
			FVector(0.42f, 0.42f, 1.f),
			5,
			true,
			0.22f + SegmentIndex * 0.18f);
	}

	bBuilt = VisualPieces.Num() > 0;
}

void AT66HeroBounceAttackVFX::ResolveRuntimePalette(FLinearColor& OutTintColor, FLinearColor& OutPrimaryColor, FLinearColor& OutSecondaryColor, FLinearColor& OutOutlineColor, float& OutGlowStrength, FString& OutPaletteMode) const
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

	BuildExampleBounceRuntimePalette(TintColor, OutTintColor, OutPrimaryColor, OutSecondaryColor, OutOutlineColor, OutGlowStrength);
	OutPaletteMode = TEXT("HeroMono");
}

void AT66HeroBounceAttackVFX::UpdatePieces(const float Age01) const
{
	FLinearColor RuntimeTintColor;
	FLinearColor RuntimePrimaryColor;
	FLinearColor RuntimeSecondaryColor;
	FLinearColor RuntimeOutlineColor;
	float RuntimeGlowStrength = 0.f;
	FString PaletteMode;
	ResolveRuntimePalette(RuntimeTintColor, RuntimePrimaryColor, RuntimeSecondaryColor, RuntimeOutlineColor, RuntimeGlowStrength, PaletteMode);

	for (const FT66HeroBounceVisualPiece& Piece : VisualPieces)
	{
		if (!Piece.Mesh || !Piece.MID)
		{
			continue;
		}

		const float LocalAge = FMath::Clamp(Age01 + Piece.Phase * 0.08f, 0.f, 1.f);
		const float Wave = FMath::Sin((Age01 + Piece.Phase) * PI);
		const FVector Scale = Piece.bImpact
			? FVector(Piece.BaseScale.X * FMath::Lerp(0.86f, 1.32f, LocalAge), Piece.BaseScale.Y * FMath::Lerp(0.86f, 1.32f, LocalAge), 1.f)
			: FVector(Piece.BaseScale.X * FMath::Lerp(1.02f, 0.88f, Age01), Piece.BaseScale.Y * (1.f + Wave * 0.16f), 1.f);

		Piece.Mesh->SetRelativeLocation(Piece.BaseLocation + FVector(0.f, 0.f, Piece.bImpact ? Wave * 3.f : 0.f));
		Piece.Mesh->SetRelativeRotation(Piece.bImpact ? FRotator(90.f, LocalAge * 64.f, 0.f) : Piece.BaseRotation);
		Piece.Mesh->SetRelativeScale3D(Scale);

		Piece.MID->SetScalarParameterValue(TEXT("Age01"), LocalAge);
		Piece.MID->SetVectorParameterValue(TEXT("TintColor"), RuntimeTintColor);
		Piece.MID->SetVectorParameterValue(TEXT("PrimaryColor"), Piece.bImpact ? RuntimeTintColor : RuntimePrimaryColor);
		Piece.MID->SetVectorParameterValue(TEXT("SecondaryColor"), Piece.bImpact ? RuntimePrimaryColor : RuntimeSecondaryColor);
		Piece.MID->SetVectorParameterValue(TEXT("OutlineColor"), RuntimeOutlineColor);
		Piece.MID->SetScalarParameterValue(TEXT("GlowStrength"), Piece.bImpact ? (RuntimeGlowStrength + 0.10f) : RuntimeGlowStrength);
	}
}

void AT66HeroBounceAttackVFX::LogConfiguredBegin() const
{
	if (!IsHeroBounceStageVerbose() || !bBuilt)
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
		TEXT("[ATTACK VFX][ExampleBounce] Begin Req=%d Source=%s Actor=%s PaletteMode=%s Links=%d Tint=(%.2f,%.2f,%.2f,%.2f) Primary=(%.2f,%.2f,%.2f,%.2f) Secondary=(%.2f,%.2f,%.2f,%.2f) Outline=(%.2f,%.2f,%.2f,%.2f) Glow=%.2f"),
		DebugRequestId,
		DebugHeroID.IsNone() ? TEXT("Unknown") : *DebugHeroID.ToString(),
		*GetName(),
		*PaletteMode,
		FMath::Max(0, ChainPositions.Num() - 1),
		RuntimeTintColor.R, RuntimeTintColor.G, RuntimeTintColor.B, RuntimeTintColor.A,
		RuntimePrimaryColor.R, RuntimePrimaryColor.G, RuntimePrimaryColor.B, RuntimePrimaryColor.A,
		RuntimeSecondaryColor.R, RuntimeSecondaryColor.G, RuntimeSecondaryColor.B, RuntimeSecondaryColor.A,
		RuntimeOutlineColor.R, RuntimeOutlineColor.G, RuntimeOutlineColor.B, RuntimeOutlineColor.A,
		RuntimeGlowStrength);
}
