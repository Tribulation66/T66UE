# Findings And Limitations

## 2026-05-13 Setup

- Blender 5.1.1 is installed at `C:\Program Files\Blender Foundation\Blender 5.1\blender.exe`.
- Rigify enabled successfully in Blender 5.1.1.
- Rigodotify cloned from `https://github.com/catprisbrey/Rigodotify.git`.
- Rigodotify commit installed locally: `4ee6e34b1580a0fac07e31c6cfed30addad182aa`.
- Rigodotify install failed when packaged with `__init__.py` at zip root.
- Rigodotify install succeeded after repackaging with a top-level `Rigodotify/` prefix.
- Rigodotify enabled successfully in Blender 5.1.1.
- Blender reported a Rigodotify panel naming warning: `GodotMecanim_Panel` does not contain `_PT_` with prefix and suffix. This did not block enablement.
- `Tools/setup_rigging_animation_infrastructure.ps1 -SkipBlenderInstall` was run successfully after setup; it confirmed Rigodotify was already up to date and skipped the existing Quaternius extracts.

## Quaternius Packages

Installed from Downloads:

- `Universal Animation Library[Standard].zip`
- `Universal Animation Library 2[Standard].zip`
- `Universal Base Characters[Standard].zip`
- `Universal Animation Library[Source].zip`
- `Universal Animation Library 2[Source].zip`
- `Universal Base Characters[Source].zip`

Initial Standard extracted asset inventory:

- 1 `.blend`
- 3 `.glb`
- 18 `.gltf`
- 29 `.fbx`
- 18 `.bin`
- 55 `.png`
- 5 `.txt`

Source extracted asset inventory:

- 23 `.blend`
- 131 `.fbx`
- 92 `.gltf`
- 3 `.glb`
- 93 `.bin`
- 101 `.png`
- 5 `.txt`
- 3 `.zip`

Current source baseline:

- `Universal Animation Library Source\UAL1.blend` is the preferred first source for Arthur idle, walk, jump, and roll because it contains the simple base locomotion clips.
- `Universal Animation Library 2 Source\UAL2.blend` is useful for later parkour/combat variants.
- `Universal Base Characters Source\Base Characters\Superhero_Male_FullBody.blend` and related base-character scenes are available for editable body/reference work.
- Standard packages remain useful for engine import checks and fallback references, but Source `.blend` files are now the baseline for source-of-truth animation editing.

## Unreal GLTF Import Note

The Unreal setup screenshots and local `Unreal-Engine-README.txt` solve the Unreal import/retargeting side, not the Blender source-editing side.

Use this interpretation:

- For Unreal import, Quaternius recommends GLTF/GLB because rigged FBX exported from Blender can import at the wrong scale and break retargeting.
- Import animations enabled, 30 kHz bone-animation baking, and snap-to-closest-frame settings are appropriate for bringing the library into Unreal.
- Source `.blend` files are now available and should be used for original rig/action editing.
- The Standard GLB files still import into Blender with usable action lists, so they remain valid for import-path experiments and for learning the skeleton/action contract.

## Blender Inventory Probe

`Tools/inspect_animation_assets.py` was run through Blender 5.1.1 against the key Standard and Source package files.

Results:

- `UAL1_Standard.glb`: imports successfully, 1 armature, 45 actions.
- `UAL2_Standard.glb`: imports successfully, 1 armature, 43 actions.
- `Mannequin_F.blend`: opens successfully, 2 armatures, no stored actions in the inspected file.
- `Superhero_Male_FullBody.gltf`: imports successfully, 1 armature, no actions.
- `Superhero_Female_FullBody.gltf`: imports successfully, 1 armature, no actions.
- `UAL1.blend`: opens successfully, 1 armature, 127 actions.
- `UAL2.blend`: opens successfully, 1 armature, 135 actions.

Tooling notes:

