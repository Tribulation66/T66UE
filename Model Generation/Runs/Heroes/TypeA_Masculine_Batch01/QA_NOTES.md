# TypeA Masculine Batch01 QA Notes

## 2026-05-02 Completion Check

- Manifest entries: `57`
- Local raw TRELLIS GLBs: `57`
- Manifest status: `57 model_ready`
- Blender lineup scenes/renders: `12`
- Contact sheet: [TypeA_Batch01_AllLineups_ContactSheet.png](C:/UE/T66/Model%20Generation/Scenes/TypeA_Batch01_AllLineups_ContactSheet.png)

## Important Quality Finding

The batch is generation-complete, but not art-approved.

Several fallback-derived seed images produced TRELLIS outputs with flat source-image panels, background cards, or poster-like geometry. This is especially visible in the fallback-generated heroes where the input image was not a clean isolated pure-green production reference.

Use this batch state as proof that the pipeline can generate all requested parts, but reroll or manually clean any fallback-derived asset that fails Blender lineup QA before production import.

## 2026-05-04 Source-Image Process Audit

The next Chad process pass is documented in
[Chad_Pass02_ProcessBuild](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild).

The source-image audit gate is:
[audit_chad_source_images.py](C:/UE/T66/Model%20Generation/Scripts/audit_chad_source_images.py)

Batch01 diagnostic artifacts:

- [source diagnostic contact sheet](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/Review/ProcessBuild_20260504/approved_seed_diagnostic_contact_sheet.png)
- [source audit JSON](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/Review/ProcessBuild_20260504/batch01_source_audit_green.json)

The calibrated legacy-green audit found `28` failed sources out of `57`. The
most important body/head failures were:

- square or fallback body canvases that should not be accepted for Pass02 bodies
- Shroud, xQc, Moist, North, Asmon, Robo beach, Billy, and Rabbit body sources that need reroll or manual cleanup before reuse
- xQc, Moist, North, Asmon, Billy, Rabbit, and some older head sources with non-flat borders, internal green, or portrait-panel contamination
- North standard head is not a green-background head-only source at all
- Asmon and Billy heads show internal green contamination before TRELLIS

This confirms the main failure split: source PNG approval is the primary
problem; TRELLIS amplifies those bad inputs; assembly constants remain usable on
clean parts. `model_ready` continues to mean only that a GLB exists.

## Blender Lineups

- [Arthur beach pair](C:/UE/T66/Model%20Generation/Scenes/TypeA_Batch01_Arthur_BeachPair_Lineup.png)
- [Lu Bu five-piece](C:/UE/T66/Model%20Generation/Scenes/TypeA_Batch01_LuBu_FivePiece_Lineup.png)
- [Mike five-piece](C:/UE/T66/Model%20Generation/Scenes/TypeA_Batch01_Mike_FivePiece_Lineup.png)
- [George five-piece](C:/UE/T66/Model%20Generation/Scenes/TypeA_Batch01_George_FivePiece_Lineup.png)
- [Robo five-piece](C:/UE/T66/Model%20Generation/Scenes/TypeA_Batch01_Robo_FivePiece_Lineup.png)
- [Billy five-piece](C:/UE/T66/Model%20Generation/Scenes/TypeA_Batch01_Billy_FivePiece_Lineup.png)
- [Rabbit five-piece](C:/UE/T66/Model%20Generation/Scenes/TypeA_Batch01_Rabbit_FivePiece_Lineup.png)
- [Shroud five-piece](C:/UE/T66/Model%20Generation/Scenes/TypeA_Batch01_Shroud_FivePiece_Lineup.png)
- [xQc five-piece](C:/UE/T66/Model%20Generation/Scenes/TypeA_Batch01_xQc_FivePiece_Lineup.png)
- [Moist five-piece](C:/UE/T66/Model%20Generation/Scenes/TypeA_Batch01_Moist_FivePiece_Lineup.png)
- [North five-piece](C:/UE/T66/Model%20Generation/Scenes/TypeA_Batch01_North_FivePiece_Lineup.png)
- [Asmon five-piece](C:/UE/T66/Model%20Generation/Scenes/TypeA_Batch01_Asmon_FivePiece_Lineup.png)

## Head / Body Assembly

Head-to-body assembly was tested and batch-run on 2026-05-02.

- Assembly script: [assemble_typea_head_body.py](C:/UE/T66/Model%20Generation/Scripts/assemble_typea_head_body.py)
- Assembled GLBs: [Assembly/HeadBody](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/Assembly/HeadBody)
- Assembly report: [head_body_assembly_report.json](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/Assembly/head_body_assembly_report.json)
- Multi-angle renders: [Renders/HeadBodyAssembly](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/Renders/HeadBodyAssembly)
- Blender scenes: [TypeA_Batch01_HeadBodyAssembly](C:/UE/T66/Model%20Generation/Scenes/TypeA_Batch01_HeadBodyAssembly)

