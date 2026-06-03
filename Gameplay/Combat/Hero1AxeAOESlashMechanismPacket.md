# Hero 1 Axe AOE Slash Mechanism Packet

**Created:** 2026-05-24
**Status:** Accepted for the isolated Checkpoint 1 carrier/sweep/reveal implementation pass on 2026-05-26 by Pablo's go-ahead plus Claude review `Saved/AgentReviews/20260526T_hero1_axe_aoe_checkpoint1_pass/20260526T214616-pass1/claude_review_pass1.md`. Updated 2026-05-27 with same-view editor-isolation, shared aura material gates, Candidate03 north-facing aura correction evidence, and gameplay hitbox/damage proof. Updated 2026-05-28 with the first production binding, item-stat wiring proof, and crescent-band hitbox alignment proof for Hero 1 axe AOE only. This does not approve final visual polish, future weapon bindings, idol overlays, or shipped-content release.
**Owner:** Gameplay/Combat isolated VFX lab.
**Parent plan:** `Gameplay/Combat/Hero1AxeVFXPlan.md`

## 1. Working Goal

Build the next isolated Hero 1 axe AOE VFX pass as a real slash system, not a static crescent lookalike.

The next pass must prove the primary slash carrier, motion, material animation, erosion, breakup, and layer timing before any secondary impact or support particles are treated as useful polish.

## 2. User Constraints

- The accepted visual design is frozen for now; do not keep redesigning the AOE slash during backend-wiring work.
- Do not change the current red/blue placeholder projectile system except where a production VFX binding explicitly suppresses the placeholder for its own weapon row.
- Production binding is approved for the Hero 1 axe AOE backend seam only. Future weapons/idols need their own reviewed binding rows, effect packets, and proof.
- Do not treat the current static crescent prototype as complete.
- Do not substitute a faster method when the reference mechanism is what produces the target result.
- Use Unreal-owned capture for visual proof.
- Current visual target is AOE-specific: a red/blue American-flag-inspired crescent slash with a white impact position.

## 3. Process Sources

Primary process rules:

- `AGENTS.md`, Section 2: PPF, artifact parity, mechanism manifest, mechanism close, and anti-lookalike rule.
- `AGENTS.md`, Section 4: Niagara combat VFX process.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`: generic carrier archetype, mask/material manifest, parameter-sweep discriminator, anti-bake, and mask/material close rules.
- `Gameplay/Combat/Hero1AxeSharedAuraMaterialResearchPlan.md`: shared base aura material source-selection and family-language gate.
- `Gameplay/Combat/Hero1AxeVFXPlan.md`, Section 4.1: Required AOE mechanism manifest.
- `Gameplay/Combat/Hero1AxeVFXPlan.md`, Section 3.1: authoritative Hero 1 canonical research source selection.

Research sources already collected:

- `Saved/VFXResearch/Hero1Axe/source_matrix_2026-05-24.md`
- `Saved/VFXResearch/Hero1Axe/notegpt_analysis_2026-05-24.md`
- `Saved/AgentReviews` Claude source-evidence reviews

Key reference observations from the research notes:

- `Anatomy Of A Slash`: a slash is built from a base shape, highlight, support details, impact, movement, and dissipation. The base establishes the shape; the shader movement and dissipation make it read as a slash instead of an overlay.
- `Unreal Engine 5 - Sword Slash VFX - Niagara Tutorial`: uses an empty Niagara system, mesh renderer, custom flattened arc mesh, lifetime around `0.35`, mesh orientation and mesh rotation force, dynamic material parameters, texture panning, power/erosion, and multiple layers including bright, orange/background, and dark translucent backing.
- `Create a Multi-Layer Ribbon Trail VFX`: shows material-side edge bands, color gradients, opacity variation, fade over life, and layered ribbon colors.
- `Weapon Trail Effect In Unreal Engine Niagara`: shows time-sampled trail behavior, ribbon width/direction, short lifetime, color curves, and motion-support particles.

### 3.1 Current AOE Replication Start

The parent plan owns the canonical source decision. For this AOE packet, the current source pointer is:

- Authoritative selection: `Gameplay/Combat/Hero1AxeVFXPlan.md`, Section 3.1.
- AOE implementation and supporting technique sources: use the parent plan's current canonical source selection.
- Current user visual target: AOE-specific American-flag-inspired half-moon axe slash with red and blue energy bands, a white impact position at enemy contact, non-uniform multi-shape slash carrier, multi-color body/core/edge variation, and visible dissipation. This packet cannot support a `FULL` visual-acceptance claim until the generated visual target or contact sheet is saved to a repo path and Pablo explicitly approves that exact saved variant.

Before any further Niagara implementation, run an adherence/tooling audit against this packet and the owning generic procedure:

| Mechanism or artifact | Owning doc | Required method | Current implementation status | Discrepancy type | Next action |
|---|---|---|---|---|---|
| Primary carrier archetype | `CombatVFXAuthoringProcedure.md` / this packet | `ArcSlash` mesh or ribbon carrier authored in Niagara/material/renderer assets | Not audited in this pass | `TBD` | Compare current lab assets and scripts against the required carrier method. |
| A-to-B sweep motion | This packet | Leading edge travels across the frontal cone; full crescent is not present in first active frame | Not audited in this pass | `TBD` | Compare captures and Niagara timing against the F/F+3/F+6/F+10 gate. |
| Progressive arc reveal | This packet | Material/Niagara-driven draw-on across mesh UV/path progress | Not audited in this pass | `TBD` | Inspect material driver and captured frame sequence. |
| Material animation | `CombatVFXAuthoringProcedure.md` / this packet | Live-driven panning, dynamic parameters, or equivalent internal motion | Not audited in this pass | `TBD` | Inspect material graph and parameter drivers. |
| Erosion and dissipation | `CombatVFXAuthoringProcedure.md` / this packet | Noise/mask-driven spatial breakup over lifetime | Not audited in this pass | `TBD` | Inspect material graph and late-life frames. |
| Shape taper and breakup | `CombatVFXAuthoringProcedure.md` / this packet | Non-uniform silhouette, edge bands, masks, or mesh UV layout | Not audited in this pass | `TBD` | Compare source technique and current mesh/material. |
| Layered base/highlight/backing timing | `CombatVFXAuthoringProcedure.md` / this packet | Distinct bright, body, and dark/backing layers with separate timing or material behavior | Not audited in this pass | `TBD` | Inspect emitters/renderers/materials before adding support particles. |

Discrepancy type values: `ADHERENCE DRIFT`, `TOOLING BLOCKER`, `MISSING SOURCE DETAIL`, `USER DECISION NEEDED`. Multiple labels are allowed when a row has both an adherence issue and a tooling obstacle.

### 3.2 Visual Target Brief - American Flag AOE Crescent

This visual target is active for the AOE lab only. It does not redefine DOT, Pierce, or Bounce, which continue to use the shared ethereal axe language in the parent plan.

Mockup authority:

- A generated mockup controls visual direction only after Pablo approves the exact saved artifact.
- It cannot replace the source/tutorial method, artifact parity gate, mechanism manifest, F/F+3/F+6/F+10 reveal gate, or anti-lookalike discriminator in Sections 5, 6, 9, and 10.
- `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/` is the planned evidence folder. This packet remains the tracked source of truth.

Intent:

- A broad frontal 180-degree axe-weight crescent slash.
- Red and blue energy bands are the main crescent colors.
- White is primarily the impact-position color at enemy contact; Pablo may approve extra white streak accents if the generated mockup supports them.
- The result should read as "American flag concept" through red/blue/white energy language, not literal cloth, UI, stars, typography, or a flag decal.

Temporal panels:

1. Start: the slash begins as a partial blue/red leading arc near point A, with the full crescent not yet visible.
2. Mid: the crescent grows across the frontal cone, showing non-uniform red/blue bands and internal streaks.
3. Impact/full: the arc reaches enemy contact, with a strong white impact flare/position and readable red/blue crescent body.
4. Dissipate: the crescent breaks into red/blue fragments, streaks, motes, or eroded bands while the white impact fades.

Required colors/shapes:

- Red/blue crescent body with visible color separation, not a uniform single-color arc.
- Non-uniform edge and thickness: thinner start/tail, heavier middle/impact body, broken or tapered ends.
- Bright white impact position at or near the target contact point.
- Broad axe-cleave shape, heavier and wider than a sword slash.

Forbidden readings:

- A static patriotic logo or flag overlay.
- A perfectly uniform mathematical half-ring.
- A full crescent that appears all at once without visible travel.
- Generic sparks/shockwave with no slash carrier.
- A sword-thin arc that loses the axe-weight read.

Approval artifact:

- Planned folder: `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/`.
- Candidate contact sheet generated 2026-05-26: `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/hero1_axe_aoe_american_flag_contact_sheet.png`.
- Worker result: `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/worker_result.md`.
- Status: approved by Pablo on 2026-05-26 as the binding visual target for the next AOE lab pass.
- Approval note: Pablo approved the image as a good process-development start because it has two clearly designated red and blue areas and a strong visual read. Use it as reference and flag any issues it causes.
- Known image-driven caveats to flag during implementation:
  - The mockup has no enemies or dungeon floor, so gameplay readability and enemy contact placement still require Unreal-owned capture validation.
  - The mockup is a stylized 2D contact sheet, not the final gameplay camera angle.
  - White appears as both impact flare and internal streak accents; the final Niagara pass may use limited white streak accents if they support the approved read, but the main white read should remain the impact/contact position.
  - The small axe/weapon graphic in the mockup is reference context only; the AOE pass remains Niagara slash-first and must not reintroduce a separate weapon-layer carrier unless explicitly approved.

### 3.3 Visual Decomposition

| Visual component | Purpose | Notes |
|---|---|---|
| Primary crescent silhouette | Establish the half-moon AOE hit shape. | Must map to `ArcSlash` carrier in Niagara/material assets and satisfy existing Sections 5, 6, 9, and 10. |
| Blue energy band | Main cool body layer. | Candidate inner/backing layer or lower-energy fill; should not flatten into one uniform blue strip. |
| Red energy band | Main warm body/highlight layer. | Candidate outer/leading band, internal streak, or alternate layer timing. |
| White impact position | Contact readability. | Candidate impact flare/support emitter at enemy contact after primary slash is working; must not replace the crescent carrier. |
| Internal streaks/breakup | Avoid uniformity and create production texture. | Candidate authored mask, UV panning, erosion noise, or multiple material layers. |
| Dissipation fragments | Prove end-of-life behavior. | Candidate erosion mask, fragment sprites, motes, and late-life opacity/timing curves. |

### 3.4 Niagara Mapping

| Visual component | Niagara/material carrier | Driver | Evidence needed |
|---|---|---|---|
| Primary red/blue crescent | `ArcSlash` mesh renderer or approved path/ribbon carrier in `NS_Hero1AxeAOE_MeshSlash` | Normalized age reveal plus Niagara timing/orientation | Frame range proves point A to point B reveal; full arc is not present in first active frame. |
| Blue body layer | Distinct material layer or renderer using mask/gradient control | Dynamic material parameters, UV panning, opacity over life | Material graph and frames show independent blue body behavior. |
| Red highlight/body layer | Distinct material layer or renderer using mask/gradient control | Dynamic material parameters, render order, scale/timing offset | Frames show red layer has different timing, shape, or motion than blue layer. |
| White impact position | Secondary impact emitter or material-driven contact flare after primary carrier passes | Timed burst at impact/full panel | Frames show white concentrated at contact, not spread uniformly across the crescent. |
| Non-uniform streaks and edge breakup | Authored mask/noise texture and erosion controls | UV panning, erosion threshold, alpha/edge width curves | Close frames show uneven bands, tapered edge, and breakup over lifetime. |
| Dissipation | Material erosion plus optional support particles | Normalized age, opacity curve, motes/streak lifetime | Late frames show breakup/dissolve instead of pop-off. |

### 3.5 Acceptance Rubric For The Mockup-Driven Pass

This rubric layers onto the existing gates. It does not waive any required row in Sections 5, 6, 9, or 10.

Required still-frame match:

- Broad red/blue half-moon crescent.
- White impact position at enemy contact.
- Non-uniform thickness, streaks, and broken/tapered edges.
- Enemies remain readable through or around the effect.

Required temporal match:

- Start, mid, impact/full, and dissipate frames are distinguishable.
- The slash travels from point A to point B.
- Internal color/streak motion is visible.
- End-of-life dissolves or breaks up instead of disappearing as a solid whole.

Required gameplay readability:

- From the accepted gameplay camera, the effect reads as a frontal AOE axe slash.
- The hit window and impact location are understandable.
- Red/blue/white language is clear without becoming a UI flag graphic.

Explicit non-goals:

- No live combat integration in this lab pass.
- No literal flag, stars, typography, cloth, or decal unless Pablo explicitly approves a generated variant with those details.
- No claim of production readiness without Unreal-owned video, frame evidence, mechanism close, and Pablo approval of the visual target match.

Process stop:

- Next stop is the Codex CLI imagegen worker creating the saved same-view target artifact or reaching a tooling blocker. Pablo must approve the exact same-view visual target before any Niagara pass is accepted against it.

### 3.6 Same-View Editor-Isolation Gate

This gate extends `CombatVFXAuthoringProcedure.md`, Section 3.3 for the AOE lab.

Current status:

- Baseline route implemented and verified on 2026-05-27 through `Scripts/CaptureT66NiagaraMRQIsolation.ps1`.
- Required before the next visual-fidelity handoff to Pablo.
- The earlier top/black Niagara editor screenshot remains useful diagnostic evidence, but the official repeatable route is now the MRQ script below.

Canonical AOE editor-isolation view:

- target system: `/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash`,
- view: top-down orthographic unless a later reviewed packet explicitly changes it,
- background: plain black or an equivalent neutral non-dungeon background,
- render mode: unlit or equivalent high-contrast preview,
- framing: full slash, support, and impact visible with roughly 10-15 percent margin; no cropped red, blue, white, or support layer,
- timing: diagnostic slow-preview is allowed for still capture, but the packet must label it diagnostic and still require gameplay timing evidence separately,
- output folder convention: `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/EditorIsolation/<timestamp>/`.

Canonical capture command:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66NiagaraMRQIsolation.ps1 `
  -OutputDir C:\UE\T66\Saved\VFXResearch\Hero1Axe\AOE_AmericanFlagVisualTarget\EditorIsolation\<timestamp> `
  -ResX 1400 -ResY 1400 `
  -OrthoWidth 1250
