// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66HeroAOEAttackVFX.h"
#include "Gameplay/T66HeroBounceAttackVFX.h"
#include "Gameplay/T66HeroDOTAttackVFX.h"
#include "Gameplay/T66FireIdolAttackVFX.h"
#include "Gameplay/T66HeroOneAttackVFX.h"
#include "Gameplay/T66IdolProcVFX.h"
#include "Gameplay/T66IdolVolumeAttackVFX.h"
#include "Core/T66GameInstance.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Data/T66DataTypes.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	static TAutoConsoleVariable<int32> CVarT66VFXHeroOnePierceEnabled(
		TEXT("T66.VFX.Hero1PierceExample"),
		1,
		TEXT("Use the stage-1 Example VFX path for Hero_1 pierce attacks. 0 falls back to the legacy pixel line."));

	static TAutoConsoleVariable<int32> CVarT66VFXHeroOneVerbose(
		TEXT("T66.VFX.Hero1Verbose"),
		1,
		TEXT("Emit detailed logs for the Hero_1 stage-1 Example VFX path."));

	static TAutoConsoleVariable<int32> CVarT66VFXHeroPierceExample(
		TEXT("T66.VFX.HeroPierceExample"),
		1,
		TEXT("Use the stage-2 Example VFX path for non-Hero_1 hero pierce attacks. 0 falls back to the legacy hero pierce pixels."));

	static TAutoConsoleVariable<int32> CVarT66VFXHeroPierceVerbose(
		TEXT("T66.VFX.HeroPierceVerbose"),
		1,
		TEXT("Emit detailed logs for the stage-2 generic hero pierce Example VFX path."));

	static TAutoConsoleVariable<int32> CVarT66VFXIdolPierceExample(
		TEXT("T66.VFX.IdolPierceExample"),
		0,
		TEXT("Use the stage-3 generic debug VFX path for idol pierce attacks. 0 uses the idol-specific runtime actor plus pixel overlay."));

	static TAutoConsoleVariable<int32> CVarT66VFXIdolPierceVerbose(
		TEXT("T66.VFX.IdolPierceVerbose"),
		1,
		TEXT("Emit detailed logs for the stage-3 idol pierce Example VFX path."));

	static TAutoConsoleVariable<int32> CVarT66VFXHeroAOEExample(
		TEXT("T66.VFX.HeroAOEExample"),
		1,
		TEXT("Use the stage-4 Example VFX path for hero AOE attacks. 0 falls back to the legacy slash pixel patterns."));

	static TAutoConsoleVariable<int32> CVarT66VFXHeroAOEVerbose(
		TEXT("T66.VFX.HeroAOEVerbose"),
		1,
		TEXT("Emit detailed logs for the stage-4 hero AOE Example VFX path."));

	static TAutoConsoleVariable<int32> CVarT66VFXHeroBounceExample(
		TEXT("T66.VFX.HeroBounceExample"),
		1,
		TEXT("Use the stage-5 Example VFX path for hero bounce attacks. 0 falls back to the legacy bounce pixel patterns."));

	static TAutoConsoleVariable<int32> CVarT66VFXHeroBounceVerbose(
		TEXT("T66.VFX.HeroBounceVerbose"),
		1,
		TEXT("Emit detailed logs for the stage-5 hero bounce Example VFX path."));

	static TAutoConsoleVariable<int32> CVarT66VFXHeroDOTExample(
		TEXT("T66.VFX.HeroDOTExample"),
		1,
		TEXT("Use the stage-6 Example VFX path for hero DOT attacks. 0 falls back to the legacy DOT pixels."));

	static TAutoConsoleVariable<int32> CVarT66VFXHeroDOTVerbose(
		TEXT("T66.VFX.HeroDOTVerbose"),
		1,
		TEXT("Emit detailed logs for the stage-6 hero DOT Example VFX path."));

	static TAutoConsoleVariable<int32> CVarT66VFXIdolAOEExample(
		TEXT("T66.VFX.IdolAOEExample"),
		0,
		TEXT("Use the stage-7 generic debug VFX path for idol AOE attacks. 0 prefers fire/Zibra or idol-specific runtime actors."));

	static TAutoConsoleVariable<int32> CVarT66VFXIdolAOEVerbose(
		TEXT("T66.VFX.IdolAOEVerbose"),
		1,
		TEXT("Emit detailed logs for the stage-7 idol AOE Example VFX path."));

	static TAutoConsoleVariable<int32> CVarT66VFXFireIdolCandidate(
		TEXT("T66.VFX.FireIdolCandidate"),
		static_cast<int32>(ET66FireIdolTestCandidate::ZibraShockwave),
		TEXT("Select the non-flipbook Fire idol gameplay-test candidate. 0=Blender3D, 1=ZibraShockwave."));

	static TAutoConsoleVariable<int32> CVarT66VFXIdolBounceExample(
		TEXT("T66.VFX.IdolBounceExample"),
		0,
		TEXT("Use the stage-8 generic debug VFX path for idol bounce attacks. 0 uses the idol-specific runtime actor and pixel chain."));

	static TAutoConsoleVariable<int32> CVarT66VFXIdolBounceVerbose(
		TEXT("T66.VFX.IdolBounceVerbose"),
		1,
		TEXT("Emit detailed logs for the stage-8 idol bounce Example VFX path."));

	static TAutoConsoleVariable<int32> CVarT66VFXIdolDOTExample(
		TEXT("T66.VFX.IdolDOTExample"),
		0,
		TEXT("Use the stage-9 generic debug VFX path for idol DOT attacks. 0 prefers volumetric or idol-specific runtime actors."));

	static TAutoConsoleVariable<int32> CVarT66VFXIdolDOTVerbose(
		TEXT("T66.VFX.IdolDOTVerbose"),
		1,
		TEXT("Emit detailed logs for the stage-9 idol DOT Example VFX path."));

	static TAutoConsoleVariable<int32> CVarT66VFXIdolLegacyPixels(
		TEXT("T66.VFX.IdolLegacyPixels"),
		0,
		TEXT("Keep the old pixel-overlay idol embellishments on top of the replacement idol actors. 0 disables them so the replacement visuals stand alone."));

	const TCHAR* HeroOneStreakMaterialPath = TEXT("/Game/VFX/Hero1/MI_Hero1_Attack_Streak.MI_Hero1_Attack_Streak");
	const TCHAR* HeroOneImpactMaterialPath = TEXT("/Game/VFX/Hero1/MI_Hero1_Attack_Impact.MI_Hero1_Attack_Impact");
	int32 GHeroOneStage1RequestSerial = 0;
	int32 GHeroPierceStage2RequestSerial = 0;
	int32 GIdolPierceStage3RequestSerial = 0;
	int32 GHeroAOEStage4RequestSerial = 0;
	int32 GHeroBounceStage5RequestSerial = 0;
	int32 GHeroDOTStage6RequestSerial = 0;
	int32 GIdolAOEStage7RequestSerial = 0;
	int32 GIdolBounceStage8RequestSerial = 0;
	int32 GIdolDOTStage9RequestSerial = 0;

	const TCHAR* GetT66AttackCategoryName(const ET66AttackCategory Category)
	{
		switch (Category)
		{
		case ET66AttackCategory::Pierce: return TEXT("Pierce");
		case ET66AttackCategory::AOE:    return TEXT("AOE");
		case ET66AttackCategory::Bounce: return TEXT("Bounce");
		case ET66AttackCategory::DOT:    return TEXT("DOT");
		default:                         return TEXT("Unknown");
		}
	}

	const TCHAR* GetT66ItemRarityName(const ET66ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black:  return TEXT("Black");
		case ET66ItemRarity::Red:    return TEXT("Red");
		case ET66ItemRarity::Yellow: return TEXT("Yellow");
		case ET66ItemRarity::White:  return TEXT("White");
		default:                     return TEXT("Unknown");
		}
	}

	FVector ResolveGroundAnchor(UWorld* World, const FVector& ApproxLocation, const AActor* IgnoreActor)
	{
		if (!World)
		{
			return ApproxLocation;
		}

		FCollisionQueryParams Params(SCENE_QUERY_STAT(T66IdolVFXGroundAnchor), false, IgnoreActor);
		FHitResult Hit;
		const FVector Start = ApproxLocation + FVector(0.f, 0.f, 400.f);
		const FVector End = ApproxLocation - FVector(0.f, 0.f, 1400.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params) ||
			World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			return Hit.ImpactPoint;
		}

		return ApproxLocation;
	}

	FVector ResolveProcVFXAnchor(AActor* TargetActor, UWorld* World)
	{
		if (!TargetActor)
		{
			return FVector::ZeroVector;
		}

		FVector ApproxLocation = TargetActor->GetActorLocation();
		if (const UCapsuleComponent* Capsule = TargetActor->FindComponentByClass<UCapsuleComponent>())
		{
			ApproxLocation.Z -= Capsule->GetScaledCapsuleHalfHeight();
		}

		return ResolveGroundAnchor(World, ApproxLocation, TargetActor);
	}

	FLinearColor BuildExamplePierceTint(const FLinearColor& BaseColor)
	{
		const FLinearColor ClampedColor(
			FMath::Clamp(BaseColor.R, 0.f, 1.f),
			FMath::Clamp(BaseColor.G, 0.f, 1.f),
			FMath::Clamp(BaseColor.B, 0.f, 1.f),
			1.f);

		return FLinearColor(
			FMath::Lerp(ClampedColor.R, 1.0f, 0.42f),
			FMath::Lerp(ClampedColor.G, 0.96f, 0.38f),
			FMath::Lerp(ClampedColor.B, 0.92f, 0.34f),
			1.f);
	}

	float GetPierceRarityGlowBoost(const ET66ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Red:    return 0.10f;
		case ET66ItemRarity::Yellow: return 0.18f;
		case ET66ItemRarity::White:  return 0.26f;
		case ET66ItemRarity::Black:
		default:                     return 0.0f;
		}
	}

	void BuildIdolExamplePiercePalette(
		const FLinearColor& IdolColor,
		const ET66ItemRarity Rarity,
		FLinearColor& OutTintColor,
		FLinearColor& OutPrimaryColor,
		FLinearColor& OutSecondaryColor,
		FLinearColor& OutOutlineColor,
		float& OutGlowStrength)
	{
		const FLinearColor ClampedColor(
			FMath::Clamp(IdolColor.R, 0.f, 1.f),
			FMath::Clamp(IdolColor.G, 0.f, 1.f),
			FMath::Clamp(IdolColor.B, 0.f, 1.f),
			1.f);

		FLinearColor IdolHSV = ClampedColor.LinearRGBToHSV();
		IdolHSV.G = FMath::Clamp(FMath::Max(IdolHSV.G * 1.55f, 0.72f), 0.f, 1.f);
		IdolHSV.B = FMath::Clamp(FMath::Max(IdolHSV.B * 1.12f, 0.82f), 0.f, 1.f);

		const FLinearColor VividPrimary = IdolHSV.HSVToLinearRGB();
		const FLinearColor BrightTint = FLinearColor::LerpUsingHSV(VividPrimary, FLinearColor::White, 0.12f);
		const FLinearColor DeepSecondary = FLinearColor::LerpUsingHSV(VividPrimary, FLinearColor::Black, 0.58f);
		const FLinearColor DeepOutline = FLinearColor::LerpUsingHSV(DeepSecondary, FLinearColor::Black, 0.68f);

		OutTintColor = BrightTint;
		OutPrimaryColor = VividPrimary;
		OutSecondaryColor = DeepSecondary;
		OutOutlineColor = DeepOutline;
		OutGlowStrength = 1.90f + GetPierceRarityGlowBoost(Rarity);
	}

	bool IsExamplePierceVFXReady(const int32 RequestId, const bool bLogResult)
	{
		static TWeakObjectPtr<UMaterialInterface> CachedStreakMaterial;
		static TWeakObjectPtr<UMaterialInterface> CachedImpactMaterial;

		UMaterialInterface* StreakMaterial = CachedStreakMaterial.Get();
		if (!StreakMaterial)
		{
			StreakMaterial = LoadObject<UMaterialInterface>(nullptr, HeroOneStreakMaterialPath);
			CachedStreakMaterial = StreakMaterial;
		}

		UMaterialInterface* ImpactMaterial = CachedImpactMaterial.Get();
		if (!ImpactMaterial)
		{
			ImpactMaterial = LoadObject<UMaterialInterface>(nullptr, HeroOneImpactMaterialPath);
			CachedImpactMaterial = ImpactMaterial;
		}

		const bool bReady = (StreakMaterial != nullptr) && (ImpactMaterial != nullptr);
		if (bLogResult && (CVarT66VFXHeroOneVerbose.GetValueOnGameThread() != 0 || CVarT66VFXHeroPierceVerbose.GetValueOnGameThread() != 0 || CVarT66VFXIdolPierceVerbose.GetValueOnGameThread() != 0))
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("[ATTACK VFX][ExamplePierce] AssetPreflight Req=%d Ready=%d Streak=%s Impact=%s StreakPath=%s ImpactPath=%s"),
				RequestId,
				bReady ? 1 : 0,
				StreakMaterial ? TEXT("OK") : TEXT("MISSING"),
				ImpactMaterial ? TEXT("OK") : TEXT("MISSING"),
				HeroOneStreakMaterialPath,
				HeroOneImpactMaterialPath);
		}

		return bReady;
	}

	bool IsExampleAOEVFXReady(const int32 RequestId, const bool bLogResult, const FName EffectSourceID = NAME_None)
	{
		static TWeakObjectPtr<UMaterialInterface> CachedImpactMaterial;
		UMaterialInterface* ImpactMaterial = CachedImpactMaterial.Get();
		if (!ImpactMaterial)
		{
			ImpactMaterial = LoadObject<UMaterialInterface>(nullptr, HeroOneImpactMaterialPath);
			CachedImpactMaterial = ImpactMaterial;
		}

		const bool bReady = ImpactMaterial != nullptr;
		if (bLogResult && (CVarT66VFXHeroAOEVerbose.GetValueOnGameThread() != 0 || CVarT66VFXIdolAOEVerbose.GetValueOnGameThread() != 0))
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("[ATTACK VFX][ExampleAOE] AssetPreflight Req=%d Source=%s Ready=%d Impact=%s ImpactPath=%s"),
				RequestId,
				EffectSourceID.IsNone() ? TEXT("DefaultAOE") : *EffectSourceID.ToString(),
				bReady ? 1 : 0,
				ImpactMaterial ? TEXT("OK") : TEXT("MISSING"),
				HeroOneImpactMaterialPath);
		}

		return bReady;
	}

	ET66FireIdolTestCandidate GetConfiguredFireIdolCandidate(int32& OutRawValue, bool& bOutWasClamped)
	{
		OutRawValue = CVarT66VFXFireIdolCandidate.GetValueOnGameThread();
		return AT66FireIdolAttackVFX::SanitizeCandidate(OutRawValue, bOutWasClamped);
	}

	bool TrySpawnIdolVolumeEffect(
		UWorld* World,
		AActor* OwnerActor,
		const FName IdolID,
		const ET66AttackCategory Category,
		AActor* FollowTarget,
		const FVector& Location,
		const float Radius,
		const float Duration,
		const FLinearColor& IdolColor,
		const int32 RequestId)
	{
		if (!World || !AT66IdolVolumeAttackVFX::ShouldUseVolume(IdolID, Category))
		{
			return false;
		}

		FString BindingSummary;
		FString FailureReason;
		const bool bReady = AT66IdolVolumeAttackVFX::Preflight(IdolID, Category, BindingSummary, FailureReason);
		if (bReady)
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("[IDOL VOLUME VFX] Preflight Req=%d Idol=%s Category=%s Success=1 FailurePoint=None Fallback=None Details=%s"),
				RequestId,
				*IdolID.ToString(),
				Category == ET66AttackCategory::DOT ? TEXT("DOT") : TEXT("AOE"),
				*BindingSummary);
		}
		else
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[IDOL VOLUME VFX] Preflight Req=%d Idol=%s Category=%s Success=0 FailurePoint=Preflight Fallback=IdolProcVFX Details=%s FailureReason=%s"),
				RequestId,
				*IdolID.ToString(),
				Category == ET66AttackCategory::DOT ? TEXT("DOT") : TEXT("AOE"),
				*BindingSummary,
				*FailureReason);
			return false;
		}

		const FTransform SpawnTransform(FRotator::ZeroRotator, Location);
		AT66IdolVolumeAttackVFX* VolumeEffect = World->SpawnActorDeferred<AT66IdolVolumeAttackVFX>(
			AT66IdolVolumeAttackVFX::StaticClass(),
			SpawnTransform,
			OwnerActor,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!VolumeEffect)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[IDOL VOLUME VFX] Spawn Req=%d Idol=%s Category=%s Success=0 FailurePoint=SpawnActorDeferred Fallback=IdolProcVFX Details=World->SpawnActorDeferred returned null."),
				RequestId,
				*IdolID.ToString(),
				Category == ET66AttackCategory::DOT ? TEXT("DOT") : TEXT("AOE"));
			return false;
		}

		VolumeEffect->InitEffect(IdolID, Category, FollowTarget, Location, Radius, Duration, IdolColor, RequestId);
		UGameplayStatics::FinishSpawningActor(VolumeEffect, SpawnTransform);

		UE_LOG(
			LogTemp,
			Log,
			TEXT("[IDOL VOLUME VFX] Spawn Req=%d Idol=%s Category=%s Success=1 FailurePoint=None Fallback=None Actor=%s Profile=%s"),
			RequestId,
			*IdolID.ToString(),
			Category == ET66AttackCategory::DOT ? TEXT("DOT") : TEXT("AOE"),
			*VolumeEffect->GetName(),
			*AT66IdolVolumeAttackVFX::DescribeProfile(IdolID, Category));

		return true;
	}

	bool IsExampleBounceVFXReady(const int32 RequestId, const bool bLogResult)
	{
		static TWeakObjectPtr<UMaterialInterface> CachedStreakMaterial;
		static TWeakObjectPtr<UMaterialInterface> CachedImpactMaterial;

		UMaterialInterface* StreakMaterial = CachedStreakMaterial.Get();
		if (!StreakMaterial)
		{
			StreakMaterial = LoadObject<UMaterialInterface>(nullptr, HeroOneStreakMaterialPath);
			CachedStreakMaterial = StreakMaterial;
		}

		UMaterialInterface* ImpactMaterial = CachedImpactMaterial.Get();
		if (!ImpactMaterial)
		{
			ImpactMaterial = LoadObject<UMaterialInterface>(nullptr, HeroOneImpactMaterialPath);
			CachedImpactMaterial = ImpactMaterial;
		}

		const bool bReady = (StreakMaterial != nullptr) && (ImpactMaterial != nullptr);
		if (bLogResult && CVarT66VFXHeroBounceVerbose.GetValueOnGameThread() != 0)
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("[ATTACK VFX][ExampleBounce] AssetPreflight Req=%d Ready=%d Streak=%s Impact=%s StreakPath=%s ImpactPath=%s"),
				RequestId,
				bReady ? 1 : 0,
				StreakMaterial ? TEXT("OK") : TEXT("MISSING"),
				ImpactMaterial ? TEXT("OK") : TEXT("MISSING"),
				HeroOneStreakMaterialPath,
				HeroOneImpactMaterialPath);
		}

		return bReady;
	}

	bool IsExampleDOTVFXReady(const int32 RequestId, const bool bLogResult)
	{
		static TWeakObjectPtr<UMaterialInterface> CachedImpactMaterial;

		UMaterialInterface* ImpactMaterial = CachedImpactMaterial.Get();
		if (!ImpactMaterial)
		{
			ImpactMaterial = LoadObject<UMaterialInterface>(nullptr, HeroOneImpactMaterialPath);
			CachedImpactMaterial = ImpactMaterial;
		}

		const bool bReady = ImpactMaterial != nullptr;
		if (bLogResult && CVarT66VFXHeroDOTVerbose.GetValueOnGameThread() != 0)
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("[ATTACK VFX][ExampleDOT] AssetPreflight Req=%d Ready=%d Impact=%s ImpactPath=%s"),
				RequestId,
				bReady ? 1 : 0,
				ImpactMaterial ? TEXT("OK") : TEXT("MISSING"),
				HeroOneImpactMaterialPath);
		}

		return bReady;
	}

	void BuildIdolExampleAOEPalette(
		const FLinearColor& IdolColor,
		const ET66ItemRarity Rarity,
		FLinearColor& OutTintColor,
		FLinearColor& OutPrimaryColor,
		FLinearColor& OutSecondaryColor,
		FLinearColor& OutOutlineColor,
		float& OutGlowStrength)
	{
		BuildIdolExamplePiercePalette(IdolColor, Rarity, OutTintColor, OutPrimaryColor, OutSecondaryColor, OutOutlineColor, OutGlowStrength);
		OutGlowStrength += 0.06f;
	}

	void BuildIdolExampleBouncePalette(
		const FLinearColor& IdolColor,
		const ET66ItemRarity Rarity,
		FLinearColor& OutTintColor,
		FLinearColor& OutPrimaryColor,
		FLinearColor& OutSecondaryColor,
		FLinearColor& OutOutlineColor,
		float& OutGlowStrength)
	{
		BuildIdolExamplePiercePalette(IdolColor, Rarity, OutTintColor, OutPrimaryColor, OutSecondaryColor, OutOutlineColor, OutGlowStrength);
		OutGlowStrength += 0.03f;
	}

	void BuildIdolExampleDOTPalette(
		const FLinearColor& IdolColor,
		const ET66ItemRarity Rarity,
		FLinearColor& OutTintColor,
		FLinearColor& OutPrimaryColor,
		FLinearColor& OutSecondaryColor,
		FLinearColor& OutOutlineColor,
		float& OutGlowStrength)
	{
		BuildIdolExamplePiercePalette(IdolColor, Rarity, OutTintColor, OutPrimaryColor, OutSecondaryColor, OutOutlineColor, OutGlowStrength);
		OutGlowStrength += 0.10f;
	}
}

