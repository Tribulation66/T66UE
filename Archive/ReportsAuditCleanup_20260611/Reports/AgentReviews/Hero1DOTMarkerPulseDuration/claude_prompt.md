You are Claude acting as Operator for the T66 repo. Codex has approved this bounded implementation/proof phase.

Working task:
Operator: Claude.
Validator: Codex.
Scope: change the Hero 1 DOT placeholder markers so each of the three periodic dot pulses disappears after 0.3 seconds. Keep the 0.5 second reveal cadence, so the current placeholder shows only one dot at a time. Preserve one moving DOT shot and one authoritative `HeroPrimaryDot` DOT payload. Mini/minigame systems remain out of scope.
Stop condition: source/docs/proof packet are updated, compile and Unreal-owned DOT proof capture are attempted, and Codex can validate the result.

Read and follow:
- `AGENTS.md`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Reports/AGENTS.md`

Current live seam:
- `Source/T66/Gameplay/T66DotMarkerVFX.h/.cpp` owns the temporary DOT marker spheres.
- Current behavior creates markers hidden, reveals marker 0 immediately, marker 1 at +0.5s, marker 2 at +1.0s, and leaves each visible until the marker actor self-destructs.
- The user now wants each marker to disappear after a short duration. For these placeholder markers, visible duration should be 0.3 seconds.
- This should not become a universal rule for all future DOT/final-art projectiles unless a future effect packet declares it; document it as the current placeholder pulse duration.

Implementation requirements:
- Add a named visible-duration constant, default `0.3f`, in the marker actor.
- Keep `MarkerRevealIntervalSeconds = 0.5f`.
- On reveal, make only that marker visible and schedule a hide/disappear for the same marker after 0.3 seconds.
- The current timing should be:
  - marker 0 visible 0.00s to 0.30s,
  - marker 1 visible 0.50s to 0.80s,
  - marker 2 visible 1.00s to 1.30s.
- Make timer callbacks safe if the actor is destroyed before hide fires.
- Preserve no collision, no damage authority, no DOT tuning changes, and `SnapToTargetNotIncludingScale` alignment.
- Add a stable proof token for disappear/hide such as `T66DotApplicatorMarkerHidden`, including target, marker index, marker count, planned reveal delay, and visible duration.
- Update `Gameplay/Combat/Hero1AxeDOTMechanismPacket.md` to describe reveal cadence plus 0.3s visible pulse duration for the temporary placeholder pass.

Verification to attempt:
- Focused compile:
  `"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE`
- Unreal-owned capture:
  `.\Scripts\CaptureT66GameplayVideo.ps1 -CaptureMode hero1axedotvfxbinding -UseHero1AxePreviewStaging -NoHero1AxeTargets -FrameCount 120 -FrameRate 12 -CaptureIntervalSeconds 0.08 -DelaySeconds 5 -TimeoutSeconds 220 -EvidenceBundle -EvidenceAutoSelectFrames -EvidenceLabel Hero1DOTMarkerPulseDuration`
- Inspect logs for the expected one-shot/one-payload/three-reveal/three-hide counts and alignment.

Output requirements:
- Write your operator packet under `Reports/AgentReviews/Hero1DOTMarkerPulseDuration/operator_packet.md`.
- First non-empty line of the packet must be exactly:
  `Operator Packet: COMPLETE`
- Include changed files, verification commands/results, capture path, log evidence, caveats, and token usage if exposed by the helper.
