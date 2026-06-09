// Copyright Tribulation 66. All Rights Reserved.

#include "T66CreateTestRoomPhysicsAssetCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/SkeletalMesh.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "PhysicsAssetUtils.h"
#include "PhysicsEngine/AggregateGeom.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "PhysicsEngine/ConstraintDrives.h"
#include "PhysicsEngine/ConstraintTypes.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintTemplate.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66CreateTestRoomPhysicsAsset, Log, All);

namespace
{
	const TCHAR* T66DefaultSkeletalMeshPath = TEXT("/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/SK_Hero_1_Chad_Male_FriendSlop.SK_Hero_1_Chad_Male_FriendSlop");
	const TCHAR* T66DefaultPhysicsAssetPackagePath = TEXT("/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/PA_Hero_1_Chad_Male_FriendSlop_TestRoom");

	bool T66SaveAsset(UObject* Asset)
	{
		if (!Asset)
		{
			return false;
		}

		UPackage* Package = Asset->GetPackage();
		if (!Package)
		{
			return false;
		}

		Package->MarkPackageDirty();

		const FString PackageName = Package->GetName();
		const FString Filename = FPackageName::LongPackageNameToFilename(
			PackageName,
			FPackageName::GetAssetPackageExtension());

		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		return UPackage::SavePackage(Package, Asset, *Filename, SaveArgs);
	}

	FString NormalizeAssetPackagePath(FString AssetPathOrObjectPath)
	{
		AssetPathOrObjectPath.TrimStartAndEndInline();
		if (AssetPathOrObjectPath.Contains(TEXT(".")))
		{
			return FPackageName::ObjectPathToPackageName(AssetPathOrObjectPath);
		}
		return AssetPathOrObjectPath;
	}

	FString JsonString(const FString& Value)
	{
		return FString::Printf(TEXT("\"%s\""), *Value.ReplaceCharWithEscapedChar());
	}

	FString JsonBool(const bool bValue)
	{
		return bValue ? TEXT("true") : TEXT("false");
	}

	FString JsonVector(const FVector& Value)
	{
		return FString::Printf(TEXT("[%.3f, %.3f, %.3f]"), Value.X, Value.Y, Value.Z);
	}

	FString JsonRotator(const FRotator& Value)
	{
		return FString::Printf(TEXT("[%.3f, %.3f, %.3f]"), Value.Roll, Value.Pitch, Value.Yaw);
	}

	const TCHAR* LinearMotionName(const ELinearConstraintMotion Motion)
	{
		switch (Motion)
		{
		case LCM_Free:
			return TEXT("Free");
		case LCM_Limited:
			return TEXT("Limited");
		case LCM_Locked:
			return TEXT("Locked");
		default:
			return TEXT("Unknown");
		}
	}

	const TCHAR* AngularMotionName(const EAngularConstraintMotion Motion)
	{
		switch (Motion)
		{
		case ACM_Free:
			return TEXT("Free");
		case ACM_Limited:
			return TEXT("Limited");
		case ACM_Locked:
			return TEXT("Locked");
		default:
			return TEXT("Unknown");
		}
	}

	const TCHAR* CollisionEnabledName(const ECollisionEnabled::Type Type)
	{
		switch (Type)
		{
		case ECollisionEnabled::NoCollision:
			return TEXT("NoCollision");
		case ECollisionEnabled::QueryOnly:
			return TEXT("QueryOnly");
		case ECollisionEnabled::PhysicsOnly:
			return TEXT("PhysicsOnly");
		case ECollisionEnabled::QueryAndPhysics:
			return TEXT("QueryAndPhysics");
		default:
			return TEXT("Unknown");
		}
	}

	void AppendJsonStringArray(FString& Json, const TCHAR* FieldName, const TArray<FString>& Values, const bool bTrailingComma)
	{
		Json += FString::Printf(TEXT("  \"%s\": ["), FieldName);
		for (int32 Index = 0; Index < Values.Num(); ++Index)
		{
			Json += JsonString(Values[Index]);
			if (Index + 1 < Values.Num())
			{
				Json += TEXT(", ");
			}
		}
		Json += bTrailingComma ? TEXT("],\n") : TEXT("]\n");
	}

