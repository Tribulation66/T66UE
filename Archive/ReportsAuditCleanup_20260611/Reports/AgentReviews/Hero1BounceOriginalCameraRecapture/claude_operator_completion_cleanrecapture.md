# Claude Operator Completion — Hero 1 Bounce Original-Camera Clean Recapture

Operator: Claude (claude-opus-4-8), FullOperator under Codex approval.
Date: 2026-05-29
Task: Produce a corrected fresh Unreal-owned video capture of the Hero 1 Bounce proof using the original accepted Bounce proof camera/framing, but with the preview-spawn targets suppressed (`-NoHero1AxeTargets`) so the chain matches the intended two-link proof.

> **Headline finding (read first):** The capture, framing, and yellow-block criteria all pass. **However, the `LinkCount=2` / "no `LinkIndex=2`" verification criterion did NOT pass.** Suppressing the preview spawn targets did not reduce the chain — the run still produced `LinkCount=3` with a third link (`LinkIndex=2` / `ChainIndex=2`). The premise that `-T66Hero1AxeAOESpawnTargets` caused the third link is incorrect; see "Why the third link is still present" below. This is an Operator work artifact, not a greenlight — Codex should validate.

## Exact Capture Command

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/CaptureT66GameplayVideo.ps1 `
  -CaptureMode hero1axebouncevfxbinding `
  -UseHero1AxePreviewStaging `
  -NoHero1AxeTargets `
  -FrameCount 72 `
  -FrameRate 12 `
  -EvidenceBundle `
  -EvidenceAutoSelectFrames `
  -Output "C:/UE/T66/Saved/VideoCaptures/Hero1BounceOriginalCameraClean_20260529/hero1axebouncevfxbinding.mp4"
```

`-UseReviewCamera` was **omitted** (per contract). Echoed engine command line (verified from both the script output and `Saved/Logs/T66.log:374` `LogInit: Command Line:`):

```
... -T66GameplayAutoCapture=hero1axebouncevfxbinding
-ExecCmds="T66.Camera.GameplayPreset 1,T66.Camera.LockedChasePitch -30,T66.Camera.LockedChaseArmLength 540,
           T66.Camera.LockedChasePivotHeight 145,T66.Camera.ConstrainAgainstTowerWalls 0,
           T66.Combat.DebugView 2,T66.Combat.DebugLabels 1,T66.Combat.ImpactSourceVerbose 1"
