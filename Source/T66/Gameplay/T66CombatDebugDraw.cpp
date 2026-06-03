// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CombatDebugDraw.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66CombatDebugDraw, Log, All);

namespace
{
#if UE_BUILD_SHIPPING
	static constexpr int32 T66DefaultCombatDebugView = 0;
#else
	static constexpr int32 T66DefaultCombatDebugView = 0;
#endif

	static TAutoConsoleVariable<int32> CVarT66CombatDebugView(
		TEXT("T66.Combat.DebugView"),
		T66DefaultCombatDebugView,
		TEXT("Combat debug draw mode. 0=off, 1=target hitboxes/hurtboxes, 2=damage volumes, 3=both. Non-shipping builds default to 0; enable explicitly for diagnosis."));

	static TAutoConsoleVariable<int32> CVarT66CombatDebugLabels(
		TEXT("T66.Combat.DebugLabels"),
		0,
		TEXT("Draw combat debug labels. 0=off, 1=on."));

	static TAutoConsoleVariable<float> CVarT66CombatDebugThickness(
		TEXT("T66.Combat.DebugThickness"),
		1.8f,
		TEXT("Line thickness for combat hitbox and damage-volume debug draw."));

	static int32 T66GetCombatDebugView()
	{
#if UE_BUILD_SHIPPING
		return 0;
#else
		return FMath::Clamp(CVarT66CombatDebugView.GetValueOnGameThread(), 0, 3);
#endif
	}

	static float T66GetCombatDebugThickness()
	{
		return FMath::Clamp(CVarT66CombatDebugThickness.GetValueOnGameThread(), 0.25f, 12.f);
	}

	static bool T66ShouldDrawLabels()
	{
#if UE_BUILD_SHIPPING
		return false;
#else
		return CVarT66CombatDebugLabels.GetValueOnGameThread() != 0;
#endif
	}

#if !UE_BUILD_SHIPPING
	static void T66LogCombatDebugCVarChanged(IConsoleVariable*)
	{
		UE_LOG(
			LogT66CombatDebugDraw,
			Log,
			TEXT("[CombatDebug] RuntimeValues DebugView=%d DebugLabels=%d"),
			FMath::Clamp(CVarT66CombatDebugView.GetValueOnGameThread(), 0, 3),
			CVarT66CombatDebugLabels.GetValueOnGameThread() != 0 ? 1 : 0);
	}

	struct FT66CombatDebugCVarLogger
	{
		FT66CombatDebugCVarLogger()
		{
			if (IConsoleVariable* DebugView = CVarT66CombatDebugView.AsVariable())
			{
				DebugView->SetOnChangedCallback(FConsoleVariableDelegate::CreateStatic(&T66LogCombatDebugCVarChanged));
			}
			if (IConsoleVariable* DebugLabels = CVarT66CombatDebugLabels.AsVariable())
			{
				DebugLabels->SetOnChangedCallback(FConsoleVariableDelegate::CreateStatic(&T66LogCombatDebugCVarChanged));
			}
		}
	};

	static FT66CombatDebugCVarLogger GT66CombatDebugCVarLogger;
#endif

	static FColor T66GetHitZoneColor(const ET66HitZoneType HitZoneType, const bool bActive)
	{
		if (!bActive)
		{
			return FColor(92, 92, 92);
		}

		switch (HitZoneType)
		{
		case ET66HitZoneType::Head:
			return FColor(255, 214, 64);
		case ET66HitZoneType::Core:
			return FColor(255, 78, 150);
		case ET66HitZoneType::WeakPoint:
			return FColor(210, 92, 255);
		case ET66HitZoneType::LeftArm:
		case ET66HitZoneType::RightArm:
			return FColor(96, 220, 120);
		case ET66HitZoneType::LeftLeg:
		case ET66HitZoneType::RightLeg:
			return FColor(72, 180, 255);
		case ET66HitZoneType::Body:
		default:
			return FColor(64, 200, 255);
		}
	}

	static FColor T66GetDamageVolumeColor(const bool bActive)
	{
		return bActive ? FColor(255, 64, 32) : FColor(255, 168, 32);
	}

	static constexpr float T66TransientDamageVolumeLifetimeSeconds = 0.35f;

	static FColor T66GetTriggerVolumeColor(const bool bArmed)
	{
		return bArmed ? FColor(80, 255, 160) : FColor(90, 120, 120);
	}

	static void T66DrawLabel(const UWorld* World, const FVector& Location, const FString& Label, const FColor& Color)
	{
		if (!World || Label.IsEmpty() || !T66ShouldDrawLabels())
		{
			return;
		}

		DrawDebugString(World, Location, Label, nullptr, Color, 0.f, true, 0.9f);
	}
}

bool T66CombatDebugDraw::ShouldDrawHitboxes()
{
	return (T66GetCombatDebugView() & 1) != 0;
}

bool T66CombatDebugDraw::ShouldDrawDamageVolumes()
{
	return (T66GetCombatDebugView() & 2) != 0;
}

