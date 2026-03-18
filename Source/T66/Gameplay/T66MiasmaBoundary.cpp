// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiasmaBoundary.h"

#include "Components/SceneComponent.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Gameplay/T66HeroBase.h"
#include "ImageUtils.h"
#include "Kismet/GameplayStatics.h"
#include "KismetProceduralMeshLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/Paths.h"
#include "ProceduralMeshComponent.h"

namespace
{
	struct FT66BoundaryMeshSection
	{
		TArray<FVector> Verts;
		TArray<int32> Tris;
		TArray<FVector> Normals;
		TArray<FVector2D> UVs;

		bool HasGeometry() const
		{
			return Verts.Num() > 0 && Tris.Num() > 0;
		}
	};

	struct FT66BoundarySample
	{
		float Along = 0.f;
		float GroundZ = 0.f;
		float VerticalFaceTopZ = 0.f;
		float RidgeDepth = 0.f;
		float RidgeHeight = 0.f;
		float OuterDepth = 0.f;
		float SampleBaseSink = 0.f;
		float OuterDrop = 0.f;
	};

	enum class ET66BoundarySide : uint8
	{
		North,
		South,
		East,
		West
	};

	static FVector T66GetBoundaryAlongDirection(ET66BoundarySide Side)
	{
		switch (Side)
		{
		case ET66BoundarySide::East:
		case ET66BoundarySide::West:
			return FVector(0.f, 1.f, 0.f);
		default:
			return FVector(1.f, 0.f, 0.f);
		}
	}

	static FVector T66GetBoundaryOutwardDirection(ET66BoundarySide Side)
	{
		switch (Side)
		{
		case ET66BoundarySide::North:
			return FVector(0.f, 1.f, 0.f);
		case ET66BoundarySide::South:
			return FVector(0.f, -1.f, 0.f);
		case ET66BoundarySide::East:
			return FVector(1.f, 0.f, 0.f);
		default:
			return FVector(-1.f, 0.f, 0.f);
		}
	}

	static FVector T66GetBoundaryOrigin(ET66BoundarySide Side, float SafeHalfExtent)
	{
		switch (Side)
		{
		case ET66BoundarySide::North:
			return FVector(0.f, SafeHalfExtent, 0.f);
		case ET66BoundarySide::South:
			return FVector(0.f, -SafeHalfExtent, 0.f);
		case ET66BoundarySide::East:
			return FVector(SafeHalfExtent, 0.f, 0.f);
		default:
			return FVector(-SafeHalfExtent, 0.f, 0.f);
		}
	}

	static FVector T66MakeBoundaryPoint(ET66BoundarySide Side, float SafeHalfExtent, float Along, float OutwardOffset, float Z)
	{
		return T66GetBoundaryOrigin(Side, SafeHalfExtent)
			+ T66GetBoundaryAlongDirection(Side) * Along
			+ T66GetBoundaryOutwardDirection(Side) * OutwardOffset
			+ FVector(0.f, 0.f, Z);
	}

	static uint32 T66BoundaryHash(uint32 Seed, uint32 A, uint32 B)
	{
		uint32 Value = Seed ^ 0x9E3779B9u;
		Value ^= A + 0x7F4A7C15u + (Value << 6) + (Value >> 2);
		Value ^= B + 0x85EBCA6Bu + (Value << 6) + (Value >> 2);
		return Value;
	}

	static float T66SampleGroundZ(UWorld* World, const AActor* IgnoredActor, const FVector& SampleLocation, float FallbackZ)
	{
		if (!World)
		{
			return FallbackZ;
		}

		FCollisionQueryParams Params(SCENE_QUERY_STAT(T66BoundaryGroundTrace), false, IgnoredActor);
		FHitResult Hit;
		const FVector TraceStart = SampleLocation + FVector(0.f, 0.f, 8000.f);
		const FVector TraceEnd = SampleLocation - FVector(0.f, 0.f, 20000.f);

		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params) ||
			World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
		{
			return Hit.ImpactPoint.Z;
		}

