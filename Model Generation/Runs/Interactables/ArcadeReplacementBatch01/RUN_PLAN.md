# Arcade Replacement Batch01

Completed workspace for the missing or renamed interactable model batch. Source
images were generated with pure white, shadowless backgrounds, Trellis produced
the raw GLBs, Blender converted them to Unreal-ready FBX plus RGB base-color
textures, and Unreal imported the final static meshes/materials/textures.

## Global Source Rule

Every generated source image for this batch must use a pure white, shadowless
background:

- exact intended background: `#ffffff`
- no alpha requirement for the prompt; opaque white is acceptable
- no floor, ground plane, pedestal, platform, reflection, cast shadow, contact
  shadow, gradient, gray patch, border card, UI, text, watermark, or scene
- one centered subject unless the asset description explicitly requires a
  paired subject, such as the Gambler demon with his stand

## Targets

| Target ID | Intent | State |
| --- | --- | --- |
| `GamblerDemonStand` | Replace the current casino/gambler interactable look with a demon gambler guy and a small betting stand in front of him. | Generated and imported to `/Game/Characters/NPCs/Gambler/GamblerDemonStand/GamblerDemonStand` |
| `ArcadeMachine` | Canonical arcade cabinet model for the popup arcade interactable. | Generated and imported to `/Game/World/Interactables/ArcadeMachine/ArcadeMachine` |
| `ArcadeAmplifierPickup` | Main amplifier pickup reward model. | Generated and imported to `/Game/World/Interactables/ArcadeAmplifierPickup/ArcadeAmplifierPickup` |
| `ArcadeAmplifierPickup_Charged` | Brighter charged/readable amplifier pickup variant for reward feedback or VFX handoff. | Generated and imported to `/Game/World/Interactables/ArcadeAmplifierPickup/ArcadeAmplifierPickup_Charged` |
| `Chest` | Canonical chest target at `Content/World/Interactables/Chests/ChestModel/Chest.uasset`. | Generated and imported to `/Game/World/Interactables/Chests/ChestModel/Chest` |
| `Vehicle` | Rename-facing target for the old internal `ArcadeTruck` class; keep the same model rather than regenerating. Current data points the Vehicle display mesh at `/Game/World/Props/Tractor.Tractor`. | Reused existing model; data row renamed to `Vehicle` |

## Verified Outputs

- Source images: `Inputs/source_images`
- Raw Trellis GLBs: `Raw/Trellis`
- Blender preview sheet: `Notes/trellis_preview_contact_sheet.png`
- Unreal-ready FBX and base-color texture manifest:
  `Notes/unreal_ready/ArcadeReplacementBatch01_UnrealReadyManifest.json`
- Unreal import wrapper: `Scripts/ImportArcadeReplacementBatch01AndExit.py`
- Unreal verification report:
  `Saved/Audits/ArcadeReplacementBatch01Verify.json`

## Import Notes

1. Raw GLB import through UE Interchange returned zero imported assets in this
   pass, so the production path for this batch is GLB -> Blender FBX/PNG -> UE.
2. Exported base-color textures are flattened over white before import so
   transparent image padding cannot create black/empty UE textures.
3. The UE verification pass confirmed each generated material is parented to
   `/Game/Materials/M_Environment_Unlit` and has both `DiffuseColorMap` and
   `BaseColorTexture` bound to its generated texture.
