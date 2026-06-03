You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

The first non-empty line of your review must be exactly one of these four lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: NEEDS_HUMAN_DECISION
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Verdict meanings:
- APPROVE: the reviewed plan/output is safe for Codex to proceed to implementation under the reviewed scope. Codex should not ask for redundant manual user approval after APPROVE unless the user explicitly marked the work planning-only, asked Codex to stop before implementation, the packet has an unresolved user-only decision, or AGENTS/PPF requires explicit approval for a method substitution.
- REVISE: Codex can resolve the issue by improving the plan/output, inspecting more repo state, tightening verification, changing implementation approach, or otherwise doing more Codex-owned work. Codex should revise and rerun review.
- NEEDS_HUMAN_DECISION: the plan/output depends on product direction, vision, risk acceptance, scope choice, or another decision only the user can make. Codex should save a decision block, ask once, and stop until the user answers.
- BLOCK: the plan/output cannot safely proceed because of a hard blocker, missing prerequisite, external-state issue, unavailable credential/context, or contradiction that is not solved by normal Codex revision.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\ProjectileVFXPipelineDiscovery\projectile_vfx_pipeline_discovery_report.md
- Output scope: review of the packet below only.

<review_packet>
# T66 Projectile/VFX Pipeline Discovery Report

Working task:
Operator: Codex
Validator: Claude
Scope: Read-only investigation of the current projectile/VFX content pipeline, Chad 1 AOE Black teardown, Niagara authoring mechanism, imagegen integration, templating/reuse, current VFX inventory, and per-projectile cost surfaces. No code, data, asset, config, content, or save changes beyond this report artifact.
Stop condition: Answer Groups A-H with file:line and asset-path evidence.

## Current Counts

- Active production Combat VFX weapon rows: 4, all for Hero 1 / Chad 1 black-tier weapon attacks: AOE, Pierce, Bounce, DOT. Evidence: `Content/Data/CombatVFXBindings.csv:2-5`.
- Active production Combat VFX idol rows: 0 in `CombatVFXBindings.csv`; the current active rows are all `SourceType=WeaponBase`. Evidence: `Content/Data/CombatVFXBindings.csv:2-5`; the VFX process index says idol overlays are architecture/proof placeholder paths only, with no approved production idol Niagara rows (`Gameplay/Combat/VFX_PROCESS_INDEX.md:28-32`).
- Chad 1 AOE Black production system: 1 Niagara system, 1 slash arc mesh, 3 slash mesh emitters, 4 support sprite emitters, 3 source mask textures, 3 slash materials, 4 support materials. Evidence: AOE layer configs (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:82-117`), support configs (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:136-198`), required assets (`Scripts/ValidateCombatVFXProductionBindings.py:60-76`).
- Current enemy/boss projectile manager capacity: 512 managed projectile slots. Evidence: `Source/T66/Gameplay/T66ProjectileManagerSubsystem.h:127-134`.

## Group A - End-to-end pipeline overview

### A1. Stages from concept to in-game

1. Process selection and effect packet routing.
   - Combat VFX tasks start from `Gameplay/Combat/VFX_PROCESS_INDEX.md`, which routes tasks to the Combat VFX authoring procedure and per-effect packets (`Gameplay/Combat/VFX_PROCESS_INDEX.md:7-19`).
   - The accepted authoring procedure defines visual-target gates, carrier archetypes, material order, Niagara construction order, capture, and validation (`Gameplay/Combat/CombatVFXAuthoringProcedure.md:68-87`, `Gameplay/Combat/CombatVFXAuthoringProcedure.md:225-265`, `Gameplay/Combat/CombatVFXAuthoringProcedure.md:430-503`).

2. Optional imagegen visual target.
   - For repo-bound VFX mockups, the process uses a separate local Codex CLI worker and account-backed imagegen, not API-key scripts (`Gameplay/Combat/CombatVFXAuthoringProcedure.md:68-80`).
   - Chad 1 AOE Black has a project-bound four-panel visual target under `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/`; the worker used built-in account-backed Codex `image_gen` and produced a prompt, contact sheet, and result note (`Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/worker_result.md:3-16`).
   - The generated target is visual direction, not implementation. The process explicitly says source evidence and the effect packet own mechanisms, masks, materials, timing, and verification gates, while the mockup owns composition, color balance, silhouette intent, layer readability, impact placement, and style target (`Gameplay/Combat/CombatVFXAuthoringProcedure.md:72-77`).

3. Source texture/mask generation.
   - Chad 1 AOE Black source masks are generated by `Scripts/GenerateHero1AxeAOETextures.py`, which writes to `C:\UE\T66\SourceAssets\VFX\Hero1Axe\AOE` at 512x128 (`Scripts/GenerateHero1AxeAOETextures.py:1-12`).
   - The script generates `T_Hero1AxeAOE_StreakMask.png`, `T_Hero1AxeAOE_DissolveNoise.png`, and `T_Hero1AxeAOE_ImpactMask.png` (`Scripts/GenerateHero1AxeAOETextures.py:103-113`).