		return FallbackZ;
	}

	static void T66AppendQuadFacing(
		FT66BoundaryMeshSection& Section,
		const FVector& A,
		const FVector& B,
		const FVector& C,
		const FVector& D,
		const FVector& PreferredNormal)
	{
		FVector V0 = A;
		FVector V1 = B;
		FVector V2 = C;
		FVector V3 = D;

		FVector FaceNormal = FVector::CrossProduct(V1 - V0, V2 - V0).GetSafeNormal();
		if (!PreferredNormal.IsNearlyZero() && FVector::DotProduct(FaceNormal, PreferredNormal) < 0.f)
		{
			V1 = D;
			V2 = C;
			V3 = B;
			FaceNormal = FVector::CrossProduct(V1 - V0, V2 - V0).GetSafeNormal();
		}

		const float USize = FMath::Max((V1 - V0).Size() / 800.f, 1.f);
		const float VSize = FMath::Max((V3 - V0).Size() / 800.f, 1.f);
		const int32 Base = Section.Verts.Num();

		Section.Verts.Add(V0);
		Section.Verts.Add(V1);
		Section.Verts.Add(V2);
		Section.Verts.Add(V3);

		Section.Normals.Add(FaceNormal);
		Section.Normals.Add(FaceNormal);
		Section.Normals.Add(FaceNormal);
		Section.Normals.Add(FaceNormal);

		Section.UVs.Add(FVector2D(0.f, 0.f));
		Section.UVs.Add(FVector2D(USize, 0.f));
		Section.UVs.Add(FVector2D(USize, VSize));
		Section.UVs.Add(FVector2D(0.f, VSize));

		Section.Tris.Add(Base);
		Section.Tris.Add(Base + 1);
		Section.Tris.Add(Base + 2);
		Section.Tris.Add(Base);
		Section.Tris.Add(Base + 2);
		Section.Tris.Add(Base + 3);
	}

	static void T66AppendTriangleFacing(
		FT66BoundaryMeshSection& Section,
		const FVector& A,
		const FVector& B,
		const FVector& C,
		const FVector& PreferredNormal)
	{
		FVector V0 = A;
		FVector V1 = B;
		FVector V2 = C;

		FVector FaceNormal = FVector::CrossProduct(V1 - V0, V2 - V0).GetSafeNormal();
		if (!PreferredNormal.IsNearlyZero() && FVector::DotProduct(FaceNormal, PreferredNormal) < 0.f)
		{
			Swap(V1, V2);
			FaceNormal = FVector::CrossProduct(V1 - V0, V2 - V0).GetSafeNormal();
		}

		const int32 Base = Section.Verts.Num();
		Section.Verts.Add(V0);
		Section.Verts.Add(V1);
		Section.Verts.Add(V2);

		Section.Normals.Add(FaceNormal);
		Section.Normals.Add(FaceNormal);
		Section.Normals.Add(FaceNormal);

		Section.UVs.Add(FVector2D(0.f, 0.f));
		Section.UVs.Add(FVector2D(FMath::Max((V1 - V0).Size() / 800.f, 1.f), 0.f));
		Section.UVs.Add(FVector2D(0.f, FMath::Max((V2 - V0).Size() / 800.f, 1.f)));

		Section.Tris.Add(Base);
		Section.Tris.Add(Base + 1);
		Section.Tris.Add(Base + 2);
	}

	static void T66LoadBoundaryCliffMaterials(TArray<UMaterialInterface*>& OutMaterials)
	{
		static const TCHAR* PreferredPaths[] = {
			TEXT("/Game/World/Cliffs/MI_HillTile1.MI_HillTile1"),
			TEXT("/Game/World/Cliffs/MI_HillTile2.MI_HillTile2"),
			TEXT("/Game/World/Cliffs/MI_HillTile3.MI_HillTile3"),
			TEXT("/Game/World/Cliffs/MI_HillTile4.MI_HillTile4"),
		};

		for (const TCHAR* Path : PreferredPaths)
		{
			if (UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, Path))
			{
				OutMaterials.Add(Material);
			}
		}
	}

	static UTexture2D* T66LoadOuterWallTexture()
	{
		static TObjectPtr<UTexture2D> CachedTexture = nullptr;
		if (CachedTexture)
		{
			return CachedTexture.Get();
		}

		static const TCHAR* ImportedTexturePaths[] = {
			TEXT("/Game/World/Cliffs/T_OuterWallTexture.T_OuterWallTexture"),
			TEXT("/Game/World/Cliffs/OuterWallTexture.OuterWallTexture"),
		};
		for (const TCHAR* TexturePath : ImportedTexturePaths)
		{
			if (UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, TexturePath))
			{
				CachedTexture = Texture;
				return Texture;
			}
		}

		const FString SourceTexturePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("SourceAssets/OuterWallTexture.png"));
		if (!FPaths::FileExists(SourceTexturePath))
		{
			return nullptr;
		}

		UTexture2D* Texture = FImageUtils::ImportFileAsTexture2D(SourceTexturePath);
		if (!Texture)
		{
			return nullptr;
		}

		Texture->SRGB = true;
		Texture->LODGroup = TEXTUREGROUP_World;
		Texture->UpdateResource();
		Texture->AddToRoot();
		CachedTexture = Texture;
		return Texture;
	}

	static UMaterialInterface* T66CreateOuterWallMaterial(UObject* Outer)
	{
		static const TCHAR* ImportedMaterialPaths[] = {
			TEXT("/Game/World/Cliffs/MI_OuterWallTexture.MI_OuterWallTexture"),
			TEXT("/Game/World/Cliffs/MI_OuterWall.MI_OuterWall"),
		};
		for (const TCHAR* MaterialPath : ImportedMaterialPaths)
		{
			if (UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, MaterialPath))
			{
				return Material;
			}
		}

		UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
		if (!BaseMaterial)
		{
			return nullptr;
		}

		UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Outer);
		if (!Material)
		{
			return nullptr;
		}

		UTexture* ColorTexture = T66LoadOuterWallTexture();
		if (!ColorTexture)
		{
			ColorTexture = LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
		}
		if (!ColorTexture)
		{
			ColorTexture = LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"));
		}
		if (ColorTexture)
		{
			Material->SetTextureParameterValue(TEXT("DiffuseColorMap"), ColorTexture);
			Material->SetTextureParameterValue(TEXT("BaseColorTexture"), ColorTexture);
		}

		const FLinearColor RockTint = T66LoadOuterWallTexture()
			? FLinearColor::White
			: FLinearColor(0.31f, 0.26f, 0.20f, 1.f);
		Material->SetVectorParameterValue(TEXT("Tint"), RockTint);
		Material->SetVectorParameterValue(TEXT("BaseColor"), RockTint);
		Material->SetScalarParameterValue(TEXT("Brightness"), 1.f);
		return Material;
	}

	static void T66AppendBoundarySegment(
		FT66BoundaryMeshSection& Section,
		ET66BoundarySide Side,
		float SafeHalfExtent,
		const FT66BoundarySample& A,
		const FT66BoundarySample& B,
		bool bCapStart,
		bool bCapEnd)
	{
		const FVector AlongDir = T66GetBoundaryAlongDirection(Side);
		const FVector OutwardDir = T66GetBoundaryOutwardDirection(Side);

		const FVector AInner = T66MakeBoundaryPoint(Side, SafeHalfExtent, A.Along, 0.f, A.GroundZ - A.SampleBaseSink);
		const FVector AKnee = T66MakeBoundaryPoint(Side, SafeHalfExtent, A.Along, 0.f, A.VerticalFaceTopZ);
		const FVector ARidge = T66MakeBoundaryPoint(Side, SafeHalfExtent, A.Along, A.RidgeDepth, A.GroundZ + A.RidgeHeight);
		const FVector AOuter = T66MakeBoundaryPoint(Side, SafeHalfExtent, A.Along, A.OuterDepth, A.GroundZ - A.SampleBaseSink - A.OuterDrop);

		const FVector BInner = T66MakeBoundaryPoint(Side, SafeHalfExtent, B.Along, 0.f, B.GroundZ - B.SampleBaseSink);
		const FVector BKnee = T66MakeBoundaryPoint(Side, SafeHalfExtent, B.Along, 0.f, B.VerticalFaceTopZ);
		const FVector BRidge = T66MakeBoundaryPoint(Side, SafeHalfExtent, B.Along, B.RidgeDepth, B.GroundZ + B.RidgeHeight);
		const FVector BOuter = T66MakeBoundaryPoint(Side, SafeHalfExtent, B.Along, B.OuterDepth, B.GroundZ - B.SampleBaseSink - B.OuterDrop);

		T66AppendQuadFacing(Section, AInner, BInner, BKnee, AKnee, -OutwardDir);
		T66AppendQuadFacing(Section, AKnee, BKnee, BRidge, ARidge, -OutwardDir);
		T66AppendQuadFacing(Section, ARidge, BRidge, BOuter, AOuter, OutwardDir);
		T66AppendQuadFacing(Section, AOuter, BOuter, BInner, AInner, FVector(0.f, 0.f, -1.f));

		if (bCapStart)
		{
			T66AppendQuadFacing(Section, AInner, AKnee, ARidge, AOuter, -AlongDir);
		}
		if (bCapEnd)
		{
			T66AppendQuadFacing(Section, BInner, BOuter, BRidge, BKnee, AlongDir);
		}
	}
}

