# Combat VFX Infrastructure Inventory

**Created:** 2026-05-26
**Updated:** 2026-05-28 for the verified MRQ editor-isolation route, shared aura material research plan, Candidate03 north-aura capture, Hero 1 AOE hitbox proof, Hero 1 AOE production VFX binding/item-scaling proof, Hero 1 AOE crescent-band hitbox alignment proof, and the generic visual/damage alignment contract.
**Status:** Infrastructure inventory only. This document is not an authoring procedure, validator contract, or production approval gate.

## 1. Purpose

This inventory records the current combat VFX docs, scripts, config seams, runtime capture paths, and Hero 1 axe lab tooling so later VFX infrastructure work extends the live system instead of creating parallel process or tooling.

Durable baseline index: after root and folder routers, future agents should start from `Gameplay/Combat/VFX_PROCESS_INDEX.md`. This inventory remains historical/current-state evidence; it is not the first-read procedure, DoD matrix, or packet template.

Authority remains elsewhere:

- `AGENTS.md` owns global process, review, PPF, capture, and prohibited-substitution rules.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md` owns the generic combat VFX authoring procedure.
- Per-effect plans and packets own effect-specific mechanisms, visual targets, paths, tuned values, and acceptance evidence.

Discovery note: this file lives under `Gameplay/Combat/` because `Gameplay/README.md` routes combat runtime and combat VFX authoring to that folder. If this inventory becomes a recurring handoff document, a later reviewed workstream should add a small index link rather than making this file a new source of truth.

Evidence labels used below:

- `Confirmed by file inspection`: the file text was inspected in this Workstream 0 pass.
- `Not runtime-verified in Workstream 0`: the seam exists in files, but no Unreal commandlet, capture, cook, validator, or staged build was run in this pass.
- `Inventory finding`: a discrepancy or gap discovered while reading files. It is not fixed by this pass.

## 2. Current Source-Of-Truth Hierarchy

| Area | Current file | Inventory status | Notes |
|---|---|---|---|
| Global process | `AGENTS.md` | Confirmed by file inspection | Defines no-substitution, PPF, artifact parity, mechanism manifest, Claude review, imagegen, Unreal capture, and Niagara combat VFX rules. |
| Gameplay router | `Gameplay/GAMEPLAY_AGENTS.md` | Confirmed by file inspection | Routes combat VFX work through `Gameplay/Combat/CombatVFXAuthoringProcedure.md`. |
| Gameplay index | `Gameplay/README.md` | Confirmed by file inspection | Routes combat docs to `Gameplay/Combat/`. |
| Generic VFX procedure | `Gameplay/Combat/CombatVFXAuthoringProcedure.md` | Confirmed by file inspection | Already owns source intake, mockup gate, carrier archetypes, staged authoring, mask/material manifest, parameter evidence, editor pitfalls, and close templates. |
| Visual/damage alignment contract | `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md` | Confirmed by file inspection | Owns the generic anchor, footprint, offset, tolerance, and marker-vs-area-read contract for keeping VFX presentation aligned with authoritative damage geometry. |
| Hero 1 axe plan | `Gameplay/Combat/Hero1AxeVFXPlan.md` | Confirmed by file inspection | Owns isolated Hero 1 axe VFX goals, current canonical source selection, lab boundaries, and future AOE/DOT/Summon/Bounce direction. |
| Hero 1 AOE packet | `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md` | Confirmed by file inspection | Owns the current AOE American-flag visual target, AOE mechanism gates, hitbox alignment rule, and current verification expectations. |
| Hero 1 shared aura material plan | `Gameplay/Combat/Hero1AxeSharedAuraMaterialResearchPlan.md` | Confirmed by file inspection | Owns the source-fidelity plan, shared material roles, parameter contract, and editor-isolation material gate for Hero 1 axe base attacks. |
| Combat runtime reference | `Gameplay/Combat/MASTER_COMBAT.md` | Confirmed by targeted search | Documents current temporary projectile presentation, Hero 1 AOE sector query, and `hero1axeaoehitbox` proof mode. |

No generic fillable combat VFX packet template was found under `Gameplay/Combat/`; current path search found only the generic procedure and the Hero 1 axe plan/packet. The existing Hero 1 AOE packet is the closest instance example.

## 3. Current Research And Visual Target Cache

| Area | Current path | Inventory status | Notes |
|---|---|---|---|
| Hero 1 research notes | `Saved/VFXResearch/Hero1Axe/source_matrix_2026-05-24.md` and `Saved/VFXResearch/Hero1Axe/notegpt_analysis_2026-05-24.md` | Confirmed by file path/search only | Used by current docs as source evidence references. Not re-reviewed in Workstream 0. |
| Pablo-provided transcripts | `Saved/VFXResearch/Hero1Axe/notegpt_transcripts/` | Confirmed by file path/search only | Current docs say video evidence comes from Pablo-provided transcripts or already stored Pablo-provided transcript paths. |
| Historical local extraction artifacts | `Saved/VFXResearch/Hero1Axe/_historical_extraction_outputs/20260525_transcript_extraction_artifacts/` | Confirmed by file path/search only | Current docs mark these as historical artifacts, not current source truth. Do not revive this path. |
| Approved AOE visual target | `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/hero1_axe_aoe_american_flag_contact_sheet.png` | Confirmed by file path/search and packet text | Current AOE packet records this as Pablo-approved visual direction for the AOE lab pass. |

## 4. Cook And Lab Isolation

| Seam | Current file | Inventory status | Notes |
|---|---|---|---|
| Cook exclusion | `Config/DefaultGame.ini` | Confirmed by file inspection | `/Game/VFXLab` is present in `DirectoriesToNeverCook`. |
| Always-cook conflict | `Config/DefaultGame.ini` | Confirmed by file inspection | `/Game/VFXLab` was not found in `DirectoriesToAlwaysCook`. |
| Validator cook check | `Scripts/ValidateHero1AxeAOELabVFX.py` | Confirmed by file inspection | `validate_config()` parses `DefaultGame.ini` and fails if `/Game/VFXLab` is missing from never-cook or present in always-cook. |
| AssetRegistry isolation | `Scripts/ValidateHero1AxeAOELabVFX.py` | Confirmed by file inspection | `validate_asset_registry()` checks live-to-lab and lab-to-live dependencies. Allowed lab dependency prefixes are `/Game/VFXLab`, `/Script/`, `/Niagara/`, and `/Engine/`. |

Not runtime-verified in Workstream 0: no Unreal validator commandlet was run, so current AssetRegistry state was not proven live in this pass.

## 5. Capture Tooling Inventory

### 5.1 `Scripts/CaptureT66GameplayVideo.ps1`

Confirmed by file inspection:

- Default capture mode is `hero1axeaoe`.
- Main parameters include project, editor exe, map, capture mode, output MP4, frame directory, frame prefix, resolution, frame count, frame rate, capture interval, delay, post-capture delay, timeout, `ExecCmds`, extra args, review camera, Hero 1 axe preview staging, Hero 1 target options, hitbox timing, `RemoveFrames`, and `PrintOnly`.
- FFmpeg resolution is more robust than earlier ad hoc paths: it refreshes process PATH from machine/user/process values, then checks `ffmpeg`, `%USERPROFILE%\bin\ffmpeg.cmd`, Python `imageio_ffmpeg`, and WinGet Gyan FFmpeg installs.
- `-UseHero1AxePreviewStaging` applies the current locked preview camera, locks camera zoom, centers the player, offsets the lab actor, and optionally spawns preview enemies.
- `hero1axeaoehitbox` mode adds hitbox fire/VFX lead args and forces combat debug view/debug labels into `ExecCmds`.
- The script launches Unreal with `-T66GameplayAutoCapture`, `-T66GameplayAutoScreenshotSequenceDir`, frame prefix, count, interval, screenshot delay, and post-capture delay.
- The script verifies Unreal exits successfully, verifies the requested number of PNG frames exist, encodes MP4 through ffmpeg, and retains frame sequences unless `-RemoveFrames` is passed.
- `-PrintOnly` exists and returns before launching Unreal.
- Workstream 2 capture-evidence extension adds optional `-EvidenceBundle`, `-EvidenceRoot`, `-EvidenceLabel`, and `-EvidenceSelectedFrames` parameters.
- `-EvidenceBundle` preserves the existing Unreal-owned frame sequence and MP4 flow, then packages review artifacts through `Scripts/BuildT66VideoEvidenceBundle.py`.

Current evidence bundle output:

- `ffprobe.json`
- `manifest.json`
- `contact_sheet.png`
- selected-frame PNG copies
- `selected_frames.md`
- `visibility_checklist.md`

Remaining gaps confirmed by file inspection:

- No generic VFX acceptance wrapper exists; current VFX-specific flags are Hero 1 axe oriented.
- Baseline durable Niagara editor-isolation capture exists for Hero 1 axe slash/aura lab work through `Scripts/CaptureT66NiagaraMRQIsolation.ps1`. It is not yet generalized into a fully effect-agnostic acceptance wrapper.
- No automated computer-vision scoring exists, by design. Visibility/readability review remains manual.

Runtime verification added on 2026-05-27: `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/EditorIsolation/20260527_MRQSquareRouteProbe/manifest.json` reports a successful square top-down MRQ render with non-black VFX pixels, frame margins, and Niagara particle-log evidence.

Latest Hero 1 AOE visual correction proof added on 2026-05-27:

- same-view actual: `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/EditorIsolation/20260527_Candidate03NorthAuraPass08OpaqueBlackRgbReview/actual.png`,
- same-view notes: `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/EditorIsolation/20260527_Candidate03NorthAuraPass08OpaqueBlackRgbReview/mismatch_notes.md`,
- same-view background proof: `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/EditorIsolation/20260527_Candidate03NorthAuraPass08OpaqueBlackRgbReview/background_pixel_check.md`,
- gameplay hitbox video: `Saved/VideoCaptures/Hero1AxeAOE_Candidate03_NorthAuraHitbox_20260527_Clean50/Hero1AxeAOE_Candidate03_NorthAuraHitbox_Clean50.mp4`,
- gameplay contact sheet: `Saved/VideoCaptures/Hero1AxeAOE_Candidate03_NorthAuraHitbox_20260527_Clean50/evidence/contact_sheet.png`,
- durable hitbox log excerpt: `Saved/VideoCaptures/Hero1AxeAOE_Candidate03_NorthAuraHitbox_20260527_Clean50/evidence/hitbox_proof_log_excerpt.md`.

MRQ failure-path proof added on 2026-05-27: `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/EditorIsolation/20260527_ForcedFail_InvalidSystem_AfterOpaqueBlackRgb/` used an invalid Niagara system path. The render stdout still contained `Movie Pipeline completed` and `RequestExitWithStatus(0, 0)`, but the wrapper exited with `LASTEXITCODE=1`; its manifest reports `render_success=false`, `failure_mode=verification_failed`, `has_non_black_pixels=false`, `has_particle_log_evidence=false`, and `review_background=opaque_black_rgb_preserved`.

### 5.3 `Scripts/CaptureT66NiagaraMRQIsolation.ps1`

Confirmed by implementation and runtime verification:

- Uses the official MRQ command-line shape: project, generated map, `-game`, `-LevelSequence=...`, and `-MoviePipelineConfig=...`.
- Calls `Scripts/SetupT66NiagaraMRQIsolation.py` to create a temporary `/Game/VFXLab/Temp/MRQ` map, Level Sequence, and Movie Pipeline config.
- Uses a square top-down orthographic camera and writes an opaque black review PNG by preserving rendered RGB over black.
- Renders the actual `AT66Hero1AxeAOEVFXLabActor` and target Niagara system; the script does not create the VFX silhouette.
- Writes `actual.png`, `actual_crop.png`, `contact_sheet.png`, `mismatch_notes.md`, `manifest.json`, setup/render logs, and setup/cleanup manifests.
- Cleans up generated MRQ temp assets by default. `-KeepTempAssets` is available only for debugging.
- Performs a simple non-black bounding-box check so cropped or empty frames fail before a handoff.
- Treats a null PowerShell process exit code as success only when the command has exited and stdout contains known Unreal success markers plus artifact verification passes. This prevents false failure on successful MRQ/Python runs without weakening real failure detection.

Orientation note:

- In the current square top-down MRQ route, world `+X` renders toward the top of the output image and world `+Y` renders toward screen-right. Hero 1 AOE Candidate03 uses world `+X` for the same-view north/contact direction.

Current limitations:

- This is an editor-isolation visual gate, not temporal proof.
- It does not replace gameplay MP4/frame evidence, hitbox proof, or Pablo visual approval.
- It is currently tuned for Hero 1 axe slash/aura lab work and should be generalized only through a later reviewed workstream.

### 5.2 Runtime Screenshot Sequence Support

Confirmed by file inspection in `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`:

- Gameplay automation parses `T66GameplayAutoScreenshot`, `T66GameplayAutoScreenshotSequenceDir`, `T66GameplayAutoScreenshotSequenceCount`, `T66GameplayAutoScreenshotSequenceInterval`, `T66GameplayAutoScreenshotSequencePrefix`, and `T66GameplayAutoCapture`.
- Screenshot sequence count is clamped to `1..240`.
- Sequence interval is clamped to `0.016..2.0` seconds.

Not runtime-verified in Workstream 0: no screenshot sequence was captured.

## 6. Hero 1 Axe Runtime Capture Modes

Confirmed by file inspection in `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`:

| Mode | Current behavior | Status |
|---|---|---|
| `hero1axeaoe` | Enters Hero 1 axe lab preview branch. Hides HUD, pauses enemy director spawning, destroys existing enemies, may center player, may lock camera arm, spawns `AT66Hero1AxeAOEVFXLabActor`, optionally spawns fixed preview targets. | Confirmed by file inspection, not runtime-verified in Workstream 0. |
| `vfxlabhero1axeaoe` | Same branch as `hero1axeaoe`. | Confirmed by file inspection, not runtime-verified in Workstream 0. |
| `hero1axeaoehitbox` | Equips Hero 1 black AOE weapon, disables movement, spawns fixed inside/outside proof targets, schedules lab VFX before attack fire, calls `PerformAutomationAutoAttackNow()`, logs HP before/after and pass/fail per target, and emits damage-number proof feedback. | Confirmed by file inspection and runtime-verified on 2026-05-27. |

Runtime verification added on 2026-05-27: `Saved/Logs/T66.log` lines 908-914 show setup/equip/target staging for `hero1axeaoehitbox`; lines 1070-1077 show three expected-hit targets taking damage and two expected-miss targets taking no damage, all with `Result=PASS`. A durable excerpt is stored with the final clean evidence bundle.

Inventory finding:

- `Hero1AxeVFXPlan.md` still mentions adding `-T66GameplayAutoCapture=vfxlab` as a potential Step 0.5 mode, but the live source currently uses `hero1axeaoe`, `vfxlabhero1axeaoe`, and `hero1axeaoehitbox`. A future doc cleanup or generic capture workstream should decide whether a generic `vfxlab` mode is still desired.

## 7. Validation And Setup Tooling

### 7.1 `Scripts/ValidateHero1AxeAOELabVFX.py`

Confirmed by file inspection:

- Validates `Config/DefaultGame.ini` cook isolation.
- Validates required Hero 1 AOE lab assets:
  - `/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash`
  - slash bright/body/dark materials
  - support materials
  - `SM_Hero1AxeAOE_SlashArc`
  - `T_Hero1AxeAOE_StreakMask`
  - `T_Hero1AxeAOE_DissolveNoise`
  - `T_Hero1AxeAOE_ImpactMask`
- Validates deprecated old assets are absent, including older crescent, weapon slash seed, shockwave seed, older reveal/crescent/vertex/weapon-art materials.
- Validates expected blend modes for slash layer materials and support materials.
- Validates bidirectional AssetRegistry isolation.
- Validates runtime source fragments in the lab actor, setup script, commandlet, overlay capture automation, and gameplay capture script.
- Validates negative-path self-tests for lab path detection and allowed dependency prefixes.
- Forbids old shortcut fragments such as procedural/static mesh component primary carriers, old seed systems, sprite-size/tint actor-side layers, older Art/Layer fragments, and old lookalike asset names.
- Checks the hitbox proof mode is under a non-shipping guard by source-text placement.

Gaps confirmed by file inspection:

- It is Hero 1 AOE specific, not a generic validator for future DOT/Summon/Bounce effects.
- It validates structure and source guards, not visual mechanism success. It cannot prove point A to point B sweep, erosion, color readability, impact placement, or visual fidelity.
- It does not emit a standalone JSON/Markdown validation report.
- It does not validate contact sheet/frame-selection evidence. Baseline evidence-bundle tooling now exists, but the Hero 1 AOE structural validator is not wired to require or parse those artifacts.

Not runtime-verified in Workstream 0: the validator was not run.

### 7.2 `Scripts/SetupHero1AxeAOELabVFX.py`

Confirmed by file inspection:

- Owns `/Game/VFXLab/Hero1Axe/AOE`, `/Game/VFXLab/Hero1Axe/Shared`, and `NS_Hero1AxeAOE_MeshSlash`.
- Imports source textures from `SourceAssets/VFX/Hero1Axe/AOE`.
- Defines three slash layer material configs: `Bright`, `Body`, and `Dark`.
- Defines support material configs: `ImpactFlare`, `DirectionalSpark`, `Mote`, and `GroundTrace`.
- Deletes deprecated old lab assets before rebuilding current material shells.
- Rebuilds material graphs with dynamic parameter `SlashAge`, authored detail masks, dissolve panning, reveal controls, impact mask, glow/opacity parameters, and layer-specific colors.
- Prepares the Niagara package slot; the C++ commandlet owns mesh/system generation and renderer binding.

Not runtime-verified in Workstream 0: the setup script was not run and no assets were changed.

### 7.3 `Scripts/GenerateHero1AxeAOETextures.py`

Confirmed by file inspection:

- Generates project-owned source PNG masks under `SourceAssets/VFX/Hero1Axe/AOE`.
- Output texture names are:
  - `T_Hero1AxeAOE_StreakMask.png`
  - `T_Hero1AxeAOE_DissolveNoise.png`
  - `T_Hero1AxeAOE_ImpactMask.png`
- Uses deterministic random seeds for streak and dissolve masks.

Not runtime-verified in Workstream 0: the generator was not run and no texture files were changed.

### 7.4 `Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp`

Confirmed by file inspection:

- Owns creation of `NS_Hero1AxeAOE_MeshSlash` and `SM_Hero1AxeAOE_SlashArc`.
- Defines slash layer configs for bright/body/dark mesh-rendered emitters with different lifetimes, scales, colors, rotation force, and lever radius.
- Defines support emitter configs.
- Links dynamic material parameter input to `Particles.NormalizedAge`.
- Adds `MeshRotationForce` before `SolveRotationalForcesAndVelocity`.
- Builds a non-uniform slash arc mesh from authored profile samples spanning `-90` to `90` degrees with varying inner radius, outer radius, and tangent offset.
- Binds mesh renderers for slash layers and sprite renderers for support emitters.

Not runtime-verified in Workstream 0: the commandlet was not run.

### 7.5 `Source/T66/Gameplay/T66Hero1AxeAOEVFXLabActor.cpp`

Confirmed by file inspection:

- Loads `/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash` by default, with `-T66Hero1AxeAOEOverrideNiagara=` override support.
- Uses one `UNiagaraComponent` named `SlashNiagaraComponent`.
- Disables collision and overlap generation on the Niagara component.
- Restarts the slash cycle with `DeactivateImmediate()` then `Activate(true)`.
- Logs static and runtime diagnostics for component state, emitter handles, renderers, mesh/material binding, system instance state, particle counts, and bounds.

Known issue already documented in `Source/T66/Gameplay/pending_issues_Gameplay.md`:

- The lab actor uses a deprecated Niagara emitter readiness API. This pass did not fix it.

Not runtime-verified in Workstream 0: the lab actor was not run.

## 8. Runtime Combat And Hitbox Contract Inventory

Confirmed by file inspection / targeted search:

- `Gameplay/Combat/MASTER_COMBAT.md` states temporary projectile presentation is still centralized through `FT66TemporaryProjectileSystem` and that current hero/idol placeholders are presentation, not authority.
- `Gameplay/Combat/MASTER_COMBAT.md` states Hero 1 AOE axe splash target selection uses a query-only target-anchored 180-degree sector, oriented from the hero attack origin toward the primary target.
- `Gameplay/Combat/MASTER_COMBAT.md` states new irregular attack visuals such as slashes or arcs must keep VFX as presentation and use explicit logical query shapes for damage.
- `Source/T66/Gameplay/T66CombatComponent.*` currently owns auto-attack behavior and placeholder/idol VFX methods, but this Workstream 0 pass did not inspect it deeply enough to design production binding.
- `Source/T66/Gameplay/T66TemporaryProjectileSystem.*` currently maps hero attack categories to blue placeholder shapes and idol overlays.

Current status:

- A first production VFX binding contract now exists for Hero 1 axe AOE. It maps the weapon row `Hero_1_black_aoe` to the promoted Niagara system `/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash` through `Content/Data/DT_CombatVFXBindings`.
- `Hero_1_black_aoe` now uses `AoeInnerRadiusRatio=0.54` in `Content/Data/Weapons.csv`, producing a crescent-band logical hitbox that matches the current broad slash visual. Other AOE rows currently use `0.00` and remain filled-sector/sphere behavior until they receive their own reviewed effect packet.
- Hero 1 AOE hitbox proof remains combat-authority proof: the VFX is presentation, while target selection, damage, and expected-hit/expected-miss checks stay in `UT66CombatComponent`.
- Future idol runtime binding should extend the combat VFX binding contract with `IdolModifier`/equivalent rows and effect-packet metadata. Idol VFX overlays should not be implemented by resurrecting placeholder projectile presentation paths.

## 8.1 Production VFX Binding And Item-Scaling Infrastructure

Implemented and runtime-verified on 2026-05-28:

| Seam | Current file/path | Status | Notes |
|---|---|---|---|
| Binding data type | `Source/T66/Data/T66DataTypes.h` | Implemented | `FT66CombatVFXBindingData` stores binding ID, source type, source ID, attack category, Niagara system path, effect packet ID, VFX profile, visual radius/playback values, and production/fallback flags. |
| Binding table source | `Content/Data/CombatVFXBindings.csv` | Implemented | Contains active Hero 1 weapon-base rows `Hero1Axe_AOE_Base` (`Hero_1_black_aoe`), `Hero1Axe_RetiredLine_Base` (`Hero_1_black_retired-line`), `Hero1Axe_Bounce_Base` (`Hero_1_black_bounce`), and `Hero1Axe_DOT_Base` (`Hero_1_black_dot`). Idol category proofs are placeholder/proof paths, not production idol rows. |
| Runtime DataTable asset | `Content/Data/DT_CombatVFXBindings.uasset` | Implemented | Created/refreshed by `Scripts/SetupCombatVFXBindingsDataTable.py`. |
| GameInstance access | `Source/T66/Core/T66GameInstance.h/.cpp` | Implemented | Loads/caches `DT_CombatVFXBindings` and exposes binding lookup. `BP_T66GameInstance` is assigned by the setup script. |
| Production slash assets | `Content/VFX/Hero1/Axe/AOE/` and `Content/VFX/Hero1/Axe/Shared/` | Implemented | Promoted from the isolated Hero 1 axe AOE lab asset path. Normal combat must not reference `/Game/VFXLab`. |
| Production promotion script | `Scripts/PromoteHero1AxeAOEVFXToProduction.py` | Implemented | Runs the lab setup in production-target mode and the `T66Hero1AxeAOEVFX` commandlet with `-T66Hero1AxeAOEProduction`. |
| Production validator | `Scripts/ValidateCombatVFXProductionBindings.py` | Implemented | Checks CSV/table assignment, required production assets, no production dependency on `/Game/VFXLab`, and source guard fragments. |
| Combat dispatcher | `Source/T66/Gameplay/T66CombatComponent.cpp` | Implemented | Resolves production binding for weapon-base attacks, spawns the bound Niagara system, suppresses temporary projectile visuals only when a production binding succeeds, and logs `CombatVFXProductionSpawned`. |
| Category item stat consumption | `Source/T66/Gameplay/T66CombatShared.h/.cpp` and `Source/T66/Gameplay/T66CombatComponent.cpp` | Implemented | AOE damage/speed/scale secondary stats now feed the combat attack values consumed by the logical hitbox and VFX presentation. |
| AOE crescent-band geometry | `Source/T66/Data/T66DataTypes.h`, `Content/Data/Weapons.csv`, `Source/T66/Gameplay/T66CombatComponent.cpp` | Implemented for Hero 1 AOE | `AoeInnerRadiusRatio=0.54` creates a hollow center for `Hero_1_black_aoe`; all other AOE weapon rows remain `0.00` unless separately reviewed. |
| Readable playback clamp | `Source/T66/Gameplay/T66CombatComponent.cpp` | Implemented | Strong attack-speed items still shorten the combat fire interval; only slash VFX presentation playback is clamped to avoid collapsing below a readable `0.20s` duration. Logs include both raw and applied playback multipliers. |
| Binding proof capture mode | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | Implemented | `-T66GameplayAutoCapture=hero1axeaoevfxbinding` equips `Hero_1_black_aoe`, optionally grants deterministic proof items, and fires real combat while expecting the production dispatcher to spawn VFX. |
| Binding proof wrapper | `Scripts/RunHero1AxeAOEVFXBindingProof.ps1` | Implemented | Runs baseline, AOE scale, AOE speed, and AOE damage proof captures with MP4, frame sequence, evidence bundle, and per-case log excerpts. |

Verification evidence:

- Build: `T66Editor Win64 Development` succeeded on 2026-05-28 after the runtime binding/category-stat/playback changes.
- Production binding validator: `Scripts/ValidateCombatVFXProductionBindings.py` succeeded on 2026-05-28.
- Runtime proof batch: `Saved/VideoCaptures/Hero1AxeAOE_VFXBindingProof_20260528_001540/Hero1AxeAOEVFXBindingProofSummary.md`.
- Baseline proof log: `Saved/VideoCaptures/Hero1AxeAOE_VFXBindingProof_20260528_001540/Baseline/proof_log_excerpt.md`.
- AOE scale proof log: `Saved/VideoCaptures/Hero1AxeAOE_VFXBindingProof_20260528_001540/AoeScale/proof_log_excerpt.md`.
- AOE speed proof log: `Saved/VideoCaptures/Hero1AxeAOE_VFXBindingProof_20260528_001540/AoeSpeed/proof_log_excerpt.md`.
- AOE damage proof log: `Saved/VideoCaptures/Hero1AxeAOE_VFXBindingProof_20260528_001540/AoeDamage/proof_log_excerpt.md`.
- Crescent-band hitbox cleanup video: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/Hero1AxeAOE_HitboxCleanup.mp4`.
- Crescent-band hitbox cleanup contact sheet: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/evidence/contact_sheet.png`.
- Crescent-band hitbox cleanup log: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/T66.log`.

