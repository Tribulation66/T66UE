// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66CombatHitZoneComponent.h"
#include "Gameplay/T66BossProjectile.h"
#include "Gameplay/T66BossGroundAOE.h"
#include "Gameplay/T66GameMode.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/T66VisualUtil.h"
#include "Engine/World.h"

namespace
{
	APawn* T66ResolveClosestBossTargetPawn(const AActor* ContextActor)
	{
		const UWorld* World = ContextActor ? ContextActor->GetWorld() : nullptr;
		if (!World)
		{
			return nullptr;
		}

		const FVector Origin = ContextActor->GetActorLocation();
		APawn* BestPawn = nullptr;
		float BestDistSq = TNumericLimits<float>::Max();
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			APawn* Pawn = It->Get() ? It->Get()->GetPawn() : nullptr;
			if (!Pawn)
			{
				continue;
			}

			const float DistSq = FVector::DistSquared2D(Origin, Pawn->GetActorLocation());
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestPawn = Pawn;
			}
		}

		return BestPawn;
	}

	void T66ApplyBossDisplacement(ACharacter* Character, const FVector& Origin, float Distance, const bool bTowardOrigin)
	{
		if (!Character || FMath::IsNearlyZero(Distance))
		{
			return;
		}

		FVector Dir = Origin - Character->GetActorLocation();
		Dir.Z = 0.f;
		const float Magnitude = Dir.Size();
		if (Magnitude <= KINDA_SMALL_NUMBER)
		{
			return;
		}

		Dir /= Magnitude;
		const float ClampedDistance = FMath::Clamp(FMath::Abs(Distance), 0.f, bTowardOrigin ? FMath::Max(0.f, Magnitude - 80.f) : FMath::Abs(Distance));
		if (ClampedDistance <= 0.f)
		{
			return;
		}

		const FVector Delta = (bTowardOrigin ? Dir : -Dir) * ClampedDistance;
		Character->SetActorLocation(Character->GetActorLocation() + Delta, true);
		if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
		{
			Move->StopMovementImmediately();
		}
	}

	FName T66GetDefaultBossPartID(const ET66HitZoneType HitZoneType)
	{
		switch (HitZoneType)
		{
		case ET66HitZoneType::Head: return FName(TEXT("Head"));
		case ET66HitZoneType::Core: return FName(TEXT("Core"));
		case ET66HitZoneType::WeakPoint: return FName(TEXT("WeakPoint"));
		case ET66HitZoneType::LeftArm: return FName(TEXT("LeftArm"));
		case ET66HitZoneType::RightArm: return FName(TEXT("RightArm"));
		case ET66HitZoneType::LeftLeg: return FName(TEXT("LeftLeg"));
		case ET66HitZoneType::RightLeg: return FName(TEXT("RightLeg"));
		case ET66HitZoneType::Body:
		default:
			return FName(TEXT("Body"));
		}
	}

	FVector T66GetDefaultBossPartLocation(const ET66HitZoneType HitZoneType)
	{
		switch (HitZoneType)
		{
		case ET66HitZoneType::Head: return FVector(0.f, 0.f, 180.f);
		case ET66HitZoneType::Core: return FVector(0.f, 0.f, 120.f);
		case ET66HitZoneType::WeakPoint: return FVector(40.f, 0.f, 120.f);
		case ET66HitZoneType::LeftArm: return FVector(0.f, -90.f, 120.f);
		case ET66HitZoneType::RightArm: return FVector(0.f, 90.f, 120.f);
		case ET66HitZoneType::LeftLeg: return FVector(0.f, -45.f, 45.f);
		case ET66HitZoneType::RightLeg: return FVector(0.f, 45.f, 45.f);
		case ET66HitZoneType::Body:
		default:
			return FVector(0.f, 0.f, 110.f);
		}
	}

	FT66BossPartDefinition T66MakeBossPartDefinition(
		const ET66HitZoneType HitZoneType,
		const float HPWeight,
		const float DamageMultiplier,
		const float Radius,
		const FVector& RelativeOffset = FVector::ZeroVector)
	{
		FT66BossPartDefinition Definition;
		Definition.PartID = T66GetDefaultBossPartID(HitZoneType);
		Definition.HitZoneType = HitZoneType;
		Definition.HPWeight = HPWeight;
		Definition.DamageMultiplier = DamageMultiplier;
		Definition.RelativeLocation = T66GetDefaultBossPartLocation(HitZoneType) + RelativeOffset;
		Definition.Radius = Radius;
		Definition.bTargetable = true;
		return Definition;
	}

	ET66BossPartProfile T66ResolveLegacyBossPartProfile(const FName BossID)
	{
		const FString BossIDString = BossID.ToString();
		if (!BossIDString.StartsWith(TEXT("Boss_")))
		{
			return ET66BossPartProfile::UseActorDefault;
		}

		const FString Suffix = BossIDString.RightChop(5);
		if (!Suffix.IsNumeric())
		{
			return ET66BossPartProfile::UseActorDefault;
		}

		const int32 BossNumber = FCString::Atoi(*Suffix);
		switch (BossNumber % 4)
		{
		case 1:
			return ET66BossPartProfile::HumanoidBalanced;
		case 2:
			return ET66BossPartProfile::Sharpshooter;
		case 3:
			return ET66BossPartProfile::Juggernaut;
		case 0:
		default:
			return ET66BossPartProfile::Duelist;
		}
	}
}

