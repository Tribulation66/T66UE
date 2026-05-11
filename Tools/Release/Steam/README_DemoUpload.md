# CHADPOCALYPSE Demo Steam Upload

The demo uses the same `T66.uproject` and source tree as the full game. It becomes a demo through the central release-variant gate and the Steam demo AppID.

## IDs

- Full game AppID: `4464300`
- Demo AppID: `4718770`
- Demo depot ID: `4718771`

## Stage The Demo

From `C:\UE\T66`:

```powershell
.\Scripts\StageDemoBuild.ps1 -ClientConfig Shipping
```

This stages to:

```text
C:\UE\T66\Saved\StagedBuildsDemo\Windows\T66
```

The script writes a local-only `steam_appid.txt` with `4718770` beside the demo executable and creates `C:\UE\T66\T66 Demo Standalone.lnk` with `-T66Demo`.

## Upload The Demo

```powershell
.\Tools\Release\Steam\UploadDemoToSteam.ps1 -Description "Demo build YYYY-MM-DD"
```

The wrapper creates the SteamPipe scripts if missing:

- `app_build_4718770_root.vdf`
- `depot_build_4718771.vdf`

It reuses `UploadToSteam.ps1`, copies the staged build into `ContentBuilder\content\CHADPOCALYPSE_DEMO`, and strips every `steam_appid.txt` before upload.

## Validation

- Launch `T66 Demo Standalone.lnk` locally and confirm only `Easy`, `Medium`, `Hero_1`, `Hero_2`, `Hero_3`, and `Hero_4` are selectable.
- After uploading, set the build live only on a private/test branch first.
- Install the Steam demo AppID `4718770` from Steam and confirm the overlay reports the demo app, not the full game app.
