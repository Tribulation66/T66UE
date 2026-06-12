# Combat VFX Authoring Procedure

**Created:** 2026-05-24
**Scope:** Generic authoring procedure for T66 combat VFX built from Pablo-provided source evidence, Niagara/material workflow, and capture-based validation.
**Status:** Procedure only. This document does not approve new Unreal assets, live combat integration, or production promotion.

## 1. Ownership And Non-Drift Rule

This document owns the generic VFX authoring sequence and the mask/material discriminator schema.

For combat VFX, this document is the generic implementation layer for the `AGENTS.md` Section 2 research-first replication rule. It defines the reusable effect-building process; per-effect plans and packets name the concrete sources, paths, values, and acceptance evidence.

Authoritative layers:

- `AGENTS.md` owns global PPF, artifact parity, mechanism manifest, anti-lookalike, Claude review, and Unreal-owned capture rules.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md` owns the generic carrier, mask, material, Niagara layer, and validation sequence.
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md` owns the required visual-to-damage anchor, footprint, offset, tolerance, and marker-vs-area-read contract.
- Per-effect plans and packets, such as `Gameplay/Combat/Hero1AxeVFXPlan.md` and `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`, own effect-specific paths, manifests, checkpoints, numeric starting values, tuned values, and acceptance results.

Do not copy per-effect numeric values into this document as new normative requirements. Reference the owning packet or research note instead. If a future effect needs different values, update the per-effect packet, not this generic procedure.

## 2. Core Principle

A professional combat effect is not produced by using Niagara, masks, or textures as isolated nouns. It comes from an ordered chain of live behaviors:

1. readable primary carrier shape,
2. project-owned or licensed authored masks/textures,
3. material graph behavior that turns those masks into reveal, edge weighting, motion, erosion, color depth, and opacity,
4. Niagara timing that drives renderer motion and material parameters,
5. layered emitters with deliberately different timing, scale, color, render order, and dissipation,
6. support particles that reinforce the primary motion after the primary carrier already reads,
7. Unreal-owned capture and frame-range review.

If a pass skips or substitutes a load-bearing behavior, report it as `PARTIAL`. A correctly named mask, emitter, or parameter does not prove the behavior exists.

## 3. Required Source Analysis

Before authoring an effect, create or update the effect packet with:

- primary reference source or source category,
- Pablo-provided transcript/source evidence for the relevant material, mesh, module, and curve timestamps,
- `observed`, `inferred`, and `tuned` labels for values,
- primary carrier archetype,
- artifact parity gate,
- mechanism manifest,
- mask/material manifest,
- cheap wrong result and anti-lookalike discriminator,
- visual/damage alignment block from `CombatVFXVisualDamageAlignmentContract.md`,
- impact-context contract block from `CombatVFXImpactContextContract.md` when the effect publishes, consumes, or chains combat impact contexts,
- Unreal-owned capture plan.

Tutorial and marketplace assets are technique references unless their license explicitly permits adaptation and shipping.

### 3.1 Source Evidence Intake

For YouTube or video references, do not run local transcript, caption, or video-source extraction workflows. Use only:

- a transcript/source path Pablo explicitly provides in the request or thread,
- a Pablo-provided transcript already stored in the effect packet's named transcript folder,
- non-video written source material that is already available through normal project or web research.

If the needed video transcript is not already available through one of those paths, ask Pablo for the transcript. For Hero 1 axe research, the current Pablo-provided transcript folder is `Saved/VFXResearch/Hero1Axe/notegpt_transcripts/`.

Generated local extraction-output folders are historical artifacts only. They are not current source truth and must not be used as the process to obtain new transcripts. For the Hero 1 axe pass, those artifacts were quarantined under `Saved/VFXResearch/Hero1Axe/_historical_extraction_outputs/20260525_transcript_extraction_artifacts/`.