UT66CombatComponent::UT66CombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	ShotSfx = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/Audio/SFX/Shot.Shot")));
	SlashVFXNiagara = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1")));
	PixelVFXNiagara = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle")));
}

void UT66CombatComponent::SetLockedTarget(AActor* InTarget)
{
	LockedTarget = InTarget;
}

void UT66CombatComponent::ClearLockedTarget()
{
	LockedTarget.Reset();
}

void UT66CombatComponent::PerformScopedPiercingShot(const FVector& Start, const FVector& End)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World)
	{
		return;
	}

	const float LineLength = FVector::Dist(Start, End);
	if (LineLength <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float TubeRadius = 55.f;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;
	const FVector MidPoint = (Start + End) * 0.5f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(
		Overlaps,
		MidPoint,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere((LineLength * 0.5f) + TubeRadius),
		Params);

	TSet<AActor*> HitActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!Target || HitActors.Contains(Target) || !IsValidAutoTarget(Target))
		{
			continue;
		}

		const float DistSq = FMath::PointDistToSegmentSquared(Target->GetActorLocation(), Start, End);
		if (DistSq <= (TubeRadius * TubeRadius))
		{
			HitActors.Add(Target);
			if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Target))
			{
				const float DamageMultiplier = FMath::Max(0.01f, 1.f - Enemy->GetEffectiveArmor());
				const int32 RawDamage = FMath::Max(1, FMath::CeilToInt(static_cast<float>(Enemy->CurrentHP) / DamageMultiplier));
				Enemy->TakeDamageFromHero(RawDamage, SourceID, NAME_None);
			}
			else if (AT66BossBase* Boss = Cast<AT66BossBase>(Target))
			{
				const int32 DesiredFinalDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(Boss->MaxHP) * 0.05f));
				const float DamageMultiplier = FMath::Max(0.01f, 1.f - Boss->GetEffectiveArmor());
				const int32 RawDamage = FMath::Max(1, FMath::CeilToInt(static_cast<float>(DesiredFinalDamage) / DamageMultiplier));
				Boss->TakeDamageFromHeroHit(RawDamage, SourceID, NAME_None);
			}
		}
	}

	SpawnPierceVFX(Start, End, FLinearColor(0.95f, 0.95f, 1.f));
	PlayShotSfx();
}

// ---------------------------------------------------------------------------
// BeginPlay — set up delegates, range sphere, fire timer.
// ---------------------------------------------------------------------------
void UT66CombatComponent::BeginPlay()
{
	Super::BeginPlay();

	BaseAttackRange = AttackRange;
	BaseFireIntervalSeconds = FireIntervalSeconds;
	BaseDamagePerShot = DamagePerShot;

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		CachedRunState = GI->GetSubsystem<UT66RunStateSubsystem>();
		CachedFloatingCombatText = GI->GetSubsystem<UT66FloatingCombatTextSubsystem>();
		CachedIdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>();
		if (CachedRunState)
		{
			CachedRunState->InventoryChanged.AddDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
			CachedRunState->HeroProgressChanged.AddDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
			CachedRunState->SurvivalChanged.AddDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
			CachedRunState->DevCheatsChanged.AddDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
			CachedRunState->SetDOTDamageApplier([this](AActor* Target, int32 Damage, FName SourceIdolID)
			{
				ApplyDamageToActor(Target, Damage, NAME_None, SourceIdolID);
			});
		}
		if (CachedIdolManager)
		{
			CachedIdolManager->IdolStateChanged.AddDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
		}
	}

	RecomputeFromRunState();

	CachedShotSfx = ShotSfx.LoadSynchronous();
	CachedSlashVFXNiagara = SlashVFXNiagara.LoadSynchronous();
	CachedPixelVFXNiagara = PixelVFXNiagara.LoadSynchronous();
	if (CachedPixelVFXNiagara)
	{
		UE_LOG(LogTemp, Log, TEXT("[VFX] Pixel particle system loaded: NS_PixelParticle"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[VFX] NS_PixelParticle not found; falling back to VFX_Attack1. Run CreatePixelParticleNiagara.py + configure in editor."));
	}

	// --- Create the range detection sphere ---
	AActor* Owner = GetOwner();
	if (Owner)
	{
		RangeSphere = NewObject<USphereComponent>(Owner, TEXT("CombatRangeSphere"));
		if (RangeSphere)
		{
			RangeSphere->SetupAttachment(Owner->GetRootComponent());
			RangeSphere->SetSphereRadius(AttackRange);
			RangeSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
			RangeSphere->SetGenerateOverlapEvents(true);
			RangeSphere->SetHiddenInGame(true);
			RangeSphere->SetCanEverAffectNavigation(false);
			// No physics response — purely a query volume.
			RangeSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
			RangeSphere->RegisterComponent();

			RangeSphere->OnComponentBeginOverlap.AddDynamic(this, &UT66CombatComponent::OnRangeBeginOverlap);
			RangeSphere->OnComponentEndOverlap.AddDynamic(this, &UT66CombatComponent::OnRangeEndOverlap);
		}
	}

	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &UT66CombatComponent::TryFire, EffectiveFireIntervalSeconds, true, EffectiveFireIntervalSeconds);

	UE_LOG(LogTemp, Log, TEXT("[GOLD] CombatComponent: initialized — overlap sphere (radius=%.0f), VFX pooling (AutoRelease), timer-based fire (%.2fs)"),
		AttackRange, EffectiveFireIntervalSeconds);
}

void UT66CombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireTimerHandle);
	}

	if (CachedRunState)
	{
		CachedRunState->InventoryChanged.RemoveDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
		CachedRunState->HeroProgressChanged.RemoveDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
		CachedRunState->SurvivalChanged.RemoveDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
		CachedRunState->DevCheatsChanged.RemoveDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
	}
	if (CachedIdolManager)
	{
		CachedIdolManager->IdolStateChanged.RemoveDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
	}

	// Clean up the sphere.
	if (RangeSphere)
	{
		RangeSphere->DestroyComponent();
		RangeSphere = nullptr;
	}
	EnemiesInRange.Empty();

	Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------------------
// Overlap callbacks — maintain EnemiesInRange.
// ---------------------------------------------------------------------------
void UT66CombatComponent::OnRangeBeginOverlap(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/, bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
	if (!OtherActor) return;

	// Only track enemies and bosses (Gambler/Vendor are AT66BossBase).
	if (Cast<AT66EnemyBase>(OtherActor) || Cast<AT66BossBase>(OtherActor))
	{
		EnemiesInRange.AddUnique(OtherActor);
	}
}

void UT66CombatComponent::OnRangeEndOverlap(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/)
{
	if (!OtherActor) return;
	EnemiesInRange.RemoveAll([OtherActor](const TWeakObjectPtr<AActor>& Weak) { return !Weak.IsValid() || Weak.Get() == OtherActor; });
}

// ---------------------------------------------------------------------------
// IsValidAutoTarget — returns true if the actor is alive and targetable.
// ---------------------------------------------------------------------------
bool UT66CombatComponent::IsValidAutoTarget(AActor* A)
{
	if (!A) return false;
	if (AT66EnemyBase* E = Cast<AT66EnemyBase>(A)) return E->CurrentHP > 0;
	if (AT66BossBase* B = Cast<AT66BossBase>(A)) return B->IsAwakened() && B->IsAlive();
	return false;
}

// ---------------------------------------------------------------------------
// FindClosestEnemyInRange — walks EnemiesInRange (small list) instead of all
// world actors.  Returns nullptr if nothing valid is in range.
// ---------------------------------------------------------------------------
AActor* UT66CombatComponent::FindClosestEnemyInRange(const FVector& FromLocation, float MaxRangeSq,
	const TSet<AActor*>* ExcludeSet) const
{
	AActor* Best = nullptr;
	float BestDistSq = MaxRangeSq;

	for (const TWeakObjectPtr<AActor>& Weak : EnemiesInRange)
	{
		AActor* A = Weak.Get();
		if (!A) continue;
		if (ExcludeSet && ExcludeSet->Contains(A)) continue;
		if (!IsValidAutoTarget(A)) continue;
		const float DistSq = FVector::DistSquared(FromLocation, A->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = A;
		}
	}
	return Best;
}

void UT66CombatComponent::HandleInventoryChanged()
{
	UE_LOG(LogTemp, Warning, TEXT("[IDOL CACHE] HandleInventoryChanged owner=%s"), GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
	const float OldInterval = EffectiveFireIntervalSeconds;
	RecomputeFromRunState();
	if (!FMath::IsNearlyEqual(OldInterval, EffectiveFireIntervalSeconds, 0.001f))
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(FireTimerHandle);
			World->GetTimerManager().SetTimer(FireTimerHandle, this, &UT66CombatComponent::TryFire, EffectiveFireIntervalSeconds, true, EffectiveFireIntervalSeconds);
		}
	}

	// Keep the hero's range ring in sync (attack range changes with Scale).
	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(GetOwner()))
	{
		Hero->RefreshAttackRangeRing();
	}

	// Update the detection sphere to match the new attack range.
	if (RangeSphere)
	{
		RangeSphere->SetSphereRadius(AttackRange);
	}
}

void UT66CombatComponent::RecomputeFromRunState()
{
	AttackRange = BaseAttackRange;
	EffectiveFireIntervalSeconds = BaseFireIntervalSeconds;
	EffectiveDamagePerShot = BaseDamagePerShot;
	ProjectileScaleMultiplier = 1.f;
	bHasCachedHeroData = false;
	CachedHeroData = FHeroData{};
	CachedIdolSlots.Reset();

	if (!CachedRunState)
	{
		return;
	}

	UT66GameInstance* GI = GetWorld() ? Cast<UT66GameInstance>(GetWorld()->GetGameInstance()) : nullptr;
	if (GI)
	{
		if (AT66HeroBase* Hero = Cast<AT66HeroBase>(GetOwner()))
		{
			bHasCachedHeroData = GI->GetHeroData(Hero->HeroID, CachedHeroData);
		}

		UT66IdolManagerSubsystem* IdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>();
		const TArray<FName>& Idols = IdolManager ? IdolManager->GetEquippedIdols() : CachedRunState->GetEquippedIdols();
		CachedIdolSlots.Reserve(Idols.Num());
		for (int32 Slot = 0; Slot < Idols.Num(); ++Slot)
		{
			FCachedIdolSlot Entry;
			Entry.IdolID = Idols[Slot];
			Entry.Rarity = IdolManager ? IdolManager->GetEquippedIdolRarityInSlot(Slot) : CachedRunState->GetEquippedIdolRarityInSlot(Slot);
			Entry.bValid = !Entry.IdolID.IsNone() && GI->GetIdolData(Entry.IdolID, Entry.IdolData);
			CachedIdolSlots.Add(MoveTemp(Entry));
		}
	}

	int32 ValidIdolCount = 0;
	for (int32 Slot = 0; Slot < CachedIdolSlots.Num(); ++Slot)
	{
		const FCachedIdolSlot& Entry = CachedIdolSlots[Slot];
		if (!Entry.bValid || Entry.IdolID.IsNone())
		{
			continue;
		}

		++ValidIdolCount;
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[IDOL CACHE] Slot=%d Idol=%s Rarity=%s Category=%s"),
			Slot,
			*Entry.IdolID.ToString(),
			GetT66ItemRarityName(Entry.Rarity),
			GetT66AttackCategoryName(Entry.IdolData.Category));
	}
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[IDOL CACHE] owner=%s totalSlots=%d validSlots=%d"),
		GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
		CachedIdolSlots.Num(),
		ValidIdolCount);

	const float AttackSpeedMult = CachedRunState->GetItemAttackSpeedMultiplier();
	const float DamageMult = CachedRunState->GetItemDamageMultiplier();
	const float ScaleMult = CachedRunState->GetItemScaleMultiplier();

	const float HeroAttackSpeedMult = CachedRunState->GetHeroAttackSpeedMultiplier() * CachedRunState->GetLastStandAttackSpeedMultiplier() * CachedRunState->GetRallyAttackSpeedMultiplier() * CachedRunState->GetEnduranceAttackSpeedMultiplier();
	const float HeroDamageMult = CachedRunState->GetHeroDamageMultiplier() * CachedRunState->GetEnduranceDamageMultiplier() * CachedRunState->GetBrawlersFuryDamageMultiplier();
	const float HeroScaleMult = CachedRunState->GetHeroScaleMultiplier();

	const float TotalAttackSpeed = AttackSpeedMult * HeroAttackSpeedMult;
	const float TotalDamage = DamageMult * HeroDamageMult;
	const float TotalScale = ScaleMult * HeroScaleMult;

	// Per HUD/combat spec: Scale stat affects attack range (larger scale = larger range).
	// Use the DataTable base range (hero-specific) instead of the C++ default.
	const float DataTableRange = CachedRunState->GetHeroBaseAttackRange();
	const float EffectiveBaseRange = (DataTableRange > 0.f) ? DataTableRange : BaseAttackRange;
	AttackRange = FMath::Clamp(EffectiveBaseRange * FMath::Max(0.1f, TotalScale), 200.f, 50000.f);

	EffectiveFireIntervalSeconds = FMath::Clamp(BaseFireIntervalSeconds / FMath::Max(0.01f, TotalAttackSpeed), 0.05f, 10.f);
	EffectiveDamagePerShot = FMath::Clamp(FMath::RoundToInt(static_cast<float>(BaseDamagePerShot) * TotalDamage), 1, 999999);
	ProjectileScaleMultiplier = FMath::Clamp(TotalScale, 0.1f, 10.f);

	// Dev Power toggle: auto-attack does absurd damage.
	if (CachedRunState->GetDevPowerEnabled())
	{
		EffectiveDamagePerShot = 999999;
	}

	// Keep the detection sphere in sync whenever stats are recomputed.
	if (RangeSphere)
	{
		RangeSphere->SetSphereRadius(AttackRange);
	}
}

void UT66CombatComponent::PlayShotSfx()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Apply user SFX volume even if SoundClass assets aren't set up yet.
	UGameInstance* GI = World->GetGameInstance();
	UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	const float MasterVol = PS ? FMath::Clamp(PS->GetMasterVolume(), 0.f, 1.f) : 1.f;
	const float SfxVol = PS ? FMath::Clamp(PS->GetSfxVolume(), 0.f, 1.f) : 1.f;
	const float EffectiveSfx = MasterVol * SfxVol;
	if (EffectiveSfx <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	USoundBase* Sound = CachedShotSfx;
	if (!Sound)
	{
		if (!bShotSfxWarnedMissing)
		{
			bShotSfxWarnedMissing = true;
			UE_LOG(LogTemp, Warning, TEXT("Shot SFX not found. Import your Shot audio into UE so '/Game/Audio/SFX/Shot' exists (SoundWave/SoundCue)."));
		}
		return;
	}

	// Auto attack is effectively UI-ish for now; play 2D so it always reads clearly.
	UGameplayStatics::PlaySound2D(World, Sound, EffectiveSfx);
}

float UT66CombatComponent::GetAutoAttackCooldownProgress() const
{
	UWorld* World = GetWorld();
	if (!World || EffectiveFireIntervalSeconds <= 0.f) return 1.f;
	const float Now = static_cast<float>(World->GetTimeSeconds());
	const float Elapsed = Now - LastFireTime;
	return FMath::Clamp(Elapsed / EffectiveFireIntervalSeconds, 0.f, 1.f);
}

