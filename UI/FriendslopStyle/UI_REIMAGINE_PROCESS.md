# UI Reimagine — Canonical Process (v2, consolidated 2026-06-10)

THE single authoritative process for translating an approved reference image into the
live screen with true fidelity. Supersedes and consolidates:
  - REFERENCE_EXACT_IMPLEMENTATION.md  (archived: Archive/)
  - Saved/Codex/UI/UIReimagine/transplant/TRANSPLANT_PROCESS.md (archived alongside)
The reference-APPROVAL exercise (producing the reference with the user) remains
documented at Saved/Codex/UI/UIReimagine/approvals/rules.md.

## Phase 0 — Reference + ground truth
1. Approved reference normalized to 1920x1080 (reference_1920.png).
2. EVERY geometry number is read off annotated-scale strips: wide-short crops
   (<=720px tall renders) with labeled pixel grid, viewed at native scale. NEVER
   eyeball from full frames; NEVER trust instrument windows that presuppose positions
   (circular measurement). A crop boundary is a CLAIM about where an element ends —
   validate it against the RAW reference at zoom before it becomes an asset.

## Phase 0.5 — Reference generation prompt rules (learned HeroSelection 2026-06-10)
Every reference-generation prompt MUST include:
- Style anchor: attach the latest APPROVED screen reference as the style authority.
- Component contract: enumerate every component from the live screen's components.md.
- ANTI-OVERLAP CLAUSE (verbatim-class language): "No element may overlap or touch any
  other element or its containing panel's rim. Every button/dropdown/row sits fully
  inside its panel with visible margin on all sides. PANEL BORDERS MUST NOT TOUCH:
  every pair of panels has a clear dark gap between their rims, including vertically
  stacked panels in the same column. Resize or shrink elements/panels as needed to
  guarantee this." Overlapping or rim-touching reference art creates impossible
  geometry downstream — reject at approval if present.
- Anti-distortion: fresh generation, no compounding edits of edits beyond small deltas;
  attach the previous version when iterating and list ONLY the changes.

## Phase 1 — Asset acquisition (in order of preference)
A. RADICAL FIDELITY (composite painted regions: bars, fused panels): use the reference
   pixels THEMSELVES. Crop the whole region, patch ONLY truly-dynamic zones
   (numbers/values), alpha = the region's real content extent. Static text (labels)
   stays BAKED; per-language bakes are a build-step punch-list item.
B. EXTRACTION (standalone elements): imagegen EDITS the element's crop — "recreate this
   exact image, remove <content>, transparent outside". One element per request. No
   sizes, no style adjectives in the prompt; the crop is the spec.
C. NEVER description-based generation. It produces flat non-material art regardless of
   prompt quality. (Graveyard: every W1-W15 description batch needed replacement.)

Mechanical post-processing rules:
- Alpha-trim, then size-normalize. Plate aspect must match its slot (Law: aspect).
- Edge contamination (neighboring chrome baked into a crop): trim by color-mask scan.
- Mechanical INPAINTING of structured art (gradients, gloss, rims, ridge tubes) DOES NOT
  WORK — lerp/slab/profile fills all failed. Only patch zones that are genuinely
  horizontally-uniform (per-row replicate from a clean band), e.g. plain strip body.

## Phase 2 — Gates (all blocking; gate_status.md tracks every row)
GATE A (assets): each asset viewed by eye NEXT TO its source crop at native scale
  (pairs <= ~700px wide; segment regions, never full-width strips — wide pairs compress
  rims into hairlines and produce false verdicts in BOTH directions).
GATE B (screen): stage -> capture -> per-segment pairs vs the REFERENCE (never vs the
  asset, never vs the previous pass) -> walk EVERY element, not just what changed.
  Fast iteration loop BEFORE staging: composite the asset over the real background
  crop in PIL and judge — saves a 5-10 min stage cycle per round.
