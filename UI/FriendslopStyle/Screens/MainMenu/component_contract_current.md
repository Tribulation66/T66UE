# FriendslopStyle Main Menu Current Component Contract

Reference:
`UI/FriendslopStyle/Reference/MainMenu/Current/main_menu_reference_04_foreground_right_panel_icon_filters_coupon_cli.png`

Archived prior contracts:

- `UI/FriendslopStyle/Archive/PassArtifacts/MainMenu/pass13_component_contract.md`
- `UI/FriendslopStyle/Archive/PassArtifacts/MainMenu/pass14_component_contract_addendum.md`

This is the active Main Menu component contract. It supersedes pass-numbered
contracts for future work unless a new current contract replaces it.

## Visual Asset Provenance

- Runtime plates may contain only blank rubber/chrome material: silhouette,
  bevel, gloss, shadow, outline, and fill.
- Production visual pixels for title art, chrome plates, and background art are
  authored by account-backed built-in imagegen run in a separate local Codex CLI
  worker, or by a separately documented user-approved exception.
- Do not generate Main Menu iteration assets in the main Codex app chat. Do not
  use `OPENAI_API_KEY`, OpenAI API scripts, web image URLs, browser screenshots,
  old generated-image folders, or manual pixel repair as substitutes for a
  fresh CLI worker.
- Each accepted generated asset must have a worker record: request/prompt,
  logs, final status, output PNG path, and token/hash data when exposed.
- Each approved full-screen reference must have a generated textless reference
  breakdown produced by a separate local Codex CLI worker. The textless
  breakdown is cropped into family contexts for generation input only; it is not
  runtime chrome and is not a source for manual runtime extraction.
- Runtime family worker prompts must be extraction-only and must not contain
  descriptive/adjectival material, shape, color, vibe, polish, or
  game-comparison language. See
  `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`
  Section 2.2.1 for the full Allowed/Forbidden definition. The textless family
  crop is the only visual style authority; prompt text may list elements,
  output paths, alpha/canvas requirements, and no-baked-content rules only.
- Reference crops are comparison and measurement targets only. They are not
  runtime asset sources.
- Family contact sheets are evidence only. Runtime assets must come from
  individualized backgroundless PNGs emitted by the family worker. Cropping is
  allowed only to create textless family contexts, reference measurement
  targets, or to trim an already-approved individual generated candidate.
- Alpha extraction is allowed only to remove matte/chroma from a blank generated
  candidate or to validate alpha. It may not lift a region from the full
  reference into runtime.
- Contact sheets are evidence only. Nothing on a contact sheet is imported.
- If a candidate includes text fragments, icons, player data, title fragments,
  smears, pillows, masks, local discontinuities, or patched content zones, it
  fails and must be regenerated or replaced.

## Live Ownership

Slate owns all title text when live title is used, subtitle, labels, glyphs,
icons, player names, scores, counts, online/offline dots, checkbox state,
localization, hover/pressed/disabled states, and interaction behavior.

The title must not be cropped from the full reference. It must be either a clean
title-only generated asset that passes alpha/no-crop/no-content gates or a live
Slate title treatment with measured containment.

## Current Required Rows

`element_manifest.md` is the active full-screen ledger. Every implementation
pass must classify the five Main Menu visual families as visual `PASS` or visual
`FAIL`. For every visual `FAIL` family, the pass then classifies each element
inside that family as visual `PASS` or visual `FAIL`, launches one CLI imagegen
worker for that family, and implements all generated assets. The rows below are
element-level contract reminders inside those families, not a replacement for
the five-family manifest ledger.

| Component family | Runtime owner | Rule | Verification evidence |
|---|---|---|---|
| Title branding | Generated title-only asset or live Slate title | No full-reference crop; no clipping; contained in title region. | `title_not_cropped`; title crop/contact view |
| Subtitle | Slate live text | Contained in title region; no overlap with title or CTA. | dump containment |
| CTA primary | Blank generated chrome plus live label/icons | No baked skulls/text, no pillow center, no manual patching. | `cta_clean_blank_plate`; label/icon containment |
| CTA secondary | Blank generated chrome plus live label/icons | Dark/default state remains blank chrome with live label. | crop/contact view |
| Topbar icon plate | Blank chrome plus live glyph | No baked glyph under live glyph; shape matches topbar family. | `topbar_icon_ownership_and_shape` |
| Search pill | Blank chrome plus live icon/text | No fake baked search text; no flat/thin or smeared center. | `search_pill_match` |
| Left panel | Blank panel chrome plus live rows | Rubber frame reads like reference; rows contained inside content inset. | `left_panel_frame_match`; row containment |
| Right panel | Blank panel chrome plus live controls/rows | Rubber frame reads like reference; rows and controls contained. | `right_panel_frame_match` |
| Local leaderboard row | Slate/plate state convention | Red outline with dark interior; not red fill. | `leaderboard_local_row_style` |
| Checkbox | Slate/plate state convention | Uniform rounded square state. | `metric_checkbox_shape` |
| Online/invite | Slate live state | Online dot green, offline dot gray, invite green. | `online_header_dot`; `invite_green_state` |

## Containment Contract

The verifier checklist must include `contained_in` rows for:

- `MainMenu.Center.Title` in `MainMenu.Center.TitleRegion`.
- `MainMenu.Center.Subtitle` in `MainMenu.Center.TitleRegion`.
- CTA labels/icons in their owning CTA buttons.
- Friend row action buttons/text in their owning rows.
- Friend rows inside the left panel content area.
- Leaderboard controls and local row contents inside the right panel content
  area.

## Acceptance

Friendslop Main Menu acceptance is no longer expressed as `FULL`, `PARTIAL`, or
a visual scorecard `Result`. Codex reports objective process coverage and the
wiring/functionality gate only.

Required final evidence:

- reference and produced capture/contact sheet for user visual review;
- five-family assessment and failed-family element breakdown;
- one CLI worker record per failed visual family;
- generated asset implementation paths;
- sizing/fitting work performed and blockers, if any;
- wiring/functionality gate: `PASS` or `FAIL`.

`VerifyUIFidelity.py` may be used as optional technical evidence for structure,
containment, or wiring metadata, but its numeric count does not control
Friendslop visual acceptance.

