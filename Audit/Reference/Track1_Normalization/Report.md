# Track 1 Normalization Report

## Task 1 - QuadRetro Pipeline Luminance Normalization

- Function file: `Model Generation/Scripts/Core/QuadRetro/t66_quad_retro_character_pipeline.py`
- New function: `normalize_texture_luminance`
- Integration point: after diffuse bake and transparent-pixel dilation, before pixelation.
- Wrapper: `Model Generation/Scripts/Core/QuadRetro/RunQuadRetroCharacterPipeline.ps1`
- Wrapper parameters: `-NormalizeLuminance $true`, `-TargetLuminance 0.50`, `-MaxScalingFactor 4.0`, `-SaturationBoost 1.0`.
- Report keys added: `normalize_before_luminance`, `normalize_after_luminance`, `normalize_scaling_factor`, `saturation_boost_applied`.

Synthetic verification: a 64x64 mapped dark RGBA image with mean linear luminance `0.14996` was normalized to `0.50000` with scale `3.33423`. Result: PASS, within `0.50 +/- 0.02`.

## Task 2 - Retroactive Texture Normalization

- Worker script: `Scripts/RetroactivelyNormalizeCharacterTextures.py`
- Source CSV updated: `Content/Data/CharacterVisuals.csv`
- Data table reloaded and saved: `/Game/Data/DT_CharacterVisuals`
- Comparison images: `Audit/Reference/Track1_Normalization/Comparisons/`
- Result payload: `Audit/Reference/Track1_Normalization/retroactive_normalization_results.json`

Processed `75` textures. Failures: `0`.

| Metric | Before | After |
|---|---:|---:|
| Mean luminance | 0.0748 | 0.2151 |
| Min luminance | 0.0044 | 0.0176 |
| Max luminance | 0.2559 | 0.4842 |
| Stddev luminance | 0.0584 | 0.1239 |

Verification images:

Forest Mushroom Brute:

![Forest Mushroom Brute](Verification_Forest_MushroomBrute.png)

Hell Great Dragon:

![Hell Great Dragon](Verification_Hell_GreatDragon.png)

Dungeon Slime:

![Dungeon Slime](Verification_Dungeon_Slime.png)

Per-asset results:

| enemy_name | before_luminance | after_luminance | scaling_factor | new_asset_path |
|---|---:|---:|---:|---|
| Hero_1_Chad | 0.1345 | 0.3819 | 3.7184 | `/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/RoyalChad_QuadRetro/Textures/RoyalChad_QuadRetro_Pixelated_512_Normalized.RoyalChad_QuadRetro_Pixelated_512_Normalized` |
| Hero_2_Chad | 0.0708 | 0.2401 | 4.0000 | `/Game/Characters/Heroes/Hero_2/Chad/QuadRetro/ChineseChad_QuadRetro/Textures/ChineseChad_QuadRetro_Pixelated_512_Normalized.ChineseChad_QuadRetro_Pixelated_512_Normalized` |
| Hero_3_Chad | 0.1839 | 0.2806 | 2.7193 | `/Game/Characters/Heroes/Hero_3/Chad/QuadRetro/BoxerChad_QuadRetro/Textures/BoxerChad_QuadRetro_Pixelated_512_Normalized.BoxerChad_QuadRetro_Pixelated_512_Normalized` |
| Hero_4_Chad | 0.1707 | 0.3586 | 2.9298 | `/Game/Characters/Heroes/Hero_4/Chad/QuadRetro/FoundingChad_QuadRetro/Textures/FoundingChad_QuadRetro_Pixelated_512_Normalized.FoundingChad_QuadRetro_Pixelated_512_Normalized` |
| Hero_5_Chad | 0.1357 | 0.3570 | 3.6859 | `/Game/Characters/Heroes/Hero_5/Chad/QuadRetro/RoboChad_QuadRetro/Textures/RoboChad_QuadRetro_Pixelated_512_Normalized.RoboChad_QuadRetro_Pixelated_512_Normalized` |
| Hero_6_Chad | 0.0149 | 0.0598 | 4.0000 | `/Game/Characters/Heroes/Hero_6/Chad/QuadRetro/BillyChad_QuadRetro/Textures/BillyChad_QuadRetro_Pixelated_512_Normalized.BillyChad_QuadRetro_Pixelated_512_Normalized` |
| Hero_7_Chad | 0.0796 | 0.2879 | 4.0000 | `/Game/Characters/Heroes/Hero_7/Chad/QuadRetro/RabbitChad_QuadRetro/Textures/RabbitChad_QuadRetro_Pixelated_512_Normalized.RabbitChad_QuadRetro_Pixelated_512_Normalized` |
| Hero_8_Chad | 0.0352 | 0.1323 | 4.0000 | `/Game/Characters/Heroes/Hero_8/Chad/QuadRetro/CSChad_QuadRetro/Textures/CSChad_QuadRetro_Pixelated_512_Normalized.CSChad_QuadRetro_Pixelated_512_Normalized` |
| Hero_9_Chad | 0.1223 | 0.3468 | 4.0000 | `/Game/Characters/Heroes/Hero_9/Chad/QuadRetro/GambaChad_QuadRetro/Textures/GambaChad_QuadRetro_Pixelated_512_Normalized.GambaChad_QuadRetro_Pixelated_512_Normalized` |
| Hero_10_Chad | 0.0323 | 0.1027 | 4.0000 | `/Game/Characters/Heroes/Hero_10/Chad/QuadRetro/MonotoneChad_QuadRetro/Textures/MonotoneChad_QuadRetro_Pixelated_512_Normalized.MonotoneChad_QuadRetro_Pixelated_512_Normalized` |
| Hero_11_Chad | 0.0863 | 0.2862 | 4.0000 | `/Game/Characters/Heroes/Hero_11/Chad/QuadRetro/BaldChad_QuadRetro/Textures/BaldChad_QuadRetro_Pixelated_512_Normalized.BaldChad_QuadRetro_Pixelated_512_Normalized` |
| Hero_12_Chad | 0.0377 | 0.1418 | 4.0000 | `/Game/Characters/Heroes/Hero_12/Chad/QuadRetro/RoachChad_QuadRetro/Textures/RoachChad_QuadRetro_Pixelated_512_Normalized.RoachChad_QuadRetro_Pixelated_512_Normalized` |
| Saint | 0.1431 | 0.4690 | 3.4943 | `/Game/Characters/NPCs/Saint/QuadRetro/Textures/T_SM_Saint_QuadRetro_Pixelated_512_Normalized.T_SM_Saint_QuadRetro_Pixelated_512_Normalized` |
| Ouroboros | 0.0770 | 0.2418 | 4.0000 | `/Game/Characters/NPCs/Ouroboros/QuadRetro/Textures/T_SM_Ouroboros_QuadRetro_Pixelated_512_Normalized.T_SM_Ouroboros_QuadRetro_Pixelated_512_Normalized` |
| Gambler | 0.0251 | 0.0947 | 4.0000 | `/Game/Characters/NPCs/Gambler/QuadRetro/Textures/T_SM_Gambler_QuadRetro_Pixelated_512_Normalized.T_SM_Gambler_QuadRetro_Pixelated_512_Normalized` |
| Hero_1_Stacy | 0.1710 | 0.4197 | 2.9246 | `/Game/Characters/Heroes/Hero_1/Stacy/QuadRetro/RoyalStacy_QuadRetro/Textures/RoyalStacy_QuadRetro_Pixelated_512_Normalized.RoyalStacy_QuadRetro_Pixelated_512_Normalized` |
| Hero_2_Stacy | 0.0518 | 0.1630 | 4.0000 | `/Game/Characters/Heroes/Hero_2/Stacy/QuadRetro/ChineseStacy_QuadRetro/Textures/ChineseStacy_QuadRetro_Pixelated_512_Normalized.ChineseStacy_QuadRetro_Pixelated_512_Normalized` |
| Hero_3_Stacy | 0.0837 | 0.2524 | 4.0000 | `/Game/Characters/Heroes/Hero_3/Stacy/QuadRetro/BoxerStacy_QuadRetro/Textures/BoxerStacy_QuadRetro_Pixelated_512_Normalized.BoxerStacy_QuadRetro_Pixelated_512_Normalized` |
| Hero_4_Stacy | 0.2559 | 0.3631 | 1.9535 | `/Game/Characters/Heroes/Hero_4/Stacy/QuadRetro/FoundingStacy_QuadRetro/Textures/FoundingStacy_QuadRetro_Pixelated_512_Normalized.FoundingStacy_QuadRetro_Pixelated_512_Normalized` |
| Hero_5_Stacy | 0.2106 | 0.4593 | 2.3746 | `/Game/Characters/Heroes/Hero_5/Stacy/QuadRetro/RoboStacy_QuadRetro/Textures/RoboStacy_QuadRetro_Pixelated_512_Normalized.RoboStacy_QuadRetro_Pixelated_512_Normalized` |
| Hero_6_Stacy | 0.0505 | 0.1859 | 4.0000 | `/Game/Characters/Heroes/Hero_6/Stacy/QuadRetro/BillyStacy_QuadRetro/Textures/BillyStacy_QuadRetro_Pixelated_512_Normalized.BillyStacy_QuadRetro_Pixelated_512_Normalized` |
| Hero_7_Stacy | 0.0876 | 0.2888 | 4.0000 | `/Game/Characters/Heroes/Hero_7/Stacy/QuadRetro/RabbitStacy_QuadRetro/Textures/RabbitStacy_QuadRetro_Pixelated_512_Normalized.RabbitStacy_QuadRetro_Pixelated_512_Normalized` |
| Hero_8_Stacy | 0.0301 | 0.1117 | 4.0000 | `/Game/Characters/Heroes/Hero_8/Stacy/QuadRetro/CSStacy_QuadRetro/Textures/CSStacy_QuadRetro_Pixelated_512_Normalized.CSStacy_QuadRetro_Pixelated_512_Normalized` |
| Hero_9_Stacy | 0.1263 | 0.3620 | 3.9589 | `/Game/Characters/Heroes/Hero_9/Stacy/QuadRetro/GambaStacy_QuadRetro/Textures/GambaStacy_QuadRetro_Pixelated_512_Normalized.GambaStacy_QuadRetro_Pixelated_512_Normalized` |
| Hero_10_Stacy | 0.0463 | 0.1224 | 4.0000 | `/Game/Characters/Heroes/Hero_10/Stacy/QuadRetro/MonotoneStacy_QuadRetro/Textures/MonotoneStacy_QuadRetro_Pixelated_512_Normalized.MonotoneStacy_QuadRetro_Pixelated_512_Normalized` |
| Hero_11_Stacy | 0.0334 | 0.1224 | 4.0000 | `/Game/Characters/Heroes/Hero_11/Stacy/QuadRetro/BaldStacy_QuadRetro/Textures/BaldStacy_QuadRetro_Pixelated_512_Normalized.BaldStacy_QuadRetro_Pixelated_512_Normalized` |
| Hero_12_Stacy | 0.0188 | 0.0717 | 4.0000 | `/Game/Characters/Heroes/Hero_12/Stacy/QuadRetro/RoachStacy_QuadRetro/Textures/RoachStacy_QuadRetro_Pixelated_512_Normalized.RoachStacy_QuadRetro_Pixelated_512_Normalized` |
| Dungeon_SewerSlimeKing | 0.0462 | 0.1748 | 4.0000 | `/Game/Characters/Enemies/Bosses/Dungeon_SewerSlimeKing/QuadRetro/Dungeon_SewerSlimeKing_QuadRetro/Textures/Dungeon_SewerSlimeKing_QuadRetro_Pixelated_512_Normalized.Dungeon_SewerSlimeKing_QuadRetro_Pixelated_512_Normalized` |
| Dungeon_WebMatriarch | 0.0514 | 0.1951 | 4.0000 | `/Game/Characters/Enemies/Bosses/Dungeon_WebMatriarch/QuadRetro/Dungeon_WebMatriarch_QuadRetro/Textures/Dungeon_WebMatriarch_QuadRetro_Pixelated_512_Normalized.Dungeon_WebMatriarch_QuadRetro_Pixelated_512_Normalized` |
| Dungeon_BoneJailer | 0.0334 | 0.1313 | 4.0000 | `/Game/Characters/Enemies/Bosses/Dungeon_BoneJailer/QuadRetro/Dungeon_BoneJailer_QuadRetro/Textures/Dungeon_BoneJailer_QuadRetro_Pixelated_512_Normalized.Dungeon_BoneJailer_QuadRetro_Pixelated_512_Normalized` |
| Dungeon_BaelFallenChad | 0.0274 | 0.1050 | 4.0000 | `/Game/Characters/Enemies/Bosses/Dungeon_BaelFallenChad/QuadRetro/Dungeon_BaelFallenChad_QuadRetro/Textures/Dungeon_BaelFallenChad_QuadRetro_Pixelated_512_Normalized.Dungeon_BaelFallenChad_QuadRetro_Pixelated_512_Normalized` |
| Forest_BrambleTreant | 0.0437 | 0.1739 | 4.0000 | `/Game/Characters/Enemies/Bosses/Forest_BrambleTreant/QuadRetro/Forest_BrambleTreant_QuadRetro/Textures/Forest_BrambleTreant_QuadRetro_Pixelated_512_Normalized.Forest_BrambleTreant_QuadRetro_Pixelated_512_Normalized` |
| Forest_MyconidQueen | 0.1406 | 0.3235 | 3.5554 | `/Game/Characters/Enemies/Bosses/Forest_MyconidQueen/QuadRetro/Forest_MyconidQueen_QuadRetro/Textures/Forest_MyconidQueen_QuadRetro_Pixelated_512_Normalized.Forest_MyconidQueen_QuadRetro_Pixelated_512_Normalized` |
| Forest_ThornHive | 0.0339 | 0.1328 | 4.0000 | `/Game/Characters/Enemies/Bosses/Forest_ThornHive/QuadRetro/Forest_ThornHive_QuadRetro/Textures/Forest_ThornHive_QuadRetro_Pixelated_512_Normalized.Forest_ThornHive_QuadRetro_Pixelated_512_Normalized` |
| Forest_BuerVerdantChad | 0.0442 | 0.1667 | 4.0000 | `/Game/Characters/Enemies/Bosses/Forest_BuerVerdantChad/QuadRetro/Forest_BuerVerdantChad_QuadRetro/Textures/Forest_BuerVerdantChad_QuadRetro_Pixelated_512_Normalized.Forest_BuerVerdantChad_QuadRetro_Pixelated_512_Normalized` |
| Ocean_ReefCrabColossus | 0.0878 | 0.3422 | 4.0000 | `/Game/Characters/Enemies/Bosses/Ocean_ReefCrabColossus/QuadRetro/Ocean_ReefCrabColossus_QuadRetro/Textures/Ocean_ReefCrabColossus_QuadRetro_Pixelated_512_Normalized.Ocean_ReefCrabColossus_QuadRetro_Pixelated_512_Normalized` |
| Ocean_AbyssalJellyfish | 0.1171 | 0.3812 | 4.0000 | `/Game/Characters/Enemies/Bosses/Ocean_AbyssalJellyfish/QuadRetro/Ocean_AbyssalJellyfish_QuadRetro/Textures/Ocean_AbyssalJellyfish_QuadRetro_Pixelated_512_Normalized.Ocean_AbyssalJellyfish_QuadRetro_Pixelated_512_Normalized` |
| Ocean_DrownedCaptain | 0.0400 | 0.1585 | 4.0000 | `/Game/Characters/Enemies/Bosses/Ocean_DrownedCaptain/QuadRetro/Ocean_DrownedCaptain_QuadRetro/Textures/Ocean_DrownedCaptain_QuadRetro_Pixelated_512_Normalized.Ocean_DrownedCaptain_QuadRetro_Pixelated_512_Normalized` |
| Ocean_FocalorDrownedChad | 0.0385 | 0.1440 | 4.0000 | `/Game/Characters/Enemies/Bosses/Ocean_FocalorDrownedChad/QuadRetro/Ocean_FocalorDrownedChad_QuadRetro/Textures/Ocean_FocalorDrownedChad_QuadRetro_Pixelated_512_Normalized.Ocean_FocalorDrownedChad_QuadRetro_Pixelated_512_Normalized` |
| Martian_RedSandBehemoth | 0.0723 | 0.2688 | 4.0000 | `/Game/Characters/Enemies/Bosses/Martian_RedSandBehemoth/QuadRetro/Martian_RedSandBehemoth_QuadRetro/Textures/Martian_RedSandBehemoth_QuadRetro_Pixelated_512_Normalized.Martian_RedSandBehemoth_QuadRetro_Pixelated_512_Normalized` |
| Martian_CrystalMantis | 0.0835 | 0.2809 | 4.0000 | `/Game/Characters/Enemies/Bosses/Martian_CrystalMantis/QuadRetro/Martian_CrystalMantis_QuadRetro/Textures/Martian_CrystalMantis_QuadRetro_Pixelated_512_Normalized.Martian_CrystalMantis_QuadRetro_Pixelated_512_Normalized` |
| Martian_PlasmaSaucerPrime | 0.1077 | 0.2988 | 4.0000 | `/Game/Characters/Enemies/Bosses/Martian_PlasmaSaucerPrime/QuadRetro/Martian_PlasmaSaucerPrime_QuadRetro/Textures/Martian_PlasmaSaucerPri__QuadRetro_Pixelated_512_Normalized.Martian_PlasmaSaucerPri__QuadRetro_Pixelated_512_Normalized` |
| Martian_StolasAstralChad | 0.0468 | 0.1742 | 4.0000 | `/Game/Characters/Enemies/Bosses/Martian_StolasAstralChad/QuadRetro/Martian_StolasAstralChad_QuadRetro/Textures/Martian_StolasAstralChad_QuadRetro_Pixelated_512_Normalized.Martian_StolasAstralChad_QuadRetro_Pixelated_512_Normalized` |
| Hell_Horseman_Conquest | 0.2163 | 0.4842 | 2.3113 | `/Game/Characters/Enemies/Bosses/Hell_Horseman_Conquest/QuadRetro/Hell_Horseman_Conquest_QuadRetro/Textures/Hell_Horseman_Conquest_QuadRetro_Pixelated_512_Normalized.Hell_Horseman_Conquest_QuadRetro_Pixelated_512_Normalized` |
| Hell_Horseman_War | 0.0122 | 0.0490 | 4.0000 | `/Game/Characters/Enemies/Bosses/Hell_Horseman_War/QuadRetro/Hell_Horseman_War_QuadRetro/Textures/Hell_Horseman_War_QuadRetro_Pixelated_512_Normalized.Hell_Horseman_War_QuadRetro_Pixelated_512_Normalized` |
| Hell_Horseman_Famine | 0.0166 | 0.0651 | 4.0000 | `/Game/Characters/Enemies/Bosses/Hell_Horseman_Famine/QuadRetro/Hell_Horseman_Famine_QuadRetro/Textures/Hell_Horseman_Famine_QuadRetro_Pixelated_512_Normalized.Hell_Horseman_Famine_QuadRetro_Pixelated_512_Normalized` |
| Hell_Horseman_Death | 0.0211 | 0.0843 | 4.0000 | `/Game/Characters/Enemies/Bosses/Hell_Horseman_Death/QuadRetro/Hell_Horseman_Death_QuadRetro/Textures/Hell_Horseman_Death_QuadRetro_Pixelated_512_Normalized.Hell_Horseman_Death_QuadRetro_Pixelated_512_Normalized` |
| Hell_FalseProphet | 0.0405 | 0.1583 | 4.0000 | `/Game/Characters/Enemies/Bosses/Hell_FalseProphet/QuadRetro/Hell_FalseProphet_QuadRetro/Textures/Hell_FalseProphet_QuadRetro_Pixelated_512_Normalized.Hell_FalseProphet_QuadRetro_Pixelated_512_Normalized` |
| Hell_Antichrist | 0.0397 | 0.1555 | 4.0000 | `/Game/Characters/Enemies/Bosses/Hell_Antichrist/QuadRetro/Hell_Antichrist_QuadRetro/Textures/Hell_Antichrist_QuadRetro_Pixelated_512_Normalized.Hell_Antichrist_QuadRetro_Pixelated_512_Normalized` |
| Hell_GreatDragon | 0.0203 | 0.0538 | 4.0000 | `/Game/Characters/Enemies/Bosses/Hell_GreatDragon/QuadRetro/Hell_GreatDragon_QuadRetro/Textures/Hell_GreatDragon_QuadRetro_Pixelated_512_Normalized.Hell_GreatDragon_QuadRetro_Pixelated_512_Normalized` |
| Dungeon_Slime | 0.1841 | 0.3780 | 2.7160 | `/Game/Characters/Enemies/Regular/Dungeon_Slime/QuadRetro/Dungeon_Slime_QuadRetro/Textures/Dungeon_Slime_QuadRetro_Pixelated_512_Normalized.Dungeon_Slime_QuadRetro_Pixelated_512_Normalized` |
| Dungeon_Skeleton | 0.1383 | 0.4370 | 3.6156 | `/Game/Characters/Enemies/Regular/Dungeon_Skeleton/QuadRetro/Dungeon_Skeleton_QuadRetro/Textures/Dungeon_Skeleton_QuadRetro_Pixelated_512_Normalized.Dungeon_Skeleton_QuadRetro_Pixelated_512_Normalized` |
| Dungeon_WebSpider | 0.2061 | 0.3464 | 2.4263 | `/Game/Characters/Enemies/Regular/Dungeon_WebSpider/QuadRetro/Dungeon_WebSpider_QuadRetro/Textures/Dungeon_WebSpider_QuadRetro_Pixelated_512_Normalized.Dungeon_WebSpider_QuadRetro_Pixelated_512_Normalized` |
| Dungeon_RabidRat | 0.0202 | 0.0791 | 4.0000 | `/Game/Characters/Enemies/Regular/Dungeon_RabidRat/QuadRetro/Dungeon_RabidRat_QuadRetro/Textures/Dungeon_RabidRat_QuadRetro_Pixelated_512_Normalized.Dungeon_RabidRat_QuadRetro_Pixelated_512_Normalized` |
| Dungeon_Bat | 0.1005 | 0.2902 | 4.0000 | `/Game/Characters/Enemies/Regular/Dungeon_Bat/QuadRetro/Dungeon_Bat_QuadRetro/Textures/Dungeon_Bat_QuadRetro_Pixelated_512_Normalized.Dungeon_Bat_QuadRetro_Pixelated_512_Normalized` |
| Forest_MushroomBrute | 0.0524 | 0.2079 | 4.0000 | `/Game/Characters/Enemies/Regular/Forest_MushroomBrute/QuadRetro/Forest_MushroomBrute_QuadRetro/Textures/Forest_MushroomBrute_QuadRetro_Pixelated_512_Normalized.Forest_MushroomBrute_QuadRetro_Pixelated_512_Normalized` |
| Forest_TreantSapling | 0.0282 | 0.1092 | 4.0000 | `/Game/Characters/Enemies/Regular/Forest_TreantSapling/QuadRetro/Forest_TreantSapling_QuadRetro/Textures/Forest_TreantSapling_QuadRetro_Pixelated_512_Normalized.Forest_TreantSapling_QuadRetro_Pixelated_512_Normalized` |
| Forest_ThornImp | 0.0517 | 0.1931 | 4.0000 | `/Game/Characters/Enemies/Regular/Forest_ThornImp/QuadRetro/Forest_ThornImp_QuadRetro/Textures/Forest_ThornImp_QuadRetro_Pixelated_512_Normalized.Forest_ThornImp_QuadRetro_Pixelated_512_Normalized` |
| Forest_Boar | 0.0171 | 0.0678 | 4.0000 | `/Game/Characters/Enemies/Regular/Forest_Boar/QuadRetro/Forest_Boar_QuadRetro/Textures/Forest_Boar_QuadRetro_Pixelated_512_Normalized.Forest_Boar_QuadRetro_Pixelated_512_Normalized` |
| Forest_Wasp | 0.1546 | 0.4125 | 3.2346 | `/Game/Characters/Enemies/Regular/Forest_Wasp/QuadRetro/Forest_Wasp_QuadRetro/Textures/Forest_Wasp_QuadRetro_Pixelated_512_Normalized.Forest_Wasp_QuadRetro_Pixelated_512_Normalized` |
| Ocean_CrabGuard | 0.0437 | 0.1625 | 4.0000 | `/Game/Characters/Enemies/Regular/Ocean_CrabGuard/QuadRetro/Ocean_CrabGuard_QuadRetro/Textures/Ocean_CrabGuard_QuadRetro_Pixelated_512_Normalized.Ocean_CrabGuard_QuadRetro_Pixelated_512_Normalized` |
| Ocean_DrownedSailor | 0.0125 | 0.0498 | 4.0000 | `/Game/Characters/Enemies/Regular/Ocean_DrownedSailor/QuadRetro/Ocean_DrownedSailor_QuadRetro/Textures/Ocean_DrownedSailor_QuadRetro_Pixelated_512_Normalized.Ocean_DrownedSailor_QuadRetro_Pixelated_512_Normalized` |
| Ocean_Jellyfish | 0.1108 | 0.3214 | 4.0000 | `/Game/Characters/Enemies/Regular/Ocean_Jellyfish/QuadRetro/Ocean_Jellyfish_QuadRetro/Textures/Ocean_Jellyfish_QuadRetro_Pixelated_512_Normalized.Ocean_Jellyfish_QuadRetro_Pixelated_512_Normalized` |
| Ocean_SharkPup | 0.1021 | 0.3179 | 4.0000 | `/Game/Characters/Enemies/Regular/Ocean_SharkPup/QuadRetro/Ocean_SharkPup_QuadRetro/Textures/Ocean_SharkPup_QuadRetro_Pixelated_512_Normalized.Ocean_SharkPup_QuadRetro_Pixelated_512_Normalized` |
| Ocean_GhostRay | 0.0467 | 0.1847 | 4.0000 | `/Game/Characters/Enemies/Regular/Ocean_GhostRay/QuadRetro/Ocean_GhostRay_QuadRetro/Textures/Ocean_GhostRay_QuadRetro_Pixelated_512_Normalized.Ocean_GhostRay_QuadRetro_Pixelated_512_Normalized` |
| Martian_DroneGrunt | 0.0190 | 0.0761 | 4.0000 | `/Game/Characters/Enemies/Regular/Martian_DroneGrunt/QuadRetro/Martian_DroneGrunt_QuadRetro/Textures/Martian_DroneGrunt_QuadRetro_Pixelated_512_Normalized.Martian_DroneGrunt_QuadRetro_Pixelated_512_Normalized` |
| Martian_CrystalCrawler | 0.0980 | 0.1898 | 4.0000 | `/Game/Characters/Enemies/Regular/Martian_CrystalCrawler/QuadRetro/Martian_CrystalCrawler_QuadRetro/Textures/Martian_CrystalCrawler_QuadRetro_Pixelated_512_Normalized.Martian_CrystalCrawler_QuadRetro_Pixelated_512_Normalized` |
| Martian_PlasmaSpitter | 0.0219 | 0.0866 | 4.0000 | `/Game/Characters/Enemies/Regular/Martian_PlasmaSpitter/QuadRetro/Martian_PlasmaSpitter_QuadRetro/Textures/Martian_PlasmaSpitter_QuadRetro_Pixelated_512_Normalized.Martian_PlasmaSpitter_QuadRetro_Pixelated_512_Normalized` |
| Martian_RocketLeaper | 0.0319 | 0.1276 | 4.0000 | `/Game/Characters/Enemies/Regular/Martian_RocketLeaper/QuadRetro/Martian_RocketLeaper_QuadRetro/Textures/Martian_RocketLeaper_QuadRetro_Pixelated_512_Normalized.Martian_RocketLeaper_QuadRetro_Pixelated_512_Normalized` |
| Martian_SaucerDrone | 0.0730 | 0.2195 | 4.0000 | `/Game/Characters/Enemies/Regular/Martian_SaucerDrone/QuadRetro/Martian_SaucerDrone_QuadRetro/Textures/Martian_SaucerDrone_QuadRetro_Pixelated_512_Normalized.Martian_SaucerDrone_QuadRetro_Pixelated_512_Normalized` |
| Hell_Imp | 0.0165 | 0.0658 | 4.0000 | `/Game/Characters/Enemies/Regular/Hell_Imp/QuadRetro/Hell_Imp_QuadRetro/Textures/Hell_Imp_QuadRetro_Pixelated_512_Normalized.Hell_Imp_QuadRetro_Pixelated_512_Normalized` |
| Hell_BoneKnight | 0.0044 | 0.0176 | 4.0000 | `/Game/Characters/Enemies/Regular/Hell_BoneKnight/QuadRetro/Hell_BoneKnight_QuadRetro/Textures/Hell_BoneKnight_QuadRetro_Pixelated_512_Normalized.Hell_BoneKnight_QuadRetro_Pixelated_512_Normalized` |
| Hell_FireSkull | 0.1275 | 0.4263 | 3.9231 | `/Game/Characters/Enemies/Regular/Hell_FireSkull/QuadRetro/Hell_FireSkull_QuadRetro/Textures/Hell_FireSkull_QuadRetro_Pixelated_512_Normalized.Hell_FireSkull_QuadRetro_Pixelated_512_Normalized` |
| Hellhound | 0.0433 | 0.1337 | 4.0000 | `/Game/Characters/Enemies/Regular/Hellhound/QuadRetro/Hellhound_QuadRetro/Textures/Hellhound_QuadRetro_Pixelated_512_Normalized.Hellhound_QuadRetro_Pixelated_512_Normalized` |
| Hell_Gargoyle | 0.0076 | 0.0303 | 4.0000 | `/Game/Characters/Enemies/Regular/Hell_Gargoyle/QuadRetro/Hell_Gargoyle_QuadRetro/Textures/Hell_Gargoyle_QuadRetro_Pixelated_512_Normalized.Hell_Gargoyle_QuadRetro_Pixelated_512_Normalized` |

