# Chad/Stacy Quad Retro Medium Summary - 2026-05-07 01:25

Output root: `C:\UE\T66\Model Generation\Runs\Heroes\ChadStacySourceImageBatch01\QuadRetroPipeline\Medium`

## Result

- Completed heroes: 24 / 24
- Failed heroes: 0 / 24
- QA contact sheet: [ChadStacy_Medium_all24_front_contact_sheet.png](../QuadRetroPipeline/Medium/Renders/ChadStacy_Medium_all24_front_contact_sheet.png)

All 24 canonical hero folders have `Models\<Label>_QuadRetro.glb`, a report JSON, and a front QA render. The reports confirm the expected color-preserving Medium settings: `texture_size=512`, `bake_size=1024`, `palette_mode=none`, `dither_type=none`, and `dither_strength=0`.

## Run Notes

- RoyalChad's initial baked result was black. It was fixed with the color-preserving bake path and promoted into the canonical `RoyalChad` folder.
- Several raw sources stalled Quad Remesher when fed at full TRELLIS density. Those heroes were remeshed from a temporary decimated source while preserving the original source mesh/materials for texture bake.
- GambaStacy's 10k temporary-source run completed but visually collapsed. It was retried at `qremesh_source_target_tris=20000`, produced `15331` quads / `31049` triangles, and the fixed output was promoted into the canonical `GambaStacy` folder.
- `_Inspect` contains the archived failed GambaStacy 10k report/render. Retry/test folders under the Medium root are not counted as completed heroes.

## Outputs

