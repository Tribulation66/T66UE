# Pixal3D Non-Enemy Model Source Images

This folder is split for two parallel model-generation agents.

Use the production Pixal3D ToonStyle workflow before importing anything into playable content:

`C:\UE\T66\Model Generation\Instructions\09_PIXAL3D_TOONSTYLE_PRODUCTION_IMPORT_INSTRUCTIONS.md`

Hard boundaries:
- These are accepted source PNGs only. Do not add failed iterations, contact sheets, enemies, or the skipped loot crate.
- Do not manually assign ToonStyle materials. Use the manifest-driven production path so Tint, outline, close-the-gap B, inner-line texture, import settings, and validation JSON are produced.
- Use `X-Decimation=200000` with remesh enabled unless the server reports a fallback, and surface any fallback in the import report.
- Refresh the staged standalone build only after generated assets are imported into playable content.

## Agent Folders

| Folder | Owner | PNG count | Scope |
| --- | --- | ---: | --- |
| `Agent_1_StaticWorldAssets` | Agent 1 | 24 | Interactables, boost pickup coins, gates, and easy visual props |
| `Agent_2_Characters` | Agent 2 | 26 | Heroes, hero demo skins, companions, and companion demo skins |

Each folder has its own `README.md` with the exact source-image list and runtime/data wiring notes.
