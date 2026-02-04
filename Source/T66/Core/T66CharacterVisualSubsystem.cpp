// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "UObject/SoftObjectPath.h"

static const TCHAR* T66_DefaultCharacterVisualsDTPath = TEXT("/Game/Data/DT_CharacterVisuals.DT_CharacterVisuals");
static const FName T66_AnimSkeletonTag(TEXT("Skeleton"));
static const FName T66_CharactersRootPath(TEXT("/Game/Characters"));

/** If the given path points to a non-animation (e.g. SkeletalMesh), try the same path with _Anim suffix (e.g. AM_X.AM_X -> AM_X_Anim.AM_X_Anim). */
static UAnimationAsset* LoadAnimationFallbackWithAnimSuffix(const TSoftObjectPtr<UAnimationAsset>& SoftPath)
{
	FString PathStr = SoftPath.ToString();
	if (PathStr.IsEmpty() || PathStr.Contains(TEXT("_Anim."))) return nullptr;
	int32 DotIdx;
	if (!PathStr.FindLastChar(TEXT('.'), DotIdx)) return nullptr;
	FString Base = PathStr.Left(DotIdx);
	FString ObjName = PathStr.Mid(DotIdx + 1);
	FString NewPath = Base + TEXT("_Anim.") + ObjName + TEXT("_Anim");
	return Cast<UAnimationAsset>(TSoftObjectPtr<UAnimationAsset>(FSoftObjectPath(NewPath)).LoadSynchronous());
}

/** If path is Package_Anim.Object_Anim and package doesn't exist, try Package.Object_Anim (FBX import creates package without _Anim). */
static UAnimationAsset* LoadAnimationFallbackStripPackageAnimSuffix(const TSoftObjectPtr<UAnimationAsset>& SoftPath)
{
	FString PathStr = SoftPath.ToString();
	int32 DotIdx;
	if (PathStr.IsEmpty() || !PathStr.FindLastChar(TEXT('.'), DotIdx)) return nullptr;
	FString Base = PathStr.Left(DotIdx);
	FString ObjName = PathStr.Mid(DotIdx + 1);
	if (!Base.EndsWith(TEXT("_Anim"))) return nullptr;
	FString BaseStrip = Base.LeftChop(4); // strip "_Anim"
	FString NewPath = BaseStrip + TEXT(".") + ObjName;
	return Cast<UAnimationAsset>(TSoftObjectPtr<UAnimationAsset>(FSoftObjectPath(NewPath)).LoadSynchronous());
}

UAnimationAsset* UT66CharacterVisualSubsystem::FindFallbackLoopingAnim(USkeleton* Skeleton) const
{
	if (!Skeleton)
	{
		return nullptr;
	}

	const FName SkelKey(*Skeleton->GetPathName());
	if (const TObjectPtr<UAnimationAsset>* Cached = SkeletonAnimCache.Find(SkelKey))
	{
		return Cached ? Cached->Get() : nullptr;
	}

	UAnimationAsset* Chosen = nullptr;
	FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& Registry = ARM.Get();

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Add(T66_CharactersRootPath);
	Filter.ClassPaths.Add(UAnimSequence::StaticClass()->GetClassPathName());

	TArray<FAssetData> Assets;
	Registry.GetAssets(Filter, Assets);

	const FString SkeletonPath = Skeleton->GetPathName();

	// Prefer an Idle if present, otherwise Walk/Run, otherwise first match.
	int32 BestScore = -1;
	FAssetData BestAsset;

	for (const FAssetData& A : Assets)
	{
		FString Tag;
		if (!A.GetTagValue(T66_AnimSkeletonTag, Tag))
		{
			continue;
		}
		// Tag formats can vary; be permissive.
		if (!Tag.Contains(SkeletonPath))
		{
			continue;
		}

		const FString Name = A.AssetName.ToString().ToLower();
		int32 Score = 0;
		if (Name.Contains("idle")) Score += 100;
		if (Name.Contains("walk")) Score += 50;
		if (Name.Contains("run")) Score += 40;
		if (Name.Contains("loop")) Score += 10;

		if (Score > BestScore)
		{
			BestScore = Score;
			BestAsset = A;
		}
	}

	if (BestScore >= 0)
	{
		UObject* Obj = BestAsset.GetAsset(); // loads
		Chosen = Cast<UAnimationAsset>(Obj);
	}

	SkeletonAnimCache.Add(SkelKey, Chosen);
	return Chosen;
}

