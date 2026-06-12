Operator Packet: COMPLETE

**Operator:** Claude (read-only). **Validator:** Codex. **Scope:** planning only — no edits, no build, no capture. This profile cannot compile/run/capture; any current-verification gate below requires a full Operator run.

## What I confirmed in-tree
- Gate `UsesImpactPresentationForIdol` (`T66CombatComponent.cpp:2161`) is hardcoded to `Idol_Water` + `AOE`. It governs three things: projectile-lane suppression (`:3164`, `:3192`), the impact-presentation branch (`:3337`), and the diagnostic counters (`:3289`+).
- The branch (`:3367`–`:3454`) builds an `IdolModifier` context from `PrimaryWeaponImpactContext`, preserves `ParentSourceID` (`:3373`), applies idol-owned **AOE-shaped** damage via `BuildSlashTargets` over `IdolData.AoeRadius`, then `TrySpawnBoundIdolImpactVFX` → else `SpawnWaterIdolImpactPlaceholderVFX` (blue sphere area-read).
- All four weapon categories already publish a primary weapon impact context (`PerformPierce/Slash/Bounce/DOT` → `PublishWeaponImpactContext`). The AOE path (`PerformSlash:2356`) is the proof driver.
- Diagnostics are Water-specific (`CombatIdolImpactDiagnostic SourceID=Idol_Water`, `ExpectedWaterIdolImpactContexts`…). The contract (`:91`–`119`) requires future proofs to map to the generalized `CombatImpactChainDiagnostic` schema, not Water field names.
- VFX helpers: `SpawnWaterIdolImpactPlaceholderVFX` (impact, area-read). Legacy `SpawnIdolPierceVFX/BounceVFX/DOTVFX` exist but route through imported-asset paths and fall back to `SpawnPierceVFX/SpawnBounceVFX/SpawnDOTVFX` primitives — the arch doc (`:13`) forbids resurrecting that temporary-projectile path *as production*.
- `CombatVFXBindings.csv`: only WeaponBase rows (AOE/Pierce/Bounce). **No `IdolModifier` rows exist**, so `TrySpawnBoundIdolImpactVFX` will resolve `Result=None` and fall to the dev placeholder for every idol until rows are authored.
- `Idols.csv` category data differs by shape: Pierce/Bounce carry `ProjectileSpeed`/`FalloffPerHit` (no radius); DOT carries `DotTickInterval`/`DotDuration`; only AOE carries `AoeRadius`. The current branch's radius query is AOE-native and does **not** translate to the other three.

## Answers

**Q1 — Generalize, don't special-case.** Agree. Replace the hardcoded predicate with an allowlist/flag-driven `UsesImpactPresentationForIdol` covering the chosen proof idols across all four categories; replace `SpawnWaterIdolImpactPlaceholderVFX` with a category-dispatching `SpawnIdolImpactPlaceholderVFX` (line/chain/lingering-area/sphere) reusing the existing `Spawn{Pierce,Bounce,DOT}VFX` primitives; keep `TrySpawnBoundIdolImpactVFX` as the production attempt and placeholder as dev fallback; migrate the diagnostic block to the generalized `CombatImpactChainDiagnostic` schema keyed per idol `SourceID`. One context pipeline, four presentations. Three parallel branches would triplicate the parity/suppression/diagnostic logic and drift.

**Q2 — Default proof idols (one per category):**
- **Pierce → `Idol_Light`** — a beam reads as a pierce line most cleanly off a single impact point; `Idol_Steel` is the alternate if Pablo wants a less "glowy" placeholder.
- **Bounce → `Idol_Electric`** — chain-lightning is the canonical bounce read and matches the chain placeholder.
- **DOT → `Idol_Poison`** — neutral, unambiguous DOT semantics (cloud/lingering); `Idol_Lava`/`Idol_Curse` carry extra connotation.

**Q3 — Seams to change later (implementation):**
- `T66CombatComponent.cpp`: generalize the predicate (and its two suppression sites + counter loop); generalize the impact-presentation branch to dispatch presentation by `IdolData.Category`; migrate diagnostics to `CombatImpactChainDiagnostic` per `SourceID`.
- `T66CombatVFX.cpp` + `T66CombatComponent.h`: new category-dispatching placeholder spawner reusing base presentation primitives (not the legacy idol-asset helpers).
- `CombatVFXBindings.csv`: add `IdolModifier` rows for the three idols (+ Water) with `BaseVisualRadius`/`EffectPacketID`/`bDevelopmentFallbackAllowed`; until then dev fallback is expected.
- Docs: move the worked example in `CombatVFXIdolOverlayArchitecture.md` / `CombatVFXImpactContextContract.md` off Water-only; author an effect packet + impact-context contract block per idol per `CombatVFXAuthoringProcedure.md`; declare visual/damage alignment per `CombatVFXVisualDamageAlignmentContract.md`.
- Capture: per-category base-only vs base+idol + neutral controls.

**Q4 — Questions for Pablo before implementation:**
1. **Damage shape (the load-bearing one):** when a Pierce/Bounce/DOT idol fires from the AOE weapon's single impact point, should it apply its *category-native* damage (line / chain / damage-over-time) or reuse the AOE radius query for first proof? The current branch is radius-only and the non-AOE idols carry no radius. Presentation-only-plus-existing-damage vs full category-native damage materially changes scope.
2. Confirm idol picks (Light / Electric / Poison) or substitute.
3. Confirm the proof stays **AOE-weapon-only** (you said "return to the AOE weapon") rather than also wiring the native Pierce/Bounce/DOT weapons this pass.
4. Is dev-fallback placeholder acceptable for first proof (no real Niagara), or do you want at least one production `IdolModifier` binding row authored?
5. Per the alignment contract, what's the approved marker-vs-line/area read for each non-AOE shape driven from the AOE point?

**Q5 — Proof gates (full Operator run):**
- Compiles.
- Per idol: `CombatImpactChainDiagnostic ContextParity=PASS`, `ParentSourceID=Hero_1_black_aoe`, expected == actual downstream contexts, skip/fallback counters zero or explained.
- `DamageBySource SourceID=Idol_<X>` present for each idol that owns damage.
- Neutral control (AOE weapon, no idol / wrong idol): target diagnostic + `DamageBySource` absent, weapon context still publishes.
- Projectile-lane suppression confirmed (no double travelling lane).
- Diagnostics use the generalized schema, not Water-only names.
- Capture: base-only vs base+idol per category; base weapon VFX still visible; placeholder readable.

**Stop condition met:** questions, assumptions, seams, and plan identified; no changes made. Hand to Codex to validate, and resolve Q4.1 (damage shape) with Pablo before any implementation.
