// Copyright Tribulation 66. All Rights Reserved.

#include "T66MotionRigPhysicsAssetCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Misc/PackageName.h"
#include "PhysicsAssetUtils.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintTemplate.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66MotionRigPA, Log, All);

namespace
{
	// Both Hero 1 body types (Chad = male, Stacy = female) get a physics
	// asset, generated from the same authored params.
	struct FT66MotionRigPACharacter
	{
		const TCHAR* MeshPath;
		const TCHAR* PhysicsAssetPackage;
	};
	const FT66MotionRigPACharacter MotionRigPACharacters[] =
	{
		{ TEXT("/Game/Characters/MotionRig/Hero_1_Male/SK_MotionRig_Hero1Male.SK_MotionRig_Hero1Male"),
		  TEXT("/Game/Characters/MotionRig/Hero_1_Male/PA_MotionRig_Hero1Male") },
		{ TEXT("/Game/Characters/MotionRig/Hero_1_Female/SK_MotionRig_Hero1Female.SK_MotionRig_Hero1Female"),
		  TEXT("/Game/Characters/MotionRig/Hero_1_Female/PA_MotionRig_Hero1Female") },
	};

	bool SaveAssetPackage(UObject* Asset)
	{
		UPackage* Package = Asset ? Asset->GetPackage() : nullptr;
		if (!Package)
		{
			return false;
		}
		Package->MarkPackageDirty();
		const FString Filename = FPackageName::LongPackageNameToFilename(
			Package->GetName(), FPackageName::GetAssetPackageExtension());
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		return UPackage::SavePackage(Package, Asset, *Filename, SaveArgs);
	}
}

int32 UT66MotionRigPhysicsAssetCommandlet::Main(const FString& Params)
{
	int32 Result = 0;
	for (const FT66MotionRigPACharacter& Character : MotionRigPACharacters)
	{
		UE_LOG(LogT66MotionRigPA, Display, TEXT("=== MotionRig PA: %s ==="), Character.MeshPath);
		Result |= BuildPhysicsAssetForCharacter(Character.MeshPath, Character.PhysicsAssetPackage);
	}
	return Result;
}