AT66BossBase::AT66BossBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Needed so AddMovementInput works (it requires a Controller when bForce=false).
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 350.f;
		Move->bOrientRotationToMovement = true;
		Move->RotationRate = FRotator(0.f, 720.f, 0.f);
		BaseMoveSpeed = Move->MaxWalkSpeed;
	}
	bUseControllerRotationYaw = false;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Align primitive mesh to ground when capsule is grounded:
	// capsule half-height~88, sphere half-height=50*6=300 => relative Z = 300 - 88 = 212.
	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 212.f));

	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		VisualMesh->SetStaticMesh(Sphere);
		VisualMesh->SetRelativeScale3D(FVector(6.f, 6.f, 6.f)); // very large sphere
	}
	if (UMaterialInstanceDynamic* Mat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.9f, 0.05f, 0.05f, 1.f));
	}

	// Prepare built-in SkeletalMeshComponent for imported models.
	if (USkeletalMeshComponent* Skel = GetMesh())
	{
		Skel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Skel->SetVisibility(false, true); // shown only when a character visual mapping exists
	}

	EnsureDefaultBossPartDefinitions();
}

void AT66BossBase::AssignBossPartDefinitionsForProfile(const ET66BossPartProfile InProfile)
{
	if (InProfile == ET66BossPartProfile::UseActorDefault)
	{
		return;
	}

	bUsesBossPartHitZones = true;
	BossPartDefinitions.Reset();
	BossPartDefinitions.Reserve(6);

	switch (InProfile)
	{
	case ET66BossPartProfile::HumanoidBalanced:
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::Core, 0.32f, 1.00f, 84.f, FVector(0.f, 0.f, 2.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::Head, 0.16f, 1.55f, 46.f, FVector(0.f, 0.f, 4.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::LeftArm, 0.13f, 1.10f, 46.f, FVector(0.f, -8.f, 6.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::RightArm, 0.13f, 1.10f, 46.f, FVector(0.f, 8.f, 6.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::LeftLeg, 0.13f, 1.00f, 44.f, FVector(0.f, -6.f, 0.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::RightLeg, 0.13f, 1.00f, 44.f, FVector(0.f, 6.f, 0.f)));
		break;

	case ET66BossPartProfile::Sharpshooter:
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::Core, 0.28f, 1.00f, 76.f, FVector(6.f, 0.f, 6.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::Head, 0.12f, 1.90f, 38.f, FVector(0.f, 0.f, 12.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::LeftArm, 0.16f, 1.20f, 50.f, FVector(10.f, -18.f, 10.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::RightArm, 0.16f, 1.20f, 50.f, FVector(10.f, 18.f, 10.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::LeftLeg, 0.14f, 0.95f, 40.f, FVector(0.f, -6.f, -4.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::RightLeg, 0.14f, 0.95f, 40.f, FVector(0.f, 6.f, -4.f)));
		break;

	case ET66BossPartProfile::Juggernaut:
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::Core, 0.40f, 1.00f, 96.f, FVector(0.f, 0.f, 0.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::Head, 0.14f, 1.25f, 52.f, FVector(0.f, 0.f, 6.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::LeftArm, 0.14f, 0.95f, 56.f, FVector(-4.f, -10.f, 6.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::RightArm, 0.14f, 0.95f, 56.f, FVector(-4.f, 10.f, 6.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::LeftLeg, 0.09f, 1.10f, 48.f, FVector(0.f, -4.f, 4.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::RightLeg, 0.09f, 1.10f, 48.f, FVector(0.f, 4.f, 4.f)));
		break;

	case ET66BossPartProfile::Duelist:
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::Core, 0.26f, 1.05f, 74.f, FVector(8.f, 0.f, 4.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::Head, 0.15f, 1.70f, 42.f, FVector(0.f, 0.f, 10.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::LeftArm, 0.17f, 1.28f, 50.f, FVector(12.f, -20.f, 8.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::RightArm, 0.17f, 1.28f, 50.f, FVector(12.f, 20.f, 8.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::LeftLeg, 0.125f, 0.90f, 42.f, FVector(4.f, -8.f, -2.f)));
		BossPartDefinitions.Add(T66MakeBossPartDefinition(ET66HitZoneType::RightLeg, 0.125f, 0.90f, 42.f, FVector(4.f, 8.f, -2.f)));
		break;

	case ET66BossPartProfile::UseActorDefault:
	default:
		break;
	}
}

void AT66BossBase::EnsureDefaultBossPartDefinitions()
{
	if (BossPartDefinitions.Num() > 0)
	{
		for (FT66BossPartDefinition& Definition : BossPartDefinitions)
		{
			if (Definition.PartID.IsNone())
			{
				Definition.PartID = T66GetDefaultBossPartID(Definition.HitZoneType);
			}

			if (Definition.RelativeLocation.IsNearlyZero())
			{
				Definition.RelativeLocation = T66GetDefaultBossPartLocation(Definition.HitZoneType);
			}

			Definition.HPWeight = FMath::Max(0.01f, Definition.HPWeight);
			Definition.DamageMultiplier = FMath::Max(0.1f, Definition.DamageMultiplier);
			Definition.Radius = FMath::Max(1.f, Definition.Radius);
		}
		return;
	}

	auto AddDefaultPart = [this](const ET66HitZoneType HitZoneType, const float HPWeight, const float DamageMultiplier, const float Radius)
	{
		FT66BossPartDefinition& Part = BossPartDefinitions.AddDefaulted_GetRef();
		Part.PartID = T66GetDefaultBossPartID(HitZoneType);
		Part.HitZoneType = HitZoneType;
		Part.HPWeight = HPWeight;
		Part.DamageMultiplier = DamageMultiplier;
		Part.RelativeLocation = T66GetDefaultBossPartLocation(HitZoneType);
		Part.Radius = Radius;
		Part.bTargetable = true;
	};

	AddDefaultPart(ET66HitZoneType::Core, 0.34f, 1.0f, 84.f);
	AddDefaultPart(ET66HitZoneType::Head, 0.16f, 1.5f, 46.f);
	AddDefaultPart(ET66HitZoneType::LeftArm, 0.12f, 1.0f, 42.f);
	AddDefaultPart(ET66HitZoneType::RightArm, 0.12f, 1.0f, 42.f);
	AddDefaultPart(ET66HitZoneType::LeftLeg, 0.13f, 1.0f, 44.f);
	AddDefaultPart(ET66HitZoneType::RightLeg, 0.13f, 1.0f, 44.f);
}

void AT66BossBase::RefreshCombatHitZoneState()
{
	const bool bEnableHitZones = bUsesBossPartHitZones && BossPartStates.Num() > 0;

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_Visibility, bEnableHitZones ? ECR_Ignore : ECR_Block);
	}

	for (FT66BossPartRuntimeState& Part : BossPartStates)
	{
		UT66CombatHitZoneComponent* Zone = Part.ZoneComponent;
		if (!Zone)
		{
			continue;
		}

		const bool bEnableZone = bEnableHitZones && Zone->bTargetable;
		Zone->HitZoneType = Part.HitZoneType;
		Zone->HitZoneName = Part.PartID;
		Zone->DamageMultiplier = FMath::Max(0.1f, Part.DamageMultiplier);
		Zone->SetCollisionEnabled(bEnableZone ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		Zone->SetGenerateOverlapEvents(bEnableZone);
		Zone->SetHiddenInGame(true);
	}
}

void AT66BossBase::BuildBossPartSnapshots(TArray<FT66BossPartSnapshot>& OutBossParts) const
{
	OutBossParts.Reset();
	for (const FT66BossPartRuntimeState& Part : BossPartStates)
	{
		FT66BossPartSnapshot& Snapshot = OutBossParts.AddDefaulted_GetRef();
		Snapshot.PartID = Part.PartID;
		Snapshot.HitZoneType = Part.HitZoneType;
		Snapshot.MaxHP = FMath::Max(1, Part.MaxHP);
		Snapshot.CurrentHP = FMath::Clamp(Part.CurrentHP, 0, Snapshot.MaxHP);
	}
}

void AT66BossBase::PushBossPartStateToRunState() const
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
	{
		TArray<FT66BossPartSnapshot> Snapshots;
		BuildBossPartSnapshots(Snapshots);
		RunState->SetBossActiveWithParts(BossID, Snapshots);
	}
}

bool AT66BossBase::RestoreBossPartStateFromRunState()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState || !RunState->GetBossActive() || RunState->GetActiveBossID() != BossID)
	{
		return false;
	}

	const TArray<FT66BossPartSnapshot>& SavedParts = RunState->GetBossPartSnapshots();
	if (SavedParts.Num() <= 0)
	{
		return false;
	}

	TMap<FName, FT66BossPartSnapshot> SavedById;
	for (const FT66BossPartSnapshot& Part : SavedParts)
	{
		SavedById.Add(Part.PartID, Part);
	}

	bool bAppliedAnyState = false;
	CurrentHP = 0;
	for (FT66BossPartRuntimeState& Part : BossPartStates)
	{
		if (const FT66BossPartSnapshot* SavedPart = SavedById.Find(Part.PartID))
		{
			Part.CurrentHP = FMath::Clamp(SavedPart->CurrentHP, 0, FMath::Max(1, Part.MaxHP));
			bAppliedAnyState = true;
		}
		else
		{
			Part.CurrentHP = Part.MaxHP;
		}

		CurrentHP += Part.CurrentHP;
	}

	if (!bAppliedAnyState)
	{
		CurrentHP = MaxHP;
		return false;
	}

	CurrentHP = FMath::Clamp(CurrentHP, 0, MaxHP);
	PushBossPartStateToRunState();
	return true;
}

int32 AT66BossBase::FindFallbackBossPartIndex() const
{
	for (int32 Index = 0; Index < BossPartStates.Num(); ++Index)
	{
		const FT66BossPartRuntimeState& Part = BossPartStates[Index];
		if (Part.CurrentHP > 0 && Part.ZoneComponent && Part.ZoneComponent->bTargetable)
		{
			return Index;
		}
	}

	for (int32 Index = 0; Index < BossPartStates.Num(); ++Index)
	{
		const FT66BossPartRuntimeState& Part = BossPartStates[Index];
		if (Part.CurrentHP > 0)
		{
			return Index;
		}
	}

	return BossPartStates.Num() > 0 ? 0 : INDEX_NONE;
}

int32 AT66BossBase::ResolveBossPartIndex(const UPrimitiveComponent* HitComponent, const ET66HitZoneType PreferredZone, const FName PreferredPartID) const
{
	if (BossPartStates.Num() <= 0)
	{
		return INDEX_NONE;
	}

	if (PreferredPartID != NAME_None)
	{
		for (int32 Index = 0; Index < BossPartStates.Num(); ++Index)
		{
			const FT66BossPartRuntimeState& Part = BossPartStates[Index];
			if (Part.PartID == PreferredPartID && Part.CurrentHP > 0)
			{
				return Index;
			}
		}
	}

	if (HitComponent)
	{
		for (int32 Index = 0; Index < BossPartStates.Num(); ++Index)
		{
			const FT66BossPartRuntimeState& Part = BossPartStates[Index];
			if (Part.ZoneComponent == HitComponent && Part.CurrentHP > 0)
			{
				return Index;
			}
		}
	}

	if (PreferredZone != ET66HitZoneType::None)
	{
		for (int32 Index = 0; Index < BossPartStates.Num(); ++Index)
		{
			const FT66BossPartRuntimeState& Part = BossPartStates[Index];
			if (Part.HitZoneType == PreferredZone && Part.CurrentHP > 0)
			{
				return Index;
			}
		}
	}

	return FindFallbackBossPartIndex();
}

float AT66BossBase::GetBossPartDamageMultiplier(const int32 PartIndex) const
{
	if (!BossPartStates.IsValidIndex(PartIndex))
	{
		return 1.f;
	}

	return FMath::Max(0.1f, BossPartStates[PartIndex].DamageMultiplier);
}

void AT66BossBase::RebuildBossPartState(const bool bPreserveCurrentPercent)
{
	EnsureDefaultBossPartDefinitions();

	TMap<FName, float> ExistingHealthPctByPart;
	if (bPreserveCurrentPercent)
	{
		for (const FT66BossPartRuntimeState& Part : BossPartStates)
		{
			const int32 PartMaxHP = FMath::Max(1, Part.MaxHP);
			ExistingHealthPctByPart.Add(Part.PartID, FMath::Clamp(static_cast<float>(Part.CurrentHP) / static_cast<float>(PartMaxHP), 0.f, 1.f));
		}
	}

	for (FT66BossPartRuntimeState& Part : BossPartStates)
	{
		if (Part.ZoneComponent)
		{
			Part.ZoneComponent->DestroyComponent();
		}
	}
	BossPartStates.Reset();

	float TotalWeight = 0.f;
	for (const FT66BossPartDefinition& Definition : BossPartDefinitions)
	{
		TotalWeight += FMath::Max(0.01f, Definition.HPWeight);
	}
	TotalWeight = FMath::Max(0.01f, TotalWeight);

	int32 RemainingMaxHP = FMath::Max(1, MaxHP);
	for (int32 Index = 0; Index < BossPartDefinitions.Num(); ++Index)
	{
		const FT66BossPartDefinition& Definition = BossPartDefinitions[Index];
		const int32 RemainingParts = BossPartDefinitions.Num() - Index;
		int32 PartMaxHP = 1;
		if (Index == BossPartDefinitions.Num() - 1)
		{
			PartMaxHP = FMath::Max(1, RemainingMaxHP);
		}
		else
		{
			PartMaxHP = FMath::Clamp(
				FMath::RoundToInt(static_cast<float>(MaxHP) * FMath::Max(0.01f, Definition.HPWeight) / TotalWeight),
				1,
				FMath::Max(1, RemainingMaxHP - (RemainingParts - 1)));
		}
		RemainingMaxHP = FMath::Max(0, RemainingMaxHP - PartMaxHP);

		FT66BossPartRuntimeState& RuntimePart = BossPartStates.AddDefaulted_GetRef();
		RuntimePart.PartID = Definition.PartID.IsNone() ? T66GetDefaultBossPartID(Definition.HitZoneType) : Definition.PartID;
		RuntimePart.HitZoneType = Definition.HitZoneType;
		RuntimePart.DamageMultiplier = FMath::Max(0.1f, Definition.DamageMultiplier);
		RuntimePart.MaxHP = PartMaxHP;

		const float PreviousHealthPct = ExistingHealthPctByPart.Contains(RuntimePart.PartID)
			? ExistingHealthPctByPart[RuntimePart.PartID]
			: 1.f;
		RuntimePart.CurrentHP = bPreserveCurrentPercent
			? FMath::Clamp(FMath::RoundToInt(static_cast<float>(PartMaxHP) * PreviousHealthPct), 0, PartMaxHP)
			: PartMaxHP;

		const FName ComponentName = MakeUniqueObjectName(this, UT66CombatHitZoneComponent::StaticClass(), RuntimePart.PartID);
		UT66CombatHitZoneComponent* Zone = NewObject<UT66CombatHitZoneComponent>(this, ComponentName);
		if (Zone)
		{
			Zone->SetupAttachment(RootComponent);
			Zone->RegisterComponent();
			AddInstanceComponent(Zone);
			Zone->SetRelativeLocation(Definition.RelativeLocation);
			Zone->SetSphereRadius(FMath::Max(1.f, Definition.Radius));
			Zone->HitZoneType = RuntimePart.HitZoneType;
			Zone->HitZoneName = RuntimePart.PartID;
			Zone->DamageMultiplier = RuntimePart.DamageMultiplier;
			Zone->bTargetable = Definition.bTargetable;
			RuntimePart.ZoneComponent = Zone;
		}
	}

	CurrentHP = 0;
	for (const FT66BossPartRuntimeState& Part : BossPartStates)
	{
		CurrentHP += Part.CurrentHP;
	}
	CurrentHP = FMath::Clamp(CurrentHP, 0, MaxHP);
	if (!bAwakened && !bPreserveCurrentPercent)
	{
		CurrentHP = 0;
	}
	RefreshCombatHitZoneState();
}

bool AT66BossBase::SupportsCombatHitZones() const
{
	return bUsesBossPartHitZones && BossPartStates.Num() > 0;
}

FT66CombatTargetHandle AT66BossBase::ResolveCombatTargetHandle(const UPrimitiveComponent* HitComponent, const ET66HitZoneType PreferredZone) const
{
	FT66CombatTargetHandle Handle;
	Handle.Actor = const_cast<AT66BossBase*>(this);

	const int32 PartIndex = ResolveBossPartIndex(HitComponent, PreferredZone);
	if (!BossPartStates.IsValidIndex(PartIndex))
	{
		Handle.HitZoneType = PreferredZone == ET66HitZoneType::None ? ET66HitZoneType::Core : PreferredZone;
		Handle.HitZoneName = T66GetDefaultBossPartID(Handle.HitZoneType);
		Handle.AimPoint = GetActorLocation();
		return Handle;
	}

	const FT66BossPartRuntimeState& Part = BossPartStates[PartIndex];
	Handle.HitZoneType = Part.HitZoneType;
	Handle.HitZoneName = Part.PartID;
	Handle.HitComponent = Part.ZoneComponent;
	Handle.AimPoint = Part.ZoneComponent ? Part.ZoneComponent->GetComponentLocation() : GetActorLocation();
	return Handle;
}

FVector AT66BossBase::GetAimPointForHitZone(const ET66HitZoneType HitZoneType) const
{
	return ResolveCombatTargetHandle(nullptr, HitZoneType).AimPoint;
}

void AT66BossBase::InitializeBoss(const FBossData& BossData)
{
	BossID = BossData.BossID;
	// Standardized HP baseline: bosses are 1000+ HP.
	MaxHP = FMath::Max(1000, BossData.MaxHP);
	AwakenDistance = BossData.AwakenDistance;
	FireIntervalSeconds = BossData.FireIntervalSeconds;
	ProjectileSpeed = BossData.ProjectileSpeed;
	ProjectileDamageHearts = BossData.ProjectileDamageHearts;
	PointValue = FMath::Max(0, BossData.PointValue);
	const ET66BossPartProfile ResolvedPartProfile =
		(BossData.BossPartProfile != ET66BossPartProfile::UseActorDefault)
		? BossData.BossPartProfile
		: T66ResolveLegacyBossPartProfile(BossData.BossID);
	AssignBossPartDefinitionsForProfile(ResolvedPartProfile);

	// Conservative default if DT doesn't specify a score: tie to HP scale.
	if (PointValue <= 0)
	{
		PointValue = FMath::Clamp(MaxHP / 10, 100, 5000);
	}

	BaseMaxHP = MaxHP;
	BaseProjectileDamageHearts = ProjectileDamageHearts;
	bBaseTuningInitialized = true;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = BossData.MoveSpeed;
		BaseMoveSpeed = BossData.MoveSpeed;
	}
	if (VisualMesh)
	{
		if (UMaterialInstanceDynamic* Mat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0))
		{
			Mat->SetVectorParameterValue(TEXT("BaseColor"), BossData.PlaceholderColor);
		}
	}

	// All stage bosses use the same "Boss" mesh from DT_CharacterVisuals (BossID kept for stats; visual is shared).
	static const FName BossVisualID(TEXT("Boss"));
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
		{
			const bool bApplied = Visuals->ApplyCharacterVisual(BossVisualID, GetMesh(), VisualMesh, true);
			if (!bApplied && GetMesh())
			{
				GetMesh()->SetVisibility(false, true);
			}
			else if (bApplied && GetMesh())
			{
				if (UCapsuleComponent* Capsule = GetCapsuleComponent())
				{
					FVector MeshLocation = GetMesh()->GetRelativeLocation();
					MeshLocation.Z += Capsule->GetScaledCapsuleHalfHeight();
					GetMesh()->SetRelativeLocation(MeshLocation);
				}
			}
		}
	}

	// Apply current run difficulty (boss is usually dormant until awaken).
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			ApplyDifficultyScalar(RunState->GetDifficultyScalar());
		}
	}
	else
	{
		RebuildBossPartState(false);
	}
}

void AT66BossBase::ApplyDifficultyScalar(float Scalar)
{
	if (!bBaseTuningInitialized)
	{
		BaseMaxHP = MaxHP;
		BaseProjectileDamageHearts = ProjectileDamageHearts;
		bBaseTuningInitialized = true;
	}

	const float ClampedScalar = FMath::Clamp(Scalar, 1.0f, 99.0f);
	MaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseMaxHP) * ClampedScalar));
	ProjectileDamageHearts = FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseProjectileDamageHearts) * ClampedScalar));
	RebuildBossPartState(bAwakened);

	// If boss is active, rescale the RunState boss bar max HP while preserving percent.
	if (bAwakened)
	{
		PushBossPartStateToRunState();
	}
}

