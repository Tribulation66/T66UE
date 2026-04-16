# T66Mini Online Multiplayer Checklist

Last updated: 2026-04-15

## 1. Purpose

This file tracks the work required to bring real online co-op to Mini Chadpocalypse.

Target outcome for the first online pass:

- two real Steam players can party up
- the mini main menu friends panel shows live friend data and invite/join affordances
- both players can enter the same mini run together
- both players stay visible and interactive in the same live mini battle

This is a checklist only. It does not lock implementation details beyond the recommended first-pass scope below.

## 1.1 Editor Implementation Status

Completed in the editor-only pass on 2026-04-15:

- [x] Mini-specific lobby metadata now travels through the shared party/session layer:
  - mini hero
  - mini companion
  - mini difficulty
  - mini idol loadout
  - mini ready state
  - mini frontend screen state
- [x] The mini left-panel friend list is now interactive:
  - invite
  - join
  - pending invite state
  - in-party state
- [x] Mini pre-run flow is synchronized around party state:
  - host-owned difficulty
  - guest ready / unready
  - host start gating on both players being valid and ready
- [x] Mini online launch now uses the shared Steam/session flow instead of solo-only `OpenLevel()`.
- [x] Online mini run policy is now explicit in code:
  - 2-player oriented
  - party before start
  - no late join once the run begins
- [x] Mini online bootstrap now uses a host-owned party run model instead of a transient-only co-op bootstrap.
- [x] Mini online battle autosaves now capture per-player party snapshots on the host.
- [x] Mini party save-slot load now exists for the supported editor path:
  - host-only
  - current-party validation
  - mid-wave battle resume supported
  - intermission/shop resume still blocked
- [x] Mini battle runtime was converted to a supported host-authoritative co-op path:
  - server-owned player combat
  - server-owned wave progression
  - server-owned enemy spawn and AI
  - server-owned pickups, interactables, and traps
  - server-owned companion healing/follow logic
- [x] Mini battle actors now replicate enough for a live editor co-op run:
  - player pawns
  - enemies
  - player projectiles
  - enemy projectiles
  - pickups
  - interactables
  - hazard traps
  - companions
- [x] Online mini pause and save-and-quit assumptions were removed from the supported path.
- [x] Online mini loot-crate interaction no longer depends on a local pause flow.
- [x] Mini battle HUD now reads local live pawn state instead of solo run-save state for loadout/inventory.
- [x] Online mini run completion now prepares each client for a clean return to frontend summary flow, then server-travels the party back together.
- [x] Online mini intermission/shop now round-trips through the frontend as a synchronized host-authoritative flow:
  - gameplay returns the party to `MiniShop`
  - host publishes authoritative intermission state through the shared lobby profile
  - guests submit shop requests through the shared lobby profile
  - host processes guest requests and republishes the resulting state
  - host advances the party from `MiniShop` to `MiniIdolSelect`
- [x] Online mini disconnect/abandon handling now has deterministic behavior:
  - mid-run guest disconnect saves the host-owned party snapshot
  - remaining players are returned to frontend instead of being stranded in battle
  - intermission/shop flow rejects incompatible or broken party compositions
- [x] `T66Editor Win64 Development` currently builds with these changes.
- [x] The staged Development standalone has been refreshed at `Saved/StagedBuilds/Windows/T66`.

Still outside this editor-only pass:

- [ ] Steam-installed two-account validation on AppID `4464300`
- [ ] Packaged/staged standalone behavioral validation
- [ ] Mini-specific rich presence / backend diagnostics polish

## 2. Current Repo Baseline

- `UT66MiniMainMenuScreen` already reads friend and party data from the shared `UT66PartySubsystem` and avatar/presence data from `UT66SteamHelper`.
- The mini left panel is now wired into the shared party/session state and exposes invite/join affordances.
- Fresh online mini runs now bootstrap through a host-owned party run object instead of a transient guest-only run seed.
- Mini host save slots can now carry party-owned mid-wave battle state in the editor path.
- Online mini now has a synchronized shop/intermission loop and deterministic disconnect fallback, but still needs real Steam-installed validation.

## 3. Recommended First Online Scope

