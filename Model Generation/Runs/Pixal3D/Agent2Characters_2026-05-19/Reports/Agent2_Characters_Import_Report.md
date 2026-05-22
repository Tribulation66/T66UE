# Agent 2 Character Pixal3D ToonStyle Import Report

Date: 2026-05-19

Scope: Agent 2 character source pack only.

Source root: `C:\UE\T66\SourceAssets\ToonStyle\ModelSourceInputs\Pixal3D_NonEnemyModelSource_2026-05-19\Agent_2_Characters`

Run root: `C:\UE\T66\Model Generation\Runs\Pixal3D\Agent2Characters_2026-05-19`

Manifest: `C:\UE\T66\Model Generation\Pixal3D\production_asset_replacement_manifest.json`

Production report: `C:\UE\T66\Model Generation\Runs\Pixal3D\Agent2Characters_2026-05-19\Reports\Pixal3D_ToonStyle_Production_Import_Report.json`

Character data validation: `C:\UE\T66\Saved\Codex\Agent2CharacterVisualsValidation.json`

## Summary

- Imported all 26 PNGs from the Agent 2 character source folder.
- Updated `Content\Data\CharacterVisuals.csv` and reloaded `DT_CharacterVisuals`.
- Left `Content\Data\Heroes.csv` and `Content\Data\Companions.csv` as identity/unlock owners.
- Hero demo skin rows use runtime skin ID `Beachgoer`; the existing hero skin UI displays this as `Demo`.
- No `Hero_2_Chad_Beachgoer` or `Hero_2_Stacy_Beachgoer` rows were added.
- Added/replaced both Chad and Stacy visual rows where the README calls for male/female variants.
- Source image luminance/quality warnings were ignored per user override; source PNGs were not modified.
- Pixal3D export headers reported requested settings for every asset: `X-Pixal3D-Export-Decimation=200000`, `X-Pixal3D-Export-Remesh=1`, `X-Pixal3D-Export-Label=requested`.
- Fallbacks: none.

## Validation

- Pixal3D detached batch completed with 26 successful HTTP 200 rows in `Logs\pixal3d_generation_status.jsonl`.
- Blender ToonStyle foundation pipeline generated all 26 FBXs, outline FBXs, manifests, ToonStyle textures, close-the-gap B vertex colors, and inner-line textures.
- Production wrapper verify completed with `ok: true`, 26 assets, and zero asset errors.
- `Scripts\SetupCharacterVisualsDataTable.py` reloaded `DT_CharacterVisuals` successfully.
- `Saved\Codex\ValidateAgent2CharacterVisualsAndExit.py` verified 26 expected rows and reported zero errors.

## Imported Assets