```

Route proof:

- Artifact folder: `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/EditorIsolation/20260527_MRQSquareRouteProbe/`.
- `actual.png` is a square top-down MRQ render of the actual lab Niagara system normalized into an opaque black review PNG.
- `manifest.json` reports `render_success=true`, `is_square=true`, `has_non_black_pixels=true`, `margins_pass=true`, and `has_particle_log_evidence=true`.
- Bounding box in the proof manifest: `MinX=301`, `MinY=439`, `MaxX=1096`, `MaxY=710`, with margins `Left=301`, `Top=439`, `Right=303`, `Bottom=689` at `1400x1400`.
- The proof explicitly does not accept the current VFX visual; it only proves the editor-isolation route.

Required artifacts:

```text
manifest.json
target.png
actual.png
actual_crop.png
contact_sheet.png
mismatch_notes.md
```

Same-view target rule:

- Do not generate or approve the next binding imagegen target until the editor-isolation frame is locked.
- The target image must use the same view/crop as the actual editor-isolation capture: black background, no hero, no enemies, no dungeon, full VFX visible.
- If the capture zoom, angle, or crop changes, regenerate or explicitly re-approve the target.

AOE mismatch rubric:

- Shape: red and blue slash bands must both follow the same frontal half-moon crescent direction. A circular ring, swirl, or two-layer circle read fails.
- Material: the base attack should read as shared aura-slash energy with soft edges, streaks, and dissolve, not a checker pattern, tiled grid, or flat noise sheet.
- Color placement: red is expected to read as the outer or leading aspect unless Pablo approves a different target; blue is expected to read as the inner/body or backing aspect.
- Impact: the white impact point must attach to the contact side of the blue/red slash instead of floating as a small disconnected spark.
- Framing: the full active effect must be visible in the editor-isolation crop.

The editor-isolation gate can return `PARTIAL` and guide iteration, but a future `FULL` visual claim must also pass gameplay capture, temporal mechanism close, hitbox/damage evidence where applicable, and Pablo approval.

### 3.6.1 Same-View Close: 2026-05-27 Candidate03 North Aura

Status: `PRESENT` for the current requested correction: north-facing orientation, simpler aura read, red outer/top band, blue inner/body band, centered white impact point, and full square framing.

Evidence:

- Approved same-view target: `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/SameViewTarget_20260527/hero1_axe_aoe_same_view_target_candidate03.png`
- Actual same-view capture: `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/EditorIsolation/20260527_Candidate03NorthAuraPass08OpaqueBlackRgbReview/actual.png`
- Crop/contact sheet: `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/EditorIsolation/20260527_Candidate03NorthAuraPass08OpaqueBlackRgbReview/actual_crop.png` and `contact_sheet.png`
- Review notes: `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/EditorIsolation/20260527_Candidate03NorthAuraPass08OpaqueBlackRgbReview/mismatch_notes.md`
- Manifest: `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/EditorIsolation/20260527_Candidate03NorthAuraPass08OpaqueBlackRgbReview/manifest.json`
- Background proof: `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/EditorIsolation/20260527_Candidate03NorthAuraPass08OpaqueBlackRgbReview/background_pixel_check.md`

Repeatability:

- The same-view evidence is not a one-off editor screenshot. It is generated by rerunning `Scripts/CaptureT66NiagaraMRQIsolation.ps1` with the recorded output folder, resolution, orthographic camera, and target Niagara system. Visual judgment still requires human review of `actual.png`, but the capture route itself is deterministic and artifact-checked.

Implementation notes:

- Candidate03 follows the top-down MRQ coordinate mapping owned by `CombatVFXAuthoringProcedure.md`, Section 3.3; it uses world `+X` for the same-view north/contact direction in both the mesh profile and support positions.
- `RotationForceZ` is held at `0.0` for the current visual-lock pass so the red/blue layers do not peel apart into an elemental/electric read. This temporary state is tracked in `pending_issues_Combat.md`; future animation polish may reintroduce controlled layer timing, but only if the same-view shape remains locked.
- The material pass reduces streak/dissolve strength and raises detail floors to keep the base attack in the shared aura-slash family rather than a checker/electric family.
- The white impact/support point is centered at the north contact position for this same-view gate. Gameplay hit confirmation remains the separate hitbox proof below.

Current caveat:

- The same-view structure now passes the requested correction. The rebuilt gameplay evidence bundle uses frame 46 as the visible gameplay slash/impact frame; future brightness/scale polish is still allowed, but it must preserve the locked same-view shape.

### 3.7 Shared Aura Material Gate

The AOE material pass must use `Gameplay/Combat/Hero1AxeSharedAuraMaterialResearchPlan.md` before final visual polish.

For this AOE target, the shared material gate specifically rejects:

- checkerboard or visible tiled-grid texture readings,
- a uniform mathematical half-ring,
- flat color bands without soft edge, internal streak, or erosion behavior,
- one-off material choices that cannot plausibly be reused by DOT, Pierce, and Bounce.

The first accepted AOE material should establish reusable Hero 1 axe base-attack material behavior for:

- soft aura body,
- edge/core banding,
- directional streaks,
- internal panning noise,
- erosion/dissolve,
- optional dark/backing contrast,
- Niagara-driven reveal, erosion, opacity, intensity, edge width, color, and panning controls.

The AOE packet may tune shape, red/blue/white colors, timing, and impact placement, but the base material vocabulary should remain reusable for later Hero 1 axe attacks unless a reviewed packet approves a different family.

## 4. PPF Check For The Next Implementation

```text
PPF CHECK
Objective: Build an isolated Hero 1 axe AOE half-moon slash that visibly sweeps from point A to point B, erodes/dissipates, and reads as an axe-themed attack at gameplay camera distance.
Proven process: Niagara/material/texture workflow using mesh or ribbon carrier, material animation, erosion/dissipation, timing curves, source/reference analysis, and Unreal-owned capture.
My planned implementation: Use a Niagara mesh-rendered flat arc slash carrier first, with a generated project-owned arc mesh, additive/translucent slash materials, dynamic material parameters, mesh orientation/rotation timing curves, and staged captures that prove motion before polish.
Same method class: YES, if the carrier, motion, material animation, erosion, and layer timing are authored in Niagara/material/mesh assets and evidenced through frame ranges.
If NO, why: Any fallback to a static sprite mask, actor-side geometry, actor-arranged point components, or generic spark/shockwave carrier is not the same method class.
User approval required before proceeding: YES. Implementation should start only after this packet is accepted.
Verification evidence: setup/import logs, validator logs, inspectable assets, Unreal-owned MP4, ffprobe output, and representative frame ranges showing the mechanism close.
```

## 5. Artifact Parity Gate

| Reference artifact/category | Role | Required | Planned artifact/path | Status before implementation | Evidence needed |
|---|---|---:|---|---|---|
| Slash arc carrier | Primary | Yes | `/Game/VFXLab/Hero1Axe/Shared/SM_Hero1AxeAOE_SlashArc` and `/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash` | Planned | Static mesh asset imported from project-generated source; Niagara system uses mesh renderer, not sprite-only carrier. |
| Additive bright slash material | Primary | Yes | `/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_Slash_Reveal` for Checkpoint 1; future full pass adds additive/body/dark layer materials | Planned | Checkpoint 1 material must prove material-authoritative draw-on reveal. Full acceptance still requires texture/noise mask, UV panning or equivalent material animation, dynamic parameters, and erosion. |
| Dark/backing slash material | Primary for full effect | No for Checkpoint 1 | Future `/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_Slash_Dark` or equivalent | Deferred for Checkpoint 1 | Translucent layer provides body/contrast without hiding enemies in a later layer checkpoint. |
| Niagara timing curves | Primary | Yes | Inside `/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash` | Planned | Mesh orientation/rotation force or equivalent Niagara-side curve creates visible point A to point B sweep. |
| Runtime preview actor | Support | Yes | `AT66Hero1AxeAOEVFXLabActor` or replacement capture-only actor | Planned | Actor places and triggers the Niagara system only; it must not create the slash silhouette or motion by component arrangement. |
| Secondary impact/support particles | Secondary | No | Deferred | Deferred | Add only after primary slash mechanism close is at least `PARTIAL` with carrier and sweep present. |

## 6. Mechanism Manifest

Completion is `FULL` only when every required mechanism below is `PRESENT` with evidence. Any missing required mechanism means the result must be reported as `PARTIAL`.

The primary carrier archetype for this packet is `ArcSlash`. Do not reclassify it to waive reveal, erosion, or mask/material requirements without written justification and re-review. The generic mask/material manifest and parameter-sweep close for `ArcSlash` are owned by `Gameplay/Combat/CombatVFXAuthoringProcedure.md`; this packet owns only the AOE-specific mechanism implementation plan and evidence paths.

| Mechanism | Required | Planned implementation | Evidence needed |
|---|---:|---|---|
| Primary carrier archetype | Yes | Use a Niagara mesh renderer with a project-owned flat arc mesh. The mesh should be generated from a flattened cylinder/arc concept, widened for axe cleave weight, with UVs suitable for taper/noise material motion. | Asset inspection shows a mesh renderer using `SM_Hero1AxeAOE_SlashArc`. Capture frames show the slash has stable world orientation and is not a camera-facing static sprite. |
| A-to-B sweep motion | Yes | Use Niagara-side motion plus progressive arc reveal. The slash leading edge must move across the frontal cone; a complete pre-formed crescent that only rotates rigidly, scales, or fades does not satisfy this mechanism. First target is a short lifetime around `0.35` to `0.55` seconds, fast initial motion, slower finish. | Niagara module/curve evidence plus frame range where the leading edge occupies measurably different angular/spatial positions. The full arc must not be present in the first active frame. |
| Progressive arc reveal | Yes | Use radial/length-wise UV reveal, dynamic material parameter, mesh UV mask, or equivalent normalized-age draw-on so the arc is carved on from point A to point B. UV panning alone is not enough; it may animate texture detail but does not prove draw-on. | Material/Niagara evidence and frame range showing the active arc growing along its path rather than a whole crescent appearing at once. |
| Material animation | Yes | Use dynamic material parameters and UV panning/offset, or equivalent lifetime-driven material motion. Planned parameters: `Power`, `Erosion`, `TilingX`, `TilingY`, `SpeedX`, `SpeedY`. | Material graph evidence and frame range showing internal texture motion on the slash, not only mesh rotation or reveal. |
| Erosion and dissipation | Yes | Use noise/Voronoi or project-owned mask texture driven by normalized age/dynamic parameter so the slash dissolves over lifetime. | Material graph and Niagara parameter/curve evidence; late-life frames show breakup/dissolve instead of pop-off. |
| Shape taper and breakup | Yes | Use mesh UV layout plus texture/noise masks and edge bands. The result must avoid a uniform mathematical half-ring. | Close frame inspection shows uneven/tapered edge and non-uniform energy distribution. |
| Layered base/highlight/backing timing | Yes | Use at least three primary layers or renderers: bright highlight, warm/orange body, and darker translucent backing. Layers should differ in lifetime, render order, scale, color, tiling, speed, or erosion curve. | Niagara renderer/emitter evidence and frame range showing layers do not behave identically. |
| Impact/support particles | No | Defer. Candidate later: flare at strike center, stretched directional particles, torus/arc motes, dust/embers, wind/curl/noise, drag. | Not required for the first mechanism checkpoint. |

## 7. Carrier Decision

Use the mesh-rendered arc first.

Reason:

- The strongest slash-specific tutorial uses a custom flat arc mesh with a Niagara mesh renderer.
- The AOE attack is a standalone frontal cleave, not a weapon-attached trail. A ribbon path is useful later for Pierce, Bounce trails, weapon sockets, and support ribbons, but the AOE base shape needs the deliberate control of a mesh arc.
- A mesh arc is easier to inspect for artifact parity than a generated ribbon if the first checkpoint is only carrier plus sweep.
- Trade-off: unlike a ribbon, a mesh arc has no inherent travel. The "slash vs. overlay" read therefore depends on progressive reveal and leading-edge motion. That reveal/sweep animation is load-bearing and must be proven at Checkpoint 1.

Rejected for the next full slash pass unless explicitly approved:

- Static camera-facing sprite crescent.
- Actor-created geometry or actor-arranged point components.
- Generic spark/shockwave systems as the primary readable shape.

## 8. Tooling Feasibility Plan

Before investing in the full effect, prove these tooling seams:

0. Isolation precondition
   - Confirm `/Game/VFXLab` cook exclusion is present before authoring any new lab asset.
   - Run bidirectional AssetRegistry isolation validation before and after authoring: live combat must not reference `/Game/VFXLab`, and `/Game/VFXLab` must not unexpectedly reference live combat assets.
   - If the cook-exclusion or isolation validator is missing or inconclusive, stop and fix the validation seam before authoring effect assets.

1. Mesh source generation/import
   - Generate a project-owned flat arc mesh source for the lab.
   - Import it as a static mesh under `/Game/VFXLab/Hero1Axe/AOE`.
   - Do not use runtime `UProceduralMeshComponent` as the visible slash carrier.

2. Niagara renderer binding
   - Create or refresh a lab Niagara system.
   - Ensure at least one mesh renderer points at `SM_Hero1AxeAOE_SlashArc`.
   - If Python cannot author the renderer safely, use a C++ commandlet or stop and report the tooling blocker.

3. Material graph authoring
   - Create lab-owned additive and translucent materials.
   - Confirm the material graph can express mask, panning, power, erosion, opacity, and ParticleColor/dynamic parameter inputs.
   - Apply the `ArcSlash` mask/material discriminator rules from `Gameplay/Combat/CombatVFXAuthoringProcedure.md`.
   - If a material parameter cannot be driven by Niagara, stop and report the blocker instead of baking a static lookalike.

4. Curve/timing authoring
   - Confirm Niagara-side timing curves can be authored or bound by script/commandlet.
   - Commandlet edits are explicitly in scope for this lab if renderer, lifetime, material binding, fixed bounds, or timing authoring must be corrected to pass Checkpoint 1.
   - If the commandlet still cannot author the needed lifetime/timing behavior after a focused attempt, stop and present the tooling limitation before substituting actor-side motion.

5. Reveal authority
   - The Checkpoint 1 reveal path is material-authoritative: `M_Hero1AxeAOE_Slash_Reveal` must use normalized particle age against the arc mesh U coordinate, or an equivalent material parameter that is still consumed by this material.
   - Niagara may supply age/lifetime/timing and may bind renderer state, but a Niagara-only reveal that bypasses the reveal material does not satisfy the Checkpoint 1 artifact gate.
   - The capture-only lab actor may set Niagara component custom time dilation to make the source emitter's normalized-particle-age reveal inspectable, but it may not create the slash silhouette, assemble slash components, or move actor-side geometry to simulate the sweep.
   - `Scripts/SetupHero1AxeAOELabVFX.py` is intentionally destructive only inside `/Game/VFXLab/Hero1Axe/AOE` and `/Game/VFXLab/Hero1Axe/Shared` generated lab assets. Do not use it for production content.

## 9. Staged Checkpoints

### Checkpoint 1: Carrier And Sweep Only

Goal:

- Prove a mesh-rendered arc carrier exists.
- Prove the slash sweeps from point A to point B.

Allowed:

- Basic unlit material.
- No support particles.
- No final color polish.

Required evidence:

- Cook-exclusion and AssetRegistry isolation validation passed before authoring.
- Asset inspection/validator confirms mesh renderer.
- Unreal-owned MP4 captured with `Scripts/CaptureT66GameplayVideo.ps1`.
- Checkpoint 1 mechanism proof should capture at `FrameRate 24` and `CaptureIntervalSeconds 0.04` so the sub-half-second slash has enough temporal samples for the F/F+3/F+6/F+10 gate without slowing or speeding playback.
- Capture uses the locked gameplay-preview camera from `Scripts/CaptureT66GameplayVideo.ps1 -UseHero1AxePreviewStaging`; do not move the camera to flatter the effect unless the user explicitly approves a new gameplay camera. The default checkpoint camera is arm length `540`, pitch `-30`, pivot height `145`, and `T66.Camera.ConstrainAgainstTowerWalls 0`.
- Checkpoint 1 capture must not add secondary support particles, impact bursts, sparks, dust, embers, hit flashes, or extra layer stacks to mask the primary carrier/reveal. If inherited source-emitter particles remain visible and confuse the carrier read, remove or disable them in the lab asset before accepting the checkpoint.
- Frame range from the gameplay camera shows start, mid-sweep, late-sweep, and dissipation.
- Use the first complete slash cycle visible in the captured frame sequence. Let `F` be the first active frame where the slash is visible and not already complete.
- Arc coverage is judged visually along the final 184-degree arc span in the locked capture frames. The threshold is a mechanism gate, not a polish score.
- Measurement method: inspect the saved PNG frame sequence from the MP4 run and compare the visible luminous carrier length against the final visible carrier length for that same cycle. If visual judgment is ambiguous, count the visible arc by approximate angular sectors across the 184-degree mesh span and document the selected frame numbers in the mechanism close.
- Start frame `F`: visible active arc coverage must be less than 40% of the final arc span.
- Mid frame `F+3` or `F+4`: visible active arc coverage must be between 40% and 80% of the final arc span.
- Late frame `F+6` or `F+7`: visible active arc coverage must be at least 80% of the final arc span.
- Dissipation frame `F+10` or later: the slash must be fading or gone rather than popping from a fully opaque complete crescent.
- Across the start-to-late frame range, the leading edge must advance at least 80 degrees of the 184-degree arc or an equivalent visibly measurable sweep across the target line.
- The first active frame cannot contain the complete crescent. The leading edge must move across the frontal cone.
- Mechanism close may be `PARTIAL`, but must have `Primary carrier archetype`, `A-to-B sweep motion`, and `Progressive arc reveal` as `PRESENT`.
- The material is the sole draw-on reveal authority for Checkpoint 1. Niagara may provide age/lifetime/timing and renderer binding, but the visible growing arc must be produced by `M_Hero1AxeAOE_Slash_Reveal` consuming normalized age against the arc mesh UVs.
- For Checkpoint 1, `Material animation`, `Erosion and dissipation`, `Shape taper and breakup`, and `Layered base/highlight/backing timing` must be reported as `DEFERRED` unless they are actually implemented and evidenced. `DEFERRED` here means intentionally out of this checkpoint, not accepted as final quality.

### Checkpoint 1 Close: 2026-05-27 Carrier/Sweep Diagnostic

Status: `PARTIAL` for the full Hero 1 axe AOE attack, but `PRESENT` for the Checkpoint 1 carrier/sweep/reveal mechanisms.

Implementation:

- `AT66Hero1AxeAOEVFXLabActor` now supports capture-only `-T66Hero1AxeAOEComponentTimeDilation=<float>` and `-T66Hero1AxeAOECycleDuration=<float>` command-line arguments.
- These arguments are allowed only for lab/capture inspection of the Niagara component's normalized-age reveal. They do not create the slash silhouette, do not move actor-side geometry, and do not replace final playback timing approval.
- The primary silhouette remains `/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash` using `/Game/VFXLab/Hero1Axe/Shared/SM_Hero1AxeAOE_SlashArc` and the Niagara mesh renderer/material path.

Evidence:

- Review greenlight: `Saved/AgentReviews/20260526T_hero1_axe_aoe_checkpoint1_pass/20260526T214616-pass1/claude_review_pass1.md`
- Structural validation: `Saved/Logs/Hero1AxeAOE_Checkpoint1_PostSlowCycleValidate.log`
- Video: `Saved/VideoCaptures/Hero1AxeAOE_Checkpoint1_CarrierOnlySlowCycle_20260527_011900/Hero1AxeAOE_Checkpoint1_CarrierOnlySlowCycle.mp4`
- Contact sheet: `Saved/VideoCaptures/Hero1AxeAOE_Checkpoint1_CarrierOnlySlowCycle_20260527_011900/evidence/contact_sheet.png`
- Selected frame notes: `Saved/VideoCaptures/Hero1AxeAOE_Checkpoint1_CarrierOnlySlowCycle_20260527_011900/evidence/selected_frames.md`
- Visibility checklist: `Saved/VideoCaptures/Hero1AxeAOE_Checkpoint1_CarrierOnlySlowCycle_20260527_011900/evidence/visibility_checklist.md`

Selected proof cycle:

- `F=37`: first active frame, visible mark below the 40% start threshold.
- `F+3=40`: mid-sweep partial crescent, visually within the 40-80% range.
- `F+6=43`: late-sweep near-full crescent, above the 80% threshold.
- `F+10=47`: dissipated/gone frame.

Deferred:

- Full real-time playback timing approval.
- Support particles, impact burst, live hitbox/damage sync, final layer polish, production promotion, and visual-fidelity scoring against the American-flag mockup.

### Checkpoint 2: Material Animation And Erosion

Goal:

- Add internal material motion and dissolve.

Required evidence:

- Material graph includes panning/dynamic parameter/erosion.
- Frame range shows internal texture motion and late-life breakup.

### Checkpoint 3: Taper, Breakup, And Layers

Goal:

- Remove uniform half-ring read.
- Add base/highlight/backing layer separation.

Required evidence:

- Frame inspection shows non-uniform edge, color variation, and layer timing differences.

### Checkpoint 4: Optional Support

Goal:

- Add impact/support particles only after primary slash is accepted.

Candidate support:

- Strike flare.
- Stretched particles moving in slash tangent direction.
- Short-lived embers/dust.
- Faint ground skim or light scar.

## 10. Anti-Lookalike Test

Cheap wrong result:

- A static half-moon crescent appears over the enemies and disappears.

Required discriminator:

- The accepted slash must visibly travel from point A to point B and erode/dissipate over time.
- The accepted slash must show spatial displacement of the leading edge and progressive reveal of the active arc. Opacity stages of a fully formed crescent are not sufficient.
- A complete crescent that only rotates rigidly, scales, fades in, or fades out is the lookalike, not a pass.

Proof rule:

- One still image cannot pass.
- The capture must use the gameplay camera angle and include a frame range with at least start, mid-sweep, late sweep, and dissipation frames.
- If the result would still pass with motion and erosion removed, the gate is too weak and the result is `PARTIAL`.

## 10.1 Damage Hitbox Alignment

The slash VFX is presentation, not combat authority.

This close is the current worked reference for `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`: Hero 1 AOE is a band-anchored irregular VFX whose readable slash carrier and query-only DamageVolume are proven together, while combat logic remains authoritative.

Authoritative Hero 1 AOE damage uses the combat component's logical query path:

- primary target damage still routes through `FT66CombatTargetHandle` and `ApplyDamageToTargetHandle`,
- secondary targets use a query-only 180-degree frontal sector around the primary impact point,
- the sector is oriented from the hero attack origin toward the primary target,
- broad phase uses a pawn overlap around the impact point,
- narrow phase rejects candidates outside the sector angle or radius, and rejects candidates inside the hollow center when the weapon row has `AoeInnerRadiusRatio > 0`,
- secondary hits must preserve `HasUnblockedAutoAttackPath(...)`,
- debug proof must show the sector DamageVolume separately from the VFX.

Do not use Niagara collision, the slash render mesh, per-poly visual mesh collision, or material opacity as the damage authority.

Acceptance evidence:

- sector debug draw visible with `T66.Combat.DebugView=2` or `3`,
- boundary test proves an enemy inside the half-angle is hit and an enemy outside/behind is not,
- wall-blocking test proves secondary sector hits do not pass through dungeon walls,
- damage still reaches enemy/boss hit-zone handlers through combat target handles.
- `-T66GameplayAutoCapture=hero1axeaoehitbox` stages the isolated boundary proof, equips the Hero 1 black AOE weapon, fires one real combat AOE through the combat component, and logs `[Hero1AxeAOEHitboxProof]` HP deltas for fixed inside/outside targets.
- The hitbox proof may spawn the current `/Game/VFXLab` Niagara slash as presentation-only evidence aligned to the combat impact sector. This does not make the VFX actor, mesh, material, opacity, or Niagara collision a damage authority.
- The hitbox proof may echo visible floating combat text, short-lived debug number strings, and on-screen proof messages from the measured HP delta after the real combat hit. This is proof feedback only; the authoritative damage result remains the combat-component attack and target HP delta.

### 10.2 Hitbox Close: 2026-05-27 Candidate03 North Aura

Status: `PRESENT` for the lab hitbox/damage proof. The VFX remains presentation-only; damage authority came from the real Hero 1 black AOE combat-component attack.

Evidence:

- Gameplay video: `Saved/VideoCaptures/Hero1AxeAOE_Candidate03_NorthAuraHitbox_20260527_Clean50/Hero1AxeAOE_Candidate03_NorthAuraHitbox_Clean50.mp4`
- Contact sheet: `Saved/VideoCaptures/Hero1AxeAOE_Candidate03_NorthAuraHitbox_20260527_Clean50/evidence/contact_sheet.png`
- Evidence manifest: `Saved/VideoCaptures/Hero1AxeAOE_Candidate03_NorthAuraHitbox_20260527_Clean50/evidence/manifest.json`
- Visibility review: `Saved/VideoCaptures/Hero1AxeAOE_Candidate03_NorthAuraHitbox_20260527_Clean50/evidence/visibility_checklist.md`
- Durable log excerpt: `Saved/VideoCaptures/Hero1AxeAOE_Candidate03_NorthAuraHitbox_20260527_Clean50/evidence/hitbox_proof_log_excerpt.md`
- Runtime log source: `Saved/Logs/T66.log`

Logged boundary proof:

- `Primary`, `InsideForward`, and `InsideSide` were spawned with `ExpectedHit=1`.
- `OutsideBehind` and `OutsideBackSide` were spawned with `ExpectedHit=0`.
- Log lines 1061-1068 show `DamageNumber` events for the three expected-hit targets only, HP deltas for all three hits, and `Result=PASS` for all five expected hit/miss checks.

Visual proof:

- The gameplay contact sheet uses frame 46 for Codex's visible slash assessment aligned to the debug AOE sector and frames 46 and 49 for floating damage-number confirmation.
- The VFX is perceptible in the locked gameplay camera for this lab proof. Final production polish still requires Pablo sign-off.

### 10.3 Production Binding Close: 2026-05-28 Hero 1 AOE Backend Wiring

Status: `PRESENT` for the first Hero 1 axe AOE production-binding seam. `PARTIAL` for the full attack family pipeline because future weapons, idol overlays, and final visual-polish gates are not implemented by this close.

Implemented:

- `Content/Data/CombatVFXBindings.csv` row `Hero1Axe_AOE_Base` binds source ID `Hero_1_black_aoe` and attack category `AOE` to `/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash`.
- `Content/Data/DT_CombatVFXBindings.uasset` is generated/refreshed by `Scripts/SetupCombatVFXBindingsDataTable.py`.
- `BP_T66GameInstance` has its `CombatVFXBindingsDataTable` assigned by the setup script, and `UT66GameInstance` exposes binding lookup.
- `UT66CombatComponent` resolves the production binding when Hero 1 black AOE fires, suppresses the temporary projectile only when the production binding succeeds, and logs `CombatVFXProductionSpawned`.
- AOE damage, speed, and scale item secondary stats now feed the effective damage, fire interval, hitbox radius, and production VFX scale/playback values used by the attack.
- Strong speed scaling is split from visual readability: the raw fire-interval-derived multiplier is logged, while the slash visual playback is clamped to preserve at least a `0.20s` expected visual duration.

Verification:

- Build succeeded: `T66Editor Win64 Development`, 2026-05-28.
- Production binding validator succeeded: `Scripts/ValidateCombatVFXProductionBindings.py`, 2026-05-28.
- Runtime proof summary: `Saved/VideoCaptures/Hero1AxeAOE_VFXBindingProof_20260528_001540/Hero1AxeAOEVFXBindingProofSummary.md`.
- Baseline proof: `Saved/VideoCaptures/Hero1AxeAOE_VFXBindingProof_20260528_001540/Baseline/proof_log_excerpt.md`.
- AOE scale proof: `Saved/VideoCaptures/Hero1AxeAOE_VFXBindingProof_20260528_001540/AoeScale/proof_log_excerpt.md`.
- AOE speed proof: `Saved/VideoCaptures/Hero1AxeAOE_VFXBindingProof_20260528_001540/AoeSpeed/proof_log_excerpt.md`.
- AOE damage proof: `Saved/VideoCaptures/Hero1AxeAOE_VFXBindingProof_20260528_001540/AoeDamage/proof_log_excerpt.md`.

Proof highlights:

- All four proof cases use real combat firing through `hero1axeaoevfxbinding`; no manual lab VFX actor is spawned.
- Baseline logs `CombatVFXProductionSpawned Binding=Hero1Axe_AOE_Base`, production Niagara path, `EffectiveSlashRadius=437.52`, `VisualScale=1.006`, and `EffectiveDamagePerShot=28`.
- AOE scale proof logs `Item_AoeScale`, `AoeScaleValue=5.146`, `EffectiveSlashRadius=1740.86`, and `VisualScale=4.002`.
- AOE speed proof logs `Item_AoeSpeed`, `AoeSpeedValue=5.600`, and `EffectiveFireIntervalSeconds=0.173`. Gameplay cadence is not clamped here; only the presentation playback is clamped from `RawVisualPlaybackMultiplier=5.768` to `VisualPlaybackMultiplier=2.300` so the expected visual duration remains `0.200s`.
- AOE damage proof logs `Item_AoeDamage`, `AoeDamageValue=4.370`, and `EffectiveDamagePerShot=121`. The non-round-number ratio is expected from internal base damage precision and final integer damage rounding.
- Every proof case logs expected-hit targets passing and expected-miss targets remaining unhit.
- The scale case's `AoeScaleValue=5.146` resolves to roughly a 4x production visual and hitbox scale because the runtime multiplier removes already-applied hero/global baseline scale before applying the category-specific item multiplier.
- Deterministic proof item grants are limited to the `hero1axeaoevfxbinding` automation path in `T66PlayerController_Overlays.cpp`; they are not a normal-run item grant path.
- `Scripts/ValidateCombatVFXProductionBindings.py` is expected to fail non-zero if the binding row, production Niagara asset, GameInstance DataTable assignment, or `/Game/VFXLab` isolation guard drifts.

Limitations:

- This close proves backend wiring and production dispatch. It is not a new final visual-fidelity approval.
- The proof capture contact sheets are useful backend evidence, but visual-polish acceptance still belongs to the same-view MRQ gate plus gameplay-camera review.
- Future idol-driven VFX modifiers should use the binding/effect-packet seam rather than modifying the temporary projectile placeholder system.

### 10.4 Crescent-Band Hitbox Close: 2026-05-28 Hero 1 AOE

Status: `PRESENT` for the current Hero 1 black AOE visual/logical hitbox alignment. `PARTIAL` for the full VFX family pipeline because item-stat confirmation through normal acquisition, future weapon bindings, idol overlays, and final visual-polish gates remain future work.

Implemented:

- `FWeaponData::AoeInnerRadiusRatio` defines an optional hollow center for AOE rows. `0.0` keeps existing filled-sector behavior.
- `Scripts/SetupWeaponsDataTable.py` writes `AoeInnerRadiusRatio=0.54` only for `Hero_1_black_aoe`; all other AOE weapon rows currently write `0.00`.
- `UT66CombatComponent` passes `EffectiveSlashInnerRadius` through the Hero 1 frontal-sector target query and production VFX spawn log.
- `Hero1Axe_AOE_Base` uses `BaseVisualRadius=411.4` so its production visual scale matches the runtime outer damage radius after the current weapon stat multiplier.
- `-T66GameplayAutoCapture=hero1axeaoevfxbinding` stages explicit band-boundary proof targets: `Primary`, `InsideBandForward`, `InsideBandSide`, `InsideAngleEdge`, `InnerHollow`, `OutsideAngleEdge`, `OutsideBehind`, and `OutsideRadius`.

Verification:

- Build succeeded: `T66Editor Win64 Development`, 2026-05-28.
- Weapons DataTable reload succeeded: `Scripts/SetupWeaponsDataTable.py`, 2026-05-28.
- Combat VFX binding DataTable reload succeeded: `Scripts/SetupCombatVFXBindingsDataTable.py`, 2026-05-28.
- Production binding validator succeeded: `Scripts/ValidateCombatVFXProductionBindings.py`, 2026-05-28.
- Gameplay video: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/Hero1AxeAOE_HitboxCleanup.mp4`.
- Contact sheet: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/evidence/contact_sheet.png`.
- Evidence manifest: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/evidence/manifest.json`.
- Visibility review: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/evidence/visibility_checklist.md`.
- Runtime log source: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/T66.log`.

