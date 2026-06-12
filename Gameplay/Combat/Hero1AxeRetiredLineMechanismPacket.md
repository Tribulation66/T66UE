# Hero 1 Axe RetiredLine Mechanism Packet

**Created:** 2026-05-24 (scaffold). **Activated:** 2026-05-29 for the first Hero 1 axe retired lane production VFX pass.
**Status:** Active structural implementation packet for the Hero 1 / Chad 1 retired lane weapon VFX as a forward vertical slash, authored under Codex Operator approval `Reports/AgentReviews/Hero1RetiredLineVerticalSlashVFX/codex_operator_approval.md`. This packet approves building and validating the reusable retired lane VFX binding/runtime structure and its Unreal-owned proof. It does not approve a final `FULL` visual-fidelity claim; final visual acceptance still requires captured evidence and Pablo approval where the effect requires it.
**Owner:** Gameplay/Combat isolated VFX lab.
**Parent plan:** `Gameplay/Combat/Hero1AxeVFXPlan.md`
**Process index:** `Gameplay/Combat/VFX_PROCESS_INDEX.md`

## 1. Working Goal

Build the Hero 1 axe retired lane weapon projectile VFX as a **forward vertical slash** that travels along the locked retired lane, using the same red/blue/white material/texture language as the accepted Hero 1 AOE slash family.

RetiredLine is distinct from AOE: AOE is a band-anchored frontal crescent; RetiredLine is a `PathAnchored` forward lane. The primary RetiredLine silhouette must be a vertical blade-plane carrier that travels or reveals forward along the RetiredLine direction, not the AOE radial crescent recolored, rotated, or pinned to the hero.

## 2. User Constraints

- RetiredLine reads as a slash, specifically a vertical slash forward, reusing the AOE red/blue/white colors and material/texture vocabulary.
- Reuse the existing Hero 1 AOE slash material/texture family where possible; do not invent a new material family for RetiredLine.
- Preserve all existing AOE behavior: the `Hero1Axe_AOE_Base` binding row, AOE production assets, and `BaseVisualRadius=411.4` enforcement.
- Damage authority remains combat logic in `PerformRetiredLine`. Niagara collision, render mesh geometry, and material opacity are never the damage authority.
- The primary silhouette must come from Niagara/material/renderer/emitter assets, not actor-side debug geometry or procedural C++ helper shapes.
- Use Unreal-owned capture paths for visual proof.
- Do not implement DOT, Bounce, idols, or unrelated weapons in this pass. Mini/minigame systems are out of scope.

## 3. Process Sources

- `AGENTS.md`: PPF, artifact parity, mechanism manifest, anti-lookalike, Niagara combat VFX process, Unreal-owned capture.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`: carrier archetype, mask/material manifest, parameter-sweep discriminator, anti-bake, editor isolation, gameplay capture.
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`: anchor taxonomy and footprint mapping. RetiredLine primary carrier is `PathAnchored`.
- `Gameplay/Combat/CombatVFXImpactContextContract.md`: weapon impact-context publication for effects that can drive downstream sources.
- `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`: CSV and DataTable move together; production `.uasset` paths require packet/validator review.
- `Gameplay/Combat/MASTER_COMBAT.md`: combat runtime spine; RetiredLine/AOE behavior; VFX is presentation.
- Worked reference: `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md` for the AOE material/asset family and the production-binding/proof pattern.

The accepted AOE material family is the source vocabulary: shared red/blue/white slash-layer materials (bright additive, body additive, dark translucent), UV-driven reveal/erosion/edge-band, and Niagara-driven dynamic material parameters. RetiredLine reuses this vocabulary with a vertical-blade carrier geometry instead of the AOE radial crescent mesh.

## 4. Carrier Decision

Primary archetype: `PathAnchored`, authored as a Niagara mesh-rendered **vertical blade-plane** carrier.

Reason:

- RetiredLine is a forward lane attack defined by `LineLength` and `TubeRadius` in the combat impact context, not a frontal cone. A path/lane carrier maps directly to that authoritative geometry.
- A vertical blade-plane mesh oriented along the RetiredLine forward direction gives the deliberate, inspectable silhouette the user asked for (a vertical slash forward), and it is unambiguously not the AOE crescent.
- The AOE radial-crescent mesh (`SM_Hero1AxeAOE_SlashArc`) is explicitly rejected as the RetiredLine silhouette. RetiredLine gets its own lane mesh.

Rejected for the RetiredLine primary carrier unless explicitly approved:

- The AOE crescent mesh recolored, rotated, or scaled.
- A static vertical mesh rigidly attached to the hero with no forward travel/reveal.
- Actor-created geometry, actor-arranged point components, or procedural C++ helper shapes as the readable silhouette.
- Generic spark/shockwave systems as the primary readable shape.

## 5. PPF Check

