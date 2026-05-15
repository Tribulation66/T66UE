# Hero Selection Closeout and Stage 2 Readiness

Date: 2026-05-11

## A. Final Clean Pass

Palette restore:
- `FT66FlatStyle` is back on the loop-default palette: purple default border `#5E1F8C`, red selected border `#E1232D`, and green ready state `#1FB358`.
- The Hero Selection checklist and geometry notes were restored from the temporary grey/green experiment to the purple/red loop default.
- Runtime red accent assets restored: `RuntimeDependencies\T66\UI\Icons\Flat\skull.png` and `RuntimeDependencies\T66\UI\HeroSelection\Skins\skin_default_stub.png`.

Step 0 audit:

```powershell
rg -n "SourceAssets/UI/Reference|RuntimeDependencies/T66/UI/Reference|MakeReference|Get.*ReferenceScrollBarStyle|ST66Reference|ST66RetroUIRetainedSurface|M_UI_Glow|M_UI_RetroRetainer|MakeRetroUIChromeSurface|MakeRetroUIChromeOverlay" Source\T66\UI\Screens\HeroSelection Source\T66\UI\Screens\T66HeroSelectionScreen.cpp Source\T66\UI\Screens\T66HeroSelectionScreen.h
```

Output: `ZERO_MATCHES`. Verdict: no legacy chrome reachable from Hero Selection.

Step 0.5 geometry:
- No geometry changes were needed for closeout.
- Table: `C:\UE\T66\UI\Geometry\hero_selection_reference_geometry.md`
- Overlay: `C:\UE\T66\UI\Geometry\hero_selection_reference_geometry_overlay.png`

Capture and dump:

```powershell
.\Scripts\CaptureT66UIScreen.ps1 -Screen HeroSelection -Output C:\UE\T66\Saved\Codex\UI\HeroSelection\pass_11_capture.png -DelaySeconds 5.5 -TimeoutSeconds 120 -ExtraArgs @("-T66AutoDumpScreen=C:\UE\T66\Saved\Codex\UI\HeroSelection\pass_11_dump.json")
```

The final capture temporarily used clean staged SaveGames so the reference initial state resolves to ARTHUR. The staged SaveGames were restored afterward from `C:\UE\T66\Saved\Codex\UI\HeroSelection\pass_11_savegames_backup`.

Verification:

```powershell
python C:\UE\T66\Scripts\VerifyUIFidelity.py --reference "C:\UE\T66\UI\Screen References\Hero Selection.png" --capture "C:\UE\T66\Saved\Codex\UI\HeroSelection\pass_11_capture.png" --dump "C:\UE\T66\Saved\Codex\UI\HeroSelection\pass_11_dump.json" --checklist "C:\UE\T66\UI\Checklists\hero_selection_checklist.md" --output "C:\UE\T66\Saved\Codex\UI\HeroSelection\pass_11_report.md" --contact-sheet "C:\UE\T66\Saved\Codex\UI\HeroSelection\pass_11_contact_sheet.png"
```

Result: `PASS=651 FAIL=0 UNSURE=0`.

Final artifacts:
- Capture: `C:\UE\T66\Saved\Codex\UI\HeroSelection\pass_11_capture.png`
- Dump: `C:\UE\T66\Saved\Codex\UI\HeroSelection\pass_11_dump.json`
- Report: `C:\UE\T66\Saved\Codex\UI\HeroSelection\pass_11_report.md`
- Contact sheet: `C:\UE\T66\Saved\Codex\UI\HeroSelection\pass_11_contact_sheet.png`

