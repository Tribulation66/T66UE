// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * Small visual helpers used across gameplay actors.
 * Centralizes common logic and avoids repeated sync loads + duplicated code.
 */
struct FT66VisualUtil
{
	static UMaterialInterface* GetPlaceholderColorMaterial();

	static void ApplyT66Color(UStaticMeshComponent* Mesh, UObject* Outer, const FLinearColor& Color);

	// Engine basic shapes (cached).
	static UStaticMesh* GetBasicShapeCube();
	static UStaticMesh* GetBasicShapeSphere();
	static UStaticMesh* GetBasicShapeCylinder();
	static UStaticMesh* GetBasicShapeCone();
};

