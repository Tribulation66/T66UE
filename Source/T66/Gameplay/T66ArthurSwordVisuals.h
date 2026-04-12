// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UStaticMesh;

namespace T66ArthurSwordVisuals
{
	T66_API const TCHAR* GetSwordMeshAssetPath();
	T66_API UStaticMesh* LoadSwordMesh();
}