// ---------------------------------------------------------------------------
// TryFire — the hero auto-attack heartbeat.
// Target finding now walks the small EnemiesInRange list (maintained by sphere
// overlap events) instead of doing 3x TActorIterator over the entire world.
// ---------------------------------------------------------------------------
void UT66CombatComponent::TryFire()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	UWorld* World = GetWorld();
	if (!World) return;

	if (bSuppressAutoAttack)
	{
		return;
	}

	FLagScopedScope LagScope(World, TEXT("CombatComponent::TryFire"));

	// Safe zone rule: if hero is inside any NPC safe bubble, do not fire.
	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(OwnerActor))
	{
		if (Hero->IsInSafeZone())
		{
			return;
		}
	}

	LastFireTime = static_cast<float>(World->GetTimeSeconds());

	// Crit: roll per hit; multiply damage and pass EventType_Crit for floating text.
	auto ResolveCrit = [this](int32 BaseDamage) -> TPair<int32, FName>
	{
		if (!CachedRunState) return { BaseDamage, NAME_None };
		const float CritChance = CachedRunState->GetCritChance01();
		const bool bCrit = (CritChance > 0.f && FMath::FRand() < CritChance);
		if (bCrit)
		{
			const float Mult = CachedRunState->GetCritDamageMultiplier();
			return { FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseDamage) * Mult)), UT66FloatingCombatTextSubsystem::EventType_Crit };
		}
		return { BaseDamage, NAME_None };
	};

	FVector MyLoc = OwnerActor->GetActorLocation();

	// Close/Long range damage: multiply base damage by range-based multiplier (close = 0–10% of range, long = 90–100%).
	// OutRangeEvent set when in close/long zone and multiplier != 1 (for floating text on hero).
	auto GetRangeMultipliedDamage = [this, MyLoc](int32 BaseDamage, AActor* Target, FName* OutRangeEvent = nullptr) -> int32
	{
		if (!CachedRunState || !Target) return BaseDamage;
		const float Dist = FVector::Dist(MyLoc, Target->GetActorLocation());
		const float CloseThresh = CachedRunState->GetCloseRangeThreshold();
		const float LongThresh = CachedRunState->GetLongRangeThreshold();
		if (Dist <= CloseThresh)
		{
			const float Mult = CachedRunState->GetCloseRangeDamageMultiplier();
			if (OutRangeEvent && Mult != 1.f) *OutRangeEvent = UT66FloatingCombatTextSubsystem::EventType_CloseRange;
			return FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseDamage) * Mult));
		}
		if (Dist >= LongThresh)
		{
			const float Mult = CachedRunState->GetLongRangeDamageMultiplier();
			if (OutRangeEvent && Mult != 1.f) *OutRangeEvent = UT66FloatingCombatTextSubsystem::EventType_LongRange;
			return FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseDamage) * Mult));
		}
		return BaseDamage;
	};

	const float RangeSq = AttackRange * AttackRange;

	bool bHasCachedPierceTargets = false;
	AActor* CachedPiercePrimaryTarget = nullptr;
	float CachedPierceLineLength = 0.f;
	float CachedPierceRadius = 0.f;
	FVector CachedPierceDirection = FVector::ForwardVector;
	TArray<AActor*> CachedPierceTargets;

	bool bHasCachedSlashTargets = false;
	AActor* CachedSlashPrimaryTarget = nullptr;
	float CachedSlashRadius = 0.f;
	TArray<AActor*> CachedSlashTargets;

	auto BuildPierceTargets = [&](AActor* QueryPrimaryTarget, float LineLength, float PierceRadius, TArray<AActor*>& OutTargets, FVector& OutDir, const FVector* OverrideTargetLocation = nullptr)
	{
		if (!QueryPrimaryTarget && !OverrideTargetLocation)
		{
			OutTargets.Reset();
			OutDir = FVector::ForwardVector;
			return;
		}

		const bool bCanUseCache = (OverrideTargetLocation == nullptr);
		if (bCanUseCache
			&& bHasCachedPierceTargets
			&& CachedPiercePrimaryTarget == QueryPrimaryTarget
			&& FMath::IsNearlyEqual(CachedPierceLineLength, LineLength)
			&& FMath::IsNearlyEqual(CachedPierceRadius, PierceRadius))
		{
			OutTargets = CachedPierceTargets;
			OutDir = CachedPierceDirection;
			return;
		}

		const FVector TargetLoc = OverrideTargetLocation ? *OverrideTargetLocation : QueryPrimaryTarget->GetActorLocation();
		OutDir = (TargetLoc - MyLoc).GetSafeNormal();
		if (OutDir.IsNearlyZero())
		{
			OutDir = OwnerActor->GetActorForwardVector().GetSafeNormal();
			if (OutDir.IsNearlyZero())
			{
				OutDir = FVector::ForwardVector;
			}
		}
		const float HalfLen = FMath::Max(1.f, (LineLength * 0.5f) - PierceRadius);
		const FVector MidPoint = MyLoc + OutDir * (LineLength * 0.5f);
		const FQuat Rot = FQuat::FindBetween(FVector::UpVector, OutDir);
		const FCollisionShape Cap = FCollisionShape::MakeCapsule(PierceRadius, HalfLen);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerActor);

		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByChannel(Overlaps, MidPoint, Rot, ECC_Pawn, Cap, Params);

		OutTargets.Reset();
		if (QueryPrimaryTarget && IsValidAutoTarget(QueryPrimaryTarget))
		{
			OutTargets.Add(QueryPrimaryTarget);
		}
		for (const FOverlapResult& O : Overlaps)
		{
			AActor* A = O.GetActor();
			if (A && IsValidAutoTarget(A) && A != QueryPrimaryTarget && (Cast<AT66EnemyBase>(A) || Cast<AT66BossBase>(A)))
			{
				OutTargets.AddUnique(A);
			}
		}
		OutTargets.Sort([&MyLoc](const AActor& A, const AActor& B)
		{
			return FVector::DistSquared(MyLoc, A.GetActorLocation()) < FVector::DistSquared(MyLoc, B.GetActorLocation());
		});

		if (bCanUseCache)
		{
			bHasCachedPierceTargets = true;
			CachedPiercePrimaryTarget = QueryPrimaryTarget;
			CachedPierceLineLength = LineLength;
			CachedPierceRadius = PierceRadius;
			CachedPierceDirection = OutDir;
			CachedPierceTargets = OutTargets;
		}
	};

	auto BuildSlashTargets = [&](AActor* QueryPrimaryTarget, float Radius, TArray<AActor*>& OutTargets, const FVector* OverrideCenter = nullptr)
	{
		if (!QueryPrimaryTarget && !OverrideCenter)
		{
			OutTargets.Reset();
			return;
		}

		const bool bCanUseCache = (OverrideCenter == nullptr);
		if (bCanUseCache
			&& bHasCachedSlashTargets
			&& CachedSlashPrimaryTarget == QueryPrimaryTarget
			&& FMath::IsNearlyEqual(CachedSlashRadius, Radius))
		{
			OutTargets = CachedSlashTargets;
			return;
		}

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerActor);
		Params.AddIgnoredActor(QueryPrimaryTarget);

		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByChannel(
			Overlaps,
			OverrideCenter ? *OverrideCenter : QueryPrimaryTarget->GetActorLocation(),
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(Radius),
			Params);

		OutTargets.Reset();
		if (QueryPrimaryTarget && IsValidAutoTarget(QueryPrimaryTarget))
		{
			OutTargets.Add(QueryPrimaryTarget);
		}
		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (AActor* Hit = Overlap.GetActor(); Hit && IsValidAutoTarget(Hit))
			{
				OutTargets.AddUnique(Hit);
			}
		}

		if (bCanUseCache)
		{
			bHasCachedSlashTargets = true;
			CachedSlashPrimaryTarget = QueryPrimaryTarget;
			CachedSlashRadius = Radius;
			CachedSlashTargets = OutTargets;
		}
	};

	// Purge stale weak pointers (destroyed actors that didn't fire EndOverlap).
	EnemiesInRange.RemoveAll([](const TWeakObjectPtr<AActor>& Weak) { return !Weak.IsValid(); });

	// Hero attack category (Pierce/Bounce/AOE/DOT) and data for Bounce/DOT params.
	ET66AttackCategory AttackCategory = ET66AttackCategory::AOE;
	FHeroData HeroDataForPrimary;
	bool bHaveHeroData = bHasCachedHeroData;
	FName CurrentHeroID = NAME_None;
	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(OwnerActor))
	{
		CurrentHeroID = Hero->HeroID;
		if (bHasCachedHeroData)
		{
			AttackCategory = CachedHeroData.PrimaryCategory;
			HeroDataForPrimary = CachedHeroData;
		}
	}

	// --- Pierce (straight line): full range so enemies behind the first are hit; 10% damage reduction per pierced target. ---
	auto PerformPierce = [&](AActor* PrimaryTarget, float PrimaryDamageMult) -> bool
	{
		if (!PrimaryTarget) return false;

		const FVector TargetLoc = PrimaryTarget->GetActorLocation();
		const float LineLength = AttackRange;
		const float PierceRadius = 80.f * ProjectileScaleMultiplier;
		FVector Dir = FVector::ForwardVector;
		TArray<AActor*> InLine;
		BuildPierceTargets(PrimaryTarget, LineLength, PierceRadius, InLine, Dir);

		for (int32 i = 0; i < InLine.Num(); ++i)
		{
			const float Multiplier = FMath::Max(0.1f, 1.f - 0.1f * static_cast<float>(i));
			const float PrimaryMult = (i == 0) ? PrimaryDamageMult : 1.f;
			const int32 BaseDmg = FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * Multiplier * PrimaryMult));
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(BaseDmg, InLine[i], &RangeEvent);
			const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
			ApplyDamageToActor(InLine[i], Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
		}

		if (CurrentHeroID == FName(TEXT("Hero_1")))
		{
			const float FurthestDistance = InLine.Num() > 0
				? FVector::Dist2D(MyLoc, InLine.Last()->GetActorLocation())
				: FVector::Dist2D(MyLoc, TargetLoc);
			const FVector VFXStart = MyLoc + FVector(0.f, 0.f, 8.f);
			const FVector VFXEnd = MyLoc + Dir * FMath::Min(LineLength, FurthestDistance + 80.f) + FVector(0.f, 0.f, 8.f);
			const FVector ImpactLoc = TargetLoc + FVector(0.f, 0.f, 10.f);
			SpawnHeroOnePierceVFX(VFXStart, VFXEnd, ImpactLoc);
		}
		else
		{
			const float FurthestDistance = InLine.Num() > 0
				? FVector::Dist2D(MyLoc, InLine.Last()->GetActorLocation())
				: FVector::Dist2D(MyLoc, TargetLoc);
			const FVector VFXStart = MyLoc + FVector(0.f, 0.f, 8.f);
			const FVector VFXEnd = MyLoc + Dir * FMath::Min(LineLength, FurthestDistance + 80.f) + FVector(0.f, 0.f, 8.f);
			const FVector ImpactLoc = TargetLoc + FVector(0.f, 0.f, 10.f);
			const FLinearColor HeroTint = bHaveHeroData ? HeroDataForPrimary.PlaceholderColor : FLinearColor::White;
			SpawnHeroPierceVFX(VFXStart, VFXEnd, ImpactLoc, HeroTint, CurrentHeroID);
		}
		PlayShotSfx();
		return true;
	};

	// --- AoE Slash ---
	auto PerformSlash = [&](AActor* PrimaryTarget, float PrimaryDamageMult) -> bool
	{
		if (!PrimaryTarget) return false;

		const FVector SlashCenter = PrimaryTarget->GetActorLocation();
		const float EffectiveSlashRadius = SlashRadius * ProjectileScaleMultiplier;

		TArray<AActor*> SlashTargets;
		BuildSlashTargets(PrimaryTarget, EffectiveSlashRadius, SlashTargets);

		const int32 HitCount = SlashTargets.Num();
		float ArcaneMult = 1.f;
		if (CachedRunState && CachedRunState->GetPassiveType() == ET66PassiveType::ArcaneAmplification)
		{
			if (HitCount >= 5) ArcaneMult = 1.35f;
			else if (HitCount >= 3) ArcaneMult = 1.2f;
		}

		const int32 PrimaryDmg = FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * PrimaryDamageMult * ArcaneMult));
		{
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(PrimaryDmg, PrimaryTarget, &RangeEvent);
			const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
			ApplyDamageToActor(PrimaryTarget, Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
		}

		const int32 SplashDmg = FMath::Max(1, FMath::RoundToInt(static_cast<float>(EffectiveDamagePerShot) * ArcaneMult));
		for (int32 TargetIndex = 1; TargetIndex < SlashTargets.Num(); ++TargetIndex)
		{
			AActor* Hit = SlashTargets[TargetIndex];
			if (Hit)
			{
				FName RangeEvent;
				const int32 RangeDmg = GetRangeMultipliedDamage(SplashDmg, Hit, &RangeEvent);
				const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
				ApplyDamageToActor(Hit, Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
			}
		}

		const FLinearColor HeroTint = bHaveHeroData ? HeroDataForPrimary.PlaceholderColor : FLinearColor::White;
		SpawnHeroSlashVFX(SlashCenter, EffectiveSlashRadius, HeroTint, CurrentHeroID);
		PlayShotSfx();
		return true;
	};

	// --- Bounce ---
	auto PerformBounce = [&](AActor* PrimaryTarget, float PrimaryDamageMult) -> bool
	{
		if (!PrimaryTarget) return false;
		const int32 BaseBounce = FMath::Max(1, bHaveHeroData ? HeroDataForPrimary.BaseBounceCount : 1);
		const int32 ChaosBonus = CachedRunState ? CachedRunState->GetChaosTheoryBonusBounceCount() : 0;
		const float TimeNow = static_cast<float>(World->GetTimeSeconds());
		const int32 JuicedBonus = (JuicedEndTime > TimeNow) ? JuicedBonusBounce : 0;
		const int32 BounceCount = BaseBounce + ChaosBonus + JuicedBonus;
		const float Falloff = FMath::Clamp(bHaveHeroData ? HeroDataForPrimary.FalloffPerHit : 0.f, 0.f, 0.95f);
		const float BounceRangeSq = AttackRange * AttackRange;
		const FVector PrimaryLoc = PrimaryTarget->GetActorLocation();
		TArray<AActor*> Chain;
		TArray<FVector> ChainPositions;
		Chain.Add(PrimaryTarget);
		ChainPositions.Add(MyLoc);
		ChainPositions.Add(PrimaryLoc);
		const int32 PrimaryDmg = FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * PrimaryDamageMult));
		{
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(PrimaryDmg, PrimaryTarget, &RangeEvent);
			const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
			ApplyDamageToActor(PrimaryTarget, Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
		}
		FVector CurrentLoc = PrimaryLoc;
		TSet<AActor*> HitSet;
		HitSet.Add(PrimaryTarget);
		int32 BouncesLeft = BounceCount - 1;
		float DamageMult = 1.f - Falloff;
		while (BouncesLeft > 0)
		{
			AActor* Next = FindClosestEnemyInRange(CurrentLoc, BounceRangeSq, &HitSet);
			if (!Next) break;
			Chain.Add(Next);
			ChainPositions.Add(Next->GetActorLocation());
			HitSet.Add(Next);
			const int32 BounceDmg = FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * DamageMult));
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(BounceDmg, Next, &RangeEvent);
			const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
			ApplyDamageToActor(Next, Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
			// StaticCharge: 20% chance to confuse bounced targets.
			if (CachedRunState && CachedRunState->GetPassiveType() == ET66PassiveType::StaticCharge)
			{
				if (FMath::FRand() < 0.2f)
				{
					if (AT66EnemyBase* BounceEnemy = Cast<AT66EnemyBase>(Next))
						BounceEnemy->ApplyConfusion(1.5f);
				}
			}
			CurrentLoc = Next->GetActorLocation();
			DamageMult *= (1.f - Falloff);
			--BouncesLeft;
		}
		const FLinearColor HeroTint = bHaveHeroData ? HeroDataForPrimary.PlaceholderColor : FLinearColor::White;
		SpawnHeroBounceVFX(ChainPositions, HeroTint, CurrentHeroID);
		PlayShotSfx();
		return true;
	};

	// --- DOT ---
	auto PerformDOT = [&](AActor* PrimaryTarget, float PrimaryDamageMult) -> bool
	{
		if (!PrimaryTarget || !CachedRunState) return false;
		const float Duration = (bHaveHeroData && HeroDataForPrimary.DotDuration > 0.f) ? HeroDataForPrimary.DotDuration : 3.f;
		const float TickInterval = (bHaveHeroData && HeroDataForPrimary.DotTickInterval > 0.f) ? HeroDataForPrimary.DotTickInterval : 0.5f;
		const int32 Ticks = FMath::Max(1, FMath::RoundToInt(Duration / TickInterval));
		const int32 InitialDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(EffectiveDamagePerShot) * 0.5f * PrimaryDamageMult));
		const float DotTotalDamage = static_cast<float>(FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * PrimaryDamageMult) - InitialDamage));
		const float DamagePerTick = DotTotalDamage / static_cast<float>(Ticks);
		{
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(InitialDamage, PrimaryTarget, &RangeEvent);
			const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
			ApplyDamageToActor(PrimaryTarget, Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
		}
		CachedRunState->ApplyDOT(PrimaryTarget, Duration, TickInterval, DamagePerTick, NAME_None);
		// Frostbite: DOT attacks slow enemy move speed by 30%.
		if (CachedRunState->HasFrostbite())
		{
			if (AT66EnemyBase* DotEnemy = Cast<AT66EnemyBase>(PrimaryTarget))
				DotEnemy->ApplyMoveSlow(0.7f, Duration);
		}
		const FLinearColor HeroTint = bHaveHeroData ? HeroDataForPrimary.PlaceholderColor : FLinearColor::White;
		SpawnHeroDOTVFX(PrimaryTarget, PrimaryTarget->GetActorLocation(), Duration, 80.f, HeroTint, CurrentHeroID);
		PlayShotSfx();
		return true;
	};

	// ---------------------------------------------------------------------------
	// Resolve primary target: locked > closest in EnemiesInRange.
	// No TActorIterator — just walk the small overlap list.
	// ---------------------------------------------------------------------------
	AActor* PrimaryTarget = nullptr;
	if (AActor* Locked = LockedTarget.Get())
	{
		const float DistSq = FVector::DistSquared(MyLoc, Locked->GetActorLocation());
		if (DistSq <= RangeSq && IsValidAutoTarget(Locked))
			PrimaryTarget = Locked;
		else
			ClearLockedTarget();
	}
	if (!PrimaryTarget)
	{
		PrimaryTarget = FindClosestEnemyInRange(MyLoc, RangeSq);
	}

	// Marksman's Focus: consecutive hits on same target stack +8% damage (max 5).
	float PrimaryDamageMultiplier = 1.f;
	if (CachedRunState && CachedRunState->GetPassiveType() == ET66PassiveType::MarksmanFocus && PrimaryTarget)
	{
		if (PrimaryTarget == LastMarksmanTarget.Get())
			MarksmanStacks = FMath::Min(5, MarksmanStacks + 1);
		else
			MarksmanStacks = 1;
		LastMarksmanTarget = PrimaryTarget;
		PrimaryDamageMultiplier = 1.f + 0.08f * static_cast<float>(MarksmanStacks);
	}

	// QuickDraw: first attack after 2s idle = 2× damage.
	if (CachedRunState)
		PrimaryDamageMultiplier *= CachedRunState->GetQuickDrawDamageMultiplier();

	// Deadeye ultimate buff: 2× damage while active.
	const float Now = static_cast<float>(World->GetTimeSeconds());
	if (DeadeyeEndTime > Now)
		PrimaryDamageMultiplier *= 2.f;

	// RabidFrenzy: every hit applies a short DOT (handled after the attack switch below).

	// Headshot: 15% chance for 3× damage (multiplicative with crit).
	if (CachedRunState && CachedRunState->RollHeadshot())
		PrimaryDamageMultiplier *= 3.f;

	// Notify RunState that an attack was fired (for Overclock counter, QuickDraw timer).
	if (CachedRunState)
		CachedRunState->NotifyAttackFired();

	// Hero primary attack (Pierce / Bounce / AOE / DOT; VFX white).
	if (PrimaryTarget)
	{
		const FVector PrimaryTargetImpactLocation = PrimaryTarget->GetActorLocation();
		switch (AttackCategory)
		{
		case ET66AttackCategory::Pierce: (void)PerformPierce(PrimaryTarget, PrimaryDamageMultiplier); break;
		case ET66AttackCategory::Bounce: (void)PerformBounce(PrimaryTarget, PrimaryDamageMultiplier); break;
		case ET66AttackCategory::AOE:   (void)PerformSlash(PrimaryTarget, PrimaryDamageMultiplier); break;
		case ET66AttackCategory::DOT:   (void)PerformDOT(PrimaryTarget, PrimaryDamageMultiplier); break;
		default: (void)PerformSlash(PrimaryTarget, PrimaryDamageMultiplier); break;
		}

		// Overclock: every 8th attack fires a second time immediately.
		if (CachedRunState && CachedRunState->ShouldOverclockDouble())
		{
			switch (AttackCategory)
			{
			case ET66AttackCategory::Pierce: (void)PerformPierce(PrimaryTarget, 1.f); break;
			case ET66AttackCategory::Bounce: (void)PerformBounce(PrimaryTarget, 1.f); break;
			case ET66AttackCategory::AOE:   (void)PerformSlash(PrimaryTarget, 1.f); break;
			case ET66AttackCategory::DOT:   (void)PerformDOT(PrimaryTarget, 1.f); break;
			default: break;
			}
		}

		// Evasive: if flagged, apply bonus 3s DOT to the primary target (50% of hit damage).
		if (CachedRunState && CachedRunState->ConsumeEvasiveBonusDOT() && PrimaryTarget)
		{
			const float BonusDotDamage = static_cast<float>(EffectiveDamagePerShot) * 0.5f;
			const float EvasiveTicks = 6.f;
			CachedRunState->ApplyDOT(PrimaryTarget, 3.f, 0.5f, BonusDotDamage / EvasiveTicks, NAME_None);
			SpawnDOTVFX(PrimaryTarget->GetActorLocation(), 3.f, 60.f, FLinearColor(0.6f, 0.2f, 0.8f));
		}

		// RabidFrenzy ultimate buff: every hit applies a short DOT.
		if (RabidFrenzyEndTime > Now && CachedRunState && PrimaryTarget)
		{
			const float FrenzyDotDmg = static_cast<float>(EffectiveDamagePerShot) * 0.3f;
			const float FrenzyTicks = 4.f;
			CachedRunState->ApplyDOT(PrimaryTarget, 2.f, 0.5f, FrenzyDotDmg / FrenzyTicks, NAME_None);
			SpawnDOTVFX(PrimaryTarget->GetActorLocation(), 2.f, 50.f, FLinearColor(0.9f, 0.3f, 0.1f));
		}

		// Idol attacks: one per equipped idol, each with unique color.
		if (CachedRunState)
		{
			const float IdolRange = AttackRange;
			// Bounce search radius = hero attack range, centered on the last hit enemy each step.
			const float BounceSearchRadius = IdolRange;
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[IDOL PROC] owner=%s target=%s cachedIdolSlots=%d"),
				GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
				*PrimaryTarget->GetName(),
				CachedIdolSlots.Num());

			int32 IdolVisualIndex = 0;
			for (const FCachedIdolSlot& CachedIdolSlot : CachedIdolSlots)
			{
				if (!CachedIdolSlot.bValid || CachedIdolSlot.IdolID.IsNone()) continue;
				const FName IdolID = CachedIdolSlot.IdolID;
				const ET66ItemRarity IdolRarity = CachedIdolSlot.Rarity;
				const FIdolData& IdolData = CachedIdolSlot.IdolData;
				const float IdolVisualDelay = static_cast<float>(IdolVisualIndex) * 0.035f;

				const int32 IdolDamage = FMath::Max(1, FMath::RoundToInt(IdolData.GetDamageAtRarity(IdolRarity)));
				const FVector PrimaryLoc = PrimaryTargetImpactLocation;
				const FVector PrimaryVFXLoc = ResolveGroundAnchor(World, PrimaryLoc, PrimaryTarget);
				UE_LOG(
					LogTemp,
					Warning,
					TEXT("[IDOL PROC] Index=%d Idol=%s Rarity=%s Category=%s Damage=%d Delay=%.3f targetLoc=%s vfxLoc=%s"),
					IdolVisualIndex,
					*IdolID.ToString(),
					GetT66ItemRarityName(IdolRarity),
					GetT66AttackCategoryName(IdolData.Category),
					IdolDamage,
					IdolVisualDelay,
					*PrimaryLoc.ToCompactString(),
					*PrimaryVFXLoc.ToCompactString());

				switch (IdolData.Category)
				{
				case ET66AttackCategory::Pierce:
				{
					const float LineLength = IdolRange;
					const float PierceRadius = 60.f;
					FVector Dir = FVector::ForwardVector;
					TArray<AActor*> InLine;
					BuildPierceTargets(PrimaryTarget, LineLength, PierceRadius, InLine, Dir, &PrimaryLoc);
					for (int32 i = 0; i < InLine.Num(); ++i)
					{
						const float Mult = FMath::Max(0.1f, 1.f - 0.1f * static_cast<float>(i));
						const int32 Dmg = FMath::Max(1, FMath::RoundToInt(IdolDamage * Mult));
						FName RangeEvent;
						const int32 RangeDmg = GetRangeMultipliedDamage(Dmg, InLine[i], &RangeEvent);
						const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
						ApplyDamageToActor(InLine[i], Resolved.Key, Resolved.Value, IdolID, RangeEvent);
					}
					SpawnIdolPierceVFX(IdolID, IdolRarity, PrimaryVFXLoc, PrimaryVFXLoc + Dir * (LineLength * 0.5f), PrimaryVFXLoc, IdolVisualDelay);
					break;
				}
				case ET66AttackCategory::AOE:
				{
					const float Radius = FMath::Max(50.f, IdolData.GetPropertyAtRarity(IdolRarity));
					if (IsValidAutoTarget(PrimaryTarget))
					{
						FName RangeEvent;
						const int32 RangeDmg = GetRangeMultipliedDamage(IdolDamage, PrimaryTarget, &RangeEvent);
						const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
						ApplyDamageToActor(PrimaryTarget, Resolved.Key, Resolved.Value, IdolID, RangeEvent);
					}
					TArray<AActor*> SlashTargets;
					BuildSlashTargets(PrimaryTarget, Radius, SlashTargets, &PrimaryLoc);
					for (int32 TargetIndex = 0; TargetIndex < SlashTargets.Num(); ++TargetIndex)
					{
						if (AActor* Hit = SlashTargets[TargetIndex])
						{
							if (Hit == PrimaryTarget)
							{
								continue;
							}
							FName RangeEvent;
							const int32 RangeDmg = GetRangeMultipliedDamage(IdolDamage, Hit, &RangeEvent);
							const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
							ApplyDamageToActor(Hit, Resolved.Key, Resolved.Value, IdolID, RangeEvent);
						}
					}
					SpawnIdolAOEVFX(IdolID, IdolRarity, PrimaryVFXLoc, Radius, IdolVisualDelay);
					break;
				}
				case ET66AttackCategory::Bounce:
				{
					// BounceCount = number of jumps FROM the primary target to other enemies.
					// At Black rarity (BaseProperty=1) the bolt bounces once (hits 1 extra enemy).
					const int32 BounceCount = FMath::Max(1, FMath::RoundToInt(IdolData.GetPropertyAtRarity(IdolRarity)));
					const float IdolFalloff = FMath::Clamp(IdolData.FalloffPerHit, 0.f, 0.95f);
					const float BounceRangeSq = BounceSearchRadius * BounceSearchRadius;

					// Damage the primary target (auto-attack already hit it; idol adds extra damage).
					if (IsValidAutoTarget(PrimaryTarget))
					{
						FName RangeEvent;
						const int32 RangeDmg = GetRangeMultipliedDamage(IdolDamage, PrimaryTarget, &RangeEvent);
						const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
						ApplyDamageToActor(PrimaryTarget, Resolved.Key, Resolved.Value, IdolID, RangeEvent);
					}

					// Chain starts FROM the primary target (not from the hero).
					// VFX will only show enemy->enemy segments so it looks like a bolt bouncing off.
					TArray<FVector> ChainPositions;
					ChainPositions.Add(PrimaryVFXLoc);

					FVector CurrentLoc = PrimaryLoc;
					TSet<AActor*> HitSet;
					HitSet.Add(PrimaryTarget);
					int32 BouncesLeft = BounceCount;
					float IdolDamageMult = 1.f - IdolFalloff;

					while (BouncesLeft > 0)
					{
						AActor* Next = FindClosestEnemyInRange(CurrentLoc, BounceRangeSq, &HitSet);
						if (!Next) break;
						ChainPositions.Add(ResolveProcVFXAnchor(Next, World));
						HitSet.Add(Next);
						const int32 BounceDmg = FMath::Max(1, FMath::RoundToInt(IdolDamage * IdolDamageMult));
						FName RangeEvent;
						const int32 RangeDmg = GetRangeMultipliedDamage(BounceDmg, Next, &RangeEvent);
						const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
						ApplyDamageToActor(Next, Resolved.Key, Resolved.Value, IdolID, RangeEvent);
						CurrentLoc = Next->GetActorLocation();
						IdolDamageMult *= (1.f - IdolFalloff);
						--BouncesLeft;
					}
					SpawnIdolBounceVFX(IdolID, IdolRarity, ChainPositions, IdolVisualDelay);
					break;
				}
				case ET66AttackCategory::DOT:
				{
					const float Duration = FMath::Max(0.5f, IdolData.GetPropertyAtRarity(IdolRarity));
					const float TickInterval = FMath::Max(0.1f, IdolData.DotTickInterval);
					const int32 Ticks = FMath::Max(1, FMath::RoundToInt(Duration / TickInterval));
					const float DamagePerTick = static_cast<float>(IdolDamage) / static_cast<float>(Ticks);
					if (IsValidAutoTarget(PrimaryTarget))
					{
						CachedRunState->ApplyDOT(PrimaryTarget, Duration, TickInterval, DamagePerTick, IdolID);
					}
					SpawnIdolDOTVFX(IdolID, IdolRarity, IsValidAutoTarget(PrimaryTarget) ? PrimaryTarget : nullptr, PrimaryVFXLoc, Duration, 80.f, IdolVisualDelay);
					break;
				}
				default:
					break;
				}

				++IdolVisualIndex;
			}
		}
	}
}

