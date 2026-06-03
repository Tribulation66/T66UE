# Pending Issues - Scripts

## Static Mesh Import Core Still Owns Shared GLB Helpers

- Severity tag: [Minor]
- What's wrong: The final cleanup prompt approved deleting the old static GLB import workflow, but `Scripts/ImportStaticMeshes.py` and `Scripts/MakeGLBImportsUnlit.py` are still imported by active QuadRetro, CoherentThemeKit01, weapon projectile, world NPC/interactable, and generated-kit verification scripts. Deleting them now would break those active workflows.
- Why it's out of scope now: This cleanup pass can retire the old skeletal/generic import scripts, but replacing the shared static-mesh import core requires updating multiple active import families to a new shared helper first.
- What fixing it would entail: Extract the still-needed `ImportStaticMeshes.py` helpers into a smaller reusable module, update all active importers and verifiers to use it, then delete the old monolithic static import script and GLB unlit helper when no imports remain.

## Headless Interchange Import Can Crash After Saving

- Severity tag: [Major]
- What's wrong: `UnrealEditor-Cmd.exe -run=pythonscript` import tasks that route through Interchange can save static mesh or texture assets, then crash in headless mode with a Slate assertion like `CurrentApplication.IsValid()` / `ModeManagerInteractiveToolsContext None`. A full-editor `-ExecutePythonScript` import can also complete all imports and data-table saves, then hit `Object is not packaged: ModeManagerInteractiveToolsContext None` during shutdown. The PIXALTEST and PIXALTEST2 mob imports hit this while importing `/Game/Characters/Mobs/PIXALTEST` and `/Game/Characters/Mobs/PIXALTEST2`; the InteractablesPlaceholderPass01 import hit it after successfully writing its clean import report.
- Why it's out of scope now: The PIXALTEST asset was recovered and verified by splitting the workflow into a saved package import, a no-import texture-defaults pass, and a no-import verification pass. Replacing the shared import path would affect multiple active asset families.
- What fixing it would entail: Move commandlet imports onto a supported headless path, or run import tasks through full editor execution with a deterministic quit mechanism, then update the reusable import wrappers and validation docs.

## Full-Editor GLB Import Can Return No Assets

- Severity tag: [Major]
- What's wrong: The Phase 1A.1 Lu Bu matrix import called `Scripts/ImportStaticMeshes.py` against normalized GLBs through full-editor `-ExecutePythonScript`, but the GLB `AssetImportTask` completed with zero returned object paths for all six files. The pass recovered by converting the same normalized outputs to FBX and importing those with normals and tangents preserved, but that is a workflow fallback rather than a clean direct-GLB solution.
- Why it's out of scope now: Phase 1A.1 needed the six static meshes placed in the TestRoom for visual comparison, not a rewrite of the shared Interchange/static mesh import layer.
- What fixing it would entail: Reproduce the GLB zero-object-path behavior in a small isolated import test, determine whether it is an Interchange option issue or an automation/API limitation, then update `ImportStaticMeshes.py` and ToonStyle import wrappers to produce deterministic static mesh assets directly from GLB.

## Pixal3D FBX Fallback Emits Extensionless WebP Textures

- Severity tag: [Major]
- What's wrong: The Phase 1A.1 Lu Bu FBX fallback preserved Pixal3D textures as extensionless `Image_0` files under `SourceAssets/ToonStyle/Pixal3D/Phase1A/LuBu_Matrix/FBX/*_normalized.fbm/`. Those files are WebP payloads, but Unreal's FBX import did not import and bind them as usable material textures, leaving the TestRoom variants blue/teal and untextured until `ToonStyle/Source/ImportLuBuMatrixTexturesAndBindMaterials.py` converted them to PNG and assigned explicit material instances.
- Why it's out of scope now: The immediate fix only needed the six Lu Bu matrix variants visible for Pablo's settings comparison. Replacing the direct-GLB/import fallback path is broader ToonStyle pipeline work.
- What fixing it would entail: Teach the ToonStyle import path to extract embedded Pixal3D texture payloads deterministically, preserve file extensions/content types, import textures before mesh/material binding, and verify each static mesh slot references an expected texture-backed material.

## Gameplay HUD Capture Script Uses Invalid Widget Dump Target

- Severity tag: [Minor]
- What's wrong: `Scripts/CaptureT66UIWidget.ps1` builds a gameplay widget dump argument as `-T66AutoDumpWidget="GameplayHUD:<path>"`, but the runtime dump parser rejects `GameplayHUD` with `Invalid target 'GameplayHUD'. Expected Class=, Tag=, ViewportIndex=, or Actor=.` Screenshots are still produced, but the script exits nonzero when the dump file is missing.
- Why it's out of scope now: The current fix is limited to restoring gameplay input/HUD after the atmosphere pass; changing the automation selector contract could affect other UI capture workflows.
- What fixing it would entail: Update the script to use a valid selector for the gameplay HUD, likely `Class=<UT66GameplayHUDWidget class path>` or a dedicated runtime tag, then add a smoke check that confirms both screenshot and JSON dump are created.

## Frontend Capture Has No Tag-Click Step

- Severity tag: [Minor]
- What's wrong: `Scripts/CaptureT66UIScreen.ps1` can open a frontend screen, capture it, and dump its widget tree, but it cannot perform a deterministic Unreal-owned click on a tagged Slate widget before capture. Ad hoc OS mouse injection is unreliable with off-screen/DPI-scaled automation windows, which makes tab/dropdown/button interaction regressions harder to prove without manual input.
- Why it's out of scope now: The current pass fixes Run Summary tab/button wiring and only needs a normal Run Summary capture/log smoke; adding reusable click automation would touch the frontend automation contract.
- What fixing it would entail: Add a command-line automation path such as `-T66AutoClickTag=<Tag>` with an optional delay, resolve the tag through the widget tree/geometry, inject the click through Unreal/Slate, then capture/dump after the interaction and update `CaptureT66UIScreen.ps1` to expose it.

## Resolved: Gameplay VFX Capture Timeout After Writing Nearly All Frames

- Severity tag: [Minor]
- What's wrong: The first Hero 1 AOE hitbox proof attempt wrote 71 Unreal-owned PNG frames and logged the complete `[Hero1AxeAOEHitboxProof]` hit/miss proof, but `Scripts/CaptureT66GameplayVideo.ps1` timed out waiting for the requested 72-frame capture to finish.
- Resolution: The final evidence was replaced by a clean 50-frame capture at `Saved/VideoCaptures/Hero1AxeAOE_Candidate03_NorthAuraHitbox_20260527_Clean50/`, which completed, encoded, and generated the evidence bundle without timeout.
- Remaining note: A future generic wrapper enhancement could still add retained-frame recovery metadata, but it is no longer blocking this Hero 1 AOE lab proof.

## Resolved: VFX Evidence Bundle Has Opt-In Auto Frame Selection [Minor]

- Severity tag: [Minor]
- Resolution: `Scripts/BuildT66VideoEvidenceBundle.py` now has an explicit `--auto-select-frames` mode that selects `start`, `mid`, `impact`, and `dissipate` from saturated/non-background frame activity while preserving manual `--selected-frames` overrides and the previous default fixed-index behavior. `Scripts/CaptureT66GameplayVideo.ps1` exposes this as `-EvidenceAutoSelectFrames`.
- Remaining note: Existing proof scripts keep manual/default frame selection unless the opt-in switch is passed. This preserves earlier proof reproducibility while giving future VFX packets an automated best-frame evidence path.
