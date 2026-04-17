# T66 Master Steamworks

**Last updated:** 2026-04-16  
**Scope:** Steamworks memory file and operating guideline for future agents. This is the canonical handoff for T66 Steam app state, SteamPipe upload workflow, private-testing process, current known build state, and the repeatable PowerShell flow used on this machine.  
**Companion docs:** `MASTER DOCS/MASTER_BACKEND.md`, `MASTER DOCS/T66_MASTER_GUIDELINES.md`  
**Maintenance rule:** Update this file after any Steamworks change, Steam build upload, branch/live-build switch, key request, release-state change, or multiplayer-validation milestone. If the change also affects backend auth/routes, update `MASTER DOCS/MASTER_BACKEND.md` in the same pass.

## 1. Current Steam Snapshot

- **Steam app:** `CHADPOCALYPSE`
- **AppID:** `4464300`
- **Primary Windows depot:** `4464301`
- **Current local Steam install state on this machine:** `buildid "22815997"` from `C:\Program Files (x86)\Steam\steamapps\appmanifest_4464300.acf` on 2026-04-16
- **Current release model:** unreleased app, private testing through Steam + Release Override keys
- **Current default branch used for testing:** `default`
- **Current Git backup snapshot:** branch `codex/version-2.2`, tag `v2.2`, commit `28a85e82`
- **Important mismatch to remember:** the source snapshot on `v2.2` includes work that may be newer than the currently installed Steam build. Do not assume the repo HEAD and Steam build are the same until a new upload is completed and the client manifest confirms the new `buildid`.

## 2. Source Of Truth Files

Future agents should treat these files and locations as the operational source of truth:

- `C:\UE\T66\Tools\Steam\UploadToSteam.ps1`
- `C:\SteamworksSDK\sdk\tools\ContentBuilder\scripts\CHADPOCALYPSE\app_build_4464300.vdf`
- `C:\Program Files (x86)\Steam\steamapps\appmanifest_4464300.acf`
- `C:\UE\T66\MASTER DOCS\MASTER_BACKEND.md`
- `C:\UE\T66\steam_appid.txt`
- `C:\UE\T66\Config\DefaultEngine.ini`
- `C:\UE\T66\Saved\StagedBuilds\Windows\`

## 3. The Correct Upload Workflow

### 3.1 Preferred path

Use the local PowerShell wrapper. It is the safest path because it:

- clears the Steam ContentBuilder content root first
- recopies the staged build into the Steam content root
- optionally injects a fresh description into the VDF
- avoids stale-content uploads that happened earlier in this project

### 3.2 Correct build source

The correct build source is the **root staged Windows folder**:

```powershell
C:\UE\T66\Saved\StagedBuilds\Windows
```

Do **not** upload the inner folder:

```powershell
C:\UE\T66\Saved\StagedBuilds\Windows\T66
```

Uploading the inner `T66` folder caused the earlier `.uproject`/bootstrap launch failure because Steam must receive the packaged root containing:

- `T66.exe`
- `Engine\...`
- `T66\...`

### 3.3 Standard upload command

Preferred command:

```powershell
powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Tools\Steam\UploadToSteam.ps1" `
  -BuildSource "C:\UE\T66\Saved\StagedBuilds\Windows" `
  -Description "Private multiplayer test YYYY-MM-DD short note"
```

Optional beta-branch live set:

```powershell
powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Tools\Steam\UploadToSteam.ps1" `
  -BuildSource "C:\UE\T66\Saved\StagedBuilds\Windows" `
  -Description "Private multiplayer test YYYY-MM-DD short note" `
  -SetLiveBeta "branch_name"
