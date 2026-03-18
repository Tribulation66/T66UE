// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66LavaPatch.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/T66RunStateSubsystem.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Gameplay/T66HeroBase.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr float T66LavaTwoPi = 6.28318530718f;
	const TCHAR* T66LavaBaseMaterialPath = TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit");
	const TCHAR* T66WhiteTexturePath = TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture");
	const TCHAR* T66DefaultTexturePath = TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture");

	float T66LavaFrac(float Value)
	{
		return Value - FMath::FloorToFloat(Value);
	}

	float T66LavaSaturate(float Value)
	{
		return FMath::Clamp(Value, 0.f, 1.f);
	}

	float T66LavaSmoothStep(float Edge0, float Edge1, float Value)
	{
		if (FMath::IsNearlyEqual(Edge0, Edge1))
		{
			return Value < Edge0 ? 0.f : 1.f;
		}

		const float T = T66LavaSaturate((Value - Edge0) / (Edge1 - Edge0));
		return T * T * (3.f - 2.f * T);
	}

	FVector2D T66LavaHash2(const FVector2D& P)
	{
		return FVector2D(
			T66LavaFrac(FMath::Sin(FVector2D::DotProduct(P, FVector2D(127.1f, 311.7f))) * 43758.5453123f),
			T66LavaFrac(FMath::Sin(FVector2D::DotProduct(P, FVector2D(269.5f, 183.3f))) * 43758.5453123f));
	}

	struct FT66LavaCellSample
	{
		float Closest = BIG_NUMBER;
		float SecondClosest = BIG_NUMBER;
	};

	FT66LavaCellSample T66SampleLavaCells(const FVector2D& UV, float Density, float Phase)
	{
		FT66LavaCellSample Result;

		const FVector2D P = UV * Density;
		const FVector2D Cell(FMath::FloorToFloat(P.X), FMath::FloorToFloat(P.Y));
		const FVector2D Local = P - Cell;

		// Worley-style cell distances give us the dark pools + bright crack edges.
		for (int32 Y = -1; Y <= 1; ++Y)
		{
			for (int32 X = -1; X <= 1; ++X)
			{
				const FVector2D NeighborCell = Cell + FVector2D(static_cast<float>(X), static_cast<float>(Y));
				const FVector2D Jitter = T66LavaHash2(NeighborCell);
				const FVector2D Drift(
					FMath::Sin(Phase * (0.65f + Jitter.X * 0.25f) + NeighborCell.Y * 1.73f + Jitter.Y * 4.0f),
					FMath::Cos(Phase * (0.50f + Jitter.Y * 0.25f) + NeighborCell.X * 1.37f + Jitter.X * 4.0f));
				const FVector2D FeaturePoint =
					FVector2D(static_cast<float>(X), static_cast<float>(Y)) + Jitter + Drift * 0.16f;

				const float DistSq = (Local - FeaturePoint).SizeSquared();
				if (DistSq < Result.Closest)
				{
					Result.SecondClosest = Result.Closest;
					Result.Closest = DistSq;
				}
				else if (DistSq < Result.SecondClosest)
				{
					Result.SecondClosest = DistSq;
				}
			}
		}

		Result.Closest = FMath::Sqrt(Result.Closest);
		Result.SecondClosest = FMath::Sqrt(Result.SecondClosest);
		return Result;
	}

	UTexture* T66GetWhiteFallbackTexture()
	{
		static TObjectPtr<UTexture> Cached = nullptr;
		if (!Cached)
		{
			Cached = LoadObject<UTexture>(nullptr, T66WhiteTexturePath);
		}
		if (!Cached)
		{
			Cached = LoadObject<UTexture>(nullptr, T66DefaultTexturePath);
		}
		return Cached.Get();
	}

	int32 T66GetLocationFrameOffset(const FVector& Location, int32 FrameCount)
	{
		if (FrameCount <= 1)
		{
			return 0;
		}

		const float Hash = T66LavaFrac(
			FMath::Sin(Location.X * 0.00091f + Location.Y * 0.00117f + Location.Z * 0.00037f) * 43758.5453123f);
		return FMath::Clamp(FMath::FloorToInt(Hash * static_cast<float>(FrameCount)), 0, FrameCount - 1);
	}
}

