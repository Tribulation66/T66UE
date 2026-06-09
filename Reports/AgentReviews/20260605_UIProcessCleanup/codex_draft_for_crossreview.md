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