- [ ] Lock v1 scope to `2-player online only`.
- [ ] Lock v1 session policy to `party before start only`.
- [ ] Lock v1 join policy to `no late join after the mini battle has started`.
- [ ] Lock v1 save policy to `no online mid-wave save/resume` unless host-authoritative co-op save support is explicitly built.
- [ ] Lock v1 authority model to `host-authoritative listen server using the existing Steam party/session stack`.

## 3.1 Complete Implementation Order

1. Lock the mini online product rules so the implementation targets one complete path instead of a moving target.
2. Extend the shared party/session model to carry mini-specific lobby data.
3. Turn the mini main menu friends panel into a real party/session surface instead of a static display.
4. Build a synchronized mini pre-run lobby flow for both players.
5. Replace solo mini `OpenLevel()` launch with session-backed mini travel.
6. Define the authoritative multiplayer runtime contract for mini battle.
7. Retrofit the mini gameplay framework for multiplayer-safe controller, player-state, spawn, and match-state ownership.
8. Convert `AT66MiniPlayerPawn` and player combat to server-authoritative co-op behavior.
9. Convert enemies, projectiles, pickups, interactables, traps, and wave progression to multiplayer-safe ownership.
10. Rework intermission, shop, pause, and return-to-frontend flows for co-op.
11. Finalize online save/resume and progression policy for mini runs.
12. Add mini-specific backend, Steam presence, and diagnostics coverage.
13. Validate the full two-account Steam path on AppID `4464300` until invite, lobby, battle, intermission, completion, and disconnect handling are all clean.

## 4. Shared Party and Session Reuse

- [ ] Decide that mini online reuses `UT66PartySubsystem`, `UT66SessionSubsystem`, `UT66SteamHelper`, and the existing backend invite flow instead of creating a second mini-only Steam stack.
- [ ] Add mini-specific session metadata requirements:
  - selected mini hero
  - selected mini companion
  - selected mini idol loadout
  - selected mini difficulty
  - mini-ready state
- [ ] Decide whether those fields extend the existing lobby profile model or live in a dedicated mini lobby payload.
- [ ] Add a mini party-size rule so the mini flow advertises and enforces `Duo` instead of inheriting unrelated party-size behavior.
- [ ] Define how mini invite acceptance should route:
  - mini main menu open
  - regular main menu open
  - already in a regular-game lobby
  - already in a mini lobby

## 5. Mini Main Menu Friends Panel

- [ ] Convert the mini left panel from display-only to interactive.
- [ ] Add row actions for online friends:
  - invite to mini party
  - join friend if they are already hosting a mini party and join is allowed
- [ ] Add visual state for:
  - online
  - offline
  - already in your party
  - invite pending
  - in mini lobby
  - in mini run
- [ ] Wire the panel to `UT66PartySubsystem::OnPartyStateChanged()`.
- [ ] Wire the panel to `UT66SessionSubsystem::OnSessionStateChanged()`.
- [ ] Surface pending invite acceptance while the mini shell is open.
- [ ] Add host-only controls for removing a remote party member from the mini party.
- [ ] Decide whether the mini shell should show only party-relevant friends first or the full Steam friend list.
- [ ] Remove the current hard display cap of three online and three offline entries if that is only placeholder UI.

## 6. Mini Pre-Run Lobby Flow

- [ ] Create a real mini pre-run lobby state instead of jumping straight from main menu into solo character select.
- [ ] Decide whether character select, companion select, difficulty select, and idol select remain separate mini screens or collapse into one shared co-op lobby screen.
- [ ] Make host authority explicit:
  - host owns start
  - host owns difficulty
  - each player owns their own hero/companion/idol choices
- [ ] Add lobby ready state for each player.
- [ ] Gate match start until both players have valid mini selections and are ready.
- [ ] Show both players' selected mini loadouts in the pre-run UI.
- [ ] Decide whether duplicate heroes and duplicate companions are allowed in v1.
- [ ] Add disconnect/leave handling before the run starts.

## 7. Travel and Match Start

- [ ] Replace solo `OpenLevel()` mini launch with session-backed travel into the mini battle map.
- [ ] Add a host start path for mini that uses listen-server travel.
- [ ] Add a guest join path that travels into the same mini battle session.
- [ ] Ensure the mini travel options load the mini game mode for both host and guest.
- [ ] Ensure both players spawn into the battle map and receive the correct pawn/controller classes.
- [ ] Ensure post-run return flow can bring both players back to the correct frontend state without dropping one player into a broken menu state.

