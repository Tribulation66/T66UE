# Section 1 - Engine and Build Configuration

## Engine Version

- `T66.uproject` declares `EngineAssociation: "5.7"`.
- Installed engine version from `C:\Program Files\Epic Games\UE_5.7\Engine\Build\Build.version`:
  - `MajorVersion=5`
  - `MinorVersion=7`
  - `PatchVersion=1`
  - `Changelist=48512491`
  - `CompatibleChangelist=47537391`
  - `BranchName="++UE5+Release-5.7"`
  - `IsPromotedBuild=1`

## UProject Modules

`T66.uproject` declares runtime modules:

- `T66`
- `T66Mini`
- `T66TD`
- `T66Idle`
- `T66Deck`

It also declares editor module:

- `T66Editor`

All have `LoadingPhase="Default"`.

## Enabled Plugins

All plugin entries in the `Plugins` block are enabled:

- `ModelingToolsEditorMode`, `TargetAllowList=["Editor"]`
- `PythonScriptPlugin`, `TargetAllowList=["Editor"]`
- `EditorScriptingUtilities`, `TargetAllowList=["Editor"]`
- `OnlineSubsystemSteam`
- `SocketSubsystemSteamIP`
- `ProceduralMeshComponent`
- `AnimToTexture`
- `ElectraPlayer`

## Build.cs Inventory

Build files found:

- `Source/T66/T66.Build.cs`
- `Source/T66Mini/T66Mini.Build.cs`
- `Source/T66TD/T66TD.Build.cs`
- `Source/T66Idle/T66Idle.Build.cs`
- `Source/T66Deck/T66Deck.Build.cs`
- `Source/T66Editor/T66Editor.Build.cs`

`T66.Build.cs` public dependencies:

- `Core`
- `CoreUObject`
- `Engine`
- `InputCore`
- `SlateCore`
- `UMG`
- `HTTP`
- `Json`
- `DeveloperSettings`

`T66.Build.cs` private dependencies:

- `EnhancedInput`
- `AIModule`
- `NavigationSystem`
- `Slate`
- `AssetRegistry`
- `Landscape`
- `Foliage`
- `ApplicationCore`
- `Niagara`
- `JsonUtilities`
- `OnlineSubsystem`
- `OnlineSubsystemUtils`
- `Steamworks`
- `OnlineSubsystemSteam`
- `ImageWrapper`
- `MediaAssets`
- `Media`
- `ProceduralMeshComponent`
- `Projects`
- `RenderCore`

`T66.Build.cs` Win64-only block:

- Adds WebView2 include path from `ThirdParty/WebView2/build/native/include`.
- Links `user32.lib`, `ole32.lib`, `shlwapi.lib`.
- Delay-loads and stages `WebView2Loader.dll`.
- Stages `steam_appid.txt`.
- Defines `T66_WITH_WEBVIEW2=1` only for Win64.
- Defines `T66_WITH_WEBVIEW2=0` for all other platforms.

Loose runtime dependencies staged by `T66.Build.cs` include:

- `RuntimeDependencies/T66/Arcade`
- `RuntimeDependencies/T66/Fonts`
- `RuntimeDependencies/T66/UI`
- `RuntimeDependencies/T66/Video`
- `Content/Movies`

Other module notes:

- `T66Mini.Build.cs` depends on `T66`, `HTTP`, `Json`, `JsonUtilities`, `ImageWrapper`, `OnlineSubsystem`, `OnlineSubsystemUtils`, and `Steamworks`.
- `T66TD.Build.cs`, `T66Idle.Build.cs`, and `T66Deck.Build.cs` are smaller Slate/JSON/T66 runtime modules and stage their respective data/source asset roots.
- `T66Editor.Build.cs` is editor-only and depends on `UnrealEd`, `UMGEditor`, `Blutility`, `EditorSubsystem`, `AssetTools`, `AssetRegistry`, `LandscapeEditor`, `Foliage`, `LevelEditor`, and `ToolMenus`.

## Target.cs Files

`Source/T66.Target.cs`:

- `Type = TargetType.Game`
- `DefaultBuildSettings = BuildSettingsVersion.V6`
- `IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7`
- Adds module `T66`

`Source/T66Editor.Target.cs`:

- `Type = TargetType.Editor`
- `DefaultBuildSettings = BuildSettingsVersion.V6`
- `IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7`
- Adds modules `T66` and `T66Editor`

No target file contains:

- `OptedInModulePlatforms`
- `bUsesSteam`
- Linux/Mac-specific platform blocks
- platform allow/deny lists

## BuildConfiguration Defaults

Repo-local and user-level UBT `BuildConfiguration.xml` files exist but are effectively empty:

- `Saved/UnrealBuildTool/BuildConfiguration.xml`
- `C:\Users\DoPra\AppData\Roaming\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml`

I did not build Shipping, so Shipping cleanliness is unproven. Static structure looks broadly Shipping-aware because editor modules/plugins are separated, but runtime plugin/build surface should be tested in Shipping, especially Steam, Electra, AnimToTexture, ProceduralMesh, and WebView2 exclusion.

## Root and Build Scripts

No custom `.bat`, `.cmd`, or `.ps1` files were found directly in repo root or `Build/`.

`Build/Windows/FileOpenOrder` contains:

- `CookerOpenOrder.log`
- `EditorOpenOrder.log`

Workflow scripts live elsewhere:

- `Scripts/StageStandaloneBuild.ps1`
- `Scripts/StageDemoBuild.ps1`
- UI capture scripts under `Scripts/`
- Steam upload scripts under `Tools/Release/Steam/`

`StageStandaloneBuild.ps1` is Win64-oriented and stages to `Saved\StagedBuilds`.

