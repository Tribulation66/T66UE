// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66MediaViewerSubsystem.generated.h"

/** Which short-form video platform the media viewer is showing. */
UENUM(BlueprintType)
enum class ET66MediaViewerSource : uint8
{
	TikTok  UMETA(DisplayName = "TikTok"),
	Shorts  UMETA(DisplayName = "Shorts"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnT66MediaViewerOpenChanged, bool, bIsOpen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnT66MediaViewerSourceChanged, ET66MediaViewerSource, NewSource);

#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
#include "Core/T66WebView2Host.h"
#endif

/**
 * Media Viewer for TikTok and YouTube Shorts.
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

	/** Get/set which platform is active (TikTok or Shorts). Navigates to the new URL immediately if open. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MediaViewer")
	ET66MediaViewerSource GetMediaViewerSource() const { return ActiveSource; }

	UFUNCTION(BlueprintCallable, Category = "MediaViewer")
	void SetMediaViewerSource(ET66MediaViewerSource NewSource);

	UPROPERTY(BlueprintAssignable, Category = "MediaViewer")
	FOnT66MediaViewerOpenChanged OnMediaViewerOpenChanged;

	UPROPERTY(BlueprintAssignable, Category = "MediaViewer")
	FOnT66MediaViewerSourceChanged OnMediaViewerSourceChanged;

	// Windows-only WebView2 integration (called from HUD to align the overlay with the video panel).
	void SetTikTokWebView2ScreenRect(const FIntRect& ScreenRectPx);

	// Video controls (prev/next). TikTok: scroll-based; YouTube Shorts: keyboard ArrowDown/ArrowUp.
	void TikTokPrev();
	void TikTokNext();

	// Pre-warm WebView2 in the background so login + CSS formatting are done before first toggle.
	// Safe to call multiple times.
	void PrewarmTikTok();

	/** Return the initial URL for a given source. */
	static const TCHAR* GetUrlForSource(ET66MediaViewerSource Source);

private:
	bool bIsOpen = false;
	ET66MediaViewerSource ActiveSource = ET66MediaViewerSource::TikTok;

#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
	TUniquePtr<FT66WebView2Host> TikTokWebView2;
	bool bHasTikTokWebView2Rect = false;
	FIntRect TikTokWebView2Rect;
	bool bHasPrewarmedTikTok = false;
#endif

	// Snapshot of audio state before opening the viewer (so we can restore exactly).
	float PrevMasterVolume01 = 1.0f;
	bool bPrevMuteWhenUnfocused = false;
	float PrevMusicVolume01 = 1.0f;
	float PrevSfxVolume01 = 1.0f;

	void ApplyMutedAudio();
	void RestoreAudio();
};

