Original user request:

> Ok I dont mind these suggestions in any case we will continue to improve the process go ahead and implement Phase 1 and Phase 2 and then with the changes, we adopted go ahead and do another version of the screen attempting to get closer to the reference image.

Current task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: implement FriendslopStyle doc organization Phase 1 and Phase 2, then produce a new Main Menu screen pass closer to the Round06 reference using the updated no-manual-visual-authoring process.
Stop condition: docs are reorganized with a router/central authority, stale iteration docs are clearly separated, a fresh screen pass is produced with artifacts, and verification evidence or blockers are reported.

Relevant constraints:

- Do not use native goal tools.
- Codex is Operator, Claude is Validator per `.t66/operator-state.json`.
- `ANTHROPIC_API_KEY` is not set in Process/User/Machine scope.
- Do not use API imagegen fallback or `OPENAI_API_KEY`.
- Built-in account-backed imagegen is the approved visual-asset generation path.
- Manual/Pillow/OpenCV/skimage/ImageMagick/etc. may be used for measurement, crop/alpha cleanup of already approved generated candidates, slice tests, contact sheets, and validation only. They may not author, patch, inpaint, clone, smooth, recolor, synthesize, or salvage production UI pixels.
- Reference crops are comparison/measurement targets only. They are not runtime asset sources. Title must not be cropped from the full reference.
- Contact sheets are evidence only. Nothing on a contact sheet is imported.
- If a component visually fails because the asset pixels are wrong, regenerate or replace the asset; do not hide the failure with manual patching, pillow centers, blur fills, tint-only changes, or masked live-content zones.
- If imagegen cannot produce a needed asset due to TooManyRequests/session issues, stop/fork rather than continuing with approximation assets.
- UI visual work must follow `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`, `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md`, and `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`.
- Use Unreal-owned capture paths and `VerifyUIFidelity.py`; structural PASS counts are not enough. Visual scorecard controls acceptance.

Planned doc organization from prior user-approved analysis:

- Add `UI/FriendslopStyle/README.md` router.
- Declare `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` the central Friendslop UI process authority.
- Tighten central process with explicit visual asset authorship/provenance rules and replace permissive correction wording.
- Reorganize or clearly archive historical iteration docs so future agents do not treat old prompts/inpaint/clean-sheet/pass artifacts as current rules.
- Suggested stronger structure:
  - `UI/FriendslopStyle/Screens/MainMenu/` for current Main Menu docs.
  - `UI/FriendslopStyle/Reference/MainMenu/Current/` for current reference files.
  - `UI/FriendslopStyle/Archive/ReferenceIterations/` for old Round01-Round05 and noncurrent Round06 prompt/worker history.
  - `UI/FriendslopStyle/Archive/DeprecatedSliceSpecs/` for inpaint/clean_sheet/pass11/pass12 specs.

Need independent validator answer:

1. Confirm whether this implementation scope is safe/clear or identify blockers.
2. Suggest the safest doc reorg steps that preserve active references and avoid breaking path consumers.
3. Identify high-risk files/scripts/code likely to need update if paths move.
4. Give guidance for the next screen pass under the new no-manual-visual-authoring rule.
5. Do not mutate files; read-only advice only.