| Source PNG | GLB | FBX | Imported Unreal StaticMesh | Target path | Data row | Validation | Fallback |
|---|---|---|---|---|---|---|---|
| `hero_1_chad_george_founding_male.png` | `Hero_1_Chad.glb` | `Hero_1_Chad.fbx` | `/Game/Characters/Heroes/Hero_1/Chad/Pixal3DToonStyle/SM_Hero_1_Chad.SM_Hero_1_Chad` | `/Game/Characters/Heroes/Hero_1/Chad/Pixal3DToonStyle` | `Hero_1_Chad` | verified | none; requested decimation 200000, remesh 1 |
| `hero_1_stacy_george_founding_female.png` | `Hero_1_Stacy.glb` | `Hero_1_Stacy.fbx` | `/Game/Characters/Heroes/Hero_1/Stacy/Pixal3DToonStyle/SM_Hero_1_Stacy.SM_Hero_1_Stacy` | `/Game/Characters/Heroes/Hero_1/Stacy/Pixal3DToonStyle` | `Hero_1_Stacy` | verified | none; requested decimation 200000, remesh 1 |
| `hero_2_chad_lubu_male.png` | `Hero_2_Chad.glb` | `Hero_2_Chad.fbx` | `/Game/Characters/Heroes/Hero_2/Chad/Pixal3DToonStyle/SM_Hero_2_Chad.SM_Hero_2_Chad` | `/Game/Characters/Heroes/Hero_2/Chad/Pixal3DToonStyle` | `Hero_2_Chad` | verified | none; requested decimation 200000, remesh 1 |
| `hero_2_stacy_lubu_female.png` | `Hero_2_Stacy.glb` | `Hero_2_Stacy.fbx` | `/Game/Characters/Heroes/Hero_2/Stacy/Pixal3DToonStyle/SM_Hero_2_Stacy.SM_Hero_2_Stacy` | `/Game/Characters/Heroes/Hero_2/Stacy/Pixal3DToonStyle` | `Hero_2_Stacy` | verified | none; requested decimation 200000, remesh 1 |
| `hero_3_chad_boxer_male.png` | `Hero_3_Chad.glb` | `Hero_3_Chad.fbx` | `/Game/Characters/Heroes/Hero_3/Chad/Pixal3DToonStyle/SM_Hero_3_Chad.SM_Hero_3_Chad` | `/Game/Characters/Heroes/Hero_3/Chad/Pixal3DToonStyle` | `Hero_3_Chad` | verified | none; requested decimation 200000, remesh 1 |
| `hero_3_stacy_boxer_female.png` | `Hero_3_Stacy.glb` | `Hero_3_Stacy.fbx` | `/Game/Characters/Heroes/Hero_3/Stacy/Pixal3DToonStyle/SM_Hero_3_Stacy.SM_Hero_3_Stacy` | `/Game/Characters/Heroes/Hero_3/Stacy/Pixal3DToonStyle` | `Hero_3_Stacy` | verified | none; requested decimation 200000, remesh 1 |
| `hero_4_chad_billy_cowboy_male.png` | `Hero_4_Chad.glb` | `Hero_4_Chad.fbx` | `/Game/Characters/Heroes/Hero_4/Chad/Pixal3DToonStyle/SM_Hero_4_Chad.SM_Hero_4_Chad` | `/Game/Characters/Heroes/Hero_4/Chad/Pixal3DToonStyle` | `Hero_4_Chad` | verified | none; requested decimation 200000, remesh 1 |
| `hero_4_stacy_billy_cowboy_female.png` | `Hero_4_Stacy.glb` | `Hero_4_Stacy.fbx` | `/Game/Characters/Heroes/Hero_4/Stacy/Pixal3DToonStyle/SM_Hero_4_Stacy.SM_Hero_4_Stacy` | `/Game/Characters/Heroes/Hero_4/Stacy/Pixal3DToonStyle` | `Hero_4_Stacy` | verified | none; requested decimation 200000, remesh 1 |
| `hero_5_chad_yakub_male.png` | `Hero_5_Chad.glb` | `Hero_5_Chad.fbx` | `/Game/Characters/Heroes/Hero_5/Chad/Pixal3DToonStyle/SM_Hero_5_Chad.SM_Hero_5_Chad` | `/Game/Characters/Heroes/Hero_5/Chad/Pixal3DToonStyle` | `Hero_5_Chad` | verified | none; requested decimation 200000, remesh 1 |
| `hero_5_stacy_yakub_female.png` | `Hero_5_Stacy.glb` | `Hero_5_Stacy.fbx` | `/Game/Characters/Heroes/Hero_5/Stacy/Pixal3DToonStyle/SM_Hero_5_Stacy.SM_Hero_5_Stacy` | `/Game/Characters/Heroes/Hero_5/Stacy/Pixal3DToonStyle` | `Hero_5_Stacy` | verified | none; requested decimation 200000, remesh 1 |
| `hero_1_chad_george_founding_male_demo.png` | `Hero_1_Chad_Beachgoer.glb` | `Hero_1_Chad_Beachgoer.fbx` | `/Game/Characters/Heroes/Hero_1/Chad/Beachgoer/Pixal3DToonStyle/SM_Hero_1_Chad_Beachgoer.SM_Hero_1_Chad_Beachgoer` | `/Game/Characters/Heroes/Hero_1/Chad/Beachgoer/Pixal3DToonStyle` | `Hero_1_Chad_Beachgoer` | verified | none; requested decimation 200000, remesh 1 |
| `hero_1_stacy_george_founding_female_demo.png` | `Hero_1_Stacy_Beachgoer.glb` | `Hero_1_Stacy_Beachgoer.fbx` | `/Game/Characters/Heroes/Hero_1/Stacy/Beachgoer/Pixal3DToonStyle/SM_Hero_1_Stacy_Beachgoer.SM_Hero_1_Stacy_Beachgoer` | `/Game/Characters/Heroes/Hero_1/Stacy/Beachgoer/Pixal3DToonStyle` | `Hero_1_Stacy_Beachgoer` | verified | none; requested decimation 200000, remesh 1 |
| `hero_3_chad_boxer_male_demo.png` | `Hero_3_Chad_Beachgoer.glb` | `Hero_3_Chad_Beachgoer.fbx` | `/Game/Characters/Heroes/Hero_3/Chad/Beachgoer/Pixal3DToonStyle/SM_Hero_3_Chad_Beachgoer.SM_Hero_3_Chad_Beachgoer` | `/Game/Characters/Heroes/Hero_3/Chad/Beachgoer/Pixal3DToonStyle` | `Hero_3_Chad_Beachgoer` | verified | none; requested decimation 200000, remesh 1 |
| `hero_3_stacy_boxer_female_demo.png` | `Hero_3_Stacy_Beachgoer.glb` | `Hero_3_Stacy_Beachgoer.fbx` | `/Game/Characters/Heroes/Hero_3/Stacy/Beachgoer/Pixal3DToonStyle/SM_Hero_3_Stacy_Beachgoer.SM_Hero_3_Stacy_Beachgoer` | `/Game/Characters/Heroes/Hero_3/Stacy/Beachgoer/Pixal3DToonStyle` | `Hero_3_Stacy_Beachgoer` | verified | none; requested decimation 200000, remesh 1 |
| `hero_4_chad_billy_cowboy_male_demo.png` | `Hero_4_Chad_Beachgoer.glb` | `Hero_4_Chad_Beachgoer.fbx` | `/Game/Characters/Heroes/Hero_4/Chad/Beachgoer/Pixal3DToonStyle/SM_Hero_4_Chad_Beachgoer.SM_Hero_4_Chad_Beachgoer` | `/Game/Characters/Heroes/Hero_4/Chad/Beachgoer/Pixal3DToonStyle` | `Hero_4_Chad_Beachgoer` | verified | none; requested decimation 200000, remesh 1 |
| `hero_4_stacy_billy_cowboy_female_demo.png` | `Hero_4_Stacy_Beachgoer.glb` | `Hero_4_Stacy_Beachgoer.fbx` | `/Game/Characters/Heroes/Hero_4/Stacy/Beachgoer/Pixal3DToonStyle/SM_Hero_4_Stacy_Beachgoer.SM_Hero_4_Stacy_Beachgoer` | `/Game/Characters/Heroes/Hero_4/Stacy/Beachgoer/Pixal3DToonStyle` | `Hero_4_Stacy_Beachgoer` | verified | none; requested decimation 200000, remesh 1 |
| `hero_5_chad_yakub_male_demo.png` | `Hero_5_Chad_Beachgoer.glb` | `Hero_5_Chad_Beachgoer.fbx` | `/Game/Characters/Heroes/Hero_5/Chad/Beachgoer/Pixal3DToonStyle/SM_Hero_5_Chad_Beachgoer.SM_Hero_5_Chad_Beachgoer` | `/Game/Characters/Heroes/Hero_5/Chad/Beachgoer/Pixal3DToonStyle` | `Hero_5_Chad_Beachgoer` | verified | none; requested decimation 200000, remesh 1 |
| `hero_5_stacy_yakub_female_demo.png` | `Hero_5_Stacy_Beachgoer.glb` | `Hero_5_Stacy_Beachgoer.fbx` | `/Game/Characters/Heroes/Hero_5/Stacy/Beachgoer/Pixal3DToonStyle/SM_Hero_5_Stacy_Beachgoer.SM_Hero_5_Stacy_Beachgoer` | `/Game/Characters/Heroes/Hero_5/Stacy/Beachgoer/Pixal3DToonStyle` | `Hero_5_Stacy_Beachgoer` | verified | none; requested decimation 200000, remesh 1 |
| `companion_01_light_skin_black_rap_vixen.png` | `Companion_01.glb` | `Companion_01.fbx` | `/Game/Characters/Companions/Companion_01/Default/Pixal3DToonStyle/SM_Companion_01.SM_Companion_01` | `/Game/Characters/Companions/Companion_01/Default/Pixal3DToonStyle` | `Companion_01` | verified | none; requested decimation 200000, remesh 1 |
| `companion_01_light_skin_black_rap_vixen_demo.png` | `Companion_01_Beachgoer.glb` | `Companion_01_Beachgoer.fbx` | `/Game/Characters/Companions/Companion_01/Beachgoer/Pixal3DToonStyle/SM_Companion_01_Beachgoer.SM_Companion_01_Beachgoer` | `/Game/Characters/Companions/Companion_01/Beachgoer/Pixal3DToonStyle` | `Companion_01_Beachgoer` | verified | none; requested decimation 200000, remesh 1 |
| `companion_02_blonde_tavern_barmaid.png` | `Companion_02.glb` | `Companion_02.fbx` | `/Game/Characters/Companions/Companion_02/Default/Pixal3DToonStyle/SM_Companion_02.SM_Companion_02` | `/Game/Characters/Companions/Companion_02/Default/Pixal3DToonStyle` | `Companion_02` | verified | none; requested decimation 200000, remesh 1 |
| `companion_02_blonde_tavern_barmaid_demo.png` | `Companion_02_Beachgoer.glb` | `Companion_02_Beachgoer.fbx` | `/Game/Characters/Companions/Companion_02/Beachgoer/Pixal3DToonStyle/SM_Companion_02_Beachgoer.SM_Companion_02_Beachgoer` | `/Game/Characters/Companions/Companion_02/Beachgoer/Pixal3DToonStyle` | `Companion_02_Beachgoer` | verified | none; requested decimation 200000, remesh 1 |
| `companion_03_brunette_college_girl.png` | `Companion_03.glb` | `Companion_03.fbx` | `/Game/Characters/Companions/Companion_03/Default/Pixal3DToonStyle/SM_Companion_03.SM_Companion_03` | `/Game/Characters/Companions/Companion_03/Default/Pixal3DToonStyle` | `Companion_03` | verified | none; requested decimation 200000, remesh 1 |
| `companion_03_brunette_college_girl_demo.png` | `Companion_03_Beachgoer.glb` | `Companion_03_Beachgoer.fbx` | `/Game/Characters/Companions/Companion_03/Beachgoer/Pixal3DToonStyle/SM_Companion_03_Beachgoer.SM_Companion_03_Beachgoer` | `/Game/Characters/Companions/Companion_03/Beachgoer/Pixal3DToonStyle` | `Companion_03_Beachgoer` | verified | none; requested decimation 200000, remesh 1 |
| `companion_04_black_haired_office_lady.png` | `Companion_04.glb` | `Companion_04.fbx` | `/Game/Characters/Companions/Companion_04/Default/Pixal3DToonStyle/SM_Companion_04.SM_Companion_04` | `/Game/Characters/Companions/Companion_04/Default/Pixal3DToonStyle` | `Companion_04` | verified | none; requested decimation 200000, remesh 1 |
| `companion_04_black_haired_office_lady_demo.png` | `Companion_04_Beachgoer.glb` | `Companion_04_Beachgoer.fbx` | `/Game/Characters/Companions/Companion_04/Beachgoer/Pixal3DToonStyle/SM_Companion_04_Beachgoer.SM_Companion_04_Beachgoer` | `/Game/Characters/Companions/Companion_04/Beachgoer/Pixal3DToonStyle` | `Companion_04_Beachgoer` | verified | none; requested decimation 200000, remesh 1 |