4. Unreal Python setup for textures/materials.
   - `Scripts/SetupHero1AxeAOELabVFX.py` imports those masks, builds AOE slash and support materials, and prepares the Niagara package slot (`Scripts/SetupHero1AxeAOELabVFX.py:1-10`, `Scripts/SetupHero1AxeAOELabVFX.py:25-39`, `Scripts/SetupHero1AxeAOELabVFX.py:735-746`).
   - It switches between lab and production paths through `-T66Hero1AxeAOEProduction` or the `T66_HERO1_AXE_AOE_TARGET=production` environment variable (`Scripts/SetupHero1AxeAOELabVFX.py:25-34`).

5. C++ commandlet authoring of meshes and Niagara systems.
   - AOE: `UT66Hero1AxeAOEVFXCommandlet` creates `/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash`, the shared slash arc mesh, emitters, modules, and renderers; production paths are selected with `-T66Hero1AxeAOEProduction` (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:42-66`, `Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:808-859`, `Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:1063-1325`).
   - Pierce, Bounce, and DOT have their own C++ commandlets and production path flags while reusing the shared Hero 1 AOE material vocabulary (`Source/T66/Gameplay/T66Hero1AxePierceVFXCommandlet.cpp:41-70`, `Source/T66/Gameplay/T66Hero1AxeBounceVFXCommandlet.cpp:41-78`, `Source/T66/Gameplay/T66Hero1AxeDOTVFXCommandlet.cpp:41-80`).

6. Production binding registration.
   - `Content/Data/CombatVFXBindings.csv` contains the active binding rows (`Content/Data/CombatVFXBindings.csv:1-5`).
   - `Scripts/SetupCombatVFXBindingsDataTable.py` owns reload from CSV into `/Game/Data/DT_CombatVFXBindings` and assignment to `/Game/Blueprints/Core/BP_T66GameInstance` (`Scripts/SetupCombatVFXBindingsDataTable.py:1-6`, `Scripts/SetupCombatVFXBindingsDataTable.py:17-35`, `Scripts/SetupCombatVFXBindingsDataTable.py:121-157`, `Scripts/SetupCombatVFXBindingsDataTable.py:191-222`).

7. Runtime resolution and spawn.
   - `UT66GameInstance` defaults the Combat VFX DataTable to `/Game/Data/DT_CombatVFXBindings.DT_CombatVFXBindings` (`Source/T66/Core/T66GameInstance.cpp:165-169`) and resolves rows by source type, source ID, and attack category (`Source/T66/Core/T66GameInstance.cpp:890-920`).
   - `UT66CombatComponent::ResolveCombatVFXBinding` loads the bound Niagara system; `TrySpawnBoundWeaponBaseSlashVFX` computes transform/scale/playback and spawns one Niagara component with `ENCPoolMethod::AutoRelease` for the direct bound weapon path (`Source/T66/Gameplay/T66CombatComponent.cpp:1060-1097`, `Source/T66/Gameplay/T66CombatComponent.cpp:1116-1294`).

8. Validation and proof capture.
   - `Scripts/ValidateCombatVFXProductionBindings.py` validates binding structure, assets, source guards, and data contracts, but does not prove final visual fidelity (`Scripts/ValidateCombatVFXProductionBindings.py:1-9`, `Scripts/ValidateCombatVFXProductionBindings.py:21-26`, `Scripts/ValidateCombatVFXProductionBindings.py:314-373`).
   - `Scripts/CaptureT66NiagaraMRQIsolation.ps1` uses Unreal command-line Movie Render Queue with `Scripts/SetupT66NiagaraMRQIsolation.py`, then writes `actual.png`, `actual_crop.png`, `contact_sheet.png`, `mismatch_notes.md`, and `manifest.json` (`Scripts/CaptureT66NiagaraMRQIsolation.ps1:1-10`, `Scripts/CaptureT66NiagaraMRQIsolation.ps1:209-279`, `Scripts/CaptureT66NiagaraMRQIsolation.ps1:287-343`).

### A2. Automated vs manual, local vs external

- Manual/process-gated stages: effect-packet interpretation, visual-target approval, visual acceptance, and any statement that an effect has final player-facing visual approval. The procedure states the mockup approval fields that must be recorded before it becomes binding (`Gameplay/Combat/CombatVFXAuthoringProcedure.md:81-87`).
- Automated local stages: source mask generation by Python, Unreal Python material/texture setup, C++ Unreal commandlets for mesh/Niagara construction, DataTable reload, production binding validator, MRQ isolation capture, and gameplay capture (`Scripts/GenerateHero1AxeAOETextures.py:103-113`, `Scripts/SetupHero1AxeAOELabVFX.py:1-10`, `Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:1063-1325`, `Scripts/SetupCombatVFXBindingsDataTable.py:191-222`, `Scripts/ValidateCombatVFXProductionBindings.py:1-9`, `Gameplay/Combat/CombatVFXAuthoringProcedure.md:154-165`).
- External/RunPod/chadpocalypse-gpu: in the current Combat VFX pipeline files inspected for this report, the projectile/VFX pipeline is local to `C:\UE\T66` plus account-backed Codex imagegen for mockups. The combat VFX imagegen path explicitly says account-backed Codex/imagegen and no API scripts (`Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/codex_cli_imagegen_worker_prompt.md:5-13`). No current projectile/VFX stage inspected here is driven by `chadpocalypse-gpu`.

### A3. Asset locations and folder conventions

- Source mask PNGs: `SourceAssets/VFX/Hero1Axe/AOE/T_Hero1AxeAOE_StreakMask.png`, `T_Hero1AxeAOE_DissolveNoise.png`, `T_Hero1AxeAOE_ImpactMask.png` (`Scripts/GenerateHero1AxeAOETextures.py:103-113`).
- Lab assets: `/Game/VFXLab/Hero1Axe/...`; current config excludes `/Game/VFXLab` from cooking (`Config/DefaultGame.ini:13`).
- Production assets: `/Game/VFX/Hero1/Axe/AOE`, `/Game/VFX/Hero1/Axe/Pierce`, `/Game/VFX/Hero1/Axe/Bounce`, `/Game/VFX/Hero1/Axe/DOT`, and `/Game/VFX/Hero1/Axe/Shared`. `/Game/VFX` is always cooked (`Config/DefaultGame.ini:27`).
- Imported vendor/idol fallback effects: `/Game/Stylized_VFX_StPack/...`, always cooked (`Config/DefaultGame.ini:22`) and referenced by idol VFX lookup code (`Source/T66/Gameplay/T66CombatVFX.cpp:434-450`).
- Binding data: CSV source at `Content/Data/CombatVFXBindings.csv`; runtime DataTable at `/Game/Data/DT_CombatVFXBindings` (`Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md:7-18`, `Scripts/SetupCombatVFXBindingsDataTable.py:17-35`).
- Research/proof artifacts: `Saved/VFXResearch/...` for VFX research, mockups, and editor-isolation captures (`Gameplay/Combat/CombatVFXAuthoringProcedure.md:142-165`).

## Group B - Chad 1 AOE Black full teardown

### B1. Assets that compose Chad 1 AOE Black

Production-bound row:

- `Hero1Axe_AOE_Base`, SourceType `WeaponBase`, SourceID `Hero_1_black_aoe`, AttackCategory `AOE`, Niagara `/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.NS_Hero1AxeAOE_MeshSlash`, profile `MeshSlashAOE`, `bSuppressTemporaryProjectile=True`, BaseVisualRadius `411.4`, BasePlaybackSeconds `0.46` (`Content/Data/CombatVFXBindings.csv:2`).

Production assets:

- Niagara system: `Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset` (`Scripts/ValidateCombatVFXProductionBindings.py:37-39`, `Scripts/ValidateCombatVFXProductionBindings.py:60-62`).
- Mesh: `Content/VFX/Hero1/Axe/Shared/SM_Hero1AxeAOE_SlashArc.uasset` (`Scripts/ValidateCombatVFXProductionBindings.py:60-63`).
- Slash materials: `Content/VFX/Hero1/Axe/Shared/M_Hero1AxeAOE_Slash_Bright.uasset`, `M_Hero1AxeAOE_Slash_Body.uasset`, `M_Hero1AxeAOE_Slash_Dark.uasset` (`Scripts/ValidateCombatVFXProductionBindings.py:63-66`).
- Source textures as production uassets: `Content/VFX/Hero1/Axe/Shared/T_Hero1AxeAOE_StreakMask.uasset`, `T_Hero1AxeAOE_DissolveNoise.uasset`, `T_Hero1AxeAOE_ImpactMask.uasset` (`Scripts/ValidateCombatVFXProductionBindings.py:67-69`).
- Support materials: `Content/VFX/Hero1/Axe/Shared/M_Hero1AxeAOE_ImpactFlare.uasset`, `M_Hero1AxeAOE_DirectionalSpark.uasset`, `M_Hero1AxeAOE_Mote.uasset`, `M_Hero1AxeAOE_GroundTrace.uasset` (defined in setup config at `Scripts/SetupHero1AxeAOELabVFX.py:124-168`; bound to sprite renderers at `Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:1238-1277`).

Niagara emitters:

- Mesh slash emitters: `Emitter_AxeAOESlash_Bright`, `Emitter_AxeAOESlash_Body`, `Emitter_AxeAOESlash_Dark` (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:82-117`).
- Support sprite emitters: `Emitter_AxeAOEImpact_Flare`, `Emitter_AxeAOESupport_DirectionalSparks`, `Emitter_AxeAOESupport_Motes`, `Emitter_AxeAOESupport_GroundTrace` (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:136-198`).

### B2. Niagara system structure

- The commandlet creates a new `UNiagaraSystem`, initializes it with `UNiagaraSystemFactoryNew::InitializeSystem`, adds the three slash layer emitters, then adds four support emitters unless carrier-only diagnostic mode is requested (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:808-859`).
- Slash emitters are CPU-sim, local-space, deterministic, fixed-bounds emitters (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:433-458`).
- Each slash layer uses `EmitterState`, `SpawnBurst_Instantaneous` with spawn count 1, `SystemLocation`, `InitialMeshRotation`, particle lifetime/scale/color parameters, `UpdateAge`, `DynamicMaterialParameters`, `MeshRotationForce`, and `SolveRotationalForcesAndVelocity` (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:471-660`).
- The material dynamic parameter `SlashAge` is defined in the slash material and is driven from `Particles.NormalizedAge` through the Niagara dynamic material parameter node (`Scripts/SetupHero1AxeAOELabVFX.py:437-441`, `Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:551-593`).
- The three slash emitters bind mesh renderers to `SM_Hero1AxeAOE_SlashArc` with per-layer material overrides (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:1189-1235`).
- The four support emitters bind sprite renderers with support materials, facing/alignment settings, and view-depth sorting (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:1238-1277`).
- Current AOE runtime spawn does not set effect-specific `User.*` parameters; it sets spawn location, rotation, scale, auto-release pooling, custom time dilation, and translucent sort priority (`Source/T66/Gameplay/T66CombatComponent.cpp:1228-1255`). Bounce and DOT attached carriers do set `User.Color`, `User.Tint`, and `Color` on their carrier components (`Source/T66/Gameplay/T66CombatComponent.cpp:1560-1577`).

