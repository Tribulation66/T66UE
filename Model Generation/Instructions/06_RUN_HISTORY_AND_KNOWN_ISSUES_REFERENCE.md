# Run History And Known Issues

This file keeps durable lessons only. Full run folders, logs, renders, and local handoff prompts are cleanup targets.

## Durable Lessons

- Split head/body generation was unstable compared with one coherent source image.
- Proxy redraw and alpha-cutout inputs performed worse than clean direct source images.
- Environment drift can break TRELLIS dependencies; verify the RunPod environment before a batch.
- Fresh pods need Hugging Face auth before TRELLIS server start.
- Long console output can look like a hang; check logs and process state.
- CRLF shell scripts copied from Windows can fail on Linux pods.
- Blender MCP can start disconnected; relaunch the helper before falling back to headless-only work.
- Source PNG existence is not art approval.
- TypeA/Mike and QuadRetro legacy scripts are historical. Current work should prefer manifest-driven TRELLIS/Pixal3D, FriendSlop raw import, Blender QA, and Unreal import scripts.

## Completed Or Superseded Evidence

- Arthur-era prototype outputs were superseded and deleted during Alpha 0.1 cleanup.
- TypeA/Mike prototype scripts in root `Scripts` were deleted; any remaining legacy references should be treated as historical.
- Full `Model Generation/Runs`, `Scenes`, `Archive`, and `Reference` generated folders were deleted during cleanup. Recreate fresh run output from scripts/manifests if needed.

## Current Bias

Use fewer, better source images; run deterministic checks; keep scripts reusable; delete generated evidence once the result is imported, rejected, or summarized.
