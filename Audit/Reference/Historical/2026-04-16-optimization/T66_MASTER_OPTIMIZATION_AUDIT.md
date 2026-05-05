# T66 Master Optimization Audit

As of April 17, 2026.

This document is the master optimization and responsiveness audit for `T66`, `T66Mini`, Steam/session plumbing, backend invite services, and shared infrastructure. This phase is audit-only. No gameplay code, UI code, backend code, or build-system changes are included here.

## Scope and Source of Truth

This audit is grounded in the current repo and backend, with existing audits used as supporting context rather than primary authority.

Primary inputs:

- `MASTER DOCS/README.md`
- `MASTER DOCS/T66_MASTER_GUIDELINES.md`
- `MASTER DOCS/T66_PROJECT_CATALOGUE.md`
- `MASTER DOCS/MASTER_BACKEND.md`
- `MASTER DOCS/MASTER_STEAMWORKS.md`
- `MASTER DOCS/MASTER_COMBAT.md`
- `MASTER DOCS/MASTER_MAP_DESIGN.md`
- `Audit/Finished/PERFORMANCE_AUDIT.md`
- `Audit/Reference/T66_UI_AUDIT.md`
- `Audit/audit 4.16.26/T66_Hygiene_Optimization_Audit.md`
- `C:\UE\Backend`

Governing assumptions:

- Existing master docs and current repo behavior win when older notes conflict.
- Player-perceived responsiveness outranks ideal architecture when the two compete.
- Steam Deck and low-end friendliness remain first-class constraints, but the initial measurement baseline is packaged Development standalone on desktop Windows plus Steam-installed multiplayer runs.
- Recommendations are ordered by feel impact first, then by structural leverage.

## Executive Summary

### Top Feel Killers

1. **Party invite accept -> actual joined party latency is being paid across multiple hops instead of one healthy-path handoff.**
   The current flow spans invite polling, accept POST, modal rebuild, session bootstrap, optional friend-session lookup, optional session destroy, and travel. The backend route logic is thin; the felt delay is mostly orchestration and retry behavior on the client side.
2. **`PowerUp` open cost is likely dominated by heavyweight screen rebuild and image-resolution work.**
   `PowerUp` is `UT66ShopScreen`, and the current frontend model activates screens by building/rebuilding widget trees, refreshing screen state immediately on activation, and resolving numerous image assets. That is exactly the kind of path that makes a top-bar click feel sluggish.
3. **Hero select -> gameplay is partially preloaded, but the preload contract is not fully async-safe.**
   `UT66GameInstance` does meaningful preloading work, but `UT66CharacterVisualSubsystem::PreloadCharacterVisual` still routes through synchronous visual resolution. Several gameplay-adjacent assets still have `LoadSynchronous()` fallbacks in important first-use paths.
4. **Steady-state jank risk remains high where runtime work is still frame-driven instead of event-driven.**
   The main runtime has already improved in some places through registries, but Mini still has multiple per-tick `TActorIterator` scans. Material parameter churn, HUD rebuild work, hidden widget-component overhead, and repeated VFX/MID setup continue to threaten stable frame pacing.

### Top Architectural Liabilities

- The frontend is still too rebuild-centric for high-frequency navigation and modals.
- Asset loading policy is not strict enough about forbidding runtime loose-file imports and synchronous fallback loads on user-facing paths.
- Multiplayer joining is not optimized around a single healthy path with immediate handoff data.
- Mini does not yet have architectural parity with the main runtime for actor lookup, pooling, and presentation cost control.

### Fix Before Art Focus

These are the highest-value items to finish before the next art-heavy phase:

1. Remove healthy-path fixed retry behavior and extra hops from invite accept -> join flow.
2. Make `PowerUp` and other top-bar destinations warm-open fast by eliminating screen rebuild and sync asset work from the open path.
3. Convert gameplay preload from "async followed by sync fallback" to a truly async readiness contract.
4. Eliminate Mini per-tick actor scans and first-wave presentation churn.
5. Standardize scenario-based instrumentation so every future "this feels slow" complaint maps to the same timing markers.

## Canonical Scenario Catalog

Every latency claim in this document should be measured against one of these scenarios. Start and end markers are mandatory so "fast" and "slow" stay comparable across runs.

| ID | Scenario | Start Marker | End Marker |
| --- | --- | --- | --- |
| `FE-01` | Main menu -> `PowerUp` cold open | User click enters `HandleShopClicked()` / input dispatch | First fully drawn `UT66ShopScreen` frame with interaction enabled |
| `FE-02` | Main menu -> `PowerUp` warm open | User click enters `HandleShopClicked()` | First fully drawn warm-open frame with interaction enabled |
| `FE-03` | Main menu -> `Settings` / `Achievements` / `Account Status` cold open | User click enters top-bar handler | First fully drawn destination screen frame |
| `FE-04` | Cached tab or screen switch warm | Navigation input dispatch | First fully drawn destination tab/screen frame |
| `FE-05` | Click/input -> visible feedback | Input event begins | User-visible state change appears on screen |
| `FE-06` | Hero select -> gameplay | Confirm/start action begins | Party is spawned, controllable, and first gameplay input is accepted |
| `FE-07` | Gameplay -> frontend return | Return/leave action begins | First fully interactive frontend screen frame |
| `MP-01` | Invite send -> pending invite visible | Host submits invite | Invite appears in target player's pending-invite UI |
| `MP-02` | Invite receive -> modal visible | Client receives or polls invite result | Invite modal is visible and actionable |
| `MP-03` | Invite accept -> joined party lobby | Guest presses accept | Guest is in host party/lobby and ready for follow-up action |
| `MP-04` | Invite accept while host already has active party session | Guest presses accept | Guest either joins immediately or receives deterministic fallback without dead time |
| `MP-05` | Invite accept while guest already has active party session | Guest presses accept | Guest reaches correct joined state after any required handoff |
| `MP-06` | Mismatch/fallback join path | Fallback path begins | User receives success or actionable failure state |
| `GP-01` | Tower stage start | Gameplay map opens or stage init begins | Stage is visually stable and gameplay-ready |
| `GP-02` | First enemy wave | Wave-start trigger fires | First wave reaches steady frame pacing |
| `GP-03` | High-enemy-count steady state | Encounter enters target density | Frame pacing remains inside budget without repeated hitches |
| `GP-04` | Boss encounter | Boss spawn/intro begins | Boss phase reaches steady playable state |
| `CT-01` | Attack input -> hit feedback | Attack input or auto-attack decision occurs | Visible and audible hit feedback starts |
| `MN-01` | Mini character select open | User opens Mini character select | First fully interactive character-select frame |
| `MN-02` | Mini gameplay wave | Mini wave-start trigger fires | Mini gameplay reaches steady frame pacing |