AT66LavaPatch::AT66LavaPatch()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.f / 12.f;

	DamageBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	DamageBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageBox->SetGenerateOverlapEvents(true);
	DamageBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = DamageBox;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetCastShadow(false);
	VisualMesh->bReceivesDecals = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneFinder.Succeeded())
	{
		VisualMesh->SetStaticMesh(PlaneFinder.Object);
	}

	RefreshPatchLayout();
}

void AT66LavaPatch::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshPatchLayout();
	EnsureVisualMaterial();
}

void AT66LavaPatch::BeginPlay()
{
	Super::BeginPlay();

	RefreshPatchLayout();

	if (bSnapToGroundOnBeginPlay)
	{
		SnapOriginToGround();
	}

	GenerateAnimationFrames();

	if (GeneratedFrames.Num() > 0)
	{
		StartFrameOffset = T66GetLocationFrameOffset(GetActorLocation(), GeneratedFrames.Num());
		CurrentFrameIndex = StartFrameOffset;
	}

	EnsureVisualMaterial();

	if (GeneratedFrames.IsValidIndex(CurrentFrameIndex))
	{
		ApplyAnimationFrame(CurrentFrameIndex);
	}

	if (UWorld* World = GetWorld())
	{
		AnimationStartTimeSeconds = World->GetTimeSeconds();
	}

	if (DamageBox)
	{
		DamageBox->OnComponentBeginOverlap.AddDynamic(this, &AT66LavaPatch::OnDamageBoxBeginOverlap);
		DamageBox->OnComponentEndOverlap.AddDynamic(this, &AT66LavaPatch::OnDamageBoxEndOverlap);
	}

	UpdateAnimationTickState();
}

void AT66LavaPatch::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!LavaMID)
	{
		return;
	}

	LavaMID->SetScalarParameterValue(TEXT("Brightness"), Brightness);

	UWorld* World = GetWorld();
	if (!World || AnimationFPS <= KINDA_SMALL_NUMBER || GeneratedFrames.Num() <= 1)
	{
		return;
	}

	const float Elapsed = FMath::Max(World->GetTimeSeconds() - AnimationStartTimeSeconds, 0.f);
	const int32 FrameIndex =
		(StartFrameOffset + FMath::FloorToInt(Elapsed * AnimationFPS)) % GeneratedFrames.Num();
	if (FrameIndex != CurrentFrameIndex)
	{
		ApplyAnimationFrame(FrameIndex);
	}
}

void AT66LavaPatch::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}

	OverlappingHero.Reset();
	Super::EndPlay(EndPlayReason);
}

void AT66LavaPatch::OnDamageBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bDamageHero || DamagePerTick <= 0 || !OtherActor)
	{
		return;
	}

	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero)
	{
		return;
	}

	OverlappingHero = Hero;
	ApplyDamageTick();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DamageTimerHandle,
			this,
			&AT66LavaPatch::ApplyDamageTick,
			FMath::Max(DamageIntervalSeconds, 0.05f),
			true,
			FMath::Max(DamageIntervalSeconds, 0.05f));
	}
}

void AT66LavaPatch::OnDamageBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero || OverlappingHero.Get() != Hero)
	{
		return;
	}

	OverlappingHero.Reset();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}
}

void AT66LavaPatch::RefreshPatchLayout()
{
	if (DamageBox)
	{
		const FVector HalfExtents(PatchSize * 0.5f, PatchSize * 0.5f, FMath::Max(CollisionHeight * 0.5f, 20.f));
		DamageBox->SetBoxExtent(HalfExtents);
	}

	if (VisualMesh)
	{
		const float UniformScale = FMath::Max(PatchSize / 100.f, 0.1f);
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, HoverHeight));
		VisualMesh->SetRelativeScale3D(FVector(UniformScale, UniformScale, 1.f));
	}
}

