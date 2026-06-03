Codex Validation: ACCEPTED

Task: Hero 1 DOT marker pulse duration.
Operator: Claude (`claude-opus-4-8`, FullOperator).
Validator: Codex.

Validated result:
- `AT66DotMarkerVFX` keeps the 0.5s marker reveal cadence and adds a 0.3s per-marker visible duration.
- Marker 0 is visible from the immediate reveal until its 0.3s hide timer, marker 1 from +0.5s to +0.8s, and marker 2 from +1.0s to +1.3s.
- Each marker hide is a visual-only `SetVisibility(false, true)` operation with guarded index/null checks.
- No DOT damage tuning, target acquisition, idol behavior, collision authority, or source identity changed.
- `Gameplay/Combat/Hero1AxeDOTMechanismPacket.md` now documents the 0.3s pulse as the current placeholder behavior only, not a universal rule for future DOT/final-art projectiles.

Validated code anchors:
- `Source/T66/Gameplay/T66DotMarkerVFX.h:39` keeps `MarkerRevealIntervalSeconds = 0.5f`.
- `Source/T66/Gameplay/T66DotMarkerVFX.h:50` defines `MarkerVisibleDurationSeconds = 0.3f`.
- `Source/T66/Gameplay/T66DotMarkerVFX.cpp:116-139` reveals a marker and schedules its hide after `MarkerVisibleDurationSeconds`.
- `Source/T66/Gameplay/T66DotMarkerVFX.cpp:143-168` hides a marker safely and logs `T66DotApplicatorMarkerHidden`.
- `Gameplay/Combat/Hero1AxeDOTMechanismPacket.md:14` records the pulse-window contract and states that the exact 0.3s duration is placeholder-specific.

Verification accepted:
- Focused compile: `Reports/AgentReviews/Hero1DOTMarkerPulseDuration/compile_log.txt` shows `Result: Succeeded`. One unrelated pre-existing Niagara deprecation warning remains in `T66Hero1AxeAOEVFXLabActor.cpp`.
- Unreal-owned capture: `C:\UE\T66\Saved\VideoCaptures\hero1axedotvfxbinding_20260530_000342\hero1axedotvfxbinding.mp4`.
- Auto evidence bundle: `C:\UE\T66\Saved\VideoCaptures\hero1axedotvfxbinding_20260530_000342\evidence\contact_sheet.png`.
- Manual pulse strip: `C:\UE\T66\Saved\VideoCaptures\hero1axedotvfxbinding_20260530_000342\evidence_pulse_manual\contact_sheet.png`, sampling frames 45, 48, 50, and 55. It shows a marker pulse, an empty gap after the hide, the next marker pulse, and a later single marker pulse with no accumulation.
- Durable log proof: `Reports/AgentReviews/Hero1DOTMarkerPulseDuration/dot_proof_loglines.txt`.
- Current live log count check against `Saved/Logs/T66.log` matches expected counts: one `T66DotShotSpawned`, one `T66DotApplicatorMarkersSpawned MarkerCount=3`, three `T66DotApplicatorMarkerRevealed`, three `T66DotApplicatorMarkerHidden VisibleDurationSeconds=0.30`, one `T66DotPayloadApplied ... Source=HeroPrimaryDot`, and one alignment line with `OffsetSize=0.000`.

Visual caveat:
- These are still temporary placeholder spheres. The capture proves the requested pulse lifetime and non-accumulation behavior; it does not approve final DOT Niagara art.

Repo-state note:
- `Source/T66/Gameplay/T66DotMarkerVFX.h/.cpp` remain untracked source files from the DOT placeholder feature batch. They must be included when this work is staged/committed.

PPF CLOSE
Process used: Combat VFX placeholder structure using the repo gameplay capture process plus log-backed damage authority and pulse-duration proof.
Matches declared process: YES for the temporary structure; final DOT Niagara art remains deferred by scope.
Evidence: focused compile, Unreal-owned MP4, manual pulse strip, and DOT shot/reveal/hide/payload/alignment log markers listed above.

MECHANISM CLOSE
Mechanism: short per-marker pulse.
Status: PRESENT
Evidence: three `T66DotApplicatorMarkerHidden` lines with `VisibleDurationSeconds=0.30`; manual pulse strip frame 45 has one dot, frame 48 has no dot, frame 50 has the next dot, frame 55 has a later single dot.
Discriminator test: markers do not accumulate; each pulse disappears before the next one appears.
Reported status: FULL

MECHANISM CLOSE
Mechanism: single authoritative DOT payload.
Status: PRESENT
Evidence: exactly one `T66DotPayloadApplied ... Source=HeroPrimaryDot`; marker components remain visual-only and no-collision.
Discriminator test: three visual pulses do not produce three DOT payloads or three damage lanes.
Reported status: FULL