	bool WritePhysicsAssetReport(
		const FString& ReportPath,
		const FString& MeshPath,
		const FString& PhysicsAssetPackagePath,
		const UPhysicsAsset* PhysicsAsset,
		const float MinBoneSize,
		const bool bBodyForAll)
	{
		if (ReportPath.IsEmpty() || !PhysicsAsset)
		{
			return true;
		}

		TArray<FString> BodyBones;
		BodyBones.Reserve(PhysicsAsset->SkeletalBodySetups.Num());
		for (const USkeletalBodySetup* BodySetup : PhysicsAsset->SkeletalBodySetups)
		{
			BodyBones.Add(BodySetup ? BodySetup->BoneName.ToString() : FString());
		}

		TArray<FString> ConstraintPairs;
		ConstraintPairs.Reserve(PhysicsAsset->ConstraintSetup.Num());
		for (const UPhysicsConstraintTemplate* ConstraintTemplate : PhysicsAsset->ConstraintSetup)
		{
			if (!ConstraintTemplate)
			{
				ConstraintPairs.Add(FString());
				continue;
			}
			ConstraintPairs.Add(FString::Printf(
				TEXT("%s:%s"),
				*ConstraintTemplate->DefaultInstance.ConstraintBone1.ToString(),
				*ConstraintTemplate->DefaultInstance.ConstraintBone2.ToString()));
		}

		FString Json;
		Json += TEXT("{\n");
		Json += FString::Printf(TEXT("  \"ok\": true,\n"));
		Json += FString::Printf(TEXT("  \"mesh\": %s,\n"), *JsonString(MeshPath));
		Json += FString::Printf(TEXT("  \"physics_asset\": %s,\n"), *JsonString(PhysicsAssetPackagePath));
		Json += FString::Printf(TEXT("  \"body_count\": %d,\n"), PhysicsAsset->SkeletalBodySetups.Num());
		Json += FString::Printf(TEXT("  \"constraint_count\": %d,\n"), PhysicsAsset->ConstraintSetup.Num());
		Json += FString::Printf(TEXT("  \"min_bone_size\": %.3f,\n"), MinBoneSize);
		Json += FString::Printf(TEXT("  \"body_for_all\": %s,\n"), bBodyForAll ? TEXT("true") : TEXT("false"));
		Json += FString::Printf(TEXT("  \"collision_disabled_pair_count\": %d,\n"), PhysicsAsset->CollisionDisableTable.Num());
		AppendJsonStringArray(Json, TEXT("body_bones"), BodyBones, true);
		AppendJsonStringArray(Json, TEXT("constraint_pairs"), ConstraintPairs, true);

		Json += TEXT("  \"body_details\": [\n");
		for (int32 BodyIndex = 0; BodyIndex < PhysicsAsset->SkeletalBodySetups.Num(); ++BodyIndex)
		{
			const USkeletalBodySetup* BodySetup = PhysicsAsset->SkeletalBodySetups[BodyIndex];
			const FBodyInstance* BodyInstance = BodySetup ? &BodySetup->DefaultInstance : nullptr;
			Json += TEXT("    {\n");
			Json += FString::Printf(TEXT("      \"bone_name\": %s,\n"), BodySetup ? *JsonString(BodySetup->BoneName.ToString()) : TEXT("\"\""));
			if (BodyInstance)
			{
				Json += FString::Printf(TEXT("      \"collision_enabled\": %s,\n"), *JsonString(CollisionEnabledName(BodyInstance->GetCollisionEnabled(false))));
				Json += FString::Printf(TEXT("      \"linear_damping\": %.3f,\n"), BodyInstance->LinearDamping);
				Json += FString::Printf(TEXT("      \"angular_damping\": %.3f,\n"), BodyInstance->AngularDamping);
				Json += FString::Printf(TEXT("      \"mass_scale\": %.3f,\n"), BodyInstance->MassScale);
				Json += FString::Printf(TEXT("      \"override_iteration_counts\": %s,\n"), *JsonBool(BodyInstance->GetPositionSolverIterationCount() >= 0));
				Json += FString::Printf(TEXT("      \"position_solver_iterations\": %d,\n"), BodyInstance->GetPositionSolverIterationCount());
				Json += FString::Printf(TEXT("      \"velocity_solver_iterations\": %d,\n"), BodyInstance->GetVelocitySolverIterationCount());
				Json += FString::Printf(TEXT("      \"projection_solver_iterations\": %d,\n"), BodyInstance->GetProjectionSolverIterationCount());
				Json += FString::Printf(TEXT("      \"inertia_conditioning\": %s,\n"), *JsonBool(BodyInstance->IsInertiaConditioningEnabled()));
			}
			Json += TEXT("      \"sphyls\": [");
			if (BodySetup)
			{
				for (int32 SphylIndex = 0; SphylIndex < BodySetup->AggGeom.SphylElems.Num(); ++SphylIndex)
				{
					const FKSphylElem& Sphyl = BodySetup->AggGeom.SphylElems[SphylIndex];
					Json += FString::Printf(
						TEXT("{\"radius\": %.3f, \"length\": %.3f, \"center\": %s, \"rotation\": %s}"),
						Sphyl.Radius,
						Sphyl.Length,
						*JsonVector(Sphyl.Center),
						*JsonRotator(Sphyl.Rotation));
					if (SphylIndex + 1 < BodySetup->AggGeom.SphylElems.Num())
					{
						Json += TEXT(", ");
					}
				}
			}
			Json += TEXT("],\n");
			Json += FString::Printf(TEXT("      \"sphere_count\": %d,\n"), BodySetup ? BodySetup->AggGeom.SphereElems.Num() : 0);
			Json += FString::Printf(TEXT("      \"box_count\": %d,\n"), BodySetup ? BodySetup->AggGeom.BoxElems.Num() : 0);
			Json += FString::Printf(TEXT("      \"convex_count\": %d\n"), BodySetup ? BodySetup->AggGeom.ConvexElems.Num() : 0);
			Json += BodyIndex + 1 < PhysicsAsset->SkeletalBodySetups.Num() ? TEXT("    },\n") : TEXT("    }\n");
		}
		Json += TEXT("  ],\n");

		Json += TEXT("  \"constraint_details\": [\n");
		for (int32 ConstraintIndex = 0; ConstraintIndex < PhysicsAsset->ConstraintSetup.Num(); ++ConstraintIndex)
		{
			const UPhysicsConstraintTemplate* ConstraintTemplate = PhysicsAsset->ConstraintSetup[ConstraintIndex];
			const FConstraintInstance* Constraint = ConstraintTemplate ? &ConstraintTemplate->DefaultInstance : nullptr;
			Json += TEXT("    {\n");
			if (Constraint)
			{
				float ProjectionLinearAlpha = 0.f;
				float ProjectionAngularAlpha = 0.f;
				float ProjectionLinearTolerance = 0.f;
				float ProjectionAngularTolerance = 0.f;
				Constraint->GetProjectionParams(ProjectionLinearAlpha, ProjectionAngularAlpha, ProjectionLinearTolerance, ProjectionAngularTolerance);
				Json += FString::Printf(TEXT("      \"constraint_bone1\": %s,\n"), *JsonString(Constraint->ConstraintBone1.ToString()));
				Json += FString::Printf(TEXT("      \"constraint_bone2\": %s,\n"), *JsonString(Constraint->ConstraintBone2.ToString()));
				Json += FString::Printf(TEXT("      \"linear_x_motion\": %s,\n"), *JsonString(LinearMotionName(Constraint->GetLinearXMotion())));
				Json += FString::Printf(TEXT("      \"linear_y_motion\": %s,\n"), *JsonString(LinearMotionName(Constraint->GetLinearYMotion())));
				Json += FString::Printf(TEXT("      \"linear_z_motion\": %s,\n"), *JsonString(LinearMotionName(Constraint->GetLinearZMotion())));
				Json += FString::Printf(TEXT("      \"linear_limit\": %.3f,\n"), Constraint->GetLinearLimit());
				Json += FString::Printf(TEXT("      \"swing1_motion\": %s,\n"), *JsonString(AngularMotionName(Constraint->GetAngularSwing1Motion())));
				Json += FString::Printf(TEXT("      \"swing2_motion\": %s,\n"), *JsonString(AngularMotionName(Constraint->GetAngularSwing2Motion())));
				Json += FString::Printf(TEXT("      \"twist_motion\": %s,\n"), *JsonString(AngularMotionName(Constraint->GetAngularTwistMotion())));
				Json += FString::Printf(TEXT("      \"swing1_limit\": %.3f,\n"), Constraint->GetAngularSwing1Limit());
				Json += FString::Printf(TEXT("      \"swing2_limit\": %.3f,\n"), Constraint->GetAngularSwing2Limit());
				Json += FString::Printf(TEXT("      \"twist_limit\": %.3f,\n"), Constraint->GetAngularTwistLimit());
				Json += FString::Printf(TEXT("      \"disable_collision\": %s,\n"), *JsonBool(!!Constraint->ProfileInstance.bDisableCollision));
				Json += FString::Printf(TEXT("      \"parent_dominates\": %s,\n"), *JsonBool(Constraint->IsParentDominatesEnabled()));
				Json += FString::Printf(TEXT("      \"projection_enabled\": %s,\n"), *JsonBool(Constraint->IsProjectionEnabled()));
				Json += FString::Printf(TEXT("      \"projection_linear_alpha\": %.3f,\n"), ProjectionLinearAlpha);
				Json += FString::Printf(TEXT("      \"projection_angular_alpha\": %.3f,\n"), ProjectionAngularAlpha);
				Json += FString::Printf(TEXT("      \"projection_linear_tolerance\": %.3f,\n"), ProjectionLinearTolerance);
				Json += FString::Printf(TEXT("      \"projection_angular_tolerance\": %.3f,\n"), ProjectionAngularTolerance);
				Json += FString::Printf(TEXT("      \"shock_propagation_alpha\": %.3f\n"), Constraint->GetShockPropagationAlpha());
			}
			Json += ConstraintIndex + 1 < PhysicsAsset->ConstraintSetup.Num() ? TEXT("    },\n") : TEXT("    }\n");
		}
		Json += TEXT("  ]\n");
		Json += TEXT("}\n");

		IFileManager::Get().MakeDirectory(*FPaths::GetPath(ReportPath), true);
		return FFileHelper::SaveStringToFile(Json, *ReportPath);
	}

