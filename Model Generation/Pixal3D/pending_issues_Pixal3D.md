# Pending Issues - Pixal3D

## Loot Bag Yellow fails during CuMesh fill_holes at R1536 T4096

Severity: [Major]

What's wrong: `lootbag_yellow_v01.png` fails on the Pixal3D RunPod at the locked Phase 1A settings (`X-Resolution=1536`, `X-Texture-Size=4096`, default 12-step sampling). The failure happens before GLB export, inside `pixal3d_image_to_3d.py` -> `decode_latent()` -> `m.fill_holes()` -> CuMesh `get_vertex_edge_adjacency()`, with CUDA out-of-memory. Evidence is captured in `SourceAssets/ToonStyle/Pixal3D/Phase1A/LootBagYellowRetrySingle/Logs/09_lootbag_yellow_error.txt` and `pixal3d_generation_status.jsonl`.

Why it's out of scope now: Phase 1A.2 is validating the selected R1536/T4096 default-quality setting. Lowering resolution, changing the source image, patching Pixal3D internals, or adding a special low-memory decode path would change the comparison target and needs Pablo's approval.

What fixing it would entail: Try a focused retry matrix for this one asset: lower `X-Resolution`, lower `X-Max-Num-Tokens`, altered source silhouette/background cleanup, or a Pixal3D server-side low-memory mode around `decode_latent()` / `fill_holes()`. If the chosen fix changes generation settings, document the exception in the ToonStyle asset manifest before importing it into UE.

## Slime Phase 1C source fails Pixal3D fill_holes at original R1536 T4096 settings

Severity: [Major]

What's wrong: the Phase 1C Slime isolated PNG passes QA, but Pixal3D cannot generate a new Slime at R1536/T4096 default sampling. Failures repeated on the old A40 pod and the fresh dual-RTX-4090 pod, including CuMesh fill_holes OOM at `utils.h` line 42 / `connectivity.cu` line 419 and one normal-mode NAF conditioning OOM. Evidence lives under `SourceAssets/ToonStyle/Pixal3D/Phase1C/LineupBatch/Logs/03_slime/`, `Retries/03_slime_retry01/`, and `NewPod/03_slime_original*`. The Phase 1A Slime GLB was copied into the Phase 1C raw slot as a retained comparator.

Why it's out of scope now: Pablo accepted retaining the Phase 1A Slime model as the Phase 1C comparator, so this is no longer a Part 1 gate blocker. A regenerated Slime is deferred to a later Slime-only content pass.

What fixing it would entail: If Pablo later wants a regenerated Slime, run a focused Slime-only pass: retry R1024 T4096, regenerate a smaller/simpler closed silhouette, or use a larger single-GPU pod. Keep the retained comparator documented.

Hypothesis to investigate in a future Slime-only pass: the fill_holes failure repeats across materially different source images and across two different GPU configurations, which points at Slime's topology (small closed rounded form with thin eye/mouth features) triggering something specific in CuMesh fill_holes, not a memory budget issue alone.                                                                                                                                                                                                                                                                                                                           
## Loot Chest and Loot Crate no-remesh topology can misbehave at outline boundaries

Severity: [Major]

What's wrong: Phase 1C accepted Loot Chest and Loot Crate from Pixal3D's no-remesh export path because remesh export lost too much hard-surface detail. The Phase 1C diagnostic/remediation cleanup path (`Merge By Distance` plus normal recalculation) reduced non-manifold edges but did not fully restore clean topology. Outline rendering can still misbehave at affected topology boundaries on these two assets.

Why it's out of scope now: Phase 1C R1 is material/code remediation only and does not rerun or replace production Pixal3D assets. The no-remesh meshes are acceptable for the current production baseline as long as the limitation is explicit.

What fixing it would entail: Revisit these two assets in Phase 1D or a dedicated content pass with Pixal3D remesh enabled at adjusted settings, hand-decimation/retopology through Quad Remesher, or a broader content-pipeline replacement for hard-surface props.
## Safe export fill_holes fallback still needs topology review

Severity: [Minor]

What's wrong: The repo server can now recover from known export-time CuMesh `fill_holes` crashes by skipping that one hole-fill call in the child export worker. The resulting GLB may still contain open boundaries, different UV seams, or topology that behaves poorly with outline rendering, deformation, or later retopology. Check `X-Pixal3D-Export-Safe-Fill-Holes` and `X-Pixal3D-Export-Fill-Holes-Skipped` and `X-Pixal3D-Export-CPU-UV-Unwraps` in response headers or run reports before treating an output as clean.

Why it's out of scope now: This pass is a reliability fix for RunPod export failures and docs. It does not add a full manifoldness validator, topology repair pass, or Unreal import gate for safe-fill-holes outputs.

What fixing it would entail: Add automated mesh-quality checks after Blender import, record boundary/non-manifold counts in QA metadata, and route any safe-fill-holes output through Quad Retro or manual retopology before production import.

## Manual camera mode for AI portrait inputs is not wired into the T66 Pixal3D endpoint

Severity: [Major]

What's wrong: AI-generated full-body source portraits can make MoGe estimate an unstable implied camera, which can produce Pixal3D outputs that lean forward or backward. The desired production behavior is manual camera mode with a locked `manual_fov` of `45.0` degrees for these portrait-style inputs. However, the T66 Pixal3D server currently has no manual-camera request contract: `Model Generation/Pixal3D/Server/pixal3d_server.py` always loads MoGe, always calls `camera_params_from_image(...)`, and `/generate` accepts no camera mode or manual FOV header. The production wrapper and detached batch runner also have no `camera_mode` or `manual_fov` settings. Evidence and source-image FOV sampling are documented in `Saved/Codex/Pixal3D/ManualCameraMode/Implementation_Report.md`.

Why it's out of scope now: Pablo is moving back to visual assessment of the existing test-room models. This is a future-generation pipeline correction, not a blocker for inspecting models already generated/imported.

What fixing it would entail: Add server support for manual camera mode, likely via `X-Camera-Mode: manual` and `X-Manual-FOV: 45.0`, with response headers proving what camera path ran. Then wire `camera_mode` and `manual_fov` through `run_pixal3d_batch.py`, `run_pixal3d_toonstyle_production_import.py`, production manifests, `PipelineSpec.md`, and Pixal3D instructions. Production validation should require manual mode and FOV; MoGe/auto mode should be diagnostic-only. Regenerate exactly one previously leaning asset and verify uprightness with PCA or bounding-box measurements before broader reprocessing.
