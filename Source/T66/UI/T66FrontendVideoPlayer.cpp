// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66FrontendVideoPlayer.h"

#include "Blueprint/UserWidget.h"
#include "Components/ActorComponent.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Styling/SlateBrush.h"
#include "FileMediaSource.h"
#include "HAL/FileManager.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UI/T66FrontendVideoCatalog.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66FrontendVideo, Log, All);

namespace
{
	UWorld* ResolveFrontendVideoWorld(const UObject* Object)
	{
		if (!Object)
		{
			return nullptr;
		}

		for (const UObject* Outer = Object; Outer; Outer = Outer->GetOuter())
		{
			if (const UWorld* World = Cast<UWorld>(Outer))
			{
				return const_cast<UWorld*>(World);
			}
			if (const AActor* Actor = Cast<AActor>(Outer))
			{
				return Actor->GetWorld();
			}
			if (const UActorComponent* Component = Cast<UActorComponent>(Outer))
			{
				return Component->GetWorld();
			}
			if (const UUserWidget* UserWidget = Cast<UUserWidget>(Outer))
			{
				return UserWidget->GetWorld();
			}
			if (const UGameInstance* GameInstance = Cast<UGameInstance>(Outer))
			{
				return GameInstance->GetWorld();
			}
		}

		return GEngine && GEngine->GameViewport
			? GEngine->GameViewport->GetWorld()
			: nullptr;
	}

	bool ShouldUsePosterSurfaceForRetainedCRT(const UObject* Object)
	{
		if (FParse::Param(FCommandLine::Get(), TEXT("T66DisableFrontendCRT")))
		{
			return false;
		}

		if (const UWorld* World = ResolveFrontendVideoWorld(Object))
		{
			if (const UGameInstance* GameInstance = World->GetGameInstance())
			{
				if (const UT66PlayerSettingsSubsystem* PlayerSettings = GameInstance->GetSubsystem<UT66PlayerSettingsSubsystem>())
				{
					const FT66RetroFXSettings Settings = PlayerSettings->GetRetroFXSettings();
					return Settings.UIFullScreenCRTEnabled;
				}
			}
		}

		const FT66RetroFXSettings Defaults;
		return Defaults.UIFullScreenCRTEnabled;
	}

	UTexture2D* LoadFrontendPosterTexture(const FString& PosterPath, const FName DebugName)
	{
		if (PosterPath.IsEmpty())
		{
			return nullptr;
		}

		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(PosterPath))
		{
			if (IFileManager::Get().FileExists(*CandidatePath))
			{
				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
					CandidatePath,
					TextureFilter::TF_Trilinear,
					TEXT("FrontendVideoPoster")))
				{
					return Texture;
				}
			}
		}

		UE_LOG(LogT66FrontendVideo, Warning, TEXT("Frontend video '%s' missing poster '%s'"), *DebugName.ToString(), *PosterPath);
		return nullptr;
	}
}

bool UT66FrontendVideoPlayer::OpenVideo(const FT66FrontendVideoAsset& Asset, const FVector2D& ImageSize, const FName DebugName)
{
	const FString AbsoluteMoviePath = T66FrontendVideoCatalog::ResolveMovieAbsolutePath(Asset.MoviePath);
	CurrentDebugName = DebugName;

	if (ShouldUsePosterSurfaceForRetainedCRT(this))
	{
		return OpenPosterSurface(Asset, ImageSize, DebugName);
	}

	if (AbsoluteMoviePath.IsEmpty() || !IFileManager::Get().FileExists(*AbsoluteMoviePath))
	{
		UE_LOG(LogT66FrontendVideo, Warning, TEXT("Frontend video '%s' missing movie '%s'"), *DebugName.ToString(), *AbsoluteMoviePath);
		CloseVideo();
		return false;
	}

	if (CurrentMoviePath == AbsoluteMoviePath && bHasOpenVideo && VideoBrush.IsValid())
	{
		VideoBrush->ImageSize = ImageSize;
		return true;
	}

	CloseVideo();
	CurrentMoviePath = AbsoluteMoviePath;

	MediaPlayer = NewObject<UMediaPlayer>(this);
	MediaTexture = NewObject<UMediaTexture>(this);
	FileMediaSource = NewObject<UFileMediaSource>(this);
	if (!MediaPlayer || !MediaTexture || !FileMediaSource)
	{
		CloseVideo();
		return false;
	}

	MediaPlayer->SetLooping(true);
	MediaPlayer->OnMediaOpened.AddDynamic(this, &UT66FrontendVideoPlayer::HandleMediaOpened);
	MediaPlayer->OnMediaOpenFailed.AddDynamic(this, &UT66FrontendVideoPlayer::HandleMediaOpenFailed);

	MediaTexture->SetMediaPlayer(MediaPlayer);
	MediaTexture->UpdateResource();

	FileMediaSource->SetFilePath(AbsoluteMoviePath);

	VideoBrush = MakeShared<FSlateBrush>();
	VideoBrush->DrawAs = ESlateBrushDrawType::Image;
	VideoBrush->Tiling = ESlateBrushTileType::NoTile;
	VideoBrush->ImageSize = ImageSize;
	VideoBrush->SetResourceObject(MediaTexture);

	bHasOpenVideo = MediaPlayer->OpenSource(FileMediaSource);
	if (!bHasOpenVideo)
	{
		UE_LOG(LogT66FrontendVideo, Warning, TEXT("Frontend video '%s' failed to open '%s'"), *DebugName.ToString(), *AbsoluteMoviePath);
		CloseVideo();
		return false;
	}

	return true;
}

