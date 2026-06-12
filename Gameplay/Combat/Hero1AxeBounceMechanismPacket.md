# Hero 1 Axe Bounce Mechanism Packet

**Created:** scaffold (infrastructure-only). **Activated:** 2026-05-29 for the first Hero 1 / Chad 1 Bounce production VFX pass.
**Revised:** 2026-05-29 (Hero1BounceProjectileTravelFix) — corrected from a static `ImpactAnchored` per-link slash to a **moving two-link projectile/slash carrier sequence** per Pablo's correction. Authored under Codex Operator approval `Reports/AgentReviews/Hero1BounceProjectileTravelFix/codex_operator_approval.md`.
**Revised again:** 2026-05-29 (Hero1BounceProjectileTravelFix revision) — the moving carrier's **visible primary silhouette is now the authored Bounce Niagara slash** (`NS_Hero1AxeBounce_MeshSlash`), not the generic temporary cube/profile. The temporary `AT66HeroProjectile` is retained only as a hidden visual-only mover/lifetime root; its cube/profile meshes are hidden when the authored Niagara carrier is attached. A presentation-only minimum link travel time (from the binding's `BasePlaybackSeconds`) keeps a short link readable. Authored under Codex Operator approval `Reports/AgentReviews/Hero1BounceProjectileTravelFix/codex_operator_approval_revise.md`.
**Runtime proof correction:** 2026-05-29 — the second visible link now launches from the previous visual projectile's arrival callback, not from a detached timer. The next-link spawn is deferred one game tick after arrival and starts with a small 36uu clearance along the next segment so it can originate at the hit enemy without instantiating inside that enemy's collision. This matches Pablo's "when it hits the enemy, one projectile flies to another enemy" requirement and gives the proof a concrete LinkIndex=0 then LinkIndex=1 runtime discriminator.
**Proof-harness correction:** 2026-05-29 — Bounce proof captures now use a prewarmed authored Niagara carrier, hide off-path negative-control targets while keeping their HP/no-hit checks active, and VFX capture commands disable camera wall occlusion so the camera readability fade material cannot appear as a cream rectangle over the hero. A capture that shows the wall-occlusion rectangle or unrelated off-path controls in the projectile path is considered proof-contaminated, not Bounce evidence.
**Status:** Active implementation packet for the Hero 1 Bounce weapon VFX as a **moving projectile/slash carrier** that travels from the hero to the locked primary enemy, and then — only after that link arrives — travels from the primary enemy to a second chained enemy. Exactly one visible link is in flight at a time. The earlier `ImpactAnchored` static-slash-per-impact direction is superseded: static impact-only slashes and three simultaneous projectiles do **not** satisfy the requested behavior. This packet does **not** claim a final `FULL` visual-fidelity result; final visual acceptance still requires Unreal-owned captured evidence and Pablo approval.
**Owner:** Gameplay/Combat isolated VFX lab.
**Parent process:** `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
**Process index:** `Gameplay/Combat/VFX_PROCESS_INDEX.md`
**Related runtime doc:** `Gameplay/Combat/MASTER_COMBAT.md`

## 1. Working Goal

Build the Hero 1 Bounce weapon VFX as a **moving two-link projectile/slash carrier sequence** in the Hero 1 red/blue/white vocabulary:

1. One visible Bounce link travels from the hero attack origin to the locked primary enemy.
2. After that link reaches the primary enemy, exactly one visible Bounce link travels from the primary enemy to the next chained enemy (and so on per chain link).
3. Only one link is visibly in flight per segment — never a burst of three simultaneous projectiles, and never a static impact-only slash.

Bounce is distinct from AOE and RetiredLine: AOE is a band-anchored frontal crescent; RetiredLine is a `PathAnchored` forward lane. Bounce is a **chained, moving link sequence** — a carrier visibly travels hero->primary, then primary->next, one link at a time.

## 2. User Constraints

- Bounce reads as a **moving projectile/slash carrier** that travels between chain links, colored in the Hero 1 **red/blue/white** vocabulary.
- The hero shoots exactly **one** visible Bounce link toward the primary enemy; after it reaches the primary, exactly **one** link flies from the primary to a second enemy.
- Do **not** spawn three simultaneous projectiles, and do **not** accept a static impact-only slash placed at each hit point as the result.
- Preserve Bounce damage, target selection, chain damage, the production binding structure, and the red/blue/white Hero 1 slash vocabulary.
- Damage authority remains combat logic in `PerformBounce`. Niagara collision, render mesh geometry, projectile collision, and material opacity are never the damage authority; the moving carrier is visual-only.
- Use Unreal-owned capture paths for visual proof; the proof must show motion over time (frame range), not a single still.
- Bounce proof video must be captured from the declared standard angle without the camera wall-occlusion fade rectangle, visible off-path negative controls, unrelated post-proof mobs, or other foreground occluders hiding the projectile path.
- Out of scope this pass: DOT, RetiredLine, AOE redesign, idols, balance/stat retuning, Mini/minigame systems, unrelated weapons, Git mutation, broad Git/LFS scans.

## 3. Process Sources

- `AGENTS.md`: PPF, artifact parity, mechanism manifest, anti-lookalike, Niagara combat VFX process, Unreal-owned capture, and optional validator/direct-Claude routing.
- `OPTIONAL_VALIDATOR_PROTOCOL.md`: optional validator and direct-Claude boundaries when cross-model validation or Claude direct work is explicitly requested.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`: carrier archetype, mask/material manifest, parameter-sweep discriminator, anti-bake, editor isolation, gameplay capture. Bounce is typically `BeamHop` plus `RibbonTrail`/`SupportImpact`; the user-requested slash carrier is declared and justified here.
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`: production readiness needs packet, binding, visual/damage alignment, impact-context proof, gameplay capture, and Pablo visual approval.
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`: anchor taxonomy and footprint mapping. Bounce uses a moving link carrier travelling between consecutive chain endpoints (per the carrier decision below).
- `Gameplay/Combat/CombatVFXImpactContextContract.md`: weapon impact-context publication/chaining; video alone cannot prove context wiring.
- `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`: CSV and DataTable move together; production `.uasset` paths require packet/validator review.
- `Gameplay/Combat/MASTER_COMBAT.md`: combat runtime spine; Bounce chain behavior; VFX is presentation.
- Worked reference: `Gameplay/Combat/Hero1AxeRetiredLineMechanismPacket.md` (and its parent AOE packet) for the accepted production-binding/proof pattern and material family.

