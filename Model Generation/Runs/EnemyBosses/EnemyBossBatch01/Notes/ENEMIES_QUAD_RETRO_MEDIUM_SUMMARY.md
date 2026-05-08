# EnemyBossBatch01 Regular Enemies Quad Retro Medium Summary

Updated UTC: 2026-05-07T16:00:40+00:00

Scope: 25 regular enemies from `Content/Data/Enemies.csv`. Bosses are handled separately.

## Counts

- Completed: 25/25
- Failed: 0
- Settings verified: `target_quads=12000`, `texture_size=512`, `bake_size=1024`, `palette_mode=none`, `dither_type=none`, `dither_strength=0`, `qremesh_report.last_progress=2`.

## QA

- Raw Trellis front contact sheet: [Enemy_TrellisFront_ContactSheet.png](../QA/Enemy_TrellisFront_ContactSheet.png)
- Quad Retro Medium front contact sheet: [Enemy_QuadRetro_Medium_Front_ContactSheet.png](../QA/Enemy_QuadRetro_Medium_Front_ContactSheet.png)
- Trellis manifest: [Stage01_Enemies_TrellisManifest.json](../Reports/Stage01_Enemies_TrellisManifest.json)
- Quad Retro run log: [Stage02_Enemies_QuadRetro_RunLog.jsonl](../Reports/Stage02_Enemies_QuadRetro_RunLog.jsonl)

## Reruns / Fixes

- Regenerated source/Trellis inputs for `Dungeon_Slime`, `Forest_TreantSapling`, `Ocean_Jellyfish`, `Hell_BoneKnight`, `Hell_FireSkull`, and `Hell_Gargoyle` to remove card/background geometry or wrong reads.
- Regenerated `Ocean_DrownedSailor` after Blender crashed on the first raw GLB during material handling; the simpler replacement completed.
- Regenerated `Martian_RocketLeaper` after a detached side object caused a 15-minute Quad Remesher timeout; the replacement completed in under a minute.
- Regenerated `Martian_SaucerDrone` after the first Quad Retro output passed report checks but rendered as a shredded thin disc; the thicker saucer source completed cleanly.

## Outputs

