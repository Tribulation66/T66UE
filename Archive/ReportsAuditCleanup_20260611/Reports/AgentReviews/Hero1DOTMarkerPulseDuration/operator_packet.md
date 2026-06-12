Operator Packet: COMPLETE

# Operator Packet: Hero1DOTMarkerPulseDuration

## Working Task And Tier

Working task:
- Operator: Claude (FullOperator).
- Validator: Codex.
- Scope: Make each of the three Hero 1 DOT placeholder markers disappear after a 0.3s visible pulse, while keeping the 0.5s reveal cadence so only one placeholder dot shows at a time. Preserve one moving DOT shot and one authoritative `HeroPrimaryDot` DOT payload. Mini/minigame out of scope.
- Stop condition: source/docs/proof packet updated; focused compile and Unreal-owned DOT capture attempted; Codex can validate.

Validation depth: full. Reason: runtime C++ change to combat presentation plus Unreal-owned capture proof (multiple proof classes: compile log + gameplay capture + log tokens).

Tier: bounded single-phase implementation/proof under a valid Codex approval artifact.

Scope boundaries: visual-only placeholder marker lifetime. No damage authority, collision, DOT tuning, targeting, idol, or source-identity changes.

## Roles And Tool Profile

- Operator model: claude-opus-4-8 (FullOperator).
- Validator model: Codex.
- Finisher: Codex (final user-facing report).
- Helper / tool profile: `Scripts\Invoke-ClaudeDirectRead.ps1 -ToolProfile FullOperator` (this run).
- Codex approval artifact: `C:\UE\T66\Reports\AgentReviews\Hero1DOTMarkerPulseDuration\codex_operator_approval.md` (first line `Codex Approval: APPROVE`).
- Operator run dir / manifest: written by the helper under `Reports/AgentReviews/ClaudeDirectRead/<Run>/manifest.json` after this run completes (not readable mid-run).

## User Constraints And Out Of Scope

- Constraints: keep `MarkerRevealIntervalSeconds = 0.5f`; add a named visible-duration constant default `0.3f`; only the revealed marker visible; schedule per-marker hide at +0.3s; safe timer callbacks if actor destroyed before hide; preserve `SnapToTargetNotIncludingScale` alignment, no collision, no damage authority, no DOT tuning; add a stable hide proof token; document this as the current placeholder pulse duration only (not a universal rule).
- Implementation approval: granted via Codex approval artifact above.
- Mini/minigame: out of scope, untouched.
- Out of scope: DOT damage tuning, target acquisition, idol behavior, source identity, Bounce/Pierce/AOE weapon behavior, final Niagara DOT art, git/LFS operations.

## Applicable Instructions Read

