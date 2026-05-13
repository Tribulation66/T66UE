# T66 UI Technical Handoff For Claude

This document explains how T66 currently builds UI in Unreal technical terms. It is meant to be handed to another model or engineer before they edit UI code. The short version is: T66 is a C++ Slate-first UI. Most active screens are `UUserWidget` classes that construct Slate widget trees in C++, then a central `UT66UIManager` handles screen lifetime, caching, navigation, modal stacking, and viewport ownership. Legacy WBP assets should not be treated as the source of truth unless current source code explicitly references them.

## Current UI Source Of Truth

Primary UI code lives under:

- `Source/T66/UI/`
- `Source/T66/Gameplay/T66PlayerController_Frontend.cpp`
- `Source/T66/Gameplay/T66PlayerController_Frontend.h`
- `Source/T66/Systems/` for UI-facing subsystems
- `Source/T66/Localization/` for localized display text
- `Content/Data/` for data-authored UI/gameplay content where applicable
- `SourceAssets/UI/Reference/` for source UI reference art and generated chrome assets
- `UI/` for UI generation prompts and process docs
- `Audit/Reference/T66_UI_AUDIT.md` for older but still useful UI audit notes

The main technical files to understand first are:

- `Source/T66/UI/T66UITypes.h`
- `Source/T66/UI/T66ScreenBase.h`
- `Source/T66/UI/T66ScreenBase.cpp`
- `Source/T66/UI/T66UIManager.h`
- `Source/T66/UI/T66UIManager.cpp`
- `Source/T66/UI/Style/T66Style.h`
- `Source/T66/UI/Style/T66Style.cpp`
- `Source/T66/UI/Screens/T66ScreenSlateHelpers.h`
- `Source/T66/UI/Screens/T66ScreenSlateHelpers.cpp`
- `Source/T66/Gameplay/T66PlayerController_Frontend.cpp`

## Architectural Position

T66 does not use a typical Blueprint-widget-first frontend. The active pattern is:

1. A screen is identified by `ET66ScreenType`.
2. The player controller resolves that enum to a native `UT66ScreenBase` class.
3. `UT66UIManager` creates or reuses a `UUserWidget` instance for that screen.
4. The screen's `RebuildWidget()` creates a Slate tree.
5. Styling and reference-art loading are centralized through `FT66Style`, `T66ScreenSlateHelpers`, and runtime brush/font/texture helpers.

That means UI edits generally happen in C++ Slate code, not by opening a WBP and moving widgets around.

## Screen Identity

All registered frontend screens and modals use `ET66ScreenType` in `Source/T66/UI/T66UITypes.h`.

Important rules:

- Do not reorder existing enum values.
- Removed screen slots are intentionally left as gaps so serialized values do not shift.
- New screens should get new enum values rather than reusing old values casually.
- Some enum values are named like modals but are routed as full screens.

Current important enum entries include:

- `MainMenu`
- `SaveSlots`
- `HeroSelection`
- `CompanionSelection`
- `Settings`
- `Achievements`
- `Minigames`
- `PauseMenu`
- `ReportBug`
- `RunSummary`
- `PowerUp`
- `HeroGrid`
- `CompanionGrid`
- `LanguageSelect`
- `QuitConfirmation`
- `PartyInvite`
- `AccountStatus`
- `PlayerSummaryPicker`
- `SavePreview`
- Mini, TD, Idle, Deck, and Versus minigame screens
- `Challenges`
- `DailyDescent`

## Class Resolution And Registration

Frontend setup is owned by `AT66PlayerController` in `T66PlayerController_Frontend.cpp`.

The important methods are:

- `ResolveScreenClass(ET66ScreenType ScreenType)`
- `AutoLoadScreenClasses()`
- frontend UI initialization logic that creates `UT66UIManager`
- registration calls that map screen enums to native classes

`ResolveScreenClass()` is the final fallback resolver. It checks:

1. Theme-specific class maps when applicable.
2. Special native screens such as hero selection.
3. `RuntimeScreenClasses`.
4. A switch statement that returns native screen classes with `StaticClass()`.
5. Some minigame module screens via `LoadClass<UT66ScreenBase>()`.

`AutoLoadScreenClasses()` registers core native screens like main menu, hero selection, companion selection, save slots, settings, quit confirmation, hero grid, and companion grid.

During frontend initialization, the player controller creates:

```cpp
UIManager = NewObject<UT66UIManager>(this);
UIManager->Initialize(this);
```

It then registers core screens, modals, and minigame screens with `UIManager->RegisterScreenClass(...)`.

## UI Manager

`UT66UIManager` is the central runtime owner for frontend screen instances.

File:

- `Source/T66/UI/T66UIManager.h`
- `Source/T66/UI/T66UIManager.cpp`

It stores:

- `OwningPlayer`
- `ScreenClasses`
- `ScreenCache`
- `CurrentScreen`
- `CurrentScreenType`
- `CurrentModal`
- `FrontendTopBar`
- `FrontendBackButton`
- `RetroFXPreviewPopup`
- `NavigationHistory`

The manager is responsible for:

- Registering screen classes.
- Creating widgets with `CreateWidget<UT66ScreenBase>()`.
- Caching screen instances by `ET66ScreenType`.
- Setting each screen's `UIManager`.
- Setting each screen's `ScreenType`.
- Switching current screens.
- Showing and closing modals.
- Managing navigation history.
- Updating the frontend top bar and back button.
- Broadcasting screen-change events.

## Screen Creation

The manager path is:

```cpp
UT66UIManager::ShowScreen(...)
UT66UIManager::SwitchToScreen(...)
UT66UIManager::CreateScreen(...)
```

`CreateScreen()` first checks `ScreenCache`. If the screen already exists, it returns the cached instance. If not, it looks up the class in `ScreenClasses`, creates the widget, assigns manager/type state, and caches it.

That means most screen objects persist while the frontend is active. Screen code must be written as reusable stateful widgets, not one-shot throwaway objects.

## Screen Switching

When switching screens, `UT66UIManager::SwitchToScreen()` does the following:

1. Closes any active modal.
2. Deactivates the previous screen.
3. Removes the previous screen from viewport.
4. Optionally pushes the previous screen to navigation history.
5. Sets `CurrentScreen` and `CurrentScreenType`.
6. Marks the target as non-modal.
7. Adds the target to the viewport at z-order `0`.
8. Calls `OnScreenActivated()`.
9. Updates top bar/back button visibility and state.
10. Broadcasts `OnScreenChanged`.

The manager also handles the "same screen requested again" case by refreshing the screen rather than recreating it.

## Modal Behavior

Most modals are also `UT66ScreenBase` classes. Normal modals are shown through:

```cpp
UT66UIManager::ShowModal(ET66ScreenType ModalType)
```

Normal modal behavior:

- Creates or reuses a screen instance.
- Sets `bIsModal = true`.
- Adds it to the viewport at z-order `100`.
- Stores it as `CurrentModal`.
- Calls `OnScreenActivated()`.

Closing a modal:

- Calls `OnScreenDeactivated()`.
- Removes it from the viewport.
- Clears modal state.
- Optionally refreshes the underlying screen if `ShouldRefreshUnderlyingScreenOnModalClose()` returns true.

Important exception:

- `Challenges` and `DailyDescent` are special-cased in `ShowModal()` and routed through `ShowScreen()` instead. They behave as full screens even though they may be reached through modal-style navigation controls.

## Base Screen Contract

Most active frontend screens inherit from `UT66ScreenBase`.

File:

- `Source/T66/UI/T66ScreenBase.h`
- `Source/T66/UI/T66ScreenBase.cpp`

`UT66ScreenBase` inherits from `UUserWidget`, but its UI is normally built by returning Slate widgets.

Key fields:

- `ScreenType`
- `bIsModal`
- `UIManager`
- `bHasBuiltSlateUI`
- `bSlateRebuildQueued`
- `bHasBeenActivated`

Key lifecycle methods:

- `OnScreenActivated()`
- `OnScreenDeactivated()`
- `RefreshScreen()`
- `HandleBackAction()`
- `ShouldRefreshUnderlyingScreenOnModalClose()`
- `ForceRebuildSlate()`
- `RequestDeferredSlateRebuild()`

Key construction methods:

- `BuildSlateUI()`
- `RebuildWidget()`

The default rebuild path is:

```cpp
TSharedRef<SWidget> UT66ScreenBase::RebuildWidget()
{
    MarkSlateUIBuilt();
    return FT66Style::MakeResponsiveRoot(BuildSlateUI());
}
```

That is the standard "screen builds Slate, style wraps responsive root" path.

## BuildSlateUI Pattern