// ---------------------------------------------------------------------------
// ApplyDamageToActor — dispatches to the correct TakeDamage method per type.
// ---------------------------------------------------------------------------
void UT66CombatComponent::ApplyDamageToActor(AActor* Target, int32 DamageAmount, FName EventType, FName SourceID, FName RangeEventForHero)
{
	if (!Target) return;
	// Toxin Stacking (Rogue): enemies with active DOT take +15% damage from all sources.
	if (CachedRunState)
	{
		const float ToxinMult = CachedRunState->GetToxinStackingDamageMultiplier(Target);
		if (ToxinMult > 1.f)
			DamageAmount = FMath::Max(1, FMath::RoundToInt(static_cast<float>(DamageAmount) * ToxinMult));
	}
	const FName ResolvedSource = SourceID.IsNone() ? UT66DamageLogSubsystem::SourceID_AutoAttack : SourceID;
	UT66FloatingCombatTextSubsystem* FloatingText = CachedFloatingCombatText;
	AActor* Hero = GetOwner();

	if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Target))
	{
		if (E->CurrentHP > 0)
		{
			const bool bEnemyDied = E->TakeDamageFromHero(DamageAmount, ResolvedSource, EventType);
			if (!bEnemyDied && ResolvedSource == UT66DamageLogSubsystem::SourceID_AutoAttack && Hero)
			{
				E->ApplyAutoAttackKnockback(Hero->GetActorLocation());
			}
		}
	}
	else if (AT66BossBase* B = Cast<AT66BossBase>(Target))
	{
		if (B->IsAwakened() && B->IsAlive())
		{
			B->TakeDamageFromHeroHit(DamageAmount, ResolvedSource, EventType);
		}
	}

	// Life steal: % chance per hit; when it procs, heal 10% of damage dealt.
	if (DamageAmount > 0 && CachedRunState)
	{
		const float LsChance = FMath::Clamp(CachedRunState->GetLifeStealFraction(), 0.f, 1.f);
		if (LsChance > 0.f && FMath::FRand() < LsChance)
		{
			CachedRunState->HealHP(static_cast<float>(DamageAmount) * 0.1f);
			if (FloatingText && Hero) FloatingText->ShowStatusEvent(Hero, UT66FloatingCombatTextSubsystem::EventType_LifeSteal);
		}
	}

	// Invisibility: chance to confuse (apply confusion) the enemy we hit.
	if (Target && DamageAmount > 0 && CachedRunState)
	{
		const float InvisChance = CachedRunState->GetInvisibilityChance01();
		if (InvisChance > 0.f && FMath::FRand() < InvisChance)
		{
			if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Target))
			{
				E->ApplyConfusion(3.f);
				if (FloatingText) FloatingText->ShowStatusEvent(Target, UT66FloatingCombatTextSubsystem::EventType_Confusion);
				if (FloatingText && Hero) FloatingText->ShowStatusEvent(Hero, UT66FloatingCombatTextSubsystem::EventType_Invisibility);
			}
		}
	}

	// Taunt: on hit, apply armor debuff to the target (enemy or boss) using Taunt stat.
	if (Target && DamageAmount > 0 && CachedRunState)
	{
		const float Aggro = CachedRunState->GetAggroMultiplier();
		if (Aggro > 1.f)
		{
			const float ReductionAmount = FMath::Min(0.5f, Aggro - 1.f);
			const float DurationSeconds = 3.f;
			if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Target))
			{
				E->ApplyArmorDebuff(ReductionAmount, DurationSeconds);
				if (FloatingText) FloatingText->ShowStatusEvent(Target, UT66FloatingCombatTextSubsystem::EventType_Taunt);
			}
			else if (AT66BossBase* B = Cast<AT66BossBase>(Target))
			{
				B->ApplyArmorDebuff(ReductionAmount, DurationSeconds);
				if (FloatingText) FloatingText->ShowStatusEvent(Target, UT66FloatingCombatTextSubsystem::EventType_Taunt);
			}
		}
	}

	// Close/Long range bonus: show on hero when applicable.
	if (FloatingText && Hero && !RangeEventForHero.IsNone())
	{
		FloatingText->ShowStatusEvent(Hero, RangeEventForHero);
	}
}

// ---------------------------------------------------------------------------
UNiagaraSystem* UT66CombatComponent::GetActiveVFXSystem() const
{
	return CachedPixelVFXNiagara ? CachedPixelVFXNiagara : CachedSlashVFXNiagara;
}

// ---------------------------------------------------------------------------
// Helper: find NS_PixelParticle (or VFX_Attack1 fallback) from any world.
// ---------------------------------------------------------------------------
static UNiagaraSystem* FindPixelVFXSystem()
{
	UNiagaraSystem* Pixel = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle"));
	if (Pixel) return Pixel;
	return LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1"));
}

static FVector4 T66MakeBloodTint(bool bBrightCore)
{
	if (bBrightCore)
	{
		return FVector4(
			FMath::FRandRange(0.80f, 1.00f),
			FMath::FRandRange(0.01f, 0.05f),
			FMath::FRandRange(0.01f, 0.04f),
			1.f);
	}

	return FVector4(
		FMath::FRandRange(0.35f, 0.72f),
		FMath::FRandRange(0.00f, 0.03f),
		FMath::FRandRange(0.00f, 0.03f),
		1.f);
}

static void T66ApplyPixelVFXParams(UNiagaraComponent* NiagaraComponent, const FVector4& Tint, const FVector2D& SpriteSize)
{
	if (!NiagaraComponent)
	{
		return;
	}

	const FLinearColor TintColor(Tint.X, Tint.Y, Tint.Z, Tint.W);
	NiagaraComponent->SetVariableLinearColor(FName(TEXT("User.Tint")), TintColor);
	NiagaraComponent->SetVariableLinearColor(FName(TEXT("User.Color")), TintColor);
	NiagaraComponent->SetVariableVec2(FName(TEXT("User.SpriteSize")), SpriteSize);
}

static UNiagaraComponent* T66SpawnBudgetedPixel(
	UWorld* World,
	UNiagaraSystem* VFX,
	const FVector& Location,
	const FVector4& Tint,
	const FVector2D& SpriteSize,
	ET66PixelVFXPriority Priority,
	const FRotator& Rotation = FRotator::ZeroRotator,
	bool bAutoDestroy = true)
{
	if (!World || !VFX)
	{
		return nullptr;
	}

	if (UT66PixelVFXSubsystem* PixelVFX = World->GetSubsystem<UT66PixelVFXSubsystem>())
	{
		return PixelVFX->SpawnPixelAtLocation(
			Location,
			FLinearColor(Tint.X, Tint.Y, Tint.Z, Tint.W),
			SpriteSize,
			Priority,
			Rotation,
			FVector(1.f),
			VFX,
			bAutoDestroy);
	}

	const ENCPoolMethod PoolingMethod = bAutoDestroy ? ENCPoolMethod::AutoRelease : ENCPoolMethod::None;

	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		VFX,
		Location,
		Rotation,
		FVector(1.f),
		bAutoDestroy,
		true,
		PoolingMethod);
	T66ApplyPixelVFXParams(NiagaraComponent, Tint, SpriteSize);
	return NiagaraComponent;
}

