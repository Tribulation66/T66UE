// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FT66MapPreset;

/** A single platform (cube) in the procedural map graph. */
struct FT66PlatformNode
{
	FVector2D Position = FVector2D::ZeroVector;
	float TopZ = 0.f;
	float SizeX = 0.f;
	float SizeY = 0.f;
	int32 GridRow = 0;
	int32 GridCol = 0;
	TArray<int32> Connections;
};

/** A ramp (wedge) connecting two platforms. */
struct FT66RampEdge
{
	int32 LowerIndex = -1;
	int32 HigherIndex = -1;
	float Width = 0.f;
	float Depth = 0.f;
	float PerpOffset = 0.f;

	/** True when this is a cardinal-X connection (horizontal); false for cardinal-Y (vertical). */
	bool bAlongX = true;
};

/** Output of the procedural map generator. */
struct FT66ProceduralMapResult
{
	TArray<FT66PlatformNode> Platforms;
	TArray<FT66RampEdge> Ramps;
	bool bValid = false;
};

/**
 * Grid-based procedural map generator.
 * Produces platforms on an NxN grid with seeded elevations and ramp connections.
 * Guarantees full graph connectivity (every platform reachable from the start area).
 */
class T66_API FT66ProceduralMapGenerator
{
public:
	static FT66ProceduralMapResult Generate(const FT66MapPreset& Preset);
};
