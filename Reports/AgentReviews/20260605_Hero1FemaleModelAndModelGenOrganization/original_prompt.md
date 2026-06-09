# Original Prompt And Task Contract

## User Request

The user first asked to finish the information-gathering pass for the Blender-only Fall Guys/rubber material mission:

- Determine whether changing the Pixel3D/source-image inputs is likely to help more than Blender material/look-dev.
- Use the existing model for the next Blender work.
- Check which model the game currently uses for Hero 1 female.
- Hero 1 male is already confirmed to use the correct FriendSlop model.
- Avoid Unreal work; Blender material work is a later next step.

The user then expanded the request before final answer:

- Continue and finish the previous information gathering.
- Before answering, implement organization cleanup under Model Generation.
- Make the Model Generation README/docs clear that current work should use FriendSlop models, not the AccuRig ones.
- Put the newly generated FriendSlop Pixel3D models in the clear Model Generation location.
- Archive old/non-current model generations so future agents do not confuse them with active sources.
- Do not stop at reporting; organize and clean first, then answer.

## Working task

Operator: Codex
Validator: Claude
Scope: Finish the Hero 1 female/current-model and Pixel3D-vs-Blender investigation, then organize Model Generation so current FriendSlop/Pixal3D easy-difficulty models are clearly the active source set, unused legacy/generated model batches are archived, and README/process docs remove the AccuRig ambiguity. No Unreal work, no Blender look-dev edits yet, no deletion, no Git operations.
Stop condition: Provide the current model finding, the Blender/source-generation recommendation, exact organization changes, archive paths, docs updated, and verification/token counts.

## Repo Rules

- Root router: `AGENTS.md`.
- Role state: `.t66/operator-state.json` selects Codex Operator and Claude Validator.
- Operator/Validator process: `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Model Generation router: `Model Generation/MODEL_GENERATION_AGENTS.md`.
- Read model-generation instructions and pending issue docs before editing.
- Do not call native goal tools.
- Do not run Unreal or Blender for this cleanup; this is organization and documentation before the later Blender material pass.
- Do not delete assets for this requested organization step. Archiving/moving under Model Generation is allowed when path-safe and documented.