void AT66BossBase::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->RegisterBoss(this);
		}
	}

	bAwakened = false;
	CurrentHP = 0;
	ArmorDebuffAmount = 0.f;
	ArmorDebuffSecondsRemaining = 0.f;
	ConfusionSecondsRemaining = 0.f;
	MoveSlowMultiplier = 1.f;
	MoveSlowSecondsRemaining = 0.f;
	ForcedRunAwaySecondsRemaining = 0.f;
	StunSecondsRemaining = 0.f;
	RootSecondsRemaining = 0.f;
	FreezeSecondsRemaining = 0.f;
	CachedWanderDir = FVector::ZeroVector;
	WanderDirRefreshAccum = 0.f;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		BaseMoveSpeed = Move->MaxWalkSpeed;
	}

	RebuildBossPartState(false);
}

void AT66BossBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->UnregisterBoss(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AT66BossBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	APawn* PlayerPawn = ResolvePlayerPawn();
	if (!PlayerPawn) return;

	const FVector MyLoc = GetActorLocation();
	const FVector PlayerLoc = PlayerPawn->GetActorLocation();

	if (!bAwakened)
	{
		if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				if (RunState->GetBossActive() && RunState->GetActiveBossID() == BossID && RunState->GetBossCurrentHP() > 0)
				{
					Awaken();
				}
			}
		}

		if (bAwakened)
		{
			RestoreBossPartStateFromRunState();
		}

		if (FVector::DistSquared(MyLoc, PlayerLoc) <= AwakenDistance * AwakenDistance)
		{
			Awaken();
		}
		return;
	}

	// Decay armor debuff
	if (ArmorDebuffSecondsRemaining > 0.f)
	{
		ArmorDebuffSecondsRemaining -= DeltaSeconds;
		if (ArmorDebuffSecondsRemaining <= 0.f)
		{
			ArmorDebuffSecondsRemaining = 0.f;
			ArmorDebuffAmount = 0.f;
		}
	}

	if (MoveSlowSecondsRemaining > 0.f)
	{
		MoveSlowSecondsRemaining = FMath::Max(0.f, MoveSlowSecondsRemaining - DeltaSeconds);
		if (MoveSlowSecondsRemaining <= 0.f)
		{
			MoveSlowMultiplier = 1.f;
		}
	}

	if (ForcedRunAwaySecondsRemaining > 0.f)
	{
		ForcedRunAwaySecondsRemaining = FMath::Max(0.f, ForcedRunAwaySecondsRemaining - DeltaSeconds);
	}

	if (ConfusionSecondsRemaining > 0.f)
	{
		ConfusionSecondsRemaining = FMath::Max(0.f, ConfusionSecondsRemaining - DeltaSeconds);
	}

	if (StunSecondsRemaining > 0.f)
	{
		StunSecondsRemaining = FMath::Max(0.f, StunSecondsRemaining - DeltaSeconds);
	}

	if (RootSecondsRemaining > 0.f)
	{
		RootSecondsRemaining = FMath::Max(0.f, RootSecondsRemaining - DeltaSeconds);
	}

	if (FreezeSecondsRemaining > 0.f)
	{
		FreezeSecondsRemaining = FMath::Max(0.f, FreezeSecondsRemaining - DeltaSeconds);
	}

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		const float ControlMultiplier = (FreezeSecondsRemaining > 0.f) ? 0.f : MoveSlowMultiplier;
		Move->MaxWalkSpeed = BaseMoveSpeed * ControlMultiplier;
		if (FreezeSecondsRemaining > 0.f || StunSecondsRemaining > 0.f)
		{
			Move->StopMovementImmediately();
			return;
		}
		if (RootSecondsRemaining > 0.f)
		{
			Move->StopMovementImmediately();
			return;
		}
	}

	const bool bRunAway = ForcedRunAwaySecondsRemaining > 0.f;
	if (ConfusionSecondsRemaining > 0.f)
	{
		WanderDirRefreshAccum += DeltaSeconds;
		if (WanderDirRefreshAccum >= 0.9f || CachedWanderDir.IsNearlyZero())
		{
			WanderDirRefreshAccum = 0.f;
			CachedWanderDir = FVector(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), 0.f).GetSafeNormal();
		}
		if (!CachedWanderDir.IsNearlyZero())
		{
			AddMovementInput(CachedWanderDir, 0.45f);
		}
		return;
	}

	// Chase player
	FVector ToPlayer = PlayerLoc - MyLoc;
	ToPlayer.Z = 0.f;
	const float Len = ToPlayer.Size();
	if (Len > 10.f)
	{
		ToPlayer /= Len;
		AddMovementInput(bRunAway ? -ToPlayer : ToPlayer, 1.f);
	}
}