### B3. Visual components in the exemplar

Chad 1 AOE Black is built from these layered components:

- Primary broad crescent carrier: non-uniform static mesh arc generated by the commandlet. The mesh has 96 segments with interpolated inner/outer radii, ripple, tangent offsets, and two triangles per segment (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:874-920`, `Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:962-1011`).
- Bright slash layer: blue additive highlight, 0.32s lifetime, inner-biased material settings (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:84-94`, `Scripts/SetupHero1AxeAOELabVFX.py:40-68`).
- Body slash layer: red additive body, 0.38s lifetime, outer-biased material settings (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:95-105`, `Scripts/SetupHero1AxeAOELabVFX.py:69-96`).
- Dark slash layer: translucent backing/contrast layer, 0.46s lifetime (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:106-116`, `Scripts/SetupHero1AxeAOELabVFX.py:97-123`).
- Localized impact flare, directional sparks, motes, and ground trace support emitters (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:136-198`).
- Material behaviors: panned dissolve UV, streak mask, dissolve noise, impact mask, reveal/fade, width softness/power, radial bias, core/tip/impact color banding, emissive glow, and opacity (`Scripts/SetupHero1AxeAOELabVFX.py:429-500`, `Scripts/SetupHero1AxeAOELabVFX.py:551-668`).

### B4. Runtime wiring to the weapon

- The row `Hero1Axe_AOE_Base` binds `Hero_1_black_aoe` to the AOE Niagara system and suppresses the temporary projectile visual (`Content/Data/CombatVFXBindings.csv:2`).
- The row struct is presentation-only; combat passes already-resolved context into VFX, and the row does not override gameplay (`Source/T66/Data/T66DataTypes.h:37-42`).
- `PerformSlash` builds the AOE damage/impact context with damage center, impact point, forward direction, radius, inner radius, sector half-angle, and hit target handles, then publishes that context and calls `TrySpawnBoundWeaponBaseSlashVFX` (`Source/T66/Gameplay/T66CombatComponent.cpp:2345-2407`).
- `TrySpawnBoundWeaponBaseSlashVFX` resolves the binding, computes AOE center/band/path/impact anchoring, scale, playback multiplier, and expected duration, then spawns the Niagara component with `UNiagaraFunctionLibrary::SpawnSystemAtLocation` and `ENCPoolMethod::AutoRelease` (`Source/T66/Gameplay/T66CombatComponent.cpp:1116-1294`).
- The temporary weapon projectile lane is skipped when the binding says to suppress it (`Source/T66/Gameplay/T66CombatComponent.cpp:1099-1114`, `Source/T66/Gameplay/T66CombatComponent.cpp:3250-3255`).

### B5. Body form, sim target, and imagegen texture use

- The projectile body for Chad 1 AOE Black is a mesh-rendered Niagara carrier, not a flipbook. Three CPU-sim slash emitters render the commandlet-built `SM_Hero1AxeAOE_SlashArc` mesh, and four support emitters render sprites (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:433-458`, `Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:1189-1277`).
- The generated imagegen contact sheet is not imported as a flipbook/sprite/material texture for the current AOE effect. The implemented runtime material uses generated project-owned masks from `SourceAssets/VFX/Hero1Axe/AOE`, imported as `StreakMask`, `DissolveNoise`, and `ImpactMask` (`Scripts/GenerateHero1AxeAOETextures.py:103-113`, `Scripts/SetupHero1AxeAOELabVFX.py:453-479`).

