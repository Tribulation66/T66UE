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
class UMeshComponent;
class UWorld;

struct FT66VisualUtil
{
	static UMaterialInterface* GetPlaceholderColorMaterial();

	static void ApplyT66Color(UStaticMeshComponent* Mesh, UObject* Outer, const FLinearColor& Color);

	/** Replace any Lit engine material on a mesh component with an Unlit placeholder. */
	static void EnsureUnlitMaterials(UMeshComponent* Mesh, UObject* Outer);

	/** Iterate every mesh component in the world and replace Lit engine materials with Unlit. */
	static void EnsureAllWorldMeshesUnlit(UWorld* World);

	// Engine basic shapes (cached).
	static UStaticMesh* GetBasicShapeCube();
	static UStaticMesh* GetBasicShapeSphere();
	static UStaticMesh* GetBasicShapeCylinder();
	static UStaticMesh* GetBasicShapeCone();
};