The accepted Hero 1 slash material family is the source vocabulary: shared red/blue/white slash-layer materials (bright additive, body additive, dark translucent), UV-driven reveal/erosion/edge-band, and Niagara-driven dynamic material parameters. Bounce reuses this vocabulary by tinting the moving carrier in the Hero 1 red/blue/white slash colors.

## 4. Carrier Decision

Primary archetype: **moving projectile/slash link carrier, staged one link at a time**. Each link is a visual-only moving carrier that travels from the link's start point to the link's authoritative impact point (hero->primary, then primary->next), launched sequentially so only one link is in flight per segment.

Runtime: `PerformBounce` resolves all damage, target selection, and per-link `PerChainLink` impact contexts up front (authoritative), then resolves the `Hero1Axe_Bounce_Base` / `AttackCategory=Bounce` production binding to obtain the authored Niagara system `/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash` and calls `StageBounceProjectileChain`. Link 0 launches immediately. Each later link is launched by the previous visual-only projectile's `SetVisualArrivalCallback` when it reaches its authoritative endpoint, then deferred one game tick so the next projectile is not spawned inside the previous projectile's destruction path. For chained links after link 0, the visual start is offset 36uu along the next segment to clear the previous hit target's collision while still reading as originating from that enemy. The carrier is a visual-only `AT66HeroProjectile` (zero damage) spawned through deferred actor spawn with collision disabled before `FinishSpawning`; its **visible primary silhouette is the authored Bounce Niagara slash**, attached via `AT66HeroProjectile::SetPrimaryCarrierNiagara`; the temporary cube/profile meshes are hidden so they are not the accepted carrier. The projectile actor acts only as the hidden mover/lifetime root, oriented along travel, and self-destructs on arrival. If the binding cannot be resolved, staging falls back to the temporary profile mover (development fallback) so bring-up is never invisible.

