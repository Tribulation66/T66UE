You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Audit\Pending\TechnicalAudit_2026-06-02\original_prompt.md
- Codex draft path: C:\UE\T66\Audit\Pending\TechnicalAudit_2026-06-02\TECHNICAL_AUDIT.md
- Independent answer path: C:\UE\T66\Audit\Pending\TechnicalAudit_2026-06-02\ClaudeReview\20260602T043401-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
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

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

This is a read-only, descriptive audit-generation task that the Operator (Codex) can fully execute internally. No user decision, approval, or unblocking is required, so the result is OK. My independent take is that the prompt is self-consistent and actionable as written; the work is to produce the document, and the main risk is execution fidelity (evidence honesty, scope coverage, citation accuracy), not a blocked path.

Key constraints Codex must honor while producing the document:

- **Mini scope override is legitimate.** `AGENTS.md` Â§1 sets the *default* to exclude Mini/minigames, but the user explicitly named `T66Mini`, `T66TD`, `T66Idle`, `T66Deck` "at full depth." That explicit naming satisfies the override condition, so full-depth Mini coverage is required, not a violation. All six source modules exist (`Source/T66`, `T66Deck`, `T66Editor`, `T66Idle`, `T66Mini`, `T66TD`).
- **Read-only, no mutation.** No git, no gameplay/source/data/asset edits. The only permitted write is the final document under `Audit/Pending/TechnicalAudit_2026-06-02`. An `IndependentAnswer-pass1` folder already exists under `ClaudeReview/` there â€” Codex should not clobber that.
- **Evidence tiers must be honest.** Since no runtime sweep is run, the vast majority of claims should be `READ` or `STATIC_TRACE`. `RUNTIME_VERIFIED` must not appear unless that proof was actually produced in this task; per protocol, prior captures map to `PRIOR_ARTIFACT`, not `RUNTIME_VERIFIED`.
- **Stale-comment rule.** Verify comment/doc claims against current code before assigning a lifecycle tag; treat docs/code/config/backend disagreement as findings, not silent reconciliation.
- **Schema compliance.** Use the exact lifecycle tokens, `TECH-{AREA}-{NNN}` element IDs, `TFIND-{NNN}` finding IDs, `file:line` citations, and the sourceâ†’runtime ownerâ†’UI surfaceâ†’save/run-summary/backend wiring trace on each element.

On one early flag I can ground now: **Steam AppID 4464300 is the configured production ID** (`Config/DefaultEngine.ini:308-309` `SteamDevAppId`/`SteamAppId`, `Config/DefaultDemoMode.ini:5` `FullGameSteamAppId`, plus `Binaries/Win64/steam_appid.txt`). The legacy-480 concern is whether `480` still lingers anywhere live (e.g., a stale `steam_appid.txt` in a staged build or fallback path) â€” Codex should confirm presence/absence rather than assume, and note that several `Saved/StagedBuilds*` copies carry their own `steam_appid.txt` that can drift from the source config.

## Evidence Checked

- `Audit/Pending/TechnicalAudit_2026-06-02/original_prompt.md` (read full).
- `OPERATOR_VALIDATOR_PROTOCOL.md` (read full â€” roles, boundaries, evidence/proof rules, result-line semantics).
- `AGENTS.md` Â§1â€“2 (task contract, Mini default-exclusion + explicit-naming override, process fidelity).
- Module inventory under `Source/` (six modules confirmed).
- Steam AppID grep across `Config/`, binaries, and staged builds.
- Existing `ClaudeReview/20260602T043401-IndependentAnswer-pass1` folder.

## Questions Or Blockers

None that require the user. The prompt resolves its own scope ambiguity (Mini inclusion is explicit) and permits the document write. Proceed with Operator generation.

## Caveats

- I did **not** perform the full audit walk; I cannot pre-validate individual TFIND/TECH findings. The accuracy of architecture/wiring/backend claims and `file:line` citations is on the Operator pass and should be spot-checked at cross-review, not assumed correct.
- Watch for evidence-tier inflation and for `Saved/`-tree artifacts (staged exes, cached configs) being cited as live source â€” provenance of `Saved/StagedBuilds*` content should be labeled, since those are build outputs that can lag the current source.
- "Full depth on five modules + backend + pipelines" is large; if the document is truncated or any section is left as a placeholder, that is itself a drift finding and should be marked, not silently omitted.

</validator_independent_answer>

<codex_draft>
# T66 Technical Audit

Generated: 2026-06-02  
Operator: Codex  
Validator: Claude Code  
Scope: descriptive technical audit of main T66 plus T66Mini, T66TD, T66Idle, and T66Deck.  
Verification mode: read-only repo walk, static trace, delegated read-only sub-agent reports, and Claude review. No fixes, no git operations, and no full runtime verification.

## 1. Schema And Method

### Lifecycle Status Tags

Only these lifecycle tokens are used: `ACTIVE`, `DEMO_GATED`, `HIDDEN_RUNTIME`, `PARTIAL`, `DEPRECATED`, `COMPAT_LEGACY`, `BROKEN`, `STUB`, `ORPHAN_SUSPECT`, `UNKNOWN`.

- `ACTIVE`: current code/data has an implemented, reachable path by read or static trace.
- `DEMO_GATED`: current behavior is controlled by demo/full-game gating.
- `HIDDEN_RUNTIME`: code/data exists but is reached through direct entry, loose runtime content, hidden screen registration, or another non-obvious route.
- `PARTIAL`: implementation exists, but the trace shows a missing surface, incomplete persistence, incomplete presentation, fallback dependence, or other unfinished behavior.
- `DEPRECATED`: retained surface is not the current path.
- `COMPAT_LEGACY`: retained compatibility alias, ID, fallback, or legacy bridge is present.
- `BROKEN`: static evidence shows a dead-end or contradicted behavior, not merely unproven runtime behavior.
- `STUB`: explicit no-op, placeholder, or status-only handler.
- `ORPHAN_SUSPECT`: data/source assets exist without a clear current consumer in this pass.
- `UNKNOWN`: current repo evidence was insufficient to classify further.

### Evidence Tiers

- `READ`: file, config, data row, doc, or artifact was read directly.
- `STATIC_TRACE`: source-to-consumer wiring was traced statically through code.
- `PRIOR_ARTIFACT`: prior report/proof artifact was used. This audit avoids using this tier for current technical claims except where explicitly named.
- `RUNTIME_VERIFIED`: current runtime, editor, staged build, backend, or UI verification was run during this pass. This audit did not perform runtime verification.

### Wiring Trace Format

For major elements, the trace is:

`source data/config -> runtime owner -> UI surface -> save/run-summary/backend`

When a step is absent, not found, or only partially wired, that absence is part of the element status or a `TFIND-*` finding.

## 2. Scope And Exclusions

In scope:

- Main T66 runtime module, data tables, combat/traveler/projectile/idol/boss/pet/economy/save/UI/backend/build/import systems.
- Full-depth minigame technical audit for `T66Mini`, `T66TD`, `T66Idle`, and `T66Deck`.
- Backend source under `C:\UE\Backend` where it is directly referenced by T66 docs and client code.
- Build/cook/stage/import/model/video/tooling and generated-output scale.
- Static identification of broken, stubbed, partial, compatibility, and suspicious surfaces.

Out of scope:

- Fixes, edits to gameplay/code/data/config, git operations, build refreshes, and broad runtime proof.
- Proving every individual item, boost, button, or stage at runtime.
- Live Vercel, Steamworks, or deployed KV/DB verification. Deployed reality is flagged where repo evidence cannot prove it.

## 3. Question Set And Answers

1. **What is the top-level runtime architecture?**  
   `READ`: The project is Unreal Engine 5.7 with runtime modules `T66`, `T66Mini`, `T66TD`, `T66Idle`, and `T66Deck`, plus editor module `T66Editor` (`T66.uproject:3`, `T66.uproject:8-34`). The main target only lists `T66` directly (`Source/T66.Target.cs:11-13`), while minigame modules are declared in the `.uproject`.

2. **Where does the game boot?**  
   `READ`: The default map is `/Game/Maps/FrontendLevel` and the game instance is `/Game/Blueprints/Core/BP_T66GameInstance.BP_T66GameInstance_C` (`Config/DefaultEngine.ini:17-20`). `UT66GameInstance` then owns the central data-table loading and cached table access (`Source/T66/Core/T66GameInstance.h:47-150`, `Source/T66/Core/T66GameInstance.cpp:168-180`, `Source/T66/Core/T66GameInstance.cpp:333-354`).

3. **How is runtime data loaded?**  
   `STATIC_TRACE`: Source CSV/JSON lives primarily in `Content/Data` and minigame data roots. DataTables are hardcoded or soft-referenced by `UT66GameInstance` for main game systems (`Source/T66/Core/T66GameInstance.cpp:168-180`, `Source/T66/Core/T66GameInstance.cpp:669-717`). Minigame modules use their own `*DataSubsystem` CSV loaders, for example `UT66MiniDataSubsystem`, `UT66TDDataSubsystem`, `UT66IdleDataSubsystem`, and `UT66DeckDataSubsystem` (`Source/T66Mini/Private/Core/T66MiniDataSubsystem.cpp:171-640`, `Source/T66TD/Private/Core/T66TDDataSubsystem.cpp:23-279`, `Source/T66Idle/Private/Core/T66IdleDataSubsystem.cpp:18-46`, `Source/T66Deck/Private/Core/T66DeckDataSubsystem.cpp:36-153`).

