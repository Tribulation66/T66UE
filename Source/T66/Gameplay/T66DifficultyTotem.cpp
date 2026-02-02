// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66DifficultyTotem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "UObject/SoftObjectPath.h"

AT66DifficultyTotem::AT66DifficultyTotem()
{
	PrimaryActorTick.bCanEverTick = false;
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		VisualMesh->SetStaticMesh(Cube);
		// Tall skinny rectangle
		VisualMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 4.0f));
		// Keep base planted: will be recomputed in ApplyGrowthFromInteractionCount.
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	}

	// Default expected import location (safe if missing).
	TotemMeshOverride = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/SM_DifficultyTotem.SM_DifficultyTotem")));
}

void AT66DifficultyTotem::BeginPlay()
{
	Super::BeginPlay();

	// Swap to imported mesh if present (and keep scale consistent for stacking).
	if (VisualMesh && !TotemMeshOverride.IsNull())
	{
		if (UStaticMesh* M = TotemMeshOverride.LoadSynchronous())
		{
			// Ensure we don't keep any placeholder tint material from a prior fallback.
			VisualMesh->EmptyOverrideMaterials();
			VisualMesh->SetStaticMesh(M);
			VisualMesh->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
		}
	}

	// Ensure newly spawned totems match current run interaction growth.
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	ApplyGrowthFromInteractionCount(RunState->GetTotemsActivatedCount());
}

bool AT66DifficultyTotem::Interact(APlayerController* PC)
{
	(void)PC;
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return false;

	// Infinite-use: each interaction adds exactly 1 skull (no half skulls, no rarity deltas).
	RunState->AddDifficultySkulls(1);

	// Track total interactions so all totems grow together.
	const int32 InteractionCount = RunState->RegisterTotemActivated(); // 1-based, unbounded (clamped in RunState)

	// All totems in the map grow taller on each interaction.
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AT66DifficultyTotem> It(World); It; ++It)
		{
			if (AT66DifficultyTotem* Totem = *It)
			{
				Totem->ApplyGrowthFromInteractionCount(InteractionCount);
			}
		}
	}
	return true;
}

void AT66DifficultyTotem::ApplyGrowthFromInteractionCount(int32 InteractionCount)
{
	if (!VisualMesh) return;

	const int32 Count = FMath::Clamp(InteractionCount, 0, 999);

	// Growth rule (requested): do not scale the mesh; instead, stack identical instances upward.
	// Count is 1-based externally (first use => 1). If Count is 0 (fresh run), show one segment.
	const int32 DesiredSegments = FMath::Clamp(FMath::Max(1, Count), 1, 30);

	UStaticMesh* Mesh = VisualMesh->GetStaticMesh();
	if (!Mesh)
	{
		return;
	}

	const FVector BaseScale = VisualMesh->GetRelativeScale3D();
	const FRotator BaseRot = VisualMesh->GetRelativeRotation();

	// Compute segment height from mesh bounds * scale.
	const float UnscaledHalfHeight = Mesh->GetBounds().BoxExtent.Z;
	const float SegmentHalfHeight = FMath::Max(1.f, UnscaledHalfHeight * FMath::Abs(BaseScale.Z));
	const float SegmentHeight = SegmentHalfHeight * 2.f;

	// Plant base segment.
	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, SegmentHalfHeight));

	// Ensure we have DesiredSegments-1 extra components.
	const int32 WantExtras = FMath::Max(0, DesiredSegments - 1);
	while (StackedVisualSegments.Num() > WantExtras)
	{
		if (UStaticMeshComponent* C = StackedVisualSegments.Pop())
		{
			C->DestroyComponent();
		}
	}
	while (StackedVisualSegments.Num() < WantExtras)
	{
		UStaticMeshComponent* Seg = NewObject<UStaticMeshComponent>(this);
		if (!Seg) break;

		Seg->SetStaticMesh(Mesh);
		Seg->SetRelativeScale3D(BaseScale);
		Seg->SetRelativeRotation(BaseRot);
		Seg->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Seg->SetGenerateOverlapEvents(false);
		Seg->SetCanEverAffectNavigation(false);

		Seg->SetupAttachment(RootComponent ? RootComponent : VisualMesh);
		Seg->RegisterComponent();
		StackedVisualSegments.Add(Seg);
	}

	// Position extra segments above the base.
	for (int32 i = 0; i < StackedVisualSegments.Num(); ++i)
	{
		UStaticMeshComponent* Seg = StackedVisualSegments[i];
		if (!Seg) continue;

		// Segment index: 1..DesiredSegments-1
		const int32 SegmentIndex = i + 1;
		const float Z = SegmentHalfHeight + SegmentHeight * static_cast<float>(SegmentIndex);
		Seg->SetRelativeLocation(FVector(0.f, 0.f, Z));
	}
}