## Group C - Niagara authoring mechanism

### C1. How agents create and edit Niagara systems

- Python Unreal API is used for source texture import, material graph construction, and lab/production path setup. The script's own run command invokes UnrealEditor-Cmd with `-run=pythonscript` (`Scripts/SetupHero1AxeAOELabVFX.py:1-10`).
- C++ Unreal commandlets create/edit Niagara systems, emitters, modules, static meshes, and renderers. AOE creates a `UNiagaraSystem`, adds emitters through Niagara editor APIs, builds the static mesh, binds renderers, compiles, and saves (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:808-859`, `Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:874-1011`, `Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:1063-1325`).
- Data/binding registration is scripted through `Scripts/SetupCombatVFXBindingsDataTable.py`, which enforces CSV rows and fills the DataTable from CSV (`Scripts/SetupCombatVFXBindingsDataTable.py:36-99`, `Scripts/SetupCombatVFXBindingsDataTable.py:169-222`).
- The current mechanism is not a text/intermediate representation. It is direct Unreal asset manipulation through Unreal Python and C++ editor APIs.

### C2. What can be changed by the mechanism

- Texture imports and texture settings: source file, destination, sRGB, streaming, mip generation, filtering, and compression settings (`Scripts/SetupHero1AxeAOELabVFX.py:264-331`).
- Material graphs: expressions, texture parameters, vector/scalar parameters, blend mode, unlit shading, two-sided rendering, Niagara sprite/mesh usage flags, emissive/base color/opacity connections (`Scripts/SetupHero1AxeAOELabVFX.py:429-668`, `Scripts/SetupHero1AxeAOELabVFX.py:675-727`).
- Niagara system/emitter/module structure: new emitters, `EmitterState`, burst spawn, initial rotation, parameter modules, dynamic material parameter modules, rotation forces, solvers, and support emitters (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:433-660`, `Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:808-859`).
- Static mesh geometry: the AOE slash arc mesh is generated procedurally in the commandlet (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:874-1011`).
- Renderer bindings: mesh renderers and sprite renderers are removed/replaced and rebound with materials and renderer options (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:1189-1277`).

