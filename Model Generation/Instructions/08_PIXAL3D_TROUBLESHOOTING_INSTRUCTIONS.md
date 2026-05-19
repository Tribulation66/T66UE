# Pixal3D Troubleshooting

Use this doc when Pixal3D setup, generation, export, Blender QA, or Quad Retro
handoff behaves unexpectedly.

## First Checks

1. Confirm the server was uploaded from the current repo:
   `Model Generation/Pixal3D/Server/pixal3d_server.py`.
2. Confirm health:
   `curl -sS --max-time 10 http://127.0.0.1:18001/health`.
3. Confirm the pod is using the `pixal3d` conda environment.
4. Confirm `PYTHONPATH=/workspace/Pixal3D`.
5. Confirm generated outputs are under `Model Generation/Runs/Pixal3D/...`.

Do not debug Pixal3D failures by editing the TRELLIS server or TRELLIS scripts.
The pipelines are intentionally separate.

## Failure Table

| Symptom | Likely cause | Action |
| --- | --- | --- |
| `curl: empty reply from server` | The server process died, usually from native CUDA/CuMesh export code in an older server. | Upload the current `pixal3d_server.py`, restart with `--start-server`, and keep `X-Export-Fallback: 1`. |
| HTTP 500 with `GLB export worker failed` | The worker caught an export failure without killing the model server. | Inspect the response/log. Let fallback run. If all attempts fail, retry with lower `X-Decimation`, then `X-Remesh: 0`. |
| CuMesh `clean_up.cu` / `repair_non_manifold_edges` | Export-time mesh cleanup failed. This is not an image-to-shape failure. | Keep worker export enabled. The server tries fallback decimation, no-remesh, then the safe `fill_holes` / CPU UV unwrap fallback for known CuMesh CUDA error 9 / invalid-configuration crashes. |
| CuMesh `atlas.cu` / `uv_unwrap compute_charts` | Export-time UV atlas creation failed. | Same as above. If repeated, use the generated mesh package or source image for a narrower repro. |
| `no kernel image is available for execution on the device` | CUDA extension was built for the wrong GPU arch. | Rebuild `natten==0.21.0` for the pod GPU. A40 needs CUDA arch `8.6`; L40S needs CUDA arch `8.9`. |
| Hugging Face 401 or gated model error | Missing or stale HF auth. | Use `Scripts/Invoke-Pixal3DHfLogin.ps1` with `Model Generation/LOCAL_ACCESS.env`. Do not paste tokens into docs. |
| Blender opens only the default cube | The `.blend` was opened directly, or import script failed before saving/importing. | Check the Blender import log, use absolute paths, and verify the GLB exists before launching Blender. |
| PNG links do not open in chat | Relative paths or generated paths outside the repo were used. | Use absolute filesystem paths in Markdown image links or open a `.blend` scene with imported models. |
| GLB exists but looks wrong in Blender | Export succeeded but source/readability or orientation may be poor. | Run Blender QA, inspect bounds/triangle count, then apply Quad Retro only after visual review. |

## CuMesh Remesh Reality

The failing source files reported by CUDA errors, such as `clean_up.cu` and
`atlas.cu`, are not present as editable source on the current pod. The installed
package is a binary wheel:

```text
cumesh-0.0.1-...linux_x86_64.whl
```

The pod has Python wrappers plus compiled `.so` libraries. A true CUDA
source-level fix would require CuMesh source, a sanitizer repro, a patched build,
and a pinned replacement wheel. Until that exists, the repo-owned reliability
fix is worker-isolated export plus fallback.

For export-time `fill_holes` crashes such as CuMesh `Error code: 9` /
`invalid configuration argument`, the T66 server uses a final worker-local
fallback that wraps `cumesh.CuMesh.fill_holes`. It only runs after requested,
fallback-decimation, and no-remesh export attempts fail. The patch lives only in
the short-lived export worker, skips that one hole-filling call, bypasses the
CuMesh GPU UV charting kernel with CPU xatlas, and records the skip in response
headers so the output is not mistaken for a normal export.

