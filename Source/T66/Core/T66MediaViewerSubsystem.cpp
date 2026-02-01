// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MediaViewerSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"

#include "Misc/App.h"

#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
#include "Core/T66WebView2Host.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/GenericWindow.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "Windows/WindowsHWrapper.h"
#include <objbase.h>
#include "Windows/HideWindowsPlatformTypes.h"

// Prevent Win32 macros from leaking into unity TUs.
#if defined(PlaySound)
#undef PlaySound
#endif
#if defined(UpdateResource)
#undef UpdateResource
#endif
#if defined(max)
#undef max
#endif
#if defined(min)
#undef min
#endif
#endif

void UT66MediaViewerSubsystem::ToggleMediaViewer()
{
	SetMediaViewerOpen(!bIsOpen);
}

void UT66MediaViewerSubsystem::PrewarmTikTok()
{
#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
	if (bHasPrewarmedTikTok)
	{
		return;
	}

	if (!TikTokWebView2)
	{
		TikTokWebView2 = MakeUnique<FT66WebView2Host>();
	}

	// Ensure COM is initialized for WebView2 (best effort).
	const HRESULT HrCo = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	if (FAILED(HrCo) && HrCo != RPC_E_CHANGED_MODE)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TIKTOK][WEBVIEW2] CoInitializeEx failed (0x%08x)."), static_cast<uint32>(HrCo));
	}

	void* ParentHandle = nullptr;
	if (FSlateApplication::IsInitialized())
	{
		ParentHandle = const_cast<void*>(FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr));

		TSharedPtr<SWindow> Active = FSlateApplication::Get().GetActiveTopLevelWindow();
		if (Active.IsValid() && Active->GetNativeWindow().IsValid())
		{
			ParentHandle = Active->GetNativeWindow()->GetOSWindowHandle();
		}
	}

	// Persistent web profile so the user stays logged in.
	// HARD RULE: keep WebView2 data inside the game folder (not user profile/AppData).
	const FString RootDir = GIsEditor
		? FPaths::ProjectDir()
		: FPaths::ConvertRelativePathToFull(FPaths::Combine(FPlatformProcess::BaseDir(), TEXT(".."), TEXT("..")));
	const FString UserData = FPaths::ConvertRelativePathToFull(
		FPaths::Combine(RootDir, TEXT("Saved"), TEXT("TikTokWebView2"), TEXT("UserData")));
	IFileManager::Get().MakeDirectory(*UserData, /*Tree*/true);

	if (TikTokWebView2->EnsureCreated(ParentHandle, UserData))
	{
		bHasPrewarmedTikTok = true;

		// Preload TikTok so session + CSS injection are ready before first toggle.
		if (!TikTokWebView2->HasEverNavigated())
		{
			TikTokWebView2->Navigate(TEXT("https://www.tiktok.com/"));
		}

		// Keep hidden (and hard-muted) until the player opens it.
		TikTokWebView2->Hide();
	}
#endif
}

void UT66MediaViewerSubsystem::SetMediaViewerOpen(bool bOpen)
{
	if (bIsOpen == bOpen) return;
	bIsOpen = bOpen;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			// Capture current player settings at open time so we can restore exactly on close.
			if (bIsOpen)
			{
				PrevMasterVolume01 = PS->GetMasterVolume();
				bPrevMuteWhenUnfocused = PS->GetMuteWhenUnfocused();
				PrevMusicVolume01 = PS->GetMusicVolume();
				PrevSfxVolume01 = PS->GetSfxVolume();
			}
		}
	}

	if (bIsOpen)
	{
		ApplyMutedAudio();
	}
	else
	{
		RestoreAudio();
	}