void AT66BossBase::ApplyArmorDebuff(float ReductionAmount, float DurationSeconds)
{
	const float Amt = FMath::Clamp(ReductionAmount, 0.f, 1.f);
	const float Dur = FMath::Clamp(DurationSeconds, 0.f, 30.f);
	if (Amt <= 0.f || Dur <= 0.f) return;
	ArmorDebuffAmount = FMath::Max(ArmorDebuffAmount, Amt);
	ArmorDebuffSecondsRemaining = FMath::Max(ArmorDebuffSecondsRemaining, Dur);
}

void AT66BossBase::ApplyConfusion(float DurationSeconds)
{
	const float Dur = FMath::Clamp(DurationSeconds * 0.6f, 0.f, 3.f);
	if (Dur <= 0.f || CurrentHP <= 0)
	{
		return;
	}

	ConfusionSecondsRemaining = FMath::Max(ConfusionSecondsRemaining, Dur);
}

void AT66BossBase::ApplyMoveSlow(float SpeedMultiplier, float DurationSeconds)
{
	const float Mult = FMath::Clamp(SpeedMultiplier, 0.25f, 1.f);
	const float Dur = FMath::Clamp(DurationSeconds * 0.75f, 0.f, 8.f);
	if (Dur <= 0.f || CurrentHP <= 0)
	{
		return;
	}

	MoveSlowMultiplier = FMath::Min(MoveSlowMultiplier, Mult);
	MoveSlowSecondsRemaining = FMath::Max(MoveSlowSecondsRemaining, Dur);
}