Readability: short Bounce links (~150uu) would cross the raw `ProjectileSpeed=2400` in well under a frame, so `StageBounceProjectileChain` applies a **presentation-only minimum link travel time** equal to the binding's `BasePlaybackSeconds` (0.32s). This slows only the visual mover's speed; the next link still waits for the previous mover to arrive. Damage and contexts are resolved before staging and are unaffected. The authored slash's playback is time-dilated so the effect plays across the link's travel window.

Reason:

- Pablo's correction is explicit: Bounce must read as a moving link sequence, not static impact slashes and not three simultaneous projectiles. A staged moving carrier maps the visual to the chain order over time.
- Damage stays authoritative in `PerformBounce` and is fully resolved before any visual is staged, so timing/lifetime of the visual cannot affect damage, target selection, or chain falloff.
- Sequencing by visual arrival callback guarantees exactly one visible link per segment and a delayed second link, which is the discriminator against the rejected lookalikes.

Superseded / rejected for the Bounce primary carrier unless explicitly approved:

- The previous `ImpactAnchored` static slash placed at each impact point simultaneously (now superseded by the moving sequence).
- The legacy `SpawnBounceVFX` / `TrySpawnHeroBounceVariantPixels` all-segments-at-once pixel/segment line drawn across the whole chain in one frame.
- Three simultaneous projectiles / all chain visuals spawned in the same frame.
- The AOE radial crescent mesh recolored, rotated, or scaled.
- Actor-created debug geometry, actor-arranged point components, or procedural C++ helper shapes as the readable carrier.

The authored Niagara slash (`NS_Hero1AxeBounce_MeshSlash`) is now the **moving carrier's primary silhouette**, no longer deferred. A separate connecting BeamHop/RibbonTrail between links remains optional later polish.

## 5. PPF Check

```text
PPF CHECK
Objective: Make Hero 1 Bounce read as a moving two-link projectile/slash carrier sequence — one visible link travels hero->primary, then (only after it arrives) one link travels primary->second — reusing the Hero 1 red/blue/white vocabulary, one link in flight at a time, preserving Bounce damage/target selection/chain damage and the production binding structure.
Proven process: the accepted Hero 1 combat VFX/projectile presentation pipeline plus CombatVFXAuthoringProcedure Bounce/BeamHop carrier guidance, the visual/damage alignment contract, and the PerChainLink impact-context contract. The moving carrier reuses the existing visual-only AT66HeroProjectile presentation path already used for the hero base attack projectile.
My planned implementation: PerformBounce resolves damage/targets/PerChainLink contexts up front (authoritative), then stages a moving carrier per chain link via StageBounceProjectileChain/SpawnBounceLinkProjectile; link 0 launches immediately and each later link launches from the previous visual projectile's arrival callback so exactly one link is in flight per segment. The static ImpactAnchored slash spawn is removed from the Bounce path. Per-link CombatVFXBounceLinkProjectile logs plus Unreal-owned multi-frame capture prove the sequence.
Same method class: YES — the primary visual is a real moving projectile/link carrier bound to the combat VFX/projectile presentation path, and damage authority is unchanged.
If NO, why: a static target slash, three simultaneous projectiles, actor-side debug geometry, or the AOE crescent silhouette would not be the same method class.
User approval required before proceeding: NO for this bounded corrective fix; Pablo's latest message is the correction and asks to return to projectile behavior. YES for final visual acceptance if Pablo requires a same-view imagegen/mockup match.
Verification evidence: focused C++ compile; runtime CombatVFXBounceLinkProjectile logs showing LinkIndex=0 then a delayed LinkIndex=1; per-link CombatImpactContext logs; Bounce damage proof through the combat query; and Unreal-owned multi-frame capture showing link 0 travel hero->primary then link 1 travel primary->second.
```

## 6. Artifact Parity Gate

