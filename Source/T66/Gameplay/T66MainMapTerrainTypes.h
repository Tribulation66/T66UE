// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

enum class ET66MapCellShape : uint8
{
	Flat,
	SlopePosX,
	SlopeNegX,
	SlopePosY,
	SlopeNegY,
};

enum class ET66MapCellDecoration : uint8
{
	None,
	Rock,
	Rocks,
	Log,
};

enum class ET66MapCellSurfaceFeature : uint8
{
	None,
	Hill,
	Crater,
};