	TArray<FName> MakeControlledRagdollBodyList()
	{
		TArray<FName> BodyBones;
		const TCHAR* Names[] =
		{
			TEXT("pelvis"),
			TEXT("spine_01"),
			TEXT("spine_02"),
			TEXT("spine_03"),
			TEXT("neck_01"),
			TEXT("head"),
			TEXT("upperarm_l"),
			TEXT("lowerarm_l"),
			TEXT("hand_l"),
			TEXT("upperarm_r"),
			TEXT("lowerarm_r"),
			TEXT("hand_r"),
			TEXT("thigh_l"),
			TEXT("calf_l"),
			TEXT("foot_l"),
			TEXT("thigh_r"),
			TEXT("calf_r"),
			TEXT("foot_r")
		};

		for (const TCHAR* Name : Names)
		{
			BodyBones.Add(FName(Name));
		}
		return BodyBones;
	}

	TSet<FName> MakeControlledRagdollBodySet()
	{
		TSet<FName> BodyBones;
		for (const FName BoneName : MakeControlledRagdollBodyList())
		{
			BodyBones.Add(BoneName);
		}
		return BodyBones;
	}

	bool IsCoreRagdollBody(const FName BoneName)
	{
		return BoneName == FName(TEXT("pelvis"))
			|| BoneName == FName(TEXT("spine_01"))
			|| BoneName == FName(TEXT("spine_02"))
			|| BoneName == FName(TEXT("spine_03"))
			|| BoneName == FName(TEXT("neck_01"))
			|| BoneName == FName(TEXT("head"));
	}

