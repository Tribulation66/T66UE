// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "T66BounceCourseAuditCommandlet.generated.h"

/**
 * Headless audit for the bouncy obstacle-course layer: builds tower layouts across
 * many seeds and difficulties and asserts the dry platform-chain guarantee on every
 * gameplay floor (chain gaps within jump range, Tier 2 tops above the lava cap,
 * descent hole reachable).
 *
 * Usage: -run=T66BounceCourseAudit [-seeds=50]
 * Emits [T66Proof][BounceCourseAudit] summary lines and returns non-zero on FAIL.
 */
UCLASS()
class T66_API UT66BounceCourseAuditCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UT66BounceCourseAuditCommandlet();

	virtual int32 Main(const FString& Params) override;
};