4. **How many authored data rows are in the current source data?**  
   `READ`: Main `Content/Data` has 23 CSV files, 6 JSON files, and 577 CSV rows. Key current counts are 12 heroes, 16 companions, 16 idols, 48 weapons, 30 items, 23 bosses, 25 boss attack definitions, 50 boss attacks, 20 stages, 60 enemies, and 12 status effects. Mini has 12 CSVs and 563 rows; TD has 8 CSVs plus 1 JSON and 175 CSV rows; Idle has 8 CSVs and 33 rows; Deck has 10 CSVs and 36 rows.

5. **Is combat/traveler/idol delivery live or inert?**  
   `STATIC_TRACE`: It is live in current code. `T66DataTypes.h` contains stale comments describing traveler fields as reserved/inert (`Source/T66/Data/T66DataTypes.h:1031`, `Source/T66/Data/T66DataTypes.h:1970`), but `UT66CombatComponent` detects traveler delivery and dispatches outgoing travelers with arrival callbacks and category-native fallbacks (`Source/T66/Gameplay/T66CombatComponent.cpp:4105-4371`, `Source/T66/Gameplay/T66CombatComponent.cpp:4497-4673`). This is a doc/code drift finding, not an inert-system finding.

6. **Who owns outgoing-traveler damage authority?**  
   `STATIC_TRACE`: Most current idol traveler damage is owned by combat callbacks in `UT66CombatComponent`, while `UT66OutgoingTravelerPoolSubsystem` also has a fallback `bApplyDamageOnArrival` path when no callback is supplied (`Source/T66/Gameplay/T66CombatComponent.cpp:4122-4318`, `Source/T66/Gameplay/T66OutgoingTravelerPoolSubsystem.cpp:152-216`). This is split authority and should be runtime-proven later before changing.

7. **How are boss attacks and movement driven?**  
   `STATIC_TRACE`: `UT66GameInstance` owns boss/boss-attack/movement DataTable access and row aggregation (`Source/T66/Core/T66GameInstance.h:120-150`, `Source/T66/Core/T66GameInstance.cpp:932-946`, `Source/T66/Core/T66GameInstance.cpp:1044-1158`). Boss data is DataTable-driven from `Bosses.csv`, `BossAttacks.csv`, `BossAttackDefinitions.csv`, `BossHazardDefinitions.csv`, and `BossMovementPatterns.csv`.

8. **How are pets represented?**  
   `STATIC_TRACE`: `UT66GameInstance` exposes pet table access, but it can synthesize pet-style data from boss rows if a dedicated pet table is absent (`Source/T66/Core/T66GameInstance.h:66-80`, `Source/T66/Core/T66GameInstance.cpp:736-795`). This is an active fallback but a partial data-authority surface.

9. **How are economy, vendor, gambler, and mob loot wired?**  
   `STATIC_TRACE`: `UT66RunStateSubsystem` owns gold, debt, shop purchases, buyback, vendor token pickups, mob loot stacks, and gambler anti-cheat events (`Source/T66/Core/T66RunStateSubsystem.h:394-420`, `Source/T66/Core/T66RunStateSubsystem.h:755-771`, `Source/T66/Core/T66RunStateSubsystem.h:1226-1261`, `Source/T66/Core/T66RunStateSubsystem.h:1456-1457`; `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:254-790`). Blackjack records gambler events and reports casino outcomes (`Source/T66/UI/Casino/T66CasinoGamblerTabWidget_BlackJack.cpp:427-487`, `Source/T66/Core/RunState/T66RunStateSubsystem_AntiCheat.cpp:489-563`).

10. **What does the main save snapshot include and omit?**  
    `STATIC_TRACE`: The snapshot includes owed bosses, inventory slots, boss active/ID/HP/parts, equipped idols, anti-cheat context, and mob loot (`Source/T66/Core/T66RunSaveGame.h:333-597`; `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp:20-110`, `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp:141-347`). This pass did not find live projectile/traveler pool state in the snapshot shape, so transient combat persistence is `PARTIAL`.

11. **How is UI routed to logic?**  
    `STATIC_TRACE`: `AT66PlayerController_Frontend` registers and resolves frontend screens and minigame screen aliases (`Source/T66/Gameplay/T66PlayerController_Frontend.cpp:392-402`, `Source/T66/Gameplay/T66PlayerController_Frontend.cpp:541-729`, `Source/T66/Gameplay/T66PlayerController_Frontend.cpp:1374-1501`, `Source/T66/Gameplay/T66PlayerController_Frontend.cpp:1562-1593`). `UT66ScreenBase` provides a fallback "Screen Not Implemented" placeholder (`Source/T66/UI/T66ScreenBase.cpp:39-61`), so placeholder/fallback surfaces are technically present even where most registered screens have concrete classes.

12. **Which UI buttons look suspicious?**  
    `STATIC_TRACE`: Power-up purchase/unlock paths return handled even when they fail or no subsystem is present (`Source/T66/UI/Screens/T66PowerUpScreen.cpp:1264-1323`). RetroFX dirty settings are committed during close/cancel-style cleanup (`Source/T66/UI/Screens/T66SettingsScreen_RetroFX.cpp:416-424`, `Source/T66/UI/Screens/T66SettingsScreen.cpp:132-171`). Safe Mode applies settings if the subsystem is present and returns handled without visible success/failure state in the handler (`Source/T66/UI/Screens/T66SettingsScreen_Crashing.cpp:89-95`). Bug reporting writes local reports and optionally submits only when backend/ticket are available (`Source/T66/UI/Screens/T66ReportBugScreen.cpp:289-315`).

13. **How does the backend work from the client side?**  
    `STATIC_TRACE`: `UT66BackendSubsystem` reads `BackendBaseUrl`, attaches Steam tickets as `X-Steam-Ticket`, and exposes submit-run, minigame daily/score, bug report, and account APIs (`Source/T66/Core/Backend/T66BackendSubsystem.cpp:278-290`, `Source/T66/Core/Backend/T66BackendSubsystem.cpp:437-450`, `Source/T66/Core/T66BackendSubsystem.h:208-553`). Run submission serializes run, anti-cheat, integrity, score budget, mob loot, vendor, weapon, pet, boss, and damage-map data (`Source/T66/Core/Backend/T66BackendRunSerializer.cpp:173-778`).

14. **What online services are configured?**  
    `READ`: Steam is the default online subsystem, with `SteamDevAppId` and `SteamAppId` set to 4464300 (`Config/DefaultEngine.ini:300-309`). Root `steam_appid.txt` is also 4464300. Demo config distinguishes full game 4464300 from demo 4718770 (`Config/DefaultDemoMode.ini:5-6`).

15. **Is legacy Steam AppID 480 still present?**  
    `STATIC_TRACE`: The main config is 4464300. Backend diagnostics explicitly allow both 4464300 and 480 (`C:\UE\Backend\src\app\api\client-diagnostics\route.ts:46-47`), while party-invite source uses default auth and does not pass an explicit 480 allowlist in the checked routes (`C:\UE\Backend\src\app\api\party-invite\send\route.ts:15-16`, `C:\UE\Backend\src\app\api\party-invite\pending\route.ts:5-6`, `C:\UE\Backend\src\app\api\party-invite\respond\route.ts:11-12`). This is compatibility residue plus doc drift.

16. **How does anti-cheat/integrity flow work?**  
    `STATIC_TRACE`: The client captures run integrity baseline/final hashes and mismatch booleans (`Source/T66/Core/T66RunIntegritySubsystem.cpp:81-155`, `Source/T66/Core/T66RunIntegritySubsystem.cpp:188-275`), then serializes `anti_cheat_context` and `integrity_context` into submit-run JSON (`Source/T66/Core/Backend/T66BackendRunSerializer.cpp:518-561`). Backend `submit-run` evaluates missing/non-pristine integrity as unranked, persists integrity fields, and excludes restricted accounts from ranked queries (`C:\UE\Backend\src\app\api\submit-run\route.ts:328-353`, `C:\UE\Backend\src\app\api\submit-run\route.ts:604-660`, `C:\UE\Backend\src\app\api\submit-run\route.ts:1025-1046`). Policy docs lag this current implementation.

17. **How are minigames entered and structured?**  
    `STATIC_TRACE`: `T66MinigamesScreen` routes to Mini, TD, Idle, and Deck; the frontend controller registers all four minigame screen classes (`Source/T66/UI/Screens/T66MinigamesScreen.cpp:491-509`, `Source/T66/Gameplay/T66PlayerController_Frontend.cpp:700-729`, `Source/T66/UI/T66UITypes.h:45-60`). Each minigame has its own runtime module and CSV loader.

18. **Are minigames fully built?**  
    `STATIC_TRACE`: Mini has the deepest implementation: menu/select/battle/shop/save/backend score flow. TD has active battle logic but partial save snapshot. Idle and Deck are active simplified loops with stubbed options/collection surfaces and partial leaderboard display. Detailed element rows are in Section 10.

19. **How does build/cook/stage work?**  
    `READ`: The release process cooks configured dirs/maps, stages loose runtime roots, and refreshes the standalone shortcut to `Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe` (`Config/DefaultGame.ini:14-31`, `Config/DefaultGame.ini:49-62`, `Scripts/StageStandaloneBuild.ps1:117-138`, `Scripts/StageStandaloneBuild.ps1:318-405`, `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md:153-155`).

20. **What are the largest technical risks surfaced by this descriptive pass?**  
    `READ`/`STATIC_TRACE`: Highest-value flags are doc/code drift, partial save snapshots, backend/deployed-reality uncertainty, compatibility AppID drift, UI handlers that silently handle failures, minigame leaderboard/save gaps, import-tool fragility, video/source catalog mismatch, and large generated-output/staged-output scale.

