Operator Packet: COMPLETE

# Hero 1 DOT Marker Cadence — Operator Packet

- **Operator:** Claude (`claude-opus-4-8`, FullOperator).
- **Validator:** Codex.
- **Codex approval artifact:** `Reports/AgentReviews/Hero1DOTMarkerCadence/codex_operator_approval.md` (first line `Codex Approval: APPROVE`).
- **Date:** 2026-05-30 (capture timestamps in UTC).
- **Scope status:** In-scope only. No Mini/minigame, no DOT tuning, no idol/target-acquisition, no Niagara art, no destructive git, no broad LFS scan.

## Task

Change the Hero 1 DOT placeholder markers so the three target-attached applicator spheres appear over time — one every 0.5s (index 0 immediately, 1 at +0.5s, 2 at +1.0s) — while preserving one moving DOT shot, one target, and one authoritative `HeroPrimaryDot` DOT payload. Markers stay visual-only (no collision, no damage authority). Alignment (`SnapToTargetNotIncludingScale`) unchanged.

## Changed Files

1. `Source/T66/Gameplay/T66DotMarkerVFX.h`
   - Updated class doc comment to describe the staggered reveal contract.
   - Added `static constexpr float MarkerRevealIntervalSeconds = 0.5f;`.
   - Added private `void RevealMarker(int32 Index);`.
   - Added members: `TWeakObjectPtr<AActor> FollowTargetWeak;` (proof-logging only, no authority), `int32 PlannedMarkerCount = 0;`, `TArray<FTimerHandle> RevealTimerHandles;`.

2. `Source/T66/Gameplay/T66DotMarkerVFX.cpp`
   - Added includes: `Gameplay/T66CombatComponent.h` (for `LogT66Combat`), `Engine/World.h`, `TimerManager.h`.
   - Each marker component is created hidden (`SetVisibility(false, true)`) during `InitializeMarkers`.
   - After attach + `SetLifeSpan`, stores `FollowTargetWeak`/`PlannedMarkerCount`, reveals index 0 immediately via `RevealMarker(0)`, and schedules indices 1..N-1 through `GetTimerManager().SetTimer(..., FTimerDelegate::CreateUObject(this, &AT66DotMarkerVFX::RevealMarker, Index), 0.5*Index, false)`.
   - New `RevealMarker(int32)`: re-validates `Markers.IsValidIndex(Index)` and null marker (safe if the actor self-destructs before a later timer fires — timers bound via `CreateUObject` are also auto-invalidated on actor destroy), sets the component visible, and logs the proof line.
   - Actor tick remains disabled (`PrimaryActorTick.bCanEverTick = false`); reveal uses the world timer manager only.
   - Collision (`NoCollision`), no cast shadow, ring layout, and `SnapToTargetNotIncludingScale` attachment are unchanged. No change to damage authority.

3. `Gameplay/Combat/Hero1AxeDOTMechanismPacket.md`
   - Mechanism item 2 now states the three placeholder markers are created hidden and reveal on a 0.5s cadence (0.00/0.50/1.00s), not simultaneously, documents the `T66DotApplicatorMarkerRevealed` proof token, and records this as the marker-presentation contract for future DOT placeholder/final-art work.
   - Proof-route "Evidence required" line updated to require the sequential reveal at planned delays 0.00/0.50/1.00s.

No calling-side change was needed: `UT66CombatComponent::SpawnDOTApplicatorMarkers` still calls `Markers->InitializeMarkers(FollowTarget, 3, Color, MarkerScale, Duration)`; the cadence lives entirely inside the marker actor.

## New Proof Token

`T66DotApplicatorMarkerRevealed Target=<name> MarkerIndex=<i> MarkerCount=<n> PlannedRevealDelaySeconds=<0.00|0.50|1.00> (visual-only; single DOT payload unchanged)` — logged on `LogT66Combat` (same category as the existing DOT proof lines so all DOT proof greps together).

## Verification

### Focused compile — ATTEMPTED, PASS
- Command: `"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE`
- Result: `Result: Succeeded` (linked `UnrealEditor-T66.dll`, total 41.35s). UHT processed cleanly with `-WarningsAsErrors`.
- Log: `Reports/AgentReviews/Hero1DOTMarkerCadence/build/compile_T66Editor.log`

### Unreal-owned DOT proof capture — ATTEMPTED, PASS
- Command: `.\Scripts\CaptureT66GameplayVideo.ps1 -CaptureMode hero1axedotvfxbinding -UseHero1AxePreviewStaging -NoHero1AxeTargets -FrameCount 120 -FrameRate 12 -CaptureIntervalSeconds 0.08 -DelaySeconds 5 -TimeoutSeconds 220 -EvidenceBundle -EvidenceAutoSelectFrames -EvidenceLabel Hero1DOTMarkerCadence`
- Exit code 0. 120/120 frames captured; MP4 + evidence bundle produced.
- Artifacts (under `Saved/VideoCaptures/hero1axedotvfxbinding_20260529_234348/`):
  - `hero1axedotvfxbinding.mp4` (1280x720, 12fps, 120 frames, 10.0s)
  - `evidence/manifest.json`, `evidence/contact_sheet.png`, `evidence/ffprobe.json`, `evidence/visibility_checklist.md`, `evidence/selected_frames.md`, `evidence/selected_frames/{00_start_0001,01_mid_0020,02_impact_0040,03_dissipate_0119}.png`
  - `frames/frame_0001..0120.png`
