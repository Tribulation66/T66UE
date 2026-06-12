You are Claude acting as the full-access T66 Operator.

Codex approval artifact: C:\UE\T66\Reports\AgentReviews\Hero1BounceProjectileTravelFix\codex_operator_approval.md

Codex has approved you to make changes inside the approved task contract and scope. You may use the normal Claude Code tool surface available in this environment, including file edits, shell commands, and configured MCP/editor tools such as Blender or other available MCP servers, when they are needed for the approved task.

You must stop and report Codex Approval Required: before any material scope expansion, destructive operation, credential or billing change, git commit, git push, git tag, git reset, git clean, broad Git/LFS scan over Unreal binary asset folders, or any action that contradicts AGENTS.md or folder-owned instructions. If you are unsure whether an action is inside the approved scope, stop and request Codex approval instead of doing it.

Verification freshness: if the user explicitly asks for current compile, run, capture, test, or editor verification, you must attempt that exact current verification now unless it is physically impossible. Recent or prior evidence does not satisfy an explicit current-verification request; if you cannot run it, say so explicitly instead of substituting older evidence.

Your output is an Operator work artifact and is not a greenlight. Codex will validate your actual changes, run or review verification, and write the final user-facing report.
# Claude Operator Prompt - Hero 1 Bounce Projectile Travel Fix

You are the Claude Operator for `C:\UE\T66`. Codex is the Validator/integrator. Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Gameplay/GAMEPLAY_AGENTS.md`, `Gameplay/README.md`, `Reports/AGENTS.md`, and the combat VFX process docs listed below. Mini/minigame scope is excluded.

## Working Task

Fix the Hero 1 Bounce weapon behavior/proof so Bounce is a moving two-link projectile sequence:

1. The hero shoots exactly one visible Bounce projectile/slash carrier toward the locked primary enemy.
2. After that projectile reaches/hits the primary enemy, exactly one visible Bounce projectile/slash carrier flies from that primary enemy to a second enemy.
3. Do not spawn three simultaneous projectiles or static impact-only slashes as the accepted behavior.

The result must preserve Bounce damage, target selection, chain damage, production binding structure, and the red/blue/white Hero 1 slash vocabulary.

## Current Diagnosis To Verify

The live `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md` currently says Bounce is an `ImpactAnchored` slash placed at each chain impact. That conflicts with Pablo's correction in this turn. Bounce should instead be a moving projectile/link carrier:

- first link: hero attack origin to primary enemy,
- second link: primary enemy impact point to the next chained enemy,
- one visible projectile/link at a time for the requested first-pass proof.

Existing code anchors to inspect:

- `Source/T66/Gameplay/T66CombatComponent.cpp`
  - `PerformBounce`
  - `TrySpawnBoundWeaponBaseSlashVFX`
  - existing temporary projectile / Bounce VFX helpers
- `Source/T66/Gameplay/T66CombatComponent.h`
- `Source/T66/Gameplay/T66HeroProjectile.*` only if needed for visual-only moving projectile behavior
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` for `hero1axebouncevfxbinding` capture/proof target setup
- `Content/Data/CombatVFXBindings.csv`
- `Scripts/ValidateCombatVFXProductionBindings.py`
- existing Bounce commandlet/assets only if the carrier itself must be changed to support the moving visual

## Applicable Instructions To Read

- `AGENTS.md`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/README.md`
- `Reports/AGENTS.md`
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`
- `Gameplay/Combat/CombatVFXImpactContextContract.md`
- `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`
- `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`
- `Gameplay/Combat/MASTER_COMBAT.md`
- `Gameplay/Combat/pending_issues_Combat.md`
- `Source/T66/Gameplay/pending_issues_Gameplay.md`

## PPF / Process Gate

PPF CHECK
Objective: Revise Hero 1 Bounce from static impact-only slash placement to a moving two-link projectile sequence: one projectile from hero to primary, then one projectile from primary to second enemy.
Proven process: Hero 1 combat VFX Niagara/material/mesh production pipeline plus `CombatVFXAuthoringProcedure.md` Bounce/BeamHop guidance, visual/damage alignment contract, and impact-context contract.
My planned implementation: Update the Bounce packet and runtime so the accepted primary behavior is a moving path/link carrier. The visual carrier must still be authored through Niagara/material/renderer/projectile visual systems owned by the combat VFX path, not debug geometry. Runtime should stage one visible link at a time and preserve per-link impact contexts/damage authority.
Same method class: YES if the primary visual is a real moving projectile/link carrier and remains bound to combat VFX/projectile presentation. NO if the result is static target slashes, actor-side debug geometry, or multiple simultaneous projectiles.
If NO, why: Stop and report the conflict instead of substituting.
User approval required before proceeding: NO for this bounded fix; the user's latest message is the correction and asks to return to projectile behavior.
Verification evidence: focused compile, packet/code anchors, production validator if binding/asset assumptions change, runtime log proof of one first-link projectile then one second-link projectile, and Unreal-owned gameplay capture/video evidence.