Source evidence packets sent to Claude should include Codex's first-pass opinion, the source excerpts or file paths being used, uncertainty labels, targeted questions, and the exact verification gates. Claude output is advisory; Codex remains responsible for reconciliation and implementation decisions.

### 3.2 Visual Target Mockup Gate

Use this gate when Pablo asks for an imagegen visual target, when the effect is hard to specify in words, or when an effect packet says final visual acceptance must compare against an approved mockup.

Authority split:

- source evidence and the effect packet own the method, mechanisms, artifacts, masks, materials, timing, and verification gates,
- the approved mockup owns visual direction only: composition, color balance, silhouette intent, layer readability, impact placement, and style target,
- a mockup is not a replication source under Section 3.1 and is not proof that the Niagara/material implementation exists,
- the tracked per-effect packet is source truth; files under `Saved/VFXResearch` are cache/evidence unless the packet explicitly points to them as the approved visual target artifact.

For repo-bound VFX mockups, use a separate local Codex CLI worker when practical so prompt drafting, artifact handling, and variant comparison do not clutter the main chat. The worker must use the approved account-backed imagegen path. Do not use, revive, or fall back to `OPENAI_API_KEY` API scripts for this workflow.

Before a generated mockup becomes binding, the packet must record:

- the exact saved image or contact-sheet path,
- Pablo's explicit approval of that saved variant,
- whether the image is a start, mid, impact/full, dissipate, or full temporal strip target,
- any waived details or intentional differences from the source tutorial,
- a feasibility note for how the visual target maps onto Niagara/material components.

Add these effect-specific sections to the owning packet when mockup gating is active:

```text
VISUAL TARGET BRIEF
Intent:
Temporal panels:
Required colors/shapes:
Forbidden readings:
Approval artifact:

VISUAL DECOMPOSITION
Primary silhouette:
Color layers:
Impact/readability elements:
Dissipation elements:

NIAGARA MAPPING
Visual component:
Niagara/material carrier:
Driver:
Evidence needed:

ACCEPTANCE RUBRIC
Required still-frame match:
Required temporal match:
Required gameplay readability:
Explicit non-goals:
Process stop:
```

These sections are additive only. They must reference, not replace, the existing PPF check, artifact parity gate, mechanism manifest, mask/material manifest, parameter-sweep evidence, anti-lookalike discriminator, and Unreal-owned capture gates. If the visual target conflicts with a required mechanism, stop and resolve the conflict instead of weakening the mechanism.

### 3.3 Same-View Editor Isolation Gate

Use this gate when an effect needs visual-target comparison without the player, enemies, dungeon, gameplay camera, or hit feedback hiding the pure VFX structure.

This gate is additive. It does not replace gameplay capture, temporal frame-range proof, hitbox proof, damage evidence, or Pablo approval. It exists to make shape, material, layer, and impact-placement mismatch visible before a user-facing handoff.

Do not make an effect-specific editor-isolation gate mandatory until the capture route is repeatable for that effect family. A throwaway editor tweak, desktop screenshot, private hook, or manual viewport state is not a durable capture route unless the effect packet records how to reproduce it and a saved artifact proves it works.

The canonical editor-isolation capture spec for a slash or aura review must record:

- capture mechanism and command or manual reproduction steps,
- target Niagara system or preview asset,
- camera/view mode, such as top-down orthographic and unlit,
- plain black or otherwise declared neutral background,
- zoom/framing rule with the full effect visible and no cropped active region,
- diagnostic timing or preview-time controls, if used,
- output folder and artifact paths,
- whether the capture is a diagnostic gate, acceptance gate, or both.

For same-view imagegen comparison, lock the view before generating the target. The approved generated target must use the same declared framing as the real editor-isolation capture: same approximate camera direction, background type, crop, and full-effect margin. If the real capture framing changes, regenerate or explicitly re-approve the target instead of comparing against a stale view.

Required artifact convention:

```text
Saved/VFXResearch/<EffectName>/EditorIsolation/<timestamp>/
  manifest.json
  target.png
  actual.png
  actual_crop.png
  contact_sheet.png
  mismatch_notes.md
```

Current reusable route for Hero 1 axe slash/aura isolation:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66NiagaraMRQIsolation.ps1 `
  -OutputDir C:\UE\T66\Saved\VFXResearch\Hero1Axe\AOE_AmericanFlagVisualTarget\EditorIsolation\<timestamp> `
  -ResX 1400 -ResY 1400 `
  -OrthoWidth 1250
```

This script uses Movie Render Queue command-line rendering of a generated `/Game/VFXLab/Temp/MRQ` map, sequence, and config. The generated MRQ assets are regenerable and cleaned up by default; do not treat them as production content or commit them unless a later reviewed process explicitly changes that policy. The route renders the actual lab Niagara actor/system through a square top-down orthographic camera, then writes `actual.png`, `actual_crop.png`, `contact_sheet.png`, `mismatch_notes.md`, and `manifest.json`. `actual.png` is normalized as an opaque black review image by preserving rendered RGB over black; this avoids transparent-alpha PNGs displaying as white in some viewers.

The default Hero 1 AOE proof for this route is `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/EditorIsolation/20260527_MRQSquareRouteProbe/`. Its manifest passed square framing, non-black VFX pixels, frame-margin, and Niagara particle-log evidence. This proves the capture route, not final visual fidelity.

Coordinate note for the current top-down MRQ route: with the camera at `(0, 0, +Z)` rotated straight down, world `+X` renders toward the top of `actual.png`, and world `+Y` renders toward screen-right. For a north-facing same-view slash, place the impact/support point along world `+X` and use a slash profile that spans the visual north arc. Do not fix a same-view orientation issue by rotating actor-side helper geometry; the carrier and support positions still need to come from the Niagara/material/renderer setup.

When the same-view gate is active, `mismatch_notes.md` must compare target and actual against effect-specific discriminators before the result is sent to Pablo. For slash effects, the minimum discriminators are:

- primary and secondary slash bands follow the declared attack direction,
- the shape does not collapse into a circular ring or static overlay lookalike,
- material reads as the approved aura/slash family rather than visible tiling, checkerboard, or unrelated noise,
- impact/support placement is attached to the intended contact point,
- the full active VFX is in frame.

If any required discriminator fails, continue iteration or report `PARTIAL`. Do not present the effect as accepted based only on a gameplay video or structural validator.

### 3.4 Visual / Damage Alignment Gate

Use `CombatVFXVisualDamageAlignmentContract.md` whenever the effect has a hitbox, damage query, projectile damage primitive, or combat impact context.

This gate is additive. It does not make Niagara, render mesh geometry, material opacity, or temporary visual mesh scale authoritative for damage. It forces the packet to declare how the visual carrier lines up with the authoritative combat shape.

Every applicable effect packet must record:

- authoritative damage center and impact point,
- damage shape type and extents,
- visual anchor model, pivot, offsets, and footprint,
- `BaseVisualRadius` or alternate scale mapping,
- alignment tolerance,
- whether any marker-vs-area mismatch is temporary, approved, or `PARTIAL`,
- capture/log evidence proving the VFX and DamageVolume align in the same frame range.

For AOE idol effects, a compact impact marker is not a complete area read. Final production acceptance requires a readable area footprint or an explicitly approved split between marker and area telegraph.

### 3.5 Impact Context Gate

Use `CombatVFXImpactContextContract.md` whenever the effect publishes, consumes, or chains a combat impact context. This includes weapon attacks that can trigger idols, idol modifiers that own damage or status, projectiles that report impact points, and future downstream chained effects.

This gate is separate from visual/damage alignment. Alignment proves the visual and authoritative damage footprint line up. Impact context proof shows source identity, parent identity, expected downstream count, skip/fallback counters, and damage attribution.

