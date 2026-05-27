// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UWorld;

namespace T66TestRoom
{
	FName RoomActorTag();
	FName RoomSurfaceTag();
	FName LightingActorTag();
	FName LineupActorTag();
	FName AtmosphereSparedTag();
	FVector PlayerStartLocation();
	void SpawnRoom(UWorld* World);
	void SpawnLighting(UWorld* World);
	void ScheduleCombatZones(UWorld* World);
}
