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
