Operator Packet: COMPLETE

# Hero 1 DOT Weapon — Operator Completion Packet

**Operator:** Claude (T66 Operator)
**Validator:** Codex
**Date:** 2026-05-29
**Status:** Operator work complete and submitted for Codex validation.

> **Revision (2026-05-29, marker-attachment defect fix):** Addressed the
> Validator finding that `T66DotMarkerVFX.cpp` used
> `SetActorLocation(...)` + `AttachToActor(... KeepRelativeTransform)`, which
> could preserve a world-sized relative offset after attachment and pull the
> markers off the target. Fixed to a single snap-attach
> (`SnapToTargetNotIncludingScale`) and added a `T66DotMarkerAlignment` proof
> log. Fresh log proof now shows `MarkerLoc == TargetLoc` and
> `OffsetSize=0.000` — markers sit on the enemy with a zero target-relative
> root offset. Single DOT payload semantics unchanged (still exactly one
> `ApplyDOT(HeroPrimaryDot)`). Re-compiled and re-captured; see updated
> evidence below.

> This is an Operator work artifact, **NOT a greenlight.** Final acceptance,
> visual-fidelity sign-off, production Niagara authoring, and Pablo approval are
> reserved for the Validator per the approved scope
> (`codex_operator_approval.md`).

> Packet status meaning: `COMPLETE` indicates this packet truthfully covers the
> implementation and the attempted verification. It does **not** assert that the
> pure-visual readability sub-gate passed `FULL`; that sub-gate is `PARTIAL` and
> is disclosed in "Skipped verification and why" below.

---

## Summary of changes

Implemented the Hero 1 DOT weapon placeholder structure per the approved scope:

- **One visible moving hero→target projectile before DOT application** — a
  visual-only `AT66HeroProjectile` spawned via `SpawnVisualTravelProjectile`
  (zero damage authority, no collision damage lane).
- **One existing data-authoritative DOT lane** — exactly one
  `ApplyDOT(HeroPrimaryDot)`; existing DOT tuning unchanged.
- **Three target-following visual sphere markers for the DOT duration** —
  `AT66DotMarkerVFX` spawns 3 markers attached to the target, lifespan = DOT
  duration, visual-only (NoCollision, no shadow, no damage code).
- **Proof hooks** — logs for projectile spawn/arrival, DOT apply, marker spawn,
  and DOT tick damage.
- **Capture** — registered `hero1axedotvfxbinding` mode in the Unreal-owned
  gameplay video process, including a single Primary target plus two hidden
  out-of-range control targets (disproves AOE) and a proof-readability block.
- **Process docs** — recorded the temporary placeholder status in the DOT
  mechanism packet and `MASTER_COMBAT.md`.

**Authority policy preserved:** contact damage, weapon impact-context publish,
frostbite, and SFX remain **synchronous**. Only `ApplyDOT` + marker spawn are
**deferred** to projectile arrival. Damage authority remains combat/log-owned;
the projectile and markers carry **zero** damage authority. Single DOT payload
guaranteed: exactly one `ApplyDOT`.

---

## Files touched

### Files added (new, untracked)
- `Source/T66/Gameplay/T66DotMarkerVFX.h` (1275 B) — placeholder marker actor.
- `Source/T66/Gameplay/T66DotMarkerVFX.cpp` — spawns N ring-laid sphere markers,
  snap-attaches to target, lifespan = DOT duration. Visual-only, NoCollision,
  no shadow. No damage code. **Revision:** attachment changed from
  `SetActorLocation` + `KeepRelativeTransform` to a single
  `AttachToActor(FollowTarget, FAttachmentTransformRules::SnapToTargetNotIncludingScale)`
  so the marker root sits on the target with a zero target-relative offset
  (ring offsets keep their authored size regardless of target scale).

