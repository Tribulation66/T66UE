// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66CombatHitZoneComponent.h"
#include "Gameplay/T66CombatDebugDraw.h"
#include "Gameplay/T66BossAttackTelegraph.h"
#include "Gameplay/T66BossGroundAOE.h"
#include "Gameplay/T66BossHazardSubsystem.h"
#include "Gameplay/T66BossLaneBlockerHazard.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66ProjectileManagerSubsystem.h"
#include "Core/T66AudioSubsystem.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66GameInstance.h"
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

DEFINE_LOG_CATEGORY_STATIC(LogT66BossAttackOwnership, Log, All);
DEFINE_LOG_CATEGORY_STATIC(LogT66BossMovement, Log, All);

namespace
{
	const FName T66BossAttackEvent_Queued(TEXT("Queued"));
	const FName T66BossAttackEvent_Fired(TEXT("Fired"));
	const FName T66BossAttackEvent_Suppressed(TEXT("Suppressed"));
	const FName T66BossAttackEvent_Loaded(TEXT("Loaded"));
	const FName T66BossAttackID_SewerSlimeLobeVolley(TEXT("SewerSlime_LobeVolley"));
	const FName T66BossAttackID_SewerSlimeLaneBlocker(TEXT("SewerSlime_LaneBlocker"));
	const FName T66BossAttackID_SewerSlimeMouthProjectile(TEXT("SewerSlime_MouthProjectile"));
	const FName T66BossAttackID_SewerSlimeMouthSidecar(TEXT("SewerSlime_MouthSidecar"));
	const TCHAR* T66BossAttackIDPrefix_LegacyProjectile = TEXT("LegacyProjectile_");
	const TCHAR* T66BossAttackIDPrefix_LegacyGroundAOE = TEXT("LegacyGroundAOE_");
	const FName T66BossAttackID_LegacyProjectileBalanced(TEXT("LegacyProjectile_Balanced"));
	const FName T66BossAttackID_LegacyProjectileSharpshooter(TEXT("LegacyProjectile_Sharpshooter"));
	const FName T66BossAttackID_LegacyProjectileJuggernaut(TEXT("LegacyProjectile_Juggernaut"));
	const FName T66BossAttackID_LegacyProjectileDuelist(TEXT("LegacyProjectile_Duelist"));
	const FName T66BossAttackID_LegacyProjectileGambler(TEXT("LegacyProjectile_Gambler"));
	const FName T66BossAttackID_LegacyGroundAOEBalanced(TEXT("LegacyGroundAOE_Balanced"));
	const FName T66BossAttackID_LegacyGroundAOESharpshooter(TEXT("LegacyGroundAOE_Sharpshooter"));
	const FName T66BossAttackID_LegacyGroundAOEJuggernaut(TEXT("LegacyGroundAOE_Juggernaut"));
	const FName T66BossAttackID_LegacyGroundAOEDuelist(TEXT("LegacyGroundAOE_Duelist"));
	const FName T66BossAttackID_LegacyGroundAOEGambler(TEXT("LegacyGroundAOE_Gambler"));
	const FName T66BossAttackPattern_SingleShot(TEXT("SingleShot"));
	const FName T66BossAttackPattern_FanBurst(TEXT("FanBurst"));
	const FName T66BossAttackPattern_RadialBurst(TEXT("RadialBurst"));
	const FName T66BossMovementPattern_SimpleChase(TEXT("SimpleChase"));
	const FName T66BossMovementPattern_KeepDistance(TEXT("KeepDistance"));
	const FName T66BossMovementPattern_Orbit(TEXT("Orbit"));
	const FName T66BossMovementPattern_StrafeBurst(TEXT("StrafeBurst"));
	const FName T66BossMovementPattern_RetreatThenCast(TEXT("RetreatThenCast"));
	const FName T66BossMovementPattern_AnchorDuringCast(TEXT("AnchorDuringCast"));
	const FName T66BossMovementPattern_Charge(TEXT("Charge"));
	const FName T66BossMovementMode_FallbackSimpleChase(TEXT("FallbackSimpleChase"));
	const FName T66BossMovementMode_ForcedRunAway(TEXT("ForcedRunAway"));
	const FName T66BossMovementMode_Confusion(TEXT("Confusion"));
	const FName T66BossMovementMode_Rooted(TEXT("Rooted"));
	const FName T66BossMovementMode_FrozenOrStunned(TEXT("FrozenOrStunned"));

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

	bool T66BossAttackIDStartsWith(const FName AttackID, const TCHAR* Prefix)
	{
		return AttackID.ToString().StartsWith(Prefix, ESearchCase::CaseSensitive);
	}

	ET66BossAttackProfile T66ResolveLegacyAttackProfileFromAttackID(
		const FName AttackID,
		const ET66BossAttackProfile FallbackProfile)
	{
		if (AttackID == T66BossAttackID_LegacyProjectileSharpshooter || AttackID == T66BossAttackID_LegacyGroundAOESharpshooter)
		{
			return ET66BossAttackProfile::Sharpshooter;
		}
		if (AttackID == T66BossAttackID_LegacyProjectileJuggernaut || AttackID == T66BossAttackID_LegacyGroundAOEJuggernaut)
		{
			return ET66BossAttackProfile::Juggernaut;
		}
		if (AttackID == T66BossAttackID_LegacyProjectileDuelist || AttackID == T66BossAttackID_LegacyGroundAOEDuelist)
		{
			return ET66BossAttackProfile::Duelist;
		}
		if (AttackID == T66BossAttackID_LegacyProjectileGambler || AttackID == T66BossAttackID_LegacyGroundAOEGambler)
		{
			return ET66BossAttackProfile::Gambler;
		}
		if (AttackID == T66BossAttackID_LegacyProjectileBalanced || AttackID == T66BossAttackID_LegacyGroundAOEBalanced)
		{
			return ET66BossAttackProfile::Balanced;
		}
		return FallbackProfile;
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

	const TCHAR* T66BossAttackProfileAudioSuffix(const ET66BossAttackProfile Profile)
	{
		switch (Profile)
		{
		case ET66BossAttackProfile::Sharpshooter: return TEXT("Sharpshooter");
		case ET66BossAttackProfile::Juggernaut:   return TEXT("Juggernaut");
		case ET66BossAttackProfile::Duelist:      return TEXT("Duelist");
		case ET66BossAttackProfile::Gambler:      return TEXT("Gambler");
		case ET66BossAttackProfile::Balanced:
		default:                                  return TEXT("Balanced");
		}
	}

	bool T66PlayBossProfileAudioEvent(AT66BossBase* Boss, const TCHAR* EventPrefix, const FName FallbackEventID, const FVector& Location)
	{
		if (!Boss || !EventPrefix)
		{
			return false;
		}

		const FName ProfileEventID(*FString::Printf(TEXT("%s.%s"), EventPrefix, T66BossAttackProfileAudioSuffix(Boss->AttackProfile)));
		if (UT66AudioSubsystem::PlayEventFromWorldContext(Boss, ProfileEventID, Location, Boss))
		{
			return true;
		}

		return UT66AudioSubsystem::PlayEventFromWorldContext(Boss, FallbackEventID, Location, Boss);
	}

	TArray<FName> T66ParseRequiredBossPartIDs(const FString& RequiredPartIDs)
	{
		TArray<FName> Result;
		TArray<FString> Tokens;
		RequiredPartIDs.ParseIntoArray(Tokens, TEXT("|"), true);
		if (Tokens.Num() <= 1)
		{
			Tokens.Reset();
			RequiredPartIDs.ParseIntoArray(Tokens, TEXT(","), true);
		}
		if (Tokens.Num() <= 1)
		{
			Tokens.Reset();
			RequiredPartIDs.ParseIntoArray(Tokens, TEXT(";"), true);
		}

		for (FString Token : Tokens)
		{
			Token.TrimStartAndEndInline();
			if (!Token.IsEmpty())
			{
				Result.Add(FName(*Token));
			}
		}
		return Result;
	}

	FString T66MakeBossAttackCounterKey(const FName EventID, const FName AttackID, const FName PartID)
	{
		return FString::Printf(TEXT("%s|%s|%s"), *EventID.ToString(), *AttackID.ToString(), *PartID.ToString());
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

void AT66BossBase::AssignSewerSlimeKingPartDefinitions()
{
	bUsesBossPartHitZones = true;
	BossPartDefinitions.Reset();
	BossPartDefinitions.Reserve(5);

	auto AddSlimePart = [this](
		const TCHAR* PartName,
		const ET66HitZoneType HitZoneType,
		const float HPWeight,
		const float DamageMultiplier,
		const FVector& RelativeLocation,
		const float Radius)
	{
		FT66BossPartDefinition& Part = BossPartDefinitions.AddDefaulted_GetRef();
		Part.PartID = FName(PartName);
		Part.HitZoneType = HitZoneType;
		Part.HPWeight = HPWeight;
		Part.DamageMultiplier = DamageMultiplier;
		Part.RelativeLocation = RelativeLocation;
		Part.Radius = Radius;
		Part.bTargetable = true;
	};

	AddSlimePart(TEXT("LeftLobe"), ET66HitZoneType::LeftArm, 0.16f, 1.10f, FVector(35.f, -165.f, 136.f), 118.f);
	AddSlimePart(TEXT("RightLobe"), ET66HitZoneType::RightArm, 0.16f, 1.10f, FVector(35.f, 165.f, 136.f), 118.f);
	AddSlimePart(TEXT("LeftBase"), ET66HitZoneType::LeftLeg, 0.16f, 0.95f, FVector(20.f, -118.f, 62.f), 104.f);
	AddSlimePart(TEXT("RightBase"), ET66HitZoneType::RightLeg, 0.16f, 0.95f, FVector(20.f, 118.f, 62.f), 104.f);
	AddSlimePart(TEXT("MouthCore"), ET66HitZoneType::Head, 0.36f, 1.35f, FVector(92.f, 0.f, 132.f), 142.f);
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

		const bool bEnableZone = bEnableHitZones && Zone->bTargetable && Part.CurrentHP > 0;
		Zone->HitZoneType = Part.HitZoneType;
		Zone->HitZoneName = Part.PartID;
		Zone->DamageMultiplier = FMath::Max(0.1f, Part.DamageMultiplier);
		Zone->SetCollisionEnabled(bEnableZone ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		Zone->SetGenerateOverlapEvents(bEnableZone);
		Zone->SetHiddenInGame(true);
	}
}

void AT66BossBase::DrawCombatDebug() const
{
	if (!bAwakened || CurrentHP <= 0)
	{
		return;
	}

	for (const FT66BossPartRuntimeState& Part : BossPartStates)
	{
		const bool bActive = Part.CurrentHP > 0 && Part.ZoneComponent && Part.ZoneComponent->bTargetable;
		const FString Label = FString::Printf(TEXT("Boss Hurtbox: %s"), *Part.PartID.ToString());
		T66CombatDebugDraw::DrawHitZone(Part.ZoneComponent, Part.HitZoneType, bActive, Label);
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

void AT66BossBase::LoadBossAttackOwnershipRows()
{
	BossAttackOwnershipRows.Reset();
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	if (T66GI && !BossID.IsNone())
	{
		T66GI->GetBossAttackOwnershipRows(BossID, BossAttackOwnershipRows);
	}

	for (FT66BossAttackOwnershipData& Row : BossAttackOwnershipRows)
	{
		if (Row.AttackRowID.IsNone())
		{
			Row.AttackRowID = Row.AttackID;
		}
	}

	UE_LOG(
		LogT66BossAttackOwnership,
		Display,
		TEXT("BossAttackOwnershipRowsLoaded BossID=%s RowCount=%d"),
		*BossID.ToString(),
		BossAttackOwnershipRows.Num());

	for (const FT66BossAttackOwnershipData& Row : BossAttackOwnershipRows)
	{
		RecordBossAttackOwnershipEvent(T66BossAttackEvent_Loaded, &Row, Row.OwningPartID, TEXT("LoadBossAttackOwnershipRows"));
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
	RefreshCombatHitZoneState();
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

	return INDEX_NONE;
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
	if (!IsAlive() || IsActorBeingDestroyed())
	{
		return Handle;
	}

	Handle.Actor = const_cast<AT66BossBase*>(this);

	const int32 PartIndex = ResolveBossPartIndex(HitComponent, PreferredZone);
	if (!BossPartStates.IsValidIndex(PartIndex))
	{
		if (BossPartStates.Num() > 0)
		{
			Handle.Reset();
			return Handle;
		}

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
	bDefeated = false;
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
	if (BossData.BossID == FName(TEXT("Dungeon_SewerSlimeKing")))
	{
		AssignSewerSlimeKingPartDefinitions();
		AttackProfile = ET66BossAttackProfile::Juggernaut;
		GroundAOEIntervalSeconds = 0.f;
	}
	else
	{
		AssignBossPartDefinitionsForProfile(ResolvedPartProfile);
		ConfigureAttackProfileFromBossPartProfile(ResolvedPartProfile);
	}
	AttackPrimaryColor = BossData.PlaceholderColor;
	AttackPrimaryColor.A = 1.f;
	AttackSecondaryColor = T66MakeAttackSecondaryColor(AttackPrimaryColor);
	BossMovementProfileID = BossData.BossMovementProfileID;
	LoadBossAttackOwnershipRows();
	LoadBossMovementPatternRows();

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
		if (!VisualMesh->GetStaticMesh())
		{
			if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
			{
				VisualMesh->SetStaticMesh(Sphere);
			}
		}
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 212.f));
		VisualMesh->SetRelativeScale3D(FVector(6.f, 6.f, 6.f));
		VisualMesh->SetHiddenInGame(false, true);
		VisualMesh->SetVisibility(true, true);
	}

	bool bAppliedVisual = false;
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
		{
			const FName BossVisualID = BossID.IsNone() ? FName(TEXT("Boss")) : BossID;
			bAppliedVisual = Visuals->ApplyCharacterVisual(BossVisualID, GetMesh(), nullptr, true, false, false, VisualMesh);
			if (!bAppliedVisual && BossVisualID != FName(TEXT("Boss")))
			{
				bAppliedVisual = Visuals->ApplyCharacterVisual(FName(TEXT("Boss")), GetMesh(), VisualMesh, true, false, false, VisualMesh);
			}
			if (USkeletalMeshComponent* SkelMesh = GetMesh())
			{
				if (bAppliedVisual && SkelMesh->IsVisible() && !SkelMesh->bHiddenInGame)
				{
					SkelMesh->SetRelativeScale3D(SkelMesh->GetRelativeScale3D() * 3.0f);
				}
				else
				{
					SkelMesh->SetVisibility(false, true);
					SkelMesh->SetHiddenInGame(true, true);
				}
			}
		}
	}
	const bool bSkeletalBodyVisible = GetMesh()
		&& GetMesh()->GetSkeletalMeshAsset()
		&& GetMesh()->IsVisible()
		&& !GetMesh()->bHiddenInGame;
	const bool bStaticBodyVisible = VisualMesh
		&& VisualMesh->GetStaticMesh()
		&& VisualMesh->IsVisible()
		&& !VisualMesh->bHiddenInGame;
	if (!bSkeletalBodyVisible && !bStaticBodyVisible && VisualMesh)
	{
		if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
		{
			VisualMesh->SetStaticMesh(Sphere);
		}
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 212.f));
		VisualMesh->SetRelativeScale3D(FVector(6.f, 6.f, 6.f));
		VisualMesh->SetHiddenInGame(false, true);
		VisualMesh->SetVisibility(true, true);
		if (UMaterialInstanceDynamic* Mat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0))
		{
			Mat->SetVectorParameterValue(TEXT("BaseColor"), BossData.PlaceholderColor);
		}
		UE_LOG(
			LogT66BossAttackOwnership,
			Warning,
			TEXT("BossVisualFallbackForced BossID=%s AppliedVisual=%d Location=%s"),
			*BossID.ToString(),
			bAppliedVisual ? 1 : 0,
			*GetActorLocation().ToString());
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

#if !UE_BUILD_SHIPPING
bool AT66BossBase::HasVisibleBossBodyForAutomation() const
{
	const bool bSkeletalBodyVisible = GetMesh()
		&& GetMesh()->GetSkeletalMeshAsset()
		&& GetMesh()->IsVisible()
		&& !GetMesh()->bHiddenInGame;
	const bool bStaticBodyVisible = VisualMesh
		&& VisualMesh->GetStaticMesh()
		&& VisualMesh->IsVisible()
		&& !VisualMesh->bHiddenInGame;
	return bSkeletalBodyVisible || bStaticBodyVisible;
}
#endif

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

void AT66BossBase::ApplyEndgameBossMultipliers(const float HealthScalar, const float DamageScalar, const float ScaleScalar)
{
	if (!bBaseTuningInitialized)
	{
		BaseMaxHP = MaxHP;
		BaseProjectileDamageHearts = ProjectileDamageHearts;
		bBaseTuningInitialized = true;
	}

	const float ClampedHealthScalar = FMath::Clamp(HealthScalar, 0.1f, 99.f);
	const float ClampedDamageScalar = FMath::Clamp(DamageScalar, 0.1f, 99.f);
	MaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(MaxHP) * ClampedHealthScalar));
	CurrentHP = MaxHP;
	ProjectileDamageHearts = FMath::Max(1, FMath::RoundToInt(static_cast<float>(ProjectileDamageHearts) * ClampedDamageScalar));
	RebuildBossPartState(false);