static void T66SpawnBloodSpray(
	UWorld* World,
	UNiagaraSystem* VFX,
	const FVector& Location,
	int32 PixelCount,
	float BurstRadius,
	float CoreRadiusScale)
{
	if (!World || !VFX)
	{
		return;
	}

	const int32 TotalPixels = FMath::Max(PixelCount, 24);
	const int32 CoreCount = FMath::Max(8, FMath::RoundToInt(static_cast<float>(TotalPixels) * 0.25f));
	const int32 JetCount = FMath::Clamp(FMath::RoundToInt(static_cast<float>(TotalPixels) / 8.f), 4, 10);
	const int32 SprayCount = FMath::Max(12, TotalPixels - CoreCount);
	const float CoreRadius = BurstRadius * CoreRadiusScale;

	for (int32 i = 0; i < CoreCount; ++i)
	{
		const FVector Offset = FMath::VRand() * FMath::FRandRange(0.f, CoreRadius);
		T66SpawnBudgetedPixel(
			World,
			VFX,
			Location + Offset,
			T66MakeBloodTint(true),
			FVector2D(FMath::FRandRange(4.0f, 6.0f), FMath::FRandRange(4.0f, 6.0f)),
			ET66PixelVFXPriority::High);
	}

	int32 Remaining = SprayCount;
	for (int32 JetIndex = 0; JetIndex < JetCount; ++JetIndex)
	{
		const int32 JetsLeft = JetCount - JetIndex;
		const int32 JetPixels = FMath::Max(2, Remaining / FMath::Max(1, JetsLeft));
		Remaining -= JetPixels;

		FVector Dir(
			FMath::FRandRange(-1.0f, 1.0f),
			FMath::FRandRange(-1.0f, 1.0f),
			FMath::FRandRange(-0.25f, 0.45f));
		Dir = Dir.GetSafeNormal();
		if (Dir.IsNearlyZero())
		{
			Dir = FVector::ForwardVector;
		}

		FVector Right = FVector::CrossProduct(Dir, FVector::UpVector).GetSafeNormal();
		if (Right.IsNearlyZero())
		{
			Right = FVector::RightVector;
		}
		const FVector Upish = FVector::CrossProduct(Right, Dir).GetSafeNormal();
		const float JetLength = FMath::FRandRange(BurstRadius * 0.30f, BurstRadius * 0.95f);

		for (int32 PixelIndex = 0; PixelIndex < JetPixels; ++PixelIndex)
		{
			const float T = FMath::Clamp(
				(static_cast<float>(PixelIndex) + FMath::FRandRange(0.15f, 0.95f)) / static_cast<float>(JetPixels),
				0.f, 1.f);
			const float Along = JetLength * FMath::Pow(T, 0.72f);
			const float Jitter = FMath::Lerp(BurstRadius * 0.02f, BurstRadius * 0.10f, T);
			const FVector Offset =
				Dir * Along +
				Right * FMath::FRandRange(-Jitter, Jitter) +
				Upish * FMath::FRandRange(-Jitter * 0.35f, Jitter * 0.35f);

			const bool bCoreStreak = T < 0.25f;
			T66SpawnBudgetedPixel(
				World,
				VFX,
				Location + Offset,
				T66MakeBloodTint(bCoreStreak),
				bCoreStreak
					? FVector2D(FMath::FRandRange(3.5f, 5.0f), FMath::FRandRange(3.5f, 5.0f))
					: FVector2D(FMath::FRandRange(2.0f, 3.8f), FMath::FRandRange(2.0f, 3.8f)),
				ET66PixelVFXPriority::High);
		}
	}
}

// ---------------------------------------------------------------------------
// SpawnDeathBurstAtLocation — static, usable from enemies/bosses without a
// CombatComponent. Loads the VFX system on first call and caches it.
// ---------------------------------------------------------------------------
void UT66CombatComponent::SpawnDeathBurstAtLocation(UWorld* World, const FVector& Location, int32 NumParticles, float BurstRadius)
{
	if (!World) return;
	static TWeakObjectPtr<UNiagaraSystem> CachedBloodBurstVFX;
	UNiagaraSystem* BloodBurstVFX = CachedBloodBurstVFX.Get();
	if (!BloodBurstVFX)
	{
		BloodBurstVFX = FindPixelVFXSystem();
		CachedBloodBurstVFX = BloodBurstVFX;
	}
	if (!BloodBurstVFX) return;

	T66SpawnBloodSpray(World, BloodBurstVFX, Location, FMath::Max(NumParticles * 3, 42), BurstRadius, 0.08f);
}

// ---------------------------------------------------------------------------
// SpawnDeathVFX — hero death: larger red burst.
// ---------------------------------------------------------------------------
void UT66CombatComponent::SpawnDeathVFX(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World) return;

	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!VFX) return;

	T66SpawnBloodSpray(World, VFX, Location, 84, 150.f, 0.09f);
}

// ---------------------------------------------------------------------------
// SpawnSlashVFX — crescent arc of tiny pixel particles.
// ---------------------------------------------------------------------------
void UT66CombatComponent::SpawnSlashVFX(const FVector& Location, float Radius, const FLinearColor& Color)
{
	UWorld* World = GetWorld();
	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!World || !VFX) return;

	constexpr int32 NumParticles = 24;
	constexpr float ArcAngleDeg = 120.f;
	constexpr float StartAngleDeg = -ArcAngleDeg * 0.5f;
	constexpr float SpreadScale = 0.35f;
	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);

	for (int32 i = 0; i < NumParticles; ++i)
	{
		const float T = (NumParticles > 1) ? (static_cast<float>(i) / static_cast<float>(NumParticles - 1)) : 0.5f;
		const float AngleRad = FMath::DegreesToRadians(StartAngleDeg + ArcAngleDeg * T);
		const FVector Offset(FMath::Cos(AngleRad) * Radius * SpreadScale, FMath::Sin(AngleRad) * Radius * SpreadScale, 0.f);
		const FVector SpawnLoc = Location + Offset;
		T66SpawnBudgetedPixel(World, VFX, SpawnLoc, ColorVec, FVector2D(2.5f, 2.5f), ET66PixelVFXPriority::Medium);
	}
}

// ---------------------------------------------------------------------------
// SpawnPierceVFX — clean straight line of evenly spaced tiny pixels.
// ---------------------------------------------------------------------------
void UT66CombatComponent::SpawnPierceVFX(const FVector& Start, const FVector& End, const FLinearColor& Color)
{
	UWorld* World = GetWorld();
	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Legacy] Pierce spawn skipped: no world."));
		return;
	}
	if (!VFX)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ATTACK VFX][Legacy] Pierce spawn skipped: no active Niagara system. Start=(%.1f,%.1f,%.1f) End=(%.1f,%.1f,%.1f)"),
			Start.X, Start.Y, Start.Z,
			End.X, End.Y, End.Z);
		return;
	}

	constexpr int32 NumParticles = 40;
	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);

	for (int32 i = 0; i < NumParticles; ++i)
	{
		const float T = (NumParticles > 1) ? (static_cast<float>(i) / static_cast<float>(NumParticles - 1)) : 0.5f;
		const FVector SpawnLoc = FMath::Lerp(Start, End, T);
		T66SpawnBudgetedPixel(World, VFX, SpawnLoc, ColorVec, FVector2D(2.0f, 2.0f), ET66PixelVFXPriority::Medium);
	}
}

void UT66CombatComponent::SpawnHeroOnePierceVFX(const FVector& Start, const FVector& End, const FVector& ImpactLocation)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const int32 RequestId = ++GHeroOneStage1RequestSerial;
	const float TraceLength2D = FVector::Dist2D(Start, End);
	const bool bVerbose = CVarT66VFXHeroOneVerbose.GetValueOnGameThread() != 0;
	const bool bUseExamplePath = CVarT66VFXHeroOnePierceEnabled.GetValueOnGameThread() != 0;
	UNiagaraSystem* LegacyVFX = GetActiveVFXSystem();
	const FString LegacyVFXName = LegacyVFX ? LegacyVFX->GetName() : TEXT("None");
	if (bVerbose)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][Stage1] Hero_1 Pierce request Req=%d Owner=%s Time=%.3f Start=(%.1f,%.1f,%.1f) End=(%.1f,%.1f,%.1f) Impact=(%.1f,%.1f,%.1f) Trace2D=%.1f UseExample=%d LegacyVFX=%s"),
			RequestId,
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			World->GetTimeSeconds(),
			Start.X, Start.Y, Start.Z,
			End.X, End.Y, End.Z,
			ImpactLocation.X, ImpactLocation.Y, ImpactLocation.Z,
			TraceLength2D,
			bUseExamplePath ? 1 : 0,
			*LegacyVFXName);
	}

	if (!bUseExamplePath)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage1] Hero_1 Example path disabled Req=%d via T66.VFX.Hero1PierceExample. Falling back to legacy pierce pixels with VFX=%s."), RequestId, *LegacyVFXName);
		SpawnPierceVFX(Start, End, FLinearColor(1.f, 0.95f, 0.8f, 1.f));
		return;
	}

	if (!IsExamplePierceVFXReady(RequestId, true))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage1] Hero_1 Example path missing materials Req=%d. Falling back to legacy pierce pixels with VFX=%s."), RequestId, *LegacyVFXName);
		SpawnPierceVFX(Start, End, FLinearColor(1.f, 0.95f, 0.8f, 1.f));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66HeroOneAttackVFX* Effect = World->SpawnActor<AT66HeroOneAttackVFX>(
		AT66HeroOneAttackVFX::StaticClass(),
		Start,
		FRotator::ZeroRotator,
		SpawnParams);
	if (Effect)
	{
		if (bVerbose)
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("[ATTACK VFX][Stage1] Hero_1 Example actor spawned Req=%d Actor=%s Owner=%s"),
				RequestId,
				*Effect->GetName(),
				GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
		}
		Effect->InitEffect(Start, End, ImpactLocation, FLinearColor(1.f, 0.97f, 0.88f, 1.f), RequestId, FName(TEXT("Hero_1")));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage1] Hero_1 Example actor spawn failed Req=%d. Falling back to legacy pierce pixels with VFX=%s."), RequestId, *LegacyVFXName);
	SpawnPierceVFX(Start, End, FLinearColor(1.f, 0.95f, 0.8f, 1.f));
}

void UT66CombatComponent::SpawnBounceVFX(const TArray<FVector>& ChainPositions, const FLinearColor& Color)
{
	UWorld* World = GetWorld();
	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!World || !VFX || ChainPositions.Num() < 2) return;

	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);

	for (int32 i = 0; i < ChainPositions.Num() - 1; ++i)
	{
		const FVector ChainStart = ChainPositions[i];
		const FVector ChainEnd   = ChainPositions[i + 1];
		const FVector Dir   = (ChainEnd - ChainStart).GetSafeNormal();
		if (Dir.IsNearlyZero()) continue;
		const FRotator Rot  = Dir.Rotation();
		const float    Dist = FVector::Dist(ChainStart, ChainEnd);

		const int32 Num = FMath::Max(8, FMath::RoundToInt(Dist / 20.f));
		for (int32 j = 0; j < Num; ++j)
		{
			const float T = (Num > 1) ? (static_cast<float>(j) / static_cast<float>(Num - 1)) : 0.5f;
			T66SpawnBudgetedPixel(
				World,
				VFX,
				FMath::Lerp(ChainStart, ChainEnd, T),
				ColorVec,
				FVector2D(2.0f, 2.0f),
				ET66PixelVFXPriority::Medium,
				Rot);
		}

		// Slightly larger impact pixel at each bounce point.
		T66SpawnBudgetedPixel(World, VFX, ChainStart, ColorVec, FVector2D(3.5f, 3.5f), ET66PixelVFXPriority::High);
	}
}

void UT66CombatComponent::SpawnDOTVFX(const FVector& Location, float Duration, float Radius, const FLinearColor& Color)
{
	UWorld* World = GetWorld();
	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!World || !VFX) return;

	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);
	UNiagaraComponent* NC = T66SpawnBudgetedPixel(
		World,
		VFX,
		Location,
		ColorVec,
		FVector2D(2.0f, 2.0f),
		ET66PixelVFXPriority::Medium,
		FRotator::ZeroRotator,
		true);
	if (NC)
	{
		if (AActor* Owner = NC->GetOwner())
			Owner->SetLifeSpan(FMath::Max(0.1f, Duration));
	}
}