| Reference artifact/category | Role | Required | Planned artifact/path | Status | Evidence needed |
|---|---|---:|---|---|---|
| Authored moving Bounce slash carrier | Primary | Yes | `NS_Hero1AxeBounce_MeshSlash` attached to a visual-only `AT66HeroProjectile` (zero damage) via `SetPrimaryCarrierNiagara`; cube/profile meshes hidden | Present (SAME) | Per-link `CombatVFXBounceLinkProjectile ... Carrier=/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash` logs and multi-frame capture showing the authored slash travelling between link endpoints. |
| Shared Hero 1 projectile/slash colors | Primary | Yes (reuse) | Hero 1 projectile color (`FT66TemporaryProjectileSystem::HeroProjectileColor`) + shared red/blue/white vocabulary | Reused | Same red/blue/white vocabulary tinting the Bounce carrier. |
| Sequential staging runtime | Primary | Yes | `StageBounceProjectileChain` / `SpawnBounceChainLinkSequential` / `SpawnBounceLinkProjectile` in `T66CombatComponent.cpp`, called from `PerformBounce`; later links launch from `AT66HeroProjectile::SetVisualArrivalCallback`, deferred one tick, with collision-safe 36uu chained-start clearance | Present | Logs showing `LinkIndex=0` launched immediately and `LinkIndex=1` launched after the prior visual projectile arrives. |
| Presentation-only minimum link travel | Primary | Yes | `StageBounceProjectileChain` clamps each link's visual travel to the binding `BasePlaybackSeconds` (0.32s); damage timing unchanged | Present | `CombatVFXBounceLinkProjectile ... TravelSeconds>=0.32` for short links. |
| Production binding row | Primary | Yes (preserved) | `Content/Data/CombatVFXBindings.csv` row `Hero1Axe_Bounce_Base` + `DT_CombatVFXBindings.uasset` | Preserved | Existing row/DataTable kept intact (no edit needed); AOE/retired-lane rows preserved. |
| PerChainLink impact contexts | Primary | Yes (preserved) | `PerformBounce` per-link `PerChainLink` impact contexts | Preserved | Per-link `CombatImpactContext` logs with distinct impact points and chain indices. |
| Connecting BeamHop/RibbonTrail support | Secondary | No | Deferred | Deferred | Add only after the moving carrier reads correctly. |

## 7. Mechanism Manifest

Completion is `FULL` only when every required mechanism is `PRESENT` with evidence. Any missing required mechanism means the result is reported `PARTIAL`.

| Mechanism | Required | Planned implementation | Evidence needed |
|---|---:|---|---|
| Authored Bounce Niagara slash as moving carrier | Yes | Resolve the Bounce binding and attach `NS_Hero1AxeBounce_MeshSlash` as the visible silhouette of each link's visual-only mover (`SetPrimaryCarrierNiagara`); hide the temporary cube/profile meshes. | `CombatVFXBounceLinkProjectile ... Carrier=...NS_Hero1AxeBounce_MeshSlash` logs and capture showing the authored red/blue slash (not a cube) travelling along each segment. |
| Moving link carrier per segment | Yes | Spawn a visual-only moving carrier that travels along each Bounce segment (hero->primary, then primary->next) via `SpawnBounceLinkProjectile`. | Multi-frame capture and runtime logs showing the carrier moving along the primary segment and a separate carrier moving along the second segment. |
| Sequential one-link-at-a-time staging | Yes | `StageBounceProjectileChain` launches link 0 immediately; each later link is triggered by the previous visual-only projectile's arrival callback, deferred one tick, so only one link is in flight per segment. | Logs showing `LinkIndex=0` then a time-delayed `LinkIndex=1`; capture showing the second link only after the first arrives. |
| Per-link official impact context | Yes | `PerformBounce` publishes one official weapon impact context per chain link (`PerChainLink`), unchanged by this fix. | Per-link `CombatImpactContext` logs with distinct impact points / chain indices for primary and second target. |
| Hero 1 red/blue color reuse | Yes | Tint the moving carrier with the Hero 1 projectile color / red/blue/white vocabulary. | Capture readability showing the shared red/blue vocabulary on the carrier. |
| Visual/damage alignment | Yes | The moving carrier is zero-damage and travels toward each authoritative impact point; chained links may start 36uu along the next segment to clear target collision, but the endpoint remains the official impact point. Damage remains the `PerformBounce` per-target query, fully resolved before staging. | `CombatVFXBounceLinkProjectile` logs + capture showing the carrier path toward each hit point, separate from damage application. |
| Connecting hop/trail support | No | Defer BeamHop/RibbonTrail link-connection until the moving carrier is accepted. | Not required for the first Bounce mechanism close. |