Important limitation:

- The 2026-05-28 proof batch is backend/integration evidence, not a renewed visual-design approval. The locked design remains the accepted Candidate03 direction. The binding proof confirms production asset dispatch, hitbox authority, damage numbers, and item stat response; future visual-polish work should re-enter through the editor-isolation and gameplay-camera visual gates before claiming final VFX quality.
- Damage values in the proof logs are final integer combat damage after internal precision and rounding. AOE damage proof reporting `EffectiveDamagePerShot=121` from `AoeDamageValue=4.370` is expected and should not be read as a binding failure.
- Scale values in the proof logs are category-specific runtime multipliers after already-applied hero/global baseline scale is removed. AOE scale proof maps `AoeScaleValue=5.146` to roughly 4x effective radius/visual scale by design.
- Deterministic proof item grants live only in the `hero1axeaoevfxbinding` automation path; normal gameplay item acquisition is not changed by the proof harness.
- `Content/Data/CombatVFXBindings.csv` and `Content/Data/DT_CombatVFXBindings.uasset` are live generated assets used by the setup/validator flow, but `git ls-files` returns no entries for them in this repository state. `Scripts/SetupCombatVFXBindingsDataTable.py` now enforces the load-bearing `Hero1Axe_AOE_Base` row before reload, and the validator fails if `BaseVisualRadius` drifts from `411.4`. Future agents must inspect the files directly and run the setup/validator scripts instead of assuming narrow Git status tells the whole binding story.