- Blender 5.1 action data no longer exposes `action.fcurves` in the same way older scripts expect. The inspection helper now treats missing `fcurves` as zero instead of failing.
- Opening `Mannequin_F.blend` in background mode skipped its embedded `rig_ui.py` because scripts are disabled by default. This is expected for safe background inspection.
- Importing the Universal Base Characters GLTF files emitted missing-image warnings for `T_Hair_1_Normal_png.png` and `T_Eye_Normal_png.png`. The mesh and armature still imported. Treat this as a material-path issue to resolve before using the base characters for polished reference renders.

## Working Assumption

Use the Source packages for Arthur and future source-of-truth hero animation work. Keep the Standard packages for import-path comparison and regression checks.

## 2026-05-14 Arthur Pilot Visual QA

What was visually tested:

- The Arthur pilot `.blend` generated rendered frame previews for idle, walk, jump, and roll.
- `Tools/make_preview_contact_sheets.py` built `Runs/Arthur_Animation_Pilot_20260514/PreviewFrames/Arthur_All_Actions_Contact_Sheet.png`.
- The contact sheet was inspected before Unreal import.
- A separate object-delta retarget test was visually rejected because it caused severe target mesh deformation. That output should not be used as the Arthur baseline.

What changed based on visual findings:

- The rejected object-delta transfer path was abandoned.
- The accepted imported pass uses the local-basis retarget path because it preserved Arthur's proportions and produced readable idle, walk, jump, and roll silhouettes.
- The roll source action was switched to `Roll_RM` for the final pilot export.

Current quality status:

- The roll reads as an intentional combat roll in the current preview camera.
- The jump and walk read as functional first-pass locomotion clips.
- The idle is usable as a prototype loop but still reads stylized and needs a polish pass before it becomes a hero-quality baseline.
- The Arthur pilot is playable and verified in Unreal, but it is not a final production-accepted animation set because it has not yet gone through the required front/side/three-quarter correction loop.

Process gap to close before scaling:

- `Tools/render_arthur_action_previews.py` currently renders one preview camera. Before treating a future humanoid set as production accepted, extend or replace it with a multi-view humanoid preview renderer that outputs front, side, three-quarter, and gameplay-camera contact sheets plus written QA notes.

## 2026-05-14 Arthur Runtime Row Rollback

Problem:

- The first Arthur pilot import wired the skeletal animation experiment directly into the live `Hero_1_Chad` visual row.
- `Hero_1_Chad` is the row selected by normal Royal Chad hero selection.
- The live row previously pointed to the QuadRetro static mesh `/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/SM_Hero_1_Chad_QuadRetro.SM_Hero_1_Chad_QuadRetro` with scale `(X=1.011123,Y=1.011123,Z=1.011123)`.
- The pilot row pointed it instead at `/Game/Characters/Heroes/Hero_1/Chad/SK_Hero_1_Chad.SK_Hero_1_Chad` with scale `(X=1,Y=1,Z=1)`.
- That skeletal mesh was not proven to be the same playable hero-selection visual. In gameplay it produced an old headless small model plus an oversized model/head around the map.

Fix:

- Restore `Hero_1_Chad` to the QuadRetro static mesh row.
- Keep the skeletal experiment isolated until the correct QuadRetro replacement is validated.
- After the accepted QuadRetro pass was promoted, delete `Hero_1_Chad_AnimPilot`, the old root-level `SK_Hero_1_Chad` assets, and the pilot-specific tools/runs so the failed path cannot be selected or rerun by accident.

Rule:

- Never wire a skeletal pilot into a live hero-selection row until the exact selected runtime mesh, scale, attachments, camera behavior, animation response, and staged standalone output are visually verified in gameplay.

## 2026-05-14 Royal Chad QuadRetro UAL Retarget Pass

What was proven before replacing live Arthur:

- Normal Royal Chad/Arthur selection resolves to `Hero_1_Chad`.
- The correct live visual source was the QuadRetro static mesh `/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/SM_Hero_1_Chad_QuadRetro.SM_Hero_1_Chad_QuadRetro` and normalized Royal Chad texture.
- The correct source model was found at `Model Generation/Runs/Pixal3D/HeroArthur01/Post/QuadRetro/arthur_royal_chad/Models/arthur_royal_chad_QuadRetro.glb`.
- That source is static-only: one mesh, no armature, no actions, and no usable vertex groups.
- The old root-level `SK_Hero_1_Chad` pilot was not the selected Royal Chad visual and previously caused the old headless/giant-model failure when wired live.

What worked:

- `Tools/create_arthur_quadretro_ual_animation_source.py` imports the exact QuadRetro GLB, imports `External/Quaternius/Universal Animation Library Source/UAL1.blend`, builds a Rigodotify-style deform skeleton around the visible mesh, retargets UAL source motion, bakes the former live row scale `1.011123`, and exports the skeletal mesh plus action FBXs.
- The accepted UAL mapping is `Idle_Loop` -> idle, `Walk_Formal_Loop` -> walk, `Jump_Start` + `Jump_Loop` + `Jump_Land` -> jump, and `Roll_RM` -> roll.
- The accepted roll bake strips source root motion because T66 supplies actor-forward travel at runtime, then mirrors the roll target's local-X sagittal rotation component so the in-place clip tumbles forward instead of reading as a backflip.
- Root XY is kept in-place because T66 movement and roll direction are actor-driven, including the existing one-button roll path.
- `Tools/render_arthur_action_previews.py` supports front, side, three-quarter, and gameplay-camera views with an action-prefix filter and target armature selector.
- `Tools/make_preview_contact_sheets.py` builds per-view and all-view contact sheets from the multi-view manifest.
- `Tools/import_arthur_quadretro_animation_to_unreal.py` imports into `/Game/Characters/Heroes/Hero_1/Chad/QuadRetroUALQA`, creates a skeletal-safe unlit material instance, writes a temporary `Hero_1_Chad_QuadRetroUALQA` row, and reloads `DT_CharacterVisuals`.
- `Tools/verify_arthur_quadretro_animation_in_unreal.py` validates the skeletal mesh, skeleton, material parent, texture, four AnimSequences, temporary row, and promoted live row when `T66_ARTHUR_QUADRETRO_EXPECT_LIVE_PROMOTED=1`.
- `Scripts/RunRiggingAnimationToolAndExit.py` is the stable Unreal Python wrapper for tools under paths with spaces.

What did not work:

- The manual/procedural `QuadRetroAnimQA` pass was readable but not professional. It did not follow the intended UAL/Rigodotify-quality path and was deleted after the UAL retarget passed.
- Running the import through `UnrealEditor-Cmd.exe -run=pythonscript` hit a Slate application assertion in this path. The working import path is full editor `UnrealEditor.exe -ExecutePythonScript=...` with `T66_ARTHUR_QUADRETRO_QUIT_EDITOR=1`.
- Passing tool paths through 8.3 short names such as `MODELG~1` / `RIGGIN~1` can make Unreal's Python runner treat the path as Python text and fail with `SyntaxError`. Use the wrapper instead.
- Blender 5.1 layered action data can make old scripts report zero `action.fcurves` even when the action is valid. The inspection helper now counts nested/layered fcurves too.
- Direct source first-frame retargeting caused source-pose offsets. The accepted script neutralizes each source action against its first frame.
- Early weighting caused prop/cape/robe deformation artifacts. The accepted script uses spatial weighting, action-specific damping, and hidden source-armature handling to keep attachments stable.
- Initial front and three-quarter preview cameras were back-facing. They were corrected before accepting the contact sheets.
- A later reimport accidentally assigned the normalized static-mesh texture atlas to the GLB-derived skeletal material. That made the in-game hero texture appear scrambled even though the row still named a Royal Chad texture. The fixed path uses the original GLB-layout texture `/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/RoyalChad_QuadRetro/Textures/RoyalChad_QuadRetro_Pixelated_512.RoyalChad_QuadRetro_Pixelated_512` for the row and material instance.
- The same reimport carried over the old static row yaw and made Arthur walk backward. The promoted skeletal row now uses `MeshRelativeRotation=(Pitch=0,Yaw=90.000000,Roll=0)`.
- Switching only from UAL `Roll` to `Roll_RM` did not fix the roll direction once root motion was stripped; the baked local pose still opened backward from the side view. The accepted fix is the target-roll local-X sagittal mirror in `Tools/create_arthur_quadretro_ual_animation_source.py`, verified against side and gameplay-camera contact sheets before Unreal export.
- The importer module docstring must stay raw (`r"""..."""`) because the usage examples contain Windows paths such as `C:\UE\...`; a normal docstring can fail with a Python `unicodeescape` syntax error before the import runs.
- One full-editor reimport wrote the Unreal import report and saved the assets, then the editor exited with `-1073741819` during shutdown. Treat the report and the commandlet verifier as the source of truth before repeating the same full-editor import; do not keep rerunning imports just to chase a shutdown-only exit code.

