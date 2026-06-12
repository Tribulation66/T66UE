# Claude Operator Completion — Hero 1 Bounce Original-Camera Recapture

Operator: Claude (claude-opus-4-8), FullOperator under Codex approval.
Date: 2026-05-29
Task: Capture-only re-shoot of the Hero 1 Bounce projectile proof using the **original accepted Bounce proof camera/framing** (from `hero1axebouncevfxbinding_20260529_080850`), plus read-only diagnosis of the pale yellow block seen in the prior wrong-angle `Hero1BounceStandardAngle_20260529` capture. No implementation, no patching.

## Exact Capture Command

```
powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/CaptureT66GameplayVideo.ps1 `
  -CaptureMode hero1axebouncevfxbinding `
  -UseHero1AxePreviewStaging `
  -FrameCount 72 `
  -FrameRate 12 `
  -EvidenceBundle `
  -EvidenceAutoSelectFrames `
  -Output "C:/UE/T66/Saved/VideoCaptures/Hero1BounceOriginalCameraRecapture_20260529/hero1axebouncevfxbinding.mp4"
```

`-UseReviewCamera` was **omitted** (per scope). The script applied the preview-staging camera + staging args. Echoed engine command line (key tokens):

```
... -T66GameplayAutoCapture=hero1axebouncevfxbinding
-ExecCmds="T66.Camera.GameplayPreset 1,T66.Camera.LockedChasePitch -30,T66.Camera.LockedChaseArmLength 540,
           T66.Camera.LockedChasePivotHeight 145,T66.Camera.ConstrainAgainstTowerWalls 0,
           T66.Combat.DebugView 2,T66.Combat.DebugLabels 1,T66.Combat.ImpactSourceVerbose 1"
