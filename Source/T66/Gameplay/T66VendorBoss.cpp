// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66VendorBoss.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66VisualUtil.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AT66VendorBoss::AT66VendorBoss()
{
	BossID = FName(TEXT("VendorBoss"));
	MaxHP = 1000;
	FireIntervalSeconds = 0.8f;
	ProjectileSpeed = 950.f;
	ProjectileDamageHearts = 1;

	GroundAOEIntervalSeconds = 6.f;
	GroundAOERadius = 350.f;
	GroundAOEWarningSeconds = 1.2f;
	GroundAOEBaseDamageHP = 30;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 520.f;
	}

	bUsesBossPartHitZones = true;
	BossPartDefinitions.Reset();
	BossPartDefinitions.Reserve(6);

	auto GetPartID = [](const ET66HitZoneType HitZoneType) -> FName
	{
		switch (HitZoneType)
		{
		case ET66HitZoneType::Head: return FName(TEXT("Head"));
		case ET66HitZoneType::Core: return FName(TEXT("Core"));
		case ET66HitZoneType::LeftArm: return FName(TEXT("LeftArm"));
		case ET66HitZoneType::RightArm: return FName(TEXT("RightArm"));
		case ET66HitZoneType::LeftLeg: return FName(TEXT("LeftLeg"));
		case ET66HitZoneType::RightLeg: return FName(TEXT("RightLeg"));
		case ET66HitZoneType::Body:
		default:
			return FName(TEXT("Body"));
		}
	};

	auto AddPart = [this, GetPartID](const ET66HitZoneType HitZoneType, const float HPWeight, const float DamageMultiplier, const FVector& RelativeLocation, const float Radius)
	{
		FT66BossPartDefinition& Part = BossPartDefinitions.AddDefaulted_GetRef();
		Part.PartID = GetPartID(HitZoneType);
		Part.HitZoneType = HitZoneType;
		Part.HPWeight = HPWeight;
		Part.DamageMultiplier = DamageMultiplier;
		Part.RelativeLocation = RelativeLocation;
		Part.Radius = Radius;
		Part.bTargetable = true;
	};

	AddPart(ET66HitZoneType::Core, 0.36f, 1.00f, FVector(0.f, 0.f, 118.f), 92.f);
	AddPart(ET66HitZoneType::Head, 0.10f, 1.40f, FVector(0.f, 0.f, 184.f), 40.f);
	AddPart(ET66HitZoneType::LeftArm, 0.19f, 1.28f, FVector(12.f, -122.f, 130.f), 58.f);
	AddPart(ET66HitZoneType::RightArm, 0.19f, 1.28f, FVector(12.f, 122.f, 130.f), 58.f);
	AddPart(ET66HitZoneType::LeftLeg, 0.08f, 0.85f, FVector(0.f, -44.f, 42.f), 46.f);
	AddPart(ET66HitZoneType::RightLeg, 0.08f, 0.85f, FVector(0.f, 44.f, 42.f), 46.f);
}

void AT66VendorBoss::BeginPlay()
{
	Super::BeginPlay();

	// Placeholder mesh: cube, dark (overridden by ApplyCharacterVisual if mapped).
	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		VisualMesh->SetStaticMesh(Cube);
		VisualMesh->SetRelativeScale3D(FVector(4.f, 4.f, 4.f));
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 112.f));
	}
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FLinearColor(0.08f, 0.08f, 0.10f, 1.f));

	// Vendor boss must use the same mesh as Vendor NPC (per rule).
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
		{
			const bool bApplied = Visuals->ApplyCharacterVisual(FName(TEXT("VendorBoss")), GetMesh(), VisualMesh, true);
			if (!bApplied && GetMesh())
			{
				GetMesh()->SetVisibility(false, true);
			}
		}
	}

	ForceAwaken();
}

void AT66VendorBoss::Die()
{
	UWorld* World = GetWorld();
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->ResetVendorAnger();
		}
	}

	// Respawn Vendor NPC so the house becomes interactable again this stage.
	if (World)
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<AT66VendorNPC>(AT66VendorNPC::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, P);
	}

	Super::Die();
}