void AT66LavaPatch::SnapOriginToGround()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Here = GetActorLocation();
	const FVector Start = Here + FVector(0.f, 0.f, 5000.f);
	const FVector End = Here - FVector(0.f, 0.f, 12000.f);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(T66LavaPatchGroundTrace), false, this);
	FHitResult Hit;
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params) ||
		World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		SetActorLocation(Hit.ImpactPoint, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AT66LavaPatch::EnsureVisualMaterial()
{
	if (!VisualMesh)
	{
		return;
	}

	if (!LavaMID)
	{
		if (UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, T66LavaBaseMaterialPath))
		{
			LavaMID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			if (LavaMID)
			{
				VisualMesh->SetMaterial(0, LavaMID);
			}
		}
	}

	if (!LavaMID)
	{
		return;
	}

	const bool bHasFrames = GeneratedFrames.Num() > 0;
	const int32 SafeFrameIndex = GeneratedFrames.IsValidIndex(CurrentFrameIndex) ? CurrentFrameIndex : INDEX_NONE;
	UTexture* TextureToUse = (bHasFrames && SafeFrameIndex != INDEX_NONE)
		? GeneratedFrames[SafeFrameIndex].Get()
		: T66GetWhiteFallbackTexture();

	if (TextureToUse)
	{
		LavaMID->SetTextureParameterValue(TEXT("DiffuseColorMap"), TextureToUse);
		LavaMID->SetTextureParameterValue(TEXT("BaseColorTexture"), TextureToUse);
	}

	const FLinearColor TintColor = bHasFrames ? FLinearColor::White : GlowColor;
	LavaMID->SetVectorParameterValue(TEXT("Tint"), TintColor);
	LavaMID->SetVectorParameterValue(TEXT("BaseColor"), TintColor);
	LavaMID->SetScalarParameterValue(TEXT("Brightness"), bHasFrames ? Brightness : 1.f);
}

void AT66LavaPatch::GenerateAnimationFrames()
{
	GeneratedFrames.Reset();
	CurrentFrameIndex = INDEX_NONE;

	const int32 Resolution = FMath::Clamp(TextureResolution, 16, 256);
	const int32 ClampedFrames = FMath::Clamp(AnimationFrames, 4, 64);
	const FVector ActorLocation = GetActorLocation();
	const FVector2D EffectivePatternOffset =
		PatternOffset + FVector2D(ActorLocation.X, ActorLocation.Y) * 0.00013f;

	GeneratedFrames.Reserve(ClampedFrames);
	for (int32 FrameIndex = 0; FrameIndex < ClampedFrames; ++FrameIndex)
	{
		if (UTexture2D* Texture = BuildFrameTexture(FrameIndex, ClampedFrames, Resolution, EffectivePatternOffset))
		{
			GeneratedFrames.Add(Texture);
		}
	}
}

UTexture2D* AT66LavaPatch::BuildFrameTexture(
	int32 FrameIndex,
	int32 ClampedFrames,
	int32 Resolution,
	const FVector2D& EffectivePatternOffset) const
{
	if (Resolution <= 0 || ClampedFrames <= 0)
	{
		return nullptr;
	}

	TArray<FColor> Pixels;
	Pixels.SetNumUninitialized(Resolution * Resolution);

	const float Phase = (static_cast<float>(FrameIndex) / static_cast<float>(ClampedFrames)) * T66LavaTwoPi;

	for (int32 Y = 0; Y < Resolution; ++Y)
	{
		for (int32 X = 0; X < Resolution; ++X)
		{
			const FVector2D UV(
				(static_cast<float>(X) + 0.5f) / static_cast<float>(Resolution),
				(static_cast<float>(Y) + 0.5f) / static_cast<float>(Resolution));

			const FLinearColor LavaColor = SampleLavaColor(UV, Phase, EffectivePatternOffset);
			Pixels[Y * Resolution + X] = LavaColor.ToFColorSRGB();
		}
	}

	UTexture2D* Texture = UTexture2D::CreateTransient(Resolution, Resolution, PF_B8G8R8A8);
	if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
	{
		return nullptr;
	}

	Texture->SRGB = true;
	Texture->Filter = TF_Nearest;
	Texture->AddressX = TA_Wrap;
	Texture->AddressY = TA_Wrap;
	Texture->LODGroup = TEXTUREGROUP_Pixels2D;
	Texture->NeverStream = true;
#if WITH_EDITORONLY_DATA
	Texture->MipGenSettings = TMGS_NoMipmaps;
#endif

	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* MipData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(MipData, Pixels.GetData(), Pixels.Num() * sizeof(FColor));
	Mip.BulkData.Unlock();
	Texture->UpdateResource();

	return Texture;
}

