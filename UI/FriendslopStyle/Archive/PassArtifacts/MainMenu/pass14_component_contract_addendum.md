# FriendslopStyle Main Menu Pass14 Component Contract Addendum

Reference: `UI/FriendslopStyle/Reference/MainMenu/Round06/main_menu_reference_01_current_capture_stronger_rubber_cli.png`

Base contract: `UI/FriendslopStyle/Archive/PassArtifacts/MainMenu/pass13_component_contract.md`

This addendum narrows pass14 around the pass13 visual failures. It does not
declare the Main Menu complete. A pass14 result is accepted only as far as the
current capture, crop/contact sheet, verifier report, and visual scorecard prove.

## Corrected Pass14 Extraction Rule

Pass14 runtime plates must be generated or selected against exact reference
crops, not invented from a freeform component sheet.

### User-Approved Pass14 Exception - 2026-06-06

The user approved option 1 from
`Reports/AgentReviews/20260605_FriendslopProductionRedo/decision_block.md`:
direct reference-derived runtime plates are allowed for the Main Menu pass14
families, with strict gates and no API/CLI fallback.

For this Main Menu pass14 only, this user decision supersedes the earlier
handoff prohibition at
`Reports/AgentReviews/20260605_FriendslopProductionRedo/fresh_agent_main_menu_pass14_prompt_final.md`
line 115 against creating runtime plates from reference crops. It does not
supersede the plate-vs-live-content ownership rules below.

This exception is narrow:

- crop-derived plates with live-content zones removed are allowed only when the
  component gate passes;
- raw unmodified reference crops remain forbidden as runtime plates;
- built-in account-backed imagegen remains allowed, but is not required for this
  direct reference-derived path;
- CLI/API/`OPENAI_API_KEY` fallback remains forbidden;
- this does not create a global FriendslopStyle permission for future screens.

- Reference crops are gate targets and comparison artifacts only.
- Content masks identify regions that Slate will own at runtime. Under the
  2026-06-06 exception, the crop-derived runtime plate may preserve unmasked
  reference chrome pixels and locally replace masked live-content zones.
- Built-in account-backed imagegen may produce a blank candidate after the
  reference crop has been loaded into context, but the generated candidate must
  pass the component gate before runtime wiring.
- If the crop-derived path cannot produce a candidate that passes silhouette,
  material, and no-baked-content gates, stop for a user decision. Do not switch
  to CLI/API/`OPENAI_API_KEY`, and do not ship a raw crop/masked lookalike.

## Ownership Rules

- Runtime chrome plates own only blank rubber/chrome material: silhouette, bevel,
  gloss, shadow, outline, and fill.
- Slate owns all title text, labels, glyphs/icons, player names, scores, counts,
  online/offline dots, checkboxes, row states, localization, and interaction.
- Runtime plates must not contain baked text, skulls, settings/language/power
  glyphs, player avatars, score text, checkbox marks, row data, or painted-over
  removal artifacts.
- A generated or reused plate is not eligible for runtime wiring until the
  reference-derived component gate passes: alpha, crop, no-baked-content,
  silhouette/edge similarity, material similarity, and manual visual review.

## Component Gate Artifacts

Each primary plate family needs this artifact set before runtime wiring:

- exact reference crop;
- content-removal mask and mask overlay;
- generated blank candidate source;
- final alpha candidate at runtime descriptor size;
- checkerboard alpha preview;
- reference-vs-candidate difference/contact view;
- component gate report with explicit PASS/FAIL status.

Initial pass14 gate packet:

`Saved/Codex/UI/FriendslopStyle/MainMenu/pass14_reference_component_gate/`

## Pass14 Required Rows