## 9. Inventory Findings

1. Resolved on 2026-05-27: `Hero1AxeAOESlashMechanismPacket.md` verification commands now reference `Scripts/SetupHero1AxeAOELabVFX.py`, followed by the `T66Hero1AxeAOEVFX` commandlet. `Scripts/SetupHero1AxeAOEMeshSlash.py` does not exist and should not be reintroduced as a doc reference unless a real script is added.

2. No generic combat VFX packet template exists under `Gameplay/Combat/`. The Hero 1 AOE mechanism packet is currently the only concrete packet example.

3. Capture tooling now supports an optional evidence bundle for MP4-from-frame-sequence captures: contact sheet, ffprobe metadata artifact, selected-frame notes, copied selected frames, manifest, and visibility/readability checklist. The bundle is a packaging layer, not visual acceptance.

4. Validation is stronger than a simple asset-presence check, but it is Hero 1 AOE specific. It does not yet provide a generic carrier-archetype validator or reusable report format for DOT/Summon/Bounce.

5. `/Game/VFXLab` cook exclusion exists in config and is checked by the validator, but Workstream 0 did not run Unreal to prove current AssetRegistry isolation.

6. Resolved for Hero 1 AOE only on 2026-05-28: production binding now exists for `Hero_1_black_aoe` through `DT_CombatVFXBindings`, with proof mode `hero1axeaoevfxbinding`. Future weapons/idols still need their own reviewed binding rows and production-promotion proof.

