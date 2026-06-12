# Pending Issues - Scripts

## Resolved: Durable Save Integrity Gate Reloads A Stale Protected Slot [Major]

- Severity tag: [Major]
- What's wrong: After adding a local SHA-256 fallback for hosts where `Get-FileHash` is unavailable, `Scripts/RunDurableSaveIntegritySmokeGate.ps1` reaches the staged executable and passes the queue/shutdown phase, but the reload verification phase can load preexisting protected-slot metadata instead of the just-seeded integrity marker. The failing rerun reported `LoadedOk=0` and a loaded map from an older `T66_SessionLoadedTravel_*` marker while expecting the new `T66_SaveIntegrity_*` marker.
- Why it is out of scope now: The current pass is the Game Over ranking-pyramid/backend-mode wiring. Fixing the durable save harness or packaged save-root selection would be a separate save automation task, and the gate restored the protected save hashes after the failed run.
- What fixing it would entail: Trace the packaged Development save roots used by `T66.Save.QueueIntegrityShutdown` and `T66.Save.VerifyIntegrityReload`, isolate or clear only the intended harness slot for the current run, preserve protected-slot restore behavior, then rerun the full staged readiness gate without skipping durable validation.
- Resolution: Resolved 2026-06-09: root cause was slot collision — `RunSessionLoadedTravelSmoke.ps1` seeds slot 8 and the durable gate also defaulted to slot 8, so reload verification read the loaded-travel marker. Durable gate and `RunPreReleaseSmokeSuite.ps1` now default to slot 7 (slots 0-2 SaveSlots fixture, slot 8 loaded-travel fixture). Gate rerun pending the next staged readiness pass.

## Resolved: SaveSlots Load-Click Smoke Has A Protected Solo Slot Fixture [Major]

- Resolution: `Scripts/RunSaveSlotsLoadClickSmoke.ps1` now creates a deterministic local solo SaveSlots proof by snapshotting protected save roots, seeding a first-page slot through the development-only `T66.Save.QueueIntegrityShutdown` harness, opening `SaveSlots`, writing a pre-click dump, clicking `SaveSlots.SlotN.LoadButton` through non-shipping Slate tag automation, asserting the enabled-click log plus `TransitionToGameplayLevel` preload/open markers, and restoring the protected slot/index files afterward.
- Remaining note: This fixture intentionally proves the local solo loaded-save resume path only. Multiplayer-shaped loaded-save travel-plan proof now lives in `Scripts/RunSessionLoadedTravelSmoke.ps1`.

## Resolved: Session Loaded-Travel Smoke Has A Protected Duo Slot Fixture [Major]

- Resolution: `Scripts/RunSessionLoadedTravelSmoke.ps1` now creates a deterministic session-owned loaded-save travel-plan proof by snapshotting protected save roots, seeding a Duo host/guest save through the development-only `T66.Session.QueueLoadedTravelSeed` harness, reloading it in a fresh process through `T66.Session.VerifyLoadedTravelPlan`, asserting loaded-save metadata, owner/party shape, snapshot import, `UT66SessionSubsystem` apply-plan state, and computed gameplay `?listen` travel URL, then restoring the protected slot/index files afterward.
- Remaining note: This is intentionally plan-level standalone proof with `LiveTravelSkipped=1`. A real two-process host/client travel and remote-client join proof remains a future multiplayer automation layer, not a prerequisite for this owner-local harness.

## Pre-Release Frontend Smoke Can Fail On Top-Bar Navigation Dump Anchors [Major]

- Severity tag: [Major]
- What's wrong: `Scripts/RunPreReleaseSmokeSuite.ps1` launched `RunFrontendTagClickSmokeMatrix.ps1` during staged standalone readiness checks and the frontend matrix intermittently failed on top-bar navigation dump assertions. One run failed on `05_TopBarPowerUpNavigation` because the case log missed `Frontend automation: widget dump wrote`; a later rerun failed on `04_TopBarSettingsNavigation` because the dump missed `SettingsRetroFX.Root`. A final `-SkipStage` readiness rerun then passed the full frontend/durable/lifecycle suite, so this is not an active blocker but should be treated as a possible frontend smoke flake.
- Why it is out of scope now: The current pass is the SaveSlots loaded-save load-click fixture, not the full top-bar navigation smoke matrix. The SaveSlots fixture has a separate passing proof against the latest staged executable at `Saved/AgentReviews/SaveSlotsLoadClickSmokeFixture/save_slots_load_click_smoke_after_latest_stage/summary.json`, and the latest broader readiness rerun passed at `Saved/StagedBuildReadiness/20260607_164614/summary.json`.
- What fixing it would entail: Reproduce the failing top-bar matrix cases in isolation, inspect whether screen transition naming, root tags, dump delay, or dump command timing is racing, then adjust the matrix anchors/timing and rerun the full pre-release smoke suite.

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