	if (ScaleScalar > 0.f && !FMath::IsNearlyEqual(ScaleScalar, 1.f))
	{
		SetActorScale3D(GetActorScale3D() * FMath::Clamp(ScaleScalar, 0.25f, 8.f));
	}

	if (bAwakened)
	{
		PushBossPartStateToRunState();
	}
}

void AT66BossBase::LoadBossMovementPatternRows()
{
	BossMovementPatternRows.Reset();
	if (BossMovementProfileID.IsNone())
	{
		UE_LOG(LogT66BossMovement, Display, TEXT("BossMovementRowsLoaded BossID=%s MovementProfileID=None RowCount=0 Fallback=SimpleChase"), *BossID.ToString());
		return;
	}

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	if (T66GI)
	{
		T66GI->GetBossMovementPatternRows(BossMovementProfileID, BossMovementPatternRows);
	}

	UE_LOG(
		LogT66BossMovement,
		Display,
		TEXT("BossMovementRowsLoaded BossID=%s MovementProfileID=%s RowCount=%d"),
		*BossID.ToString(),
		*BossMovementProfileID.ToString(),
		BossMovementPatternRows.Num());
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

void AT66BossBase::RecordBossAttackOwnershipEvent(
	const FName EventID,
	const FT66BossAttackOwnershipData* AttackRow,
	const FName PartID,
	const TCHAR* Context)
{
	const FName AttackID = AttackRow ? AttackRow->AttackID : NAME_None;
	const FName AttackRowID = AttackRow ? AttackRow->AttackRowID : NAME_None;
	const FName OwningPartID = AttackRow ? AttackRow->OwningPartID : NAME_None;

#if !UE_BUILD_SHIPPING
	const FString Key = T66MakeBossAttackCounterKey(EventID, AttackID, PartID);
	int32& Counter = BossAttackOwnershipAutomationCounters.FindOrAdd(Key);
	++Counter;
#endif

	UE_LOG(
		LogT66BossAttackOwnership,
		Display,
		TEXT("BossAttackOwnershipEvent Event=%s BossID=%s AttackRowID=%s AttackID=%s OwningPartID=%s PartID=%s Context=%s"),
		*EventID.ToString(),
		*BossID.ToString(),
		*AttackRowID.ToString(),
		*AttackID.ToString(),
		*OwningPartID.ToString(),
		*PartID.ToString(),
		Context ? Context : TEXT("None"));
}

#if !UE_BUILD_SHIPPING
void AT66BossBase::ResetBossAttackOwnershipAutomationCounters()
{
	BossAttackOwnershipAutomationCounters.Reset();
}

int32 AT66BossBase::GetBossAttackOwnershipAutomationCounter(const FName EventID, const FName AttackID, const FName PartID) const
{
	const FString Key = T66MakeBossAttackCounterKey(EventID, AttackID, PartID);
	if (const int32* Counter = BossAttackOwnershipAutomationCounters.Find(Key))
	{
		return *Counter;
	}
	return 0;
}

bool AT66BossBase::KillBossPartForAutomation(const FName PartID)
{
	for (FT66BossPartRuntimeState& Part : BossPartStates)
	{
		if (Part.PartID != PartID)
		{
			continue;
		}

		const bool bWasAlive = Part.CurrentHP > 0;
		Part.CurrentHP = 0;
		CurrentHP = 0;
		for (const FT66BossPartRuntimeState& RuntimePart : BossPartStates)
		{
			CurrentHP += RuntimePart.CurrentHP;
		}
		CurrentHP = FMath::Clamp(CurrentHP, 0, MaxHP);
		if (bWasAlive)
		{
			UE_LOG(LogT66BossAttackOwnership, Display, TEXT("BossAttackOwnershipAutomationKilledPart BossID=%s PartID=%s"), *BossID.ToString(), *PartID.ToString());
			RefreshCombatHitZoneState();
			PushBossPartStateToRunState();
		}
		return bWasAlive;
	}
	return false;
}

void AT66BossBase::ResetBossMovementAutomationState()
{
	LastBossMovementAutomationMode = NAME_None;
}

FName AT66BossBase::GetBossMovementAutomationMode() const
{
	return LastBossMovementAutomationMode;
}
#endif

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
	if (UT66ProjectileManagerSubsystem* ProjectileManager = World->GetSubsystem<UT66ProjectileManagerSubsystem>())
	{
		FT66ManagedProjectileFireParams FireParams;
		FireParams.SourceActor = this;
		FireParams.SourceID = BossID;
		FireParams.Origin = SpawnLoc;
		FireParams.Direction = ShotDirection;
		FireParams.Speed = ProjectileSpeed * FMath::Max(0.35f, SpeedScale);
		FireParams.Damage = FMath::Max(1, ProjectileDamageHearts) * 20.f;
		FireParams.BossAttackProfile = AttackProfile;
		FireParams.BossPrimaryColor = AttackPrimaryColor;
		FireParams.BossSecondaryColor = AttackSecondaryColor;
		FireParams.bUseBossSecondaryTint = bUseSecondaryTint;
		FireParams.BossVisualScaleMultiplier = 1.f;
		FireParams.AttackCategory = ET66AttackCategory::AOE;
		if (ProjectileManager->FireBossProjectile(FireParams))
		{
			T66PlayBossProfileAudioEvent(this, TEXT("Boss.Projectile.Fire"), FName(TEXT("Boss.Projectile.Fire")), SpawnLoc);
		}
	}
}