## 8. Mini Runtime Networking Conversion

- [ ] Add a mini-specific multiplayer runtime contract:
  - authoritative server owns wave progression
  - authoritative server owns enemy spawning
  - authoritative server owns trap spawning
  - authoritative server owns pickups/interactables
  - clients send input/intents
  - clients do not own game progression
- [ ] Audit all `GetFirstPlayerController()` and single-local-player assumptions across `Source/T66Mini`.
- [ ] Replace singleton player access with per-controller, per-player-state, or replicated collections as appropriate.
- [ ] Introduce mini runtime player state data for both players.
- [ ] Decide whether mini reuses `AT66SessionPlayerState` during battle or adds a dedicated `AT66MiniPlayerState`.
- [ ] Replicate wave timer, difficulty, stage state, and match-complete state from a multiplayer-safe game state.
- [ ] Make pawn spawning multiplayer-safe for two simultaneous players.
- [ ] Make movement and aiming network-safe.
- [ ] Convert auto-fire and ultimate activation to server-authoritative execution.
- [ ] Make damage, death, revive, XP, gold, materials, and level-up progression server-authoritative.
- [ ] Make companion logic multiplayer-safe.
- [ ] Replicate or reconstruct combat VFX/SFX in a net-safe way.

## 9. Actor-by-Actor Gameplay Pass

- [ ] `AT66MiniPlayerPawn`
  - convert stats/state from local-only runtime values to replicated or server-owned values
  - ensure both pawns can move, aim, attack, take damage, and die independently
- [ ] `AT66MiniGameMode`
  - remove one-player assumptions from wave start, save hooks, battle completion, and player lookup
- [ ] `AT66MiniGameState`
  - promote battle state into replicated multiplayer-safe fields
- [ ] Enemies
  - server spawns and owns enemies
  - clients see consistent positions, health, and death
- [ ] Projectiles
  - decide between replicated projectile actors or server-hit with client FX
- [ ] Pickups
  - server owns spawn, vacuum, and reward grants
- [ ] Interactables
  - server validates use and reward payout
- [ ] Traps
  - server owns spawn timing, pulses, and damage
- [ ] Shop/intermission
  - host-authoritative entry and exit
  - synchronized offer state for both players

## 10. HUD and Frontend Runtime UI

- [ ] Ensure the in-battle HUD reads only the owning local player's pawn data for health, gold, XP, cooldowns, and items.
- [ ] Add remote-player visibility cues:
  - remote player nameplate
  - remote health state
  - remote revive/down state if supported
- [ ] Decide whether the mini HUD needs a compact party panel during battle.
- [ ] Ensure pause flow is redesigned for online play:
  - remove local world pause assumptions
  - replace with host-only menu or non-pausing overlay
- [ ] Ensure shop/intermission UI supports two connected players without desync.

## 11. Save, Resume, and Progression Policy

- [ ] Explicitly disable current solo save-slot load/resume flow for online mini runs unless a co-op save format is implemented.
- [ ] If co-op saves are later supported, define:
  - host ownership of the save
  - party fingerprint validation
  - reconnect policy
  - resume eligibility for both players
- [ ] Prevent solo saves from being injected into an online mini session by accident.
- [ ] Decide how profile progression and unlock credit are awarded for both players after a successful online run.
- [ ] Decide whether failed co-op runs award any partial progress.

## 12. Backend, Steam, and Diagnostics

- [ ] Reuse the existing backend invite path for mini parties where useful.
- [ ] Add mini-specific session diagnostics so failed mini joins are distinguishable from regular-game joins.
- [ ] Add mini-specific rich presence text for:
  - in mini menu
  - in mini lobby
  - in mini run
- [ ] Confirm Steam overlay invites still work cleanly from the mini shell.
- [ ] Confirm backend invite acceptance still routes into the correct mini party flow.
- [ ] Ensure AppID and branch mismatch failures are surfaced clearly in mini UI.

## 13. Testing and Packaging