| Component family | Runtime owner | Required pass14 rule | Verification row/evidence |
|---|---|---|---|
| Title branding | Slate live title, not cropped bitmap | `MainMenu.Center.Title` must be contained inside `MainMenu.Center.TitleRegion`; if a PNG title is used, it must be transparent, uncropped, and validated. | `title_not_cropped`; `MainMenu.Center.Title contained_in=MainMenu.Center.TitleRegion`; visual crop |
| Subtitle | Slate live text | Fit inside title region with ellipsis or down-only scaling; no overlap with title or CTA. | `MainMenu.Center.Subtitle contained_in=MainMenu.Center.TitleRegion`; dump geometry |
| CTA primary | Blank `cta_primary_round06.png` chrome plus live label/icons | No baked label, skull, highlight smear, or painted-over center; live left/right icons stay inside the button. | CTA crop; label/icon containment rows |
| CTA secondary | Blank `cta_secondary_round06.png` chrome plus live label/icons | Same ownership rule as CTA primary; disabled/default state remains dark, not red fill. | CTA crop; label containment rows |
| Topbar icon plate | Blank `topbar_icon_dark_round06.png` chrome plus live glyph | Must not reuse filter icon silhouette as a final topbar plate unless the visual scorecard accepts it; no baked glyph under live glyph. | `topbar_icon_ownership_and_shape`; topbar crop |
| Search pill | Blank `search_field_round06.png` chrome plus live editable text/icon | Pill must be rounded/thick enough to read like the reference; no stretched smear or fake baked search text. | `search_pill_match`; `MainMenu.Left.SearchField contained_in=MainMenu.Left.Panel` |
| Left side panel | Blank `left_panel_round06.png` chrome plus live rows | Frame must read as the reference side-panel rubber shell, not a flat dark card; rows stay inside. | `left_panel_frame_match`; row containment rows |
| Right leaderboard panel | Blank `leaderboard_panel_round06.png` chrome plus live controls/rows | Frame must read as the reference leaderboard shell; filter buttons and rows stay contained. | `right_panel_frame_match`; leaderboard containment rows |
| Local leaderboard row | Slate/plate state convention | Red outline with dark interior; do not regress to red fill. | `leaderboard_local_row_style` |
| Checkbox state | Slate state convention | Uniform rounded square; no clipped old crop state. | `metric_checkbox_shape` |
| Online/invite states | Slate live state | Online dot green, offline dot gray, invite button visibly green. | `online_header_dot`; `invite_green_state` |

## Containment Contract

The verifier checklist must include explicit `contained_in` rows for the pass14
failure classes:

- `MainMenu.Center.Title` in `MainMenu.Center.TitleRegion`.
- `MainMenu.Center.Subtitle` in `MainMenu.Center.TitleRegion`.
- `MainMenu.Center.EnterTribulationButton.Label` in `MainMenu.Center.EnterTribulationButton`.
- `MainMenu.Center.EnterTribulationButton.LeftIcon` and `.RightIcon` in `MainMenu.Center.EnterTribulationButton`.
- `MainMenu.Center.LoadGameButton.Label` in `MainMenu.Center.LoadGameButton`.
- `MainMenu.Left.OnlineFriendRow01.ActionButton` and `.ActionText` in `MainMenu.Left.OnlineFriendRow01`.
- `MainMenu.Left.OfflineFriendRow01.ActionButton` and `.ActionText` in `MainMenu.Left.OfflineFriendRow01`.
- `MainMenu.Right.RankingRowLocal.Avatar` in `MainMenu.Right.RankingRowLocal`.

## Pass14 Acceptance Notes

- `VerifyUIFidelity.py` reporting `FAIL=0` is not enough if
  `pass14_visual_scorecard.md` says `Result: FAIL`.
- The visual scorecard must include at least: `title_not_cropped`,
  `topbar_icon_ownership_and_shape`, `achievements_text_fit`,
  `search_pill_match`, `cta_clean_blank_plate`, `left_panel_frame_match`,
  `right_panel_frame_match`, `leaderboard_local_row_style`,
  `metric_checkbox_shape`, `online_header_dot`, `invite_green_state`, and
  `row_containment`.
- Responsive and manual-interaction gates remain open until current pass14
  evidence is produced. If they are skipped, the final report must say so.
