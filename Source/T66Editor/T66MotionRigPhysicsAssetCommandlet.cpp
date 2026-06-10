// Copyright Tribulation 66. All Rights Reserved.

#include "T66MotionRigPhysicsAssetCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
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
	const TCHAR* MotionRigMeshPath =
		TEXT("/Game/Characters/MotionRig/Hero_1/SK_MotionRig_Hero1.SK_MotionRig_Hero1");
	const TCHAR* MotionRigPhysicsAssetPackage =
		TEXT("/Game/Characters/MotionRig/Hero_1/PA_MotionRig_Hero1");

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
	USkeletalMesh* SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, MotionRigMeshPath);
	if (!SkeletalMesh)
	{
		UE_LOG(LogT66MotionRigPA, Error, TEXT("MotionRig skeletal mesh not found: %s"), MotionRigMeshPath);
		return 1;
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
