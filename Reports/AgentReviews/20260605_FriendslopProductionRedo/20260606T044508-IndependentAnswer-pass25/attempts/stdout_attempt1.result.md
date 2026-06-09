Result: OK

## Independent Answer

The images confirm all three of the user's failure observations, and they expose the core defect: the v4 gate reports `Result: PASS` with `Alpha IoU=1.000` on every family, yet the runtime composite is visibly wrong. The gate is measuring silhouette/alpha/chrome-MAD on isolated plates, not the perceptual fidelity of the assembled, live-rendered menu against the reference. That is why "technically blank but visually different" sails through.

**1. Replace masking/inpaint plate extraction with a clean-chrome reconstruction, not pixel surgery.**
The CTA and topbar pillows are inpaint remnants from locally replacing live-content zones on a reference crop. Stop patching reference pixels. For each chrome family:
- Reconstruct the plate from the reference's *geometry + material*, not its pixels: capture silhouette, corner radius, bevel width, gloss gradient direction, outline color/thickness, fill color as parameters, then author a flat blank plate (Slate brush / 9-slice or a freshly generated blank) that matches those parameters. No content zone ever existed, so there is nothing to inpaint over.
- A plate is acceptable only if it is uniform across the area that will hold live content — no residual halo, smear, or darker/lighter "ghost" where text used to be. The current CTA fails this.

**2. The title must stop being a bitmap.** The produced `CHADPOCALYPSE` is a degraded raster; the reference is clean stylized lettering. Per the contract's `title_not_cropped` rule, render it as Slate live text with a matching display font (+ outline/gradient material), not a cropped/regenerated PNG. If no font reproduces the stylized look, that becomes a user asset decision (see Blockers) — but the default target is live text, not a baked image.

**New gates to reject "blank but different" plates:**
- **Composite perceptual gate (decisive):** the existing alpha/silhouette/MAD checks stay, but acceptance is gated on a *rendered-runtime-vs-reference* comparison of the full menu and per-region crops (CTA, title region, each topbar button), using a perceptual metric (SSIM/LPIPS-style or a structured visual scorecard) with explicit thresholds. `Alpha IoU=1.000` must not be able to PASS a region the scorecard marks FAIL.
- **Flatness / residual-content gate:** within each plate's content area, measure local variance / gradient against the expected smooth chrome; reject if there is a halo or smear above threshold. This catches the CTA pillow and topbar smudge that the current gate misses.
- **Material-direction gate:** verify gloss/bevel orientation and outline thickness match the reference, not just average chrome MAD — average MAD passed while the CTA still reads as a different button.

**Text fitting/placement validation (topbar + CTA):**
- Enforce the contract's `contained_in` rows at runtime by dumping live widget geometry, not by eyeballing the plate: label and both icons inside the button bounds, with a minimum margin and a max-fill ratio so text neither overflows nor sits cramped (the topbar's weak fit).
- Add per-button checks: label baseline centered, font down-only scaling with ellipsis, icon-to-text spacing within tolerance, and a "label legibility" scorecard row. Topbar buttons need the same containment + min-margin row each.
- These must run on the *live capture*, so the validator reads actual rendered geometry, closing the gap between structural PASS and visual FAIL.

**What Codex should do before any next implementation pass:**
1. Add the composite perceptual gate + flatness gate + the topbar/CTA containment scorecard rows to the gate definition (spec only, no menu edits yet) and re-score the existing pass14 capture so the report reflects the real FAIL state instead of the current all-PASS.
2. Re-classify the v4 PASS report: it is invalid because it asserts PASS on visually-failing families. Mark it superseded.
3. Re-derive the CTA, title, and topbar plates via parameter reconstruction (no inpaint) and prove flatness before any runtime wiring.
4. Only then do an implementation pass, and accept it solely on the visual scorecard, per repo rule.

## Evidence Checked
- Reference vs produced captures: title is degraded/distorted vs clean; CTA shows a lighter pillow/halo behind text and a slightly different pill shape; topbar labels are cramped with smudging — all three user claims substantiated.
- `pass14_direct_reference_v4_component_gate_report.md`: all six families Auto+Manual PASS, `Alpha IoU=1.000`, `Result: PASS` — contradicts the visible runtime defects, confirming the gate measures the wrong thing.
- Contract addendum: already mandates `title_not_cropped`, no painted-over removal artifacts, `contained_in` rows, and visual-scorecard-over-FAIL=0 — the recommendations tighten enforcement of rules that already exist rather than inventing new policy.

## Questions Or Blockers
- Title asset decision (surface to user, do not block the recommendation): if no available font reproduces the stylized `CHADPOCALYPSE` lettering as live Slate text, the user must choose between (a) sourcing/approving a display font + material, or (b) approving a transparent, uncropped, validated title PNG under the existing exception. This is a user asset/scope call, not a Codex fix — but it does not prevent Codex from delivering the workflow recommendation now.

## Caveats
- The 2026-06-06 user-approved direct-reference-derived exception still stands; these recommendations operate inside it (tighter gates, no inpaint remnants) and do not relax the no-baked-content, no-raw-crop, or no-API/CLI rules. Do not treat "reconstruct from parameters" as license to paste reference pixels.
- I did not open `AGENTS.md` / `OPERATOR_VALIDATOR_PROTOCOL.md` this turn; the recommendation assumes the standard read-only validator protocol and visual-scorecard-controls-acceptance rule quoted in the prompt — Codex should confirm no protocol clause forbids adding the perceptual gate before scoping the work.

RESULT: OK
