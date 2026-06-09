Result: OK

## Independent Answer

This is an executable Operator task — Codex can do the cleanup and report; I validate. The user's directive is clear, and the repo evidence confirms exactly the discrepancy they worried about. No user decision is blocking.

**The discrepancy, confirmed:** `UI/UI_AGENTS.md` still routes to the obsolete imagegen-chrome pipeline while the active direction is flat/Slate. Specifically:
- `UI_AGENTS.md:9` trigger words include `generated chrome, button plate, sprite sheet`.
- `UI_AGENTS.md:14` "Read First" points at `UI_GENERATION_INSTRUCTIONS.md` "for generated UI chrome."
- `UI_AGENTS.md:18` points at `MASTER_REFERENCE_UI_GENERATION_PROMPT.md`.
- `UI_AGENTS.md:25` bans "Pillow/PIL for generated UI art."

`UI_GENERATION_INSTRUCTIONS.md:9` is the smoking gun — its Core Goal is "Use image generation to create text-free raster UI chrome." That is precisely the pre-flat approach the user killed. `UI_FLAT_REDESIGN_REFERENCE.md:18,42,56` confirm the flat redesign *replaces* the PNG-composited/imagegen chrome system.

**Recommended Codex actions (proposal-then-edit):**
1. **Remove/archive the obsolete imagegen-chrome docs**: `UI_GENERATION_INSTRUCTIONS.md` and `MASTER_REFERENCE_UI_GENERATION_PROMPT.md` (the entire generate-text-free-raster-chrome pipeline + its prompt template).
2. **Rewrite `UI_AGENTS.md`** so routing, trigger words, and hard rules point only at the flat/Slate pipeline (fidelity loop, layout/sizing, screen-modal, flat redesign reference). Drop `generated chrome / button plate / sprite sheet / generated UI chrome / Pillow-for-UI-art` language.
3. **Reconcile, don't blindly keep, the other UI instruction docs** (`UI_FIDELITY_LOOP_INSTRUCTIONS`, `UI_SCREEN_WORKFLOW_INSTRUCTIONS`, `UI_IMPLEMENTATION_INSTRUCTIONS`, `UI_SCREEN_MODAL_INSTRUCTIONS`): purge embedded imagegen-chrome/sprite-sheet assumptions where they conflict with flat-native chrome.
4. **Fix stale handoff/audit references** that conflict: `UI_STAGE2_FRESH_AGENT_HANDOFF_REFERENCE.md`, `screen_name_resolver_audit.md`, and the dangling companion-doc path in `UI_FLAT_REDESIGN_REFERENCE.md:7` (it cites `UI\T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md`, but that file actually lives under `Audit\Reference\`).

**Critical preservation guardrails (must not over-delete):**
- **Keep imagegen for icons/content stubs.** The flat pipeline itself uses image generation for missing icons (`UI_FLAT_REDESIGN_REFERENCE.md:28,52,302` and §6.3). Do not blanket-strip "image generation" from UI docs — only the *raster UI chrome* usage is obsolete.
- **Do not touch the FriendSlop content imagegen pipeline** (~200 files under `FriendSlop\ImageGen\`, `ToonStyle`, `Model Generation`). That is content artwork, unrelated to UI chrome.
- **This is doc/process cleanup, not runtime.** Do not delete `FT66Style` PNG runtime code, `M_UI_Glow`, the Ultrakill plate library, or helpers — that is the doc's own Stage 3 runtime work, explicitly out of scope here.

## Evidence Checked
- `Reports/AgentReviews/20260605_UIProcessCleanup/original_prompt.md` (task contract + scope).
- `UI/UI_AGENTS.md` (full) — confirmed obsolete routing.
- `UI/Instructions/UI_GENERATION_INSTRUCTIONS.md` (full) — confirmed it IS the imagegen-chrome relic.
- `UI/Reference/UI_FLAT_REDESIGN_REFERENCE.md` (lines 1-620) — confirmed flat is the active pipeline and that imagegen-for-icons stays.
- Repo-wide grep for `imagegen/generated chrome` (210 hits) — confirmed most are legitimate content pipelines, not UI chrome.

## Questions Or Blockers
None requiring the user. The "what counts as a relic" judgment is Codex's to propose and mine to review per the protocol — it is not a user-only decision. If Codex wants to *hard-delete* (vs. archive) docs, prefer archive/move given irreversibility, but that doesn't gate the task.

## Caveats
- I read the two pivotal docs in full but only the first 620/855 lines of `UI_FLAT_REDESIGN_REFERENCE.md`, and did not open every UI instruction file. The exact relic list needs Codex's complete UI-folder inventory, which I'll confirm on pass-2 review.
- Watch for cross-references: deleting `UI_GENERATION_INSTRUCTIONS.md` / `MASTER_REFERENCE_UI_GENERATION_PROMPT.md` will leave dangling links in any doc that points to them (e.g., `UI/README.md`, audit/handoff docs). Codex must grep-and-fix those, or the cleanup just relocates the inconsistency.
- Stop condition requires Codex to report exact files changed plus verification (dead-link grep) — hold the task open until that lands.
