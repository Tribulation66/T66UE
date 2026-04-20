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

	FVector T66RotatePlanarVector(const FVector& Direction, const float Degrees)
	{
		return FRotator(0.f, Degrees, 0.f).RotateVector(Direction).GetSafeNormal();
	}

	FLinearColor T66MakeAttackSecondaryColor(const FLinearColor& InPrimary)
	{
		return FLinearColor(
			FMath::Clamp(InPrimary.R * 0.55f + 0.38f, 0.f, 1.f),
			FMath::Clamp(InPrimary.G * 0.35f + 0.46f, 0.f, 1.f),
			FMath::Clamp(InPrimary.B * 0.25f + 0.10f, 0.f, 1.f),
			1.f);
	}

	FVector T66ResolvePlanarRightVector(const FVector& Forward)
	{
		const FVector PlanarForward = FVector(Forward.X, Forward.Y, 0.f).GetSafeNormal();
		return FVector(-PlanarForward.Y, PlanarForward.X, 0.f).GetSafeNormal();
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

	AttackPrimaryColor = FLinearColor(0.95f, 0.16f, 0.12f, 1.f);
	AttackSecondaryColor = T66MakeAttackSecondaryColor(AttackPrimaryColor);

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

void AT66BossBase::ConfigureAttackProfileFromBossPartProfile(const ET66BossPartProfile InProfile)
{
	switch (InProfile)
	{
	case ET66BossPartProfile::Sharpshooter:
		AttackProfile = ET66BossAttackProfile::Sharpshooter;
		break;
	case ET66BossPartProfile::Juggernaut:
		AttackProfile = ET66BossAttackProfile::Juggernaut;
		break;
	case ET66BossPartProfile::Duelist:
		AttackProfile = ET66BossAttackProfile::Duelist;
		break;
	case ET66BossPartProfile::HumanoidBalanced:
	case ET66BossPartProfile::UseActorDefault:
	default:
		AttackProfile = ET66BossAttackProfile::Balanced;
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
	ConfigureAttackProfileFromBossPartProfile(ResolvedPartProfile);
	AttackPrimaryColor = BossData.PlaceholderColor;
	AttackPrimaryColor.A = 1.f;
	AttackSecondaryColor = T66MakeAttackSecondaryColor(AttackPrimaryColor);

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

	// Use the same live mob visual as the in-game Cow enemy so boss testing matches
	// the actual enemy asset path rather than a static showcase prop.
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
		{
			const bool bApplied = Visuals->ApplyCharacterVisual(FName(TEXT("Cow")), GetMesh(), VisualMesh, true);
			if (USkeletalMeshComponent* SkelMesh = GetMesh())
			{
				if (bApplied)
				{
					SkelMesh->SetRelativeScale3D(SkelMesh->GetRelativeScale3D() * 3.0f);
				}
				else
				{
					SkelMesh->SetVisibility(false, true);
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

void AT66BossBase::RefreshRunStateBossState() const
{
	PushBossPartStateToRunState();
}

float AT66BossBase::GetHealthPercent() const
{
	if (MaxHP <= 0)
	{
		return 1.f;
	}

	if (!bAwakened && CurrentHP <= 0)
	{
		return 1.f;
	}

	return FMath::Clamp(static_cast<float>(CurrentHP) / static_cast<float>(MaxHP), 0.f, 1.f);
}

int32 AT66BossBase::GetAttackPhaseIndex() const
{
	const float HealthPercent = GetHealthPercent();
	if (HealthPercent <= 0.34f)
	{
		return 2;
	}
	if (HealthPercent <= 0.67f)
	{
		return 1;
	}
	return 0;
}

FVector AT66BossBase::ResolveGroundLocation(const FVector& PreferredLocation) const
{
	FVector TargetLoc = PreferredLocation;
	if (const UWorld* World = GetWorld())
	{
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(
			Hit,
			TargetLoc + FVector(0.f, 0.f, 500.f),
			TargetLoc - FVector(0.f, 0.f, 1000.f),
			ECC_WorldStatic))
		{
			TargetLoc = Hit.ImpactPoint + FVector(0.f, 0.f, 5.f);
		}
	}
	return TargetLoc;
}

void AT66BossBase::ClearPendingAttackTimers()
{
	if (UWorld* World = GetWorld())
	{
		for (FTimerHandle& Handle : PendingAttackTimerHandles)
		{
			World->GetTimerManager().ClearTimer(Handle);
		}
	}

	PendingAttackTimerHandles.Reset();
}

void AT66BossBase::QueueTimedAttackLambda(FTimerDelegate&& Delegate, const float DelaySeconds)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	PendingAttackTimerHandles.RemoveAllSwap([World](const FTimerHandle& Handle)
	{
		return !Handle.IsValid() || !World->GetTimerManager().TimerExists(Handle);
	});

	FTimerHandle& Handle = PendingAttackTimerHandles.AddDefaulted_GetRef();
	World->GetTimerManager().SetTimer(Handle, MoveTemp(Delegate), FMath::Max(0.001f, DelaySeconds), false);
}

void AT66BossBase::SpawnProjectileInDirection(const FVector& Direction, const float SpeedScale, const FVector& SpawnOffset, const bool bUseSecondaryTint)
{
	if (!bAwakened || CurrentHP <= 0 || StunSecondsRemaining > 0.f || FreezeSecondsRemaining > 0.f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector ShotDirection = Direction.GetSafeNormal();
	if (ShotDirection.IsNearlyZero())
	{
		return;
	}

	const FVector SpawnLoc = GetActorLocation() + FVector(0.f, 0.f, 84.f) + SpawnOffset;
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AT66BossProjectile* Proj = World->SpawnActor<AT66BossProjectile>(AT66BossProjectile::StaticClass(), SpawnLoc, ShotDirection.Rotation(), SpawnParams))
	{
		Proj->DamageHearts = ProjectileDamageHearts;
		Proj->ConfigureVisualStyle(AttackProfile, AttackPrimaryColor, AttackSecondaryColor, bUseSecondaryTint);
		Proj->SetTargetLocation(SpawnLoc + ShotDirection * 1000.f, ProjectileSpeed * FMath::Max(0.35f, SpeedScale));
	}
}

void AT66BossBase::QueueProjectileShotDirection(const FVector& Direction, const float DelaySeconds, const float SpeedScale, const FVector& SpawnOffset, const bool bUseSecondaryTint)
{
	const FVector ShotDirection = Direction.GetSafeNormal();
	if (ShotDirection.IsNearlyZero())
	{
		return;
	}

	TWeakObjectPtr<AT66BossBase> WeakThis(this);
	QueueTimedAttackLambda(
		FTimerDelegate::CreateLambda([WeakThis, ShotDirection, SpeedScale, SpawnOffset, bUseSecondaryTint]()
		{
			if (!WeakThis.IsValid())
			{
				return;
			}

			WeakThis->SpawnProjectileInDirection(ShotDirection, SpeedScale, SpawnOffset, bUseSecondaryTint);
		}),
		DelaySeconds);
}

void AT66BossBase::QueueProjectileShotTowards(const FVector& TargetLocation, const float DelaySeconds, const float YawOffsetDegrees, const float SpeedScale, const FVector& SpawnOffset, const bool bUseSecondaryTint)
{
	FVector Direction = TargetLocation - (GetActorLocation() + FVector(0.f, 0.f, 84.f));
	Direction.Z = 0.f;
	Direction = Direction.GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = GetActorForwardVector();
	}

	QueueProjectileShotDirection(T66RotatePlanarVector(Direction, YawOffsetDegrees), DelaySeconds, SpeedScale, SpawnOffset, bUseSecondaryTint);
}

void AT66BossBase::QueueProjectileFanBurst(
	const FVector& TargetLocation,
	const int32 ShotCount,
	const float SpreadDegrees,
	const float DelayStepSeconds,
	const float SpeedScale,
	const float InitialDelaySeconds,
	const float SideOffsetDistance,
	const bool bUseSecondaryTint)
{
	if (ShotCount <= 0)
	{
		return;
	}

	const FVector BaseDirection = (TargetLocation - GetActorLocation()).GetSafeNormal();
	const FVector Right = T66ResolvePlanarRightVector(BaseDirection);
	const float StartYaw = (ShotCount > 1) ? (-SpreadDegrees * 0.5f) : 0.f;
	const float StepYaw = (ShotCount > 1) ? (SpreadDegrees / static_cast<float>(ShotCount - 1)) : 0.f;

	for (int32 Index = 0; Index < ShotCount; ++Index)
	{
		const float NormalizedIndex = (ShotCount > 1)
			? (static_cast<float>(Index) / static_cast<float>(ShotCount - 1) - 0.5f) * 2.f
			: 0.f;
		const FVector SpawnOffset = Right * (SideOffsetDistance * NormalizedIndex);
		QueueProjectileShotTowards(
			TargetLocation,
			InitialDelaySeconds + DelayStepSeconds * static_cast<float>(Index),
			StartYaw + StepYaw * static_cast<float>(Index),
			SpeedScale,
			SpawnOffset,
			bUseSecondaryTint && ((Index % 2) == 1));
	}
}

void AT66BossBase::QueueRadialBurst(
	const int32 ShotCount,
	const float DelayStepSeconds,
	const float StartAngleDegrees,
	const float SpeedScale,
	const float InitialDelaySeconds,
	const bool bUseSecondaryTint)
{
	if (ShotCount <= 0)
	{
		return;
	}

	const float StepDegrees = 360.f / static_cast<float>(ShotCount);
	for (int32 Index = 0; Index < ShotCount; ++Index)
	{
		const float AngleDegrees = StartAngleDegrees + StepDegrees * static_cast<float>(Index);
		const FVector Direction = FRotator(0.f, AngleDegrees, 0.f).Vector();
		QueueProjectileShotDirection(
			Direction,
			InitialDelaySeconds + DelayStepSeconds * static_cast<float>(Index),
			SpeedScale,
			FVector::ZeroVector,
			bUseSecondaryTint && ((Index % 2) == 1));
	}
}

void AT66BossBase::SpawnGroundAOEAtLocation(const FVector& WorldLocation, const float RadiusScale, const float WarningScale, const bool bUseSecondaryTint)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AT66BossGroundAOE* AOE = World->SpawnActor<AT66BossGroundAOE>(AT66BossGroundAOE::StaticClass(), ResolveGroundLocation(WorldLocation), FRotator::ZeroRotator, Params))
	{
		AOE->Radius = GroundAOERadius * FMath::Max(0.35f, RadiusScale);
		AOE->WarningDurationSeconds = GroundAOEWarningSeconds * FMath::Max(0.55f, WarningScale);
		AOE->ConfigureVisualStyle(AttackProfile, bUseSecondaryTint ? AttackSecondaryColor : AttackPrimaryColor, AttackSecondaryColor);

		UGameInstance* GI = World->GetGameInstance();
		UT66RunStateSubsystem* RS = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		const int32 Stage = RS ? FMath::Max(1, RS->GetCurrentStage()) : 1;
		AOE->DamageHP = FMath::Max(10, FMath::RoundToInt(static_cast<float>(GroundAOEBaseDamageHP) * FMath::Pow(1.25f, static_cast<float>(Stage - 1))));
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
	ClearPendingAttackTimers();

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
		ClearPendingAttackTimers();
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

	ClearPendingAttackTimers();

	const FVector TargetLocation = PlayerPawn->GetActorLocation();
	const FVector PlanarToTarget = (TargetLocation - GetActorLocation()).GetSafeNormal2D();
	const FVector Side = T66ResolvePlanarRightVector(PlanarToTarget.IsNearlyZero() ? GetActorForwardVector() : PlanarToTarget);
	const int32 Phase = GetAttackPhaseIndex();
	const float PhaseScale = 1.f + 0.08f * static_cast<float>(Phase);

	switch (AttackProfile)
	{
	case ET66BossAttackProfile::Sharpshooter:
		QueueProjectileFanBurst(TargetLocation, Phase >= 1 ? 5 : 3, Phase == 0 ? 8.f : 12.f, 0.04f, 1.18f + 0.06f * static_cast<float>(Phase), 0.f, 18.f, true);
		QueueProjectileShotTowards(TargetLocation, 0.16f, 0.f, 1.36f + 0.08f * static_cast<float>(Phase), FVector::ZeroVector, false);
		if (Phase >= 2)
		{
			QueueProjectileShotTowards(TargetLocation, 0.28f, -6.f, 1.24f, FVector::ZeroVector, true);
			QueueProjectileShotTowards(TargetLocation, 0.32f, 6.f, 1.24f, FVector::ZeroVector, true);
		}
		break;

	case ET66BossAttackProfile::Juggernaut:
		QueueProjectileFanBurst(TargetLocation, Phase == 0 ? 5 : 7, 30.f + 4.f * static_cast<float>(Phase), 0.05f, 0.94f + 0.05f * static_cast<float>(Phase), 0.f, 46.f, false);
		QueueRadialBurst(Phase == 0 ? 6 : (Phase == 1 ? 8 : 10), 0.025f, FMath::FRandRange(0.f, 360.f), 0.76f + 0.05f * static_cast<float>(Phase), 0.18f, true);
		break;

	case ET66BossAttackProfile::Duelist:
		QueueProjectileShotTowards(TargetLocation, 0.f, -10.f, 1.12f * PhaseScale, -Side * 52.f, false);
		QueueProjectileShotTowards(TargetLocation, 0.05f, 10.f, 1.12f * PhaseScale, Side * 52.f, true);
		if (Phase >= 1)
		{
			QueueProjectileFanBurst(TargetLocation, 4 + Phase, 20.f, 0.05f, 1.04f + 0.05f * static_cast<float>(Phase), 0.18f, 26.f, true);
		}
		if (Phase >= 2)
		{
			QueueRadialBurst(6, 0.03f, PlanarToTarget.Rotation().Yaw + 30.f, 0.96f, 0.34f, true);
		}
		break;

	case ET66BossAttackProfile::Vendor:
		QueueProjectileFanBurst(TargetLocation, 4 + Phase, 22.f + 4.f * static_cast<float>(Phase), 0.05f, 1.00f + 0.04f * static_cast<float>(Phase), 0.f, 52.f, false);
		QueueProjectileShotTowards(TargetLocation, 0.12f, -18.f, 1.10f, FVector::ZeroVector, true);
		QueueProjectileShotTowards(TargetLocation, 0.17f, 18.f, 1.10f, FVector::ZeroVector, true);
		if (Phase >= 2)
		{
			QueueRadialBurst(8, 0.025f, PlanarToTarget.Rotation().Yaw + 22.5f, 0.90f, 0.30f, false);
		}
		break;

	case ET66BossAttackProfile::Gambler:
		QueueRadialBurst(6 + Phase * 2, 0.02f, FMath::FRandRange(0.f, 360.f), 0.86f + 0.04f * static_cast<float>(Phase), 0.f, true);
		QueueProjectileFanBurst(TargetLocation, 3 + Phase, 16.f + 2.f * static_cast<float>(Phase), 0.05f, 1.10f + 0.05f * static_cast<float>(Phase), 0.16f, 20.f, false);
		if (Phase >= 2)
		{
			QueueProjectileShotTowards(TargetLocation, 0.32f, FMath::FRandRange(-12.f, 12.f), 1.35f, FVector::ZeroVector, true);
		}
		break;

	case ET66BossAttackProfile::Balanced:
	default:
		QueueProjectileFanBurst(TargetLocation, Phase == 0 ? 3 : (Phase == 1 ? 5 : 6), 14.f + 4.f * static_cast<float>(Phase), 0.06f, 1.00f + 0.06f * static_cast<float>(Phase), 0.f, 36.f, false);
		if (Phase >= 1)
		{
			QueueProjectileShotTowards(TargetLocation, 0.22f, 0.f, 1.18f, FVector::ZeroVector, true);
		}
		if (Phase >= 2)
		{
			QueueProjectileFanBurst(TargetLocation, 3, 10.f, 0.05f, 1.24f, 0.34f, 0.f, true);
		}
		break;
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

	const FVector TargetLoc = ResolveGroundLocation(PlayerPawn->GetActorLocation());
	const FVector Forward = (TargetLoc - GetActorLocation()).GetSafeNormal2D();
	const FVector Right = T66ResolvePlanarRightVector(Forward.IsNearlyZero() ? GetActorForwardVector() : Forward);
	const int32 Phase = GetAttackPhaseIndex();

	switch (AttackProfile)
	{
	case ET66BossAttackProfile::Sharpshooter:
		SpawnGroundAOEAtLocation(TargetLoc, 0.82f, 0.90f, false);
		SpawnGroundAOEAtLocation(TargetLoc + Right * GroundAOERadius * 0.95f, 0.72f, 0.82f, true);
		if (Phase >= 1)
		{
			SpawnGroundAOEAtLocation(TargetLoc - Right * GroundAOERadius * 0.95f, 0.72f, 0.82f, true);
		}
		if (Phase >= 2)
		{
			SpawnGroundAOEAtLocation(TargetLoc + Forward * GroundAOERadius * 0.95f, 0.78f, 0.76f, false);
		}
		break;

	case ET66BossAttackProfile::Juggernaut:
		SpawnGroundAOEAtLocation(GetActorLocation(), 1.18f + 0.10f * static_cast<float>(Phase), 1.00f, false);
		if (Phase >= 1)
		{
			SpawnGroundAOEAtLocation(TargetLoc, 1.00f, 0.86f, true);
		}
		if (Phase >= 2)
		{
			SpawnGroundAOEAtLocation(TargetLoc + Right * GroundAOERadius, 0.84f, 0.78f, true);
			SpawnGroundAOEAtLocation(TargetLoc - Right * GroundAOERadius, 0.84f, 0.78f, true);
		}
		break;

	case ET66BossAttackProfile::Duelist:
		SpawnGroundAOEAtLocation(TargetLoc + Right * GroundAOERadius * 0.7f, 0.76f, 0.82f, false);
		SpawnGroundAOEAtLocation(TargetLoc - Right * GroundAOERadius * 0.7f, 0.76f, 0.82f, true);
		if (Phase >= 1)
		{
			SpawnGroundAOEAtLocation(TargetLoc + Forward * GroundAOERadius * 0.82f, 0.72f, 0.76f, true);
		}
		if (Phase >= 2)
		{
			SpawnGroundAOEAtLocation(TargetLoc - Forward * GroundAOERadius * 0.82f, 0.72f, 0.76f, false);
		}
		break;

	case ET66BossAttackProfile::Vendor:
		SpawnGroundAOEAtLocation(TargetLoc, 0.92f, 0.94f, false);
		SpawnGroundAOEAtLocation(TargetLoc + Right * GroundAOERadius * 0.78f, 0.74f, 0.82f, true);
		SpawnGroundAOEAtLocation(TargetLoc - Right * GroundAOERadius * 0.78f, 0.74f, 0.82f, true);
		if (Phase >= 1)
		{
			SpawnGroundAOEAtLocation(TargetLoc + Forward * GroundAOERadius * 0.78f, 0.74f, 0.80f, false);
			SpawnGroundAOEAtLocation(TargetLoc - Forward * GroundAOERadius * 0.78f, 0.74f, 0.80f, false);
		}
		if (Phase >= 2)
		{
			SpawnGroundAOEAtLocation(GetActorLocation(), 1.08f, 0.88f, true);
		}
		break;

	case ET66BossAttackProfile::Gambler:
	{
		const int32 SpotCount = Phase == 0 ? 3 : (Phase == 1 ? 5 : 6);
		const float RandomStart = FMath::FRandRange(0.f, 360.f);
		for (int32 Index = 0; Index < SpotCount; ++Index)
		{
			const float Angle = RandomStart + (360.f / static_cast<float>(SpotCount)) * static_cast<float>(Index);
			const FVector Offset = FRotator(0.f, Angle, 0.f).Vector() * (GroundAOERadius * (Phase >= 2 ? 0.92f : 0.72f));
			SpawnGroundAOEAtLocation(TargetLoc + Offset, 0.70f + 0.05f * static_cast<float>(Phase), 0.82f, (Index % 2) == 1);
		}
		if (Phase >= 2)
		{
			SpawnGroundAOEAtLocation(TargetLoc, 0.88f, 0.76f, true);
		}
		break;
	}

	case ET66BossAttackProfile::Balanced:
	default:
		SpawnGroundAOEAtLocation(TargetLoc, 1.00f, 1.00f, false);
		if (Phase >= 1)
		{
			SpawnGroundAOEAtLocation(TargetLoc + Right * GroundAOERadius * 0.85f, 0.82f, 0.88f, true);
			SpawnGroundAOEAtLocation(TargetLoc - Right * GroundAOERadius * 0.85f, 0.82f, 0.88f, true);
		}
		if (Phase >= 2)
		{
			SpawnGroundAOEAtLocation(TargetLoc + Forward * GroundAOERadius * 0.9f, 0.78f, 0.80f, false);
		}
		break;
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
		ClearPendingAttackTimers();
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

