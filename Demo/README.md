# T66 Steam Demo Workspace

**Goal:** Prepare the CHADPOCALYPSE demo for Steam Next Fest: October 2026 while keeping the production game, demo build lane, backend policy, and Steamworks release state explicit.

**Full game AppID:** `4464300`  
**Demo AppID:** `4718770`  
**Demo depot ID:** `4718771`  
**Publisher ID shown during creation:** `369728`  
**Developer Comp package ID:** `1641847`  
**Beta Testing package ID:** `1641848`  
**Demo package ID:** `1641849`  
**Store item ID:** `1181780`  
**Demo depot name:** `CHADPOCALYPSE Demo Content`  
**Steamworks config publish status:** initial demo app/depot metadata published; app remains unreleased/private  
**Target event:** Steam Next Fest: October 2026  
**Target public demo release window:** before October 19, 2026 at 10:00 AM PDT

## Current Decision

- Use the existing `T66.uproject`; do not fork the Unreal project for the demo.
- Create a separate Steam demo AppID attached to full game AppID `4464300`.
- Enable the optional separate demo store page because the demo is intended for Steam Next Fest.
- Keep the demo private during infrastructure testing through Steam keys and unreleased Steamworks state.
- Do not globally release the demo until the demo build, store page, backend rules, and Next Fest registration are ready.

## Official References

- Steam demos: https://partner.steamgames.com/doc/store/application/demos?language=english
- Steam Next Fest: October 2026: https://partner.steamgames.com/doc/marketing/upcoming_events/nextfest/2026october
- Steam Next Fest parent docs: https://partner.steamgames.com/doc/marketing/upcoming_events/nextfest?language=english
- Steam keys: https://partner.steamgames.com/doc/features/keys?language=english
- SteamPipe uploads: https://partner.steamgames.com/doc/sdk/uploading?language=english

## October 2026 Next Fest Dates

From Valve's October 2026 Next Fest page:

- August 31, 2026: registration deadline.
- September 21, 2026: submit demo build and store page for review if the demo should be live for Press Preview.
- October 5, 2026: all required items must be submitted for review for Steam Next Fest readiness.
- October 8, 2026: Press Preview starts at 10:00 AM PDT.
- October 19, 2026: Steam Next Fest begins at 10:00 AM PDT; demo must be live before this.
- October 26, 2026: Steam Next Fest concludes.

## Steamworks Creation Checklist

- From the base game landing page for `4464300`, open **All Associated Packages, DLC, Demos And Tools**.
- Choose **Add Demo**.
- Check **Enable store page for this demo**.
- Click **Create Demo**.
- Record the generated demo AppID in this README. Confirmed associated demo AppID: `4718770`.
- Open the demo app's Steamworks settings.
- Confirm the app type is `Demo`.
- Confirm the base game AppID is `4464300`.
- Create or confirm the Windows demo depot. Confirmed depot ID: `4718771`.
- Record the demo depot ID in this README.
- Keep the demo unreleased until ready for public demo release.
- Steamworks initial app/depot metadata has been published. Steamworks confirmed the unreleased app is only accessible to app owners with a release-state override package.

## Private Testing Model

Use Steam keys while the demo is unreleased:

- Use Release State Override keys for small closed testing.
- Do not sell Release State Override keys.
- Do not use Default Release keys for unreleased demo testing.
- External testers redeem through Steam: **Games -> Activate a Product on Steam...**.
- Partner accounts may already own the demo automatically once associated.

## T66 Build Lane Plan

The demo should be a build/distribution lane, not a separate project:

- Keep the demo in the existing `T66.uproject` and existing source tree.
- Add a central release-variant/content-gate layer instead of forking data tables or creating a second project folder.
- Add a demo runtime flag or config path that can be enabled for staged demo builds.
- Stage demo builds separately from the full game at `Saved/StagedBuildsDemo/Windows/T66`.
- Create demo SteamPipe VDF scripts targeting demo AppID `4718770` and demo depot `4718771`.
- Add a demo upload wrapper or parameterize `Tools/Release/Steam/UploadToSteam.ps1`.
- Ensure local-only `steam_appid.txt` is not uploaded to the demo depot.
- Verify installed Steam demo builds through the demo `appmanifest_*.acf`, not just the Steam UI.

Current local command path:

```powershell
.\Scripts\StageDemoBuild.ps1 -ClientConfig Shipping
.\Tools\Release\Steam\UploadDemoToSteam.ps1 -Description "Demo build YYYY-MM-DD"
```

## Demo Content Scope

Current first-pass scope:

- Available difficulties: `Easy`, `Medium`.
- Locked difficulties: `Hard`, `VeryHard`, `Impossible`.
- Available heroes: `Hero_1`, `Hero_2`, `Hero_3`, `Hero_4`.
- Locked heroes: every hero after `Hero_4`.

Implementation rule:

- Do not delete full-game rows from `Content/Data/Heroes.csv`.
- Do not create a second copy of the Unreal project.
- Do not create demo-only hard-coded C++ defaults for gameplay data.
- Gate demo availability through central release/content settings so UI, save loading, party/lobby sync, gameplay entry, and backend submission all agree.

## Backend And Online Policy

T66's ranked and account systems are backend-authoritative, so the demo must not accidentally behave like the full production app.

Required planning items:

- Add the demo AppID to backend Steam ticket validation intentionally.
- Decide whether demo runs are fully unranked, demo-only ranked, or isolated by demo-prefixed leaderboard keys.
- Keep demo submissions out of full-game `leaderboard_entries` unless explicitly approved.
- Preserve `integrity_context.steam_app_id` so backend moderation can distinguish demo and full-game clients.
- Decide whether demo accounts use the same player profile rows or a separate demo scope.
- Confirm `/api/client-config` build policy has a demo AppID entry before using it as an update gate.

## Remaining Content Scope Decisions

Open decisions:

- Whether multiplayer is included in the demo.
- Whether saves carry into the full game.
- Whether Steam Cloud is shared with the full game.
- Whether achievements are disabled in the demo.
- Whether the demo menu includes a Steam overlay link to the full game's store page.

## Store Page Requirements

Because the separate demo store page is enabled, prepare demo-specific store material:

- Short description focused only on demo content.
- Long description focused only on demo content.
- Demo-specific screenshots.
- Demo-specific trailer or clear gameplay video if used.
- Capsule/library assets that clearly mark the app as a demo.
- Content survey that reflects demo content, not full-game-only content.
- Supported features, languages, and player modes that match the demo.

## Validation Checklist

Before private demo keys:

- Demo AppID and depot exist.
- Demo build launches from Steam.
- Steam overlay initializes under the demo AppID.
- Backend auth succeeds or online features are intentionally disabled.
- Full-game-only features are gated.
- Demo cannot write full-game ranked rows by accident.
- Demo store page checklist is not blocking private testing.

Before Press Preview or Next Fest:

- Demo build submitted for review.
- Demo store page submitted for review.
- Base game store page is public/Coming Soon and eligible.
- Next Fest registration is complete from the base game AppID, not the demo AppID.
- Demo branch/live build is verified through the Steam app manifest.
- Demo launch, quit, crash logs, and update behavior are tested from Steam.

## Repo Additions

Implemented local demo infrastructure:

- `Scripts/StageDemoBuild.ps1`
- `Tools/Release/Steam/UploadDemoToSteam.ps1`
- `Tools/Release/Steam/README_DemoUpload.md`

Remaining external/non-repo setup:

- SteamPipe app/depot VDF scripts under the Steamworks ContentBuilder script root are generated by `UploadDemoToSteam.ps1` if missing
- backend AppID allow-list update in `C:\UE\Backend`
