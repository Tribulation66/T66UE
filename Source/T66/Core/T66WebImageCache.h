// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66WebImageCache.generated.h"

class UTexture2D;

/**
 * Downloads and caches web images (e.g. Steam avatars) as UTexture2D.
 * Uses FHttpModule for async downloads and IImageWrapper for decoding.
 */
UCLASS()
class T66_API UT66WebImageCache : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Returns a cached texture for the URL, or nullptr if not yet downloaded. */
	UTexture2D* GetCachedImage(const FString& Url) const;

	/** True if the URL is already cached (loaded). */
	bool HasCachedImage(const FString& Url) const;

	/**
	 * Request an image download. If already cached, calls OnReady immediately.
	 * If a download is in-flight, piggybacks on it.
	 * OnReady is called on the game thread with the texture (or nullptr on failure).
	 */
	void RequestImage(const FString& Url, TFunction<void(UTexture2D*)> OnReady);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWebImageReady, const FString& /*Url*/, UTexture2D* /*Texture*/);
	FOnWebImageReady OnWebImageReady;

private:
	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<UTexture2D>> CachedTextures;

	TSet<FString> PendingDownloads;

	TMap<FString, TArray<TFunction<void(UTexture2D*)>>> Waiters;

	void OnDownloadComplete(const FString& Url, const TArray<uint8>& Data, bool bSuccess);
	UTexture2D* CreateTextureFromData(const TArray<uint8>& Data, const FString& Url);
};
