Result: OK

## Independent Answer

**Yes — proceed by building reference-derived extraction + per-component gating artifacts first, and do NOT wire anything (including the old generic plates) into runtime until each component passes its gate.** But one correction to the plan: do not "restore the generic generated plates" as runtime assets. They are the rejected output. Restore/keep them only as *baseline comparison material* in a proof folder, quarantined from runtime. Wiring them back in is the exact thing the user stopped you for.

Recommended continuation, ordered:

1. **Reference-first crops, per component.** For each component the Main Menu needs (title, left/right rubber panels, CTA primary/secondary, search pill, topbar icon, topbar text tab, friend row, section header, dropdown, leaderboard tabs, local row, checkbox states), cut an exact crop from `main_menu_reference_01_current_capture_stronger_rubber_cli.png`. These crops are **gate targets**, not runtime assets.
2. **Derive a per-component spec** from each crop: silhouette/aspect, bevel/gloss/shadow, outline-vs-fill material, content holes (where live text/icon will sit). This is what enforces "exactly like the reference, without text/content."
3. **Regenerate clean blank plates** via the built-in account-backed `image_gen` on a flat `#00ff00` chroma-key background, then `remove_chroma_key.py` for alpha. Blank means chrome only — no baked text/icons/skulls/scores.
4. **Gate each plate before wiring** with the artifact set Codex already proposed: reference crop | regenerated plate | alpha/checker preview | difference overlay vs the reference crop | silhouette/edge/material check | explicit manual visual PASS. OpenCV/Pillow are allowed *only* for measuring/diffing these, never to author the art (prior failure class #4).
5. **Only after a component's gate PASSes**, wire it through `FT66FriendslopStyle` descriptors / explicit brushes without regressing other enum users. Containment + measured-fit helpers and the visual scorecard remain the holistic acceptance gate.

**Correction to Codex's recommendation #14 ("masked inpaint/edit if available"):** per the imagegen SKILL.md, true mask/inpaint with direct local-file + mask control is **CLI-fallback only** (`scripts/image_gen.py`, requires `OPENAI_API_KEY`). The built-in tool's "edit" is a *regenerative* edit on an image loaded via `view_image` — it preserves invariants by re-generation, not pixel-exact masking, and exposes no mask parameter. So the masked-inpaint variant is **not available** on the path pass14 is allowed to use. The plan above sidesteps this: the reference crop is used as the *comparison gate*, not as a tool-side mask. Codex should not present built-in masked inpaint as an available technique.

**Mandatory stop condition (user-only).** If a component cannot pass its silhouette/material gate through built-in chroma-key regeneration, the only ways to get pixel-exact reference fidelity are (a) CLI/API mask inpaint — **forbidden by pass14 rule 6 and the imagegen no-CLI policy**, or (b) shipping a direct cropped reference pixel-plate as the runtime asset — which **collides with failure-class #1 (no crop/masked plates as runtime assets)**. Both are policy changes. At that point Codex must **STOP and ask the user**, not silently switch to CLI/`OPENAI_API_KEY` and not silently ship a raw crop. Per rule 6, a built-in `TooManyRequests`/session error is a transient → fork/restart fresh session, not a fallback trigger.

## Evidence Checked
- `pass14_reference_extraction_process_prompt.md` — the correction, Codex's recommendation, continuation request.
- `fresh_agent_main_menu_pass14_prompt_final.md` — rule 6 (no API/`OPENAI_API_KEY`, built-in only; transient → fork), failure classes #1 and #4, plate-vs-live ownership, gating/scorecard precedence, recommended order, acceptance bar.
- `~/.codex/skills/.system/imagegen/SKILL.md` — built-in vs CLI modes; masks/direct-file editing are CLI-only; built-in edit needs `view_image` and is regenerative; transparent path = chroma-key + `remove_chroma_key.py`.

## Questions Or Blockers
None blocking now — Codex can build all extraction + gating artifacts on the built-in path without the user. The user decision is *contingent*: it only triggers if a component fails the gate and the only remaining options are the forbidden CLI/API path or shipping a raw reference crop.

## Caveats
- I did not open the pass13 artifacts/captures; assessment is process/capability-grounded, not a fresh visual diff.
- "Reference-first extraction" must stay disciplined: crops are gate targets, regenerated plates are the runtime assets. If Codex's draft instead extracts-and-ships the crop, that quietly re-enters failure-class #1 and should be corrected before wiring.
- "Looks exactly like the reference" via built-in regeneration is an approximation bounded by the gate's tolerance, not guaranteed pixel-identity — set the difference-overlay/silhouette thresholds explicitly so PASS is meaningful.
