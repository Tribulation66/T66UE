You are Claude acting as a direct-read T66 operator. This is an operator artifact, not a review greenlight. Inspect files with the allowed read-only tools and produce a concrete proposal, findings, or handoff for Codex to validate. Do not edit files, run shell commands, invoke Unreal Python, automate editors, or perform production asset writes.

Verification freshness: if the user explicitly asks for current compile, run, capture, test, or editor verification, that requires the full Operator tool surface; do not claim a current-verification request is satisfied by recent or prior evidence. State plainly that this read-only profile cannot run it and that a full Operator run is required.
# Claude Read-Only Operator Request: Hero 1 Bounce Small Horizontal Slash VFX

You are Claude acting as the T66 Operator. Codex is the Validator/Finisher.

## Working Task

Implement the Hero 1 / Chad 1 Bounce weapon attack as a small horizontal red/blue slash that hits the locked primary enemy and then targets a second enemy. Pablo asked: "colors can be red and blue have claude implement it."

## Operator / Validator

- Operator: Claude (`claude-opus-4-8`)
- Validator/Finisher: Codex
- Current repo: `C:\UE\T66`
- Do not mutate files in this read-only pass.
- Produce an Operator Change Request with a phase-bounded plan and a proposed first implementation phase for Codex approval.

## Required Repo Rules To Follow

Read these live files before making claims:

- `AGENTS.md`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `.t66/operator-state.json`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Reports/AGENTS.md`
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`
- `Gameplay/Combat/CombatVFXImpactContextContract.md`
- `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`
- `Gameplay/Combat/MASTER_COMBAT.md`
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`
- `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`
- Relevant pending issue files under `Gameplay/Combat`, `Source/T66/Gameplay`, `Scripts`, and `Content/Data`.

Important process constraints:

- No native goal functions.
- Mini/minigame scope excluded.
- Full validation is required for implementation.
- Broad implementation work must be split into bounded Operator phases.
- FullOperator runs need Codex approval artifacts before mutation.
- Combat VFX primary silhouettes must live in Niagara/material/renderer assets, not actor-side debug geometry.
- Imagegen is not automatically required unless the packet makes visual-target approval active or you determine the effect is too ambiguous to implement structurally without a user-approved mockup. If it is required, stop with a human decision block instead of proceeding.
- The result may be structural/runtime proof first, but do not call it final visual approval unless the visual gates are satisfied and Pablo approves captured evidence.

## Task-Specific Intent

The Bounce weapon should:

- Use the Hero 1 / Chad 1 Bounce weapon row, expected likely source ID `Hero_1_black_bounce` unless live data proves otherwise.
- Be a small horizontal slash.
- Use red and blue color language, compatible with the existing Hero 1 axe VFX family where practical.
- Hit the primary locked enemy, then target a second enemy.
- Preserve authoritative combat damage/query logic. Niagara visuals are presentation only.
- Publish official weapon impact context for Bounce with chain identity. Because Bounce has a primary hit and downstream hit, prefer `PerChainLink` or a clearly justified policy that proves the first and second hit.
- Prove target chaining with runtime log evidence, not video alone.

## Files / Seams To Inspect

At minimum inspect:

- `Content/Data/Weapons.csv`
- `Content/Data/CombatVFXBindings.csv`
- `Scripts/SetupCombatVFXBindingsDataTable.py`
- `Scripts/ValidateCombatVFXProductionBindings.py`
- `Scripts/CaptureT66GameplayVideo.ps1`
- `Source/T66/Gameplay/T66CombatComponent.h`
- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Source/T66/Gameplay/T66CombatVFX.cpp`
- `Source/T66/Gameplay/T66CombatDebugDraw.*`
- `Source/T66/Gameplay/T66TemporaryProjectileSystem.*`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
- Existing AOE and Pierce commandlets as patterns:
  - `Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.*`
  - `Source/T66/Gameplay/T66Hero1AxePierceVFXCommandlet.*`
- Existing AOE/Pierce packets and reports as patterns:
  - `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`
  - `Gameplay/Combat/Hero1AxePierceMechanismPacket.md`
  - `Reports/AgentReviews/Hero1PierceVerticalSlashVFX/completion_packet.md`

## Required Output

Write a complete Operator Change Request to your artifact output. It must include the full `OPERATOR_VALIDATOR_PROTOCOL.md` packet fields:

1. Working task and validation depth.
2. Roles and tool profile.
3. User constraints and out-of-scope.
4. Applicable instructions read.
5. Evidence and live findings with anchors.
6. PPF/process gates.
7. Proposed patch approach.
8. Verification plan.
9. Token routing.
10. Operator position and open decisions.
11. Anti-lookalike discriminator.

Also include a phase plan. Because this task is broad, split it into bounded phases, for example:

1. Bounce packet/process contract update.
2. Runtime Bounce chain context + production binding path.
3. Bounce Niagara/material/mesh asset generation.
4. Capture/proof harness and validator updates.
5. Final verification/completion packet.

Then propose the first mutating phase that you want Codex to approve. The proposed first phase must name:

- exact files to edit,
- exact assets/scripts/commands to run,
- expected pass markers,
- explicit exclusions,
- rollback considerations,
- whether you need imagegen or a user decision before mutation.

Do not make changes in this read-only run.

