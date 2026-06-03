# Completion Packet: Hero 1 Bounce Original-Camera Harness Fix

Operator: Claude (`claude-opus-4-8`)
Validator/Finisher: Codex
Codex approval artifact: `Reports/AgentReviews/Hero1BounceOriginalCameraRecapture/codex_operator_approval_harnessfix.md` (first line `Codex Approval: APPROVE`)

## Outcome

PASS (Operator evidence; Codex validates). The original accepted Bounce
camera/framing now proves the current requested two-link behavior: one moving
projectile hero -> Primary, then one moving projectile Primary -> ChainSecond,
with no third Bounce projectile/link.

Root cause confirmed at fire time: the Bounce chain target search
(`FindClosestTargetHandleInRange`) walks the actor registry's enemies,
lightweight mobs, and bosses. The proof preamble destroyed `AT66EnemyBase`
world actors but never cleared lightweight `AT66MobBase` mobs or bosses, so a
surviving non-proof/world enemy became an unintended third bounce link
(`LinkCount=3`, `LinkIndex=2`). The fresh isolation log shows exactly what was
leaking in: `ReleasedMobs=31 DestroyedEnemies=0 RemovedBosses=1`.

## Files Changed

- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
  - Line ~3837 (fire-timer weak lambda capture list): added
    `bBounceVFXBindingProofMode` so the fire callback can gate the isolation to
    the Bounce proof only.
  - Lines ~3864 (immediately before
    `SetLockedTarget`/`PerformAutomationAutoAttackNow`): added a Bounce-only
    target-population isolation block. Immediately before firing it removes
    every damageable target that is **not** a staged proof target (proof targets
    carry `HitboxProofTag`):
    - Lightweight mobs (`Registry->GetActiveMobs()`): removed via
      `MobManager->ReleaseMob(...)` (which unregisters from manager + registry
      and deactivates/pools), with a `Registry->UnregisterMob(...)` + `Destroy()`
      fallback if the manager is missing.
    - Stray enemies (`Registry->GetEnemies()` without the proof tag):
      `Registry->UnregisterEnemy(...)` then `Destroy()`.
    - Bosses (`Registry->GetBosses()`): `Registry->UnregisterBoss(...)` then
      `Destroy()`.
    - Emits `[Hero1AxeBounceProof] IsolatedTargetPopulation ReleasedMobs=%d
      DestroyedEnemies=%d RemovedBosses=%d ProofTargets=%d`.

Production Bounce combat semantics were not touched. `T66CombatComponent.cpp`
was read-only for analysis; no edit. The negative proof controls were left
registered and far out of range, so they still appear in HP-delta checking and
report `ExpectedHit=0 ActualHit=0`.

Blast radius: limited to the `hero1axebouncevfxbinding` automation proof path.
Rollback: revert the two edits in `T66PlayerController_Overlays.cpp`.

## Verification

### Compile
- Command: `"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex`
- Result: `Result: Succeeded` (33.58s; linked `UnrealEditor-T66.dll`).

