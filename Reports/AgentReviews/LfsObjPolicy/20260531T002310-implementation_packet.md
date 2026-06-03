Operator Packet: Codex

Task Contract
- Working task: Implement the agreed OBJ policy change.
- Operator: Codex
- Validator: Claude
- Scope: `.gitattributes`, `.gitignore`, validation report artifacts, and index-only untracking of generated-run `.obj` files. Generated files may be staged as removals from Git with `git rm --cached`, but must not be deleted from disk or force-added.
- Stop condition: Durable `.obj` files resolve to Git LFS, generated Pixal3D run `.obj` files are ignored, and validation passes.

Implementation Plan
- Add a global Git LFS rule for `*.obj` so intentionally tracked OBJ mesh assets are stored through LFS.
- Add a narrow generated-run ignore for `Model Generation/Runs/**/*.obj`, matching the existing generated-run ignores for `.fbx`, `.glb`, image/video outputs, raw exports, logs, and QA folders.
- Keep existing generated outputs untouched.
- Do not broaden ignores to all `.obj` files because durable OBJ source/handoff assets should remain trackable through LFS.

Files Changed
- `.gitattributes`
  - Added: `*.obj filter=lfs diff=lfs merge=lfs -text`
- `.gitignore`
  - Added: `Model Generation/Runs/**/*.obj`
- Git index
  - Untracked 10 already-tracked `.obj` files under `Model Generation/Runs/Pixal3D/HeroDemoLineup_20260522_AccuRig/AccuRig_Textured/...` using `git rm --cached`.
  - The files still exist locally; they are removed only from the Git index.

Live Verification
- `git check-attr filter diff merge text -- "Model Generation/Runs/Pixal3D/sample.obj" "SourceAssets/Models/sample.obj"`
  - Both sample paths now report `filter: lfs`, `diff: lfs`, `merge: lfs`, `text: unset`.
  - These are synthetic sample paths used to verify rule behavior; they are not claimed to be real tracked files.
- `git check-ignore -v --no-index "Model Generation/Runs/Pixal3D/HeroDemoLineup_20260522_AccuRig/Processed/Hero_3_Stacy/Working/Hero_3_Stacy.obj"`
  - Matched `.gitignore:54:Model Generation/Runs/**/*.obj`.
- Path-limited count under `Model Generation/Runs`:
  - `VISIBLE_UNTRACKED_OBJ_UNDER_RUNS=0`
  - `IGNORED_OBJ_UNDER_RUNS=28`
  - 10 of these were previously tracked generated-run OBJ files and were untracked with `git rm --cached`; the other 18 were already untracked and are now hidden by the ignore rule.
- `git ls-files "*.obj"`
  - No output. There are no tracked `.obj` files remaining.
- `git lfs ls-files | Select-String -Pattern "\.obj"`
  - No output. There are no existing `.obj` LFS entries after untracking generated-run OBJ files.
- `git ls-files "Model Generation/Runs/**/*.obj"`
  - No output. There are no tracked generated-run `.obj` files remaining.
- Existence check for one formerly tracked generated-run OBJ:
  - `EXISTS=True`
  - `git check-ignore -v --no-index` matched `.gitignore:54:Model Generation/Runs/**/*.obj`.
- `git diff --check -- .gitattributes .gitignore`
  - No whitespace errors. Git emitted only line-ending normalization warnings.
- `git ls-files --eol -- .gitattributes .gitignore`
  - `.gitattributes`: `i/lf w/mixed attr/text eol=lf`
  - `.gitignore`: `i/lf w/lf attr/`
  - The warning is line-ending normalization metadata, not a whitespace error.
- `git status --short --untracked-files=no -- .gitattributes .gitignore "Model Generation/Runs" "Source/T66/UI/Screens/T66MinigamesScreen.cpp"`
  - Intended staged changes: `.gitattributes`, `.gitignore`, and 10 generated-run OBJ removals.
  - Known unrelated pre-existing unstaged tracked change remains: `Source/T66/UI/Screens/T66MinigamesScreen.cpp`.
  - The Mini/minigame file was not inspected or changed in this pass; it is included only as a narrow status sentinel for the known deferred change.

Out Of Scope
- No cleanup/delete of existing generated output folders.
- No broad tree status over Unreal binary folders.
- No commit or push.
- No Mini/minigame files inspected or changed.

Risks/Caveats
- Because all previously tracked `.obj` files were generated-run outputs, they were untracked instead of renormalized into LFS.
- Classification basis: all 10 formerly tracked OBJ files lived under `Model Generation/Runs/Pixal3D/HeroDemoLineup_20260522_AccuRig/AccuRig_Textured/...`; the Model Generation router says generated `Runs` are cleanup targets, not durable source.
- If a future durable `.obj` source asset is intentionally added outside ignored generated-run paths, the new `.gitattributes` rule will route it through LFS.
- Existing line-ending warnings were reported by Git for `.gitattributes` and `.gitignore`; `git diff --check` found no whitespace errors.
