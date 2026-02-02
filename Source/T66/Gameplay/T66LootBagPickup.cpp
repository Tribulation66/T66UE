// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/SoftObjectPath.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

#include "Gameplay/T66PlayerController.h"

AT66LootBagPickup::AT66LootBagPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	InitialLifeSpan = 120.f;

	// Physics root: blocks the world so the bag can fall naturally with gravity.
	PhysicsRoot = CreateDefaultSubobject<USphereComponent>(TEXT("PhysicsRoot"));
	PhysicsRoot->SetSphereRadius(30.f);
	PhysicsRoot->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PhysicsRoot->SetGenerateOverlapEvents(false);
	PhysicsRoot->SetCollisionResponseToAllChannels(ECR_Ignore);
	PhysicsRoot->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	PhysicsRoot->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	RootComponent = PhysicsRoot;

	// Pickup sphere: large overlap radius so interaction is easy.
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->SetSphereRadius(120.f);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FallMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("FallMovement"));
	FallMovement->SetUpdatedComponent(PhysicsRoot);
	FallMovement->InitialSpeed = 0.f;
	FallMovement->MaxSpeed = 5000.f;
	FallMovement->ProjectileGravityScale = 1.f;
	FallMovement->bRotationFollowsVelocity = false;
	FallMovement->bShouldBounce = false;

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
		const float R = PhysicsRoot ? PhysicsRoot->GetScaledSphereRadius() : 0.f;
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 10.f - R));
	}
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

		// Configure collision radius from bounds so the bag lands on the ground naturally.
		if (PhysicsRoot)
		{
			const FBoxSphereBounds B2 = Mesh->GetBounds();
			const float XY = FMath::Max(B2.BoxExtent.X, B2.BoxExtent.Y);
			const float R = FMath::Clamp(XY, 12.f, 140.f);
			PhysicsRoot->SetSphereRadius(R);
		}

		// Ground the mesh to the *physics root* bottom using its bounds, independent of pivot placement.
		const FBoxSphereBounds B = Mesh->GetBounds();
		const float BottomZ = (B.Origin.Z - B.BoxExtent.Z);
		const float R = PhysicsRoot ? PhysicsRoot->GetScaledSphereRadius() : 0.f;
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, -BottomZ - R));
		return;
	}

	// Fallback: colored cube.
	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		VisualMesh->EmptyOverrideMaterials();
		VisualMesh->SetStaticMesh(Cube);
		VisualMesh->SetRelativeScale3D(FVector(0.35f, 0.25f, 0.20f));
		// Approximate collision radius from scaled cube half-extents (100 cube * scale / 2).
		if (PhysicsRoot)
		{
			const float HalfX = 50.f * 0.35f;
			const float HalfY = 50.f * 0.25f;
			PhysicsRoot->SetSphereRadius(FMath::Clamp(FMath::Max(HalfX, HalfY), 12.f, 60.f));
		}
		const float R = PhysicsRoot ? PhysicsRoot->GetScaledSphereRadius() : 0.f;
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 10.f - R));
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

