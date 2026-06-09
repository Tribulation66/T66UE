# FriendslopStyle Asset Registry

Status: Main Menu pilot registry. The current process authority is
`UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`; this file
records asset provenance and runtime paths only.

Pass13/pass14/pass15-era reference-crop, inpaint, and procedural reconstruction
plates are diagnostic history unless a current screen contract explicitly
accepts a specific asset. User review identified masked centers, baked-icon
ownership collisions, wrong row fill styles, title-crop failures, and other
visual discrepancies that a structured PASS count did not catch.

Process note: this registry documents asset files; it does not authorize a
visual method. Production visual pixels for new or corrected Friendslop assets
must come from account-backed built-in imagegen run in a separate local Codex
CLI worker, or from a separately documented user-approved exception. Reference
crops are measurement/comparison targets only. Cropping, alpha extraction,
resizing, slicing, and contact sheets may package or verify already-approved
generated candidates but may not author, patch, inpaint, blur, recolor, clone,
or synthesize production pixels.

Every accepted generated or regenerated asset needs an auditable worker record:
request/prompt, worker logs, final status, output PNG path, and token/hash data
when exposed. The main Codex app chat may coordinate and review assets, but it
does not generate Friendslop iteration pixels.

## Source Generation

Prompt: `C:\UE\T66\UI\FriendslopStyle\SourcePrompts\MainMenu\runtime_chrome_sheet_01.prompt.md`

Generated source sheet with chroma key: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\friendslop_mainmenu_runtime_chrome_sheet_source_chromakey.png`

Alpha source sheet: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\friendslop_mainmenu_runtime_chrome_sheet_alpha.png`

Chroma-key removal command:

```powershell
python C:\Users\DoPra\.codex\skills\.system\imagegen\scripts\remove_chroma_key.py --input C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\friendslop_mainmenu_runtime_chrome_sheet_source_chromakey.png --out C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\friendslop_mainmenu_runtime_chrome_sheet_alpha.png --auto-key border --soft-matte --transparent-threshold 12 --opaque-threshold 220 --despill
```

## Runtime Chrome

Each runtime PNG has a matching source copy under `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\` and a staged-runtime copy under `C:\UE\T66\RuntimeDependencies\T66\UI\FriendslopStyle\MainMenu\`.

## Runtime Fonts

Status: active FriendslopStyle Main Menu runtime text candidate.

- `LilitaOne-Regular.ttf`
  - Source: Google Fonts repository, `ofl/lilitaone/LilitaOne-Regular.ttf`
  - License: SIL Open Font License 1.1, stored as `RuntimeDependencies/T66/Fonts/LilitaOne-OFL.txt`
  - Runtime path: `C:\UE\T66\RuntimeDependencies\T66\Fonts\LilitaOne-Regular.ttf`
  - Owner: `T66RuntimeUIFontAccess::MakeFriendslopFont`
  - Reason: static heavy rounded display font, used to avoid Slate rendering the previous variable Fredoka font at its thin default instance.

## Shared Primitives Pass02 Textless Runtime Assets

Status: active runtime assets for the reusable standard modal and standard tooltip.

Pass02 regenerated the modal and tooltip shells as textless images through
separate local Codex CLI workers. The assets were not manually cropped, masked,
cloned, repainted, or inpainted to remove text. All modal and tooltip copy is
owned by live Slate `FText` for localization.

- Modal textless worker:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\SharedPrimitives\pass02_workers\standard_modal_textless\`
- Tooltip textless worker:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\SharedPrimitives\pass02_workers\standard_tooltip_textless\`
- Implementation record:
  `C:\UE\T66\UI\FriendslopStyle\Screens\SharedPrimitives\pass02_implementation.md`
- Runtime specs:
  `C:\UE\T66\UI\FriendslopStyle\Screens\SharedPrimitives\slice_specs.md`
- Final capture contact sheet:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\SharedPrimitives\implementation_captures_20260608\standard_modal_tooltip_contact_sheet.png`

