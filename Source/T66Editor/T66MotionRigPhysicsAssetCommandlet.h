// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "Commandlets/Commandlet.h"
#include "T66MotionRigPhysicsAssetCommandlet.generated.h"

// MotionRig lane (MOTION_RIG.md section 3): generates the physics assets for
// the MotionRig skeletal meshes (Hero 1 male + female) deterministically —
// capsule bodies for every bone (tiny MinBoneSize so hands/feet are not
// skipped), generated constraints, asset assigned back to each mesh. Owns PA
// generation; the importer's create-physics-asset flag is kept off.
//
// Run:
//   UnrealEditor-Cmd.exe C:\UE\T66\T66.uproject -run=T66MotionRigPhysicsAsset
//       -unattended -nop4 -nosplash -stdout
UCLASS()
class UT66MotionRigPhysicsAssetCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	virtual int32 Main(const FString& Params) override;

private:
	static int32 BuildPhysicsAssetForCharacter(
		const TCHAR* MotionRigMeshPath, const TCHAR* MotionRigPhysicsAssetPackage);
};