## 4. Architecture Map

### Module Map

| Element ID | Status | Evidence | Description | Citations |
|---|---|---:|---|---|
| TECH-ARCH-001 | ACTIVE | READ | Unreal 5.7 project with main `T66`, minigame runtime modules `T66Mini`, `T66TD`, `T66Idle`, `T66Deck`, and editor module `T66Editor`. | `T66.uproject:3`, `T66.uproject:8-34` |
| TECH-ARCH-002 | ACTIVE | READ | Main game target uses build settings V6 and directly lists only `T66`; minigame modules are project-declared runtime modules. | `Source/T66.Target.cs:11-13`, `T66.uproject:13-29` |
| TECH-ARCH-003 | ACTIVE | READ | Enabled plugins include Steam online subsystems, ProceduralMeshComponent, AnimToTexture, ElectraPlayer, MovieRenderPipeline, Python scripting/editor scripting, and ModelingTools editor mode. | `T66.uproject:61-109` |
| TECH-ARCH-004 | ACTIVE | READ | The main `T66` module depends on Slate/UMG, HTTP/JSON, Niagara, OnlineSubsystem, EnhancedInput, MoviePlayer, WebBrowser/WebView2 on Win64, and stages WebView2 and `steam_appid.txt`. | `Source/T66/T66.Build.cs:12-25`, `Source/T66/T66.Build.cs:54-70`, `Source/T66/T66.Build.cs:86-89` |

### Runtime Authority Division

| Area | Runtime owner | Authority summary | Evidence | Status |
|---|---|---|---:|---|
| Data tables | `UT66GameInstance` | Central hardcoded and cached DataTable access for main-game systems. | STATIC_TRACE | ACTIVE |
| Combat/idols/travelers | `UT66CombatComponent`, `UT66OutgoingTravelerPoolSubsystem` | Combat computes effects and damage callbacks; outgoing-traveler pool owns visual/projectile dispatch and fallback arrival damage. | STATIC_TRACE | PARTIAL |
| Bosses | `UT66GameInstance`, boss runtime actors/components | Boss rows, attack rows, movement rows and hazard definitions are DataTable-driven. | STATIC_TRACE | ACTIVE |
| Economy/inventory | `UT66RunStateSubsystem` | Gold, inventory, shop, buyback, debt, vendor tokens, mob loot, gambler event capture. | STATIC_TRACE | ACTIVE |
| Save/run snapshots | `UT66RunStateSubsystem`, `UT66RunSaveGame` | Persistent snapshot of core run, boss, inventory, idols, anti-cheat, mob loot; transient projectile/traveler state not found. | STATIC_TRACE | PARTIAL |
| Backend | `UT66BackendSubsystem`, `C:\UE\Backend` routes | Steam-ticket authenticated run/minigame/report/account APIs. | STATIC_TRACE | PARTIAL |
| UI | `AT66PlayerController_Frontend`, screen classes | Registered screen name routing plus fallback placeholder screen base. | STATIC_TRACE | PARTIAL |
| Build/stage | `StageStandaloneBuild.ps1`, release docs | Cook, stage, loose runtime roots, shortcut refresh. | READ | ACTIVE |
| Minigames | `T66Mini`, `T66TD`, `T66Idle`, `T66Deck` modules | Separate modules and CSV loaders entered from main minigames UI. | STATIC_TRACE | PARTIAL |

## 5. Data And Asset Flow

### Main Data

| Element ID | Status | Evidence | Wiring Trace | Citations |
|---|---|---:|---|---|
| TECH-DATA-001 | ACTIVE | READ | `Content/Data/*.csv|json -> DT_* uassets -> UT66GameInstance cached table getters -> combat/UI/run/backend consumers`. Current source count: 23 CSV, 6 JSON, 577 CSV rows. | `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md:58-66`, `Config/DefaultGame.ini:14-29`, `Source/T66/Core/T66GameInstance.cpp:168-180`, `Source/T66/Core/T66GameInstance.cpp:333-354` |
| TECH-HERO-001 | ACTIVE | READ | `Heroes.csv -> DT_Heroes -> UT66GameInstance::GetHeroDataTable -> selection/combat/run summary surfaces`. Current main source count: 12 heroes. | `Content/Data/Heroes.csv:1`, `Source/T66/Core/T66GameInstance.h:66-80`, `Source/T66/Core/T66GameInstance.cpp:712-717` |
| TECH-COMPANION-001 | ACTIVE | READ | `Companions.csv -> DT_Companions -> GameInstance getter -> hero/companion UI and run state`. Current main source count: 16 companions. | `Content/Data/Companions.csv:1`, `Source/T66/Core/T66GameInstance.h:66-80`, `Source/T66/Core/T66GameInstance.cpp:712-717` |
| TECH-ITEM-001 | ACTIVE | STATIC_TRACE | `Items.csv -> DT_Items -> run-state inventory/shop/buyback/vendor token/mob loot consumers -> save/backend run summary`. Current main source count: 30 items. | `Content/Data/Items.csv:31`, `Source/T66/Core/T66GameInstance.cpp:712-717`, `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:254-790`, `Source/T66/Core/Backend/T66BackendRunSerializer.cpp:673-720` |
| TECH-IDOL-001 | ACTIVE | STATIC_TRACE | `Idols.csv -> DT_Idols -> combat idol manager/equipped idols -> combat component traveler/native delivery -> save equipped idols -> backend no-idol/idol context`. Current main source count: 16 idols. | `Content/Data/Idols.csv:1`, `Source/T66/Core/T66GameInstance.cpp:712-717`, `Source/T66/Gameplay/T66CombatComponent.cpp:4105-4371`, `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp:105-110`, `Source/T66/Core/Backend/T66BackendRunSerializer.cpp:661` |
| TECH-WEAPON-001 | ACTIVE | STATIC_TRACE | `Weapons.csv -> DT_Weapons -> GameInstance getter -> combat/weapon context -> backend weapon fields`. Current main source count: 48 weapons. | `Content/Data/Weapons.csv:1`, `Source/T66/Core/T66GameInstance.cpp:712-717`, `Source/T66/Core/Backend/T66BackendRunSerializer.cpp:711` |
| TECH-ENEMY-001 | ACTIVE | READ | `Enemies.csv -> DT_Enemies -> GameInstance enemy table -> combat/spawn/runtime consumers`. Current main source count: 60 enemies. | `Content/Data/Enemies.csv:1`, `Source/T66/Core/T66GameInstance.h:120-150`, `Source/T66/Core/T66GameInstance.cpp:932-946` |
| TECH-BOSS-001 | ACTIVE | STATIC_TRACE | `Bosses/BossAttacks/BossAttackDefinitions/BossHazards/BossMovementPatterns CSV -> GameInstance boss getters/aggregation -> boss runtime -> save boss snapshot -> backend boss fields`. | `Content/Data/Bosses.csv:1`, `Content/Data/BossAttacks.csv:1`, `Content/Data/BossAttackDefinitions.csv:1`, `Content/Data/BossMovementPatterns.csv:1`, `Source/T66/Core/T66GameInstance.cpp:1044-1158`, `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp:76-80`, `Source/T66/Core/Backend/T66BackendRunSerializer.cpp:746` |
| TECH-STAGE-001 | ACTIVE | READ | `Stages.csv -> DT_Stages -> GameInstance stage getter -> progression/spawn/backend context`. Current main source count: 20 stages. | `Content/Data/Stages.csv:1`, `Source/T66/Core/T66GameInstance.h:120-150`, `Source/T66/Core/T66GameInstance.cpp:932-946` |

### Loose Runtime Content And Visual Pipelines

| Element ID | Status | Evidence | Description | Citations |
|---|---|---:|---|---|
| TECH-ASSET-001 | ACTIVE | READ | `DefaultGame.ini` always cooks core game dirs and stages loose runtime roots for `RuntimeDependencies`, movies, Mini, TD, Deck, and Idle data/source art. | `Config/DefaultGame.ini:14-31`, `Config/DefaultGame.ini:49-62` |
| TECH-ASSET-002 | PARTIAL | READ | `RuntimeDependencies` contains a large UI/video loose-content surface: 817 files, mostly PNG and JSON. This is intentional fallback/runtime content but increases packaging provenance risk. | `Config/DefaultGame.ini:49-62`, `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md:115` |
| TECH-VIDEO-001 | PARTIAL | READ | Video generation has separate source manifests and runtime catalog. Current runtime catalog has fewer entries than the source generation manifest in this pass, so catalog parity is flagged. | `Video Generation/README.md:3`, `Video Generation/Instructions/00_VIDEO_GENERATION_ROUTING_INSTRUCTIONS.md:19`, `RuntimeDependencies/T66/Video/frontend_videos.json:1`, `Video Generation/Manifests/frontend_videos.json:1` |
| TECH-MODEL-001 | PARTIAL | READ | Pixal3D/ToonStyle import is process-owned and manifest-driven, with production import instructions and replacement manifests. Generated-output scale is high and must be treated as pipeline artifact, not runtime content by default. | `Model Generation/MODEL_GENERATION_AGENTS.md:21`, `Model Generation/Instructions/00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md:3`, `Model Generation/Instructions/09_PIXAL3D_TOONSTYLE_PRODUCTION_IMPORT_INSTRUCTIONS.md:22`, `Model Generation/Pixal3D/production_asset_replacement_manifest.json:1`, `ToonStyle/Source/ImportPixal3DAsset_Phase1C.py:20` |

## 6. Runtime Systems

