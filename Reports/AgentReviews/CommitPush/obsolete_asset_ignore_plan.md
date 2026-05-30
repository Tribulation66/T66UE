# Obsolete Asset Ignore Plan

Task contract: Treat the currently deleted tracked asset files as obsolete, add narrow ignore rules so those exact obsolete/generated assets do not reappear as Git changes if regenerated, and keep runtime DataTable `.uasset` files tracked.

User decision: "Yes treat the currently deleted as obsolete."

Current evidence:
- `.gitattributes` routes `*.uasset`, `*.umap`, `*.png`, `*.jpg`, `*.tga`, `*.wav`, and `*.zip` through Git LFS.
- `.gitignore` already keeps raw source art and generated model runs local, but explicitly says to keep runtime `Content` assets in Git.
- Current deleted tracked LFS assets:
  - `Content/UI/Sprites/Interactables/QuickReviveIcon.uasset`
  - `Content/World/Cliffs/MI_HillTile1.uasset`
  - `Content/World/Cliffs/MI_HillTile2.uasset`
  - `Content/World/Cliffs/MI_HillTile3.uasset`
  - `Content/World/Cliffs/MI_HillTile4.uasset`
  - `Content/World/Cliffs/T_HillTile1.uasset`
  - `Content/World/Cliffs/T_HillTile2.uasset`
  - `Content/World/Cliffs/T_HillTile3.uasset`
  - `Content/World/Cliffs/T_HillTile4.uasset`
  - `Content/World/Interactables/Vending/Materials/MI_QuickReviveVending_Pixal3D.uasset`
  - `Content/World/Interactables/Vending/Materials/MI_QuickReviveVending_Pixal3D_Outline.uasset`
  - `Content/World/Interactables/Vending/Materials/M_QuickReviveVending_QuadRetro.uasset`
  - `Content/World/Interactables/Vending/QuickReviveVending_QuadRetro.uasset`
  - `Content/World/Interactables/Vending/SM_QuickReviveVending_Pixal3D.uasset`
  - `Content/World/Interactables/Vending/SM_QuickReviveVending_Pixal3D_Outline.uasset`
  - `Content/World/Interactables/Vending/Textures/T_QuickReviveVending_Pixal3D_0.uasset`
  - `Content/World/Interactables/Vending/Textures/T_QuickReviveVending_Pixal3D_1.uasset`
  - `Content/World/Interactables/Vending/Textures/T_QuickReviveVending_Pixal3D_InnerLines.uasset`
  - `Content/World/Interactables/Vending/Textures/T_QuickReviveVending_Pixal3D_Tint.uasset`
  - `Content/World/Interactables/Vending/Textures/T_QuickReviveVending_QuadRetro_Pixelated_512.uasset`
- Current modified tracked DataTable assets such as `Content/Data/DT_*.uasset` should remain tracked and are not part of this ignore/untrack plan.

Planned edit:
- Add a narrow `.gitignore` block for exact obsolete paths/patterns:
  - `Content/UI/Sprites/Interactables/QuickReviveIcon.uasset`
  - `Content/World/Cliffs/MI_HillTile*.uasset`
  - `Content/World/Cliffs/T_HillTile*.uasset`
  - exact QuickRevive vending material/mesh/texture paths under `Content/World/Interactables/Vending/...`
- Do not add a broad `*.uasset`, `Content/World/`, or `Content/` ignore.
- Stage the deleted obsolete tracked files plus `.gitignore` after the edit. Do not stage unrelated LFS asset changes yet.

Verification:
- `git check-ignore -v` on representative obsolete paths should show the new `.gitignore` rule.
- `git check-ignore` on `Content/Data/DT_Stages.uasset` should produce no ignore match.
- `git status --porcelain=v1 -uno -- <obsolete paths>` should show the deletions ready to stage or staged as deleted after `git add -u`/path-scoped staging.
- No reset, clean, restore, broad `git add -A`, or broad `git rm -r --cached Content` is allowed.

Remaining decision after this plan:
- Commit/push still needs final scope/version decision for the rest of the tracked changes.
