You are Claude acting as Operator for T66 under Codex approval.

Working task:
Produce a corrected fresh Unreal-owned video capture of Hero 1 Bounce using the original accepted Bounce proof camera/framing, but without the extra preview targets that caused the prior original-camera recapture to show `LinkCount=3`.

Use this exact capture shape:

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

Critical checks:
- `-UseReviewCamera` must be omitted.
- The engine command line should include `-T66Hero1AxeAOECenterPlayer`.
- The engine command line should NOT include `-T66Hero1AxeAOESpawnTargets`.
- The contact sheet should match the original `hero1axebouncevfxbinding_20260529_080850` camera/framing: hero visible from behind, enemies ahead, wall/stairs to the right, no pale yellow block over the hero.
- The runtime log should show the current two-link proof only: `LinkCount=2`, `LinkIndex=0`, arrival callback/deferred spawn, `LinkIndex=1`, no `LinkIndex=2`.
- Damage proof should pass for Primary and ChainSecond, and out-of-chain controls should remain unhit.

Write:
`Reports/AgentReviews/Hero1BounceOriginalCameraRecapture/claude_operator_completion_cleanrecapture.md`

Include:
- exact command,
- video path,
- evidence paths,
- framing validation,
- whether yellow block appears,
- why the previous original-camera recapture had a third link,
- key log markers,
- caveats.

No source/content/data edits, no Git operations, no cleanup, no deleting prior evidence.
