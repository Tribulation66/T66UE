# Unreal Import And Validation

Generated assets are not runtime assets until they pass the Unreal import path and validation for their domain.

## Import Script Pattern

- Static meshes: `Scripts/ImportStaticMeshes.py` plus the relevant full-editor wrapper.
- Skeletal meshes: `Scripts/ImportSkeletalMeshes.py` plus the relevant full-editor wrapper.
- Data reloads: the matching `Scripts/Setup*DataTable.py` script.
- Material pass: `Scripts/MakeGLBImportsUnlit.py`, `Scripts/MakeCharacterMaterialsUnlit.py`, or domain-specific repair scripts when needed.
- Verification: `Scripts/VerifyImportBatch.py` or the domain-specific verifier.

## Standalone Rule

When a change affects the playable standalone build, refresh the staged standalone build and verify the `T66 Standalone.lnk` shortcut points to:

`C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`

Do not use `-SkipCook` for a new imported asset. A source-only or docs-only cleanup does not require staging.

## Generated Output Retention

Keep lightweight manifests and status summaries only while they are needed to drive import. Delete raw TRELLIS, Blender, scene, render, log, and archive output after the imported assets have been verified or rejected.
