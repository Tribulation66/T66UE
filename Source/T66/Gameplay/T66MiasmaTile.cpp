// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiasmaTile.h"
#include "Gameplay/T66HeroBase.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

AT66MiasmaTile::AT66MiasmaTile()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = Box;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		VisualMesh->SetStaticMesh(Cube);
	}

	// Full black
	if (UMaterialInstanceDynamic* Mat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.f, 0.f, 0.f, 1.f));
	}
}

void AT66MiasmaTile::BeginPlay()
{
	Super::BeginPlay();

	// Configure size (thin sheet slightly above ground)
	const FVector HalfExtents(TileSize * 0.5f, TileSize * 0.5f, 60.f);
	Box->SetBoxExtent(HalfExtents);

	// Cube is 100 units by default
	const float ScaleXY = TileSize / 100.f;
	VisualMesh->SetRelativeScale3D(FVector(ScaleXY, ScaleXY, 0.05f));
	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 5.f));

	Box->OnComponentBeginOverlap.AddDynamic(this, &AT66MiasmaTile::OnBoxBeginOverlap);
	Box->OnComponentEndOverlap.AddDynamic(this, &AT66MiasmaTile::OnBoxEndOverlap);
}

void AT66MiasmaTile::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || bHeroOverlapping) return;
	if (!Cast<AT66HeroBase>(OtherActor)) return;

	bHeroOverlapping = true;
	ApplyMiasmaDamageTick();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DamageTimerHandle, this, &AT66MiasmaTile::ApplyMiasmaDamageTick, DamageIntervalSeconds, true, DamageIntervalSeconds);
	}
}

void AT66MiasmaTile::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;
	if (!Cast<AT66HeroBase>(OtherActor)) return;

	bHeroOverlapping = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}
}

void AT66MiasmaTile::ApplyMiasmaDamageTick()
{
	if (!bHeroOverlapping) return;
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState)
	{
		RunState->ApplyDamage(1);
	}
}

