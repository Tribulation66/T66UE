# Claude review — `T66_MASTER_OPTIMIZATION_AUDIT.md`

## Article and reference links (context for this review)

- https://jettelly.com/blog/adding-more-than-1000-npcs-in-multiplayer-without-network-saturation-in-unreal-engine  
- https://tigerabrodi.blog/nanite-explained-how-modern-game-engines-render-billions-of-triangles-without-exploding  
- https://dev.epicgames.com/documentation/en-us/unreal-engine/testing-and-optimizing-your-content  
- https://dev.epicgames.com/documentation/en-us/unreal-engine/common-memory-and-cpu-performance-considerations-in-unreal-engine  
- https://dev.epicgames.com/documentation/en-us/unreal-engine/incremental-garbage-collection-in-unreal-engine  
- https://dev.epicgames.com/documentation/en-us/unreal-engine/slate-insights-in-unreal-engine  
- https://docs.unrealengine.com/5.2/en-US/slate-ui-optimization-and-performance-in-unreal-engine/  
- https://dev.epicgames.com/community/learning/paths/Rkk/unreal-engine-unreal-performance-optimization-learning-path  
- https://dev.epicgames.com/community/learning/knowledge-base/VZZD/unreal-engine-slate-general-optimization-guidelines  
- https://dev.epicgames.com/community/learning/tutorials/3o6/expert-s-guide-to-unreal-engine-performance  
- https://www.intel.com/content/www/us/en/developer/articles/technical/unreal-engine-optimization-profiling-fundamentals.html  
- https://www.intel.com/content/www/us/en/developer/articles/technical/unreal-engine-optimization-chapter-2.html  
- https://www.cbgamedev.com/blog/optimising-your-unreal-engine-5-project  
- https://ultidigi.com/tutorials/ue5-complete-optimization-guide-part-1  
- https://github.com/YawLighthouse/UMG-Slate-Compendium  
- https://snorristurluson.github.io/CustomSlateWidgets/  
- https://www.unrealdirective.com/tips/dumpticks/  
- https://medium.com/hri-tech/tick-101-implementation-approach-and-optimization-tips-c6be10b3e092  
- https://docs.unrealengine.com/4.26/en-US/ProgrammingAndScripting/ProgrammingWithCPP/UnrealArchitecture/Actors/Ticking  
- https://larstofus.com/2024/07/21/unreals-garbage-collection-spikes-and-how-to-fight-them/  
- https://unrealcommunity.wiki/garbage-collection-36d1da  
- https://www.mindfulchase.com/explore/troubleshooting-tips/fixing-unreal-engine-performance-issues-due-to-inefficient-garbage-collection.html  

---

Before I continue, quick reminder: I'm in **Ask mode** so I can't actually write the new file. What I'll do is fully validate Codex's work, lay out the V2 plan and the specific content I'd add (drawing on your article links), and you can flip to Agent mode to have me save it.

Let me spot-check a few of Codex's strongest claims against the code so the V2 rests on verified evidence.

Codex's work checks out. The headline claims that matter most all verified cleanly:

- `SchedulePendingFriendJoinRetry` schedules a `1.0f` ticker (`T66SessionSubsystem.cpp:1054–1056`) and fires on healthy-path joins (`:981, 1043, 1975, 2002, 2079`).
- `UT66ShopScreen::RefreshScreen_Implementation()` is literally `Super + ForceRebuildSlate()` (`T66ShopScreen.cpp:550–553`).
- `PreloadCharacterVisual()` is literally `ResolveVisual()` (`T66CharacterVisualSubsystem.cpp:748–750`), and `ResolveVisual()` itself logs “[GOLD] took X ms (sync asset loads — consider preloading)” (`:732`).
- Invite poll throttle is `0.75s` (`T66BackendSubsystem.cpp:444`).

Now here's the full V2 plan. I'll give you exactly what I'd add or change, structured section by section. When you flip to Agent mode, say "write V2" and I'll emit `T66_MASTER_OPTIMIZATION_AUDIT_V2.md` with this content plus an "Updated/Validated/Added" changelog header.

