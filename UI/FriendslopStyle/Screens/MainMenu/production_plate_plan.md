# FriendslopStyle Main Menu Round06 Production Plate Plan

Status: current Main Menu production plan, revised 2026-06-06 after the
no-manual-visual-authoring correction.

Reference: `C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Current\main_menu_reference_04_foreground_right_panel_icon_filters_coupon_cli.png`

Invalidated prototype: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass09_capture.png`

Failure review: `C:\UE\T66\Reports\AgentReviews\20260605_FriendslopFailureRetrospective\codex_failure_review_draft.md`

## Decision Record

- The old pass09 method of tuning Slate constants over generic pilot rubber atoms is rejected.
- Runtime chrome must use Round06-matched production plates, either
  size-specific or slice-proven, authored through account-backed built-in
  imagegen in a separate local Codex CLI worker unless a current screen
  contract documents a user-approved exception.
- For the current approved reference, generation starts from a CLI-worker
  textless reference breakdown. Each failed family worker receives the textless
  family crop and outputs a contact sheet plus individualized backgroundless PNG
  files. Manual cropping from family sheets is not a runtime asset source.
- `CHADPOCALYPSE` is treated as branding, but it must not be cropped from the
  full reference. It must be a clean title-only generated asset or a measured
  live Slate title treatment. Subtitle and button labels remain live Slate text.
- Deterministic reference-fidelity capture is required as visual evidence for
  user review. Current live Steam/backend data is used only for
  robustness/overflow checks.

## PPF Check

```text
PPF CHECK
Objective: Rebuild FriendslopStyle Main Menu from reference-matched per-element/per-size plates, deterministic fidelity fixture data, and user-reviewed visual evidence.
Proven process: UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md plus UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md, with the generic pilot-plate loophole closed.
My planned implementation: Author production plates through account-backed built-in imagegen in separate local Codex CLI workers using Round06 crops only as measurement/comparison targets, prove each plate in a source/runtime contact sheet, integrate the accepted plate set, preserve live data/text where required, then produce current capture/dump/contact evidence, sizing/fitting notes, and a wiring/functionality PASS/FAIL gate.
Same method class: YES
If NO, why: N/A
User approval required before proceeding: NO. User explicitly approved following the replacement process.
Verification evidence: plate contact sheet, fixture capture/dump, live-data robustness capture, side-by-side/contact evidence, worker records, sizing/fitting notes, wiring/functionality gate, and optional structural verifier output when useful.
```

## Deterministic Fixture Requirement

Reference parity capture must run with deterministic data:

- profile name `Solobro`
- ticket count `53`
- one online friend row matching the Round06 topology
- four offline friend rows matching the Round06 topology
- one local leaderboard row with rank/name/score topology
- stable avatars or production-equivalent content stubs

Implementation may add a frontend automation/fixture path if no existing one can force this data. A separate live-data capture must prove current local data does not overflow rows, panels, or controls.

## Production Plate Matrix

Each row must pass source/runtime comparison against the Round06 measurement
target before it can be used for acceptance. Reference crops are comparison
artifacts only, not runtime plate sources.

| Plate ID | Reference target bbox 1920 | Runtime asset | Use | Plate class | Live overlay | Acceptance notes |
|---|---:|---|---|---|---|---|
| `topbar_strip_round06` | `(16,12,1888,100)` | `topbar_strip_round06.png` | top bar outer strip | size-specific | controls live | Keep full strip silhouette and shadow; no text. |
| `topbar_icon_dark_round06` | `(24,24,96,74)` | `topbar_icon_dark_round06.png` | settings/language buttons | fixed-size | live icon glyphs | Blank icon plate, no baked gear/globe. |
| `topbar_tab_dark_round06` | `(252,24,292,74)` | `topbar_tab_dark_round06.png` | ACCOUNT/POWER/ACHIEVEMENTS tabs | horizontal-slice candidate | live labels | Must keep thick bevel and black body at tab height. |
| `topbar_tab_red_round06` | `(592,24,325,80)` | `topbar_tab_red_round06.png` | HOME selected tab | horizontal-slice candidate | live label | Red fill, strong bevel; no baked HOME. |
| `topbar_ticket_round06` | `(1572,24,172,74)` | `topbar_ticket_round06.png` | ticket badge plate | size-specific | live count, separate ticket icon | Value must stay live; pass24 coupon ticket icon is a separate runtime image. |
| `topbar_coupon_ticket_icon_round09` | `(1618,50,40,28)` | `topbar_coupon_ticket_icon_round09.png` | coupon ticket icon | fixed-size | no text | Classic fair-ticket icon; no currency value baked. |
| `topbar_power_red_round06` | `(1776,24,96,74)` | `topbar_power_red_round06.png` | power button | fixed-size | live or baked icon | Static power glyph is acceptable as control icon. |
| `left_panel_round06` | `(18,145,500,892)` | `left_panel_round06.png` | left social shell | size-specific | all children live | Must match panel thickness and interior darkness. |
| `profile_row_round06` | `(40,181,460,108)` | `profile_row_round06.png` | profile row | size-specific | avatar/name/progress live | Text/progress areas blanked. |
| `search_field_round06` | `(40,301,460,60)` | `search_field_round06.png` | friend search field | size-specific | editable text live | Blank field, no placeholder baked. |
| `section_header_round06` | `(40,372,460,42)` | `section_header_round06.png` | ONLINE/OFFLINE headers | size-specific | label/count live | Blank header with left indicator left to Slate. |
| `friend_row_round06` | `(40,420,460,58)` | `friend_row_round06.png` | friend rows | size-specific | avatar/name/level/fav/action live | Must keep row content inside panel. |
| `invite_button_green_round06` | `(337,373,80,45)` | `invite_button_green_round06.png` | online invite action | size-specific | label live | Green button must match reference density. |
| `offline_button_dark_round06` | `(337,486,80,45)` | `offline_button_dark_round06.png` | offline action | size-specific | label live | Disabled/dark button family. |
| `party_slot_round06` | `(52,905,94,94)` | `party_slot_round06.png` | party slots | fixed-size | avatar/plus live | Use fixed image; no 9-slice. |
| `title_logo_round06` | `(590,124,730,100)` | `title_logo_round06.png` | CHADPOCALYPSE title | title-only generated branding asset or live Slate title | no full-reference crop | Must be uncropped and free of subtitle/background fragments. |
| `cta_primary_round06` | `(640,748,680,104)` | `cta_primary_round06.png` | ENTER TRIBULATION | size-specific | label live, skulls may be baked or live | No baked label. |
| `cta_secondary_round06` | `(650,884,660,94)` | `cta_secondary_round06.png` | LOAD GAME | size-specific | label live | No baked label. |
| `filter_panel_round09` | `(1350,130,516,90)` | `filter_panel_round09.png` | right filter panel shell | size-specific | filter icons live | Horizontal icon panel above leaderboard body. |
| `filter_icon_red_round06` | `(1374,143,150,64)` | `filter_icon_red_round06.png` | selected right filter | size-specific | icon live | No baked GLOBAL/SOCIAL/STREAMERS text. |
| `filter_icon_dark_round06` | `(1542,143,150,64)` | `filter_icon_dark_round06.png` | unselected right filters | size-specific | icons live | No baked GLOBAL/SOCIAL/STREAMERS text. |
| `leaderboard_panel_round06` | `(1350,234,516,790)` | `leaderboard_panel_round06.png` | right leaderboard shell | size-specific | all content live | Must sit in front of background/statue. |
| `leaderboard_tab_red_round06` | `(1370,312,232,52)` | `leaderboard_tab_red_round06.png` | WEEKLY selected | size-specific | label live | No baked text. |
| `leaderboard_tab_dark_round06` | `(1614,312,232,52)` | `leaderboard_tab_dark_round06.png` | ALL TIME default | size-specific | label live | No baked text. |
| `dropdown_dark_round06` | `(1370,372,232,52)` | `dropdown_dark_round06.png` | party/difficulty dropdowns | size-specific | label/arrow live | No baked text. |
| `checkbox_checked_round06` | `(1370,436,28,28)` | `checkbox_checked_round06.png` | high-score metric checkbox | fixed-size | label live | Checkbox image only; label live. |
| `checkbox_empty_round06` | `(1614,436,28,28)` | `checkbox_empty_round06.png` | speed-run metric checkbox | fixed-size | label live | Checkbox image only; label live. |
| `table_header_band_round06` | `(1370,531,476,29)` | `table_header_band_round06.png` | table header area | size-specific | RANK/NAME/SCORE live | No baked table labels. |
| `ranking_row_red_round06` | `(1370,568,476,46)` | `ranking_row_red_round06.png` | local ranking row | size-specific | rank/avatar/name/score live | No baked rank/name/score. |

## Gate Before Runtime Integration

- Every `Runtime asset` exists in source and runtime roots.
- Each source plate has an alpha channel or a documented fixed opaque generated
  asset class.
- A contact sheet shows every plate at target runtime size beside the matching
  textless family crop or Round06 measurement crop.
- The runtime asset path for each element points to an individualized worker
  output, not a manual crop from a family sheet.
- The contact sheet is inspected before code edits.
- Any row that fails material, silhouette, or containment in the contact sheet is
  regenerated or repackaged before Unreal integration. Repackaging may only
  crop/alpha/slice already-approved generated pixels; it may not patch visual
  content.