void UT66CombatComponent::SpawnIdolPierceVFX(const FName& IdolID, const ET66ItemRarity Rarity, const FVector& Start, const FVector& End, const FVector& ImpactLocation, const float StartDelaySeconds)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const FLinearColor IdolColor = UT66IdolManagerSubsystem::GetIdolColor(IdolID);
	const int32 RequestId = ++GIdolPierceStage3RequestSerial;
	const bool bVerbose = CVarT66VFXIdolPierceVerbose.GetValueOnGameThread() != 0;
	const bool bUseExamplePath = CVarT66VFXIdolPierceExample.GetValueOnGameThread() != 0;
	const float TraceLength2D = FVector::Dist2D(Start, End);

	FLinearColor PaletteTint;
	FLinearColor PalettePrimary;
	FLinearColor PaletteSecondary;
	FLinearColor PaletteOutline;
	float PaletteGlow = 0.f;
	BuildIdolExamplePiercePalette(IdolColor, Rarity, PaletteTint, PalettePrimary, PaletteSecondary, PaletteOutline, PaletteGlow);

	if (bVerbose)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][Stage3] Idol Pierce request Req=%d Idol=%s Rarity=%s Owner=%s Time=%.3f Start=(%.1f,%.1f,%.1f) End=(%.1f,%.1f,%.1f) Impact=(%.1f,%.1f,%.1f) Trace2D=%.1f Delay=%.3f UseExample=%d IdolColor=(%.2f,%.2f,%.2f,%.2f) Tint=(%.2f,%.2f,%.2f,%.2f) Primary=(%.2f,%.2f,%.2f,%.2f) Secondary=(%.2f,%.2f,%.2f,%.2f)"),
			RequestId,
			*IdolID.ToString(),
			GetT66ItemRarityName(Rarity),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			World->GetTimeSeconds(),
			Start.X, Start.Y, Start.Z,
			End.X, End.Y, End.Z,
			ImpactLocation.X, ImpactLocation.Y, ImpactLocation.Z,
			TraceLength2D,
			StartDelaySeconds,
			bUseExamplePath ? 1 : 0,
			IdolColor.R, IdolColor.G, IdolColor.B, IdolColor.A,
			PaletteTint.R, PaletteTint.G, PaletteTint.B, PaletteTint.A,
			PalettePrimary.R, PalettePrimary.G, PalettePrimary.B, PalettePrimary.A,
			PaletteSecondary.R, PaletteSecondary.G, PaletteSecondary.B, PaletteSecondary.A);
	}

	if (bUseExamplePath)
	{
		if (!IsExamplePierceVFXReady(RequestId, true))
		{
			UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage3] Idol Pierce missing materials Req=%d Idol=%s. Falling back to legacy idol pierce actor."), RequestId, *IdolID.ToString());
		}
		else
		{
			FActorSpawnParameters ExampleSpawnParams;
			ExampleSpawnParams.Owner = GetOwner();
			ExampleSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AT66HeroOneAttackVFX* ExampleEffect = World->SpawnActor<AT66HeroOneAttackVFX>(
				AT66HeroOneAttackVFX::StaticClass(),
				Start,
				FRotator::ZeroRotator,
				ExampleSpawnParams);
			if (ExampleEffect)
			{
				if (bVerbose)
				{
					UE_LOG(
						LogTemp,
						Log,
						TEXT("[ATTACK VFX][Stage3] Idol Pierce Example actor spawned Req=%d Idol=%s Actor=%s Owner=%s"),
						RequestId,
						*IdolID.ToString(),
						*ExampleEffect->GetName(),
						GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
				}

				ExampleEffect->SetPaletteOverride(PaletteTint, PalettePrimary, PaletteSecondary, PaletteOutline, PaletteGlow, FName(TEXT("IdolTwoTone")));
				ExampleEffect->InitEffect(Start, End, ImpactLocation, IdolColor, RequestId, IdolID);
				return;
			}

			UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage3] Idol Pierce Example actor spawn failed Req=%d Idol=%s. Falling back to legacy idol pierce actor."), RequestId, *IdolID.ToString());
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66IdolProcVFX* Effect = World->SpawnActor<AT66IdolProcVFX>(AT66IdolProcVFX::StaticClass(), Start, FRotator::ZeroRotator, SpawnParams);
	if (Effect)
	{
		FT66IdolProcVFXRequest Request;
		Request.IdolID = IdolID;
		Request.Rarity = Rarity;
		Request.Category = ET66AttackCategory::Pierce;
		Request.Start = Start;
		Request.End = End;
		Request.Impact = ImpactLocation;
		Request.StartDelay = StartDelaySeconds;
		Effect->InitEffect(Request);
		UE_LOG(LogTemp, Warning, TEXT("[IDOL VFX] Pierce Idol=%s Rarity=%s Delay=%.3f Actor=%s Req=%d"), *IdolID.ToString(), GetT66ItemRarityName(Rarity), StartDelaySeconds, *Effect->GetName(), RequestId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[IDOL VFX] Pierce spawn failed Idol=%s Rarity=%s Req=%d"), *IdolID.ToString(), GetT66ItemRarityName(Rarity), RequestId);
	}

	if (CVarT66VFXIdolLegacyPixels.GetValueOnGameThread() != 0)
	{
		SpawnPierceVFX(Start + FVector(0.f, 0.f, 24.f), End + FVector(0.f, 0.f, 24.f), IdolColor);
	}
}

void UT66CombatComponent::SpawnIdolAOEVFX(const FName& IdolID, const ET66ItemRarity Rarity, const FVector& Location, const float Radius, const float StartDelaySeconds)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const FLinearColor IdolColor = UT66IdolManagerSubsystem::GetIdolColor(IdolID);
	const int32 RequestId = ++GIdolAOEStage7RequestSerial;
	const bool bVerbose = CVarT66VFXIdolAOEVerbose.GetValueOnGameThread() != 0;
	const bool bUseExamplePath = CVarT66VFXIdolAOEExample.GetValueOnGameThread() != 0;

	FLinearColor PaletteTint;
	FLinearColor PalettePrimary;
	FLinearColor PaletteSecondary;
	FLinearColor PaletteOutline;
	float PaletteGlow = 0.f;
	BuildIdolExampleAOEPalette(IdolColor, Rarity, PaletteTint, PalettePrimary, PaletteSecondary, PaletteOutline, PaletteGlow);

	if (bVerbose)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][Stage7] Idol AOE request Req=%d Idol=%s Rarity=%s Owner=%s Time=%.3f Radius=%.1f Delay=%.3f UseExample=%d Center=(%.1f,%.1f,%.1f) IdolColor=(%.2f,%.2f,%.2f,%.2f) Tint=(%.2f,%.2f,%.2f,%.2f) Primary=(%.2f,%.2f,%.2f,%.2f) Secondary=(%.2f,%.2f,%.2f,%.2f)"),
			RequestId,
			*IdolID.ToString(),
			GetT66ItemRarityName(Rarity),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			World->GetTimeSeconds(),
			Radius,
			StartDelaySeconds,
			bUseExamplePath ? 1 : 0,
			Location.X, Location.Y, Location.Z,
			IdolColor.R, IdolColor.G, IdolColor.B, IdolColor.A,
			PaletteTint.R, PaletteTint.G, PaletteTint.B, PaletteTint.A,
			PalettePrimary.R, PalettePrimary.G, PalettePrimary.B, PalettePrimary.A,
			PaletteSecondary.R, PaletteSecondary.G, PaletteSecondary.B, PaletteSecondary.A);
	}

	if (IdolID == FName(TEXT("Idol_Fire")))
	{
		int32 RawFireCandidateValue = 0;
		bool bFireCandidateClamped = false;
		const ET66FireIdolTestCandidate FireCandidate = GetConfiguredFireIdolCandidate(RawFireCandidateValue, bFireCandidateClamped);
		if (bFireCandidateClamped)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[FIRE IDOL TEST] CandidateSelected Req=%d Source=%s RawCVar=%d Candidate=%s Success=0 FailurePoint=CVarSanitize Fallback=None Details=Unsupported candidate value; defaulted to ZibraShockwave."),
				RequestId,
				*IdolID.ToString(),
				RawFireCandidateValue,
				AT66FireIdolAttackVFX::GetCandidateName(FireCandidate));
		}

		FString BindingSummary;
		FString FailureReason;
		const bool bFireCandidateReady = AT66FireIdolAttackVFX::PreflightCandidate(FireCandidate, BindingSummary, FailureReason);
		if (bFireCandidateReady)
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("[FIRE IDOL TEST] CandidatePreflight Req=%d Source=%s Candidate=%s RawCVar=%d Success=1 FailurePoint=None Fallback=None Details=%s"),
				RequestId,
				*IdolID.ToString(),
				AT66FireIdolAttackVFX::GetCandidateName(FireCandidate),
				RawFireCandidateValue,
				*BindingSummary);
		}
		else
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[FIRE IDOL TEST] CandidatePreflight Req=%d Source=%s Candidate=%s RawCVar=%d Success=0 FailurePoint=Preflight Fallback=LegacyIdolProcVFX Details=%s FailureReason=%s"),
				RequestId,
				*IdolID.ToString(),
				AT66FireIdolAttackVFX::GetCandidateName(FireCandidate),
				RawFireCandidateValue,
				*BindingSummary,
				*FailureReason);
		}

		if (!bFireCandidateReady)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage7][Fire] Candidate preflight failed Req=%d Idol=%s. Falling back to idol-specific AOE actor."), RequestId, *IdolID.ToString());
		}
		else
		{
			const FTransform SpawnTransform(FRotator::ZeroRotator, Location);
			AT66FireIdolAttackVFX* FireEffect = World->SpawnActorDeferred<AT66FireIdolAttackVFX>(
				AT66FireIdolAttackVFX::StaticClass(),
				SpawnTransform,
				GetOwner(),
				nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (FireEffect)
			{
				UE_LOG(
					LogTemp,
					Log,
					TEXT("[FIRE IDOL TEST] CandidateSpawned Req=%d Source=%s Candidate=%s Actor=%s Success=1 FailurePoint=None Fallback=None Details=Deferred actor allocated."),
					RequestId,
					*IdolID.ToString(),
					AT66FireIdolAttackVFX::GetCandidateName(FireCandidate),
					*FireEffect->GetName());

				FireEffect->ConfigureCandidate(FireCandidate);
				FireEffect->InitEffect(Location, Radius, IdolColor, RequestId, IdolID);
				UGameplayStatics::FinishSpawningActor(FireEffect, SpawnTransform);
				return;
			}

			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[FIRE IDOL TEST] CandidateSpawned Req=%d Source=%s Candidate=%s Actor=None Success=0 FailurePoint=SpawnActorDeferred Fallback=IdolProcVFX Details=World->SpawnActorDeferred returned null."),
				RequestId,
				*IdolID.ToString(),
				AT66FireIdolAttackVFX::GetCandidateName(FireCandidate));
		}
	}
	else if (TrySpawnIdolVolumeEffect(World, GetOwner(), IdolID, ET66AttackCategory::AOE, nullptr, Location, Radius, 0.f, IdolColor, RequestId))
	{
		return;
	}
	else if (bUseExamplePath)
	{
		if (!IsExampleAOEVFXReady(RequestId, true, IdolID))
		{
			UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage7] Idol AOE missing materials Req=%d Idol=%s. Falling back to idol-specific AOE actor."), RequestId, *IdolID.ToString());
		}
		else
		{
			FActorSpawnParameters ExampleSpawnParams;
			ExampleSpawnParams.Owner = GetOwner();
			ExampleSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AT66HeroAOEAttackVFX* ExampleEffect = World->SpawnActor<AT66HeroAOEAttackVFX>(
				AT66HeroAOEAttackVFX::StaticClass(),
				Location,
				FRotator::ZeroRotator,
				ExampleSpawnParams);
			if (ExampleEffect)
			{
				if (bVerbose)
				{
					UE_LOG(
						LogTemp,
						Log,
						TEXT("[ATTACK VFX][Stage7] Idol AOE Example actor spawned Req=%d Idol=%s Actor=%s Owner=%s"),
						RequestId,
						*IdolID.ToString(),
						*ExampleEffect->GetName(),
						GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
				}

				ExampleEffect->SetPaletteOverride(PaletteTint, PalettePrimary, PaletteSecondary, PaletteOutline, PaletteGlow, FName(TEXT("IdolTwoTone")));
				ExampleEffect->InitEffect(Location, Radius, IdolColor, RequestId, IdolID);
				return;
			}

			UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage7] Idol AOE Example actor spawn failed Req=%d Idol=%s. Falling back to idol-specific AOE actor."), RequestId, *IdolID.ToString());
		}
	}
	else if (IdolID == FName(TEXT("Idol_Fire")))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[FIRE IDOL TEST] CandidateBypassed Req=%d Source=%s Candidate=None Success=0 FailurePoint=Unknown Fallback=IdolProcVFX Details=The fire test candidate path was unavailable so the idol-specific AOE actor was used."),
			RequestId,
			*IdolID.ToString());
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66IdolProcVFX* Effect = World->SpawnActor<AT66IdolProcVFX>(AT66IdolProcVFX::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
	if (Effect)
	{
		FT66IdolProcVFXRequest Request;
		Request.IdolID = IdolID;
		Request.Rarity = Rarity;
		Request.Category = ET66AttackCategory::AOE;
		Request.Center = Location;
		Request.Radius = Radius;
		Request.StartDelay = StartDelaySeconds;
		Effect->InitEffect(Request);
		UE_LOG(LogTemp, Warning, TEXT("[IDOL VFX] AOE Idol=%s Rarity=%s Radius=%.1f Delay=%.3f Actor=%s Req=%d"), *IdolID.ToString(), GetT66ItemRarityName(Rarity), Radius, StartDelaySeconds, *Effect->GetName(), RequestId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[IDOL VFX] AOE spawn failed Idol=%s Rarity=%s Req=%d"), *IdolID.ToString(), GetT66ItemRarityName(Rarity), RequestId);
	}

	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (VFX && CVarT66VFXIdolLegacyPixels.GetValueOnGameThread() != 0)
	{
		const FVector4 ColorVec(IdolColor.R, IdolColor.G, IdolColor.B, IdolColor.A);
		const FVector RingCenter = Location + FVector(0.f, 0.f, 12.f);
		const float RingRadius = FMath::Max(40.f, Radius * 0.34f);
		const int32 RingCount = (Rarity == ET66ItemRarity::White) ? 40 : 28;
		for (int32 PixelIndex = 0; PixelIndex < RingCount; ++PixelIndex)
		{
			const float Angle = (2.f * PI * static_cast<float>(PixelIndex)) / static_cast<float>(RingCount);
			const FVector Offset(FMath::Cos(Angle) * RingRadius, FMath::Sin(Angle) * RingRadius, 0.f);
			T66SpawnBudgetedPixel(World, VFX, RingCenter + Offset, ColorVec, FVector2D(3.0f, 3.0f), ET66PixelVFXPriority::High);
		}

		const int32 BurstCount = (Rarity == ET66ItemRarity::White) ? 18 : 10;
		for (int32 PixelIndex = 0; PixelIndex < BurstCount; ++PixelIndex)
		{
			const FVector Offset(
				FMath::FRandRange(-RingRadius * 0.35f, RingRadius * 0.35f),
				FMath::FRandRange(-RingRadius * 0.35f, RingRadius * 0.35f),
				FMath::FRandRange(0.f, 18.f));
			T66SpawnBudgetedPixel(World, VFX, RingCenter + Offset, ColorVec, FVector2D(3.5f, 3.5f), ET66PixelVFXPriority::High);
		}
	}
}

void UT66CombatComponent::SpawnIdolBounceVFX(const FName& IdolID, const ET66ItemRarity Rarity, const TArray<FVector>& ChainPositions, const float StartDelaySeconds)
{
	UWorld* World = GetWorld();
	if (!World || ChainPositions.Num() < 2)
	{
		return;
	}
	const FLinearColor IdolColor = UT66IdolManagerSubsystem::GetIdolColor(IdolID);
	const int32 RequestId = ++GIdolBounceStage8RequestSerial;
	const bool bVerbose = CVarT66VFXIdolBounceVerbose.GetValueOnGameThread() != 0;
	const bool bUseExamplePath = CVarT66VFXIdolBounceExample.GetValueOnGameThread() != 0;

	FLinearColor PaletteTint;
	FLinearColor PalettePrimary;
	FLinearColor PaletteSecondary;
	FLinearColor PaletteOutline;
	float PaletteGlow = 0.f;
	BuildIdolExampleBouncePalette(IdolColor, Rarity, PaletteTint, PalettePrimary, PaletteSecondary, PaletteOutline, PaletteGlow);

	if (bVerbose)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][Stage8] Idol Bounce request Req=%d Idol=%s Rarity=%s Owner=%s Time=%.3f Links=%d Delay=%.3f UseExample=%d Start=(%.1f,%.1f,%.1f) End=(%.1f,%.1f,%.1f) IdolColor=(%.2f,%.2f,%.2f,%.2f) Tint=(%.2f,%.2f,%.2f,%.2f) Primary=(%.2f,%.2f,%.2f,%.2f) Secondary=(%.2f,%.2f,%.2f,%.2f)"),
			RequestId,
			*IdolID.ToString(),
			GetT66ItemRarityName(Rarity),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			World->GetTimeSeconds(),
			ChainPositions.Num() - 1,
			StartDelaySeconds,
			bUseExamplePath ? 1 : 0,
			ChainPositions[0].X, ChainPositions[0].Y, ChainPositions[0].Z,
			ChainPositions.Last().X, ChainPositions.Last().Y, ChainPositions.Last().Z,
			IdolColor.R, IdolColor.G, IdolColor.B, IdolColor.A,
			PaletteTint.R, PaletteTint.G, PaletteTint.B, PaletteTint.A,
			PalettePrimary.R, PalettePrimary.G, PalettePrimary.B, PalettePrimary.A,
			PaletteSecondary.R, PaletteSecondary.G, PaletteSecondary.B, PaletteSecondary.A);
	}

	if (bUseExamplePath)
	{
		if (!IsExampleBounceVFXReady(RequestId, true))
		{
			UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage8] Idol Bounce missing materials Req=%d Idol=%s. Falling back to legacy idol bounce actor."), RequestId, *IdolID.ToString());
		}
		else
		{
			FActorSpawnParameters ExampleSpawnParams;
			ExampleSpawnParams.Owner = GetOwner();
			ExampleSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AT66HeroBounceAttackVFX* ExampleEffect = World->SpawnActor<AT66HeroBounceAttackVFX>(
				AT66HeroBounceAttackVFX::StaticClass(),
				ChainPositions[0],
				FRotator::ZeroRotator,
				ExampleSpawnParams);
			if (ExampleEffect)
			{
				if (bVerbose)
				{
					UE_LOG(
						LogTemp,
						Log,
						TEXT("[ATTACK VFX][Stage8] Idol Bounce Example actor spawned Req=%d Idol=%s Actor=%s Owner=%s"),
						RequestId,
						*IdolID.ToString(),
						*ExampleEffect->GetName(),
						GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
				}

				ExampleEffect->SetPaletteOverride(PaletteTint, PalettePrimary, PaletteSecondary, PaletteOutline, PaletteGlow, FName(TEXT("IdolTwoTone")));
				ExampleEffect->InitEffect(ChainPositions, IdolColor, RequestId, IdolID);
				return;
			}

			UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage8] Idol Bounce Example actor spawn failed Req=%d Idol=%s. Falling back to legacy idol bounce actor."), RequestId, *IdolID.ToString());
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66IdolProcVFX* Effect = World->SpawnActor<AT66IdolProcVFX>(AT66IdolProcVFX::StaticClass(), ChainPositions[0], FRotator::ZeroRotator, SpawnParams);
	if (Effect)
	{
		FT66IdolProcVFXRequest Request;
		Request.IdolID = IdolID;
		Request.Rarity = Rarity;
		Request.Category = ET66AttackCategory::Bounce;
		Request.ChainPositions = ChainPositions;
		Request.StartDelay = StartDelaySeconds;
		Effect->InitEffect(Request);
		UE_LOG(LogTemp, Warning, TEXT("[IDOL VFX] Bounce Idol=%s Rarity=%s Segments=%d Delay=%.3f Actor=%s Req=%d"), *IdolID.ToString(), GetT66ItemRarityName(Rarity), ChainPositions.Num() - 1, StartDelaySeconds, *Effect->GetName(), RequestId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[IDOL VFX] Bounce spawn failed Idol=%s Rarity=%s Req=%d"), *IdolID.ToString(), GetT66ItemRarityName(Rarity), RequestId);
	}

	TArray<FVector> ElevatedPositions;
	ElevatedPositions.Reserve(ChainPositions.Num());
	for (const FVector& Pos : ChainPositions)
	{
		ElevatedPositions.Add(Pos + FVector(0.f, 0.f, 24.f));
	}
	if (CVarT66VFXIdolLegacyPixels.GetValueOnGameThread() != 0)
	{
		SpawnBounceVFX(ElevatedPositions, IdolColor);
	}
}

void UT66CombatComponent::SpawnIdolDOTVFX(const FName& IdolID, const ET66ItemRarity Rarity, AActor* FollowTarget, const FVector& Location, const float Duration, const float Radius, const float StartDelaySeconds)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const FLinearColor IdolColor = UT66IdolManagerSubsystem::GetIdolColor(IdolID);
	const int32 RequestId = ++GIdolDOTStage9RequestSerial;
	const bool bVerbose = CVarT66VFXIdolDOTVerbose.GetValueOnGameThread() != 0;
	const bool bUseExamplePath = CVarT66VFXIdolDOTExample.GetValueOnGameThread() != 0;

	FLinearColor PaletteTint;
	FLinearColor PalettePrimary;
	FLinearColor PaletteSecondary;
	FLinearColor PaletteOutline;
	float PaletteGlow = 0.f;
	BuildIdolExampleDOTPalette(IdolColor, Rarity, PaletteTint, PalettePrimary, PaletteSecondary, PaletteOutline, PaletteGlow);

	if (bVerbose)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][Stage9] Idol DOT request Req=%d Idol=%s Rarity=%s Owner=%s Time=%.3f Follow=%d Duration=%.2f Radius=%.1f Delay=%.3f UseExample=%d Location=(%.1f,%.1f,%.1f) IdolColor=(%.2f,%.2f,%.2f,%.2f) Tint=(%.2f,%.2f,%.2f,%.2f) Primary=(%.2f,%.2f,%.2f,%.2f) Secondary=(%.2f,%.2f,%.2f,%.2f)"),
			RequestId,
			*IdolID.ToString(),
			GetT66ItemRarityName(Rarity),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			World->GetTimeSeconds(),
			FollowTarget ? 1 : 0,
			Duration,
			Radius,
			StartDelaySeconds,
			bUseExamplePath ? 1 : 0,
			Location.X, Location.Y, Location.Z,
			IdolColor.R, IdolColor.G, IdolColor.B, IdolColor.A,
			PaletteTint.R, PaletteTint.G, PaletteTint.B, PaletteTint.A,
			PalettePrimary.R, PalettePrimary.G, PalettePrimary.B, PalettePrimary.A,
			PaletteSecondary.R, PaletteSecondary.G, PaletteSecondary.B, PaletteSecondary.A);
	}

	if (TrySpawnIdolVolumeEffect(World, GetOwner(), IdolID, ET66AttackCategory::DOT, FollowTarget, Location, Radius, Duration, IdolColor, RequestId))
	{
		return;
	}

	if (bUseExamplePath)
	{
		if (!IsExampleDOTVFXReady(RequestId, true))
		{
			UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage9] Idol DOT missing materials Req=%d Idol=%s. Falling back to idol-specific DOT actor."), RequestId, *IdolID.ToString());
		}
		else
		{
			FActorSpawnParameters ExampleSpawnParams;
			ExampleSpawnParams.Owner = GetOwner();
			ExampleSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AT66HeroDOTAttackVFX* ExampleEffect = World->SpawnActor<AT66HeroDOTAttackVFX>(
				AT66HeroDOTAttackVFX::StaticClass(),
				Location,
				FRotator::ZeroRotator,
				ExampleSpawnParams);
			if (ExampleEffect)
			{
				if (bVerbose)
				{
					UE_LOG(
						LogTemp,
						Log,
						TEXT("[ATTACK VFX][Stage9] Idol DOT Example actor spawned Req=%d Idol=%s Actor=%s Owner=%s"),
						RequestId,
						*IdolID.ToString(),
						*ExampleEffect->GetName(),
						GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
				}

				ExampleEffect->SetPaletteOverride(PaletteTint, PalettePrimary, PaletteSecondary, PaletteOutline, PaletteGlow, FName(TEXT("IdolTwoTone")));
				ExampleEffect->InitEffect(FollowTarget, Location, Duration, Radius, IdolColor, RequestId, IdolID);
				return;
			}

			UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage9] Idol DOT Example actor spawn failed Req=%d Idol=%s. Falling back to idol-specific DOT actor."), RequestId, *IdolID.ToString());
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66IdolProcVFX* Effect = World->SpawnActor<AT66IdolProcVFX>(AT66IdolProcVFX::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
	if (Effect)
	{
		FT66IdolProcVFXRequest Request;
		Request.IdolID = IdolID;
		Request.Rarity = Rarity;
		Request.Category = ET66AttackCategory::DOT;
		Request.Center = Location;
		Request.Duration = Duration;
		Request.Radius = Radius;
		Request.StartDelay = StartDelaySeconds;
		Request.FollowTarget = FollowTarget;
		Effect->InitEffect(Request);
		UE_LOG(LogTemp, Warning, TEXT("[IDOL VFX] DOT Idol=%s Rarity=%s Duration=%.2f Delay=%.3f Actor=%s Req=%d"), *IdolID.ToString(), GetT66ItemRarityName(Rarity), Duration, StartDelaySeconds, *Effect->GetName(), RequestId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[IDOL VFX] DOT spawn failed Idol=%s Rarity=%s Req=%d"), *IdolID.ToString(), GetT66ItemRarityName(Rarity), RequestId);
	}

	if (CVarT66VFXIdolLegacyPixels.GetValueOnGameThread() != 0)
	{
		SpawnDOTVFX(Location + FVector(0.f, 0.f, 28.f), FMath::Min(Duration, 1.6f), Radius, IdolColor);
	}
}

// ---------------------------------------------------------------------------
// Hero-specific VFX: unique pixel shapes per HeroID.
// ---------------------------------------------------------------------------

void UT66CombatComponent::SpawnHeroPierceVFX(const FVector& Start, const FVector& End, const FVector& ImpactLocation, const FLinearColor& Color, const FName& HeroID)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const int32 RequestId = ++GHeroPierceStage2RequestSerial;
	const bool bVerbose = CVarT66VFXHeroPierceVerbose.GetValueOnGameThread() != 0;
	const bool bUseExamplePath = CVarT66VFXHeroPierceExample.GetValueOnGameThread() != 0 && HeroID != FName(TEXT("Hero_1"));
	UNiagaraSystem* LegacyVFX = GetActiveVFXSystem();
	const FString LegacyVFXName = LegacyVFX ? LegacyVFX->GetName() : TEXT("None");
	const float TraceLength2D = FVector::Dist2D(Start, End);
	const FLinearColor ExampleTint = BuildExamplePierceTint(Color);

	if (bVerbose)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][Stage2] Hero Pierce request Req=%d Hero=%s Owner=%s Time=%.3f Start=(%.1f,%.1f,%.1f) End=(%.1f,%.1f,%.1f) Impact=(%.1f,%.1f,%.1f) Trace2D=%.1f UseExample=%d LegacyVFX=%s BaseTint=(%.2f,%.2f,%.2f,%.2f) ExampleTint=(%.2f,%.2f,%.2f,%.2f)"),
			RequestId,
			*HeroID.ToString(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			World->GetTimeSeconds(),
			Start.X, Start.Y, Start.Z,
			End.X, End.Y, End.Z,
			ImpactLocation.X, ImpactLocation.Y, ImpactLocation.Z,
			TraceLength2D,
			bUseExamplePath ? 1 : 0,
			*LegacyVFXName,
			Color.R, Color.G, Color.B, Color.A,
			ExampleTint.R, ExampleTint.G, ExampleTint.B, ExampleTint.A);
	}

	if (bUseExamplePath)
	{
		if (!IsExamplePierceVFXReady(RequestId, true))
		{
			UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage2] Hero Pierce missing materials Req=%d Hero=%s. Falling back to legacy hero pierce pixels with VFX=%s."), RequestId, *HeroID.ToString(), *LegacyVFXName);
		}
		else
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AT66HeroOneAttackVFX* Effect = World->SpawnActor<AT66HeroOneAttackVFX>(
				AT66HeroOneAttackVFX::StaticClass(),
				Start,
				FRotator::ZeroRotator,
				SpawnParams);
			if (Effect)
			{
				if (bVerbose)
				{
					UE_LOG(
						LogTemp,
						Log,
						TEXT("[ATTACK VFX][Stage2] Hero Pierce Example actor spawned Req=%d Hero=%s Actor=%s Owner=%s"),
						RequestId,
						*HeroID.ToString(),
						*Effect->GetName(),
						GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
				}
				Effect->InitEffect(Start, End, ImpactLocation, Color, RequestId, HeroID);
				return;
			}

			UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage2] Hero Pierce Example actor spawn failed Req=%d Hero=%s. Falling back to legacy hero pierce pixels with VFX=%s."), RequestId, *HeroID.ToString(), *LegacyVFXName);
		}
	}

	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!World || !VFX)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage2] Hero Pierce legacy fallback unavailable Req=%d Hero=%s LegacyVFX=%s"), RequestId, *HeroID.ToString(), *LegacyVFXName);
		return;
	}

	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);
	const FVector Dir = (End - Start).GetSafeNormal();
	const float Length = FVector::Dist(Start, End);

	if (HeroID == FName(TEXT("Hero_1")))
	{
		SpawnHeroOnePierceVFX(Start, End, ImpactLocation);
		return;
	}
	if (HeroID == FName(TEXT("Hero_5")))
	{
		// George: Tight needle — 48 pixels in a very narrow line
		constexpr int32 N = 48;
		for (int32 i = 0; i < N; ++i)
		{
			const float T = static_cast<float>(i) / static_cast<float>(N - 1);
			T66SpawnBudgetedPixel(World, VFX, FMath::Lerp(Start, End, T), ColorVec, FVector2D(1.5f, 1.5f), ET66PixelVFXPriority::Medium);
		}
	}
	else if (HeroID == FName(TEXT("Hero_8")))
	{
		// Billy: Short shotgun burst — 16 pixels in a 45° cone
		constexpr int32 N = 16;
		constexpr float ConeDeg = 45.f;
		for (int32 i = 0; i < N; ++i)
		{
			const float T = (N > 1) ? (static_cast<float>(i) / static_cast<float>(N - 1)) : 0.5f;
			const float Angle = -ConeDeg * 0.5f + ConeDeg * T;
			const FVector ShotDir = Dir.RotateAngleAxis(Angle, FVector::UpVector);
			const float Dist = FMath::FRandRange(Length * 0.2f, Length * 0.6f);
			T66SpawnBudgetedPixel(World, VFX, Start + ShotDir * Dist, ColorVec, FVector2D(2.0f, 2.0f), ET66PixelVFXPriority::Medium);
		}
	}
	else if (HeroID == FName(TEXT("Hero_11")))
	{
		// Shroud: Twin lines — two parallel lines, 8 pixels each
		const FVector Right = FVector::CrossProduct(FVector::UpVector, Dir).GetSafeNormal() * 30.f;
		constexpr int32 N = 8;
		for (int32 Line = 0; Line < 2; ++Line)
		{
			const FVector Offset = (Line == 0) ? Right : -Right;
			for (int32 i = 0; i < N; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(N - 1);
				T66SpawnBudgetedPixel(World, VFX, FMath::Lerp(Start + Offset, End + Offset, T), ColorVec, FVector2D(2.0f, 2.0f), ET66PixelVFXPriority::Medium);
			}
		}
	}
	else
	{
		SpawnPierceVFX(Start, End, Color);
	}
}