Standalone build and smoke status:

- `Scripts/StageStandaloneBuild.ps1` ran without `-SkipCook`; BuildCookRun completed with ExitCode 0.
- `C:\UE\T66\T66 Standalone.lnk` target: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Taskbar `T66 Standalone.lnk` target: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Staged smoke launched `T66.exe` for 35 seconds with log `Saved/StandaloneLogs/Track1_NormalizationSmoke.log`.
- Cooked output contains 75 `_Normalized.uasset` character textures.
- Smoke log normalized-asset error lines: 0.
- Smoke log had unrelated pre-existing warnings for optional profiler DLLs, Steam P2P config, and audio SoundClass preload; no normalized texture load failure was found.

## Task 3 - Source Image Rules Rewrite

Updated [Model Generation/Instructions/02_SOURCE_IMAGE_RULES.md](../../../Model%20Generation/Instructions/02_SOURCE_IMAGE_RULES.md).

Changes made:

- made duo-color discipline the central source approval rule
- added measured source luminance target of `0.55-0.65`, with `<0.45` as a hard reject
- documented Pixal3D/TRELLIS luminance loss and the role of QuadRetro normalization
- tightened composition requirements around front view, clean white background, no lighting, no shadows, and no environment elements
- clarified that style is not locked; cartoon, grounded, angular, and Chad/Stacy directions are all valid if technical bars pass
- updated the approval gate to require visual duo-color approval and a quick luminance sample before retopo, rigging, import, or promotion
- documented image-generator detail leakage and stronger negative prompt mitigation

## Deviations And Issues

- Several existing textures were too dark to reach `0.50` after normalization because `max_scaling_factor=4.0` capped the lift as intended. The synthetic test proves the function reaches target when the cap allows it; the roster distribution records the capped production result.
- Unreal editor automation exited mid-run without a Python traceback during a few texture-processing runs. The worker was hardened to write CSV/JSON progress after every row and to resume from completed PNG/UV work or already-imported `_Normalized` Texture2D assets. Final result completed all 75 rows with zero recorded failures.
- GLTF export remains the default mesh export for UV extraction. `SM_Gambler_QuadRetro` is explicitly routed through FBX first because the earlier GLTF path was unstable for that asset.
- Some source asset names are already truncated in the existing project, so the normalized asset path follows the actual imported Texture2D package path rather than inventing a longer name.