void AT66BossBase::ApplyForcedRunAway(float DurationSeconds)
{
	const float Dur = FMath::Clamp(DurationSeconds * 0.55f, 0.f, 3.f);
	if (Dur <= 0.f || CurrentHP <= 0)
	{
		return;
	}

	ForcedRunAwaySecondsRemaining = FMath::Max(ForcedRunAwaySecondsRemaining, Dur);
}

void AT66BossBase::ApplyStun(float DurationSeconds)
{
	const float Dur = FMath::Clamp(DurationSeconds * 0.55f, 0.f, 1.5f);
	if (Dur <= 0.f || CurrentHP <= 0)
	{
		return;
	}

	StunSecondsRemaining = FMath::Max(StunSecondsRemaining, Dur);
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
	}
}

void AT66BossBase::ApplyRoot(float DurationSeconds)
{
	const float Dur = FMath::Clamp(DurationSeconds * 0.65f, 0.f, 2.5f);
	if (Dur <= 0.f || CurrentHP <= 0)
	{
		return;
	}

	RootSecondsRemaining = FMath::Max(RootSecondsRemaining, Dur);
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
	}
}

void AT66BossBase::ApplyFreeze(float DurationSeconds)
{
	const float Dur = FMath::Clamp(DurationSeconds * 0.5f, 0.f, 1.5f);
	if (Dur <= 0.f || CurrentHP <= 0)
	{
		return;
	}

	FreezeSecondsRemaining = FMath::Max(FreezeSecondsRemaining, Dur);
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
	}
}

