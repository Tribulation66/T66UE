You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\EnemyDamageReactionThreshold_20260609\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\EnemyDamageReactionThreshold_20260609\operator_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\EnemyDamageReactionThreshold_20260609\20260609T043933-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
User request:

> I want to change the physics a bit, not all damage from enemies should throw you and disable you, damage from enemies should only give you knockback and increase your % and then once above 50% damage starts throwing you and disabling you with the distance you are thrown and the duration of the disable scaling up from 50% to 99. At 100 you die

Working task:
Operator: Codex
Validator: Claude
Scope: Change enemy damage behavior so enemy hits always add percent and apply knockback, but only start throw/disable reactions once the hero is above 50%, scaling throw distance and disable duration from 50% through 99%, with death still at 100%.
Stop condition: Live physics/damage ownership is inspected, the behavior is implemented in the correct runtime path, current compile/gameplay proof is run, staged standalone is refreshed if the playable build is affected, Validator review is complete, and token usage is reported.

Relevant repo rules:
- Use live repo state, not stale memory.
- Codex is Operator and Claude is Validator per `.t66/operator-state.json`.
- Claude must be invoked through the local helper after confirming no `ANTHROPIC_API_KEY` is set.
- Runtime physics/gameplay changes require focused compile, staged standalone validation when playable behavior changes, and Unreal-owned capture/log proof.
- Current physics owner is `UT66HeroPhysicsComponent`; normal play is capsule/CharacterMovement, qualifying hit reactions enter hit-triggered full ragdoll.
- Movement has a separate non-ragdoll launch/knockback layer through `LaunchCharacter`.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

The request is implementable internally and maps cleanly onto the live damage path. No user-only decision blocks it, so Codex can proceed. The behavior change lives in `UT66RunStateSubsystem` combat code, not in a Blueprint.

**Live ownership (confirmed by reading the runtime):**
- Every accepted enemy hit flows through `UT66RunStateSubsystem::ApplyDamage` (`Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp:911`). It already does two things on each hit: adds percent via `ApplyHeroDamagePercent` (`:1075`) and applies the physics reaction via `ApplyDamagePhysicsReaction` (`:1097`).
- `ApplyDamagePhysicsReaction` (`:402`) unconditionally calls `HeroPhysics->ApplyPhysicsReaction`, which routes to `EnterRagdoll` (`T66HeroPhysicsComponent.cpp:476/479`) — i.e. **today every hit throws and disables**. There is no 50% gate yet.
- Death at 100% already exists: `HeroDamageDeathPercent = 100.f` (`T66RunStateSubsystem.h:1717`), checked at `:1152`.
- Launch magnitude already scales with percent, but anchored at 0%: `GetHeroDamageLaunchScale` lerps `PercentBeforeDamage/100` from 1.0→`LaunchScaleAt100` (`:362`). Disable duration scales via `RagdollDurationDamagePercent` → `ComputeHealthScaledRagdollMaxSeconds()`.

