# Quad Retro Medium Summary - Chad/Stacy Source Image Batch 01

Generated: `2026-05-06T22:59:33-03:00`

Status: incomplete. Do not treat this batch as done.

## Counts

- Completed: `6 / 24`
- Failed: `18 / 24`
- Missing/unattempted: `0 / 24`

Execution used the color-preserving Medium values: `target_quads=12000`,
`adaptive_size=50`, `texture_size=512`, `bake_size=1024`,
`palette_mode=none`, `dither_type=none`, and `dither_strength=0`.

Inline `RenderQA` was not used for the completed remesh passes after the first
BoxerChad run proved that inline QA rendering could hang after model export and
before the report was written. Front QA renders were generated afterward with
the runbook-approved background QA helper.

Contact sheet: [completed front contact sheet](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/Renders/ChadStacy_Medium_completed_front_contact_sheet.png)

## Completed Models

| Label | Output GLB | Report | Front QA | Quads | Tris | Quad Remesher seconds |
| --- | --- | --- | --- | ---: | ---: | ---: |
| BoxerChad | [GLB](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/BoxerChad/Models/BoxerChad_QuadRetro.glb) | [report](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/BoxerChad/Reports/BoxerChad_QuadRetro_report.json) | [front](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/BoxerChad/Renders/BoxerChad_front.png) | 13754 | 27669 | 10.043 |
| RoyalChad | [GLB](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/RoyalChad/Models/RoyalChad_QuadRetro.glb) | [report](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/RoyalChad/Reports/RoyalChad_QuadRetro_report.json) | [front](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/RoyalChad/Renders/RoyalChad_front.png) | 14499 | 29115 | 234.384 |
| BillyChad | [GLB](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/BillyChad/Models/BillyChad_QuadRetro.glb) | [report](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/BillyChad/Reports/BillyChad_QuadRetro_report.json) | [front](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/BillyChad/Renders/BillyChad_front.png) | 19814 | 40286 | 71.131 |
| RabbitChad | [GLB](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/RabbitChad/Models/RabbitChad_QuadRetro.glb) | [report](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/RabbitChad/Reports/RabbitChad_QuadRetro_report.json) | [front](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/RabbitChad/Renders/RabbitChad_front.png) | 14147 | 28380 | 25.071 |
| BoxerStacy | [GLB](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/BoxerStacy/Models/BoxerStacy_QuadRetro.glb) | [report](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/BoxerStacy/Reports/BoxerStacy_QuadRetro_report.json) | [front](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/BoxerStacy/Renders/BoxerStacy_front.png) | 16557 | 33499 | 20.548 |
| BaldStacy | [GLB](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/BaldStacy/Models/BaldStacy_QuadRetro.glb) | [report](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/BaldStacy/Reports/BaldStacy_QuadRetro_report.json) | [front](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/BaldStacy/Renders/BaldStacy_front.png) | 16721 | 33776 | 58.111 |

## Failed Models

All failed models were attempted through the same foreground Blender wrapper,
one at a time. The failure pattern was an early Quad Remesher stall around
progress `0.19-0.22`, followed by the bounded timeout.

| Label | Failure report | Exact failure |
| --- | --- | --- |
| ChineseChad | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/ChineseChad/Reports/ChineseChad_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 45s`; earlier 240s debug attempts also stalled; progress `0.1917 0.072`. |
| FoundingChad | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/FoundingChad/Reports/FoundingChad_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 75s`; progress `0.2002 0.081`. |
| RoboChad | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/RoboChad/Reports/RoboChad_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 75s`; progress `0.1917 0.063`. |
| CSChad | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/CSChad/Reports/CSChad_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 75s`; progress `0.2006 0.061`. |
| GambaChad | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/GambaChad/Reports/GambaChad_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 75s`; progress `0.2054 0.060`. |
| MonotoneChad | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/MonotoneChad/Reports/MonotoneChad_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 75s`; progress `0.2008 0.063`. |
| BaldChad | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/BaldChad/Reports/BaldChad_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 75s`; progress `0.1917 0.068`. |
| RoachChad | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/RoachChad/Reports/RoachChad_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 75s`; progress `0.2015 0.069`. |
| RoyalStacy | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/RoyalStacy/Reports/RoyalStacy_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 75s`; progress `0.2176 0.065`. |
| ChineseStacy | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/ChineseStacy/Reports/ChineseStacy_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 75s`; progress `0.2010 0.066`. |
| FoundingStacy | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/FoundingStacy/Reports/FoundingStacy_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 75s`; progress `0.2020 0.062`. |
| RoboStacy | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/RoboStacy/Reports/RoboStacy_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 75s`; progress `0.2065 0.066`. |
| BillyStacy | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/BillyStacy/Reports/BillyStacy_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 75s`; progress `0.2002 0.066`. |
| RabbitStacy | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/RabbitStacy/Reports/RabbitStacy_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 75s`; progress `0.2009 0.060`. |
| CSStacy | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/CSStacy/Reports/CSStacy_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 75s`; progress `0.1996 0.069`. |
| GambaStacy | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/GambaStacy/Reports/GambaStacy_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 75s`; progress `0.2009 0.059`. |
| MonotoneStacy | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/MonotoneStacy/Reports/MonotoneStacy_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 75s`; progress `0.1993 0.075`. |
| RoachStacy | [failure](C:/UE/T66/Model%20Generation/Runs/Heroes/ChadStacySourceImageBatch01/QuadRetroPipeline/Medium/RoachStacy/Reports/RoachStacy_QuadRetro_failure.json) | `RuntimeError: Quad Remesher timed out after 75s`; progress `0.1990 0.072`. |

## Process Notes

- Preflight confirmed no stale `blender.exe` or `xremesh.exe` processes before the main run.
- Existing `_Control`, `_AdaptTest`, and `_LaunchTest` folders were treated as test output and were not counted as completed labels.
- The wrapper was updated to expose the pipeline script's existing timeout option, and the Blender Python script now exits with code `1` on exceptions so failed foreground runs do not leave stale Blender windows.
- Final post-run process check confirmed no `blender.exe` or `xremesh.exe` processes were running.
