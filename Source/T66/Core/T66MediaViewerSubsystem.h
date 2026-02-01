// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66MediaViewerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnT66MediaViewerOpenChanged, bool, bIsOpen);

#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
#include "Core/T66WebView2Host.h"
#endif

/**
 * Foundation for the TikTok / YouTube Shorts Media Viewer feature.
 * Implements audio muting rule: while viewer is open, mute all other game audio and restore on close.
 */
UCLASS()
class T66_API UT66MediaViewerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MediaViewer")
	bool IsMediaViewerOpen() const { return bIsOpen; }

	UFUNCTION(BlueprintCallable, Category = "MediaViewer")
	void ToggleMediaViewer();

	UFUNCTION(BlueprintCallable, Category = "MediaViewer")
	void SetMediaViewerOpen(bool bOpen);

	UPROPERTY(BlueprintAssignable, Category = "MediaViewer")
	FOnT66MediaViewerOpenChanged OnMediaViewerOpenChanged;

	// Windows-only TikTok/WebView2 integration (called from HUD to align the overlay with the phone panel).
	void SetTikTokWebView2ScreenRect(const FIntRect& ScreenRectPx);

	// TikTok-only controls (no arbitrary browsing UI).
	void TikTokPrev();
	void TikTokNext();

private:
	bool bIsOpen = false;

#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
	TUniquePtr<FT66WebView2Host> TikTokWebView2;
	bool bHasTikTokWebView2Rect = false;
	FIntRect TikTokWebView2Rect;
#endif

	// Snapshot of audio state before opening the viewer (so we can restore exactly).
	float PrevMasterVolume01 = 1.0f;
	bool bPrevMuteWhenUnfocused = false;
	float PrevMusicVolume01 = 1.0f;
	float PrevSfxVolume01 = 1.0f;

	void ApplyMutedAudio();
	void RestoreAudio();
};

