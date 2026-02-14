// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2

/**
 * Minimal WebView2 host for Windows-only short-form video playback (TikTok, Instagram Reels, YouTube Shorts).
 *
 * Notes:
 * - Requires WebView2 Runtime installed (Edge WebView2 runtime).
 * - Requires WebView2Loader.dll shipped next to the exe (we stage it via Build.cs runtime dependency).
 * - Uses a persistent UserData folder under Saved/ so the user stays logged in.
 */
class FT66WebView2Host
{
public:
	FT66WebView2Host();
	~FT66WebView2Host();

	// Non-copyable.
	FT66WebView2Host(const FT66WebView2Host&) = delete;
	FT66WebView2Host& operator=(const FT66WebView2Host&) = delete;

	/** Ensure underlying WebView2 is created. `ParentWindowHandle` should be the Win32 HWND (passed as void* to keep headers clean). */
	bool EnsureCreated(void* ParentWindowHandle, const FString& UserDataFolder);

	/** Show/position the webview in parent client coordinates (pixels). */
	void ShowAtRect(const FIntRect& ClientRectPx);

	/** Show/position the webview using desktop screen coordinates (pixels). */
	void ShowAtScreenRect(const FIntRect& ScreenRectPx);

	/** Hide the webview window (keeps session). */
	void Hide();

	/** Navigate to a URL (TikTok-only allowlist enforced). */
	void Navigate(const FString& Url);

	/** TikTok-only controls (no arbitrary browsing). */
	void PrevVideo();
	void NextVideo();

	/** True once we've navigated to any URL. */
	bool HasEverNavigated() const;

	/** True if controller is ready. */
	bool IsReady() const;

private:
	struct FImpl;
	TUniquePtr<FImpl> Impl;
};

#endif // PLATFORM_WINDOWS && T66_WITH_WEBVIEW2