void UT66FrontendVideoPlayer::BeginDestroy()
{
	CloseVideo();
	Super::BeginDestroy();
}

void UT66FrontendVideoPlayer::CloseVideo()
{
	if (MediaPlayer)
	{
		MediaPlayer->OnMediaOpened.RemoveDynamic(this, &UT66FrontendVideoPlayer::HandleMediaOpened);
		MediaPlayer->OnMediaOpenFailed.RemoveDynamic(this, &UT66FrontendVideoPlayer::HandleMediaOpenFailed);
		MediaPlayer->Close();
	}

	MediaPlayer = nullptr;
	MediaTexture = nullptr;
	FileMediaSource = nullptr;
	if (PosterTexture && PosterTexture->IsRooted())
	{
		PosterTexture->RemoveFromRoot();
	}
	PosterTexture = nullptr;
	VideoBrush.Reset();
	CurrentMoviePath.Reset();
	CurrentPosterPath.Reset();
	bHasOpenVideo = false;
	bUsingPosterSurface = false;
}

const FSlateBrush* UT66FrontendVideoPlayer::GetVideoBrush() const
{
	return bHasOpenVideo && VideoBrush.IsValid()
		? VideoBrush.Get()
		: nullptr;
}

void UT66FrontendVideoPlayer::HandleMediaOpened(FString OpenedUrl)
{
	if (MediaPlayer)
	{
		MediaPlayer->Play();
	}
	UE_LOG(LogT66FrontendVideo, Log, TEXT("Frontend video '%s' opened '%s'"), *CurrentDebugName.ToString(), *OpenedUrl);
}

void UT66FrontendVideoPlayer::HandleMediaOpenFailed(FString FailedUrl)
{
	UE_LOG(LogT66FrontendVideo, Warning, TEXT("Frontend video '%s' failed media open '%s'"), *CurrentDebugName.ToString(), *FailedUrl);
	bHasOpenVideo = false;
}

bool UT66FrontendVideoPlayer::OpenPosterSurface(const FT66FrontendVideoAsset& Asset, const FVector2D& ImageSize, const FName DebugName)
{
	if (Asset.PosterPath.IsEmpty())
	{
		UE_LOG(LogT66FrontendVideo, Warning, TEXT("Frontend CRT video '%s' has no poster fallback; skipping retained media texture"), *DebugName.ToString());
		CloseVideo();
		return false;
	}

	if (bUsingPosterSurface && CurrentPosterPath == Asset.PosterPath && bHasOpenVideo && VideoBrush.IsValid())
	{
		VideoBrush->ImageSize = ImageSize;
		return true;
	}

	CloseVideo();
	CurrentDebugName = DebugName;
	CurrentPosterPath = Asset.PosterPath;
	PosterTexture = LoadFrontendPosterTexture(CurrentPosterPath, DebugName);
	if (!PosterTexture)
	{
		CloseVideo();
		return false;
	}
	PosterTexture->AddToRoot();

	VideoBrush = MakeShared<FSlateBrush>();
	VideoBrush->DrawAs = ESlateBrushDrawType::Image;
	VideoBrush->Tiling = ESlateBrushTileType::NoTile;
	VideoBrush->ImageSize = ImageSize;
	VideoBrush->SetResourceObject(PosterTexture);

	bHasOpenVideo = true;
	bUsingPosterSurface = true;
	UE_LOG(LogT66FrontendVideo, Log, TEXT("Frontend CRT video '%s' using poster surface '%s'"), *DebugName.ToString(), *CurrentPosterPath);
	return true;
}