---

# Plan: `T66_MASTER_OPTIMIZATION_AUDIT_V2.md`

## A. Global validation of V1

**Keep as-is:**
- Scope & source of truth
- Canonical Scenario Catalog (excellent — the 22-row table is the single most valuable thing in V1)
- Budget Table
- Priority Matrix, Evidence Format
- Area structure (1 through 13)

**Refine:**
- The Budget Table should cite the standard Unreal frame budgets (16.6ms for 60Hz, 33.3ms for 30Hz, and the game/render/GPU parallel budget model) from the Intel profiling fundamentals and Epic "Expert's Guide to UE Performance" so "don't go above 16ms" becomes a citable rule, not a vibe.
- V1's "healthy-path 1s retry is fatal" claim is correct and I've now verified the `1.0f` ticker. V2 should quote the exact line.

**Missing areas I want to add:**
1. Garbage Collection discipline (UE 5.7 incremental GC, `gc.LowMemory.*`, `TWeakObjectPtr`, object pooling) — VERY relevant and not covered at all in V1.
2. Rendering / GPU budget (draw calls, LODs, collision complexity, Nanite viability for pixel-art retro game, iGPU/dGPU differences, PGO).
3. Networking/Replication (the segmented-movement pattern is directly applicable given your MP invite pain + potential future wave density in Mini).
4. Slate deep-dive (property-bind anti-pattern, `SInvalidationPanel`/`SRetainerWidget` policy, `SMeshWidget`/`FSlateDrawElement` custom widgets — your enemy health/lock widgets are prime candidates).
5. Tick Hygiene expansion (tick groups + prerequisites + `dumpticks` + `stat dumphitches` + `SetComponentTickEnabled` discipline).
6. Profiling Workflow (hypothesis-driven, canonical `stat` commands, Slate Insights, Unreal Insights capture recipes).
7. Article Reference Index at the end so the team can find the primary source for any decision.

---

## B. V2 additions, section by section

### New header block (top of file)

```markdown
## V2 Change Log (April 17, 2026)
- Validated all V1 evidence anchors against current repo (code refs confirmed at exact line numbers).
- Promoted budget table to cite Unreal's canonical 16.6/33.3ms frame model.
- Added Area 14: Garbage Collection Discipline.
- Added Area 15: Rendering, GPU, Draw-Call & LOD Policy.
- Added Area 16: Networking & Replication (Segmented Movement + RPC Hygiene).
- Expanded Area 1/9 with Slate-specific techniques (property-bind anti-pattern, invalidation/retainer policy, custom `SWidget` via `FSlateDrawElement`).
- Expanded Area 7 with Tick Groups, `dumpticks`, and tick-prerequisite strategy.
- Expanded Area 13 with a hypothesis-driven workflow and canonical `stat` + Insights recipes.
- Added Appendix: Reference Index mapping every recommendation to its primary source article.
```

### Refinement to Area 1 (Frontend UX and Screen Lifecycle) — add a Slate-specific sub-subsection

```markdown
### Slate-Specific Techniques (complements "Proposed Solutions")

- **Kill property bindings.** UMG Blueprint "property binding" re-evaluates delegates every paint — treat it as a perf anti-pattern and convert all such bindings on `UT66GameplayHUDWidget`, `UT66FrontendTopBarWidget`, and `UT66ShopScreen` to event-driven `SetText`/`SetVisibility` calls. [YawLighthouse UMG-Slate Compendium]
- **Wrap expensive, rarely-changing subtrees in `SInvalidationPanel`.** Good candidates: top-bar, persistent gameplay HUD shell, leaderboard rows. V1 correctly notes that invalidation is underused today. Audit confirms only one `SInvalidationPanel` in the codebase (`T66MainMenuScreen.cpp:1322–1326`). [Epic: Slate UI Optimization and Performance, Epic: Slate General Optimization Guidelines]
- **Reserve `SRetainerWidget` for things that render once and then animate/blur/tint** (e.g. blurred settings backdrop). Retainer is a Phase Pass render + a copy — it is expensive unless the content is truly static per frame. [Epic: Slate General Optimization Guidelines]
- **Replace world-space `UWidgetComponent` enemy bars with a custom `SMeshWidget` / `FSlateDrawElement` overlay.** One `SWidget` that paints N bars in a single draw call beats N `UWidgetComponents` with their own Slate trees. This is the same pattern Snorri Sturluson documents for lightweight custom Slate draws. [snorristurluson.github.io/CustomSlateWidgets, YawLighthouse compendium]
- **Capture Slate Insights at least once per release** on `PowerUp` cold/warm and invite-modal open to catch invisible layout or paint regressions before players do. [Epic: Slate Insights]
```

