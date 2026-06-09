Result: OK

## Independent Answer

The proposed reference-image-first process is sound and is, in essence, what T66 already does for flat UI migration — just generalized to a net-new screen group ("Friendslop") instead of a port. The right move is to **adopt the existing fidelity loop as the backbone** rather than invent a parallel "imagegen-then-match" process, because the existing loop already encodes the hard-won guardrails.

The one critical distinction the user's framing glosses over: there are two very different uses of imagegen, and only one is allowed.

- **Allowed:** imagegen produces a *visual design target* — a mockup / "V3 reference" image you measure against and check toward. This is exactly `UI/Screen References/<screen>.png` in the fidelity loop. It is a review artifact, never shipped.
- **Forbidden (the retired failure mode):** imagegen output becomes the *actual runtime chrome* — raster panels, borders, button plates, modal shells composited into the live UI. `UI/README.md:20` and `UI/UI_AGENTS.md:24` explicitly ban this.

So the recommendation: **yes, generate a reference image first — but the screen itself is still built as flat Slate via `FT66FlatStyle`, and "matches the reference" is judged primarily by the structural dump + `VerifyUIFidelity.py`, not by eyeballing the raster.** The reference image is the design source; flat Slate is the implementation; the dump is the source of truth for objective comparison (`UI_FIDELITY_LOOP_INSTRUCTIONS.md:48-52`).

### Required guardrails (so it doesn't repeat the imagegen-chrome failure)
1. **Reference ≠ asset.** The generated image is a target only. Persist it under `UI/Screen References/` (per project convention) and treat it as immutable review input. Chrome in the live screen is Slate-native, full stop.
2. **Imagegen in-screen is limited to content stubs and flat icon glyphs**, recorded in `UI/content_stubs_registry.md` / `UI/icon_manifest.md` (loop §9, `UI_AGENTS.md:25`). A whole-screen generated mockup is a reference, not a stub — don't wire it into the runtime.
3. **"Looks like the reference" must resolve to structural claims**, not LLM visual judgment, wherever possible (color/text/geometry via dump). Reserve visual judgment for genuinely visual items (artwork resemblance, font feel) and escalate persistent UNSUREs to the user (loop §2.3, §6).
4. **Geometry extraction step (Step 0.5) still applies** — measure the generated reference, normalize to 1920×1080, sanity-overlay before building. A generated mockup can have arbitrary/unbuildable proportions, so the overlay sanity check matters more here, not less.
5. **Transparency/output caveat:** imagegen transparent outputs need the cautious path noted in the prompt; a full-screen mockup should be opaque PNG anyway.
6. **Termination/escalation unchanged** — zero-FAIL gate, 5-pass cap, ESCALATE packet to the user (loop §8).

### What Codex should tell the user before writing any process docs
- Confirm the framing: the generated image is the **design reference**, and Friendslop screens are still **flat Slate built and verified through the existing fidelity loop** — not generated chrome. Get explicit agreement on that line, since the user's wording ("make the screen look like the reference image") is ambiguous about whether the image becomes the asset.
- Recommend **not creating a brand-new standalone process** — instead, base `UI/FriendslopUI` on `UI_FIDELITY_LOOP_INSTRUCTIONS.md` with a thin Friendslop-specific addendum (screen list, per-screen specs, the up-front "generate the design reference" step). Avoid doc drift; the loop doc is canonical (`UI_FIDELITY_LOOP_INSTRUCTIONS.md:1002`).
- Note one genuinely new wrinkle vs. the existing loop: existing screens were *ports of an existing design*; Friendslop appears to be *new design*. So the generated reference is doing original design work, which means the user should expect a **design-approval gate on the reference image itself** before any implementation begins — there's no prior ground truth to fall back on.
- Get the user's go/no-go on adopting this as the basis for `UI/FriendslopUI` before writing docs or assets (per the stop condition).

## Evidence Checked
- `UI/README.md` — flat Slate pipeline; generated raster chrome banned; retired imagegen-chrome workflow noted (lines 3, 19-22, 50).
- `UI/UI_AGENTS.md` — hard rules: no generated raster chrome; imagegen only for content stubs/icons (lines 23-25).
- `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md` — full loop: reference images, geometry extraction (§0.5), structural-over-visual verification (§2.3), content-stub policy (§9), termination/escalation (§8), canonical-doc clause (§13).
- Original prompt's stated repo context matches the live files.

## Questions Or Blockers
None that block the recommendation. The user's go/no-go on adopting the process — and approval of the design-reference image itself — is the natural next step after this recommendation, not an escalation blocker now.

## Caveats
- I did not check whether a `UI/FriendslopUI` folder or any Friendslop screen specs already exist; Codex should confirm the target location and master-plan integration before authoring docs.
- "Friendslop" screen scope (which screens/modals) isn't defined in the files I read — the per-screen specs that the loop depends on don't exist yet and will need authoring.
- The imagegen transparent-output / persistence caveat is taken from the prompt's framing; I did not independently verify the imagegen skill's current behavior.
