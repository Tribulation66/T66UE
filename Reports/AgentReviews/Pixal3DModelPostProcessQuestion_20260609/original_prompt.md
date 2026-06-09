User request:

Was there any editing or post processing done on the models or is that the pure export from Pixal3D?

Task contract:

Working task:
Operator: Codex
Validator: Claude if the local T66 validator path is available
Scope: answer whether the generated GLBs were locally edited/post-processed, or whether they are the Pixal3D exports as downloaded.
Stop condition: verify the batch status/Blender metadata and give a concise answer.

Evidence:

- Batch status for `Hero1Stacy.glb` and `Hero2Chad.glb` shows HTTP 200 responses downloaded from the Pixal3D server.
- Response headers show Pixal3D export settings:
  - `X-Pixal3D-Export-Decimation: 200000`
  - `X-Pixal3D-Export-Remesh: 1`
  - `X-Pixal3D-Export-Attempts: 1`
  - `X-Pixal3D-Export-CPU-UV-Unwraps: 0`
  - `X-Pixal3D-Export-Safe-Fill-Holes: 0`
  - `X-Pixal3D-Export-Label: requested`
- Local Blender QA metadata has `export: null`, and `triangles_before_decimate` equals `triangles_after_decimate`.
- The side-by-side `.blend` imports the GLBs and applies scene placement/scale for review only; it does not overwrite the GLBs.

Draft answer:

The `Outputs/Hero1Stacy.glb` and `Outputs/Hero2Chad.glb` files are the Pixal3D server exports as downloaded. I did not locally edit, retopo, decimate, rig, cleanup, export, or otherwise post-process those GLB files after download.

One distinction: Pixal3D's own export step did run with the requested production export settings: remesh enabled and decimation target `200000`. That is part of the Pixal3D server export, not a separate local post-processing pass.

The Blender work was only for QA/review: importing the GLBs, rendering images, and placing the imported meshes side by side in a `.blend` scene. Those scene transforms do not modify the original GLB outputs.