Every applicable effect packet must record:

- weapon publication policy, such as one context per attack, target, projectile, path segment, or chain link,
- source identity through `SourceType` and `SourceID`,
- `ParentSourceID` rule for downstream contexts,
- official impact point rule,
- idol or downstream consumption rule,
- expected downstream context count,
- damage/status source proof,
- neutral-control proof,
- generalized diagnostic schema or an explicit mapping from effect-specific diagnostic fields to the generalized schema.

For idol effects, the idol must publish its own impact context and own impact point even if no later chaining consumes it yet. The proof cannot be inherited from the weapon context, and a gameplay video cannot satisfy this gate without runtime context and damage-source logs.

## 4. Carrier Archetype Assignment

Every effect packet must declare exactly one primary carrier archetype. The declaration must be justified against the effect's mechanism manifest.

Changing the primary carrier archetype later requires a written reason and re-review. This prevents relabeling a slash as an aura or support effect just to waive reveal, erosion, or layer gates.

Supported archetypes:

- `ArcSlash`: mesh arc, slash strip, or path/ribbon carrier for AOE and slash-like RetiredLine effects.
- `RibbonTrail`: weapon trail, trailing ribbon, or motion-history strip.
- `PersistentAura`: target-attached or world-attached ring/aura with pulse or orbit behavior.
- `BeamHop`: Start/End beam, hop, chain, or path segment.
- `SupportImpact`: impact flares, sparks, motes, wisps, dust, or secondary accent particles.

`SupportImpact` cannot be the primary carrier for an attack slash, trail, aura, or projectile path unless the user explicitly approves a different visual target.

## 5. Staged Authoring Order

Use staged gates so early probes stay cheap, while full acceptance remains strict.

### Checkpoint 1: Carrier, Motion, And Reveal

Prove the primary carrier exists and performs its required motion. A minimal material is allowed, but if the effect's manifest requires progressive reveal, reveal is still required here.

Evidence:

- asset inspection or validator proves the primary renderer/carrier,
- frame range shows required A-to-B, orbit, Start/End, or path behavior,
- if required, spatial reveal changes which portion of the carrier exists.

### Checkpoint 2: Material Behaviors

Add the required mask/material manifest for the carrier archetype.

Evidence:

- each required mask/material behavior has a parameter-sweep or controlled-frame proof,
- required temporal parameters are live-driven, not static,
- allowed flipbooks pass the freeze test in Section 8.

### Checkpoint 3: Layer Stack

Add base, highlight, backing, and any carrier-required layer variants.

Evidence:

- layers differ in timing, scale, color, render order, tiling, panning, erosion, or opacity,
- dark/backing contrast renders through Translucent blend or another proven non-additive contrast method,
- removing a layer would visibly reduce readability, not merely remove redundant brightness.

### Checkpoint 4: Support And Impact

Add support particles only after the required primary carrier and mask/material discriminators pass.

Evidence:

- support reinforces swing, hit, trail, orbit, hop, or dissipation,
- support does not replace or obscure the primary carrier,
- enemy/target readability remains intact.

## 6. Mask And Material Manifest Schema

Each required mask or material behavior gets one row in the effect packet:

```text
MASK / MATERIAL MANIFEST ROW
Name:
Carrier archetype:
Required for this effect: YES/NO
Purpose:
Required behavior:
Driven parameter:
Expected driver: NormalizedAge / UserParam / PathProgress / TickPulse / LoopPhase / OrbitPhase / StaticAllowed
Static allowed: YES/NO
Packed channel:
Source/license:
Proof/discriminator:
Cheap wrong result:
Status: PLANNED/PRESENT/ABSENT/DEFERRED
```

Rules:

- `Reveal` cannot be static when required.
- `Erosion` cannot be static when required.
- Static masks are allowed only for stable silhouette, body, or edge weighting when another live parameter drives the temporal behavior.
- A mask whose parameter sweep does not visibly change the intended behavior is a no-op and cannot pass.
- Use channel packing when reasonable. The manifest must record which channel owns which role so the texture set is not inflated just to satisfy names.

