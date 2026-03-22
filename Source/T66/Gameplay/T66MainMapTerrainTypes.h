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
	Tree1,
	Tree2,
	Tree3,
	Rock,
	Rocks,
	Log,
};
