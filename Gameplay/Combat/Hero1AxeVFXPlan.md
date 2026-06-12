# Hero 1 Axe VFX Plan

**Last updated:** 2026-05-25
**Scope:** Step 0 planning document for isolated Hero 1 axe-themed combat VFX research and lab authoring.
**Status:** Hero 1 AOE now has production binding, item-stat backend proof, and crescent-band hitbox proof for `Hero_1_black_aoe`. Final visual-polish approval remains separate. DOT, Summon, and Bounce are infrastructure-only scaffolds until their own packets approve real effects.

## Current Status Snapshot - 2026-05-28

Hero 1 AOE has moved beyond the old static-crescent lab prototype for backend purposes: `Hero1Axe_AOE_Base` binds `Hero_1_black_aoe` to the promoted production Niagara system, and the combat component owns item-stat scaling plus crescent-band hitbox authority. This validator proves Combat VFX binding structure, required assets, source guards, and declared data contracts. It does not prove visual fidelity, temporal mechanism quality, final player-facing readability, or Pablo visual approval.

Current status by attack category:

- `AOE`: production binding and hitbox backend proof exist; final visual-polish approval remains separate.
- `DOT`: active production binding row (`Hero1Axe_DOT_Base` → `Hero_1_black_dot`) bound to the moving aura-ring carrier Niagara system; final visual-polish approval remains separate.
- `RetiredLine`: infrastructure-only packet exists; no active production binding row or VFX asset.
- `Bounce`: infrastructure-only packet exists; no active production binding row or VFX asset.
- Idol overlays: architecture-only seam; no active idol overlay rows or assets.

## 1. Working Goal

Build a production-quality isolated VFX lab process for four Hero 1 axe-themed attack visuals before integrating anything into live combat.

The first targets are the four weapon altar attack categories:

- `AOE`: frontal 180-degree half-moon axe slash.
- `DOT`: aura axe that hits a target and remains spinning/orbiting while damage ticks.
- `RetiredLine`: straight horizontal slash or fissure along a line.
- `Bounce`: spirit/aura axe silhouettes that chain from one enemy to another.

The visual style target is ethereal axe aura: readable axe silhouettes, bright edge energy, clear slash arcs, and a shared supernatural material language across all four attacks.

## 2. Current Boundaries

Except for the approved Step 1 AOE lab prototype, this plan intentionally does not modify:

- current red/blue placeholder projectile VFX,
- `FT66TemporaryProjectileSystem` profiles,
- `UT66WeaponManagerSubsystem` combat behavior,
- Hero/idol data rows,
- Blueprint or Niagara assets,
- staged standalone builds.

These VFX should be built and validated in isolation first. Production integration is a later, separately approved phase.

The approved AOE lab prototype may add:

- `/Game/VFXLab/Hero1Axe` lab-only material assets,
- `/Game/VFXLab/Hero1Axe` lab-only Niagara assets,
- `/Game/VFXLab` cook-exclusion config,
- explicit capture-only runtime plumbing,
- setup/validation scripts under `Scripts`,
- Unreal-owned screenshot evidence for visual review.

## 3. Hard Gates Before Building

Do not start Step 1 research implementation or asset authoring until these gates are resolved:

- Confirm Hero 1 weapon identity.
  - The current user direction is axe-themed VFX.
  - Before building, decide whether this is a spear-to-axe redesign, an alternate variant, or throwaway R&D.
- Confirm `/Game/VFXLab` is free and reserved for isolated VFX lab assets.
- Confirm lab assets under `/Game/VFXLab` are excluded from shipping cooks before any assets are authored there.
- Confirm raw research bundles stay under `Saved/VFXResearch/Hero1Axe` and not under `Content`.
- Confirm any copied, duplicated, adapted, or seeded asset has a license that permits adaptation and intended shipped game use.
- Confirm tutorial videos are used for learning techniques only unless their assets are explicitly licensed for reuse.

Resolution for the approved AOE lab prototype:

- Hero 1 axe is treated as isolated R&D only, not an official spear-to-axe production data change.
- `/Game/VFXLab` is reserved for lab assets and must stay cook-excluded.
- Raw research remains under `Saved/VFXResearch/Hero1Axe`.
- The 2026-05-24 AOE crescent-sprite prototype is isolated R&D and is `PARTIAL`, not a complete slash. It proved the static-mask path is insufficient because it lacks a moving carrier, material animation, erosion, texture breakup, and multi-frame A-to-B motion.
- The next accepted AOE slash pass must satisfy the mechanism manifest in Section 4.1 before it can be described as a slash implementation. Tutorials are technique references only.
- The old procedural slash/weapon-layer path is not approved for this pass. The slash silhouette must be inspectable in the Niagara asset, renderer material, renderer mesh/ribbon, or emitter logic, not assembled by C++ procedural geometry or actor-arranged point components.

### 3.1 Canonical Research Source Selection

This section is the authoritative Hero 1 axe source-selection record. Effect packets and dated research notes should point here instead of copying this decision.

Current canonical implementation source:

- `Unreal Engine 5 - Sword Slash VFX - Niagara Tutorial` by Gabriel Aguiar: `https://www.youtube.com/watch?v=djlnnPvFR0Q`
- Role: primary method/mechanism reference for the AOE half-moon slash and a strong implementation seed for lane-style slash behavior.
- Why selected: existing research marks it P0 for AOE because it demonstrates the closest solved process class: an empty Niagara system, custom flattened arc model, mesh renderer, short slash lifetime, mesh orientation/rotation force, dynamic material parameters, panning/tiling, power/erosion, additive bright layer, warm body layer, dark translucent backing layer, and impact stack.
- Replication boundary: copy the method and mechanisms, not tutorial-owned assets. Values from the tutorial remain `observed` starting points unless the effect packet marks them `inferred` or `tuned`.

Supporting technique references:

- `Create a Multi-Layer Ribbon Trail VFX in UE5 Niagara`: P0 material/edge/layer reference for bright core, edge bands, color progression, fade, and shared ethereal material behavior.
- `Weapon Trail Effect In Unreal Engine Niagara`: P0 support/trail reference for dynamic parameters, erosion/wipe timing, low-opacity glow, sparks, and later Bounce/DOT trail accents.
- `Anatomy Of A Slash`: P2 visual anatomy/reference-quality source for base, highlight, support detail, impact, movement, and dissipation readability.

New video/transcript collection is not the default next step. If an adherence/tooling audit marks a required mechanism or artifact as `MISSING SOURCE DETAIL`, ask Pablo for the needed transcript or source artifact.

## 4. Proposed Isolation Layout

Tracked planning notes:

- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/Hero1AxeVFXPlan.md`
- `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`

Untracked research cache:

- `Saved/VFXResearch/Hero1Axe/notegpt_transcripts/` for Pablo-provided NoteGPT transcript files.
- `Saved/VFXResearch/Hero1Axe/<source-id>/notes.md`
- `Saved/VFXResearch/Hero1Axe/<source-id>/keyframes/` only when frames or screenshots are provided by Pablo or captured through an approved source-evidence path.
- Historical generated local extraction folders are not current source truth. They were quarantined under `Saved/VFXResearch/Hero1Axe/_historical_extraction_outputs/20260525_transcript_extraction_artifacts/`.

Future Unreal lab assets, only after cook exclusion is verified:

- `/Game/VFXLab/Hero1Axe/AOE`
- `/Game/VFXLab/Hero1Axe/DOT`
- `/Game/VFXLab/Hero1Axe/RetiredLine`
- `/Game/VFXLab/Hero1Axe/Bounce`
- `/Game/VFXLab/Hero1Axe/Shared`

Current AOE artifact manifest and completeness status:

| Artifact/category | Role | Required for full slash | Current path/status | Acceptance evidence |
|---|---|---:|---|---|
| Niagara primary slash carrier | Primary | Yes | `/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash` is the current lab carrier and remains `PARTIAL` until capture proves A-to-B sweep, reveal, erosion, and layer behavior. | Full acceptance requires a Niagara mesh-rendered arc or ribbon-style time path, or an explicitly approved equivalent that visibly sweeps from point A to point B. |
| Slash arc mesh | Primary | Yes | `/Game/VFXLab/Hero1Axe/Shared/SM_Hero1AxeAOE_SlashArc` is the current project-owned carrier mesh. | Full acceptance requires role inspection plus capture evidence that the mesh/material combination avoids a uniform static half-ring. |
| Slash renderer material | Primary | Yes | `/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_Slash_Reveal` is the current reveal material and remains `PARTIAL` until material animation, non-uniform breakup, color/layer variation, and erosion are evidenced. | Full acceptance requires texture/noise/taper breakup plus material animation and erosion/dissipation over particle lifetime. |
| Slash mask texture | Primary support | Yes | `/Game/VFXLab/Hero1Axe/Shared/T_Hero1AxeAOE_StreakMask` is the current streak/mask texture support. | Full acceptance requires proof that the mask contributes to non-uniform shape or internal streak/color variation, not only asset presence. |
| Runtime one-shot timing | Primary | Yes | `AT66Hero1AxeAOEVFXLabActor` is `PARTIAL` because it only activates/scales the full shape. | Full acceptance requires Niagara/material-driven reveal, active window, and dissipation with multi-frame evidence. |
| Secondary support sparks/shockwave | Secondary | No | Deferred | May be added only after the primary crescent slash reads correctly. |

### 4.1 Required AOE Mechanism Manifest

The next AOE implementation must satisfy this mechanism manifest before it can be called a slash. These are required behaviors, not optional polish. Completion is `FULL` only when every `Required: Yes` mechanism is `PRESENT` with evidence.

Generic carrier archetype assignment, mask/material manifests, parameter-sweep discriminators, anti-bake rules, and mask/material close requirements are owned by `Gameplay/Combat/CombatVFXAuthoringProcedure.md`. This section owns the AOE-specific mechanism requirements and acceptance evidence.

| Mechanism | Required | Planned implementation class | Evidence needed |
|---|---:|---|---|
| Primary carrier archetype | Yes | Niagara mesh renderer using a flat arc slash mesh, or a Niagara ribbon/path carrier that samples a sweep over time. A static camera-facing sprite is not sufficient unless the user explicitly approves it as a temporary lookalike. | Asset inspection showing mesh/ribbon renderer or equivalent carrier, plus capture frames showing the carrier has spatial orientation and thickness appropriate to the slash. |
| A-to-B sweep motion | Yes | Mesh rotation force/orientation curve, ribbon samples over time, or equivalent Niagara-side motion with leading-edge travel across the frontal cone. A complete crescent that only rotates rigidly, scales, or fades does not satisfy this mechanism. | Niagara module/curve evidence and a frame range showing the leading edge at measurably different angular/spatial positions. The full arc must not be present in the first active frame. |
| Progressive arc reveal | Yes | Radial/length-wise UV reveal, dynamic material parameter, mesh UV mask, or equivalent normalized-age draw-on so the active arc is carved on from point A to point B. UV panning alone is not enough. | Material/Niagara evidence and frame range showing the active arc growing along its path instead of a fully formed crescent appearing at once. |
| Material animation | Yes | Dynamic material parameters, UV panning, flipbook, or equivalent lifetime-driven material motion. | Material graph/node evidence plus frame range showing internal motion, not only actor scale, reveal, or visibility. |
| Erosion and dissipation | Yes | Erosion/noise/opacity curve over normalized lifetime so the slash breaks apart or dissolves instead of popping off. | Material graph and Niagara parameter/curve evidence, plus late-life frames showing dissolution. |
| Shape taper and breakup | Yes | Tapered texture mask, noise/Voronoi breakup, non-uniform edge bands, or an arc mesh UV layout that avoids a perfectly uniform geometric half-ring. | Texture/material/mesh evidence and close frame inspection showing non-uniform slash edges. |
| Layered base and highlight timing | Yes | Base, highlight, and optional dark/backing layers with distinct lifetime, color, scale, blend, tiling, or erosion behavior. | Niagara emitter/renderer/layer evidence and frame range showing different layer behavior. |
| Impact and support particles | No | Impact flare, stretched directional particles, lingering motes, wind/curl/noise, drag, and secondary glows. | Deferred until the primary carrier, sweep, and dissipation pass visual review. |

Anti-lookalike discriminator:

- Cheap wrong result: a static half-moon crescent appears over enemies and disappears.
- Required discriminator: the accepted slash must visibly travel from point A to point B, progressively reveal the active arc, and erode/dissipate over time. A single still image cannot prove this; use an Unreal-owned MP4 plus representative frame range from the gameplay camera.
- A complete crescent that only rotates rigidly, scales, fades in, or fades out is the lookalike, not a pass.

Forbidden AOE primary carriers:

- `P_Hero1AxeAOE_WeaponSlashSeed` or `P_Hero1AxeAOE_ShockwaveSeed` duplicated from `P_Weapon_03` / `P_Shockwave_Expl_03`,
- generic spark/distortion/shockwave systems as the only or primary visible shape,
- `UProceduralMeshComponent`, actor-side static mesh layers, or C++ arc/lattice geometry as the slash silhouette source,
- actor-arranged point Niagara components where the actor transform math, rather than the Niagara asset/material, creates the crescent.
- a static camera-facing sprite crescent as the accepted full AOE slash, unless the user explicitly approves it as a partial placeholder or lookalike checkpoint.

Promotion into production combat paths must be explicit and later. Lab assets should not become live references by accident.

## 5. Research Source Rules

### 5.1 Tutorial Videos

Tutorials are valid sources for:

- Niagara module structure,
- material graph ideas,
- spawn/lifetime/velocity values shown on screen,
- curve shapes,
- beam, ribbon, mesh, and sprite setup patterns,
- alignment and camera-facing tricks,
- authoring workflow order,
- performance warnings.

Tutorials are not valid sources for copying:

- textures,
- meshes,
- flipbooks,
- exact curves,
- paid pack assets visible in the tutorial,
- creator project files without a reuse license.

If the tutorial shows a value, record it as `observed`. If a value is inferred from visual timing or shape, record it as `inferred`. If it is a lab-tuned value, record it as `tuned`.

### 5.2 Free Unreal/Fab/Epic Samples

Free assets should be inspected before paid assets, but free does not mean shippable.

For each candidate, record:

- source name and URL,
- license summary,
- whether adaptation and shipping are allowed,
- whether Niagara systems and materials are editable,
- whether effects are real systems or mostly baked flipbooks,
- which Hero 1 effect they could inform,
- which specific technique is useful.

### 5.3 Paid Packs

Buying a pack is useful only if it fills a specific gap found after free/tutorial research.

A paid pack is a good candidate when:

- it includes editable Niagara systems, materials, curves, meshes, and textures,
- it is close to the target ethereal slash/aura style,
- it can teach or seed at least two of the four Hero 1 effects,
- its license permits adaptation and intended shipped game use,
- it is not just a collection of baked video-like flipbooks.

Bought assets should be duplicated into lab space and treated as reference or seed material. They should not be wired directly into combat.

## 6. Source Evidence Intake And Claude Review

Do not use local video transcript, caption, or video-source extraction for this process.

For any YouTube or video source that needs transcript evidence:

- use a transcript path Pablo explicitly provides in the request or thread,
- or use a Pablo-provided transcript already stored in this effect packet's named folder: `Saved/VFXResearch/Hero1Axe/notegpt_transcripts/`,
- otherwise ask Pablo for the transcript.

The existing generated local extraction-output folders under `Saved/VFXResearch/Hero1Axe` are historical artifacts only and are not current source truth. They were quarantined under `Saved/VFXResearch/Hero1Axe/_historical_extraction_outputs/20260525_transcript_extraction_artifacts/`.

For each useful video, create a research note with:

- what effect type it informs,
- timestamped implementation notes,
- frame references,
- observed values,
- inferred values,
- uncertainty notes,
- what should be tested in Unreal.

Frames are evidence for visual anatomy and visible settings. They do not guarantee hidden exact math values. Hidden values must be treated as starting guesses and tuned in the lab.

### 6.1 Claude Advisory Review

After a Pablo-provided transcript/source evidence bundle exists, send it to Claude for advisory analysis through the local Claude Code CLI.

Before every Claude run:

- Verify `ANTHROPIC_API_KEY` is unset in Process/User/Machine.
- Use the local Claude Code CLI only, not Anthropic API billing.

Evidence packet contents:

- video metadata,
- Pablo-provided transcript/source evidence paths,
- selected keyframe or screenshot files when available,
- timestamps for every referenced frame,
- Codex first-pass notes,
- the `observed`, `inferred`, and `tuned` uncertainty labels defined in Section 5.1,
- targeted questions for whichever effect the source informs: `AOE`, `DOT`, `RetiredLine`, or `Bounce`.

Default review cadence:

- Run one Claude review per video first.
- After 3-5 videos have individual reviews, optionally run a batch synthesis review across those source notes.

Claude output is advisory. Codex remains responsible for implementation decisions and must reconcile every Claude recommendation as:

- `accepted`,
- `rejected` with evidence,
- `needs Unreal lab validation`.

Ask Claude to distinguish evidence-backed facts from guesses, cite the transcript segment or keyframe behind each claim, identify missing timestamps/frames worth inspecting, and flag any license or asset-copying risk. DOT attachment and Bounce chaining must remain marked visual-only until real combat integration proves them.

## 7. Four Effect Targets

### 7.1 AOE Half-Moon Slash

Goal:

- A frontal 180-degree ethereal axe sweep.
- Reads as a broad axe slash, not a sword slash or generic fire arc.
- Covers a half-circle in front of the hero.
- Current visual target is an AOE-specific American-flag-inspired crescent: red and blue energy bands on a broad half-moon axe slash, with a white impact position at enemy contact.
- This color identity is specific to the AOE target and does not redefine the shared ethereal axe language for DOT, RetiredLine, or Bounce.
- The target still requires non-uniform multi-shape slash carrier behavior, multi-color body/core/edge variation, and a visible impact spot.
- Before the target can receive a `FULL` visual-acceptance claim, the generated mockup or contact sheet must be saved to a repo path and Pablo must explicitly approve that exact saved variant.

Likely ingredients:

- Niagara mesh-rendered arc or ribbon/path carrier authored inside Niagara/material assets,
- visible A-to-B sweep motion driven by Niagara modules, renderer orientation, ribbon sampling, or material/mesh timing,
- lifetime-driven material animation with UV panning, dynamic parameters, flipbook, or equivalent,
- erosion/dissipation over lifetime,
- tapered texture/noise breakup so the shape is not a uniform half-ring,
- layered base/highlight/backing timing,
- emissive edge material,
- secondary sparks or shards at leading edge,
- optional ground skim or faint shock trace.

Validation:

- recognizable from a still image,
- recognizable as a slash over a frame range, not just as a static crescent in one still,
- frame sequence shows point A to point B motion and late-life dissipation,
- readable at gameplay camera distance,
- visible hit window,
- not so bright that enemies disappear.

### 7.2 DOT Spinning Aura Axe

Goal:

- An axe aura strikes a target and remains attached or orbiting while DOT ticks.
- The visual communicates "ongoing damage" without covering the target completely.

Likely ingredients:

- small axe silhouette or arc mesh,
- orbiting or spinning transform,
- pulsing aura material,
- tick pulse accent,
- short ambient trail.

Validation:

- clear stuck/orbiting state,
- readable tick pulse,
- target remains visible,
- attachment fidelity is not final until real combat integration.

### 7.3 RetiredLine Horizontal Slash/Fissure

Goal:

- A straight horizontal slash that travels or appears along a lane.
- Should feel like an axe force wave or fissure, not a bullet.

Likely ingredients:

- elongated slash mesh or ribbon strip,
- directional material panning,
- impact sparks at the leading edge,
- optional ground fissure/decal if it improves readability.

Validation:

- direction is immediately readable,
- line length and width are clear,
- does not look like a normal projectile beam unless intentionally approved.

### 7.4 Bounce Spirit Axe Silhouettes

Goal:

- Spirit/aura axe silhouettes chain from enemy to enemy.
- It should imply bounce behavior with a visible hop path.

Likely ingredients:

- axe silhouette mesh or sprite,
- curved trail between points,
- impact flash on each contact,
- fade/reform between hops,
- optional ghost duplicates for speed.

Validation:

- bounce path is readable,
- each hop has a clear hit moment,
- chain behavior is not final until real combat targeting/pathing integration.

## 8. Recommended Build Order

1. `AOE`
   - Establishes the shared axe aura material and broad slash language after the shared material research packet and editor-isolation gate are in place.
   - Easiest to judge from still captures.
2. `DOT`
   - Reuses the axe aura style and de-risks spinning/attached presentation early.
3. `RetiredLine`
   - Reuses the slash language in a straight-line lane shape.
4. `Bounce`
   - Most combat-pathing-specific, so it should come after the visual language is accepted.

## 9. Draft Acceptance Rubric

An isolated visual pass is acceptable only if:

- the effect reads as axe-themed within 1 second,
- every required mechanism in the relevant mechanism manifest is `PRESENT` or the result is explicitly reported as `PARTIAL`,
- the effect has a clear start, active hit window, and dissipation,
- temporal mechanisms have multi-frame evidence; still-frame evidence alone cannot prove motion, reveal, erosion, or dissipation,
- the anti-lookalike discriminator is named and proven,
- the main shape remains readable at gameplay camera distance,
- the effect does not hide enemies or damage feedback,
- the four effects share a coherent ethereal axe language,
- the effect has Unreal-owned screenshot or capture evidence,
- visual acceptance includes frame inspection: validator-green is necessary but not sufficient,
- the effect has source notes linking major choices to tutorial frames, inspectable assets, or lab tuning,
- active visual-target work passes the same-view editor-isolation gate when the owning packet requires it,
- the effect has AssetRegistry isolation proof before promotion,
- any seed asset has license-to-adapt and license-to-ship notes.

Visual-only lab acceptance does not prove final combat correctness. DOT attachment and Bounce chaining remain explicitly unproven until they are wired into real targeting and pathing.

## 10. Draft Performance Budget

This budget is provisional and needs user sign-off before becoming a hard requirement.

Measurement plan:

- Compare against a no-effect lab baseline.
- Prefer existing T66 PerformanceSystem output if it can be invoked for the lab scene.
- If not, add a lab-only frame-time JSON/CSV capture path during Step 0.5.

Initial targets:

- no obvious hitch above 50 ms during spawn/despawn,
- average frame delta increase no more than roughly 2 ms over baseline for representative use,
- no sustained visible stutter in capture.

Initial concurrency assumptions, pending real combat spawn caps:

- `AOE`: 1-2 active.
- `DOT`: up to 8 active target attachments.
- `RetiredLine`: 1-3 active.
- `Bounce`: up to 3 chains with 5 hops each.

These numbers must be checked against real T66 combat spawn caps before production integration.

## 11. Step 0.5 Instrumentation Plan

Step 0.5 is approved only for the first AOE lab prototype. It remains unapproved for DOT, Summon, Bounce, live combat, and production promotion.

Potential Step 0.5 work:

- Add `-T66GameplayAutoCapture=vfxlab` to the existing gameplay automation capture path.
- Keep the mode no-op unless the explicit command-line value is present.
- Prefer editor/development-only behavior or shipping-inert behavior.
- Add an AssetRegistry isolation check:
  - fail if live assets reference `/Game/VFXLab`,
  - fail if `/Game/VFXLab` unexpectedly references live combat assets,
  - allow only approved shared dependencies.
- Verify cook exclusion for `/Game/VFXLab` before authoring assets there.
- Add or verify frame-time measurement for lab captures.

Normal gameplay must behave identically when `-T66GameplayAutoCapture=vfxlab` is absent.

## 12. Open Decisions

- Is Hero 1 officially changing from spear/lane guard identity to axe identity?
- Should the isolated lab use `/Game/VFXLab/Hero1Axe`, or another reserved content namespace?
- Which cook-exclusion field is correct for this UE 5.7 project?
- Can the existing PerformanceSystem emit per-frame deltas for a lab scene?
- Which first tutorials and free editable sample packs are worth analyzing?
- At what visual quality level should a paid pack purchase become justified?

## 13. Next Action

The research pass for the first AOE prototype has been completed through `Saved/VFXResearch/Hero1Axe/source_matrix_2026-05-24.md`, `Saved/VFXResearch/Hero1Axe/notegpt_analysis_2026-05-24.md`, and the Claude transcript reviews under `Saved/AgentReviews`.

The current process-development next action is to lock the review infrastructure and material-language plan before further AOE visual polish:

1. Review and accept or revise `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`.
2. Implement or verify a durable same-view editor-isolation capture route before making an editor gate mandatory for handoff.
3. Update the AOE packet with the canonical editor view, zoom/framing rule, artifact paths, and mismatch rubric.
4. Use the approved account-backed imagegen path through a separate local Codex CLI worker to generate a same-view VFX-only target after the editor framing is fixed.
5. Use `Gameplay/Combat/Hero1AxeSharedAuraMaterialResearchPlan.md` to select and review the shared aura material source before locking material behavior.
6. Add cook/config isolation for `/Game/VFXLab` if it is not already present.
7. Author or generate a flat arc slash mesh or Niagara ribbon/path carrier under `/Game/VFXLab/Hero1Axe/AOE`.
8. Author original slash material assets under `/Game/VFXLab/Hero1Axe/Shared` with texture/noise breakup, material animation, and erosion/dissipation.
9. Add Niagara timing curves for A-to-B sweep motion, active hit window, and layer fade.
10. Add capture-only runtime plumbing for `-T66GameplayAutoCapture=hero1axeaoe`.
11. Validate bidirectional AssetRegistry isolation.
12. Capture the effect through the editor-isolation gate and Unreal-owned gameplay automation, then inspect frame ranges for the mechanism manifest and anti-lookalike discriminator.

For the remaining three effects, the next non-runtime action is still a research pass:

1. Pick 3-5 slash/aura/Niagara tutorials with implementation detail.
2. Ask Pablo for transcripts for any video source that lacks a usable Pablo-provided transcript.
3. Use Pablo-provided screenshots/keyframes/clips or approved source evidence when visual details are missing.
4. Inspect free editable Unreal/Fab/Epic sample content.
5. Produce a source matrix before buying any paid pack.

No additional VFX should be authored in `/Game/VFXLab` until the cook-exclusion and isolation checks are in place and passing for the AOE prototype.

## Superseded Historical Notes

Older references to the 2026-05-24 static crescent prototype remain useful as history: that prototype was `PARTIAL`, proved that static-mask/lookalike paths were insufficient, and should not be treated as the current AOE backend state. The current AOE baseline is documented in `Hero1AxeAOESlashMechanismPacket.md`, `CombatVFXInfrastructureInventory.md`, and `VFX_PROCESS_INDEX.md`.