```text
PPF CHECK
Objective: Build the Hero 1 axe retired lane VFX as a forward vertical slash that travels/reveals along the retired lane, reuses the AOE red/blue/white material language, maps to LineLength/TubeRadius, and is visually distinct from the AOE crescent.
Proven process: the accepted Hero 1 AOE Niagara/material/mesh pipeline (Python material/texture authoring + C++ commandlet for mesh+Niagara + promote + datatable setup + validator), plus the PathAnchored visual/damage alignment and impact-context contracts.
My planned implementation: a RetiredLine-specific vertical blade-plane mesh and PathAnchored Niagara system reusing the AOE slash-layer materials/textures, a Hero1Axe_RetiredLine_Base production binding row, a PathAnchored runtime spawn branch driven by LineLength/TubeRadius/Forward, and Unreal-owned capture/log proof.
Same method class: YES for structural implementation and binding proof, because the carrier, materials, reveal, and binding are authored in Niagara/material/mesh assets and evidenced through logs and captures.
If NO, why: any fallback to a static sprite, actor-side geometry, or reuse of the AOE crescent silhouette is not the same method class.
User approval required before proceeding: NO for structural implementation (approved). YES later for final visual acceptance if a same-view imagegen target is required.
Verification evidence: setup/import logs, datatable refresh, validator pass, focused C++ compile, runtime CombatVFXProductionSpawned log with VisualAnchorModel=PathAnchored, retired lane damage proof, and Unreal-owned capture/evidence bundle.
```

## 6. Artifact Parity Gate

| Reference artifact/category | Role | Required | Planned artifact/path | Status | Evidence needed |
|---|---|---:|---|---|---|
| Vertical blade-plane lane mesh | Primary | Yes | `/Game/VFXLab/Hero1Axe/RetiredLine/SM_Hero1AxeRetiredLine_BladePlane` and production `/Game/VFX/Hero1/Axe/RetiredLine/SM_Hero1AxeRetiredLine_BladePlane` | Planned | Mesh renderer uses a vertical blade-plane lane mesh authored from project-owned source, not the AOE crescent. |
| PathAnchored Niagara system | Primary | Yes | `/Game/VFXLab/Hero1Axe/RetiredLine/NS_Hero1AxeRetiredLine_MeshSlash` and production `/Game/VFX/Hero1/Axe/RetiredLine/NS_Hero1AxeRetiredLine_MeshSlash` | Planned | Niagara mesh renderer + AOE-family materials; forward reveal/travel along the lane. |
| Shared AOE slash materials | Primary | Yes (reuse) | Existing `/Game/VFXLab/Hero1Axe/Shared` + `/Game/VFX/Hero1/Axe/AOE` slash materials/textures | Reused | Same red/blue/white material/texture vocabulary applied to the RetiredLine carrier. |
| Production binding row | Primary | Yes | `Content/Data/CombatVFXBindings.csv` row `Hero1Axe_RetiredLine_Base`, regenerated into `DT_CombatVFXBindings.uasset` | Planned | CSV row + DataTable refresh; AOE row preserved. |
| PathAnchored runtime spawn | Primary | Yes | `TrySpawnBoundWeaponBaseSlashVFX` PathAnchored branch + `PerformRetiredLine` spawn call | Planned | Runtime log `CombatVFXProductionSpawned` with `VisualAnchorModel=PathAnchored`. |
| Secondary impact/support particles | Secondary | No | Deferred | Deferred | Add only after the primary lane carrier reads correctly. |

## 7. Mechanism Manifest

Completion is `FULL` only when every required mechanism is `PRESENT` with evidence. Any missing required mechanism means the result is reported `PARTIAL`.

| Mechanism | Required | Planned implementation | Evidence needed |
|---|---:|---|---|
| Forward lane travel | Yes | Niagara/carrier and runtime spawn parameters use `Forward` and `LineLength`; the carrier travels or reveals forward along the locked retired lane. | Multi-frame capture showing the carrier moving/revealing forward along the lane, not appearing whole and static. |
| Vertical slash-plane silhouette | Yes | Authored vertical blade-plane mesh oriented along the RetiredLine forward direction, with vertical extent in Z and lane length along Forward. | Same-view and gameplay captures where RetiredLine reads as a vertical forward slash, visually distinct from the AOE radial crescent. |
| Hero 1 AOE material language reuse | Yes | Reuse the AOE red/blue/white slash-layer materials/textures on the RetiredLine carrier. | Asset/material inspection plus capture readability showing the shared red/blue/white vocabulary. |
| Visual/damage alignment | Yes | PathAnchored footprint maps to `LineLength` (lane length) and `TubeRadius` (lane half-width); damage remains the authoritative retired lane/capsule query. | Debug DamageVolume/log proof plus VFX path/width evidence; `CombatVFXProductionSpawned` logs lane geometry. |
| Material animation / reveal | No (deferred polish) | Reuse AOE dynamic-parameter reveal/erosion on the lane carrier. | Frame range showing internal motion/reveal if claimed; otherwise reported `DEFERRED`. |
| Secondary impact/support particles | No | Defer until the primary lane carrier is accepted. | Not required for the first RetiredLine mechanism close. |

## 8. Visual/Damage Alignment Block

Per `CombatVFXVisualDamageAlignmentContract.md`:

- Anchor model: `PathAnchored` (primary RetiredLine carrier).
- Authoritative damage center: the retired lane/capsule query in `PerformRetiredLine` (combat logic).
- Impact point: `FT66CombatImpactContext.ImpactPoint` (primary target/lane impact), published with `bImpactPointValid=true`.
- Damage shape type: line/tube (lane) along the RetiredLine forward direction.
- Damage extents: `LineLength` (lane length, from `AttackRange`) and `TubeRadius` (lane half-width).
- Visual anchor: the hero attack origin (`AttackOrigin`), with the lane extending forward along `Forward`.
- Visual pivot: the RetiredLine Niagara system pivot placed at the lane start.
- Visual footprint: lane length mapped to `LineLength`; lane visible half-width mapped to `TubeRadius`.
- Footprint mapping (alternate to the radius formula): non-uniform lane scale. Length axis scales from `LineLength`; width/height axes scale from `TubeRadius` and the authored blade-plane proportions. This carrier does not use the AOE `VisualScale = (EffectiveDamageRadius / BaseVisualRadius) * VisualScaleMultiplier` uniform-radius formula because RetiredLine is a lane, not a radial footprint; the binding row still carries a calibration `BaseVisualRadius` for compatibility, but the runtime PathAnchored branch maps length/width directly from the impact-context lane extents.
- Tolerance: visual lane endpoint within `50` units of the authoritative lane start/end; visible lane half-width within `~15%` of `TubeRadius` at gameplay camera distance; no carrier drift that makes RetiredLine read as AOE-centered.

## 9. Impact Context Contract Block

Per `CombatVFXImpactContextContract.md`:

```text
IMPACT CONTEXT CONTRACT
Role: WeaponPublisher
Weapon context publication policy: OnePrimary (one retired lane weapon impact context per RetiredLine activation)
Eligible context rule: a RetiredLine activation that resolves a primary target/lane with a valid impact point
Expected downstream context count: 0 (no idol/downstream consumer in this pass)
SourceType: WeaponBase
SourceID: Hero_1_black_retired-line
ParentSourceID rule: None for the weapon base context
Impact point rule: use the official RetiredLine impact point (primary target/lane impact), not the hero/visual origin
Damage source proof: retired lane damage applied through the combat target handle in PerformRetiredLine
Neutral control: N/A for this pass (no idol consumer); add when an idol consumes RetiredLine contexts
Diagnostic schema: CombatImpactContext fields per contract; downstream chain schema deferred until a consumer exists
Legacy fallback allowed: NO
Video-only proof accepted: NO
Intentional exceptions: none
```

`PerformRetiredLine` already builds and publishes the weapon impact context with `ImpactPoint`, `Forward`, `LineLength`, `TubeRadius`, and `bImpactPointValid=true`. This pass adds the bound production VFX spawn from that same official context; it does not move the context to a visual origin.

## 10. Anti-Lookalike Test

Cheap wrong result:

- The retired lane VFX is the AOE crescent recolored or rotated, or a static vertical mesh stuck to the hero with no forward motion.

Required discriminator:

- RetiredLine is `PathAnchored`, uses lane length and tube radius, and travels or reveals forward along the locked retired lane.
- RetiredLine is visually distinguishable from the AOE radial/band-anchored crescent in same-view and gameplay captures.
- If the result would still pass with forward travel/reveal removed, or if it is indistinguishable from the AOE crescent, the gate is `PARTIAL`.

Proof rule:

- One still image cannot pass the travel mechanism; the capture must include a frame range showing forward lane progression.
- Damage proof must show authoritative retired lane damage through the combat query, separate from the VFX.

## 11. Verification Commands

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=pythonscript -script='C:\UE\T66\Scripts\SetupHero1AxeRetiredLineLabVFX.py' -unattended -nop4 -nosplash

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=T66Hero1AxeRetiredLineVFX -unattended -nop4 -nosplash

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReloadFromIDE

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=pythonscript -script='C:\UE\T66\Scripts\SetupCombatVFXBindingsDataTable.py' -unattended -nop4 -nosplash

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=pythonscript -script='C:\UE\T66\Scripts\ValidateCombatVFXProductionBindings.py' -unattended -nop4 -nosplash
```

Implementation may adjust script names, but must keep equivalent setup, build, datatable refresh, validator, and Unreal-owned capture/log evidence.

## 12. Mechanism Close Template

```text
MECHANISM CLOSE
Mechanism: Forward lane travel
Status: PRESENT/ABSENT/DEFERRED
Evidence:

Mechanism: Vertical slash-plane silhouette
Status: PRESENT/ABSENT/DEFERRED
Evidence:

Mechanism: Hero 1 AOE material language reuse
Status: PRESENT/ABSENT/DEFERRED
Evidence:

Mechanism: Visual/damage alignment
Status: PRESENT/ABSENT/DEFERRED
Evidence:

Discriminator test:
Reported status: FULL/PARTIAL
```

## 13. Approval Gate

Structural implementation is approved by the Codex Operator approval artifact and Pablo's go-ahead. Final `FULL` visual-fidelity acceptance is deferred: it requires Unreal-owned captured evidence and Pablo approval of the visual target match. A structural implementation pass may be reported `PARTIAL` visually while the binding/runtime structure and proof are complete.
