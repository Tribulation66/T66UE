// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BossGate.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66BossBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

AT66BossGate::AT66BossGate()
{
	PrimaryActorTick.bCanEverTick = true;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(60.f, 80.f, 180.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = TriggerBox;

	UStaticMesh* CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	const float PoleRadius = 40.f;
	const float PoleHeight = 320.f;
	const float PoleSpacing = 100.f;

	PoleLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PoleLeft"));
	PoleLeft->SetStaticMesh(CylinderMesh);
	PoleLeft->SetRelativeLocation(FVector(0.f, -PoleSpacing, PoleHeight * 0.5f));
	PoleLeft->SetRelativeScale3D(FVector(PoleRadius / 50.f, PoleRadius / 50.f, PoleHeight / 100.f));
	PoleLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PoleLeft->SetupAttachment(RootComponent);

	PoleRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PoleRight"));
	PoleRight->SetStaticMesh(CylinderMesh);
	PoleRight->SetRelativeLocation(FVector(0.f, PoleSpacing, PoleHeight * 0.5f));
	PoleRight->SetRelativeScale3D(FVector(PoleRadius / 50.f, PoleRadius / 50.f, PoleHeight / 100.f));
	PoleRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PoleRight->SetupAttachment(RootComponent);
}

void AT66BossGate::BeginPlay()
{
	Super::BeginPlay();
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AT66BossGate::OnBoxBeginOverlap);
}

void AT66BossGate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bTriggered) return;

	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Pawn) return;

	const float Dist2D = FVector::Dist2D(Pawn->GetActorLocation(), GetActorLocation());
	if (Dist2D <= TriggerDistance2D)
	{
		TryTriggerForActor(Pawn);
	}
}

void AT66BossGate::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryTriggerForActor(OtherActor);
}

void AT66BossGate::TryTriggerForActor(AActor* OtherActor)
{
	if (bTriggered || !OtherActor) return;
	if (!Cast<AT66HeroBase>(OtherActor)) return;

	bTriggered = true;

	// Awaken the stage boss immediately once the player passes the boss pillars.
	UWorld* World = GetWorld();
	if (!World) return;
	for (TActorIterator<AT66BossBase> It(World); It; ++It)
	{
		if (AT66BossBase* Boss = *It)
		{
			Boss->ForceAwaken();
			break;
		}
	}
}