Typical screen implementation:

```cpp
TSharedRef<SWidget> UMyScreen::BuildSlateUI()
{
    return SNew(SOverlay)
        + SOverlay::Slot()
        [
            ...
        ];
}
```

Screens usually compose:

- `SOverlay`
- `SBorder`
- `SVerticalBox`
- `SHorizontalBox`
- `SGridPanel`
- `SScrollBox`
- `SBox`
- `STextBlock`
- `SImage`
- style-built buttons and panels from `FT66Style`
- helper-built reference art from `T66ScreenSlateHelpers`

Screens should prefer local helper functions for repeated subtrees. Larger screens often split logic into additional `.cpp` files under a subfolder, such as the hero selection and settings screens.

## Refresh Pattern

Because screens are cached, visual state should update through:

- `RefreshScreen_Implementation()`
- stored `TSharedPtr<STextBlock>` handles
- stored `TSharedPtr<SWidget>` handles
- lightweight setter calls
- `RequestDeferredSlateRebuild()` when the structure needs to be rebuilt

Do not assume `BuildSlateUI()` runs every time a user opens a screen. Cached screens are common.

`RequestDeferredSlateRebuild()` is preferred when a screen needs a structural rebuild in response to input. It defers the rebuild to the next tick so the current Slate event is not tearing down the tree it is still traversing.

## Navigation Helpers

`UT66ScreenBase` exposes helper methods:

- `NavigateTo(ET66ScreenType TargetScreen)`
- `NavigateBack()`
- `ShowModal(ET66ScreenType ModalType)`
- `CloseModal()`

Screen code can call these instead of reaching directly into the manager. For more complex flows, screens may call `UIManager` explicitly, but the helper path is the normal style.

Buttons generally bind handlers through Slate delegates:

```cpp
.OnClicked_Lambda([this]()
{
    NavigateTo(ET66ScreenType::Settings);
    return FReply::Handled();
})
```

or through named methods when the behavior is more complex.

## Top Bar And Back Button

The frontend top bar is not just a generic screen child. It has its own widget:

- `Source/T66/UI/T66FrontendTopBarWidget.h`
- `Source/T66/UI/T66FrontendTopBarWidget.cpp`

The top bar is managed by `UT66UIManager`, refreshed during screen changes, and customized based on `CurrentScreenType`.

The top bar uses reference layout rectangles from `T66MainMenuReferenceLayout` and related helpers. It builds stateful Slate button plates, labels, currency display, account/settings/language/menu buttons, and the power-off button.

The top bar also has screen-specific behavior. For example, `DailyDescent` uses a smaller button set than the main menu style top bar.

The frontend back button is similarly owned separately:

- `Source/T66/UI/T66FrontendBackButtonWidget.h`
- `Source/T66/UI/T66FrontendBackButtonWidget.cpp`

## Styling System

Shared UI style is centralized in `FT66Style`.

Files:

- `Source/T66/UI/Style/T66Style.h`
- `Source/T66/UI/Style/T66Style.cpp`

Important public builders:

- `FT66Style::MakeButton(...)`
- `FT66Style::MakeBareButton(...)`
- `FT66Style::MakePanel(...)`
- `FT66Style::MakeDropdown(...)`
- `FT66Style::MakeDropdownOptionButton(...)`
- `FT66Style::MakeResponsiveRoot(...)`

Important parameter structs:

- `FT66ButtonParams`
- `FT66BareButtonParams`
- `FT66PanelParams`
- `FT66DropdownParams`
- `FT66DropdownOptionParams`

`FT66ButtonParams` is the common route for text buttons. It carries:

- label text
- click delegate
- button type
- border/background visual choices
- min width
- height
- font size
- font weight
- padding
- enabled/visibility attributes
- dynamic label attribute
- color overrides
- text shadow controls
- optional custom content

`FT66BareButtonParams` is used when a button already has custom content or reference-art states and needs Slate button behavior without the standard text-button composition.

## Button Types

`ET66ButtonType` defines shared button intent:

- `Neutral`
- `Primary`
- `Danger`
- `Success`
- `ToggleActive`
- `Row`

These map to consistent colors, hover behavior, and visual treatments. New screen code should use existing button types unless it is intentionally adding a new reusable style.

## Reference-Art Buttons

Many newer screens use generated reference UI chrome instead of pure-color Slate boxes.

Common helper:

```cpp
T66ScreenSlateHelpers::MakeReferenceSlicedPlateButton(...)
```

This is important for non-square reference buttons. The intended pattern is:

- Use a text-free PNG plate or button state.
- Use sliced rendering so the center stretches without distorting ornate ends.
- Render live Slate text above the plate.
- Keep texture filtering nearest or otherwise pixel-faithful where required.
- Clamp minimum width through `NormalizeReferenceSlicedButtonMinWidth()` when needed.

Do not bake labels into button images. Labels are live text for localization, state updates, accessibility, and resolution independence.

## Panels, Borders, And Chrome

Panel and border construction comes from:

- `FT66Style::MakePanel(...)`
- `T66ScreenSlateHelpers::MakeReferenceSharedBorder(...)`
- `T66ScreenSlateHelpers::MakeReferenceHorizontalSlicedImage(...)`
- `T66ScreenSlateHelpers::MakeReferenceProgressBar(...)`
- `T66OverlayChromeStyle::MakePanel(...)`
- `T66OverlayChromeStyle::MakeSlotPanel(...)`

General rule:

- Use Slate layout for structure.
- Use generated or source PNGs for text-free chrome.
- Use slices or tiling for art that needs resizing.
- Do not stretch ornate full PNGs across arbitrary sizes unless the asset was authored for that exact size.

## Component Wrappers

There are a few reusable UI component classes:

- `Source/T66/UI/Components/ST66Button.cpp`
- `Source/T66/UI/Components/ST66Panel.cpp`
- `Source/T66/UI/Components/T66Button.cpp`

`ST66Button` and `ST66Panel` are thin Slate wrappers around `FT66Style`.

`UT66Button` is a UMG-facing wrapper. It applies `FT66Style` to `UButton`, sets the hand cursor, binds click and hover events, and can execute configured actions like navigation, modal open/close, start game, quit game, select hero, or select companion. It exists for compatibility and wrappers, but it is not the dominant new-screen pattern.

For new frontend work, prefer native Slate composition in the screen class unless the existing local pattern requires one of these wrappers.

## Runtime Texture And Brush Access

Runtime UI asset access is centralized so screens do not all implement their own file-loading logic.

Files:

- `Source/T66/UI/Style/T66RuntimeUITextureAccess.h`
- `Source/T66/UI/Style/T66RuntimeUITextureAccess.cpp`
- `Source/T66/UI/Style/T66RuntimeUIBrushAccess.h`
- `Source/T66/UI/Style/T66RuntimeUIBrushAccess.cpp`

Important functions include:

- `MakeProjectDirPath(...)`
- `MakeProjectContentPath(...)`
- `MakeProjectRuntimeDependencyPath(...)`
- `MapSourceRelativePathToRuntimeDependencyRelativePath(...)`
- `BuildLooseTextureCandidatePaths(...)`
- `LoadAssetTexture(...)`
- `ImportFileTexture(...)`
- `ImportFileTextureWithGeneratedMips(...)`
- `ResolveOptionalTextureBrush(...)`
- `ResolveDeletedThemeButtonPlateBrush(...)`
- `ConfigureSimpleReferenceFallbackBrush(...)`

These helpers let the runtime find reference art from source/dependency paths and create Slate brushes for it. Screens should use these shared helpers or higher-level style/screen helpers instead of inventing ad hoc image import logic.

## Runtime Font Access

Font resolution is centralized in:

- `Source/T66/UI/Style/T66RuntimeUIFontAccess.h`
- `Source/T66/UI/Style/T66RuntimeUIFontAccess.cpp`

Important functions include:

- `ResolveLockedUIFontPath(...)`
- `IsBoldWeight(...)`
- `MakeFontFromAbsoluteFile(...)`
- `MakeLocalizedEngineFont(...)`

Screens should not hardcode random font file paths. Use the shared style/font access path so localized fallback behavior remains consistent.

## Reference Layout System

Several screens are built against a 1920x1080 reference layout and then mapped into the live viewport.

Important files:

- `Source/T66/UI/Style/T66ReferenceLayout.h`
- `Source/T66/UI/Style/T66MainMenuReferenceLayout.generated.h`
- `Source/T66/UI/Style/T66HeroSelectionReferenceLayout.generated.h`
- `Source/T66/UI/Style/T66HeroSelectionReferenceLayoutHelpers.h`
- `Source/T66/UI/Style/T66HeroSelectionReferenceLayoutHelpers.cpp`