void UT66CombatComponent::SpawnHeroSlashVFX(const FVector& Location, float Radius, const FLinearColor& Color, const FName& HeroID)
{
	UWorld* World = GetWorld();
	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!World) return;

	const int32 RequestId = ++GHeroAOEStage4RequestSerial;
	const bool bVerbose = CVarT66VFXHeroAOEVerbose.GetValueOnGameThread() != 0;
	const bool bUseExamplePath = CVarT66VFXHeroAOEExample.GetValueOnGameThread() != 0;
	const FString LegacyVFXName = VFX ? VFX->GetName() : TEXT("None");

	if (bVerbose)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][Stage4] Hero AOE request Req=%d Hero=%s Owner=%s Time=%.3f Location=(%.1f,%.1f,%.1f) Radius=%.1f UseExample=%d LegacyVFX=%s BaseTint=(%.2f,%.2f,%.2f,%.2f)"),
			RequestId,
			*HeroID.ToString(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			World->GetTimeSeconds(),
			Location.X, Location.Y, Location.Z,
			Radius,
			bUseExamplePath ? 1 : 0,
			*LegacyVFXName,
			Color.R, Color.G, Color.B, Color.A);
	}

	if (bUseExamplePath)
	{
		if (!IsExampleAOEVFXReady(RequestId, true, HeroID))
		{
			UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage4] Hero AOE missing materials Req=%d Hero=%s. Falling back to legacy slash pixels with VFX=%s."), RequestId, *HeroID.ToString(), *LegacyVFXName);
		}
		else
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AT66HeroAOEAttackVFX* Effect = World->SpawnActor<AT66HeroAOEAttackVFX>(
				AT66HeroAOEAttackVFX::StaticClass(),
				Location,
				FRotator::ZeroRotator,
				SpawnParams);
			if (Effect)
			{
				if (bVerbose)
				{
					UE_LOG(
						LogTemp,
						Log,
						TEXT("[ATTACK VFX][Stage4] Hero AOE Example actor spawned Req=%d Hero=%s Actor=%s Owner=%s"),
						RequestId,
						*HeroID.ToString(),
						*Effect->GetName(),
						GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
				}
				Effect->InitEffect(Location, Radius, Color, RequestId, HeroID);
				return;
			}

			UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage4] Hero AOE Example actor spawn failed Req=%d Hero=%s. Falling back to legacy slash pixels with VFX=%s."), RequestId, *HeroID.ToString(), *LegacyVFXName);
		}
	}

	if (!VFX)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage4] Hero AOE legacy fallback unavailable Req=%d Hero=%s LegacyVFX=%s"), RequestId, *HeroID.ToString(), *LegacyVFXName);
		return;
	}

	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);

	if (HeroID == FName(TEXT("Hero_3")))
	{
		// Lu Bu: Wide cleave — 28 pixels in full 180° arc
		constexpr int32 N = 28;
		constexpr float ArcDeg = 180.f;
		for (int32 i = 0; i < N; ++i)
		{
			const float T = static_cast<float>(i) / static_cast<float>(N - 1);
			const float Angle = FMath::DegreesToRadians(-ArcDeg * 0.5f + ArcDeg * T);
			const FVector Offset(FMath::Cos(Angle) * Radius * 0.4f, FMath::Sin(Angle) * Radius * 0.4f, 0.f);
			T66SpawnBudgetedPixel(World, VFX, Location + Offset, ColorVec, FVector2D(2.5f, 2.5f), ET66PixelVFXPriority::Medium);
		}
	}
	else if (HeroID == FName(TEXT("Hero_4")))
	{
		// Mike: Tight burst — 16 pixels in a small 90° cone
		constexpr int32 N = 16;
		constexpr float ArcDeg = 90.f;
		for (int32 i = 0; i < N; ++i)
		{
			const float T = static_cast<float>(i) / static_cast<float>(N - 1);
			const float Angle = FMath::DegreesToRadians(-ArcDeg * 0.5f + ArcDeg * T);
			const float R = Radius * 0.25f * FMath::FRandRange(0.5f, 1.f);
			const FVector Offset(FMath::Cos(Angle) * R, FMath::Sin(Angle) * R, 0.f);
			T66SpawnBudgetedPixel(World, VFX, Location + Offset, ColorVec, FVector2D(3.0f, 3.0f), ET66PixelVFXPriority::Medium);
		}
	}
	else if (HeroID == FName(TEXT("Hero_10")))
	{
		// Dog: Ring slam — 32 pixels in a full 360° ring around target
		constexpr int32 N = 32;
		for (int32 i = 0; i < N; ++i)
		{
			const float Angle = (2.f * PI * static_cast<float>(i)) / static_cast<float>(N);
			const FVector Offset(FMath::Cos(Angle) * Radius * 0.35f, FMath::Sin(Angle) * Radius * 0.35f, 0.f);
			T66SpawnBudgetedPixel(World, VFX, Location + Offset, ColorVec, FVector2D(2.0f, 2.0f), ET66PixelVFXPriority::Medium);
		}
	}
	else if (HeroID == FName(TEXT("Hero_15")))
	{
		// Asmon: Double arc — inner 12 + outer 12 at 120° and 150°
		constexpr int32 N = 12;
		for (int32 i = 0; i < N; ++i)
		{
			const float T = static_cast<float>(i) / static_cast<float>(N - 1);
			const float InnerAngle = FMath::DegreesToRadians(-60.f + 120.f * T);
			const FVector InnerOff(FMath::Cos(InnerAngle) * Radius * 0.25f, FMath::Sin(InnerAngle) * Radius * 0.25f, 0.f);
			T66SpawnBudgetedPixel(World, VFX, Location + InnerOff, ColorVec, FVector2D(2.0f, 2.0f), ET66PixelVFXPriority::Medium);

			const float OuterAngle = FMath::DegreesToRadians(-75.f + 150.f * T);
			const FVector OuterOff(FMath::Cos(OuterAngle) * Radius * 0.4f, FMath::Sin(OuterAngle) * Radius * 0.4f, 0.f);
			T66SpawnBudgetedPixel(World, VFX, Location + OuterOff, FVector4(0.95f, 0.85f, 0.1f, 1.f), FVector2D(2.5f, 2.5f), ET66PixelVFXPriority::Medium);
		}
	}
	else
	{
		SpawnSlashVFX(Location, Radius, Color);
	}
}

void UT66CombatComponent::SpawnHeroBounceVFX(const TArray<FVector>& ChainPositions, const FLinearColor& Color, const FName& HeroID)
{
	UWorld* World = GetWorld();
	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!World || ChainPositions.Num() < 2) return;

	const int32 RequestId = ++GHeroBounceStage5RequestSerial;
	const bool bVerbose = CVarT66VFXHeroBounceVerbose.GetValueOnGameThread() != 0;
	const bool bUseExamplePath = CVarT66VFXHeroBounceExample.GetValueOnGameThread() != 0;
	const FString LegacyVFXName = VFX ? VFX->GetName() : TEXT("None");

	if (bVerbose)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][Stage5] Hero Bounce request Req=%d Hero=%s Owner=%s Time=%.3f Links=%d UseExample=%d LegacyVFX=%s BaseTint=(%.2f,%.2f,%.2f,%.2f) Start=(%.1f,%.1f,%.1f) End=(%.1f,%.1f,%.1f)"),
			RequestId,
			*HeroID.ToString(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			World->GetTimeSeconds(),
			ChainPositions.Num() - 1,
			bUseExamplePath ? 1 : 0,
			*LegacyVFXName,
			Color.R, Color.G, Color.B, Color.A,
			ChainPositions[0].X, ChainPositions[0].Y, ChainPositions[0].Z,
			ChainPositions.Last().X, ChainPositions.Last().Y, ChainPositions.Last().Z);
	}

	if (bUseExamplePath)
	{
		if (!IsExampleBounceVFXReady(RequestId, true))
		{
			UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage5] Hero Bounce missing materials Req=%d Hero=%s. Falling back to legacy bounce pixels with VFX=%s."), RequestId, *HeroID.ToString(), *LegacyVFXName);
		}
		else
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AT66HeroBounceAttackVFX* Effect = World->SpawnActor<AT66HeroBounceAttackVFX>(
				AT66HeroBounceAttackVFX::StaticClass(),
				ChainPositions[0],
				FRotator::ZeroRotator,
				SpawnParams);
			if (Effect)
			{
				if (bVerbose)
				{
					UE_LOG(
						LogTemp,
						Log,
						TEXT("[ATTACK VFX][Stage5] Hero Bounce Example actor spawned Req=%d Hero=%s Actor=%s Owner=%s"),
						RequestId,
						*HeroID.ToString(),
						*Effect->GetName(),
						GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
				}
				Effect->InitEffect(ChainPositions, Color, RequestId, HeroID);
				return;
			}

			UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage5] Hero Bounce Example actor spawn failed Req=%d Hero=%s. Falling back to legacy bounce pixels with VFX=%s."), RequestId, *HeroID.ToString(), *LegacyVFXName);
		}
	}

	if (!VFX)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage5] Hero Bounce legacy fallback unavailable Req=%d Hero=%s LegacyVFX=%s"), RequestId, *HeroID.ToString(), *LegacyVFXName);
		return;
	}

	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);

	if (HeroID == FName(TEXT("Hero_6")))
	{
		// Zeus: Long chain — dense trail (1 pixel per 15u)
		for (int32 i = 0; i < ChainPositions.Num() - 1; ++i)
		{
			const FVector A = ChainPositions[i];
			const FVector B = ChainPositions[i + 1];
			const float Dist = FVector::Dist(A, B);
			const int32 N = FMath::Max(4, FMath::RoundToInt(Dist / 15.f));
			for (int32 j = 0; j < N; ++j)
			{
				const float T = static_cast<float>(j) / static_cast<float>(N - 1);
				T66SpawnBudgetedPixel(World, VFX, FMath::Lerp(A, B, T), ColorVec, FVector2D(1.5f, 1.5f), ET66PixelVFXPriority::Medium);
			}
		}
	}
	else if (HeroID == FName(TEXT("Hero_7")))
	{
		// Robo: Heavy bolts — fewer, larger pixels (3.5px)
		for (int32 i = 0; i < ChainPositions.Num() - 1; ++i)
		{
			const FVector A = ChainPositions[i];
			const FVector B = ChainPositions[i + 1];
			const float Dist = FVector::Dist(A, B);
			const int32 N = FMath::Max(3, FMath::RoundToInt(Dist / 60.f));
			for (int32 j = 0; j < N; ++j)
			{
				const float T = static_cast<float>(j) / static_cast<float>(N - 1);
				T66SpawnBudgetedPixel(World, VFX, FMath::Lerp(A, B, T), ColorVec, FVector2D(3.5f, 3.5f), ET66PixelVFXPriority::Medium);
			}
		}
	}
	else if (HeroID == FName(TEXT("Hero_12")))
	{
		// xQc: Scatter bounce — fast, tiny (1.5px) with random offset
		for (int32 i = 0; i < ChainPositions.Num() - 1; ++i)
		{
			const FVector A = ChainPositions[i];
			const FVector B = ChainPositions[i + 1];
			const float Dist = FVector::Dist(A, B);
			const int32 N = FMath::Max(6, FMath::RoundToInt(Dist / 15.f));
			for (int32 j = 0; j < N; ++j)
			{
				const float T = static_cast<float>(j) / static_cast<float>(N - 1);
				const FVector Jitter(FMath::FRandRange(-20.f, 20.f), FMath::FRandRange(-20.f, 20.f), FMath::FRandRange(-10.f, 10.f));
				T66SpawnBudgetedPixel(World, VFX, FMath::Lerp(A, B, T) + Jitter, ColorVec, FVector2D(1.5f, 1.5f), ET66PixelVFXPriority::Medium);
			}
		}
	}
	else if (HeroID == FName(TEXT("Hero_9")))
	{
		// Rabbit: Quick ricochet — very small (1.5px), sparse
		for (int32 i = 0; i < ChainPositions.Num() - 1; ++i)
		{
			const FVector A = ChainPositions[i];
			const FVector B = ChainPositions[i + 1];
			const float Dist = FVector::Dist(A, B);
			const int32 N = FMath::Max(4, FMath::RoundToInt(Dist / 25.f));
			for (int32 j = 0; j < N; ++j)
			{
				const float T = static_cast<float>(j) / static_cast<float>(N - 1);
				T66SpawnBudgetedPixel(World, VFX, FMath::Lerp(A, B, T), ColorVec, FVector2D(1.5f, 1.5f), ET66PixelVFXPriority::Medium);
			}
		}
	}
	else
	{
		SpawnBounceVFX(ChainPositions, Color);
	}
}