-T66Hero1AxeAOECenterPlayer -T66GameplayAutoLockCameraZoom -T66GameplayAutoCameraArmLength=540
-T66Hero1AxeAOELabForwardOffset=360 -T66Hero1AxeAOELabRightOffset=0 -T66Hero1AxeAOELabVerticalOffset=-82
-T66Hero1AxeAOEHitboxFireDelay=7.6 -T66Hero1AxeAOEHitboxVFXLeadSeconds=0.12
```

### Critical-flag checks (all confirmed)

| Check | Result |
|---|---|
| `-UseReviewCamera` omitted | ✅ omitted |
| Command line includes `-T66Hero1AxeAOECenterPlayer` | ✅ present |
| Command line does NOT include `-T66Hero1AxeAOESpawnTargets` | ✅ absent (and no `-T66Hero1AxeAOETargetCount/Spacing/ForwardOffset/Mob`) |

Same preview-staging camera as the original accepted proof (pitch **-30**, arm **540**, pivot **145**), hero relocated into the open AOE lab via `-T66Hero1AxeAOECenterPlayer`. The `-NoHero1AxeTargets` branch in `Scripts/CaptureT66GameplayVideo.ps1:176-184` correctly skipped the entire `-T66Hero1AxeAOESpawnTargets` block.

## Artifact Paths

- Video (fresh, non-empty, **246,402 bytes**): `C:\UE\T66\Saved\VideoCaptures\Hero1BounceOriginalCameraClean_20260529\hero1axebouncevfxbinding.mp4`
- Frames (retained, 72 PNGs): `C:\UE\T66\Saved\VideoCaptures\Hero1BounceOriginalCameraClean_20260529\frames\`
- Evidence root: `C:\UE\T66\Saved\VideoCaptures\Hero1BounceOriginalCameraClean_20260529\evidence\`
  - `manifest.json`
  - `ffprobe.json` (h264, 1280x720, 12/1 fps, nb_frames=72, duration 6.000s, size 246,402)
  - `contact_sheet.png`
  - `selected_frames\00_start_frame_0003.png`, `01_mid_frame_0030.png`, `02_impact_frame_0057.png`, `03_dissipate_frame_0071.png`
  - `selected_frames.md`, `visibility_checklist.md`

## Framing Validation vs Original `080850` Proof

Reference: `Saved/VideoCaptures/hero1axebouncevfxbinding_20260529_080850/` (impact frame 0042).

| Element | Original `080850` | Clean recapture `Hero1BounceOriginalCameraClean_20260529` |
|---|---|---|
| Hero | From behind, bottom-center, inside white targeting ring | Match — from behind, bottom-center, inside white ring |
| Enemies (proof) | Two green slimes directly ahead (Primary + ChainSecond) | Match — two green slimes directly ahead |
| Wall / arena | Dark stone tower wall surrounding, right/upper edge | Match — dark stone wall surrounding, right/upper edge |
| Damage numbers | 28 / 26 (+ a faint 24) at impact | Match — 28 (Primary) / 26 (ChainSecond) at impact |
| Pale yellow block over hero | Absent | **Absent** |

Verdict: framing matches the original accepted Bounce proof. Hero visible from behind, enemies ahead, wall/stairs to the right, **no pale yellow occluder block**.

## Yellow Block: ABSENT

Confirmed by visual inspection of all four selected frames (start 0003 / mid 0030 / impact 0057 / dissipate 0071) and the contact sheet. No pale slab over the hero in any frame. (Consistent with the prior diagnosis: `-UseHero1AxePreviewStaging` relocates the hero away from the tower-entrance wall geometry, so nothing sits on the camera→hero line.)

## Damage Proof (PASS)

From `Saved/Logs/T66.log` (this run; log opened 10:03:40 local = `2026.05.29-13.04` UTC in log timestamps):

```
[Hero1AxeAOEHitboxProof] DamageNumber Target=Primary Amount=28                                              (L983)
[Hero1AxeAOEHitboxProof] Target=Primary ExpectedHit=1 ActualHit=1 HPBefore=20000 HPAfter=19972 Result=PASS  (L984)
[Hero1AxeAOEHitboxProof] DamageNumber Target=ChainSecond Amount=26                                          (L985)
[Hero1AxeAOEHitboxProof] Target=ChainSecond ExpectedHit=1 ActualHit=1 HPBefore=20000 HPAfter=19974 Result=PASS (L986)
[Hero1AxeAOEHitboxProof] Target=OutOfChainRangeForward ExpectedHit=0 ActualHit=0 ... Result=PASS            (L987)
[Hero1AxeAOEHitboxProof] Target=OutOfChainRangeSide   ExpectedHit=0 ActualHit=0 ... Result=PASS             (L988)
[Hero1AxeAOEHitboxProof] Target=OutsideBehind         ExpectedHit=0 ActualHit=0 ... Result=PASS             (L989)
```

Primary and ChainSecond PASS; all three out-of-chain controls remain unhit (PASS).

## Bounce Chain Log Markers — `LinkCount=3` (criterion NOT met)

The contract required the two-link proof only (`LinkCount=2`, `LinkIndex=0`, arrival callback/deferred spawn, `LinkIndex=1`, no `LinkIndex=2`). The run instead produced a **three-link** chain:

```
CombatVFXBounceChainSequentialAttempt LinkIndex=0 LinkCount=3 Start=V(Z=64) End=V(X=360,Z=64) ...        (L976)
CombatVFXBounceLinkProjectile        LinkIndex=0 LinkCount=3 ... Time=12.646                              (L977)
CombatVFXBounceLinkArrivalCallback   NextLinkIndex=1 ChainPoints=4 CarrierValid=1 Time=12.646            (L978)
CombatVFXBounceLinkArrivalDeferredSpawn NextLinkIndex=1 ChainPoints=4 CarrierValid=1 Time=12.847         (L980)
CombatVFXBounceChainSequentialAttempt LinkIndex=1 LinkCount=3 Start=V(X=360,Y=36,Z=64) End=V(X=360,Y=150,Z=64) (L981)
CombatVFXBounceLinkProjectile        LinkIndex=1 LinkCount=3 ... Time=12.847                              (L982)
CombatVFXBounceLinkArrivalCallback   NextLinkIndex=2 ChainPoints=4 CarrierValid=1 Time=12.847            (L990)
CombatVFXBounceLinkArrivalDeferredSpawn NextLinkIndex=2 ChainPoints=4 CarrierValid=1 Time=12.997         (L992)
CombatVFXBounceChainSequentialAttempt LinkIndex=2 LinkCount=3 Start=V(X=336.93,Y=122.40,Z=62.59) End=V(X=-230.53,Y=-556.40,Z=28.00) (L993)
CombatVFXBounceLinkProjectile        LinkIndex=2 LinkCount=3 ... End=V(X=-230.53,Y=-556.40,Z=28.00)      (L994)
```

Per-link impact contexts (`L973-975`) confirm three official Bounce links: `ChainIndex=0` dmg 28, `ChainIndex=1` dmg 26, `ChainIndex=2` dmg 24 at `V(X=-230.53, Y=-556.40)` with `HitTargets=1`.

So: **`LinkCount=2` was NOT achieved; `LinkIndex=2` is present.** This is the same chain depth the previous original-camera recapture had, and the same depth the original accepted `080850` proof shows (its impact frame carries a faint third damage number, `24`).

## Why the previous original-camera recapture had a third link — and why this clean recapture still does

The previous recapture attributed `LinkCount=3` to the extra preview targets added by `-UseHero1AxePreviewStaging`. **That diagnosis is incorrect.** Evidence:

1. **`LinkCount` is derived from the bounce target search, not the preview targets.** `LinkCount = ChainPositions.Num() - 1` (`Source/T66/Gameplay/T66CombatComponent.cpp:1323`). `ChainPositions` is seeded with the hero and Primary (`:2240-2241`), then extended by a `while (BouncesLeft > 0)` loop that calls `FindClosestTargetHandleInRange(...)` and `ChainPositions.Add(NextLoc)` for each additional in-range target (`:2282-2288`). `BouncesLeft` comes from the weapon's `BounceCount = BaseBounce + ChaosBonus + JuicedBonus + WeaponBounceBonus` (`:2226-2231`). The cosmetic `-T66Hero1AxeAOESpawnTargets` slimes are not part of this calculation.

2. **The proof harness spawns its own 5 targets regardless of the preview flag.** This run (with `-NoHero1AxeTargets`) still logged `Spawned Target=Primary/ChainSecond/OutOfChainRangeForward/OutOfChainRangeSide/OutsideBehind` and `Armed ... Targets=5` (`L909-915`). No `SpawnTargets`/preview-slime spawn appears anywhere in the log.

3. **The third link goes behind-left to a world enemy, not to a forward preview slime.** Preview slimes spawn *ahead* of the hero (`-T66Hero1AxeAOETargetForwardOffset=210`, +Y spacing). But the third link endpoint is *behind-left*: this run `V(X=-230.53, Y=-556.40)`; the prior recapture `V(X=96.29, Y=-226.11)`. Both are negative-Y/behind targets — i.e. real `GameplayLevel` world enemies (visible as the pink blob on the left edge and the brown creature at lower-right in the mid/dissipate frames), found by `FindClosestTargetHandleInRange`. Removing the preview slimes only changed *which* world enemy the third bounce found; it never removed the third bounce.

**Conclusion:** the third link is governed by the proof weapon's `BounceCount` (≥3 hits available) plus the presence of additional in-range combat targets in `GameplayLevel`. `-NoHero1AxeTargets` correctly suppressed the cosmetic preview targets (command line verified) but had **no effect on the chain depth**, because the preview targets were never the source of the third link. To actually produce a `LinkCount=2` proof you would need to constrain `BounceCount`/range or stage the proof so only two valid targets are in bounce range — a runtime/proof-harness change that is **out of scope** here (no source/content/data edits permitted, and no Bounce-behavior change authorized).

## Verification summary against the contract

| Required check | Result |
|---|---|
| Fresh MP4 exists, non-empty | ✅ 246,402 bytes |
| Evidence bundle has `manifest.json`, `ffprobe.json`, `contact_sheet.png` | ✅ all present |
| Framing matches original `080850` proof | ✅ match |
| No pale yellow block | ✅ absent |
| `-UseReviewCamera` omitted | ✅ |
| `-T66Hero1AxeAOECenterPlayer` present | ✅ |
| `-T66Hero1AxeAOESpawnTargets` absent | ✅ |
| Damage proof: Primary + ChainSecond PASS, controls unhit | ✅ all PASS |
| **`LinkCount=2`, no `LinkIndex=2`** | ❌ **NOT met — `LinkCount=3`, `LinkIndex=2` present** |

## Caveats

- Capture-only + read-only diagnosis. No source-code, content, CSV, DataTable, or asset edits. No Git mutating commands or broad Git/LFS scans. No prior evidence deleted — the original `hero1axebouncevfxbinding_20260529_080850` and the prior `Hero1BounceOriginalCameraRecapture_20260529` folders remain intact.
- The single deviation from the contract's expected outcome is the chain depth: the run reproduces the same `LinkCount=3` behavior as both the prior recapture and the original accepted proof. I did **not** alter any Bounce/chain behavior to force a two-link result, since that is excluded by scope and would require a runtime or proof-harness change.
- Log timestamps in `T66.log` are UTC (`2026.05.29-13.04`), corresponding to the local capture at 10:03–10:04. The log was confirmed to be this run via the recorded `LogInit: Command Line:` (`L374`) matching the exact command above.
- Recommended follow-up for Codex (NOT performed): decide whether the accepted Bounce proof is genuinely a two-link or three-link proof. If two-link is intended, a separate accepted task is needed to constrain the proof-harness target layout / weapon `BounceCount` so only Primary + ChainSecond are in bounce range; the camera/staging side is already correct.
