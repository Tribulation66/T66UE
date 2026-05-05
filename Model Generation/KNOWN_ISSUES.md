# Known Issues

This file captures the current technical and quality problems in the model-generation workflow.

## 1. Split Head/Body Generation Is Unstable

Symptom:

- head-only or split-body passes often complete but produce unusable geometry or empty-looking renders

Impact:

- split generation is not reliable enough to be the default path yet

Current mitigation:

- do not use split generation until the source PNG passes the Chad Pass02 gates
- body sources must be truly headless with an open neck socket
- head sources must be head plus a narrow insertable neck plug only
- Mike Pass02 showed split generation can work when the source silhouette is
  strict, but the head/body join still needs front, side, back, and close-neck
  review before rigging

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

- run [launch_blender_lab_mcp.ps1](C:/UE/T66/Model%20Generation/Tools/BlenderLabMCP/launch_blender_lab_mcp.ps1) first
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

## 11. PowerShell SSH Server Launch Can Break PATH

Symptom:

- launching the TRELLIS server from a Windows PowerShell SSH command can write `nohup: command not found` or `pgrep: command not found` to the server log if `$PATH` is escaped incorrectly

Impact:

- the pod can be fully bootstrapped, but the server never starts because basic Ubuntu tools are hidden by a bad launch-time `PATH`
- a process grep can also match the launch shell itself, making the server look alive when `/health` is still dead

Current mitigation:

- use absolute paths for the server launch from PowerShell: `/usr/bin/nohup` and `/opt/conda/envs/trellis2/bin/python`
- set `PATH=/usr/local/cuda/bin:/usr/bin:/bin:/opt/conda/envs/trellis2/bin` in the remote command
- verify readiness with `curl -s http://127.0.0.1:8000/health` and `/workspace/TRELLIS.2/trellis_server.log`, not process grep alone

## 12. First Generate Can Pause While rembg Downloads U2Net

Symptom:

- the first TRELLIS `/generate` request on a fresh pod appears slow before sampling starts
- server logs show `rembg` creating `/root/.u2net/u2net.onnx`

Impact:

- the pod is not necessarily hung; background removal is downloading its roughly 176 MB model cache
- later requests run normally once the cache exists

Current mitigation:

- prewarm the cache before a long batch:

```bash
python - <<'PY'
from PIL import Image
from rembg import remove
remove(Image.new("RGB", (8, 8), (0, 255, 0)))
PY
```

- verify `/root/.u2net/u2net.onnx` exists if first-job timing matters

## 13. Fresh Pods Need Hugging Face Auth Before TRELLIS Server Start

Symptom:

- `trellis_server.py` exits during `[TRELLIS] Loading pipeline...`
- the log reports `401 Client Error` for `facebook/dinov3-vitl16-pretrain-lvd1689m`
- `/health` never responds even though the conda environment and native extensions are installed

Impact:

- the pod can look fully bootstrapped but cannot load the image-conditioned pipeline
- repeated health polling will not recover it because the gated DINOv3 model needs authenticated Hugging Face access

Current mitigation:

- before launching `trellis_server.py` on a fresh pod, provide a Hugging Face token with access to `facebook/dinov3-vitl16-pretrain-lvd1689m`
- use `HF_TOKEN`/`HUGGING_FACE_HUB_TOKEN` or `huggingface_hub.login(...)` inside the pod; do not print the token in logs
- do not save raw Hugging Face tokens into repo docs, prompt files, checked-in scripts, or run notes
- verify auth with a small model-load or `hf auth whoami`, then start the server and check `curl -s http://127.0.0.1:8000/health`

## 14. Source PNG Existence Is Not Art Approval

Symptom:

- fallback or weak ImageGen PNGs can exist in `approved_seed_images` but still contain panels, floor cards, shadows, duplicated bodies, internal green holes, or non-headless bodies
- TRELLIS then turns those defects into poster geometry, flat-card bodies, malformed heads, or impossible head/body joins

Impact:

- `model_ready` can be true while the model is still art-invalid
- assembly constants can look wrong even though the real failure happened in the source PNG or raw GLB

