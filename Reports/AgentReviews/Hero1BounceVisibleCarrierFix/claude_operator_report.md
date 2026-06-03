# Operator Report — Hero 1 Bounce Visible Carrier Fix

**Verdict: BLOCKED — NEEDS HUMAN DECISION.** Runtime is fully correct (exactly two links, no third, damage preserved, authored Niagara carrier preserved), but the validator's core blocker is NOT met: the authored Bounce slash does **not** visibly originate at each link's start and travel along the path. After exhausting the two in-scope runtime presentation mechanisms, the remaining options cross the asset-modification gate or risk the user's "don't overcomplicate" guidance, so I am surfacing the fork rather than continuing to iterate.

Operator: Claude (`claude-opus-4-8`). Validator: Codex.

## User goal & constraints
- Goal (verbatim): the projectile "should be a projectile that looks just like the first iteration of the bounce one. The only issue the original had was that it appeared on the enemies rather than originating at the hero and moving toward enemies. We may have overcomplicated the solution."
- Preserve: Bounce damage authority, target selection/chain behavior, exactly one visible link per segment, link 0 hero→primary then link 1 primary→second sequencing, per-link impact-context publication, the authored Niagara slash (`NS_Hero1AxeBounce_MeshSlash`) as the primary silhouette, Mini/minigame exclusion.
- Gates honored: did **not** substitute the carrier method class; did **not** rewrite any `.uasset` or regenerate assets; did not touch Mini.

## What is verified correct (runtime)
From `Saved/Logs/T66.log`, run `2026.05.29-14.58`, capture dir `Hero1BounceVisibleCarrierFix_20260529g`:
```
[Hero1AxeBounceProof] SetBounceProofLinkTravelSeconds=0.30
[Hero1AxeBounceProof] SetFixedTimeStep DT=0.040 for 1.50s game-time
CombatImpactContext Phase=WeaponPrimary ... AttackCategory=Bounce ... ChainIndex=0 HitTargets=1 EffectiveDamage=28
CombatVFXBounceLinkProjectile LinkIndex=0 LinkCount=2 Start=V(Y=3575.00, Z=1266.15) End=V(Y=3215.00, Z=1254.15) Speed=1125.6 TravelSeconds=0.320 Carrier=.../NS_Hero1AxeBounce_MeshSlash Time=12.794
CombatVFXBounceLinkArrivalCallback NextLinkIndex=1 ChainPoints=3 CarrierValid=1 Time=12.954
CombatVFXBounceLinkProjectile LinkIndex=1 LinkCount=2 Start=V(X=36.00, Y=3215.00, Z=1254.15) End=V(X=150.00, Y=3215.00, Z=1254.15) Speed=356.2 TravelSeconds=0.320 Carrier=.../NS_Hero1AxeBounce_MeshSlash Time=12.954
[Hero1AxeBounceProof] ResetFixedTimeStep
```
- Exactly two links: `LinkIndex=0` then `LinkIndex=1`, **no `LinkIndex=2`**.
- Damage preserved: primary 28 (`ChainIndex=0`), second 26 (`ChainIndex=1`).
- Authored carrier preserved for both links: `NS_Hero1AxeBounce_MeshSlash`.
- Capture density solved: fixed timestep (`DT=0.040`) keeps the game clock advancing a constant amount per rendered frame despite PNG-write stalls, so each ~0.32s link spans multiple screenshot frames. Frame↔engine mapping: link0 launch = `frame_0042` (engine `[815]`), arrival/link1 launch = `frame_0046` (engine `[819]`); link1 travel ≈ `frame_0046`→`frame_0054`.

## The blocker that remains (visual)
The authored slash does **not** visibly originate at each link's start and travel to its end:
- `evidence/link0_tight.png` (frames 42–46, hero→primary): a cyan slash appears at the **hero's head** in frames 44–46 and grows in place; it never translates upward toward the primary target.
- `evidence/link1_strip.png` (frames 46,48,50,52,54, primary→second): link1's slash should originate at the **primary** (above the hero), but it again renders **at the hero's head**, not at the primary, and does not travel sideways.

