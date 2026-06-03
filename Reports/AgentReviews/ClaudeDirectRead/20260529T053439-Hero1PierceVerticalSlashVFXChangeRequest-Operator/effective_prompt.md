You are Claude acting as a direct-read T66 operator. This is an operator artifact, not a review greenlight. Inspect files with the allowed read-only tools and produce a concrete proposal, findings, or handoff for Codex to validate. Do not edit files, run shell commands, invoke Unreal Python, automate editors, or perform production asset writes.
# Claude Operator Change Request Prompt: Hero 1 Pierce Vertical Slash VFX

You are Claude acting as Operator for `C:\UE\T66`.

Do not edit files, run mutating commands, invoke Unreal/editor commandlets, generate assets, or capture video in this run. This run is for an Operator Change Request / Operator Packet only. Codex will validate your packet and may approve a later full Operator run.

## Task Contract

Working task: Build the Chad 1 / Hero 1 Pierce weapon projectile VFX as a forward vertical slash, using the same red, blue, and white visual language and reusable colors/materials/textures as the existing Hero 1 AOE attack where appropriate.
Operator: Claude.
Validator: Codex.
Scope: Combat VFX for Hero 1 Pierce. Assume the first production target is `Hero_1_black_pierce` unless live repo evidence shows that is unsafe or wrong. Do not include Mini/minigame scope.
Stop condition: Produce a complete Operator Change Request / Operator Packet that Codex can approve or reject before implementation, or produce a `NEEDS_HUMAN_DECISION` decision gate if a user-only decision blocks safe scope.

## User Direction

Pablo's visual direction:

- "Pierce weapon projectile"
- "it should indeed be a slash but a vertical slash forward"
- "We can use the red and blue and white, same colors and materials/textures as the aoe attack."

Interpretation to verify against live files:

- This should be a slash/force-wave visual, not a bullet-like projectile.
- It should be forward/lane-oriented for Pierce.
- It should reuse the Hero 1 AOE material/color language when that is already authored and reusable.

## Required Live Reads

Read the relevant live repo files before proposing scope. Include line anchors in your packet for load-bearing claims.

Required process docs:

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
- `Gameplay/Combat/pending_issues_Combat.md`

Relevant VFX/reference docs:

- `Gameplay/Combat/Hero1AxeVFXPlan.md`
- `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`
- Any existing Hero 1 Pierce packet/doc if present.
- Existing AOE proof/script docs only as needed to mirror the successful path.

Relevant runtime/data/tooling anchors to inspect:

- `Content/Data/Weapons.csv` entries for `Hero_1_*_pierce`
- `Content/Data/Heroes.csv` entry for `Hero_1`
- `Content/Data/CombatVFXBindings.csv` if present
- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Source/T66/Gameplay/T66CombatVFX.cpp`
- `Source/T66/Gameplay/T66HeroOneAttackVFX.cpp`
- `Source/T66/Gameplay/T66HeroOneAttackVFX.h`
- `Source/T66/Gameplay/T66TemporaryProjectileSystem.cpp`
- `Scripts/SetupCombatVFXBindingsDataTable.py`
- `Scripts/ValidateCombatVFXProductionBindings.py`
- `Scripts/CaptureT66GameplayVideo.ps1`
- Any Hero 1 AOE proof wrappers or capture wrappers that should be mirrored.

## Required Output

Write a complete Operator Packet / Change Request with these sections:

1. Working Task And Validation Depth
2. Roles And Tool Profile
3. User Constraints And Out Of Scope
4. Applicable Instructions Read
5. Evidence And Live Findings
6. PPF And Process Gates
7. Proposed Patch Approach
8. Verification Plan
9. Token Routing
10. Operator Position And Open Decisions
11. Anti-Lookalike Discriminator

The Proposed Patch Approach must be concrete enough for Codex approval. For each proposed file/script/data/asset path, include:

- exact path,
- planned change,
- why it is needed,
- expected blast radius,
- rollback note.

If you believe the next phase should include imagegen first, say so clearly and make that a scope decision. If the current user direction is enough to proceed without imagegen, say so and explain why.

Do not ask multiple reworded questions. If one user-only decision blocks implementation, emit a decision gate and stop.

