# Section 9 - macOS Readiness

## Mac Platform Support

No project `Build.cs` or target file contains:

- Mac `SupportedPlatforms`
- `PLATFORM_MAC`
- `PLATFORM_APPLE`
- Mac-specific module logic

Config does include Mac RHI target:

- `SF_METAL_SM6`

No Mac build artifact was found under:

- `Binaries`
- `Intermediate`
- `Saved`

## Metal RHI Considerations

No obvious source usage found for:

- DX12-only intrinsics.
- wave intrinsics.
- mesh shaders.
- hardware ray tracing dependency.

Renderer config disables hardware ray tracing/path tracing style risks:

- Lumen hardware ray tracing disabled.
- Ray tracing disabled.
- Ray-traced shadows disabled.

Remaining unproven areas:

- Metal SM6 cook.
- ToonStyle custom material nodes.
- Niagara systems from third-party packs.
- Electra/media playback behavior.
- Steam overlay/OSS behavior on Mac.

## Enabled Plugin Platform Support

Engine plugin descriptors for enabled plugins:

- `ModelingToolsEditorMode`
  - editor module.
  - Project entry target-limited to Editor.

- `PythonScriptPlugin`
  - `PythonScriptPluginPreload` runtime.
  - `PythonScriptPlugin` uncooked-only.
  - Project entry target-limited to Editor.

- `EditorScriptingUtilities`
  - editor module.
  - Project entry target-limited to Editor.

- `OnlineSubsystemSteam`
  - runtime module.
  - `PlatformAllowList=Win64,Mac,Linux`.

- `SocketSubsystemSteamIP`
  - runtime no-commandlet module.
  - `PlatformAllowList=Win64,Mac`.
  - Mac supported, Linux not supported.

- `ProceduralMeshComponent`
  - runtime module.
  - no platform allow list seen.

- `AnimToTexture`
  - runtime plus editor module.
  - no platform allow list seen.

- `ElectraPlayer`
  - core runtime modules allow Win64, Mac, Linux.
  - `ElectraProtron` modules allow Win64 and Mac.
  - transitive D3D12 decoder plugin is Win64-only, but that is expected for D3D12 video decoding.

## Windows-Only Libraries

`T66.Build.cs` links Windows-only libraries only in the Win64 block:

- `user32.lib`
- `ole32.lib`
- `shlwapi.lib`
- `WebView2Loader.dll`

Non-Windows builds get `T66_WITH_WEBVIEW2=0`.

Steamworks engine module includes Mac linkage for:

- `libsteam_api.dylib`

## macOS Readiness Summary

Mac has fewer obvious blockers than Steam Deck/Linux because `OnlineSubsystemSteam`, `SocketSubsystemSteamIP`, and Electra core paths advertise Mac support.

Still unproven:

- Mac cook/build.
- Metal SM6 shader compatibility.
- runtime media behavior.
- filesystem case behavior.
- controller/UI flow.
- Steam overlay/OSS behavior.