Important structs:

- `FT66ReferenceRect`
- `FT66ReferenceTransform`

Typical use:

1. Define or use a reference rectangle from the generated layout.
2. Transform it into the current target size.
3. Place Slate widgets using `SCanvas`, `SBox`, or helper methods.

This pattern is used when a screen must align tightly to reference UI art or screenshots.

## Responsive Root

The default `UT66ScreenBase::RebuildWidget()` wraps screen output in:

```cpp
FT66Style::MakeResponsiveRoot(BuildSlateUI())
```

The responsive root uses a reference resolution of 1920x1080 and a custom scaling wrapper so screens can retain intended composition across viewport sizes.

Important caveat:

- Not every UI surface goes through the base screen path. Some major widgets and overlays have custom rebuild paths. Examples include the main menu, frontend top bar, run summary, and gameplay HUD/overlay surfaces. Always inspect the specific class before assuming it uses the base responsive wrapper.

## Screen Helpers

Shared screen helpers are in:

- `Source/T66/UI/Screens/T66ScreenSlateHelpers.h`
- `Source/T66/UI/Screens/T66ScreenSlateHelpers.cpp`

Useful helpers include:

- `MakeTopBarScreenLayoutMetrics(...)`
- `MakeTopBarScreenRoot(...)`
- `MakeSlateBrush(...)`
- `MakeResponsiveGridModalMetrics(...)`
- `MakeFilledButtonText(...)`
- `MakeReferenceHorizontalSlicedImage(...)`
- `NormalizeReferenceSlicedButtonMinWidth(...)`
- `MakeReferenceMainMenuElementAssetPath(...)`
- `MakeReferenceChromeElementAssetPath(...)`
- `MakeReferenceLongPanelAssetPath(...)`
- `MakeReferenceRedSquareButtonAssetPath(...)`
- `MakeReferenceChromeButtonAssetPath(...)`
- `MakeReferenceSharedBorder(...)`
- `MakeReferenceSlicedPlateButton(...)`
- `MakeReferenceProgressBar(...)`
- `MakeResponsiveGridTile(...)`
- `MakeResponsiveGridModal(...)`
- `MakeCenteredScrimModal(...)`
- `MakeTwoButtonRow(...)`

Before adding a one-off layout helper to a screen, check whether this file already has the needed layout/button/panel behavior.

## Gameplay HUD And Overlay UI

Not all UI is frontend menu UI. Gameplay HUD and overlays live under:

- `Source/T66/UI/T66GameplayHUDWidget.h`
- `Source/T66/UI/T66GameplayHUDWidget.cpp`
- `Source/T66/UI/HUD/`
- `Source/T66/UI/Overlays/`

Gameplay overlays include things like casino, crate, collector, combat, level-up, trade, and other in-run UI surfaces.

Some overlay styling is centralized in:

- `Source/T66/UI/Style/T66OverlayChromeStyle.h`
- `Source/T66/UI/Style/T66OverlayChromeStyle.cpp`

Overlay UI may use a different layout contract than frontend menu screens. Do not force every overlay into a frontend screen pattern. Inspect the class and follow its local style.

## Text And Localization

Display text should be live Slate text, not baked into PNG assets.

Use:

- `FText`
- `NSLOCTEXT(...)`
- localized subsystem helpers where the screen already does so
- `TAttribute<FText>` for dynamic labels

Avoid:

- `FString` for user-facing display text unless immediately converted to `FText`
- baking labels, numbers, player names, scores, dates, costs, or stats into images
- duplicating translation strings in multiple screens

The UI generation process explicitly separates text-free chrome from live runtime text.

## Runtime Data In UI

Screens commonly pull from:

- `UT66GameInstance`
- player/account subsystems
- local save data
- leaderboard systems
- minigame subsystems
- settings subsystems
- localization subsystem
- data tables in `Content/Data`

Because screens are cached, state should be refreshed when activated and when underlying data changes. Prefer storing pointers to specific text/image/list widgets and updating them during refresh. Use full rebuilds only when the widget structure changes.

## Cursor And Hover Behavior

Buttons should use the hand cursor on hover.

Shared button paths set:

```cpp
EMouseCursor::Hand
```

The style code also uses `FSlateApplication::Get().QueryCursor()` through `RefreshMouseCursorQuery()` to force cursor refresh when hover state changes. This matters because Unreal can otherwise keep showing the old hardware cursor until the next movement/input event.