int32 UT66MotionRigPhysicsAssetCommandlet::BuildPhysicsAssetForCharacter(
	const TCHAR* MotionRigMeshPath, const TCHAR* MotionRigPhysicsAssetPackage)
{
	USkeletalMesh* SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, MotionRigMeshPath);
	if (!SkeletalMesh)
	{
		UE_LOG(LogT66MotionRigPA, Error, TEXT("MotionRig skeletal mesh not found: %s"), MotionRigMeshPath);
		return 1;
	}

	// Unit guard. The import itself must deliver a centimeter reference pose:
	// the skeletal FBX bakes a 1-frame cm bind-pose animation and
	// ImportMotionRig.py imports with use_t0_as_ref_pose=True, so reference
	// AND render bind are built from cm data at import time. Post-import ref
	// surgery is banned here — the LOD render data carries bind-dependent
	// caches from import that CalculateInvRefMatrices does not refresh, so a
	// rescaled ref pose still renders crumpled skin.
	{
		const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();
		const TArray<FTransform>& RefPose = RefSkeleton.GetRefBonePose();

		// Component-space pose: the unit truth. A local-only read of pelvis
		// can show 0.98 both for a collapsed meter skeleton AND for a correct
		// cm skeleton whose root bone carries the Blender armature-node
		// scale of 100 — only the accumulated transform separates the two.
		TArray<FTransform> ComponentPose;
		ComponentPose.SetNum(RefSkeleton.GetRawBoneNum());
		for (int32 BoneIndex = 0; BoneIndex < RefSkeleton.GetRawBoneNum(); ++BoneIndex)
		{
			const int32 ParentIndex = RefSkeleton.GetParentIndex(BoneIndex);
			ComponentPose[BoneIndex] = ParentIndex != INDEX_NONE
				? RefPose[BoneIndex] * ComponentPose[ParentIndex]
				: RefPose[BoneIndex];
		}

		const int32 PelvisIndex = RefSkeleton.FindBoneIndex(TEXT("pelvis"));
		const int32 CalfIndex = RefSkeleton.FindBoneIndex(TEXT("calf_l"));
		const int32 HeadIndex = RefSkeleton.FindBoneIndex(TEXT("head"));
		const float PelvisLocalZ = PelvisIndex != INDEX_NONE ? RefPose[PelvisIndex].GetTranslation().Z : -1.f;
		const float PelvisCompZ = PelvisIndex != INDEX_NONE ? ComponentPose[PelvisIndex].GetTranslation().Z : -1.f;
		const float HeadCompZ = HeadIndex != INDEX_NONE ? ComponentPose[HeadIndex].GetTranslation().Z : -1.f;
		const float CalfLocal = CalfIndex != INDEX_NONE ? RefPose[CalfIndex].GetTranslation().Size() : -1.f;
		const FVector RootScale = RefPose.Num() > 0 ? RefPose[0].GetScale3D() : FVector::ZeroVector;
		UE_LOG(LogT66MotionRigPA, Display,
			TEXT("MOTIONRIG_PA_REFPOSE pelvisLocalZ=%.2f pelvisCompZ=%.2f headCompZ=%.2f calfLocal=%.2f rootScale=%s"),
			PelvisLocalZ, PelvisCompZ, HeadCompZ, CalfLocal, *RootScale.ToCompactString());
		if (PelvisCompZ < 50.0f)
		{
			UE_LOG(LogT66MotionRigPA, Error,
				TEXT("Reference pose is collapsed (component-space pelvisZ=%.2f, expected ~98.1). ")
				TEXT("Reimport via ImportMotionRig.py (use_t0_as_ref_pose + baked bind pose); ")
				TEXT("refusing to generate a physics asset from a collapsed skeleton."), PelvisCompZ);
			return 1;
		}

		// Keep the USkeleton asset's copy of the reference pose in lockstep
		// with the mesh — a mismatch detonates the physics->bone blend.
		if (USkeleton* Skeleton = SkeletalMesh->GetSkeleton())
		{
			Skeleton->UpdateReferencePoseFromMesh(SkeletalMesh);
			SaveAssetPackage(Skeleton);
		}
	}

	UPackage* Package = CreatePackage(MotionRigPhysicsAssetPackage);
	if (!Package)
	{
		UE_LOG(LogT66MotionRigPA, Error, TEXT("Could not create package %s"), MotionRigPhysicsAssetPackage);
		return 1;
	}
	Package->FullyLoad();

	const FString AssetName = FPackageName::GetLongPackageAssetName(MotionRigPhysicsAssetPackage);
	UPhysicsAsset* PhysicsAsset = FindObject<UPhysicsAsset>(Package, *AssetName);
	if (!PhysicsAsset)
	{
		PhysicsAsset = NewObject<UPhysicsAsset>(Package, FName(*AssetName), RF_Public | RF_Standalone);
		FAssetRegistryModule::AssetCreated(PhysicsAsset);
	}
	else
	{
		// Regenerate from scratch on re-runs.
		PhysicsAsset->SkeletalBodySetups.Empty();
		PhysicsAsset->ConstraintSetup.Empty();
		PhysicsAsset->UpdateBodySetupIndexMap();
		PhysicsAsset->UpdateBoundsBodiesArray();
	}

	FPhysAssetCreateParams Params2;
	Params2.MinBoneSize = 2.f;          // every MotionRig bone gets a body, hands and feet included
	Params2.GeomType = EFG_Sphyl;       // capsules: the readable chunky silhouette
	Params2.VertWeight = EVW_DominantWeight;
	Params2.bCreateConstraints = true;
	Params2.bBodyForAll = true;         // bodies even for bones with thin vertex coverage
	Params2.bDisableCollisionsByDefault = true; // adjacent-body collisions off; world collisions live on the body instances

	FText ErrorMessage;
	const bool bCreated = FPhysicsAssetUtils::CreateFromSkeletalMesh(
		PhysicsAsset, SkeletalMesh, Params2, ErrorMessage, /*bSetToMesh*/ true, /*bShowProgress*/ false);
	if (!bCreated)
	{
		UE_LOG(LogT66MotionRigPA, Error, TEXT("CreateFromSkeletalMesh failed: %s"), *ErrorMessage.ToString());
		return 1;
	}

	// Cull bodies for non-canonical bones (the FBX armature root sneaks in as
	// a body at the origin and would collide with the floor between the feet).
	{
		static const TSet<FName> CanonicalBones = {
			TEXT("pelvis"), TEXT("spine_01"), TEXT("spine_02"), TEXT("head"),
			TEXT("clavicle_l"), TEXT("upperarm_l"), TEXT("lowerarm_l"), TEXT("hand_l"),
			TEXT("clavicle_r"), TEXT("upperarm_r"), TEXT("lowerarm_r"), TEXT("hand_r"),
			TEXT("thigh_l"), TEXT("calf_l"), TEXT("foot_l"),
			TEXT("thigh_r"), TEXT("calf_r"), TEXT("foot_r") };

		for (int32 BodyIndex = PhysicsAsset->SkeletalBodySetups.Num() - 1; BodyIndex >= 0; --BodyIndex)
		{
			const USkeletalBodySetup* Setup = PhysicsAsset->SkeletalBodySetups[BodyIndex];
			if (Setup && !CanonicalBones.Contains(Setup->BoneName))
			{
				UE_LOG(LogT66MotionRigPA, Display, TEXT("Culling non-canonical body: %s"), *Setup->BoneName.ToString());
				for (int32 ConstraintIndex = PhysicsAsset->ConstraintSetup.Num() - 1; ConstraintIndex >= 0; --ConstraintIndex)
				{
					const UPhysicsConstraintTemplate* Constraint = PhysicsAsset->ConstraintSetup[ConstraintIndex];
					if (Constraint
						&& (Constraint->DefaultInstance.ConstraintBone1 == Setup->BoneName
							|| Constraint->DefaultInstance.ConstraintBone2 == Setup->BoneName))
					{
						FPhysicsAssetUtils::DestroyConstraint(PhysicsAsset, ConstraintIndex);
					}
				}
				FPhysicsAssetUtils::DestroyBody(PhysicsAsset, BodyIndex);
			}
		}
		PhysicsAsset->UpdateBodySetupIndexMap();
		PhysicsAsset->UpdateBoundsBodiesArray();
	}

	// Free every angular limit: the generated cone limits (~swing 30-45 deg)
	// clamp the walk clip's thigh swing, so the legs could never articulate —
	// feet dragged at bean speed (foot-slide ratio ~1.0 across captures).
	// MotionRig stability comes from the SLERP drives, not limits; floppy
	// over-rotation on impacts is part of the look.
	for (UPhysicsConstraintTemplate* Constraint : PhysicsAsset->ConstraintSetup)
	{
		if (Constraint)
		{
			Constraint->DefaultInstance.SetAngularSwing1Limit(ACM_Free, 0.f);
			Constraint->DefaultInstance.SetAngularSwing2Limit(ACM_Free, 0.f);
			Constraint->DefaultInstance.SetAngularTwistLimit(ACM_Free, 0.f);
		}
	}

	// Floating-feet fix: the auto-generated foot capsules around the chunky
	// boots reach BELOW the sole plane, so when those capsules rest on the
	// floor the whole skeleton rides high and the boot meshes hover ~2-3cm.
	// Clamp each foot capsule so its lowest point at the rest pose sits
	// exactly on the sole plane (component z = 0).
	{
		const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();
		TArray<FTransform> ComponentPose;
		ComponentPose.SetNum(RefSkeleton.GetRawBoneNum());
		for (int32 BoneIndex = 0; BoneIndex < RefSkeleton.GetRawBoneNum(); ++BoneIndex)
		{
			const int32 ParentIndex = RefSkeleton.GetParentIndex(BoneIndex);
			ComponentPose[BoneIndex] = ParentIndex != INDEX_NONE
				? RefSkeleton.GetRefBonePose()[BoneIndex] * ComponentPose[ParentIndex]
				: RefSkeleton.GetRefBonePose()[BoneIndex];
		}

		for (const FName FootBone : { FName(TEXT("foot_l")), FName(TEXT("foot_r")) })
		{
			const int32 BodyIndex = PhysicsAsset->FindBodyIndex(FootBone);
			const int32 BoneIndex = RefSkeleton.FindBoneIndex(FootBone);
			if (BodyIndex == INDEX_NONE || BoneIndex == INDEX_NONE)
			{
				continue;
			}
			USkeletalBodySetup* Setup = PhysicsAsset->SkeletalBodySetups[BodyIndex];
			const FTransform& BoneComp = ComponentPose[BoneIndex];
			for (FKSphylElem& Sphyl : Setup->AggGeom.SphylElems)
			{
				const FTransform ElemComp = Sphyl.GetTransform() * BoneComp;
				const FVector Axis = ElemComp.GetUnitAxis(EAxis::Z);
				const float Lowest = ElemComp.GetLocation().Z
					- FMath::Abs(Axis.Z) * Sphyl.Length * 0.5f - Sphyl.Radius;
				if (Lowest < 0.f)
				{
					const FVector DeltaBone = BoneComp.InverseTransformVector(FVector(0.f, 0.f, -Lowest));
					Sphyl.Center += DeltaBone;
					UE_LOG(LogT66MotionRigPA, Display,
						TEXT("  %s capsule raised %.1fcm to the sole plane"),
						*FootBone.ToString(), -Lowest);
				}
			}
			Setup->InvalidatePhysicsData();
			Setup->CreatePhysicsMeshes();
		}
	}

	UE_LOG(LogT66MotionRigPA, Display, TEXT("MotionRig physics asset: %d bodies, %d constraints"),
		PhysicsAsset->SkeletalBodySetups.Num(), PhysicsAsset->ConstraintSetup.Num());
	for (USkeletalBodySetup* Setup : PhysicsAsset->SkeletalBodySetups)
	{
		if (Setup)
		{
			UE_LOG(LogT66MotionRigPA, Display, TEXT("  body: %s (spheres=%d sphyls=%d boxes=%d)"),
				*Setup->BoneName.ToString(),
				Setup->AggGeom.SphereElems.Num(),
				Setup->AggGeom.SphylElems.Num(),
				Setup->AggGeom.BoxElems.Num());
		}
	}

	const bool bSavedPA = SaveAssetPackage(PhysicsAsset);
	const bool bSavedMesh = SaveAssetPackage(SkeletalMesh);
	UE_LOG(LogT66MotionRigPA, Display, TEXT("MOTIONRIG_PA_RESULT=%s (savedPA=%d savedMesh=%d)"),
		(bSavedPA && bSavedMesh) ? TEXT("PASS") : TEXT("FAIL"), bSavedPA ? 1 : 0, bSavedMesh ? 1 : 0);

	return (bSavedPA && bSavedMesh) ? 0 : 1;
}