#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
	// Windows-only: show/hide a real WebView2 panel for TikTok playback (H.264/AAC supported by Edge runtime).
	if (bIsOpen)
	{
		if (!TikTokWebView2)
		{
			TikTokWebView2 = MakeUnique<FT66WebView2Host>();
		}

		// Ensure COM is initialized for WebView2 (best effort).
		const HRESULT HrCo = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
		if (FAILED(HrCo) && HrCo != RPC_E_CHANGED_MODE)
		{
			UE_LOG(LogTemp, Warning, TEXT("[TIKTOK][WEBVIEW2] CoInitializeEx failed (0x%08x)."), static_cast<uint32>(HrCo));
		}

		void* ParentHandle = nullptr;
		if (FSlateApplication::IsInitialized())
		{
			// Best-effort: prefer "best parent for dialogs" (covers packaged + editor),
			// then fall back to active top-level if needed.
			ParentHandle = const_cast<void*>(FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr));

			TSharedPtr<SWindow> Active = FSlateApplication::Get().GetActiveTopLevelWindow();
			if (Active.IsValid() && Active->GetNativeWindow().IsValid())
			{
				ParentHandle = Active->GetNativeWindow()->GetOSWindowHandle();
			}
		}
		UE_LOG(LogTemp, Log, TEXT("[TIKTOK][WEBVIEW2] EnsureCreated parent hwnd=0x%p"), ParentHandle);

		// Persistent web profile so the user stays logged in.
		// HARD RULE: keep WebView2 data inside the game folder (not user profile/AppData).
		// Editor: ProjectDir; Packaged: <GameRoot>/Saved/...
		const FString RootDir = GIsEditor
			? FPaths::ProjectDir()
			: FPaths::ConvertRelativePathToFull(FPaths::Combine(FPlatformProcess::BaseDir(), TEXT(".."), TEXT("..")));
		const FString UserData = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(RootDir, TEXT("Saved"), TEXT("TikTokWebView2"), TEXT("UserData")));
		IFileManager::Get().MakeDirectory(*UserData, /*Tree*/true);

		if (TikTokWebView2->EnsureCreated(ParentHandle, UserData))
		{
			// If HUD has already provided an anchored rect, use it. Otherwise keep hidden until it does.
			if (bHasTikTokWebView2Rect)
			{
				TikTokWebView2->ShowAtScreenRect(TikTokWebView2Rect);
			}

			// Only navigate on the first ever open. Subsequent toggles should simply show/hide the existing session
			// (so we don't force a relogin mid-run).
			if (!TikTokWebView2->HasEverNavigated())
			{
				TikTokWebView2->Navigate(TEXT("https://www.tiktok.com/"));
			}
		}
	}
	else
	{
		if (TikTokWebView2)
		{
			TikTokWebView2->Hide();
		}
	}
#endif

	OnMediaViewerOpenChanged.Broadcast(bIsOpen);
}

#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
void UT66MediaViewerSubsystem::SetTikTokWebView2ScreenRect(const FIntRect& ScreenRectPx)
{
	// Avoid spamming MoveWindow every tick when nothing moved.
	if (bHasTikTokWebView2Rect && TikTokWebView2Rect == ScreenRectPx)
	{
		return;
	}
	TikTokWebView2Rect = ScreenRectPx;
	bHasTikTokWebView2Rect = true;

	if (bIsOpen && TikTokWebView2)
	{
		TikTokWebView2->ShowAtScreenRect(ScreenRectPx);
	}
}

void UT66MediaViewerSubsystem::TikTokPrev()
{
	if (!bIsOpen || !TikTokWebView2) return;
	TikTokWebView2->PrevVideo();
}

void UT66MediaViewerSubsystem::TikTokNext()
{
	if (!bIsOpen || !TikTokWebView2) return;
	TikTokWebView2->NextVideo();
}
#else
void UT66MediaViewerSubsystem::SetTikTokWebView2ScreenRect(const FIntRect& /*ScreenRectPx*/)
{
}

void UT66MediaViewerSubsystem::TikTokPrev()
{
}

void UT66MediaViewerSubsystem::TikTokNext()
{
}
#endif

void UT66MediaViewerSubsystem::ApplyMutedAudio()
{
	// Mute all game audio while viewer is open.
	FApp::SetVolumeMultiplier(0.0f);

	// Also mute unfocused audio to avoid leaks during alt-tab.
	FApp::SetUnfocusedVolumeMultiplier(0.0f);
}

void UT66MediaViewerSubsystem::RestoreAudio()
{
	// Restore the exact values captured before opening.
	FApp::SetVolumeMultiplier(FMath::Clamp(PrevMasterVolume01, 0.0f, 1.0f));
	FApp::SetUnfocusedVolumeMultiplier(bPrevMuteWhenUnfocused ? 0.0f : 1.0f);

	// Re-apply class volumes if assets exist (optional foundation).
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->SetMusicVolume(PrevMusicVolume01);
			PS->SetSfxVolume(PrevSfxVolume01);
			PS->SetMasterVolume(PrevMasterVolume01);
			PS->SetMuteWhenUnfocused(bPrevMuteWhenUnfocused);
		}
	}
}