## Standard Budget Table

This table defines the optimization bar for this project. Anything materially above these numbers should be treated as a real defect unless there is a hard platform or engine reason.

| Budget Area | Target |
| --- | --- |
| Frontend screen/modal open, cold | `<= 300 ms` |
| Frontend screen/modal open, warm | `<= 150 ms` |
| Cached tab/screen switch | `<= 50 ms` |
| Click/input -> visible feedback | `<= 50 ms` |
| Invite send -> pending invite visible on healthy backend/network | `<= 500 ms` |
| Invite accept -> joined party lobby happy path | `<= 1000 ms` with no fixed 1 s retry on healthy path |
| Non-travel avoidable hitch budget, desktop baseline | No repeated game-thread spikes `> 16 ms` |
| Non-travel avoidable hitch budget, low-end / Steam Deck baseline | No repeated game-thread spikes `> 25 ms` |
| Packaged gameplay target | Stable `60 FPS` without avoidable `60 -> 30` oscillation from tick/query/material/VFX churn |

## Priority Matrix

- `P0 feel wins`
  Immediate player-facing responsiveness problems, hitch sources, or architectural issues that directly block responsiveness work.
- `P1 foundational fixes`
  Important infrastructure, lifecycle, and data-flow changes that materially improve responsiveness or reduce regression risk, but are not the first things the player feels.
- `P2 longer-horizon gold-standard cleanups`
  Improvements that align the project with stronger industry practice, reduce technical drag, or improve iteration speed, but should not outrank direct feel wins.

## Canonical Evidence Format

Every future optimization finding should fit this shape:

- `Scenario`
- `Subsystem / area`
- `Current behavior`
- `Repo evidence`
- `Current risk`
- `Proposed remediation`
- `Validation method`

## Measurement and Validation Standard

- Use packaged Development standalone as the main truth for frontend/gameplay timings.
- Use a Steam-installed build for invite/join testing.
- Capture cold and warm runs separately.
- Use Unreal Insights for deep captures, but keep a lightweight repeatable pass built around:
  - `T66.LagTracker.Enabled`
  - `T66.LagTracker.DumpSummary`
  - `T66.Perf.Dump`
  - `[LOAD]` logs
  - `Saved/Diagnostics/Multiplayer/*.json`
- Require timing markers around both success and fallback paths.

## Area 1: Frontend UX and Screen Lifecycle

**Priority: `P0`**

### Current Situation

The frontend screen model is still activation-and-rebuild oriented. `UT66UIManager::ShowScreen()` and `ShowModal()` create or reuse a screen object, add it to the viewport, then immediately call `OnScreenActivated()`. `UT66ScreenBase::OnScreenActivated_Implementation()` immediately calls `RefreshScreen()`, and heavy screens such as `UT66ShopScreen` implement `RefreshScreen_Implementation()` as `ForceRebuildSlate()`.

This is functional, but it means the perceived open cost of a top-bar click is dominated by whatever work the destination screen performs during activation and rebuild.

### Responsiveness / Jank Risks

- A top-bar click can trigger full widget-tree rebuild on a heavy destination screen.
- Modal close/open can cause the underlying screen to refresh again, creating chained UI work during interaction-heavy flows.
- Warm navigation is not guaranteed to be warm if screen activation still forces rebuild or asset resolution.
- Responsive layout changes can trigger additional rebuild pressure through top-bar refresh behavior.

### Structural / Gold-Standard Risks

- The current model conflates `screen activation`, `screen state refresh`, and `screen widget rebuild`.
- Screen reuse exists, but the lifecycle still behaves like "recreate the experience" instead of "reactivate cached state".
- High-traffic screens are not clearly split into `warm persistent shell` plus `data-bound content`.

### Evidence Anchors

- `Source/T66/UI/T66UIManager.cpp`
  - `CreateScreen()`
  - `ShowScreen()`
  - `ShowModal()`
  - `CloseModal()`
- `Source/T66/UI/T66ScreenBase.cpp`
  - `RebuildWidget()`
  - `OnScreenActivated_Implementation()`
  - `ForceRebuildSlate()`
- `Source/T66/UI/T66FrontendTopBarWidget.cpp`
  - `HandleShopClicked()`
  - top-bar rebuild behavior
- `Source/T66/UI/Screens/T66ShopScreen.cpp`
  - `RefreshScreen_Implementation()`
  - `BuildSlateUI()`

### Proposed Solutions

- Split screen lifecycle into explicit phases:
  - `cold construct`
  - `warm activate`
  - `data refresh`
  - `full rebuild only when layout contract changed`
- Treat top-bar screens as persistent screen instances with stable widget trees and localized dirty-region updates.
- Prevent modal close from forcing expensive full-screen refresh unless the underlying data actually changed.
- Add navigation telemetry around each `ShowScreen()` and `ShowModal()` path so the screen manager itself reports timing budgets by screen type.
- Build a screen policy table:
  - `always cached`
  - `pooled`
  - `rebuild only on version bump`
  - `safe to destroy`

### Target Budgets

