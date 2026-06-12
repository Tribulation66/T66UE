# Combat Projectile Idol Diagnostic Review Packet

## Working Goal

Add a focused combat diagnostic proving whether each weapon impact context produces a matching Water idol impact context, then compile and verify it with the repo gameplay capture/log process.

## Operator / Validator

- Operator: Codex in the active workspace.
- Validator: Claude Code CLI using `claude-opus-4-8` through `Scripts\Invoke-ClaudePlanReview.ps1`.
- API billing guard: `ANTHROPIC_API_KEY` was checked in Process/User/Machine scope and returned empty output before this packet.

## Request Classification

- Tier: Tier 1.
- Reason: Runtime gameplay source and proof script changes, plus Unreal-owned capture/log verification.
- PPF: Not applicable for this pass because no new Niagara/material/visual carrier is being authored. This is diagnostic instrumentation and proof-harness tightening around an existing Water idol placeholder path.

## Applicable Instructions Read

- `AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/pending_issues_Combat.md`
- `Gameplay/README.md`
- `Gameplay/Combat/MASTER_COMBAT.md`
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Scripts/CaptureT66GameplayVideo.ps1`
- `Scripts/RunHero1AxeAOEWaterIdolImpactProof.ps1`

## Current Live-State Findings

- `FT66CombatComponent::PerformSlash` now records weapon impact contexts in `WeaponImpactContexts` via `PublishWeaponImpactContext`.
- The current Water idol impact-presentation branch consumes only `PrimaryWeaponImpactContext`, then emits an idol `FT66CombatImpactContext`, placeholder VFX, and idol-sourced damage from that primary weapon context.
- The proof harness already checks the Water case for weapon context, idol context, Water placeholder spawn, Water damage, and visual/damage alignment logs.
- The current proof harness does not explicitly count expected Water idol contexts against actual idol contexts, so it cannot clearly distinguish "weapon impact happened but Water idol skipped" from visual clutter or unrelated projectile-like events.

## Planned Edit Scope

### `Source/T66/Gameplay/T66CombatComponent.cpp`

Add logging-only diagnostic counters inside the existing idol modifier section:

- Count Water impact-presentation idol slots.
- Count weapon impact contexts with valid impact points.
- Compute expected Water idol impact contexts as `ImpactPresentationIdolSlots * EligibleWeaponImpactContexts`.
- Count actual Water idol impact contexts emitted by the current Water branch.
- Count skip reasons:
  - no primary weapon context
  - primary weapon context exists but lacks a valid impact point
  - legacy fallback used after an impact-presentation Water slot could not use weapon context
- Emit a `CombatIdolImpactDiagnostic SourceID=Idol_Water ... WaterIdolContextParity=PASS|FAIL ...` summary only when a Water impact-presentation slot is active or Water impact diagnostics were touched.
- Keep all new logs gated behind `CVarT66CombatImpactSourceVerbose`.

This is intentionally diagnostic-only. It should not change damage, targeting, projectile motion, VFX placement, cooldowns, idol activation, or current single-primary behavior.

### `Scripts/RunHero1AxeAOEWaterIdolImpactProof.ps1`

Tighten the Water proof case by adding required log patterns for:

- `CombatIdolImpactDiagnostic SourceID=Idol_Water`
- `WaterIdolContextParity=PASS`
- `WaterSkippedNoWeaponContext=0`
- `WaterSkippedInvalidImpactPoint=0`
- `WaterLegacyFallbacks=0`

Add required regex patterns, not fixed literal counts, for the runtime-derived positive counts:

- `WeaponImpactContexts=[1-9][0-9]*`
- `EligibleWeaponImpactContexts=[1-9][0-9]*`
- `ImpactPresentationIdolSlots=[1-9][0-9]*`
- `ExpectedWaterIdolImpactContexts=[1-9][0-9]*`
- `WaterIdolImpactContexts=[1-9][0-9]*`

The parity token, not hardcoded `=1` counts, is the assertion that the diagnostic is checking the current runtime count correctly. This avoids failing a correct build if the current attack path emits more than one weapon impact context.

Also include diagnostic lines in the proof excerpt pattern list if that list exists in the current script body.

Add `CombatIdolImpactDiagnostic SourceID=Idol_Water` to the Earth-neutral forbidden patterns so the "no Water overlay when Water is not equipped" case also enforces that the Water diagnostic did not run.

## Out Of Scope

- Refactoring Water to iterate all weapon contexts.
- Adding the final Water Niagara effect.
- Changing idol damage balance or targeting.
- Changing projectile lock-on or auto-attack policy.
- Changing Overclock setup or other idol slots.
- Touching Mini/minigame systems.

## Risks And Rollback

- Risk: The summary could be logged too broadly and pollute Earth-neutral proof. Mitigation: log only when Water impact-presentation diagnostics are relevant and add an Earth-neutral forbidden pattern for the summary line.
- Risk: Fixed proof counts could be wrong if the current attack emits multiple contexts. Mitigation: assert positive runtime-derived counts with regex and assert structural parity with `WaterIdolContextParity=PASS`.
- Rollback: remove the added diagnostic counters/logs and required proof patterns. No asset or data migration is involved.

## Verification Plan

1. Compile focused editor target:
   - `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReloadFromIDE`
2. Run the repo proof wrapper:
   - `Scripts\RunHero1AxeAOEWaterIdolImpactProof.ps1`
3. Confirm proof artifacts include:
   - MP4 video for the Water case.
   - Water log excerpt containing weapon context, idol context, placeholder spawn, idol damage, and the new diagnostic summary.
   - Earth-neutral case still proving Water overlay does not spawn when Water is not equipped.
4. Run `ffprobe` or use the wrapper's video evidence checks if present.
5. Stage standalone Development build if the compile and proof pass, because the touched source is runtime gameplay code.

## Review Request

Please review the planned diagnostic scope only. Identify any flawed assumptions, missing files, unsafe scope, contradictions with repo instructions, or inadequate verification gates. The requested output scope is permission to implement this diagnostic and proof-harness tightening, not permission for a broader idol architecture refactor.
