// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BossAttackTelegraph.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Gameplay/T66VisualUtil.h"

AT66BossAttackTelegraph::AT66BossAttackTelegraph()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.5f;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetCastShadow(false);
	VisualMesh->SetReceivesDecals(false);
	VisualMesh->SetCanEverAffectNavigation(false);
	RootComponent = VisualMesh;

	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		VisualMesh->SetStaticMesh(Sphere);
	}
	VisualMesh->SetRelativeScale3D(InitialScale);
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, TelegraphColor);
}

void AT66BossAttackTelegraph::ConfigureTelegraph(
	UStaticMesh* Mesh,
	const FLinearColor& Color,
	const FVector& StartScale,
	const FVector& EndScale,
	const float DurationSeconds,
	const float SpinDegreesPerSecond)
{
	if (Mesh && VisualMesh)
	{
		VisualMesh->SetStaticMesh(Mesh);
	}

	TelegraphColor = Color;
	TelegraphColor.A = 1.f;
	InitialScale = StartScale;
	FinalScale = EndScale;
	Duration = FMath::Max(0.05f, DurationSeconds);
	SpinRate = SpinDegreesPerSecond;
	Age = 0.f;
	InitialLifeSpan = Duration + 0.12f;

	if (VisualMesh)
	{
		VisualMesh->SetRelativeScale3D(InitialScale);
		FT66VisualUtil::ApplyT66Color(VisualMesh, this, TelegraphColor);
	}
}

void AT66BossAttackTelegraph::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Age += DeltaSeconds;
	const float Alpha = FMath::Clamp(Age / FMath::Max(0.05f, Duration), 0.f, 1.f);
	const float Pulse = 1.f + FMath::Sin(Alpha * PI * 6.f) * 0.08f;

	if (VisualMesh)
	{
		VisualMesh->SetRelativeScale3D(FMath::Lerp(InitialScale, FinalScale, Alpha) * Pulse);
		AddActorWorldRotation(FRotator(0.f, SpinRate * DeltaSeconds, 0.f));
	}

	if (Age >= Duration)
	{
		Destroy();
	}
}
