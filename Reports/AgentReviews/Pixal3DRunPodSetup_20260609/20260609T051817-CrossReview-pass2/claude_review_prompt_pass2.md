You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\Pixal3DRunPodSetup_20260609\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\Pixal3DRunPodSetup_20260609\codex_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\Pixal3DRunPodSetup_20260609\20260609T044329-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
# Original Prompt

User request:

> Set up Pixal 3D on this pod.

Screenshot-provided pod connection:

- Pod name: Pixal3D
- SSH direct TCP: `ssh root@69.30.85.73 -p 22079 -i ~/.ssh/id_ed25519`
- RunPod SSH proxy: `ssh 9nl6plrihaoybh-6441175b@ssh.runpod.io -i ~/.ssh/id_ed25519`

Task contract:

```text
Working task:
Operator: Codex
Validator: Claude if the local T66 validator path is available
Scope: set up the Pixal3D service on the RunPod shown in the screenshot, using the repo's current Pixal3D setup instructions and verify service health.
Stop condition: the pod is reachable, setup is run or resumed, and /health proves the service is up with the expected pipeline state, or the concrete blocker is reported.
```

Relevant repo rules:

- Follow `AGENTS.md` and `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Read the current Model Generation/Pixal3D routers and setup instructions.
- Use `Model Generation/Instructions/07_PIXAL3D_RUNPOD_SETUP_INSTRUCTIONS.md`.
- Use `Model Generation/Instructions/08_PIXAL3D_TROUBLESHOOTING_INSTRUCTIONS.md` for failures.
- Keep scope setup/health only; do not start generation batches or Unreal import unless separately requested.
- Do not paste or persist Hugging Face tokens in reports or chat.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

This is executable Operator work for Codex — no user-only decision blocks it. Codex should proceed with setup-and-health only, in this order:

1. **Reach the pod and identify the GPU first.** Use the direct TCP form from the screenshot (`root@69.30.85.73 -p 22079`) or the RunPod proxy. Run `nvidia-smi` before bootstrapping — the GPU arch is *not* in the screenshot and it determines the `natten` build: A40 → CUDA arch `8.6` (default), L40S → arch `8.9` (must export `PIXAL3D_NATTEN_SOURCE_ARCH=8.9`). Skipping this risks the `no kernel image is available for execution on the device` failure.
2. **Stage and run bootstrap.** Copy `Pixal3D/Scripts/bootstrap_pixal3d_pod.sh` to `/tmp/t66_pixal3d/` on the pod, then run it (with the arch override if L40S).
3. **HF auth if needed**, via `Invoke-Pixal3DHfLogin.ps1` with `-PodIp`/`-PodPort`. The helper reads the token from `Model Generation/LOCAL_ACCESS.env` (present in repo). Never echo/persist the token in reports or chat.
4. **Upload + start server** with the smoke runner using `--upload-support --start-server --skip-generation --skip-qa` (uses the current repo `pixal3d_server.py`).
5. **Verify health.** `curl http://127.0.0.1:18001/health` over SSH must return `"status":"ok"`, `"pipeline_loaded":true`, plus GPU name and VRAM numbers. Stop condition met when health passes; otherwise report the concrete blocker and consult doc `08`.

Do **not** start generation batches, smoke generation (`--force-generation`), or Unreal import — those are out of scope unless separately requested.

## Evidence Checked
- `07_PIXAL3D_RUNPOD_SETUP_INSTRUCTIONS.md` — pod shape, port 18001, bootstrap, arch rebuild rule, health criteria.
- `OPERATOR_VALIDATOR_PROTOCOL.md` — roles, scope boundaries, billing/token notes.
- Confirmed present: `Model Generation/LOCAL_ACCESS.env`, `run_pixal3d_smoke.py`.

## Questions Or Blockers
None requiring the user. Open operational unknowns Codex resolves on-pod: actual GPU model (drives natten arch), whether HF auth is required for this image, and whether a prior partial setup must be resumed vs. fresh.