	bool IsSkeletonAncestor(const USkeletalMesh* SkeletalMesh, const FName PotentialAncestor, const FName BoneName)
	{
		if (!SkeletalMesh)
		{
			return false;
		}

		const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();
		const int32 AncestorIndex = RefSkeleton.FindBoneIndex(PotentialAncestor);
		int32 CurrentIndex = RefSkeleton.FindBoneIndex(BoneName);
		if (AncestorIndex == INDEX_NONE || CurrentIndex == INDEX_NONE || AncestorIndex == CurrentIndex)
		{
			return false;
		}

		while (CurrentIndex != INDEX_NONE)
		{
			CurrentIndex = RefSkeleton.GetParentIndex(CurrentIndex);
			if (CurrentIndex == AncestorIndex)
			{
				return true;
			}
		}
		return false;
	}

	FName SelectControlledConstraintChildBone(const USkeletalMesh* SkeletalMesh, const FConstraintInstance& Constraint)
	{
		if (IsSkeletonAncestor(SkeletalMesh, Constraint.ConstraintBone1, Constraint.ConstraintBone2))
		{
			return Constraint.ConstraintBone2;
		}
		if (IsSkeletonAncestor(SkeletalMesh, Constraint.ConstraintBone2, Constraint.ConstraintBone1))
		{
			return Constraint.ConstraintBone1;
		}
		return Constraint.ConstraintBone2;
	}

	FName FindNearestControlledAncestorBody(const USkeletalMesh* SkeletalMesh, const FName ChildBoneName, const TSet<FName>& AllowedBodies)
	{
		if (!SkeletalMesh)
		{
			return NAME_None;
		}

		const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();
		int32 CurrentIndex = RefSkeleton.FindBoneIndex(ChildBoneName);
		if (CurrentIndex == INDEX_NONE)
		{
			return NAME_None;
		}

		CurrentIndex = RefSkeleton.GetParentIndex(CurrentIndex);
		while (CurrentIndex != INDEX_NONE)
		{
			const FName CandidateName = RefSkeleton.GetBoneName(CurrentIndex);
			if (AllowedBodies.Contains(CandidateName))
			{
				return CandidateName;
			}
			CurrentIndex = RefSkeleton.GetParentIndex(CurrentIndex);
		}

		return NAME_None;
	}

	int32 EnsureControlledRagdollBodyGraph(UPhysicsAsset* PhysicsAsset, const USkeletalMesh* SkeletalMesh, const TSet<FName>& AllowedBodies)
	{
		if (!PhysicsAsset || !SkeletalMesh)
		{
			return 0;
		}

		TSet<FName> ConstrainedChildren;
		for (const UPhysicsConstraintTemplate* ConstraintTemplate : PhysicsAsset->ConstraintSetup)
		{
			if (!ConstraintTemplate)
			{
				continue;
			}

			const FName ChildBoneName = SelectControlledConstraintChildBone(SkeletalMesh, ConstraintTemplate->DefaultInstance);
			if (!ChildBoneName.IsNone())
			{
				ConstrainedChildren.Add(ChildBoneName);
			}
		}

		int32 CreatedConstraintCount = 0;
		for (const FName ChildBoneName : MakeControlledRagdollBodyList())
		{
			if (ChildBoneName == FName(TEXT("pelvis"))
				|| !AllowedBodies.Contains(ChildBoneName)
				|| ConstrainedChildren.Contains(ChildBoneName)
				|| PhysicsAsset->FindBodyIndex(ChildBoneName) == INDEX_NONE)
			{
				continue;
			}

			const FName ParentBoneName = FindNearestControlledAncestorBody(SkeletalMesh, ChildBoneName, AllowedBodies);
			if (ParentBoneName.IsNone() || PhysicsAsset->FindBodyIndex(ParentBoneName) == INDEX_NONE)
			{
				UE_LOG(LogT66CreateTestRoomPhysicsAsset, Warning, TEXT("Could not reconnect controlled ragdoll body %s; no kept ancestor body found."),
					*ChildBoneName.ToString());
				continue;
			}

			const int32 NewConstraintIndex = FPhysicsAssetUtils::CreateNewConstraint(PhysicsAsset, ChildBoneName);
			UPhysicsConstraintTemplate* ConstraintTemplate = PhysicsAsset->ConstraintSetup.IsValidIndex(NewConstraintIndex)
				? PhysicsAsset->ConstraintSetup[NewConstraintIndex]
				: nullptr;
			if (!ConstraintTemplate)
			{
				continue;
			}

			FConstraintInstance& Constraint = ConstraintTemplate->DefaultInstance;
			Constraint.ConstraintBone1 = ChildBoneName;
			Constraint.ConstraintBone2 = ParentBoneName;
			Constraint.SnapTransformsToDefault(EConstraintTransformComponentFlags::All, PhysicsAsset);
			ConstraintTemplate->SetDefaultProfile(Constraint);
			ConstrainedChildren.Add(ChildBoneName);
			++CreatedConstraintCount;

			UE_LOG(LogT66CreateTestRoomPhysicsAsset, Display, TEXT("Reconnected controlled ragdoll body %s to %s."),
				*ChildBoneName.ToString(),
				*ParentBoneName.ToString());
		}

		return CreatedConstraintCount;
	}