- `AGENTS.md` — root router; derive task contract, no native goal tools, Unreal-owned capture for visual proof, exact verification reporting. Takeaway: stay inside approved scope and prove with Unreal-owned evidence.
- `OPERATOR_VALIDATOR_PROTOCOL.md` — Operator/Validator stack, proof-bearing FullOperator routing, verification freshness, packet shape. Takeaway: produce a complete packet; Claude proof is evidence, Codex validates and finalizes.
- `Gameplay/GAMEPLAY_AGENTS.md` — gameplay runtime owner; runtime-facing changes need compile/build verification; preserve data-authored tuning. Takeaway: compile is required; do not move tuning into C++.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md` — generic combat VFX authoring/validation; capture via `Scripts/CaptureT66GameplayVideo.ps1`; structural proof != visual fidelity. Takeaway: this placeholder pass does not claim final visual fidelity; it proves structure/timing via logs + capture.
- `Reports/AGENTS.md` — durable packet under `Reports/AgentReviews/<TaskSlug>/`; raw captures stay discoverable. Takeaway: packet + preserved loglines live in this task folder.

## Evidence And Live Findings

Live seam (pre-change):
- `Source/T66/Gameplay/T66DotMarkerVFX.h:39` — `MarkerRevealIntervalSeconds = 0.5f` (kept).
- `Source/T66/Gameplay/T66DotMarkerVFX.cpp:79-97` — index 0 revealed immediately, indices 1/2 via world timer at +0.5s/+1.0s; markers stayed visible until self-destruct (`SetLifeSpan`).
- `Source/T66/Gameplay/T66DotMarkerVFX.cpp:100-124` — `RevealMarker` set visibility true + logged `T66DotApplicatorMarkerRevealed`; no hide.

Caller / payload integrity (unchanged): `Source/T66/Gameplay/T66CombatComponent.cpp` owns `SpawnDOTApplicatorMarkers` / `SpawnVisualTravelProjectile` and the single `ApplyDOT(... HeroPrimaryDot)` payload — not touched by this change.

Post-change live findings (this run):
- Focused compile `Result: Succeeded` — `Reports/AgentReviews/Hero1DOTMarkerPulseDuration/compile_log.txt`. Only warning is pre-existing C4996 in `T66Hero1AxeAOEVFXLabActor.cpp` (unrelated lab actor), not in the edited files.
- Unreal-owned capture written: `Saved/VideoCaptures/hero1axedotvfxbinding_20260530_000342/hero1axedotvfxbinding.mp4` (+ frames + `evidence/` bundle with contact sheet/manifest). Log: `Reports/AgentReviews/Hero1DOTMarkerPulseDuration/capture_log.txt`.
- Proof tokens from `Saved/Logs/T66.log` (preserved at `Reports/AgentReviews/Hero1DOTMarkerPulseDuration/dot_proof_loglines.txt`):
  - `T66DotShotSpawned Target=T66EnemyBase_0 ...` ×1.
  - `T66DotApplicatorMarkersSpawned ... MarkerCount=3` ×1.
  - `T66DotApplicatorMarkerRevealed ... MarkerIndex=0/1/2 PlannedRevealDelaySeconds=0.00/0.50/1.00` ×3.
  - `T66DotApplicatorMarkerHidden ... MarkerIndex=0/1/2 PlannedRevealDelaySeconds=0.00/0.50/1.00 VisibleDurationSeconds=0.30` ×3 (new token).
  - `T66DotPayloadApplied ... Source=HeroPrimaryDot (single payload)` ×1.
  - `T66DotMarkerAlignment ... TargetRelativeOffset=X=0.000 Y=0.000 Z=0.000 OffsetSize=0.000` — alignment preserved.
  - Ordering confirms reveal→hide before the next reveal: only one marker visible at a time.

## PPF And Process Gates

This is a bounded behavior change to a temporary placeholder visual (timing/visibility lifetime of engine sphere markers + a log token + a doc note). It is not new Niagara/material/mesh authoring, so the full Niagara-VFX PPF/artifact-parity/mechanism ceremony does not govern it; `Hero1AxeDOTMechanismPacket.md` already classifies these markers as intentional placeholders and explicitly does not approve final visual fidelity. The change preserves the declared placeholder mechanism manifest (single shot, hit-triggered staggered markers, single `HeroPrimaryDot` payload, target-following persistence, impact-context publication) and only refines marker visible-lifetime. Proof is via compile + Unreal-owned capture + stable log tokens, per the combat VFX capture route.

## Proposed Patch Approach (as implemented)

1. `Source/T66/Gameplay/T66DotMarkerVFX.h`
   - Added `static constexpr float MarkerVisibleDurationSeconds = 0.3f;` with a comment marking it the current placeholder pulse duration (not a universal rule).
   - Declared `void HideMarker(int32 Index);`.
   - Added `TArray<FTimerHandle> HideTimerHandles;`.
   - Updated class doc comment to describe the 0.3s pulse windows (0.00–0.30 / 0.50–0.80 / 1.00–1.30).
   - Blast radius: header-only declarations; no ABI consumers outside this actor. Rollback: revert file.

2. `Source/T66/Gameplay/T66DotMarkerVFX.cpp`
   - Updated the staggered-reveal comment to describe the pulse.
   - In `RevealMarker`, after making the marker visible + logging, schedule a per-marker hide via the world timer manager at `MarkerVisibleDurationSeconds` (relative to actual reveal), bound to this UObject; handle stored in `HideTimerHandles`.
   - Added `HideMarker(int32 Index)`: re-validates index/marker, sets visibility false, logs new `T66DotApplicatorMarkerHidden` token with target, marker index, marker count, planned reveal delay, and visible duration.
   - Blast radius: visual-only; no collision/damage/tuning paths touched. Timers bound to UObject auto-invalidate on destroy; index re-validated. Rollback: revert file.

3. `Gameplay/Combat/Hero1AxeDOTMechanismPacket.md`
   - Mechanism item 2 retitled to "staggered marker pulse"; documents 0.3s visible pulse, the three visibility windows, the new `T66DotApplicatorMarkerHidden` token, and that the 0.3s pulse is the current placeholder only (future effects opt in via their own packet).
   - Proof Route evidence line updated to require the hide token at `VisibleDurationSeconds=0.30`.
   - Blast radius: docs only. Rollback: revert file.

## Verification Plan (executed)

- Focused compile: `"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE` → `Result: Succeeded`. Pass marker: `Result: Succeeded`, no errors in edited files. Failure would mean a syntax/binding error in the marker actor.
- Unreal-owned capture: `Scripts\CaptureT66GameplayVideo.ps1 -CaptureMode hero1axedotvfxbinding -UseHero1AxePreviewStaging -NoHero1AxeTargets -FrameCount 120 -FrameRate 12 -CaptureIntervalSeconds 0.08 -DelaySeconds 5 -TimeoutSeconds 220 -EvidenceBundle -EvidenceAutoSelectFrames -EvidenceLabel Hero1DOTMarkerPulseDuration` → MP4 + evidence bundle written.
- Log inspection: confirmed 1 shot / 1 MarkerCount=3 spawn / 3 reveals (0.00,0.50,1.00) / 3 hides (0.30) / 1 HeroPrimaryDot payload / alignment OffsetSize=0.000, with reveal→hide ordering.
- Intentionally not run: staged standalone refresh (this is a placeholder VFX timing change exercised through the gameplay capture harness, not a standalone-shortcut-affecting change requested by the user); commit/push/tag (excluded). Note for Codex: wall-clock deltas between reveal/hide timestamps differ from 0.3s/0.5s because capture frame-stepping/time-dilation affects wall clock; the authoritative timing is carried by the logged `PlannedRevealDelaySeconds` and `VisibleDurationSeconds` token values (game-time), matching the prior cadence-proof convention.

