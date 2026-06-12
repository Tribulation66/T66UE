Operator Packet: Codex

Task Contract
- Working task: Apply LFS coverage for `.mp4`, `.ogg`, `.fbx/.FBX`, `.glb`, `.blend`, `.blend1`, and `.npz`, after a cleanup classification pass, using the same keep-vs-generated cleanup idea as the OBJ pass.
- Operator: Codex
- Validator: Claude
- Scope: `.gitattributes`, `.gitignore`, targeted LFS renormalization of durable keepers, index-only untracking of generated cleanup candidates, and report artifacts. Do not delete local generated files from disk. Do not commit or push.
- Stop condition: Durable keepers are LFS-backed in the index, generated candidates are untracked/ignored, checks pass, and Claude validates the result.

Cleanup Classification Before Edits
- Keep and migrate to LFS:
  - 99 `.mp4` files under `Content/Movies/**`.
  - 5 `.ogg` files under `Content/Audio/**`.
  - 9 Content-owned FBX files under `Content/UE5RFX/**`, including one uppercase `.FBX`.
- Cleanup/untrack instead of migrate:
  - 14 `.glb` files under `Model Generation/Experiments/**`.
  - 8 `.fbx` files under `Model Generation/Experiments/**`.
  - 4 `.blend` files under `Model Generation/Experiments/**` or `BlenderViewers/**`.
  - 3 `.blend1` files under `Model Generation/Experiments/**`.
  - 20 `.npz` files under `Model Generation/Runs/**`.
- Classification basis:
  - Video Generation docs say shipped runtime movies belong under `Content/Movies`.
  - Model Generation docs say generated `Runs`, `Experiments`, scenes, and local outputs are cleanup targets, not durable source.
  - `BlenderViewers/Pixal3D_Experiment2/...` is a generated experiment viewer, so it follows the cleanup side.

Files Changed
- `.gitattributes`
  - Added LFS rules for `*.mp4`, `*.ogg`, `*.fbx`, `*.FBX`, `*.glb`, `*.blend`, `*.blend1`, and `*.npz`.
  - Keeps existing `*.obj` rule from the prior OBJ pass.
- `.gitignore`
  - Added `Model Generation/Runs/**/*.npz`.
  - Added narrow `BlenderViewers/**/*.blend` and `BlenderViewers/**/*.blend1` ignores.
  - Existing ignores already cover `Model Generation/Experiments/` and most generated run outputs.
- Git index
  - Ran targeted `git rm --cached` for 49 generated cleanup candidates. Local files remain on disk.
  - Ran targeted `git add --renormalize` for the 113 durable keepers.

Verification
- `git check-attr filter diff merge text` samples:
  - `Content/Movies/MainMenuBackground.mp4`: `filter=lfs`, `diff=lfs`, `merge=lfs`, `text` unset.
  - `Content/Audio/OSTS/Theme.ogg`: `filter=lfs`, `diff=lfs`, `merge=lfs`, `text` unset.
  - `Content/UE5RFX/Meshes/Examples/UE5RFX_N64_Sample.FBX`: `filter=lfs`, `diff=lfs`, `merge=lfs`, `text` unset.
  - `Model Generation/Experiments/Pixal3D_Experiment2/Outputs/Variant_D.glb`: `filter=lfs`, `diff=lfs`, `merge=lfs`, `text` unset.
  - `Model Generation/Experiments/Style_Lock_V2_Mushroom_Variants/Comparison.blend`: `filter=lfs`, `diff=lfs`, `merge=lfs`, `text` unset.
  - `Model Generation/Runs/Pixal3D/sample.npz`: `filter=lfs`, `diff=lfs`, `merge=lfs`, `text` unset.
- `git lfs ls-files -n` cross-check after renormalization:
  - `.mp4`: 99 tracked, 99 LFS-backed.
  - `.ogg`: 5 tracked, 5 LFS-backed.
  - `.fbx/.FBX` Content keepers: 9 tracked, 9 LFS-backed.
  - `.glb`: 0 tracked.
  - `.blend`: 0 tracked.
  - `.blend1`: 0 tracked.
  - `.npz`: 0 tracked.
- Index pointer sample checks:
  - `git cat-file -p :Content/Movies/MainMenuBackground.mp4` begins with the Git LFS pointer header.
  - `git cat-file -p :Content/Audio/OSTS/Theme.ogg` begins with the Git LFS pointer header.
  - `git cat-file -p :Content/UE5RFX/Meshes/Examples/UE5RFX_N64_Sample.FBX` begins with the Git LFS pointer header.
- Cleanup sample checks:
  - `Model Generation/Experiments/Pixal3D_Experiment2/Outputs/Variant_D.glb`: exists locally and is ignored by `Model Generation/Experiments/`.
  - `Model Generation/Experiments/Style_Lock_V2_Mushroom_Variants/Comparison.blend`: exists locally and is ignored by `Model Generation/Experiments/`.
  - `Model Generation/Runs/Pixal3D/HeroDemoLineup_20260522_AccuRig/Processed/Hero_1_Chad/Working/Hero_1_Chad_uv_triangles.npz`: exists locally and is ignored by `Model Generation/Runs/**/*.npz`.
  - `BlenderViewers/Pixal3D_Experiment2/Pixal3D_Experiment2_SideBySide.blend`: exists locally and is ignored by `BlenderViewers/**/*.blend`.
- `git ls-files` cleanup check:
  - No tracked files remain for `Model Generation/Experiments/**/*.glb`, `Model Generation/Experiments/**/*.fbx`, `Model Generation/Experiments/**/*.blend`, `Model Generation/Experiments/**/*.blend1`, `Model Generation/Runs/**/*.npz`, or `BlenderViewers/**/*.blend`.
- `git diff --cached --check -- .gitattributes .gitignore Content/Movies Content/Audio Content/UE5RFX "Model Generation/Experiments" "Model Generation/Runs" BlenderViewers`
  - Passed with no output.
- `git status --short --untracked-files=no` scoped to affected paths:
  - Shows `.gitattributes` and `.gitignore` staged.
  - Shows 99 `.mp4`, 5 `.ogg`, and 9 Content FBX files staged as modified due to LFS pointer normalization.
  - Shows 49 newly staged generated cleanup removals.
  - Shows the prior 10 OBJ generated-run removals still staged from the OBJ pass.
  - Known unrelated pre-existing `Source/T66/UI/Screens/T66MinigamesScreen.cpp` remains unstaged and untouched.

Out Of Scope
- No file deletion from disk.
- No commit or push.
- No runtime rebuild or staged standalone verification, because runtime media bytes were not edited; only Git storage representation was normalized to LFS pointers.
- No Mini/minigame work.

Risks/Caveats
- Existing historical Git blobs are not rewritten; this converts the current index/tip for future commits.
- A later commit/push will upload the newly normalized LFS objects.
- `.blend`/`.blend1` remain broadly ignored by existing `.gitignore`; durable Blender files can still be force-added or handled with a future explicit keep path if the repo needs one.