- `FE-01`: `<= 300 ms`
- `FE-02`: `<= 150 ms`
- `FE-03`: `<= 300 ms`
- `FE-04`: `<= 50 ms`
- `FE-05`: `<= 50 ms`

### Validation Scenarios

- `FE-01`, `FE-02`, `FE-03`, `FE-04`, `FE-05`
- Compare current `PowerUp`, `Settings`, `Achievements`, and `Account Status` behavior under cold and warm runs.
- Capture whether modal close/open chains trigger extra rebuilds on the same input sequence.

## Area 2: UI Asset Loading and Brush/Texture Policy

**Priority: `P0`**

### Current Situation

The UI asset path is partially modernized but not strict enough yet. The project has `UT66UITexturePoolSubsystem` and async brush binding patterns, but important screens still rely on synchronous texture loads or loose runtime file imports as fallback behavior. `UT66ShopScreen` is the clearest example because `PowerUp` is both image-heavy and player-facing.

### Responsiveness / Jank Risks

- `PowerUp` can feel slow if it resolves or imports multiple textures during open.
- Hero-select and main-menu screens still contain synchronous texture-pool fallback paths.
- Runtime loose-file import fallback is especially dangerous on cold loads, slow disks, and packaged builds with content drift.
- Brush ownership reset and rebuild behavior increase open-path work even when the underlying art is unchanged.

### Structural / Gold-Standard Risks

- UI asset policy currently allows "we can import from loose files if packaged assets are missing", which is a resilience feature but not a gold-standard runtime contract.
- There is no strict separation between `editor/content recovery path` and `shipping runtime path`.
- The project has async building blocks but not a hard guarantee that user-facing screens will only do async-safe work during open.

### Evidence Anchors

- `Source/T66/UI/Screens/T66ShopScreen.cpp`
  - `BuildSlateUI()`
  - `OwnedBrushes` reset
  - `MakeShopImageBrush` candidate-path logic
  - `RefreshScreen_Implementation()` -> `ForceRebuildSlate()`
- `Source/T66/UI/Style/T66RuntimeUITextureAccess.cpp`
  - `BuildLooseTextureCandidatePaths()`
  - `LoadAssetTexture()`
  - `ImportFileTexture()`
  - `ImportFileTextureWithGeneratedMips()`
- `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp`
  - `EnsureTexturesLoadedSync()`
- `Source/T66/UI/Screens/T66MainMenuScreen.cpp`
  - `EnsureTexturesLoadedSync()`
  - loose icon import fallback
- `Source/T66/UI/T66VendorOverlayWidget.cpp`
  - synchronous texture ensure path

### Proposed Solutions

- Establish a hard runtime policy:
  - no loose-file texture import on user-facing shipping paths
  - no sync texture loads during screen open for top-level screens and common modals
- Keep the current fallback utilities for editor/debug recovery only, behind explicit non-shipping or diagnostics gates.
- Convert `PowerUp`, hero select, main menu, and other high-traffic screens to:
  - predeclared asset manifests
  - async prewarm on frontend entry
  - placeholder-first render with late brush fill only if needed
- Stop full `OwnedBrushes` reset on ordinary state refresh when asset identity did not change.
- Add per-screen "asset warm completeness" logging so it is obvious when a warm-open path still falls back to sync work.

### Target Budgets

- `FE-01`: `<= 300 ms`
- `FE-02`: `<= 150 ms`
- `FE-03`: `<= 300 ms`
- `FE-05`: `<= 50 ms`
- `FE-06`: no first-use sync asset hitch on confirm/travel path

### Validation Scenarios

- `FE-01`, `FE-02`, `FE-03`, `FE-06`
- Cold and warm `PowerUp` opens with asset cache cleared vs warmed.
- Hero-select open, character portrait change, and gameplay confirm under log capture for sync fallback detection.

## Area 3: Multiplayer Invite, Join, and Steam Session Flow

**Priority: `P0`**

### Current Situation

The current invite system is functional and already instrumented better than many projects, but the happy path is still too multi-step. `UT66BackendSubsystem` polls pending invites, posts accept/reject decisions, and records diagnostics. `UT66SessionSubsystem` then manages session bootstrap, direct-join attempts, friend-session lookup, retries, and travel.

The main problem is not that the backend routes are heavy. It is that the guest-experience latency is spread across too many client-side stages, and at least one fixed retry path still waits one second.

### Responsiveness / Jank Risks

- Pending invite polling is throttled at `0.75 s` on the normal path.
- Accept flow rebuilds modal state before and during backend/session work.
- Join flow may destroy an existing session before completing the new join path.
- Healthy-path joins can still fall into retry machinery meant for fallback cases.
- A fixed `1.0 s` retry is fatal to the user's "why doesn't this feel instant like Dota?" complaint.

### Structural / Gold-Standard Risks

- The flow is not centered on one authoritative invite acceptance payload that immediately enables joining.
- Fallback behavior is too entangled with healthy-path behavior.
- Session destroy and new-session join are not clearly modeled as a fast handoff state machine with hard deadlines.

### Evidence Anchors

- `Source/T66/Core/T66BackendSubsystem.cpp`
  - `PollPendingPartyInvites()` at the throttled poll path
  - `RespondToPartyInvite()`
  - `SaveMultiplayerDiagnosticLocally()`
- `Source/T66/UI/Screens/T66PartyInviteModal.cpp`
  - accept handling
  - UI rebuilds around accept state changes
  - session-join kickoff after backend response
- `Source/T66/Core/T66SessionSubsystem.cpp`
  - `JoinPartySessionByLobbyId()`
  - `SchedulePendingFriendJoinRetry()`
  - `HandleFindFriendSessionComplete()`
  - `HandleJoinSessionComplete()`
  - `StartGameplayTravel()`
  - `PendingDirectJoinConnectString` handoff logic
- `MASTER DOCS/MASTER_STEAMWORKS.md`
  - confirms current real-AppID multiplayer path is active and important enough to optimize, not redesign from scratch

### Proposed Solutions