### Capture (exact approved command)
```
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
- Result: exit 0. No `-UseReviewCamera` used (original-camera route only).

### Evidence paths
- MP4: `C:/UE/T66/Saved/VideoCaptures/Hero1BounceOriginalCameraFixed_20260529/hero1axebouncevfxbinding.mp4`
- Contact sheet: `.../evidence/contact_sheet.png`
- ffprobe: `.../evidence/ffprobe.json`
- Manifest: `.../evidence/manifest.json`
- Frames: `.../frames/` (frame_0001..frame_0072)
- Editor log: `C:/UE/T66/Saved/Logs/T66.log` (run timestamped 2026.05.29-13.17)

### MP4 / ffprobe
- 1280x720, `r_frame_rate 12/1`, `nb_frames 72`, `duration 6.000000`,
  size 215495 bytes (non-empty). Matches the 1280x720 / 72 frames / 12 fps /
  6 second acceptance shape.

### Log proof — two-link behavior (no third link)
```
[Hero1AxeBounceProof] IsolatedTargetPopulation ReleasedMobs=31 DestroyedEnemies=0 RemovedBosses=1 ProofTargets=5
CombatVFXBounceChainSequentialAttempt LinkIndex=0 LinkCount=2 Start=V(Z=64.00) End=V(X=360.00, Z=64.00) Distance=360.00 ...
CombatVFXBounceLinkProjectile         LinkIndex=0 LinkCount=2 ... Carrier=/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash...
CombatVFXBounceLinkArrivalCallback    NextLinkIndex=1 ChainPoints=3 CarrierValid=1 Time=12.750
CombatVFXBounceLinkArrivalDeferredSpawn NextLinkIndex=1 ChainPoints=3 CarrierValid=1 Time=12.953
CombatVFXBounceChainSequentialAttempt LinkIndex=1 LinkCount=2 Start=V(X=360.00, Y=36.00, Z=64.00) End=V(X=360.00, Y=150.00, Z=64.00) ...
CombatVFXBounceLinkProjectile         LinkIndex=1 LinkCount=2 ...
```
- `LinkCount=2` throughout; `LinkIndex=0` then `LinkIndex=1`.
- Grep for `LinkIndex=2` and `LinkCount=3` in the run log: **no matches**.
- Arrival callback + deferred spawn proves link 0 -> link 1 hand-off
  (`CombatVFXBounceLinkArrivalCallback NextLinkIndex=1`,
  `CombatVFXBounceLinkArrivalDeferredSpawn NextLinkIndex=1`).

### Log proof — damage rows
```
Target=Primary       ExpectedHit=1 ActualHit=1 HPBefore=20000 HPAfter=19972 Result=PASS   (-28)
Target=ChainSecond   ExpectedHit=1 ActualHit=1 HPBefore=20000 HPAfter=19974 Result=PASS   (-26)
Target=OutOfChainRangeForward ExpectedHit=0 ActualHit=0 HPBefore=20000 HPAfter=20000 Result=PASS
Target=OutOfChainRangeSide    ExpectedHit=0 ActualHit=0 HPBefore=20000 HPAfter=20000 Result=PASS
Target=OutsideBehind          ExpectedHit=0 ActualHit=0 HPBefore=20000 HPAfter=20000 Result=PASS
```
- Primary and ChainSecond damage PASS; all three negative controls remain unhit.

### Camera / yellow-block status
- Fixed recapture contact sheet framing matches the original accepted
  `hero1axebouncevfxbinding_20260529_080850` proof: hero visible from behind
  (range ring, center-bottom), enemies ahead, curved wall/stairs to the right.
- No pale yellow slab over the hero in any selected frame
  (START f3, MID f23, IMPACT f43, DISSIPATE f69). MID/IMPACT show the blue
  moving `NS_Hero1AxeBounce_MeshSlash` carrier traveling along the link.
- The only intended visual difference from 080850 is the staged target count:
  two expected-hit targets (Primary, ChainSecond) instead of the old
  multi-target cluster, which is exactly the requested two-link proof.

## Review / Validation

This is Operator evidence, not a greenlight. Codex should validate:
- The two edits stay within `T66PlayerController_Overlays.cpp` and do not change
  production Bounce semantics in `T66CombatComponent.cpp`.
- The isolation runs only under `bBounceVFXBindingProofMode`.
- The fresh MP4/log/contact sheet are the ones cited above.

## Token Ledger
AuthoringTokens: Claude=Unavailable (direct in-environment Operator session; no helper manifest emitted)
ReviewTokens: Codex=Unavailable (set by Codex at validation)
FinishTokens: Codex=Unavailable
OperatorIsAuthoring: YES
PerModel: Claude=Unavailable, Codex=Unavailable
TargetMet: Unavailable
Notes: Claude performed discovery, the source edit, compile, and capture/proof. No `Scripts\Invoke-ClaudeDirectRead.ps1` manifest exists for this run, so a numeric Claude token count is not exposed here.

## Caveats
- Out-of-scope dead-code note already tracked in
  `Source/T66/Gameplay/pending_issues_Gameplay.md`
  ("TrySpawnBoundWeaponBaseSlashVFX ImpactAnchored Bounce Branch Is Now
  Unreached"); not touched by this fix.
- The isolation pools/destroys non-proof mobs and one boss present in the proof
  level at fire time; this only affects the automation proof world, not
  production gameplay.
- The capture's editor log is `Saved/Logs/T66.log` (rotates on next editor
  launch); the durable proof excerpts are quoted above in this packet.