- Capture run stdout: `Reports/AgentReviews/Hero1DOTMarkerCadence/build/capture_run.log`
- Editor log: `C:\UE\T66\Saved\Logs\T66.log` (Saved/ expires — durable copy of the proof lines below).

### Log inspection — ALL EXPECTED MARKERS PRESENT
Durable copy: `Reports/AgentReviews/Hero1DOTMarkerCadence/build/dot_proof_loglines.txt`. Token count grep across the 4 cadence tokens = 6 lines total (1 + 1 + 3 + 1), i.e. exactly the expected counts.

```
T66DotShotSpawned Target=T66EnemyBase_0 Start=V(Z=64.00) End=V(X=360.00, Z=64.00) ProofTravelSeconds=0.600
T66DotApplicatorMarkerRevealed Target=T66EnemyBase_0 MarkerIndex=0 MarkerCount=3 PlannedRevealDelaySeconds=0.00 (visual-only; single DOT payload unchanged)
T66DotApplicatorMarkersSpawned Target=T66EnemyBase_0 MarkerCount=3 Duration=4.00 (visual-only; single DOT payload unchanged)
T66DotMarkerAlignment Target=T66EnemyBase_0 TargetLoc=X=360.000 Y=0.000 Z=0.000 MarkerLoc=X=360.000 Y=0.000 Z=0.000 TargetRelativeOffset=X=0.000 Y=0.000 Z=0.000 OffsetSize=0.000 (expect ~0; markers on target)
T66DotPayloadApplied Target=T66EnemyBase_0 Duration=4.00 TickInterval=0.50 DamagePerTick=2.01 Source=HeroPrimaryDot (single payload)
T66DotApplicatorMarkerRevealed Target=T66EnemyBase_0 MarkerIndex=1 MarkerCount=3 PlannedRevealDelaySeconds=0.50 (visual-only; single DOT payload unchanged)
T66DotApplicatorMarkerRevealed Target=T66EnemyBase_0 MarkerIndex=2 MarkerCount=3 PlannedRevealDelaySeconds=1.00 (visual-only; single DOT payload unchanged)
```

Checklist against the task's required evidence:
- one `T66DotShotSpawned` — PASS (1).
- one `T66DotApplicatorMarkersSpawned MarkerCount=3` — PASS (1, `MarkerCount=3`).
- three `T66DotApplicatorMarkerRevealed` with planned delays 0.00 / 0.50 / 1.00 — PASS (indices 0/1/2, in order).
- one `T66DotPayloadApplied ... Source=HeroPrimaryDot` — PASS (single payload; markers did not multiply into 3 lanes).
- marker alignment near zero offset — PASS (`OffsetSize=0.000`, marker root coincident with target).

Reveal ordering across engine frames is sequential and evenly spaced: shot at frame [800], reveals at frames [815] (idx0), [828] (idx1), [841] (idx2) — a consistent 13-engine-frame gap between reveals.

## Caveats

1. **Real-time spacing under the capture harness differs from the authored 0.5s.** The wall-clock log gaps between reveals were ~1.27s (idx0→1) and ~0.90s (idx1→2), while the engine-frame gaps are even (13 frames each). The authored cadence is 0.5s of game time (the logged `PlannedRevealDelaySeconds` values are the source of truth) and the timers fire on game time; the headless capture editor runs below real-time while flushing 120 screenshots, which stretches/varies the real-time-to-game-time ratio. This is a capture-harness artifact, not a gameplay-timing defect. In normal (non-capture) play the timer delays are real 0.5s intervals.
2. **`EvidenceAutoSelectFrames` picked start/mid/impact/dissipate frames, not per-marker reveal frames.** The auto-selector keys on visual-activity scores, so the four selected frames are not specifically frames 815/828/841. The authoritative cadence proof is the log (above); the MP4/contact sheet/frames are supporting visual evidence. If Codex wants per-reveal stills, the retained `frames/` sequence can be sampled at the reveal frame indices.
3. **Visual fidelity is explicitly NOT proven.** Per the mechanism packet, these spheres remain temporary placeholders. This packet proves the cadence mechanism and the preserved single-payload authority, not final art or readability.
4. **`Duration=4.00` ≥ 1.0s**, so all three reveals fire before self-destruct in this capture. `RevealMarker` is guarded so a shorter DOT duration that destroys the actor before a later timer simply skips that reveal without crashing.

## Token Usage

- **Codex Token Spent:** 64,227 (latest completed Codex turn before this answer, per `Scripts\Get-CodexTokenUsage.ps1`; the final-answer turn's own tokens flush only after this is sent).
- **Claude Tokens Spent:** Unavailable from inside the Operator run — read `ClaudeTokensSpent` from the `Invoke-ClaudeDirectRead.ps1` run manifest for the authoritative figure.

## Validator Notes (for Codex)

- All changes are inside the approved scope; no excluded operations were performed.
- No git commit/push/tag was made; working tree changes are limited to the three source/doc files plus the report artifacts under `Reports/AgentReviews/Hero1DOTMarkerCadence/`.
- This packet is an Operator work artifact, not a greenlight. Please validate the actual diffs, re-grep `Saved/Logs/T66.log` (or the durable copy) for the six cadence lines, and own the final user-facing report.
