// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/SoftObjectPath.h"

#include "T66UITexturePoolSubsystem.generated.h"

class UTexture2D;
struct FStreamableHandle;

/**
 * Central owner for UTexture2D resources used by Slate brushes.
 *
 * Why:
 * - `FSlateBrush::SetResourceObject(UObject*)` does NOT keep UObjects alive.
 * - If UI loads textures via soft refs and only stores them in Slate brushes, GC can collect them,
 *   leading to Slate paint-time crashes (e.g. `Attempted to access resource ... pending kill`).
 *
 * This subsystem:
 * - Async-loads textures from soft references
 * - Keeps strong UPROPERTY refs so GC cannot collect while in use
 * - Deduplicates requests and fans out completion to multiple requesters safely (weak UObject gating)
 */
UCLASS()
class T66_API UT66UITexturePoolSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	/** Returns a loaded texture if currently resident; never triggers a load. */
	UTexture2D* GetLoadedTexture(const TSoftObjectPtr<UTexture2D>& Soft) const;

	/** Requests an async load; invokes callback once loaded (or with nullptr on failure). */
	void RequestTexture(
		const TSoftObjectPtr<UTexture2D>& Soft,
		UObject* Requester,
		TFunction<void(UTexture2D* Loaded)> OnReady);

	/**
	 * Requests an async load with a stable request key.
	 * If the requester later rebinds the same key to a different texture path, stale completions are ignored.
	 *
	 * Typical usage: per-slot UI icons (inventory slot 0..N), hero carousel slots, etc.
	 */
	void RequestTexture(
		const TSoftObjectPtr<UTexture2D>& Soft,
		UObject* Requester,
		FName RequestKey,
		TFunction<void(UTexture2D* Loaded)> OnReady);

	/** Clear all cached textures (use sparingly; mostly for debugging). */
	void ClearAll();

private:
	struct FWaiter
	{
		bool bHasRequester = false;
		TWeakObjectPtr<UObject> Requester;
		FName RequestKey = NAME_None;
		FSoftObjectPath ExpectedPath;
		TFunction<void(UTexture2D*)> Callback;
	};

	// Strong refs so GC cannot collect while cached.
	UPROPERTY(Transient)
	TMap<FSoftObjectPath, TObjectPtr<UTexture2D>> LoadedTextures;

	// In-flight async loads (kept until completion).
	TMap<FSoftObjectPath, TSharedPtr<FStreamableHandle>> ActiveLoads;

	// Waiters for a given asset path.
	TMap<FSoftObjectPath, TArray<FWaiter>> WaitersByPath;

	// Tracks latest requested path for (Requester, Key) so stale async completions can be ignored.
	TMap<TWeakObjectPtr<UObject>, TMap<FName, FSoftObjectPath>> LatestRequestedPathByKey;

	void HandleLoaded(const FSoftObjectPath& Path);
};