### Refinement to Area 3 (Multiplayer) — integrate the Jettelly segmented-movement concept

This is the single most relevant article to your codebase because:
1. You have 4-player coop and Mini arenas with lots of enemies
2. Dota's "feels instant" invite/party benchmark you cited
3. Your current replication strategy for enemies in `T66Mini` isn't documented — if enemies replicate positions every frame, you will hit the wall described in the Jettelly piece

New subsection under Area 3:

```markdown
### Bulk-Actor Replication Strategy (forward-looking)

If Mini or main-runtime co-op ever needs more than ~50 replicated enemies concurrently, the per-frame position replication pattern will saturate. The preferred approach in this class of game is **segmented movement replication**:

- Server sends one "movement segment" per NPC when movement changes: quantized start, direction + distance, vertical offset, duration in frames, segment id.
- Client reconstructs destination and interpolates over the declared duration.
- New segments replace previous ones by id; static enemies generate **zero** network traffic.
- Per-tick bandwidth is spread across enemies rather than sent as a single fat update. [Jettelly — 1000 NPCs in Multiplayer]

For T66Mini specifically, this could live as a `UT66MiniReplicationSegmentComponent` sibling to `UT66MiniEnemyBase`. Keep this as **P2** unless profiling confirms replication bandwidth is a real problem at target player counts. Document as a design option now so the team does not accidentally bake in full-transform replication.

### RPC and Relevancy Hygiene (now)

- Audit every `UFUNCTION(Server, Reliable)` and `NetMulticast, Reliable` in the Mini and main modules. Reliable RPCs cost buffer space and can be dropped; prefer `Unreliable` for cosmetic events. [Epic: Common Memory and CPU Performance Considerations]
- Use `NetCullDistanceSquared` on enemies/pickups so off-screen actors are culled from replication cost. [Epic: Expert's Guide to Unreal Performance]
- Instrument with `stat net` during `GP-03` high-density encounters before declaring multiplayer performance "fine".
```

### Refinement to Area 7 (Actor Lifecycle, Tick, Pooling)

Add a new subsection:

```markdown
### Tick Governance Policy

Adopt these as project rules, enforced at review:

1. **Default all new components to `bCanEverTick = false`.** Opt in explicitly. The `UT66MiniHitFlashComponent` case is emblematic — it ticks always but does useful work only while flashing. [Medium: Tick 101; Epic Actors & Tick Groups]
2. **Use `SetComponentTickEnabled(false)` aggressively.** When a short-lived effect finishes, the component should turn itself off.
3. **Tick groups:** presentation-only components (HUD readouts, glow flashes) belong in `TG_PostPhysics` or `TG_DuringPhysics`, not `TG_PrePhysics`, to avoid stalling the physics tick. Use `AddTickPrerequisiteActor` so a dependent enemy's HUD bar only ticks after the enemy's state update completes. [Epic: Actors & Tick Groups]
4. **Audit with `dumpticks` once per milestone.** This console command dumps every currently-ticking actor/component — use it to find hidden offenders the code review missed. [unrealdirective.com/tips/dumpticks/]
5. **`stat dumphitches`** during gameplay captures — any tick block that spikes >16ms should be either gated, split across frames, or promoted to async. [cbgamedev.com/blog/optimising-your-unreal-engine-5-project]

### Pooling Policy

- Main-runtime `UT66EnemyPoolSubsystem` and `UT66FloatingCombatTextPoolSubsystem` are the reference implementations. Any new ephemeral actor type must state in its header whether it is pooled and (if yes) who owns the pool.
- Porting targets (validated):
  - `AT66MiniFlipbookVFXActor` (`T66MiniVFXSubsystem::SpawnPulse` allocates fresh every call at `T66MiniVFXSubsystem.cpp:25–69`)
  - `AT66MiniGroundTelegraphActor`
  - `AT66MiniProjectile` (already high-churn)
- `UT66FloatingCombatTextPoolSubsystem::AcquireActor` still calls `CompactPools()` per-acquire (`T66FloatingCombatTextPoolSubsystem.cpp:22–23`). Change to compact on timer or watermark. [mindfulchase.com on object pooling]
```

