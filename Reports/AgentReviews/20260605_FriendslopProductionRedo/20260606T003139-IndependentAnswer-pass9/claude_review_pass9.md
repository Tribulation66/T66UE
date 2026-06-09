Result: OK

## Independent Answer

The four problems share one root cause, visible in both the capture and the generator script: `pass12_generate_reference_inpaint_plates.py` takes a **single reference screenshot**, crops it into ~26 fixed slices, runs Pillow "cleaning" (median fills + Gaussian blur + masks) over each slice, and saves the results as the runtime UI textures. Every symptom the user names follows from treating a raster screenshot as the source of UI material.

**1. OpenCV / skimage / Pillow — purpose and viability**
- Purpose here: raster image processing. Pillow does the crop/mask/blur/composite; OpenCV/skimage are general computer-vision libs for the same class of pixel work (edge/inpaint/denoise). In this pipeline they were used to slice the reference and "inpaint" baked text/icons out of the crops so the slices could be reused as button/panel art.
- Viability: **No, not for production UI.** These tools manipulate existing pixels; they cannot synthesize crisp, resolution-independent UI. Cleaning baked content out of a JPEG-quality crop always leaves smear. They are fine for *analysis* (measuring the reference, diffing a capture vs reference) but wrong as the *asset source*.
- Actual solution: author the UI natively in UMG/Slate — solid/gradient brushes, real fonts, vector or cleanly-authored icon textures, design tokens for color/radius. Use the reference only as a visual target to measure against, not as material to cut up. Keep OpenCV/Pillow for the verifier/diff step only.

**2. Sizing and icon-on-icon**
- Cause: the plates are fixed-size crops that **still contain the reference's own baked icons/text** (e.g. `cta_*`, `*_icon_*` specs), and the live UMG widget then draws *its own* icon/label on top. Reference icon + live icon = the doubled icon. Achievement text overflow is the same class: a fixed plate sized to the reference while the live string/font exceeds the container, with no auto-size/wrap.
- Solution: plates must carry **no baked content** (or be replaced by plain brushes); size containers to content with auto-wrap/SizeBox constraints; let the widget own all icons and text exclusively.

**3. Masked / smudged "paint layer" on the central buttons**
- Cause: `clean_control` builds a blurred median "row_edge_fill" and composites it over the crop interior (lines 125–148) specifically to erase baked text. That composited blur *is* the layer of paint / whitener-and-rewritten / manual-mask look the user sees. It is an inpaint artifact, unavoidable when you smear over baked pixels.
- Solution: stop inpainting bitmaps entirely. Draw the CTAs as native solid/gradient rounded brushes — there is nothing to "clean" if nothing is baked in.

**4. Color / shape / detail discrepancies**
- Leaderboard row (red outline+dark interior vs. red-filled), non-uniform high-score checkbox, missing green Invite button, missing green online dot.
- Cause (two strands): (a) colors/shapes are derived from **blurred median samples** of crops, so hue drifts and rounded-shape uniformity is lost; the `ranking_row_red` plate fills its interior rather than preserving an outline. (b) Some intended states exist in the generator (`invite_button_green_round06`) or reference but are **not wired into / are overridden by** the live widgets, so green Invite and the online-status green dot never render.
- Solution: define explicit design tokens (exact colors, corner radii, state variants) and render shapes as uniform engine brushes; ensure the live widgets actually bind the intended states (green invite, outline-only ranking row, green online indicator) instead of inheriting a blurred crop.

Bottom line for the user's underlying question: the screenshot-crop-and-inpaint approach is the defect, not a tuning problem. The next iteration should rebuild these elements as native UMG/Slate widgets driven by design tokens, keeping the image libraries only for measurement/verification.

## Evidence Checked
- `pass12_generate_reference_inpaint_plates.py` — confirms crop-from-single-reference + Pillow inpaint/blur pipeline, fixed-size SPECS, baked-content masks, and `save_all` writing into SourceAssets/RuntimeDependencies/StagedBuilds.
- `friendslop_pass12_material_verdict.md` — verifier reports `PASS=251 FAIL=0` but only covers topology/containment/state, with multiple "PASS with accepted delta" — explains why automated gates pass while the eye fails.
- `friendslop_pass12_fixture_capture.png` — shows doubled sphinx icons on ENTER TRIBULATION, smudged central buttons, no green Invite, no online green dot, non-uniform leaderboard row/checkbox.
- `friendslop_pass12_material_crop_sheet.png` — reference-vs-plate rows show the blurred/painted plate quality.

## Questions Or Blockers
None blocking — the prompt asks for analysis only, and the evidence is sufficient to answer all four. Codex can deliver this directly. (No user decision is required because no implementation or scope change was requested.)

## Caveats
- I inferred the green-Invite / online-dot gap from the SPEC list plus the capture; whether it's a missing widget binding vs. an override would need a quick look at the live UMG widget to pin down exactly — but the fix direction (native widgets + tokens) holds either way.
- The verdict's `PASS=251` is real but measures structure, not material fidelity; don't let it be cited as evidence the look is acceptable.