6A. Resolved for Hero 1 AOE only on 2026-05-28: `Hero_1_black_aoe` now has a crescent-band logical hitbox with explicit inside-band, inside-angle-edge, inner-hollow, outside-angle-edge, behind, and outside-radius proof targets. Future irregular VFX shapes should add effect-specific logical geometry fields and proof targets instead of relying on visual mesh collision.

7. The lab actor has a known deprecated Niagara readiness API issue in `Source/T66/Gameplay/pending_issues_Gameplay.md`.

8. This inventory is not linked from an index yet. If future agents need it as a first-read artifact, add a small link in a later reviewed documentation pass.

## 10. Recommended Next Workstreams

These are recommendations only. Workstream 0 does not approve their implementation.

### Workstream 1: Packet Instance Template

Goal:

- Create a fillable per-effect packet template that references `CombatVFXAuthoringProcedure.md` instead of copying its generic rules.

Extend existing:

- Use `Hero1AxeAOESlashMechanismPacket.md` as the concrete example.
- Use `CombatVFXAuthoringProcedure.md` as the authority for generic carrier/mask/material/close rules.

Key acceptance:

- Template includes effect-specific slots only.
- It separates lab visual acceptance, logical damage acceptance, and production promotion.
- It explicitly avoids becoming a second procedure.

