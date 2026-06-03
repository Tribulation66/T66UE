# Claude Operator Completion — Hero 1 Bounce Standard-Angle Capture

Operator: Claude (claude-opus-4-8), FullOperator under Codex approval.
Date: 2026-05-29
Task: Capture-only re-shoot of the Hero 1 Bounce projectile proof at the standard enemy-visible gameplay angle (the prior video used the review/top-down camera and was unreadable).

## Exact Capture Command

```
powershell -NoProfile -ExecutionPolicy Bypass -File "Scripts/CaptureT66GameplayVideo.ps1" `
  -CaptureMode hero1axebouncevfxbinding `
  -Output "C:\UE\T66\Saved\VideoCaptures\Hero1BounceStandardAngle_20260529\hero1axebouncevfxbinding.mp4" `
  -ResX 1280 -ResY 720 -FrameCount 120 -FrameRate 30 `
  -EvidenceBundle -EvidenceAutoSelectFrames `
  -EvidenceLabel "Hero1BounceStandardAngle_20260529"
```

`-UseReviewCamera` was **omitted** (see below). The script applied the mode's default gameplay-style preview camera via ExecCmds:

```
T66.Camera.GameplayPreset 1,T66.Camera.LockedChasePitch -30,T66.Camera.LockedChaseArmLength 540,T66.Camera.LockedChasePivotHeight 145,T66.Camera.ConstrainAgainstTowerWalls 0,T66.Combat.DebugView 2,T66.Combat.DebugLabels 1,T66.Combat.ImpactSourceVerbose 1
```

Engine launch args (from script echo) included `-T66GameplayAutoCapture=hero1axebouncevfxbinding`, `-T66Hero1AxeAOEHitboxFireDelay=7.6`, `-T66Hero1AxeAOEHitboxVFXLeadSeconds=0.12`. No `-UseReviewCamera` token present.

## Artifact Paths

- Video (fresh, non-empty, 93,156 bytes): `C:\UE\T66\Saved\VideoCaptures\Hero1BounceStandardAngle_20260529\hero1axebouncevfxbinding.mp4`
- Frames (retained, 120 PNGs): `C:\UE\T66\Saved\VideoCaptures\Hero1BounceStandardAngle_20260529\frames\`
- Evidence root: `C:\UE\T66\Saved\VideoCaptures\Hero1BounceStandardAngle_20260529\evidence\`
  - `manifest.json`
  - `ffprobe.json` (h264, 1280x720, 30/1 fps, nb_frames=120, duration 4.000s)
  - `contact_sheet.png`
  - `selected_frames\00_start_frame_0042.png`, `01_mid_frame_0043.png`, `02_impact_frame_0043.png`, `03_dissipate_frame_0052.png`
  - `selected_frames.md`, `visibility_checklist.md`

## Camera Angle — `-UseReviewCamera` Omitted: YES

Confirmed by the printed engine command line (no `-UseReviewCamera`) and by the applied camera CVars: pitch **-30**, arm length **540**, pivot **145**. This is the gameplay-style preview framing. The review/top-down camera (which would be pitch **-72**, arm **1550**) was not used.

## Enemies Visible: YES

Visual inspection of the retained frames and contact sheet confirms the standard oblique over-shoulder gameplay angle (hero box in foreground, ground plane and arena ring visible — not a top-down plan view). The slime targets are visible to the right of the hero with floating damage numbers rendered above them:

- `02_impact_frame_0043.png` / `frame_0048.png`: green slime target(s) on the right with damage numbers **28** and **26** floating above the hit enemies; additional enemy markers/labels visible upper-left.
- `03_dissipate_frame_0052.png`: same framing, slime + damage numbers still legible.

This resolves Pablo's complaint that enemies and the projectile were not visible at the prior top-down angle.

## Key Log Markers (Bounce sequencing + damage proof)

Source: `C:\UE\T66\Saved\Logs\T66.log` (this run, world time ~12.66s; editor session timestamps 2026.05.29-12.44.xx).

Sequential Bounce chain (LinkIndex 0 -> arrival callback -> deferred spawn -> LinkIndex 1):

```
CombatVFXBounceChainSequentialAttempt LinkIndex=0 LinkCount=2 ... TravelSeconds=0.320
CombatVFXBounceLinkProjectile LinkIndex=0 LinkCount=2 ... Carrier=/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash Time=12.661
CombatVFXBounceLinkArrivalCallback NextLinkIndex=1 ChainPoints=3 CarrierValid=1 Time=12.661
CombatVFXBounceLinkArrivalDeferredSpawn NextLinkIndex=1 ChainPoints=3 CarrierValid=1 Time=12.961
CombatVFXBounceChainSequentialAttempt LinkIndex=1 LinkCount=2 ... TravelSeconds=0.320
CombatVFXBounceLinkProjectile LinkIndex=1 LinkCount=2 ... Carrier=/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash Time=12.961
```

Damage proof rows (primary + second/chain target both PASS; negative controls PASS):

```
[Hero1AxeAOEHitboxProof] DamageNumber Target=Primary Amount=28
[Hero1AxeAOEHitboxProof] Target=Primary ExpectedHit=1 ActualHit=1 HPBefore=20000 HPAfter=19972 Result=PASS
[Hero1AxeAOEHitboxProof] DamageNumber Target=ChainSecond Amount=26
[Hero1AxeAOEHitboxProof] Target=ChainSecond ExpectedHit=1 ActualHit=1 HPBefore=20000 HPAfter=19974 Result=PASS
[Hero1AxeAOEHitboxProof] Target=OutOfChainRangeForward ExpectedHit=0 ActualHit=0 ... Result=PASS
[Hero1AxeAOEHitboxProof] Target=OutOfChainRangeSide ExpectedHit=0 ActualHit=0 ... Result=PASS
[Hero1AxeAOEHitboxProof] Target=OutsideBehind ExpectedHit=0 ActualHit=0 ... Result=PASS
```

Impact context for the primary hit:

```
CombatImpactContext Phase=WeaponPrimary SourceType=WeaponBase SourceID=Hero_1_black_bounce HeroID=Hero_1 AttackCategory=Bounce ChainIndex=0 HitTargets=1 EffectiveDamage=28
```

## Caveats

- This is capture/proof work only. No source-code, content, CSV, DataTable, or asset edits were made. No Git mutating commands or broad Git/LFS scans were run. No prior evidence was deleted (the earlier review-camera bounce folders `hero1axebouncevfxbinding_20260529_074129/074452/074645/080850` remain intact).
- Auto-select clustered the four evidence frames into a narrow window (42/43/43/52) because the activity scorer keys on the projectile/impact VFX, which is brief at 30fps. The full 120-frame sequence and contact sheet are retained for broader review; enemies and damage numbers are legible across the active window. The `mp4` is 4.0s at 30fps.
- The output `.mp4` is small (93 KB) because the scene is mostly static flat-shaded geometry; this is expected for this proof staging, not a truncation — `ffprobe` reports nb_frames=120, duration 4.000s.

## Validation Pointers for Codex

- Fresh MP4 exists and is non-empty: yes (93,156 bytes).
- Evidence bundle includes `manifest.json`, `ffprobe.json`, `contact_sheet.png`: yes.
- Standard gameplay-style angle, enemies visible: yes (frames + CVar evidence above).
- Sequential Bounce proof + per-target damage PASS rows: present in `Saved/Logs/T66.log` as quoted.