**What the change requires:**
1. Keep percent-gain + knockback on every hit (already true for percent).
2. Gate the throw/disable: only call the ragdoll path (`ApplyPhysicsReaction`/`EnterRagdoll`) when percent is above 50%. Below 50%, apply a non-disabling knockback through the separate `LaunchCharacter` layer (the repo's stated non-ragdoll launch path) instead of ragdoll.
3. Re-anchor the throw-distance and disable-duration scaling so they ramp from 50% (not 0%) up through 99%, by changing the `Alpha` basis in `GetHeroDamageLaunchScale` and the duration scale to `(percent-50)/(100-50)`.

This is a runtime physics/gameplay change, so the stop-condition obligations apply: focused compile, staged-standalone refresh if playable behavior changes, and Unreal-owned capture/log proof (the `[CombatDamage]`/`[CombatDamagePhysics]` log lines already exist for verification).

## Evidence Checked
- `T66RunStateSubsystem_Combat.cpp` — full `ApplyDamage`, `ApplyDamagePhysicsReaction`, `GetHeroDamageLaunchScale`, `ApplyHeroDamagePercent`, lethal handling.
- `T66HeroPhysicsComponent.cpp:426–544` — `ApplyPhysicsReaction`/`EnterRagdoll`, confirming no threshold and that disable duration keys off `RagdollDurationDamagePercent`.
- `T66KnockbackComponent.cpp` — confirms the separate `LaunchCharacter` fallback layer exists for non-ragdoll knockback.
- `T66RunStateSubsystem.h:1717` — death percent = 100.
- `.t66/operator-state.json` — Codex Operator, Claude Validator (matches).

## Questions Or Blockers
None that require the user. The threshold and 50→99 scaling are specified in the prompt; remaining choices are tuning defaults Codex can pick.

## Caveats
- **Boundary semantics** ("once above 50%"): decide strict `>50` vs `>=50`, and whether the gate tests percent *before* or *after* this hit's gain — i.e. the hit that crosses 50% should it throw on that hit or the next. Pick one and state it in the log/proof; not user-blocking.
- **"Knockback only" below 50%** should route through the `LaunchCharacter`/`UT66KnockbackComponent` layer, not `EnterRagdoll`, otherwise the hero still gets disabled. Verify the below-threshold path does not suppress input or enter ragdoll.
- Existing CVars (`t66.HealthPercent.*`) currently assume 0%-anchored scaling; their descriptions/defaults should be revisited so the 50%-anchored ramp doesn't double-apply.
- `EnemyDamageMultiplier`, invuln window, and knockback-invuln bypass (`:1019–1028`) interact with hit cadence — confirm rapid sub-50% hits still accumulate percent toward the 50% gate as expected during capture.
- Confirm non-enemy sources (traps, lava, miasma) are intentionally in or out of scope; the prompt says "enemy" but `ApplyDamage` is shared across all damage sources.

</validator_independent_answer>

<codex_draft>
Result: OK

## Operator Draft

Implemented the requested enemy-damage reaction threshold.

Scope:
- Enemy-style damage sources are identified by `AT66EnemyBase`, `AT66MobBase`, or delivery names beginning with `Enemy`.
- Enemy hits still apply damage percent through `UT66RunStateSubsystem::ApplyDamage`.
- Enemy hits at or below `t66.HealthPercent.EnemyDisableStartPercent` (default 50) now apply `LaunchCharacter` knockback only and do not call `UT66HeroPhysicsComponent::ApplyPhysicsReaction`.
- Enemy hits whose resulting percent is above the threshold call the hero physics ragdoll path.
- Enemy ragdoll launch scale and duration scale now ramp from `t66.HealthPercent.EnemyDisableStartPercent` (default 50) through `t66.HealthPercent.EnemyDisableFullPercent` (default 99).
- Death remains owned by `HeroDamagePercent >= 100`.
- Non-enemy ragdoll paths keep their existing duration curve by using the new hero-physics duration-window parameters only when the run-state passes enemy-specific values.

Files changed:
- `Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp`
- `Source/T66/Core/T66RunStateSubsystem.h`
- `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.cpp`
- `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.h`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
- `Gameplay/Physics/CURRENT_STATE.md`
- `Gameplay/Physics/MASTER_PHYSICS.md`
- `Gameplay/Combat/MASTER_COMBAT.md`

Verification performed:
- Focused editor compile passed:
  `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -FromMsBuild`
- Editor gameplay proof passed:
  `Scripts\CaptureT66GameplayVideo.ps1 -CaptureMode combatdamagelog ...`
  Log evidence in `Saved\Logs\T66.log`:
  - `Mode=EnemyKnockbackOnly` for 20 percent enemy hit, `PercentAfter=20.00`, `HeroKnockbackAfterFirst=0`, `BelowThresholdNoRagdoll=1`
  - `Mode=EnemyRagdoll` for 60 percent enemy hit, `PercentAfter=60.00`, `ThrowAlpha=0.204`, `DurationScaleStart=50.00`, `DurationScaleFull=99.00`, `AboveThresholdRagdoll=1`
- Staged readiness wrapper was run:
  `Scripts\RunStagedBuildReadinessGate.ps1 -OutputRoot C:\UE\T66\Saved\StagedBuildReadiness\EnemyDamageReactionThreshold_20260609_045804 -ContinueOnFailure`
  Stage passed, staged exe exists, project-root shortcut and pinned taskbar shortcut target the refreshed staged exe. Wrapper status is FAIL because the child smoke suite reports `BUILD_CONFIG_UNSUPPORTED` for durable-save and lifecycle gates in this Development staged run; frontend tag-click passed.
- Targeted staged executable proof passed:
  `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe /Game/Maps/GameplayLevel -T66GameplayAutoCapture=combatdamagelog ...`
  Exit code 0. Log evidence in `Saved\StagedProof\EnemyDamageReactionThreshold_20260609_0507\run.log`:
  - `Mode=EnemyKnockbackOnly` for 20 percent enemy hit.
  - `Mode=EnemyRagdoll` for 60 percent enemy hit with `DurationScaleStart=50.0` and `DurationScaleFull=99.0`.
  - `[HealthPercentQA] ... BelowThresholdNoRagdoll=1 AboveThresholdRagdoll=1 ... FullHealReset=1`.

Known caveat:
- Boss deliveries are intentionally not included in the enemy threshold unless they use an `Enemy*` delivery name. The prompt said enemies, and boss/trap/lava/miasma paths remain outside this scoped change.

</codex_draft>