	void SelectControlledConstraintLimits(const FName ChildBoneName, float& OutSwing1, float& OutSwing2, float& OutTwist)
	{
		OutSwing1 = 35.f;
		OutSwing2 = 35.f;
		OutTwist = 10.f;

		if (ChildBoneName == FName(TEXT("spine_01"))
			|| ChildBoneName == FName(TEXT("spine_02"))
			|| ChildBoneName == FName(TEXT("spine_03"))
			|| ChildBoneName == FName(TEXT("neck_01")))
		{
			OutSwing1 = 18.f;
			OutSwing2 = 18.f;
			OutTwist = 7.f;
		}
		else if (ChildBoneName == FName(TEXT("head")))
		{
			OutSwing1 = 24.f;
			OutSwing2 = 24.f;
			OutTwist = 9.f;
		}
		else if (ChildBoneName == FName(TEXT("upperarm_l")) || ChildBoneName == FName(TEXT("upperarm_r")))
		{
			OutSwing1 = 64.f;
			OutSwing2 = 72.f;
			OutTwist = 24.f;
		}
		else if (ChildBoneName == FName(TEXT("lowerarm_l")) || ChildBoneName == FName(TEXT("lowerarm_r")))
		{
			OutSwing1 = 8.f;
			OutSwing2 = 58.f;
			OutTwist = 7.f;
		}
		else if (ChildBoneName == FName(TEXT("hand_l")) || ChildBoneName == FName(TEXT("hand_r")))
		{
			OutSwing1 = 24.f;
			OutSwing2 = 24.f;
			OutTwist = 8.f;
		}
		else if (ChildBoneName == FName(TEXT("thigh_l")) || ChildBoneName == FName(TEXT("thigh_r")))
		{
			OutSwing1 = 48.f;
			OutSwing2 = 34.f;
			OutTwist = 14.f;
		}
		else if (ChildBoneName == FName(TEXT("calf_l")) || ChildBoneName == FName(TEXT("calf_r")))
		{
			OutSwing1 = 8.f;
			OutSwing2 = 56.f;
			OutTwist = 6.f;
		}
		else if (ChildBoneName == FName(TEXT("foot_l")) || ChildBoneName == FName(TEXT("foot_r")))
		{
			OutSwing1 = 30.f;
			OutSwing2 = 20.f;
			OutTwist = 8.f;
		}
	}

	void SelectControlledBodyMinimums(const FName BoneName, float& OutMinRadius, float& OutMinLength, float& OutMassScale)
	{
		OutMinRadius = 12.f;
		OutMinLength = 20.f;
		OutMassScale = 0.75f;

		if (BoneName == FName(TEXT("pelvis")))
		{
			OutMinRadius = 30.f;
			OutMinLength = 38.f;
			OutMassScale = 2.6f;
		}
		else if (BoneName == FName(TEXT("spine_01")) || BoneName == FName(TEXT("spine_02")) || BoneName == FName(TEXT("spine_03")))
		{
			OutMinRadius = 25.f;
			OutMinLength = 28.f;
			OutMassScale = 1.8f;
		}
		else if (BoneName == FName(TEXT("neck_01")))
		{
			OutMinRadius = 12.f;
			OutMinLength = 14.f;
			OutMassScale = 0.9f;
		}
		else if (BoneName == FName(TEXT("head")))
		{
			OutMinRadius = 18.f;
			OutMinLength = 18.f;
			OutMassScale = 1.1f;
		}
		else if (BoneName == FName(TEXT("upperarm_l")) || BoneName == FName(TEXT("upperarm_r")))
		{
			OutMinRadius = 13.f;
			OutMinLength = 32.f;
			OutMassScale = 0.7f;
		}
		else if (BoneName == FName(TEXT("lowerarm_l")) || BoneName == FName(TEXT("lowerarm_r")))
		{
			OutMinRadius = 11.f;
			OutMinLength = 28.f;
			OutMassScale = 0.55f;
		}
		else if (BoneName == FName(TEXT("hand_l")) || BoneName == FName(TEXT("hand_r")))
		{
			OutMinRadius = 9.f;
			OutMinLength = 12.f;
			OutMassScale = 0.35f;
		}
		else if (BoneName == FName(TEXT("thigh_l")) || BoneName == FName(TEXT("thigh_r")))
		{
			OutMinRadius = 16.f;
			OutMinLength = 42.f;
			OutMassScale = 0.95f;
		}
		else if (BoneName == FName(TEXT("calf_l")) || BoneName == FName(TEXT("calf_r")))
		{
			OutMinRadius = 13.f;
			OutMinLength = 34.f;
			OutMassScale = 0.75f;
		}
		else if (BoneName == FName(TEXT("foot_l")) || BoneName == FName(TEXT("foot_r")))
		{
			OutMinRadius = 10.f;
			OutMinLength = 24.f;
			OutMassScale = 0.65f;
		}
	}