void UT66CombatComponent::SpawnHeroDOTVFX(AActor* FollowTarget, const FVector& Location, float Duration, float Radius, const FLinearColor& Color, const FName& HeroID)
{
	UWorld* World = GetWorld();
	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!World) return;

	const int32 RequestId = ++GHeroDOTStage6RequestSerial;
	const bool bVerbose = CVarT66VFXHeroDOTVerbose.GetValueOnGameThread() != 0;
	const bool bUseExamplePath = CVarT66VFXHeroDOTExample.GetValueOnGameThread() != 0;
	const FString LegacyVFXName = VFX ? VFX->GetName() : TEXT("None");

	if (bVerbose)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ATTACK VFX][Stage6] Hero DOT request Req=%d Hero=%s Owner=%s Time=%.3f Follow=%d UseExample=%d LegacyVFX=%s Location=(%.1f,%.1f,%.1f) Duration=%.2f Radius=%.1f BaseTint=(%.2f,%.2f,%.2f,%.2f)"),
			RequestId,
			*HeroID.ToString(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			World->GetTimeSeconds(),
			FollowTarget ? 1 : 0,
			bUseExamplePath ? 1 : 0,
			*LegacyVFXName,
			Location.X, Location.Y, Location.Z,
			Duration,
			Radius,
			Color.R, Color.G, Color.B, Color.A);
	}

	if (bUseExamplePath)
	{
		if (!IsExampleDOTVFXReady(RequestId, true))
		{
			UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage6] Hero DOT missing materials Req=%d Hero=%s. Falling back to legacy DOT pixels with VFX=%s."), RequestId, *HeroID.ToString(), *LegacyVFXName);
		}
		else
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AT66HeroDOTAttackVFX* Effect = World->SpawnActor<AT66HeroDOTAttackVFX>(
				AT66HeroDOTAttackVFX::StaticClass(),
				Location,
				FRotator::ZeroRotator,
				SpawnParams);
			if (Effect)
			{
				if (bVerbose)
				{
					UE_LOG(
						LogTemp,
						Log,
						TEXT("[ATTACK VFX][Stage6] Hero DOT Example actor spawned Req=%d Hero=%s Actor=%s Owner=%s"),
						RequestId,
						*HeroID.ToString(),
						*Effect->GetName(),
						GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
				}
				Effect->InitEffect(FollowTarget, Location, Duration, Radius, Color, RequestId, HeroID);
				return;
			}

			UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage6] Hero DOT Example actor spawn failed Req=%d Hero=%s. Falling back to legacy DOT pixels with VFX=%s."), RequestId, *HeroID.ToString(), *LegacyVFXName);
		}
	}

	if (!VFX)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ATTACK VFX][Stage6] Hero DOT legacy fallback unavailable Req=%d Hero=%s LegacyVFX=%s"), RequestId, *HeroID.ToString(), *LegacyVFXName);
		return;
	}

	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);

	if (HeroID == FName(TEXT("Hero_2")))
	{
		// Merlin: Spiral orb — 12 pixels in a tight spiral
		constexpr int32 N = 12;
		for (int32 i = 0; i < N; ++i)
		{
			const float T = static_cast<float>(i) / static_cast<float>(N);
			const float Angle = T * 4.f * PI;
			const float R = Radius * 0.3f * T;
			const FVector Offset(FMath::Cos(Angle) * R, FMath::Sin(Angle) * R, T * 30.f);
			T66SpawnBudgetedPixel(World, VFX, Location + Offset, ColorVec, FVector2D(2.0f, 2.0f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
		}
	}
	else if (HeroID == FName(TEXT("Hero_13")))
	{
		// Moist: Drip trail — 8 pixels falling vertically
		constexpr int32 N = 8;
		for (int32 i = 0; i < N; ++i)
		{
			const FVector Offset(FMath::FRandRange(-15.f, 15.f), FMath::FRandRange(-15.f, 15.f), -static_cast<float>(i) * 12.f);
			T66SpawnBudgetedPixel(World, VFX, Location + Offset, ColorVec, FVector2D(2.0f, 2.0f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
		}
	}
	else if (HeroID == FName(TEXT("Hero_14")))
	{
		// North: Frost ring — 16 pixels expanding outward
		constexpr int32 N = 16;
		for (int32 i = 0; i < N; ++i)
		{
			const float Angle = (2.f * PI * static_cast<float>(i)) / static_cast<float>(N);
			const FVector Offset(FMath::Cos(Angle) * Radius * 0.4f, FMath::Sin(Angle) * Radius * 0.4f, 0.f);
			T66SpawnBudgetedPixel(World, VFX, Location + Offset, FVector4(0.5f, 0.8f, 1.f, 1.f), FVector2D(2.0f, 2.0f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
		}
	}
	else if (HeroID == FName(TEXT("Hero_16")))
	{
		// Forsen: Stubborn pulse — 6 pixels in a small cluster
		constexpr int32 N = 6;
		for (int32 i = 0; i < N; ++i)
		{
			const FVector Offset(FMath::FRandRange(-20.f, 20.f), FMath::FRandRange(-20.f, 20.f), FMath::FRandRange(-10.f, 10.f));
			T66SpawnBudgetedPixel(World, VFX, Location + Offset, ColorVec, FVector2D(2.5f, 2.5f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
		}
	}
	else
	{
		SpawnDOTVFX(Location, Duration, Radius, Color);
	}
}

void UT66CombatComponent::PerformUltimateSpearStorm(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const FRotator HeroRot = OwnerActor->GetActorRotation();
	const FVector Forward = HeroRot.Vector();
	const float LineLength = 1200.f;
	const int32 NumRays = 10;
	const float TubeRadius = 100.f;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);
	World->OverlapMultiByChannel(Overlaps, HeroLoc, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(LineLength), Params);

	TSet<AActor*> AlreadyHit;
	for (int32 i = 0; i < NumRays; ++i)
	{
		const float AngleDeg = (360.f / NumRays) * i;
		const FVector Dir = Forward.RotateAngleAxis(AngleDeg, FVector::UpVector);
		const FVector Start = HeroLoc;
		const FVector End = HeroLoc + Dir * LineLength;

		for (const FOverlapResult& O : Overlaps)
		{
			AActor* A = O.GetActor();
			if (!A || !IsValidAutoTarget(A) || AlreadyHit.Contains(A)) continue;
			FVector Pt = A->GetActorLocation();
			const float DistSq = FMath::PointDistToSegmentSquared(Pt, Start, End);
			if (DistSq <= TubeRadius * TubeRadius)
			{
				AlreadyHit.Add(A);
				ApplyDamageToActor(A, UltimateDamage, NAME_None, SourceID);
			}
		}
		if (GetActiveVFXSystem())
			SpawnPierceVFX(Start, End, FLinearColor(0.9f, 0.85f, 0.3f, 1.f));
	}
}

void UT66CombatComponent::PerformUltimateChainLightning(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const float ChainRange = 1500.f;
	const float ChainRangeSq = ChainRange * ChainRange;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);
	World->OverlapMultiByChannel(Overlaps, HeroLoc, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(ChainRange), Params);

	TArray<AActor*> ValidTargets;
	for (const FOverlapResult& O : Overlaps)
	{
		AActor* A = O.GetActor();
		if (A && IsValidAutoTarget(A))
			ValidTargets.Add(A);
	}

	TArray<FVector> ChainPositions;
	ChainPositions.Add(HeroLoc);
	FVector CurrentLoc = HeroLoc;
	TSet<AActor*> HitSet;

	while (ValidTargets.Num() > 0)
	{
		AActor* Nearest = nullptr;
		float BestDistSq = ChainRangeSq;
		for (AActor* A : ValidTargets)
		{
			if (HitSet.Contains(A)) continue;
			const float DistSq = FVector::DistSquared(CurrentLoc, A->GetActorLocation());
			if (DistSq < BestDistSq) { BestDistSq = DistSq; Nearest = A; }
		}
		if (!Nearest) break;
		HitSet.Add(Nearest);
		ValidTargets.Remove(Nearest);
		ApplyDamageToActor(Nearest, UltimateDamage, NAME_None, SourceID);
		CurrentLoc = Nearest->GetActorLocation();
		ChainPositions.Add(CurrentLoc);
	}

	if (ChainPositions.Num() >= 2 && GetActiveVFXSystem())
		SpawnBounceVFX(ChainPositions, FLinearColor(1.f, 0.85f, 0.2f, 1.f));
}

void UT66CombatComponent::PerformUltimatePrecisionStrike(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const FVector Forward = OwnerActor->GetActorRotation().Vector();
	const float LineLength = AttackRange;
	const float TubeRadius = 100.f;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;
	const FVector End = HeroLoc + Forward * LineLength;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(Overlaps, HeroLoc + Forward * (LineLength * 0.5f), FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(LineLength * 0.5f), Params);

	for (const FOverlapResult& O : Overlaps)
	{
		AActor* A = O.GetActor();
		if (!A || !IsValidAutoTarget(A)) continue;
		const float DistSq = FMath::PointDistToSegmentSquared(A->GetActorLocation(), HeroLoc, End);
		if (DistSq <= TubeRadius * TubeRadius)
			ApplyDamageToActor(A, UltimateDamage * 5, NAME_None, SourceID);
	}
	SpawnPierceVFX(HeroLoc, End, FLinearColor(0.95f, 0.95f, 1.f));
}

void UT66CombatComponent::PerformUltimateFanTheHammer(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const FVector Forward = OwnerActor->GetActorRotation().Vector();
	const float LineLength = AttackRange;
	const float TubeRadius = 80.f;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;
	constexpr int32 NumShots = 6;
	constexpr float ConeAngleDeg = 60.f;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);
	World->OverlapMultiByChannel(Overlaps, HeroLoc, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(LineLength), Params);

	TSet<AActor*> AlreadyHit;
	for (int32 i = 0; i < NumShots; ++i)
	{
		const float AngleOff = -ConeAngleDeg * 0.5f + ConeAngleDeg * (static_cast<float>(i) / static_cast<float>(NumShots - 1));
		const FVector Dir = Forward.RotateAngleAxis(AngleOff, FVector::UpVector);
		const FVector End = HeroLoc + Dir * LineLength;

		for (const FOverlapResult& O : Overlaps)
		{
			AActor* A = O.GetActor();
			if (!A || !IsValidAutoTarget(A)) continue;
			const float DistSq = FMath::PointDistToSegmentSquared(A->GetActorLocation(), HeroLoc, End);
			if (DistSq <= TubeRadius * TubeRadius && !AlreadyHit.Contains(A))
			{
				AlreadyHit.Add(A);
				ApplyDamageToActor(A, UltimateDamage, NAME_None, SourceID);
			}
		}
		SpawnPierceVFX(HeroLoc, End, FLinearColor(0.9f, 0.7f, 0.2f));
	}
}

void UT66CombatComponent::PerformUltimateDeadeye(int32 UltimateDamage)
{
	UWorld* World = GetWorld();
	if (!World) return;
	DeadeyeEndTime = static_cast<float>(World->GetTimeSeconds()) + 4.f;
}

void UT66CombatComponent::PerformUltimateDischarge(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const float Range = AttackRange;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);
	World->OverlapMultiByChannel(Overlaps, HeroLoc, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(Range), Params);

	TArray<FVector> Positions;
	Positions.Add(HeroLoc);
	for (const FOverlapResult& O : Overlaps)
	{
		AActor* A = O.GetActor();
		if (A && IsValidAutoTarget(A))
		{
			ApplyDamageToActor(A, UltimateDamage, NAME_None, SourceID);
			Positions.Add(A->GetActorLocation());
		}
	}
	if (Positions.Num() >= 2 && GetActiveVFXSystem())
		SpawnBounceVFX(Positions, FLinearColor(0.5f, 0.8f, 1.f));
}

void UT66CombatComponent::PerformUltimateJuiced()
{
	UWorld* World = GetWorld();
	if (!World) return;
	JuicedEndTime = static_cast<float>(World->GetTimeSeconds()) + 5.f;
	JuicedBonusBounce = 5;
}

void UT66CombatComponent::PerformUltimateDeathSpiral(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const FVector Forward = OwnerActor->GetActorRotation().Vector();
	const float LineLength = 1000.f;
	const float TubeRadius = 80.f;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;
	constexpr int32 NumRays = 12;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);
	World->OverlapMultiByChannel(Overlaps, HeroLoc, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(LineLength), Params);

	TSet<AActor*> AlreadyHit;
	for (int32 i = 0; i < NumRays; ++i)
	{
		const float AngleDeg = (360.f / NumRays) * static_cast<float>(i);
		const FVector Dir = Forward.RotateAngleAxis(AngleDeg, FVector::UpVector);
		const FVector End = HeroLoc + Dir * LineLength;

		for (const FOverlapResult& O : Overlaps)
		{
			AActor* A = O.GetActor();
			if (!A || !IsValidAutoTarget(A) || AlreadyHit.Contains(A)) continue;
			const float DistSq = FMath::PointDistToSegmentSquared(A->GetActorLocation(), HeroLoc, End);
			if (DistSq <= TubeRadius * TubeRadius)
			{
				AlreadyHit.Add(A);
				ApplyDamageToActor(A, UltimateDamage, NAME_None, SourceID);
			}
		}
		SpawnBounceVFX({HeroLoc, End}, FLinearColor(0.4f, 0.1f, 0.6f));
	}
}

void UT66CombatComponent::PerformUltimateShockwave(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const float Radius = AttackRange * 2.f;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);
	World->OverlapMultiByChannel(Overlaps, HeroLoc, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(Radius), Params);

	for (const FOverlapResult& O : Overlaps)
	{
		AActor* A = O.GetActor();
		if (A && IsValidAutoTarget(A))
			ApplyDamageToActor(A, UltimateDamage, NAME_None, SourceID);
	}
	SpawnSlashVFX(HeroLoc, Radius, FLinearColor(0.9f, 0.5f, 0.1f));
}

void UT66CombatComponent::PerformUltimateTidalWave(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const FVector Forward = OwnerActor->GetActorRotation().Vector();
	const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
	const float LineLength = AttackRange;
	const float TubeRadius = 80.f;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;
	constexpr int32 NumLines = 5;
	const float Spacing = 120.f;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);
	World->OverlapMultiByChannel(Overlaps, HeroLoc + Forward * (LineLength * 0.5f), FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(LineLength), Params);

	TSet<AActor*> AlreadyHit;
	for (int32 i = 0; i < NumLines; ++i)
	{
		const float Offset = (static_cast<float>(i) - static_cast<float>(NumLines - 1) * 0.5f) * Spacing;
		const FVector Start = HeroLoc + Right * Offset;
		const FVector End = Start + Forward * LineLength;

		for (const FOverlapResult& O : Overlaps)
		{
			AActor* A = O.GetActor();
			if (!A || !IsValidAutoTarget(A) || AlreadyHit.Contains(A)) continue;
			const float DistSq = FMath::PointDistToSegmentSquared(A->GetActorLocation(), Start, End);
			if (DistSq <= TubeRadius * TubeRadius)
			{
				AlreadyHit.Add(A);
				ApplyDamageToActor(A, UltimateDamage, NAME_None, SourceID);
			}
		}
		SpawnPierceVFX(Start, End, FLinearColor(0.3f, 0.6f, 0.9f));
	}
}

void UT66CombatComponent::PerformUltimateGoldRush(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const float Range = AttackRange;
	const float ExplosionRadius = 250.f;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);
	World->OverlapMultiByChannel(Overlaps, HeroLoc, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(Range), Params);

	TArray<AActor*> Targets;
	for (const FOverlapResult& O : Overlaps)
	{
		AActor* A = O.GetActor();
		if (A && IsValidAutoTarget(A))
			Targets.Add(A);
	}

	const int32 MaxExplosions = FMath::Min(8, Targets.Num());
	TSet<AActor*> AlreadyHit;
	for (int32 i = 0; i < MaxExplosions; ++i)
	{
		const FVector Center = Targets[i]->GetActorLocation();
		if (!AlreadyHit.Contains(Targets[i]))
		{
			AlreadyHit.Add(Targets[i]);
			ApplyDamageToActor(Targets[i], UltimateDamage, NAME_None, SourceID);
		}
		for (const FOverlapResult& O : Overlaps)
		{
			AActor* A = O.GetActor();
			if (A && IsValidAutoTarget(A) && !AlreadyHit.Contains(A))
			{
				if (FVector::DistSquared(A->GetActorLocation(), Center) <= ExplosionRadius * ExplosionRadius)
				{
					AlreadyHit.Add(A);
					ApplyDamageToActor(A, UltimateDamage, NAME_None, SourceID);
				}
			}
		}
		SpawnSlashVFX(Center, ExplosionRadius, FLinearColor(0.95f, 0.85f, 0.1f));
	}
}

void UT66CombatComponent::PerformUltimateMiasmaBomb(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World || !CachedRunState) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const float Range = AttackRange;

	AActor* NearestTarget = FindClosestEnemyInRange(HeroLoc, Range * Range);
	const FVector Center = NearestTarget ? NearestTarget->GetActorLocation() : (HeroLoc + OwnerActor->GetActorRotation().Vector() * 500.f);
	const float Radius = 600.f;
	const float Duration = 5.f;
	const float TickInterval = 0.5f;
	const int32 Ticks = FMath::Max(1, FMath::RoundToInt(Duration / TickInterval));
	const float DmgPerTick = static_cast<float>(UltimateDamage) / static_cast<float>(Ticks);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(Overlaps, Center, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(Radius), Params);

	for (const FOverlapResult& O : Overlaps)
	{
		AActor* A = O.GetActor();
		if (A && IsValidAutoTarget(A))
			CachedRunState->ApplyDOT(A, Duration, TickInterval, DmgPerTick, NAME_None);
	}
	SpawnDOTVFX(Center, Duration, Radius, FLinearColor(0.5f, 0.9f, 0.2f));
}

void UT66CombatComponent::PerformUltimateRabidFrenzy()
{
	UWorld* World = GetWorld();
	if (!World) return;
	RabidFrenzyEndTime = static_cast<float>(World->GetTimeSeconds()) + 4.f;
}

void UT66CombatComponent::PerformUltimateBlizzard(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World || !CachedRunState) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const float Range = AttackRange;
	const float Duration = 6.f;
	const float TickInterval = 0.5f;
	const int32 Ticks = FMath::Max(1, FMath::RoundToInt(Duration / TickInterval));
	const float DmgPerTick = static_cast<float>(UltimateDamage) / static_cast<float>(Ticks);
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(Overlaps, HeroLoc, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(Range), Params);

	for (const FOverlapResult& O : Overlaps)
	{
		AActor* A = O.GetActor();
		if (A && IsValidAutoTarget(A))
		{
			CachedRunState->ApplyDOT(A, Duration, TickInterval, DmgPerTick, NAME_None);
			if (AT66EnemyBase* E = Cast<AT66EnemyBase>(A))
				E->ApplyMoveSlow(0.6f, Duration);
		}
	}
	SpawnSlashVFX(HeroLoc, Range * 0.5f, FLinearColor(0.5f, 0.8f, 1.f));
}
