TECHNICAL AUDIT - RE-RUN ON THE THINNED TREE (DESCRIPTIVE, READ-ONLY, NO GIT)

The tree changed substantially since the first technical audit. Produce a fresh technical audit of the current tree. Descriptive, flag-don't-fix, no runtime verification, same schema as the prior technical audit, with one schema addition:

- Add lifecycle status tag `SHELVED` for built-but-disabled/parked/preserved/gated systems, distinct from `DEPRECATED`.
- Map demoted minigame shell classes, T66Versus, T66Buried, and Daily Descent as `SHELVED`.
- Keep evidence tiers `READ`, `STATIC_TRACE`, `PRIOR_ARTIFACT`, `RUNTIME_VERIFIED`.
- Keep `TECH-{AREA}-{NNN}` element IDs, `TFIND-{NNN}` finding IDs, cross-audit suffix conventions, file:line citations, element rows, finding rows, and source-data -> runtime-owner -> UI-surface -> save/run-summary/backend wiring traces.

Known structural changes to describe and verify against live repo:

- Four minigames are no longer separate runtime modules; they are shell classes inside the main T66 module. Confirm module declarations are gone from `T66.uproject`, `.Build.cs` scaffolding is removed, and shell classes satisfy former soft-load routes (`/Script/T66Mini`, etc.) without dangling references. Flag any orphaned route.
- Central `T66ShelvedFeatureGate` is the single source of truth for shelved visibility/entry; previously fragmented gating should now defer to it. Confirm consolidation; flag any gating site that still acts independently.
- Casino registry now 4 games; shop is 4-slot weighted; vendor is guaranteed/floor; companion cage unlock chain; per-difficulty rarity getters deleted. Trace each to confirm new wiring and old paths gone.
- Confirm removal of minigame module scaffolding, arcade runtime tower spawn/popup/descriptors, old casino game widgets/enums, per-difficulty rarity fields/getters. Flag residue as findings.

Carry-forward open findings to re-verify and keep if still current:

- Split outgoing-traveler damage authority: combat callbacks vs pool fallback arrival damage.
- Save snapshot omits live projectile/traveler state.
- Silently handled UI handlers: power-up purchase returns handled on failure, RetroFX commits on cancel, Safe Mode/bug-report handled-without-status.
- Backend AppID 480 residue and anti-cheat policy doc drift.
- Co-op per-member ticket not proven; client-config KV unverified.
- Video runtime catalog vs source manifest drift; reconcile to live manifest.
- Pet data boss-row fallback / no DT_Pets.
- Idol traveler stale comments may now be corrected; verify.
- Build provenance: multiple staged roots, AppID config.

Use sub-agents staged by area. Operator: Codex. Validator: Claude Code. Cite file:line. No fixes, no git. Produce the full current-state technical document.

