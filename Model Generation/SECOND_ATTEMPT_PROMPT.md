# Second Attempt Prompt

This file is historical. For the current continuation prompt, use [CURRENT_HANDOFF_PROMPT.md](C:/UE/T66/Model%20Generation/CURRENT_HANDOFF_PROMPT.md).

Use this prompt in a fresh Codex session after creating a new RunPod pod:

```text
Use [MASTER_WORKFLOW.md](C:/UE/T66/Model Generation/MASTER_WORKFLOW.md) as the authoritative source of truth for this task. The JSX file in Model Generation/Reference is historical reference only.

Start from a fresh RunPod pod and recreate the locked JSX-parity TRELLIS.2 environment exactly:
- use the recommended RunPod template from the master doc
- use transformers==5.2.0
- keep the BiRefNet -> rembg override
- keep the cfg_strength fix
- keep image_feature_extractor.py on the repo-original implementation
- use the canonical Flask server behavior

Use the pod bootstrap script at [bootstrap_trellis2_pod.sh](C:/UE/T66/Model Generation/Scripts/bootstrap_trellis2_pod.sh) as the setup reference, but verify the final environment against [ENVIRONMENT_LOCK.md](C:/UE/T66/Model Generation/ENVIRONMENT_LOCK.md).

For this attempt, do not start with split head/body generation. Start with full-body generation only.

Use this input as the primary baseline:
- [Arthur_LegacyDirect_Body_Green.png](C:/UE/T66/Model Generation/Runs/Arthur/Inputs/Arthur_LegacyDirect_Body_Green.png)

Generation rules:
- opaque green PNG input only
- preprocess_image=True
- X-Texture-Size=2048
- X-Decimation=200000
- generate multiple seeds, one at a time
- use pod-local curl plus scp, not the SSH tunnel, unless there is a strong reason otherwise

For each run:
- save the raw GLB into Model Generation/Runs/Arthur/Raw
- render a QA image in Blender into Model Generation/Runs/Arthur/Renders
- compare results against [MESH_APPROVAL_CHECKLIST.md](C:/UE/T66/Model Generation/MESH_APPROVAL_CHECKLIST.md)
- record the run in [RUN_HISTORY.md](C:/UE/T66/Model Generation/RUN_HISTORY.md)

Primary goal:
- find the best full-body raw Arthur mesh that clearly improves on the current best body pass

Secondary goal:
- only after a strong full-body result exists, evaluate whether a separate head pass is still necessary

Do not start retopo, weapon attachment, or Unreal import unless the raw full-body mesh is clearly acceptable.
```
