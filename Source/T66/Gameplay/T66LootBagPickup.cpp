// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66LootBagPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

#include "Gameplay/T66PlayerController.h"

static void ApplyT66Color(UStaticMeshComponent* Mesh, UObject* Outer, const FLinearColor& Color)
{
	if (!Mesh) return;

	// Prefer our placeholder material so the color parameter name is stable ("Color").
	if (UMaterialInterface* ColorMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_PlaceholderColor.M_PlaceholderColor")))
	{
		if (UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, Outer ? Outer : Mesh))
		{
			Mat->SetVectorParameterValue(FName("Color"), Color);
			Mesh->SetMaterial(0, Mat);
			return;
		}
	}

	// Fallback to whatever material is on the mesh.
	if (UMaterialInstanceDynamic* Mat = Mesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Mat->SetVectorParameterValue(FName("Color"), Color);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
	}
}

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

	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		VisualMesh->SetStaticMesh(Cube);
		VisualMesh->SetRelativeScale3D(FVector(0.35f, 0.25f, 0.20f));
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 20.f));
	}
	ApplyT66Color(VisualMesh, this, FLinearColor(0.9f, 0.8f, 0.25f, 1.f));
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
	ApplyT66Color(VisualMesh, this, FT66RarityUtil::GetRarityColor(LootRarity));
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