- Define a strict healthy join path with these rules:
  - accept returns all data needed for immediate join when available
  - no fixed retry timer on healthy conditions
  - no friend-session lookup if direct lobby join/connect data is already authoritative
- Separate fallback join logic from happy-path join logic in code and diagnostics.
- Replace generic polling-feel with faster eventing where feasible:
  - immediate local refresh after send/accept
  - optional push-style refresh or short burst polling around active invite interactions
- Make active-session teardown conditional and deadline-driven rather than open-ended.
- Emit a single structured join timeline:
  - invite accepted
  - backend responded
  - join started
  - session discovered
  - travel started
  - lobby joined

### Target Budgets

- `MP-01`: `<= 500 ms`
- `MP-02`: actionable modal visibility with no dead polling gap during active invite interactions
- `MP-03`: `<= 1000 ms` happy path
- `MP-04`, `MP-05`, `MP-06`: deterministic fallback with explicit timeline and no hidden one-second stalls unless an external platform operation truly requires it

### Validation Scenarios

- `MP-01`, `MP-02`, `MP-03`, `MP-04`, `MP-05`, `MP-06`
- Run healthy network, degraded network, host-already-in-party, guest-already-in-party, and stale-invite cases.
- Measure backend route time, client response time, session lookup time, retry time, and travel completion separately.

## Area 4: Backend / API Responsiveness

**Priority: `P1`**

### Current Situation

The backend invite routes are comparatively thin. `send`, `pending`, and `respond` mostly authenticate, read/write invite state, and return lightweight results. `party-invites.ts` uses KV-backed storage, TTL, and simple queue limits. This is good news because it means the backend is unlikely to be the dominant cause of the multi-second join feel by itself.

### Responsiveness / Jank Risks

- Repeated polling plus per-request auth/ticket overhead can still accumulate noticeable delay.
- A thin route can still feel slow if the client waits for it serially before starting the next step.
- Diagnostics upload should never compete with the healthy path or block UI state changes.

### Structural / Gold-Standard Risks

- The API contract still looks request-by-request rather than journey-based.
- The accept response likely does not collapse enough of the next join step into a single authoritative payload.
- Healthy-path latency budgets are not yet enforced at the API boundary.

### Evidence Anchors

- `C:\UE\Backend\src\app\api\party-invite\send\route.ts`
- `C:\UE\Backend\src\app\api\party-invite\pending\route.ts`
- `C:\UE\Backend\src\app\api\party-invite\respond\route.ts`
- `C:\UE\Backend\src\lib\party-invites.ts`
- `MASTER DOCS/MASTER_BACKEND.md`

### Proposed Solutions

- Promote the API contract from endpoint-level success to flow-level success:
  - accept response should include the best immediately joinable host/session payload available
  - clients should not need avoidable follow-up discovery calls on the happy path
- Add server timing fields to invite responses so backend and client time can be separated cleanly in diagnostics.
- Keep diagnostics upload async and decoupled from the user-facing path.
- Create explicit SLOs for:
  - invite send
  - pending lookup
  - invite respond
  - diagnostics accept

### Target Budgets

- `MP-01` backend contribution should be comfortably inside the `<= 500 ms` total budget
- `MP-03` backend contribution should not be the reason the flow misses `<= 1000 ms`

### Validation Scenarios

- `MP-01`, `MP-03`
- Compare route timing against end-to-end timing to confirm where latency is actually being spent.
- Capture auth/ticket overhead separately from route handler time.

## Area 5: Gameplay Travel and Load-Time Readiness

**Priority: `P0`**

### Current Situation

`UT66GameInstance` already does meaningful preload work:

- `PrimeHeroSelectionAssetsAsync()`
- `PrimeHeroSelectionPreviewVisualsAsync()`
- `PreloadGameplayAssets()`
- `HandleGameplayAssetsPreloaded()`
- `TransitionToGameplayLevel()`

This is the right direction. The problem is that some of the most important "preload" work still resolves through synchronous fallbacks. `UT66CharacterVisualSubsystem::PreloadCharacterVisual()` simply calls `ResolveVisual()`, and `ResolveVisual()` still performs synchronous object loading. Several data-table getters and first-use gameplay VFX/SFX paths also still use `LoadSynchronous()`.

### Responsiveness / Jank Risks

- Hero select -> gameplay can still hitch after "preload completed" because the preload contract is not actually complete.
- First-use VFX/SFX loads can steal time from the first seconds of gameplay.
- Travel may still feel longer or jerkier than necessary because some blocking work happens after the player believes loading is already handled.

### Structural / Gold-Standard Risks

- The project has async preload orchestration without a strict "nothing essential can still sync load after ready" rule.
- Soft-reference fallbacks in global getters create too many places where synchronous loading can leak back in.
- Readiness is not clearly divided into:
  - metadata ready
  - visuals ready
  - VFX/SFX ready
  - gameplay-ready

### Evidence Anchors

- `Source/T66/Core/T66GameInstance.cpp`
  - `PrimeHeroSelectionAssetsAsync()`
  - `PrimeHeroSelectionPreviewVisualsAsync()`
  - `PreloadGameplayAssets()`
  - `HandleGameplayAssetsPreloaded()`
  - `TransitionToGameplayLevel()`
  - multiple `LoadSynchronous()` data-table fallback points
- `Source/T66/Core/T66CharacterVisualSubsystem.cpp`
  - `ResolveVisual()`
  - `PreloadCharacterVisual()`
  - sync visual-load logging
- `Source/T66/Gameplay/T66PlayerController.cpp`
  - cached Niagara loads
- `Source/T66/Gameplay/T66CombatVFX.cpp`
  - Niagara sync loads
- `Source/T66/Gameplay/T66CombatComponent.cpp`
  - shot SFX sync load

### Proposed Solutions

