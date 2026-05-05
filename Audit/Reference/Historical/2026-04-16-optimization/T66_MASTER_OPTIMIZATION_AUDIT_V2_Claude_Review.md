# Claude review — `T66_MASTER_OPTIMIZATION_AUDIT_V2.md`

## Article and reference links (context for this review)

### ISM / HISM

- https://medium.com/mossyblog/devlog-ism-instance-static-mesh-magic-part-1-d448cd020564
- https://unrealcommunity.wiki/using-per-instance-custom-data-on-instanced-static-mesh-bpiygo0s
- https://blog.rime.red/unreal-static-mesh-and-decal-optimisations/
- https://forums.unrealengine.com/t/instanced-static-mesh-transform-update-performance/577043
- https://forums.unrealengine.com/t/tips-custom-primitive-data-and-per-instance-custom-data/1212364
- https://dev.epicgames.com/documentation/en-us/unreal-engine/instanced-static-mesh-component-in-unreal-engine

### Steam / Steamworks

- https://partner.steamgames.com/doc/features/multiplayer/networking
- https://partner.steamgames.com/doc/api/ISteamnetworkingSockets
- https://partner.steamgames.com/doc/features/multiplayer/steamdatagramrelay
- https://partner.steamgames.com/doc/features/leaderboards
- https://partner.steamgames.com/doc/webapi/ISteamLeaderboards

### Unreal ↔ Steam

- https://dev.epicgames.com/documentation/en-us/unreal-engine/online-subsystem-steam-interface-in-unreal-engine
- https://docs.unrealengine.com/4.27/en-US/ProgrammingAndScripting/Online/Steam
- https://ikrima.dev/ue4guide/networking/online-subsystem/using-multiple-custom-online-subsystems/
- https://forums.unrealengine.com/t/is-there-anyone-who-can-run-the-online-subsystem-steam-properly-in-ue-5-6/2543935

### Vercel backend

- https://vercel.com/docs/fluid-compute
- https://vercel.com/kb/guide/how-can-i-improve-serverless-function-lambda-cold-start-performance-on-vercel
- https://vercel.com/blog/scale-to-one-how-fluid-solves-cold-starts
- https://www.getautonoma.com/blog/vercel-fluid-compute
- https://www.openstatus.dev/blog/monitoring-latency-vercel-edge-vs-serverless
- https://vercel.com/blog/edge-functions-generally-available
- https://northflank.com/blog/vercel-backend-limitations

### Leaderboards / anti-cheat

- https://heroiclabs.com/docs/nakama/concepts/leaderboards/
- https://x.com/andkalysh/status/2011077469630058603
- https://x.com/OutoftheboxP/status/2012134245540512207

---

I verified each article against the actual repo before making any recommendations. Here is the full validation + addition pass on `T66_MASTER_OPTIMIZATION_AUDIT_V2.md`.

## Opening judgment

V2 is strong. It already did the hardest filtering work — it softened Nanite, corrected the "no instancing" overstatement, refused to cargo-cult GC CVars, and kept the scenario/budget spine. The things that should go into V3 are **narrow, evidence-backed additions that the V2 author did not have in hand**: a concrete ISM update anti-pattern found in the repo, a missing Steam packet-handler config line, a UE 5.5+ rendering behavior that changes when ISM is even worth doing, and a small set of Vercel behaviors that matter specifically because T66's backend happens to already be structured correctly (so the V3 notes are mostly "codify the thing you're already doing right so nobody breaks it").

Bottom line: V3 is an **accretion pass**, not a correction pass. V2 doesn't need to be rewritten.

---

## Article relevance triage (what applies to T66, what does not, and why)

I verified each article against the repo before scoring relevance. The point is to stop Codex from turning an article into a recommendation when the repo already disproves or doesn't use the thing.