## Export Fallback Contract

The T66 Pixal3D server exports GLBs in a child process so native CUDA failures do
not poison or kill the long-lived model server. The server attempts:

1. requested settings
2. requested remesh state with `X-Fallback-Decimation` (`30000` default)
3. fallback decimation with remesh disabled
4. fallback decimation with remesh disabled and safe CuMesh `fill_holes` skip plus CPU UV unwrap enabled

The response headers record what actually happened:

```text
X-Pixal3D-Export-Attempt
X-Pixal3D-Export-Label
X-Pixal3D-Export-Decimation
X-Pixal3D-Export-Remesh
X-Pixal3D-Export-Attempts
X-Pixal3D-Export-Safe-Fill-Holes
X-Pixal3D-Export-Fill-Holes-Skipped
X-Pixal3D-Export-CPU-UV-Unwraps
```

The smoke runner also writes `generation_attempts` into the run report. Check
that field before deciding whether a model came from requested remesh or a
fallback export.

## Known Production Baseline

Pixal3D is production-cleared for T66 replacement assets. The current
production target is:

```text
X-Seed: 1337
X-Resolution: 1536
X-Texture-Size: 4096
X-Decimation: 200000
X-Remesh: 1
X-Export-Fallback: 1
X-Fallback-Decimation: 80000
X-Safe-Fill-Holes-Fallback: 1
```

With worker-isolated export, earlier smoke tests proved the server can recover
from export failures. For production imports, any fallback below the requested
200k target is allowed only when the response headers/report surface it and the
asset passes Blender/ToonStyle validation.

## When To Lower Settings

Lower settings only after the current server and fallback path are confirmed.

Recommended sequence:

1. Keep `X-Remesh: 1`, lower `X-Decimation` to `80000`.
2. Keep `X-Decimation: 80000`, set `X-Remesh: 0`.
3. Leave `X-Safe-Fill-Holes-Fallback: 1` unless you need a strict repro of the
   original CuMesh crash.
4. Reduce texture size only if the error is clearly memory pressure, not CuMesh
   cleanup or UV atlas failure.

If a fallback output is intended for runtime content, run the production wrapper
verification and document the fallback state in the replacement manifest/report.

Do not change source-image style to work around CuMesh until export settings
and server version have been verified.

## Required Evidence Before Handoff

A Pixal3D run is not proven by a GLB path alone. Capture:

- `/health` response after server start
- command line or manifest settings
- GLB byte size
- response export headers or `generation_attempts`, including whether safe
  `fill_holes` fallback was used
- Blender QA render
- Blender QA metadata with triangle count and bounds
- Quad Retro report for character-like outputs when used

If any of those are missing, say exactly what was skipped and why.

## Avoid Long SSH Hangs

Do not run a multi-model Pixal3D batch as one long foreground SSH command. A
remote loop can finish every GLB while the local SSH/tool session still waits on
a stranded pipe, shell wrapper, or transport state. That makes Codex look hung
even though the pod is idle and the files are complete.

For batches longer than one model, use
`Model Generation/Pixal3D/Scripts/run_pixal3d_batch.py`. It implements this
process:

1. Upload a self-contained remote job script to the pod.
2. Launch it detached with `nohup` or `setsid`.
3. Write a PID file, append one JSONL status row per variant, and write a final
   `DONE` sentinel file with the exit code.
4. Poll status with short SSH commands every 30 to 60 seconds.
5. Treat the job as complete when either the `DONE` sentinel exists or all
   expected output GLBs exist, have nonzero byte size, and the status JSONL
   records HTTP 200 for every row.
6. Keep remote generation stdout/stderr redirected to a log file. Do not stream
   tqdm output through the controlling SSH command.

For one-off custom-parameter experiments, still prefer one short foreground
request per variant over a single remote loop. If a foreground request must be
used, set a local timeout close to the expected model time plus margin, not a
multi-hour timeout for the whole batch.
