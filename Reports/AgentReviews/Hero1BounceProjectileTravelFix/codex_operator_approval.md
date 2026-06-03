Codex Approval: APPROVE

# Codex Operator Approval - Hero 1 Bounce Projectile Travel Fix

Approved operator: Claude
Validator/integrator: Codex
Tool profile: FullOperator through `Scripts\Invoke-ClaudeDirectRead.ps1`

## Approved Task

Fix Hero 1 Bounce so the accepted visible projectile behavior is one moving link from hero to primary enemy, then one moving link from that primary enemy to a second enemy. Static impact-only slashes and three simultaneous projectiles do not satisfy the user's correction.

## Approved Scope

Allowed edits:

- `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`
- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Source/T66/Gameplay/T66CombatComponent.h`
- `Source/T66/Gameplay/T66HeroProjectile.h`
- `Source/T66/Gameplay/T66HeroProjectile.cpp`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
- `Content/Data/CombatVFXBindings.csv` only if the revised Bounce contract requires binding timing/notes correction
- `Scripts/ValidateCombatVFXProductionBindings.py` only if it must validate the revised Bounce contract
- Bounce VFX commandlet/assets only if needed to make the moving carrier render correctly
- report files under `Reports/AgentReviews/Hero1BounceProjectileTravelFix/`

## Approved Tool Surface

Full Claude Operator tool surface through the local Claude Code CLI, using the user's Claude subscription. Shell/build/editor/capture commands are approved only inside the task scope.

## Required Process Rules

- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Gameplay/GAMEPLAY_AGENTS.md`, `Reports/AGENTS.md`, and the Combat VFX process docs.
- Preserve damage authority in combat logic; VFX remains presentation.
- Preserve per-link official Bounce impact contexts where they already exist.
- Primary proof must show motion over time, not a single still.
- The carrier must not be debug geometry, static target-only slash spawn, or the legacy three-projectile burst behavior.

## Explicitly Excluded Actions

- Do not touch Mini/minigame scope.
- Do not change DOT, Pierce, AOE, idols, unrelated weapons, balance, or item stat tuning.
- Do not use Git commit/push/tag/reset/clean.
- Do not perform broad Git/LFS scans.
- Do not claim final visual approval without Codex/user-facing Unreal-owned video validation.

## Verification Required After Operator Run

- Focused `T66Editor Win64 Development` compile for code changes.
- Combat VFX binding validator if binding/assets/scripts change.
- Runtime proof logs or capture command showing one projectile link hero->primary and one link primary->second, sequentially.
- Claude completion report with changed files, root cause, verification, and caveats.

## Approval Rationale

The live Bounce packet and prior implementation are now known to mismatch the user's intended projectile behavior. The scope is narrow enough to approve as one corrective Operator phase because it is limited to the Hero 1 Bounce packet, runtime visual staging, and directly relevant proof/capture surfaces.
