# T66 Master Optimization Audit V5

As of April 17, 2026.

This is the validated `V5` master optimization audit for `T66`, `T66Mini`, Steam/session plumbing, backend invite services, and shared infrastructure. It supersedes `T66_MASTER_OPTIMIZATION_AUDIT_V4.md` as the current consolidated master.

`V5` was authored as the final audit/planning master before implementation. The addendum below records the integrated implementation state and packaged-runtime validation checkpoint reached afterward so this file remains the single optimization source of truth.

## V5 Implementation Status Addendum

As of April 17, 2026, the repo and the rebuilt staged standalone at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe` contain a large integrated implementation pass against this audit.

### Implemented From This Audit

- frontend hot-screen lifecycle improvements across `MainMenu`, `HeroSelection`, `Settings`, `PowerUp`, and other cached screen paths
- invite accept / direct-join path reductions and healthier multiplayer happy-path orchestration
- gameplay preload, character-visual readiness, and first-use combat presentation warmup improvements
- main-runtime scan, HUD, overlay, terrain, and stage-bootstrap cleanup
- Mini runtime, Mini UI, Mini projectile, and Mini startup cleanup
- packaged standalone rebuild/validation workflow wired into the optimization closeout process instead of stopping at editor validation

### Packaged Validation Closed In This Implementation Pass

- packaged frontend boot on the rebuilt staged standalone
- packaged direct `GameplayLevel` startup smoke on the rebuilt staged standalone
- packaged direct `T66MiniBattleMap` startup smoke on the rebuilt staged standalone
- packaged first-combat-frame presentation warmup for the direct gameplay path

The last concrete packaged-only issue surfaced during this pass was direct gameplay startup falling back from `NS_PixelParticle` to `VFX_Attack1`. That was closed by adding a small startup combat-presentation preload in `UT66GameInstance`, then rebuilding and restaging the standalone. After that fix, packaged gameplay startup logs showed `NS_PixelParticle` resolving successfully instead of falling back.

### Remaining Validation That Still Requires A Human / Live Environment

- real Steam-to-Steam invite accept -> joined lobby timing validation
- interactive gameplay-run smoke for actual HUD/combat feel during a live run
- interactive Mini-run smoke for real combat feel and runtime behavior during a live run

### Build / Packaging Rule Learned During Implementation

- `Scripts/StageStandaloneBuild.ps1 -ClientConfig Development -SkipCook` is acceptable only for code-only restages against unchanged cooked content.
- After changes to runtime asset references, async preload/warmup behavior, cook/staging rules, map travel targets, always-cook coverage, or other packaged-runtime asset-resolution paths, use a fresh cooked standalone build before trusting packaged validation results.
- When packaged behavior disagrees with editor behavior, packaged standalone still wins; validate and debug against the rebuilt staged standalone, not PIE convenience paths.

## V5 Change Log

- Revalidated the current `V4` master against the repo, config, and the Batch 4 UI article set targeted at T66's actual UI stack.
- Preserved the full `V4` structure and implementation guidance so no high-value findings are stranded in the earlier masters.
- Added validated refinements for:
  - global invalidation readiness and compatibility auditing for hot UI paths in `Area 1`, `Area 9`, and `Area 13`
  - hot-screen `TAttribute` / `Visibility_Lambda` reduction guidance and volatility rules for animated widgets in `Area 1`, `Area 9`, and `Area 12`
  - UI ownership/lifecycle refinement around `UT66UIManager`, existing screen-cache behavior, and CommonUI-as-option framing in `Area 1`
  - per-actor widget-component alternatives and expanded Slate/UI profiling workflow in `Area 9` and `Area 13`
  - additional official and supplementary UI references in the appendices
- Corrected the main overstatements from the Batch 4 UI article pass before merging them:
  - screen caching already exists in `UT66UIManager`, so the immediate local issue is activation-triggered rebuild, not total re-creation on every open
  - current repo evidence points much more strongly at C++ Slate attribute polling and rebuild lifecycle cost than at Blueprint property binding abuse
  - DPI scaling is already configured in `DefaultEngine.ini`; the missing pieces are player-facing scale control and systematic hardcoded-size cleanup
  - CommonUI remains a measure-first architectural option, not an automatic rewrite order

## Validation Summary for Batch 4 UI Articles

### Accepted

- Add a global-invalidation readiness audit and compatibility pass for the hottest UI paths.
- Add explicit volatile-child guidance for animated Slate/UMG subtrees when their parents are wrapped in invalidation.
- Add a hot-screen `TAttribute` / `Visibility_Lambda` reduction pass for the screens and HUD paths the player touches most.
- Add `UT66UIManager` ownership/lifecycle review, while preserving the fact that screen caching already exists.
- Add projected single-HUD / aggregate draw alternatives as a real option for dense per-actor UI such as enemy bars, lock indicators, floating text, and cooldown bars.
- Expand the tooling section with UI-specific capture and debug workflows, including Slate Insights, Widget Reflector, `Slate.ShowOverdraw`, and `Slate.ShowBatching`.

### Refined Before Inclusion

- "Instantiate once and show/hide" was kept only in refined form because `UT66UIManager` already caches screens by `ET66ScreenType`; the real local issue is that warm activation still triggers rebuild/refresh work.
- Blueprint binding concerns were softened because sampled core UI blueprint assets did not show property-binding abuse; the stronger local evidence is in C++ Slate attribute and lambda usage.
- DPI guidance was refined because the repo already uses `UIScaleRule=ShortestSide` with a configured DPI curve; the actionable additions are player-facing scale control and removal of unnecessary hardcoded literal sizing over time.
- CommonUI guidance was kept as an architectural option, not a required migration, because the current bespoke stack still works and should only be replaced if measured lifecycle/input/focus pain justifies it.

### Not Promoted to V5 as Canonical Rules

- Blanket CommonUI migration.
- Blanket Retainer Box wrapping.
- Blanket claims that Blueprint property bindings are one of the repo's current top UI hotspots.
- Blanket "turn on global invalidation and ship" guidance without a compatibility pass.

## Validation Summary for Batch 3 Stack-Targeted Articles

### Accepted

- Add a bounded `ProceduralMeshComponent` collision/bootstrap audit item for generated surface features in `T66MainMapTerrain`.
- Add projectile, telegraph, and transient-VFX pooling parity as a concrete combat and Mini runtime concern.
- Add a Niagara system-budget/scalability pass tied to the current `NS_PixelParticle` / `VFX_Attack1` usage.
- Add Mini projectile replication as its own hotspot, including a low-risk quantization audit and a future aggregation option.
- Add always-cook scope review for the large `Stylized_VFX_StPack` and `UE5RFX` directories before the art pass grows content further.

### Refined Before Inclusion

- `HISM` stays a measure-first follow-up, not a blanket migration order.
- Nanite and Virtual Shadow Maps stay an asset-class review item, not a blanket enable/disable rule for a stylized low-poly project.
- Flow-field pathfinding stays a future scalability option for maze-heavy many-units-one-goal scenarios, not an immediate replacement for the current direct-chase model.
- `RealtimeMeshComponent` stays a supplementary fallback, not a default recommendation ahead of native-path measurement.

### Not Promoted to the Master as Canonical Rules

- Blanket terrain rewrite around `ProceduralMeshComponent`.
- Blanket removal of `Stylized_VFX_StPack` or `UE5RFX` from always-cook while live runtime references still exist.
- Blanket `ISM -> HISM` migration without capture-backed need.
- Blanket Nanite policy changes without asset-class and shadow-pass captures.

## Validation Summary for the Claude V2 Review

### Accepted

- Add ISM update-discipline guidance tied to concrete repo evidence.
- Add a Steam networking/OSS guardrail so future optimization work stays at the UE session/net-driver layer instead of drifting into deprecated raw Steam networking advice.
- Add backend cold-start, concurrent invocation, and workload-placement invariants.
- Expand the appendices with the additional validated Steam, Vercel, and ISM references from the review.

### Refined Before Inclusion

- The Steam auth config gap was kept, but the implementation note was rewritten around Epic's current Steam-auth documentation rather than the review's more specific `SteamAuthHandlerComponent` / `IpNetDriver` snippet.
- The Steam networking addition was kept at the guardrail level: the repo clearly uses `SteamNetDriver` and does not call deprecated raw Steam networking APIs, but this audit does not claim more transport-layer detail than the code actually proves.
- The authoritative-write rule was preserved, but rewritten around the real split in the repo:
  - mainline `T66` run submission is backend-authoritative
  - `T66Mini` has direct client-side Steam leaderboard uploads for its own arcade subsystem
- The Vercel section was refined into serverless-concurrency and workload-placement invariants rather than a blanket "migrate routes to Edge" recommendation.
- The per-instance custom-data article was kept as supplementary reading only; current repo evidence supports a future-reading note, not an implementation item.

### Not Promoted to the Master as Canonical Rules

- Blanket claims that there is no Steam leaderboard integration anywhere in the repo.
- A direct recommendation to move invite routes to Edge without route-level runtime validation and measured need.
- Raw Steam networking API details that do not apply to the current UE OSS / `SteamNetDriver` stack.
- Per-instance custom-data work as an active item before there is a real material/shader use case.

## Scope and Source of Truth

This audit is grounded first in the current repo and backend. Existing audits and external articles are supporting inputs, not authority over the code. Where article advice conflicted with current repo reality or official Unreal guidance, the current master keeps the repo-grounded version.

Primary local inputs:

- `MASTER DOCS/README.md`
- `MASTER DOCS/T66_MASTER_GUIDELINES.md`
- `MASTER DOCS/T66_PROJECT_CATALOGUE.md`
- `MASTER DOCS/MASTER_BACKEND.md`
- `MASTER DOCS/MASTER_STEAMWORKS.md`
- `MASTER DOCS/MASTER_COMBAT.md`
- `MASTER DOCS/MASTER_MAP_DESIGN.md`
- `Audit/PERFORMANCE_AUDIT.md`
- `Audit/T66_UI_AUDIT.md`
- `Audit/audit 4.16.26/T66_Hygiene_Optimization_Audit.md`
- `Audit/T66_MASTER_OPTIMIZATION_AUDIT.md`
- `Audit/audit 4.16.26/T66_MASTER_OPTIMIZATION_AUDIT_Claude_Review.md`
- `Audit/T66_MASTER_OPTIMIZATION_AUDIT_V2.md`
- `Audit/audit 4.16.26/T66_MASTER_OPTIMIZATION_AUDIT_V2_Claude_Review.md`
- `C:\UE\Backend`

Primary external validation sources used for this master:

- Epic documentation on profiling, UMG/Slate invalidation, Slate Insights, clipping, DPI, CommonUI, ticking, RPC reliability, instanced static meshes, widget components, and incremental GC
- Epic documentation, blogs, and talks on UMG best practices, data-driven UI architecture, replication performance, and Virtual Shadow Maps
- Riot's Global Invalidation postmortem for VALORANT, used only as a production case-study reference for rollout risk and payoff
- Epic documentation on OnlineSubsystemSteam and Steam auth enablement
- Steamworks documentation on Steam networking and SDR
- Vercel documentation on Fluid Compute and function concurrency
- Unreal console/stat command reference
- Selected UI/Slate, runtime mesh, pooling, Niagara, and crowd-pathfinding articles used only when they aligned with repo evidence

Governing assumptions:

- Existing master docs and current repo behavior win when older notes conflict.
- Player-perceived responsiveness outranks ideal architecture when the two compete.
- Desktop packaged Development standalone is the primary measurement baseline; Steam-installed multiplayer runs are required for invite/join validation.
- Steam Deck and low-end friendliness remain first-class constraints, but the first pass should not invent arbitrary targets that were never captured on target hardware.
- Community articles are included only as supplementary reading unless they align with repo evidence and official Unreal guidance.

## Executive Summary

### Top Feel Killers

1. **Party invite accept -> actual joined party latency is still paying for too many client-side stages on the healthy path.**
   The backend routes are thin. The felt delay is being spent across poll cadence, accept POST, UI refresh, session bootstrap, optional destroy-before-join, optional friend-session lookup, retry logic, and travel. The fixed `1.0f` retry in `UT66SessionSubsystem::SchedulePendingFriendJoinRetry()` remains one of the clearest concrete latency offenders.
2. **`PowerUp`, top-bar destinations, and other hot frontend screens are still paying rebuild and asset cost instead of mostly paying warm activation cost.**
   `PowerUp` maps to `UT66ShopScreen`, and that screen still refreshes via `ForceRebuildSlate()`. The current frontend lifecycle activates screens by calling `OnScreenActivated()` and immediately refreshing them, so the user is paying actual widget-build, bound-attribute, and asset-resolution cost.
3. **Hero select -> gameplay has meaningful preload work, but the readiness contract is not yet fully async-safe.**
   `UT66GameInstance` queues real preload work, but `UT66CharacterVisualSubsystem::PreloadCharacterVisual()` still routes through synchronous visual resolution. Several VFX/SFX/data paths still retain `LoadSynchronous()` exposure.
4. **Steady-state jank risk is still highest wherever the game relies on frame-driven scans, repeated material writes, rebuild-heavy UI, and allocation-heavy UI/VFX patterns.**
   Mini remains the weakest runtime slice here. The main runtime is better than before, but still not clean enough in HUD/material/widget-component behavior, and the UI layer still has no measured global-invalidation policy on the hottest paths.

### Top Architectural Liabilities

- The frontend remains too rebuild-centric for hot navigation and modals.
- Hot UI still leans too hard on bound Slate attributes, `Visibility_Lambda(...)`, active timers, and bespoke controller-owned lifecycle management without one project-wide invalidation strategy.
- The UI asset contract is still too permissive about sync and fallback behavior on user-facing paths.
- Multiplayer joining is not yet expressed as one strict healthy-path state machine with explicit fallback isolation.
- Mini does not yet share the same lookup, pooling, and presentation discipline as the stronger main-runtime systems.
- The project has useful profiling hooks, but it still lacks one hard acceptance rubric tying every complaint to the same scenario, budget, and capture recipe.

### Immediate Before Art Focus

1. Remove healthy-path fixed retry behavior and extra join hops from invite accept -> joined lobby.
2. Make `PowerUp`, top-bar destinations, and other hot frontend screens cheap on warm open by eliminating avoidable rebuild and sync asset work on activation and trialing global invalidation on compatible paths.
3. Turn gameplay preload into a genuinely async-ready contract instead of async followed by synchronous fallback.
4. Remove Mini per-tick actor scans and high-churn transient VFX allocation from common gameplay loops.
5. Add stable timing markers and repeatable capture recipes for screen open, invite flow, stage bootstrap, and first-combat readiness.
6. Reduce bound Slate attribute and persistent UI rebuild cost on the highest-traffic screens and HUD paths, and decide whether dense in-world UI stays on per-actor widget components or moves toward aggregated HUD rendering.

## Canonical Scenario Catalog

Every latency claim in this document should be measured against one of these scenarios. Start and end markers are mandatory.

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
| `GP-03` | High-enemy-count steady state | Encounter enters target density | Frame pacing remains inside budget without repeated avoidable hitches |
| `GP-04` | Boss encounter | Boss spawn/intro begins | Boss phase reaches steady playable state |
| `CT-01` | Attack input -> hit feedback | Attack input or auto-attack decision occurs | Visible and audible hit feedback starts |
| `MN-01` | Mini character select open | User opens Mini character select | First fully interactive character-select frame |
| `MN-02` | Mini gameplay wave | Mini wave-start trigger fires | Mini gameplay reaches steady frame pacing |

## Standard Budget Table

These are the project-level responsiveness targets. For frame-time reasoning, `60 FPS` means roughly `16.6 ms` per frame, and `30 FPS` means roughly `33.3 ms` per frame. When profiling, check whether game thread, render thread, or GPU is the limiting stage.

| Budget Area | Target |
| --- | --- |
| Frontend screen/modal open, cold | `<= 300 ms` |
| Frontend screen/modal open, warm | `<= 150 ms` |
| Cached tab/screen switch | `<= 50 ms` |
| Click/input -> visible feedback | `<= 50 ms` |
| Invite send -> pending invite visible on healthy backend/network | `<= 500 ms` |
| Invite accept -> joined party lobby happy path | `<= 1000 ms` with no fixed `1 s` retry on the healthy path |
| Non-travel avoidable hitch budget, desktop baseline | No repeated game-thread spikes `> 16 ms` |
| Non-travel avoidable hitch budget, low-end / Steam Deck baseline | No repeated game-thread spikes `> 25 ms` |
| Packaged gameplay target | Stable `60 FPS` without avoidable `60 -> 30` oscillation from tick/query/material/VFX churn |

## Priority Matrix

- `P0 feel wins`
  Immediate player-facing responsiveness problems, hitch sources, or architectural issues that directly block responsiveness work.
- `P1 foundational fixes`
  Important infrastructure, lifecycle, and data-flow changes that materially improve responsiveness or reduce regression risk, but are not the first things the player feels.
- `P2 longer-horizon gold-standard cleanups`
  Improvements that align the project with stronger industry practice, reduce technical drag, or create future scalability headroom, but should not outrank direct feel wins.

## Canonical Evidence Format

Every optimization finding should fit this shape:

- `Scenario`
- `Subsystem / area`
- `Current behavior`
- `Repo evidence`
- `Current risk`
- `Proposed remediation`
- `Validation method`

## Measurement and Validation Standard

- Use packaged Development standalone as the primary truth for frontend and gameplay timings.
- Use a Steam-installed build for invite/join testing.
- Capture cold and warm runs separately.
- Use Unreal Insights for deep captures, but keep a repeatable lightweight pass built around:
  - `T66.LagTracker.Enabled`
  - `T66.LagTracker.DumpSummary`
  - `T66.Perf.Dump`
  - `[LOAD]` logs
  - `Saved/Diagnostics/Multiplayer/*.json`
  - official `stat` commands such as `stat unit`, `stat game`, `stat gpu`, `stat rhi`, `stat scenerendering`, `stat slate`, `stat gc`, `stat net`
  - `Dumpticks`
  - `DumpHitches`
- Require timing markers around both success and fallback paths.
- Prefer one explicit hypothesis per optimization pass; do not bundle unrelated fixes and then guess which one helped.

## Area 1: Frontend UX and Screen Lifecycle

**Priority: `P0`**

### Current Situation

The frontend screen model already has partial caching, but it is still activation-and-rebuild oriented. `UT66UIManager` caches screens by `ET66ScreenType`, yet `ShowScreen()` and `ShowModal()` still create or reuse a screen object, add it to the viewport, then immediately call `OnScreenActivated()`. `UT66ScreenBase::OnScreenActivated_Implementation()` immediately calls `RefreshScreen()`, and heavy screens such as `UT66ShopScreen` implement `RefreshScreen_Implementation()` as `ForceRebuildSlate()`.

The local issue is not "the project recreates every screen every time." The local issue is that warm opens still spend real refresh/rebuild work. That problem is amplified by the fact that `UT66UIManager` is created as a controller-owned `UObject` rather than a cross-level subsystem, and `ForceRebuildSlate()` appears roughly `65` times across main and Mini UI source.

### Responsiveness / Jank Risks

- A top-bar click can trigger full widget-tree rebuild on a heavy destination screen.
- Modal close/open can cause the underlying screen to refresh again.
- Warm navigation is not guaranteed to stay warm if activation still triggers rebuild or asset resolution.
- Some custom Slate widgets use `RegisterActiveTimer(0.f, ...)`, which adds more frame-driven UI work that must be captured explicitly.
- Frequent `TAttribute` and `Visibility_Lambda(...)` use means hot-screen values can still be polled or invalidated more often than necessary.

### Structural / Gold-Standard Risks

- The lifecycle still conflates `cold construct`, `warm activate`, `data refresh`, and `full rebuild`.
- Screen reuse exists, but not enough of the user experience is actually preserved between activations.
- The project has no explicit hot-screen invalidation policy yet, so static and animated UI are not being separated deliberately.
- The manager lifetime and focus/input stack are still bespoke; if they keep fighting future work, a more standard activatable-stack model stays on the table.
- Bound Slate attributes on hot screens keep UI logic too polling-oriented.

### Evidence Anchors

- `Source/T66/Gameplay/T66PlayerController_Frontend.cpp`
  - `NewObject<UT66UIManager>(...)`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
  - `NewObject<UT66UIManager>(...)`
- `Source/T66/UI/T66UIManager.cpp`
  - `CreateScreen()`
  - `ShowScreen()`
  - `ShowModal()`
  - `CloseModal()`
- `Source/T66/UI/T66ScreenBase.cpp`
  - `RebuildWidget()`
  - `OnScreenActivated_Implementation()`
  - `ForceRebuildSlate()`
- `Config/DefaultGame.ini`
  - `[/Script/CommonUI.CommonUISettings]`
  - no corresponding CommonUI implementation found in project source
- `Source/T66/UI/T66FrontendTopBarWidget.cpp`
  - `HandleShopClicked()`
  - `Text_Lambda(...)` usage for top-bar values
- `Source/T66/UI/Screens/T66ShopScreen.cpp`
  - `RefreshScreen_Implementation()`
  - `BuildSlateUI()`
- `Source/T66/UI/Screens/T66SettingsScreen.cpp`
  - hot-path `Text_Lambda(...)` dropdown content
- `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp`
  - heavy bound-attribute usage
- `Source/T66/UI/T66GameplayHUDWidget.cpp`
  - bound visibility and tick-driven updates
- `Source/T66/UI/ST66AnimatedBackground.cpp`
- `Source/T66/UI/ST66AnimatedBorderGlow.cpp`
- `Source/T66/UI/ST66PulsingIcon.cpp`

### Proposed Solutions

- Split the lifecycle into explicit phases:
  - `cold construct`
  - `warm activate`
  - `data refresh`
  - `full rebuild only when layout contract changed`
- Keep the existing screen cache, but stop treating cached activation as an excuse to rebuild the full widget tree.
- Treat top-bar screens as persistent shells with stable widget trees and localized dirty updates.
- Prevent modal close from refreshing the underlying screen unless the data truly changed.
- Replace bound Slate attributes on always-visible or frequently opened UI with explicit event-driven `SetText`, `SetVisibility`, and `Invalidate` calls.
- Review whether `UT66UIManager` should move to a `UGameInstanceSubsystem` or another persistent owner so cross-level UI ownership and warm state are explicit.
- Treat `UCommonActivatableWidgetStack` / CommonUI as a `P2` architecture option only if the bespoke manager continues to fight focus, input, lifecycle, or stacking work; do not rewrite just because the config exists.
- Profile and budget active-timer-driven custom widgets separately so their cost stays intentional.
- Preserve the current DPI baseline (`UIScaleRule=ShortestSide` with an existing curve), but add player-facing `ApplicationScale` control and continue literal-size cleanup as a later UI-quality pass.
- Add navigation telemetry around every `ShowScreen()` / `ShowModal()` path.

### Target Budgets

- `FE-01`: `<= 300 ms`
- `FE-02`: `<= 150 ms`
- `FE-03`: `<= 300 ms`
- `FE-04`: `<= 50 ms`
- `FE-05`: `<= 50 ms`

### Validation Scenarios

- `FE-01`, `FE-02`, `FE-03`, `FE-04`, `FE-05`
- Capture cold and warm opens for `PowerUp`, `Settings`, `Achievements`, and `Account Status`.
- Use Slate Insights and Widget Reflector at least once on `PowerUp` cold open and invite-modal open.

## Area 2: UI Asset Loading and Brush/Texture Policy

**Priority: `P0`**

### Current Situation

The UI asset path is partially modernized but not strict enough yet. The project has `UT66UITexturePoolSubsystem` and async brush-binding patterns, but important screens still retain synchronous texture loading or runtime loose-file import fallbacks. The issue is not limited to `PowerUp`; the frontend top bar, hero selection, and some overlays still tolerate runtime texture fallback behavior on user-facing paths.

### Responsiveness / Jank Risks

- `PowerUp` can resolve/import textures during open.
- Hero selection and main menu still have synchronous texture fallback paths.
- Top-bar destinations can still pay unexpected texture work during what should be a cheap warm-screen open.
- Loose-file import fallback is especially risky on cold paths and packaged builds.
- Brush reset/rebuild behavior increases work even when the art identity did not change.

### Structural / Gold-Standard Risks

- Shipping runtime paths still tolerate fallback behavior that should be limited to editor/debug recovery.
- The project has async building blocks but not a hard runtime rule that common screens must open without sync asset work.
- UI asset readiness is not surfaced as a first-class screen-open dependency.

### Evidence Anchors

- `Source/T66/UI/Screens/T66ShopScreen.cpp`
  - `OwnedBrushes` reset
  - runtime candidate-path logic
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
  - loose import fallback
- `Source/T66/UI/T66FrontendTopBarWidget.cpp`
  - runtime texture access / fallback helpers on hot frontend chrome
- `Source/T66/UI/T66VendorOverlayWidget.cpp`
  - sync texture ensure path

### Proposed Solutions

- Define a strict runtime policy:
  - no loose-file texture import on shipping user-facing paths
  - no sync texture loads during hot screen/modal activation
- Keep existing fallback helpers for editor/dev recovery only.
- Harden the frontend asset contract so `PowerUp`, top-bar destinations, `Settings`, and hero selection open from warmed manifests on the healthy path.
- Prewarm high-traffic screen asset manifests when entering the frontend and when transitioning into hero-select.
- Stop resetting/rebuilding brushes on ordinary state refresh if the brush identity has not changed.
- Log screen-open asset warm completeness so warm opens can be audited directly.

### Target Budgets

- `FE-01`: `<= 300 ms`
- `FE-02`: `<= 150 ms`
- `FE-03`: `<= 300 ms`
- `FE-05`: `<= 50 ms`
- `FE-06`: no first-confirm sync asset hitch

### Validation Scenarios

- `FE-01`, `FE-02`, `FE-03`, `FE-06`
- Compare cold vs warm opens with sync-load logging enabled and note whether top-bar opens remain asset-clean on the warm path.

## Area 3: Multiplayer Invite, Join, and Steam Session Flow

**Priority: `P0`**

### Current Situation

The invite system is functional and already better instrumented than many games at this stage, but the happy path is still too multi-step. `UT66BackendSubsystem` polls invites, responds to accept/reject, and records diagnostics. `UT66SessionSubsystem` handles join orchestration, direct-join attempts, retry logic, and travel. At the transport/config layer the project is already on UE's Steam net-driver path in `Config/DefaultEngine.ini`, which is the right baseline, but connection-auth and future relay policy are not yet explicitly codified.

### Responsiveness / Jank Risks

- Pending invite polling is throttled at `0.75 s` on the normal path.
- Accept flow rebuilds modal state around the backend response path.
- Join flow may destroy an active party session before joining the new one.
- Healthy-path joins can still fall into retry machinery.
- A fixed `1.0 s` retry is still present and unacceptable for the healthy path.
- Healthy-path join work is still vulnerable to wasted investigation time if future optimization passes chase raw Steam networking advice that does not apply to the current stack.

### Structural / Gold-Standard Risks

- The flow is not centered on one authoritative accept payload that immediately enables joining.
- Fallback join logic is too entangled with normal join logic.
- Session teardown and rejoin are not modeled as one tight, deadline-bounded state machine.
- The supported Steam/OSS networking stack is not stated explicitly enough, which leaves room for future work to drift toward deprecated or irrelevant raw Steam API guidance.
- Steam auth packet-handler enablement is missing from config today.
- Relay/SDR policy is not yet documented as a future launch-readiness item distinct from the current invite/join latency problem.

### Evidence Anchors

- `Source/T66/Core/T66BackendSubsystem.cpp`
  - `PollPendingPartyInvites()`
  - `RespondToPartyInvite()`
  - `SaveMultiplayerDiagnosticLocally()`
- `Source/T66/UI/Screens/T66PartyInviteModal.cpp`
  - accept flow
  - UI rebuilds around accept state changes
- `Source/T66/Core/T66SessionSubsystem.cpp`
  - `JoinPartySessionByLobbyId()`
  - `SchedulePendingFriendJoinRetry()`
  - `HandleFindFriendSessionComplete()`
  - `HandleJoinSessionComplete()`
  - `PendingDirectJoinConnectString`
- `Config/DefaultEngine.ini`
  - `SteamNetDriver` definition
  - no Steam auth packet-handler entry present
- `MASTER DOCS/MASTER_STEAMWORKS.md`

### Proposed Solutions

- Define one strict healthy path:
  - accept response returns the best immediately joinable session/lobby payload available
  - no fixed retry on healthy conditions
  - no friend-session lookup when direct join data is already authoritative
- Separate fallback join logic from healthy-path join logic in code and diagnostics.
- Replace generic polling feel with interaction-aware refresh behavior where feasible.
- Make active-session destroy-before-join explicit, bounded, and instrumented.
- Codify the networking stack contract:
  - optimize invite/join at the UE OSS, session-subsystem, and net-driver configuration layers
  - do not add deprecated raw Steam networking API work unless the architecture intentionally leaves the current UE path
- Before public multiplayer, enable Steam auth packet handling per the current Unreal Steam-auth documentation and validate it in packaged logs/diagnostics.
- Treat explicit Steam relay / SDR evaluation as `P2` launch-readiness work, not as a current fix for the invite accept -> joined lobby delay.
- Emit one structured timeline for invite/join:
  - invite accepted
  - backend responded
  - join started
  - session discovered
  - travel started
  - lobby joined

### Target Budgets

- `MP-01`: `<= 500 ms`
- `MP-02`: invite becomes actionable without dead polling gaps during active interactions
- `MP-03`: `<= 1000 ms` happy path
- `MP-04`, `MP-05`, `MP-06`: deterministic fallback with no hidden dead second

### Validation Scenarios

- `MP-01`, `MP-02`, `MP-03`, `MP-04`, `MP-05`, `MP-06`
- Measure backend route time, client response time, session discovery time, retry time, and travel separately.

## Area 4: Backend / API Responsiveness

**Priority: `P1`**

### Current Situation

The backend invite routes are comparatively thin. They authenticate, read/write invite state, and return lightweight responses. That is good news, because it means the multi-second feel problem is not primarily a heavy route-handler problem. The backend also already fits the request/response shape that Vercel is good at, and the repo currently keeps runtime configuration simple: no per-route `maxDuration` overrides were found, and `/api/health` is the only route that explicitly declares `runtime = "edge"`. The concurrency, cold-start, and trust-boundary rules still need to be written down so they stay true as the backend grows.

### Responsiveness / Jank Risks

- Poll cadence and auth overhead still add latency if the client waits serially at each step.
- Diagnostics should never compete with the user-facing path.
- A fast route can still feel slow if the client blocks on it before doing the next thing.
- `/api/party-invite/respond` is disproportionately exposed to cold-start vs warm behavior because it is often the first meaningful backend hit in the guest's session.

### Structural / Gold-Standard Risks

- The API contract still looks endpoint-shaped instead of flow-shaped.
- Accept does not appear to collapse enough of the next join stage into one payload.
- End-to-end latency SLOs are not explicit.
- Concurrent serverless execution makes module-scope mutable state a correctness risk even when routes stay thin.
- The repo contains two leaderboard architectures with different trust boundaries:
  - mainline `T66` run submission is backend-authoritative
  - `T66Mini` uses direct Steam leaderboard uploads in its own subsystem
- Workload-placement constraints are not yet codified, which makes it too easy to design worker-like or long-lived features into Vercel routes by accident.

### Evidence Anchors

- `C:\UE\Backend\src\app\api\party-invite\send\route.ts`
- `C:\UE\Backend\src\app\api\party-invite\pending\route.ts`
- `C:\UE\Backend\src\app\api\party-invite\respond\route.ts`
- `C:\UE\Backend\src\app\api\submit-run\route.ts`
- `C:\UE\Backend\src\app\api\health\route.ts`
- `C:\UE\Backend\src\lib\party-invites.ts`
- `C:\UE\Backend\src\lib\anti-cheat.ts`
- `C:\UE\Backend\src\lib\kv.ts`
- `C:\UE\Backend\src\lib\steam.ts`
- `C:\UE\Backend\src\lib\steam-profile.ts`
- `C:\UE\Backend\vercel.json`
- `Source/T66Mini/Private/Core/T66MiniLeaderboardSubsystem.cpp`
- `MASTER DOCS/MASTER_BACKEND.md`

### Proposed Solutions

- Promote the API contract from endpoint success to flow success.
- Include flow-stage timing in invite responses and diagnostics.
- Keep diagnostics upload strictly decoupled from healthy-path UX.
- Define SLOs for invite send, pending lookup, and respond.
- Record cold vs warm timing separately for `/api/party-invite/respond` and `/api/party-invite/pending` during `MP-01` and `MP-03`.
- Treat the current runtime config as the baseline to preserve:
  - do not add per-route runtime or duration overrides casually
  - any future override needs a measured reason and a regression check on invite/join scenarios
- Treat backend code as concurrent-instance-safe:
  - module-scope mutable maps, arrays, counters, and dedupe caches are forbidden
  - safe lazy singletons like the current Upstash HTTP client pattern are acceptable
- Codify the trust boundary:
  - privileged credentials remain server-only
  - mainline authoritative run acceptance stays on the backend behind anti-cheat
  - `T66Mini`'s direct Steam leaderboard uploads are a separate arcade subsystem, not precedent for moving mainline authoritative writes to the client
- Codify workload placement:
  - Vercel is for request/response routes and cron
  - anything needing persistent connections, long-lived workers, or tighter-than-supported scheduling belongs elsewhere
- Do not move invite routes to Edge by assumption; only evaluate that route-by-route after runtime/library validation and only if captures prove it matters.

### Target Budgets

- Backend contribution must fit comfortably inside the `MP-01` and `MP-03` budgets.

### Validation Scenarios

- `MP-01`, `MP-03`
- Compare route timing vs end-to-end timing on healthy and degraded networks.

## Area 5: Gameplay Travel and Load-Time Readiness

**Priority: `P0`**

### Current Situation

`UT66GameInstance` already performs useful preload work for hero selection and gameplay travel, which is the right direction. The problem is that the readiness contract is still porous because some of the most important "preload" work still routes through synchronous fallback logic.

### Responsiveness / Jank Risks

- Hero select -> gameplay can still hitch after preload completion.
- First-use VFX/SFX loads can spill into the first seconds of gameplay.
- Travel may feel longer than necessary because some blocking work is deferred too late.

### Structural / Gold-Standard Risks

- The project does not yet enforce a strict `mandatory assets are warm before gameplay-ready` rule.
- Soft-reference getters and fallback loaders allow sync behavior to leak back in.
- Readiness is not clearly split into metadata, visuals, combat feedback, and world bootstrap.

### Evidence Anchors

- `Source/T66/Core/T66GameInstance.cpp`
  - `PrimeHeroSelectionAssetsAsync()`
  - `PrimeHeroSelectionPreviewVisualsAsync()`
  - `PreloadGameplayAssets()`
  - `HandleGameplayAssetsPreloaded()`
  - `TransitionToGameplayLevel()`
  - multiple `LoadSynchronous()` data-table fallbacks
- `Source/T66/Core/T66CharacterVisualSubsystem.cpp`
  - `ResolveVisual()`
  - `PreloadCharacterVisual()`
- `Source/T66/Gameplay/T66PlayerController.cpp`
- `Source/T66/Gameplay/T66CombatVFX.cpp`
- `Source/T66/Gameplay/T66CombatComponent.cpp`

### Proposed Solutions

- Replace `PreloadCharacterVisual()` with a genuinely async-first visual warm path.
- Move critical gameplay VFX/SFX into the preload contract.
- Convert high-value sync fallback getters into startup/cache initialization with explicit failure handling.
- Introduce a gameplay-ready barrier that only releases when mandatory selected-run assets are warm.
- Defer non-critical work out of the travel-critical path.

### Target Budgets

- `FE-06`: no avoidable post-confirm sync hitch
- `GP-01`: first-playable state without repeated spikes above platform budget
- `CT-01`: first-use combat feedback without loading stall

### Validation Scenarios

- `FE-06`, `GP-01`, `GP-02`, `CT-01`
- Record `[LOAD]` logs before and after every preload refactor.

## Area 6: Combat Responsiveness

**Priority: `P1`**

### Current Situation

The combat design goals already emphasize clarity and responsiveness. Runtime support for those goals is mixed: enemy pooling exists, but first-use asset warmup, projectile/presentation spawning policy, and feedback readiness are still not strong enough.

### Responsiveness / Jank Risks

- First-use combat VFX/SFX still retain sync-load exposure.
- Combat can feel inconsistent if target selection, hit feedback, and presentation readiness are not prewarmed together.
- Spawn-heavy projectile, telegraph, and transient-VFX bursts can still create avoidable combat-open spikes even where enemy pooling exists.
- Scoped/manual targeting flows can expose UI and asset latency more than standard play.

### Structural / Gold-Standard Risks

- Combat readiness is not yet a first-class preload domain.
- Pooling policy is inconsistent across enemies, projectiles, and presentation helpers.
- The project risks treating combat feel as mostly design tuning instead of a measured runtime latency problem.

### Evidence Anchors

- `MASTER DOCS/MASTER_COMBAT.md`
- `Source/T66/Gameplay/T66EnemyDirector.cpp`
  - pooled enemy acquire path
- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Source/T66/Gameplay/T66CombatVFX.cpp`
- `Source/T66/Gameplay/T66BossBase.cpp`
- `Source/T66/Gameplay/T66BossProjectile.cpp`
- `Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp`
- `Source/T66/Gameplay/Traps/T66WallArrowTrap.cpp`
- `Source/T66/Gameplay/T66PlayerController.cpp`
- `Source/T66/Core/T66CharacterVisualSubsystem.cpp`

### Proposed Solutions

- Create a combat-warmup bundle for common SFX, VFX, and selected hero/companion visuals.
- Instrument `CT-01` from input/attack decision to first visible or audible feedback.
- Keep opening combat on cached data and prewarmed feedback assets only.
- Extend pooling discipline beyond enemy bodies:
  - identify combat projectiles, telegraphs, and repeated transient effects that are still spawned live in hot paths
  - either pool them or prove that their spawn/destroy cost stays below budget in `CT-01`, `GP-03`, and `GP-04`
- Review whether bullet-dense or repeated combat visuals actually need full Niagara/actor behavior or whether simpler pooled meshes, flipbooks, or cheaper effect variants are enough.
- Treat first-hit feel as a required regression gate, not a nice-to-have.

### Target Budgets

- `CT-01`: `<= 50 ms` from input/attack decision to first feedback on the healthy path

### Validation Scenarios

- `CT-01`, `GP-02`, `GP-04`

## Area 7: Actor Lifecycle, Tick, Pooling, and World-Query Hygiene

**Priority: `P0`**

### Current Situation

The main runtime has already adopted some stronger patterns, especially through `UT66ActorRegistrySubsystem` and existing pool systems. The problem is that those standards are not yet universal, especially in Mini and some transient presentation helpers.

### Responsiveness / Jank Risks

- Mini still contains multiple per-tick `TActorIterator` scans.
- Some components tick continuously even when their useful work is short-lived.
- Pool usage exists, but some pooled systems still do unnecessary maintenance work on hot paths.
- Mini transient VFX still spawn fresh actors rather than reusing pooled instances.

### Structural / Gold-Standard Risks

- The project still mixes registry/event-driven patterns with frame-scan patterns.
- Tick discipline is not yet explicit enough to serve as a code-review gate.
- Pooling standards are not yet unified across runtime gameplay actors, text actors, and Mini VFX helpers.

### Evidence Anchors

- `MASTER DOCS/T66_MASTER_GUIDELINES.md`
- `MASTER DOCS/T66_PROJECT_CATALOGUE.md`
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
- Pooling:
  - `Source/T66/Core/T66EnemyPoolSubsystem.cpp`
  - `Source/T66/Core/T66FloatingCombatTextPoolSubsystem.cpp`
  - `Source/T66Mini/Private/Core/T66MiniVFXSubsystem.cpp`

### Proposed Solutions

- Make registry/overlap/listener patterns the default across both main and Mini runtime.
- Remove per-tick `TActorIterator` usage from gameplay-critical Mini paths first.
- Adopt project tick governance rules:
  - new components default to `bCanEverTick = false`
  - enable tick only while active
  - call `SetComponentTickEnabled(false)` aggressively when work completes
  - use tick groups and prerequisites deliberately, not by default
- Use `Dumpticks` and `DumpHitches` as milestone checks, not just one-off debugging tools.
- Keep `UT66EnemyPoolSubsystem` as a strong reference pattern.
- Fix obvious pool inefficiency such as `UT66FloatingCombatTextPoolSubsystem::AcquireActor()` compacting pools on every acquire.
- Add pooling for Mini pulses/telegraphs before scaling content further.

### Target Budgets

- `GP-02`, `GP-03`, `MN-02`: no repeated spikes above platform hitch budget from avoidable scan/tick churn

### Validation Scenarios

- `GP-02`, `GP-03`, `MN-02`
- Compare tick counts, scan counts, and hitch summaries before/after each fix.

## Area 8: World Generation, Stage Assembly, and Spawn Pacing

**Priority: `P1`**

### Current Situation

Stage startup is already partially instrumented through `[LOAD]` logs around terrain and structure preparation. That is a good base, but some terrain/theme setup still performs runtime object work that should be considered part of a stricter bootstrap contract. `T66MainMapTerrain` is not "all PMC," but it does build bounded generated surface-feature visuals and collision helpers through `UProceduralMeshComponent`.

### Responsiveness / Jank Risks

- Stage entry can hitch if terrain/theme assets are still being resolved during active startup.
- Interactable and structure spawn bursts can create visible startup stutter.
- Generated surface-feature collision creation can become a startup hitch source if complexity or count grows without explicit limits.
- Miasma/spawn pacing logic can add steady-state variance when updates happen every frame without change detection.

### Structural / Gold-Standard Risks

- World generation is not yet clearly phased into resolve, spawn, presentation stabilize, and release.
- Runtime object/material work remains too distributed.
- The project does not yet state when bounded runtime procedural geometry should stay PMC versus being handed off to a static-mesh or simplified-collision path once generation settles.

### Evidence Anchors

- `MASTER DOCS/MASTER_MAP_DESIGN.md`
- `Source/T66/Gameplay/T66MainMapTerrain.cpp`
  - bounded surface-feature count
  - generated visual and collision `UProceduralMeshComponent` children
  - `bUseAsyncCooking = true`
  - `bUseComplexAsSimpleCollision = true`
- `Source/T66/Core/T66GameInstance.cpp`
- `[LOAD]` logs in runtime:
  - `SpawnMainMapTerrain finished in ... ms`
  - `SpawnStageStructuresAndInteractables finished in ... ms`
  - `PrepareMainMapStage finished in ... ms`
- `Source/T66/Gameplay/T66MiasmaManager.cpp`

### Proposed Solutions

- Pre-resolve terrain/theme/material bundles before stage release.
- Spread non-critical stage assembly across short frame budgets.
- Log stage-bootstrap phases under stable names.
- Add explicit timing around terrain surface-feature generation and collision-enable phases so `GP-01` can prove whether the bounded PMC path matters.
- Treat the current PMC path as acceptable only while it stays inside budget. If it shows up in `GP-01`, fix it in this order:
  - reduce feature count, collision complexity, or collision scope
  - move settled generated geometry to a runtime-built static-mesh or equivalent handoff path
  - only evaluate third-party runtime-mesh replacements if native-path options still fail the capture
- Dirty-check and coalesce world-presentation updates such as brightness or spawn-count maintenance.

### Target Budgets

- `GP-01`: stage start reaches stable playability without repeated avoidable hitches
- `GP-02`: first wave begins without residual stage-bootstrap spikes

### Validation Scenarios

- `GP-01`, `GP-02`

## Area 9: HUD, Rendering, Materials, and VFX

**Priority: `P1`**

### Current Situation

The HUD and VFX systems already show optimization awareness, but there is still too much frame-driven presentation work and too little explicit UI invalidation policy or Niagara budgeting. `UT66GameplayHUDWidget` is now large enough to be its own risk surface, at roughly `7,150` lines, and the project still has no explicit `Slate.EnableGlobalInvalidation` policy in config. HUD tick, bound Slate attributes, repeated material parameter writes, per-enemy widget components, sync VFX warmup, and transient Mini VFX actors are all contributing factors.

### Responsiveness / Jank Risks

- Gameplay HUD tick can accumulate timer, DPS, map, and boss UI cost.
- Boss UI rebuilds child content instead of diffing or reusing.
- Without explicit invalidation strategy, static HUD shells and animated subtrees can both redraw more often than they should.
- Material parameter writes are repeated even when values may not have changed.
- Repeated Niagara system spawns and translucent-particle cost can create spikes that get misdiagnosed as "general combat lag."
- Enemy widget components add per-actor UI cost and still rely on `EWidgetSpace::Screen` plus `SetDrawAtDesiredSize(true)`.
- Hero cooldown bars and floating combat text add more per-actor UI cost on top of enemy widgets.
- Mini VFX currently allocates actors for pulses and telegraphs at runtime.

### Structural / Gold-Standard Risks

- UI and VFX are still too comfortable doing work every frame "because it's presentation."
- Bound attribute usage is widespread enough that UI cost can become invisible until it spikes.
- Widget-component policy is not explicit enough for density-heavy encounters.
- The project has custom Slate capability already, but it is not yet being used to replace the densest per-actor UI patterns.
- Retainers vs invalidation vs volatility rules are not yet codified, so future fixes can easily make redraw behavior worse instead of better.
- The project does not yet have one explicit Niagara rule set for system count, emitter count, scalability, or significance on hot combat paths.

### Evidence Anchors

- `Config/DefaultEngine.ini`
  - no `Slate.EnableGlobalInvalidation` entry present
- `Source/T66/UI/T66GameplayHUDWidget.cpp`
  - `NativeTick()`
  - `RefreshSpeedRunTimers()`
  - `RefreshBossBar()`
  - scoped UI text `FString::Printf(...)`
  - widespread `.Visibility_Lambda(...)`
- `Source/T66/UI/ST66AnimatedBackground.cpp`
  - `RegisterActiveTimer(0.f, ...)`
  - paint invalidation every timer tick
- `Source/T66/UI/ST66AnimatedBorderGlow.cpp`
  - `RegisterActiveTimer(0.f, ...)`
  - paint invalidation every timer tick
- `Source/T66/UI/ST66PulsingIcon.cpp`
  - `RegisterActiveTimer(0.f, ...)`
  - paint invalidation every timer tick
- `Source/T66/Gameplay/T66CombatComponent.cpp`
  - common combat Niagara soft references
- `Source/T66/Gameplay/T66CombatVFX.cpp`
  - sync Niagara resolution
  - repeated `SpawnSystemAtLocation` / `SpawnSystemAttached` paths
- `Source/T66/Gameplay/Traps/T66FloorFlameTrap.cpp`
  - direct fire-system load path
- `Source/T66/Gameplay/T66MiasmaManager.cpp`
- `Source/T66/Gameplay/T66LavaPatch.cpp`
- `Source/T66/Gameplay/T66EnemyBase.cpp`
  - `UWidgetComponent` health and lock widgets
- `Source/T66/Gameplay/T66HeroBase.cpp`
  - world-space cooldown widget component
- `Source/T66/Gameplay/T66FloatingCombatTextActor.cpp`
  - widget-component presentation path
- `Source/T66/UI/ST66RingWidget.cpp`
- `Source/T66/UI/ST66CrosshairWidget.cpp`
- `Source/T66/UI/ST66DotWidget.cpp`
- `Source/T66/UI/ST66LockDotWidget.cpp`
- `Source/T66Mini/Private/Core/T66MiniVFXSubsystem.cpp`
  - fresh actor spawn for telegraphs and pulses
- `Source/T66Mini/Private/VFX/T66MiniFlipbookVFXActor.cpp`
- `Source/T66Mini/Private/VFX/T66MiniGroundTelegraphActor.cpp`
- `Source/T66Mini/Private/VFX/T66MiniVfxShared.h`

### Proposed Solutions

- Convert HUD refreshes to event-driven or interval-bounded updates wherever possible.
- Diff boss-part UI instead of clearing/rebuilding full child sets.
- Pilot `Slate.EnableGlobalInvalidation=true` on a test branch/build and explicitly audit compatibility on top bar, `PowerUp`, `Settings`, hero selection, and gameplay HUD before treating it as a default.
- Prefer `Invalidation Box`-style caching first for static shells; use Retainer Panels only where measurements show draw-call reduction is worth the repaint cost and extra render-target memory.
- If static parents become invalidated, mark animated children volatile or otherwise exempt them from forcing full parent redraw every frame.
- Change-gate every `SetScalarParameterValue` / `SetVectorParameterValue` path that can avoid duplicate writes.
- Evaluate `UMaterialParameterCollection` for shared global values such as synchronized brightness/intensity.
- Convert high-frequency UI lambdas on always-visible HUD paths to event-driven state changes or explicit invalidation.
- Add an explicit Niagara audit pass for `GP-03`, `GP-04`, and `MN-02`:
  - `stat Niagara`
  - `stat gpu`
  - `ProfileGPU`
  - Timing Insights / GPU Visualizer where relevant
- Put high-frequency combat systems under effect-type/scalability/significance rules and justify when a hot effect should stay CPU sim, move to a simpler pooled representation, or be merged into fewer systems.
- Prefer fewer high-value active systems and pooled components over repeated one-shot spawns where the same visual pattern recurs many times per fight.
- If widget components remain, audit whether `Manually Redraw` / redraw throttling or a consolidated overlay can reduce per-enemy UI cost.
- Evaluate projected single-HUD / aggregate draw alternatives for enemy bars, lock markers, floating text, and cooldown bars before accepting per-actor widget cost as permanent. The repo already has several `SLeafWidget`-based custom widgets, so this is a feasible internal pattern rather than a hypothetical one.
- Treat custom draw-overlay solutions as `P2` architecture unless simpler wins fail first.
- Pool Mini telegraphs and pulses instead of spawning fresh actors in common loops.

### Target Budgets

- `GP-03`, `GP-04`, `MN-02`: no repeated avoidable hitches from HUD/material/VFX churn

### Validation Scenarios

- `GP-03`, `GP-04`, `MN-02`
- Capture `stat slate`, `stat scenerendering`, `stat rhi`, `stat gpu`, `Slate.ShowOverdraw`, `Slate.ShowBatching`, and Slate Insights where relevant.

## Area 10: Mini Chadpocalypse Subsystem

**Priority: `P0`**

### Current Situation

Mini is still the weakest runtime slice from a performance-hygiene perspective. It combines per-tick actor scans, replicated movement on many actor types, direct projectile actor spawning, transient VFX allocation, and HUD lookup churn.

### Responsiveness / Jank Risks

- Mini character select and gameplay wave startup are vulnerable to avoidable UI and actor-query costs.
- Battle HUD boss lookup via `TActorIterator` is too expensive for a live HUD.
- Repeated Mini projectile spawn/destroy and replicated movement can create both frame-time churn and online scaling problems.
- Hit-flash and other presentation helpers continue ticking when they should go idle.
- Fresh VFX actor spawns amplify both frame-time churn and future GC pressure.

### Structural / Gold-Standard Risks

- Mini has drifted away from the stronger patterns that exist in the main runtime.
- Mini replication and projectile policy are drifting from intentional budgeting toward default engine behavior.
- If Mini remains the exception, every future optimization pass becomes duplicated work.

### Evidence Anchors

- `Source/T66Mini/Private/UI/T66MiniBattleHUD.cpp`
- `Source/T66Mini/Private/Gameplay/T66MiniEnemyBase.cpp`
- `Source/T66Mini/Private/Gameplay/T66MiniEnemyProjectile.cpp`
- `Source/T66Mini/Private/Gameplay/T66MiniPickup.cpp`
- `Source/T66Mini/Private/Gameplay/T66MiniInteractable.cpp`
- `Source/T66Mini/Private/Gameplay/T66MiniHazardTrap.cpp`
- `Source/T66Mini/Private/Gameplay/T66MiniProjectile.cpp`
- `Source/T66Mini/Private/Gameplay/T66MiniPlayerPawn.cpp`
  - direct projectile spawn paths
- `Source/T66Mini/Private/Gameplay/Components/T66MiniHitFlashComponent.cpp`
- `Source/T66Mini/Private/Core/T66MiniVFXSubsystem.cpp`
- `Source/T66/Gameplay/T66PlayerController_Frontend.cpp`
  - Mini screen class `LoadClass` resolution

### Proposed Solutions

- Bring Mini up to main-runtime standards:
  - registry-based actor lookup
  - projectile and transient-effect budgeting instead of default actor-spawn churn
  - pooled transient VFX
  - tick-off-by-default presentation helpers
  - cached frontend screens and warmed assets
- Remove per-tick HUD scan paths first.
- Audit whether Mini projectile actors truly need replicated movement and per-instance actor lifetime on the hot path, or whether part of that work can become pooled, predicted, or aggregated.
- Audit Mini screen-open cost separately from gameplay runtime cost.
- Make Mini share the same acceptance scenarios and budgets as the main game.

### Target Budgets

- `MN-01`: same cold/warm frontend budgets as other high-traffic screens
- `MN-02`: same hitch budgets as the main runtime

### Validation Scenarios

- `MN-01`, `MN-02`

## Area 11: Save / Load and State Propagation

**Priority: `P1`**

### Current Situation

State handoff between frontend, save, session, and gameplay is capable but diffuse. Several systems mutate `UT66GameInstance`, session player state, or frontend state, which is functional but sequencing-heavy.

### Responsiveness / Jank Risks

- State application can create soft stalls that look like slow UI or travel.
- Save restoration and party-state application can carry hidden work into join/travel paths.

### Structural / Gold-Standard Risks

- State propagation is still spread across multiple systems with overlapping responsibility.
- There is not yet one explicit contract for selection-lock, snapshot-ready, party-applied, and gameplay-ready.

### Evidence Anchors

- `Source/T66/Core/T66SessionSubsystem.cpp`
  - `ApplyLoadedRunToGameInstance()`
  - `ApplySavedPartyProfilesToCurrentSession()`
  - `BuildCurrentRunSaveSnapshot()`
  - `StartLoadedGameplayTravel()`
- `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp`
  - `PrepareGameInstanceForLoadedSave()`
- Mini state systems:
  - `T66MiniSaveSubsystem`
  - `T66MiniFrontendStateSubsystem`
  - `T66MiniRunStateSubsystem`

### Proposed Solutions

- Define an explicit state-handoff pipeline with timings.
- Keep state application idempotent and deadline-bounded during travel-critical paths.
- Separate selection state, save snapshot state, party/session state, and runtime world state.
- Defer anything not required for first-playable state.

### Target Budgets

- `FE-06`, `FE-07`, `MP-03`: state propagation should not be the hidden reason budgets are missed

### Validation Scenarios

- `FE-06`, `FE-07`, `MP-03`

## Area 12: Module / Build / Code Hygiene

**Priority: `P2`**

### Current Situation

The build/codebase is workable but still carries dependency-surface, API-hygiene, and Slate-hygiene debt. This is not the first thing the player feels, but it affects iteration speed and the safety of future optimization work. On the UI side, the repo leans heavily on `TAttribute` and lambda-bound state in hot widgets without a corresponding local pattern around `TSlateAttribute`, managed attributes, or explicit volatility/invalidation discipline.

### Responsiveness / Jank Risks

- Mostly indirect, but hot-path by-value APIs, dynamic delegates, and pervasive polling-style UI attributes can still add avoidable work.

### Structural / Gold-Standard Risks

- Public dependency surfaces are broader than necessary.
- Header fan-out remains high in some core areas.
- Delegate policy is not always explicit about hot path vs Blueprint-facing usage.
- The project does not yet have one clear rule for when a UI value should be event-driven, `TAttribute`-driven, or explicitly invalidated.

### Evidence Anchors

- `Source/T66/T66.Build.cs`
- `Source/T66Mini/T66Mini.Build.cs`
- `Source/T66/Core/T66LagTrackerSubsystem.h`
- `Source/T66/Core/T66RunStateSubsystem.cpp`
  - by-value `GetInventory()`
- `Source/T66/UI/T66GameplayHUDWidget.cpp`
- `Source/T66/UI/T66LeaderboardPanel.cpp`
- `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp`
- `Source/T66/UI/Screens/T66SettingsScreen.cpp`
- project-wide dynamic delegate usage in core headers
- project-wide `TAttribute` / `Visibility_Lambda(...)` usage across main and Mini UI source

### Proposed Solutions

- Review `.Build.cs` public dependency surfaces and demote unnecessary public modules.
- Reduce header fan-out in frequently included core headers.
- Use native delegates on hot paths unless Blueprint exposure is required.
- Convert hot by-value getters to safer reference/snapshot patterns where appropriate.
- Define a local Slate attribute policy:
  - event-driven updates first on hot widgets
  - reserve `TAttribute` for cold/simple cases
  - use explicit invalidation/volatility rules for animated or frequently changing values

### Target Budgets

- No direct player-facing runtime target

### Validation Scenarios

- Build-time tracking
- hot-path code review

## Area 13: Observability and Performance Tooling

**Priority: `P0`**

### Current Situation

The project already has better observability than many teams at this stage. `UT66LagTrackerSubsystem` is real, `[LOAD]` logs already exist, and multiplayer diagnostics are saved locally. The missing piece is a standardized workflow and acceptance rubric, especially for UI where there is still no canonical invalidation, redraw, or batching capture recipe.

### Responsiveness / Jank Risks

- Existing tooling is not yet mapped tightly enough to the scenario catalog.
- Some performance commands and recipes remain under-documented.
- UI complaints can still collapse into "menus feel slow" unless screen-open captures include Slate-specific tools and invalidation/debug views.
- Without a common workflow, the team can still argue from impressions.

### Structural / Gold-Standard Risks

- Instrumentation exists, but not yet one canonical sign-off standard per scenario.
- Optimization work can still "finish" without before/after proof.
- The UI layer still lacks one agreed A/B workflow for global invalidation, redraw cost, clipping/batching behavior, and active-timer cost.

### Evidence Anchors

- `Source/T66/Core/T66LagTrackerSubsystem.h`
- `Source/T66/Core/T66LagTrackerSubsystem.cpp`
- `Source/T66/Core/T66GameInstance.cpp`
  - `[LOAD]` preload logs
- `Saved/Diagnostics/Multiplayer/*.json`
- `Docs/Systems/T66_Console_Commands.md`
- repo/config search
  - no project-level `Slate.EnableGlobalInvalidation` entry
  - no explicit UI profiling workflow doc for Widget Reflector / Slate batching / overdraw found

### Proposed Solutions

- Map every scenario ID to explicit logs, traces, or counters.
- Adopt a hypothesis-driven optimization workflow:
  1. reproduce in one canonical scenario
  2. capture a baseline
  3. form one explicit hypothesis
  4. use the smallest tool that proves/disproves it
  5. fix, re-measure, compare
  6. promote the fix to a regression gate
- Recommended baseline tool set:
  - `stat unit`
  - `stat game`
  - `stat gpu`
  - `stat Niagara`
  - `ProfileGPU`
  - `stat rhi`
  - `stat scenerendering`
  - `stat slate`
  - `stat gc`
  - `stat net`
  - `Dumpticks`
  - `DumpHitches`
  - Unreal Insights
  - Slate Insights
  - Widget Reflector
  - `Slate.ShowOverdraw`
  - `Slate.ShowBatching`
  - Asset Loading Insights
- For UI work, add one explicit A/B recipe:
  1. capture hot-screen cold and warm opens on the current build
  2. capture Slate timing, batching, and overdraw
  3. trial one change such as global invalidation, attribute removal, or redraw throttling
  4. re-capture and keep only the change that wins without regressions
- Update internal docs so the team can run this workflow quickly.

### Target Budgets

- Instrumentation should make it obvious when `FE-01` through `MN-02` pass or fail

### Validation Scenarios

- All scenario IDs in this document
- Require one lightweight capture and one deep capture for every major `P0` remediation

## Area 14: Garbage Collection Discipline

**Priority: `P1`**

Promote to `P0` only if captures show repeated GC-induced spikes above the platform hitch budget.

### Current Situation

The project currently has no explicit garbage-collection tuning in `Config/DefaultEngine.ini`, while several systems still create allocation churn through UI rebuilds, transient VFX actor spawns, and frequent text/material updates.

### Responsiveness / Jank Risks

- Periodic GC spikes can masquerade as random gameplay hitching.
- High transient allocation churn in Mini VFX and rebuild-heavy UI increases GC pressure.
- If incremental reachability is enabled without safe pointer discipline, it introduces correctness risk rather than just performance risk.

### Structural / Gold-Standard Risks

- The project does not yet have an explicit allocation/ownership policy for transient gameplay and presentation objects.
- GC tuning is easy to cargo-cult; Unreal's incremental reachability requires safe object-pointer usage.
- Pooling is not yet being used as a GC-pressure reduction strategy consistently across all hot transient systems.

### Evidence Anchors

- `Config/DefaultEngine.ini`
  - no explicit GC/incremental low-memory tuning found
- `Source/T66/UI/Screens/T66ShopScreen.cpp`
  - rebuild-heavy refresh path
- `Source/T66/UI/T66GameplayHUDWidget.cpp`
  - frequent `FString::Printf(...)` / `SetText(...)` paths
- `Source/T66Mini/Private/Core/T66MiniVFXSubsystem.cpp`
  - fresh actor spawn for telegraphs and pulses
- `Source/T66/Core/T66FloatingCombatTextPoolSubsystem.cpp`
  - `CompactPools()` on every acquire
- Official Unreal incremental GC guidance
  - incremental reachability requires safe `TObjectPtr` usage

### Proposed Solutions

- Capture a baseline first using `stat gc` and Memory/Unreal Insights during long gameplay runs.
- Reduce allocation churn before tuning GC:
  - pool Mini telegraphs/pulses
  - reduce avoidable UI rebuilds
  - reduce per-frame text allocation in hot widgets
- Run a pointer-safety audit before enabling incremental reachability globally:
  - `UPROPERTY` for owned UObject references
  - `TObjectPtr` / `TWeakObjectPtr` as appropriate
- If GC spikes remain after churn reduction, then evaluate:
  - `gc.AllowIncrementalReachability`
  - `gc.AllowIncrementalGather`
  - `gc.IncrementalBeginDestroyEnabled`
  - `gc.IncrementalGCTimePerFrame`
  - `gc.LowMemory.*`
- Tune low-memory GC mode only with target-hardware captures, especially for Deck/low-end scenarios.

### Target Budgets

- No GC-induced spike `> 16 ms` on desktop baseline
- No GC-induced spike `> 25 ms` on low-end / Steam Deck baseline

### Validation Scenarios

- `GP-03`, `MN-02`, `FE-02`
- Ten-minute soak tests with periodic `stat gc` and Insights captures

## Area 15: Rendering, GPU, Collision, Instancing, and LOD Policy

**Priority: `P1`**

### Current Situation

The project is stylized/pixel-art-forward but still uses standard Unreal rendering infrastructure. The repo already uses `UInstancedStaticMeshComponent` in several places, project config already has `r.Nanite.ProjectEnabled=True` and `r.Shadow.Virtual.Enable=1`, and `T66MainMapTerrain` already mixes instancing with bounded generated `ProceduralMeshComponent` surface helpers. The immediate problem is not "rendering has no optimization" but rather that there is no explicit project-level rendering/collision/cook policy tied to gameplay scenarios. The other missing piece is update discipline: a correct decision to use instancing can still perform badly if the update path dirties render state one instance at a time.

### Responsiveness / Jank Risks

- Dense enemy waves, widget components, and translucent VFX can produce GPU spikes even when CPU work is also high.
- Bounded runtime procedural-geometry collision creation can still hitch stage bootstrap if it grows without explicit limits.
- Collision complexity in prop/terrain helpers can add unnecessary CPU cost.
- Inconsistent instancing policy can turn repeated visuals into avoidable draw and update cost.
- Batched visual systems can still hitch if they add or mutate many instances one-by-one on the game thread.
- Blanket Nanite use on low-poly assets, or VSM-heavy scenes with too many non-Nanite or WPO-heavy casters, can waste GPU budget instead of saving it.
- Large always-cook directories can inflate build/startup scope if they are not narrowed to the assets the shipped runtime truly uses.

### Structural / Gold-Standard Risks

- There is no explicit asset-review checklist for collision, shadows, instancing, or LOD readiness.
- Rendering guidance is vulnerable to "engine folklore" unless tied to captures.
- Nanite and instancing decisions could be made dogmatically instead of from measured scene cost.
- The project does not yet state a rule for when instancing work should use batched add/update APIs with one final render-state dirty.
- The project does not yet state a rule for when bounded runtime procedural geometry should be converted to a settled static representation.
- Large always-cook content directories do not yet have a documented "why is this still global?" review gate.

### Evidence Anchors

- `Config/DefaultEngine.ini`
  - `r.Nanite.ProjectEnabled=True`
  - `r.Shadow.Virtual.Enable=1`
- `Config/DefaultGame.ini`
  - `DirectoriesToAlwaysCook` entries for `/Game/Stylized_VFX_StPack`
  - `DirectoriesToAlwaysCook` entries for `/Game/UE5RFX`
- `Source/T66/Gameplay/T66MainMapTerrain.cpp`
  - `UInstancedStaticMeshComponent` usage
  - generated surface-feature `UProceduralMeshComponent` usage
  - bounded surface-feature counts
  - `UseComplexAsSimple` enable path
- `Source/T66/Gameplay/T66MiasmaManager.cpp`
  - instanced tile usage
  - one-at-a-time `AddInstance(...)` growth path
- `Source/T66/Gameplay/T66MiasmaBoundary.cpp`
  - instanced wall usage
- `Source/T66/Gameplay/Traps/T66FloorSpikePatchTrap.cpp`
  - instanced spike usage
- `Source/T66/Core/T66PropSubsystem.cpp`
  - `UseComplexAsSimple` path for meshes
- `Source/T66/Gameplay/T66CombatVFX.cpp`
  - direct references into `/Game/Stylized_VFX_StPack/...`
- `Source/T66/Gameplay/Traps/T66FloorFlameTrap.cpp`
  - direct references into `/Game/Stylized_VFX_StPack/...`
- `Source/T66/Core/T66RetroFXSubsystem.cpp`
  - direct references into `/Game/UE5RFX/...`
- `Source/T66/Gameplay/T66EnemyBase.cpp`
  - per-enemy widget components
- `Source/T66/Gameplay/T66HeroBase.cpp`
  - `SetDefaultCustomPrimitiveDataFloat(...)` placeholder-mesh usage

### Proposed Solutions

- Establish a rendering capture policy for `GP-01`, `GP-03`, `GP-04`, and `MN-02` using:
  - `stat unit`
  - `stat gpu`
  - `ProfileGPU`
  - `stat rhi`
  - `stat scenerendering`
- Create an asset-review checklist:
  - repeated static visuals justify ISM/HISM or explicit non-instanced exception
  - collision mode justified per class
  - shadow casting reviewed for small props and VFX
  - LOD presence reviewed on performance-relevant meshes
- Add an explicit ISM update-discipline rule:
  - when adding or updating many instances in one frame, prefer batched add/update APIs and suppress per-instance render-state invalidation
  - dirty render state once at the end of the batch
  - `T66MainMapTerrain` is the local good pattern; `T66MiasmaManager::EnsureSpawnedCount()` is the concrete local anti-pattern to retire
- Extend existing instancing patterns beyond terrain/traps only when captures justify it.
- Treat `HISM` as a measured follow-up, not a default migration:
  - use it when repeated visuals are spatially broad enough that hierarchical culling is likely to matter
  - do not convert by rote just because the repo currently has no `HISM`
- Measure before manually converting repeated props to ISM/HISM by rote. On UE 5.7, engine-side batching may already absorb some repeated-mesh cases, so explicit conversion should be justified by:
  - proven draw/update cost
  - burst-spawn/update behavior
  - or a collision/data-sharing reason
- Audit terrain/prop systems that force `UseComplexAsSimple`; keep that only where justified.
- Add a Nanite/VSM asset-class review:
  - low-poly static meshes
  - skeletal/non-Nanite shadow casters
  - WPO-heavy assets
  - translucent-heavy scenes
  - use captures before changing policy
- Do not prioritize blanket Nanite work until captures show a mesh-triangle or shadow-pass bottleneck. Current repo evidence points more strongly at CPU/UI/replication issues than at triangle throughput.
- Treat `T66MainMapTerrain` surface-feature PMC usage as a bounded but measurable path. If `GP-01` shows it on the critical path, fix it in order:
  - reduce feature count or collision scope
  - simplify collision representation
  - hand settled generated geometry off to a runtime-built static representation
  - only then consider third-party runtime-mesh replacements if the native path still fails budget
- Audit always-cook scope before the art pass:
  - keep globally cooked directories only when the shipped runtime truly needs whole-directory coverage
  - narrow `Stylized_VFX_StPack` and `UE5RFX` toward referenced subpaths/assets where possible
  - re-measure cold packaged boot and package size after each scope reduction
- Keep per-instance custom-data work in the supplementary/future bucket until a concrete material path justifies it. Current repo evidence shows no per-instance custom-data system, only limited default custom primitive data on hero placeholder meshes.

### Target Budgets

- Desktop baseline: GPU stays within the frame budget during `GP-03` and `GP-04`
- Low-end/Deck: define a captured platform target after baseline measurement instead of assuming one

### Validation Scenarios

- `FE-01`, `GP-01`, `GP-03`, `GP-04`, `MN-02`
- Capture `ProfileGPU`, `stat gpu`, `stat rhi`, `stat scenerendering`, and packaged cold-boot/package-size deltas before and after any asset-policy change

## Area 16: Networking, Replication, and Relevancy Policy

**Priority: `P1`**

### Current Situation

Invite/join UX is covered in `Area 3`, but runtime replication policy needs its own section. Mini already replicates movement on many actor types, including projectile actors, and several Mini gameplay classes are replicated even though the repo shows no explicit tuning for cull distance, update frequency, replication priority, quantized vector types, or aggregation helpers such as `FFastArraySerializer`.

### Responsiveness / Jank Risks

- If online Mini density rises, replicated movement on enemies, pickups, interactables, traps, projectiles, and companions can turn into both bandwidth cost and server/client frame cost.
- Projectile-heavy online Mini scenarios can become replication hot spots because the current model spawns and replicates many short-lived projectile actors directly.
- Reliable RPCs are currently used in several control-flow paths; reliable RPCs are appropriate for true must-arrive state transitions, but they must be audited carefully because they suspend later reliable execution until acknowledged.
- Without relevancy and update tuning, off-screen or low-value actors can consume budget that should go to active combat.

### Structural / Gold-Standard Risks

- The project does not yet have an explicit replication budget or per-class relevancy policy.
- There is no visible replication-graph or actor-priority tuning in the repo.
- Movement replication is currently a default behavior in several Mini classes rather than a consciously budgeted decision.
- The repo does not yet show a quantization or aggregation strategy for high-volume replicated combat state.

### Evidence Anchors

- `Source/T66Mini/Private/Gameplay/T66MiniEnemyBase.cpp`
  - `bReplicates = true`
  - `SetReplicateMovement(true)`
- `Source/T66Mini/Private/Gameplay/T66MiniEnemyProjectile.cpp`
- `Source/T66Mini/Private/Gameplay/T66MiniProjectile.cpp`
- `Source/T66Mini/Private/Gameplay/T66MiniPickup.cpp`
- `Source/T66Mini/Private/Gameplay/T66MiniInteractable.cpp`
- `Source/T66Mini/Private/Gameplay/T66MiniHazardTrap.cpp`
- `Source/T66Mini/Private/Gameplay/T66MiniCompanionBase.cpp`
- `Source/T66Mini/Private/Gameplay/T66MiniPlayerPawn.cpp`
  - server unreliable move/aim RPCs
  - direct projectile spawn paths
  - reliable ultimate activation RPC
- `Source/T66Mini/Public/Gameplay/T66MiniPlayerController.h`
  - reliable server/client travel and summary RPCs
- Repo search result
  - no explicit `NetCullDistanceSquared`, `NetUpdateFrequency`, `MinNetUpdateFrequency`, `NetPriority`, `FVector_NetQuantize`, `FFastArraySerializer`, or `ReplicationGraph` configuration found in project code/config

### Proposed Solutions

- Define replication classes:
  - critical player state
  - combat-authoritative state
  - infrequent interactable state
  - cosmetic local-only state
- Audit every Mini actor class that currently uses `SetReplicateMovement(true)` and justify it.
- Audit projectile replication separately from enemy/player replication:
  - decide what truly needs authoritative replicated actors
  - what can be local or predicted
  - and what can be aggregated rather than replicated actor-by-actor
- Add class-specific tuning where justified:
  - `NetCullDistanceSquared`
  - `NetUpdateFrequency`
  - `MinNetUpdateFrequency`
  - `NetPriority`
- Add a low-risk serialization audit for high-volume replicated vectors or RPC payloads:
  - prefer quantized vector types such as `FVector_NetQuantize` where gameplay precision allows
- Keep reliable RPCs only for state transitions that truly must arrive in order; high-frequency input and cosmetic events should remain unreliable or local where possible.
- Use `stat net` and Network Insights for online Mini density scenarios before speculating.
- If online density captures point at projectile replication specifically, evaluate a manager / `FFastArraySerializer` style aggregation approach before jumping straight to `ReplicationGraph`.
- Treat `ReplicationGraph`, segmented movement, or more advanced NPC replication schemes as `P2` future options only if online density captures prove ordinary movement replication is the bottleneck.

### Target Budgets

- `MP-03`: no network-orchestration stalls hiding inside the join flow
- Online `MN-02`: network bandwidth and replication cost must not become the reason gameplay misses the hitch budget

### Validation Scenarios

- `MP-03`, `MP-06`, `MN-02`
- Capture `stat net` and Network Insights during online Mini runs with rising actor density

## Recommended Validation Passes

### Frontend

- `FE-01` main menu -> `PowerUp` cold
- `FE-02` main menu -> `PowerUp` warm
- `FE-03` main menu -> `Settings`
- `FE-03` main menu -> `Achievements`
- `FE-03` main menu -> `Account Status`
- `FE-06` hero select -> gameplay
- `FE-07` gameplay -> frontend
- Run one UI-specific A/B pass on hot frontend screens:
  - current behavior
  - global invalidation trial build
  - targeted attribute-removal / rebuild-reduction pass
- Record Slate timing, batching, and overdraw on at least `PowerUp`, `Settings`, and gameplay HUD.

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
- `CT-01` first attack and hit feedback
- `MN-01` Mini character select
- `MN-02` Mini gameplay wave
- Packaged cold boot and package-size deltas after any always-cook scope change

## Ranked Remediation List

### Immediate Before Art

1. Remove healthy-path fixed retry behavior and extra orchestration from invite accept -> joined lobby.
2. Make `PowerUp`, top-bar destinations, and other hot frontend screens fast on warm open by eliminating full rebuild and sync asset work on activation.
3. Convert character-visual preload into a genuinely async-ready contract.
4. Warm critical gameplay VFX/SFX and selected-hero presentation assets before travel completes.
5. Remove Mini per-tick `TActorIterator` usage from enemy, pickup, interactable, hazard, projectile, and battle-HUD paths.
6. Pool Mini telegraphs/pulses and stop per-acquire compaction work in hot pooling paths.
7. Add canonical timing markers for screen open, invite milestones, gameplay-ready release, and stage bootstrap.
8. Run a compatibility-gated global invalidation pass on hot UI paths and keep it only where captures prove it helps.
9. Reduce high-frequency bound Slate attributes and rebuild-heavy hot UI paths.
10. Decide whether dense in-world UI should remain on per-actor widget components or begin moving toward aggregated HUD rendering.
11. Extend pooling discipline to repeated combat projectiles, telegraphs, and transient VFX where captures show live spawn/destroy cost.
12. Narrow always-cook scope for large content directories before the art pass grows payload further.

### Next Engineering Pass

1. Rework frontend lifecycle around persistent shells, warm activation, and dirty-region refresh.
2. Define strict runtime UI asset policy with no loose-file import fallback on shipping user-facing paths.
3. Add a player-facing UI scale option on top of the existing DPI curve and continue hardcoded-size cleanup.
4. Convert high-value data-table and soft-reference getters away from ad hoc synchronous fallback.
5. Add explicit tick governance and pooling review gates.
6. Phase stage assembly into measured bootstrap slices.
7. Establish a replication/relevancy policy for Mini before enemy density grows further online.
8. Baseline GC behavior and reduce allocation churn before tuning GC CVars.
9. Run a measured Nanite/VSM and translucent-VFX review by asset class rather than by blanket engine setting.
10. Decide whether the bespoke UI manager should remain controller-owned, move to a persistent subsystem, or eventually give way to an activatable-stack model.

### Later Structural Cleanup

1. Reduce build/module public dependency surface and header fan-out.
2. Review dynamic delegate usage, hot-path by-value APIs, and Slate attribute discipline.
3. Expand internal docs for profiling commands, Slate/Insights recipes, Widget Reflector workflows, and regression gates.
4. Standardize rendering/collision/instancing review policy across content.
5. Keep Mini and main-runtime optimization standards aligned so the project does not fork into two different quality bars.

## Final Opinion

`V1` was directionally correct. The Claude review passes added several useful areas, but the biggest improvement through `V5` is still **better filtering**, not volume:

- keep the concrete repo-grounded feel killers from `V1`
- preserve the broader infrastructure topics from `V2`
- add the validated Steam, backend, instancing, PMC, Niagara, replication, cook-scope, and UI-lifecycle rules from the later passes
- remove or soften the parts that were too absolute, too article-driven, or not actually true of this repo

The result is a stronger master document for implementation. The project already has some correct instincts in place:

- actor registry usage
- real lag tracking
- load logging
- multiplayer diagnostics
- custom Slate/widget craftsmanship in several areas
- some existing instancing and pooling

The problem remains inconsistency. `V5` keeps the original thesis: the game feels janky not because one system is catastrophically wrong, but because multiple important paths still mix modern async/event-driven patterns with older rebuild-centric, sync-fallback, scan-heavy, spawn-heavy, and over-broad content/cook behavior.

The biggest new lesson from the final UI pass is that T66's UI cost is not mainly "too many widgets" in the abstract. The stronger local evidence is:

- rebuild-centric screen activation
- permissive UI asset fallback on hot paths
- high-frequency `TAttribute` / `Visibility_Lambda(...)` usage on hot widgets
- animated widgets without a formal invalidation/volatility policy
- dense per-actor widget-component presentation in gameplay

`V5` keeps the original thesis and adds the supporting guardrails that stop future work from backsliding:

- keep healthy-path multiplayer work at the session/OSS layer and clean up the state machine before chasing transport folklore
- treat backend concurrency, cold-start, and trust boundaries as explicit engineering rules
- treat instancing as a measured rendering tool, with batched update discipline, not as a cargo-cult checkbox
- treat bounded PMC, Niagara, projectile replication, always-cook scope, and UI invalidation architecture as measurable engineering contracts rather than as places to copy generic Unreal advice blindly

That is the right thing to fix before the art pass.

## Appendix A: Official Reference Index

Primary official references used to validate `V5`:

- [Testing and Optimizing Your Content](https://dev.epicgames.com/documentation/en-us/unreal-engine/testing-and-optimizing-your-content)
- [Introduction to Performance Profiling and Configuration in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/introduction-to-performance-profiling-and-configuration-in-unreal-engine?application_version=5.6)
- [Stat Commands in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/stat-commands-in-unreal-engine?application_version=5.6)
- [Optimizing User Interfaces in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/optimizing-user-interfaces-in-unreal-engine?application_version=5.7)
- [Optimization Guidelines for UMG in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/optimization-guidelines-for-umg-in-unreal-engine?application_version=5.6)
- [Invalidation in Slate and UMG for Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/invalidation-in-slate-and-umg-for-unreal-engine?application_version=5.6)
- [Using the Invalidation Box for UMG in Unreal Engine](https://dev.epicgames.com/documentation/zh-cn/unreal-engine/using-the-invalidation-box-for-umg-in-unreal-engine)
- [Slate Clipping System](https://dev.epicgames.com/documentation/en-us/unreal-engine/slate-clipping-system)
- [DPI Scaling in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/dpi-scaling-in-unreal-engine)
- [Slate Insights in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/slate-insights-in-unreal-engine?application_version=5.6)
- [UCommonActivatableWidgetStack API Reference](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/CommonUI/UCommonActivatableWidgetStack)
- [Optimizing and Building UI for AAA Games | Unreal Fest Online 2020](https://dev.epicgames.com/community/learning/talks-and-demos/jy5/optimizing-and-building-ui-for-aaa-games-unreal-fest-online-2020)
- [UMG Best Practices](https://www.unrealengine.com/tech-blog/umg-best-practices)
- [Actor Ticking in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/actor-ticking-in-unreal-engine?application_version=5.6)
- [Remote Procedure Calls in Unreal Engine](https://dev.epicgames.com/documentation/ar-ar/unreal-engine/remote-procedure-calls-in-unreal-engine)
- [Replicated Object Execution Order in Unreal Engine](https://dev.epicgames.com/documentation/ar-ar/unreal-engine/replicated-object-execution-order-in-unreal-engine)
- [Performance and Bandwidth Tips](https://dev.epicgames.com/documentation/en-us/unreal-engine/performance-and-bandwidth-tips?application_version=4.27)
- [Widget Components in Unreal Engine](https://dev.epicgames.com/documentation/es-es/unreal-engine/widget-components-in-unreal-engine)
- [bDrawAtDesiredSize API Reference](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/UMG/Components/UWidgetComponent/bDrawAtDesiredSize)
- [Instanced Static Mesh Component in Unreal Engine](https://dev.epicgames.com/documentation/es-mx/unreal-engine/instanced-static-mesh-component-in-unreal-engine)
- [Incremental Garbage Collection in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/incremental-garbage-collection-in-unreal-engine?application_version=5.6)
- [Unreal Engine Console Variables Reference](https://dev.epicgames.com/documentation/es-es/unreal-engine/unreal-engine-console-variables-reference%3Fapplication_version%3D5.6)
- [Virtual Shadow Maps in Fortnite Battle Royale Chapter 4](https://www.unrealengine.com/tech-blog/virtual-shadow-maps-in-fortnite-battle-royale-chapter-4?lang=en-US)
- [Online Subsystem Steam Interface in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/online-subsystem-steam-interface-in-unreal-engine)
- [FOnlineAuthUtilsSteam::IsSteamAuthEnabled](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/OnlineSubsystemSteam/FOnlineAuthUtilsSteam/IsSteamAuthEnabled)
- [Steam Networking (Steamworks Documentation)](https://partner.steamgames.com/doc/features/multiplayer/networking)
- [Steam Datagram Relay (Steamworks Documentation)](https://partner.steamgames.com/doc/features/multiplayer/steamdatagramrelay)
- [Fluid Compute](https://vercel.com/docs/fluid-compute)
- [Fluid compute concurrency](https://vercel.com/docs/functions/configuring-functions/concurrency)

## Appendix B: Supplementary Reading Preserved from the Review

These were useful as secondary inputs during refinement, but `V5` only promoted ideas from them when they aligned with repo evidence or official guidance.

- [Jettelly - Adding more than 1000 NPCs in multiplayer without network saturation in Unreal Engine](https://jettelly.com/blog/adding-more-than-1000-npcs-in-multiplayer-without-network-saturation-in-unreal-engine)
- [Tiger Abrodi - Nanite Explained](https://tigerabrodi.blog/nanite-explained-how-modern-game-engines-render-billions-of-triangles-without-exploding)
- [VALORANT - Performance Boost: Global Invalidation](https://playvalorant.com/en-gb/news/dev/performance-boost-valorant-s-global-invalidation/)
- [YawLighthouse - UMG/Slate Compendium](https://github.com/YawLighthouse/UMG-Slate-Compendium)
- [benui - UI Best Practices](https://unreal-garden.com/tutorials/ui-best-practices/)
- [benui - UI Performance](https://unreal-garden.com/tutorials/ui-performance/)
- [benui - UI Resolution](https://unreal-garden.com/tutorials/ui-resolution/)
- [Snorri Sturluson - Custom Slate Widgets](https://snorristurluson.github.io/CustomSlateWidgets/)
- [Snorri Sturluson - Activatable Widgets](https://snorristurluson.github.io/ActivatableWidgets/)
- [hzfishy - Building Slate Widgets](https://notes.hzfishy.fr/Unreal-Engine/Slate/Building-Slate-Widgets)
- [Alibaba Cloud - UI Optimization Tips in Unreal Engine 4](https://topic.alibabacloud.com/a/ui-optimization-tips-in-unreal-engine-4_8_8_10274886.html)
- [Jonathan A. Daley - Get UMG DPI Scale at Runtime](https://gist.github.com/JonathanADaley/151f26b145981336371b73def45209e7)
- [benui - UE-BUITween](https://github.com/benui-dev/UE-BUITween)
- [Unreal Directive - Dumpticks](https://www.unrealdirective.com/tips/dumpticks/)
- [Mossyblog - Devlog: ISM Instance Static Mesh Magic](https://medium.com/mossyblog/devlog-ism-instance-static-mesh-magic-part-1-d448cd020564)
- [Rime - Unreal Static Mesh and Decal Optimisations](https://blog.rime.red/unreal-static-mesh-and-decal-optimisations/)
- [Unreal Forum - Instanced Static Mesh transform update performance](https://forums.unrealengine.com/t/instanced-static-mesh-transform-update-performance/577043)
- [Unreal Community Wiki - Using Per Instance Custom Data on Instanced Static Mesh](https://unrealcommunity.wiki/using-per-instance-custom-data-on-instanced-static-mesh-bpiygo0s)
- [Unreal Forum - Tips: Custom Primitive Data and Per Instance Custom Data](https://forums.unrealengine.com/t/tips-custom-primitive-data-and-per-instance-custom-data/1212364)
- [Gradientspace - Mesh Generation and Editing at Runtime in UE4.26](https://www.gradientspace.com/tutorials/2020/10/23/runtime-mesh-generation-in-ue426)
- [TriAxis Games - RealtimeMeshComponent](https://github.com/TriAxis-Games/RealtimeMeshComponent)
- [Unreal Forum - Procedural Mesh Component Collision Baking/Custom Collision](https://forums.unrealengine.com/t/procedural-mesh-component-collision-baking-custom-collision/355860)
- [Unreal Engine - How Unreal Engine empowered a small team to build Luna Abyss' beautiful bullet hell prison](https://www.unrealengine.com/developer-interviews/how-unreal-engine-empowered-a-small-team-to-build-luna-abyss-beautiful-bullet-hell-prison?lang=en-US)
- [More VFX Academy - Complete Guide to Niagara VFX Optimization in Unreal Engine](https://morevfxacademy.com/complete-guide-to-niagara-vfx-optimization-in-unreal-engine/)
- [More VFX Academy - Niagara VFX Optimization: Part 2](https://morevfxacademy.com/niagara-vfx-optimization-part-2-profiling-scalability-and-performance-tips/)
- [AMD GPUOpen - Unreal Engine Performance Guide](https://gpuopen.com/learn/unreal-engine-performance-guide/)
- [Yoreei - Crowd Pathfinder](https://github.com/yoreei/crowd_pathfinder)
- [Leif Erkenbrach - Flow Field Pathfinding](https://leifnode.com/2013/12/flow-field-pathfinding/)
- [Vorixo - Network Managers](https://vorixo.github.io/devtricks/network-managers/)
- [Steve Streeting - Unreal Network Prediction for Projectiles](https://www.stevestreeting.com/2024/12/12/unreal-network-prediction-for-projectiles/)
- [80 Level - Introduction to Replication Graph: Reducing Network Bandwidth in UE5](https://80.lv/articles/introduction-to-replication-graph-reducing-network-bandwidth-in-ue5)
- [Outscal - Mastering Object Pooling in Unreal Engine](https://outscal.com/blog/unreal-engine-object-pooling)
- [Vercel KB - Improve serverless function cold starts](https://vercel.com/kb/guide/how-can-i-improve-serverless-function-lambda-cold-start-performance-on-vercel)
- [Vercel Blog - Scale to one: how Fluid solves cold starts](https://vercel.com/blog/scale-to-one-how-fluid-solves-cold-starts)
- [OpenStatus - Monitoring latency: Vercel Edge vs Serverless](https://www.openstatus.dev/blog/monitoring-latency-vercel-edge-vs-serverless)
- [Northflank - Vercel backend limitations](https://northflank.com/blog/vercel-backend-limitations)
- [Nakama docs - Leaderboard concepts](https://heroiclabs.com/docs/nakama/concepts/leaderboards/)
