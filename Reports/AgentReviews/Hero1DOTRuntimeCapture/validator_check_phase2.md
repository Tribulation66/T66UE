Validator Verdict: APPROVE

# Hero 1 Axe DOT Runtime Capture Validator Check

Task: validate the DOT weapon production-binding video captured through the Unreal-owned repo video process.

Operator: Claude
Validator: Codex

## Artifact Check

- Operator packet first line: `Operator Packet: COMPLETE`
- Video: `C:\UE\T66\Saved\VideoCaptures\hero1axedotvfxbinding_20260530_063509\hero1axedotvfxbinding.mp4`
- Evidence bundle: `C:\UE\T66\Saved\VideoCaptures\hero1axedotvfxbinding_20260530_063509\evidence`
- ffprobe evidence: h264, 1280x720, 12 fps, 10.000s, 120 frames.
- Contact sheet inspected: `C:\UE\T66\Saved\VideoCaptures\hero1axedotvfxbinding_20260530_063509\evidence\contact_sheet.png`

## Runtime Structure Evidence

- Moving carrier: one `T66DotShotSpawned` log line with carrier `/Game/VFX/Hero1/Axe/DOT/NS_Hero1AxeDOT_MeshSlash`.
- Staggered visual markers: three reveal/hide pairs at planned delays `0.00`, `0.50`, and `1.00`, each with `VisibleDurationSeconds=0.30`.
- Single damage payload: one `T66DotPayloadApplied` log line with `Source=HeroPrimaryDot (single payload)`.
- Visual hygiene: contact sheet shows a usable locked chase angle, visible target contact zone, no wall-occlusion rectangle contamination, and multi-frame temporal coverage.

## Process Close

PPF CLOSE

Process used: `Gameplay/Combat/CombatVFXAuthoringProcedure.md` plus `Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode hero1axedotvfxbinding -UseHero1AxePreviewStaging -NoHero1AxeTargets -FrameCount 120 -EvidenceBundle -EvidenceAutoSelectFrames`.

Matches declared process: YES

Evidence: MP4, ffprobe metadata, contact sheet, selected frames, and runtime log markers in `Saved\Logs\T66.log`.

MECHANISM CLOSE

Mechanism: moving DOT carrier shot

Status: PRESENT

Evidence: `T66DotShotSpawned` log line and multi-frame contact sheet.

Discriminator test: not a static enemy-side spawn; the video/contact sheet shows travel across frames and log records `ProofTravelSeconds=0.600`.

Reported status: FULL

Mechanism: staggered marker pulse

Status: PRESENT

Evidence: reveal/hide log pairs at 0.00/0.50/1.00s and 0.30s visible duration.

Discriminator test: not simultaneous lingering markers; each marker has a separate planned delay and hide.

Reported status: FULL

Mechanism: single DOT payload

Status: PRESENT

Evidence: one `T66DotPayloadApplied` log line with `Source=HeroPrimaryDot (single payload)`.

Discriminator test: not three independent damage applications; cosmetic markers are tagged visual-only while one payload is applied.

Reported status: FULL
