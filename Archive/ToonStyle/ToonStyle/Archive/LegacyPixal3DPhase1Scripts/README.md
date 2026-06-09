# Legacy Pixal3D Phase 1 Scripts

This folder archives early Phase 1/Phase 1B Pixal3D import helpers.

These scripts are historical only:

- `ImportPixal3DAsset.py`
- `ImportLuBuMatrixStaticMeshesAndExit.py`
- `ImportLuBuMatrixTexturesAndBindMaterials.py`
- `RunPixal3DToUE.ps1`

Do not use these scripts for assets that enter playable T66 content. They predate
the production ToonStyle stack and do not enforce the current Pixal3D production
contract:

- `X-Decimation=200000`
- manifest-driven replacement rows
- `ImportPixal3DAsset_Phase1C.py`
- ToonStyle foundation Blender processing
- Tint texture generation and binding
- close-the-gap B-channel authoring and validation
- inner-line texture generation and binding
- hard wrapper verification/reporting

Production Pixal3D replacement work must start from:

`Model Generation/Instructions/09_PIXAL3D_TOONSTYLE_PRODUCTION_IMPORT_INSTRUCTIONS.md`

Canonical entrypoint:

```powershell
python "Model Generation/Pixal3D/Scripts/run_pixal3d_toonstyle_production_import.py" --allow-template validate
```

For a real production run, replace or remove the checked-in template manifest
row and run without `--allow-template`.
