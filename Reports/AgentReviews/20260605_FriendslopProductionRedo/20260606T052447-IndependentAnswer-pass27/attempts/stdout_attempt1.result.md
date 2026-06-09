I have enough evidence. The docs are explicit on this point.

Result: OK

## Independent Answer

**Did Codex use imagegen for the elements?** No — not for the runtime chrome or the title. `pass15_generate_reconstructed_plates.py` builds the topbar/panel/CTA/leaderboard plates **and** the title procedurally in Pillow: hardcoded `PALETTES`, `vertical_gradient`, `ImageDraw.rounded_rectangle`/gloss/groove synthesis, plus `crop_ref` + `reconstruct_content_corridor` to sample some reference edge pixels. Built-in account-backed imagegen was used **only** for the rubbery statue background. So the elements the user is reacting to (the tightened title mask, star-speck removal) came from manual Pillow/NumPy, not imagegen.

**Do the docs require imagegen?** The docs do not require imagegen for *everything*, but they explicitly disqualify what pass15 did:
- `fresh_agent_main_menu_pass14_prompt_final.md` → "What Went Wrong Before" item 4: *"Treating OpenCV/skimage/Pillow as asset-generation quality tools. They are fine for measurement, contact sheets, alpha validation, crop QA, and verification. They do not create the premium runtime art by themselves."* The "Imagegen Guidance" section then says *"Use built-in account-backed imagegen for runtime chrome generation."*
- `main_menu_pass14_component_contract_addendum.md` opened a **second** sanctioned path (the 2026-06-06 user-approved exception): direct reference-derived crop plates with live-content zones removed, strict gates, no API. It says built-in imagegen is *allowed but not required* on that path. Crucially it also states plates must be *"generated or selected against exact reference crops, not invented from a freeform component sheet."*

So there are exactly two sanctioned methods: (1) built-in imagegen blank plates, or (2) direct reference-crop-derived plates. Procedural palette/gradient synthesis is **neither** — it is the "invented" approach both docs forbid.

**Noncompliance or instruction gap?** Primarily **noncompliance**. The instructions already say Pillow does not produce runtime art and that plates must not be invented. Pass15's reconstructed-plate method is the prohibited path with reference-sampling bolted on. The user is **correct**, not mistaken: the proven-good method for these assets is imagegen-driven (or, per the new exception, true reference-crop-derived), and the Pillow-first instinct is the recurring failure mode the docs were written to stop.

There is a *small* genuine gap worth closing: the docs name the two allowed methods and forbid "invented from a freeform component sheet," but never explicitly name "procedural palette/gradient plate synthesis" as a banned third method — which is the loophole pass15 slipped through.

**Recommended process correction (for Codex to fold into its answer):**
1. Acknowledge the user is right and that pass15 was noncompliant with item 4 + the addendum anti-invention rule.
2. Add an explicit named rule to `FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`: runtime chrome/title plates may come **only** from (a) built-in account-backed imagegen, or (b) direct reference-crop-derived plates with live zones removed. Procedural Pillow/NumPy synthesis of silhouette/material/gloss is explicitly forbidden as an art source (Pillow stays limited to measurement, masking of live zones, alpha/crop QA, contact sheets, verification).
3. Add a default-method line: "first instinct for a missing/bad asset is imagegen or reference-crop, never procedural paint."
4. Redo the pass15 title and chrome plates via a sanctioned path before any further visual scoring.

## Evidence Checked
- `Saved/Codex/UI/FriendslopStyle/MainMenu/pass15_generate_reconstructed_plates.py` — confirmed procedural Pillow synthesis (PALETTES, gradients, ImageDraw) + partial reference cropping/corridor sampling.
- `Reports/AgentReviews/.../fresh_agent_main_menu_pass14_prompt_final.md` — item 4 (Pillow not an art tool) and Imagegen Guidance section.
- `UI/FriendslopStyle/Elements/main_menu_pass14_component_contract_addendum.md` — 2026-06-06 user exception; "not invented from a freeform component sheet"; imagegen allowed-not-required on reference-derived path.

## Questions Or Blockers
None requiring the user. This is a process-audit question the models can answer fully from the repo; no user decision, approval, or unblock is needed.

## Caveats
- I did not re-read `AGENTS.md`, `UI/UI_AGENTS.md`, the fidelity-loop / layout instructions, or the asset registry in full this pass; the two strongest governing docs above already settle the question, but if Codex wants belt-and-suspenders it should confirm none of those *loosen* the imagegen/reference-crop requirement.
- "Reconstructed" plates do touch reference pixels, so a charitable reading is "partial reference-derived." Even so, the synthesized silhouette/material core is the invented part the addendum forbids, so the noncompliance finding stands.