## 7. Required Manifests By Carrier Archetype

### 7.1 ArcSlash

Use for AOE half-moon slash, slash strips, and slash-like RetiredLine effects.

Required for full acceptance:

- `SilhouetteMask`: defines the readable body shape.
- `SoftEdgeGradient`: feathers the cutout and avoids hard-stencil edges.
- `RevealMask`: spatial draw-on from point A to point B.
- `EdgeBandMask`: hot rim, leading edge, or core/edge separation.
- `DirectionalStreaks`: travel-axis energy grain.
- `ErosionDissolve`: spatial breakup and dissipation.
- `InternalNoise`: live energy variation, not the primary silhouette by itself.

Discriminators:

- Reveal proof: at fixed camera/system setup, sweeping `Reveal` changes the spatial extent drawn from A to B. The undrawn region must be absent, not a low-alpha complete shape.
- Erosion proof: sweeping `Erosion` breaks the body apart spatially, preferably trailing-edge or direction weighted. A uniform fade is a lookalike.
- Streak proof: changing `SpeedX/SpeedY` or equivalent motion parameter moves detail along the intended travel axis.
- Edge proof: changing `EdgeWidth` or equivalent changes rim/core width without resizing the whole carrier.

### 7.2 RibbonTrail

Use for weapon trails, motion-history strips, Bounce trails, and some retired lane ribbons.

Required for full acceptance:

- `TrailWidthOrShapeMask`: controls ribbon body/readable width.
- `SoftEdgeGradient`: prevents hard ribbon clipping.
- `EdgeBandMask`: bright edge or hot center line.
- `DirectionalStreaks`: motion-aligned flow.
- `ErosionDissolve` or `TailFade`: head/tail fade and dissipation.

Valid drivers include `NormalizedAge`, ribbon age, sampled motion history, path progress, or user parameters.

Discriminators:

- frame range shows head/tail fade instead of a fully uniform strip,
- streaks flow along ribbon travel or weapon motion,
- edge banding remains readable at gameplay camera distance,
- if using socket sampling, width/direction follows the sampled base/tip or equivalent path.

### 7.3 PersistentAura

Use for DOT auras, target-attached rings, orbiting weapon silhouettes, and long-lived pulsing fields.

Required for full acceptance:

- `RingBandMask` or a procedural radial ring function,
- `SoftEdgeGradient`,
- `PulseThicknessOrIntensity`,
- optional `OrbitTrailMask` when orbiting silhouettes leave trails,
- optional `ErosionDissolve` when pieces dissolve/reform.

Valid drivers include `TickPulse`, loop phase, orbit phase, or explicit user parameters. `NormalizedAge` is not required for persistent infinite-life DOT systems.

Discriminators:

- target-following proof if attached behavior is claimed,
- tick pulse visibly changes thickness, intensity, scale, or opacity at the intended cadence,
- orbit/spin proof shows real angular motion, not only torus placement,
- target remains readable through the aura.

### 7.4 BeamHop

Use for Bounce chains, spirit-axe hops, lightning-like Start/End systems, and some straight retired lane beams.

Required for full acceptance:

- `BeamWidthShapeMask`,
- `SoftEdgeGradient`,
- `DirectionalStreaks` or path noise,
- `EndImpactFlareMask`,
- optional `ReformDissolve` for spirit axe reassembly.

Valid drivers include path progress, hop phase, user Start/End parameters, or `NormalizedAge` depending on implementation.

Discriminators:

- Start/End controls place the carrier on the intended targets or lane points,
- hop travel or beam formation is visible over time when required,
- impact fires at the end point and faces or aligns to the intended impact normal when applicable,
- jitter is low enough to preserve axe/spirit identity unless an elemental idol layer intentionally adds lightning.

