// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BossLaneBlockerHazard.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/T66RunStateSubsystem.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Gameplay/T66CombatDebugDraw.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"

AT66BossLaneBlockerHazard::AT66BossLaneBlockerHazard()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 3.0f;

	DamageBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	DamageBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamageBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageBox->SetGenerateOverlapEvents(false);
	RootComponent = DamageBox;

	TelegraphMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TelegraphMesh"));
	TelegraphMesh->SetupAttachment(RootComponent);
	TelegraphMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TelegraphMesh->SetCastShadow(false);
	TelegraphMesh->SetReceivesDecals(false);
	TelegraphMesh->SetCanEverAffectNavigation(false);

	ActiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ActiveMesh"));
	ActiveMesh->SetupAttachment(RootComponent);
	ActiveMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ActiveMesh->SetCastShadow(false);
	ActiveMesh->SetReceivesDecals(false);
	ActiveMesh->SetCanEverAffectNavigation(false);
	ActiveMesh->SetVisibility(false, true);

	if (UStaticMesh* Cylinder = FT66VisualUtil::GetBasicShapeCylinder())
	{
		TelegraphMesh->SetStaticMesh(Cylinder);
		ActiveMesh->SetStaticMesh(Cylinder);
	}
	TelegraphMesh->SetRelativeScale3D(VisualScale * 0.55f);
	ActiveMesh->SetRelativeScale3D(VisualScale);
	FT66VisualUtil::ApplyT66Color(TelegraphMesh, this, TelegraphColor);
	FT66VisualUtil::ApplyT66Color(ActiveMesh, this, ActiveColor);
}

void AT66BossLaneBlockerHazard::ConfigureHazard(
	const FLinearColor& InTelegraphColor,
	const FLinearColor& InActiveColor,
	const FVector& InVisualScale,
	const FVector& InDamageExtent,
	const float InWarningSeconds,
	const float InActiveSeconds,
	const int32 InDamageHP)
{
	TelegraphColor = InTelegraphColor;
	TelegraphColor.A = 1.f;
	ActiveColor = InActiveColor;
	ActiveColor.A = 1.f;
	VisualScale = InVisualScale;
	DamageExtent = FVector(
		FMath::Max(20.f, InDamageExtent.X),
		FMath::Max(20.f, InDamageExtent.Y),
		FMath::Max(20.f, InDamageExtent.Z));
	WarningSeconds = FMath::Max(0.05f, InWarningSeconds);
	ActiveSeconds = FMath::Max(0.1f, InActiveSeconds);
	DamageHP = FMath::Max(1, InDamageHP);
	InitialLifeSpan = WarningSeconds + ActiveSeconds + 0.2f;

	if (DamageBox)
	{
		DamageBox->SetBoxExtent(DamageExtent);
	}
	if (TelegraphMesh)
	{
		TelegraphMesh->SetRelativeScale3D(VisualScale * 0.55f);
		FT66VisualUtil::ApplyT66Color(TelegraphMesh, this, TelegraphColor);
	}
	if (ActiveMesh)
	{
		ActiveMesh->SetRelativeScale3D(VisualScale);
		FT66VisualUtil::ApplyT66Color(ActiveMesh, this, ActiveColor);
		ActiveMesh->SetVisibility(false, true);
	}
}

void AT66BossLaneBlockerHazard::BeginPlay()
{
	Super::BeginPlay();

	if (DamageBox)
	{
		DamageBox->OnComponentBeginOverlap.AddDynamic(this, &AT66BossLaneBlockerHazard::OnDamageBoxOverlap);
		DamageBox->SetBoxExtent(DamageExtent);
	}
}

void AT66BossLaneBlockerHazard::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	T66CombatDebugDraw::DrawDamageBox(DamageBox, bActivated ? TEXT("Boss Lane Damage") : TEXT("Boss Lane Warning"), bActivated);

	Age += DeltaSeconds;
	if (!bActivated)
	{
		const float Alpha = FMath::Clamp(Age / FMath::Max(0.05f, WarningSeconds), 0.f, 1.f);
		const float Pulse = 1.f + FMath::Sin(Alpha * PI * 8.f) * 0.08f;
		if (TelegraphMesh)
		{
			TelegraphMesh->SetRelativeScale3D(FMath::Lerp(VisualScale * 0.55f, VisualScale * 1.08f, Alpha) * Pulse);
			TelegraphMesh->AddLocalRotation(FRotator(0.f, 240.f * DeltaSeconds, 0.f));
		}
		if (Age >= WarningSeconds)
		{
			ActivateHazard();
		}
		return;
	}

	if (ActiveMesh)
	{
		const float ActiveAge = FMath::Max(0.f, Age - WarningSeconds);
		const float Pulse = 1.f + FMath::Sin(ActiveAge * PI * 5.f) * 0.04f;
		ActiveMesh->SetRelativeScale3D(VisualScale * Pulse);
		ActiveMesh->AddLocalRotation(FRotator(0.f, 150.f * DeltaSeconds, 0.f));
	}

	if (Age >= WarningSeconds + ActiveSeconds)
	{
		Destroy();
	}
}

void AT66BossLaneBlockerHazard::ActivateHazard()
{
	bActivated = true;

	if (TelegraphMesh)
	{
		TelegraphMesh->SetVisibility(false, true);
	}
	if (ActiveMesh)
	{
		ActiveMesh->SetVisibility(true, true);
	}
	if (DamageBox)
	{
		DamageBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		DamageBox->SetGenerateOverlapEvents(true);

		TArray<AActor*> OverlappingActors;
		DamageBox->GetOverlappingActors(OverlappingActors, AT66HeroBase::StaticClass());
		for (AActor* Actor : OverlappingActors)
		{
			ApplyDamageToActor(Actor);
		}
	}
}

void AT66BossLaneBlockerHazard::OnDamageBoxOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bActivated)
	{
		return;
	}

	ApplyDamageToActor(OtherActor);
}

void AT66BossLaneBlockerHazard::ApplyDamageToActor(AActor* OtherActor)
{
	if (!OtherActor || !OtherActor->IsA<AT66HeroBase>() || DamagedActors.Contains(OtherActor))
	{
		return;
	}

	DamagedActors.Add(OtherActor);

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
	{
		RunState->ApplyDamage(DamageHP, GetOwner(), FName(TEXT("BossLaneBlocker")), this);
	}
}