### Combat, Projectiles, Travelers, And Idols

| Element ID | Status | Evidence | Wiring Trace | Citations |
|---|---|---:|---|---|
| TECH-COMBAT-001 | ACTIVE | STATIC_TRACE | `Idol/weapon/status data -> UT66CombatComponent -> effect delivery/damage application -> run state/backend damage maps`. Combat has explicit damage application helpers and category-native idol impact paths. | `Source/T66/Gameplay/T66CombatComponent.cpp:4497-4673`, `Source/T66/Gameplay/T66CombatComponent.cpp:4799-4920`, `Source/T66/Core/Backend/T66BackendRunSerializer.cpp:771-778` |
| TECH-PROJECTILE-001 | PARTIAL | STATIC_TRACE | `Combat effect request -> outgoing traveler pool visual/projectile -> callback or fallback arrival damage`. Static trace confirms dispatch; current pass did not runtime-prove every projectile item/boost. | `Source/T66/Gameplay/T66CombatComponent.cpp:4139-4158`, `Source/T66/Gameplay/T66OutgoingTravelerPoolSubsystem.cpp:152-216`, `Source/T66/Gameplay/T66OutgoingTravelerPoolSubsystem.cpp:468-538` |
| TECH-IDOL-002 | ACTIVE | STATIC_TRACE | Traveler delivery comment is stale; current adapter dispatches single, pierce, bounce, DOT, and AOE traveler paths with fallback local impacts. | `Source/T66/Data/T66DataTypes.h:1031`, `Source/T66/Data/T66DataTypes.h:1970`, `Source/T66/Gameplay/T66CombatComponent.cpp:4105-4371` |
| TECH-PET-001 | PARTIAL | STATIC_TRACE | Pet table access exists, but fallback derives pet data from boss rows if the dedicated pet table is absent. | `Source/T66/Core/T66GameInstance.h:66-80`, `Source/T66/Core/T66GameInstance.h:513`, `Source/T66/Core/T66GameInstance.h:577`, `Source/T66/Core/T66GameInstance.cpp:736-795` |
| TECH-STATUS-001 | ACTIVE | READ | Status effects are DataTable-driven through `DT_StatusEffects`. Current main source count: 12 rows. | `Content/Data/StatusEffects.csv:1`, `Source/T66/Core/T66GameInstance.h:120-150`, `Source/T66/Core/T66GameInstance.cpp:932-946` |

### Economy, Loot, Vendor, And Gambler

| Element ID | Status | Evidence | Wiring Trace | Citations |
|---|---|---:|---|---|
| TECH-ECONOMY-001 | ACTIVE | STATIC_TRACE | `Items/shop/mob loot/vendor token data -> UT66RunStateSubsystem -> UI shop/casino/loot surfaces -> save snapshot -> backend mob_loot/vendor/weapon fields`. | `Source/T66/Core/T66RunStateSubsystem.h:394-420`, `Source/T66/Core/T66RunStateSubsystem.h:755-771`, `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:254-790`, `Source/T66/Core/Backend/T66BackendRunSerializer.cpp:673-720` |
| TECH-MOBLOOT-001 | ACTIVE | STATIC_TRACE | Mob loot stacks are clamped, saved, and serialized to backend context. | `Source/T66/Core/T66RunStateSubsystem.h:126`, `Source/T66/Core/T66RunSaveGame.h:572-597`, `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp:479-525`, `Source/T66/Core/Backend/T66BackendRunSerializer.cpp:673` |
| TECH-GAMBLER-001 | ACTIVE | STATIC_TRACE | Blackjack records gambler anti-cheat rounds and reports casino results. Legacy gambler-token compatibility exists separately. | `Source/T66/UI/Casino/T66CasinoGamblerTabWidget_BlackJack.cpp:427-487`, `Source/T66/Core/RunState/T66RunStateSubsystem_AntiCheat.cpp:489-563`, `Source/T66/Core/RunState/pending_issues_RunState.md:7` |
| TECH-LOOT-001 | PARTIAL | STATIC_TRACE | Loot-wheel boost mechanics lock and commit boost interactables, and the wheel has detail text, but pending UI issue records missing focused toast/card presentation. | `Source/T66/Gameplay/World/T66LootWheelInteractable.cpp:216-317`, `Source/T66/UI/Widgets/T66LootWheelOverlayWidget.cpp:1113-1118`, `Source/T66/UI/pending_issues_UI.md:10-15` |

### Save, Run-State, And Backend Serialization

| Element ID | Status | Evidence | Wiring Trace | Citations |
|---|---|---:|---|---|
| TECH-SAVE-001 | ACTIVE | STATIC_TRACE | `UT66RunStateSubsystem -> FT66SavedRunSnapshot -> UT66RunSaveGame -> import/export snapshot`. Snapshot covers inventory, boss, idols, anti-cheat, and mob loot. | `Source/T66/Core/T66RunSaveGame.h:333-597`, `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp:7-110`, `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp:141-347` |
| TECH-SAVE-002 | PARTIAL | STATIC_TRACE | Snapshot shape did not show live projectile/traveler pool or transient combat-timer persistence in this pass. | `Source/T66/Core/T66RunSaveGame.h:333-597`, `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp:20-110` |
| TECH-BACKEND-001 | ACTIVE | STATIC_TRACE | `UT66BackendSubsystem -> Steam ticket -> /api/submit-run -> backend schemas/routes -> ranked/unranked persistence`. | `Source/T66/Core/Backend/T66BackendSubsystem.cpp:278-290`, `Source/T66/Core/Backend/T66BackendSubsystem.cpp:437-450`, `Source/T66/Core/Backend/T66BackendRunApi.cpp:14-55`, `C:\UE\Backend\src\app\api\submit-run\route.ts:127-139` |
| TECH-BACKEND-002 | ACTIVE | STATIC_TRACE | Run summary includes run stats, anti-cheat context, integrity context, score budget, no-idol, mob loot, vendor, weapon, pet, boss, and damage maps. | `Source/T66/Core/Backend/T66BackendRunSerializer.cpp:173-778`, `C:\UE\Backend\src\app\api\submit-run\schemas.ts:203-261` |
| TECH-BACKEND-003 | PARTIAL | STATIC_TRACE | Co-op submission forwards party-member summaries through host payload; this pass did not find per-member Steam-ticket proof in backend route. | `Source/T66/Core/Backend/T66BackendRunApi.cpp:77-105`, `Source/T66/Core/Backend/T66BackendRunApi.cpp:227-275`, `C:\UE\Backend\src\app\api\submit-run\route.ts:127-139` |
| TECH-STEAM-001 | ACTIVE | READ | Steam AppID is configured as 4464300 in config and root `steam_appid.txt`. Demo/full-game IDs are separately configured. | `Config/DefaultEngine.ini:300-309`, `steam_appid.txt:1`, `Config/DefaultDemoMode.ini:5-6` |
| TECH-STEAM-002 | COMPAT_LEGACY | STATIC_TRACE | Backend diagnostics still explicitly accepts legacy 480 in addition to 4464300, while party routes use default auth. | `C:\UE\Backend\src\app\api\client-diagnostics\route.ts:46-47`, `C:\UE\Backend\src\app\api\party-invite\send\route.ts:15-16`, `C:\UE\Backend\src\app\api\party-invite\pending\route.ts:5-6`, `C:\UE\Backend\src\app\api\party-invite\respond\route.ts:11-12` |
| TECH-INTEGRITY-001 | PARTIAL | STATIC_TRACE | Client integrity context is captured and submitted; backend ranks/unranks based on submitted integrity outcome. Policy docs lag current source. | `Source/T66/Core/T66RunIntegritySubsystem.cpp:81-155`, `Source/T66/Core/Backend/T66BackendRunSerializer.cpp:518-561`, `C:\UE\Backend\src\app\api\submit-run\route.ts:328-353`, `C:\UE\Backend\src\app\api\submit-run\route.ts:604-660` |

## 7. UI-To-Logic Wiring

| Element ID | Status | Evidence | Wiring Trace | Citations |
|---|---|---:|---|---|
| TECH-UI-001 | ACTIVE | STATIC_TRACE | `Screen enum/name -> AT66PlayerController_Frontend registry/resolver -> concrete screen class -> handler`. Main and minigame screens are registered by name. | `Source/T66/Gameplay/T66PlayerController_Frontend.cpp:392-402`, `Source/T66/Gameplay/T66PlayerController_Frontend.cpp:700-729`, `Source/T66/Gameplay/T66PlayerController_Frontend.cpp:1374-1501`, `Source/T66/Gameplay/T66PlayerController_Frontend.cpp:1562-1593` |
| TECH-UI-002 | PARTIAL | STATIC_TRACE | Base screen fallback can display "Screen Not Implemented" if a screen class does not override/build real content. | `Source/T66/UI/T66ScreenBase.cpp:39-61` |
| TECH-UI-003 | PARTIAL | STATIC_TRACE | Power-up unlock/purchase handlers call subsystem methods but return handled with minimal failure feedback in the observed handlers. | `Source/T66/UI/Screens/T66PowerUpScreen.cpp:1264-1323` |
| TECH-UI-004 | PARTIAL | STATIC_TRACE | Report bug writes local report, submits backend only when backend/ticket are available, logs/closes after submit path. | `Source/T66/UI/Screens/T66ReportBugScreen.cpp:208-315` |
| TECH-UI-005 | PARTIAL | STATIC_TRACE | Safe Mode setting handler applies settings if subsystem exists and returns handled without explicit user-facing success/failure state in the handler. | `Source/T66/UI/Screens/T66SettingsScreen_Crashing.cpp:89-95` |
| TECH-UI-006 | PARTIAL | STATIC_TRACE | RetroFX close/cancel-style paths commit dirty pending settings during cleanup. | `Source/T66/UI/Screens/T66SettingsScreen_RetroFX.cpp:168-170`, `Source/T66/UI/Screens/T66SettingsScreen_RetroFX.cpp:416-424`, `Source/T66/UI/Screens/T66SettingsScreen.cpp:132-171` |
| TECH-UI-007 | STUB | STATIC_TRACE | Some achievement reference/secret rows are placeholder-only, while live achievement rows call player settings. | `Source/T66/UI/Screens/T66AchievementsScreen.cpp:1261-1271`, `Source/T66/UI/Screens/T66AchievementsScreen.cpp:2128-2136`, `Source/T66/UI/Screens/T66AchievementsScreen.cpp:2191` |
| TECH-UI-008 | STUB | STATIC_TRACE | Hero-selection Steam avatar integration is explicitly logged as placeholder. | `Source/T66/UI/HeroSelection/T66HeroSelectionScreen_Build.cpp:737-741` |