void AT66BossBase::ApplyPullTowards(const FVector& PullOrigin, float Distance)
{
	if (CurrentHP <= 0)
	{
		return;
	}

	T66ApplyBossDisplacement(this, PullOrigin, Distance * 0.5f, true);
}

void AT66BossBase::ApplyPushAwayFrom(const FVector& PushOrigin, float Distance)
{
	if (CurrentHP <= 0)
	{
		return;
	}

	T66ApplyBossDisplacement(this, PushOrigin, Distance * 0.45f, false);
}

void AT66BossBase::Awaken()
{
	if (bAwakened) return;
	bAwakened = true;
	CurrentHP = MaxHP;
	RebuildBossPartState(false);

	const bool bRestoredExistingState = RestoreBossPartStateFromRunState();
	if (!bRestoredExistingState)
	{
		PushBossPartStateToRunState();
	}

	// Start firing projectiles
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(FireTimerHandle, this, &AT66BossBase::FireAtPlayer, FireIntervalSeconds, true, 0.25f);

		if (GroundAOEIntervalSeconds > 0.f)
		{
			World->GetTimerManager().SetTimer(AOETimerHandle, this, &AT66BossBase::SpawnGroundAOE, GroundAOEIntervalSeconds, true, GroundAOEIntervalSeconds * 0.5f);
		}
	}
}