Conclusion: the visible slash presents at/near the hero/owner for both links rather than tracking each link's segment. This is the same symptom across two independent presentation mechanisms (below).

## Mechanisms attempted (both in-scope, both insufficient)
1. **Drive a free-spawned component by world location** (`SpawnSystemAtLocation` + `AT66HeroProjectile::Tick` → `DrivenCarrierComponent->SetWorldLocation` each frame, time-based lerp travel). Result (run f, `_20260529f`): slash revealed near the hero, did not traverse to the primary.
2. **Hard-attach the carrier to the moving projectile** (`SpawnSystemAttached` to the projectile root with `SnapToTarget`, no `SetWorldLocation` drive, so it inherits the actor's `SetActorLocation` travel). Result (run g, `_20260529g`, current build): identical symptom — slash renders at the hero for both links; link1 does not even originate at the primary.

Because `bLocalSpace=true` (commandlet lines 313/798) means moving the component should move the rendered particles, and hard attachment still did not produce travel, the evidence points to the authored system itself presenting as a short, in-place, age-revealed burst anchored to the owner/spawn frame rather than a streak that reads as travelling across the ~360-unit gap.

## Decision fork (NEEDS HUMAN DECISION)
I have exhausted the in-scope runtime presentation options. The remaining paths each cross a gate or risk a prior instruction:
- **A. Re-author `NS_Hero1AxeBounce_MeshSlash`** via the commandlet so the slash is a longer streak and/or its reveal/local-space maps to travel along the segment. This is an **asset-regeneration change → NEEDS_HUMAN_DECISION gate**.
- **B. Diagnose whether the cyan blob at the hero is a separate owner-anchored effect masking the real carrier**, then fix that. More investigation; no asset change; uncertain payoff.
- **C. Multi-burst stamping**: spawn the authored slash (first-iteration `ImpactAnchored` look) at several staggered points along each segment so the eye reads progression hero→target. No asset change, but risks the user's explicit "don't overcomplicate" guidance.
- **D. Accept current behavior** — the slash forms at/near the hero on each link — and document that "visible travel" is not achievable with the authored asset as-is.

**Recommendation:** Option A (re-author the asset to a travelling streak) is the only path that robustly satisfies "originates at the hero and moves toward enemies" while keeping the authored system, but it requires explicit approval to modify the asset. Awaiting the human's call.

## Files changed this session
- `Source/T66/Gameplay/T66CombatComponent.cpp` — `SpawnBounceLinkProjectile`: carrier now `SpawnSystemAttached` to the projectile root (`SnapToTarget`), scale via `SetWorldScale3D`, color vars + `SetTranslucentSortPriority(14)`; no longer registered as a `SetWorldLocation`-driven component. Added proof CVar `T66.Bounce.ProofReadableTravelSeconds`.
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` — proof-mode block sets the travel CVar and enables `FApp::SetUseFixedTimeStep(true)` / `SetFixedDeltaTime(0.04)` for capture density, with a timer that resets it.
- (Carried from prior session) `Source/T66/Gameplay/T66HeroProjectile.h/.cpp` — `SetDrivenCarrierComponent` / `SetTimedVisualTravel` time-based travel.

## Verification
- **Compile:** `Build.bat T66Editor Win64 Development -Project=C:/UE/T66/T66.uproject -WaitMutex -FromMsBuild` → `Result: Succeeded` (17.49s; compiled `Module.T66.26.cpp`, linked `UnrealEditor-T66.dll`).
- **Capture:** exit 0 → `Saved/VideoCaptures/Hero1BounceVisibleCarrierFix_20260529g/hero1axebouncevfxbinding.mp4` (120 frames, 12fps), frames in `frames/`, evidence strips in `evidence/`.

## Token usage
- Claude (this operator session): not exposed via a local counter; running through the Claude Code subscription helper (no Anthropic API billing used).