## 8. Minigames

### Shared Minigame Host

| Element ID | Status | Evidence | Wiring Trace | Citations |
|---|---|---:|---|---|
| TECH-MINIHOST-001 | ACTIVE | STATIC_TRACE | `Minigames screen -> frontend controller aliases/classes -> individual minigame module screen`. | `Source/T66/UI/Screens/T66MinigamesScreen.cpp:491-509`, `Source/T66/Gameplay/T66PlayerController_Frontend.cpp:541-729`, `Source/T66/UI/T66UITypes.h:45-60` |
| TECH-MINIHOST-002 | PARTIAL | READ | Minigame router requires minigame work to start from `Gameplay/Minigames`, and docs lag several implementations. | `Gameplay/Minigames/MINIGAMES_AGENTS.md:5-19`, `Gameplay/Minigames/README.md:27` |

### T66Mini

| Element ID | Status | Evidence | Wiring Trace | Citations |
|---|---|---:|---|---|
| TECH-MINI-001 | ACTIVE | READ | `T66Mini` is a runtime module. | `T66.uproject:13-14`, `Source/T66Mini/T66Mini.Build.cs:10` |
| TECH-MINI-002 | PARTIAL | READ | Module depends on UMG/Slate/T66 plus HTTP/JSON/OnlineSubsystem/Steamworks and stages loose Mini data/source art; pending issue records stale missing include dirs. | `Source/T66Mini/T66Mini.Build.cs:10-34`, `Source/T66Mini/pending_issues_T66Mini.md:3-6`, `Config/DefaultGame.ini:54-56` |
| TECH-MINI-003 | ACTIVE | STATIC_TRACE | `Content/Mini/Data -> UT66MiniDataSubsystem -> menus/battle/shop/save/backend score`. Loader covers heroes, idols, companions, difficulties, stages, enemies, bosses, waves, interactables, items, tuning, circus games. | `Source/T66Mini/Private/Core/T66MiniDataSubsystem.cpp:171-640` |
| TECH-MINI-004 | PARTIAL | READ | Mini docs/data roster drift: current Mini heroes CSV has 12 rows while docs elsewhere reference larger rosters. | `Content/Mini/Data/T66Mini_Heroes.csv:1-13` |
| TECH-MINI-005 | ACTIVE | STATIC_TRACE | Main frontend reaches Mini main menu and selection flow. | `Source/T66/UI/Screens/T66MinigamesScreen.cpp:491`, `Source/T66/Gameplay/T66PlayerController_Frontend.cpp:700-1429`, `Source/T66Mini/Private/UI/T66MiniMainMenuScreen.cpp:237` |
| TECH-MINI-006 | ACTIVE | STATIC_TRACE | Mini has character, companion, difficulty, idol, save-slot, battle, shop, and circus screens/subsystems. | `Source/T66Mini/Private/UI/T66MiniCharacterSelectScreen.cpp:440`, `Source/T66Mini/Private/UI/T66MiniCompanionSelectScreen.cpp:493`, `Source/T66Mini/Private/UI/T66MiniDifficultySelectScreen.cpp:464`, `Source/T66Mini/Private/UI/T66MiniIdolSelectScreen.cpp:635`, `Source/T66Mini/Private/UI/T66MiniSaveSlotsScreen.cpp:211`, `Source/T66Mini/Private/UI/T66MiniShopScreen.cpp:1448`, `Source/T66Mini/Private/Core/T66MiniCircusSubsystem.cpp:135-490` |
| TECH-MINI-007 | ACTIVE | STATIC_TRACE | Mini battle owns simulation tick, waves, enemies, projectiles, pickups, interactables, traps, shop/summary transitions, save persistence. | `Source/T66Mini/Private/UI/T66MiniBattleScreen.cpp:260-367`, `Source/T66Mini/Private/UI/T66MiniBattleScreen.cpp:2198-2312`, `Source/T66Mini/Private/UI/T66MiniBattleScreen.cpp:3220` |
| TECH-MINI-008 | PARTIAL | STATIC_TRACE | Mini save/run state has active-run subsystem and slots, but daily leaderboard display path is partial with empty daily rows while all-time/local and backend score hooks exist. | `Source/T66Mini/Public/Core/T66MiniRunSaveGame.h:190`, `Source/T66Mini/Private/Core/T66MiniRunStateSubsystem.cpp:10`, `Source/T66Mini/Private/Core/T66MiniSaveSubsystem.cpp:80-294`, `Source/T66Mini/Private/UI/T66MiniMainMenuScreen.cpp:193`, `Source/T66Mini/Private/Core/T66MiniLeaderboardSubsystem.cpp:87` |
| TECH-MINI-009 | ACTIVE | STATIC_TRACE | Mini visual subsystem searches cooked `/Game/Mini` candidates and loose source art. | `Source/T66Mini/Private/Core/T66MiniVisualSubsystem.cpp:92-288`, `Config/DefaultGame.ini:20`, `Config/DefaultGame.ini:54-56` |

### T66TD

| Element ID | Status | Evidence | Wiring Trace | Citations |
|---|---|---:|---|---|
| TECH-TD-001 | ACTIVE | READ | `T66TD` is a runtime module. | `T66.uproject:18-19`, `Source/T66TD/T66TD.Build.cs:10` |
| TECH-TD-002 | PARTIAL | READ | TD stages loose data/source art but `/Game/TD` is not an always-cook root in current config. | `Source/T66TD/T66TD.Build.cs:10-29`, `Config/DefaultGame.ini:57-58`, `Config/DefaultGame.ini:14-29` |
| TECH-TD-003 | ACTIVE | STATIC_TRACE | `Content/TD/Data -> UT66TDDataSubsystem -> menu/select/battle/save/backend score`. Loader covers heroes, combat, enemy archetypes, tuning, theme rules, difficulties, maps, stages, and JSON layouts. | `Source/T66TD/Private/Core/T66TDDataSubsystem.cpp:23-279`, `Source/T66TD/Public/Core/T66TDDataTypes.h:9-308` |
| TECH-TD-004 | PARTIAL | READ | TD docs/data roster drift: docs reference 16 heroes while current TD hero/combat CSVs have 12 rows. | `Gameplay/Minigames/TD/T66TD_Memory_Progression.md:28`, `Content/TD/Data/T66TD_Heroes.csv:1`, `Content/TD/Data/T66TD_HeroCombat.csv:1` |
| TECH-TD-005 | ACTIVE | STATIC_TRACE | Main frontend reaches TD main menu and difficulty flow. | `Source/T66/UI/Screens/T66MinigamesScreen.cpp:497`, `Source/T66/Gameplay/T66PlayerController_Frontend.cpp:718-1465`, `Source/T66TD/Private/UI/T66TDMainMenuScreen.cpp:203-233`, `Source/T66TD/Private/UI/T66TDDifficultySelectScreen.cpp:419-467` |
| TECH-TD-006 | ACTIVE | STATIC_TRACE | TD battle owns board/path/pads/waves/spawns/enemy move/leak/tower place/upgrade/sell/fire/victory/defeat/reward/score. | `Source/T66TD/Private/UI/T66TDBattleScreen.cpp:712-994`, `Source/T66TD/Private/UI/T66TDBattleScreen.cpp:1144-1545`, `Source/T66TD/Private/UI/T66TDBattleScreen.cpp:1703-2055`, `Source/T66TD/Private/UI/T66TDBattleScreen.cpp:2758` |
| TECH-TD-007 | PARTIAL | STATIC_TRACE | TD save is profile/stage-level; this pass did not find entity/tower/spawn-queue state fields sufficient to reconstruct a mid-wave board. | `Source/T66TD/Public/Core/T66TDRunSaveGame.h:9`, `Source/T66TD/Private/Core/T66TDSaveSubsystem.cpp:17-124`, `Source/T66TD/Public/Core/T66TDProfileSaveGame.h:16`, `Source/T66TD/Private/UI/T66TDBattleScreen.cpp:1492` |
| TECH-TD-008 | PARTIAL | STATIC_TRACE | TD local all-time display and backend score submit exist; daily rows are empty in current menu builder. | `Source/T66TD/Private/UI/T66TDMainMenuScreen.cpp:171-176`, `Source/T66TD/Private/UI/T66TDBattleScreen.cpp:1545` |
| TECH-TD-009 | ACTIVE | STATIC_TRACE | TD visual subsystem loads loose hero/enemy/boss PNGs and map backgrounds. | `Source/T66TD/Private/Core/T66TDVisualSubsystem.cpp:32-87`, `Source/T66TD/Private/UI/T66TDBattleScreen.cpp:3339`, `Content/TD/README.md:1`, `Content/TD/Data/README.md:1` |

