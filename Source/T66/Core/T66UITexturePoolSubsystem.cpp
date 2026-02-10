// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66UITexturePoolSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture2D.h"

void UT66UITexturePoolSubsystem::Deinitialize()
{
	// Cancel any in-flight loads (best-effort) and drop waiters so nothing calls back after teardown.
	for (TPair<FSoftObjectPath, TSharedPtr<FStreamableHandle>>& Pair : ActiveLoads)
	{
		if (Pair.Value.IsValid())
		{
			Pair.Value->CancelHandle();
		}
	}
	ActiveLoads.Reset();
	WaitersByPath.Reset();
	LatestRequestedPathByKey.Reset();
	LoadedTextures.Reset();

	Super::Deinitialize();
}

UTexture2D* UT66UITexturePoolSubsystem::GetLoadedTexture(const TSoftObjectPtr<UTexture2D>& Soft) const
{
	if (Soft.IsNull())
	{
		return nullptr;
	}

	// First: try direct pointer (already loaded).
	if (UTexture2D* Direct = Soft.Get())
	{
		return Direct;
	}

	// Second: cached path.
	const FSoftObjectPath Path = Soft.ToSoftObjectPath();
	if (!Path.IsValid())
	{
		return nullptr;
	}

	if (const TObjectPtr<UTexture2D>* Found = LoadedTextures.Find(Path))
	{
		return Found ? Found->Get() : nullptr;
	}

	return nullptr;
}

void UT66UITexturePoolSubsystem::RequestTexture(
	const TSoftObjectPtr<UTexture2D>& Soft,
	UObject* Requester,
	TFunction<void(UTexture2D* Loaded)> OnReady)
{
	RequestTexture(Soft, Requester, NAME_None, MoveTemp(OnReady));
}

void UT66UITexturePoolSubsystem::RequestTexture(
	const TSoftObjectPtr<UTexture2D>& Soft,
	UObject* Requester,
	FName RequestKey,
	TFunction<void(UTexture2D* Loaded)> OnReady)
{
	if (!OnReady)
	{
		return;
	}

	if (Soft.IsNull())
	{
		OnReady(nullptr);
		return;
	}

	// If already loaded/cached, fulfill immediately.
	if (UTexture2D* Loaded = GetLoadedTexture(Soft))
	{
		OnReady(Loaded);
		return;
	}

	const FSoftObjectPath Path = Soft.ToSoftObjectPath();
	if (!Path.IsValid())
	{
		OnReady(nullptr);
		return;
	}

	// Record latest desired path for this key (used to suppress stale completions).
	if (Requester && !RequestKey.IsNone())
	{
		LatestRequestedPathByKey.FindOrAdd(Requester).Add(RequestKey, Path);
	}

	// Register waiter (gated by weak UObject).
	{
		// De-dupe: if the same requester asks for the same texture repeatedly while in-flight,
		// only keep the first callback to avoid unbounded waiter growth.
		TArray<FWaiter>& Waiters = WaitersByPath.FindOrAdd(Path);
		const bool bAlreadyWaiting = Requester && Waiters.ContainsByPredicate([&](const FWaiter& Existing)
		{
			return Existing.bHasRequester
				&& Existing.Requester.Get() == Requester
				&& Existing.RequestKey == RequestKey;
		});

		if (!bAlreadyWaiting)
		{
			FWaiter W;
			W.bHasRequester = (Requester != nullptr);
			W.Requester = Requester;
			W.RequestKey = RequestKey;
			W.ExpectedPath = Path;
			W.Callback = MoveTemp(OnReady);
			Waiters.Add(MoveTemp(W));
		}
	}

	// If already in-flight, we're done.
	if (ActiveLoads.Contains(Path))
	{
		return;
	}

	// Kick async load.
	TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		Path,
		FStreamableDelegate::CreateWeakLambda(this, [this, Path]()
		{
			HandleLoaded(Path);
		}));

	if (Handle.IsValid())
	{
		ActiveLoads.Add(Path, Handle);
	}
	else
	{
		// Failed to start load; complete waiters with null.
		HandleLoaded(Path);
	}
}

void UT66UITexturePoolSubsystem::ClearAll()
{
	LoadedTextures.Reset();
}

void UT66UITexturePoolSubsystem::EnsureTexturesLoadedSync(const TArray<FSoftObjectPath>& Paths)
{
	for (const FSoftObjectPath& Path : Paths)
	{
		if (!Path.IsValid() || LoadedTextures.Contains(Path))
		{
			continue;
		}
		TSoftObjectPtr<UTexture2D> Soft(Path);
		if (UTexture2D* Tex = Soft.LoadSynchronous())
		{
			LoadedTextures.Add(Path, Tex);
		}
	}
}

void UT66UITexturePoolSubsystem::HandleLoaded(const FSoftObjectPath& Path)
{
	// Drop handle (if any).
	ActiveLoads.Remove(Path);

	// Resolve loaded object (may still be null on failure).
	UTexture2D* Tex = nullptr;
	if (UObject* Obj = Path.ResolveObject())
	{
		Tex = Cast<UTexture2D>(Obj);
	}
	if (!Tex)
	{
		// As a fallback, try soft ptr Get() via path (should be loaded if async succeeded).
		TSoftObjectPtr<UTexture2D> Soft(Path);
		Tex = Soft.Get();
	}

	if (Tex)
	{
		LoadedTextures.Add(Path, Tex);
	}

	// Fan out to waiters (gated).
	TArray<FWaiter> Waiters;
	if (TArray<FWaiter>* Found = WaitersByPath.Find(Path))
	{
		Waiters = MoveTemp(*Found);
		WaitersByPath.Remove(Path);
	}

	for (FWaiter& W : Waiters)
	{
		// If no requester was provided, this callback is intentionally ungated.
		if (!W.bHasRequester || W.Requester.IsValid())
		{
			// If a key was provided, suppress stale completions (e.g. brush rebound to a different icon before load completes).
			if (W.bHasRequester && !W.RequestKey.IsNone())
			{
				if (TMap<FName, FSoftObjectPath>* PerKey = LatestRequestedPathByKey.Find(W.Requester))
				{
					if (const FSoftObjectPath* LatestPath = PerKey->Find(W.RequestKey))
					{
						if (*LatestPath != W.ExpectedPath)
						{
							continue;
						}
					}
				}
			}
			if (W.Callback)
			{
				W.Callback(Tex);
			}
		}
	}
}

