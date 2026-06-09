Original user question:

The user accepts the stronger no-manual-visual-authoring direction, but is concerned that even terms like crop, alpha extraction, and contact sheets can be abused. The example is that the title was approached as a crop from the full reference instead of regenerating a clean title-only asset, and bad crop/contact-sheet outputs contained text fragments, icons, or wrong boundaries. The user asks whether that concern is valid, how to explain the boundary, and how to organize the 48 FriendslopStyle markdown files so future agents do not mistake iteration artifacts for current rules. They suggest a README that explains the structure and one central UI process file.

Current task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: process-analysis only: clarify crop/alpha/contact-sheet boundary and propose organization for 48 FriendslopStyle markdown files. No file edits yet.
Stop condition: give recommended rule structure and document organization plan.

Live repo facts checked:

- `.t66/operator-state.json`: Codex operator, Claude validator.
- `ANTHROPIC_API_KEY`: not set in Process/User/Machine scope.
- There is no `UI/FriendslopStyle/README.md`.
- There are 48 markdown files under `UI/FriendslopStyle`.
- Folder count summary:
  - 2 root FriendslopStyle docs
  - 2 Checklists docs
  - 4 Elements docs
  - 1 Geometry doc
  - 6 SliceSpecs docs
  - 1 SourcePrompts/MainMenu prompt
  - 32 Reference/MainMenu docs under Round01-Round06 prompts/workers/manifests
- Search for `deprecated|archive|historical|source of truth|do not use` found no clear deprecation/router markers for older iteration docs.
- Existing likely current docs:
  - `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`
  - `UI/FriendslopStyle/friendslop_asset_registry.md`
  - `UI/FriendslopStyle/Checklists/main_menu_checklist.md`
  - `UI/FriendslopStyle/Checklists/visual_scorecard_template.md`
  - `UI/FriendslopStyle/Elements/main_menu_element_manifest.md`
  - `UI/FriendslopStyle/Elements/main_menu_round06_production_plate_plan.md`
  - `UI/FriendslopStyle/Elements/main_menu_pass13_component_contract.md`
  - `UI/FriendslopStyle/Elements/main_menu_pass14_component_contract_addendum.md`
  - `UI/FriendslopStyle/Geometry/main_menu_reference_geometry.md`
  - `UI/FriendslopStyle/SliceSpecs/main_menu_slice_specs.md`
  - `UI/FriendslopStyle/SliceSpecs/main_menu_round06_production_slice_specs.md`
- Existing likely historical/iteration docs include Round01-Round06 reference prompts/workers/manifests and old slice specs such as clean_sheet, inpaint, pass11, pass12 reference-inpaint.

Please provide an independent validator answer. Focus on:

1. Whether the user's concern about crop/alpha/contact-sheet language is correct.
2. A precise boundary: when crop/alpha/contact sheets are allowed, and when they must fail/stop/regenerate instead.
3. A suggested organization for the 48 markdown files, including central process doc, README/router, active docs, screen-specific docs, and archived/history docs.
4. Any doc wording that prevents future agents from using crop/manual tools to sneak in production pixels or baked text/title/icon content.