void T66CombatDebugDraw::DrawHitZone(const USphereComponent* Zone, const ET66HitZoneType HitZoneType, const bool bActive, const FString& Label)
{
	if (!ShouldDrawHitboxes() || !Zone)
	{
		return;
	}

	const UWorld* World = Zone->GetWorld();
	if (!World)
	{
		return;
	}

	const FColor Color = T66GetHitZoneColor(HitZoneType, bActive);
	DrawDebugSphere(
		World,
		Zone->GetComponentLocation(),
		Zone->GetScaledSphereRadius(),
		24,
		Color,
		false,
		0.f,
		0,
		T66GetCombatDebugThickness());
	T66DrawLabel(World, Zone->GetComponentLocation() + FVector(0.f, 0.f, Zone->GetScaledSphereRadius() + 18.f), Label, Color);
}

void T66CombatDebugDraw::DrawDamageSphere(const USphereComponent* Sphere, const FString& Label, const bool bActive)
{
	if (!ShouldDrawDamageVolumes() || !Sphere)
	{
		return;
	}

	const UWorld* World = Sphere->GetWorld();
	if (!World)
	{
		return;
	}

	const FColor Color = T66GetDamageVolumeColor(bActive);
	DrawDebugSphere(
		World,
		Sphere->GetComponentLocation(),
		Sphere->GetScaledSphereRadius(),
		24,
		Color,
		false,
		0.f,
		0,
		T66GetCombatDebugThickness());
	T66DrawLabel(World, Sphere->GetComponentLocation() + FVector(0.f, 0.f, Sphere->GetScaledSphereRadius() + 18.f), Label, Color);
}

void T66CombatDebugDraw::DrawDamageSphere(UWorld* World, const FVector& Center, const float Radius, const FString& Label, const bool bActive)
{
	if (!ShouldDrawDamageVolumes() || !World || Radius <= 0.f)
	{
		return;
	}

	const FColor Color = T66GetDamageVolumeColor(bActive);
	DrawDebugSphere(
		World,
		Center,
		Radius,
		32,
		Color,
		false,
		T66TransientDamageVolumeLifetimeSeconds,
		0,
		T66GetCombatDebugThickness());
	T66DrawLabel(World, Center + FVector(0.f, 0.f, Radius + 18.f), Label, Color);
}

void T66CombatDebugDraw::DrawDamageSector(
	UWorld* World,
	const FVector& Center,
	const FVector& Forward,
	const float Radius,
	const float HalfAngleDegrees,
	const FString& Label,
	const bool bActive,
	const float InnerRadius)
{
	if (!ShouldDrawDamageVolumes() || !World || Radius <= 0.f || HalfAngleDegrees <= 0.f)
	{
		return;
	}

	FVector PlanarForward(Forward.X, Forward.Y, 0.f);
	if (!PlanarForward.Normalize())
	{
		PlanarForward = FVector::ForwardVector;
	}

	const float ClampedRadius = FMath::Max(1.f, Radius);
	const float ClampedInnerRadius = FMath::Clamp(InnerRadius, 0.f, ClampedRadius - 1.f);
	const float ClampedHalfAngle = FMath::Clamp(HalfAngleDegrees, 1.f, 179.f);
	const int32 SegmentCount = FMath::Clamp(FMath::CeilToInt((ClampedHalfAngle * 2.f) / 8.f), 8, 64);
	const FColor Color = T66GetDamageVolumeColor(bActive);
	const FVector DrawCenter = Center + FVector(0.f, 0.f, 16.f);
	const float Thickness = T66GetCombatDebugThickness();

	auto PointOnArc = [&](const float AngleDegrees, const float Distance)
	{
		return DrawCenter + PlanarForward.RotateAngleAxis(AngleDegrees, FVector::UpVector) * Distance;
	};

	FVector PrevOuter = PointOnArc(-ClampedHalfAngle, ClampedRadius);
	FVector PrevInner = PointOnArc(-ClampedHalfAngle, ClampedInnerRadius);
	for (int32 SegmentIndex = 1; SegmentIndex <= SegmentCount; ++SegmentIndex)
	{
		const float T = static_cast<float>(SegmentIndex) / static_cast<float>(SegmentCount);
		const float Angle = FMath::Lerp(-ClampedHalfAngle, ClampedHalfAngle, T);
		const FVector Outer = PointOnArc(Angle, ClampedRadius);
		DrawDebugLine(World, PrevOuter, Outer, Color, false, T66TransientDamageVolumeLifetimeSeconds, 0, Thickness);
		if (ClampedInnerRadius > 0.f)
		{
			const FVector Inner = PointOnArc(Angle, ClampedInnerRadius);
			DrawDebugLine(World, PrevInner, Inner, Color, false, T66TransientDamageVolumeLifetimeSeconds, 0, Thickness);
			PrevInner = Inner;
		}
		PrevOuter = Outer;
	}

	const FVector LeftOuter = PointOnArc(-ClampedHalfAngle, ClampedRadius);
	const FVector RightOuter = PointOnArc(ClampedHalfAngle, ClampedRadius);
	const FVector LeftInner = PointOnArc(-ClampedHalfAngle, ClampedInnerRadius);
	const FVector RightInner = PointOnArc(ClampedHalfAngle, ClampedInnerRadius);
	DrawDebugLine(World, LeftInner, LeftOuter, Color, false, T66TransientDamageVolumeLifetimeSeconds, 0, Thickness);
	DrawDebugLine(World, RightInner, RightOuter, Color, false, T66TransientDamageVolumeLifetimeSeconds, 0, Thickness);
	DrawDebugLine(World, DrawCenter, DrawCenter + PlanarForward * ClampedRadius, Color, false, T66TransientDamageVolumeLifetimeSeconds, 0, Thickness * 0.65f);

	const FVector LabelLocation = DrawCenter + PlanarForward * ((ClampedInnerRadius + ClampedRadius) * 0.5f) + FVector(0.f, 0.f, 42.f);
	T66DrawLabel(World, LabelLocation, Label, Color);
}

