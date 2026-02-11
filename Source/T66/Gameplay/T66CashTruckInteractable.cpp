// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CashTruckInteractable.h"
#include "Gameplay/T66CashTruckMimicEnemy.h"
#include "Core/T66RunStateSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/SoftObjectPath.h"

AT66CashTruckInteractable::AT66CashTruckInteractable()
{
	// Body: cube
	if (VisualMesh)
	{
		VisualMesh->SetRelativeScale3D(FVector(2.2f, 1.2f, 1.0f));
		// Grounded: wheels radius ~11, body half-height 50 => center at ~61.
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 61.f));
	}

	WheelMeshes.SetNum(4);
	for (int32 i = 0; i < WheelMeshes.Num(); ++i)
	{
		const FName Name = FName(*FString::Printf(TEXT("Wheel_%d"), i));
		WheelMeshes[i] = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		WheelMeshes[i]->SetupAttachment(RootComponent);
		WheelMeshes[i]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (UStaticMesh* Cylinder = FT66VisualUtil::GetBasicShapeCylinder())
	{
		for (UStaticMeshComponent* W : WheelMeshes)
		{
			if (!W) continue;
			W->SetStaticMesh(Cylinder);
			// wheels: thin cylinders
			W->SetRelativeScale3D(FVector(0.22f, 0.22f, 0.08f));
			W->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
		}
	}

	// wheel placement around the body footprint
	static const FVector Offsets[4] =
	{
		FVector( 120.f,  70.f,  11.f),
		FVector( 120.f, -70.f,  11.f),
		FVector(-120.f,  70.f,  11.f),
		FVector(-120.f, -70.f,  11.f),
	};
	for (int32 i = 0; i < WheelMeshes.Num(); ++i)
	{
		if (WheelMeshes[i])
		{
			WheelMeshes[i]->SetRelativeLocation(Offsets[i]);
		}
	}

	ApplyRarityVisuals();

	// Default expected import locations (safe if missing).
	// Note: per-color subfolders avoid material/texture name collisions (Material_001, Image_0...).
	MeshBlack = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Trucks/Black/SM_CashTruck_Black.SM_CashTruck_Black")));
	MeshRed = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Trucks/Red/SM_CashTruck_Red.SM_CashTruck_Red")));
	MeshYellow = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Trucks/Yellow/SM_CashTruck_Yellow.SM_CashTruck_Yellow")));
	MeshWhite = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Trucks/White/SM_CashTruck_White.SM_CashTruck_White")));
}

void AT66CashTruckInteractable::ApplyRarityVisuals()
{
	auto PickMesh = [&]() -> const TSoftObjectPtr<UStaticMesh>&
	{
		switch (Rarity)
		{
			case ET66Rarity::Black: return MeshBlack;
			case ET66Rarity::Red: return MeshRed;
			case ET66Rarity::Yellow: return MeshYellow;
			case ET66Rarity::White: return MeshWhite;
			default: return MeshBlack;
		}
	};

	UStaticMesh* M = nullptr;
	const TSoftObjectPtr<UStaticMesh>& Ptr = PickMesh();
	if (!Ptr.IsNull())
	{
		M = Ptr.Get();
		if (!M)
		{
			M = Ptr.LoadSynchronous();
		}
	}

	if (M && VisualMesh)
	{
		// Ensure we don't keep any placeholder tint material from a prior fallback.
		VisualMesh->EmptyOverrideMaterials();
		VisualMesh->SetStaticMesh(M);
		VisualMesh->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
		VisualMesh->SetRelativeRotation(FRotator::ZeroRotator);
		// Ground to actor origin using bounds (handles pivots already at the base).
		const FBoxSphereBounds B = M->GetBounds();
		const float BottomZ = (B.Origin.Z - B.BoxExtent.Z);
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, -BottomZ));

		// Hide placeholder wheels (imported mesh usually includes them).
		for (UStaticMeshComponent* W : WheelMeshes)
		{
			if (W) W->SetVisibility(false, true);
		}
		return;
	}

	// Make rarity obvious: tint the truck body.
	const FLinearColor BodyC = bIsMimic ? FLinearColor(0.35f, 0.10f, 0.55f, 1.f) : FT66RarityUtil::GetRarityColor(Rarity);
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, BodyC);

	const FLinearColor WheelC = bIsMimic ? FLinearColor(0.05f, 0.05f, 0.05f, 1.f) : FLinearColor(0.15f, 0.15f, 0.18f, 1.f);
	for (UStaticMeshComponent* W : WheelMeshes)
	{
		FT66VisualUtil::ApplyT66Color(W, this, WheelC);
	}
}

bool AT66CashTruckInteractable::Interact(APlayerController* PC)
{
	if (bConsumed) return false;

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return false;

	if (bIsMimic)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<AT66CashTruckMimicEnemy>(AT66CashTruckMimicEnemy::StaticClass(), GetActorLocation(), GetActorRotation(), SpawnParams);
		bConsumed = true;
		Destroy();
		return true;
	}

	int32 Gold = 50;
	switch (Rarity)
	{
	case ET66Rarity::Black:  Gold = 50; break;
	case ET66Rarity::Red:    Gold = 150; break;
	case ET66Rarity::Yellow: Gold = 300; break;
	case ET66Rarity::White:  Gold = 600; break;
	default:                 Gold = 50; break;
	}

	RunState->AddGold(Gold);
	bConsumed = true;
	Destroy();
	return true;
}

float AT66CashTruckInteractable::GetLuckChestMultiplier() const
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return 1.f;
	return RunState->GetSecondaryStatValue(ET66SecondaryStatType::TreasureChest);
}