void AT66BossBase::SpawnProjectileInDirectionForAttackRow(
	const FT66BossAttackOwnershipData& AttackRow,
	const FVector& Direction,
	const float SpeedScale,
	const FVector& SpawnOffset,
	const bool bUseSecondaryTint,
	const FName VisualProfileID,
	const float VisualScaleMultiplier,
	const ET66AttackCategory ProjectileCategory,
	const TSoftObjectPtr<UStaticMesh> ProjectileMesh,
	const float ProjectileMeshScale)
{
	if (!bAwakened || CurrentHP <= 0 || StunSecondsRemaining > 0.f || FreezeSecondsRemaining > 0.f)
	{
		return;
	}

	FName DeadPartID = NAME_None;
	if (!AreBossAttackPartsAlive(AttackRow, DeadPartID))
	{
		RecordBossAttackOwnershipEvent(T66BossAttackEvent_Suppressed, &AttackRow, DeadPartID, TEXT("LegacyProjectileDelayedShot"));
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
	if (UT66ProjectileManagerSubsystem* ProjectileManager = World->GetSubsystem<UT66ProjectileManagerSubsystem>())
	{
		FT66ManagedProjectileFireParams FireParams;
		FireParams.SourceActor = this;
		FireParams.SourceID = BossID;
		FireParams.Origin = SpawnLoc;
		FireParams.Direction = ShotDirection;
		FireParams.Speed = ProjectileSpeed * FMath::Max(0.35f, SpeedScale);
		FireParams.Damage = FMath::Max(1, ProjectileDamageHearts) * 20.f;
		FireParams.BossAttackProfile = T66ResolveLegacyAttackProfileFromAttackID(AttackRow.AttackID, AttackProfile);
		FireParams.BossPrimaryColor = AttackPrimaryColor;
		FireParams.BossSecondaryColor = AttackSecondaryColor;
		FireParams.bUseBossSecondaryTint = bUseSecondaryTint;
		FireParams.VisualProfileID = VisualProfileID;
		FireParams.BossVisualScaleMultiplier = VisualScaleMultiplier;
		FireParams.AttackCategory = ProjectileCategory;
		FireParams.ProjectileMesh = ProjectileMesh;
		FireParams.ProjectileMeshScale = ProjectileMeshScale;
		if (ProjectileManager->FireBossProjectile(FireParams))
		{
			RecordBossAttackOwnershipEvent(T66BossAttackEvent_Fired, &AttackRow, AttackRow.OwningPartID, TEXT("LegacyProjectileDelayedShot"));
			T66PlayBossProfileAudioEvent(this, TEXT("Boss.Projectile.Fire"), FName(TEXT("Boss.Projectile.Fire")), SpawnLoc);
		}
	}
}

void AT66BossBase::SpawnScaledProjectileInDirection(
	const FVector& Direction,
	const float SpeedScale,
	const FVector& SpawnOffset,
	const bool bUseSecondaryTint,
	const float VisualScaleMultiplier)
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
	if (UT66ProjectileManagerSubsystem* ProjectileManager = World->GetSubsystem<UT66ProjectileManagerSubsystem>())
	{
		FT66ManagedProjectileFireParams FireParams;
		FireParams.SourceActor = this;
		FireParams.SourceID = BossID;
		FireParams.Origin = SpawnLoc;
		FireParams.Direction = ShotDirection;
		FireParams.Speed = ProjectileSpeed * FMath::Max(0.35f, SpeedScale);
		FireParams.Damage = FMath::Max(1, ProjectileDamageHearts) * 20.f;
		FireParams.BossAttackProfile = AttackProfile;
		FireParams.BossPrimaryColor = AttackPrimaryColor;
		FireParams.BossSecondaryColor = AttackSecondaryColor;
		FireParams.bUseBossSecondaryTint = bUseSecondaryTint;
		FireParams.BossVisualScaleMultiplier = VisualScaleMultiplier;
		FireParams.AttackCategory = ET66AttackCategory::AOE;
		if (ProjectileManager->FireBossProjectile(FireParams))
		{
			T66PlayBossProfileAudioEvent(this, TEXT("Boss.Projectile.Fire"), FName(TEXT("Boss.Projectile.Fire")), SpawnLoc);
		}
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

void AT66BossBase::QueueProjectileShotDirectionForAttackRow(
	const FT66BossAttackOwnershipData& AttackRow,
	const FVector& Direction,
	const float DelaySeconds,
	const float SpeedScale,
	const FVector& SpawnOffset,
	const bool bUseSecondaryTint,
	const FName VisualProfileID,
	const float VisualScaleMultiplier,
	const ET66AttackCategory ProjectileCategory,
	const TSoftObjectPtr<UStaticMesh> ProjectileMesh,
	const float ProjectileMeshScale)
{
	const FVector ShotDirection = Direction.GetSafeNormal();
	if (ShotDirection.IsNearlyZero())
	{
		return;
	}

	TWeakObjectPtr<AT66BossBase> WeakThis(this);
	QueueTimedAttackLambda(
		FTimerDelegate::CreateLambda([WeakThis, AttackRow, ShotDirection, SpeedScale, SpawnOffset, bUseSecondaryTint, VisualProfileID, VisualScaleMultiplier, ProjectileCategory, ProjectileMesh, ProjectileMeshScale]()
		{
			if (!WeakThis.IsValid())
			{
				return;
			}

			WeakThis->SpawnProjectileInDirectionForAttackRow(AttackRow, ShotDirection, SpeedScale, SpawnOffset, bUseSecondaryTint, VisualProfileID, VisualScaleMultiplier, ProjectileCategory, ProjectileMesh, ProjectileMeshScale);
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

void AT66BossBase::QueueProjectileShotTowardsForAttackRow(
	const FT66BossAttackOwnershipData& AttackRow,
	const FVector& TargetLocation,
	const float DelaySeconds,
	const float YawOffsetDegrees,
	const float SpeedScale,
	const FVector& SpawnOffset,
	const bool bUseSecondaryTint,
	const FName VisualProfileID,
	const float VisualScaleMultiplier,
	const ET66AttackCategory ProjectileCategory,
	const TSoftObjectPtr<UStaticMesh> ProjectileMesh,
	const float ProjectileMeshScale)
{
	FVector Direction = TargetLocation - (GetActorLocation() + FVector(0.f, 0.f, 84.f));
	Direction.Z = 0.f;
	Direction = Direction.GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = GetActorForwardVector();
	}

	QueueProjectileShotDirectionForAttackRow(AttackRow, T66RotatePlanarVector(Direction, YawOffsetDegrees), DelaySeconds, SpeedScale, SpawnOffset, bUseSecondaryTint, VisualProfileID, VisualScaleMultiplier, ProjectileCategory, ProjectileMesh, ProjectileMeshScale);
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

void AT66BossBase::QueueProjectileFanBurstForAttackRow(
	const FT66BossAttackOwnershipData& AttackRow,
	const FVector& TargetLocation,
	const int32 ShotCount,
	const float SpreadDegrees,
	const float DelayStepSeconds,
	const float SpeedScale,
	const float InitialDelaySeconds,
	const float SideOffsetDistance,
	const bool bUseSecondaryTint,
	const FName VisualProfileID,
	const float VisualScaleMultiplier,
	const ET66AttackCategory ProjectileCategory,
	const TSoftObjectPtr<UStaticMesh> ProjectileMesh,
	const float ProjectileMeshScale)
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
		QueueProjectileShotTowardsForAttackRow(
			AttackRow,
			TargetLocation,
			InitialDelaySeconds + DelayStepSeconds * static_cast<float>(Index),
			StartYaw + StepYaw * static_cast<float>(Index),
			SpeedScale,
			SpawnOffset,
			bUseSecondaryTint && ((Index % 2) == 1),
			VisualProfileID,
			VisualScaleMultiplier,
			ProjectileCategory,
			ProjectileMesh,
			ProjectileMeshScale);
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

void AT66BossBase::QueueRadialBurstForAttackRow(
	const FT66BossAttackOwnershipData& AttackRow,
	const int32 ShotCount,
	const float DelayStepSeconds,
	const float StartAngleDegrees,
	const float SpeedScale,
	const float InitialDelaySeconds,
	const bool bUseSecondaryTint,
	const FName VisualProfileID,
	const float VisualScaleMultiplier,
	const ET66AttackCategory ProjectileCategory,
	const TSoftObjectPtr<UStaticMesh> ProjectileMesh,
	const float ProjectileMeshScale)
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
		QueueProjectileShotDirectionForAttackRow(
			AttackRow,
			Direction,
			InitialDelaySeconds + DelayStepSeconds * static_cast<float>(Index),
			SpeedScale,
			FVector::ZeroVector,
			bUseSecondaryTint && ((Index % 2) == 1),
			VisualProfileID,
			VisualScaleMultiplier,
			ProjectileCategory,
			ProjectileMesh,
			ProjectileMeshScale);
	}
}

bool AT66BossBase::IsSewerSlimeKingBoss() const
{
	return BossID == FName(TEXT("Dungeon_SewerSlimeKing"));
}

bool AT66BossBase::IsBossPartAlive(const FName PartID) const
{
	if (PartID.IsNone())
	{
		return true;
	}

	for (const FT66BossPartRuntimeState& Part : BossPartStates)
	{
		if (Part.PartID == PartID)
		{
			return Part.CurrentHP > 0;
		}
	}

	return false;
}

FVector AT66BossBase::GetBossPartWorldLocation(const FName PartID) const
{
	for (const FT66BossPartRuntimeState& Part : BossPartStates)
	{
		if (Part.PartID == PartID)
		{
			return Part.ZoneComponent ? Part.ZoneComponent->GetComponentLocation() : GetActorLocation() + FVector(0.f, 0.f, 120.f);
		}
	}

	return GetActorLocation() + FVector(0.f, 0.f, 120.f);
}

bool AT66BossBase::AreBossAttackPartsAlive(const FT66BossAttackOwnershipData& AttackRow, FName& OutDeadPartID) const
{
	OutDeadPartID = NAME_None;
	if (!IsBossPartAlive(AttackRow.OwningPartID))
	{
		OutDeadPartID = AttackRow.OwningPartID;
		return false;
	}

	for (const FName RequiredPartID : T66ParseRequiredBossPartIDs(AttackRow.RequiredPartIDs))
	{
		if (!IsBossPartAlive(RequiredPartID))
		{
			OutDeadPartID = RequiredPartID;
			return false;
		}
	}

	return true;
}

bool AT66BossBase::CanSelectBossAttackRow(const FT66BossAttackOwnershipData& AttackRow, const int32 Phase, FName& OutDeadPartID) const
{
	if (!AttackRow.bSelectable || AttackRow.BossID != BossID || AttackRow.SelectionWeight <= 0.f)
	{
		OutDeadPartID = NAME_None;
		return false;
	}

	if (Phase < AttackRow.MinPhase || Phase > AttackRow.MaxPhase)
	{
		OutDeadPartID = NAME_None;
		return false;
	}

	return AreBossAttackPartsAlive(AttackRow, OutDeadPartID);
}

const FT66BossAttackOwnershipData* AT66BossBase::PickBossAttackRowByPrefix(
	const TCHAR* AttackIDPrefix,
	bool& bOutHasMatchingRows,
	FName& OutSuppressedPartID) const
{
	bOutHasMatchingRows = false;
	OutSuppressedPartID = NAME_None;

	TArray<const FT66BossAttackOwnershipData*> AvailableRows;
	const int32 Phase = GetAttackPhaseIndex();
	for (const FT66BossAttackOwnershipData& Row : BossAttackOwnershipRows)
	{
		if (Row.BossID != BossID || !Row.bSelectable || !T66BossAttackIDStartsWith(Row.AttackID, AttackIDPrefix))
		{
			continue;
		}

		bOutHasMatchingRows = true;
		FName DeadPartID = NAME_None;
		if (CanSelectBossAttackRow(Row, Phase, DeadPartID))
		{
			AvailableRows.Add(&Row);
		}
		else if (OutSuppressedPartID.IsNone())
		{
			OutSuppressedPartID = DeadPartID;
		}
	}

	if (AvailableRows.Num() <= 0)
	{
		return nullptr;
	}

	float TotalWeight = 0.f;
	for (const FT66BossAttackOwnershipData* Row : AvailableRows)
	{
		TotalWeight += Row ? FMath::Max(0.f, Row->SelectionWeight) : 0.f;
	}

	if (TotalWeight <= KINDA_SMALL_NUMBER)
	{
		return AvailableRows[0];
	}

	float Pick = FMath::FRandRange(0.f, TotalWeight);
	for (const FT66BossAttackOwnershipData* Row : AvailableRows)
	{
		Pick -= Row ? FMath::Max(0.f, Row->SelectionWeight) : 0.f;
		if (Pick <= 0.f)
		{
			return Row;
		}
	}

	return AvailableRows.Last();
}

const FT66BossAttackOwnershipData* AT66BossBase::FindBossAttackRowByAttackID(const FName AttackID, const FName OwningPartID) const
{
	for (const FT66BossAttackOwnershipData& Row : BossAttackOwnershipRows)
	{
		if (Row.BossID == BossID
			&& Row.AttackID == AttackID
			&& (OwningPartID.IsNone() || Row.OwningPartID == OwningPartID))
		{
			return &Row;
		}
	}
	return nullptr;
}

const FT66BossAttackOwnershipData* AT66BossBase::PickSewerSlimeKingAttackRow() const
{
	TArray<const FT66BossAttackOwnershipData*> AvailableRows;
	const int32 Phase = GetAttackPhaseIndex();
	for (const FT66BossAttackOwnershipData& Row : BossAttackOwnershipRows)
	{
		FName DeadPartID;
		if (CanSelectBossAttackRow(Row, Phase, DeadPartID))
		{
			AvailableRows.Add(&Row);
		}
	}

	if (AvailableRows.Num() <= 0)
	{
		return nullptr;
	}

	if (AvailableRows.Num() > 1)
	{
		AvailableRows.RemoveAllSwap([this](const FT66BossAttackOwnershipData* Row)
		{
			return Row && Row->OwningPartID == LastSewerSlimeKingAttackPart;
		});
	}

	float TotalWeight = 0.f;
	for (const FT66BossAttackOwnershipData* Row : AvailableRows)
	{
		TotalWeight += Row ? FMath::Max(0.f, Row->SelectionWeight) : 0.f;
	}
	if (TotalWeight <= 0.f)
	{
		return AvailableRows[FMath::RandRange(0, AvailableRows.Num() - 1)];
	}

	float Roll = FMath::FRandRange(0.f, TotalWeight);
	for (const FT66BossAttackOwnershipData* Row : AvailableRows)
	{
		const float Weight = Row ? FMath::Max(0.f, Row->SelectionWeight) : 0.f;
		if (Roll <= Weight)
		{
			return Row;
		}
		Roll -= Weight;
	}
	return AvailableRows.Last();
}

const FT66BossAttackOwnershipData* AT66BossBase::FindSewerSlimeKingAttackRowForPart(const FName AttackPartID) const
{
	for (const FT66BossAttackOwnershipData& Row : BossAttackOwnershipRows)
	{
		if (Row.BossID == BossID && Row.bSelectable && Row.OwningPartID == AttackPartID)
		{
			return &Row;
		}
	}
	return nullptr;
}

const FT66BossAttackOwnershipData* AT66BossBase::FindSewerSlimeKingMouthSidecarRow() const
{
	for (const FT66BossAttackOwnershipData& Row : BossAttackOwnershipRows)
	{
		if (Row.BossID == BossID && !Row.bSelectable && Row.AttackID == T66BossAttackID_SewerSlimeMouthSidecar)
		{
			return &Row;
		}
	}
	return nullptr;
}

void AT66BossBase::SpawnSewerSlimeKingTelegraph(
	const FName AttackPartID,
	const FVector& Location,
	const float DurationSeconds,
	const float ScaleMultiplier,
	const bool bCylinder)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AT66BossAttackTelegraph* Telegraph = World->SpawnActor<AT66BossAttackTelegraph>(
		AT66BossAttackTelegraph::StaticClass(),
		Location,
		FRotator::ZeroRotator,
		SpawnParams))
	{
		UStaticMesh* TelegraphMesh = bCylinder ? FT66VisualUtil::GetBasicShapeCylinder() : FT66VisualUtil::GetBasicShapeSphere();
		const bool bMouthAttack = AttackPartID == FName(TEXT("MouthCore"));
		const FLinearColor TelegraphColor = bMouthAttack
			? FLinearColor(0.68f, 1.00f, 0.10f, 1.f)
			: FLinearColor(0.20f, 0.95f, 0.08f, 1.f);
		const FVector EndScale = bCylinder
			? FVector(ScaleMultiplier, ScaleMultiplier, 0.32f)
			: FVector(ScaleMultiplier);
		Telegraph->ConfigureTelegraph(TelegraphMesh, TelegraphColor, EndScale * 0.18f, EndScale, DurationSeconds, bMouthAttack ? 260.f : 180.f);
	}
}

void AT66BossBase::QueueSewerSlimeKingLobeVolley(
	const FT66BossAttackOwnershipData& AttackRow,
	APawn* InitialPlayerPawn,
	const bool bUseSecondaryTint)
{
	static const float ShotYawOffsets[] = { -10.f, -5.f, 0.f, 5.f, 10.f };
	const FName AttackPartID = AttackRow.OwningPartID;
	TWeakObjectPtr<AT66BossBase> WeakThis(this);
	TWeakObjectPtr<APawn> WeakInitialPlayer(InitialPlayerPawn);

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(ShotYawOffsets); ++Index)
	{
		const float Delay = 0.58f + static_cast<float>(Index) * 0.12f;
		const float YawOffset = ShotYawOffsets[Index];
		QueueTimedAttackLambda(
			FTimerDelegate::CreateLambda([WeakThis, WeakInitialPlayer, AttackRow, AttackPartID, YawOffset, bUseSecondaryTint, Index]()
			{
				if (!WeakThis.IsValid())
				{
					return;
				}

				AT66BossBase* Boss = WeakThis.Get();
				FName DeadPartID;
				if (!Boss->AreBossAttackPartsAlive(AttackRow, DeadPartID))
				{
					Boss->RecordBossAttackOwnershipEvent(T66BossAttackEvent_Suppressed, &AttackRow, DeadPartID, TEXT("SewerSlimeLobeVolleyDelayedShot"));
					return;
				}

				APawn* PlayerPawn = Boss->ResolvePlayerPawn();
				if (!PlayerPawn && WeakInitialPlayer.IsValid())
				{
					PlayerPawn = WeakInitialPlayer.Get();
				}
				if (!PlayerPawn)
				{
					return;
				}

				const FVector PartWorldLocation = Boss->GetBossPartWorldLocation(AttackPartID);
				FVector Direction = PlayerPawn->GetActorLocation() - PartWorldLocation;
				Direction.Z = 0.f;
				Direction = T66RotatePlanarVector(Direction.GetSafeNormal(), YawOffset);
				const FVector SpawnOffset = PartWorldLocation - (Boss->GetActorLocation() + FVector(0.f, 0.f, 84.f));
				Boss->SpawnScaledProjectileInDirection(Direction, 0.88f + static_cast<float>(Index) * 0.025f, SpawnOffset, bUseSecondaryTint, 1.12f);
				Boss->RecordBossAttackOwnershipEvent(T66BossAttackEvent_Fired, &AttackRow, AttackPartID, TEXT("SewerSlimeLobeVolleyDelayedShot"));
			}),
			Delay);
	}
}

void AT66BossBase::SpawnSewerSlimeKingLaneBlocker(const FT66BossAttackOwnershipData& AttackRow, const FVector& TargetLocation)
{
	FName DeadPartID;
	if (!AreBossAttackPartsAlive(AttackRow, DeadPartID))
	{
		RecordBossAttackOwnershipEvent(T66BossAttackEvent_Suppressed, &AttackRow, DeadPartID, TEXT("SewerSlimeLaneBlockerImmediate"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector Forward = TargetLocation - GetActorLocation();
	Forward.Z = 0.f;
	Forward = Forward.GetSafeNormal();
	if (Forward.IsNearlyZero())
	{
		Forward = GetActorForwardVector().GetSafeNormal2D();
	}
	const FVector Right = T66ResolvePlanarRightVector(Forward);
	const FName AttackPartID = AttackRow.OwningPartID;
	const float SideSign = (AttackPartID == FName(TEXT("LeftBase"))) ? -1.f : 1.f;
	const FVector HazardBase = ResolveGroundLocation(TargetLocation + Forward * 100.f + Right * SideSign * 255.f);
	const FVector SpawnLocation = HazardBase + FVector(0.f, 0.f, 48.f);
	const int32 DamageHP = FMath::Max(18, ProjectileDamageHearts * 22);

	if (UT66BossHazardSubsystem* HazardSubsystem = World->GetSubsystem<UT66BossHazardSubsystem>())
	{
		FT66BossHazardSpawnParams HazardParams;
		HazardParams.SourceActor = this;
		HazardParams.SourceID = BossID;
		HazardParams.HazardID = AttackRow.AttackID;
		HazardParams.Location = SpawnLocation;
		HazardParams.Rotation = FRotator::ZeroRotator;
		HazardParams.RadiusScale = 1.f;
		HazardParams.TelegraphScale = 1.f;
		HazardParams.VisualScaleMultiplier = 1.f;
		HazardParams.DamageOverrideHP = DamageHP;
		if (HazardSubsystem->SpawnBossHazard(HazardParams))
		{
			RecordBossAttackOwnershipEvent(T66BossAttackEvent_Fired, &AttackRow, AttackPartID, TEXT("BossHazardDefinitionLaneBlocker"));
			return;
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AT66BossLaneBlockerHazard* Hazard = World->SpawnActor<AT66BossLaneBlockerHazard>(
		AT66BossLaneBlockerHazard::StaticClass(),
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams))
	{
		Hazard->ConfigureHazard(
			FLinearColor(0.36f, 1.00f, 0.06f, 1.f),
			FLinearColor(0.06f, 0.88f, 0.20f, 1.f),
			FVector(2.65f, 2.65f, 0.80f),
			FVector(185.f, 185.f, 115.f),
			0.78f,
			1.45f,
			DamageHP);
		RecordBossAttackOwnershipEvent(T66BossAttackEvent_Fired, &AttackRow, AttackPartID, TEXT("SewerSlimeLaneBlockerImmediate"));
	}
}

void AT66BossBase::SpawnSewerSlimeKingMouthProjectile(const FT66BossAttackOwnershipData& AttackRow, const FVector& TargetLocation)
{
	FName DeadPartID;
	if (!AreBossAttackPartsAlive(AttackRow, DeadPartID))
	{
		RecordBossAttackOwnershipEvent(T66BossAttackEvent_Suppressed, &AttackRow, DeadPartID, TEXT("SewerSlimeMouthDelayedShot"));
		return;
	}

	const FName AttackPartID = AttackRow.OwningPartID;
	const FVector MouthLocation = GetBossPartWorldLocation(AttackPartID);
	FVector Direction = TargetLocation - MouthLocation;
	Direction.Z = 0.f;
	Direction = Direction.GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = GetActorForwardVector().GetSafeNormal2D();
	}

	const FVector SpawnOffset = MouthLocation - (GetActorLocation() + FVector(0.f, 0.f, 84.f));
	SpawnScaledProjectileInDirection(Direction, 0.82f, SpawnOffset, true, 2.35f);
	RecordBossAttackOwnershipEvent(T66BossAttackEvent_Fired, &AttackRow, AttackPartID, TEXT("SewerSlimeMouthDelayedShot"));
}

void AT66BossBase::FireSewerSlimeKingAttack(APawn* PlayerPawn, const FName ForcedAttackPartID)
{
	if (!PlayerPawn)
	{
		return;
	}

	if (BossAttackOwnershipRows.Num() <= 0)
	{
		UE_LOG(LogT66BossAttackOwnership, Warning, TEXT("BossAttackOwnershipMissingRows BossID=%s Context=FireSewerSlimeKingAttack"), *BossID.ToString());
		return;
	}

	const FT66BossAttackOwnershipData* AttackRow = ForcedAttackPartID.IsNone()
		? PickSewerSlimeKingAttackRow()
		: FindSewerSlimeKingAttackRowForPart(ForcedAttackPartID);
	if (!AttackRow)
	{
		UE_LOG(
			LogT66BossAttackOwnership,
			Display,
			TEXT("BossAttackOwnershipSuppressed BossID=%s ForcedPart=%s Reason=NoAuthoredAttackRow"),
			*BossID.ToString(),
			*ForcedAttackPartID.ToString());
		return;
	}

	FName DeadPartID;
	if (!AreBossAttackPartsAlive(*AttackRow, DeadPartID))
	{
		RecordBossAttackOwnershipEvent(T66BossAttackEvent_Suppressed, AttackRow, DeadPartID, ForcedAttackPartID.IsNone() ? TEXT("SewerSlimeSchedulerSelection") : TEXT("SewerSlimeForcedSelection"));
		return;
	}

	FireSewerSlimeKingAttackRow(PlayerPawn, *AttackRow);
}

void AT66BossBase::FireSewerSlimeKingAttackRow(APawn* PlayerPawn, const FT66BossAttackOwnershipData& AttackRow)
{
	if (!PlayerPawn)
	{
		return;
	}

	const FName AttackPartID = AttackRow.OwningPartID;
	LastSewerSlimeKingAttackPart = AttackPartID;
	RecordBossAttackOwnershipEvent(T66BossAttackEvent_Queued, &AttackRow, AttackPartID, TEXT("SewerSlimeAttackRow"));
	NotifyBossMovementAttackCoordinationStarted(AttackRow);

	const FVector TargetLocation = PlayerPawn->GetActorLocation();
	FVector Facing = TargetLocation - GetActorLocation();
	Facing.Z = 0.f;
	if (!Facing.IsNearlyZero())
	{
		SetActorRotation(Facing.Rotation());
	}

	if (AttackRow.AttackID != T66BossAttackID_SewerSlimeMouthProjectile)
	{
		const FT66BossAttackOwnershipData* MouthSidecarRow = FindSewerSlimeKingMouthSidecarRow();
		if (MouthSidecarRow)
		{
			FName DeadPartID;
			if (AreBossAttackPartsAlive(*MouthSidecarRow, DeadPartID))
			{
				const FVector MouthTelegraphLocation = GetBossPartWorldLocation(MouthSidecarRow->OwningPartID) + FVector(0.f, 0.f, 18.f);
				SpawnSewerSlimeKingTelegraph(MouthSidecarRow->OwningPartID, MouthTelegraphLocation, 0.32f, 0.82f, false);
				QueueTimedAttackLambda(
					FTimerDelegate::CreateWeakLambda(this, [this, SidecarRow = *MouthSidecarRow, TargetLocation]()
					{
						SpawnSewerSlimeKingMouthProjectile(SidecarRow, TargetLocation);
					}),
					0.34f);
				RecordBossAttackOwnershipEvent(T66BossAttackEvent_Queued, MouthSidecarRow, MouthSidecarRow->OwningPartID, TEXT("SewerSlimeMouthSidecar"));
			}
			else
			{
				RecordBossAttackOwnershipEvent(T66BossAttackEvent_Suppressed, MouthSidecarRow, DeadPartID, TEXT("SewerSlimeMouthSidecar"));
			}
		}
	}

	if (AttackRow.AttackID == T66BossAttackID_SewerSlimeLobeVolley)
	{
		const FVector TelegraphLocation = GetBossPartWorldLocation(AttackPartID) + FVector(0.f, 0.f, 28.f);
		SpawnSewerSlimeKingTelegraph(AttackPartID, TelegraphLocation, 0.56f, 1.05f, false);
		QueueSewerSlimeKingLobeVolley(AttackRow, PlayerPawn, AttackPartID == FName(TEXT("RightLobe")));
		return;
	}

	if (AttackRow.AttackID == T66BossAttackID_SewerSlimeLaneBlocker)
	{
		SpawnSewerSlimeKingLaneBlocker(AttackRow, TargetLocation);
		return;
	}

	if (AttackRow.AttackID == T66BossAttackID_SewerSlimeMouthProjectile)
	{
		const FVector TelegraphLocation = GetBossPartWorldLocation(AttackPartID) + FVector(0.f, 0.f, 28.f);
		SpawnSewerSlimeKingTelegraph(AttackPartID, TelegraphLocation, 0.95f, 1.8f, false);
		QueueTimedAttackLambda(
			FTimerDelegate::CreateWeakLambda(this, [this, AttackRow, TargetLocation]()
			{
				SpawnSewerSlimeKingMouthProjectile(AttackRow, TargetLocation);
			}),
			0.98f);
		return;
	}

	RecordBossAttackOwnershipEvent(T66BossAttackEvent_Suppressed, &AttackRow, AttackPartID, TEXT("UnsupportedSewerSlimeAttackID"));
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
		T66PlayBossProfileAudioEvent(this, TEXT("Boss.AOE.Warning"), FName(TEXT("Boss.AOE.Warning")), AOE->GetActorLocation());
	}
}

void AT66BossBase::SpawnGroundAOEAtLocationForAttackRow(
	const FT66BossAttackOwnershipData& AttackRow,
	const FVector& WorldLocation,
	const float RadiusScale,
	const float WarningScale,
	const bool bUseSecondaryTint)
{
	if (!bAwakened || CurrentHP <= 0 || StunSecondsRemaining > 0.f || FreezeSecondsRemaining > 0.f)
	{
		return;
	}

	FName DeadPartID = NAME_None;
	if (!AreBossAttackPartsAlive(AttackRow, DeadPartID))
	{
		RecordBossAttackOwnershipEvent(T66BossAttackEvent_Suppressed, &AttackRow, DeadPartID, TEXT("LegacyGroundAOEImmediate"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	UT66RunStateSubsystem* RS = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const int32 Stage = RS ? FMath::Max(1, RS->GetCurrentStage()) : 1;
	const int32 DamageHP = FMath::Max(10, FMath::RoundToInt(static_cast<float>(GroundAOEBaseDamageHP) * FMath::Pow(1.25f, static_cast<float>(Stage - 1))));
	const FVector GroundLocation = ResolveGroundLocation(WorldLocation);

	if (UT66BossHazardSubsystem* HazardSubsystem = World->GetSubsystem<UT66BossHazardSubsystem>())
	{
		FT66BossHazardSpawnParams HazardParams;
		HazardParams.SourceActor = this;
		HazardParams.SourceID = BossID;
		HazardParams.HazardID = AttackRow.AttackID;
		HazardParams.Location = GroundLocation;
		HazardParams.Rotation = FRotator::ZeroRotator;
		HazardParams.RadiusScale = FMath::Max(0.35f, RadiusScale);
		HazardParams.TelegraphScale = FMath::Max(0.55f, WarningScale);
		HazardParams.VisualScaleMultiplier = 1.f;
		HazardParams.DamageOverrideHP = DamageHP;
		if (HazardSubsystem->SpawnBossHazard(HazardParams))
		{
			RecordBossAttackOwnershipEvent(T66BossAttackEvent_Fired, &AttackRow, AttackRow.OwningPartID, TEXT("BossHazardDefinitionImmediate"));
			T66PlayBossProfileAudioEvent(this, TEXT("Boss.AOE.Warning"), FName(TEXT("Boss.AOE.Warning")), GroundLocation);
			return;
		}
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AT66BossGroundAOE* AOE = World->SpawnActor<AT66BossGroundAOE>(AT66BossGroundAOE::StaticClass(), GroundLocation, FRotator::ZeroRotator, Params))
	{
		AOE->Radius = GroundAOERadius * FMath::Max(0.35f, RadiusScale);
		AOE->WarningDurationSeconds = GroundAOEWarningSeconds * FMath::Max(0.55f, WarningScale);
		const ET66BossAttackProfile VisualProfile = T66ResolveLegacyAttackProfileFromAttackID(AttackRow.AttackID, AttackProfile);
		AOE->ConfigureVisualStyle(VisualProfile, bUseSecondaryTint ? AttackSecondaryColor : AttackPrimaryColor, AttackSecondaryColor);

		AOE->DamageHP = DamageHP;
		RecordBossAttackOwnershipEvent(T66BossAttackEvent_Fired, &AttackRow, AttackRow.OwningPartID, TEXT("LegacyGroundAOEImmediate"));
		T66PlayBossProfileAudioEvent(this, TEXT("Boss.AOE.Warning"), FName(TEXT("Boss.AOE.Warning")), AOE->GetActorLocation());
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
	bDefeated = false;
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

void AT66BossBase::TickSimpleChaseMovement(const FVector& MyLoc, const FVector& PlayerLoc, const bool bRunAway)
{
	FVector ToPlayer = PlayerLoc - MyLoc;
	ToPlayer.Z = 0.f;
	const float Len = ToPlayer.Size();
	if (Len > 10.f)
	{
		ToPlayer /= Len;
		AddMovementInput(bRunAway ? -ToPlayer : ToPlayer, 1.f);
	}
#if !UE_BUILD_SHIPPING
	LastBossMovementAutomationMode = bRunAway ? T66BossMovementMode_ForcedRunAway : T66BossMovementMode_FallbackSimpleChase;
#endif
}

void AT66BossBase::NotifyBossMovementAttackCoordinationStarted(const FT66BossAttackOwnershipData& AttackRow)
{
	ActiveBossMovementAttackID = AttackRow.AttackID;
	ActiveBossMovementAttackPartID = AttackRow.OwningPartID;
	BossMovementAttackCoordinationSecondsRemaining = FMath::Max(BossMovementAttackCoordinationSecondsRemaining, 2.0f);
	BossMovementAttackCoordinationSecondsSinceStart = 0.f;
}

bool AT66BossBase::DoesMovementPatternRequireAttackCoordination(const FT66BossMovementPatternData& PatternRow) const
{
	return !PatternRow.CoordinatedAttackID.IsNone() || !PatternRow.CoordinatedPartID.IsNone();
}

bool AT66BossBase::IsMovementPatternAttackCoordinationActive(const FT66BossMovementPatternData& PatternRow) const
{
	if (!DoesMovementPatternRequireAttackCoordination(PatternRow))
	{
		return true;
	}

	if (BossMovementAttackCoordinationSecondsRemaining <= 0.f)
	{
		return false;
	}

	if (!PatternRow.CoordinatedAttackID.IsNone() && PatternRow.CoordinatedAttackID != ActiveBossMovementAttackID)
	{
		return false;
	}

	if (!PatternRow.CoordinatedPartID.IsNone())
	{
		if (PatternRow.CoordinatedPartID != ActiveBossMovementAttackPartID)
		{
			return false;
		}
		if (!IsBossPartAlive(PatternRow.CoordinatedPartID))
		{
			return false;
		}
	}

	if (PatternRow.CoordinationWindowSeconds > 0.f
		&& BossMovementAttackCoordinationSecondsSinceStart > PatternRow.CoordinationWindowSeconds)
	{
		return false;
	}

	return true;
}

const FT66BossMovementPatternData* AT66BossBase::PickBossMovementPatternRow() const
{
	if (BossMovementProfileID.IsNone())
	{
		return nullptr;
	}

	TArray<const FT66BossMovementPatternData*> AvailableRows;
	TArray<const FT66BossMovementPatternData*> ActiveCoordinatedRows;
	const int32 Phase = GetAttackPhaseIndex();
	for (const FT66BossMovementPatternData& Row : BossMovementPatternRows)
	{
		if (!Row.bEnabled
			|| Row.MovementProfileID != BossMovementProfileID
			|| Row.Weight <= 0.f
			|| Phase < Row.MinPhase
			|| Phase > Row.MaxPhase)
		{
			continue;
		}
		if (DoesMovementPatternRequireAttackCoordination(Row))
		{
			if (IsMovementPatternAttackCoordinationActive(Row))
			{
				ActiveCoordinatedRows.Add(&Row);
			}
			continue;
		}
		AvailableRows.Add(&Row);
	}

	if (ActiveCoordinatedRows.Num() > 0)
	{
		AvailableRows = MoveTemp(ActiveCoordinatedRows);
	}

	if (AvailableRows.Num() <= 0)
	{
		return nullptr;
	}

	float TotalWeight = 0.f;
	for (const FT66BossMovementPatternData* Row : AvailableRows)
	{
		TotalWeight += Row ? FMath::Max(0.f, Row->Weight) : 0.f;
	}

	if (TotalWeight <= KINDA_SMALL_NUMBER)
	{
		return AvailableRows[0];
	}

	float Pick = FMath::FRandRange(0.f, TotalWeight);
	for (const FT66BossMovementPatternData* Row : AvailableRows)
	{
		Pick -= Row ? FMath::Max(0.f, Row->Weight) : 0.f;
		if (Pick <= 0.f)
		{
			return Row;
		}
	}
	return AvailableRows.Last();
}

bool AT66BossBase::TickAuthoredBossMovementPattern(const float DeltaSeconds, const FVector& MyLoc, const FVector& PlayerLoc)
{
	(void)DeltaSeconds;
	const FT66BossMovementPatternData* PatternRow = PickBossMovementPatternRow();
	if (!PatternRow)
	{
		return false;
	}

	FVector ToPlayer = PlayerLoc - MyLoc;
	ToPlayer.Z = 0.f;
	const float Distance = ToPlayer.Size();
	if (Distance <= 10.f)
	{
#if !UE_BUILD_SHIPPING
		LastBossMovementAutomationMode = FName(TEXT("Pattern.Hold"));
#endif
		return true;
	}
	ToPlayer /= Distance;

	const float InputScale = FMath::Clamp(PatternRow->SpeedScale, 0.f, 2.f);
	if (PatternRow->PatternType == T66BossMovementPattern_SimpleChase)
	{
		AddMovementInput(ToPlayer, InputScale);
#if !UE_BUILD_SHIPPING
		LastBossMovementAutomationMode = FName(TEXT("Pattern.SimpleChase"));
#endif
		return true;
	}

	if (PatternRow->PatternType == T66BossMovementPattern_KeepDistance)
	{
		if (Distance > PatternRow->MaxDistance)
		{
			AddMovementInput(ToPlayer, InputScale);
#if !UE_BUILD_SHIPPING
			LastBossMovementAutomationMode = FName(TEXT("Pattern.KeepDistance.Advance"));
#endif
		}
		else if (Distance < PatternRow->MinDistance)
		{
			AddMovementInput(-ToPlayer, InputScale);
#if !UE_BUILD_SHIPPING
			LastBossMovementAutomationMode = FName(TEXT("Pattern.KeepDistance.Retreat"));
#endif
		}
		else
		{
#if !UE_BUILD_SHIPPING
			LastBossMovementAutomationMode = FName(TEXT("Pattern.KeepDistance.Hold"));
#endif
		}
		return true;
	}

	if (PatternRow->PatternType == T66BossMovementPattern_Orbit)
	{
		FVector Direction = FVector::CrossProduct(FVector::UpVector, ToPlayer).GetSafeNormal();
		if (PatternRow->OrbitDirection < 0)
		{
			Direction *= -1.f;
		}
		if (Distance > PatternRow->MaxDistance)
		{
			Direction = (Direction * 0.72f + ToPlayer * 0.55f).GetSafeNormal();
		}
		else if (Distance < PatternRow->MinDistance)
		{
			Direction = (Direction * 0.72f - ToPlayer * 0.55f).GetSafeNormal();
		}
		AddMovementInput(Direction, InputScale);
#if !UE_BUILD_SHIPPING
		LastBossMovementAutomationMode = FName(TEXT("Pattern.Orbit"));
#endif
		return true;
	}

	if (PatternRow->PatternType == T66BossMovementPattern_StrafeBurst)
	{
		FVector Direction = FVector::CrossProduct(FVector::UpVector, ToPlayer).GetSafeNormal();
		if (PatternRow->OrbitDirection < 0)
		{
			Direction *= -1.f;
		}
		if (Distance > PatternRow->MaxDistance)
		{
			Direction = (Direction * 0.82f + ToPlayer * 0.45f).GetSafeNormal();
		}
		else if (Distance < PatternRow->MinDistance)
		{
			Direction = (Direction * 0.82f - ToPlayer * 0.45f).GetSafeNormal();
		}
		AddMovementInput(Direction, InputScale);
#if !UE_BUILD_SHIPPING
		LastBossMovementAutomationMode = FName(TEXT("Pattern.StrafeBurst"));
#endif
		return true;
	}

	if (PatternRow->PatternType == T66BossMovementPattern_RetreatThenCast)
	{
		if (!IsMovementPatternAttackCoordinationActive(*PatternRow))
		{
			return false;
		}
		AddMovementInput(-ToPlayer, InputScale);
#if !UE_BUILD_SHIPPING
		LastBossMovementAutomationMode = FName(TEXT("Pattern.RetreatThenCast.Active"));
#endif
		return true;
	}

	if (PatternRow->PatternType == T66BossMovementPattern_AnchorDuringCast)
	{
		if (!IsMovementPatternAttackCoordinationActive(*PatternRow))
		{
			return false;
		}
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->StopMovementImmediately();
		}
#if !UE_BUILD_SHIPPING
		LastBossMovementAutomationMode = FName(TEXT("Pattern.AnchorDuringCast.Active"));
#endif
		return true;
	}

	if (PatternRow->PatternType == T66BossMovementPattern_Charge)
	{
		if (Distance > FMath::Max(10.f, PatternRow->MinDistance))
		{
			AddMovementInput(ToPlayer, InputScale);
#if !UE_BUILD_SHIPPING
			LastBossMovementAutomationMode = FName(TEXT("Pattern.Charge"));
#endif
		}
		else
		{
#if !UE_BUILD_SHIPPING
			LastBossMovementAutomationMode = FName(TEXT("Pattern.Charge.Hold"));
#endif
		}
		return true;
	}

	UE_LOG(
		LogT66BossMovement,
		Warning,
		TEXT("BossMovementUnsupportedPattern BossID=%s MovementProfileID=%s PatternID=%s PatternType=%s"),
		*BossID.ToString(),
		*BossMovementProfileID.ToString(),
		*PatternRow->PatternID.ToString(),
		*PatternRow->PatternType.ToString());
	return false;
}

void AT66BossBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	DrawCombatDebug();

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
	if (BossMovementAttackCoordinationSecondsRemaining > 0.f)
	{
		BossMovementAttackCoordinationSecondsRemaining = FMath::Max(0.f, BossMovementAttackCoordinationSecondsRemaining - DeltaSeconds);
		BossMovementAttackCoordinationSecondsSinceStart += DeltaSeconds;
		if (BossMovementAttackCoordinationSecondsRemaining <= 0.f)
		{
			ActiveBossMovementAttackID = NAME_None;
			ActiveBossMovementAttackPartID = NAME_None;
			BossMovementAttackCoordinationSecondsSinceStart = 0.f;
		}
	}

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		const float ControlMultiplier = (FreezeSecondsRemaining > 0.f) ? 0.f : MoveSlowMultiplier;
		Move->MaxWalkSpeed = BaseMoveSpeed * ControlMultiplier;
		if (FreezeSecondsRemaining > 0.f || StunSecondsRemaining > 0.f)
		{
			Move->StopMovementImmediately();
#if !UE_BUILD_SHIPPING
			LastBossMovementAutomationMode = T66BossMovementMode_FrozenOrStunned;
#endif
			return;
		}
		if (RootSecondsRemaining > 0.f)
		{
			Move->StopMovementImmediately();
#if !UE_BUILD_SHIPPING
			LastBossMovementAutomationMode = T66BossMovementMode_Rooted;
#endif
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
#if !UE_BUILD_SHIPPING
		LastBossMovementAutomationMode = T66BossMovementMode_Confusion;
#endif
		return;
	}

	if (bRunAway)
	{
		TickSimpleChaseMovement(MyLoc, PlayerLoc, true);
		return;
	}

	if (TickAuthoredBossMovementPattern(DeltaSeconds, MyLoc, PlayerLoc))
	{
		return;
	}

	TickSimpleChaseMovement(MyLoc, PlayerLoc, false);
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
	bDefeated = false;
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

void AT66BossBase::ForceSewerSlimeKingAttackForAutomation(const FName AttackPartID)
{
	if (!IsSewerSlimeKingBoss())
	{
		return;
	}

	if (!bAwakened)
	{
		Awaken();
	}
	if (!bAwakened || CurrentHP <= 0)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireTimerHandle);
		World->GetTimerManager().ClearTimer(AOETimerHandle);
	}
	ClearPendingAttackTimers();
	if (APawn* PlayerPawn = ResolvePlayerPawn())
	{
		FireSewerSlimeKingAttack(PlayerPawn, AttackPartID);
	}
}

#if !UE_BUILD_SHIPPING
void AT66BossBase::ForceBossAttackForAutomation(const FName AttackID, const FName OwningPartID)
{
	if (!bAwakened || CurrentHP <= 0)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireTimerHandle);
		World->GetTimerManager().ClearTimer(AOETimerHandle);
	}
	ClearPendingAttackTimers();

	const FT66BossAttackOwnershipData* AttackRow = FindBossAttackRowByAttackID(AttackID, OwningPartID);
	if (!AttackRow)
	{
		UE_LOG(
			LogT66BossAttackOwnership,
			Warning,
			TEXT("BossAttackOwnershipAutomationMissingRow BossID=%s AttackID=%s OwningPartID=%s"),
			*BossID.ToString(),
			*AttackID.ToString(),
			*OwningPartID.ToString());
		return;
	}

	APawn* PlayerPawn = ResolvePlayerPawn();
	if (T66BossAttackIDStartsWith(AttackID, T66BossAttackIDPrefix_LegacyProjectile))
	{
		FireBossProjectileAttackRow(PlayerPawn, *AttackRow);
		return;
	}

	if (T66BossAttackIDStartsWith(AttackID, T66BossAttackIDPrefix_LegacyGroundAOE))
	{
		FireBossGroundAOEAttackRow(PlayerPawn, *AttackRow);
		return;
	}

	RecordBossAttackOwnershipEvent(T66BossAttackEvent_Suppressed, AttackRow, AttackRow->OwningPartID, TEXT("UnsupportedAutomationAttackID"));
}
#endif

bool AT66BossBase::FireAuthoredBossProjectileAttack(APawn* PlayerPawn)
{
	bool bHasMatchingRows = false;
	FName SuppressedPartID = NAME_None;
	const FT66BossAttackOwnershipData* AttackRow = PickBossAttackRowByPrefix(
		T66BossAttackIDPrefix_LegacyProjectile,
		bHasMatchingRows,
		SuppressedPartID);
	if (AttackRow)
	{
		return FireBossProjectileAttackRow(PlayerPawn, *AttackRow);
	}

	if (bHasMatchingRows)
	{
		UE_LOG(
			LogT66BossAttackOwnership,
			Display,
			TEXT("BossAttackOwnershipSuppressed BossID=%s Channel=LegacyProjectile DeadPartID=%s Reason=NoSelectableAuthoredRows"),
			*BossID.ToString(),
			*SuppressedPartID.ToString());
		return true;
	}

	return false;
}

bool AT66BossBase::FireBossProjectileAttackDefinitionRows(
	APawn* PlayerPawn,
	const FT66BossAttackOwnershipData& AttackRow,
	const FVector& TargetLocation,
	const int32 Phase,
	const FVector& PlanarToTarget,
	const FVector& Side)
{
	if (!PlayerPawn)
	{
		return false;
	}

	UWorld* World = GetWorld();
	UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	if (!T66GI)
	{
		return false;
	}

	TArray<FT66BossAttackDefinitionData> DefinitionRows;
	T66GI->GetBossAttackDefinitionRows(AttackRow.AttackID, Phase, DefinitionRows);
	if (DefinitionRows.Num() <= 0)
	{
		return false;
	}

	const FVector SafePlanarToTarget = PlanarToTarget.IsNearlyZero() ? GetActorForwardVector() : PlanarToTarget;
	const float TargetYawDegrees = SafePlanarToTarget.Rotation().Yaw;
	int32 ScheduledRows = 0;
	int32 UnsupportedRows = 0;

	for (const FT66BossAttackDefinitionData& DefinitionRow : DefinitionRows)
	{
		const FName VisualProfileID = DefinitionRow.ProjectileVisualProfileID;
		const float SpeedScale = FMath::Max(0.1f, DefinitionRow.SpeedScale);
		const float VisualScaleMultiplier = FMath::Max(0.01f, DefinitionRow.VisualScaleMultiplier);
		const ET66AttackCategory ProjectileCategory = DefinitionRow.ProjectileCategory;
		const TSoftObjectPtr<UStaticMesh> ProjectileMesh = DefinitionRow.ProjectileMesh;
		const float ProjectileMeshScale = FMath::Max(0.05f, DefinitionRow.ProjectileMeshScale);
		const FVector AuthoredOffset(DefinitionRow.SpawnOffsetX, DefinitionRow.SpawnOffsetY, DefinitionRow.SpawnOffsetZ);

		if (DefinitionRow.PatternType == T66BossAttackPattern_SingleShot)
		{
			QueueProjectileShotTowardsForAttackRow(
				AttackRow,
				TargetLocation,
				FMath::Max(0.f, DefinitionRow.InitialDelaySeconds),
				DefinitionRow.YawOffsetDegrees,
				SpeedScale,
				Side * DefinitionRow.SideOffsetDistance + AuthoredOffset,
				DefinitionRow.bUseSecondaryTint,
				VisualProfileID,
				VisualScaleMultiplier,
				ProjectileCategory,
				ProjectileMesh,
				ProjectileMeshScale);
			++ScheduledRows;
			continue;
		}

		if (DefinitionRow.PatternType == T66BossAttackPattern_FanBurst)
		{
			QueueProjectileFanBurstForAttackRow(
				AttackRow,
				TargetLocation,
				FMath::Max(1, DefinitionRow.ShotCount),
				DefinitionRow.SpreadDegrees,
				FMath::Max(0.f, DefinitionRow.DelayStepSeconds),
				SpeedScale,
				FMath::Max(0.f, DefinitionRow.InitialDelaySeconds),
				DefinitionRow.SideOffsetDistance,
				DefinitionRow.bUseSecondaryTint,
				VisualProfileID,
				VisualScaleMultiplier,
				ProjectileCategory,
				ProjectileMesh,
				ProjectileMeshScale);
			++ScheduledRows;
			continue;
		}

		if (DefinitionRow.PatternType == T66BossAttackPattern_RadialBurst)
		{
			float StartAngleDegrees = DefinitionRow.bRandomStartAngle
				? FMath::FRandRange(0.f, 360.f)
				: DefinitionRow.StartAngleDegrees;
			if (DefinitionRow.bStartAngleRelativeToTarget)
			{
				StartAngleDegrees += TargetYawDegrees;
			}

			QueueRadialBurstForAttackRow(
				AttackRow,
				FMath::Max(1, DefinitionRow.ShotCount),
				FMath::Max(0.f, DefinitionRow.DelayStepSeconds),
				StartAngleDegrees,
				SpeedScale,
				FMath::Max(0.f, DefinitionRow.InitialDelaySeconds),
				DefinitionRow.bUseSecondaryTint,
				VisualProfileID,
				VisualScaleMultiplier,
				ProjectileCategory,
				ProjectileMesh,
				ProjectileMeshScale);
			++ScheduledRows;
			continue;
		}

		++UnsupportedRows;
		UE_LOG(
			LogT66BossAttackOwnership,
			Warning,
			TEXT("BossAttackDefinitionUnsupportedPattern BossID=%s AttackID=%s DefinitionRowID=%s PatternType=%s"),
			*BossID.ToString(),
			*AttackRow.AttackID.ToString(),
			*DefinitionRow.DefinitionRowID.ToString(),
			*DefinitionRow.PatternType.ToString());
	}

	if (ScheduledRows <= 0)
	{
		return false;
	}

	RecordBossAttackOwnershipEvent(T66BossAttackEvent_Queued, &AttackRow, AttackRow.OwningPartID, TEXT("BossAttackDefinitionRows"));
	NotifyBossMovementAttackCoordinationStarted(AttackRow);
	UE_LOG(
		LogT66BossAttackOwnership,
		Display,
		TEXT("BossAttackDefinitionQueued BossID=%s AttackID=%s Phase=%d DefinitionRows=%d ScheduledRows=%d UnsupportedRows=%d Source=DT_BossAttackDefinitions"),
		*BossID.ToString(),
		*AttackRow.AttackID.ToString(),
		Phase,
		DefinitionRows.Num(),
		ScheduledRows,
		UnsupportedRows);
	return true;
}

bool AT66BossBase::FireBossProjectileAttackRow(APawn* PlayerPawn, const FT66BossAttackOwnershipData& AttackRow)
{
	if (!PlayerPawn)
	{
		return false;
	}

	FName DeadPartID = NAME_None;
	if (!AreBossAttackPartsAlive(AttackRow, DeadPartID))
	{
		RecordBossAttackOwnershipEvent(T66BossAttackEvent_Suppressed, &AttackRow, DeadPartID, TEXT("LegacyProjectileSelection"));
		return true;
	}

	const FVector TargetLocation = PlayerPawn->GetActorLocation();
	const FVector PlanarToTarget = (TargetLocation - GetActorLocation()).GetSafeNormal2D();
	const FVector Side = T66ResolvePlanarRightVector(PlanarToTarget.IsNearlyZero() ? GetActorForwardVector() : PlanarToTarget);
	const int32 Phase = GetAttackPhaseIndex();
	const float PhaseScale = 1.f + 0.08f * static_cast<float>(Phase);
	const ET66BossAttackProfile AuthoredProfile = T66ResolveLegacyAttackProfileFromAttackID(AttackRow.AttackID, AttackProfile);

	if (FireBossProjectileAttackDefinitionRows(PlayerPawn, AttackRow, TargetLocation, Phase, PlanarToTarget, Side))
	{
		return true;
	}

	RecordBossAttackOwnershipEvent(T66BossAttackEvent_Queued, &AttackRow, AttackRow.OwningPartID, TEXT("LegacyProjectileAttackRow"));
	NotifyBossMovementAttackCoordinationStarted(AttackRow);

	switch (AuthoredProfile)
	{
	case ET66BossAttackProfile::Sharpshooter:
		QueueProjectileFanBurstForAttackRow(AttackRow, TargetLocation, Phase >= 1 ? 5 : 3, Phase == 0 ? 8.f : 12.f, 0.04f, 1.18f + 0.06f * static_cast<float>(Phase), 0.f, 18.f, true);
		QueueProjectileShotTowardsForAttackRow(AttackRow, TargetLocation, 0.16f, 0.f, 1.36f + 0.08f * static_cast<float>(Phase), FVector::ZeroVector, false);
		if (Phase >= 2)
		{
			QueueProjectileShotTowardsForAttackRow(AttackRow, TargetLocation, 0.28f, -6.f, 1.24f, FVector::ZeroVector, true);
			QueueProjectileShotTowardsForAttackRow(AttackRow, TargetLocation, 0.32f, 6.f, 1.24f, FVector::ZeroVector, true);
		}
		return true;

	case ET66BossAttackProfile::Juggernaut:
		QueueProjectileFanBurstForAttackRow(AttackRow, TargetLocation, Phase == 0 ? 5 : 7, 30.f + 4.f * static_cast<float>(Phase), 0.05f, 0.94f + 0.05f * static_cast<float>(Phase), 0.f, 46.f, false);
		QueueRadialBurstForAttackRow(AttackRow, Phase == 0 ? 6 : (Phase == 1 ? 8 : 10), 0.025f, FMath::FRandRange(0.f, 360.f), 0.76f + 0.05f * static_cast<float>(Phase), 0.18f, true);
		return true;

	case ET66BossAttackProfile::Duelist:
		QueueProjectileShotTowardsForAttackRow(AttackRow, TargetLocation, 0.f, -10.f, 1.12f * PhaseScale, -Side * 52.f, false);
		QueueProjectileShotTowardsForAttackRow(AttackRow, TargetLocation, 0.05f, 10.f, 1.12f * PhaseScale, Side * 52.f, true);
		if (Phase >= 1)
		{
			QueueProjectileFanBurstForAttackRow(AttackRow, TargetLocation, 4 + Phase, 20.f, 0.05f, 1.04f + 0.05f * static_cast<float>(Phase), 0.18f, 26.f, true);
		}
		if (Phase >= 2)
		{
			QueueRadialBurstForAttackRow(AttackRow, 6, 0.03f, PlanarToTarget.Rotation().Yaw + 30.f, 0.96f, 0.34f, true);
		}
		return true;

	case ET66BossAttackProfile::Gambler:
		QueueRadialBurstForAttackRow(AttackRow, 6 + Phase * 2, 0.02f, FMath::FRandRange(0.f, 360.f), 0.86f + 0.04f * static_cast<float>(Phase), 0.f, true);
		QueueProjectileFanBurstForAttackRow(AttackRow, TargetLocation, 3 + Phase, 16.f + 2.f * static_cast<float>(Phase), 0.05f, 1.10f + 0.05f * static_cast<float>(Phase), 0.16f, 20.f, false);
		if (Phase >= 2)
		{
			QueueProjectileShotTowardsForAttackRow(AttackRow, TargetLocation, 0.32f, FMath::FRandRange(-12.f, 12.f), 1.35f, FVector::ZeroVector, true);
		}
		return true;

	case ET66BossAttackProfile::Balanced:
	default:
		QueueProjectileFanBurstForAttackRow(AttackRow, TargetLocation, Phase == 0 ? 3 : (Phase == 1 ? 5 : 6), 14.f + 4.f * static_cast<float>(Phase), 0.06f, 1.00f + 0.06f * static_cast<float>(Phase), 0.f, 36.f, false);
		if (Phase >= 1)
		{
			QueueProjectileShotTowardsForAttackRow(AttackRow, TargetLocation, 0.22f, 0.f, 1.18f, FVector::ZeroVector, true);
		}
		if (Phase >= 2)
		{
			QueueProjectileFanBurstForAttackRow(AttackRow, TargetLocation, 3, 10.f, 0.05f, 1.24f, 0.34f, 0.f, true);
		}
		return true;
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

	if (IsSewerSlimeKingBoss())
	{
		FireSewerSlimeKingAttack(PlayerPawn);
		return;
	}

	if (FireAuthoredBossProjectileAttack(PlayerPawn))
	{
		return;
	}

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

	if (bZeroDamageUnkillable)
	{
		UWorld* World = GetWorld();
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		if (UT66FloatingCombatTextSubsystem* FloatingText = GI ? GI->GetSubsystem<UT66FloatingCombatTextSubsystem>() : nullptr)
		{
			FloatingText->ShowDamageNumber(this, 0, EventType);
		}
		UE_LOG(LogTemp, Verbose, TEXT("[T66Endgame] ZeroDamageBossHit BossID=%s Source=%s Reason=%s"),
			*BossID.ToString(),
			*SourceID.ToString(),
			*ZeroDamageUnkillableReason.ToString());
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
	const bool bPartWasAlive = Part.CurrentHP > 0;
	Part.CurrentHP = FMath::Max(0, Part.CurrentHP - ReducedDamage);
	CurrentHP = 0;
	for (const FT66BossPartRuntimeState& RuntimePart : BossPartStates)
	{
		CurrentHP += RuntimePart.CurrentHP;
	}
	CurrentHP = FMath::Clamp(CurrentHP, 0, MaxHP);
	if (bPartWasAlive && Part.CurrentHP <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Boss part destroyed: BossID=%s PartID=%s"), *BossID.ToString(), *Part.PartID.ToString());
		RefreshCombatHitZoneState();
	}
	PushBossPartStateToRunState();

	if (CurrentHP <= 0)
	{
		Die();
		return true;
	}

	UT66AudioSubsystem::PlayEventAtActorFromWorldContext(this, FName(TEXT("Combat.Hit.Boss")), this);
	return false;
}

void AT66BossBase::SetZeroDamageUnkillable(const bool bEnabled, const FName Reason)
{
	bZeroDamageUnkillable = bEnabled;
	ZeroDamageUnkillableReason = bEnabled ? Reason : NAME_None;
}

bool AT66BossBase::IsCombatTargetable() const
{
	return IsValid(this)
		&& !IsActorBeingDestroyed()
		&& bAwakened
		&& IsAlive();
}

float AT66BossBase::GetEffectiveArmor() const
{
	return FMath::Clamp(Armor - ArmorDebuffAmount, -0.5f, 0.95f);
}

bool AT66BossBase::FireAuthoredBossGroundAOE(APawn* PlayerPawn)
{
	bool bHasMatchingRows = false;
	FName SuppressedPartID = NAME_None;
	const FT66BossAttackOwnershipData* AttackRow = PickBossAttackRowByPrefix(
		T66BossAttackIDPrefix_LegacyGroundAOE,
		bHasMatchingRows,
		SuppressedPartID);
	if (AttackRow)
	{
		return FireBossGroundAOEAttackRow(PlayerPawn, *AttackRow);
	}

	if (bHasMatchingRows)
	{
		UE_LOG(
			LogT66BossAttackOwnership,
			Display,
			TEXT("BossAttackOwnershipSuppressed BossID=%s Channel=LegacyGroundAOE DeadPartID=%s Reason=NoSelectableAuthoredRows"),
			*BossID.ToString(),
			*SuppressedPartID.ToString());
		return true;
	}

	return false;
}

bool AT66BossBase::FireBossGroundAOEAttackRow(APawn* PlayerPawn, const FT66BossAttackOwnershipData& AttackRow)
{
	if (!PlayerPawn)
	{
		return false;
	}

	FName DeadPartID = NAME_None;
	if (!AreBossAttackPartsAlive(AttackRow, DeadPartID))
	{
		RecordBossAttackOwnershipEvent(T66BossAttackEvent_Suppressed, &AttackRow, DeadPartID, TEXT("LegacyGroundAOESelection"));
		return true;
	}

	const FVector TargetLoc = ResolveGroundLocation(PlayerPawn->GetActorLocation());
	const FVector Forward = (TargetLoc - GetActorLocation()).GetSafeNormal2D();
	const FVector Right = T66ResolvePlanarRightVector(Forward.IsNearlyZero() ? GetActorForwardVector() : Forward);
	const int32 Phase = GetAttackPhaseIndex();
	const ET66BossAttackProfile AuthoredProfile = T66ResolveLegacyAttackProfileFromAttackID(AttackRow.AttackID, AttackProfile);

	RecordBossAttackOwnershipEvent(T66BossAttackEvent_Queued, &AttackRow, AttackRow.OwningPartID, TEXT("LegacyGroundAOEAttackRow"));
	NotifyBossMovementAttackCoordinationStarted(AttackRow);

	switch (AuthoredProfile)
	{
	case ET66BossAttackProfile::Sharpshooter:
		SpawnGroundAOEAtLocationForAttackRow(AttackRow, TargetLoc, 0.82f, 0.90f, false);
		SpawnGroundAOEAtLocationForAttackRow(AttackRow, TargetLoc + Right * GroundAOERadius * 0.95f, 0.72f, 0.82f, true);
		if (Phase >= 1)
		{
			SpawnGroundAOEAtLocationForAttackRow(AttackRow, TargetLoc - Right * GroundAOERadius * 0.95f, 0.72f, 0.82f, true);
		}
		if (Phase >= 2)
		{
			SpawnGroundAOEAtLocationForAttackRow(AttackRow, TargetLoc + Forward * GroundAOERadius * 0.95f, 0.78f, 0.76f, false);
		}
		return true;

	case ET66BossAttackProfile::Juggernaut:
		SpawnGroundAOEAtLocationForAttackRow(AttackRow, GetActorLocation(), 1.18f + 0.10f * static_cast<float>(Phase), 1.00f, false);
		if (Phase >= 1)
		{
			SpawnGroundAOEAtLocationForAttackRow(AttackRow, TargetLoc, 1.00f, 0.86f, true);
		}
		if (Phase >= 2)
		{
			SpawnGroundAOEAtLocationForAttackRow(AttackRow, TargetLoc + Right * GroundAOERadius, 0.84f, 0.78f, true);
			SpawnGroundAOEAtLocationForAttackRow(AttackRow, TargetLoc - Right * GroundAOERadius, 0.84f, 0.78f, true);
		}
		return true;

	case ET66BossAttackProfile::Duelist:
		SpawnGroundAOEAtLocationForAttackRow(AttackRow, TargetLoc + Right * GroundAOERadius * 0.7f, 0.76f, 0.82f, false);
		SpawnGroundAOEAtLocationForAttackRow(AttackRow, TargetLoc - Right * GroundAOERadius * 0.7f, 0.76f, 0.82f, true);
		if (Phase >= 1)
		{
			SpawnGroundAOEAtLocationForAttackRow(AttackRow, TargetLoc + Forward * GroundAOERadius * 0.82f, 0.72f, 0.76f, true);
		}
		if (Phase >= 2)
		{
			SpawnGroundAOEAtLocationForAttackRow(AttackRow, TargetLoc - Forward * GroundAOERadius * 0.82f, 0.72f, 0.76f, false);
		}
		return true;

	case ET66BossAttackProfile::Gambler:
	{
		const int32 SpotCount = Phase == 0 ? 3 : (Phase == 1 ? 5 : 6);
		const float RandomStart = FMath::FRandRange(0.f, 360.f);
		for (int32 Index = 0; Index < SpotCount; ++Index)
		{
			const float Angle = RandomStart + (360.f / static_cast<float>(SpotCount)) * static_cast<float>(Index);
			const FVector Offset = FRotator(0.f, Angle, 0.f).Vector() * (GroundAOERadius * (Phase >= 2 ? 0.92f : 0.72f));
			SpawnGroundAOEAtLocationForAttackRow(AttackRow, TargetLoc + Offset, 0.70f + 0.05f * static_cast<float>(Phase), 0.82f, (Index % 2) == 1);
		}
		if (Phase >= 2)
		{
			SpawnGroundAOEAtLocationForAttackRow(AttackRow, TargetLoc, 0.88f, 0.76f, true);
		}
		return true;
	}

	case ET66BossAttackProfile::Balanced:
	default:
		SpawnGroundAOEAtLocationForAttackRow(AttackRow, TargetLoc, 1.00f, 1.00f, false);
		if (Phase >= 1)
		{
			SpawnGroundAOEAtLocationForAttackRow(AttackRow, TargetLoc + Right * GroundAOERadius * 0.85f, 0.82f, 0.88f, true);
			SpawnGroundAOEAtLocationForAttackRow(AttackRow, TargetLoc - Right * GroundAOERadius * 0.85f, 0.82f, 0.88f, true);
		}
		if (Phase >= 2)
		{
			SpawnGroundAOEAtLocationForAttackRow(AttackRow, TargetLoc + Forward * GroundAOERadius * 0.9f, 0.78f, 0.80f, false);
		}
		return true;
	}
}

void AT66BossBase::SpawnGroundAOE()
{
	if (!bAwakened || CurrentHP <= 0 || StunSecondsRemaining > 0.f || FreezeSecondsRemaining > 0.f) return;

	UWorld* World = GetWorld();
	if (!World) return;

	APawn* PlayerPawn = ResolvePlayerPawn();
	if (!PlayerPawn) return;

	if (FireAuthoredBossGroundAOE(PlayerPawn))
	{
		return;
	}

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
	bDefeated = true;
	bAwakened = false;
	CurrentHP = 0;
	for (FT66BossPartRuntimeState& Part : BossPartStates)
	{
		Part.CurrentHP = 0;
	}
	RefreshCombatHitZoneState();
	if (World)
	{
		ClearPendingAttackTimers();
		World->GetTimerManager().ClearTimer(FireTimerHandle);
		World->GetTimerManager().ClearTimer(AOETimerHandle);
		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->UnregisterBoss(this);
		}
		UT66CombatComponent::SpawnDeathBurstAtLocation(World, GetActorLocation(), 32, 120.f);
		UT66AudioSubsystem::PlayEventAtActorFromWorldContext(this, FName(TEXT("Combat.Boss.Death")), this);
	}

	if (AT66GameMode* GM = World ? World->GetAuthGameMode<AT66GameMode>() : nullptr)
	{
		GM->HandleBossDefeated(this);
	}

	Destroy();
}

