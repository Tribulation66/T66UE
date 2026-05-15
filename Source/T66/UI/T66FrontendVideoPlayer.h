// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "UObject/Object.h"
#include "T66FrontendVideoPlayer.generated.h"

class UFileMediaSource;
class UMediaPlayer;
class UMediaTexture;
class UTexture2D;
struct FT66FrontendVideoAsset;

UCLASS(Transient)
class T66_API UT66FrontendVideoPlayer : public UObject
{
	GENERATED_BODY()

public:
	virtual void BeginDestroy() override;

	bool OpenVideo(const FT66FrontendVideoAsset& Asset, const FVector2D& ImageSize, FName DebugName);
	void CloseVideo();
	const FSlateBrush* GetVideoBrush() const;
	bool HasOpenVideo() const { return bHasOpenVideo; }

private:
	bool OpenPosterSurface(const FT66FrontendVideoAsset& Asset, const FVector2D& ImageSize, FName DebugName);

	UFUNCTION()
	void HandleMediaOpened(FString OpenedUrl);

	UFUNCTION()
	void HandleMediaOpenFailed(FString FailedUrl);

	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> MediaPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UMediaTexture> MediaTexture;

	UPROPERTY(Transient)
	TObjectPtr<UFileMediaSource> FileMediaSource;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> PosterTexture;

	TSharedPtr<FSlateBrush> VideoBrush;
	FString CurrentMoviePath;
	FString CurrentPosterPath;
	FName CurrentDebugName = NAME_None;
	bool bHasOpenVideo = false;
	bool bUsingPosterSurface = false;
};
