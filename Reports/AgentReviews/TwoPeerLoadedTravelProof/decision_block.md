# Two-Peer Loaded-Travel Proof Decision Block

## Working task

Operator: Codex
Validator: Claude
Scope: continue into the next multiplayer infrastructure proof by verifying current repo/runtime constraints and either implementing the two-peer loaded-travel proof or stopping on a concrete blocker.
Stop condition: live constraints are checked, Claude review is incorporated, proof or blocker is documented, and the next step is explicit.

## Current blocker

The production loaded-save multiplayer travel path is gated by `UT66SessionSubsystem::StartLoadedGameplayTravel`, which requires both:

- an active party session through `IsPartySessionActive()`
- host ownership through `IsHostingPartySession()`

`Config/DefaultEngine.ini` configures multiplayer validation for Steam (`DefaultPlatformService=Steam`, Steam NetDriver, real Steam AppID `4464300`). A deterministic production-faithful two-peer proof therefore needs a Steam-enabled host/client environment with two usable Steam identities or an equivalent approved test setup.

Current machine inspection during this pass did not show Steam or a live T66 game process running, so Codex cannot verify that the production two-identity blocker is gone.

## Decision choices

1. Production-faithful Steam proof:
   - Proves the actual session-owned loaded-save travel path.
   - Requires the user to provide/confirm a dual-Steam-identity test environment.
   - Codex can then build/run the proof around the existing Steam session boundary.

2. Engine-level local IP/NULL proof:
   - Proves a two-process host/client listen-server travel mechanics path.
   - Does not prove the Steam party-session gate, unless the runtime is explicitly reconfigured or instrumented for a non-production subsystem.
   - Must be labeled as an engine/network harness, not as production session-owned proof.

3. Host-side listen-server increment:
   - Proves that a host can perform live `ServerTravel(...?listen)` into gameplay and bind as a listen server.
   - Keeps remote-client join explicitly unproven.
   - Lowest overclaim path if the dual-Steam environment is not available yet.

## User decision

The user selected option 1 on 2026-06-08.

The active boundary is production-faithful Steam proof. Do not substitute a NULL/IP/direct-listen proof and report it as session-owned multiplayer proof.

Current follow-up artifact:

- `Reports/AgentReviews/TwoPeerLoadedTravelProof/option1_steam_proof_runbook.md`

Current blocker:

- Steam is installed but was not running when Codex checked.
- The staged packaged runtime preflight exited cleanly but logged Steam API initialization failure and fell back to `NULL`.
- Codex still needs a two-identity Steam environment to execute option 1.