### Workstream 2: Capture Evidence Bundle

Status:

- Baseline implemented by `Scripts/CaptureT66GameplayVideo.ps1 -EvidenceBundle` and `Scripts/BuildT66VideoEvidenceBundle.py`.

Original goal:

- Extend current gameplay video capture into a VFX evidence helper or wrapper.

Extend existing:

- `Scripts/CaptureT66GameplayVideo.ps1`
- `T66PlayerController_Overlays.cpp` screenshot-sequence command-line support

Implemented deliverable:

- MP4.
- Retained PNG frames.
- ffprobe metadata file.
- Contact sheet.
- selected frame notes for start/mid/impact/dissipate.
- written visibility/readability checklist.

Remaining explicit non-goal:

- Automated computer-vision scoring. Keep it manual/simple first.

### Workstream 3: Generalized VFX Validator Layer

Goal:

- Keep `ValidateHero1AxeAOELabVFX.py` as the Hero 1 AOE validator, then add a reusable layer or report format for future effects.

Extend existing:

- Cook isolation check.
- AssetRegistry isolation check.
- source guard patterns.
- deprecated shortcut checks.

Key acceptance:

- Generic checks reference carrier archetype definitions from `CombatVFXAuthoringProcedure.md`.
- Structural validation does not claim visual acceptance.
- Output is reportable and reusable across future DOT/Summon/Bounce packets.

### Workstream 4: Runtime Binding Contract

Goal:

- Write design text for how a future production weapon or idol VFX is bound to combat without wiring it yet.

Extend existing:

- `Gameplay/Combat/MASTER_COMBAT.md`
- current Hero 1 AOE hitbox proof contract
- current temporary projectile placeholder contract

Key acceptance:

- VFX remains presentation.
- logical hitbox/damage query remains authority.
- weapon/idol rows eventually map to effect packets/assets through a reviewed production-promotion gate.
- no lab asset is referenced by normal combat until explicitly promoted.

### Workstream 5: AOE Re-entry Through Pipeline

Goal:

- After Workstreams 1-4, re-enter the Hero 1 axe AOE effect through the new pipeline instead of continuing as a one-off.

Key acceptance:

- component completeness is checked first.
- visual fidelity pass compares against the approved saved mockup/contact sheet.
- result is reported `FULL` only if every required mechanism and evidence gate passes.

## 11. Workstream 0 Close

The original Workstream 0 pass produced an inventory only.

It did not:

- run Unreal,
- run ffmpeg or ffprobe,
- run validators,
- run asset setup scripts,
- run texture generation,
- modify runtime code,
- modify Unreal assets,
- accept or improve the Hero 1 axe AOE visual.
