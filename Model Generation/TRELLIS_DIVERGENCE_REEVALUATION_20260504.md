# TRELLIS Divergence Reevaluation - 2026-05-04

This note answers whether the current RunPod TRELLIS setup still needs its
deviations from the official `microsoft/TRELLIS.2` repo.

## Live Pod State

- Repo: `microsoft/TRELLIS.2`
- Commit: `5565d240c4a494caaf9ece7a554542b76ffa36d3`
- Health: `/health` returned `{"status":"ok","gpu":"NVIDIA A40"}`
- Modified upstream files:
  - `trellis2/pipelines/rembg/BiRefNet.py`
  - `trellis2/pipelines/samplers/flow_euler.py`
- Untracked T66 wrapper:
  - `/workspace/TRELLIS.2/trellis_server.py`

## Decision Summary

| Divergence | Current Decision | Reason |
| --- | --- | --- |
| `trellis_server.py` | Keep | This is a process wrapper, not a model-code fork. It gives T66 `/health`, repeatable `/generate`, RunPod curl/scp operation, and clean automation. |
| `flow_euler.py` `cfg_strength` pop | Candidate to remove | Current code search found no live `cfg_strength` caller except the patched line itself. The server passes `guidance_strength`, not `cfg_strength`. Restore official behavior in a test pod or temporary branch and run one smoke generation before deleting the patch from the locked process. |
| `BiRefNet.py` `rembg` replacement | Keep for current Mike pass, but downgrade from "canonical required" to "temporary compatibility patch" | Exact official BiRefNet currently fails in this environment because the model loads fp16 and the input tensor is fp32. A minimal dtype cast succeeds, so the long-term fix should be closer to official BiRefNet than the current `rembg` substitution. |

## BiRefNet Evidence

Exact official-style call in the current pod:

```text
official_birefnet_call=failed
RuntimeError: Input type (float) and bias type (c10::Half) should be the same
model_dtype=torch.float16
input_dtype=torch.float32
```

Official-style call with only this dtype adjustment:

```python
input_images = input_images.to(dtype=next(model.parameters()).dtype)
```

Result:

```text
official_birefnet_with_dtype_cast=ok (1, 1, 1024, 1024) torch.float16
```

## Recommended Next Steps

1. Finish the Mike head/body connection work on the current working server so
   the art-process experiment does not move under our feet.
2. After Mike connection is locked, make a temporary RunPod-side TRELLIS test:
   restore official `flow_euler.py`, keep everything else unchanged, restart
   `trellis_server.py`, and run one small known source through `/generate`.
   If it succeeds, remove the `cfg_strength` patch from the T66 setup notes.
3. Test a minimal official-compatible BiRefNet patch instead of the `rembg`
   replacement:

   ```python
   input_images = self.transform_image(image).unsqueeze(0).to("cuda")
   input_images = input_images.to(dtype=next(self.model.parameters()).dtype)
   ```

   Compare the alpha mask and one Mike source TRELLIS output against the current
   `rembg` path. Adopt it only if the visual mask and raw GLB are as good or
   better.
4. Keep `trellis_server.py`, but treat it as a T66 wrapper template. Do not
   count it as an upstream model-code divergence.

## Current Risk

The current `rembg` path is stable enough to continue Mike source and assembly
experiments, but it is not the cleanest long-term alignment with official
TRELLIS. The cleanest likely end state is:

- official `flow_euler.py`
- official BiRefNet with one dtype/device compatibility guard if needed
- T66-owned `trellis_server.py` wrapper documented outside upstream source