Runtime-wired pass02 files:

- `standard_modal_panel_textless.png`
  - Source path: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\SharedPrimitives\standard_modal_panel_textless.png`
  - Runtime path: `C:\UE\T66\RuntimeDependencies\T66\UI\FriendslopStyle\SharedPrimitives\standard_modal_panel_textless.png`
  - SHA-256: `F3DE1F7D3B5A76492C2EE4F1CA9EBC0023E186288F9735CB5BA6CB165FF4BE67`
- `standard_tooltip_panel_textless.png`
  - Source path: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\SharedPrimitives\standard_tooltip_panel_textless.png`
  - Runtime path: `C:\UE\T66\RuntimeDependencies\T66\UI\FriendslopStyle\SharedPrimitives\standard_tooltip_panel_textless.png`
  - SHA-256: `1038C062E765D0E05C2A4560A2ED12F86DE94B32572AA7DBA15A4766A0FB9CBA`

## Shared Primitives Pass05 Standard Modal Checkbox Assets

Status: active runtime assets for the opt-in reusable standard modal checkbox
row.

Pass05 generated unchecked and checked checkbox plates through a fresh local
Codex CLI worker using account-backed built-in imagegen. The label remains live
Slate `FText`; the generated PNGs do not contain `Do Not Ask Again` or any
other localized copy.

- Checkbox worker:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\SharedPrimitives\pass05_workers\standard_modal_checkbox\`
- Implementation record:
  `C:\UE\T66\UI\FriendslopStyle\Screens\SharedPrimitives\pass05_implementation.md`
- Runtime specs:
  `C:\UE\T66\UI\FriendslopStyle\Screens\SharedPrimitives\slice_specs.md`

Runtime-wired pass05 files:

- `standard_modal_checkbox_unchecked.png`
  - Source path: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\SharedPrimitives\standard_modal_checkbox_unchecked.png`
  - Runtime path: `C:\UE\T66\RuntimeDependencies\T66\UI\FriendslopStyle\SharedPrimitives\standard_modal_checkbox_unchecked.png`
  - SHA-256: `FD8B61E121545052C7184025266354D9CCFB3CBB35C51862A17815F89F4E0554`
- `standard_modal_checkbox_checked.png`
  - Source path: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\SharedPrimitives\standard_modal_checkbox_checked.png`
  - Runtime path: `C:\UE\T66\RuntimeDependencies\T66\UI\FriendslopStyle\SharedPrimitives\standard_modal_checkbox_checked.png`
  - SHA-256: `1528B3F24C9956FDBAB14FB5045AF8C84A4B0C079A6E9D6D3C3FACA2A54B83CA`

## Main Menu Pass39 Approved Asset Install

Status: current targeted install for the user-approved Pass38 generated assets.

Pass39 did not regenerate pixels. It packaged and installed the approved Pass38
local Codex CLI worker outputs, then wired them into the Main Menu runtime while
preserving live labels, counts, localized text, and click behavior where those
surfaces are controls.

- Approved worker root:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass38_asset_generation_approval\pass38_workers\`
- Install manifest:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass39_approved_asset_install\asset_install_manifest.json`
- Packaged asset contact sheet:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass39_approved_asset_install\pass39_packaged_assets_contact_sheet.png`
- Final capture:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass39_final_capture.png`
- Final dump:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass39_final_dump.json`
- Wiring/functionality gate:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass39_approved_asset_install\pass39_wiring_functionality_gate.md`

Runtime-wired pass39 files:

- `title_logo_round06.png`
- `cta_primary_round06.png`
- `cta_secondary_round06.png`
- `cta_skull_icon_round06.png`
- `topbar_settings_icon_button_round06.png`
- `topbar_coupon_icon_button_round06.png`
- `topbar_power_icon_button_round06.png`
- `filter_global_icon_button_round06.png`
- `filter_friends_icon_button_round06.png`
- `filter_streamer_icon_button_round06.png`
- `friend_favorite_star_round06.png`

