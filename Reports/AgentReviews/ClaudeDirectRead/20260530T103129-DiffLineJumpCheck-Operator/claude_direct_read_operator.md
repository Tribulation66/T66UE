## Read-only diagnosis: the +6.6M is **not** from the demo-gating/undeprication pass

**Answer to the question: No.** The ~6M-line jump was not created or touched by the 2026-05-30 demo-gating/undeprication code work. It comes from pre-existing untracked model-generation mesh exports that are slipping through both your LFS and ignore rules.

### Evidence (read-only, narrow)

**1. The files are plain-text mesh exports, not LFS, not ignored — the worst case for line counting.**
- `.gitattributes` (`C:\UE\T66\.gitattributes`): LFS patterns cover `.uasset/.umap/.ubulk/.uexp/.utexture/.udic/.upack/.png/.jpg/.tga/.wav/.zip/.zibravdb`. **No `*.obj` and no `*.mtl`.** → OBJ/MTL are *not* routed through LFS, so their full text is diffed.
- `.gitignore` (`C:\UE\T66\.gitignore` lines 48–66): under `Model Generation/Runs/**/` it ignores `*.blend/.blend1/.fbx/.glb/.png/.jpg/.jpeg/.mp4` and the `Raw/Renders/Exports/Logs/QA/` subfolders — but **omits `*.obj` and `*.mtl`**, and does not ignore the `AccuRig_Textured/` folder. → these exports are untracked *and* visible.
- No nested `.gitignore` under `Model Generation/Runs/**` (glob returned none), so the root rules govern. The only nested ignores are in `BlenderLabMCP` and `Rigging and Animation`, neither covering `Runs/Pixal3D`.

**2. The files match Codex's bucket and predate the demo-gating pass.**
- Confirmed 18 `.obj` + 18 `.mtl` under `Model Generation\Runs\Pixal3D\HumanoidGuidelineTest_20260522_100k\AccuRig_Textured\` (plus 10 more `.obj` in the sibling `HeroDemoLineup_20260522_AccuRig` run).
- Folder names are dated `20260522`; Codex's reported file timestamps are 2026-05-22 22:20–22:22. That is **8 days before** the 2026-05-30 demo-gating/undeprication work.
- The Read tool flagged the `.obj` as "binary" on its long-line heuristic, but Wavefront OBJ is ASCII vertex/face text — which is exactly why Git/your client counts ~6M additions on it (matching Codex's +6,095,525 from this bucket).

**3. The likely source of the misattribution: the `_Demo` filename suffix.** Files like `Hero_1_Chad_Demo_Textured.obj` and `Companion_BarMaidenBlonde_Demo` are **model-variant names from the 2026-05-22 Pixal3D run**, not artifacts of the 2026-05-30 demo-gating code. "Demo" in the path ≠ the demo-gating pass.

### What actually caused the UI to "suddenly" show it
The source-control client is counting every line of these untracked, non-LFS, non-ignored `.obj`/`.mtl` text meshes as additions. The 2026-05-30 pass only contributed the 201 tracked files (+7,427/-7,865) and the proof JSON dumps (hundreds of thousands of lines) — not millions.

The remaining "why now" question is a **hypothesis for Codex**: a recent working-tree cleanup (commits `ba0d1814 Apply approved working tree cleanup` / `5244ee78 Clean demo skin data without LFS assets`) likely rewrote `.gitignore` into the current extension-based list and omitted `*.obj`/`*.mtl`, newly exposing exports that an earlier broader rule had hidden.

### Handoff for Codex to validate (needs full Operator/git surface — this read-only profile can't run it)
1. `git log -p -- .gitignore` — confirm whether `*.obj`/`*.mtl`/`AccuRig_Textured/` were ever ignored and when that changed.
2. `git status --porcelain "Model Generation/Runs/Pixal3D/.../AccuRig_Textured/"` — confirm untracked (not staged).
3. Exact mtimes on the 18 `.obj` (expect 2026-05-22 22:20–22:22).
4. Confirm the +6M total is dominated by these `.obj`/`.mtl`.

### Recommended fix (for Codex to apply, consistent with the existing `.gitignore` intent "keep raw model runs … local")
Add to `.gitignore` under the Runs block:
```
Model Generation/Runs/**/*.obj
Model Generation/Runs/**/*.mtl
```
(or ignore `Model Generation/Runs/**/AccuRig_Textured/` wholesale). Verify no `.obj`/`.mtl` under `Runs/` is intended to be tracked before committing.

**Stop condition met:** the +6M is attributable to non-LFS, non-ignored untracked OBJ/MTL exports dated 2026-05-22 — not the demo-gating/undeprication pass.