-T66Hero1AxeAOECenterPlayer -T66GameplayAutoLockCameraZoom -T66GameplayAutoCameraArmLength=540
-T66Hero1AxeAOELabForwardOffset=360 -T66Hero1AxeAOELabRightOffset=0 -T66Hero1AxeAOELabVerticalOffset=-82
-T66Hero1AxeAOESpawnTargets -T66Hero1AxeAOETargetCount=3 -T66Hero1AxeAOETargetSpacing=145
-T66Hero1AxeAOETargetForwardOffset=210 -T66Hero1AxeAOETargetMob=Slime
-T66Hero1AxeAOEHitboxFireDelay=7.6 -T66Hero1AxeAOEHitboxVFXLeadSeconds=0.12
```

This is the same preview-staging camera (pitch **-30**, arm **540**, pivot **145**) used by the original accepted proof, with `-UseHero1AxePreviewStaging` relocating the hero into the open AOE lab staging area.

## Artifact Paths

- Video (fresh, non-empty, 281,178 bytes): `C:\UE\T66\Saved\VideoCaptures\Hero1BounceOriginalCameraRecapture_20260529\hero1axebouncevfxbinding.mp4`
- Frames (retained, 72 PNGs): `C:\UE\T66\Saved\VideoCaptures\Hero1BounceOriginalCameraRecapture_20260529\frames\`
- Evidence root: `C:\UE\T66\Saved\VideoCaptures\Hero1BounceOriginalCameraRecapture_20260529\evidence\`
  - `manifest.json`
  - `ffprobe.json` (h264, 1280x720, 12/1 fps, nb_frames=72, duration 6.000s, size 281,178)
  - `contact_sheet.png`
  - `selected_frames\00_start_frame_0047.png`, `01_mid_frame_0056.png`, `02_impact_frame_0066.png`, `03_dissipate_frame_0071.png`
  - `selected_frames.md`, `visibility_checklist.md`

## Framing Comparison vs Original `080850` Proof

Reference: `Saved/VideoCaptures/hero1axebouncevfxbinding_20260529_080850/` (start frame 0003, impact frame 0042).

| Element | Original `080850` | Recapture `Hero1BounceOriginalCameraRecapture_20260529` |
|---|---|---|
| Hero | From behind, bottom-center, inside white targeting ring | Match — from behind, bottom-center, inside white ring |
| Enemies | Green slimes directly ahead | Match — green slimes directly ahead |
| Wall / arena | Dark stone tower wall along the right / upper edge | Match — dark stone wall along right / upper edge |
| Damage numbers | Floating numbers at impact (e.g. 28 / 26) | Match — 28 (Primary) / 26 (ChainSecond) at impact |
| Pale yellow block | Absent | **Absent** |

Verdict: framing matches the original accepted Bounce proof. Hero visible from behind, enemies ahead, wall/stairs to the right, and **no pale yellow occluder block**.

Note (expected difference, not a framing regression): in the mid/dissipate frames a third bounce target appears to the hero's left-rear. The current chain has `LinkCount=3` (vs `LinkCount=2` in the older StandardAngle run); the link-2 endpoint is `V(X=96.29, Y=-226.11)` — behind-left of the hero — so the projectile/target there is expected behavior of the current implementation, captured in-frame.

## Yellow Block in the Recapture: ABSENT

Confirmed by visual inspection of `00_start_frame_0047`, `01_mid_frame_0056`, `02_impact_frame_0066`, `03_dissipate_frame_0071`, and the contact sheet. No pale slab over the hero in any selected frame.

## Likely Yellow-Block Source (read-only diagnosis)

Root cause: the prior `Hero1BounceStandardAngle_20260529` capture **omitted `-UseHero1AxePreviewStaging`**. Both captures used identical camera CVars (pitch -30 / arm 540 / pivot 145), so the difference is **hero placement**, not the camera.

- With `-UseHero1AxePreviewStaging`, the script adds `-T66Hero1AxeAOECenterPlayer` (+ `-T66GameplayAutoLockCameraZoom`, lab offsets, target-spawn args) — see `Scripts/CaptureT66GameplayVideo.ps1:164-185`. This relocates the hero into the open AOE lab arena, away from tower-entrance geometry, so nothing sits between the camera and the hero.
- Without it, the hero remains at the default `GameplayLevel` spawn, which is adjacent to tower wall geometry. A wall slab then sits directly on the camera→hero line and fills the lower-center foreground → the pale block.

Camera occlusion code anchors (`Source/T66/Gameplay/T66PlayerController.cpp`):
- `UpdateGameplayCameraWallOcclusion` (line 896): every frame it line-traces `CameraLocation → HeroVisibilityLocation` on `ECC_Camera` (lines 924-938) and, for each hit that passes `T66IsGameplayCameraWallComponent`, calls `ApplyCameraWallOcclusion` (lines 941-951). Gated by `T66.Camera.WallOcclusionEnabled` (default **1**, line 172-176) — neither capture disabled it; `T66.Camera.ConstrainAgainstTowerWalls 0` is a *separate* camera-collision CVar and does not turn occlusion off.
- `ApplyCameraWallOcclusion` (line 809): swaps the wall component's materials to `M_CameraWallOccluderFade` MIDs with `Opacity` clamped from `T66.Camera.WallOcclusionOpacity` (default **0.12**, lines 177-181, 841-844).
- Fade material definition — `Scripts/SetupCasinoNPCVisualRowAndCameraWallOcclusionAndExit.py:64-84`: **unlit, translucent, two-sided**, `FadeColor = (0.42, 0.46, 0.48)` (pale desaturated blue-gray), `Opacity = 0.12`.

Important caveat on the exact appearance: the observed block reads as a fairly **opaque, warm pale-cream** slab, which is *warmer and more opaque* than the fade material (cool blue-gray at 12% opacity). The most likely explanation is that the offending foreground wall is **not actually being faded** because the occluder explicitly **excludes `UInstancedStaticMeshComponent`** (and requires both the terrain-visual and traversal-barrier tags, no ceiling tag) — see `T66IsGameplayCameraWallComponent`, `Source/T66/Gameplay/T66PlayerController.cpp:214-231` (ISM rejection at line 228). If the tower wall near the default spawn is built as an ISM (or lacks the camera-wall tags), the occluder never swaps it, so it renders with its normal opaque pale-stone material and blocks the hero. Either way the trigger is the same: the hero was left next to wall geometry because preview staging was omitted.

No `CameraWallOcclu`/fade markers are emitted to the log (the system swaps materials silently), so this is inferred from the code path + the staging-arg delta, not a log line.

### Cleanup path (NOT patched this phase)

- Immediate, no code change needed for proofs: always capture Bounce/AOE proofs with `-UseHero1AxePreviewStaging` (as the original `080850` proof and this recapture do). This alone removes the block.
- Tracked follow-up (only if the in-world default-spawn occlusion should be fixed): the camera wall occluder cannot fade `UInstancedStaticMeshComponent` walls (`T66PlayerController.cpp:228`) and requires specific actor/component tags (`:206-231`). Faded coverage of ISM-built tower walls would need either ISM-aware occlusion or per-instance material override, plus verifying the tower-entrance wall carries the `T66_CameraOccludingWallVisual` + traversal-barrier tags. This touches the camera occluder runtime and is out of scope for a capture-only pass — recommend a separate accepted fix.

## Key Log Markers (this run — `C:\UE\T66\Saved\Logs\T66.log`, editor session 2026.05.29-12.55, world time ~12.6s)

Sequential Bounce chain (LinkIndex 0 → arrival callback → deferred spawn → LinkIndex 1 → … → LinkIndex 2; `LinkCount=3`):

```
CombatVFXBounceChainSequentialAttempt LinkIndex=0 LinkCount=3 ... TravelSeconds=0.320            (line 981)
CombatVFXBounceLinkProjectile LinkIndex=0 LinkCount=3 ... Carrier=.../NS_Hero1AxeBounce_MeshSlash Time=12.622  (982)
CombatVFXBounceLinkArrivalCallback NextLinkIndex=1 ChainPoints=4 CarrierValid=1 Time=12.622       (983)
CombatVFXBounceLinkArrivalDeferredSpawn NextLinkIndex=1 ChainPoints=4 CarrierValid=1 Time=12.850  (985)
CombatVFXBounceChainSequentialAttempt LinkIndex=1 LinkCount=3 ... TravelSeconds=0.320             (986)
CombatVFXBounceLinkProjectile LinkIndex=1 LinkCount=3 ... Time=12.850                              (987)
CombatVFXBounceLinkArrivalCallback NextLinkIndex=2 ChainPoints=4 CarrierValid=1 Time=12.850        (995)
CombatVFXBounceLinkArrivalDeferredSpawn NextLinkIndex=2 ... Time=13.031                            (997)
CombatVFXBounceLinkProjectile LinkIndex=2 LinkCount=3 ... Time=13.031                              (999)
```

Damage proof (primary + chain-second PASS; three negative controls PASS):

```
[Hero1AxeAOEHitboxProof] DamageNumber Target=Primary Amount=28                                    (988)
[Hero1AxeAOEHitboxProof] Target=Primary ExpectedHit=1 ActualHit=1 HPBefore=20000 HPAfter=19972 Result=PASS      (989)
[Hero1AxeAOEHitboxProof] DamageNumber Target=ChainSecond Amount=26                                (990)
[Hero1AxeAOEHitboxProof] Target=ChainSecond ExpectedHit=1 ActualHit=1 HPBefore=20000 HPAfter=19974 Result=PASS  (991)
[Hero1AxeAOEHitboxProof] Target=OutOfChainRangeForward ExpectedHit=0 ActualHit=0 ... Result=PASS  (992)
[Hero1AxeAOEHitboxProof] Target=OutOfChainRangeSide   ExpectedHit=0 ActualHit=0 ... Result=PASS   (993)
[Hero1AxeAOEHitboxProof] Target=OutsideBehind         ExpectedHit=0 ActualHit=0 ... Result=PASS   (994)
```

Impact context:

```
CombatImpactContext Phase=WeaponPrimary   ... AttackCategory=Bounce ChainIndex=0 HitTargets=1 EffectiveDamage=28  (978)
CombatImpactContext Phase=WeaponSecondary ... AttackCategory=Bounce ChainIndex=1 HitTargets=1 EffectiveDamage=26  (979)
```

## Caveats

- Capture/proof + read-only diagnosis only. No source-code, content, CSV, DataTable, or asset edits. No Git mutating commands or broad Git/LFS scans. No prior evidence deleted — the original `hero1axebouncevfxbinding_20260529_080850` and the wrong-angle `Hero1BounceStandardAngle_20260529` folders remain intact.
- The yellow-block source is diagnosed from the code path + the `-UseHero1AxePreviewStaging` staging-arg delta; the occluder swaps materials silently (no log marker), so it is not confirmable from a log line. The observed warm/opaque tint is warmer than the fade material (cool blue-gray @ 0.12), which points to an *un-faded opaque wall* (likely an ISM excluded at `T66PlayerController.cpp:228`) rather than the fade proxy itself — see diagnosis above. Confirming which specific component is involved would require an in-editor inspection of the default-spawn tower geometry, which was not part of this capture-only pass.
- Auto-select placed the four evidence frames at 47/56/66/71 (later than the original's 3/22/42/71) because the activity scorer keys on the projectile/impact VFX window; the full 72-frame sequence and contact sheet are retained for broader review.

## Validation Pointers for Codex

- Fresh MP4 exists, non-empty: yes (281,178 bytes; ffprobe nb_frames=72, duration 6.000s, 12/1 fps).
- Evidence bundle includes `manifest.json`, `ffprobe.json`, `contact_sheet.png`: yes.
- New framing matches original `080850` proof (hero from behind, enemies ahead, wall right): yes (table above + selected frames).
- No pale yellow block over the hero: yes (absent in all selected frames + contact sheet).
- Sequential Bounce proof (LinkIndex 0 → arrival/deferred → 1 → … → 2) + per-target damage PASS rows + Bounce impact context: present in `Saved/Logs/T66.log` as quoted with line numbers.