| Label | GLB | Report | Front QA | Quads | Tris | QRemesh source |
| --- | --- | --- | --- | ---: | ---: | --- |
| BoxerChad | [GLB](../QuadRetroPipeline/Medium/BoxerChad/Models/BoxerChad_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/BoxerChad/Reports/BoxerChad_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/BoxerChad/Renders/BoxerChad_front.png) | 13754 | 27669 | native |
| RoyalChad | [GLB](../QuadRetroPipeline/Medium/RoyalChad/Models/RoyalChad_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/RoyalChad/Reports/RoyalChad_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/RoyalChad/Renders/RoyalChad_front.png) | 14499 | 29115 | native |
| ChineseChad | [GLB](../QuadRetroPipeline/Medium/ChineseChad/Models/ChineseChad_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/ChineseChad/Reports/ChineseChad_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/ChineseChad/Renders/ChineseChad_front.png) | 14749 | 29931 | 10000 |
| FoundingChad | [GLB](../QuadRetroPipeline/Medium/FoundingChad/Models/FoundingChad_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/FoundingChad/Reports/FoundingChad_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/FoundingChad/Renders/FoundingChad_front.png) | 14624 | 30145 | 10000 |
| RoboChad | [GLB](../QuadRetroPipeline/Medium/RoboChad/Models/RoboChad_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/RoboChad/Reports/RoboChad_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/RoboChad/Renders/RoboChad_front.png) | 16104 | 32810 | 10000 |
| BillyChad | [GLB](../QuadRetroPipeline/Medium/BillyChad/Models/BillyChad_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/BillyChad/Reports/BillyChad_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/BillyChad/Renders/BillyChad_front.png) | 19814 | 40286 | native |
| RabbitChad | [GLB](../QuadRetroPipeline/Medium/RabbitChad/Models/RabbitChad_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/RabbitChad/Reports/RabbitChad_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/RabbitChad/Renders/RabbitChad_front.png) | 14147 | 28380 | native |
| CSChad | [GLB](../QuadRetroPipeline/Medium/CSChad/Models/CSChad_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/CSChad/Reports/CSChad_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/CSChad/Renders/CSChad_front.png) | 15013 | 30700 | 10000 |
| GambaChad | [GLB](../QuadRetroPipeline/Medium/GambaChad/Models/GambaChad_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/GambaChad/Reports/GambaChad_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/GambaChad/Renders/GambaChad_front.png) | 14332 | 28989 | 10000 |
| MonotoneChad | [GLB](../QuadRetroPipeline/Medium/MonotoneChad/Models/MonotoneChad_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/MonotoneChad/Reports/MonotoneChad_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/MonotoneChad/Renders/MonotoneChad_front.png) | 14316 | 29234 | 10000 |
| BaldChad | [GLB](../QuadRetroPipeline/Medium/BaldChad/Models/BaldChad_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/BaldChad/Reports/BaldChad_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/BaldChad/Renders/BaldChad_front.png) | 14772 | 30206 | 10000 |
| RoachChad | [GLB](../QuadRetroPipeline/Medium/RoachChad/Models/RoachChad_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/RoachChad/Reports/RoachChad_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/RoachChad/Renders/RoachChad_front.png) | 14794 | 30121 | 10000 |
| BoxerStacy | [GLB](../QuadRetroPipeline/Medium/BoxerStacy/Models/BoxerStacy_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/BoxerStacy/Reports/BoxerStacy_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/BoxerStacy/Renders/BoxerStacy_front.png) | 16557 | 33499 | native |
| RoyalStacy | [GLB](../QuadRetroPipeline/Medium/RoyalStacy/Models/RoyalStacy_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/RoyalStacy/Reports/RoyalStacy_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/RoyalStacy/Renders/RoyalStacy_front.png) | 14073 | 28315 | 10000 |
| ChineseStacy | [GLB](../QuadRetroPipeline/Medium/ChineseStacy/Models/ChineseStacy_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/ChineseStacy/Reports/ChineseStacy_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/ChineseStacy/Renders/ChineseStacy_front.png) | 14703 | 30063 | 10000 |
| FoundingStacy | [GLB](../QuadRetroPipeline/Medium/FoundingStacy/Models/FoundingStacy_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/FoundingStacy/Reports/FoundingStacy_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/FoundingStacy/Renders/FoundingStacy_front.png) | 14793 | 30002 | 10000 |
| RoboStacy | [GLB](../QuadRetroPipeline/Medium/RoboStacy/Models/RoboStacy_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/RoboStacy/Reports/RoboStacy_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/RoboStacy/Renders/RoboStacy_front.png) | 13034 | 26266 | 10000 |
| BillyStacy | [GLB](../QuadRetroPipeline/Medium/BillyStacy/Models/BillyStacy_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/BillyStacy/Reports/BillyStacy_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/BillyStacy/Renders/BillyStacy_front.png) | 15122 | 30903 | 10000 |
| RabbitStacy | [GLB](../QuadRetroPipeline/Medium/RabbitStacy/Models/RabbitStacy_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/RabbitStacy/Reports/RabbitStacy_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/RabbitStacy/Renders/RabbitStacy_front.png) | 11379 | 23948 | 10000 |
| CSStacy | [GLB](../QuadRetroPipeline/Medium/CSStacy/Models/CSStacy_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/CSStacy/Reports/CSStacy_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/CSStacy/Renders/CSStacy_front.png) | 13993 | 29393 | 10000 |
| GambaStacy | [GLB](../QuadRetroPipeline/Medium/GambaStacy/Models/GambaStacy_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/GambaStacy/Reports/GambaStacy_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/GambaStacy/Renders/GambaStacy_front.png) | 15331 | 31049 | 20000 |
| MonotoneStacy | [GLB](../QuadRetroPipeline/Medium/MonotoneStacy/Models/MonotoneStacy_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/MonotoneStacy/Reports/MonotoneStacy_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/MonotoneStacy/Renders/MonotoneStacy_front.png) | 15808 | 33134 | 10000 |
| BaldStacy | [GLB](../QuadRetroPipeline/Medium/BaldStacy/Models/BaldStacy_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/BaldStacy/Reports/BaldStacy_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/BaldStacy/Renders/BaldStacy_front.png) | 16721 | 33776 | native |
| RoachStacy | [GLB](../QuadRetroPipeline/Medium/RoachStacy/Models/RoachStacy_QuadRetro.glb) | [Report](../QuadRetroPipeline/Medium/RoachStacy/Reports/RoachStacy_QuadRetro_report.json) | [QA](../QuadRetroPipeline/Medium/RoachStacy/Renders/RoachStacy_front.png) | 17683 | 37170 | 10000 |

