Result: OK

## Independent Answer

**Likely cause of the blue/untextured raw imports.** GLB imports come in through Interchange as a StaticMesh plus a `MaterialInstanceConstant` parented to `MI_Default_Opaque → M_Default`. The project's render look depends on `MakeGLBImportsUnlit.convert_glb_imports_unlit()` to (1) read `BaseColorTexture` off the MIC, (2) reparent it to `M_GLB_Unlit`, and (3) re-set `BaseColorTexture` after reparent (UE clears overrides on reparent). "Fully blue" is the signature of an MIC that ends up with **no `BaseColorTexture` bound** — it either stayed on `M_Default` or landed in the converter's `no_texture` list. Concretely, the blue happens when one of these is true:
- Interchange named the GLB's diffuse param something other than `BaseColorTexture` (the converter's `_get_texture` falls back to `DiffuseColorMap`, but the **re-set** at `MakeGLBImportsUnlit.py:323` hardcodes `"BaseColorTexture"`, so a `DiffuseColorMap`-only asset reparents but never gets its texture re-applied → blue).
- The MIC/texture wasn't under the `scan_roots` passed at `ImportStaticMeshes.py:705-706`, so it was never converted.
- The embedded texture wasn't imported at all (raw FriendSlop GLBs may ship without an embedded image), so there is genuinely nothing to bind.

The `.fbx` branch (`ImportStaticMeshes.py:252-253`) force-disables `import_materials`/`import_textures`; the `.glb` branch does not, so a raw GLB *should* carry textures — the FBX flags are a red herring **unless** the FriendSlop assets were imported via the FBX sibling script (`ImportFriendSlopRawPixal3DFBXAndExit.py`), in which case textures are stripped by design and that is the bug.

**Safest texture-preservation repair path (read-only recommendation for Codex):**
1. Run the existing diagnostic `Scripts/DebugGLBRawImport.py` against the FriendSlop sources and read `Saved/Logs/DebugGLBRawImport.json` + the exported PNGs. That definitively shows, per asset, whether an embedded texture exists and which param name it lands under — don't guess, measure.
2. If textures exist but under `DiffuseColorMap`, fix the re-set in `MakeGLBImportsUnlit.py:323` to use the same param the read used (mirror the `_get_texture` fallback), rather than hardcoding `BaseColorTexture`.
3. Confirm `scan_roots` covers each new `/Pixal3D/` dest dir before the unlit conversion runs.
4. If the GLBs have **no** embedded texture, the repair is on the source/export side (re-export GLBs with embedded textures, or import the sibling PNG as the `BaseColorTexture`) — re-importing won't conjure a texture that isn't in the file.
5. Re-run the import and regenerate the report; the current `raw_unreal_import_report.json` shows all 49 as `ok:false` and is stale/inconsistent with the user's observation that some assets imported — do not trust it as evidence of current state.

**Old-model runtime reference surfaces.** The primary surface is `Content/Data/CharacterVisuals.csv` / `DT_CharacterVisuals` (`StaticMesh`, `SkeletalMesh`, `OutlineStaticMesh` columns). `Hero_1_Chad` currently points at the **ToonStyle** pipeline (`.../Chad/Pixal3DToonStyle/SM_Hero_1_Chad` + `AnimatedToonStyle/SK_Hero_1_Chad`), not the raw FriendSlop `/Pixal3D/` outputs — that is the "old model" the user is seeing. Enemies/NPCs route through `DT_Enemies`/`Enemies.csv` and `DT_NPCs`/`NPCs.csv` (all modified in the working tree). Replacement = repoint these rows to the generated `/Pixal3D/` mesh paths that have verified texture bindings. Also check actor/Blueprint hardcoded meshes for anything not data-driven.

## Evidence Checked
- `Scripts/ImportFriendSlopRawPixal3DAssetsAndExit.py`, `Scripts/DebugGLBRawImport.py`, `Scripts/MakeGLBImportsUnlit.py`, `Scripts/ImportStaticMeshes.py` (import_glb flags, flatten, unlit conversion order, bind/override flow).
- `Reports/AgentReviews/FriendSlopEasyPixal3D/raw_unreal_import_report.json` (stale: 49/49 `ok:false`).
- `Content/Data/CharacterVisuals.csv` header + Hero/QuadRetro rows; working-tree shows `DT_Enemies`, `DT_NPCs`, `Enemies.csv`, `NPCs.csv` modified.
- `AGENTS.md` §1–2 (task-contract authority, PPF, raw-import exception).
- Presence of sibling `ImportFriendSlopRawPixal3DFBXAndExit.py` (FBX path strips materials/textures by design).

## Questions Or Blockers
None that only the user can resolve — Codex can run the diagnostic, fix the param/scan-root issues, re-import, and repoint the data tables. Verification needs the staged standalone / in-editor run (`UnrealEditor.exe`, not `-Cmd`, for build settings), which is the normal Operator path, not a user block.

## Caveats
- **Hero static-vs-animated regression risk:** `Hero_1_Chad` uses an animated **skeletal** mesh with walk/idle/jump/roll montages. The generated FriendSlop output is a **raw static** Pixal3D GLB with no rig. Repointing the hero to the static mesh will drop animation. The user directed the swap, so it's in scope, but flag this tradeoff before committing the hero row — it may not be what they pictured for a playable hero.
- Confirm whether FriendSlop assets were imported via the GLB or the FBX sibling script; the diagnosis branches on that.
- Don't trust the existing import report; regenerate it as part of verification.
- Keep to the explicit raw-import exception — do not reintroduce ToonStyle/QuadRetro processing while fixing texture binding.