### Files modified
| File | Nature of change |
|---|---|
| `Source/T66/Gameplay/T66CombatComponent.h` | Declarations: `SpawnVisualTravelProjectile`, `SpawnDOTApplicatorMarkers(... MarkerScale)` |
| `Source/T66/Gameplay/T66CombatComponent.cpp` | `#include T66DotMarkerVFX.h`; CVar `T66.DOT.ProofReadableTravelSeconds`; `SpawnVisualTravelProjectile`; `SpawnDOTApplicatorMarkers`; reworked `PerformDOT` (synchronous authority kept; deferred ApplyDOT+markers on arrival; weak-ptr lambda; immediate fallback if no projectile). **Revision:** added focused `T66DotMarkerAlignment` proof log in `SpawnDOTApplicatorMarkers` (target loc, marker loc, target-relative offset, offset size) so marker-on-target alignment is validatable from logs. |
| `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | DOT proof mode `hero1axedotvfxbinding`: single Primary target + 2 hidden out-of-range controls (disproves AOE); proof readability block (travel 0.60s, projectile 3x, markers 0.6, fixed-timestep 0.04 for 4.0s); category = DOT |
| `Scripts/CaptureT66GameplayVideo.ps1` | Registered `hero1axedotvfxbinding` capture mode (fire-delay args, DebugView, ImpactSourceVerbose) |
| `Gameplay/Combat/Hero1AxeDOTMechanismPacket.md` | Rewrote to "Temporary placeholder structure (runtime wired)" |
| `Gameplay/Combat/MASTER_COMBAT.md` | Added DOT placeholder structure bullet |

> Note: `git diff --stat` line counts for `T66CombatComponent.cpp` and
> `T66PlayerController_Overlays.cpp` are large because the working tree carries
> pre-existing uncommitted changes from prior sessions; the DOT-specific edits
> are the subset described above. No unrelated prior-agent/user changes were
> reverted.

---

## PPF close

```text
PPF CLOSE
Process used: AGENTS.md combat VFX process — Unreal-owned focused compile, Unreal-owned gameplay video capture, and combat query/log proof for damage authority, per CombatVFXDefinitionOfDone.md. Temporary placeholder structure (no final Niagara authoring this phase, per approved scope).
Matches declared process: YES
Evidence: compile_output3.log (Result: Succeeded, post-fix); CaptureT66GameplayVideo.ps1 -CaptureMode hero1axedotvfxbinding (re-run -FrameCount 90) produced hero1axedotvfxbinding_20260529_231442 bundle; Saved/Logs/T66.log proof lines (T66DotShotSpawned, T66DotApplicatorMarkersSpawned MarkerCount=3, T66DotMarkerAlignment OffsetSize=0.000, T66DotPayloadApplied Source=HeroPrimaryDot, CombatImpactContext AttackCategory=DOT). See "Exact verification commands and results".
```

---

## Mechanism close

```text
MECHANISM CLOSE
Mechanism: Visible hero→target projectile travel before DOT application
Status: PRESENT
Evidence: T66DotShotSpawned Start/End + ProofTravelSeconds=0.600 in T66.log; spawned via SpawnVisualTravelProjectile (visual-only).
Discriminator test: A static muzzle flash would have no Start/End travel and no arrival-gated ApplyDOT; log shows distinct Start≠End and ApplyDOT deferred to arrival.
Reported status: FULL

MECHANISM CLOSE
Mechanism: Single data-authoritative DOT lane (no multiplied independent lanes)
Status: PRESENT
Evidence: T66DotPayloadApplied count=1, Source=HeroPrimaryDot count=1, tuning unchanged (Duration=4.00 TickInterval=0.50 DamagePerTick=2.01).
Discriminator test: Three independent lanes (the cheapest wrong result) would log 3 ApplyDOT / 3 payloads or 3× damage; log shows exactly 1.
Reported status: FULL

MECHANISM CLOSE
Mechanism: Three target-following visual sphere markers ON the target for DOT duration
Status: PRESENT
Evidence: T66DotApplicatorMarkersSpawned MarkerCount=3 Duration=4.00 (visual-only); T66DotMarkerAlignment TargetLoc=(Y=3215.000 Z=1190.150) MarkerLoc=(Y=3215.000 Z=1190.150) TargetRelativeOffset=(0,0,0) OffsetSize=0.000 — marker root sits exactly on the target after snap-attach.
Discriminator test (attachment defect): the prior KeepRelativeTransform path would log a large nonzero OffsetSize (world-sized offset, markers off-target); the snap-attach fix logs OffsetSize=0.000. Markers carry no damage lane (NoCollision); payload count stays 1, so markers are applicators not damage sources.
Reported status: FULL