COMPLETION GATE: no report to the user while any row is PENDING/FAIL. Visual acceptance
  tests are explicit and binary (e.g. "the bar's full bottom border is visible").

## Phase 3 — Placement
- All Slate rects from the measured table; Image-draw with FMargin(0); 9-slice only for
  variable-size elements with caps <= 40% of min runtime size.
- Composite regions render as ONE image + chromeless click regions (NoBorder buttons)
  + live text overlays only for dynamic values.
- Widget-system traps to check: reserved-height systems clip overlay widgets (the top
  bar's GetTopBarReferenceReservedHeight); slot height must equal asset height (no
  squash); DPI/ScaleBox chains verified by measuring rendered px vs authored px.

## Identity (locked)
Hellfire: charcoal/near-black bodies, ember-red rims, lava red = hero action/selected/
YOU row, flame gold = single accent, warm-cream chunky text. Green ONLY for online
status + invites. Icons may be colorful; chrome may not. Inflatable vinyl; no stitches.

## Findings from the HeroSelection process test (2026-06-10)
1. MANIFEST COMPLETENESS: enumerate EVERY sub-element explicitly, including small wells,
   slots and + buttons inside strips (the steroid wells were missed and stayed old).
   Walk the reference panel by panel listing every visible item before cutting crops.
2. PLACEMENT PATTERN for canvas-based screens: when the screen has a panel chokepoint
   (e.g. HSMakePanel), restyle via a per-tag plate map there — one edit covers every
   panel. Prefer screens' chokepoints over per-site edits.
3. FLAT-BACKGROUND REFERENCES allow mechanical panel-box detection (rim mask + flood
   fill) verified by one overlay view — much faster than strip-reading. Use it whenever
   the reference background is flat; strips remain the tool for busy backgrounds.
4. HELPERS THAT BUILD CHROME INTERNALLY (e.g. MakeFlatDropdown) resist surgical
   restyling. Either restyle the helper globally (affects all screens) or accept as
   PARTIAL with a punch-list entry — do not hack one-off copies.

## Rules from the HeroSelection round-2 user review (2026-06-10)
5. BORDERS ARE SACRED, CONTENT SHRINKS: never resize/redraw an extracted panel's border
   — it is reference truth. When content (text/icons/rows) pokes outside the rim,
   DOWNSIZE THE CONTENT (smaller fonts, tighter insets, scaled inner slots) until
   everything fits with visible margin. Content-to-panel fitting is a code problem,
   never an asset problem.
6. PANEL EXTRACTS MUST BE TRULY EMPTY: at Gate A, reject any panel extract that kept
   ghost/empty ELEMENT SHAPES inside (buttons, pills, wells). Baked shapes + live
   overlays at even slightly different positions render as DOUBLE ELEMENTS (the
   CHAD/STACY double-pill bug). Retry instruction: "completely flat dark interior,
   no button or pill shapes". Never adopt 'align overlays to baked shapes' — it's
   unverifiable and broke.
7. COMPLETENESS IS CHECKED TWICE: the manifest enumerates every element of the
   REFERENCE **and** every interactive element of the LIVE screen (BUY existed live,
   was missed). Each needs an asset row or an explicit user-decision row.

## Juice (canonical interaction states — user-tested 2026-06-10)
ONE effect, applied to every interactive button at the style layer (ApplyJuicePop in
T66FriendslopStyle.cpp): SCALE POP — hover 1.03x, press 0.97x, pivot center, bound via
RenderTransform attribute on the SButton. That's it.
- Wired into: MakeToggleGroupButton (covers MakeButton + every classic-chrome button
  game-wide), MakeCustomToggleGroupButton (hellfire customs), and chromeless bar click
  regions (MakeJuicyBarRegion in the top bar).
- REJECTED after user testing: brightness/tint variants on the brush and hover films
  over baked art — color shifts read poorly on the vinyl. Do not retry.