Logged proof highlights:

- Production spawn logged `EffectiveSlashRadius=437.52`, `EffectiveSlashInnerRadius=236.26`, `AoeInnerRadiusRatio=0.540`, `BaseVisualRadius=411.40`, and `VisualScale=1.063`.
- Expected-hit targets `Primary`, `InsideBandForward`, `InsideBandSide`, and `InsideAngleEdge` all logged `ActualHit=1` and `Result=PASS`.
- Expected-miss targets `InnerHollow`, `OutsideAngleEdge`, `OutsideBehind`, and `OutsideRadius` all logged `ActualHit=0` and `Result=PASS`.
- Frame 62 in the final contact sheet shows the visual slash and the red crescent-band DamageVolume together, including the hollow center and the near-sector-edge target layout. The VFX remains presentation-only; the combat component query remains the damage authority.

Limitations:

- The `hero1axeaoevfxbinding` proof harness grants deterministic proof items only when explicitly requested. This close did not re-prove normal item pickup/acquisition flow.
- Future visual-polish passes should not redesign this AOE in place before the item/stat and idol-overlay backend seams are checked.
- Future idol overlays should be layered through reviewed combat VFX binding/effect-packet infrastructure, not the old temporary projectile presentation path.

## 11. Development Slow Preview

The lab commandlet supports a diagnostic slow mode for Niagara editor inspection:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=T66Hero1AxeAOEVFX -T66Hero1AxeAOEDevSlowFactor=6 -unattended -nop4 -nosplash
```

This flag rebuilds `/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash` with scaled diagnostic timing while preserving the same Niagara emitters, mesh renderer, sprite renderers, materials, and support emitters. It is for visibility and screenshot capture only; do not treat the slowed asset as production timing approval.

Run the same commandlet without `-T66Hero1AxeAOEDevSlowFactor` to restore the default lab timing.

## 12. Verification Commands

Expected setup/build/validation commands for the implementation pass:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=pythonscript -script='C:\UE\T66\Scripts\SetupHero1AxeAOELabVFX.py' -unattended -nop4 -nosplash

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=T66Hero1AxeAOEVFX -unattended -nop4 -nosplash

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReloadFromIDE

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=pythonscript -script='C:\UE\T66\Scripts\ValidateHero1AxeAOELabVFX.py' -unattended -nop4 -nosplash

# The validator must include cook-exclusion and bidirectional AssetRegistry isolation checks for /Game/VFXLab before implementation is accepted.

powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66NiagaraMRQIsolation.ps1 `
  -OutputDir C:\UE\T66\Saved\VFXResearch\Hero1Axe\AOE_AmericanFlagVisualTarget\EditorIsolation\<timestamp> `
  -ResX 1400 -ResY 1400 `
  -OrthoWidth 1250

