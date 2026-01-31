// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MediaViewerSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"

#include "Misc/App.h"

void UT66MediaViewerSubsystem::ToggleMediaViewer()
{
	SetMediaViewerOpen(!bIsOpen);
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

	OnMediaViewerOpenChanged.Broadcast(bIsOpen);
}

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

