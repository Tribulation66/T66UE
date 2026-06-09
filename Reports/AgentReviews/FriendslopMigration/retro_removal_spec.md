# Retro Eradication Spec — full removal of retro code, scripts, and assets

Date: 2026-06-10. Status: SPEC ONLY — execution deferred until the in-flight uncommitted
UI pass (Friendslop standard modals + RetroFX retainer removal in T66Style.cpp /
T66ScreenSlateHelpers / 4 screens) is committed. Companion: `coverage_audit.md`
(plumbing cleanup, also deferred — execute both in one pass; they overlap).

Method: 5-finder workflow inventory (170 items) + manual verification of every contested
classification. Evidence inline.

## Scope decisions (verified, with evidence)

**IN scope — safe to remove, zero visible change:**

1. **Retro UI wrappers (no-op compat)** — `FT66Style::MakeRetroUIChromeOverlay/ChromeSurface/
   BackgroundImage/Text/Icon` (T66Style.h:522-534, .cpp:1740-1775) are already stubbed
   no-ops ("Retro FX was removed") by the in-flight pass. ~10 call sites (T66Style.cpp:1332,
   1358,1562,1603,1964; T66ScreenSlateHelpers.cpp:752,780,1107,1170). Replace each call with
   its passthrough equivalent (ChromeSurface/BackgroundImage => SBox HitTestInvisible wrap —
   the hit-test behavior MUST be preserved or chrome eats clicks; Text/Icon => bare widget;
   ChromeOverlay => drop the slot), then delete the 5 functions.
2. **Scene pixelation cluster (console-only, off by default)** — VERIFIED: `SetPixelationLevel`
   is called ONLY from the Pixel0-Pixel10 console commands (T66Pixelation.cpp:22); defaults
   level 0; no settings/gameplay caller; `FT66RetroFXSettings` no longer exists in source.
   Delete: `Source/T66/Core/T66Pixelation.cpp`, `T66PixelationSubsystem.h/.cpp`,
   `Content/UI/M_PixelationPostProcess.uasset`, debug telemetry refs in
   T66WorldRuntimeProofCommands.cpp:388,684. Zero visible change.
3. **QuadRetro (dead art direction)** — VERIFIED: live `Content/Data/CharacterVisuals.csv`
   has 0 QuadRetro rows (only the `.pre_one_master.bak` does); Shroom override is behind
   `T66AreStageLaunchObjectsEnabled()==false`. Delete: ~200+ assets under
   `Content/Characters/**/QuadRetro/` (+`QuadRetroUALQA`), `Content/World/Interactables/
   Shroom/Shroom_QuadRetro.uasset` + its texture, code: `PixelatedTextureAssetPath` field
   (T66DataTypes.h:2254-2256), the loading branch (T66CharacterVisualSubsystem.cpp:1580-1592),
   `ShroomMeshOverride` (T66StageEffects.h/.cpp:55), CSV `.bak` file, and the CSV column
   (verify importer tolerates the column drop, else leave column empty).
4. **QuadRetro scripts (10)** — ImportQuadRetro{Hero,Enemy,Boss}Visuals.py + RunImport*AndExit
   variants, ValidateBossQuadRetroVisuals.py, VerifyQuadRetroHeroVisualsAndExit.py,
   MigrateQuadRetroMaterialAssignment.py, QuadRetroCharacterPipelineDefaults.py,
   ImportWorldNpcInteractablesRetroBatch01AndExit.py + Verify variant. Update Scripts/README.md.
5. **Retro materials** — `Content/Materials/Retro/**` (RetroGeometry masters, PS1/ folder
   16 instances, M_RetroChromaticAberrationPostProcess, MPC_T66_RetroGeometry; verify
   M_T66_OutlinePostProcess has no live PPV reference before deleting),
   `Content/UI/Materials/M_UI_RetroRetainer.uasset` (retainer code already removed).
6. **Editor retro tooling** — `CreateRetroChromaticAberrationMaterial` command + function
   (T66Editor/T66UISetupSubsystem.h:74-79, .cpp:71-84,104-111,607-776).
