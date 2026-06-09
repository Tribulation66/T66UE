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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_UIProcessAudit\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_UIProcessAudit\codex_draft_for_crossreview.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_UIProcessAudit\20260605T031954-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
# Original User Request

Hello I want to work on the UI for my game, we basically changed the art direction to a friendslop sort of game following kind of the visual direction of PEAK, not in terms of theme but in terms of UI elements shapes, and visual language. However this process of using codex to build the UI is not simple and weve tried a lot so first have you and claude go over the materials and information we have for UI and let me know if our instructions and processes and guidelines are unified and simple, or if we have multiple things saying multiple things and its not organized. The pass' objective is for you two to familiarize yourself with our UI processes as well as check if there are any problems before we start.

# Task Contract

Working task: Read-only audit of T66 UI process/material documentation, focused on whether UI instructions are unified, simple, current, and organized enough for upcoming Friendslop/PEAK-like UI direction work.
Operator: Codex
Validator: Claude
Scope: Inspect live repo UI routers, UI instructions, process docs, reference docs, relevant pending issues, and supporting source routing where needed. No code/content/runtime changes. Report problems, contradictions, overlaps, stale directions, missing ownership, and recommended cleanup sequence.
Stop condition: Return a synthesized Codex and Claude assessment with evidence paths, caveats, verification performed/skipped, and token usage.

# Relevant Repo Rules

- Start from live repo state, current folder instructions, current assets, current scripts, and current machine state.
- Root router: C:\UE\T66\AGENTS.md.
- Operator/Validator protocol: C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md.
- UI router: C:\UE\T66\UI\UI_AGENTS.md.
- Reports router: C:\UE\T66\Reports\AGENTS.md.
- Current role state from C:\UE\T66\.t66\operator-state.json: Codex Operator, Claude Validator.
- This is a read-only review/familiarization pass. Do not mutate files.
- Use local Claude Code CLI via Invoke-ClaudePlanReview.ps1 only; no Anthropic API billing.
- The user wants a practical assessment, not a giant inventory dump.

# Suggested Areas To Inspect

- UI\UI_AGENTS.md
- UI\README.md
- UI\Instructions\*.md
- UI\Reference\*.md
- UI\Processes\*.md
- UI\MASTER_REFERENCE_UI_GENERATION_PROMPT.md
- UI\Checklists\pending_issues_Checklists.md
- Source\T66\UI\pending_issues_UI.md
- Any obvious current UI source/asset routing docs needed to judge whether instructions are stale or contradictory.


</original_prompt>

<validator_independent_answer>
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

</validator_independent_answer>

<codex_draft>
Working task:
Operator: Codex
Validator: Claude
Scope: Read-only audit of T66 UI process/material documentation for unity, simplicity, contradictions, stale references, and readiness for upcoming Friendslop/PEAK-like UI direction work. No code/content/runtime changes.
Stop condition: Return synthesized Codex and Claude assessment with evidence paths, caveats, verification performed/skipped, and token usage.

Codex draft assessment:

Short answer: the UI process docs are not unified or simple enough to start cleanly. There is a usable current core, but it is buried under competing entry points, stale references, and partially finished process migration notes.

Usable current core:
- UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md and UI/Reference/UI_FLAT_REDESIGN_REFERENCE.md define the Stage 2 flat frontend path: pure Slate chrome through FT66FlatStyle, no PNG chrome, capture/dump/VerifyUIFidelity evidence, destructive removal of legacy chrome rather than additive layering.
- UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md provides orthogonal responsive/layout rules that still fit the flat path.
- UI/Reference/UI_STAGE2_CAPTURE_READINESS_REFERENCE.md and live T66PlayerController_Frontend.cpp show the current capture aliases are mostly ready.

Main problems:
1. Competing source-of-truth claims. UI/README.md and UI/MASTER_REFERENCE_UI_GENERATION_PROMPT.md say the master reference generation prompt is active for every screen/modal/state and replaces older prompt bundles. That prompt requires imagegen-generated raster UI chrome and atomic sprite sheets. The flat redesign reference says it is the single source of truth for the flat redesign and explicitly removes PNG-composited chrome in favor of Slate-native FT66FlatStyle. These cannot both be the broad default for future frontend work.
2. Broken/stale technical handoff reference. Several UI docs reference UI/T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md, but that file is missing. A likely moved copy exists under Audit/Reference/T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md.
3. Stale capture-readiness material. UI/screen_name_resolver_audit.md still says Overview, History, Diplomas, Drugs, Steam, and LoadGame have unresolved capture-name mismatches, but UI_STAGE2_CAPTURE_READINESS_REFERENCE.md and current source resolver entries show those aliases now exist.
4. Process automation debt is documented inconsistently. UI_FIDELITY_LOOP_INSTRUCTIONS.md describes auto-generated checklist phases, while UI_STAGE2_FRESH_AGENT_HANDOFF_REFERENCE.md and hero_selection_closeout_and_stage2_readiness.md say full checklist auto-generation does not yet exist and some work remains hand-authored/manual.
5. Asset/path conventions conflict. UI_FIDELITY_LOOP_INSTRUCTIONS.md points missing generated icons to SourceAssets/UI/Icons/Flat, while the icon manifest and runtime files use RuntimeDependencies/T66/UI/Icons/Flat. UI/content_stubs_registry.md is referenced as a registry but is missing.
6. HUD/in-game UI should not inherit the frontend Stage 2 loop unchanged. UI/hud_and_ingame_ui_inventory.md explicitly warns that HUD work needs a HUD-specific loop preserving gameplay semantic colors/readability.
7. Existing pending issues matter before broad UI work: Source/T66/UI/pending_issues_UI.md records the missing central controller focus contract; Scripts/pending_issues_Scripts.md records lack of deterministic tag-click capture support; UI/Checklists/pending_issues_Checklists.md records a stale Main Menu baseline.

Claude agreement and Codex nuance:
- Claude independently agreed the docs are not unified and identified the main conflict between raster/reference-generation instructions and flat Slate redesign instructions.
- Codex nuance: Claude grouped UI_FIDELITY_LOOP_INSTRUCTIONS.md with the PNG/imagegen system, but the live file itself is actually part of the flat FT66FlatStyle migration. The conflict is between the master generation prompt/README defaults and the flat redesign/fidelity-loop defaults.

Recommended cleanup before implementation:
1. Pick or write one canonical art-direction entrypoint for the next UI direction, then make UI_AGENTS.md and UI/README.md route through it.
2. Split process by task type: frontend flat screen, generated content/icon stub, raster reference chrome if still allowed, loot UI animation, HUD/in-game UI.
3. Restore or redirect the missing technical handoff path.
4. Archive or update stale docs such as screen_name_resolver_audit.md after validating live aliases.
5. Normalize asset paths/registries and define which proof gates are required versus aspirational/manual.

Verification performed:
- Read root AGENTS.md, OPERATOR_VALIDATOR_PROTOCOL.md, .t66/operator-state.json, Reports/AGENTS.md, UI/UI_AGENTS.md, UI/README.md, UI instructions, UI references, UI pending issues, Source/T66/UI pending issues, Scripts pending issues, and live frontend screen resolver source.
- Ran Claude independent answer through Scripts/Invoke-ClaudePlanReview.ps1 with Result OK and ClaudeTokensSpent 407169.
- No implementation, runtime build, Unreal capture, or screenshot verification was run because the requested pass is a read-only process/material audit.

</codex_draft>