- Replace `PreloadCharacterVisual()` with a truly async-first visual warm path and a distinct "resolve already loaded visual" path.
- Move critical gameplay VFX/SFX warmup into the preload contract instead of first-use code paths.
- Convert high-value data-table accessors from ad hoc sync fallback to startup/cache initialization with explicit readiness errors.
- Introduce a gameplay readiness barrier that is only released when mandatory assets for the selected hero/party/run are warm.
- Keep travel-blocking work minimal and explicit; anything not required for first-playable state should be deferred.

### Target Budgets

- `FE-06`: no avoidable post-confirm sync hitch
- `GP-01`: stage becomes playable without repeated spikes above platform budget
- `CT-01`: first-use combat feedback should not incur loading stalls

### Validation Scenarios

- `FE-06`, `GP-01`, `GP-02`, `CT-01`
- Record `[LOAD]` logs before and after remediation.
- Verify no sync visual/audio loads occur after "gameplay ready" for common opening encounters.

## Area 6: Combat Responsiveness

**Priority: `P1`**

### Current Situation

The combat design documents already emphasize clarity, responsiveness, and readability. The codebase reflects some of that intention, but several first-use and presentation paths still risk undermining it. Combat feedback only feels sharp if hit confirmation, VFX, SFX, target selection, and animation state all appear without hidden stalls.

### Responsiveness / Jank Risks

- First-use combat VFX and SFX still have synchronous load exposure.
- Any target-resolution or combat heartbeat work that is not warmed or cached can show up as first-hit sluggishness.
- Scoped/manual targeting paths will feel inconsistent if they share heavy setup with generic first-use code.

### Structural / Gold-Standard Risks

- Combat-readiness is not yet enforced as a first-class preload domain the way it should be for a feel-critical system.
- "Responsive combat" is at risk of being treated as mostly design tuning rather than asset/runtime readiness plus deterministic presentation latency.

### Evidence Anchors

- `MASTER DOCS/MASTER_COMBAT.md`
  - combat responsiveness and readability are explicit project goals
- `Source/T66/Gameplay/T66CombatComponent.cpp`
  - synchronous shot SFX load
- `Source/T66/Gameplay/T66CombatVFX.cpp`
  - synchronous Niagara loads
- `Source/T66/Gameplay/T66PlayerController.cpp`
  - cached jump/pixel VFX sync loads
- `Source/T66/Core/T66CharacterVisualSubsystem.cpp`
  - visual resolution costs that can leak into combat presentation

### Proposed Solutions

- Add a combat-warmup bundle for:
  - common weapon SFX
  - common hit VFX
  - selected hero/companion presentation assets
- Define a measured combat responsiveness target for `CT-01`.
- Instrument the entire path from attack decision to first visual/audio confirmation.
- Keep targeting and hit-feedback code on cached data and prewarmed assets only during the first minute of a run.

### Target Budgets

- `CT-01`: `<= 50 ms` from attack decision/input to first visible or audible feedback on the healthy path
- No first-hit loading stall in opening combat sequences

### Validation Scenarios

- `CT-01`, `GP-02`, `GP-04`
- Capture first attack after map load, first special effect, first boss intro, and sustained combat under Insights and lag-tracker summaries.

## Area 7: Actor Lifecycle, Tick, Pooling, and World-Query Hygiene

**Priority: `P0`**

### Current Situation

The main runtime has already adopted some better patterns. The project catalogue explicitly documents `UT66ActorRegistrySubsystem` as the preferred way to avoid expensive world scans, and parts of the code already reflect that direction. The problem is that this discipline is not yet universal, especially in Mini.

### Responsiveness / Jank Risks

- Mini still contains multiple `TActorIterator` scans on active runtime paths.
- Hidden or always-ticking presentation components consume budget even when not visibly contributing.
- Repeated material parameter writes and avoidable per-frame work increase frame-time variance rather than average cost alone.

### Structural / Gold-Standard Risks

- The project currently has mixed architecture:
  - some systems are event/registry-driven
  - others still rely on frame-time scanning
- Mini lacks parity with the main runtime's better lookup approach.
- Pooling and lifecycle standards are not yet consistent across gameplay actors, VFX actors, widgets, and interaction helpers.

### Evidence Anchors

- `MASTER DOCS/T66_MASTER_GUIDELINES.md`
  - event-driven over per-frame work
- `MASTER DOCS/T66_PROJECT_CATALOGUE.md`
  - `T66ActorRegistrySubsystem` documented to avoid expensive scans
- Main runtime:
  - `Source/T66/Gameplay/T66EnemyBase.cpp`
  - `Source/T66/Gameplay/T66MiasmaManager.cpp`
  - `Source/T66/Gameplay/T66LavaPatch.cpp`
- Mini runtime:
  - `Source/T66Mini/Private/Gameplay/T66MiniEnemyBase.cpp`
  - `Source/T66Mini/Private/Gameplay/T66MiniPickup.cpp`
  - `Source/T66Mini/Private/Gameplay/T66MiniInteractable.cpp`
  - `Source/T66Mini/Private/Gameplay/T66MiniHazardTrap.cpp`
  - `Source/T66Mini/Private/Gameplay/T66MiniProjectile.cpp`
  - `Source/T66Mini/Private/UI/T66MiniBattleHUD.cpp`
  - `Source/T66Mini/Private/Gameplay/Components/T66MiniHitFlashComponent.cpp`

### Proposed Solutions

- Make registry/overlap/listener patterns the default across both main and Mini runtime.
- Remove per-tick `TActorIterator` usage from gameplay-critical paths first.
- Add tick governance:
  - disabled by default
  - event-enabled only while active
  - auto-disable after short-lived effects complete
- Audit hidden widget components and world-space UI allocation counts; pool or collapse where possible.
- Standardize a pooling policy for ephemeral actors and presentation helpers.

### Target Budgets

- `GP-02`, `GP-03`, `MN-02`: no repeated spikes above platform hitch budget from avoidable scan/tick churn
- Stable `60 FPS` without avoidable oscillation on high-enemy and Mini gameplay scenarios

### Validation Scenarios

