# Section 8 - Steam Deck Readiness

## Linux Platform Support

No project `Build.cs` or target file contains:

- Linux `SupportedPlatforms`
- `PLATFORM_LINUX`
- Linux-specific module logic

Config does include Linux RHI target:

- `SF_VULKAN_SM6`

No Linux build artifact was found under:

- `Binaries`
- `Intermediate`
- `Saved`

## Steam and OnlineSubsystem

Steam is enabled in:

- `T66.uproject`
- `Config/DefaultEngine.ini`
- `T66.Build.cs` dependencies
- `T66Mini.Build.cs` dependencies

Steam SDK from engine:

- Steamworks SDK version `1.61`
- Engine module has Win64, Mac, and Linux library paths.

Major Linux mismatch:

- `OnlineSubsystemSteam` supports Win64, Mac, Linux.
- `SocketSubsystemSteamIP` supports only Win64 and Mac in UE 5.7 plugin descriptor.
- T66 config selects `SocketSubsystemSteamIP.SteamNetDriver`.

This is the clearest Steam Deck/Linux blocker candidate.

## Runtime Platform Layer

Steam Deck detection exists in `UT66RuntimePlatformSubsystem`.

Detection inputs:

- `ISteamUtils::IsSteamRunningOnSteamDeck()` when Steamworks is available.
- `-T66SteamDeck`
- `-T66Desktop`
- `-T66RuntimePlatform=SteamDeck`

Config-backed Deck defaults:

- UI scale `1.1`
- scalability level `1`
- frame cap `60`
- Media Viewer disabled on Deck

Important gap:

- Media Viewer gating is wired clearly.
- Automatic application of Deck UI scale/scalability/FPS cap is not clearly wired from static call-site checks.

## Windows-Only Paths and APIs

No hardcoded `C:\` paths found in C++ source/config.

Windows-only runtime code:

- WebView2 code is guarded behind `PLATFORM_WINDOWS && T66_WITH_WEBVIEW2`.
- `T66_WITH_WEBVIEW2=0` on non-Windows.

Windows-extension-biased logic:

- `T66RunIntegritySubsystem` suspicious/module scans focus on `.dll` and `.exe`, not `.so` or `.dylib`.

Script hardcoded Windows defaults:

- `Scripts/CaptureT66UIScreen.ps1`
- `Scripts/CaptureT66UIWidget.ps1`
- `Scripts/StageStandaloneBuild.ps1`

Path construction concerns:

- Some dump/capture automation paths use `FPaths::ConvertRelativePathToFull` without an obvious confinement check.
- Some runtime loose-content paths use slash string concatenation instead of `FPaths::Combine`.

## Case Sensitivity

Feasible `/Game/...` literal case scan over `Source`, `Config`, and `Scripts` found no runtime Source/Config case mismatch.

One script-side ambiguity:

- `Scripts/ImportGameplayHUDArt.py` targets `/Game/UI/Sprites/UI/Hearts`.
- `Content/UI/Sprites/UI` also contains `HEARTS.uasset`.

Linux filesystem proof still requires cook/package validation.

## Input and Steam Input

Current state:

- Enhanced Input classes are enabled.
- Core keyboard/mouse mappings exist.
- Core gamepad mappings exist for movement, look, jump, pause, interact, ultimate, map, inventory, roll, HUD/media toggles, and triggers.
- Gameplay source still primarily uses legacy input bindings, with Enhanced Input used in selected paths.

Not found:

- Steam Input action manifest.
- `SteamInput`
- `ISteamInput`
- `SteamController`
- `InputActionManifest`

Deck risk:

- Gamepad mappings exist, but Steam Input integration and controller-only UI focus are not complete.

## UI Scale at 1280x800

Not proven.

Positive signs:

- UI docs include 1280x720 validation.
- Responsive helpers have compact/stacked thresholds.
- Steam Deck runtime profile exists.

Risks:

- Many screens still use 1920x1080 reference roots.
- No current 1280x800 capture gate was found.
- Existing pending UI issue notes missing central controller focus contract.

## Vulkan Support

Config targets Vulkan SM6 on Linux.

No obvious DX-only shader intrinsics found in source/custom shader files.

ToonStyle shader code uses `ddx`/`ddy`, which is not DX-specific.

Unproven:

- Vulkan shader cook.
- Steam Deck runtime launch.
- Niagara/material compatibility under Vulkan.