## Token Routing
TOKEN ROUTING
OperatorModel: claude-opus-4-8
OperatorTokensSpent: Unavailable (helper manifest ClaudeTokensSpent written after this run completes)
OperatorRunDir: Reports/AgentReviews/ClaudeDirectRead/<this run> (helper-assigned)
OperatorManifest: Reports/AgentReviews/ClaudeDirectRead/<this run>/manifest.json
CodexApprovalPath: Reports/AgentReviews/Hero1DOTMarkerPulseDuration/codex_operator_approval.md
ExpectedValidatorDepth: targeted
ValidatorBudgetHint: Diff `T66DotMarkerVFX.h/.cpp`; confirm `MarkerVisibleDurationSeconds=0.3f`, `MarkerRevealIntervalSeconds` unchanged at 0.5f, hide scheduled in RevealMarker, HideMarker re-validates index, new token fields; spot-check `dot_proof_loglines.txt` for the 1/1/3/3/1 counts + OffsetSize=0.000; confirm no DOT-tuning/collision/payload edits leaked.

## Operator Position And Open Decisions

- Recommendation: APPROVE. Change is bounded, compiles, and the live capture log shows the exact requested behavior (one dot visible at a time, each pulsing for 0.3s) with shot/payload/alignment invariants preserved.
- Caveats: (1) wall-clock timestamp deltas are not literally 0.3s/0.5s due to capture-time dilation — authoritative values are in the token fields; (2) `OperatorTokensSpent` is Unavailable mid-run by design; read it from the helper manifest. (3) This packet is an Operator work artifact, not a greenlight; Codex owns final validation and the user-facing report.
- Decisions only the user can make: none required for this scope.
- For Codex to challenge: whether the 0.3s pulse reads well in the MP4 at gameplay speed (visual judgment) vs. the structural log proof here.

## Anti-Lookalike Discriminator

Cheap wrong result: leaving all three markers visible (or hiding only via end-of-life self-destruct) so multiple dots show at once, or a uniform fade with no discrete per-marker hide. Discriminator: the log shows three discrete `T66DotApplicatorMarkerHidden` events each at `VisibleDurationSeconds=0.30`, interleaved as reveal0→hide0→reveal1→hide1→reveal2→hide2, proving exactly one marker is visible per window rather than an accumulating or always-on set.