When adding custom buttons, make sure they preserve:

- hand cursor
- hover state
- pressed state
- disabled state
- click debounce behavior if the local helper provides it

## Retro FX And Retainer Passes

UI visual effects are controlled through the Retro FX/settings pipeline rather than scattered per-screen post-process hacks.

Important areas:

- `FT66RetroFXSettings`
- `UT66PlayerSettingsSubsystem`
- `UT66RetroFXSubsystem`
- `Source/T66/UI/Screens/Settings/T66SettingsScreen_RetroFX.cpp`
- retainer usage in `FT66Style`

The UI retainer path separates effects by pass, including chrome, text, and background/image style effects. Effects can include pixelation, dithering, vertex snap, scanlines, and chromatic aberration depending on settings.

If a UI request is about stylization, shader-like effects, pixelation, or retro presentation, first check the Retro FX chain instead of adding one-off screen effects.

## Source Asset Folders

Accepted runtime UI source art belongs in:

- `SourceAssets/UI/Reference/Screens/<ScreenName>/`
- `SourceAssets/UI/Reference/Modals/<ModalName>/`
- `SourceAssets/UI/Reference/Shared/`

Working captures and temporary UI verification artifacts belong in:

- `Saved/Codex/UI/<ScreenName>/`
- `Saved/Screenshots/UI/`

Use the `SourceAssets` folders for durable art that the runtime or scripts should be able to find again. Do not make random source-art folders for individual tasks.

## What Image Assets Should Contain

Image assets should be text-free chrome whenever possible:

- panels
- rows
- button plates
- button states
- tabs
- dropdown shells
- slots
- scrollbars
- icons
- dividers
- meters
- frames
- decorative background art

Live Slate should handle:

- labels
- localized text
- player names
- stats
- scores
- dates
- save metadata
- balances
- costs
- counts
- selection state
- owned/unowned state
- dynamic button state

This separation keeps screens localizable, refreshable, and resilient to data changes.

## WBP And Blueprint Position

The active frontend direction is Slate-first. Some Blueprint or WBP assets may still exist from older workflows, experiments, or compatibility paths. Their existence does not prove they are used.

Before treating a WBP as live, verify at least one current reference:

- a native C++ class loads it
- a config path references it
- a map or game mode assigns it
- an asset reference chain reaches it
- runtime logs show it being created

If none of those are true, it is likely clutter or historical residue. The same rule applies to screen-like Blueprint assets generally.

## Adding A New Screen

A normal new frontend screen should follow this checklist:

1. Add a new `ET66ScreenType` value without shifting existing values.
2. Create a `UT66ScreenBase` subclass under the appropriate `Source/T66/UI/Screens/` location.
3. Implement `BuildSlateUI()`.
4. Implement `RefreshScreen_Implementation()` if it has runtime state.
5. Register the screen in frontend initialization.
6. Add or update `ResolveScreenClass()` if the screen needs explicit fallback resolution.
7. Add navigation from the relevant existing screen.
8. Use `FT66Style` and `T66ScreenSlateHelpers` before adding new style code.
9. Put source chrome art under the correct `SourceAssets/UI/Reference/...` folder.
10. Compile and capture a screenshot for visual verification.

For a minigame screen, also check the minigame module pattern. Existing minigames commonly follow:

- module entry in `T66.uproject`
- `ET66ScreenType` entry
- screen class inside the minigame module
- frontend registration/resolution
- minigames-tab navigation button

## Adding A New Modal

A normal modal should usually still be a `UT66ScreenBase` subclass.

Checklist:

1. Add or reuse an `ET66ScreenType` value.
2. Implement the modal screen class.
3. Register it with `UT66UIManager`.
4. Open it with `ShowModal(...)`.
5. Close it with `CloseModal()` or a manager call.
6. Use z-order/modal behavior from `UT66UIManager`.
7. Override `ShouldRefreshUnderlyingScreenOnModalClose()` if the modal changes data that the underlying screen displays.

Do not add a special-case full-screen modal route unless the design intentionally behaves like `Challenges` or `DailyDescent`.

## Editing An Existing Screen

Before editing a screen:

1. Locate the class in `Source/T66/UI/Screens/`.
2. Check whether the class is split into multiple `.cpp` files.
3. Check if it uses reference layout helpers.
4. Check if it has stored widget pointers that are refreshed later.
5. Check whether it relies on `RequestDeferredSlateRebuild()`.
6. Check if top bar behavior is controlled outside the screen.
7. Check whether any image paths point to `SourceAssets/UI/Reference/...`.

Then make the smallest change that fits the current local pattern.

## Common Screen Families

Core frontend screens:

- `T66MainMenuScreen`
- `T66SaveSlotsScreen`
- `T66HeroSelectionScreen`
- `T66CompanionSelectionScreen`
- `T66SettingsScreen`
- `T66AchievementsScreen`
- `T66PowerUpScreen`
- `T66MinigamesScreen`
- `T66ChallengesScreen`
- `T66DailyClimbScreen`
- `T66RunSummaryScreen`

Grid and picker screens:

- `T66HeroGridScreen`
- `T66CompanionGridScreen`
- `T66PlayerSummaryPickerScreen`
- `T66SavePreviewScreen`

System modals/screens:

- `T66PauseMenu`
- `T66QuitConfirmationModal`
- `T66ReportBugModal`
- `T66LanguageSelectScreen`
- `T66PartyInviteModal`
- `T66AccountStatusModal`

Minigame screens:

- `T66Mini*`
- `T66TowerDefense*`
- `T66IdleMainMenuScreen`
- `T66DeckMainMenuScreen`
- `T66VersusMainMenuScreen`

## Layout And Sizing Rules

The repo's UI process docs emphasize strict layout verification.

Important principles:

- Design against the intended viewport/reference layout.
- Use stable dimensions for fixed-format controls.
- Clamp button widths where text could overflow.
- Use sliced or tiled art for resizable chrome.
- Keep live text inside its container.
- Do not let hover states resize the layout.
- Do not nest decorative cards inside decorative cards.
- Do not use art stretching to solve sizing problems.

Relevant process docs:

- `UI/Processes/LAYOUT_AND_SIZING.md`
- `UI/Processes/SCREEN_MODAL_TASK.md`
- `UI/Processes/UI_GENERATION.md`
- `UI/SCREEN_WORKFLOW.md`

## Verification Workflow

For source-only UI changes, a compile is necessary but not enough.

Useful compile command pattern:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex
```

Useful screenshot workflow is usually through the repo scripts, for example:

```powershell
.\Scripts\CaptureT66UIScreen.ps1 -Screen DailyDescent -DelaySeconds 6 -CloseAfter
```

or the screen/modal-specific automation already present in the repo.

Final acceptance should include:

- successful compile
- screen opens without fatal error
- visual screenshot check against the requested target
- no obvious text overflow
- no broken texture/font fallbacks
- no unexpected WBP dependency introduced

If the change affects the playable standalone build, the project rule is to refresh the staged standalone build and verify the taskbar shortcut target:

```text
C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

Do not run full staging for every small per-screen iteration unless the task specifically affects standalone packaging or final playable validation.

## Things Claude Should Not Do

Do not:

- create new WBP screens for active frontend work unless explicitly asked and verified against current architecture
- assume a WBP is live because it exists in Content
- bake text into PNG UI assets
- renumber `ET66ScreenType`
- add ad hoc image-loading code when runtime texture helpers already exist
- add one-off style systems for a single screen when `FT66Style` or helper code can handle it
- stretch ornate PNGs instead of using slice/tile/fixed-size contracts
- hardcode UI data in C++ when the repo has data-authored or subsystem-backed sources
- bypass `UT66UIManager` for normal screen navigation
- rebuild a cached screen's full tree when updating one text value would work
- ignore the top bar/back button manager path
- treat compile success as visual verification
- delete Blueprint/WBP assets without checking references and runtime usage first

## Practical Mental Model

Think of T66 UI as a native Unreal Slate application embedded in `UUserWidget` shells:

- `ET66ScreenType` is the screen routing key.
- `AT66PlayerController` resolves and registers screen classes.
- `UT66UIManager` owns screen instances and navigation.
- `UT66ScreenBase` gives each screen a lifecycle and Slate rebuild contract.
- `FT66Style` builds shared controls and responsive roots.
- `T66ScreenSlateHelpers` builds reference-art UI pieces.
- Runtime texture/font/brush helpers keep file-backed assets consistent.
- Process docs under `UI/` define how generated/reference UI art should be authored and verified.

When editing UI, start from those seams and follow the existing screen's local construction pattern.