- No audio juice yet (explicitly deferred by the user).
- Cursor: hardware cursor PNG pair convention (Content/Slate/.../name.png + name@2x.png)
  wired in DefaultEngine.ini [/Script/Engine.UserInterfaceSettings] HardwareCursors.
  Hellfire pointer lives at Content/Slate/Cursors/hellfire_pointer*.png.

## Rules from the PowerUp + RunSummary transplants (2026-06-11)
8. CANVAS-SCALE DOCTRINE: screens whose root is a 1920x1080 SBox inside a ScaleToFit
   under a reserved top bar render at 910/1080 = 0.8426 — author canvas units =
   reference-screen px x 1.1868. Screens WITHOUT a top bar (RunSummary) are 1:1.
   ALWAYS verify the factor empirically (author N px, measure rendered px) before
   trusting any constant; the original oversized constants (540x78 tabs) are the tell.
9. ASPECT-SAFETY: never author content wider than the canvas (negative full-bleed
   margins, overflowing grids). Overflow renders at exactly 16:9 but CLIPS at other
   window aspects (user-reported cut borders). Budget: content + scrollbar reservation
   (thickness + pad) <= canvas width. Verify with captures at TWO aspects
   (CaptureT66UIScreen -ResX/-ResY).
10. SLATE TRAPS (cost a stage cycle each):
   - An SBox with HAlign/VAlign_Center renders a generated-panel brush CONTENT-SIZED;
     the box must be FILL (no alignment args) for the plate to stretch to the override.
   - AutoWrapText inside a ScaleBox pre-wraps at WrapTextAt and shrinks the line tiny;
     drop wrapping for single-line text.
   - File-based override folders (RuntimeDependencies/T66/UI/FriendslopStyle/<Screen>/)
     are the preferred chokepoint: path helpers prefer them; most of a transplant is
     dropping correctly-named PNGs. Grep the path makers FIRST.
11. MULTI-VARIANT SCREENS (one widget, N modes — e.g. RunSummary live vs leaderboard):
   approve one reference per variant, but generate variant N as a DELTA EDIT of the
   canonical variant (changed block only). Imagegen re-synthesizes the whole frame, so
   pixel-diff is meaningless — drift-check shared regions visually instead. The
   CANONICAL variant's geometry implements ALL shared components; Gate B walks shared
   segments vs the canonical ref and only the changed block vs the variant ref.
12. UNREACHABLE-STATE CAPTURES: when a variant can't be reached by a frontend screen
   override, add a deterministic fixture flag (e.g. -T66RunSummarySavedFixture builds
   a fake snapshot in LoadSavedRunSummaryIfRequested) — per AGENTS.md, captures must
   stay Unreal-owned.
13. STAGING UNDER THE HARNESS: background shell tasks hard-cap at 10 minutes — retry
   loops die silently mid-sleep. Run stage retry loops in a Monitor (60-min cap, one
   event per attempt). Stage flakes (exit 6/1/102, instant AutomationTool crashes)
   clear on retry, but ALWAYS read the log before assuming flake: a mid-stage folder
   rewrite by a parallel agent looks like a flake and isn't (wait-and-retry clears it).
14. WORKER RETRY LANGUAGE: opaque-body extraction failures (white/transparent showing
   through a pill face) retry with "FULLY OPAQUE everywhere inside its own silhouette";
   text-ghost failures retry with "no embossed remnants". A bad CROP cannot be fixed by
   a better prompt — re-cut the crop first (zoom-verify), then retry.

## Graveyard (what failed, kept so nobody retries it)
- Description-based plate generation (flat art), numeric-only asset QA (passed flat
  garbage), judging downscaled images (false PASS and false FAIL), instrument windows
  that assume positions (measured their own windows), per-element extraction for fused
  regions (scale patchwork), narrow text-erase zones (ghosts), slab tiling (comb
  artifacts), full-height profile painting (erases ridges/gloss), alpha bands guessed
  instead of read from the raw reference (amputated tube), verifying capture-vs-asset
  (both shared the same defect).
