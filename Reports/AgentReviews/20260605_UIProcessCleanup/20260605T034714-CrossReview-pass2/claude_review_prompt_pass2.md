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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_UIProcessCleanup\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_UIProcessCleanup\codex_draft_for_crossreview.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_UIProcessCleanup\20260605T033626-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
User request:

Ok great, so you did find the descrapancy I was worried about, so the reason we have that discrepency is ebcause before we went in the direction of the flat style, we had an imagegen based chrome approach, which we got rid of. So the first thing we need to do is fully clean that up and remove that, as well as everything that is not part of the clear and obejctive pipeline and seems to be relics or unrelated instructions we can get rid of. Let me know when done.

Task contract:

Working task: Clean up T66 UI documentation/process instructions by removing the obsolete imagegen-based chrome pipeline and stale/unrelated relic instructions, keeping a clear objective flat/Slate UI pipeline.
Operator: Codex
Validator: Claude
Scope: Inspect and edit live repo UI routers, UI instructions, UI reference/process docs, and directly related stale handoff/audit references. No runtime UI implementation, build, capture, release, or git operations unless required by the documentation cleanup.
Stop condition: Updated docs are internally consistent, obsolete imagegen-chrome references are removed from active routing, stale or deleted relic docs no longer conflict with the active pipeline, Claude has reviewed, and Codex reports exact files changed plus verification.

Relevant repo rules:
- Follow C:\UE\T66\AGENTS.md and OPERATOR_VALIDATOR_PROTOCOL.md.
- Codex is Operator and Claude is Validator per .t66/operator-state.json.
- Do not use native goal tools.
- Use UI/UI_AGENTS.md as the UI folder router.
- Use Reports/AGENTS.md for review artifacts.
- Preserve the distinction between obsolete imagegen UI chrome and still-allowed content artwork/icon-stub generation in the flat pipeline.
- This is a documentation/process cleanup, not a visual/UI runtime implementation pass.

</original_prompt>

<validator_independent_answer>
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

</validator_independent_answer>

<codex_draft>
Working task:
Operator: Codex
Validator: Claude
Scope: Clean up T66 UI documentation/process instructions by removing the obsolete imagegen-based chrome pipeline and stale/unrelated relic instructions, keeping a clear objective flat/Slate UI pipeline. No runtime UI implementation, build, capture, release, or git operations.
Stop condition: Updated docs are internally consistent, obsolete imagegen-chrome references are removed from active routing, stale/deleted relic docs no longer conflict with the active pipeline, Claude has reviewed, and Codex reports exact files changed plus verification.

Summary of Codex changes:

1. Active UI routing now points to the flat Slate pipeline:
   - Updated UI/UI_AGENTS.md to make UI_FLAT_REDESIGN_REFERENCE.md, UI_FIDELITY_LOOP_INSTRUCTIONS.md, UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md, UI_STAGE2_CAPTURE_READINESS_REFERENCE.md, and Audit/Reference/T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md the read-first set.
   - Removed generated-chrome/button-plate/sprite-sheet trigger language.
   - Added hard rules that raster generation is not allowed for chrome, while content stubs and flat icon glyphs remain allowed only when the flat pipeline explicitly calls for them.

2. UI/README.md is now the simple entry point:
   - States the active frontend pipeline is flat Slate through FT66FlatStyle.
   - Lists the read order and active rules.
   - Separates special processes from the default frontend pipeline.
   - States the retired imagegen-chrome prompt and sprite-sheet workflow are removed and must be recovered from Git history only for historical audit work.

3. Removed obsolete imagegen-chrome instruction/prompt files from active UI:
   - Deleted UI/MASTER_REFERENCE_UI_GENERATION_PROMPT.md.
   - Deleted UI/Instructions/UI_GENERATION_INSTRUCTIONS.md.
   - Deleted UI/Instructions/UI_SCREEN_MODAL_INSTRUCTIONS.md.
   - Deleted UI/Instructions/UI_SCREEN_WORKFLOW_INSTRUCTIONS.md.
   - Deleted UI/Instructions/UI_IMPLEMENTATION_INSTRUCTIONS.md.

4. Removed or moved relic/unrelated files out of active UI instructions:
   - Deleted stale UI/screen_name_resolver_audit.md because it contradicted current Stage 2 capture readiness/live resolver state.
   - Moved UI/Instructions/UI_SPRITE_RETRO_PROCESS_INSTRUCTIONS.md to Tools/ArtPipeline/Items/ITEM_SPRITE_RETRO_PROCESS.md and updated Tools/README.md. This preserved the live item sprite process while removing it from UI guidance.
   - Moved UI/Instructions/UI_MAIN_MENU_VIDEO_BACKGROUND_INSTRUCTIONS.md to UI/Processes/MainMenuVideoBackgroundProcedure.md and referenced it as a special process only.

5. Patched surviving flat/support docs:
   - UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md now points at Audit/Reference/T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md, records content stubs in UI/content_stubs_registry.md, fixes icon output path to RuntimeDependencies/T66/UI/Icons/Flat, and no longer assumes checklist auto-generation exists.
   - UI/Reference/UI_FLAT_REDESIGN_REFERENCE.md and UI/Reference/UI_STAGE2_FRESH_AGENT_HANDOFF_REFERENCE.md now point at the real technical handoff path.
   - UI/hud_and_ingame_ui_inventory.md now references the real technical handoff path.
   - Audit/Reference/T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md no longer says generated chrome assets are current source truth and no longer lists old UI/Processes generation docs.

6. Added missing ledger:
   - Added UI/content_stubs_registry.md with known Hero Selection content stubs from the existing closeout notes.

Verification performed:
- Read root AGENTS.md, OPERATOR_VALIDATOR_PROTOCOL.md, .t66/operator-state.json, Reports/AGENTS.md, UI/UI_AGENTS.md, UI/README.md, UI instructions/references, pending issue docs, Tools/README.md, Audit/README.md, and the active technical handoff excerpt.
- Ran Claude independent answer with Result OK and ClaudeTokensSpent 197722.
- Ran active stale-reference sweep:
  rg -n "MASTER_REFERENCE_UI_GENERATION_PROMPT|UI_GENERATION_INSTRUCTIONS|UI_SCREEN_MODAL_INSTRUCTIONS|UI_SCREEN_WORKFLOW_INSTRUCTIONS|UI_IMPLEMENTATION_INSTRUCTIONS|screen_name_resolver_audit|UI/Processes/UI_GENERATION|UI/Processes/SCREEN_MODAL_TASK|UI/SCREEN_WORKFLOW|UI/Processes/LAYOUT_AND_SIZING|generated UI chrome|generated chrome|text-free raster UI chrome|Pillow/PIL|local pixel repair" C:\UE\T66\UI C:\UE\T66\Tools C:\UE\T66\Source\T66\UI C:\UE\T66\Audit\Reference -g "*.md" -g "*.h" -g "*.cpp"
  Result: no matches after final patch.
- Ran technical handoff path sweep. Remaining matches point to Audit/Reference/T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md, which exists.
- Ran path existence checks for current read-first docs, special process docs, moved item process doc, icon manifest, and content stubs registry; all returned Exists=True.
- Ran git diff --check for UI, Tools, Audit/Reference/T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md, and the task review folder. Result: no whitespace errors; only LF/CRLF normalization warnings on touched Markdown files.
- No build, Unreal capture, runtime screenshot, or staged standalone verification was run because this is documentation/process cleanup only.

Known caveat:
- Historical Audit/Finished and Reports artifacts still mention old filenames as history. The cleanup target was active UI routing/support docs, not rewriting closed audit trails or review artifacts.

</codex_draft>