- [ ] Add a private Steam test checklist for mini:
  - both players launch through the Steam-installed build
  - host opens mini shell
  - friend list displays correctly
  - invite is sent and accepted
  - both players appear in the mini pre-run lobby
  - both players ready up
  - host starts mini run
  - both players spawn into the same mini battle
  - both players can move, attack, take damage, gain rewards, and finish a wave
  - shop/intermission stays in sync
  - run completion returns both players cleanly
- [ ] Add disconnect test cases:
  - guest leaves in lobby
  - guest disconnects in wave
  - host disconnects in wave
- [ ] Add wrong-build / wrong-branch / wrong-AppID tests.
- [ ] Validate the staged standalone build and Steam-uploaded build separately.

## 14. Definition of Done for V1

- [ ] A friend appears in the mini shell friends panel with actionable invite/join controls.
- [ ] Two Steam players can form a mini party and see both members in the mini lobby.
- [ ] Both players can lock selections, ready up, and start.
- [ ] Both players load into the same mini battle map in one live session.
- [ ] Core mini battle gameplay remains synchronized for the full run.
- [ ] Leaving, disconnecting, and end-of-run return paths do not strand the remaining player in a broken state.
- [ ] The Steam-distributed build passes the two-player private test on AppID `4464300`.

## 15. Full Support Parity Order

This is the exact implementation order to close the remaining gap between mini multiplayer and the regular game's supported multiplayer path.

Locked rules for this phase:

- no late join after a mini run has started
- no mid-run join
- no reconnect path that bypasses host-owned run state
- packaged/staged and Steam-installed validation are required parts of completion

1. Lock the permanent multiplayer rules in code and docs so there is no ambiguity:
   - no late join
   - no mid-run join
   - no online run start without both players present before launch
   - host-authoritative session remains the only supported authority model

2. Replace transient online mini run bootstrap with a real host-owned party save/load model:
   - define mini party save ownership
   - define mini party save identity validation
   - define where online mini save data persists
   - route online mini launch through the same durable run-state contract every time

3. Add mini party load travel support so the host can resume a valid co-op mini run intentionally instead of only starting fresh:
   - host loads party-compatible mini save
   - save contents propagate to party session state
   - both players travel into the same resumed mini run

4. Restore a real synchronized online mini intermission/shop flow instead of the current online shortcut:
   - both players survive wave completion into the same intermission state
   - host owns continue
   - shop state, rerolls, purchases, sell/buyback, debt, and item mutations stay synchronized
   - both players return from intermission into the next wave together

5. Replace the current online pause restrictions with a multiplayer-safe non-pausing party overlay:
   - no world pause
   - host/guest-safe controls
   - clean leave-run flow
   - clean save-and-exit flow if online mini saves are supported

6. Replace the current online loot-crate shortcut with a fully supported multiplayer behavior:
   - either synchronized presentation for both clients
   - or a deliberate instant-award flow with consistent UX and telemetry
   - no hidden local-only fallback logic

7. Finish disconnect and abandonment handling:
   - guest leaves in lobby
   - guest leaves during run
   - guest drops during intermission
   - host leaves during run
   - host leaves during intermission
   - remaining player gets a deterministic result and frontend return path

8. Define and implement the final no-rejoin policy cleanly:
   - once a run is in progress, new joins are rejected clearly
   - disconnected players do not silently re-enter
   - failure messaging explains the rule instead of looking like a broken session

9. Harden mini progression/reward parity for both players:
   - run summary correctness
   - unlock/clear credit correctness
   - duplicate-safe persistence
   - host/guest reward parity where intended
   - failed-run outcome policy

10. Harden mini-specific diagnostics and rich presence:
   - mini lobby presence
   - mini run presence
   - explicit mini join rejection diagnostics
   - explicit disconnect diagnostics
   - explicit intermission/travel diagnostics

11. Build packaged/staged regression coverage for mini multiplayer:
   - staged Development mini party flow
   - staged resume/load flow if supported
   - staged intermission loop
   - staged completion flow
   - staged disconnect handling

12. Run private Steam-installed validation on AppID `4464300` until the full supported path is clean:
   - invite
   - join
   - lobby ready
   - host start
   - same-world mini battle
   - intermission/shop loop
   - run completion
   - frontend return
   - disconnect handling

13. Treat completion as blocked until both editor and packaged/Steam paths pass. Mini multiplayer is not “fully supported” until the same flow works outside the editor repeatedly.
