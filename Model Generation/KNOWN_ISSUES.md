# Known Issues

This file captures the current technical and quality problems in the model-generation workflow.

## 1. Split Head/Body Generation Is Unstable

Symptom:

- head-only or split-body passes often complete but produce unusable geometry or empty-looking renders

Impact:

- split generation is not reliable enough to be the default path yet

Current mitigation:

- prioritize full-body generation first
- only revisit head splitting after the full-body baseline is stronger

## 2. Proxy Redraw Inputs Performed Worse Than Direct Screenshot Cutouts

Symptom:

- stylized proxy images led to severely broken meshes

Impact:

- proxy redraws are not a trustworthy baseline for benchmark reproduction

Current mitigation:

- use the most literal direct cutout from the in-game screenshot whenever possible

## 3. Alpha Cutouts Are Not Equivalent To The JSX Baseline

Symptom:

- alpha inputs bypass TRELLIS background removal and changed the conditioning path

Impact:

- alpha-based results are not a clean comparison with the older JSX workflow

Current mitigation:

- baseline tests now use opaque green-background PNGs

## 4. Environment Drift Can Break DINOv3

Symptom:

- with `transformers 5.5.4`, TRELLIS failed on DINO internals and required a temporary compatibility patch

Impact:

- outputs and debugging become unreliable if the pod drifts away from the locked baseline

Current mitigation:

- lock `transformers==5.2.0`
- keep `image_feature_extractor.py` on the repo-original implementation

## 5. SSH Tunnel Was Flaky For Long Runs

Symptom:

- local forwarded port would intermittently fail or reset during long work

Impact:

- local `curl` runs looked broken even when the pod job was still healthy

Current mitigation:

- use `scp` to upload inputs
- run `curl` on the pod against `127.0.0.1:8000`
- `scp` results back

## 6. Long Console Output Can Look Like A Hang

Symptom:

- generation can appear stuck for many minutes

Impact:

- easy to interrupt a job that is actually still processing normally

Current mitigation:

- remember the server is single-threaded
- parallel requests run serially
- `xatlas` and mesh postprocess can dominate runtime
- always check the server log before assuming failure

## 7. Equipment Grip Quality Requires Multi-Angle Validation

Symptom:

- a sword pose can look acceptable from one camera or even a small internal review loop and still read as detached, floating, or incorrectly held once the user sees it

Impact:

- equipment placement gets approved too early if it is judged from too few views or without explicit user confirmation

Current mitigation:

- require front, side, oblique, and user-like Blender checks before accepting a handheld-prop pose
- keep the current low-poly hero as the baseline
- treat transform-only attachment as the first pass, not the guaranteed solution
- improve the hand pose, grip socket, or rig step before treating the sworded hero as final

## 8. Blender MCP Can Start Disconnected

Symptom:

- Blender is installed locally, but Blender MCP sometimes is not reachable from Codex after a session crash or stale launch

Impact:

- live-scene automation can look unavailable even though Blender itself is fine

Current mitigation:

- reopen Blender and rerun [start_blender_mcp.py](C:/UE/T66/Model%20Generation/Tools/Trellis2/start_blender_mcp.py) first
- use the headless Blender helper only as the fallback path for render / decimate / export work

## 9. Decimated GLB Exports Can Emit Blender Warnings

Symptom:

- some decimated exports can warn that the mesh may be exported wrongly even when the asset still looks correct

Impact:

- it is easy to reject a usable low-poly export too early or trust a warning-free export without checking the actual file

Current mitigation:

- re-import the exported GLB in a fresh Blender session
- render a verification image from the exported file itself before Unreal import

## 10. Inline Local Image Previews In Codex Can Fail

Symptom:

- saved local PNGs can exist on disk, but the inline chat preview may still fail to render or may lag badly

Impact:

- it is easy to think an artifact is missing when the file is actually present
- review feedback gets slower if the chat preview is treated as the only inspection surface

Current mitigation:

- always save the review PNGs to disk even if they are also embedded inline
- prefer file links and the live Blender scene as the reliable review surface
- treat inline chat previews as optional convenience only