### T66Idle

| Element ID | Status | Evidence | Wiring Trace | Citations |
|---|---|---:|---|---|
| TECH-IDLE-001 | ACTIVE | READ | `T66Idle` is a runtime module and stages Idle data/source art as loose content. | `T66.uproject:23-24`, `Source/T66Idle/T66Idle.Build.cs:10-34`, `Config/DefaultGame.ini:61` |
| TECH-IDLE-002 | ACTIVE | STATIC_TRACE | `Content/Idle/Data -> UT66IdleDataSubsystem -> main menu/session/save/backend score`. Loader covers heroes, companions, items, idols, enemies, zones, stages, and tuning. | `Source/T66Idle/Private/Core/T66IdleDataSubsystem.cpp:18-46`, `Content/Idle/Data/T66Idle_Stages.csv:11` |
| TECH-IDLE-003 | PARTIAL | STATIC_TRACE | Frontend state has daily seed/session and selected hero, but profile snapshot API appears only subsystem-local in this pass. | `Source/T66Idle/Public/Core/T66IdleFrontendStateSubsystem.h:18`, `Source/T66Idle/Private/Core/T66IdleFrontendStateSubsystem.cpp:11-40` |
| TECH-IDLE-004 | ACTIVE | STATIC_TRACE | Idle uses local-first profile save with UTC timestamp, offline simulation, passive DPS cap, and safety cap. | `Source/T66Idle/Private/Core/T66IdleSaveSubsystem.cpp:117-196`, `Source/T66Idle/Public/Core/T66IdleDataTypes.h:307` |
| TECH-IDLE-005 | PARTIAL | STATIC_TRACE | Idle menu is wired, but daily builder returns empty and all-time builder creates a synthetic local row; backend score submit path exists. | `Source/T66Idle/Private/UI/T66IdleMainMenuScreen.cpp:256-306`, `Source/T66Idle/Private/UI/T66IdleMainMenuScreen.cpp:894` |
| TECH-IDLE-006 | ACTIVE | STATIC_TRACE | Active tick runs idle combat and reaches summary after final clear. | `Source/T66Idle/Private/UI/T66IdleMainMenuScreen.cpp:226-337`, `Source/T66Idle/Private/UI/T66IdleMainMenuScreen.cpp:952`, `Content/Idle/Data/T66Idle_Stages.csv:11` |
| TECH-IDLE-007 | PARTIAL | STATIC_TRACE | Tap/engine upgrades and one-time buys/unlocks are implemented; authored per-level/max-level fields are not clearly represented as save-owned item levels. | `Source/T66Idle/Public/Core/T66IdleDataTypes.h:38`, `Source/T66Idle/Private/UI/T66IdleMainMenuScreen.cpp:1017`, `Source/T66Idle/Private/UI/T66IdleMainMenuScreen.cpp:1360-1411` |
| TECH-IDLE-008 | ORPHAN_SUSPECT | STATIC_TRACE | Runtime uses loose texture helper and `Idle_Player` style art; hero-specific `Idle_Hero_*` art exists but was not traced into gameplay render use in this pass. | `Source/T66Idle/Private/UI/T66IdleMainMenuScreen.cpp:74-340`, `Source/T66/UI/T66RuntimeUITextureAccess.cpp:342`, `SourceAssets/Idle/README.md:5` |
| TECH-IDLE-009 | STUB | STATIC_TRACE | Idle Options is status/no-op style, while Load starts existing profile. Docs lag registered/playable state. | `Source/T66Idle/Private/UI/T66IdleMainMenuScreen.cpp:1312`, `Source/T66Idle/Private/UI/T66IdleMainMenuScreen.cpp:1471`, `Gameplay/Minigames/Idle/T66Idle_MasterImplementation.md:7-20` |

### T66Deck

| Element ID | Status | Evidence | Wiring Trace | Citations |
|---|---|---:|---|---|
| TECH-DECK-001 | ACTIVE | READ | `T66Deck` is a runtime module and stages Deck data/source art as loose content. | `T66.uproject:28-29`, `Source/T66Deck/T66Deck.Build.cs:10-34`, `Config/DefaultGame.ini:59-60` |
| TECH-DECK-002 | PARTIAL | READ | Deck docs still describe reserved/future state despite current runtime module and UI implementation. | `Content/Deck/README.md:1`, `Gameplay/Minigames/Deck/T66Deck_MasterImplementation.md:7`, `Source/T66Deck/T66Deck.Build.cs:10` |
| TECH-DECK-003 | ACTIVE | STATIC_TRACE | `Content/Deck/Data -> UT66DeckDataSubsystem -> menu/select/combat/save/backend score`. Loader covers cards, companions, encounters, enemies, heroes, items, relics, stage, starting decks, and tuning. | `Source/T66Deck/Private/Core/T66DeckDataSubsystem.cpp:36-153`, `Content/Deck/Data/T66Deck_Encounters.csv:6`, `Content/Deck/Data/T66Deck_Relics.csv:3` |
| TECH-DECK-004 | PARTIAL | STATIC_TRACE | Daily seed, difficulty, starting deck, current map node, seeded save, relic IDs, and run map fields exist, but current screen path does not fully consume seeded map/relic systems. | `Source/T66Deck/Public/Core/T66DeckFrontendStateSubsystem.h:17`, `Source/T66Deck/Private/Core/T66DeckFrontendStateSubsystem.cpp:51`, `Source/T66Deck/Private/Core/T66DeckSaveSubsystem.cpp:78-102`, `Source/T66Deck/Public/Core/T66DeckRunSaveGame.h:86`, `Source/T66Deck/Private/UI/T66DeckMainMenuScreen.cpp:1192-1391` |
| TECH-DECK-005 | PARTIAL | STATIC_TRACE | Deck menu is wired; Load enables when save exists; daily builder empty; all-time display is local best-floor row; backend submit exists. | `Source/T66Deck/Private/UI/T66DeckMainMenuScreen.cpp:344-407`, `Source/T66Deck/Private/UI/T66DeckMainMenuScreen.cpp:1528` |
| TECH-DECK-006 | ACTIVE | STATIC_TRACE | Hero/companion select, floor-filtered encounters, and one 10-floor stage with floor-10 boss exist. | `Source/T66Deck/Private/UI/T66DeckMainMenuScreen.cpp:445-534`, `Source/T66Deck/Private/UI/T66DeckMainMenuScreen.cpp:1281`, `Content/Deck/Data/T66Deck_Stages.csv:2` |
| TECH-DECK-007 | PARTIAL | STATIC_TRACE | Card combat is simplified: energy/damage/block/enemy intent and modulo deck-to-hand selection, not a full shuffled draw/discard deck-builder. | `Source/T66Deck/Private/UI/T66DeckMainMenuScreen.cpp:1246`, `Source/T66Deck/Private/UI/T66DeckMainMenuScreen.cpp:1854-1893`, `Content/Deck/Data/T66Deck_Cards.csv:1` |
| TECH-DECK-008 | PARTIAL | STATIC_TRACE | Card/item reward handlers exist; item effects cover max-health/block; relic CSV/save fields exist without traced acquisition/effect path. | `Source/T66Deck/Private/UI/T66DeckMainMenuScreen.cpp:728`, `Source/T66Deck/Private/UI/T66DeckMainMenuScreen.cpp:1327-1640`, `Source/T66Deck/Private/Core/T66DeckDataSubsystem.cpp:153` |
| TECH-DECK-009 | ACTIVE | STATIC_TRACE | Final boss clear routes summary, save, score submit, and `Frontend_Deck` backend context. | `Source/T66Deck/Private/UI/T66DeckMainMenuScreen.cpp:805`, `Source/T66Deck/Private/UI/T66DeckMainMenuScreen.cpp:1364-1516` |
| TECH-DECK-010 | STUB | STATIC_TRACE | Collection and Options are status-only/stub surfaces; shared WidgetGames extraction remains pending. | `Source/T66Deck/Private/UI/T66DeckMainMenuScreen.cpp:1834`, `Source/T66/Public/UI/WidgetGames/pending_issues_WidgetGames.md:5`, `Source/T66/UI/pending_issues_UI.md:3` |

## 9. Backend And Online Services

| Element ID | Status | Evidence | Description | Citations |
|---|---|---:|---|---|
| TECH-BACKEND-004 | ACTIVE | READ | Backend docs identify Vercel-hosted backend, Steam auth, leaderboards, anti-cheat, restrictions, and diagnostics as backend-owned surfaces. | `Backend/BACKEND_AGENTS.md:5`, `Backend/BACKEND_SYSTEM_REFERENCE.md:37`, `Backend/BACKEND_SYSTEM_REFERENCE.md:282` |
| TECH-BACKEND-005 | UNKNOWN | READ | Deployed backend state was not live-checked. Local docs reference `https://t66-backend.vercel.app`, but this audit does not claim current deployed health. | `Backend/BACKEND_SYSTEM_REFERENCE.md:37`, `Backend/BACKEND_SYSTEM_REFERENCE.md:282` |
| TECH-BACKEND-006 | ACTIVE | STATIC_TRACE | Steam auth validates tickets with configured/default allowed AppIDs and caches validation. | `C:\UE\Backend\src\lib\steam.ts:43-48`, `C:\UE\Backend\src\lib\steam.ts:73`, `C:\UE\Backend\src\lib\steam.ts:148-156` |
| TECH-BACKEND-007 | ACTIVE | STATIC_TRACE | Account status/restriction tables and routes exist and are used by submit-run ranking/quarantine behavior. | `C:\UE\Backend\src\app\api\account-status\route.ts:19-41`, `C:\UE\Backend\src\db\schema.ts:326-398`, `C:\UE\Backend\src\app\api\submit-run\route.ts:761-804` |
| TECH-BACKEND-008 | PARTIAL | STATIC_TRACE | Client build policy is exposed through `/api/client-config` and KV-backed backend routes, but live KV policy was not verified. | `Source/T66/Core/Backend/T66BackendAccountApi.cpp:237`, `C:\UE\Backend\src\app\api\client-config\route.ts:33-144` |
| TECH-BACKEND-009 | ACTIVE | STATIC_TRACE | Party diagnostics send app IDs/lobby IDs from UE and backend stores diagnostics in bug reports with retention fields. | `Source/T66/Core/Backend/T66BackendPartyApi.cpp:23-167`, `Source/T66/Online/T66SessionSubsystem.cpp:523`, `C:\UE\Backend\src\app\api\client-diagnostics\route.ts:40-90`, `C:\UE\Backend\src\db\schema.ts:398` |