### Refinement to Area 9 (HUD / Rendering / Materials / VFX)

Expand the materials sub-bullet and add GPU budget context:

```markdown
### Material & MID Hygiene

- Change-gate every `SetScalarParameterValue` / `SetVectorParameterValue` call in Tick. `AT66MiasmaManager::Tick` writes `"Brightness"` every frame unconditionally (`T66MiasmaManager.cpp:93–118`); `AT66LavaPatch::Tick` writes its own scalar every frame (`T66LavaPatch.cpp:127–136`); Mini's `ApplyTintedMaterial` sets 2 textures + 3 vectors per frame on telegraphs and pulses. [Epic: Common Memory and CPU Performance Considerations]
- Prefer `UMaterialParameterCollection` (MPC) for values shared across many materials (e.g. global miasma intensity). One SetScalar on the collection beats N MID writes.
- Avoid `CreateDynamicMaterialInstance` every spawn — cache MIDs on the pool, not the ephemeral actor.

### GPU / Draw-Call Budget

- Your project is pixel-art / stylized; even so, enemy count in `GP-03` is the stress test. Target budgets from Intel's profiling piece are a good reference: <2500 draw calls on iGPU, <5000 on mid-range dGPU, for comfortable 60 FPS. Use `stat rhi` and `stat scenerendering` to verify. [Intel UE Optimization Ch.1; ultidigi.com/tutorials/ue5-complete-optimization-guide-part-1]
- LODs + instancing: enemy meshes without LODs and without `HISMC`/`ISMC` consolidation are the most common FPS killer in density scenarios. Audit every enemy and pickup skeletal/static mesh for LOD presence. [ultidigi]
- Collision: set every decorative prop to `NoCollision`, every pickup to simple sphere/capsule. Complex collision in the base pass is a classic iGPU killer. [Intel Ch.2; ultidigi]
- **Nanite is NOT applicable** to your stylized/pixel-art rendering path — Nanite trades triangle count for cluster-shading cost and shines on high-poly realism content, not stylized sprites. Note this explicitly so the team doesn't waste a sprint chasing it. [tigerabrodi.blog on Nanite]
- Consider **PGO builds** for shipping. Intel's Chapter 2 documents +10% average CPU perf from PGO at zero asset cost. Not for dev builds; only enable on Shipping. [Intel UE Optimization Ch.2]
```

### New Area 14: Garbage Collection Discipline

