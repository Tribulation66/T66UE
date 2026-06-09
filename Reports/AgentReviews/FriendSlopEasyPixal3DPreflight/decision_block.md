# Decision Block: FriendSlop Easy Pixal3D Source Backgrounds

## Current Task

Prepare to run the FriendSlop Easy Pixal3D batch from the locked source images, then generate, QA, import, wire, archive replaced assets, and verify staged standalone after user approval.

## Decision Needed

The locked FriendSlop source images under `FriendSlop/ImageGen` use black backgrounds. The current source-image rule in `Model Generation/Instructions/02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md` requires clean white backgrounds and says source-image gate failures should stop before staging or generation.

The attached user prompt also explicitly says to use only the locked FriendSlop source images and cites a successful black-background Hero 1 Pixal3D probe with no visible black-background geometry.

## Choices

1. Approve using the locked black-background FriendSlop sources as a task-specific override to the standing white-background source rule. Codex will treat black-background capture as a generation/Blender QA rejection condition, not as a pre-generation stop.
2. Do not approve the override. Codex will stop before generation and require corrected white-background source images or explicit source-regeneration approval.

## Current Evidence

- SSH to `root@69.30.85.136:22021` with `~/.ssh/id_ed25519` works.
- Pixal3D health on the pod returns `status: ok`, `pipeline_loaded: true`, GPU `NVIDIA A40`, local service port `18001`.
- The seven requested source folders contain exactly 49 PNGs.
- Automated source preflight flagged all 49 PNGs for non-white edges/black background.

## Wait State

No model generation, import, or runtime wiring should begin until Pablo answers this decision.
