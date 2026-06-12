# Claude Read-Only Operator Request: Hero 1 Bounce VFX

You are Claude Operator. Codex is Validator/Finisher. Read-only only; do not edit.

Task: implement Hero 1 / Chad 1 Bounce as a small horizontal red/blue slash that hits the primary locked enemy and chains to a second enemy.

Use live repo state in `C:\UE\T66`. Required docs to read: `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `.t66/operator-state.json`, `Gameplay/GAMEPLAY_AGENTS.md`, `Reports/AGENTS.md`, `Gameplay/Combat/VFX_PROCESS_INDEX.md`, `Gameplay/Combat/CombatVFXAuthoringProcedure.md`, `Gameplay/Combat/CombatVFXDefinitionOfDone.md`, `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`, `Gameplay/Combat/CombatVFXImpactContextContract.md`, `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`, and relevant pending issues.

Inspect only the minimum code/data needed around Bounce: `Content/Data/Weapons.csv`, `Content/Data/CombatVFXBindings.csv`, `Scripts/SetupCombatVFXBindingsDataTable.py`, `Scripts/ValidateCombatVFXProductionBindings.py`, `Scripts/CaptureT66GameplayVideo.ps1`, `Source/T66/Gameplay/T66CombatComponent.*`, `Source/T66/Gameplay/T66CombatVFX.cpp`, `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`, `Source/T66/Gameplay/T66TemporaryProjectileSystem.*`, and AOE/Pierce commandlet patterns.

Output a concise Operator Change Request, max 140 lines. Include:

1. Task contract and full-validation depth.
2. Live findings with anchors.
3. Phase plan, because this is broad.
4. PPF/artifact/mechanism summary for Bounce.
5. Proposed first mutating phase for Codex approval: exact files, edits, commands, verification, exclusions, rollback.
6. Whether imagegen/user visual approval is required before structural implementation.
7. Anti-lookalike discriminator.
8. Token routing block.

Do not include long pasted code. Use path:line anchors and concise bullets.
