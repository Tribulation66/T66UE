Original user question:

The user is concerned that FriendslopStyle UI process docs need to prevent future agents from making manual/Pillow/masked/patched visual asset mistakes. They argue image generation should be treated as the only visual asset creation tool for premium UI elements, with manual/Pillow/OpenCV/skimage allowed only for measurement, extraction, alpha/crop/contact-sheet QA, and validation. They ask:

- Is that analysis correct, or should Codex push back?
- If correct, how exactly should this be reflected in the process documents?
- How many documents currently describe the process/rules for Friendslop UI screen building?

Current task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: process analysis only, plus a live count of Friendslop UI process/rule documents.
Stop condition: explain whether imagegen should be framed as the visual-asset authoring path, propose exact doc language/structure, and list/count current governing docs.

Relevant live repo facts already checked:

- `.t66/operator-state.json` configures Codex as Operator and Claude as Validator.
- `ANTHROPIC_API_KEY` is not set in Process/User/Machine scope.
- `AGENTS.md` says new bitmap visuals/mockups/sprites/reference images/image edits must use approved built-in/account-backed imagegen, no `OPENAI_API_KEY` fallback.
- `UI/UI_AGENTS.md` routes FriendslopStyle raster-chrome UI work through `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`.
- `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` says runtime chrome generation should create blank standalone plates, one component family at a time, with no labels/names/scores/text, alpha-clean transparent PNG, matched states, contact sheets, and comparison against cropped reference regions.
- That same doc currently says asset failures may be fixed by "regenerate, recrop, alpha-clean, or adjust slice margins", which is too permissive if agents read `recrop/alpha-clean` as permission to patch visual material.
- `Reports/AgentReviews/20260605_FriendslopProductionRedo/fresh_agent_main_menu_pass14_prompt_final.md` explicitly warned not to treat OpenCV/skimage/Pillow as asset-generation quality tools: they are fine for measurement/contact sheets/alpha validation/crop QA/verification but do not create premium runtime art by themselves.
- `UI/FriendslopStyle/Elements/main_menu_pass14_component_contract_addendum.md` contains a user-approved pass14-only exception for direct reference-derived runtime plates. It allows crop-derived plates with live-content zones removed only when strict gates pass, and says this is not a global permission for future screens.
- Live count found 48 markdown files under `UI/FriendslopStyle`, including many reference prompts/manifests/work requests and stale/historical slice specs. Codex is classifying 16 as current active governing/rule/spec/checklist docs:
  1. `AGENTS.md`
  2. `OPERATOR_VALIDATOR_PROTOCOL.md`
  3. `UI/UI_AGENTS.md`
  4. `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md`
  5. `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`
  6. `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`
  7. `UI/FriendslopStyle/friendslop_asset_registry.md`
  8. `UI/FriendslopStyle/Elements/main_menu_element_manifest.md`
  9. `UI/FriendslopStyle/Elements/main_menu_round06_production_plate_plan.md`
  10. `UI/FriendslopStyle/Elements/main_menu_pass13_component_contract.md`
  11. `UI/FriendslopStyle/Elements/main_menu_pass14_component_contract_addendum.md`
  12. `UI/FriendslopStyle/Checklists/main_menu_checklist.md`
  13. `UI/FriendslopStyle/Checklists/visual_scorecard_template.md`
  14. `UI/FriendslopStyle/Geometry/main_menu_reference_geometry.md`
  15. `UI/FriendslopStyle/SliceSpecs/main_menu_slice_specs.md`
  16. `UI/FriendslopStyle/SliceSpecs/main_menu_round06_production_slice_specs.md`

Please provide an independent validator answer. Focus on whether the proposed stronger rule should ban manual/Pillow/patched visual edits for production UI asset pixels, what nuance is needed, what exact doc/process changes should be made, and whether the document count/classification is reasonable.