	void ConfigureControlledSelfCollision(UPhysicsAsset* PhysicsAsset)
	{
		if (!PhysicsAsset)
		{
			return;
		}

		PhysicsAsset->CollisionDisableTable.Empty();
		for (int32 IndexA = 0; IndexA < PhysicsAsset->SkeletalBodySetups.Num(); ++IndexA)
		{
			for (int32 IndexB = IndexA + 1; IndexB < PhysicsAsset->SkeletalBodySetups.Num(); ++IndexB)
			{
				PhysicsAsset->DisableCollision(IndexA, IndexB);
			}
		}
	}

	void HardenControlledBody(USkeletalBodySetup* BodySetup)
	{
		if (!BodySetup)
		{
			return;
		}

		float MinRadius = 0.f;
		float MinLength = 0.f;
		float MassScale = 1.f;
		SelectControlledBodyMinimums(BodySetup->BoneName, MinRadius, MinLength, MassScale);

		if (BodySetup->AggGeom.SphylElems.Num() == 0)
		{
			BodySetup->AggGeom.SphylElems.Add(FKSphylElem(MinRadius, MinLength));
		}

		for (FKSphylElem& Sphyl : BodySetup->AggGeom.SphylElems)
		{
			Sphyl.Radius = FMath::Max(Sphyl.Radius, MinRadius);
			Sphyl.Length = FMath::Max(Sphyl.Length, MinLength);
		}

		FBodyInstance& BodyInstance = BodySetup->DefaultInstance;
		const bool bCoreBody = IsCoreRagdollBody(BodySetup->BoneName);
		BodyInstance.SetCollisionEnabled(ECollisionEnabled::PhysicsOnly, false);
		BodyInstance.SetOverrideIterationCounts(true);
		BodyInstance.PositionSolverIterationCount = bCoreBody ? 24 : 18;
		BodyInstance.VelocitySolverIterationCount = bCoreBody ? 8 : 6;
		BodyInstance.ProjectionSolverIterationCount = 2;
		BodyInstance.SetInertiaConditioningEnabled(true);
		BodyInstance.LinearDamping = bCoreBody ? 4.2f : 2.4f;
		BodyInstance.AngularDamping = bCoreBody ? 18.f : 11.f;
		BodyInstance.MassScale = MassScale;
		BodyInstance.SleepFamily = ESleepFamily::Sensitive;

		BodySetup->InvalidatePhysicsData();
		BodySetup->CreatePhysicsMeshes();
	}

	void ConfigureControlledRagdollPhysicsAsset(UPhysicsAsset* PhysicsAsset, const USkeletalMesh* SkeletalMesh)
	{
		if (!PhysicsAsset)
		{
			return;
		}

		const TSet<FName> AllowedBodies = MakeControlledRagdollBodySet();
		PhysicsAsset->SkeletalBodySetups.RemoveAll([&AllowedBodies](const USkeletalBodySetup* BodySetup)
		{
			return !BodySetup || !AllowedBodies.Contains(BodySetup->BoneName);
		});
		PhysicsAsset->ConstraintSetup.RemoveAll([&AllowedBodies](const UPhysicsConstraintTemplate* ConstraintTemplate)
		{
			if (!ConstraintTemplate)
			{
				return true;
			}

			const FConstraintInstance& Constraint = ConstraintTemplate->DefaultInstance;
			return !AllowedBodies.Contains(Constraint.ConstraintBone1)
				|| !AllowedBodies.Contains(Constraint.ConstraintBone2);
		});

		const int32 ReconnectedConstraintCount = EnsureControlledRagdollBodyGraph(PhysicsAsset, SkeletalMesh, AllowedBodies);
		if (ReconnectedConstraintCount > 0)
		{
			UE_LOG(LogT66CreateTestRoomPhysicsAsset, Display, TEXT("Reconnected %d controlled ragdoll constraints after pruning helper bodies."),
				ReconnectedConstraintCount);
		}

		PhysicsAsset->UpdateBodySetupIndexMap();
		PhysicsAsset->UpdateBoundsBodiesArray();

		for (USkeletalBodySetup* BodySetup : PhysicsAsset->SkeletalBodySetups)
		{
			if (!BodySetup)
			{
				continue;
			}

			HardenControlledBody(BodySetup);
		}

		for (UPhysicsConstraintTemplate* ConstraintTemplate : PhysicsAsset->ConstraintSetup)
		{
			if (!ConstraintTemplate)
			{
				continue;
			}

			FConstraintInstance& Constraint = ConstraintTemplate->DefaultInstance;
			const FName ChildBoneName = SelectControlledConstraintChildBone(SkeletalMesh, Constraint);
			float Swing1 = 0.f;
			float Swing2 = 0.f;
			float Twist = 0.f;
			SelectControlledConstraintLimits(ChildBoneName, Swing1, Swing2, Twist);

			Constraint.SetLinearLimits(LCM_Locked, LCM_Locked, LCM_Locked, 0.f);
			Constraint.SetSoftLinearLimitParams(false, 0.f, 0.f, 0.f, 0.f);
			Constraint.SetSoftSwingLimitParams(false, 0.f, 0.f, 0.f, 0.f);
			Constraint.SetSoftTwistLimitParams(false, 0.f, 0.f, 0.f, 0.f);
			Constraint.SetDisableCollision(true);
			Constraint.SetParentDominates(IsCoreRagdollBody(Constraint.ConstraintBone2));
			Constraint.SetAngularSwing1Limit(ACM_Limited, Swing1);
			Constraint.SetAngularSwing2Limit(ACM_Limited, Swing2);
			Constraint.SetAngularTwistLimit(ACM_Limited, Twist);
			Constraint.SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
			Constraint.SetOrientationDriveTwistAndSwing(true, true);
			Constraint.SetAngularVelocityDriveTwistAndSwing(true, true);
			Constraint.SetAngularDriveParams(900.f, 80.f, 0.f);
			Constraint.SetProjectionParams(true, 0.35f, 0.65f, 2.f, 6.f);
			Constraint.SetShockPropagationParams(true, IsCoreRagdollBody(Constraint.ConstraintBone2) ? 0.22f : 0.12f);
			ConstraintTemplate->SetDefaultProfile(Constraint);
		}

		ConfigureControlledSelfCollision(PhysicsAsset);
	}
}