MECHANISM CLOSE
Mechanism: Pure-visual readability in captured video (one isolated projectile + three distinct markers)
Status: DEFERRED
Evidence: Capture bundle frames/contact sheet; markers read as an indistinct cluster and the 0.6s travel is sampled in ~2 frames (see "Skipped verification and why").
Discriminator test: Target-framed camera or wider ring / slower travel would isolate the three markers and the moving projectile; not produced this phase.
Reported status: PARTIAL
```

> Overall mechanism status: **PARTIAL** — all damage/structure mechanisms are
> `PRESENT` and authoritatively evidenced; the pure-visual readability sub-gate
> is `DEFERRED`. Per AGENTS.md, any `DEFERRED` required mechanism means the
> result is reported `PARTIAL`, not described as fully complete.

---

## Visual/damage alignment close

- The visual carriers (one projectile, three sphere markers) are **temporally
  and causally aligned** with the single damage event: the projectile travels
  first, and on arrival the one `ApplyDOT(HeroPrimaryDot)` fires together with
  the 3-marker spawn. Log ordering confirms: `T66DotShotSpawned` →
  `T66DotApplicatorMarkersSpawned MarkerCount=3` → `T66DotPayloadApplied ...
  Source=HeroPrimaryDot`.
- **No visual element holds damage authority.** Projectile `Damage=0`, no
  collision damage lane; markers NoCollision, no damage code. Damage remains
  combat/log-owned.
- **No multiplied damage.** Three visual markers map to exactly one DOT payload
  (counts: markers spawn event = 1 with MarkerCount=3; payloads = 1).

---

## Impact context close

- Synchronous weapon impact-context publish is **intact and unchanged**;
  deferral applies only to `ApplyDOT` + marker spawn at projectile arrival.
- Proof line from `Saved/Logs/T66.log`:

```
CombatImpactContext Phase=WeaponPrimary SourceType=WeaponBase SourceID=Hero_1_black_dot AttackCategory=DOT HitTargets=1 EffectiveDamage=28
```

- `AttackCategory=DOT` and `HitTargets=1` confirm the DOT category routing and a
  single impacted target (the Primary), consistent with the two hidden
  out-of-range controls remaining unaffected (disproves AOE).

---

## Exact verification commands and results

### Focused compile (post-fix) — PASS
Command (UnrealBuildTool):
```
dotnet "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReloadFromIDE
```
Result:
- `Reports/AgentReviews/Hero1DOTWeapon/compile_output3.log` — `Result: Succeeded`
  (24.49 s total; recompiled `Module.T66.26.cpp`/`Module.T66.27.cpp` covering the
  edited files, then linked `UnrealEditor-T66.dll`). Only warning is the
  **pre-existing** Niagara deprecation (C4996) in the unrelated
  `T66Hero1AxeAOEVFXLabActor.cpp(353)` — not introduced here.
- Prior pre-revision compiles `compile_output.log` / `compile_output2.log` also
  `Result: Succeeded` (retained for history).

### Unreal-owned gameplay video capture (post-fix) — PASS
Command (exact mode required by the task, re-run with a wider window so the
editor stays alive through the 7.6 s fire and the 4 s DOT marker phase):
```
Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode hero1axedotvfxbinding -FrameCount 90 -DelaySeconds 5 -CaptureIntervalSeconds 0.08
```
> **Capture-window note:** the bare `-CaptureMode hero1axedotvfxbinding`
> invocation (script default `FrameCount=36`) was attempted first
> (`capture_output3.log`, bundle `..._231316`). With the default 36-frame /
> ~2.9 s window starting at the 5 s delay, the screenshot sequence completes and
> the editor exits at ~7.9 s — right as the 7.6 s fire triggers — so the DOT
> never fired and **no** `T66Dot*` lines were logged for that run. The
> `-FrameCount 90` re-run (~7.2 s window, ends ~12.2 s) covers the fire and the
> full 4 s DOT duration and produced complete log proof below.

Result: produced `hero1axedotvfxbinding.mp4` (1280×720, 90 frames) plus
`frames/` bundle (`capture_output4.log`); ffmpeg encode succeeded.

### Combat query / log proof (post-fix) — PASS
Source: current `Saved/Logs/T66.log` (run 2026.05.30-02.15):
```
T66DotShotSpawned Target=T66EnemyBase_0 Start=V(Y=3575.00, Z=1266.15) End=V(Y=3215.00, Z=1254.15) ProofTravelSeconds=0.600
T66DotApplicatorMarkersSpawned Target=T66EnemyBase_0 MarkerCount=3 Duration=4.00 (visual-only; single DOT payload unchanged)
T66DotMarkerAlignment Target=T66EnemyBase_0 TargetLoc=X=-0.000 Y=3215.000 Z=1190.150 MarkerLoc=X=-0.000 Y=3215.000 Z=1190.150 TargetRelativeOffset=X=0.000 Y=0.000 Z=0.000 OffsetSize=0.000 (expect ~0; markers on target)
T66DotPayloadApplied Target=T66EnemyBase_0 Duration=4.00 TickInterval=0.50 DamagePerTick=2.01 Source=HeroPrimaryDot (single payload)
CombatImpactContext Phase=WeaponPrimary SourceType=WeaponBase SourceID=Hero_1_black_dot ... AttackCategory=DOT ... HitTargets=1 EffectiveDamage=28
```
**Marker-on-target proof (the Validator finding):** `MarkerLoc` equals
`TargetLoc` exactly and `TargetRelativeOffset`/`OffsetSize` are `0.000` after the
snap-attach — the world-sized-offset defect is gone.
Single-payload counts (whole log): `T66DotShotSpawned`=1,
`T66DotApplicatorMarkersSpawned`=1 (MarkerCount=3), `T66DotMarkerAlignment`=1,
`T66DotPayloadApplied`=1, `Source=HeroPrimaryDot`=1.

---

## Video/log/evidence paths

- Compile logs:
  - `Reports/AgentReviews/Hero1DOTWeapon/compile_output3.log` (**post-fix**,
    `Result: Succeeded`)
  - `Reports/AgentReviews/Hero1DOTWeapon/compile_output.log`,
    `compile_output2.log` (pre-revision history)
- Capture logs:
  - `Reports/AgentReviews/Hero1DOTWeapon/capture_output4.log` (**post-fix**,
    90-frame run with full DOT log proof)
  - `Reports/AgentReviews/Hero1DOTWeapon/capture_output3.log` (default 36-frame
    run; under-captured — DOT did not fire, see capture-window note)
  - `capture_output.log`, `capture_output2.log` (pre-revision history)
- Capture bundles:
  - `Saved/VideoCaptures/hero1axedotvfxbinding_20260529_231442/` (**current,
    post-fix**; `hero1axedotvfxbinding.mp4` + 90-frame `frames/`)
  - `Saved/VideoCaptures/hero1axedotvfxbinding_20260529_231316/` (default-window
    run; DOT did not fire)
  - earlier `..._225041/` and `..._225633/` bundles (pre-revision history)
- Gameplay log: `Saved/Logs/T66.log` (post-fix run 2026.05.30-02.15).
- Process docs: `Gameplay/Combat/Hero1AxeDOTMechanismPacket.md`,
  `Gameplay/Combat/MASTER_COMBAT.md`.

---

## Skipped verification and why (including visual readability limitations)

- **Marker-on-target alignment (the Validator finding): FIXED and proven.** The
  snap-attach now logs `MarkerLoc == TargetLoc` and `OffsetSize=0.000`, so the
  three markers sit on the enemy with a zero target-relative root offset for the
  DOT duration. This is authoritative structural proof and does not depend on
  camera framing.
- **Pure-visual "video shows one projectile + three distinct markers" sub-gate:
  still PARTIAL / DEFERRED (unchanged by this fix).** The proof camera is the
  hero-centered locked-chase preset, so the target sits small in the upper
  frame. In the captured frames the markers read as a small indistinct cluster
  (3 ~60-unit spheres on a 42-unit ring overlap at that screen size), and the
  fast 0.6 s travel is sampled in only ~2 screenshot frames so the moving
  projectile is not crisply isolated. The post-fix re-run did not pass
  `-EvidenceBundle`, so no contact sheet was generated for it; readability is
  judged the same as the prior bundles. **Why deferred:** a target-framed proof
  camera (or wider marker ring / slower proof travel) is the fix, and
  post-approval authored DOT Niagara is the production path — neither is part of
  this temporary placeholder phase. The marker *placement* is no longer in doubt
  (proven by the `OffsetSize=0.000` log); only on-screen *readability* at the
  current camera distance remains a follow-up.
- **Final Niagara / imagegen DOT art: skipped by scope.** The approved scope
  prohibits final Niagara art or imagegen assets in this phase, so none were
  produced.
- **Mechanism gate is not skipped and is authoritative.** Per the Combat VFX
  Definition of Done, hitbox/damage authority is proven by combat query/log
  proof, which is present and passing above.

**Validator decision requested:** accept the mechanism on the authoritative log
proof and treat the pure-visual readability sub-gate as a follow-up, or request
a re-capture with a target-framed camera before acceptance.

---

## Token usage / Claude token availability

Per-task token accounting is not exposed to the Operator in this environment, so
exact counts cannot be reported. Claude tokens remained available throughout this
revision; the work (read defect, edit two source files, compile, two captures,
log verification, packet update) completed without hitting any token/context
limit. No blocker was caused by token availability.

---

## Constraints honored

- DOT balance unchanged; existing single DOT payload preserved.
- Spheres are visual applicators only — no independent damage lanes.
- Bounce behavior untouched; no Mini/minigame changes.
- No final Niagara/imagegen DOT art this phase.
- No combat-authority replacement by visual/mesh/Niagara/projectile-collision.
- No git commit/push/tag/reset/clean; no broad LFS/binary-asset scan.
- No unrelated prior-agent/user changes reverted.
