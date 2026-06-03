// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66DotMarkerVFX.h"
#include "Gameplay/T66VisualUtil.h"
#include "Gameplay/T66CombatComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "TimerManager.h"

AT66DotMarkerVFX::AT66DotMarkerVFX()
{
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void AT66DotMarkerVFX::InitializeMarkers(AActor* FollowTarget, const int32 MarkerCount, const FLinearColor& Color, const float MarkerScale, const float LifeSeconds)
{
	const int32 Count = FMath::Max(0, MarkerCount);
	UStaticMesh* SphereMesh = FT66VisualUtil::GetBasicShapeSphere();
	const float Scale = FMath::Max(0.01f, MarkerScale);

	// Lay the markers out in a small ring slightly above the target center so all three
	// read as distinct applicator dots clinging to the enemy.
	constexpr float RingRadius = 42.f;
	constexpr float RingHeight = 55.f;
	for (int32 Index = 0; Index < Count; ++Index)
	{
		UStaticMeshComponent* Marker = NewObject<UStaticMeshComponent>(this);
		if (!Marker)
		{
			continue;
		}
		Marker->SetupAttachment(Root);
		Marker->RegisterComponent();
		if (SphereMesh)
		{
			Marker->SetStaticMesh(SphereMesh);
		}
		Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Marker->SetCollisionProfileName(TEXT("NoCollision"));
		Marker->SetCastShadow(false);
		Marker->SetWorldScale3D(FVector(Scale));
		// Created hidden; the staggered reveal below turns each marker on over time.
		Marker->SetVisibility(false, true);

		const float Angle = (Count > 0) ? (2.f * PI * static_cast<float>(Index) / static_cast<float>(Count)) : 0.f;
		const FVector Offset(RingRadius * FMath::Cos(Angle), RingRadius * FMath::Sin(Angle), RingHeight);
		Marker->SetRelativeLocation(Offset);
		FT66VisualUtil::ApplyT66Color(Marker, this, Color);
		Markers.Add(Marker);
	}

	if (FollowTarget)
	{
		// Snap-attach so the marker root sits on the target with a zero target-relative
		// offset and follows it for the DOT duration (mechanism: target-following
		// persistence). SnapToTargetNotIncludingScale zeroes the relative location/rotation
		// while letting the markers keep their own world scale, so the ring offsets stay
		// at their authored size regardless of the target's scale. KeepRelativeTransform
		// was wrong here: pre-attach the actor's "relative" transform is its world
		// transform, so it preserved a world-sized offset and the markers drifted off-target.
		// Attachment carries no damage authority.
		AttachToActor(FollowTarget, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	SetLifeSpan(FMath::Max(0.1f, LifeSeconds));

	// Staggered reveal + short pulse: the markers exist immediately (still visual-only,
	// no collision, no damage authority) but become visible one at a time on a 0.5s
	// cadence and each disappears again after a 0.3s visible pulse, so only one
	// applicator dot reads at a time (marker 0 visible 0.00-0.30s, marker 1 0.50-0.80s,
	// marker 2 1.00-1.30s). Index 0 reveals now; later indices reveal via the world
	// timer manager, and RevealMarker schedules each marker's own hide. Timers bound to
	// this UObject are invalidated automatically if the actor is destroyed before they
	// fire, and RevealMarker/HideMarker re-validate the index, so a short DOT duration
	// cannot crash here.
	FollowTargetWeak = FollowTarget;
	PlannedMarkerCount = Markers.Num();

	if (Markers.Num() > 0)
	{
		RevealMarker(0);
	}

	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		for (int32 Index = 1; Index < Markers.Num(); ++Index)
		{
			const float Delay = MarkerRevealIntervalSeconds * static_cast<float>(Index);
			FTimerHandle& Handle = RevealTimerHandles.AddDefaulted_GetRef();
			TimerManager.SetTimer(
				Handle,
				FTimerDelegate::CreateUObject(this, &AT66DotMarkerVFX::RevealMarker, Index),
				Delay,
				false);
		}
	}
}

void AT66DotMarkerVFX::RevealMarker(const int32 Index)
{
	if (!Markers.IsValidIndex(Index))
	{
		return;
	}

	UStaticMeshComponent* Marker = Markers[Index];
	if (!Marker)
	{
		return;
	}

	Marker->SetVisibility(true, true);

	const float PlannedDelay = MarkerRevealIntervalSeconds * static_cast<float>(Index);
	UE_LOG(
		LogT66Combat,
		Display,
		TEXT("T66DotApplicatorMarkerRevealed Target=%s MarkerIndex=%d MarkerCount=%d PlannedRevealDelaySeconds=%.2f (visual-only; single DOT payload unchanged)"),
		*GetNameSafe(FollowTargetWeak.Get()),
		Index,
		PlannedMarkerCount,
		PlannedDelay);

	// Schedule this marker to disappear after its short visible pulse so only one
	// placeholder dot reads at a time. Relative to the actual reveal, so the hide tracks
	// each marker's reveal time. The timer is bound to this UObject and re-validates the
	// index in HideMarker, so a self-destruct before the hide fires is safe.
	if (UWorld* World = GetWorld())
	{
		FTimerHandle& Handle = HideTimerHandles.AddDefaulted_GetRef();
		World->GetTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateUObject(this, &AT66DotMarkerVFX::HideMarker, Index),
			MarkerVisibleDurationSeconds,
			false);
	}
}

void AT66DotMarkerVFX::HideMarker(const int32 Index)
{
	if (!Markers.IsValidIndex(Index))
	{
		return;
	}

	UStaticMeshComponent* Marker = Markers[Index];
	if (!Marker)
	{
		return;
	}

	Marker->SetVisibility(false, true);

	const float PlannedDelay = MarkerRevealIntervalSeconds * static_cast<float>(Index);
	UE_LOG(
		LogT66Combat,
		Display,
		TEXT("T66DotApplicatorMarkerHidden Target=%s MarkerIndex=%d MarkerCount=%d PlannedRevealDelaySeconds=%.2f VisibleDurationSeconds=%.2f (visual-only; single DOT payload unchanged)"),
		*GetNameSafe(FollowTargetWeak.Get()),
		Index,
		PlannedMarkerCount,
		PlannedDelay,
		MarkerVisibleDurationSeconds);
}