| Article | Verdict | Why |
| --- | --- | --- |
| Mossyblog ISM pipeline | **Apply (selectively)** | The 48/64-byte CPU/GPU instance accounting is useful background, but what actually matters to T66 is the `MarkRenderStateDirty()` batching warning. I found the exact anti-pattern in `T66MiasmaManager::EnsureSpawnedCount` (`:628-634`): `AddInstance` called one-at-a-time in a `while` loop. See Addition A1. |
| Forum: ISM transform update perf | **Apply** | Directly codifies the fix for A1 — `BatchUpdateInstancesTransforms` / batched-add with `bMarkRenderStateDirty=false`, then one manual dirty at the end. `T66MainMapTerrain.cpp:2567` already uses the batched `AddInstances(Transforms, false, false, false)` API correctly — so we have both a bad example (miasma) and a good example (terrain) in the same repo, which is perfect for writing the rule. |
| Rime.red: UE 5.5+ auto-batching | **Apply (policy-shaping, not a fix)** | T66 is on UE 5.7, so auto-batching applies. This matters because V2 Area 15 says "extend instancing beyond terrain/traps." Codex could read that as "convert more actors to ISM manually," but on UE 5.5+ that may be duplicated work since the engine auto-batches many static meshes. The rule needs to be: profile first, then decide between explicit ISM vs letting auto-batching do it. See Addition A2. |
| Unreal wiki: per-instance custom data | **Do not promote** | T66 has zero `PerInstanceCustomData` / `NumCustomDataFloats` usage in the repo. It's future reading if someone ever wants per-instance shader parameters (e.g., per-instance tint for miasma tiles), not a current action. Keep in supplementary reading only. |
| Forum: custom primitive data vs per-instance custom data | **Do not promote** | Same reason — not currently used. Reference only. |
| Epic ISM docs | **Baseline reference** | Already implicitly covered. No action needed. |
| Steam networking overview | **Apply (as a guardrail)** | Your own note at the bottom was correct. T66 is already on the modern path — `DefaultEngine.ini:290` uses `SocketSubsystemSteamIP.SteamNetDriver`, which wraps `ISteamNetworkingSockets`. No `ISteamNetworking` (deprecated) calls in the repo. V3 should codify this as a **rule** so Codex doesn't "optimize" code that's already on the right API. See Addition A3. |
| `ISteamNetworkingSockets` API reference | **Do not promote (and explicitly decline)** | T66 does not call Steam Networking Sockets directly — it goes through the UE wrapper. The `BeginAsyncRequestFakeIP` 4-port limit, the `CreateListenSocketP2P` signatures, etc. are irrelevant unless T66 drops down to the raw C API, which would mean fighting UE's OSS. Explicitly out of scope. |
| Steam Datagram Relay (SDR) | **Apply (P2, future-facing)** | `DefaultEngine.ini` has `bEnabled=true` for `OnlineSubsystemSteam` but I did not find `bAllowP2PPacketRelay` or any explicit SDR/DDoS config. For a pre-release game doing private multiplayer testing this is fine. It becomes P1 before public multiplayer ships. See Addition A4. |
| Steam Leaderboards feature doc | **Do not promote as canonical rule** | T66 does **not** use Steam leaderboards. I searched for `IOnlineLeaderboards`, `FOnlineLeaderboardWrite`, `WriteLeaderboards`, `FindLeaderboards` — zero hits. T66's leaderboards are backend-authoritative via Vercel + Upstash KV. So the hard limits (10k boards, int32, KeepBest vs ForceUpdate, AttachLeaderboardUGC) are constraints *on a hypothetical future Steam leaderboard mirror*, not on current code. If it goes in V3 at all, it goes in as "if we ever mirror to Steam leaderboards, these limits constrain the schema" — not as an active rule. |
| ISteamLeaderboards Web API | **Apply (architectural rule, not a schema rule)** | The one thing from these two articles that *is* universally applicable and that V2 does not explicitly state: **any authoritative leaderboard write path, whether Steam-backed or Vercel-backed, must live server-side behind a privileged credential, never on the client.** T66 already does this correctly — `STEAM_WEB_API_KEY` is only in `Backend/src/lib/steam.ts` and `steam-profile.ts`, and score writes go through `/api/submit-run` on the server. V3 should codify it as an anti-cheat invariant so nobody accidentally puts a score-write on the client. See Addition A5. |
| Epic OSS Steam doc | **Baseline reference** | Supports A3. No separate action. |
| UE 4.27 Steam config doc | **Apply (concrete missing config)** | This article has the one thing I can verify is **missing** from T66 today: the `SteamAuthHandlerComponent` packet handler. V2 Area 3 talks about join flow latency but not about connection authenticity. The UE 4.27/5.x docs recommend `PacketHandlerComponentClasses=... SteamAuthHandlerComponent` so Steam-authenticated connections are validated at the NetDriver layer. Not currently in `DefaultEngine.ini`. See Addition A6. |
| ikrima.dev multiple OSSs | **Do not promote** | T66 is Steam-only per `MASTER_STEAMWORKS.md`. This is P2 future reading only if cross-platform (EOS) is added. |
| UE 5.6 Steam issues thread | **Track, do not codify** | T66 is on UE 5.7. Worth mentioning in V3 as "if Steam connect/disconnect behavior regresses, check known engine thread first" but do not write rules on top of a forum thread. |
| Vercel Fluid Compute doc | **Apply (codify current reality)** | Fluid Compute is auto-default since April 2025. `Backend/vercel.json` has no per-function `runtime` / `maxDuration` overrides — that means Fluid is already in effect. V3 should state that explicitly so nobody accidentally opts into legacy non-Fluid via per-function config. See Addition A7. |
| Vercel cold-start KB | **Apply** | V2 Area 4 treats backend as "thin routes, therefore fine." That's true in steady state, but **invite-accept is often the first or only hit to `/api/party-invite/respond` in a session**, so it's disproportionately exposed to cold starts. See Addition A7. |
| Vercel scale-to-one | **Apply (claim, with caveat)** | Useful as context for A7. The 99.37% no-cold-start number is a headline; the honest frame is "burst traffic can still miss the pre-warm window." |
| Autonoma Fluid analysis | **Apply (rule)** | The module-level-mutable-state warning is the important part. I audited the backend: only `let _redis: Redis | null = null` in `Backend/src/lib/kv.ts` is module-level mutable, and it's a single-write lazy singleton of an HTTP client — safe under concurrent invocations. V3 should codify this as the invariant (no mutable maps/counters/caches at module scope). See Addition A8. |
| OpenStatus Edge vs Serverless | **Apply (targeted)** | Useful for one specific endpoint: `/api/health` already uses `runtime = "edge"`. Candidates for Edge would be any route that can complete without Node-only libs. `authenticateRequest` (Steam ticket validation) requires a network call out to `api.steampowered.com` — that actually works on Edge runtime. Not high priority, but V3 can note `/api/party-invite/pending` as a possible Edge candidate if poll latency ever becomes a felt problem. |
| Vercel Edge Functions GA | **Do not promote** | Duplicate of the above. Note Regional Edge only if T66's KV region and Edge region get mismatched — currently Upstash KV region isn't pinned in repo, so this would be speculative. |
| Northflank Vercel limits | **Apply (codify)** | The "no background jobs / no persistent connections / hard timeouts" is exactly the right frame. T66's backend is already structured consistently with these limits: Upstash Redis is HTTP (no persistent conn), the only scheduled work is Vercel Cron (`/api/cron/weekly-reset`, `/api/cron/data-retention`), no long-running workers. V3 should codify this as the "where each workload lives" contract. See Addition A9. |
| Nakama leaderboard concepts | **Do not promote** | Mental model reference. T66's schema is already defined in `leaderboard-keys.ts` / `anti-cheat.ts`. Reference only. |
| X/Twitter links | **Supplementary** | Useful atmospheric context. Not evidence. |