## 8. Visual/Damage Alignment Block

Per `CombatVFXVisualDamageAlignmentContract.md`:

- Anchor model: moving link carrier (a visual-only projectile that travels between consecutive chain endpoints, one segment at a time).
- Authoritative damage center: the per-target damage application in `PerformBounce` (combat logic) for each chain hit, fully resolved before any carrier is staged.
- Impact point: each link's authoritative hit location — the primary target aim point, then each subsequent chained target aim point. The runtime records these in the Bounce chain (`ChainPositions`); the moving carrier's destination is set to each segment's endpoint via `SetTargetLocation`.
- Damage shape type: per-target point/instant hit per chain link (not a continuous area or lane).
- Damage extents: governed by the Bounce chain query (`AttackRange` gates next-link search radius via `BounceRangeSq`); the moving carrier is a small fixed-scale visual, not a footprint that scales with an area radius.
- Visual anchor: each segment endpoint pair (start -> authoritative impact point); the carrier moves from start to end and self-destructs on arrival.
- Visual pivot: the moving carrier's own transform, travelling along the segment.
- Visual footprint: compact moving carrier; scale is authored/calibrated (`ProjectileScaleMultiplier`), not derived from an area radius.
- Footprint mapping: fixed authored carrier scale (single `ScaleMultiplier`); this carrier does not use the AOE radial `VisualScale = (EffectiveDamageRadius / BaseVisualRadius) * VisualScaleMultiplier` formula because Bounce links are point-to-point travel, not radial footprints.
- Intentional mismatch: LIMITED. For chained links after link 0 only, the visual start is offset 36uu along the next segment to avoid spawning inside the previous target's collision; the visual endpoint remains the authoritative impact point.
- Tolerance: each carrier is aimed at its authoritative chain impact point; segment 0 starts at the hero attack origin and segment N starts at the prior impact point plus the 36uu collision-clearance offset. Only one carrier in flight per segment.
- Proof route: per-link `CombatVFXBounceLinkProjectile` logs (LinkIndex/LinkCount), per-link `CombatImpactContext` logs, Bounce damage proof through the combat query, and multi-frame Unreal-owned capture showing link 0 travel hero->primary then link 1 travel primary->second.

## 9. Impact Context Contract Block

Per `CombatVFXImpactContextContract.md`:

```text
IMPACT CONTEXT CONTRACT
Role: WeaponPublisher
Weapon context publication policy: PerChainLink (one official Bounce weapon impact context per resolved chain link)
Eligible context rule: each Bounce chain link that resolves a target with a valid impact point (primary link, then each subsequent chained link)
Expected downstream context count: 0 (no idol/downstream consumer in this pass; future idol/chaining systems consume the per-link contexts)
SourceType: WeaponBase
SourceID: Hero_1_black_bounce
ParentSourceID rule: None for the weapon base context
Impact point rule: use each link's official impact point (the chained target hit location), not the hero/visual origin or one aggregated point
Damage/status source proof: Bounce damage applied per link through the combat target handle in PerformBounce
Neutral control: N/A for this pass (no idol consumer); add when an idol consumes Bounce contexts
Diagnostic schema: CombatImpactContext fields per contract, including a per-link chain index distinguishing primary from second target; downstream chain schema deferred until a consumer exists
Legacy fallback allowed: NO
Video-only proof accepted: NO
Intentional exceptions: none
```

Current runtime state: `PerformBounce` already implements the `PerChainLink` policy. The `PublishBounceLink` lambda publishes one official `FT66CombatImpactContext` per resolved chain link with a distinct `ChainIndex` (0 for the primary link, then 1, 2, ... for each subsequent chained target) and each link's own impact point, via `PublishWeaponImpactContext`. This fix preserves that per-link context publication unchanged — it only removed the static `ImpactAnchored` slash spawn from the per-link path and added the moving carrier staging. No aggregated single-context fallback remains on the Bounce path.

## 10. Anti-Lookalike Test

Cheap wrong result:

- Reuse the legacy `SpawnBounceVFX` / `TrySpawnHeroBounceVariantPixels` pixel/segment line drawn across the whole chain in one frame, recolor the AOE crescent, spawn static impact-only slashes at each hit point, or spawn three simultaneous projectiles in the same frame.

Required discriminator:

- Bounce is a moving carrier that visibly travels hero->primary, then (only after that link arrives) primary->second, with exactly one link in flight per segment, publishes a `PerChainLink` weapon impact context per link, and shows link 0 and link 1 as distinct, time-separated travel events.
- Bounce is visually distinguishable from the AOE radial/band crescent and the RetiredLine forward lane in same-view and gameplay captures.
- If the result shows all links at once, shows a static non-moving slash, or is indistinguishable from the AOE crescent or retired lane, or relies on the legacy temporary-projectile line, the gate is `PARTIAL`.

Proof rule:

- One still image cannot pass the chain mechanism; the capture must include a frame range showing the carrier travel along the primary segment and a separate, later carrier travel along the second segment.
- Damage proof must show authoritative Bounce damage applied per link through the combat query, separate from the VFX.
- Context proof must show per-link `CombatImpactContext` logs, not a single aggregated context; staging proof must show `CombatVFXBounceLinkProjectile LinkIndex=0` then a time-delayed `LinkIndex=1`.

## 11. Verification Plan

This corrective pass (Hero1BounceProjectileTravelFix) edits runtime code to stage the moving two-link carrier and updates this packet. Verification for the code change:

- Focused compile: `T66Editor Win64 Development`.
- Runtime proof: `CombatVFXBounceLinkProjectile LinkIndex=0` launched immediately, then a time-delayed `LinkIndex=1`; per-link `CombatImpactContext` logs; Bounce damage proof through the combat query.
- Unreal-owned multi-frame capture showing link 0 travel hero->primary then link 1 travel primary->second.
- Visibility hygiene: proof capture commands include `T66.Camera.WallOcclusionEnabled 0` so the camera wall-occlusion fade material cannot be mistaken for the Bounce carrier.

Representative commands:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReloadFromIDE
```

The binding row and DataTable are unchanged by this fix, so the bindings setup/validator commands are not required for this pass; run the validator only if a later phase edits the CSV/DataTable. Final visual acceptance still requires Unreal-owned captured evidence and Pablo approval.

## 12. Mechanism Close Template

```text
MECHANISM CLOSE
Mechanism: Moving link carrier per segment
Status: PRESENT/ABSENT/DEFERRED
Evidence:

Mechanism: Sequential one-link-at-a-time staging
Status: PRESENT/ABSENT/DEFERRED
Evidence:

Mechanism: Per-link official impact context
Status: PRESENT/ABSENT/DEFERRED
Evidence:

Mechanism: Hero 1 red/blue color reuse
Status: PRESENT/ABSENT/DEFERRED
Evidence:

Mechanism: Visual/damage alignment
Status: PRESENT/ABSENT/DEFERRED
Evidence:

Discriminator test:
Reported status: FULL/PARTIAL
```

## 13. Approval Gate

This corrective pass is approved by Codex Operator approval `Reports/AgentReviews/Hero1BounceProjectileTravelFix/codex_operator_approval.md` and Pablo's correction to return Bounce to moving-projectile behavior. This packet locks the corrected direction (moving two-link carrier, one link in flight per segment, sequential staging by visual arrival callback, `PerChainLink` contexts preserved) and the proof contract. The corrective pass edits runtime code (`T66CombatComponent.cpp/.h` and the visual-only arrival callback in `T66HeroProjectile`) to stage the moving carrier, defer the next-link spawn by one tick, disable visual-only carrier collision, and use a 36uu chained-start clearance offset; it removes the static slash spawn from the Bounce path. It does **not** edit CSV/DataTable, scripts, commandlets, or assets, and it does **not** claim a final `FULL` visual-fidelity result.

Final `FULL` visual-fidelity acceptance is deferred: it requires Unreal-owned captured evidence and Pablo approval of the visual target match. The imagegen/same-view mockup gate is invoked only if Pablo asks for it. This pass may be reported `PARTIAL` visually while the runtime staging and proof logs are complete.