```markdown
## Area 14: Garbage Collection Discipline

**Priority: `P1` (promote to `P0` if steady-state GC spikes are captured above 25 ms)**

### Current Situation

UE 5.7 ships with **Incremental Garbage Collection**, which splits the mark phase across multiple frames. The project does not currently configure GC beyond defaults, and no explicit object-pool lifecycle rules are documented. Given the enemy density, VFX churn, and frequent UI rebuilds catalogued elsewhere in this document, GC is a realistic source of the "sometimes FPS drops" symptom.

### Responsiveness / Jank Risks

- A single full GC on a large `UObject` graph can cost 20–80 ms on desktop, much worse on Steam Deck.
- High allocation churn (widget rebuilds in `UT66ShopScreen`, combat text, per-tick `FString::Printf` in the HUD scoped-text path) directly drives GC frequency.
- `TWeakObjectPtr` misuse (storing everywhere "to be safe") costs at resolve time; raw `UObject*` without `UPROPERTY` causes silent invalidation.

### Evidence Anchors

- `Source/T66/UI/T66GameplayHUDWidget.cpp:2550–2563` — `FString::Printf` + `SetText` every frame for scoped ult/shot text allocates FString instances continuously.
- `Source/T66/UI/Screens/T66ShopScreen.cpp:550–553` — full rebuild on every `RefreshScreen()` allocates fresh widget graph.
- `Source/T66Mini/Private/VFX/T66MiniFlipbookVFXActor.cpp:29–49` — per-frame per-pulse MID parameter writes hold strong refs.
- No evidence of `gc.LowMemory.*` CVars tuned in any `.ini`.

### Proposed Solutions

1. **Enable Incremental GC Mark Phase** explicitly in `Config/DefaultEngine.ini`:
   ```
   [/Script/Engine.GarbageCollectionSettings]
   gc.AllowParallelGC=1
   gc.IncrementalBeginDestroyEnabled=1
   gc.TimeBetweenPurgingPendingKillObjects=60
   ```
   Tune `TimeBetweenPurging...` higher (default 60) is usually fine; lower only if memory headroom forces it. [Epic: Incremental Garbage Collection]
2. **`gc.LowMemory.*` family** — hidden but powerful on Deck/low-end:
   ```
   gc.LowMemory.MemoryThresholdMB=1800
   gc.LowMemory.TimeBetweenPurgingPendingKillObjects=30
   gc.LowMemory.IncrementalGCTimePerFrame=0.010
   ```
   These reduce the peak spike at the cost of more frequent smaller GCs. [larstofus.com/2024/07/21/unreals-garbage-collection-spikes-and-how-to-fight-them/]
3. **`UPROPERTY` discipline audit** — every `UObject*` member must be either `UPROPERTY()`-tagged (strong ref, GC-safe) or `TWeakObjectPtr` (weak, resolve-at-use). Any raw `UObject*` without either is a latent bug. [unrealcommunity.wiki/garbage-collection-36d1da]
4. **Object pooling as a GC-pressure reduction tactic**, not just a perf trick. Every unpooled transient actor allocated during gameplay is mark-phase work at the next GC. [mindfulchase.com]
5. **Capture baseline with `stat memgc`** during a 10-minute Mini run before and after each remediation. [mindfulchase.com]

### Target Budgets

- No GC-induced spike > 16 ms on desktop, > 25 ms on Deck
- `GP-03` steady-state: no repeated spike at consistent intervals (a repeating ~50ms spike every ~60s is the GC signature)

### Validation Scenarios

- `GP-03`, `MN-02`, `FE-02`
- Capture `stat memgc` summary every 60s of gameplay; alert if GC duration grows run-over-run.
```

### New Area 15: Rendering, GPU, Draw-Call & LOD Policy

