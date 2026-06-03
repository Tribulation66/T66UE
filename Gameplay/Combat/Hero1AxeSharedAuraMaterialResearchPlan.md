# Hero 1 Axe Shared Aura Material Research Plan

**Created:** 2026-05-27
**Status:** Research and process plan only. This document does not approve final material values, live combat integration, or production promotion.
**Owner:** Gameplay/Combat isolated VFX lab.
**Parent plan:** `Gameplay/Combat/Hero1AxeVFXPlan.md`
**Generic procedure:** `Gameplay/Combat/CombatVFXAuthoringProcedure.md`

## 1. Goal

Define the shared base aura-slash material language for Hero 1 axe base attacks before polishing AOE, DOT, Pierce, or Bounce independently.

The material language should make the base attacks feel like one weapon family even when the carrier shape, colors, timing, hitbox, and support particles differ per attack.

## 2. Hard Rules

- Source evidence owns material mechanisms. Imagegen targets own visual direction only.
- Do not use local transcript, caption, or video-source extraction workflows. If a needed video source lacks an already available Pablo-provided transcript, ask Pablo for the transcript.
- Do not copy tutorial textures, paid-pack textures, paid-pack meshes, exact hidden values, or marketplace content unless the license explicitly permits it and the packet records the license.
- Values copied from source analysis must be labeled `observed`, `inferred`, or `tuned`.
- The material family is not accepted until it passes both editor-isolation visual comparison and gameplay capture review where applicable.

## 3. Provisional Source Candidates

These are candidates for source-fidelity review, not final selected processes.

| Source | Current use | Why it is relevant | Next action |
|---|---|---|---|
| `https://cghow.com/sword-slash-in-ue5-niagara-tutorial/` | Candidate material-mechanism source | The written page describes a disk/mesh slash material using aura texture, dynamic panning, sine shaping, noise detail, unlit/two-sided rendering, Niagara mesh renderer, size/lifetime, and dissolve. | Verify the written steps against any Pablo-provided transcript or screenshots before treating it as a selected process. |
| `https://cghow.com/slash-fx-in-ue5-niagara-tutorial/` | Candidate mask/layer source | The written page describes masks and powers, subtract/multiply/contrast shaping, three-color blend, dynamic fade/sharpness, noise distortion/tiling, static mesh use, curves, and additional layers. | Source-fidelity review before selection. Ask Pablo for the video transcript if the written page is not specific enough. |
| `https://www.gabrielaguiarprod.com/product-page/sword-slashes-vol-1-niagara` | Production-composition evidence, not a tutorial process | The page lists a production slash pack with many Niagara slashes, textures, materials, and meshes, which supports the expectation that production slashes are material/texture/mesh heavy. | Use only as evidence of asset/component depth unless the user buys or provides inspectable assets with license notes. |

## 4. Source-Fidelity Checklist

Before the shared material recipe is locked, create or update a source matrix with:

- source URL or Pablo-provided transcript/source path,
- whether the source is written, transcript, image, inspectable asset, or paid asset,
- material graph steps,
- texture/mask roles,
- Niagara driver parameters,
- layer stack behavior,
- observed/inferred/tuned values,
- missing source details,
- license/adaptation constraints,
- Claude review result and Codex reconciliation.

Reject a source as primary material evidence if it only shows a final look, marketplace marketing, or high-level overview without mechanism detail.

## 5. Shared Material Roles

The first accepted Hero 1 axe aura material packet should define these roles:

| Role | Purpose | Shared or variable |
|---|---|---|
| `SilhouetteMask` | Defines the readable base body of the slash/aura. | Variable by attack shape. |
| `SoftEdgeGradient` | Prevents hard-stencil edges and keeps the material ethereal. | Shared behavior. |
| `RevealMask` | Supports spatial draw-on where the carrier requires point A to point B reveal. | Shared behavior, variable direction. |
| `EdgeBandMask` | Creates hot rim, core separation, or leading edge. | Shared behavior, variable color/intensity. |
| `DirectionalStreaks` | Creates energy grain along the travel direction. | Shared behavior, variable orientation/scale. |
| `InternalNoise` | Adds live aura motion and depth without replacing the silhouette. | Shared behavior. |
| `ErosionDissolve` | Breaks up the slash at end of life instead of uniform fading. | Shared behavior, variable timing. |
| `BackingContrast` | Adds dark/translucent contrast when additive layers wash out. | Optional shared method. |

## 6. Parameter Contract

Use the generic names from `CombatVFXAuthoringProcedure.md` unless a selected source or Unreal implementation requires a more specific name:

- `Reveal`
- `Power`
- `Erosion`
- `TilingX`
- `TilingY`
- `SpeedX`
- `SpeedY`
- `EdgeWidth`
- `Opacity`
- `Intensity`
- `PrimaryColor`
- `SecondaryColor`
- `ImpactColor`

The packet must record which parameters are driven by Niagara, which are material-instance constants, and which are tuned per effect.

## 7. Shared And Per-Effect Variation

Shared across Hero 1 axe base attacks:

- aura-like soft body and edge behavior,
- directional streak/noise vocabulary,
- erosion/dissolve vocabulary,
- layer-stack order: body, highlight/edge, backing, internal detail, support/impact,
- material parameter semantics,
- no checkerboard, tiled grid, hard flat ring, or unrelated generic noise reading.

Variable per attack:

- carrier archetype and shape mask,
- red/blue/white or other approved palette,
- timing curves and active hit window,
- impact/support emitter placement,
- effect scale and hitbox alignment,
- whether the attack uses mesh arc, ribbon, beam/path, or persistent aura carrier.

## 8. Editor-Isolation Material Gate

After the durable editor-isolation capture route exists, the shared material review must include a VFX-only black-background target and actual capture for each major carrier family being accepted.

For AOE, the first same-view gate should prove:

- full half-moon slash fits with 10-15 percent margin,
- red and blue bands follow the same crescent direction rather than forming a circle,
- red reads as the intended outer or leading band if the target says so,
- blue reads as inner/body or backing band if the target says so,
- white impact is attached to the contact side of the slash,
- material reads as aura energy with soft edges and streak/noise motion, not checker/tiling.

Artifact folder:

```text
Saved/VFXResearch/Hero1Axe/SharedAuraMaterial/EditorIsolation/<timestamp>/
```

## 9. Acceptance Conditions

The shared material language is `PARTIAL` until all required rows below are present:

| Gate | Required evidence |
|---|---|
| Source fidelity | Source matrix proves the chosen material process has concrete steps and labels observed/inferred/tuned values. |
| Material artifacts | Lab-owned material or material-function assets exist under `/Game/VFXLab/Hero1Axe/Shared` or another approved lab path. |
| Mask roles | Required mask/material roles are present or explicitly deferred with user approval. |
| Parameter sweep | `Reveal`, `Erosion`, `SpeedX/SpeedY`, `EdgeWidth`, `Opacity`, and `Intensity` sweeps visibly affect the intended behavior. |
| Editor-isolation comparison | Same-view target and actual capture are saved with mismatch notes. |
| Gameplay readability | Gameplay capture proves enemies, damage feedback, and hitbox read are not hidden by the material. |
| Claude review | Review artifact greenlights the source selection and material close, or objections are reconciled with evidence. |

Do not call the material family production-ready until Pablo approves captured evidence.
