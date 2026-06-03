# Claude Operator Report — Hero 1 Bounce Projectile Travel Fix (Revision)

**Operator:** Claude (T66 Operator)
**Validator/integrator:** Codex
**Approval:** `Reports/AgentReviews/Hero1BounceProjectileTravelFix/codex_operator_approval_revise.md` (APPROVE)
**Date:** 2026-05-29
**Status of this artifact:** Operator work artifact. **Not** a greenlight and **not** a final visual-fidelity approval.

## Blocker Being Fixed

Codex validation of the first pass found: the sequencing was correct (one moving link hero→primary, then one moving link primary→second), but the moving carrier's visible silhouette was the **generic temporary cube/profile** (`AT66HeroProjectile` configured with `FT66TemporaryProjectileSystem::ProfileHeroBounce()`), not the authored Hero 1 Bounce red/blue horizontal slash VFX. The Combat VFX process requires the primary silhouette to live in the Niagara/material/renderer asset, not be deferred to later polish.

## Blocker Status — Quick Answers

- **Moving carrier uses authored Bounce Niagara slash:** **YES.** Each moving link now renders `/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash` as its visible primary silhouette.
- **Temporary cube/profile is hidden/support-only or still primary:** **Hidden / support-only.** The `AT66HeroProjectile` is retained solely as a hidden visual-only mover/lifetime root; its `VisualMesh`/`AccentMesh` cube/profile meshes are hidden (`SetVisibility(false)` + `SetHiddenInGame(true)` via `FT66TemporaryProjectileSystem::HideMesh`) the moment the authored Niagara carrier is attached. The cube path now only runs as a development fallback when the binding cannot be resolved.
- **Minimum visible travel duration added:** **YES** — `0.32s` per link, sourced from the Bounce binding's `BasePlaybackSeconds`. Reason: a short ~150uu link at the raw `ProjectileSpeed=2400` crosses in ~0.06s (sub-frame), so the slash would not be readable in capture. The minimum clamps the **visual** mover speed (and the derived next-link launch offset) only; damage/contexts are resolved before staging and are unaffected.

## How The Carrier Was Changed

Damage, target selection, the Bounce chain, and the `PerChainLink` impact contexts remain fully authoritative and resolved up front in `PerformBounce`, before any visual is staged. Only the moving presentation changed.

