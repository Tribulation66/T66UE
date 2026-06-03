You are Claude acting as Operator for the T66 repo. Codex has approved this bounded implementation phase.

Working task:
Operator: Claude.
Validator: Codex.
Scope: change the Hero 1 DOT placeholder markers so the three DOT applicator spheres appear over time, one every 0.5 seconds, while preserving one moving DOT shot and one authoritative DOT payload. Mini/minigame systems remain out of scope.
Stop condition: code/docs/proof packet are updated, compile and Unreal-owned DOT proof capture are attempted, and Codex can validate the result.

Repo/process constraints:
- Work in `C:\UE\T66`.
- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Gameplay/GAMEPLAY_AGENTS.md`, `Gameplay/Combat/CombatVFXAuthoringProcedure.md`, and `Reports/AGENTS.md`.
- Do not use native goal tools.
- Do not inspect/change Mini/minigame systems.
- Do not run destructive git commands or broad Unreal asset/LFS scans.
- Preserve user changes.
- Keep this to the DOT marker cadence fix unless a compile blocker directly requires a small adjacent fix.

Current live seam:
- `Source/T66/Gameplay/T66DotMarkerVFX.h/.cpp` owns the three temporary target-attached marker spheres.
- `Source/T66/Gameplay/T66CombatComponent.cpp` calls `Markers->InitializeMarkers(FollowTarget, DotMarkerCount, Color, MarkerScale, Duration);` from `SpawnDOTApplicatorMarkers`.
- Prior proof showed all three marker spheres appearing simultaneously; this request requires a half-second cadence.
- Existing damage authority must remain one `T66DotPayloadApplied ... Source=HeroPrimaryDot` payload. The three markers remain visual-only.

Implementation target:
- Hide all marker components when created, then reveal marker index 0 immediately, marker index 1 after 0.5 seconds, and marker index 2 after 1.0 seconds.
- Use Unreal timer manager or equivalent gameplay-safe timing. Keep actor tick disabled if timers are enough.
- Add proof log lines with a stable searchable token such as `T66DotApplicatorMarkerRevealed`, including target name, marker index, marker count, and planned reveal delay.
- Keep marker attachment alignment unchanged (`SnapToTargetNotIncludingScale`) and preserve no collision / no damage authority.
- If you add timer callbacks, make them safe if the marker actor is destroyed before a later reveal.
- Update `Gameplay/Combat/Hero1AxeDOTMechanismPacket.md` to state that the three placeholder DOT applicator markers reveal on a 0.5s cadence, not simultaneously.

Verification to attempt:
- Focused compile for T66Editor Win64 Development using the repo's normal build route.
- Unreal-owned proof capture:
  `.\Scripts\CaptureT66GameplayVideo.ps1 -CaptureMode hero1axedotvfxbinding -UseHero1AxePreviewStaging -NoHero1AxeTargets -FrameCount 120 -FrameRate 12 -CaptureIntervalSeconds 0.08 -DelaySeconds 5 -TimeoutSeconds 220 -EvidenceBundle -EvidenceAutoSelectFrames -EvidenceLabel Hero1DOTMarkerCadence`
- Inspect logs for:
  - one `T66DotShotSpawned`
  - one `T66DotApplicatorMarkersSpawned MarkerCount=3`
  - three `T66DotApplicatorMarkerRevealed` lines with 0.00/0.50/1.00 planned delays
  - one `T66DotPayloadApplied ... Source=HeroPrimaryDot`
  - marker alignment remains near zero offset

Output requirements:
- Write your operator packet under `Reports/AgentReviews/Hero1DOTMarkerCadence/`.
- First non-empty line of the packet must be exactly:
  `Operator Packet: COMPLETE`
- Include changed files, verification commands/results, artifact paths, logs inspected, token usage if exposed by the helper, and caveats.
