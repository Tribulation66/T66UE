// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66StartGate.h"
#include "Gameplay/T66HeroBase.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"

AT66StartGate::AT66StartGate()
{
	PrimaryActorTick.bCanEverTick = true;

	// Thin trigger between the poles: player must get really close / walk through to start timer
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(60.f, 80.f, 180.f)); // narrow passage between poles
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
	PoleLeft->SetRelativeScale3D(FVector(PoleRadius / 50.f, PoleRadius / 50.f, PoleHeight / 100.f)); // cylinder default 50 radius, 100 height
	PoleLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PoleLeft->SetupAttachment(RootComponent);

	PoleRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PoleRight"));
	PoleRight->SetStaticMesh(CylinderMesh);
	PoleRight->SetRelativeLocation(FVector(0.f, PoleSpacing, PoleHeight * 0.5f));
	PoleRight->SetRelativeScale3D(FVector(PoleRadius / 50.f, PoleRadius / 50.f, PoleHeight / 100.f));
	PoleRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PoleRight->SetupAttachment(RootComponent);
}

void AT66StartGate::BeginPlay()
{
	Super::BeginPlay();
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AT66StartGate::OnBoxBeginOverlap);
}

void AT66StartGate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Robust proximity trigger (covers cases where the player starts inside overlap or overlap events don't fire).
	if (bTriggered) return;

	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Pawn) return;

	const FVector HeroLoc = Pawn->GetActorLocation();
	const FVector GateLoc = GetActorLocation();
	const float Dist2D = FVector::Dist2D(HeroLoc, GateLoc);
	if (Dist2D <= TriggerDistance2D)
	{
		TryTriggerForActor(Pawn);
	}
}

void AT66StartGate::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryTriggerForActor(OtherActor);
}

void AT66StartGate::TryTriggerForActor(AActor* OtherActor)
{
	if (bTriggered || !OtherActor) return;
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero) return;

	UWorld* World = GetWorld();
	if (!World) return;
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return;
	UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
	if (!RunState) return;

	bTriggered = true;
	RunState->SetStageTimerActive(true);
	RunState->AddStructuredEvent(ET66RunEventType::StageEntered, FString::Printf(TEXT("Stage=%d"), RunState->GetCurrentStage()));
}
