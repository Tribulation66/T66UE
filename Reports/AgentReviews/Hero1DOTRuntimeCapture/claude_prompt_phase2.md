Run Phase 2 from:

`Reports/AgentReviews/Hero1DOTRuntimeCapture/codex_operator_approval_phase2.md`

User request:
"ok go ahead and capture the video"

Use the approved capture process exactly:

`Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode hero1axedotvfxbinding -UseHero1AxePreviewStaging -NoHero1AxeTargets -FrameCount 120 -EvidenceBundle -EvidenceAutoSelectFrames`

Do not edit gameplay, content, data, or docs. This is capture-only unless the capture script fails; if it fails, report the blocker and do not patch around it.

After capture, inspect the output folder enough to write a useful packet:

- MP4 path and file size;
- evidence bundle paths such as ffprobe/contact sheet/selected frames/manifest/visibility checklist when present;
- log lines proving `T66DotShotSpawned` with carrier `/Game/VFX/Hero1/Axe/DOT/NS_Hero1AxeDOT_MeshSlash`;
- marker reveal/hide lines at 0.00 / 0.50 / 1.00s with 0.30 visible duration;
- `T66DotPayloadApplied ... Source=HeroPrimaryDot (single payload)`;
- visibility hygiene note: whether the camera angle is useful and not contaminated by the wall-occlusion rectangle.

Produce:

`Reports/AgentReviews/Hero1DOTRuntimeCapture/operator_packet_phase2.md`

First non-empty line:

`Operator Packet: COMPLETE`