Accepted evidence paths:

- Blender source: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Arthur_QuadRetro_UAL_Retarget.blend`
- Blender manifest: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/arthur_quadretro_ual_retarget_manifest.json`
- Contact sheets: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Arthur_All_Actions_{front,side,three_quarter,gameplay}_Contact_Sheet.png`
- Roll-specific proof sheets: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/AM_Hero_1_Chad_QuadRetroUALQA_Roll_side_contact_sheet.png` and `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/AM_Hero_1_Chad_QuadRetroUALQA_Roll_gameplay_contact_sheet.png`
- All-view sheet: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Arthur_All_Actions_All_Views_Contact_Sheet.png`
- Unreal import report: `Saved/ArthurQuadRetroAnimationImportReport.json`
- Unreal verification report: `Saved/ArthurQuadRetroAnimationVerifyReport.json`
- Old-pass cleanup report: `Saved/ArthurQuadRetroOldAnimQACleanupReport.json`
- Temporary in-game screenshot: `Saved/StandaloneLogs/ArthurQuadRetroUALTempHeroQA.png`
- Live staged in-game screenshot: `Saved/StandaloneLogs/ArthurQuadRetroUALLiveHeroQA.png`
- Live staged widget dump: `Saved/StandaloneLogs/ArthurQuadRetroUALLiveHeroQA.json`
- Live staged log evidence: `Saved/StagedBuilds/Windows/T66/Saved/Logs/T66.log` lines showing `VisualID=Hero_1_Chad`, `ResolvedRow=Hero_1_Chad`, `SK_Hero_1_Chad_QuadRetroUALQA`, and `AM_Hero_1_Chad_QuadRetroUALQA_Idle`.

Texture/forward reimport fix evidence:

- Unreal verification report: `Saved/ArthurQuadRetroAnimationVerifyReport.json` with `ok=true`, zero errors, live `Hero_1_Chad` row pointing at `SK_Hero_1_Chad_QuadRetroUALQA`, `PixelatedTextureAssetPath` set to `RoyalChad_QuadRetro_Pixelated_512`, `MeshRelativeRotation=(Pitch=0,Yaw=90.000000,Roll=0)`, and material texture params `EmissiveTexture`, `BaseColorTexture`, and `DiffuseColorMap` all set to the same original GLB-layout texture.
- Staged smoke screenshot: `Saved/StandaloneLogs/ArthurQuadRetroTextureForwardFix_HeroQA.png`.
- Staged smoke log: `Saved/StandaloneLogs/ArthurQuadRetroTextureForwardFix_HeroQA.log` with `HeroBase::InitializeHero ... VisualID=Hero_1_Chad` and no fatal, material compile, or default-material fallback lines.
- Staged standalone refresh: `Scripts/StageStandaloneBuild.ps1 -ClientConfig Development` exited successfully and updated both `T66 Standalone.lnk` shortcuts to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

Roll forward fix evidence:

- Unreal import report: `Saved/ArthurQuadRetroAnimationImportReport.json` with `source_dir` set to `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Exports` and `promoted_live_row=true`.
- Unreal verification report: `Saved/ArthurQuadRetroAnimationVerifyReport.json` with `ok=true`, zero errors, live `Hero_1_Chad` row pointing at `AM_Hero_1_Chad_QuadRetroUALQA_Roll`, and roll play length `1.4666666984558105`.
- Staged smoke screenshots/logs: `Saved/StandaloneLogs/ArthurRollForwardFix_HeroRollQA.png`, `Saved/StandaloneLogs/ArthurRollForwardFix_HeroRollQA.log`, `Saved/StandaloneLogs/ArthurRollForwardFix_HeroRollQA_MidRoll.png`, and `Saved/StandaloneLogs/ArthurRollForwardFix_HeroRollQA_MidRoll.log`.

Runtime note:

- The current row uses `LoopingAnimation` for walk, `AlertAnimation` for idle, `RunAnimation` for jump, and `RollAnimation` for the one-shot roll. The existing one-button roll behavior remains owned by gameplay movement and is still bound to Left Shift and Gamepad Face Button Right.

Cleanup note:

- Deleted `Hero_1_Chad_AnimPilot` from `Content/Data/CharacterVisuals.csv`.
- Deleted root-level old pilot assets under `/Game/Characters/Heroes/Hero_1/Chad`: `SK_Hero_1_Chad`, `SK_Hero_1_Chad_Skeleton`, `AM_Hero_1_Chad_{Idle,Walk,Jump,Roll}`, `AM_Hero_1_Chad_RigIdleV2`, and the leftover root `Image_0*` / `Material_0*` import artifacts.
- Deleted obsolete pilot tools: `create_arthur_animation_pilot.py`, `import_arthur_animation_pilot_to_unreal.py`, and `verify_arthur_animation_pilot_in_unreal.py`.
- Deleted obsolete pilot run folders: `Runs/Arthur_Animation_Pilot_20260514` and `Runs/Arthur_Animation_Pilot_ObjectDelta_20260514`.
- Deleted the rejected manual/procedural `QuadRetroAnimQA` Unreal assets, run folder, and generator tool after the UAL retarget replaced it.

## 2026-05-14 Enemy VAT Pipeline Exploration

What was inspected:

- `Content/Data/Enemies.csv`
- `Content/Data/Stages.csv`
- `Content/Data/CharacterVisuals.csv`
- `Source/T66/Gameplay/T66EnemyBase.h`
- `Source/T66/Gameplay/T66EnemyBase.cpp`
- `Source/T66/Gameplay/Enemies/T66EnemyFamilyResolver.cpp`
- `Source/T66/Gameplay/Enemies/T66FlyingEnemy.cpp`
- `Source/T66/Gameplay/Enemies/T66RushEnemy.cpp`
- `Source/T66/Data/T66DataTypes.h`
- `Source/T66/Gameplay/Enemies/pending_issues_Enemies.md`
- `Model Generation/Production/Roster_v1/AgentA/Report.md`
- `Model Generation/Production/Roster_v1/AgentB/Report.md`
- UE 5.7 local `AnimToTexture` plugin headers and source references

Current Difficulty 1 / Easy mob set:

- `Slime`
- `BoneWalker`
- `RatPack`
- `CaveBat`
- `HexSlinger`
- `TombSpider`
- `StoneSentinel`
- `MimicLure`
- `BoneConjurer`
- `CryptWraith`

Runtime visual finding:

- Easy mob `CharacterVisuals.csv` rows are static-only today.
- Their live static meshes resolve under `/Game/Characters/Mobs/<EnemyID>/SM_<EnemyID>`.
- Their skeletal mesh and animation slots are empty.
- `AT66EnemyBase::ConfigureAsMob(...)` applies the row through `UT66CharacterVisualSubsystem::ApplyCharacterVisual(...)`.
- `FT66CharacterVisualRow` has skeletal animation slots, but no dedicated vertex animation texture fields.

Source finding:

- Source GLBs for all ten Easy mobs exist under `Model Generation/Production/Roster_v1`.
- Agent A owns `Slime`, `RatPack`, `HexSlinger`, `StoneSentinel`, and `BoneConjurer`.
- Agent B owns `BoneWalker`, `CaveBat`, `TombSpider`, `MimicLure`, and `CryptWraith`.
- Promote Pixal3D GLB outputs only after visual and runtime acceptance.

Runtime behavior finding:

- The implemented enemy families currently resolve to `Melee`, `Rush`, `Ranged`, and `Flying`.
- Data archetypes such as `Exploder`, `Turret`, `Necromancer`, and `Stutterer` exist in the roster data, but matching runtime subclasses are currently missing.
- Do not claim that VAT animation work implements those behaviors. It can provide visual cues only until gameplay classes are added.

VAT tooling finding:

- UE 5.7 has an Experimental `AnimToTexture` plugin at `C:\Program Files\Epic Games\UE_5.7\Engine\Plugins\Experimental\AnimToTexture`.
- The plugin exposes `UAnimToTextureBPLibrary::AnimationToTexture(UAnimToTextureDataAsset*)`.
- The local headers expose vertex and bone texture modes plus 8-bit and 16-bit precision choices.
- This is the likely Unreal-side bake path for regular mob VAT work, but T66 still needs a project-owned runtime data seam and material/application path before production promotion.

Process result:

- Added `04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md` as the general regular-enemy vertex baked animation process.
- Added `05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md` as the first Difficulty 1 batch plan with source paths, live visual paths, scale values, behavior notes, and runtime caveats.

Open implementation gap:

- T66 needs a dedicated mob VAT data table/subsystem/material application path before any Easy mob VAT asset should be wired live.
- Do not reuse `CharacterVisuals.csv` skeletal animation slots for VAT clip state.
- Do not promote bake-only skeletal sources as live mob visuals.

## 2026-05-14 Easy Mob VAT Implementation

What was built:

- `Tools/create_easy_mob_vat_sources.py` builds the Easy batch Blender scene, behavior-specific rigs/actions, FBX exports, preview frames, and manifest.
- `Tools/make_easy_mob_contact_sheets.py` builds per-mob and all-mob contact sheets.
- `Tools/import_easy_mob_vat_to_unreal.py` imports bake-only skeletal sources, runs AnimToTexture, creates VAT static meshes, textures, material instances, writes `Content/Data/MobVertexAnimations.csv`, and reloads `/Game/Data/DT_MobVertexAnimations`.
- `Tools/verify_easy_mob_vat_in_unreal.py` verifies all ten rows, assets, frame ranges, material parameters, texture dimensions, fallback `CharacterVisuals.csv` preservation, and data table row names.
- Runtime support was added through `FT66MobVertexAnimationRow`, `UT66CharacterVisualSubsystem`, and `AT66EnemyBase`.

Output evidence paths:

- Source inspection: `Saved/EasyMobSourceInspection_20260514.json`
- Blender source: `Runs/Easy_Mob_VAT_20260514/Easy_Mob_VAT_Source.blend`
- Blender manifest: `Runs/Easy_Mob_VAT_20260514/easy_mob_vat_manifest.json`
- Contact sheets: `Runs/Easy_Mob_VAT_20260514/PreviewFrames/*_AllClips_AllViews_Contact_Sheet.png`
- Contact sheet index: `Runs/Easy_Mob_VAT_20260514/PreviewFrames/Easy_Mobs_AllClips_AllViews_Index_Contact_Sheet.png`
- Unreal import report: `Saved/EasyMobVATImportReport.json`
- Unreal verification report: `Saved/EasyMobVATVerifyReport.json`
- Staged gameplay smoke log: `Saved/StandaloneLogs/EasyMobVAT_GameplaySmoke_WithAudio.log`
- Staged gameplay smoke screenshot: `Saved/StandaloneLogs/EasyMobVAT_GameplaySmoke_WithAudio.png`

What worked:

- All ten Easy source GLBs import into Blender as single-mesh static sources.
- Procedural Blender rigs can produce first production-direction clips for blobs, flying mobs, humanoid enemies, swarms, spiders, casters, sentinels, mimics, conjurers, and wraiths while preserving vertex topology for VAT.
- AnimToTexture vertex mode produced nonzero position/normal textures for all ten mobs.
- Material instance bounds written by `UpdateMaterialInstanceFromDataAsset` are valid and can be copied into the runtime CSV.
- `DT_MobVertexAnimations` loads with exactly the ten Easy mob rows, and each row preserves the original fallback texture and scale from `CharacterVisuals.csv`.

What failed or needed correction:

- `UnrealEditor-Cmd.exe -run=pythonscript` hit a Slate application assertion during automated FBX import. Full `UnrealEditor.exe -ExecutePythonScript=...` with `Scripts/RunRiggingAnimationToolAndExit.py` is the working import path.
- Windows paths inside Python docstrings caused `unicodeescape` failures when `\U` appeared. Use raw docstrings or forward slashes in usage blocks.
- Backslash script paths can be mangled when passed to Unreal's Python runner. Use forward-slash paths.
- The first VAT bake used UV channel `1`, which conflicted with lightmap UVs. The accepted path uses UV channel `2` and a UV2 material master.
- Direct Python reads of AnimToTexture `FVector3f` data-asset bounds returned zeros even when material parameters and textures were valid. The importer and verifier now treat material `MinBBox`/`SizeBBox` as authoritative after `UpdateMaterialInstanceFromDataAsset`.
- A temporary material deletion probe triggered a `ForceDeleteObject` ensure when deleting a loaded material in the same commandlet. Avoid same-process deletion of loaded assets.
- Initial bat and spider preview motion was too subtle from gameplay camera. The generator was adjusted to strengthen bat wing and spider leg deformation before export.
- The first generated VAT material custom node called `TransformLocalVectorToWorld(local_delta)`. That produced cooked material failures and default-material fallback in staged standalone. The fixed material calls `TransformLocalVectorToWorld(Parameters, local_delta)`, and the importer now recreates the generated master material when custom HLSL changes.
- A smoke run before the final full stage exposed missing OGG/Vorbis runtime DLL handling. The final staged standalone launched without `-nosound`; audio initialized, `Lib vorbis DLL was dynamically loaded`, and `OGG` registered.
- A code-only restage is not enough after VAT material/content fixes. The accepted path was full cook/stage after material repair, then code-only restage only for the later QA-spawn-position adjustment.

Runtime and staged evidence:

- `Saved/EasyMobVATVerifyReport.json` has `data_table_row_count=10`, ten mob rows, zero errors, and only the accepted Python-bound warnings documented in the batch file.
- The staged QA capture spawned all ten Easy mobs with representative clip overrides: idle, move, attack cue, hit react, and death all appeared in one gameplay scene.
- The final staged log had no matches for VAT material compile failures, invalid shader maps, default-material fallback, fatal errors, critical errors, assertion failures, `libogg`, or `libvorbis`.
- Both `C:\UE\T66\T66 Standalone.lnk` and the pinned taskbar `T66 Standalone.lnk` target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

Remaining caveats:

- The Easy VAT rows are live-runtime QA assets, not final release art until final visual acceptance.
- Gameplay archetypes beyond `Melee`, `Rush`, `Ranged`, and `Flying` remain visual-only until the missing behavior classes are implemented.
- Current playback uses one dynamic material instance per enemy. This is suitable for first live QA, but crowd-scale optimization should move toward instanced playback once the enemy rendering path is ready.