void AT66BossBase::FireAtPlayer()
{
	if (!bAwakened || CurrentHP <= 0 || StunSecondsRemaining > 0.f || FreezeSecondsRemaining > 0.f) return;

	UWorld* World = GetWorld();
	if (!World) return;

	APawn* PlayerPawn = ResolvePlayerPawn();
	if (!PlayerPawn) return;

	const FVector SpawnLoc = GetActorLocation() + FVector(0.f, 0.f, 80.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66BossProjectile* Proj = World->SpawnActor<AT66BossProjectile>(AT66BossProjectile::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (Proj)
	{
		Proj->DamageHearts = ProjectileDamageHearts;
		Proj->SetTargetLocation(PlayerPawn->GetActorLocation(), ProjectileSpeed);
	}
}

bool AT66BossBase::TakeDamageFromHeroHit(int32 DamageAmount, FName DamageSourceID, FName EventType)
{
	return TakeDamageFromHeroHitZone(DamageAmount, ResolveCombatTargetHandle(nullptr, ET66HitZoneType::Body), DamageSourceID, EventType);
}

bool AT66BossBase::TakeDamageFromHeroHitZone(int32 DamageAmount, const FT66CombatTargetHandle& TargetHandle, FName DamageSourceID, FName EventType)
{
	const FName SourceID = DamageSourceID.IsNone() ? UT66DamageLogSubsystem::SourceID_AutoAttack : DamageSourceID;
	if (!bAwakened && SourceID == UT66DamageLogSubsystem::SourceID_Ultimate)
	{
		Awaken();
	}
	if (!bAwakened || CurrentHP <= 0 || DamageAmount <= 0)
	{
		return false;
	}

	const int32 PartIndex = ResolveBossPartIndex(TargetHandle.HitComponent.Get(), TargetHandle.HitZoneType, TargetHandle.HitZoneName);
	if (!BossPartStates.IsValidIndex(PartIndex))
	{
		return false;
	}

	const float EffectiveArmor = GetEffectiveArmor();
	const int32 PartAdjustedDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(DamageAmount) * GetBossPartDamageMultiplier(PartIndex)));
	const int32 ReducedDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(PartAdjustedDamage) * (1.f - EffectiveArmor)));

	FName ResolvedEventType = EventType;
	if (BossPartStates[PartIndex].HitZoneType == ET66HitZoneType::Head && ResolvedEventType.IsNone())
	{
		ResolvedEventType = UT66FloatingCombatTextSubsystem::EventType_Headshot;
	}

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (UT66DamageLogSubsystem* DamageLog = GI ? GI->GetSubsystem<UT66DamageLogSubsystem>() : nullptr)
	{
		DamageLog->RecordDamageDealt(SourceID, ReducedDamage);
	}
	if (UT66FloatingCombatTextSubsystem* FloatingText = GI ? GI->GetSubsystem<UT66FloatingCombatTextSubsystem>() : nullptr)
	{
		FloatingText->ShowDamageNumber(this, ReducedDamage, ResolvedEventType);
	}

	FT66BossPartRuntimeState& Part = BossPartStates[PartIndex];
	Part.CurrentHP = FMath::Max(0, Part.CurrentHP - ReducedDamage);
	CurrentHP = 0;
	for (const FT66BossPartRuntimeState& RuntimePart : BossPartStates)
	{
		CurrentHP += RuntimePart.CurrentHP;
	}
	CurrentHP = FMath::Clamp(CurrentHP, 0, MaxHP);
	PushBossPartStateToRunState();

	if (CurrentHP <= 0)
	{
		Die();
		return true;
	}

	return false;
}

