# Claude Full Operator Prompt: Hero 1 Pierce Vertical Slash VFX

You are Claude acting as the Operator for `C:\UE\T66`. Codex is the Validator/Finisher.

## Task Contract

Working task: build the Hero 1 / Chad 1 Pierce weapon projectile VFX as a forward vertical slash using the red, blue, and white visual/material language from the Hero 1 AOE attack.

Operator: Claude, `claude-opus-4-8`.

Validator: Codex.

Scope: `Hero_1_black_pierce`, Pierce attack category, Combat VFX binding/data setup, Hero 1 axe Pierce packet, reusable PathAnchored runtime spawn support, Pierce lab/production Niagara/material/mesh assets, validators/proof as needed.

Stop condition: validated implementation evidence, or a user-only decision gate. If you encounter a real `NEEDS_HUMAN_DECISION`, write or reference `Reports/AgentReviews/Hero1PierceVerticalSlashVFX/decision_block.md` and stop.

## Required Process Files

Use these live files as authority:

- `AGENTS.md`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `Reports/AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`
- `Gameplay/Combat/CombatVFXImpactContextContract.md`
- `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`
- `Gameplay/Combat/MASTER_COMBAT.md`
- relevant `pending_issues_*.md` files in touched folders

## Approved Scope

Codex approval artifact: `Reports/AgentReviews/Hero1PierceVerticalSlashVFX/codex_operator_approval.md`.

Approved work:

1. Update `Gameplay/Combat/Hero1AxePierceMechanismPacket.md` from scaffold to active vertical-forward-slash packet.
2. Add a Pierce production binding row for `Hero_1_black_pierce` in `Content/Data/CombatVFXBindings.csv`.
3. Regenerate `Content/Data/DT_CombatVFXBindings.uasset` through the owning setup script.
4. Create/update Pierce lab assets under `/Game/VFXLab/Hero1Axe/Pierce/` if needed for the reviewed flow.
5. Create/promote Pierce production assets under `/Game/VFX/Hero1/Axe/Pierce/`.
6. Extend `Source/T66/Gameplay/T66CombatComponent.cpp` so `PerformPierce` can spawn a bound PathAnchored production VFX from its official Pierce impact context.
7. Extend `TrySpawnBoundWeaponBaseSlashVFX` or its local helper logic for PathAnchored scaling/orientation without changing AOE behavior.
8. Extend `Scripts/SetupCombatVFXBindingsDataTable.py` and `Scripts/ValidateCombatVFXProductionBindings.py` so AOE checks remain intact and Pierce checks become required.
9. Add only minimal proof/capture automation required to validate this Pierce pass.
10. Write a completion packet at `Reports/AgentReviews/Hero1PierceVerticalSlashVFX/completion_packet.md`.

## Explicitly Excluded Actions

- Do not commit, push, tag, reset, checkout, clean, or run broad Git/LFS scans.
- Do not inspect or edit Mini/minigame systems.
- Do not implement DOT, Bounce, idols, or unrelated weapons.
- Do not touch credentials, billing, Anthropic API setup, or environment-token config.
- Do not destructively delete anything outside new Pierce lab/production artifacts created by this approved run.
- Do not claim final `FULL` visual-fidelity/Pablo approval unless the effect packet requirements and captured evidence actually support that. A structural implementation can be `PARTIAL` visually if final visual approval remains deferred.

## PPF / Artifact / Mechanism Requirements

Use `Reports/AgentReviews/Hero1PierceVerticalSlashVFX/plan_packet.md` as the Codex-approved plan basis.

Required discriminator: Pierce must be a PathAnchored forward vertical slash. It must not be the AOE crescent reused as a static/rotated lookalike. The primary silhouette must come from Niagara/material/renderer/emitter assets, not actor-side debug geometry.

Damage authority remains the Pierce combat query in `PerformPierce`; VFX is presentation. The visual should map to `LineLength` and `TubeRadius` where practical and preserve the official weapon impact context.

## Verification Expectations

Run or report:

- binding setup/DataTable refresh command and result,
- `Scripts/ValidateCombatVFXProductionBindings.py` result,
- focused C++ compile/build verification for changed runtime code,
- runtime logs proving `Hero_1_black_pierce` spawns the bound production VFX and applies Pierce damage,
- Unreal-owned gameplay capture/evidence bundle if the route succeeds.

If capture or asset automation is blocked, provide exact command, log path, error text, and what remains unverified. Do not substitute desktop screenshots.

## Output Requirements

In your final Operator artifact, include:

- changed files/assets,
- commands run and pass/fail markers,
- verification evidence paths,
- known caveats and any deferred visual acceptance,
- token/manifest awareness if available,
- whether the implementation stayed within Codex-approved scope.
