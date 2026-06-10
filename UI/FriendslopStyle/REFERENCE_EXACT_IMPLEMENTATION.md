# Reference-Exact Implementation Loop (canonical, v1 — 2026-06-10)

The canonical process for turning an APPROVED full-screen reference into the live screen
so the capture matches the reference. Supersedes the family-breakdown loop as the
implementation procedure for reimagined screens (the approval exercise that produces the
reference is documented in Saved/Codex/UI/UIReimagine/approvals/rules.md).

Born from the MainMenu hellfire pass-1 failure analysis. Two systemic causes of drift:
(1) NO COVERAGE CONTRACT — elements get silently skipped (pass 1 left the whole top bar
on old chrome); (2) NO NUMERIC GEOMETRY TRANSFER — offsets get eyeballed (pass 1's layout
deviated everywhere). The loop below makes both impossible.

## The loop

1. MEASURE — geometry.md
   Measure EVERY visible element's bbox in the approved reference (its native px), store
   normalized fractions. Slate consumes the table: normalized rects for canvas screens
   (e.g. FNormalizedTopBarRect tables), or px-at-1920x1080 constants derived from it.
   NEVER hand-eye an offset. Tolerance ±0.01 normalized, refined by the diff step.

2. COVER — coverage_matrix.md
   One row per visible element: element -> { widget (existing tag or NEW), asset file,
   geometry row, status }. Rules: NO unmapped element may exist when coding starts; rows
   are only DONE / PARTIAL(reason) / DEFERRED(user-approved). "I restyled what I noticed"
   is the failure mode this kills. The screen is done when every row is DONE.

3. ASSET-EXACT — crop-anchored generation
   For each plate/icon: mechanically CROP the element's region from the reference
   (Pillow), attach THE CROP (not the full screen) to the imagegen worker, instruct
   "reproduce this exact surface, empty, transparent". Full-screen attachments dilute
   color fidelity (pass-1 panel_side drifted red). QA every output: alpha bounds, size,
   no baked text. Live text/data is NEVER baked.

4. CODE FROM TABLE
   All sizes/offsets come from geometry.md. Reuse existing handlers/bindings (reskin
   preserves behavior). New behavior only per the screen's approved decisions ledger.

5. VERIFY-DIFF
   Stage -> capture -> same-scale side-by-side vs reference + walk the coverage matrix
   row by row marking DONE/FAIL. Fix FAIL rows only; re-capture. 2-iteration cap per
   element, then defer with note.

6. RECORD
   Update the matrix statuses, commit code+assets separately, log decisions. Promote
   reusable plates into the shared kit folder when a second screen needs them.

## THE THREE LAWS (from MainMenu pass-1/2 failure analysis — non-negotiable)

LAW 1 — SIZE-EXACT PLATES. Every fixed-size element gets a plate generated at its exact
runtime pixel size (2x for sharpness) and rendered DrawAs=Image with FMargin(0).
Box-slicing is ONLY for genuinely variable-size elements, and the protected caps must be
<= 40% of the MINIMUM runtime dimension — verify numerically (plate_px * margin vs
target_px) BEFORE coding. Cap-budget violations are the #1 source of "cut off /
stretched" UI (pass-2 CTAs: 45px+45px caps in a 94px button = 4px center band).

LAW 2 — VERIFY THE BASIS. Before writing any normalized rect, read the actual canvas
basis constants (e.g. T66MainMenuReferenceLayout::CanvasWidth/Height). Never assume.

LAW 3 — ZOOM-CROP VERIFICATION. Full-frame screenshots hide region defects. Before
marking any matrix row DONE (and before showing the user anything), crop+zoom each
region from the capture and compare against the same crop of the reference. Judge
regions, not frames. Plates with baked glyphs/marks = regenerate, never ship.

LAW 8 — MEASURE-BACK (geometry never judged by eye). Every verification capture runs
with -T66AutoDumpScreen. A diff script compares each tagged widget's ACTUAL rect from
the dump against the reference geometry table and emits numeric per-element deltas.
An element passes geometry only when |dx|,|dy|,|dw|,|dh| <= 12px at 1920x1080. Claude's
eyes are forbidden from grading position/size.

LAW 9 — SAME-SCALE A/B PAIRS (material judged only in juxtaposition). For
material/color/proportion qualities (rim weight, fills, fusion of connected panels,
state treatments), every region verdict requires a paired image: reference crop and
capture crop at the SAME pixel scale, adjacent in one image, large enough to judge
(no downscaled full-frame sheets, no solo crops). Grade against the REFERENCE pair,
never against the previous pass.

LAW 10 — GRIDDED REFERENCE MEASUREMENT. The reference geometry table is read off a
grid-overlaid copy of the reference (labeled lines every 32/64px), not estimated from
memory of the image.

## Asset conventions
- Runtime: RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/<Screen>/ (new files only;
  never overwrite another era's assets — revert stays one CVar/commit away).
- Worker records per asset batch (request, stdout, last_message, exit code) under
  Saved/Codex/UI/UIReimagine/impl_workers/.
- 9-slice margins start from family defaults (panel 0.10/0.08, row 0.12/0.30,
  cta 0.18/0.30, pill 0.22/0.30, tab 0.14/0.26) and are tuned by the diff step.

## Identity
Hellfire (locked): charcoal/black surfaces, ember rims, lava red = the one hero action +
selected + YOU row, flame gold = single accent, warm-white chunky text (Lilita One).
Green ONLY for online status/invites. Icons may be colorful; chrome may not.
Inflatable vinyl only — no stitches, no corner bolts.
