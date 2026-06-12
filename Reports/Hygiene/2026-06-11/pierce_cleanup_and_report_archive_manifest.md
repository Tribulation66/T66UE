# Pierce Cleanup And Report Archive Manifest

Date: 2026-06-11

## Scope

This pass finished the live Pierce cleanup outside historical material, then reduced the live report/audit surface to current or still-referenced material.

Historical text inside `Reports/`, `Audit/`, and `Archive/` was not rewritten. Stale report/audit folders were moved as whole artifacts so their original contents remain available.

## Live Pierce Cleanup

Cleaned current non-historical workspace references outside `Reports/`, `Audit/`, and `Archive`:

- Regenerated `/Game/VFX/Foundation/OutgoingTravelers/NS_OutgoingTravelerPool` with `TravelerVisual.*.Summon` profile assets.
- Regenerated `Content/Localization/T66` manifest, archives, and locres files from current source/assets.
- Updated `Tools/ArtPipeline/Items/T66ProcessReimaginedItemSheets.py` from Pierce item sheet IDs to Summon item sheet IDs.
- Updated `Model Generation/Runs/Pixal3D/InflatableProjectiles_20260611/InflatableProjectiles_20260611_manifest.json` to describe the current idol categories.

## Archive Destination

Moved stale report/audit material to:

`Archive/ReportsAuditCleanup_20260611/`

Moved count: 147 paths.

Primary move groups:

- `Audit/Pending/*`
- `Audit/5-18 performance audit`
- `Audit/Inventory Cleanup`
- pre-June `Reports/AgentReviews/*`
- `Reports/RosterReview`
- stale raw proof buckets:
  - `Reports/Proof/Items`
  - `Reports/Proof/DemoGatingVisibility`
  - `Reports/Proof/UndeprecateMinigamesDemoGate`
  - `Reports/Proof/UI`

## Kept Live

Kept in place:

- current June report/review work under `Reports/AgentReviews`
- `Reports/AgentReviews/ClaudeDirectRead` as an empty routing folder for future tool output
- current/recent proof domains under `Reports/Proof`
- `Reports/Proof/CombatVFX`, because active combat/VFX process docs still reference it
- `Reports/Hygiene`
- `Audit/Finished`
- `Audit/Reference`
- `Audit/Pending` as an empty routing folder matching `Audit/README.md`

## Verification

Commands run after cleanup:

```powershell
rg -a -l -i 'pierce' . -g '!Saved/**' -g '!Intermediate/**' -g '!Binaries/**' -g '!.git/**' -g '!Reports/**' -g '!Audit/**' -g '!Archive/**'
rg --files -g '!Saved/**' -g '!Intermediate/**' -g '!Binaries/**' -g '!.git/**' -g '!Reports/**' -g '!Audit/**' -g '!Archive/**' | rg -i 'pierce'
```

Both returned no hits.

Unreal commands:

```powershell
UnrealEditor-Cmd.exe C:\UE\T66\T66.uproject -run=T66OutgoingTravelerSwarmVFX -ProductionPool -unattended -nop4
UnrealEditor-Cmd.exe C:\UE\T66\T66.uproject -run=GatherText -config=C:\UE\T66\Config\Localization\T66_Gather.ini -unattended -nop4
```

Both exited with code 0. The localization gather reported existing localization key conflict warnings unrelated to Pierce cleanup.