float AT66BossBase::GetEffectiveArmor() const
{
	return FMath::Clamp(Armor - ArmorDebuffAmount, -0.5f, 0.95f);
}

void AT66BossBase::SpawnGroundAOE()
{
	if (!bAwakened || CurrentHP <= 0 || StunSecondsRemaining > 0.f || FreezeSecondsRemaining > 0.f) return;

	UWorld* World = GetWorld();
	if (!World) return;

	APawn* PlayerPawn = ResolvePlayerPawn();
	if (!PlayerPawn) return;

	FVector TargetLoc = PlayerPawn->GetActorLocation();
	FHitResult Hit;
	if (World->LineTraceSingleByChannel(Hit, TargetLoc + FVector(0.f, 0.f, 500.f), TargetLoc - FVector(0.f, 0.f, 1000.f), ECC_WorldStatic))
	{
		TargetLoc = Hit.ImpactPoint + FVector(0.f, 0.f, 5.f);
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66BossGroundAOE* AOE = World->SpawnActor<AT66BossGroundAOE>(AT66BossGroundAOE::StaticClass(), TargetLoc, FRotator::ZeroRotator, Params);
	if (AOE)
	{
		AOE->Radius = GroundAOERadius;
		AOE->WarningDurationSeconds = GroundAOEWarningSeconds;

		UGameInstance* GI = World->GetGameInstance();
		UT66RunStateSubsystem* RS = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		const int32 Stage = RS ? FMath::Max(1, RS->GetCurrentStage()) : 1;
		AOE->DamageHP = FMath::Max(10, FMath::RoundToInt(static_cast<float>(GroundAOEBaseDamageHP) * FMath::Pow(1.25f, static_cast<float>(Stage - 1))));
	}
}

APawn* AT66BossBase::ResolvePlayerPawn()
{
	APawn* PlayerPawn = T66ResolveClosestBossTargetPawn(this);
	CachedPlayerPawn = PlayerPawn;
	return PlayerPawn;
}

void AT66BossBase::Die()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(FireTimerHandle);
		World->GetTimerManager().ClearTimer(AOETimerHandle);
		UT66CombatComponent::SpawnDeathBurstAtLocation(World, GetActorLocation(), 32, 120.f);
	}

	if (AT66GameMode* GM = World ? World->GetAuthGameMode<AT66GameMode>() : nullptr)
	{
		GM->HandleBossDefeated(this);
	}

	Destroy();
}