### C3. Limits and failure modes already documented

- Dynamic parameter wiring can fail if scripts assume named pins that are not present, so graph inspection or supported masks/components are required before claiming a parameter is driven (`Gameplay/Combat/CombatVFXAuthoringProcedure.md:571-572`).
- If an emitter, renderer, module, or binding cannot be authored by the supported Unreal/Niagara editor APIs, the documented process is to report a tooling blocker instead of substituting actor-side geometry (`Gameplay/Combat/CombatVFXAuthoringProcedure.md:572-574`).
- Structurally valid assets can render incorrectly if scale, orientation, or material bindings are not actually consumed (`Gameplay/Combat/CombatVFXAuthoringProcedure.md:573-574`).
- Headless import can assert/stall, and stale texture assets can persist after PNG regeneration (`Gameplay/Combat/CombatVFXAuthoringProcedure.md:574-576`).
- Structural validation is not visual acceptance; the procedure explicitly separates asset/binding existence from reveal, sweep, erosion, color variation, impact placement, and readability (`Gameplay/Combat/CombatVFXAuthoringProcedure.md:577-580`).

### C4. Base template or authored from scratch

- AOE creates a new Niagara system from an initialized empty system and then adds project-specific emitters/modules/renderers (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:808-859`).
- Pierce, Bounce, and DOT also create new systems from `UNiagaraSystemFactoryNew::InitializeSystem` and then add their own emitters (`Source/T66/Gameplay/T66Hero1AxePierceVFXCommandlet.cpp:472-499`, `Source/T66/Gameplay/T66Hero1AxeBounceVFXCommandlet.cpp:472-505`, `Source/T66/Gameplay/T66Hero1AxeDOTVFXCommandlet.cpp:409-438`).
- The reused template surface is not a cloned Niagara asset; it is shared material language, commandlet structure, and per-type geometry/renderer setup. The authoring procedure says shared weapon/element families should share material roles, parameter names/drivers, and evidence rules while allowing carrier shape, color palette, scale, timing, impact/support emitters, and hitbox-alignment presentation to vary (`Gameplay/Combat/CombatVFXAuthoringProcedure.md:461-486`).

## Group D - Imagegen integration

### D1. Prompt to runtime texture/sprite flow

- Current Chad 1 AOE Black imagegen flow: prompt -> saved project-bound contact sheet -> visual target/reference only. The worker prompt forbids Niagara/source/content/script edits and stops before Unreal implementation (`Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/codex_cli_imagegen_worker_prompt.md:5-13`, `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/codex_cli_imagegen_worker_prompt.md:28-34`).
- The imagegen output is a four-panel temporal contact sheet; it is not the runtime texture source (`Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/final_imagegen_prompt.md:9-18`, `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/final_imagegen_prompt.md:20-36`).
- Runtime textures for the AOE material come from deterministic mask generation in `Scripts/GenerateHero1AxeAOETextures.py`, then import through `Scripts/SetupHero1AxeAOELabVFX.py` (`Scripts/GenerateHero1AxeAOETextures.py:103-113`, `Scripts/SetupHero1AxeAOELabVFX.py:264-331`).

### D2. Scripts and output conventions

- Imagegen orchestration for this exemplar was a local Codex CLI worker using account-backed `image_gen`; the retained generated source image was copied into the project-bound VFX research folder (`Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/worker_result.md:5-16`).
- The AOE source masks are 512x128 grayscale PNGs under `SourceAssets/VFX/Hero1Axe/AOE` (`Scripts/GenerateHero1AxeAOETextures.py:10-12`, `Scripts/GenerateHero1AxeAOETextures.py:103-113`).
- Unreal import output is under lab or production `/Game/VFX...` paths according to the setup script target mode (`Scripts/SetupHero1AxeAOELabVFX.py:25-39`, `Scripts/SetupHero1AxeAOELabVFX.py:280-331`).

## Group E - Materials, shaders, retro aesthetic

### E1. Projectile materials

- Chad 1 AOE Black uses bespoke-but-shared Hero 1 axe materials: 3 slash layer materials and 4 support sprite materials. The setup script defines additive bright/body materials, a translucent dark slash backing, and additive/translucent support materials (`Scripts/SetupHero1AxeAOELabVFX.py:40-168`).
- The slash material graph uses unlit shading, configured blend mode, two-sided rendering, dynamic material parameter `SlashAge`, `StreakMask`, `DissolveNoise`, `ImpactMask`, reveal/fade, radial bias, core/tip/impact color parameters, emissive, and opacity (`Scripts/SetupHero1AxeAOELabVFX.py:429-668`).
- Support materials are unlit, Niagara-sprite capable, texture-mask driven, and fade by `SlashAge` (`Scripts/SetupHero1AxeAOELabVFX.py:675-727`).
- Pierce, Bounce, and DOT reuse the shared Hero 1 AOE slash-layer material family but use distinct carrier geometry and runtime presentation (`Source/T66/Gameplay/T66Hero1AxePierceVFXCommandlet.cpp:68-70`, `Source/T66/Gameplay/T66Hero1AxeBounceVFXCommandlet.cpp:75-78`, `Source/T66/Gameplay/T66Hero1AxeDOTVFXCommandlet.cpp:76-80`).

### E2. Retro/post-FX interaction

- Retro FX is a separate runtime subsystem that can apply PS1, N64 blur, outline, chromatic aberration, real/low fake resolution, geometry, and pixelation settings (`Source/T66/Core/T66RetroFXSettings.h:19-92`, `Source/T66/Core/T66RetroFXSettings.h:187-230`).
- The subsystem owns post-process material paths and variant PS1 material paths (`Source/T66/Core/T66RetroFXSubsystem.cpp:44-55`, `Source/T66/Core/T66RetroFXSubsystem.cpp:159-197`), ensures an unbound post-process volume and weighted blendables (`Source/T66/Core/T66RetroFXSubsystem.cpp:688-786`, `Source/T66/Core/T66RetroFXSubsystem.cpp:808-864`, `Source/T66/Core/T66RetroFXSubsystem.cpp:1801-1848`), and applies PS1 parameters such as dithering, color boost, and fog (`Source/T66/Core/T66RetroFXSubsystem.cpp:895-926`).
- Projectile VFX materials themselves are authored as unlit Niagara materials with additive/translucent layers. The report found no current per-projectile material hook that bypasses or specially opts into the Retro FX post-process stack; they render through the world as Niagara/mesh/sprite materials and any active world post-process/pixelation applies at runtime.

## Group F - Templating, reuse, parameterization

### F1. Shared vs unique surfaces

- Shared: Hero 1 weapon VFX reuse the AOE red/blue/white slash material vocabulary and source masks. Pierce, Bounce, and DOT explicitly reuse the shared AOE slash-layer materials while differentiating carrier geometry and runtime placement (`Source/T66/Gameplay/T66Hero1AxePierceVFXCommandlet.cpp:68-70`, `Source/T66/Gameplay/T66Hero1AxeBounceVFXCommandlet.cpp:75-78`, `Source/T66/Gameplay/T66Hero1AxeDOTVFXCommandlet.cpp:76-80`).
- Unique per type: carrier mesh, Niagara system path, emitter names, bounds, lifetime/scale defaults, and runtime presentation model. Pierce uses a vertical blade plane and PathAnchored scaling; Bounce uses a compact horizontal slash and moving link carrier; DOT uses an aura ring transported by a visual-only shot (`Source/T66/Gameplay/T66Hero1AxePierceVFXCommandlet.cpp:41-70`, `Source/T66/Gameplay/T66Hero1AxeBounceVFXCommandlet.cpp:41-78`, `Source/T66/Gameplay/T66Hero1AxeDOTVFXCommandlet.cpp:41-80`, `Source/T66/Gameplay/T66CombatComponent.cpp:1480-1621`, `Source/T66/Gameplay/T66CombatComponent.cpp:2538-2713`).
- The current scalable reuse model is "shared material language plus per-type commandlet-authored carriers," not a single parameter-only Niagara template.

### F2. Adjustment knobs that currently differentiate projectiles

- Binding/data knobs: `BaseVisualRadius`, `BasePlaybackSeconds`, and `VisualScaleMultiplier` in `FT66CombatVFXBindingData` (`Source/T66/Data/T66DataTypes.h:75-82`) and active rows (`Content/Data/CombatVFXBindings.csv:2-5`).
- Runtime transform/playback knobs: location, rotation, uniform or non-uniform scale, visual anchor model, custom time dilation, and sort priority (`Source/T66/Gameplay/T66CombatComponent.cpp:1171-1255`).
- Material knobs in the AOE setup script: base/core/tip/impact colors, dissolve pan, streak/dissolve/detail strengths, reveal lead/sharpness, fade start/sharpness, width softness/power, core/tip band sharpness, impact strength/alpha, glow strength, opacity boost, and radial bias (`Scripts/SetupHero1AxeAOELabVFX.py:40-123`, `Scripts/SetupHero1AxeAOELabVFX.py:551-668`).
- Niagara/renderer knobs: per-layer lifetime, spawn scale, particle color, renderer scale, rotation force, lever radius, support emitter spawn count, position, velocity, sprite size, alignment, facing, and sort hint (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:69-198`).

