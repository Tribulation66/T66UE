You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\pass14_reference_extraction_process_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\pass14_reference_gate_result_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\20260606T023759-IndependentAnswer-pass20\claude_review_pass20.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
# Pass14 Corrected Process Prompt

Original handoff: `C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\fresh_agent_main_menu_pass14_prompt_final.md`

Current user correction:

```text
Im already stopping you, because I can already tell these UI elements youre making do not match the reference image, we need a better process to extract the UI elements so they look exactly like the UI elements that are in the reference image but without the text or content, and then a stronger gate to validate the UI elements produced, so we dont get this problem what do you reccomend?
```

Codex recommendation already given:

```text
Use reference-first extraction rather than freeform imagegen; preserve exact reference silhouettes/material/lighting; remove only content holes; use account-backed imagegen only as masked inpaint/edit if available; add per-component gates comparing reference crop, mask, cleaned blank plate, alpha/checker preview, difference overlay, silhouette/edge/material checks, and manual visual PASS before runtime wiring.
```

Current continuation request:

```text
Ok continue with the solutions
```

Task for Validator:

Provide an independent repo-grounded answer for continuing pass14 after this correction. Check the T66/Friendslop process constraints and identify the safest implementation path. Assume Codex is Operator and Claude is Validator. Key question: should pass14 proceed by restoring the generic generated plates and creating reference-derived component extraction/gating artifacts before any runtime wiring? Note any mandatory stop conditions if the built-in account-backed imagegen tool cannot perform local-image/mask editing and CLI/API fallback is forbidden.

</original_prompt>

<validator_independent_answer>
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

</validator_independent_answer>

<codex_draft>
# Pass14 Reference Gate Result Draft

Operator: Codex

Validator: Claude

Scope: corrected pass14 asset process after user rejected freeform generated
component sheet.

## Work Performed

- Restored the pass13 runtime/source plates that had been overwritten by the
  rejected freeform generated sheet.
- Added a corrected reference-extraction rule to
  `UI/FriendslopStyle/Elements/main_menu_pass14_component_contract_addendum.md`.
- Appended the corrected extraction process to
  `Saved/Codex/UI/FriendslopStyle/MainMenu/pass_log.md`.
- Produced reference crops and live-content masks for nine Main Menu component
  families.
- Generated account-backed built-in imagegen candidates for the six primary
  failed families: primary CTA, secondary CTA, search pill, topbar icon plate,
  left panel, and right leaderboard panel.
- Ran the component gate before runtime wiring.

## Key Artifacts

- Reference crop/mask sheet:
  `Saved/Codex/UI/FriendslopStyle/MainMenu/pass14_reference_component_gate/proof/pass14_reference_crop_mask_sheet.png`
- Candidate gate contact sheet:
  `Saved/Codex/UI/FriendslopStyle/MainMenu/pass14_reference_component_gate/proof/pass14_candidate_gate_contact_sheet.png`
- Candidate gate report:
  `Saved/Codex/UI/FriendslopStyle/MainMenu/pass14_reference_component_gate/pass14_candidate_component_gate_report.md`
- Decision block:
  `Reports/AgentReviews/20260605_FriendslopProductionRedo/decision_block.md`

## Result

Component gate result: FAIL.

No candidate was wired into runtime. The generated candidates are blank and
content-free, but they drift from the exact reference silhouettes/materials and
remain in `REVIEW` manual status. This prevents another structural-only or
generic-lookalike acceptance.

## Decision Needed

The remaining exact-fidelity paths require user approval:

- allow direct reference-derived runtime plates with strict gates;
- or allow a true local-image/mask inpaint path;
- or explicitly accept approximate built-in-only regeneration.

</codex_draft>
