Operator Packet: COMPLETE

# Hero 1 Axe DOT VFX-Binding Runtime Capture — Phase 2 Operator Packet

Capture-only run executed under approval `codex_operator_approval_phase2.md`. No code/content/data/docs were edited. This is runtime-structure proof, not final visual-polish approval.

## Capture Command (as approved, verbatim)

```
Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode hero1axedotvfxbinding -UseHero1AxePreviewStaging -NoHero1AxeTargets -FrameCount 120 -EvidenceBundle -EvidenceAutoSelectFrames
```

- Result: exit code 0, ffmpeg encode succeeded.
- Run folder: `C:\UE\T66\Saved\VideoCaptures\hero1axedotvfxbinding_20260530_063509\`
- Editor log: `C:\UE\T66\Saved\Logs\T66.log` (run timestamp 09:35–09:36 in-log clock)

## MP4

- Path: `C:\UE\T66\Saved\VideoCaptures\hero1axedotvfxbinding_20260530_063509\hero1axedotvfxbinding.mp4`
- Size: 406,651 bytes (~397 KiB) — non-empty, playable.
- ffprobe: h264, 1280x720, yuv420p, 12/1 fps, duration 10.000s, 120 frames (`evidence/ffprobe.json`).

## Evidence Bundle

Root: `C:\UE\T66\Saved\VideoCaptures\hero1axedotvfxbinding_20260530_063509\evidence\`

- `ffprobe.json` — present (stream/format metadata above).
- `contact_sheet.png` — present (1,133,256 bytes).
- `manifest.json` — present (19,006 bytes).
- `selected_frames.md` — present; selected frames:
  - start = frame 47 @ 3.92s — `evidence/selected_frames/00_start_frame_0047.png`
  - mid = frame 76 @ 6.33s — `evidence/selected_frames/01_mid_frame_0076.png`
  - impact = frame 104 @ 8.67s — `evidence/selected_frames/02_impact_frame_0104.png`
  - dissipate = frame 119 @ 9.92s — `evidence/selected_frames/03_dissipate_frame_0119.png`
- `visibility_checklist.md` — present (manual scores left TODO by the script; see hygiene note below for operator observation).
- `frames/` — full 120-frame PNG sequence (`frame_%04d.png`).

## Runtime Proof — Log Lines (`Saved/Logs/T66.log`)

DOT shot spawned with the production-bound carrier path (single moving shot):

```
981: LogT66Combat: Display: T66DotShotSpawned Target=T66EnemyBase_0 Start=V(Z=64.00) End=V(X=360.00, Z=64.00) ProofTravelSeconds=0.600 Carrier=/Game/VFX/Hero1/Axe/DOT/NS_Hero1AxeDOT_MeshSlash.NS_Hero1AxeDOT_MeshSlash CarrierVisualScale=1.000
```

- Carrier path confirmed: `/Game/VFX/Hero1/Axe/DOT/NS_Hero1AxeDOT_MeshSlash`.
- Exactly one `T66DotShotSpawned` in the run.

Three staggered marker reveals at planned delays 0.00 / 0.50 / 1.00s, each hidden with `VisibleDurationSeconds=0.30`:

```
991:  T66DotApplicatorMarkerRevealed MarkerIndex=0 MarkerCount=3 PlannedRevealDelaySeconds=0.00 (visual-only; single DOT payload unchanged)
998:  T66DotApplicatorMarkerHidden   MarkerIndex=0 MarkerCount=3 PlannedRevealDelaySeconds=0.00 VisibleDurationSeconds=0.30 (visual-only; single DOT payload unchanged)
1001: T66DotApplicatorMarkerRevealed MarkerIndex=1 MarkerCount=3 PlannedRevealDelaySeconds=0.50 (visual-only; single DOT payload unchanged)
1004: T66DotApplicatorMarkerHidden   MarkerIndex=1 MarkerCount=3 PlannedRevealDelaySeconds=0.50 VisibleDurationSeconds=0.30 (visual-only; single DOT payload unchanged)
1007: T66DotApplicatorMarkerRevealed MarkerIndex=2 MarkerCount=3 PlannedRevealDelaySeconds=1.00 (visual-only; single DOT payload unchanged)
1013: T66DotApplicatorMarkerHidden   MarkerIndex=2 MarkerCount=3 PlannedRevealDelaySeconds=1.00 VisibleDurationSeconds=0.30 (visual-only; single DOT payload unchanged)
```

Single DOT payload (one application, production source):

```
994: LogT66Combat: Display: T66DotPayloadApplied Target=T66EnemyBase_0 Duration=4.00 TickInterval=0.50 DamagePerTick=2.01 Source=HeroPrimaryDot (single payload)
```

- Exactly one `T66DotPayloadApplied` in the run; `Source=HeroPrimaryDot (single payload)`.
- Marker alignment line (line 993) reports `OffsetSize=0.000` — markers land on target.
- All three markers tagged `visual-only; single DOT payload unchanged`, consistent with markers being cosmetic over a single payload.

## Visibility Hygiene Note

- Camera angle is useful: locked chase preset (pitch -30, arm 540, pivot 145) gives a top-down-forward view that clearly shows the attack plane, the moving green DOT carrier (MeshSlash), the target enemy, the red diamond DOT marker, and the purple DOT splat on the target.
- Not contaminated by the wall-occlusion proof rectangle: the run launched with `T66.Camera.WallOcclusionEnabled 0` and `T66.Camera.ConstrainAgainstTowerWalls 0`. Inspected selected frames (mid 0076, impact 0104) show no translucent wall-occlusion rectangle overlay. The pink/red disc near the bottom edge is the player hitbox debug gizmo (DebugView 2 / DebugLabels 1), not occlusion contamination.
- Operator visibility read: target size, attack-plane readability, occlusion/crop safety, and temporal coverage (start/mid/impact/dissipate frames span the travel + marker cycle) all look usable for runtime-structure review. The script's `visibility_checklist.md` manual score table remains TODO and is left for validator sign-off, not auto-scored here.

## Scope / Integrity

- Capture-only; no gameplay/content/data/docs edits. Generated run folder preserved.
- No git operations performed. No desktop screenshots used as proof.
- Console transcript saved to `Reports/AgentReviews/Hero1DOTRuntimeCapture/_phase2_capture_console.log`.

This Operator packet is a work artifact for Codex validation, not a user-facing greenlight.
