// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UMaterialInterface;

/**
 * Helpers to ensure preview-stage materials exist.
 *
 * All materials use the SAME simple pattern: single VectorParameter → one output pin.
 * No multi-node chains — those fail silently in UE's programmatic material API.
 *
 * Editor fallback: if assets aren't found, creates + saves them via C++ (WITH_EDITORONLY_DATA).
 */
namespace T66PreviewMaterials
{
	/** Ground: VectorParameter "BaseColor" → BaseColor (DefaultLit). */
	UMaterialInterface* GetGroundMaterial();

	/** Sky dome: VectorParameter "SkyColor" → EmissiveColor (self-lit, black BaseColor). */
	UMaterialInterface* GetSkyMaterial();

	/** Star dots: VectorParameter "StarColor" → EmissiveColor (self-lit, black BaseColor). */
	UMaterialInterface* GetStarMaterial();
}
