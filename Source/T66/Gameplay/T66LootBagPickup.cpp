// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

#include "Gameplay/T66PlayerController.h"

AT66LootBagPickup::AT66LootBagPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	InitialLifeSpan = 120.f;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SphereComponent->SetSphereRadius(120.f);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = SphereComponent;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		VisualMesh->SetStaticMesh(Cube);
		VisualMesh->SetRelativeScale3D(FVector(0.35f, 0.25f, 0.20f));
		// Cube is 100 units tall; scale Z=0.20 => 20 tall, half-height=10. Sit on ground plane.
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	}
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FLinearColor(0.9f, 0.8f, 0.25f, 1.f));
}

void AT66LootBagPickup::BeginPlay()
{
	Super::BeginPlay();
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AT66LootBagPickup::OnSphereBeginOverlap);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AT66LootBagPickup::OnSphereEndOverlap);
	UpdateVisualsFromRarity();
}

void AT66LootBagPickup::SetItemID(FName InItemID)
{
	ItemID = InItemID;
	// Visuals are driven by LootRarity (not the item), so no change here.
}

void AT66LootBagPickup::SetLootRarity(ET66Rarity InRarity)
{
	LootRarity = InRarity;
	UpdateVisualsFromRarity();
}

void AT66LootBagPickup::ConsumeAndDestroy()
{
	Destroy();
}

void AT66LootBagPickup::UpdateVisualsFromRarity()
{
	if (!VisualMesh) return;
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FT66RarityUtil::GetRarityColor(LootRarity));
}

void AT66LootBagPickup::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	AT66PlayerController* PC = Cast<AT66PlayerController>(Pawn->GetController());
	if (!PC) return;

	PC->SetNearbyLootBag(this);
}

void AT66LootBagPickup::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	AT66PlayerController* PC = Cast<AT66PlayerController>(Pawn->GetController());
	if (!PC) return;

	PC->ClearNearbyLootBag(this);
}

