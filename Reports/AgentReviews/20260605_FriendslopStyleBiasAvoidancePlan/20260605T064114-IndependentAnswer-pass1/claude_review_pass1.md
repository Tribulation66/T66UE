Result: OK

## Independent Answer

The recommendation pass can be produced by Codex/Claude now — no user decision is needed to write the next-pass strategy. One scope item must be confirmed with the user before any generation runs (see Questions), but it doesn't block the advisory deliverable.

**1. Count/pole conflict to resolve first.** The user counts "4 images" and "get rid of the third (Lethal) and switch to Fall Guys," but the working scope lists **five** poles (PEAK, Schedule I, Fall Guys, GWYF, R.E.P.O.). Round01/Round02 both shipped 5. Codex must confirm 4 vs 5 before generating — don't silently assume.

**2. Reduce imagegen house-style bias.**
- Reference closeness is now explicitly authorized for internal vision-boarding (not published), so drop the Round02 guardrail line "do not use signature layouts/copied palettes/theme transfer." That guardrail is what forced drift back to the generic chunky-fantasy default. Replace with: name the source game directly and describe its concrete UI signature (palette hex ranges, panel geometry, type weight, texture) per image.
- Add explicit negative prompting against the default bias: e.g. "no arced/curved banner title, no generic chunky fantasy RPG plates, no warm-brown wood UI" unless that specific pole calls for it.
- Pin the title treatment per-image (the arced `Chadpocalypse` repeated across 1/2/4 is the clearest drift tell) — specify flat/straight/per-style title geometry explicitly in each prompt.

**3. Ensure high variance.**
- Lock divergent axes per image: palette, panel shape language, type family, texture/material, and title geometry should each differ measurably across the set.
- Keep constant only what must stay constant: T66 layout regions and the visible content roles from the manifest's Shared Prompt Contract (nav bar, left profile/party, center title+subtitle+3 CTA, right leaderboard, and the literal labels).

**4. Isolation strategy — agree with per-image isolated workers.**
- Yes, isolated generation per image is the right call to kill sequential cross-contamination. Each image gets its own self-contained prompt with zero reference to other candidates or prior rounds, run in a clean worker context.
- Practical caveat: "unique Codex CLI per image" is fine as long as each is genuinely independent. The isolation that matters is *no shared image-history / no prior-output references in context*, not the number of CLI processes per se. Parallel independent workers achieve this.

**5. QA / uniqueness gates before delivery.**
- Verify each image independently satisfies the layout-region + content-role checklist (manifest Shared Prompt Contract).
- Pairwise distinctness check across all candidates on the locked axes (palette, panel shape, type, texture, title geometry) — reject and regen any pair that collides.
- Specifically flag the title-arc reuse as a fail condition.
- Confirm Fall Guys pole replaced Lethal Company; confirm final count matches user's answer.

**What Codex should tell the user before regenerating:** (a) confirm 4 vs 5 candidates; (b) confirm reference closeness is acceptable for internal-only boards (loosen guardrails accordingly); (c) state that isolation = no cross-image context, and list the uniqueness gates above.

## Evidence Checked
- `Round02/manifest.md` — Shared Prompt Contract, guardrails ("no signature layouts/copied palettes/theme transfer"), 5 outputs, arced `Chadpocalypse` title basis.
- `Round01/manifest.md` — Source Reference Poles list (PEAK, Schedule I, Lethal Company, GWYF, R.E.P.O.).
- `Round02/` directory — 5 candidate PNGs + contact sheet present; `_03_lethal_` is the file to replace with Fall Guys.

## Questions Or Blockers
- Final candidate count: **4 or 5**? User says 4 (and "drop the 3rd"); scope lists 5 poles with Fall Guys swapped in for Lethal. Codex should confirm before generating — not a blocker for the recommendations themselves.

## Caveats
- I did not visually open the PNGs; the drift/title-arc claims are taken from the user's report and manifest notes, not pixel inspection.
- "Closer to reference / internal-only" loosens copyright guardrails — fine for private vision boards, but these outputs should stay clearly marked non-publishable and never promote into runtime assets.