## Group G - Current VFX inventory: finished vs placeholder/scaffold

The statuses below are current-state labels from project docs and binding rows, not quality judgments.

| Item | Current state | Evidence |
|---|---|---|
| Hero 1 AOE / Chad 1 AOE Black | Active production binding row and current exemplar. Final visual-polish approval remains separate from structural validation. | `Content/Data/CombatVFXBindings.csv:2`; `Gameplay/Combat/VFX_PROCESS_INDEX.md:28`; validator scope at `Scripts/ValidateCombatVFXProductionBindings.py:21-26`. |
| Hero 1 Pierce | Active structural implementation packet and active production row; final `FULL` visual-fidelity claim is not approved by the packet. | `Content/Data/CombatVFXBindings.csv:3`; `Gameplay/Combat/Hero1AxePierceMechanismPacket.md:3-4`; `Gameplay/Combat/Hero1AxePierceMechanismPacket.md:58-63`. |
| Hero 1 Bounce | Active implementation packet and active production row; visible primary silhouette is the authored Bounce Niagara slash on a visual-only moving projectile; final `FULL` visual-fidelity result is not claimed. | `Content/Data/CombatVFXBindings.csv:4`; `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md:5-8`; `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md:51-55`. |
| Hero 1 DOT | Active production binding row with aura-ring carrier transported by a single hero-to-target visual shot; three target-following sphere applicator markers remain placeholders; final visual polish is deferred. | `Content/Data/CombatVFXBindings.csv:5`; `Gameplay/Combat/Hero1AxeDOTMechanismPacket.md:3-15`; `Gameplay/Combat/Hero1AxeDOTMechanismPacket.md:26-31`. |
| Idol overlays | Architecture plus impact-context contract only. No active production idol Niagara rows are approved by the current baseline. Idol VFX paths use imported pack effects or placeholder/fallback paths. | `Gameplay/Combat/VFX_PROCESS_INDEX.md:32`; imported idol paths at `Source/T66/Gameplay/T66CombatVFX.cpp:434-450`; bound-idol lookup/spawn path at `Source/T66/Gameplay/T66CombatVFX.cpp:1147-1242`; placeholder/fallback category routing at `Source/T66/Gameplay/T66CombatVFX.cpp:1319-1618`. |