```

### 3.4 What the wrapper targets

- **Steamworks SDK root:** `C:\SteamworksSDK\sdk`
- **App build script:** `C:\SteamworksSDK\sdk\tools\ContentBuilder\scripts\CHADPOCALYPSE\app_build_4464300.vdf`
- **ContentBuilder content root:** `C:\SteamworksSDK\sdk\tools\ContentBuilder\content\CHADPOCALYPSE`

## 4. SteamCMD Authentication Notes

### 4.1 Normal login

If cached credentials are not active:

```powershell
C:\SteamworksSDK\sdk\tools\ContentBuilder\builder\steamcmd.exe
```

Then inside `steamcmd`:

```text
login tribulation66
```

Complete password entry and Steam Guard if prompted.

### 4.2 Cached login

Once `steamcmd` has cached credentials on this machine, future uploads can usually run without manual re-entry. Even then, future agents should not assume the cache is permanent; if upload fails with `Not logged on`, re-authenticate first.

### 4.3 Direct fallback path

If the wrapper is not being used, the direct fallback is:

```text
run_app_build "C:\SteamworksSDK\sdk\tools\ContentBuilder\scripts\CHADPOCALYPSE\app_build_4464300.vdf"
```

Only use this after confirming the ContentBuilder content root has already been refreshed from the current staged build. Otherwise it may upload stale content.

## 5. Steamworks Steps Future Agents Cannot Fully Automate

These actions still require manual Steamworks dashboard access:

- requesting Release Override keys
- package/license changes
- branch/package setup
- setting the **default** branch live after upload
- any public release/store visibility action

### 5.1 Standard post-upload manual step

After a successful upload:

1. Open the Steamworks builds page for AppID `4464300`
2. Find the new build row
3. Select `default` in `Set build live on branch...`
4. Click `Preview Change`
5. Click `Set Build Live Now`

While the app is unreleased, pushing a build live on `default` is still private to accounts that own the app through partner access or redeemed keys.

## 6. Private Testing Process

### 6.1 Recommended test model

- keep the app unreleased
- upload builds to AppID `4464300`
- use Release Override beta-test keys for non-partner test accounts
- install and launch through Steam on both machines

### 6.2 Key handling

- the requesting Steamworks account can download approved keys from `Manage Keys`
- the `.zip` contains the real key values
- second-account redemption path in Steam client:
  - `Games -> Activate a Product on Steam...`

Accounts that already have dev/package access do not need a key.

## 7. Validation Checklist After Every Upload

### 7.1 Verify the installed build

Do **not** trust the Steam UI alone. The source of truth is:

```text
C:\Program Files (x86)\Steam\steamapps\appmanifest_4464300.acf
```

Confirm:

- `buildid` matches the intended uploaded build
- `TargetBuildID` matches the intended uploaded build

Steam does not always show a blue update button immediately. If the manifest did not change, the machine is still on the old build.

### 7.2 Relaunch discipline

After setting a build live:

1. fully exit Steam
2. reopen Steam
3. confirm the client updates
4. verify `appmanifest_4464300.acf`
5. only then retest multiplayer

### 7.3 Runtime launch options for diagnostics

Use:

```text
-log -forcelogflush
```

This helps preserve the last useful lines if the game crashes or hard-fails.

## 8. Log And Diagnostic Locations

### 8.1 Steam-installed build

- `C:\Program Files (x86)\Steam\steamapps\common\CHADPOCALYPSE\T66\Saved\Logs\T66.log`
- `C:\Program Files (x86)\Steam\steamapps\common\CHADPOCALYPSE\T66\Saved\Diagnostics\Multiplayer\*.json`

### 8.2 Typical handoff pattern

For multiplayer debugging, future agents should ask for:

- host `T66.log`
- guest `T66.log`
- all host/guest `Saved\Diagnostics\Multiplayer\*.json`
- a plain-English note describing host/guest, test time, and observed behavior

## 9. Known Operational Gotchas

### 9.1 Wrong staged folder level breaks launch

If Steam is pointed at the inner staged `T66` folder instead of `Saved\StagedBuilds\Windows`, the app can try to bootstrap incorrectly and fail looking for `T66.uproject`.

### 9.2 Stale ContentBuilder content can produce fake “new” builds

This happened before. A new build ID can still contain old content if the ContentBuilder content root was not refreshed. The PowerShell wrapper exists specifically to prevent that by clearing and recopying the content root.

### 9.3 Steam client update behavior is independent from in-game update UI

Do not conflate Steam’s patch/update behavior with any in-game “checking for updates” overlay. Steam updates happen before the game launches. Game code cannot force Steam to fetch a build immediately.

### 9.4 Bridge logs are transient noise, not source assets

Do not treat these as meaningful project files or include them in source-control snapshots:

- `C:\UE\T66\Tools\ChatGPTBridge\bridge_5001.err.log`
- `C:\UE\T66\Tools\ChatGPTBridge\bridge_5001.out.log`
- `C:\UE\T66\Tools\ChatGPTBridge\mini_bridge_runtime.log`

## 10. Current Multiplayer Status Snapshot

As of this handoff:

- main-game Steam multiplayer under AppID `4464300` has been brought to a working host/guest state through the recent multiplayer pass
- the current installed Steam build on this machine is `22815997`
- the Git backup snapshot is `codex/version-2.2` / `v2.2` / `28a85e82`
- source control now contains additional post-build work that future agents must not assume is already present in the installed Steam build unless a new upload has been performed and verified

## 11. Future-Agent Rule

When resuming Steam/Steamworks work:

1. read this file first
2. read `MASTER DOCS/MASTER_BACKEND.md` second
3. verify local `appmanifest_4464300.acf`
4. verify the current branch/tag/working tree
5. prefer `Tools/Steam/UploadToSteam.ps1` over manual ContentBuilder copying
6. never assume the current repo state is the same as the currently installed Steam build
