# Reference Transplant Process (v1 — 2026-06-10)

The process for translating an approved reference image into the live screen with true
fidelity. Supersedes the description-based generation in REFERENCE_EXACT_IMPLEMENTATION.md
(its geometry laws 1-10 still apply; its asset-generation stage is replaced by this).

Built from the user's diagnosis of all prior failures:
- Imagegen's strength is EXTRACTING elements from images. Description-based generation
  produces flat non-material art no matter how the prompt is worded. NEVER describe.
- Claude's visual discernment is trusted and must be USED: every asset and every capture
  is judged by eye at native scale. Numeric QA alone passes flat garbage.
- Prioritization/deferral is forbidden: a single completion gate blocks ANY report to
  the user while ANY row is not DONE.

## Pipeline

STAGE 1 — EXTRACT (one element per generation request)
  a. Crop the element's bbox (+6px margin) from the reference mechanically (geometry
     table drives the boxes).
  b. Worker prompt = the crop attached + minimal edit instruction:
     "Recreate this EXACT image as a transparent-background PNG sprite: identical
      surface, colors, rim, gloss, proportions. Remove <the text/icon/avatar>; surface
      only. Do not redesign. Do not change palette or rim thickness."
     NO size requirements, NO style words, NO material adjectives. The crop is the spec.
  c. Mechanical post: alpha-trim -> store. (Slate draws Image at the measured rect; the
     asset's aspect equals the rect's by construction.)

GATE A — ASSET EYES (blocking, per element)
  Pair image [reference crop | extracted asset] at native scale (pair <= ~700px wide).
  Verdict by eye: same object? same material (inflatable vinyl, gloss, rim weight)?
  same colors? content removed? Not pixel-identical — READ-identical.
  FAIL -> re-extract (max 2 retries) -> still FAIL -> show the user the pair and ask.
  Verdict recorded in gate_status.md. No asset is placed without PASS.

STAGE 2 — PLACE
  Code places each asset at its measured rect (instrument numbers), Image-drawn,
  FMargin(0). Fonts from measured text sizes. Structural items (e.g. cap inside panel)
  are rows in the gate file like everything else.

GATE B — SCREEN EYES (blocking, per segment)
  Build + stage + capture. Cut the SAME segment from capture and reference, pair them
  (<= 700px wide). Walk EVERY element in the segment against the checklist — not just
  elements changed this round. Numeric instrument diff runs alongside (<=12px).
  Any FAIL -> fix -> recapture -> re-walk the WHOLE segment.

GATE C — COMPLETION (blocking, global)
  gate_status.md holds one row per element/step: PENDING | PASS | FAIL | USER-SKIP
  (USER-SKIP requires the user's quoted decision). Reporting to the user is FORBIDDEN
  while any row is PENDING or FAIL. The report includes the gate table.

## Folder layout (per screen)
  transplant/<Screen>/crops/      mechanical reference crops (inputs)
  transplant/<Screen>/extracted/  imagegen outputs (post alpha-trim)
  transplant/<Screen>/gates/      pair images viewed for Gate A/B + gate_status.md
  transplant/<Screen>/workers/    worker records (request, stdout, exit, last_message)
Runtime assets stay at RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/<Screen>/
(same filenames, overwritten; git history is the revert path).
