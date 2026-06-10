// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "Commandlets/Commandlet.h"
#include "T66MotionRigPhysicsAssetCommandlet.generated.h"

// MotionRig lane (MOTION_RIG.md section 3): generates the physics asset for
// the MotionRig skeletal mesh deterministically — capsule bodies for every
// bone (tiny MinBoneSize so hands/feet are not skipped), generated
// constraints, asset assigned back to the mesh. Replaces the FBX importer's
// create-physics-asset path, which does not run under automated import.
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
};
