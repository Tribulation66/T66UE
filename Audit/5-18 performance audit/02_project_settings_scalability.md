# Section 2 - Project Settings and Scalability

## Config Files Audited

- `Config/DefaultEngine.ini`
- `Config/DefaultGame.ini`
- `Config/DefaultScalability.ini`
- `Config/DefaultDeviceProfiles.ini`
- `Config/DefaultInput.ini`

The reports summarize performance/platform-relevant contents rather than duplicating the full files. The files above remain the source of truth.

## DefaultEngine.ini Highlights

Maps:

- `GameDefaultMap=/Game/Maps/FrontendLevel`
- `EditorStartupMap=/Game/Maps/FrontendLevel`
- `GlobalDefaultGameMode=None`
- `GameInstanceClass=/Game/Blueprints/Core/BP_T66GameInstance.BP_T66GameInstance_C`

Renderer:

- Virtual textures enabled.
- Dynamic GI disabled by project renderer setting.
- Reflection method set to `2`.
- Lumen hardware ray tracing disabled.
- Ray tracing disabled.
- Nanite enabled.
- Static lighting disabled.
- Bloom enabled.
- Ambient occlusion disabled.
- Auto exposure disabled.
- Motion blur disabled.
- Lens flare disabled.
- Anti-aliasing method `0`.
- Default screen percentage `100`.

Windows RHI:

- `DefaultGraphicsRHI=DefaultGraphicsRHI_DX12`
- D3D12 targeted shader format is SM6.
- D3D11 targeted shader format is SM5.

Linux RHI:

- Removes `SF_VULKAN_SM5`.
- Adds `SF_VULKAN_SM6`.

Mac RHI:

- Removes `SF_METAL_SM5`.
- Adds `SF_METAL_SM6`.

UI scale:

- `bAllowHighDPIInGameMode=True`
- `UIScaleRule=ShortestSide`
- `ApplicationScale=1`
- DPI curve includes values for 720, 800, 900, 1080, 1440, and 2160.

Steam:

- `DefaultPlatformService=Steam`
- Steam net driver configured as `SocketSubsystemSteamIP.SteamNetDriver`
- AppID and DevAppID both `4464300`
- `bInitServerOnClient=true`

## DefaultGame.ini Highlights

Project version:

- `ProjectVersion=alpha 0.6`

Packaging/cooking:

- Always-cook directories include game audio, characters, data, items, maps, materials, minigames, source assets, Stylized VFX, ToonStyle, UI, UE5RFX, VFX, World, and the engine sky template path.
- Maps to cook include Frontend and Gameplay levels.

Backend:

- Backend URL is `https://t66-backend.vercel.app`.

Runtime loose roots:

- Runtime dependency roots include loose arcade, fonts, UI, video, and minigame data/source asset folders.

Release variants:

- Full game app ID `4464300`.
- Demo app ID `4718770`.
- Demo hero/difficulty constraints are configured.

Steam Deck runtime settings:

- `bDetectSteamDeck=true`
- `bAllowMediaViewerOnSteamDeck=false`
- `SteamDeckDefaultUIScale=1.1`
- `SteamDeckDefaultScalabilityLevel=1`
- `SteamDeckDefaultFrameRateLimit=60`

Important gap: the Steam Deck default getters exist, but current static call-site checks show Media Viewer gating is wired more clearly than automatic application of UI scale, scalability, and FPS cap.

## DefaultScalability.ini Buckets

Scalability tiers 0-3 exist.

View distance:

- `0: 0.55`
- `1: 0.70`
- `2: 0.85`
- `3: 1.00`

Shadow quality:

- Virtual shadow map page caps increase from 256 to 4096.
- Volumetric fog is disabled at 0/1 and enabled at 2/3.

Post process:

- Bloom quality scales from 0 to 5.
- Bloom screen percentage scales from 25 to 50.
- Motion blur, DOF, and lens flare scale upward at higher tiers.

Texture quality:

- Texture pool sizes: 400, 600, 800, 1000 MB.
- Anisotropy: 1, 2, 4, 8.

Effects:

- Emitter spawn rate scale: 0.125, 0.25, 0.5, 1.0.
- Niagara quality level: 0, 1, 2, 3.

Shading quality:

- Material quality levels are configured, but the mapping is unusual: tier 1 maps to `2`, tiers 2/3 map to `1`.

## DefaultDeviceProfiles.ini

Profiles exist for:

- `Windows_D3D12RHI`
- `Windows_D3D11RHI`
- `Windows_VulkanRHI`
- `WindowsClient_D3D12RHI`
- `WindowsClient_D3D11RHI`

D3D12 is higher quality. D3D11 profile is lower quality and sets:

- `r.ScreenPercentage=85`

There are no Mac/Linux-specific device profile files.

## DefaultInput.ini

Enhanced Input classes are configured:

- `DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput`
- `DefaultInputComponentClass=/Script/EnhancedInput.EnhancedInputComponent`

Keyboard/mouse and gamepad mappings exist for:

- Jump
- Pause
- HUD toggle
- Interact
- Ultimate
- Media Viewer toggle
- Full map
- Inventory
- Attack lock/unlock
- Mouse lock
- Roll
- Movement
- Look

Steam Input integration was not found.

## Settings Menu

Settings UI is native Slate under:

- `Source/T66/UI/Screens/T66SettingsScreen.*`
- `Source/T66/UI/Screens/Settings/*`

Tabs include:

- Gameplay
- Graphics
- Controls
- Media Viewer
- HUD
- Audio
- Crashing
- Retro FX

Graphics settings include:

- Resolution list: 1280x720, 1600x900, 1920x1080, 2560x1440, 3840x2160.
- Overall scalability 0-3.
- FPS cap 30, 60, 90, 120, unlimited.
- Monitor/window/display mode.
- Fog controls.

Apply path uses `UGameUserSettings::SetScreenResolution`, `SetFullscreenMode`, `SetOverallScalabilityLevel`, `SetFrameRateLimit`, and `ApplySettings(false)`.

No VSync control was found.