ARTIFACT PARITY GATE
Reference artifact/category: Moving Bounce projectile/link carrier
Role: Primary
Required: YES
Planned artifact/path: update the live Bounce packet and runtime path for Hero1Axe_Bounce_Base so it produces visible moving links hero->primary and primary->second.
Status: SAME if implemented as moving link/projectile carrier; MISSING if only target impact slashes remain.
Evidence: frame-range video and logs proving sequence order and one-at-a-time count.

MECHANISM MANIFEST
Reference/source: Pablo's current correction plus `CombatVFXAuthoringProcedure.md` Bounce/BeamHop carrier guidance.
Required mechanisms:
  1. Mechanism: First projectile movement from hero to primary
     Required: YES
     Planned implementation: Spawn/stage exactly one visible moving Bounce link from hero attack origin to the primary target impact point.
     Evidence needed: logs plus multi-frame capture showing travel before primary impact.
  2. Mechanism: Second projectile movement from primary to second enemy
     Required: YES
     Planned implementation: After the first link reaches the primary, spawn/stage exactly one visible moving Bounce link from the primary impact point to the next chain target impact point.
     Evidence needed: logs plus multi-frame capture showing delayed second-link travel.
  3. Mechanism: Single projectile/link at a time for the requested proof
     Required: YES
     Planned implementation: Do not spawn all chain visuals simultaneously. First link precedes second link.
     Evidence needed: logs/capture showing no three-projectile burst.
  4. Mechanism: Existing damage and per-link impact authority preserved
     Required: YES
     Planned implementation: Keep Bounce combat damage/query semantics and per-link official impact contexts; VFX remains presentation.
     Evidence needed: damage proof and `CombatImpactContext`/production spawn logs.

Anti-lookalike discriminator: The cheap wrong result is static slashes appearing at targets or three simultaneous projectiles. The discriminator is temporal proof that one projectile visibly travels hero-to-primary, then only after that a second projectile travels primary-to-secondary.

## Approved Edit Scope

Allowed:

- `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`
- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Source/T66/Gameplay/T66CombatComponent.h`
- `Source/T66/Gameplay/T66HeroProjectile.h`
- `Source/T66/Gameplay/T66HeroProjectile.cpp`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
- `Content/Data/CombatVFXBindings.csv` only if the binding contract needs a note or scale/timing correction
- `Scripts/ValidateCombatVFXProductionBindings.py` only if required to validate the revised Bounce contract
- Bounce VFX commandlet/assets only if needed for the moving carrier to render correctly
- report files under `Reports/AgentReviews/Hero1BounceProjectileTravelFix/`

Explicitly excluded:

- Mini/minigame files.
- DOT, Pierce, AOE, idols, unrelated weapons.
- Balance/stat retuning.
- Git commit/push/tag/reset/clean or broad Git/LFS scans.
- Replacing the carrier with debug geometry or a static non-moving lookalike.

## Verification Expected

Run what is needed for the actual changes:

1. Focused compile:
   `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReloadFromIDE`
2. If assets/bindings change, regenerate/revalidate the affected Bounce assets/DataTable and run:
   `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=pythonscript -script='C:\UE\T66\Scripts\ValidateCombatVFXProductionBindings.py' -unattended -nop4 -nosplash`
3. If you can run capture inside the approved scope, use the Unreal-owned capture process for `hero1axebouncevfxbinding`; otherwise report the exact capture command Codex must run.

## Deliverable

Write a completion report under:

`Reports/AgentReviews/Hero1BounceProjectileTravelFix/claude_operator_report.md`

The report must include:

- Files changed.
- Root cause and the old wrong behavior.
- Exact new behavior and timing/sequence rule.
- Whether the packet was updated from impact-only to moving projectile/link.
- Commands run with pass/fail evidence and log paths.
- Any skipped verification and why.
- Caveats Codex must validate with final gameplay video.

