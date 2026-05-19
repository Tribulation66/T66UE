# Pixal3D

Read `PIXAL3D_AGENTS.md` first for routing and safety rules.

Pixal3D is production-cleared for T66 model replacement assets when the
manifest-driven ToonStyle production workflow is used. Do not route playable
assets through the old one-off imports or manual material setup.

Primary production path:

1. Read `../Instructions/09_PIXAL3D_TOONSTYLE_PRODUCTION_IMPORT_INSTRUCTIONS.md`.
2. Fill `production_asset_replacement_manifest.json` and remove the template
   row before a real production run.
3. Run `Scripts/run_pixal3d_toonstyle_production_import.py`.
4. Finish with wrapper `verify` so Tint, close-the-gap B, inner-line textures,
   ToonStyle materials, Unreal bindings, and the production report are checked.

The pipeline reference is `PIXAL3D_PIPELINE_REFERENCE.md`.