& 'C:\UE\T66\Scripts\CaptureT66GameplayVideo.ps1' -UseHero1AxePreviewStaging -Output '<output mp4>' -FrameCount 120 -FrameRate 24 -CaptureIntervalSeconds 0.04 -DelaySeconds 7.0 -PostCaptureDelaySeconds 0.25 -TimeoutSeconds 260 -RemoveFrames:$false

& 'C:\Users\DoPra\.local\bin\ffprobe.cmd' -v error -select_streams v:0 -show_entries stream=width,height,nb_frames,r_frame_rate,duration -of default=nw=1:nk=1 '<output mp4>'
```

The editor-isolation route exists and is verified. Candidate03 has now satisfied same-view comparison for the requested north-facing orientation, simpler aura read, red/blue band placement, centered white impact, opaque black review background, and full square framing. Future full-production claims still require Pablo sign-off on the captured result and any additional gameplay readability polish.

Implementation may adjust script names, but it must keep equivalent setup, build, validator, editor-isolation capture, gameplay capture, ffprobe, and frame-inspection evidence.

## 13. Mechanism Close Template

Use this close format for every checkpoint:

```text
MECHANISM CLOSE
Mechanism: Primary carrier archetype
Status: PRESENT/ABSENT/DEFERRED
Evidence:

Mechanism: A-to-B sweep motion
Status: PRESENT/ABSENT/DEFERRED
Evidence:

Mechanism: Progressive arc reveal
Status: PRESENT/ABSENT/DEFERRED
Evidence:

Mechanism: Material animation
Status: PRESENT/ABSENT/DEFERRED
Evidence:

Mechanism: Erosion and dissipation
Status: PRESENT/ABSENT/DEFERRED
Evidence:

Mechanism: Shape taper and breakup
Status: PRESENT/ABSENT/DEFERRED
Evidence:

Mechanism: Layered base/highlight/backing timing
Status: PRESENT/ABSENT/DEFERRED
Evidence:

Discriminator test:
Reported status: FULL/PARTIAL
```

## 14. Approval Gate

Do not start the next implementation until this packet is accepted or revised.

If implementation starts, Checkpoint 1 is the first deliverable. It should be judged only on carrier plus A-to-B sweep. Reporting it as `PARTIAL` is expected unless all required mechanisms are present.
