# Option 1 Steam Two-Peer Loaded-Travel Proof Runbook

## Boundary

This runbook is for the production-faithful proof only. It must exercise the real Steam-backed party session path and must not be replaced by an IP, NULL, direct-listen, or single-process proof.

## Current status

Option 1 was selected on 2026-06-08.

The repo and staged build are positioned for the production proof, but the current machine is not ready to execute it:

- Staged executable exists: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Staged executable timestamp observed by Codex: `2026-06-08 03:14:28`
- Staged `steam_appid.txt` exists beside the executable and contains `4464300`
- Steam is installed at `C:\Program Files (x86)\Steam\steam.exe`
- Steam was not running when Codex checked processes
- Runtime preflight log: `C:\UE\T66\Saved\AgentReviews\TwoPeerLoadedTravelOption1\steam_preflight_quit.log`
- Runtime preflight result: packaged quit exited `0`, but Steam failed to initialize while no Steam process was running, then T66 disabled online features

Relevant preflight markers:

```text
LogOnline: Warning: STEAM: Steam API failed to initialize!
LogOnline: OSS: Created online subsystem instance for: NULL
LogT66Steam: Warning: SteamHelper: SteamAPI_Init failed for Steam App ID 4464300.
LogT66Steam: Warning: SteamHelper: Steam not available and no DevTicket configured. Online features disabled.
```

## Required environment

The proof needs all of the following at the same time:

1. Host endpoint logged into Steam identity A.
2. Client endpoint logged into Steam identity B.
3. Both identities can launch AppID `4464300`.
4. The two identities can see/join each other through Steam friends, invite, lobby, or presence.
5. Both endpoints run the same staged or Steam-distributed build.
6. Both endpoints write accessible logs with `-abslog=<path>` and `-forcelogflush`.

A single local Steam client with one logged-in account is not enough. If this is run on one physical machine, the second identity needs an approved sandbox/VM/second Windows user setup that can run a separate Steam client session.

## Production code path to prove

The proof must drive this path:

1. Host creates or enters a Steam party lobby through `UT66SessionSubsystem::EnsurePartySessionReady`.
2. Client joins that party through Steam lobby/session join, invite acceptance, or presence.
3. Host and client reach a lobby state where the game reports more than one lobby player.
4. Host opens Save Slots and clicks a Duo save slot whose saved party shape matches the active lobby.
5. `UT66SaveSlotsScreen::OnLoadClicked` detects party resume flow.
6. The host-only branch calls `UT66SessionSubsystem::StartLoadedGameplayTravel`.
7. `StartLoadedGameplayTravel` confirms active party session and host ownership, applies the loaded run, applies saved party profiles, and calls `StartGameplayTravel`.
8. `StartGameplayTravel` prepares the Steam party session for world travel and calls `ServerTravel("<GameplayMap>?listen")`.
9. The connected client follows the host into gameplay.

The client invite/join step has not been runtime-verified yet in this environment. It is the first production path to confirm once two logged-in Steam identities are available.

## Acceptance evidence

Do not mark the proof complete unless both host and client logs show the production path.

Host evidence:

- Real Steam initializes, with no fallback to `NULL` as the active proof path.
- Party session creation succeeds.
- Lobby player count is greater than one before load.
- Host is recognized as the party host.
- Save Slots load uses the party resume branch.
- `StartLoadedGameplayTravel` succeeds.
- Gameplay travel uses a `?listen` URL.
- Host reaches the gameplay map after travel.

Client evidence:

- Real Steam initializes, with no fallback to `NULL` as the active proof path.
- Client joins the host party/session.
- Client receives the host travel.
- Client reaches the same gameplay map.
- Client gameplay setup completes without timing out waiting for host run settings.

Forbidden proof substitutions:

- A single-process `T66.Session.VerifyLoadedTravelPlan` run.
- Any proof that logs `LiveTravelSkipped=1`.
- A direct IP/NULL join presented as Steam session proof.
- A host-only `ServerTravel` proof presented as remote-client proof.

## Current blocker

The current Codex machine cannot execute this runbook yet because Steam was not running, the packaged preflight could not initialize Steam, and no second logged-in Steam identity/end-point is visible to the agent. The next action is to provide or start the two-identity Steam environment, then rerun the proof with separate host and client logs.