Packaging note: approved worker outputs were alpha-bounds cropped and resized to
runtime target sizes only. No manual repainting, recoloring, clone repair,
inpainting, or descriptive re-generation was used in Pass39.

## Main Menu Pass24 Reference + Five-Family Imagegen Iteration

Status: active runtime candidates for the current Main Menu pass.

Pass24 regenerated the authoritative reference image and all five Main Menu
visual families through separate local Codex CLI workers using account-backed
built-in imagegen. The pass copied 30 accepted generated runtime PNGs into the
runtime dependency folder and recorded source-to-runtime hashes. The pass also
rejected the first generated secondary CTA because it was purple and promoted a
fresh secondary-fix worker output instead.

- Current reference:
  `C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Current\main_menu_reference_04_foreground_right_panel_icon_filters_coupon_cli.png`
- Family assessment:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass24_visual_family_element_assessment.md`
- Worker root:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass24_workers\`
- Runtime copy manifest:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass24_runtime_asset_copy_manifest.json`

Runtime-wired pass24 files:

- `topbar_strip_round06.png`
- `topbar_icon_dark_round06.png`
- `topbar_power_red_round06.png`
- `topbar_tab_dark_round06.png`
- `topbar_tab_red_round06.png`
- `topbar_ticket_round06.png`
- `topbar_coupon_ticket_icon_round09.png`
- `left_panel_round06.png`
- `profile_row_round06.png`
- `search_field_round06.png`
- `section_header_round06.png`
- `friend_row_round06.png`
- `invite_button_green_round06.png`
- `offline_button_dark_round06.png`
- `party_slot_round06.png`
- `filter_panel_round09.png`
- `filter_icon_red_round06.png`
- `filter_icon_dark_round06.png`
- `leaderboard_panel_round06.png`
- `leaderboard_tab_red_round06.png`
- `leaderboard_tab_dark_round06.png`
- `dropdown_dark_round06.png`
- `checkbox_checked_round06.png`
- `checkbox_empty_round06.png`
- `table_header_band_round06.png`
- `ranking_row_red_round06.png`
- `title_logo_round06.png`
- `cta_primary_round06.png`
- `cta_secondary_round06.png`
- `mainmenu_screen_art_mainmenu_newmm_rubbery_friendslop_pass24_1920.png`

Evidence:

- Final capture:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass24_final_capture.png`
- Final dump:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass24_final_dump.json`
- Side-by-side contact sheet:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass24_reference_vs_final_contact_sheet.png`
- Wiring/functionality gate:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass24_wiring_functionality_gate.md`

## Main Menu Pass32 Approved Outer Panel Install

Status: current targeted runtime install for approved outer-panel candidates.
This pass did not run new image generation. It installed the user-approved
Pass31 topbar outer shell and left social panel candidate, and reused the
approved left-panel candidate for `leaderboard_panel_round06.png` because the
Pass31 right-panel candidate was rejected as hollow/no-fill.

- Install manifest:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass32_approved_outer_panel_install\asset_install_manifest.json`
- Approved topbar worker output:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass31_outer_panel_workers\topbar_outer_panel_strict\topbar_outer_panel_candidate.png`
  - Worker output SHA-256: `AAC2D061E9BDDCB5719215EB6285494521F4BCF2F59F056B0DACED4DA8CAA34A`
  - Packaged runtime SHA-256: `3AD856E5AB051D101BB20792E9814DC1F6722D567BCDFEF74E26CC40BC60731B`
- Approved left-panel worker output:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass31_outer_panel_workers\left_outer_panel\left_outer_panel_candidate.png`
  - Worker output SHA-256: `5A619C04E0310654573CF5DB56B5700029031CB99FFDA36B71E74E8633AA9F7B`
  - Packaged runtime SHA-256: `7368A9D5E050310D4F3CF1479D210FF8CA3FA5910F7FDE2CE05ACDE329170635`
- Rejected right-panel worker output, not installed:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass31_outer_panel_workers\right_outer_panel_strict\right_outer_panel_candidate.png`