Scaffolding pass surfaces currently present:

- The `FT66CombatVFXBindingData` row struct supports `WeaponBase` and `IdolModifier`, Niagara system, effect packet ID, profile, suppression, fallback, radius, playback, and visual scale (`Source/T66/Data/T66DataTypes.h:31-85`).
- Four active Hero 1 black-tier weapon binding rows exist in CSV/DataTable setup script (`Scripts/SetupCombatVFXBindingsDataTable.py:36-99`).
- Runtime can resolve bound weapon and idol VFX rows, suppress temporary weapon visuals, spawn bound weapon Niagara, and attempt bound idol-impact Niagara (`Source/T66/Gameplay/T66CombatComponent.cpp:1060-1114`, `Source/T66/Gameplay/T66CombatComponent.cpp:1116-1294`, `Source/T66/Gameplay/T66CombatVFX.cpp:1147-1242`).
- Placeholder/development fallback visuals still exist for temporary hero/idol/enemy/trap profiles (`Source/T66/Gameplay/T66TemporaryProjectileSystem.cpp:49-120`, `Source/T66/Gameplay/T66TemporaryProjectileSystem.cpp:123-181`).

## Group H - Per-projectile performance cost

### H1. Chad 1 AOE Black structural cost surface

- Per spawned AOE system: 3 CPU-sim mesh slash emitters, one spawned particle each, plus 4 CPU-sim support sprite emitters with spawn counts 1, 2, 6, and 1. That is 13 particles per AOE Niagara component in the authored commandlet structure (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:82-117`, `Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:136-198`, `Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:433-458`).
- Renderer surface: 3 mesh renderers using the slash arc mesh/material layers plus 4 sprite renderers using support materials (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:1189-1277`).
- Mesh surface: the slash arc mesh has 96 segments, two generated vertices per segment boundary, and two triangles per segment in the mesh description construction path (`Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp:962-1011`).
- The current report did not find a single-instance profiling artifact specific to Chad 1 AOE Black. The current validator proves structure/assets/source guards, not cost or visual fidelity (`Scripts/ValidateCombatVFXProductionBindings.py:21-26`).