| EnemyID | GLB | Report | Front QA | Quads | Tris |
|---|---|---|---|---:|---:|
| Dungeon_Slime | [GLB](../QuadRetro/Medium/Enemies/Dungeon_Slime/Models/Dungeon_Slime_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Dungeon_Slime/Reports/Dungeon_Slime_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Dungeon_Slime/Renders/Dungeon_Slime_QuadRetro_front.png) | 16605 | 34097 |
| Dungeon_Skeleton | [GLB](../QuadRetro/Medium/Enemies/Dungeon_Skeleton/Models/Dungeon_Skeleton_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Dungeon_Skeleton/Reports/Dungeon_Skeleton_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Dungeon_Skeleton/Renders/Dungeon_Skeleton_QuadRetro_front.png) | 19231 | 40304 |
| Dungeon_WebSpider | [GLB](../QuadRetro/Medium/Enemies/Dungeon_WebSpider/Models/Dungeon_WebSpider_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Dungeon_WebSpider/Reports/Dungeon_WebSpider_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Dungeon_WebSpider/Renders/Dungeon_WebSpider_QuadRetro_front.png) | 16411 | 33673 |
| Dungeon_RabidRat | [GLB](../QuadRetro/Medium/Enemies/Dungeon_RabidRat/Models/Dungeon_RabidRat_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Dungeon_RabidRat/Reports/Dungeon_RabidRat_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Dungeon_RabidRat/Renders/Dungeon_RabidRat_QuadRetro_front.png) | 15488 | 32777 |
| Dungeon_Bat | [GLB](../QuadRetro/Medium/Enemies/Dungeon_Bat/Models/Dungeon_Bat_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Dungeon_Bat/Reports/Dungeon_Bat_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Dungeon_Bat/Renders/Dungeon_Bat_QuadRetro_front.png) | 15838 | 32231 |
| Forest_MushroomBrute | [GLB](../QuadRetro/Medium/Enemies/Forest_MushroomBrute/Models/Forest_MushroomBrute_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Forest_MushroomBrute/Reports/Forest_MushroomBrute_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Forest_MushroomBrute/Renders/Forest_MushroomBrute_QuadRetro_front.png) | 23520 | 49457 |
| Forest_TreantSapling | [GLB](../QuadRetro/Medium/Enemies/Forest_TreantSapling/Models/Forest_TreantSapling_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Forest_TreantSapling/Reports/Forest_TreantSapling_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Forest_TreantSapling/Renders/Forest_TreantSapling_QuadRetro_front.png) | 25550 | 53914 |
| Forest_ThornImp | [GLB](../QuadRetro/Medium/Enemies/Forest_ThornImp/Models/Forest_ThornImp_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Forest_ThornImp/Reports/Forest_ThornImp_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Forest_ThornImp/Renders/Forest_ThornImp_QuadRetro_front.png) | 21069 | 44257 |
| Forest_Boar | [GLB](../QuadRetro/Medium/Enemies/Forest_Boar/Models/Forest_Boar_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Forest_Boar/Reports/Forest_Boar_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Forest_Boar/Renders/Forest_Boar_QuadRetro_front.png) | 17163 | 36323 |
| Forest_Wasp | [GLB](../QuadRetro/Medium/Enemies/Forest_Wasp/Models/Forest_Wasp_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Forest_Wasp/Reports/Forest_Wasp_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Forest_Wasp/Renders/Forest_Wasp_QuadRetro_front.png) | 14273 | 29861 |
| Ocean_CrabGuard | [GLB](../QuadRetro/Medium/Enemies/Ocean_CrabGuard/Models/Ocean_CrabGuard_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Ocean_CrabGuard/Reports/Ocean_CrabGuard_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Ocean_CrabGuard/Renders/Ocean_CrabGuard_QuadRetro_front.png) | 14858 | 29981 |
| Ocean_DrownedSailor | [GLB](../QuadRetro/Medium/Enemies/Ocean_DrownedSailor/Models/Ocean_DrownedSailor_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Ocean_DrownedSailor/Reports/Ocean_DrownedSailor_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Ocean_DrownedSailor/Renders/Ocean_DrownedSailor_QuadRetro_front.png) | 26608 | 55632 |
| Ocean_Jellyfish | [GLB](../QuadRetro/Medium/Enemies/Ocean_Jellyfish/Models/Ocean_Jellyfish_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Ocean_Jellyfish/Reports/Ocean_Jellyfish_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Ocean_Jellyfish/Renders/Ocean_Jellyfish_QuadRetro_front.png) | 5915 | 12123 |
| Ocean_SharkPup | [GLB](../QuadRetro/Medium/Enemies/Ocean_SharkPup/Models/Ocean_SharkPup_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Ocean_SharkPup/Reports/Ocean_SharkPup_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Ocean_SharkPup/Renders/Ocean_SharkPup_QuadRetro_front.png) | 12444 | 24976 |
| Ocean_GhostRay | [GLB](../QuadRetro/Medium/Enemies/Ocean_GhostRay/Models/Ocean_GhostRay_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Ocean_GhostRay/Reports/Ocean_GhostRay_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Ocean_GhostRay/Renders/Ocean_GhostRay_QuadRetro_front.png) | 12866 | 26100 |
| Martian_DroneGrunt | [GLB](../QuadRetro/Medium/Enemies/Martian_DroneGrunt/Models/Martian_DroneGrunt_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Martian_DroneGrunt/Reports/Martian_DroneGrunt_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Martian_DroneGrunt/Renders/Martian_DroneGrunt_QuadRetro_front.png) | 23504 | 49308 |
| Martian_CrystalCrawler | [GLB](../QuadRetro/Medium/Enemies/Martian_CrystalCrawler/Models/Martian_CrystalCrawler_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Martian_CrystalCrawler/Reports/Martian_CrystalCrawler_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Martian_CrystalCrawler/Renders/Martian_CrystalCrawler_QuadRetro_front.png) | 12603 | 25739 |
| Martian_PlasmaSpitter | [GLB](../QuadRetro/Medium/Enemies/Martian_PlasmaSpitter/Models/Martian_PlasmaSpitter_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Martian_PlasmaSpitter/Reports/Martian_PlasmaSpitter_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Martian_PlasmaSpitter/Renders/Martian_PlasmaSpitter_QuadRetro_front.png) | 13215 | 27550 |
| Martian_RocketLeaper | [GLB](../QuadRetro/Medium/Enemies/Martian_RocketLeaper/Models/Martian_RocketLeaper_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Martian_RocketLeaper/Reports/Martian_RocketLeaper_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Martian_RocketLeaper/Renders/Martian_RocketLeaper_QuadRetro_front.png) | 17509 | 37262 |
| Martian_SaucerDrone | [GLB](../QuadRetro/Medium/Enemies/Martian_SaucerDrone/Models/Martian_SaucerDrone_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Martian_SaucerDrone/Reports/Martian_SaucerDrone_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Martian_SaucerDrone/Renders/Martian_SaucerDrone_QuadRetro_front.png) | 16531 | 34813 |
| Hell_Imp | [GLB](../QuadRetro/Medium/Enemies/Hell_Imp/Models/Hell_Imp_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Hell_Imp/Reports/Hell_Imp_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Hell_Imp/Renders/Hell_Imp_QuadRetro_front.png) | 13178 | 27896 |
| Hell_BoneKnight | [GLB](../QuadRetro/Medium/Enemies/Hell_BoneKnight/Models/Hell_BoneKnight_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Hell_BoneKnight/Reports/Hell_BoneKnight_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Hell_BoneKnight/Renders/Hell_BoneKnight_QuadRetro_front.png) | 16184 | 34375 |
| Hell_FireSkull | [GLB](../QuadRetro/Medium/Enemies/Hell_FireSkull/Models/Hell_FireSkull_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Hell_FireSkull/Reports/Hell_FireSkull_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Hell_FireSkull/Renders/Hell_FireSkull_QuadRetro_front.png) | 19841 | 41509 |
| Hellhound | [GLB](../QuadRetro/Medium/Enemies/Hellhound/Models/Hellhound_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Hellhound/Reports/Hellhound_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Hellhound/Renders/Hellhound_QuadRetro_front.png) | 14497 | 30711 |
| Hell_Gargoyle | [GLB](../QuadRetro/Medium/Enemies/Hell_Gargoyle/Models/Hell_Gargoyle_QuadRetro.glb) | [Report](../QuadRetro/Medium/Enemies/Hell_Gargoyle/Reports/Hell_Gargoyle_QuadRetro_report.json) | [QA](../QuadRetro/Medium/Enemies/Hell_Gargoyle/Renders/Hell_Gargoyle_QuadRetro_front.png) | 16503 | 34948 |
