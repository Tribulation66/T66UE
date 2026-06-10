// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "T66MotionRigInputReceiver.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UT66MotionRigInputReceiver : public UInterface
{
	GENERATED_BODY()
};

// The single seam input crosses to reach a MotionRig pawn. The player
// controller bridge calls this for local play today; a replicated input path
// can implement/feed the same interface for the multiplayer expansion without
// touching any motion code.
class T66_API IT66MotionRigInputReceiver
{
	GENERATED_BODY()

public:
	virtual void MotionRigSetMoveAxes(float ForwardValue, float RightValue) = 0;
	virtual void MotionRigJumpPressed() = 0;
	virtual void MotionRigJumpReleased() = 0;
	virtual void MotionRigDivePressed() = 0;
	virtual void MotionRigZoomCamera(float AxisValue) = 0;
};
