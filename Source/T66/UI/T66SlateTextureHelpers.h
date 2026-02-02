// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UT66UITexturePoolSubsystem;
class UTexture2D;
struct FSlateBrush;

/**
 * Helpers for safely binding soft-referenced textures to Slate brushes.
 *
 * Conventions:
 * - UI code must not call `LoadSynchronous()` for textures in gameplay UI.
 * - UI code must not rely on `FSlateBrush::SetResourceObject()` to keep textures alive.
 * - Always request via `UT66UITexturePoolSubsystem` (central owner), then set brush resource on completion.
 */
namespace T66SlateTexture
{
	/** Return currently loaded texture, or nullptr (never loads). */
	UTexture2D* GetLoaded(const UT66UITexturePoolSubsystem* Pool, const TSoftObjectPtr<UTexture2D>& Soft);

	/**
	 * Bind a shared brush to a soft texture:
	 * - If loaded: set immediately.
	 * - If not loaded: clear resource (optional) and request async load; on completion, set resource.
	 *
	 * `Requester` gates the callback (no stale calls after widget destruction).
	 */
	void BindSharedBrushAsync(
		UT66UITexturePoolSubsystem* Pool,
		const TSoftObjectPtr<UTexture2D>& Soft,
		UObject* Requester,
		const TSharedPtr<FSlateBrush>& Brush,
		FName RequestKey = NAME_None,
		bool bClearWhileLoading = true);

	/** Same as above, but for a stack brush (e.g. `FSlateBrush` member). */
	void BindBrushAsync(
		UT66UITexturePoolSubsystem* Pool,
		const TSoftObjectPtr<UTexture2D>& Soft,
		UObject* Requester,
		FSlateBrush& Brush,
		FName RequestKey = NAME_None,
		bool bClearWhileLoading = true);
}