UT66CreateTestRoomPhysicsAssetCommandlet::UT66CreateTestRoomPhysicsAssetCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UT66CreateTestRoomPhysicsAssetCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	FString MeshPath = T66DefaultSkeletalMeshPath;
	FString PhysicsAssetPackagePath = T66DefaultPhysicsAssetPackagePath;
	FString ReportPath;
	float MinBoneSize = 4.0f;
	bool bBodyForAll = true;
	FParse::Value(*Params, TEXT("Mesh="), MeshPath);
	FParse::Value(*Params, TEXT("Asset="), PhysicsAssetPackagePath);
	FParse::Value(*Params, TEXT("Report="), ReportPath);
	FParse::Value(*Params, TEXT("MinBoneSize="), MinBoneSize);
	FParse::Bool(*Params, TEXT("BodyForAll="), bBodyForAll);
	const bool bReportOnly = FParse::Param(*Params, TEXT("ReportOnly"));
	const bool bTuneExisting = FParse::Param(*Params, TEXT("TuneExisting"));
	PhysicsAssetPackagePath = NormalizeAssetPackagePath(PhysicsAssetPackagePath);

	USkeletalMesh* SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
	if (!SkeletalMesh)
	{
		UE_LOG(LogT66CreateTestRoomPhysicsAsset, Error, TEXT("Could not load skeletal mesh %s"), *MeshPath);
		return 1;
	}

	const FString AssetName = FPackageName::GetLongPackageAssetName(PhysicsAssetPackagePath);
	if (AssetName.IsEmpty())
	{
		UE_LOG(LogT66CreateTestRoomPhysicsAsset, Error, TEXT("Invalid physics asset path %s"), *PhysicsAssetPackagePath);
		return 1;
	}

	if (bReportOnly || bTuneExisting)
	{
		const FString PhysicsAssetObjectPath = FString::Printf(TEXT("%s.%s"), *PhysicsAssetPackagePath, *AssetName);
		UPhysicsAsset* ExistingPhysicsAsset = LoadObject<UPhysicsAsset>(nullptr, *PhysicsAssetObjectPath);
		if (!ExistingPhysicsAsset)
		{
			ExistingPhysicsAsset = LoadObject<UPhysicsAsset>(nullptr, *PhysicsAssetPackagePath);
		}

		if (!ExistingPhysicsAsset)
		{
			UE_LOG(LogT66CreateTestRoomPhysicsAsset, Error, TEXT("Could not load existing physics asset %s"), *PhysicsAssetPackagePath);
			return 1;
		}

		if (bTuneExisting)
		{
			ExistingPhysicsAsset->Modify();
			ConfigureControlledRagdollPhysicsAsset(ExistingPhysicsAsset, SkeletalMesh);
#if WITH_EDITORONLY_DATA
			ExistingPhysicsAsset->PreviewSkeletalMesh = SkeletalMesh;
#endif
			SkeletalMesh->SetPhysicsAsset(ExistingPhysicsAsset);
			ExistingPhysicsAsset->MarkPackageDirty();
			SkeletalMesh->MarkPackageDirty();

			if (!T66SaveAsset(ExistingPhysicsAsset))
			{
				UE_LOG(LogT66CreateTestRoomPhysicsAsset, Error, TEXT("Failed to save tuned physics asset %s"), *PhysicsAssetPackagePath);
				return 1;
			}

			if (!T66SaveAsset(SkeletalMesh))
			{
				UE_LOG(LogT66CreateTestRoomPhysicsAsset, Error, TEXT("Failed to save skeletal mesh %s after assigning tuned physics asset"), *MeshPath);
				return 1;
			}
		}

		if (!WritePhysicsAssetReport(ReportPath, MeshPath, PhysicsAssetPackagePath, ExistingPhysicsAsset, MinBoneSize, bBodyForAll))
		{
			UE_LOG(LogT66CreateTestRoomPhysicsAsset, Error, TEXT("Failed to write physics asset report %s"), *ReportPath);
			return 1;
		}

		UE_LOG(LogT66CreateTestRoomPhysicsAsset, Display, TEXT("%s physics asset %s for mesh %s. Bodies=%d Constraints=%d"),
			bTuneExisting ? TEXT("Tuned existing") : TEXT("Reported existing"),
			*PhysicsAssetPackagePath,
			*MeshPath,
			ExistingPhysicsAsset->SkeletalBodySetups.Num(),
			ExistingPhysicsAsset->ConstraintSetup.Num());
		return 0;
	}

	UPackage* Package = CreatePackage(*PhysicsAssetPackagePath);
	if (!Package)
	{
		UE_LOG(LogT66CreateTestRoomPhysicsAsset, Error, TEXT("Could not create package %s"), *PhysicsAssetPackagePath);
		return 1;
	}

	UPhysicsAsset* PhysicsAsset = FindObject<UPhysicsAsset>(Package, *AssetName);
	const bool bCreatedAsset = PhysicsAsset == nullptr;
	if (!PhysicsAsset)
	{
		PhysicsAsset = NewObject<UPhysicsAsset>(
			Package,
			*AssetName,
			RF_Public | RF_Standalone | RF_Transactional);
	}

	if (!PhysicsAsset)
	{
		UE_LOG(LogT66CreateTestRoomPhysicsAsset, Error, TEXT("Could not allocate physics asset %s"), *PhysicsAssetPackagePath);
		return 1;
	}

	PhysicsAsset->Modify();
	PhysicsAsset->SkeletalBodySetups.Empty();
	PhysicsAsset->ConstraintSetup.Empty();
	PhysicsAsset->BodySetupIndexMap.Empty();
	PhysicsAsset->CollisionDisableTable.Empty();
	PhysicsAsset->BoundsBodies.Empty();

	FPhysAssetCreateParams CreateParams;
	CreateParams.MinBoneSize = FMath::Max(0.1f, MinBoneSize);
	CreateParams.GeomType = EFG_Sphyl;
	CreateParams.bCreateConstraints = true;
	CreateParams.bDisableCollisionsByDefault = true;
	CreateParams.bBodyForAll = bBodyForAll;

	FText ErrorMessage;
	const bool bSuccess = FPhysicsAssetUtils::CreateFromSkeletalMesh(
		PhysicsAsset,
		SkeletalMesh,
		CreateParams,
		ErrorMessage,
		true,
		false);
	if (!bSuccess)
	{
		UE_LOG(LogT66CreateTestRoomPhysicsAsset, Error, TEXT("Physics asset generation failed for %s: %s"),
			*MeshPath,
			*ErrorMessage.ToString());
		return 1;
	}

	ConfigureControlledRagdollPhysicsAsset(PhysicsAsset, SkeletalMesh);

