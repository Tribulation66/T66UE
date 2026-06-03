You are Claude acting as the full-access T66 Operator.

Codex approval artifact: C:\UE\T66\Reports\AgentReviews\Hero1BounceProjectileTravelFix\codex_operator_approval_revise.md

Codex has approved you to make changes inside the approved task contract and scope. You may use the normal Claude Code tool surface available in this environment, including file edits, shell commands, and configured MCP/editor tools such as Blender or other available MCP servers, when they are needed for the approved task.

You must stop and report Codex Approval Required: before any material scope expansion, destructive operation, credential or billing change, git commit, git push, git tag, git reset, git clean, broad Git/LFS scan over Unreal binary asset folders, or any action that contradicts AGENTS.md or folder-owned instructions. If you are unsure whether an action is inside the approved scope, stop and request Codex approval instead of doing it.

Verification freshness: if the user explicitly asks for current compile, run, capture, test, or editor verification, you must attempt that exact current verification now unless it is physically impossible. Recent or prior evidence does not satisfy an explicit current-verification request; if you cannot run it, say so explicitly instead of substituting older evidence.

Your output is an Operator work artifact and is not a greenlight. Codex will validate your actual changes, run or review verification, and write the final user-facing report.
# Claude Operator Revision Prompt - Hero 1 Bounce Projectile Travel Fix

You are the Claude Operator for `C:\UE\T66`. Codex is the Validator/integrator.

The first operator pass compiled, but Codex found a validation blocker:

- The code now stages a sequential moving chain, but the moving carrier is the generic visual-only `AT66HeroProjectile` with `FT66TemporaryProjectileSystem::ProfileHeroBounce()`.
- That is a temporary cube/profile carrier, not the authored Hero 1 Bounce red/blue horizontal slash VFX carrier.
- The user asked for the Bounce weapon projectile to be the small horizontal slash behavior, and the Combat VFX process requires the primary VFX silhouette to live in the Niagara/material/renderer asset or its attached renderer logic, not be deferred to later polish.

## Revised Working Task

Revise the Hero 1 Bounce projectile travel fix so the accepted moving carrier is the authored Hero 1 Bounce red/blue horizontal slash VFX, travelling sequentially:

1. one moving slash/projectile from hero attack origin to the primary enemy,
2. then one moving slash/projectile from primary impact point to the second enemy,
3. no three-projectile simultaneous burst,
4. no static impact-only target slash as the accepted primary carrier,
5. no temporary cube as the accepted primary carrier.

The damage/target selection/per-link impact context work from the first pass should remain intact.

## Current State To Preserve

- `StageBounceProjectileChain` / `SpawnBounceLinkProjectile` exist in `Source/T66/Gameplay/T66CombatComponent.cpp`.
- `PerformBounce` publishes per-link impact contexts and stages the moving chain after damage resolves.
- Codex already patched `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` so the `hero1axebouncevfxbinding` proof harness stages only `Primary` and `ChainSecond` as expected-hit targets; do not reintroduce `ChainThird` as an expected hit for this proof.
- Focused editor build passed after the first pass and Codex harness patch.
- Combat VFX production binding validator passed after the first pass.

## Required Revision

Update the moving Bounce carrier so its primary visible silhouette is the authored Bounce slash VFX:

- Prefer resolving `Hero1Axe_Bounce_Base` / `Hero_1_black_bounce` / `AttackCategory=Bounce` and using the production Niagara system `/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash.NS_Hero1AxeBounce_MeshSlash` as the moving carrier, attached to a visual-only mover if needed.
- The temporary projectile actor may be used as a hidden mover/root/lifetime controller only if the visible primary carrier is the Niagara/material slash.
- If using `AT66HeroProjectile`, add a narrowly scoped way to attach the Bounce Niagara system as the visible primary carrier and hide or de-emphasize the temporary cube mesh.
- Keep the carrier visual-only: `Damage=0`, no projectile collision authority, no damage behavior moved into the visual.
- Ensure the link movement is readable in capture. If the raw `ProjectileSpeed=2400` makes a short 150uu link too fast, add a presentation-only minimum link travel duration using an existing binding timing value where practical (`BasePlaybackSeconds=0.32`) or a clearly named local constant. Damage timing must remain unchanged.

## Process Gate

This revision must satisfy the artifact parity gate:

ARTIFACT PARITY GATE
Reference artifact/category: Authored moving Bounce horizontal slash carrier
Role: Primary
Required: YES
Planned artifact/path: production Bounce Niagara slash VFX attached to/sequenced along the moving link path.
Status: must be SAME or PRESENT, not DEFERRED.
Evidence: code anchors plus runtime capture/log plan.

Anti-lookalike discriminator:

- Wrong result: a moving blue cube, a static target slash, or three simultaneous projectiles.
- Intended result: one authored red/blue horizontal slash carrier travels hero->primary, then one authored red/blue horizontal slash carrier travels primary->second.

## Allowed Edit Scope

Allowed:

- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Source/T66/Gameplay/T66CombatComponent.h`
- `Source/T66/Gameplay/T66HeroProjectile.h`
- `Source/T66/Gameplay/T66HeroProjectile.cpp`
- `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` only if needed to preserve/strengthen the two-target proof harness
- `Source/T66/Gameplay/pending_issues_Gameplay.md`
- report files under `Reports/AgentReviews/Hero1BounceProjectileTravelFix/`

Only if strictly necessary:

- `Content/Data/CombatVFXBindings.csv`
- `Scripts/ValidateCombatVFXProductionBindings.py`

Explicitly excluded:

- Mini/minigame scope.
- DOT, Pierce, AOE, idols, unrelated weapons.
- Balance/stat retuning.
- Git commit/push/tag/reset/clean or broad Git/LFS scans.
- Reverting unrelated existing working-tree changes.

## Verification Expected

Run after revision:

1. Focused compile:
   `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReloadFromIDE`
2. Combat VFX production binding validator if binding assumptions or source guard requirements change.
3. Report exact files changed and exact proof command Codex should run.

## Deliverable

Update or replace:

`Reports/AgentReviews/Hero1BounceProjectileTravelFix/claude_operator_report.md`

The report must clearly state whether the blocker is fixed:

- Moving carrier uses authored Bounce Niagara slash: YES/NO.
- Temporary cube/profile is hidden/support-only or still primary: explain.
- Minimum visible travel duration added: YES/NO, with value and reason.
- Commands run and pass/fail evidence.

