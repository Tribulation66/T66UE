# Pass16 Codex Draft For Cross-Review

Task: implement FriendslopStyle doc organization Phase 1/2 and produce another Main Menu screen pass closer to the Round06 reference under the updated no-manual-visual-authoring process.

Operator/Validator state: Codex operator, Claude validator, verified from `.t66/operator-state.json`.

Process changes made:

- Added `UI/FriendslopStyle/README.md`, `UI/FriendslopStyle/Screens/MainMenu/README.md`, `UI/FriendslopStyle/Archive/README.md`, `UI/FriendslopStyle/Reference/MainMenu/Current/README.md`, and `UI/FriendslopStyle/Screens/MainMenu/component_contract_current.md`.
- Moved active Main Menu docs into `UI/FriendslopStyle/Screens/MainMenu/`.
- Moved old pass artifacts, deprecated slice specs, and old reference iterations under `UI/FriendslopStyle/Archive/`.
- Moved the current Round06 reference to `UI/FriendslopStyle/Reference/MainMenu/Current/main_menu_reference_01_current_capture_stronger_rubber_cli.png`.
- Updated `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`, `UI/UI_AGENTS.md`, and `UI/FriendslopStyle/friendslop_asset_registry.md` with the no manual visual-pixel-authoring rule: production pixels must be built-in account-backed imagegen or a documented user-approved exception; crop/alpha/Pillow/OpenCV/skimage are allowed only for measurement, packaging, and QA.
- Updated `UI/FriendslopStyle/Screens/MainMenu/checklist.md` to remove stale title-as-live-label rows because the current contract permits a generated title-only asset; title remains covered by existence, geometry, containment, and visual scorecard gates.

Runtime changes made:

- Generated a new title-only `CHADPOCALYPSE` asset with built-in account-backed imagegen, chroma-key removal, alpha validation, and packaging into `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/title_logo_round06.png`.
- The active title is the v3 runtime-footprint candidate packaged at 730x100. The earlier 900x160 v2 title was superseded after cross-review flagged non-uniform runtime stretching risk.
- Replaced `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/cta_primary_round06.png` with a blank imagegen-authored red rubber plate. No erasure or center cleanup was used.
- Replaced `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/topbar_tab_dark_round06.png` and `topbar_tab_red_round06.png` with blank imagegen-authored rubber tab plates.
- Wired Main Menu background poster paths to `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/mainmenu_screen_art_mainmenu_newmm_rubbery_friendslop_pass16_1920.png`.
- Updated title layout in `Source/T66/UI/Screens/T66MainMenuScreen.cpp` to render the generated title at 730x100 inside the title region and clear the topbar.
- Updated title brush fallback size in `Source/T66/UI/Style/T66FriendslopStyle.cpp` to match the active 730x100 packaged source.

Verification:

- `Scripts/StageStandaloneBuild.ps1 -SkipCook` succeeded after closing a stale staged `T66.exe` lock.
- Final capture: `Saved/Codex/UI/FriendslopStyle/MainMenu/friendslop_pass16_fixture_capture.png`.
- Final dump: `Saved/Codex/UI/FriendslopStyle/MainMenu/friendslop_pass16_fixture_dump.json`.
- UTF-8 dump: `Saved/Codex/UI/FriendslopStyle/MainMenu/friendslop_pass16_fixture_dump_utf8.json`.
- Verifier: `Saved/Codex/UI/FriendslopStyle/MainMenu/pass16_verify_report.md`.
- Contact sheet: `Saved/Codex/UI/FriendslopStyle/MainMenu/pass16_verify_contact_sheet.png`.
- Visual scorecard: `Saved/Codex/UI/FriendslopStyle/MainMenu/pass16_visual_scorecard.md`.
- Final verifier result: `PASS=261 FAIL=1 UNSURE=0`.
- The only remaining verifier failure is `MainMenu.VisualScorecard`, because the visual scorecard is intentionally `Result: FAIL`.
- CTA and topbar tab labels remain live Slate text over blank generated plates; the generated plates do not contain labels or erased label corridors.
- The removed title live-label checklist rows were replaced by the current contract coverage: title existence, geometry, containment, no-crop visual inspection, and holistic visual scorecard. This does not weaken the title-crop gate because the current contract allows a generated title-only asset.

Honest pass status:

Pass16 is not accepted as visually matching the reference. It fixed the title cropping/layout problem, removed the primary CTA center masking, and improved topbar tab plate authorship. Remaining visual blockers include side panel frame mismatch, search/friend row/secondary button older plate quality, baked topbar glyph ownership, and background material still reading stone/tiled rather than strongly rubbery.

Skipped gates:

- Responsive/manual interaction gates were not run in this pass.