void T66CombatDebugDraw::DrawDamageCapsule(
	UWorld* World,
	const FVector& Center,
	const FQuat& Rotation,
	const float HalfHeight,
	const float Radius,
	const FString& Label,
	const bool bActive)
{
	if (!ShouldDrawDamageVolumes() || !World || Radius <= 0.f || HalfHeight <= 0.f)
	{
		return;
	}

	const FColor Color = T66GetDamageVolumeColor(bActive);
	DrawDebugCapsule(
		World,
		Center,
		HalfHeight,
		Radius,
		Rotation,
		Color,
		false,
		T66TransientDamageVolumeLifetimeSeconds,
		0,
		T66GetCombatDebugThickness());
	T66DrawLabel(World, Center + FVector(0.f, 0.f, HalfHeight + Radius + 18.f), Label, Color);
}

void T66CombatDebugDraw::DrawDamageBox(const UBoxComponent* Box, const FString& Label, const bool bActive)
{
	if (!ShouldDrawDamageVolumes() || !Box)
	{
		return;
	}

	const UWorld* World = Box->GetWorld();
	if (!World)
	{
		return;
	}

	const FColor Color = T66GetDamageVolumeColor(bActive);
	DrawDebugBox(
		World,
		Box->GetComponentLocation(),
		Box->GetScaledBoxExtent(),
		Box->GetComponentQuat(),
		Color,
		false,
		0.f,
		0,
		T66GetCombatDebugThickness());
	T66DrawLabel(World, Box->GetComponentLocation() + FVector(0.f, 0.f, Box->GetScaledBoxExtent().Z + 18.f), Label, Color);
}

void T66CombatDebugDraw::DrawDamageCapsule(const UCapsuleComponent* Capsule, const FString& Label, const bool bActive)
{
	if (!ShouldDrawDamageVolumes() || !Capsule)
	{
		return;
	}

	const UWorld* World = Capsule->GetWorld();
	if (!World)
	{
		return;
	}

	const FColor Color = T66GetDamageVolumeColor(bActive);
	DrawDebugCapsule(
		World,
		Capsule->GetComponentLocation(),
		Capsule->GetScaledCapsuleHalfHeight(),
		Capsule->GetScaledCapsuleRadius(),
		Capsule->GetComponentQuat(),
		Color,
		false,
		0.f,
		0,
		T66GetCombatDebugThickness());
	T66DrawLabel(World, Capsule->GetComponentLocation() + FVector(0.f, 0.f, Capsule->GetScaledCapsuleHalfHeight() + 18.f), Label, Color);
}

void T66CombatDebugDraw::DrawTriggerBox(const UBoxComponent* Box, const FString& Label, const bool bArmed)
{
	if (!ShouldDrawDamageVolumes() || !Box)
	{
		return;
	}

	const UWorld* World = Box->GetWorld();
	if (!World)
	{
		return;
	}

	const FColor Color = T66GetTriggerVolumeColor(bArmed);
	DrawDebugBox(
		World,
		Box->GetComponentLocation(),
		Box->GetScaledBoxExtent(),
		Box->GetComponentQuat(),
		Color,
		false,
		0.f,
		0,
		T66GetCombatDebugThickness());
	T66DrawLabel(World, Box->GetComponentLocation() + FVector(0.f, 0.f, Box->GetScaledBoxExtent().Z + 18.f), Label, Color);
}

void T66CombatDebugDraw::DrawPlayerHurtCapsule(const UCapsuleComponent* Capsule, const FString& Label)
{
	if (!ShouldDrawHitboxes() || !Capsule)
	{
		return;
	}

	const UWorld* World = Capsule->GetWorld();
	if (!World)
	{
		return;
	}

	const FColor Color(72, 240, 240);
	DrawDebugCapsule(
		World,
		Capsule->GetComponentLocation(),
		Capsule->GetScaledCapsuleHalfHeight(),
		Capsule->GetScaledCapsuleRadius(),
		Capsule->GetComponentQuat(),
		Color,
		false,
		0.f,
		0,
		T66GetCombatDebugThickness());
	T66DrawLabel(World, Capsule->GetComponentLocation() + FVector(0.f, 0.f, Capsule->GetScaledCapsuleHalfHeight() + 18.f), Label, Color);
}
