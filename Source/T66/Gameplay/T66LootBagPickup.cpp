// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/SoftObjectPath.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

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

	// Default expected import locations (safe if missing; we fall back to the cube).
	// Note: meshes are imported into per-color subfolders so their materials/textures don't collide.
	MeshBlack = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/LootBags/Black/SM_LootBag_Black.SM_LootBag_Black")));
	MeshRed = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/LootBags/Red/SM_LootBag_Red.SM_LootBag_Red")));
	MeshYellow = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/LootBags/Yellow/SM_LootBag_Yellow.SM_LootBag_Yellow")));
	MeshWhite = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/LootBags/White/SM_LootBag_White.SM_LootBag_White")));

	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		VisualMesh->SetStaticMesh(Cube);
		VisualMesh->SetRelativeScale3D(FVector(0.35f, 0.25f, 0.20f));
		// Cube is 100 units tall; scale Z=0.20 => 20 tall, half-height=10. Sit on ground plane.
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	}
}

void AT66LootBagPickup::BeginPlay()
{
	Super::BeginPlay();
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AT66LootBagPickup::OnSphereBeginOverlap);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AT66LootBagPickup::OnSphereEndOverlap);
	UpdateVisualsFromRarity();

	// Snap to ground so bags never float (spawns may happen above the floor).
	if (UWorld* World = GetWorld())
	{
		FHitResult Hit;
		const FVector Here = GetActorLocation();
		const FVector Start = Here + FVector(0.f, 0.f, 2000.f);
		const FVector End = Here - FVector(0.f, 0.f, 9000.f);
		FCollisionQueryParams Params(SCENE_QUERY_STAT(T66LootBagSnap), false, this);
		// Use object query so this works whether the ground is WorldStatic or WorldDynamic.
		FCollisionObjectQueryParams ObjParams;
		ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		if (World->LineTraceSingleByObjectType(Hit, Start, End, ObjParams, Params))
		{
			SetActorLocation(Hit.ImpactPoint, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
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

	auto PickMesh = [&]() -> const TSoftObjectPtr<UStaticMesh>&
	{
		switch (LootRarity)
		{
			case ET66Rarity::Black: return MeshBlack;
			case ET66Rarity::Red: return MeshRed;
			case ET66Rarity::Yellow: return MeshYellow;
			case ET66Rarity::White: return MeshWhite;
			default: return MeshBlack;
		}
	};

	const TSoftObjectPtr<UStaticMesh>& MeshPtr = PickMesh();
	UStaticMesh* Mesh = nullptr;
	if (!MeshPtr.IsNull())
	{
		Mesh = MeshPtr.Get();
		if (!Mesh)
		{
			Mesh = MeshPtr.LoadSynchronous();
		}
	}

	if (Mesh)
	{
		// Use real mesh (per-rarity), no tinting. Clear any prior placeholder material overrides.
		VisualMesh->EmptyOverrideMaterials();
		VisualMesh->SetStaticMesh(Mesh);
		VisualMesh->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));

		// Ground the mesh to actor origin using its bounds, independent of pivot placement.
		const FBoxSphereBounds B = Mesh->GetBounds();
		const float BottomZ = (B.Origin.Z - B.BoxExtent.Z);
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, -BottomZ));
		return;
	}

	// Fallback: colored cube.
	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		VisualMesh->EmptyOverrideMaterials();
		VisualMesh->SetStaticMesh(Cube);
		VisualMesh->SetRelativeScale3D(FVector(0.35f, 0.25f, 0.20f));
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	}
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

