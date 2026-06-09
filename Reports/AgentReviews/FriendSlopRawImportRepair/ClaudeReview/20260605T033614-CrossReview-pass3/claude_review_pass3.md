Confirmed: hero rows now point to a static mesh with skeletal+all animation fields blanked, and the new reports/logs all exist as claimed. Evidence is real. My concern is the scope decision, not the execution.

Result: NEEDS_USER

## Summary
Codex completed the task end-to-end: guideline added, raw import path patched to bind textures explicitly (PNG → `MI_SM_<AssetID>` → `M_GLB_Unlit`, setting both `BaseColorTexture` and `DiffuseColorMap`), 49+1 assets re-imported with `ok=true`/`errors=[]`, data rows repointed, DataTables reloaded, staged build succeeded, and gameplay captures show textured non-blue meshes. I verified the report/log files exist and the CSV hero rows match the description. The texture-preservation diagnosis and fix are sound and match my independent read (the hardcoded-param / FBX-strips-materials issue). The blocker is not execution quality — it's that the swap silently converts the playable hero and 10 mobs to frozen static meshes (no walk/idle/jump/roll, no VAT), which is a gameplay regression the user likely did not picture when saying "replace the models."

## Suggested Answer Patch
In the Draft Final Status, lead the caveat with the playability impact, not the smoke failure. Suggested wording:

> "Texture preservation is fixed and references are repointed. **Important tradeoff to confirm before this is final:** `Hero_1_Chad` (the player character) and 10 enemies now render as fully static raw meshes — the hero loses all locomotion/idle/jump/roll animation and the 10 mobs lose their VAT motion, because the generated FriendSlop assets are unrigged static GLBs. This is a literal reading of 'use the models we generated.' If you expected the hero to stay animated, we should keep the ToonStyle skeletal mesh for the hero (and VAT for mobs) and only swap the genuinely-blue placeholder assets."

## Issues To Fix
- The Tradeoff section understates the severity: the hero is the player-controlled character, so blanking its animation fields is not a cosmetic swap. This should be surfaced as the headline caveat, above the unrelated Safe Zone smoke failure.
- Confirm the guideline doc records *why* the FBX path needed explicit PNG binding (FBX branch force-disables material/texture import), so the next raw import doesn't regress.

## Question For User
- The user must approve the animation loss. Two paths: (a) keep the swap as-is (hero + 10 mobs become static), or (b) only replace the truly-untextured placeholder assets and leave the animated hero/VAT mobs on their existing rigged meshes. The user's instruction is ambiguous between "visually use the new look" and "accept static, no animation," and only the user can resolve that.

## Evidence Or Verification Gaps
- Evidence is strong and verifiable: `raw_fbx_unreal_import_report.json` (49), `_hero1.json` (1), `raw_runtime_reference_validation.json`, reload log, stage log all present; CSV hero rows confirmed blanked + repointed.
- Gap: the captures prove the meshes are textured and non-blue, but do not prove the hero is *playable as a static mesh* (movement with no animation may look broken in motion). The user's own in-editor playthrough is the real check.
- Staged smoke logged `Pass=0` (Safe Zone bubble) — Codex calls it out-of-scope, which is plausible, but it means the staged build was not affirmatively smoke-passed.

## Notes
- Texture-preservation root cause and repair align with my independent answer; no correction needed there.
- Raw-import exception was respected (no ToonStyle/QuadRetro reintroduced). Good.
