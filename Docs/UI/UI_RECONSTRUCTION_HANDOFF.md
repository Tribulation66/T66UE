# UI Reconstruction Handoff

## Purpose

This handoff is for continuing the T66 screen-reconstruction process from a reference image, starting with the main menu.

The immediate goal is:

- make packaged runtime screens match approved references as closely as possible
- do it with real widgets, real labels, and production-safe ownership

## First Target

The first active target is the main menu.

## Installed Skill Split

The repo-managed copies now live under `C:\UE\T66\CodexSkills\UI`.

Install or refresh the live local copies with:

`powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Scripts\InstallUIReconstructionSkills.ps1" -Validate`

After bootstrap, the live stage skills exist under `C:\Users\DoPra\.codex\skills`:

- `ui-reconstruction-orchestrator`
- `ui-reference-prep`
- `ui-style-reference`
- `ui-layout-manifest`
- `ui-sprite-families`
- `ui-runtime-reconstruction`
- `ui-packaged-review`

Use `ui-reconstruction-orchestrator` to route work to the correct stage. Use the specialist skills directly when the stage is already known.
Use `ui-reference-prep` only when the active blocker is reference usability, such as deterministic `2x/4x` exports or helper-only AI upscales.
Run the content ownership audit before style-reference generation, sprite-family generation, manifest refinement, or packaged diffing.
Prefer native Codex `image_gen` first. Use the ChatGPT bridge only when repo-local attachments, bridge manifests, or API-side controls are the real blocker.

### Primary reference files

- `C:\UE\T66\SourceAssets\Reference Main Menu.png`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\reference_main_menu_master.png`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\reference_main_menu_master_nobuttons.png`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\reference_main_menu_master_notopbarbuttons.png`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\content_ownership.json`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\reference_layout.json`

### Runtime files

- `C:\UE\T66\Source\T66\UI\Screens\T66MainMenuScreen.cpp`
- `C:\UE\T66\Source\T66\UI\T66FrontendTopBarWidget.cpp`
- `C:\UE\T66\Source\T66\UI\Components\T66LeaderboardPanel.cpp`
- `C:\UE\T66\Source\T66\UI\Style\T66MainMenuReferenceLayout.generated.h`

### Build and asset scripts

- `C:\UE\T66\Scripts\BuildReferenceMainMenuAssets.py`
- `C:\UE\T66\Scripts\SliceMainMenuGeneratedSpriteSheets.py`
- `C:\UE\T66\Scripts\BuildMainMenuCTAButtonAssets.py`

## What We Are Trying To Establish

The real deliverable is not just one fixed main menu. The real deliverable is a repeatable process for reconstructing screens from a reference image.

That process should be:

1. reference-led
2. measured
3. ownership-audited before generation
4. family-based instead of cleanup-based
5. validated in packaged runtime

## Current Lessons Learned

### 1. Do not use one flattened image as both layout and art source

The approved reference is good as a layout spec. It is not a reliable direct source for runtime plates once text contamination and mixed shells enter the frame.

### 2. Mixed ownership causes churn

The main menu currently suffers from overlapping ownership:

- top bar shell versus live top-bar buttons
- CTA stack shell versus live CTA buttons

Any future pass should fix ownership first before touching polish.

### 3. Plate proportions matter as much as placement

The recurring stretched-button problem was not only placement. It was also caused by feeding the runtime the wrong aspect-ratio art and then forcing it into measured slots.

### 4. Packaged validation is mandatory

Several passes that looked plausible in theory failed immediately in packaged runtime because of brush behavior, background contamination, or duplicated baked regions.

### 5. Upscale outputs are helper-first by default

Prepared or AI-upscaled references help inspection and prompting, but they do not fix wrong proportions or ownership by themselves and should default to `helper-only` unless deliberately reviewed.

## Current Main Menu Status

Blunt status: `close with blockers`.

The main visible blockers are:

- top bar still has ownership/alignment problems
- center CTA stack still has a shell/background conflict
- right leaderboard panel still needs a manifest-driven content rect instead of padding hacks

## Known Useful Paths

### Main menu reference assets

- `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\Center`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\LeftPanel`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\RightPanel`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\SheetSlices`

### Recent packaged screenshots

- `C:\UE\T66\output\mainmenu_finish_pass33.png`
- `C:\UE\T66\output\mainmenu_finish_pass35.png`

Use them only as diagnostics. They are not acceptance captures.

## Recommended Next-Step Order

1. Freeze one sane background owner for the main menu.
2. Resolve top-bar ownership cleanly.
3. Resolve CTA stack ownership cleanly.
4. Rebuild the right panel around measured content bounds.
5. Only then polish text fit and live-content framing.

## Hard Rules For The Next Model

- do not redesign the screen
- do not use freehand placement nudges when a manifest box exists
- do not rely on pixel cleanup after the fact as the primary method
- do not let two layers own the same visible shell region
- do not keep iterating multiple regions in one packaged pass if one of them is already regressing