## Caveats
- I did not SSH to the pod or run any command — this is a read-only Validator pass; reachability and `/health` are unproven until Codex actually runs them. Do not report success without a live health response captured this task.
- `08_PIXAL3D_TROUBLESHOOTING_INSTRUCTIONS.md` content was not opened here; rely on it only if setup fails.
- Honor the Claude Billing Guard and the no-token-in-reports rule throughout.

</validator_independent_answer>

<codex_draft>
# Pixal3D RunPod Setup Evidence

## Task Contract

Working task:
Operator: Codex
Validator: Claude if the local T66 validator path is available
Scope: set up the Pixal3D service on the RunPod shown in the screenshot, using the repo's current Pixal3D setup instructions and verify service health.
Stop condition: the pod is reachable, setup is run or resumed, and `/health` proves the service is up with the expected pipeline state, or the concrete blocker is reported.

## Pod

- Direct SSH: `root@69.30.85.73 -p 22079`
- RunPod pod ID from screenshot: `9nl6plrihaoybh`
- Hostname observed: `43c75bb1686f`
- GPU observed before setup: `NVIDIA A40, 8.6, 570.195.03, 46068 MiB`

## Setup Performed

1. Probed SSH and GPU with the screenshot's direct TCP endpoint.
2. Uploaded `Model Generation/Pixal3D/Scripts/bootstrap_pixal3d_pod.sh` to `/tmp/t66_pixal3d/bootstrap_pixal3d_pod.sh`.
3. Ran the bootstrap with A40 source-build settings:
   - `PIXAL3D_NATTEN_SOURCE_ARCH=8.6`
   - `PIXAL3D_NATTEN_N_WORKERS=16`
4. Ran `Model Generation/Pixal3D/Scripts/Invoke-Pixal3DHfLogin.ps1 -PodIp 69.30.85.73 -Port 22079`.
5. Started the service through the repo smoke runner without generation or QA:
   - `python "Model Generation/Pixal3D/Scripts/run_pixal3d_smoke.py" --pod-ip 69.30.85.73 --pod-port 22079 --upload-support --start-server --skip-generation --skip-qa --server-port 18001 --health-timeout 1800`
   - Runner output included `PIXAL3D_SERVER_PID=8377`.

## Evidence

Bootstrap log markers from `/tmp/t66_pixal3d/bootstrap.log`:

```text
Building NATTEN from source for CUDA arch 8.6
Successfully installed natten-0.21.0
torch 2.6.0+cu124
cuda 12.4
gpu NVIDIA A40
pixal3d pipeline import OK
Bootstrap complete.
```

Server process:

```text
8377 python -u /workspace/Pixal3D/pixal3d_server.py
```

Server log markers from `/workspace/Pixal3D/pixal3d_server.log`:

```text
[SPARSE] Conv backend: flex_gemm; Attention backend: flash_attn_3
[PIXAL3D] Loading pipeline from TencentARC/Pixal3D low_vram=False
[ATTENTION] Using backend: flash_attn_3
[PIXAL3D] Building image-conditioning models
[PIXAL3D] Preloading NAF upsamplers
[PIXAL3D] Loading MoGe camera model Ruicheng/moge-2-vitl
[PIXAL3D] Models loaded
[PIXAL3D] Starting server on port 18001
```

Health response from `curl http://127.0.0.1:18001/health` over SSH:

```json
{
  "attention_backend": "flash_attn_3",
  "cuda": true,
  "gpu": "NVIDIA A40",
  "loaded_at": 1780992997.584098,
  "low_vram": false,
  "model_path": "TencentARC/Pixal3D",
  "pipeline_loaded": true,
  "sparse_attention_backend": "flash_attn_3",
  "sparse_conv_backend": "flex_gemm",
  "status": "ok",
  "vram_allocated_gb": 20.2,
  "vram_reserved_gb": 20.3,
  "vram_total_gb": 47.7
}
```

## Scope Notes

- No generation batch was run.
- No Unreal import was run.
- The setup stopped at the requested service health verification.

</codex_draft>