## 10. Build, Cook, Stage, And Import Pipelines

| Element ID | Status | Evidence | Description | Citations |
|---|---|---:|---|---|
| TECH-BUILD-001 | ACTIVE | READ | `StageStandaloneBuild.ps1` parses loose runtime roots, runs BuildCookRun, copies loose roots, checks expected exe, and updates standalone shortcut. | `Scripts/StageStandaloneBuild.ps1:117-138`, `Scripts/StageStandaloneBuild.ps1:318-405` |
| TECH-BUILD-002 | PARTIAL | READ | Active staged exe provenance must distinguish `Saved\StagedBuilds\Windows` from stale sibling staged roots. This pass found multiple staged roots by read-only scan. | `Scripts/StageStandaloneBuild.ps1:364`, `Scripts/StageStandaloneBuild.ps1:399-405`, `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md:151-155` |
| TECH-BUILD-003 | PARTIAL | READ | Steam upload wrapper cleans prior depot content and removes `steam_appid.txt`, but Steam docs record uploaded builds not being live until branch switching. | `Tools/Release/Steam/UploadToSteam.ps1:46-60`, `Tools/Release/Steam/UploadToSteam.ps1:89-100`, `Release/Steam/STEAMWORKS_UPLOAD_AND_TESTING_INSTRUCTIONS.md:13-18`, `Release/Steam/STEAMWORKS_UPLOAD_AND_TESTING_INSTRUCTIONS.md:277-310` |
| TECH-PIPELINE-001 | PARTIAL | READ | CSV/JSON DataTable reload is script-owned; scripts folder has many setup/import/reload helpers, but pending issue records headless Interchange import crash risk. | `Scripts/SetupCombatRosterDataTables.py:2`, `Scripts/pending_issues_Scripts.md:10`, `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md:58-66` |
| TECH-PIPELINE-002 | PARTIAL | READ | PerformanceSystem is process-owned with schema/docs; runtime docs say schema v4 while schema families v1-v8 exist. | `PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md:6`, `PerformanceSystem/README.md:38`, `PerformanceSystem/README.md:58`, `PerformanceSystem/pending_issues_PerformanceSystem.md:3` |

## 11. Findings Register

| Finding ID | Affected Element | Status Tag | Evidence | Finding | Citations |
|---|---|---|---:|---|---|
| TFIND-001 | TECH-IDOL-002 | PARTIAL | STATIC_TRACE | Doc/code drift: traveler idol fields are documented as reserved/inert, but current combat code implements traveler delivery adapters and fallbacks. | `Source/T66/Data/T66DataTypes.h:1031`, `Source/T66/Data/T66DataTypes.h:1970`, `Source/T66/Gameplay/T66CombatComponent.cpp:4105-4371` |
| TFIND-002 | TECH-PROJECTILE-001 | PARTIAL | STATIC_TRACE | Outgoing-traveler damage authority is split between combat callbacks and pool fallback arrival damage; this is implemented but should be runtime-proven before assuming one authority. | `Source/T66/Gameplay/T66CombatComponent.cpp:4122-4318`, `Source/T66/Gameplay/T66OutgoingTravelerPoolSubsystem.cpp:152-216` |
| TFIND-003 | TECH-SAVE-002 | PARTIAL | STATIC_TRACE | Main saved snapshot omits live projectile/traveler pool state in the traced snapshot shape, so mid-combat resume is not proven complete. | `Source/T66/Core/T66RunSaveGame.h:333-597`, `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp:20-110` |
| TFIND-004 | TECH-PET-001 | PARTIAL | STATIC_TRACE | Pet data has boss-row fallback if `DT_Pets` is absent, which keeps runtime alive but blurs pet data authority. | `Source/T66/Core/T66GameInstance.cpp:736-795` |
| TFIND-005 | TECH-UI-002 | PARTIAL | STATIC_TRACE | Generic screen placeholder exists and can show "Screen Not Implemented" if a registered class lacks real content. | `Source/T66/UI/T66ScreenBase.cpp:39-61` |
| TFIND-006 | TECH-UI-003 | PARTIAL | STATIC_TRACE | Power-up purchase/unlock handlers can return handled on failed/no-subsystem paths without clear user-facing failure feedback in the handler. | `Source/T66/UI/Screens/T66PowerUpScreen.cpp:1264-1323` |
| TFIND-007 | TECH-UI-006 | PARTIAL | STATIC_TRACE | RetroFX close/cancel-style path commits dirty pending changes, so cancel behavior may not mean discard. | `Source/T66/UI/Screens/T66SettingsScreen_RetroFX.cpp:168-170`, `Source/T66/UI/Screens/T66SettingsScreen_RetroFX.cpp:416-424`, `Source/T66/UI/Screens/T66SettingsScreen.cpp:132-171` |
| TFIND-008 | TECH-UI-005 | PARTIAL | STATIC_TRACE | Safe Mode setting path applies when subsystem exists and returns handled without explicit status feedback in the handler. | `Source/T66/UI/Screens/T66SettingsScreen_Crashing.cpp:89-95` |
| TFIND-009 | TECH-UI-004 | PARTIAL | STATIC_TRACE | Bug report flow is local-first and optional-backend; it closes/logs after submit path and does not prove visible remote-submit status. | `Source/T66/UI/Screens/T66ReportBugScreen.cpp:289-315` |
| TFIND-010 | TECH-UI-007 | STUB | STATIC_TRACE | Achievement reference/secret rows still include placeholder behavior, although live rows are wired to player settings. | `Source/T66/UI/Screens/T66AchievementsScreen.cpp:1261-1271`, `Source/T66/UI/Screens/T66AchievementsScreen.cpp:2128-2136`, `Source/T66/UI/Screens/T66AchievementsScreen.cpp:2191` |
| TFIND-011 | TECH-UI-008 | STUB | STATIC_TRACE | Hero-selection Steam avatar work is explicitly placeholder. | `Source/T66/UI/HeroSelection/T66HeroSelectionScreen_Build.cpp:737-741` |
| TFIND-012 | TECH-LOOT-001 | PARTIAL | STATIC_TRACE | Loot boost mechanics commit, but focused reward presentation/toast/card lane is still recorded as missing. | `Source/T66/Gameplay/World/T66LootWheelInteractable.cpp:216-317`, `Source/T66/UI/pending_issues_UI.md:10-15` |
| TFIND-013 | TECH-STEAM-002 | COMPAT_LEGACY | STATIC_TRACE | Backend docs/source drift around AppID 480: diagnostics explicitly allows 480, but checked party-invite routes use default auth instead of explicit 480 allowlist. | `C:\UE\Backend\src\app\api\client-diagnostics\route.ts:46-47`, `C:\UE\Backend\src\app\api\party-invite\send\route.ts:15-16`, `Backend/BACKEND_SYSTEM_REFERENCE.md:165` |
| TFIND-014 | TECH-BUILD-003 | PARTIAL | READ | Steam build provenance can drift between local config, staged exe, uploaded build, installed build, and live branch; docs record uploaded builds that were not live. | `Release/Steam/STEAMWORKS_UPLOAD_AND_TESTING_INSTRUCTIONS.md:13-18`, `Release/Steam/STEAMWORKS_UPLOAD_AND_TESTING_INSTRUCTIONS.md:277-310` |
| TFIND-015 | TECH-INTEGRITY-001 | PARTIAL | STATIC_TRACE | Anti-cheat policy docs lag current source: current client submits integrity context and backend ranks/unranks from submitted integrity result. | `Source/T66/Core/Backend/T66BackendRunSerializer.cpp:518-561`, `C:\UE\Backend\src\app\api\submit-run\route.ts:328-353`, `C:\UE\Backend\src\app\api\submit-run\route.ts:604-660` |
| TFIND-016 | TECH-BACKEND-003 | PARTIAL | STATIC_TRACE | Co-op backend submission authenticates the host request, but this pass did not find per-member Steam-ticket proof for host-supplied party member summaries. | `Source/T66/Core/Backend/T66BackendRunApi.cpp:77-105`, `Source/T66/Core/Backend/T66BackendRunApi.cpp:227-275`, `C:\UE\Backend\src\app\api\submit-run\route.ts:127-139` |
| TFIND-017 | TECH-BACKEND-008 | UNKNOWN | READ | Backend client-config source exists, but deployed KV policy/current build restrictions were not verified. | `Source/T66/Core/Backend/T66BackendAccountApi.cpp:237`, `C:\UE\Backend\src\app\api\client-config\route.ts:33-144` |
| TFIND-018 | TECH-TD-007 | PARTIAL | STATIC_TRACE | TD save appears stage/profile-level and does not include enough traced board/entity/spawn-queue state for mid-wave reconstruction. | `Source/T66TD/Public/Core/T66TDRunSaveGame.h:9`, `Source/T66TD/Private/Core/T66TDSaveSubsystem.cpp:17-124`, `Source/T66TD/Private/UI/T66TDBattleScreen.cpp:1492` |
| TFIND-019 | TECH-MINI-008, TECH-TD-008, TECH-IDLE-005, TECH-DECK-005 | PARTIAL | STATIC_TRACE | Minigame daily leaderboard display is partial: daily builders/rows are empty or local-only while backend score hooks exist. | `Source/T66Mini/Private/UI/T66MiniMainMenuScreen.cpp:193`, `Source/T66TD/Private/UI/T66TDMainMenuScreen.cpp:171-176`, `Source/T66Idle/Private/UI/T66IdleMainMenuScreen.cpp:256-306`, `Source/T66Deck/Private/UI/T66DeckMainMenuScreen.cpp:344-407` |
| TFIND-020 | TECH-MINI-004, TECH-TD-004 | PARTIAL | READ | Mini/TD roster docs drift from current data counts; current Mini and TD hero CSVs show 12-row rosters. | `Content/Mini/Data/T66Mini_Heroes.csv:1-13`, `Content/TD/Data/T66TD_Heroes.csv:1`, `Gameplay/Minigames/TD/T66TD_Memory_Progression.md:28` |
| TFIND-021 | TECH-DECK-008 | ORPHAN_SUSPECT | STATIC_TRACE | Deck relic data and save fields exist, but acquisition/effect application was not traced through the current screen path. | `Content/Deck/Data/T66Deck_Relics.csv:3`, `Source/T66Deck/Public/Core/T66DeckRunSaveGame.h:86`, `Source/T66Deck/Private/UI/T66DeckMainMenuScreen.cpp:1327-1640` |
| TFIND-022 | TECH-DECK-010 | STUB | STATIC_TRACE | Deck Collection and Options are status-only/stub surfaces. | `Source/T66Deck/Private/UI/T66DeckMainMenuScreen.cpp:1834` |
| TFIND-023 | TECH-IDLE-009 | STUB | STATIC_TRACE | Idle Options is a no-op/status surface; docs still describe older/reserved state. | `Source/T66Idle/Private/UI/T66IdleMainMenuScreen.cpp:1471`, `Gameplay/Minigames/Idle/T66Idle_MasterImplementation.md:7-20` |
| TFIND-024 | TECH-TD-006 | PARTIAL | STATIC_TRACE | TD docs say implementation is deferred, while current source has active battle loop. This is doc drift, not a missing battle loop. | `Gameplay/Minigames/TD/T66TD_MasterImplementation.md:53`, `Source/T66TD/Private/UI/T66TDBattleScreen.cpp:712-2055` |
| TFIND-025 | TECH-TD-002, TECH-IDLE-001, TECH-DECK-001 | HIDDEN_RUNTIME | READ | TD/Idle/Deck rely on staged loose data/source assets rather than `/Game/TD`, `/Game/Idle`, `/Game/Deck` always-cook roots in current config. | `Config/DefaultGame.ini:14-29`, `Config/DefaultGame.ini:57-62` |
| TFIND-026 | TECH-VIDEO-001 | PARTIAL | READ | Runtime video catalog and source generation manifest differ in entry count in this pass, so video catalog parity is not guaranteed. | `RuntimeDependencies/T66/Video/frontend_videos.json:1`, `Video Generation/Manifests/frontend_videos.json:1` |
| TFIND-027 | TECH-PIPELINE-001 | PARTIAL | READ | Import tooling is broad and script-heavy; pending issue records headless Interchange import crash risk. | `Scripts/pending_issues_Scripts.md:10`, `Scripts/SetupCombatRosterDataTables.py:2` |
| TFIND-028 | TECH-ASSET-002, TECH-MODEL-001 | PARTIAL | READ | Generated and staged outputs are large: this pass observed heavy scale in `Saved\VideoCaptures`, `Saved\StagedBuilds`, `Model Generation`, and runtime dependency roots. | `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md:115`, `Model Generation/MODEL_GENERATION_AGENTS.md:21` |
| TFIND-029 | TECH-BUILD-002 | PARTIAL | READ | Multiple staged roots can coexist; active playable provenance should be tied to the shortcut target and expected exe path, not any sibling staged folder. | `Scripts/StageStandaloneBuild.ps1:364`, `Scripts/StageStandaloneBuild.ps1:399-405`, `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md:151-155` |
| TFIND-030 | TECH-PIPELINE-002 | PARTIAL | READ | PerformanceSystem docs/schema drift: README refers to current runtime schema v4 while schema families v1-v8 exist. | `PerformanceSystem/README.md:38`, `PerformanceSystem/README.md:58`, `PerformanceSystem/pending_issues_PerformanceSystem.md:3` |