- `GP-02`, `GP-03`, `MN-02`
- Compare actor-lookup counts, tick counts, and hitch summaries before and after registry/pooling work.

## Area 8: World Generation, Stage Assembly, and Spawn Pacing

**Priority: `P1`**

### Current Situation

The map and stage systems already have useful `[LOAD]` logs around terrain and structure preparation, which is a strong foundation. At the same time, parts of terrain/theme setup still perform object loads in runtime code, and stage assembly likely still includes bursty work that can make startup feel heavier than necessary.

### Responsiveness / Jank Risks

- Stage entry can hitch if terrain/theme assets are resolved during active gameplay startup rather than a true pre-stage step.
- Interactable and structure spawn bursts can create visible startup stutter even if average load time seems acceptable.
- Miasma/spawn pacing logic can add steady-state variance if visual/state updates happen every frame without change detection.

### Structural / Gold-Standard Risks

- World generation and stage assembly do not yet appear to follow a strict phased bootstrap contract:
  - content resolve
  - actor spawn
  - presentation stabilize
  - gameplay release
- Runtime `LoadObject` usage in terrain/theme paths is a sign that asset readiness is still too distributed.

### Evidence Anchors

- `MASTER DOCS/MASTER_MAP_DESIGN.md`
- `Source/T66/Gameplay/T66MainMapTerrain.cpp`
  - runtime object/material/theme load paths
- `Source/T66/Core/T66GameInstance.cpp`
  - stage-related preload sequencing
- `[LOAD]` logs in runtime:
  - `SpawnMainMapTerrain finished in ... ms`
  - `SpawnStageStructuresAndInteractables finished in ... ms`
  - `PrepareMainMapStage finished in ... ms`
- `Source/T66/Gameplay/T66MiasmaManager.cpp`

### Proposed Solutions

- Pre-resolve terrain/theme/material bundles before stage release.
- Spread non-critical stage assembly across short frame budgets rather than large bursts.
- Add explicit stage-bootstrap phases with timings logged under stable markers.
- Dirty-check and coalesce world-presentation updates such as miasma/lava brightness or spawned-count maintenance.

### Target Budgets

- `GP-01`: stage start reaches stable playability inside the travel/startup budget without repeated avoidable hitches
- `GP-02`: first wave begins without residual stage-assembly spikes

### Validation Scenarios

- `GP-01`, `GP-02`
- Measure terrain prep, structure spawn, and interactable spawn separately.
- Compare packaged runs with uncached and warmed content.

## Area 9: HUD, Rendering, Materials, and VFX

**Priority: `P1`**

### Current Situation

The HUD and VFX systems already show some optimization awareness. `UT66GameplayHUDWidget` has dirty-state logic and event bindings in several places, and the project already added lag-tracker hooks to some gameplay systems. Even so, there are still avoidable frame-driven costs and rebuild-style UI updates that can hurt frame pacing.

### Responsiveness / Jank Risks

- `NativeTick` work in the gameplay HUD can accumulate from timer updates, DPS updates, map reveal work, and boss UI maintenance.
- `RefreshBossBar()` rebuilds child widgets rather than diffing/pooling.
- Material parameter writes on lava and miasma happen repeatedly.
- World-space widgets and per-actor UI components still create render and update overhead.
- Mini VFX material application patterns can create setup churn.

### Structural / Gold-Standard Risks

- Presentation systems are still too comfortable doing work every frame "because it is UI/VFX", which is exactly where jank hides.
- MID and brush lifetime management are not yet standardized around reuse and dirty updates.

### Evidence Anchors

- `Source/T66/UI/T66GameplayHUDWidget.cpp`
  - `NativeTick()`
  - `RefreshSpeedRunTimers()`
  - `RefreshBossBar()`
- `Source/T66/Gameplay/T66MiasmaManager.cpp`
  - repeated `SetScalarParameterValue()`
- `Source/T66/Gameplay/T66LavaPatch.cpp`
  - repeated `SetScalarParameterValue()`
- `Source/T66/Gameplay/T66EnemyBase.cpp`
  - health/lock widget components
- `Source/T66Mini/Private/VFX/T66MiniFlipbookVFXActor.cpp`
- `Source/T66Mini/Private/VFX/T66MiniGroundTelegraphActor.cpp`
- `Source/T66Mini/Private/VFX/T66MiniVfxShared.h`

### Proposed Solutions

- Convert HUD refreshes to event-driven or interval-bounded updates wherever exact per-frame updates are unnecessary.
- Diff boss-part UI instead of clearing/rebuilding entire child trees.
- Dirty-check material parameter writes and avoid pushing identical values.
- Review world-space UI counts and visibility policy on enemies and interactables.
- Introduce pooled MIDs or standardized reusable presentation instances for Mini and main runtime.

### Target Budgets

- `GP-03`, `GP-04`, `MN-02`: no repeated avoidable hitches from HUD/material/VFX churn
- Stable `60 FPS` frame pacing under high-enemy or boss scenarios

### Validation Scenarios

- `GP-03`, `GP-04`, `MN-02`
- Capture frame breakdowns while toggling boss UI, heavy VFX, and dense enemy waves.

## Area 10: Mini Chadpocalypse Subsystem

**Priority: `P0`**

### Current Situation

Mini currently looks like the least optimized runtime slice from an architectural hygiene standpoint. It has multiple per-tick actor scans, UI/HUD lookup churn, effect-material setup churn, and always-ticking presentation helpers. It also still participates in broader frontend/save/load flows, so Mini issues can leak into the main user journey.

### Responsiveness / Jank Risks

- Mini character select and Mini gameplay wave startup are vulnerable to avoidable UI and actor-query costs.
- Battle HUD boss lookup via `TActorIterator` is the wrong pattern for a live HUD.
- Hit-flash and VFX helpers can consume tick/material work even after their visible effect is over.
- Mini can feel inconsistent relative to the main game if it lacks the same caching, pooling, and registry rules.

### Structural / Gold-Standard Risks