## Resolved: Gameplay HUD Capture Script Uses Invalid Widget Dump Target

- Severity tag: [Minor]
- What's wrong: `Scripts/CaptureT66UIWidget.ps1` builds a gameplay widget dump argument as `-T66AutoDumpWidget="GameplayHUD:<path>"`, but the runtime dump parser rejects `GameplayHUD` with `Invalid target 'GameplayHUD'. Expected Class=, Tag=, ViewportIndex=, or Actor=.` Screenshots are still produced, but the script exits nonzero when the dump file is missing.
- Why it's out of scope now: The current fix is limited to restoring gameplay input/HUD after the atmosphere pass; changing the automation selector contract could affect other UI capture workflows.
- What fixing it would entail: Update the script to use a valid selector for the gameplay HUD, likely `Class=<UT66GameplayHUDWidget class path>` or a dedicated runtime tag, then add a smoke check that confirms both screenshot and JSON dump are created.
- Resolution: Resolved by current state (verified 2026-06-09): `Scripts/CaptureT66UIWidget.ps1` no longer hardcodes a `GameplayHUD` target — callers pass `-Target` and the runtime accepts `Class=`/`Tag=`/`ViewportIndex=`/`Actor=` selectors (`T66WidgetDumpTargets.cpp`). Used tonight with `Class=UT66CasinoOverlayWidget` for the casino diagnosis captures.

## Resolved: Frontend Capture Has Tag-Click Step [Minor]

- Resolution: `Scripts/CaptureT66UIScreen.ps1` now exposes `-ClickTag` / `-ClickDelaySeconds` / `-WaitForExit`, backed by the non-shipping runtime flag `-T66AutoClickTag=<Tag>`. The runtime resolves tags through the same active Slate/FlatStyle metadata path as `T66.UI.DumpWidget`, validates visible/enabled button geometry, and simulates tagged `SButton` clicks through Slate.
- Remaining note: This resolves deterministic FlatStyle button proof, including Quit confirmation. If a future control is not backed by `SButton`, add that control type deliberately instead of falling back to OS mouse injection.

## Resolved: Gameplay VFX Capture Timeout After Writing Nearly All Frames

- Severity tag: [Minor]
- What's wrong: The first Hero 1 AOE hitbox proof attempt wrote 71 Unreal-owned PNG frames and logged the complete `[Hero1AxeAOEHitboxProof]` hit/miss proof, but `Scripts/CaptureT66GameplayVideo.ps1` timed out waiting for the requested 72-frame capture to finish.
- Resolution: The final evidence was replaced by a clean 50-frame capture at `Saved/VideoCaptures/Hero1AxeAOE_Candidate03_NorthAuraHitbox_20260527_Clean50/`, which completed, encoded, and generated the evidence bundle without timeout.
- Remaining note: A future generic wrapper enhancement could still add retained-frame recovery metadata, but it is no longer blocking this Hero 1 AOE lab proof.

## Resolved: VFX Evidence Bundle Has Opt-In Auto Frame Selection [Minor]

- Severity tag: [Minor]
- Resolution: `Scripts/BuildT66VideoEvidenceBundle.py` now has an explicit `--auto-select-frames` mode that selects `start`, `mid`, `impact`, and `dissipate` from saturated/non-background frame activity while preserving manual `--selected-frames` overrides and the previous default fixed-index behavior. `Scripts/CaptureT66GameplayVideo.ps1` exposes this as `-EvidenceAutoSelectFrames`.
- Remaining note: Existing proof scripts keep manual/default frame selection unless the opt-in switch is passed. This preserves earlier proof reproducibility while giving future VFX packets an automated best-frame evidence path.
