You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\FriendSlopRawImportRepair\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\FriendSlopRawImportRepair\codex_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\FriendSlopRawImportRepair\ClaudeReview\20260605T030240-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
User request:

Ok cool so I went in and found some problems, first of all, the chad 1 hero, and most of the enemies still used the old model, instead of the new one, however a few things used the new ones, which were the weapon idol, loot bag and some enemies, that previously were using placeholders the issue is they came without a texture and were fully blue, so now we need to create the friendslop import guidelines doc and the first thing to figure out is how to ensure the texture of the GLB is preserved when importing. And then go ahead and replace the models that are not using the ones we generated with the new ones we generated

Working task:
Operator: Codex
Validator: Claude
Scope: create a FriendSlop raw Pixal3D import guideline, diagnose why imported GLB-derived assets lost textures, fix the raw import path so textures/materials are preserved, and replace old-model runtime references with generated FriendSlop models where applicable.
Stop condition: guideline exists, runtime references point to generated models where applicable, imported assets have material texture bindings, and the staged standalone is refreshed or blocked verification is explicitly reported.

Relevant repo rules:
- Follow C:\UE\T66\AGENTS.md and OPERATOR_VALIDATOR_PROTOCOL.md.
- Codex is Operator; Claude is read-only Validator.
- This task is an explicit FriendSlop raw-import exception. Do not force ToonStyle/QuadRetro processing.
- Runtime-facing generated model changes need Unreal import validation and staged standalone verification.
- Avoid broad Git/LFS scans over Unreal binary content.

Independent answer request:
Inspect the repo read-only and identify the likely cause of blue/untextured imported raw Pixal3D assets, the safest texture-preservation repair path, and likely old-model runtime reference surfaces for Chad 1 and enemies.

</original_prompt>

<validator_independent_answer>
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

</validator_independent_answer>

<codex_draft>
# Codex Draft - FriendSlop Raw Import Repair

## Task Contract

Operator: Codex
Validator: Claude
Scope: create a FriendSlop raw Pixal3D import guideline, preserve GLB-derived textures on raw imports, and replace old-model runtime references with generated FriendSlop models where applicable.
Stop condition: guideline exists, raw imports have material texture bindings, runtime rows point to generated raw meshes, DataTables are reloaded, and staged standalone verification is run or reported.

## Implementation Summary

- Added `Model Generation/Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md`.
- Updated Model Generation instruction routing/README to route FriendSlop raw Pixal3D imports to that doc.
- Patched `Scripts/ImportFriendSlopRawPixal3DFBXAndExit.py` so it no longer relies on FBX/MTL auto binding:
  - Imports `Textures/<AssetID>_00_Image_0.png` as `T_<AssetID>_BaseColor`.
  - Creates/updates `MI_SM_<AssetID>` under each target dir.
  - Parents to `/Game/Materials/M_GLB_Unlit`.
  - Sets `BaseColorTexture` and `DiffuseColorMap`.
  - Assigns the material instance to every imported `StaticMesh` slot.
  - Disables FBX auto material/texture import.
- Patched `Scripts/ValidateFriendSlopRawPixal3DAndExit.py` to validate material slots and texture params, not just mesh existence.
- Patched `Scripts/ReloadFriendSlopEasyPixal3DDataTablesAndExit.py` to reload `DT_MobVertexAnimations`.
- Added a small Hero 1 FriendSlop probe manifest at `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/FriendSlopProbe_Hero1Male_20260604_1415_manifest.json`.
- Exported Hero 1 probe GLB to a raw textured FBX bundle.
- Re-imported:
  - 49 FriendSlop Easy raw FBX assets.
  - 1 Hero 1 raw FriendSlop probe asset.
- Updated `Content/Data/CharacterVisuals.csv`:
  - `Hero_1_Chad` and `Hero_1_Chad_DemoSkin` now point to `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/SM_Hero_1_Chad_Male.SM_Hero_1_Chad_Male`.
  - Skeletal mesh and walk/idle/jump/roll animation fields are blank for those two rows so the runtime does not prefer the old animated ToonStyle skeletal mesh.
- Updated `Content/Data/MobVertexAnimations.csv`:
  - Disabled VAT rows for `Slime`, `CaveBat`, `BoneWalker`, `RatPack`, `TombSpider`, `HexSlinger`, `StoneSentinel`, `MimicLure`, `BoneConjurer`, and `CryptWraith` so the raw static `CharacterVisuals.csv` rows win.
- Documented unrelated staged smoke failure in `Source/T66/Gameplay/pending_issues_Gameplay.md`.

## Key Evidence

- Hero 1 Blender export:
  - Command: `python Model Generation/Pixal3D/Scripts/export_accurig_textured_batch.py --manifest ...FriendSlopProbe_Hero1Male_20260604_1415_manifest.json --run-root ...FriendSlopProbe_Hero1Male_20260604_1415 --output-root ...RawTexturedFBX --force`
  - Result: `errors=[]`, one FBX, two 4096 textures, height 180 cm.
- Hero 1 Unreal import:
  - Report: `Reports/AgentReviews/FriendSlopEasyPixal3D/raw_fbx_unreal_import_report_hero1.json`
  - `ok=true`, `errors=[]`, `asset_count=1`.
  - Material and both texture params bound to `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Textures/T_Hero_1_Chad_Male_BaseColor.T_Hero_1_Chad_Male_BaseColor`.
- 49-asset Unreal import:
  - Report: `Reports/AgentReviews/FriendSlopEasyPixal3D/raw_fbx_unreal_import_report.json`
  - `ok=true`, `errors=[]`, `asset_count=49`.
- Raw runtime validation:
  - Report: `Reports/AgentReviews/FriendSlopEasyPixal3D/raw_runtime_reference_validation.json`
  - `ok=true`, `errors=[]`.
  - Validates 49 raw imports load, material slots point to expected material instances, texture assets exist, `BaseColorTexture`/`DiffuseColorMap` match expected base-color textures, both Chad rows point to raw Hero 1 static mesh with skeletal/animations blank, and ten mob VAT rows are disabled.
- DataTable reload:
  - Log: `Reports/AgentReviews/FriendSlopEasyPixal3D/datatable_reload_repair.log`
  - Commandlet exited 0.
- Standalone stage:
  - Log: `Reports/AgentReviews/FriendSlopEasyPixal3D/stage_standalone_repair.log`
  - `BUILD SUCCESSFUL`, staged exe `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
  - Shortcut targets verified for repo and taskbar shortcuts.
- Staged smoke:
  - Log: `C:\UE\T66\Saved\StandaloneLogs\FriendSlopContentCorrectionsSmoke.log`
  - Process exited code 0, but content smoke summary logged `Pass=0` due out-of-scope `SafeZoneVisualBubblePresent`; documented as pending issue.

## Tradeoff

Replacing `Hero_1_Chad` and `Hero_1_Chad_DemoSkin` with the raw FriendSlop Pixal3D static mesh intentionally drops the old skeletal animation fields. This follows the user's direct instruction to use the generated models as generated, but it means the current FriendSlop hero render is static until a future raw FriendSlop rig/animation process exists.

## Draft Final Status

FriendSlop raw imports are repaired for texture preservation and runtime references. The game has been staged. The user can enter the staged game and try the updated raw models. The known caveat is that the staged content-corrections smoke has an unrelated Safe Zone visual bubble failure.

</codex_draft>