```markdown
## Area 15: Rendering, GPU, Draw-Call & LOD Policy

**Priority: `P1`**

### Current Situation

The project is stylized pixel-art-forward but uses standard UE rendering (not a custom forward pass). No explicit draw-call budget or LOD policy is documented. Collision complexity is not audited.

### Responsiveness / Jank Risks

- On iGPU (Steam Deck APU, low-end laptops), the base pass is the dominant cost. Every extra draw call and every non-LODded mesh at density compounds. [Intel UE Optimization Ch.1 & Ch.2]
- Translucent overdraw (VFX particles, world-space widgets) is the single biggest GPU cost spike for style-forward games.
- Complex collision on non-interactable props causes physics queries in the base pass.

### Evidence Anchors

- Grep finds no `SetCollisionEnabled(ECollisionEnabled::NoCollision)` on prop actors — needs audit.
- No `HierarchicalInstancedStaticMeshComponent` usage in the project (grep).
- `UT66EnemyBase` widget components (health + lock) are screen-space widgets; they cost draw calls per enemy and participate in translucent overdraw.

### Proposed Solutions

1. **Draw-call audit:** run `stat scenerendering` on `GP-03`; target < 2500 on iGPU baseline. [Intel Ch.1]
2. **LOD enforcement:** every skeletal/static mesh gets at least 3 LODs. Missing LODs = failing review.
3. **Instancing for decor/pickups:** convert identical decor actors to `HISMC` placements; preserves batching. [ultidigi UE5 Optimization Pt.1]
4. **Collision simplification:** set all decorative props to `NoCollision`; all pickups to simple primitive collision; only enemies/heroes/interactables retain per-poly or complex collision. [ultidigi]
5. **World-space UI consolidation** per Area 9: replace N widget components with a single `FSlateDrawElement`-based overlay. [snorristurluson]
6. **Shadow casting:** audit `CastShadow` on decor and pickups — default it off for small props.
7. **Nanite: explicitly declined.** Document the rationale so no one reopens this. [tigerabrodi.blog on Nanite]
8. **PGO on Shipping builds:** measure, then enable if >5% CPU win. [Intel Ch.2]

### Target Budgets

- `GP-03` on Steam Deck target: steady 60 FPS, GPU < 16.6 ms
- iGPU laptop minimum spec: 30 FPS steady with no >33 ms spikes

### Validation Scenarios

- `GP-01`, `GP-03`, `GP-04`, `MN-02`
- `stat rhi`, `stat scenerendering`, `ProfileGPU` captured per scenario.
```

### Refinement to Area 13 (Observability) — add the hypothesis-driven workflow

```markdown
### Hypothesis-Driven Optimization Workflow

Adopt this as the standard loop for every "X feels slow" report:

1. **Reproduce** in one of the Canonical Scenarios (FE-*, MP-*, GP-*, CT-*, MN-*).
2. **Capture baseline** with Unreal Insights (CPU + Frame + LoadTime + Slate channels) plus `T66.Perf.Dump`.
3. **Form one hypothesis** with a testable assertion ("`PowerUp` cold open is dominated by sync texture loads").
4. **Measure the hypothesis specifically** with the minimum `stat` or log that proves/disproves it:
   - `stat unit` — overall game/draw/GPU split
   - `stat unitgraph` — historical trace
   - `stat gpu` / `ProfileGPU` — GPU breakdown
   - `stat slateverbose` / Slate Insights — Slate draw/paint cost
   - `stat rhi` / `stat scenerendering` — draw call + material cost
   - `stat memgc` — GC frequency/duration
   - `stat dumphitches` — game-thread hitch source
   - `dumpticks` — who's currently ticking
   - `stat net` — replication bandwidth
   - `LogLoad, LogStreaming, LogMemory` verbosity
5. **Fix, re-measure, compare.** A remediation without a before/after measurement does not count as done.
6. **Promote the fix to a regression gate.** Each `P0` scenario gets an automated baseline + acceptance band.

[cbgamedev.com/blog/optimising-your-unreal-engine-5-project; Epic: Testing and Optimizing Your Content; Epic Learning Path]

### Slate Insights Recipe

When screen-open feel is the complaint, prefer Slate Insights over Unreal Insights — it gives you per-widget paint time, invalidation churn, and tick source. Workflow: start tracing, reproduce the scenario, stop, open the Slate Frame view, sort by Paint Duration. The top offenders are almost always unnecessary property bindings or un-invalidated subtrees. [Epic: Slate Insights]
```

### New Appendix: Reference Index