FLinearColor AT66LavaPatch::SampleLavaColor(
	const FVector2D& BaseUV,
	float Phase,
	const FVector2D& EffectivePatternOffset) const
{
	FVector2D UV = BaseUV * FMath::Max(UVScale, 0.1f) + EffectivePatternOffset;

	const FVector2D FlowNormal = FlowDir.IsNearlyZero() ? FVector2D(1.f, 0.f) : FlowDir.GetSafeNormal();
	UV += FlowNormal * Phase * FlowSpeed * 0.20f;

	const float SafeCloseness = FMath::Max(WarpCloseness, 0.01f);
	const float Nx = UV.X / SafeCloseness;
	const float Ny = UV.Y / SafeCloseness;
	const float WarpT = Phase * WarpSpeed;
	const float WarpedX = Nx + WarpIntensity * FMath::Sin(WarpT + Ny * 2.0f);
	const float WarpedY = Ny + WarpIntensity * FMath::Sin(WarpT + Nx * 2.0f);
	const FVector2D FinalUV = FVector2D(WarpedX, WarpedY) * SafeCloseness;

	const FT66LavaCellSample Cells = T66SampleLavaCells(FinalUV, FMath::Max(CellDensity, 1.0f), Phase);
	const float Border = FMath::Max(Cells.SecondClosest - Cells.Closest, 0.0f);
	const float Crack = FMath::Pow(T66LavaSaturate(1.0f - Border * FMath::Max(EdgeContrast, 0.1f)), 2.3f);
	const float Pulse = 0.5f + 0.5f * FMath::Sin((FinalUV.X * 1.8f + FinalUV.Y * 1.25f) + Phase * 2.2f);
	const float Ember = T66LavaSaturate(1.0f - T66LavaSmoothStep(0.18f, 0.52f, Cells.Closest + Pulse * 0.06f));
	const float Heat = T66LavaSaturate(Crack * 1.18f + Ember * 0.22f);

	FLinearColor Color = FMath::Lerp(CoreColor, MidColor, T66LavaSaturate(Heat * 0.72f + Pulse * 0.10f));
	Color = FMath::Lerp(Color, GlowColor, T66LavaSaturate(FMath::Pow(Crack, 0.72f)));

	const float PoolMask = T66LavaSmoothStep(0.20f, 0.48f, Cells.Closest);
	Color = FMath::Lerp(Color, CoreColor * 0.55f, PoolMask * 0.35f);
	Color.A = 1.f;
	return Color.GetClamped(0.f, 1.f);
}

void AT66LavaPatch::ApplyAnimationFrame(int32 FrameIndex)
{
	if (!GeneratedFrames.IsValidIndex(FrameIndex))
	{
		return;
	}

	CurrentFrameIndex = FrameIndex;
	EnsureVisualMaterial();
	if (!LavaMID)
	{
		return;
	}

	if (UTexture2D* FrameTexture = GeneratedFrames[FrameIndex].Get())
	{
		LavaMID->SetTextureParameterValue(TEXT("DiffuseColorMap"), FrameTexture);
		LavaMID->SetTextureParameterValue(TEXT("BaseColorTexture"), FrameTexture);
		LavaMID->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
		LavaMID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::White);
		LavaMID->SetScalarParameterValue(TEXT("Brightness"), Brightness);
	}
}

void AT66LavaPatch::ApplyDamageTick()
{
	if (!bDamageHero || DamagePerTick <= 0)
	{
		return;
	}

	AT66HeroBase* Hero = OverlappingHero.Get();
	if (!Hero)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(DamageTimerHandle);
		}
		return;
	}

	if (Hero->IsInSafeZone())
	{
		return;
	}

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState)
	{
		RunState->ApplyDamage(DamagePerTick, this);
	}
}

void AT66LavaPatch::UpdateAnimationTickState()
{
	const bool bShouldAnimate = GeneratedFrames.Num() > 1 && AnimationFPS > KINDA_SMALL_NUMBER;
	SetActorTickEnabled(bShouldAnimate);

	if (bShouldAnimate)
	{
		PrimaryActorTick.TickInterval = 1.f / FMath::Max(AnimationFPS, 1.f);
	}
}