Runtime-wired pass32 files:

- `topbar_strip_round06.png`
- `left_panel_round06.png`
- `leaderboard_panel_round06.png` (same packaged PNG bytes as
  `left_panel_round06.png`)

## Main Menu Pass22 Five-Family Imagegen Iteration

Status: historical runtime candidate set, superseded by Pass24.

Pass22 regenerated the authoritative reference image and all five Main Menu
visual families through separate local Codex CLI workers using account-backed
built-in imagegen. The pass copied 29 generated PNGs into the runtime
dependency folder and recorded source-to-runtime hashes.

## Main Menu Round06 Pass14 Direct Reference-Derived Plates - Historical Exception

Pass14 used a user-approved exception for direct reference-derived runtime
plates, scoped only to the Main Menu pass14 families. This was not a global
FriendslopStyle permission. Later user review rejected manual/procedural
reconstruction and tightened future work toward imagegen-authored production
pixels. Treat this section as historical unless the current screen contract
explicitly re-accepts a listed asset.

Approved gate:

- Report: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass14_reference_component_gate\pass14_direct_reference_v4_component_gate_report.md`
- Contact sheets:
  - `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass14_reference_component_gate\proof\pass14_direct_reference_v3_gate_contact_sheet.png`
  - `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass14_reference_component_gate\proof\pass14_direct_reference_v4_panel_gate_contact_sheet.png`

Runtime-wired pass14 plate families:

- `left_panel_round06.png`
- `leaderboard_panel_round06.png`
- `cta_primary_round06.png`
- `cta_secondary_round06.png`
- `topbar_icon_dark_round06.png`
- `search_field_round06.png`

## Main Menu Round06 Pass16 Imagegen-Authored Replacements

Status: active runtime candidates for the pass16 screen attempt. These assets
were generated blank by account-backed built-in imagegen, then mechanically
chroma-keyed, alpha-validated, resized, and padded for Slate runtime use. No
reference-crop erasure, inpaint, blur, clone, or local pixel patching was used
to remove text or icons.

Runtime-wired pass16 replacements:

- `title_logo_round06.png`
  - Generated source: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\title_logo_round06_pass16_v3_source_chromakey.png`
  - Alpha source: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\title_logo_round06_pass16_v3_alpha.png`
  - Packaged source: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\title_logo_round06_pass16_v3_packaged_730x100.png`
- `cta_primary_round06.png`
  - Generated source: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\cta_primary_round06_pass16_source_chromakey.png`
  - Alpha source: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\cta_primary_round06_pass16_alpha.png`
  - Packaged source: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\cta_primary_round06_pass16_packaged_680x104.png`
- `topbar_tab_dark_round06.png`
  - Generated source: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\topbar_tab_dark_round06_pass16_source_chromakey.png`
  - Alpha source: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\topbar_tab_dark_round06_pass16_alpha.png`
  - Packaged source: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\topbar_tab_dark_round06_pass16_packaged_292x74.png`
- `topbar_tab_red_round06.png`
  - Generated source: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\topbar_tab_red_round06_pass16_source_chromakey.png`
  - Alpha source: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\topbar_tab_red_round06_pass16_alpha.png`
  - Packaged source: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\topbar_tab_red_round06_pass16_packaged_325x80.png`
- Main Menu poster background
  - Runtime path: `C:\UE\T66\RuntimeDependencies\T66\UI\FriendslopStyle\MainMenu\mainmenu_screen_art_mainmenu_newmm_rubbery_friendslop_pass16_1920.png`
  - Source path: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\mainmenu_screen_art_mainmenu_newmm_rubbery_friendslop_pass15_1920.png`

## Main Menu Round06 Pass17 Imagegen-Authored Topbar Plate

Status: active runtime topbar icon dark plate for the pass17 screen attempt.
Generated by a separate local Codex CLI worker using account-backed built-in
imagegen, then mechanically alpha-packaged to the 96x74 Slate runtime size.