Current mitigation:

- treat source PNG visual review as the first gate, before TRELLIS
- for Chad Pass02, prefer opaque flat-white source backgrounds with no alpha, floor, shadow, reflection, gradient, or border panel
- require body-only sources to be truly headless with a clean neck socket
- require head-only sources to be head plus neck only, with no shoulders or clothing
- reject raw body GLBs with near-flat side views or body depth under roughly `0.15m`

## 15. Windows-Copied Shell Scripts Can Keep CRLF

Symptom:

- a `.sh` copied from Windows to RunPod fails immediately with `set: pipefail\r: invalid option name`

Impact:

- the batch script is valid Bash, but Linux reads the carriage return as part of the option name

Current mitigation:

- generate checked-in RunPod scripts with LF line endings
- if a copied script still fails, run `sed -i 's/\r$//' path/to/script.sh` on the pod before `bash path/to/script.sh`

## 16. Roster DataTable Reload Should Not Reimport Audio WAVs

Symptom:

- running the full audio setup during a roster-only cleanup can exit the commandlet with a handled ensure: `Decoder for AudioFormat 'BINKA' not found`

Impact:

- the audio event JSON/DataTable rows may be valid, but the commandlet reports failure because it also tried to reimport/compress WAV assets

Current mitigation:

- use `Scripts/ReloadActiveHeroRosterDataTables.py` for roster changes
- that wrapper reloads `DT_AudioEvents` from JSON and skips WAV reimport

## 17. UE 5.7 Type A Skeletal Import Needs Full Editor, Blender Scale Bake, And Fresh Anim Names

Symptom:

- `UnrealEditor-Cmd.exe -run=pythonscript` can crash with a Slate assertion during the Type A skeletal mesh plus animation batch import
- using `-ExecCmds="py path"` treats the script path as Python source text instead of executing the file
- stale `RigIdle` animation assets can retain an invalid skeleton after a mesh skeleton is replaced, causing `Invalid USkeleton` or `FKControlRig` errors
- a single canonical import scale such as `100.0` can import Type A meshes thousands of centimeters tall instead of the intended roughly `200` cm gameplay height
- pre-normalized Type A exports can appear half buried in gameplay because `ACharacter` placement assumes a feet-origin skeletal mesh pivot

Impact:

- the rigged FBXs may be valid, but Unreal import appears broken or produces unusable scale
- gameplay placement can look wrong even when automated height checks pass
- stale animation assets can make later retries look corrupted even after the source FBX has been fixed

Current mitigation:

- import this batch through the full editor wrapper: `UnrealEditor.exe T66.uproject -ExecutePythonScript="C:/UE/T66/Scripts/RunImportTypeABatch01RiggedHeroesAndExit.py" -unattended -nop4 -nosplash`
- bake Type A rigged outputs in Blender to `2.0` meters tall with the mesh origin centered at the feet; import those FBXs with uniform scale `1.0`
- export/import the packed base-color PNGs from the Blender rig pass and repair the generated `M_Character_Unlit` material instances so they keep real textures
- keep `M_Character_Unlit` masked and route `DiffuseColorMap` alpha to `Opacity Mask`; Type A source textures use alpha cards, so opaque-only unlit materials can show white/poster panels even when the color texture is correctly bound
- if skeletons were replaced, write animations with a new suffix such as `RigIdleV2` instead of reusing stale animation asset names
- verify after import with `Scripts/VerifyTypeABatch01HeroVisuals.py` and require `24/24` rows, matching mesh/animation skeletons, near-200 cm bounds, bottom Z near zero, texture-backed material slots, and the masked `M_Character_Unlit` alpha path
- run the gameplay visual QA map/capture for human inspection before calling the import fixed; current proof images are `Saved/TypeABatch01VisualCheck/TypeABatch01_Gameplay_QA_standard_Yaw180.png` and `Saved/TypeABatch01VisualCheck/TypeABatch01_Gameplay_QA_beachgoer_Yaw180.png`