### H2. Current spawn model for N concurrent projectile VFX

- AOE and Pierce bound weapon VFX route through `TrySpawnBoundWeaponBaseSlashVFX`, which calls `UNiagaraFunctionLibrary::SpawnSystemAtLocation` and creates one auto-release Niagara component per shot (`Source/T66/Gameplay/T66CombatComponent.cpp:1228-1240`).
- Bounce uses visual-only `AT66HeroProjectile` actors as hidden movers/lifetime roots and attaches the authored Bounce Niagara carrier to the projectile root; temporary meshes are hidden when the authored carrier attaches (`Source/T66/Gameplay/T66CombatComponent.cpp:1480-1621`).
- DOT uses a visual-only hero-to-target `AT66HeroProjectile` with the authored DOT carrier if resolved, then applies one DOT payload and spawns marker visuals on arrival (`Source/T66/Gameplay/T66CombatComponent.cpp:2602-2713`).
- `AT66HeroProjectile` is an individual `AActor` with tick, collision sphere, visual mesh components, Niagara trail component, and projectile movement component (`Source/T66/Gameplay/T66HeroProjectile.cpp:18-54`). Its visual-only timed-travel mode interpolates on tick and destroys itself on arrival (`Source/T66/Gameplay/T66HeroProjectile.cpp:73-125`, `Source/T66/Gameplay/T66HeroProjectile.cpp:273-302`).

### H3. Existing pooling, batching, and manager paths

- Unreal Niagara component pooling is used through `ENCPoolMethod::AutoRelease` in weapon-bound spawns, idol-bound spawns, pixel VFX spawns, and boss trail/impact spawns (`Source/T66/Gameplay/T66CombatComponent.cpp:1231-1240`, `Source/T66/Gameplay/T66CombatVFX.cpp:1198-1207`, `Source/T66/Core/T66PixelVFXSubsystem.cpp:23-25`, `Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp:1031-1040`, `Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp:1062-1071`).
- There is an HISM-backed projectile manager, but it currently owns enemy and boss projectiles, not Hero 1 weapon/idol VFX. It stores 512 flat projectile slots, creates HISM components, preallocates 512 hidden instances per component, updates instance transforms, and handles collision/lifetime in subsystem tick (`Source/T66/Gameplay/T66ProjectileManagerSubsystem.h:44-95`, `Source/T66/Gameplay/T66ProjectileManagerSubsystem.h:127-134`, `Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp:362-370`, `Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp:884-946`, `Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp:987-1071`, `Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp:1074-1169`).
- Current manager routes include ranged/basic mobs and bosses (`Source/T66/Gameplay/Enemies/T66RangedEnemy.cpp:223-224`, `Source/T66/Gameplay/T66MobBase.cpp:752-753`, `Source/T66/Gameplay/T66BossBase.cpp:951-965`, `Source/T66/Gameplay/T66BossBase.cpp:997-1011`).
- The pixel VFX subsystem has a budgeted spawn path around `/Game/VFX/NS_PixelParticle.NS_PixelParticle`, but it still spawns Niagara components per accepted pixel request, subject to per-frame budget (`Source/T66/Core/T66PixelVFXSubsystem.cpp:20-32`, `Source/T66/Core/T66PixelVFXSubsystem.cpp:121-178`, `Source/T66/Core/T66PixelVFXSubsystem.cpp:247-286`).
- No current report-scope source path found a Niagara data-interface-fed, single-system batched Hero/Idol projectile VFX renderer. The existing batching surface is the HISM projectile manager for enemy/boss projectiles.


</review_packet>