### 7.5 SupportImpact

Use for secondary particles only.

Common masks:

- stretched spark mask,
- flare mask,
- mote/wisp mask,
- dust/debris mask,
- light/glow falloff.

Discriminators:

- support moves in the same direction or rhythm as the primary action,
- support has its own fade/dissipation,
- support does not carry the primary silhouette,
- support does not hide enemies or damage feedback.

## 8. Flipbook And Baked Content Rule

Forbidden without explicit user approval:

- a pre-composited full-effect animation or flipbook where carrier, reveal, erosion, layer timing, and support are already baked into one clip and the Niagara/material system only plays it back.

Allowed when documented:

- tiling sub-texture flipbooks,
- animated noise flipbooks,
- streak/detail flipbooks,
- small impact/support flipbooks,
- other detail textures that drive live material graph behavior.

Freeze test:

- Freezing or disabling the flipbook texture must leave carrier silhouette, reveal extent, erosion breakup, and layer timing still present and parameter-driven.
- If freezing the flipbook collapses silhouette, reveal, erosion, or layer timing, the flipbook is functioning as a full-effect bake and is forbidden unless explicitly approved as a different process.

## 9. Material Construction Order

Build materials in behavior order:

1. silhouette/body opacity,
2. soft edge falloff,
3. spatial reveal if required,
4. emissive body color,
5. edge/core banding,
6. internal streak/noise motion,
7. erosion/dissolve,
8. contrast/backing layer,
9. exposed parameters for Niagara or user control.

Common parameters:

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

`Reveal` controls spatial existence. `SpeedX/SpeedY` or panners control internal texture motion. Do not use UV panning as proof of reveal.

Dark or black backing layers cannot rely on Additive black. Use Translucent or another proven method that can actually render contrast.

### 9.1 Shared Base Material Language

When multiple attacks belong to one weapon, hero, or elemental family, define a shared material-language packet before visual polish diverges per attack. The packet must be source-evidence-driven, not a style guess.

The shared packet owns:

- selected material reference sources and transcript/source paths,
- which source observations are `observed`, `inferred`, or `tuned`,
- required mask/material roles shared across the family,
- allowed per-effect variations,
- forbidden material readings,
- parameter names and intended drivers,
- editor-isolation and gameplay evidence required before the material family is accepted.

For base aura slash families, lock the material language before making final per-effect polish decisions. Common shared roles include:

- soft body/silhouette mask,
- soft edge gradient,
- edge/core banding,
- directional streak mask,
- internal panning noise or aura detail,
- erosion/dissolve breakup,
- optional translucent/dark backing for contrast,
- Niagara-driven `Reveal`, `Erosion`, `Opacity`, `Intensity`, `EdgeWidth`, color, and panning controls.

Allowed per-effect variation normally includes carrier shape, color palette, scale, timing, impact/support emitters, and hitbox-alignment presentation. The base texture language, edge softness, streak/noise behavior, and dissolve vocabulary should remain recognizable unless the effect packet explicitly approves a different family.

If the current result reads as a checker, tile grid, hard stencil, flat ring, static overlay, or unrelated noise pattern, the shared material gate is not satisfied even if the Niagara system uses the expected mesh or renderer.

## 10. Niagara Construction Order

Build Niagara from primary to secondary:

1. create primary emitter and renderer,
2. bind mesh/ribbon/beam/sprite carrier,
3. set spawn/lifetime/activation rules,
4. set local-space/world-space/orientation rules,
5. bind material override,
6. drive material parameters through the effect's valid driver,
7. add motion, reveal, orbit, path, or Start/End curves,
8. duplicate or add base/highlight/backing layers with distinct behavior,
9. add support emitters only after primary discriminators pass,
10. set one-shot versus persistent system state intentionally.

Module order matters. If a force or motion module requires a solver or later update module, the effect packet must record and validate that ordering.

