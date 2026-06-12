You are Claude Operator for `C:\UE\T66`; Codex is Validator/integrator.

Read `AGENTS.md`, `.t66/operator-state.json`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Gameplay/GAMEPLAY_AGENTS.md`, and the local pending issues file for `Source/T66/Gameplay` before changing code.

Current user correction:
- They do not want the wrong overhead/default-spawn view.
- They want the same camera/framing as the original accepted Bounce proof from `Saved/VideoCaptures/hero1axebouncevfxbinding_20260529_080850/`: hero visible from behind, enemies ahead, wall/stairs to the right, no pale yellow slab over the hero.
- The video must prove the current requested Bounce behavior: one projectile moves from hero to the first enemy, then one projectile moves from that enemy to a second enemy. It must not prove a third projectile/link.

Known current evidence:
- `Saved/VideoCaptures/Hero1BounceOriginalCameraClean_20260529/hero1axebouncevfxbinding.mp4` has the right camera and no yellow block, but it fails behavior proof: the log shows `LinkCount=3` and `LinkIndex=2`.
- The harness already stages only expected-hit targets `Primary` and `ChainSecond` for Bounce, with controls placed far away. The third link is coming from a non-proof/world enemy still visible to `FindClosestTargetHandleInRange` at fire time.

Approved task:
1. Patch only `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` if needed.
2. Keep production Bounce combat behavior unchanged. Do not edit `T66CombatComponent.cpp` unless you stop and request Codex re-approval.
3. Immediately before the proof fires, isolate the automation target population for `bBounceVFXBindingProofMode` so the normal Bounce target search can only find the staged proof targets. Use registry unregister where appropriate before destroying/ignoring non-proof enemies.
4. Do not remove the negative proof controls from HP-delta checking; they can remain far out of range and should still report `ExpectedHit=0 ActualHit=0`.
5. Run a focused compile if feasible.
6. Capture a fresh Unreal-owned video using:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/CaptureT66GameplayVideo.ps1 `
  -CaptureMode hero1axebouncevfxbinding `
  -UseHero1AxePreviewStaging `
  -NoHero1AxeTargets `
  -FrameCount 72 `
  -FrameRate 12 `
  -EvidenceBundle `
  -EvidenceAutoSelectFrames `
  -Output "C:/UE/T66/Saved/VideoCaptures/Hero1BounceOriginalCameraFixed_20260529/hero1axebouncevfxbinding.mp4"
```

Acceptance evidence:
- The contact sheet matches the original `080850` camera/framing and has no pale yellow block over the hero.
- The MP4 is non-empty and `ffprobe`/manifest show 1280x720, 72 frames, 12 fps, 6 seconds or equivalent.
- The log proves `CombatVFXBounceChainSequentialAttempt LinkIndex=0 LinkCount=2`, then `LinkIndex=1 LinkCount=2`, and no `LinkIndex=2`.
- The log proves arrival callback/deferred spawn from link 0 to 1.
- The log proves Primary and ChainSecond damage PASS and the negative controls remain unhit.

Write the completion packet to:
`Reports/AgentReviews/Hero1BounceOriginalCameraRecapture/claude_operator_completion_harnessfix.md`