7. **UE5RFX plugin content (101.6 MB!)** — VERIFIED: zero Source references; force-cooked
   via `Config/DefaultGame.ini:24 +DirectoriesToAlwaysCook=(Path="/Game/UE5RFX")`. Delete
   the ini line, `Content/UE5RFX/` (289 files), and `Scripts/InstallUE5RFX.ps1`.
   Real cook-time + package-size win.
8. **BloodyRetro chrome preset (with asset migration FIRST)** — VERIFIED LIVE consumers:
   leaderboard filter icons are unconditional path literals
   (T66FlatLeaderboardPanel.cpp:112-117) + topbar coupon icon fallback
   (T66FrontendTopBarWidget.cpp ~863). Order: (a) copy the needed icons into
   `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/`, (b) update the 4 path literals,
   (c) collapse `ET66ReferenceChromePreset` to single preset: remove BloodyRetro enum value,
   `T66.UI.ChromePreset` CVar branch, GetReferenceChrome*Dir switches
   (T66ScreenSlateHelpers.cpp:35-74,795-875), T66Style.cpp:236-245 preset branch,
   MainMenu BloodyRetroSubtitle localization line, (d) delete
   `RuntimeDependencies/T66/UI/Reference/Screens/MainMenu/BloodyRetro/` (116 PNGs) and any
   SourceAssets twin. NOTE: overlaps the deferred Reference-naming cleanup — same files.

**OUT of scope — flagged, needs explicit user call (would visibly change the game):**

- **Pixel VFX system** (`T66PixelVFXSubsystem`, `NS_PixelParticle` + M_PixelSprite +
  T_PixelParticle): this is the LIVE combat/movement VFX carrier — jump puffs, death
  bursts, traps, plague cloud, stage effects (25+ call sites). "Pixel" aesthetic, but it is
  the current game look, not a retro post-effect. Removing/replacing = an art/VFX pass,
  not a cleanup. EXCLUDED unless the user separately orders a VFX replacement pass.
- **"Retro_Jump" audio assets** (HeltonPixelCombat pack, 3 variants): live jump SFX,
  retro in name only. EXCLUDED.
- **Tools/ArtPipeline/Items/ITEM_SPRITE_RETRO_PROCESS.md**: a separate item-sprite art
  process, not a retro effect. EXCLUDED unless that pipeline is also being retired.

## Execution order (one pass, after the in-flight UI work commits)

- R0 Preconditions: in-flight pass committed; tree clean on the touched files.
- R1 Code: items 1, 2, 3-code, 6, 8c. Focused compile after each cluster.
- R2 Assets: 8a/8b migration first (capture leaderboard + main menu + topbar to prove icons
  survive), then deletions (3-assets, 5, 7, 8d). Use file deletion, NOT broad git scans
  over Content/ (LFS hazard); stage deletions in batched git adds by folder.
- R3 Scripts: item 4 + InstallUE5RFX.ps1 + Scripts/README.md update.
- R4 Docs: update ART_DIRECTION.md (13,18,31), AGENTS.md (17,190,232),
  UI_FIDELITY_LOOP_INSTRUCTIONS.md (39,392,626-627,652), UI_FLAT_REDESIGN_REFERENCE.md
  (multiple), Scripts/README.md:21; delete stale UI/Checklists/settings_retro_fx_checklist.md
  + UI/Geometry/settings_retro_fx_reference_geometry.md; mark resolved entries in
  pending_issues_UI.md / pending_issues_Core.md. Archive/RetroFX + Archive/ToonStyle stay.
- R5 Verification gate: full stage (cook log MUST show no missing-asset warnings for
  deleted paths), captures (main menu w/ leaderboard icons, HUD, pause), pre-release smoke
  suite, and a grep gate: `rg -i "QuadRetro|BloodyRetro|RetroRetainer|PixelationSubsystem|
  MakeRetroUI|UE5RFX" Source/ Config/` returns only intentional survivors (pixel VFX,
  audio asset names if any in data).

## Save-game / persistence check

VERIFIED: no RetroFX settings persistence remains (T66PlayerSettingsSubsystem clean; the
old `FT66RetroFXSettings` removal already shipped). CharacterVisuals CSV column drop is a
DataTable reimport, not a save-game risk. No save-compat blockers.