## 11. Layer Stack

A production slash/aura/trail pass should declare its layer stack:

- base body,
- bright highlight or leading edge,
- dark/backing contrast,
- internal streak/noise,
- impact flare,
- stretched directional support,
- lingering motes/wisps/dust,
- optional ground trace/decal.

Each layer must have a reason to exist and a discriminator. Duplicate layers with identical timing, material response, and render behavior count as redundant brightness, not production layering.

## 12. Per-Effect Recipe Ownership

This procedure does not own final numbers for Hero 1 axe VFX. Use these source files for effect-specific values and staged decisions:

- `Saved/VFXResearch/Hero1Axe/notegpt_analysis_2026-05-24.md`
- `Saved/AgentReviews/Hero1AxeVFX_AOE_RetiredLine_20260524T014849/claude_review_full.md`
- `Saved/AgentReviews/Hero1AxeVFX_DOT_Bounce_20260524T015135/claude_review.md`
- `Gameplay/Combat/Hero1AxeVFXPlan.md`
- `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`

Generic mapping:

- `AOE`: usually `ArcSlash`.
- `RetiredLine`: usually `ArcSlash`, `RibbonTrail`, or `BeamHop` depending on whether the visual is a force slash, ribbon strip, or lane beam.
- `DOT`: usually `PersistentAura`.
- `Bounce`: usually `BeamHop` for the hop plus `RibbonTrail` and `SupportImpact` as secondary carriers.

The per-effect packet must choose the primary archetype and list any secondary archetypes.

## 13. Parameter-Sweep Evidence

Use the existing Unreal-owned capture path for proof and follow the `AGENTS.md` `Unreal video and screenshots` process row. Diagnostic captures may use alternate angle or zoom when labeled as diagnostic, but they do not replace a locked acceptance camera or gate unless the user explicitly approves that new gate.

- `Scripts/CaptureT66GameplayVideo.ps1` for gameplay/VFX video,
- `Scripts/CaptureT66GameplayVideo.ps1 -EvidenceBundle` for VFX review bundles after the Unreal-owned MP4/frame sequence is captured,
- the Section 3.3 editor-isolation artifact bundle when same-view target comparison is active,
- fixed camera and fixed system setup where possible,
- stepped or scripted parameter values,
- representative frame range showing before/mid/after states.

For locked combat VFX proof cameras, disable camera wall-occlusion fading unless the task is specifically testing the camera occlusion feature. The gameplay camera can legitimately swap occluding wall meshes to `/Game/Materials/M_CameraWallOccluderFade`, which may appear as a large cream translucent rectangle in captures when a tagged tower wall is between the camera and hero. That is a gameplay readability feature, not combat VFX evidence; it is proof contamination for weapon/idol captures.

The evidence bundle is additive. It does not replace the Unreal-owned capture requirement and does not approve visual fidelity by itself. A requested bundle should include the MP4, retained PNG frames, `ffprobe.json`, `manifest.json`, `contact_sheet.png`, copied selected frames, `selected_frames.md`, and `visibility_checklist.md`.

Parameter-sweep proof should cover required behaviors, not every cosmetic tweak. Required examples:

- `Reveal`: spatial extent changes from point A toward point B.
- `Erosion`: non-uniform breakup/dissolve, not uniform fade.
- `DirectionalStreaks`: motion along the intended axis.
- `EdgeWidth`: visible rim/core width shift.
- `TickPulse`: pulsed aura change at intended cadence.
- `Start/End`: BeamHop path lands on intended points.

Temporal mechanisms cannot be proven by one still. Use multi-frame evidence.

## 14. Known Automation And Editor Pitfalls

Record hard-won tooling problems here so future VFX passes do not rediscover them by trial and error.