- Mini appears to have drifted away from main-runtime standards rather than inheriting them.
- If Mini remains the exception, every future performance pass becomes a split-brain effort.

### Evidence Anchors

- `Source/T66Mini/Private/UI/T66MiniBattleHUD.cpp`
  - runtime enemy/boss lookup
- `Source/T66Mini/Private/Gameplay/T66MiniEnemyBase.cpp`
- `Source/T66Mini/Private/Gameplay/T66MiniPickup.cpp`
- `Source/T66Mini/Private/Gameplay/T66MiniInteractable.cpp`
- `Source/T66Mini/Private/Gameplay/T66MiniHazardTrap.cpp`
- `Source/T66Mini/Private/Gameplay/T66MiniProjectile.cpp`
- `Source/T66Mini/Private/Gameplay/Components/T66MiniHitFlashComponent.cpp`
- `Source/T66/Gameplay/T66PlayerController_Frontend.cpp`
  - Mini screen class `LoadClass` resolution

### Proposed Solutions

- Give Mini the same optimization standards as the main runtime:
  - registry-based actor lookup
  - pooled/transient VFX actors
  - tick-off-by-default presentation helpers
  - cached frontend screen assets
- Audit Mini screen open paths separately, especially character select and shop.
- Eliminate per-tick boss/enemy scans in the HUD.
- Fold Mini into the same scenario catalog and performance gates as the main game.

### Target Budgets

- `MN-01`: same frontend cold/warm screen budgets as other high-traffic screens
- `MN-02`: same gameplay hitch budgets as the main runtime

### Validation Scenarios

- `MN-01`, `MN-02`
- Mini frontend open timing, first wave, high-action wave, and return-to-frontend path.

## Area 11: Save / Load and State Propagation

**Priority: `P1`**

### Current Situation

State handoff between frontend, save system, session system, and gameplay is reasonably capable, but it is spread across several systems that mutate `UT66GameInstance` and session/player state. This is powerful, but it also creates sequencing risk if responsiveness work depends on predictable readiness.

### Responsiveness / Jank Risks

- If state application, save hydration, and session travel are not clearly staged, players will experience "soft stalls" that look like slow UI or travel rather than obvious errors.
- Frontend-to-gameplay handoff can carry hidden work when save snapshots or party profiles are applied at the wrong time.

### Structural / Gold-Standard Risks

- State propagation is currently powerful but diffuse.
- There is not yet one explicit contract for:
  - frontend selections locked
  - save snapshot loaded
  - party state applied
  - gameplay state ready
- Mini and main runtime both touch related concepts, increasing drift risk.

### Evidence Anchors

- `Source/T66/Core/T66SessionSubsystem.cpp`
  - `ApplyLoadedRunToGameInstance()`
  - `ApplySavedPartyProfilesToCurrentSession()`
  - `BuildCurrentRunSaveSnapshot()`
  - `StartLoadedGameplayTravel()`
- `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp`
  - `PrepareGameInstanceForLoadedSave()`
- Mini state systems
  - `T66MiniSaveSubsystem`
  - `T66MiniFrontendStateSubsystem`
  - `T66MiniRunStateSubsystem`

### Proposed Solutions

- Define an explicit state-handoff pipeline with timing markers and readiness phases.
- Keep state application idempotent and deadline-bounded during travel-critical paths.
- Distinguish:
  - `selection state`
  - `save snapshot state`
  - `party/session state`
  - `runtime world state`
- Audit which data must be ready before travel ends and which can be deferred one or more frames without visible jank.

### Target Budgets

- `FE-06`, `FE-07`, `MP-03`: state propagation should not be the hidden cause of missed responsiveness budgets

### Validation Scenarios

- `FE-06`, `FE-07`, `MP-03`
- Save-load travel, party save restoration, and Mini/main runtime state handoff scenarios.

## Area 12: Module / Build / Code Hygiene

**Priority: `P2`**

### Current Situation

The build and codebase are in a workable state, but there are still areas where dependency surface, header fan-out, delegate choice, and by-value APIs create drag. Most of this does not directly create player-facing hitching today, but it does affect iteration speed, regression risk, and long-term maintainability of optimization work.

### Responsiveness / Jank Risks

- Most items here are indirect rather than direct runtime feel risks.
- By-value hot-path API patterns can still add avoidable copies when called frequently.
- Dynamic delegate overuse on hot runtime paths can add overhead where native delegates would be more appropriate.

### Structural / Gold-Standard Risks

- Large public dependency surfaces in `.Build.cs` files increase compile-time and coupling.
- Header-heavy utility patterns, such as broad includes in frequently included headers, increase iteration drag.
- Mixed delegate policy obscures which systems are editor/Blueprint-facing versus native hot-path runtime.

### Evidence Anchors

- `Source/T66/T66.Build.cs`
- `Source/T66Mini/T66Mini.Build.cs`
- `Source/T66/Core/T66LagTrackerSubsystem.h`
- `Source/T66/Core/T66RunStateSubsystem.cpp`
  - `GetInventory()` by-value return
- project-wide dynamic delegate usage in core headers

### Proposed Solutions

- Review `.Build.cs` public dependency surfaces and demote anything that does not need to be public.
- Reduce header fan-out in core frequently included headers.
- Use native delegates on hot paths unless Blueprint exposure is required.
- Convert hot by-value getters to `const&` or snapshot APIs where appropriate and safe.
- Treat compile-time cleanup as supportive work for faster profiling, patching, and iteration, not as a replacement for runtime feel work.

### Target Budgets

- No direct player-facing runtime target
- Success criteria are lower iteration cost, lower coupling, and fewer hidden hot-path copies

### Validation Scenarios

- Build-time tracking
- Hot-path code review
- Profiling verification where by-value or dynamic delegate changes are made later

## Area 13: Observability and Performance Tooling

**Priority: `P0`**

### Current Situation

