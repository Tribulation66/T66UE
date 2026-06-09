# Decision Block - FriendSlop Ragdoll Infrastructure Scope

## Current Status

Codex and Claude agree the next technical step is not feel tuning. The next implementation phase should build a reusable ragdoll/knockback infrastructure path and validate passive ragdoll stability before Physical Animation / active-ragdoll drive is reintroduced.

## Decision Needed Before Implementation

The exact infrastructure boundary depends on three user-owned design decisions:

1. Actor coverage:
   - Recommended default: hero + bosses + selected elites use real ragdoll/knockback; normal horde uses cheap hit reactions/death pops/VFX.
   - Alternative: hero-only for now, with shared interfaces prepared but no enemy support.

2. Networking:
   - Recommended default: single-player/local authority for now; no replication architecture in the first pass.
   - Alternative: build server-authoritative/replication-safe ragdoll from the start.

3. Simultaneous physics budget:
   - Recommended default: one hero plus a very small number of premium actors, with a hard budget/fallback policy.
   - Alternative: larger active-ragdoll pool, which needs an explicit LOD/sleep/pooling coordinator earlier.

## Safe Implementation Assumption If User Approves

Build a per-character reusable C++ component path first, not a TestRoom GameMode path:

- Per-actor component owns ragdoll state, incapacitation, impulse application, PAC activation, recovery, and capsule handoff.
- Data assets/profiles own body set, Physical Animation settings, constraint profile names, impulse scales, duration, and budget class.
- TestRoom becomes a harness that triggers the component on the selected hero, not the owner of ragdoll logic.
- A later world subsystem can coordinate budgets and LOD when enemies/bosses join the system.

## Stop Condition

Do not implement until the user confirms or overrides the actor coverage, networking, and budget defaults.