Validated constants after MCP close-up review:

- Head height ratio: `0.245` of body height
- Neck overlap: `0.045` of body height
- Forward/back offset: `-0.006` of body height toward the front

The connection is visually sound on the clean standard test set: Lu Bu, Mike, George, and Robo. These were reviewed with MCP-driven front, right-side, and oblique close-up renders:

- [MCP neck checks contact sheet](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/Renders/HeadBodyAssembly/MCP_NeckChecks/TypeA_HeadBodyAssembly_MCP_NeckChecks_ContactSheet.png)

Batch output count:

- `24` assembled GLBs
- `96` multi-angle QA renders
- `24` saved Blender scenes

Important distinction: the assembly transform can place the head correctly even when the source body or head is bad. It does not fix fallback-derived panels, floor cards, duplicated bodies, or poster geometry. Treat the following contact sheets as the first review gate:

- [Standard assembled contact sheet](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/Renders/HeadBodyAssembly/TypeA_HeadBodyAssembly_Standard_AllHeroes_ContactSheet.png)
- [Beach assembled contact sheet](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/Renders/HeadBodyAssembly/TypeA_HeadBodyAssembly_BeachGoer_AllHeroes_ContactSheet.png)

Clear source reroll candidates before production approval:

- Standard: Billy, Shroud, North, and Asmon have visible panel/floor-card contamination.
- Beach: Robo, Billy, Rabbit, Shroud, Moist, and Asmon have visible source-body or panel contamination.
- Other fallback-derived assemblies may still need user art review even when the head placement itself is acceptable.

## Rigging Prototype

The first automated rigging prototype was completed on 2026-05-02 using Mike standard and tightened on 2026-05-03 for saved actions, lower-body walk proof, and fragmented-mesh stabilization.

- Rig script: [rig_typea_mike_prototype.py](C:/UE/T66/Model%20Generation/Scripts/rig_typea_mike_prototype.py)
- Rigged Blender scene: [Hero_4_Mike_TypeA_Standard_Rigged.blend](C:/UE/T66/Model%20Generation/Scenes/TypeA_Batch01_Rigging/Hero_4_Mike_TypeA_Standard_Rigged.blend)
- Exports: [Rigging/Hero_4_Mike/Exports](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/Rigging/Hero_4_Mike/Exports)
- Rig report: [rig_report.json](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/Rigging/Hero_4_Mike/rig_report.json)
- MCP rig-check contact sheet: [Hero_4_Mike_MCP_RigChecks_ContactSheet.png](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/Rigging/Hero_4_Mike/Renders/MCP_RigChecks/Hero_4_Mike_MCP_RigChecks_ContactSheet.png)
- Final MCP rest/action/walk sheet: [Hero_4_Mike_MCP_Final_RigWalk_ContactSheet.png](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/Rigging/Hero_4_Mike/Renders/MCP_RigChecks/Final/Hero_4_Mike_MCP_Final_RigWalk_ContactSheet.png)

Verified state:

- One generated armature
- `18` bones
- `Mike_Body` and `Mike_Head` both use armature modifiers
- Two saved actions: `Mike_RigPrototype_IdleAction` and `Mike_RigPrototype_WalkInPlace`
- Exported FBX reimports with one armature, `18` bones, two skinned meshes, and both actions.
- Exported GLB JSON contains only `Body_geometry_0` and `Head_geometry_0` mesh nodes with both actions. Blender's GLB importer displays one extra unweighted `Icosphere` helper on reimport, so use raw GLB inspection rather than Blender object count alone when auditing GLB contents.

Important implementation finding: assembled GLBs import with mesh placement stored on parent empties. For rigging, bake the mesh world transform directly into mesh data, detach the mesh from its placement empty, remove the empty, and only then create weights/armature modifiers. Blender `transform_apply` was not reliable enough for this in background mode.

Second implementation finding: TRELLIS Type A body meshes can contain thousands of disconnected fragments. Mike standard has more than `5k` disconnected body components, so loose details can tear if every vertex is smoothed independently. The current script stabilizes compact disconnected fragments as rigid pieces while leaving larger limb panels weighted procedurally; this is required before walk-cycle QA.

Third implementation finding: generated actions must set `use_fake_user = True` or Blender can drop non-active actions when saving the `.blend`. The exports can still contain the action, but the saved scene will not.

Current rigging note: Mike standard is acceptable as the first automated Type A proof for rest, upper-body action, and an in-place walk cycle. Keep the MCP multi-angle screenshot loop for every new hero because clothing panels, straps, cuffs, and beach props may need per-character stabilization thresholds.