AT66MiasmaBoundary::AT66MiasmaBoundary()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SceneRoot->SetMobility(EComponentMobility::Static);
	SetRootComponent(SceneRoot);

	BoundaryCliffMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("BoundaryCliffs"));
	BoundaryCliffMesh->SetupAttachment(SceneRoot);
	BoundaryCliffMesh->SetMobility(EComponentMobility::Static);
	BoundaryCliffMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoundaryCliffMesh->SetCollisionResponseToAllChannels(ECR_Block);
	BoundaryCliffMesh->SetGenerateOverlapEvents(false);
	BoundaryCliffMesh->SetCanEverAffectNavigation(false);
	BoundaryCliffMesh->bUseAsyncCooking = true;
	BoundaryCliffMesh->bUseComplexAsSimpleCollision = true;
}

void AT66MiasmaBoundary::BeginPlay()
{
	Super::BeginPlay();

	// [GOLD] Register with the actor registry (replaces TActorIterator for map markers).
	if (UWorld* W = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = W->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->RegisterMiasmaBoundary(this);
		}
	}

	BuildBoundaryCliffs();

	// Apply damage tick when player is outside the safe rectangle.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DamageTimerHandle, this, &AT66MiasmaBoundary::ApplyBoundaryDamageTick, DamageIntervalSeconds, true, DamageIntervalSeconds);
	}
}

