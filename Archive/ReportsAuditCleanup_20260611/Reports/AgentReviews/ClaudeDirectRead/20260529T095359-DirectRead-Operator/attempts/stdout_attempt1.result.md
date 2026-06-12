Done. Operator work artifact complete.

## Summary

**Fresh capture (matches original framing, no yellow block):**
- Ran `CaptureT66GameplayVideo.ps1` with `-CaptureMode hero1axebouncevfxbinding -UseHero1AxePreviewStaging -FrameCount 72 -FrameRate 12 -EvidenceBundle -EvidenceAutoSelectFrames`, output to `Saved\VideoCaptures\Hero1BounceOriginalCameraRecapture_20260529\`. No `-UseReviewCamera`.
- Video: 281,178 bytes, h264 1280x720, 72 frames @ 12fps, 6.0s. Evidence bundle has `manifest.json`, `ffprobe.json`, `contact_sheet.png`, 4 selected frames.
- Framing **matches the original `080850` proof**: hero from behind in the white ring, green slimes ahead, stone wall right, damage numbers 28/26 at impact. **No pale yellow block** in any frame.

**Yellow-block diagnosis (read-only, not patched):**
- The prior wrong-angle `Hero1BounceStandardAngle_20260529` capture used the *same camera CVars* but **omitted `-UseHero1AxePreviewStaging`** — so the hero was never relocated to the open AOE lab and stayed next to tower-entrance wall geometry, which then filled the camera→hero foreground.
- Camera occlusion path anchored: `T66PlayerController.cpp:896` (trace), `:809` (material swap to `M_CameraWallOccluderFade`), fade material is unlit cool-gray @ 0.12 opacity (`Setup...py:64-84`). The observed *opaque warm* slab is warmer than the fade proxy, most likely an **un-faded wall** — the occluder explicitly excludes `UInstancedStaticMeshComponent` (`:228`).
- Cleanup path: for proofs, always use `-UseHero1AxePreviewStaging` (no code change). A real in-world fix (ISM-aware occlusion / tagging) touches the camera occluder runtime → recommend a separate accepted fix, not this phase.

**Log markers (this run, `Saved\Logs\T66.log`):** full sequential Bounce chain LinkIndex 0→1→2 (LinkCount=3) with arrival/deferred-spawn callbacks, plus Primary=28/ChainSecond=26 damage PASS rows and three negative-control PASS rows.

Completion note written to `Reports\AgentReviews\Hero1BounceOriginalCameraRecapture\claude_operator_completion.md`. This is an Operator artifact for Codex to validate, not a greenlight.