- Dynamic parameter wiring can fail when a script assumes named pins that are not present in the current material/Niagara node. Inspect the generated graph or route through supported masks/components before claiming a parameter is driven.
- Niagara emitter creation must use the supported Unreal/Niagara editor APIs for this project. If an emitter, renderer, module, or binding cannot be authored by script, report a tooling blocker instead of substituting actor-side geometry.
- Vector scale and renderer bindings must use the format expected by the target Niagara API. A structurally valid asset can still render incorrectly if scale, orientation, or material bindings are not actually consumed.
- Python/AssetTools import paths may assert or stall in editor automation. Prefer the existing validated lab scripts/commandlets when available, and stop after repeated import failures instead of switching to a different visual method.
- UE 5.7 `AssetTools.ImportAssetTasks` can save a new texture and then crash in headless automation with `Assertion failed: CurrentApplication.IsValid()` from Slate/ContentBrowser. When this happens in `/Game/VFXLab`, verify whether the texture `.uasset` was written, rerun the setup after the import step is no longer needed, and document the crash. Do not interpret the crash as permission to skip the mask/texture artifact.
- Existing texture assets can be stale even when regenerated PNG source files changed. Check source and `.uasset` timestamps when evaluating authored masks, and force a lab reimport or delete/reimport only inside the declared generated lab folder when needed.
- Structural validation is not visual acceptance. A validator proving that an asset exists, a renderer is present, or a forbidden asset is absent does not prove reveal, sweep, erosion, color variation, impact placement, or readability.
- Diagnostic camera captures help find problems, but acceptance requires the declared gate and multi-frame proof for temporal mechanisms.
- A large cream/white rectangle in a gameplay VFX capture is usually the camera wall-occlusion fade material, not the weapon or idol effect. Confirm via `T66.Camera.WallOcclusionEnabled`, the camera-to-hero trace, and `/Game/Materials/M_CameraWallOccluderFade`; do not approve or reject VFX shape from contaminated frames.
- A Niagara editor screenshot is not process evidence unless the route, view, zoom, background, target system, timing, and output path are reproducible. If a previous screenshot was produced by a temporary editor hook or private viewport access that is no longer in the tree, treat it as diagnostic history only until a durable route is implemented and verified.
- `UnrealEditor-Cmd.exe` can occasionally return a null process exit code to PowerShell even after MRQ or Python commandlets print successful completion and request exit status `0`. `Scripts/CaptureT66NiagaraMRQIsolation.ps1` treats this as success only when the process has exited and stdout contains known success markers such as `Success - 0 error(s)`, `Python script executed successfully`, or `Movie Pipeline completed` plus `RequestExitWithStatus(0, 0)`. Do not copy this relaxation into other wrappers without equivalent success markers and output-artifact checks.
- Deprecated-asset guards are intentional when they assert that older lookalike paths are absent. Do not rename those negative checks to the new asset names.

## 15. Close Templates

### 15.1 Mask/Material Close

```text
MASK / MATERIAL CLOSE
Name:
Carrier archetype:
Required: YES/NO
Status: PRESENT/ABSENT/DEFERRED
Driver:
Static allowed: YES/NO
Evidence:
Discriminator:
Reported status: FULL/PARTIAL
```

`FULL` requires every required mask/material behavior for the primary carrier archetype to be `PRESENT` with evidence. Any missing required behavior means `PARTIAL`.

### 15.2 Impact Context Close

Use the close template from `CombatVFXImpactContextContract.md` when the effect publishes, consumes, or chains a combat impact context. The close must report weapon context publication, idol/downstream context consumption, parity, source identity, neutral control, legacy fallback count, and evidence.

### 15.3 Authoring Close

```text
COMBAT VFX AUTHORING CLOSE
Primary carrier archetype:
Secondary archetypes:
PPF close:
Artifact parity close:
Mechanism close:
Mask/material close:
Impact context close:
Anti-lookalike result:
Capture evidence:
Reported status: FULL/PARTIAL
```

Do not report an effect as production-ready until the user signs off on captured evidence.