#if WITH_EDITORONLY_DATA
	PhysicsAsset->PreviewSkeletalMesh = SkeletalMesh;
#endif
	SkeletalMesh->SetPhysicsAsset(PhysicsAsset);
	PhysicsAsset->MarkPackageDirty();
	SkeletalMesh->MarkPackageDirty();

	if (bCreatedAsset)
	{
		FAssetRegistryModule::AssetCreated(PhysicsAsset);
	}

	if (!T66SaveAsset(PhysicsAsset))
	{
		UE_LOG(LogT66CreateTestRoomPhysicsAsset, Error, TEXT("Failed to save physics asset %s"), *PhysicsAssetPackagePath);
		return 1;
	}

	if (!T66SaveAsset(SkeletalMesh))
	{
		UE_LOG(LogT66CreateTestRoomPhysicsAsset, Error, TEXT("Failed to save skeletal mesh %s after assigning physics asset"), *MeshPath);
		return 1;
	}

	if (!WritePhysicsAssetReport(ReportPath, MeshPath, PhysicsAssetPackagePath, PhysicsAsset, CreateParams.MinBoneSize, bBodyForAll))
	{
		UE_LOG(LogT66CreateTestRoomPhysicsAsset, Error, TEXT("Failed to write physics asset report %s"), *ReportPath);
		return 1;
	}

	UE_LOG(LogT66CreateTestRoomPhysicsAsset, Display, TEXT("Created TestRoom physics asset %s for mesh %s. Bodies=%d Constraints=%d MinBoneSize=%.2f BodyForAll=%d"),
		*PhysicsAssetPackagePath,
		*MeshPath,
		PhysicsAsset->SkeletalBodySetups.Num(),
		PhysicsAsset->ConstraintSetup.Num(),
		CreateParams.MinBoneSize,
		bBodyForAll ? 1 : 0);
	return 0;
#else
	UE_LOG(LogT66CreateTestRoomPhysicsAsset, Error, TEXT("Editor-only physics asset commandlet is unavailable without WITH_EDITOR."));
	return 1;
#endif
}