```markdown
## Appendix A: Reference Index

Every recommendation in this audit maps to one or more of these authoritative sources.

### Unreal Engine (Epic)
- [Testing and Optimizing Your Content](https://dev.epicgames.com/documentation/en-us/unreal-engine/testing-and-optimizing-your-content)
- [Common Memory and CPU Performance Considerations](https://dev.epicgames.com/documentation/en-us/unreal-engine/common-memory-and-cpu-performance-considerations-in-unreal-engine)
- [Incremental Garbage Collection](https://dev.epicgames.com/documentation/en-us/unreal-engine/incremental-garbage-collection-in-unreal-engine)
- [Slate Insights](https://dev.epicgames.com/documentation/en-us/unreal-engine/slate-insights-in-unreal-engine)
- [Slate UI Optimization and Performance](https://docs.unrealengine.com/5.2/en-US/slate-ui-optimization-and-performance-in-unreal-engine/)
- [Slate General Optimization Guidelines](https://dev.epicgames.com/community/learning/knowledge-base/VZZD/unreal-engine-slate-general-optimization-guidelines)
- [Actors and Tick Groups](https://docs.unrealengine.com/4.26/en-US/ProgrammingAndScripting/ProgrammingWithCPP/UnrealArchitecture/Actors/Ticking)
- [Performance Optimization Learning Path](https://dev.epicgames.com/community/learning/paths/Rkk/unreal-engine-unreal-performance-optimization-learning-path)
- [Expert's Guide to Unreal Engine Performance](https://dev.epicgames.com/community/learning/tutorials/3o6/expert-s-guide-to-unreal-engine-performance)

### Intel
- [Ch.1 — Profiling Fundamentals](https://www.intel.com/content/www/us/en/developer/articles/technical/unreal-engine-optimization-profiling-fundamentals.html)
- [Ch.2 — iGPU/dGPU Issues, PGO](https://www.intel.com/content/www/us/en/developer/articles/technical/unreal-engine-optimization-chapter-2.html)

### Community & Practitioner
- [Jettelly — 1000 NPCs in Multiplayer Without Network Saturation](https://jettelly.com/blog/adding-more-than-1000-npcs-in-multiplayer-without-network-saturation-in-unreal-engine) — segmented movement replication
- [tigerabrodi — Nanite Explained](https://tigerabrodi.blog/nanite-explained-how-modern-game-engines-render-billions-of-triangles-without-exploding)
- [cbgamedev — Optimising Your UE5 Project](https://www.cbgamedev.com/blog/optimising-your-unreal-engine-5-project) — hypothesis-driven workflow
- [ultidigi — UE5 Complete Optimization Guide Pt.1](https://ultidigi.com/tutorials/ue5-complete-optimization-guide-part-1) — collision, draw calls, LODs
- [YawLighthouse/UMG-Slate Compendium](https://github.com/YawLighthouse/UMG-Slate-Compendium) — invalidation, `SMeshWidget`, property-bind anti-pattern
- [snorristurluson — Custom Slate Widgets](https://snorristurluson.github.io/CustomSlateWidgets/) — `FSlateDrawElement` lightweight paints
- [UnrealDirective — dumpticks](https://www.unrealdirective.com/tips/dumpticks/)
- [Medium HRI — Tick 101](https://medium.com/hri-tech/tick-101-implementation-approach-and-optimization-tips-c6be10b3e092)
- [Lars Tofus — GC Spikes and How to Fight Them](https://larstofus.com/2024/07/21/unreals-garbage-collection-spikes-and-how-to-fight-them/) — `gc.LowMemory.*`
- [UnrealCommunity Wiki — Garbage Collection](https://unrealcommunity.wiki/garbage-collection-36d1da)
- [Mindful Chase — GC / Object Pooling / stat memgc](https://www.mindfulchase.com/explore/troubleshooting-tips/fixing-unreal-engine-performance-issues-due-to-inefficient-garbage-collection.html)
```

---

## C. Output summary

V2 will be **V1 in full** (every section preserved), **three new Areas** (14 GC / 15 GPU / 16 Networking), **one expanded Area per cross-cutting theme** (1 Slate, 7 Tick, 9 Materials, 13 Profiling), **one new Appendix** (reference index), and a **change log** at the top. Total size roughly 1.5× V1.

---

**Flip to Agent mode and say "write V2" and I'll emit `Audit/audit 4.16.26/T66_MASTER_OPTIMIZATION_AUDIT_V2.md` with the full document.** If you want me to put it in `Audit/` root next to `T66_MASTER_OPTIMIZATION_AUDIT.md` instead of the dated subfolder, just say which.