1. **`AT66HeroProjectile::SetPrimaryCarrierNiagara(System, Color, VisualScale, PlaybackTimeDilation)`** (new): attaches the authored Niagara system to a dedicated `PrimaryCarrierVFXComponent` (created in the ctor, attached to the **root**, not the cube, so hiding the meshes can't hide it), hides the cube/profile meshes, sets color params + scale + translucent sort priority, applies a playback time dilation, and activates. Visual-only — no damage or collision authority is moved into the carrier.
2. **`PerformBounce`** now resolves the `Hero1Axe_Bounce_Base` / `AttackCategory=Bounce` production binding via the existing `ResolveCombatVFXBinding(...)` to obtain the bound `NS_Hero1AxeBounce_MeshSlash` system, its `VisualScaleMultiplier`, and `BasePlaybackSeconds`, then passes them into staging.
3. **`StageBounceProjectileChain`** now takes the carrier system, carrier visual scale, a minimum link travel time, and the carrier playback seconds. It launches link 0 immediately through `SpawnBounceChainLinkSequential`; each later link is triggered from the previous visual projectile's arrival callback (`AT66HeroProjectile::SetVisualArrivalCallback`), then deferred one tick before spawn. Chained links start with a 36uu clearance offset along the next segment to avoid spawning inside the previous enemy's collision. Each link clamps visual travel time to `max(distance/speed, MinLinkTravelSeconds)` and derives a slowed per-link visual speed from that, so exactly one authored slash is in flight per segment and the second projectile starts when the first one reaches the enemy.
4. **`SpawnBounceLinkProjectile`** now calls `SetPrimaryCarrierNiagara(...)` when a carrier system was resolved (the authored slash path), computing a playback time dilation = `clamp(BasePlaybackSeconds / TravelSeconds, 0.25, 2.0)` so the slash plays across the link's travel window. It falls back to the temporary `ConfigureTemporaryProjectileVisual(...)` cube **only** if no carrier system resolved (dev fallback, so bring-up is never invisible). The per-link log now includes `Carrier=<system path or TemporaryProfileFallback>`.

Codex post-validation correction: the detached timer path did not launch the second visible carrier during capture, so it was replaced with visual-arrival callback chaining. The next-link spawn is deferred one tick and the visual-only carrier uses deferred actor spawning with collision disabled before `FinishSpawning`; this lets link 1 originate near the hit target without being rejected by target collision. The callback still uses a weak `UT66CombatComponent` and weak `UNiagaraSystem` reference before spawning the next link.

## Two-Target Proof Harness

`T66PlayerController_Overlays.cpp` `hero1axebouncevfxbinding` proof mode is **unchanged** and already stages only `Primary` and `ChainSecond` as expected hits, with the remaining controls placed out of chain range so the capture cannot show a third projectile. `ChainThird` was **not** reintroduced. No edit to the overlays file was necessary.

## Files Changed

| File | Change |
|---|---|
| `Source/T66/Gameplay/T66HeroProjectile.h` | Declared `SetPrimaryCarrierNiagara(...)` and added the transient `PrimaryCarrierVFXComponent`. |
| `Source/T66/Gameplay/T66HeroProjectile.cpp` | Created `PrimaryCarrierVFXComponent` (attached to root, auto-activate off) in the ctor; implemented `SetPrimaryCarrierNiagara` (attach authored Niagara, hide cube/profile meshes, set color/scale/time-dilation, activate). |
| `Source/T66/Gameplay/T66CombatComponent.h` | Extended `StageBounceProjectileChain` / `SpawnBounceLinkProjectile` signatures with carrier system, carrier visual scale, min link travel, and carrier playback seconds. |
| `Source/T66/Gameplay/T66CombatComponent.cpp` | `PerformBounce` resolves the Bounce binding and passes the authored carrier + timing into staging; `StageBounceProjectileChain` applies the presentation-only minimum link travel and propagates the carrier; `SpawnBounceLinkProjectile` attaches the authored Niagara as the primary silhouette (cube only as dev fallback) and logs `Carrier=`. |
| `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md` | Added the revision note; rewrote the carrier decision, artifact parity gate, and mechanism manifest so the authored Niagara slash is the **required Primary** moving carrier (SAME/PRESENT, not DEFERRED) and documented the minimum link travel. |
| `Source/T66/Gameplay/pending_issues_Gameplay.md` | Updated the dead-`bImpactAnchoredCarrier`-branch entry to note the authored Niagara is now rendered via the moving carrier path (`SetPrimaryCarrierNiagara`), so the static branch remains unreached. |

## What Was NOT Changed

- `Content/Data/CombatVFXBindings.csv` and `DT_CombatVFXBindings.uasset` — the Bounce row is preserved unchanged; the revision reuses the existing bound system path, so no binding edit/DataTable regen was required.
- `Scripts/ValidateCombatVFXProductionBindings.py` — unchanged; see below.
- Damage authority, target selection, chain damage/falloff, and the `PerChainLink` impact-context publication — all preserved.
- `T66PlayerController_Overlays.cpp` — unchanged (harness already correct).
- No DOT/Pierce/AOE/idol/balance/stat changes; no Mini/minigame changes; no Git mutation.

## Verification — Commands Run

| Check | Command | Result |
|---|---|---|
| Focused compile | `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReloadFromIDE` | **PASS** — `Result: Succeeded`, ~66s total (58s in executor). Only warning is the pre-existing C4996 Niagara `IsReadyToRun` deprecation in `T66Hero1AxeAOEVFXLabActor.cpp` (unrelated to this change). |

## Verification — Not Run / Deferred

- **Combat VFX binding validator** — not required by this revision and not run. Rationale: no CSV/DataTable/asset edit, and every source-guard fragment it asserts in `T66CombatComponent.cpp` (`ResolveCombatVFXBinding`, `ShouldSuppressWeaponBaseProjectileVisual`, `TrySpawnBoundWeaponBaseSlashVFX`, `CombatVFXProductionSpawned`, `bPathAnchoredCarrier`, `PathAnchored`, etc.) and in the overlays file (`hero1axebouncevfxbinding`, `ET66AttackCategory::Bounce`) is still present (nothing was removed). The Bounce CSV-row assertions (`NiagaraSystem`, `bSuppressTemporaryProjectile=True`, `BasePlaybackSeconds=0.32`) are unaffected. Run it only if a later phase edits the CSV/DataTable/guarded symbols. Full command for Codex if desired:
  ```powershell
  & 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=pythonscript -script='C:\UE\T66\Scripts\ValidateCombatVFXProductionBindings.py' -unattended -nop4 -nosplash
  ```
- **Runtime capture / multi-frame video proof** — **NOT run by this Operator pass.** Final visual acceptance requires Unreal-owned captured evidence and Pablo approval, which is excluded from Operator self-approval.

## Proof Command Codex Should Run

Capture the two-target Bounce proof (one authored slash hero→primary, then one authored slash primary→second):

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -ExecCmds="t66.capture hero1axebouncevfxbinding" -unattended -nop4 -nosplash
```

Expected runtime evidence in the log:

- `CombatVFXBounceLinkProjectile LinkIndex=0 LinkCount=2 ... Carrier=/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash...` logged immediately, then a time-delayed `LinkIndex=1 ... Carrier=...NS_Hero1AxeBounce_MeshSlash`.
- Each link's `TravelSeconds >= 0.32` for short links (presentation-only minimum).
- Per-link `CombatImpactContext` logs with distinct `ChainIndex` (0 = primary, 1 = second).
- A frame range showing the authored **red/blue horizontal slash** (not a cube) travel hero→primary, followed by a separate, later authored slash travel primary→second.

Anti-lookalike: if the capture shows a moving blue cube, a static target slash, or three simultaneous projectiles, the gate is `PARTIAL`/fail.

## Caveats

1. **Visual fidelity not approved.** This pass proves the carrier-method correction and compiles; it does not claim `FULL`. The authored-slash travel must still be confirmed in Unreal-owned capture and approved by Pablo.
2. **Carrier orientation.** The moving slash inherits the projectile root's velocity-following rotation (`bRotationFollowsVelocity`), spawned facing `(End-Start)`. If the authored slash needs a different facing offset for the horizontal read, that is a small follow-up tunable on `SetPrimaryCarrierNiagara`.
3. **Dev fallback retained.** If the binding fails to resolve at runtime, staging falls back to the temporary cube mover so bring-up is never invisible; in the shipping path the binding resolves and the authored slash is the carrier.
4. **Dead `bImpactAnchoredCarrier` branch** in `TrySpawnBoundWeaponBaseSlashVFX` remains unreached on the Bounce path (tracked in `pending_issues_Gameplay.md`); left in place to keep the diff minimal and the validator string assertion satisfied.