## 12. Scale And Risk Flags

| Area | Evidence | Scale/Risk |
|---|---:|---|
| Main source | READ | `Source/T66` is large: 647 files and about 228k lines in this pass. Static trace is useful, but full behavioral proof requires staged/runtime follow-up. |
| Minigame source | READ | `T66Mini` is materially larger than the other minigames: 41 files and about 12k lines, versus TD 20 files/about 5k lines, Idle 12 files/about 2.4k lines, Deck 12 files/about 2.8k lines. |
| Data tables | READ | Main plus minigames have at least 59 CSV/JSON data sources in the checked roots, before generated manifests and runtime dependency JSON. |
| Backend | STATIC_TRACE | Backend source and in-repo docs diverge in AppID compatibility and anti-cheat policy wording. Live deployed Vercel/KV state remains `UNKNOWN` in this audit. |
| Build provenance | READ | Local staged exe, Steam installed build, uploaded build, and live branch can differ. Release docs explicitly record uploaded builds that were not live. |
| Runtime dependencies | READ | Loose runtime content is intentional and large enough to affect cook/stage provenance. TD/Idle/Deck depend on loose data/art routes. |
| Generated media/model output | READ | Model/video/capture/staged output scale is high; generated-output cleanup and ownership should be treated separately from runtime asset audit. |
| UI feedback | STATIC_TRACE | Several UI paths are "handled" locally while backend/subsystem status is optional or silent. This is not automatically broken, but it is a high-value runtime-verification target. |
| Minigame persistence | STATIC_TRACE | Mini save is deepest; TD mid-wave persistence, Idle profile snapshot use, and Deck map/relic systems are partial. |

## 13. Cross-Audit Reference Conventions

Use these suffixes to align with Content and Inventory audits:

- `TECH-HERO-001` <-> `CONTENT-HERO-001` <-> `INV-HERO-001`
- `TECH-COMPANION-001` <-> `CONTENT-COMPANION-001` <-> `INV-COMPANION-001`
- `TECH-ITEM-001` <-> `CONTENT-ITEM-001` <-> `INV-ITEM-001`
- `TECH-IDOL-001` <-> `CONTENT-IDOL-001` <-> `INV-IDOL-001`
- `TECH-WEAPON-001` <-> `CONTENT-WEAPON-001` <-> `INV-WEAPON-001`
- `TECH-ENEMY-001` <-> `CONTENT-ENEMY-001` <-> `INV-ENEMY-001`
- `TECH-BOSS-001` <-> `CONTENT-BOSS-001` <-> `INV-BOSS-001`
- `TECH-STAGE-001` <-> `CONTENT-STAGE-001` <-> `INV-STAGE-001`
- `TECH-ECONOMY-001` <-> `CONTENT-ECONOMY-001` <-> `INV-ECONOMY-001`
- `TECH-PET-001` <-> `CONTENT-PET-001` <-> `INV-PET-001`
- `TECH-UI-001` <-> `CONTENT-UI-001` <-> `INV-UI-001`
- `TECH-SAVE-001` <-> `CONTENT-SAVE-001` <-> `INV-SAVE-001`
- `TECH-BACKEND-001` <-> `CONTENT-BACKEND-001` <-> `INV-BACKEND-001`
- `TECH-BUILD-001` <-> `CONTENT-BUILD-001` <-> `INV-BUILD-001`
- `TECH-PIPELINE-001` <-> `CONTENT-PIPELINE-001` <-> `INV-PIPELINE-001`
- `TECH-MINI-*`, `TECH-TD-*`, `TECH-IDLE-*`, and `TECH-DECK-*` should align with corresponding Content/Inventory minigame suffixes.

## 14. Verification Performed And Skipped

Performed:

- `READ`: root process docs, audit docs, folder routers, release docs, config, `.uproject`, source files, pending issue files, data roots, backend source, pipeline docs, and staged/provenance docs.
- `STATIC_TRACE`: main runtime authority, combat/traveler/idol paths, save snapshot, backend serialization, UI handlers, and all four minigame modules.
- Delegated read-only sub-agent reports by area: backend/online, build/pipelines, Mini/TD, Idle/Deck, with main/UI reports partially lost in disconnect and replaced by local targeted traces.
- Claude independent answer before drafting and Claude cross-review after drafting.

Skipped by scope:

- No Unreal editor/game launch.
- No packaged build refresh.
- No runtime UI click-through.
- No full backend/Vercel/Steamworks live verification.
- No item-by-item, boost-by-boost, or minigame-row runtime proof.
- No fixes and no git operations.


</codex_draft>
