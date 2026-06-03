Codex Approval: APPROVE

# Claude Operator Approval - Hero 1 Bounce Original-Camera Harness Fix

Task contract:
- Operator: Claude (`claude-opus-4-8`)
- Validator: Codex
- Scope: fix the `hero1axebouncevfxbinding` automation proof so the original accepted Bounce camera/framing can prove the current requested behavior: one moving projectile from hero to Primary, then one moving projectile from Primary to ChainSecond, with no third Bounce projectile/link.
- Stop condition: source changes are narrowly scoped, compile/capture evidence exists, and the fresh video/log proof passes; otherwise return a revise packet with the blocker.

Approved edit scope:
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
- Report artifact(s) under `Reports/AgentReviews/Hero1BounceOriginalCameraRecapture/`

Approved runtime/tool scope:
- Read relevant live instructions and source anchors.
- Patch the proof harness only. Do not change production Bounce combat semantics in `T66CombatComponent.cpp` unless you return for Codex re-approval.
- The intended fix is to keep normal Bounce target selection intact, but isolate the automation proof immediately before `PerformAutomationAutoAttackNow()` so non-proof/world enemies cannot become additional bounce targets.
- Use existing registry APIs when removing non-proof enemies, so `FindClosestTargetHandleInRange` cannot see stale registry entries.
- Run a focused compile/build command if available and reasonable.
- Run `Scripts/CaptureT66GameplayVideo.ps1` with the same original-camera route:
  - `-CaptureMode hero1axebouncevfxbinding`
  - `-UseHero1AxePreviewStaging`
  - `-NoHero1AxeTargets`
  - `-FrameCount 72`
  - `-FrameRate 12`
  - `-EvidenceBundle`
  - `-EvidenceAutoSelectFrames`
  - output under `C:/UE/T66/Saved/VideoCaptures/Hero1BounceOriginalCameraFixed_20260529/hero1axebouncevfxbinding.mp4`

Do not:
- Do not use `-UseReviewCamera`.
- Do not broaden into content, CSV, DataTable, Niagara, or production damage logic changes.
- Do not run Git commit/push/reset/clean.
- Do not run broad Git/LFS scans.
- Do not delete old evidence folders.

Required completion packet:
- Files changed with line anchors and rationale.
- Exact compile/capture commands and pass/fail result.
- Fresh video/evidence paths.
- Log evidence proving `LinkCount=2`, `LinkIndex=0`, `LinkIndex=1`, and no `LinkIndex=2`.
- Damage proof rows for Primary and ChainSecond, and negative controls.
- Yellow-block/camera-framing status.
- Claude token count if available.
