All flags and parameters in the Codex draft check out against the actual scripts. Here's my review.

Result: OK

## Summary
Codex's draft is a clean evidence report of an executed setup-and-health pass on the Pixal3D pod. It reached the pod, confirmed the GPU as A40 (arch 8.6, matching the bootstrap's `PIXAL3D_NATTEN_SOURCE_ARCH=8.6`), ran bootstrap, did HF login, started the server via the smoke runner with generation/QA skipped, and captured a live `/health` showing `status: ok` and `pipeline_loaded: true`. Scope was respected — no generation, no Unreal import. The stop condition is met with concrete on-pod evidence.

## Suggested Answer Patch
None required. The draft is internally consistent and the command invocations match the actual scripts.

## Issues To Fix
- None blocking. Minor: Codex passed `--health-timeout 1800` while the runner default is `2400` (run_pixal3d_smoke.py:696). Not an error — just an intentional override worth a one-line note if Codex wants the report self-explaining.

## Question For User
None. This was executable Operator work with no user-only decision; the draft already completed it.

## Evidence Or Verification Gaps
- Verified against repo: `Invoke-Pixal3DHfLogin.ps1` uses `-Port` (not `-PodPort`), so Codex's `-Port 22079` is correct — note the independent answer's `-PodPort` suggestion was the wrong param name; Codex got it right.
- Verified all smoke-runner flags used (`--pod-ip/--pod-port/--upload-support/--start-server/--skip-generation/--skip-qa/--server-port/--health-timeout`) exist in run_pixal3d_smoke.py:661-696.
- Health JSON satisfies the independent answer's criteria: `status:ok`, `pipeline_loaded:true`, GPU name, and VRAM numbers all present.
- I did not independently re-run SSH or `/health`; I trust Codex's captured JSON and logs as the live evidence. No token appears in the draft — the no-token rule was honored.

## Notes
The arch decision (8.6 for A40) was correctly resolved on-pod via `nvidia-smi`, which was the main open operational unknown. Draft is ready to ship as-is.