void AT66MiasmaBoundary::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// [GOLD] Unregister from the actor registry.
	if (UWorld* W = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = W->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->UnregisterMiasmaBoundary(this);
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void AT66MiasmaBoundary::BuildBoundaryCliffs()
{
	UWorld* World = GetWorld();
	if (!World || !BoundaryCliffMesh)
	{
		return;
	}

	BoundaryCliffMesh->ClearAllMeshSections();

	TArray<UMaterialInterface*> CliffMaterials;
	if (UMaterialInterface* OuterWallMaterial = T66CreateOuterWallMaterial(this))
	{
		CliffMaterials.Add(OuterWallMaterial);
	}
	else
	{
		T66LoadBoundaryCliffMaterials(CliffMaterials);
	}
	UMaterialInterface* FallbackMaterial = CliffMaterials.Num() == 0 ? T66CreateOuterWallMaterial(this) : nullptr;
	const int32 MaterialBucketCount = FMath::Max(CliffMaterials.Num(), 1);

	TArray<FT66BoundaryMeshSection> Sections;
	Sections.SetNum(MaterialBucketCount);

	const int32 SegmentCount = FMath::Max(SegmentsPerSide, 4);
	const float MinHeight = FMath::Min(WallHeight, MaxWallHeight);
	const float MaxHeight = FMath::Max(WallHeight, MaxWallHeight);
	const float MinDepth = FMath::Min(WallThickness, MaxWallThickness);
	const float MaxDepth = FMath::Max(WallThickness, MaxWallThickness);
	const float SideStart = -SafeHalfExtent - CornerOverlap;
	const float SideEnd = SafeHalfExtent + CornerOverlap;

	for (int32 SideIndex = 0; SideIndex < 4; ++SideIndex)
	{
		const ET66BoundarySide Side = static_cast<ET66BoundarySide>(SideIndex);
		TArray<FT66BoundarySample> Samples;
		Samples.SetNum(SegmentCount + 1);

		for (int32 SampleIndex = 0; SampleIndex <= SegmentCount; ++SampleIndex)
		{
			const uint32 Hash = T66BoundaryHash(static_cast<uint32>(BoundarySeed), static_cast<uint32>(SideIndex), static_cast<uint32>(SampleIndex));
			FRandomStream Stream(static_cast<int32>(Hash & 0x7fffffff));

			FT66BoundarySample& Sample = Samples[SampleIndex];
			Sample.Along = FMath::Lerp(SideStart, SideEnd, static_cast<float>(SampleIndex) / static_cast<float>(SegmentCount));

			const FVector GroundProbe = T66MakeBoundaryPoint(Side, SafeHalfExtent, Sample.Along, 0.f, GetActorLocation().Z);
			Sample.GroundZ = T66SampleGroundZ(World, this, GroundProbe, GetActorLocation().Z);
			Sample.OuterDepth = Stream.FRandRange(MinDepth, MaxDepth);
			Sample.RidgeDepth = Sample.OuterDepth * Stream.FRandRange(0.35f, 0.68f);
			Sample.RidgeHeight = Stream.FRandRange(MinHeight, MaxHeight);
			const float MaxVerticalFaceHeight = FMath::Max(Sample.RidgeHeight - 180.f, 300.f);
			Sample.VerticalFaceTopZ = Sample.GroundZ + FMath::Clamp(VerticalFaceHeight, 300.f, MaxVerticalFaceHeight);
			Sample.SampleBaseSink = BaseSink * Stream.FRandRange(0.85f, 1.20f);
			Sample.OuterDrop = BaseSink * Stream.FRandRange(0.10f, 0.55f);
		}

		for (int32 SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex)
		{
			const uint32 SegmentHash = T66BoundaryHash(static_cast<uint32>(BoundarySeed ^ 0x4D494153), static_cast<uint32>(SideIndex), static_cast<uint32>(SegmentIndex));
			const int32 Bucket = static_cast<int32>(SegmentHash % static_cast<uint32>(MaterialBucketCount));

			T66AppendBoundarySegment(
				Sections[Bucket],
				Side,
				SafeHalfExtent,
				Samples[SegmentIndex],
				Samples[SegmentIndex + 1],
				SegmentIndex == 0,
				SegmentIndex == SegmentCount - 1);
		}
	}

	TArray<FLinearColor> EmptyColors;
	int32 SectionIndex = 0;
	for (int32 Bucket = 0; Bucket < Sections.Num(); ++Bucket)
	{
		if (!Sections[Bucket].HasGeometry())
		{
			continue;
		}

		TArray<FProcMeshTangent> Tangents;
		TArray<FVector> Normals = Sections[Bucket].Normals;
		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Sections[Bucket].Verts, Sections[Bucket].Tris, Sections[Bucket].UVs, Normals, Tangents);

		BoundaryCliffMesh->CreateMeshSection_LinearColor(
			SectionIndex,
			Sections[Bucket].Verts,
			Sections[Bucket].Tris,
			Normals,
			Sections[Bucket].UVs,
			EmptyColors,
			Tangents,
			true);

		if (CliffMaterials.IsValidIndex(Bucket))
		{
			BoundaryCliffMesh->SetMaterial(SectionIndex, CliffMaterials[Bucket]);
		}
		else if (FallbackMaterial)
		{
			BoundaryCliffMesh->SetMaterial(SectionIndex, FallbackMaterial);
		}

		++SectionIndex;
	}

	BoundaryCliffMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoundaryCliffMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void AT66MiasmaBoundary::ApplyBoundaryDamageTick()
{
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	AT66HeroBase* Hero = Cast<AT66HeroBase>(Pawn);
	if (!Hero) return;

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	const FVector Loc = Hero->GetActorLocation();
	const bool bOutside = (FMath::Abs(Loc.X) > SafeHalfExtent) || (FMath::Abs(Loc.Y) > SafeHalfExtent);
	if (bOutside)
	{
		RunState->ApplyDamage(20);
	}
}
