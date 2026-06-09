# Original User Prompt

I want to build something that isnt a obstacle or trap but rather its an interactable called TNT and what it does is after you interact with and a few seconds go by it explodes, and damages everything around it, heros and enemies.

# Task Contract

Working task:
Operator: Codex
Validator: Claude, if the local validator path is available
Scope: add a new TNT world interactable that can be triggered by player interaction, waits a few seconds, then explodes and damages nearby heroes and enemies; integrate it with existing gameplay/interactable patterns and verify the build/runtime path.
Stop condition: implementation is complete, focused verification is run, and any unverified runtime behavior is clearly called out.

# Repo Rules

- Root process router: `C:\UE\T66\AGENTS.md`.
- Operator/Validator protocol: `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md`.
- Current role state: Codex operator, Claude validator.
- Gameplay router: `C:\UE\T66\Gameplay\GAMEPLAY_AGENTS.md`.
- World router: `C:\UE\T66\Gameplay\World\WORLD_AGENTS.md`.
- Existing interactable base: `Source/T66/Gameplay/T66WorldInteractableBase.*`.
- Prefer data-authored tuning where practical, but a narrow first-pass C++ default is acceptable if scoped and editable.
- Runtime-facing gameplay changes require focused compile and staged standalone validation.

# Current Assumptions For This Pass

- TNT is not a trap or obstacle and should not be added to the trap subsystem.
- TNT should be a reusable `AT66WorldInteractableBase` subclass.
- TNT should be Lab Collector-spawnable for live testing.
- TNT should not yet be randomly spawned into tower generation unless the user asks for map distribution/balance.
- Placeholder primitive visuals are acceptable for this gameplay pass; no new Pixal3D asset generation/import is in scope.
