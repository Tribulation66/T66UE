Result: OK

## Independent Answer

I inspected both images directly. The diagnosis below is grounded in what the pass10 capture actually shows versus the Round06 reference, and the solutions map to mechanisms the implementation instructions already require but that pass10 skipped.

### 1. Split / missing-middle line artifact

**Cause.** This is a 9-slice/3-slice authoring-and-margin failure, not a layout failure. The `ENTER TRIBULATION` and `LOAD GAME` plates are being drawn with `DrawAs=Box` where the source plate's vertical center band does not actually contain continuous body material, and/or the `FMargin` top+bottom caps sum to nearly the full source height. When the brush is stretched to runtime height, the preserved top cap and bottom cap pull apart and the empty/short center band renders as a seam — the "cut in half, middle removed" look. The reference buttons are single inflated rubber pills with an unbroken glossy body; the capture's are two stacked caps with a dead line between.

**Actual solution.**
- Treat these stable-height/variable-width buttons as **horizontal-3-slice**, not vertical/full 9-slice. Height should be fixed to the source plate height (or use a size-specific fixed-image plate). Only the width stretches; the vertical body is never stretched, so no horizontal seam can appear.
- If the button must change height, the center band must be re-authored as a clean, seam-safe, full-height fill with low detail, and margins set as normalized fractions that leave a real stretchable middle. Do not trust "looks plausible" margins (Sec. 9).
- Re-crop with enough transparent padding for the soft shadow so the cap isn't clipped.

**Acceptance gate.** The mandatory **min/normal/wide stretch test** at the runtime height plus the per-family **contact sheet at min/normal/wide** (Sec. 8–9). The plate fails the gate if any highlight/bevel/shadow smears or any seam appears at the center band. No import until that contact sheet passes and the single-brush runtime smoke test (Step E) renders clean.

### 2. Panel content not fitting inside the panel

**Cause.** Containment failure: the left social panel and right leaderboard were authored so child content (rows, search field, dropdowns, scroll body) is sized/positioned independently of the parent plate's interior content rect, so rows escape or crowd the panel walls. Absolute screen position being "close" hid it from the structural PASS count.

**Actual solution.**
- Define each panel plate with an explicit **content inset / interior content rect** (padding inside the 9-slice border) and constrain the live child list to that rect with proper clipping. The panel chrome owns the frame; Slate owns a bounded body that lists/scrolls inside the inset.
- Fix at the parent: panel size, content insets, row width/height, and list/table body constraints and clip region — per the Step I containment-failure recipe. Do not patch by nudging absolute row positions.
- Where the row count exceeds the visible body, the body must scroll/clip within the inset, never paint over the frame.

**Acceptance gate.** Containment is verified in the dump: every row/control's bounds must be inside its intended parent's content rect, plus a visual check that no content touches or crosses the plate border at min/normal/wide and across the responsive resolution set (Step K). "Row containment passed" in pass10's scorecard only covered the center stack — the left/right panels must pass the same containment check explicitly.

### 3. Reference mismatch (side panels + central buttons categorically wrong)

**Cause.** This is the already-recorded false-accept root cause: generic pilot/"blank rubber atom" plates were reused and self-passed on structural coordinates, instead of authoring reference-matched per-element/per-size plates. A plate that is merely the right semantic category ("red rubber button," "dark panel") but wrong silhouette, scale, material density, and edge treatment is explicitly a fail (Sec. 8). The capture's panels and buttons read as flat/generic against the reference's inflated, glossy, beveled rubber.

**Actual solution.**
- Re-author **per-element / per-size-family plates** that match the cropped reference region directly — left panel shell, right leaderboard shell, primary CTA, secondary CTA each as their own authored plate family with matched states — rather than stretching one generic atom across all surfaces (Sec. 8, Sec. 10 "do not force all high-quality chrome through one generic resizable brush").
- Compositing/Photoshop-equivalent cleanup is allowed and expected to hit the material bar (Sec. 2.3).
- Each plate is compared against its **cropped reference region** before slicing, so silhouette/scale/material/edge are matched, not approximated.

**Acceptance gate.** The **visual scorecard / holistic gestalt gate** (Step J) must return `Result: PASS` with the per-category rows (first-glance match, silhouette/region weight, panel/button scale hierarchy, rubber material fidelity, authored plate quality) all passing — and each plate's contact sheet must show it beside the cropped reference region. A zero-FAIL structured report is explicitly not acceptance.

### Process change required before the next production pass

No more blind full-screen production passes. Before any code/asset work, the next pass must, per the instructions:
1. Write the **PPF gate, Artifact Parity gate, and Mechanism Manifest** for Main Menu (Sec. 5–7), with the left panel, right leaderboard, and both CTAs each listed as **Required: YES, Primary** with their own planned plate paths.
2. Author and pass the **per-family contact sheet + min/normal/wide slice test + single-brush runtime smoke** before importing any full set (Step E).
3. Gate acceptance on the **visual scorecard PASS**, not the PASS/FAIL count — and both Codex and Claude must inspect the reference/capture before citing the structured count (Step J).
4. Sequence the three fixes as: re-author reference-matched per-element/per-size plates → wire panel content insets/clipping → only then re-capture and run the holistic gate.

**User decision to name before implementation resumes:** the slicing-vs-fixed-plate policy for the CTAs and panels is a taste/quality call the user owns — i.e., approve that height-variable rubber surfaces are allowed to ship as **size-specific fixed-image plates** (no vertical stretch) rather than forcing a single resizable 9-slice. If the user wants one resizable brush per surface for asset economy, the split-seam risk returns and the center band must be re-authored to a higher bar. This is the one art-direction/quality tradeoff that should be confirmed before the plate-authoring pass starts.

## Evidence Checked

- Pass10 capture `Saved/Codex/UI/FriendslopStyle/MainMenu/friendslop_pass10_fixture_capture.png` — viewed: CTA buttons show clear top/bottom cap separation with a dead center line; panels read flat/generic; rows crowd panel interiors.
- Reference `UI/FriendslopStyle/Reference/MainMenu/Round06/main_menu_reference_01_current_capture_stronger_rubber_cli.png` — viewed: single inflated rubber pills, consistent gloss/bevel, content seated inside panel insets.
- `FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` — Sec. 8 (per-element/per-size plates, contact-sheet gate), Sec. 9 (slice rules, min/normal/wide test), Step E/I/J/K gates.
- Confirmed all five named rule files exist; both evidence images exist.

## Questions Or Blockers

- None that block *answering* the three questions. The single user-only decision is the slicing-vs-size-specific-plate policy for CTAs/panels, named above; it is a prerequisite for the plate-authoring pass, not for this analysis.

## Caveats

- I did not measure pixel margins or open the pass10 dump/slice specs; the 9-slice-margin diagnosis is from visual evidence plus the instructions' known failure modes. Codex should confirm the actual `FMargin` values and source plate center band when authoring the fix.
- The "stale FlatStyle checklist" issue (Sec. 12) is orthogonal to these three problems but must not be used as evidence for the next pass — author a fresh Friendslop checklist.
- I did not re-verify pass10's PASS=249/FAIL=4 numbers against the report file; I took them from the prompt's stated context.