UDataTable* UT66CharacterVisualSubsystem::GetVisualsDataTable() const
{
	if (CachedVisualsDataTable)
	{
		return CachedVisualsDataTable;
	}

	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
		{
			// Optional: wire this in BP_T66GameInstance later; keep a safe hardcoded fallback for now.
			if (!T66GI->CharacterVisualsDataTable.IsNull())
			{
				CachedVisualsDataTable = T66GI->CharacterVisualsDataTable.LoadSynchronous();
				return CachedVisualsDataTable;
			}
		}
	}

	// Fallback: load by canonical path if present.
	CachedVisualsDataTable = LoadObject<UDataTable>(nullptr, T66_DefaultCharacterVisualsDTPath);
	return CachedVisualsDataTable;
}

FT66ResolvedCharacterVisual UT66CharacterVisualSubsystem::ResolveVisual(FName VisualID)
{
	if (VisualID.IsNone())
	{
		return FT66ResolvedCharacterVisual();
	}

	if (const FT66ResolvedCharacterVisual* Existing = ResolvedCache.Find(VisualID))
	{
		return *Existing;
	}

	FT66ResolvedCharacterVisual Res;
	Res.bHasRow = false;

	UDataTable* DT = GetVisualsDataTable();
	if (!DT)
	{
		ResolvedCache.Add(VisualID, Res);
		return Res;
	}

	if (FT66CharacterVisualRow* Row = DT->FindRow<FT66CharacterVisualRow>(VisualID, TEXT("ResolveVisual")))
	{
		Res.Row = *Row;
		Res.bHasRow = true;

		if (!Res.Row.SkeletalMesh.IsNull())
		{
			Res.Mesh = Res.Row.SkeletalMesh.LoadSynchronous();
			UE_LOG(LogTemp, Log, TEXT("[MESH] ResolveVisual VisualID=%s SkeletalMesh path=%s Loaded=%s"),
				*VisualID.ToString(), *Res.Row.SkeletalMesh.ToString(), Res.Mesh ? TEXT("YES") : TEXT("NO"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[MESH] ResolveVisual VisualID=%s SkeletalMesh path is NULL in DataTable row!"), *VisualID.ToString());
		}
		if (!Res.Row.LoopingAnimation.IsNull())
		{
			Res.LoopingAnim = Cast<UAnimationAsset>(Res.Row.LoopingAnimation.LoadSynchronous());
			if (!Res.LoopingAnim)
				Res.LoopingAnim = LoadAnimationFallbackWithAnimSuffix(Res.Row.LoopingAnimation);
			if (!Res.LoopingAnim)
				Res.LoopingAnim = LoadAnimationFallbackStripPackageAnimSuffix(Res.Row.LoopingAnimation);
		}
		if (!Res.Row.AlertAnimation.IsNull())
		{
			Res.AlertAnim = Cast<UAnimationAsset>(Res.Row.AlertAnimation.LoadSynchronous());
			if (!Res.AlertAnim)
				Res.AlertAnim = LoadAnimationFallbackWithAnimSuffix(Res.Row.AlertAnimation);
			if (!Res.AlertAnim)
				Res.AlertAnim = LoadAnimationFallbackStripPackageAnimSuffix(Res.Row.AlertAnimation);
			UE_LOG(LogTemp, Log, TEXT("[ANIM] ResolveVisual VisualID=%s AlertAnimation path=%s AlertAnim=%s"),
				*VisualID.ToString(), *Res.Row.AlertAnimation.ToString(), Res.AlertAnim ? *Res.AlertAnim->GetName() : TEXT("(null)"));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[ANIM] ResolveVisual VisualID=%s AlertAnimation is null (no alert anim row)"), *VisualID.ToString());
		}
		// If no explicit animation is set, try to find any AnimSequence for this Skeleton (cached).
		if (!Res.LoopingAnim && Res.Mesh && Res.Mesh->GetSkeleton())
		{
			Res.LoopingAnim = FindFallbackLoopingAnim(Res.Mesh->GetSkeleton());
		}
	}

	ResolvedCache.Add(VisualID, Res);
	return Res;
}

void UT66CharacterVisualSubsystem::PreloadCharacterVisual(FName VisualID)
{
	ResolveVisual(VisualID);
}

FName UT66CharacterVisualSubsystem::GetHeroVisualID(FName HeroID, ET66BodyType BodyType, FName SkinID)
{
	if (HeroID.IsNone()) return NAME_None;
	const TCHAR TypeChar = (BodyType == ET66BodyType::TypeA) ? TEXT('A') : TEXT('B');
	FString Key = FString::Printf(TEXT("%s_Type%c"), *HeroID.ToString(), TypeChar);
	if (SkinID != NAME_None && SkinID != FName(TEXT("Default")))
	{
		Key += TEXT("_");
		Key += SkinID.ToString();
	}
	return FName(*Key);
}

FName UT66CharacterVisualSubsystem::GetCompanionVisualID(FName CompanionID, FName SkinID)
{
	if (CompanionID.IsNone()) return NAME_None;
	if (SkinID.IsNone() || SkinID == FName(TEXT("Default")))
	{
		return CompanionID;
	}
	return FName(*(CompanionID.ToString() + TEXT("_") + SkinID.ToString()));
}

bool UT66CharacterVisualSubsystem::ApplyCharacterVisual(
	FName VisualID,
	USkeletalMeshComponent* TargetMesh,
	USceneComponent* PlaceholderToHide,
	bool bEnableSingleNodeAnimation,
	bool bUseAlertAnimation,
	bool bIsPreviewContext)
{
	if (VisualID.IsNone() || !TargetMesh)
	{
		return false;
	}

	const FT66ResolvedCharacterVisual Res = ResolveVisual(VisualID);
	if (!Res.bHasRow || !Res.Mesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MESH] ApplyCharacterVisual FAILED for VisualID=%s: bHasRow=%d, Mesh=%s"),
			*VisualID.ToString(), Res.bHasRow ? 1 : 0, Res.Mesh ? *Res.Mesh->GetName() : TEXT("(null)"));
		return false;
	}

	TargetMesh->SetSkeletalMesh(Res.Mesh);
	TargetMesh->SetRelativeRotation(Res.Row.MeshRelativeRotation);

	const FVector Scale = Res.Row.MeshRelativeScale.IsNearlyZero() ? FVector(1.f, 1.f, 1.f) : Res.Row.MeshRelativeScale;
	TargetMesh->SetRelativeScale3D(Scale);

	FVector RelLoc = Res.Row.MeshRelativeLocation;
	if (Res.Row.bAutoGroundToActorOrigin)
	{
		const FBoxSphereBounds B = Res.Mesh->GetBounds();
		// Account for scale (approx; bounds are in mesh space).
		const float BottomZ = (B.Origin.Z - B.BoxExtent.Z) * Scale.Z;
		RelLoc.Z -= BottomZ;
	}

	// Default for ACharacter: keep mesh bottom aligned to capsule bottom (ground contact).
	bool bAppliedAutoCenterFix = false;
	if (ACharacter* OwnerChar = Cast<ACharacter>(TargetMesh->GetOwner()))
	{
		if (UCapsuleComponent* Cap = OwnerChar->GetCapsuleComponent())
		{
			const float CapsuleHalfHeight = Cap->GetScaledCapsuleHalfHeight();
			const FBoxSphereBounds B = Res.Mesh->GetBounds();
			const float BottomZ = (B.Origin.Z - B.BoxExtent.Z) * Scale.Z;
			const float CurrentBottom = RelLoc.Z + BottomZ;
			const float DesiredBottom = -CapsuleHalfHeight;
			RelLoc.Z += (DesiredBottom - CurrentBottom);

			// Some non-biped FBX exports have a huge local-space bounds origin (pivot far from geometry),
			// which effectively moves the mesh far away from the capsule (appearing "invisible" in-game).
			// Fix: auto-center XY by subtracting the rotated bounds origin. This is gated by a small
			// threshold so normal character meshes (with near-zero origin) are unaffected.
			const FVector LocalOriginXY(B.Origin.X * Scale.X, B.Origin.Y * Scale.Y, 0.f);
			const float OriginXYSize = LocalOriginXY.Size();
			if (OriginXYSize > 1.f)
			{
				const FVector RotatedOriginXY = Res.Row.MeshRelativeRotation.RotateVector(LocalOriginXY);
				RelLoc.X -= RotatedOriginXY.X;
				RelLoc.Y -= RotatedOriginXY.Y;
				bAppliedAutoCenterFix = true;

#if !UE_BUILD_SHIPPING
				static int32 LoggedCenterFixes = 0;
				if (LoggedCenterFixes < 12)
				{
					++LoggedCenterFixes;
					UE_LOG(LogTemp, Verbose, TEXT("[INVIS] CharacterVisuals: AutoCenteredXY VisualID=%s OriginXY=(%.1f,%.1f) RotatedXY=(%.1f,%.1f)"),
						*VisualID.ToString(),
						LocalOriginXY.X, LocalOriginXY.Y,
						RotatedOriginXY.X, RotatedOriginXY.Y);
				}
#endif
			}
		}
	}
	TargetMesh->SetRelativeLocation(RelLoc);

	TargetMesh->SetHiddenInGame(false, true);
	TargetMesh->SetVisibility(true, true);

	if (PlaceholderToHide)
	{
		PlaceholderToHide->SetVisibility(false, true);
		PlaceholderToHide->SetHiddenInGame(true, true);
	}

	if (bEnableSingleNodeAnimation)
	{
		UAnimationAsset* AnimToPlay = (bUseAlertAnimation && Res.AlertAnim) ? Res.AlertAnim : Res.LoopingAnim;
		// Log animation type and duration for debugging
		float AnimDuration = 0.f;
		FString AnimClass = TEXT("(null)");
		if (AnimToPlay)
		{
			AnimClass = AnimToPlay->GetClass()->GetName();
			AnimDuration = AnimToPlay->GetPlayLength();
		}
		UE_LOG(LogTemp, Log, TEXT("[ANIM] ApplyCharacterVisual VisualID=%s bUseAlertAnimation=%d bIsPreviewContext=%d Res.AlertAnim=%s Res.LoopingAnim=%s AnimToPlay=%s Class=%s Duration=%.3f"),
			*VisualID.ToString(), bUseAlertAnimation ? 1 : 0, bIsPreviewContext ? 1 : 0,
			Res.AlertAnim ? *Res.AlertAnim->GetName() : TEXT("(null)"),
			Res.LoopingAnim ? *Res.LoopingAnim->GetName() : TEXT("(null)"),
			AnimToPlay ? *AnimToPlay->GetName() : TEXT("(null)"),
			*AnimClass, AnimDuration);
		if (AnimToPlay)
		{
			// Preview (hero selection): ensure no AnimBlueprint overrides, mesh always ticks so animation is visible in scene capture.
			if (bIsPreviewContext)
			{
				TargetMesh->SetAnimInstanceClass(nullptr);
				TargetMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
				TargetMesh->SetComponentTickEnabled(true);
				TargetMesh->bEnableUpdateRateOptimizations = false; // Preview is only visible to SceneCapture; avoid skipping anim updates.
			}
			// Reinitialize animation state after mesh change (otherwise PlayAnimation fails after SetSkeletalMesh).
			TargetMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			TargetMesh->InitAnim(true);
			// In preview we use alert anim and want it to loop; otherwise use row's loop setting.
			const bool bLoop = bUseAlertAnimation ? true : Res.Row.bLoopAnimation;
			TargetMesh->PlayAnimation(AnimToPlay, bLoop);
			TargetMesh->SetPosition(0.f);
			UE_LOG(LogTemp, Log, TEXT("[ANIM] PlayAnimation called: AnimMode=%d IsPlaying=%d Position=%.3f bLoop=%d"),
				(int32)TargetMesh->GetAnimationMode(), TargetMesh->IsPlaying() ? 1 : 0, TargetMesh->GetPosition(), bLoop ? 1 : 0);
		}
	}

	// ============================================================
	// INVIS fail-safe:
	// If an imported mesh winds up far away from the owning actor (bad pivot/bounds),
	// the enemy/boss can become effectively invisible even though gameplay/collision exists.
	// Detect that case and fall back to placeholder visuals, and log actionable diagnostics.
	// ============================================================
	{
		const AActor* Owner = Cast<AActor>(TargetMesh->GetOwner());
		const FVector OwnerLoc = Owner ? Owner->GetActorLocation() : TargetMesh->GetComponentLocation();
		const FBoxSphereBounds WorldB = TargetMesh->CalcBounds(TargetMesh->GetComponentTransform());

		const float DistToOwner = (WorldB.Origin - OwnerLoc).Size();
		const bool bBoundsTiny = (WorldB.SphereRadius < 1.f);
		const bool bBoundsFar = (DistToOwner > 5000.f); // 50m+ away from capsule = effectively invisible in normal gameplay.
		const bool bBoundsFinite =
			FMath::IsFinite(WorldB.Origin.X) && FMath::IsFinite(WorldB.Origin.Y) && FMath::IsFinite(WorldB.Origin.Z) &&
			FMath::IsFinite(WorldB.BoxExtent.X) && FMath::IsFinite(WorldB.BoxExtent.Y) && FMath::IsFinite(WorldB.BoxExtent.Z) &&
			FMath::IsFinite(WorldB.SphereRadius);

		if (bBoundsTiny || bBoundsFar || !bBoundsFinite)
		{
#if !UE_BUILD_SHIPPING
			static int32 LoggedInvisFixes = 0;
			if (LoggedInvisFixes < 32)
			{
				++LoggedInvisFixes;
				const FString MeshName = Res.Mesh ? Res.Mesh->GetName() : FString(TEXT("None"));
				const FString MeshPath = Res.Mesh ? Res.Mesh->GetPathName() : FString(TEXT("None"));
				const FString OwnerName = Owner ? Owner->GetName() : FString(TEXT("None"));

				UE_LOG(LogTemp, Error,
					TEXT("[INVIS] Rejecting visual apply (fallback to placeholder). VisualID=%s Owner=%s Mesh=%s MeshPath=%s DistToOwner=%.1f BoundsCenter=(%.1f,%.1f,%.1f) BoundsExtent=(%.1f,%.1f,%.1f) SphereRadius=%.1f RelLoc=(%.1f,%.1f,%.1f) RelRot=(P=%.1f,Y=%.1f,R=%.1f) RelScale=(%.3f,%.3f,%.3f) AutoCenterXY=%d"),
					*VisualID.ToString(),
					*OwnerName,
					*MeshName,
					*MeshPath,
					DistToOwner,
					WorldB.Origin.X, WorldB.Origin.Y, WorldB.Origin.Z,
					WorldB.BoxExtent.X, WorldB.BoxExtent.Y, WorldB.BoxExtent.Z,
					WorldB.SphereRadius,
					RelLoc.X, RelLoc.Y, RelLoc.Z,
					Res.Row.MeshRelativeRotation.Pitch, Res.Row.MeshRelativeRotation.Yaw, Res.Row.MeshRelativeRotation.Roll,
					Scale.X, Scale.Y, Scale.Z,
					bAppliedAutoCenterFix ? 1 : 0
				);
			}
#endif

			// Revert: hide skeletal mesh, show placeholder.
			TargetMesh->SetHiddenInGame(true, true);
			TargetMesh->SetVisibility(false, true);
			if (PlaceholderToHide)
			{
				PlaceholderToHide->SetHiddenInGame(false, true);
				PlaceholderToHide->SetVisibility(true, true);
			}
			return false;
		}
	}

	return true;
}

