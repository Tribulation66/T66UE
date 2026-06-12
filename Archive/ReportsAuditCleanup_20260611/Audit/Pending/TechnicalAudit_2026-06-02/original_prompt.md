Working task:
Operator: Codex
Validator: Claude Code
Scope: Full descriptive Technical audit for C:\UE\T66, including main T66 plus T66Mini, T66TD, T66Idle, and T66Deck at full depth. Read-only repo/source/doc/data/backend walk. No fixes, no git operations, no runtime verification sweep.
Stop condition: Produce the full technical audit document with question set, answered architecture/wiring/pipeline/backend sections, shared lifecycle tags, evidence tiers, element IDs, finding IDs, file:line citations, and verification/token notes.

User request:
TECHNICAL AUDIT - GENERATE THE FULL DOCUMENT.

Produce a ground-truth description of how the game is actually built: architecture, tools/services, backend, pipelines, wiring, and where things are broken, partial, or non-functional. Descriptive pass only. Flag suspicious static evidence such as empty handlers, unread stats, dead-end wires, stubs, stale docs, and docs/code/config/backend drift. Do not fix. Do not run full runtime verification of every item.

Scope change from critique: Mini/minigames are now in scope at full depth. Audit T66Mini, T66TD, T66Idle, and T66Deck as thoroughly as the main T66 module: architecture, data flow, UI wiring, and current state.

Shared schema:
- Lifecycle status tags, exact tokens: ACTIVE, DEMO_GATED, HIDDEN_RUNTIME, PARTIAL, DEPRECATED, COMPAT_LEGACY, BROKEN, STUB, ORPHAN_SUSPECT, UNKNOWN.
- Evidence tier on every claim: READ / STATIC_TRACE / PRIOR_ARTIFACT / RUNTIME_VERIFIED.
- Element IDs: TECH-{AREA}-{NNN}, using area suffixes such as COMBAT, PROJECTILE, IDOL, ECONOMY, PET, BOSS, SAVE, BACKEND, UI, PIPELINE, BUILD, MINI, TD, IDLE, DECK.
- Finding IDs: TFIND-{NNN}.
- Use wiring trace: source data -> runtime owner -> UI surface -> save/run-summary/backend.

Document structure:
scope/exclusions; architecture map; data and asset flow; runtime systems and authority division; UI-to-logic wiring; backend and online services; build/cook/stage and import pipelines; known partial/broken/stubbed surfaces; cross-audit references.

Specific early flags to investigate:
Steam AppID 4464300 vs legacy 480; backend doc vs source vs deployed drift; anti-cheat policy drift; UI placeholder/fallback screens; silent-handled purchase/settings/report paths; loot-boost presentation; inert combat/status APIs; boss/pet fallback; outgoing-traveler damage authority; saved-snapshot omissions; import-tooling fragility; generated-output scale; staged-exe provenance.

Repo rules:
Follow AGENTS.md, OPERATOR_VALIDATOR_PROTOCOL.md, and folder routers. Comments/docs may lag runtime, so verify stale-comment claims against current code before classifying. Treat docs/code/config/backend disagreement as findings on affected elements. The audit may write the final document artifact under Audit/Pending/TechnicalAudit_2026-06-02, but must not edit gameplay/source/data assets and must not run git.