The project already has better observability infrastructure than many teams at this stage. `UT66LagTrackerSubsystem` exposes useful CVars and dump commands. `[LOAD]` logs already exist around several preload and stage-start systems. Multiplayer diagnostics are saved locally and can be sent to backend. This is good enough to build a repeatable performance discipline now.

### Responsiveness / Jank Risks

- Existing tooling is not yet mapped tightly enough to the scenario catalog in this audit.
- Some commands and capture recipes are still under-documented, which makes it easier for optimization work to become anecdotal.
- Without consistent markers, the team can waste time arguing about impressions instead of comparing runs.

### Structural / Gold-Standard Risks

- Instrumentation exists, but the project does not yet have one canonical performance acceptance rubric per scenario.
- There is not yet a standardized "optimization sign-off" checklist for feature work before it reaches players.

### Evidence Anchors

- `Source/T66/Core/T66LagTrackerSubsystem.h`
- `Source/T66/Core/T66LagTrackerSubsystem.cpp`
  - `T66.LagTracker.Enabled`
  - `T66.LagTracker.ThresholdMs`
  - `T66.LagTracker.RecordThresholdMs`
  - `T66.LagTracker.FrameHitchThresholdMs`
  - `T66.LagTracker.RecentWindowMs`
  - `T66.LagTracker.SummaryTopN`
  - `T66.LagTracker.DumpSummary`
  - `T66.Perf.Dump`
  - `T66.LagTracker.Reset`
- `Source/T66/Core/T66GameInstance.cpp`
  - `[LOAD]` preload logs
- runtime diagnostics output
  - `Saved/Diagnostics/Multiplayer/*.json`
- `Docs/Systems/T66_Console_Commands.md`
  - currently incomplete relative to available performance commands

### Proposed Solutions

- Map every scenario in this audit to explicit logs, traces, or counters.
- Add standard markers for:
  - screen open start/end
  - invite send/receive/accept/join milestones
  - gameplay-ready release
  - first combat-ready milestone
  - stage-bootstrap phases
- Update internal docs so lag-tracker and perf commands are easy to use.
- Define an acceptance rubric for each `P0` fix:
  - budget met
  - no hidden sync fallback
  - repeatable across cold and warm runs

### Target Budgets

- Instrumentation should make it obvious when `FE-01` through `MN-02` pass or fail
- Tooling overhead itself must remain negligible during ordinary packaged playtesting

### Validation Scenarios

- All scenario IDs in this document
- Require one lightweight capture and one deep capture for every major `P0` remediation before calling it complete

## Recommended Validation Passes

### Frontend

- Cold and warm:
  - `FE-01` main menu -> `PowerUp`
  - `FE-03` main menu -> `Settings`
  - `FE-03` main menu -> `Achievements`
  - `FE-03` main menu -> `Account Status`
- `FE-06` hero select -> gameplay
- `FE-07` gameplay -> frontend

### Multiplayer

- `MP-01` invite send
- `MP-02` invite receive/modal visibility
- `MP-03` invite accept -> joined party lobby
- `MP-04` host already in party
- `MP-05` guest already in party
- `MP-06` mismatch/fallback

### Gameplay

- `GP-01` tower stage start
- `GP-02` first enemy wave
- `GP-03` high-enemy-count steady state
- `GP-04` boss encounter
- `MN-01` Mini character select
- `MN-02` Mini gameplay wave
- `CT-01` first attack and first visible hit feedback after load

## Ranked Remediation List

### Immediate Before Art

1. Refactor invite accept -> join into one healthy-path state machine with no fixed `1.0 s` retry on healthy conditions.
2. Make `PowerUp` warm-open fast by eliminating full rebuild and sync/loose asset work from the open path.
3. Convert character-visual preload from sync disguised as preload into a true async-ready contract.
4. Warm critical gameplay VFX/SFX and selected-hero presentation assets before travel completes.
5. Remove Mini per-tick `TActorIterator` usage from enemy, pickup, interactable, hazard, projectile, and battle-HUD paths.
6. Add canonical timing markers for screen opens, invite milestones, gameplay-ready release, and stage bootstrap.
7. Diff or pool boss-bar and similar HUD rebuild paths instead of clearing and rebuilding on refresh.
8. Stop repeated identical material-parameter writes where change detection is sufficient.

### Next Engineering Pass

1. Rework frontend screen lifecycle around persistent shells, warm activation, and dirty-region refresh.
2. Define strict runtime UI asset policy with no loose-file import fallback on shipping user-facing paths.
3. Convert high-value data-table and soft-reference getters away from ad hoc `LoadSynchronous()` fallbacks.
4. Establish pooled lifecycle standards for transient VFX, telegraphs, and short-lived presentation actors in both main and Mini runtime.
5. Phase stage assembly and interactable spawn work into measured bootstrap slices.
6. Formalize state-handoff readiness across save/load, party state, frontend selection, and gameplay travel.

### Later Structural Cleanup

1. Reduce build/module public dependency surface and header fan-out in core runtime modules.
2. Review dynamic delegate usage and replace with native delegates on hot paths where Blueprint exposure is not needed.
3. Normalize by-value getter and copy-heavy API patterns in hot subsystems.
4. Expand documentation for performance commands, capture recipes, and regression gates.
5. Keep Mini and main-runtime optimization standards aligned so future performance work applies once, not twice.

## Final Opinion

The project is not in bad shape. It already has several correct instincts in place:

- meaningful preload work
- actor registry usage in some runtime areas
- lag-tracker support
- multiplayer diagnostics
- solid master-doc guidance around event-driven work and avoiding unnecessary per-frame cost

The main issue is inconsistency. The codebase currently mixes modern async/event-driven patterns with older rebuild-centric, sync-fallback, and frame-scan behaviors. That inconsistency is exactly why the game can feel responsive in some paths and janky in others.

If the team completes the `Immediate Before Art` list first, the game should feel materially faster even before any art pass lands. That is the right order: fix the plumbing now, then let the art phase build on a runtime that feels snappy instead of fragile.