- `topbar_icon_dark_round06.png`
  - Worker request: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass17_workers\topbar_icon_dark\request.md`
  - Corrected worker record/logs: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass17_workers\topbar_icon_dark_attempt2\`
  - Worker output: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass17_workers\topbar_icon_dark\topbar_icon_dark_round06_worker_output.png`
  - Worker output SHA-256: `133aa623f0df1a8a8c590317bcc64e83cab102611b9ae4b24a5bd07ef3f05ac4`
  - Source alpha copy: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\topbar_icon_dark_round06_pass17_source_alpha.png`
  - Packaged source: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\topbar_icon_dark_round06_pass17_packaged_96x74.png`
  - Runtime path: `C:\UE\T66\RuntimeDependencies\T66\UI\FriendslopStyle\MainMenu\topbar_icon_dark_round06.png`
  - Runtime SHA-256: `2E67619A02DEC5462C960007ACFB36A3EC9CB0A9468EEB0933C509E8D27CB5EF`

Pass17 runtime ownership check: `UT66FrontendTopBarWidget` uses
`TopbarIconDarkRound06` for settings/globe icon buttons and
`TopbarPowerRedRound06` for the power button, with live Slate glyph overlays.
`topbar_settings_round06.png`, `topbar_language_round06.png`, and
`topbar_power_icon_round06.png` remain loose historical assets but are not the
current topbar brush path in `FT66FriendslopStyle`.

Pass17 is historical evidence from a superseded one-family pass. Current
five-family visual iteration records live under the pass22 ledger:
`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass22_visual_family_element_assessment.md`.

## Main Menu Round06 Pass18 Imagegen-Authored Topbar Shell

Status: active runtime topbar outer shell for the pass18 screen attempt.
Generated by a separate local Codex CLI worker using account-backed built-in
imagegen, then installed directly at the 1888x100 Slate runtime target size.

- `topbar_strip_round06.png`
  - Worker request: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass18_workers\topbar_strip\request.md`
  - Worker record/logs: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass18_workers\topbar_strip\`
  - Worker output: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass18_workers\topbar_strip\topbar_strip_round06_worker_output.png`
  - Worker output SHA-256: `7A4B93039B3EDD3F8BD47647264AA400AA4B90CD4EFC256265A70B6A5544A3A0`
  - Source alpha copy: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\topbar_strip_round06_pass18_source_alpha.png`
  - Canonical source copy: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\topbar_strip_round06.png`
  - Runtime path: `C:\UE\T66\RuntimeDependencies\T66\UI\FriendslopStyle\MainMenu\topbar_strip_round06.png`
  - Runtime SHA-256: `7A4B93039B3EDD3F8BD47647264AA400AA4B90CD4EFC256265A70B6A5544A3A0`

Pass18 is historical evidence from a superseded ledger pass. The current active
process uses five visual families with visual `PASS`/`FAIL`, followed by layout
and wiring `PASS`/`FAIL`. Pass18 gaps are tracked in:
`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass18_manifest_lane_ledger.md`.

## Main Menu Round06 Pass12 Reference-Crop Fixed Plates - Diagnostic Only

Pass12 replaced the pass11 deterministic blank plates with fixed-size plates
derived from the corresponding Round06 reference crops. It is no longer
authoritative for acceptance. Do not use the pass12 generator as the production
base for new runtime plates.

The pass12 generator is
`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass12_generate_reference_inpaint_plates.py`.
The script writes source copies, project runtime copies, and staged-build
runtime copies so `T66.exe` captures load the current assets.

Specs and proof:

- Fixed-plate spec: `C:\UE\T66\UI\FriendslopStyle\Archive\DeprecatedSliceSpecs\main_menu_round06_pass12_reference_inpaint_plate_specs.md`
- Plate contact sheet: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass12_reference_inpaint_plate_contact_sheet.png`
- Reference plate comparison: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass12_reference_inpaint_plate_comparison.png`
- Fixture capture: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_fixture_capture.png`
- Fixture verifier: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_fixture_fidelity.md`
- Fixture visual scorecard: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_fixture_visual_scorecard.md`
- Fixture result: `PASS=251 FAIL=0 UNSURE=0`

## Main Menu Round06 Pass11 Fixed Plates

Pass11 replaces the pass10 clean-sheet composite plates with deterministic
fixed-size blank plates generated by
`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass11_generate_fixed_plates.py`.
The script writes source copies, project runtime copies, and staged-build
runtime copies so `T66.exe` captures load the current assets.

Pass11 is retained as structural evidence only. Claude cross-review rejected it
as incomplete for Round06 material fidelity because the plates were generic
blank controls rather than reference-region-derived chrome.

Specs and proof:

- Fixed-plate spec: `C:\UE\T66\UI\FriendslopStyle\Archive\DeprecatedSliceSpecs\main_menu_round06_pass11_fixed_plate_specs.md`
- Plate contact sheet: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass11_fixed_plate_contact_sheet.png`
- Reference plate comparison: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass11_reference_plate_comparison.png`
- Fixture capture: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass11_fixture_capture.png`
- Fixture verifier: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass11_fixture_fidelity.md`
- Fixture result: `PASS=251 FAIL=0 UNSURE=0`

| Asset | Source Path | Runtime Path | Owner |
|---|---|---|---|
| `panel_large_dark.png` | `SourceAssets/UI/FriendslopStyle/MainMenu/panel_large_dark.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/panel_large_dark.png` | `FT66FriendslopStyle` |
| `button_long_dark.png` | `SourceAssets/UI/FriendslopStyle/MainMenu/button_long_dark.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/button_long_dark.png` | `FT66FriendslopStyle` |
| `button_primary_red.png` | `SourceAssets/UI/FriendslopStyle/MainMenu/button_primary_red.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/button_primary_red.png` | `FT66FriendslopStyle` |
| `button_action_green.png` | `SourceAssets/UI/FriendslopStyle/MainMenu/button_action_green.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/button_action_green.png` | `FT66FriendslopStyle` |
| `pill_dark.png` | `SourceAssets/UI/FriendslopStyle/MainMenu/pill_dark.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/pill_dark.png` | `FT66FriendslopStyle` |
| `row_dark.png` | `SourceAssets/UI/FriendslopStyle/MainMenu/row_dark.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/row_dark.png` | `FT66FriendslopStyle` |
| `row_selected_red.png` | `SourceAssets/UI/FriendslopStyle/MainMenu/row_selected_red.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/row_selected_red.png` | `FT66FriendslopStyle` |
| `party_slot_dark.png` | `SourceAssets/UI/FriendslopStyle/MainMenu/party_slot_dark.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/party_slot_dark.png` | `FT66FriendslopStyle` |
| `icon_button_dark.png` | `SourceAssets/UI/FriendslopStyle/MainMenu/icon_button_dark.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/icon_button_dark.png` | `FT66FriendslopStyle` |
| `small_square_dark.png` | `SourceAssets/UI/FriendslopStyle/MainMenu/small_square_dark.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/small_square_dark.png` | `FT66FriendslopStyle` |
| `checkbox_checked_red.png` | `SourceAssets/UI/FriendslopStyle/MainMenu/checkbox_checked_red.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/checkbox_checked_red.png` | `FT66FriendslopStyle` |
| `checkbox_empty_dark.png` | `SourceAssets/UI/FriendslopStyle/MainMenu/checkbox_empty_dark.png` | `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/checkbox_empty_dark.png` | `FT66FriendslopStyle` |

## Runtime Rule

Do not use the full-screen reference image as runtime UI. Runtime UI must keep
text, icons, portraits, score data, counts, hover/click state, and localization
live. Chrome quality should come from transparent authored plates or plate
families matched to the approved reference regions; generic reusable atoms are
allowed only when the current process records reference/capture/contact evidence
and the user accepts them for the runtime size.