Build evidence:
- Editor compile succeeded: `Build.bat T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex`
- Staged standalone rebuild succeeded: `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development`
- Shortcut targets verified:
  - `C:\UE\T66\T66 Standalone.lnk` -> `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  - Pinned taskbar `T66 Standalone.lnk` -> `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`

## B. Manual Interaction Checklist

Checklist path: `C:\UE\T66\Saved\Codex\UI\HeroSelection\manual_interaction_checklist.md`

Coverage confirmed:
- Hero portrait carousel: seven visible portrait toggles plus left/right arrows.
- Skin row selection: Default, Beachgoer, Crusader, Golden Paladin.
- CHAD / STACY gender toggle.
- BACK button.
- LAB button.
- BUY button.
- CLEAR button.
- PREVIEW button on Beachgoer and an N/A row for any other rendered preview buttons.
- DIFFICULTY dropdown.
- CHOOSE COMPANION button.
- ENTER button.
- CHALLENGES button.
- MODS button.

The checklist now describes expected behavior in plain language, including red selected treatment, purple default treatment, and which controls should update preview/state. Pablo still needs to fill `Works`, `Doesn't Work`, or `N/A`. Any `Doesn't Work` is a loop FAIL.

Strict loop status: automated visual/data gate is DONE, Pablo visual review is accepted, and manual interaction verification is ready but externally pending.

## C. Open Items Audit

### 1. Backend Wiring Gaps

Placeholder handlers currently in use:
- `C:\UE\T66\Source\T66\UI\Screens\T66HeroSelectionScreen.cpp:175` `HandleLabClicked()` logs `Action OpenLab clicked - backend not yet implemented`.
- `C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionScreen_Build.cpp:463` Beachgoer PREVIEW binds a placeholder lambda that logs `Action SkinPreview clicked - backend not yet implemented`.
- `C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionScreen_Build.cpp:614` logs `TODO: Hero Selection Steam party slots use a placeholder Steam avatar until Steam profile image integration is exposed to the UI layer.`

Other closeout-tested controls are wired to existing handlers: Back, hero carousel arrows and portraits, skin row selection, Buy/Clear drug slots, CHAD/STACY, Choose Companion, Enter, Challenges, and Mods.

### 2. Content Stubs in Use

Source stub roots:
- `C:\UE\T66\SourceAssets\UI\ContentStubs\HeroSelection\skin_default_stub.png`
- `C:\UE\T66\SourceAssets\UI\ContentStubs\HeroSelection\skin_beachgoer_stub.png`
- `C:\UE\T66\SourceAssets\UI\ContentStubs\HeroSelection\skin_crusader_stub.png`
- `C:\UE\T66\SourceAssets\UI\ContentStubs\HeroSelection\skin_golden_paladin_stub.png`
- `C:\UE\T66\SourceAssets\UI\ContentStubs\HeroSelection\companion_chad_male_blue.png`
- `C:\UE\T66\SourceAssets\UI\ContentStubs\HeroSelection\companion_stacy_female_pink.png`

Runtime Hero Selection stubs:
- `C:\UE\T66\RuntimeDependencies\T66\UI\HeroSelection\Skins\skin_default_stub.png`
- `C:\UE\T66\RuntimeDependencies\T66\UI\HeroSelection\Skins\skin_beachgoer_stub.png`
- `C:\UE\T66\RuntimeDependencies\T66\UI\HeroSelection\Skins\skin_crusader_stub.png`
- `C:\UE\T66\RuntimeDependencies\T66\UI\HeroSelection\Skins\skin_golden_paladin_stub.png`
- `C:\UE\T66\RuntimeDependencies\T66\UI\HeroSelection\Companions\companion_chad_male_blue.png`
- `C:\UE\T66\RuntimeDependencies\T66\UI\HeroSelection\Companions\companion_stacy_female_pink.png`

Flat icon assets originating from the Hero Selection imagegen/icon workflow:
- `C:\UE\T66\RuntimeDependencies\T66\UI\Icons\Flat\chad_icon.png`
- `C:\UE\T66\RuntimeDependencies\T66\UI\Icons\Flat\stacy_icon.png`
- `C:\UE\T66\RuntimeDependencies\T66\UI\Icons\Flat\skull.png`
- `C:\UE\T66\RuntimeDependencies\T66\UI\Icons\Flat\ticket.png`
- `C:\UE\T66\RuntimeDependencies\T66\UI\Icons\Flat\steam_placeholder.png`

`steam_placeholder.png` is explicitly a placeholder. The other four are generated UI glyph assets following the M1 reference-region process; replace them only if the production art pass decides to author final glyphs separately.

### 3. Icon Manifest State

Manifest: `C:\UE\T66\UI\icon_manifest.md`

Hero Selection icons regenerated through M1 reference-region workflow:
- `ticket.png` from `C:\UE\T66\UI\IconSourceCrops\HeroSelection\ticket_icon_source_crop.png`
- `skull.png` from `C:\UE\T66\UI\IconSourceCrops\HeroSelection\enter_skull_source_crop.png`
- `chad_icon.png` from `C:\UE\T66\UI\IconSourceCrops\HeroSelection\chad_icon_source_crop.png`
- `stacy_icon.png` from `C:\UE\T66\UI\IconSourceCrops\HeroSelection\stacy_icon_source_crop.png`

Still not production-ready:
- `steam_placeholder.png` is documented as missing / preserve brand. It should be replaced with an approved Steam brand asset or real Steam avatar once that UI path is wired.

Potential future audit:
- `lab_flask.png` is used by the LAB button from the shared flat icon library. It was not part of the M1 Hero Selection re-generation pass; if Pablo wants exact reference-region matching for that glyph too, include it in a future icon-only pass.

### 4. Steam Profile Integration

Current Hero Selection state:
- Steam party slots render `RuntimeDependencies\T66\UI\Icons\Flat\steam_placeholder.png`.
- `C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionScreen_Build.cpp:255` creates the placeholder brush.
- `C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionScreen_Build.cpp:369` tags the per-slot Steam avatar region.

Existing subsystem path that could be wired later:
- `C:\UE\T66\Source\T66\Core\T66SteamHelper.h:102` exposes `UT66SteamHelper::GetLocalAvatarTexture()`.
- `C:\UE\T66\Source\T66\Core\T66SteamHelper.h:105` exposes `GetAvatarTextureForSteamId()`.
- `C:\UE\T66\Source\T66\Core\T66SteamHelper.cpp:464` refreshes/caches the local Steam avatar texture.

No Hero Selection code currently asks `UT66SteamHelper` for the avatar texture. The future path is `UGameInstance -> UT66SteamHelper -> GetLocalAvatarTexture() -> Slate brush resource for Slot01.SteamAvatar`.

### 5. Character Preview Camera

The camera framing fix is in the preview-stage path, not Slate:
- `C:\UE\T66\Source\T66\Gameplay\T66HeroPreviewStage.cpp` uses selection-specific framing constants and `FrameCameraToPreview()` based on the active hero component bounds/capsule.
- `C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionPreviewController.cpp:443` calls `T66PositionHeroPreviewCamera()`.
- `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Frontend.cpp:1474` applies the stage's ideal camera transform to the frontend preview camera.

Readiness: the fix is bounds-based and should work across heroes rather than hardcoding Arthur. Known limitation: pass 11 automated verification only proves the default Arthur state; a manual all-hero carousel sweep is still needed to confirm every model frames well.

### 6. Methodology Debts

Resolved:
- `FT66FlatStyle::MakeFlatLabel` exists and is used by Hero Selection labels.
- Dump metadata reports `is_label`.
- Interactivity metadata reports `has_click_handler` and `toggle_group`.
- `-T66AutoDumpScreen` is the standard capture+dump path and worked for pass 11.

Still needs work before Stage 2 scale:
- Geometry extraction remains a visual/manual measurement step, but overlay rendering is now scriptable via `Scripts\GenerateUIGeometryOverlay.py`.
- Full checklist auto-generation from spec + geometry table does not exist yet; Stage 2 agents should hand-author or build a generator before each screen.
- Manual interaction verification is still an external human checklist, not automated click testing.
- Icon manifest needs per-screen icon pass discipline before each migration so generated placeholders do not drift from reference style.

## D. Stage 2 Readiness Assessment

### 1. Screen Name Resolver

Detailed findings are saved at `C:\UE\T66\UI\Reference\UI_STAGE2_CAPTURE_READINESS_REFERENCE.md`.

Ready by resolver after pre-rollout cleanup:
- `Overview` -> `AccountStatus` with Overview tab active.
- `History` -> `AccountStatus` with History tab active.
- `Diplomas` -> `PowerUp` with permanent/diploma tab active.
- `Drugs` -> `PowerUp` with single-use/drugs tab active.
- `SteamAchievements` -> `Achievements` with Steam/Achievements tab active.
- `Minigames`
- `SettingsRetroFX` -> `Settings` with Retro FX tab active.
- `DailyDescent`
- `Challenges`
- `LoadGame` -> `SaveSlots`
- `RunSummary`

### 2. Other Per-Screen Specs

The current Section 7.2 specs are strong visual summaries, but the non-Hero Selection specs are not yet complete enough for the stricter Stage 2 loop without additions.

Overview example:
- Needs an `Interactivity` subsection: top-bar buttons, sub-tabs, filter dropdowns, tooltip/info icons, any row/card click actions.
- Needs label-vs-button assignments: e.g. player name, account status text, warning paragraph, table headers, progress-count labels should be labels; top-bar/sub-tab/filter controls should be buttons/dropdowns.
- Needs a per-screen icon inventory: settings, globe, account/profile, power-up, achievements, minigames, ticket, power, info, shield, bar chart, trophy, stopwatch.
- Needs content-stub policy decisions: player avatar, account name, progress counts, leaderboard/table rows.
- Needs a Step 0.5 geometry table and overlay before checklist authoring.

### 3. Geometry Tables

Only Hero Selection currently has a completed geometry table and overlay:
- `C:\UE\T66\UI\Geometry\hero_selection_reference_geometry.md`
- `C:\UE\T66\UI\Geometry\hero_selection_reference_geometry_overlay.png`

The visual extraction itself remains a manual Step 0.5 activity, but overlay generation is now reusable:

```powershell
python C:\UE\T66\Scripts\GenerateUIGeometryOverlay.py --geometry C:\UE\T66\UI\Geometry\<screen>_reference_geometry.md --output C:\UE\T66\UI\Geometry\<screen>_reference_geometry_overlay.png
```

### 4. Other Stage 2 Methodology Notes

- Sub-tab/category capture aliases are now in place before starting Overview.
- Keep the pass log eight-heading template mandatory.
- Keep final verifier output as the automated gate, but do not call a screen complete until manual interaction verification is returned with no `Doesn't Work` items.
- Treat generated content stubs as acceptable visual unblockers, but record source crop/prompt/path in the icon manifest or screen closeout doc.
