Result: OK

## Independent Answer

The instructions are **not unified**. T66 currently documents **two contradictory, fully-specified UI systems**, and the routers still point at the older one. This is the single most important thing to fix before any PEAK/friendslop work starts.

**The two conflicting systems:**

1. **PNG-composited "match the reference exactly" system** — `UI/MASTER_REFERENCE_UI_GENERATION_PROMPT.md` (208 lines), `UI/Instructions/UI_GENERATION_INSTRUCTIONS.md`, `UI_FIDELITY_LOOP_INSTRUCTIONS.md`. Built on imagegen sprite sheets, the Ultrakill reference plate library, glow material, and a hard rule: *"Match the reference exactly. Do not upgrade, embellish, or reinterpret it."* It assumes you already have an exact reference screenshot to clone.

2. **Flat Slate-native redesign** — `UI/Reference/UI_FLAT_REDESIGN_REFERENCE.md` (855 lines), which calls *itself* "the single source of truth." It explicitly **removes** PNG chrome, glow, and sprite-sheet imagegen — the exact opposite of system #1 — in favor of `FT66FlatStyle` code constants, a locked palette, and V3 mockups. This is the one that actually looks PEAK/friendslop-like (axis-aligned rectangles, solid colors, no glow), and it is **already in execution** (Hero Selection pilot iterated through Stage 1 Pass 5; V3 mockups exist in `Screen References/`).

**Why this is a real problem, not cosmetic:**

- `UI/UI_AGENTS.md` (the router an agent reads first) **never mentions the flat redesign**. Its "Read First" list points only at the PNG/fidelity-loop system. An agent starting fresh will load the *wrong*, now-superseded direction and the "do not embellish, clone the screenshot" rule — directly contradicting a fresh art-direction change.
- `UI/README.md` names a *third* set of "primary" docs (`UI_SCREEN_MODAL_INSTRUCTIONS`, `UI_SCREEN_WORKFLOW_INSTRUCTIONS`) that differ from UI_AGENTS.md's list. Three docs each claim primacy: master prompt, UI_AGENTS, flat-redesign.
- The flat-redesign doc's mandatory companion, `UI/T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md` ("read this first for architecture"), **does not exist**. Its own source-of-truth chain is broken.
- ~50 per-screen checklists + two `pending_issues` files add more surface area with no single index tying them to whichever system is canonical.

**Recommended cleanup sequence (for the user to approve before starting):**
1. User confirms which direction is canonical (see blocker below).
2. Rewrite `UI/UI_AGENTS.md` "Read First" + `UI/README.md` to point at the one surviving system; explicitly mark the other as archived/superseded (don't silently delete — it has working capture/build policy worth salvaging).
3. Either restore or remove the dangling `T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md` reference.
4. Designate one doc as the single entry point; demote the rest to "supporting" with no competing "source of truth" claims.

## Evidence Checked
- `UI/UI_AGENTS.md` — router; routes only to PNG/fidelity-loop system, omits flat redesign.
- `UI/README.md` — names a different "primary" supporting-doc set.
- `UI/MASTER_REFERENCE_UI_GENERATION_PROMPT.md` — full read; PNG/imagegen/"clone exactly" system.
- `UI/Reference/UI_FLAT_REDESIGN_REFERENCE.md` (lines 1–620 of 855) — self-declared SoT; removes PNG chrome; references a missing companion doc.
- Confirmed missing: `UI/T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md`.
- `UI/Screen References/` — V3 mockups + Hero Selection Stage 1 Pass 1–5 compares exist → flat redesign is mid-execution.
- Directory listings of `UI/Instructions`, `UI/Reference`, `UI/Processes`, `UI/Checklists`; confirmed both `pending_issues` files exist.

## Questions Or Blockers
- **User-only decision (gates the cleanup, not this audit):** Is the new "friendslop/PEAK-like" direction (a) the existing `UI_FLAT_REDESIGN_REFERENCE.md` flat system, (b) a refinement of it, or (c) a brand-new third direction? Until you confirm, we can't label the flat-redesign doc as current-vs-stale, and we can't tell agents which of the two systems to obey. The audit's finding stands either way; the *fix* needs this answer.

## Caveats
- I read the flat-redesign doc through line 620 of 855 (per-screen specs 7.2 continue beyond); the system-level contradiction is fully established, but I didn't enumerate every per-screen spec.
- I did not open `UI_GENERATION_INSTRUCTIONS.md` / `UI_FIDELITY_LOOP_INSTRUCTIONS.md` line-by-line; classification is from how UI_AGENTS.md and the master prompt invoke them. Codex should skim them to confirm they belong to system #1 before archiving.
- Read-only pass; no files inspected for runtime correctness, and nothing was modified. Token usage not separately instrumented on my side.