---

## Verified repo facts from this pass

These are the new evidence anchors V3 can cite directly. All verified against current code as of this review.

- **Steam NetDriver:** `Config/DefaultEngine.ini:289-290` uses `SocketSubsystemSteamIP.SteamNetDriver` as the primary with `OnlineSubsystemUtils.IpNetDriver` fallback. This is the modern (ISteamNetworkingSockets-backed) path.
- **OnlineSubsystemSteam:** `DefaultEngine.ini:292-298` — `bEnabled=true`, `SteamDevAppId=4464300`, `SteamAppId=4464300`, `bInitServerOnClient=true`.
- **Missing config:** no `PacketHandlerComponentClasses` / `SteamAuthHandlerComponent` entry. No `bAllowP2PPacketRelay` setting. No explicit SDR config.
- **ISM good pattern:** `T66MainMapTerrain.cpp:2567` — `Component->AddInstances(InstanceTransforms, false, false, false);` (batched add).
- **ISM anti-pattern:** `T66MiasmaManager.cpp:628-633` — `while (...) TileInstances->AddInstance(FTransform(...));` one-at-a-time, each call invalidating render state.
- **ISM single add (okay for small counts):** `T66MiasmaBoundary.cpp:125`, `T66FloorSpikePatchTrap.cpp:352` — single `AddInstance` calls, low risk.
- **No per-instance custom data anywhere in repo.**
- **No Steam leaderboard integration anywhere in repo** — no `IOnlineLeaderboards`, no `FOnlineLeaderboardWrite`. All leaderboard writes go through `UT66LeaderboardSubsystem` → backend `/api/submit-run`.
- **Backend module-level state:** only `let _redis: Redis | null = null` in `Backend/src/lib/kv.ts:4` (safe singleton; HTTP client, not persistent connection).
- **Backend runtime config:** no per-route `runtime` or `maxDuration` overrides except `/api/health` (`runtime = "edge"`). `vercel.json` has only cron definitions.
- **Backend publisher-key-equivalent exposure:** only `STEAM_WEB_API_KEY` is referenced, only from `Backend/src/lib/steam.ts` and `Backend/src/lib/steam-profile.ts`, never from client-routable code paths. No publisher API key in the repo (and there shouldn't be one).
- **Authoritative writes:** score submission in `/api/submit-run` is server-side; client-side Unreal code calls the backend rather than writing Steam leaderboards directly.

---

## Proposed V3 additions / refinements

These are specific, bounded inserts. Each one names the V2 section it modifies, what to add, and the evidence behind it.

### A1 — Area 15 Rendering/GPU/Instancing: add the ISM-update rule (NEW subsection)

V2 Area 15 mentions instancing policy but does not mention the **update-cost** side of ISM, which is the one that actually bites in T66. Add a subsection:

> **ISM update discipline**
>
> When adding or updating many instances in one frame, use the batched APIs and suppress render-state invalidation per-instance. Dirty the render state **once at the end**, not on every call. Repo evidence:
>
> - Good: `Source/T66/Gameplay/T66MainMapTerrain.cpp:2567` uses `Component->AddInstances(InstanceTransforms, false, false, false);` — batched add with no per-instance render-state invalidation.
> - Bad: `Source/T66/Gameplay/T66MiasmaManager.cpp:628-633` calls `TileInstances->AddInstance(...)` one-at-a-time inside a `while` loop. For small `DesiredCount - SpawnedTileCount` deltas this is invisible, but at stage boot or stage rebuild the loop can iterate over hundreds of tiles and marks render-state dirty on each iteration.
>
> Remediation pattern:
>
> 1. Build a `TArray<FTransform>` of all transforms needed this frame.
> 2. Call the batched add API (`AddInstances` / `BatchUpdateInstancesTransforms`) with `bMarkRenderStateDirty=false`, `bWorldSpace=false`, `bUpdateNavigation=false` when safe.
> 3. Emit one `MarkRenderStateDirty()` / `NavigationSystem::UpdateComponentInNavOctree(...)` after the batch completes.
>
> Validation: `GP-01` (stage start) and `GP-03` (steady state). Capture `stat scenerendering` and `ProfileGPU` before/after on stage-rebuild.

### A2 — Area 15 Rendering/GPU/Instancing: refine the "extend instancing further" recommendation

V2 currently says: *"Extend existing instancing patterns beyond terrain/traps where repeated decor or prop actors remain individual."* On UE 5.5+ this is too aggressive as stated. Replace with:

> On UE 5.5+ the engine auto-batches eligible static meshes even without explicit ISM. Before manually converting a repeated-prop class to ISM/HISM, capture `ProfileGPU` on `GP-01` / `GP-03` and confirm auto-batching isn't already collapsing those draws. Manual ISM is still the right answer for:
>
> - meshes that need per-instance transforms or data that auto-batching won't catch,
> - meshes that are spawned/updated in bursts (use A1 pattern),
> - collision/physics scenarios where one shared collision primitive is acceptable.
>
> Do not convert to ISM purely on the theory that "instancing = faster." Measure first.

### A3 — Area 3 Multiplayer: add a Steam networking API invariant

V2 Area 3 focuses on the invite/join orchestration but does not state the underlying API contract. Add a short subsection:

> **Steam networking API invariant**
>
> T66 uses `ISteamNetworkingSockets` through UE's `SocketSubsystemSteamIP.SteamNetDriver` (`Config/DefaultEngine.ini:290`). Legacy `ISteamNetworking` is not used anywhere in the repo. Future work must not add direct calls to deprecated Steam networking APIs, and optimization suggestions that reference `ISteamNetworking`, raw `CreateP2PSession*`, or `BeginAsyncRequestFakeIP` port limits do not apply.
>
> All join/travel optimization lives at the UE OSS layer or in the session-subsystem state machine, not at the raw socket layer.

### A4 — Area 3 Multiplayer: add SDR as a P2 future item

> **Steam Datagram Relay (SDR) — P2, before public launch**
>
> The repo currently has no SDR-specific configuration. For a pre-release game running private multiplayer testing this is acceptable. Before a public multiplayer release, evaluate enabling Valve's relay network for:
>
> - DDoS protection on matchmaking-discovered connections,
> - routing improvements for cross-region peer traffic,
> - cross-platform fallback behavior if/when non-Steam platforms are added.
>
> SDR is not a responsiveness fix for current felt problems (invite accept → joined lobby latency). It is a network-quality fix for future online conditions. Do not promote to P1 until target online population testing indicates it is needed.

### A5 — Area 3 or a new Area 17 Anti-cheat: add the authoritative-writes invariant

This is an invariant that deserves its own short section because it protects the leaderboard system from a whole class of cheats:

> **Area 17: Authoritative writes and privileged credentials**
>
> **Priority: `P0` as an invariant, not a project to do.**
>
> Every authoritative state write — leaderboard scores, run acceptance, account flags, admin actions — must run server-side, behind a privileged credential, never from the Unreal client. This is already true in T66 today:
>
> - `Backend/src/lib/steam.ts` and `Backend/src/lib/steam-profile.ts` are the only places `STEAM_WEB_API_KEY` is referenced, and both paths are server-only.
> - Run submission goes through `/api/submit-run` on the backend; the Unreal client has no score-write path that bypasses the backend.
> - No publisher API key is present in the repo (it would live in Vercel env if it ever existed).
>
> V3 codifies this as an invariant so future work cannot regress it. Specifically:
>
> - Never add `IOnlineLeaderboards` writes on the client. If Steam leaderboards are ever added as a mirror, the write must happen server-side through `ISteamLeaderboards` (Steam Web API, publisher-key-gated) from the Vercel backend.
> - Never add a Vercel route that accepts a client-declared score as authoritative without the anti-cheat pipeline in `Backend/src/lib/anti-cheat.ts` validating it.
> - Never commit a publisher API key to the repo. It must live only in Vercel env.

Note: Steam leaderboard hard limits (10k boards, int32 score, up to 64 int32 side channels, KeepBest vs ForceUpdate, AttachLeaderboardUGC) are listed here as **constraints on a hypothetical future mirror**, not as current schema rules. Current schema lives in `Backend/src/lib/leaderboard-keys.ts`.

### A6 — Area 3 Multiplayer: add the SteamAuth packet handler gap as a concrete fix

This one is actionable and small:

> **Missing SteamAuthHandlerComponent packet handler**
>
> UE's documented Steam configuration recommends attaching `SteamAuthHandlerComponent` to the NetDriver so Steam-authenticated connections are validated at the packet-handler layer. `Config/DefaultEngine.ini` currently has no `PacketHandlerComponentClasses` entry referencing it. Before public multiplayer, add:
>
> ```ini
> [/Script/OnlineSubsystemUtils.IpNetDriver]
> +PacketHandlerComponentClasses=/Script/OnlineSubsystemSteam.SteamAuthHandlerComponent
> ```
>
> Then validate with a packaged build (`Steam authentication handler initialized` in log, and `steam_auth_*` diagnostics in `Saved/Diagnostics/Multiplayer/`).
>
> This is a trust/integrity fix rather than a latency fix, but it belongs under Area 3 because connection-layer auth is part of the multiplayer readiness contract.

### A7 — Area 4 Backend: add cold-start exposure and Fluid Compute status

Replace V2's Area 4 "Current Situation" paragraph end or add:

> **Cold start exposure on invite-accept is asymmetric**
>
> Invite-accept (`/api/party-invite/respond`) is the one backend route where a single user's first hit of the session is often the **only** hit. That makes it disproportionately exposed to cold-start latency even when steady-state route time is excellent. Vercel Fluid Compute (default since April 2025) reduces this via scale-to-one pre-warming, but it does not eliminate bursty cold starts. `Backend/vercel.json` does not currently override per-function runtime/duration, so Fluid is in effect as default.
>
> Action: include cold-start vs warm request time as a discriminated metric in `MP-03` captures, and keep `/api/party-invite/respond` path-simple so Fluid's pre-warm window is used efficiently. Do not add `export const runtime = "edge"` to routes that depend on Node-only libs (`@upstash/redis` is fine on Node serverless; Edge migration would need validation).

### A8 — Area 4 Backend: add the module-level mutable state invariant

> **Module-level mutable state is forbidden under Fluid concurrency**
>
> Under Fluid Compute, one process instance handles multiple concurrent invocations. Any module-level mutable state (counters, caches, maps written after init) becomes a race condition across concurrent requests. Current repo status:
>
> - `Backend/src/lib/kv.ts` has `let _redis: Redis | null = null` — safe, it's a write-once lazy singleton of an HTTP client.
> - All other identified module-level bindings are `const` primitives/config (e.g. `MAX_IDOLS`, `CACHE_TTL_SECONDS`, `SCORE_HARD_CAP`, `DIFFICULTY_FINAL_STAGE`).
>
> Invariant for V3: backend API route modules and `src/lib/` modules must not hold mutable maps, arrays, counters, or "dedupe" caches at module scope. Caches belong in Upstash KV. Per-request state belongs in request-scoped variables.

### A9 — Area 4 Backend: codify the "what lives on Vercel vs. elsewhere" contract

> **Workload placement contract (Vercel is for request/response and cron, not workers)**
>
> Vercel's serverless model is not a home for long-running workers, persistent TCP connections, or background jobs. T66's backend is already structured consistently with this:
>
> - **On Vercel:** request/response routes under `Backend/src/app/api/*/route.ts` and scheduled Vercel Cron (`/api/cron/weekly-reset`, `/api/cron/data-retention`) from `Backend/vercel.json`.
> - **Not on Vercel:** anything that would need to run continuously or hold a socket.
>
> Invariants for V3:
>
> 1. New backend features that fit into `request → validate → read/write KV → respond` stay on Vercel.
> 2. Anything requiring persistent connections (websockets, custom TCP), long-running computation beyond the 60s Pro / 800s Enterprise timeout, or scheduled work more frequent than Vercel Cron's minute granularity must live somewhere else (dedicated worker VM, Upstash Qstash for queues, or equivalent).
> 3. Replay validation or match arbitration, if added, must be designed with this boundary in mind from the start.

### A10 — Appendix B: update supplementary reading

Append these to V2's supplementary reading, tagged with the same "only promoted when aligned with repo evidence" rule V2 already uses:

- Mossyblog — ISM pipeline and instance accounting
- Rime.red — UE 5.5+ auto-batching caveat
- Unreal forum — ISM transform update batched-dirty pattern
- Steam Partner — OnlineSubsystem/networking/SDR/Leaderboards
- UE 4.27 OSS Steam config — SteamAuthHandlerComponent reference
- Vercel — Fluid Compute, cold-start KB, Edge vs Serverless, backend limits
- Autonoma — Fluid Compute concurrency analysis
- Nakama — leaderboard concept reference

---

## Corrections to V2 that are NOT required

To keep V3 honest: nothing in V2 that I re-checked turned out to be wrong. The Claude-Review corrections that V2 accepted (the Nanite softening, the "instancing exists, policy uneven" rewrite, the GC sequencing, the invalidation claim) all hold up against the codebase as it stands today.

The V2-era changelog line *"No `NoCollision` policy on props was corrected"* is still accurate. I did not re-audit every `SetCollisionProfileName` call in this pass, so I'm not expanding Area 15's collision subsection.

---

## Explicit decisions to *not* promote

- **HISM-over-ISM rules as a blanket preference** — Epic docs support picking ISM vs HISM based on whether you need culling/LOD hierarchy. Repo usage is mostly flat grids (miasma, terrain instances, spikes), which is the correct ISM case. No change.
- **Hard-coded instance-count or draw-call targets from Mossyblog** — V2 already declined this. Keep it declined.
- **Per-instance custom data as a current rule** — Zero repo usage. Reference only.
- **Steam leaderboard hard limits as current schema rules** — T66 doesn't use Steam leaderboards. Listed only as a hypothetical future mirror constraint under A5.
- **Edge runtime migration for auth routes** — Steam ticket validation works on Edge, but it's a rewrite with no measured upside. Skip until cold-start captures show a problem A7's pre-warming doesn't solve.
- **Nakama's operators (set/best/incr)** — T66 already has its own operator semantics in `Backend/src/lib/leaderboard-keys.ts` and `anti-cheat.ts`. Not adopted.
- **Multiple-OSS / EOS guidance** — Steam-only project. Future reading.

---

## Suggested V3 priority deltas (small)

Only three items I would change in V2's priority ordering:

- **A1 (ISM update batching in miasma)** — drop into V2 Area 15 as a concrete `P1` with a specific code fix. It's the one rendering-adjacent item in this pass with a clear "find line, batch the call" shape.
- **A6 (SteamAuthHandlerComponent config)** — `P1`, before public multiplayer. Small, safe config change.
- **A7/A8/A9 (backend invariants)** — all `P1` as **documentation/invariants**, `P2` as active work, because the code already matches the invariants today.

Everything else (SDR, Steam Leaderboard mirroring, Edge migration, per-instance custom data) is `P2` future reading.
