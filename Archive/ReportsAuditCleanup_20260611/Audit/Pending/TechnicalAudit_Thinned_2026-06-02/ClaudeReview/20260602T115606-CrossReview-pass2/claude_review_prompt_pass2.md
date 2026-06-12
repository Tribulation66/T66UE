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
- Original prompt path: C:\UE\T66\Audit\Pending\TechnicalAudit_Thinned_2026-06-02\original_prompt.md
- Codex draft path: C:\UE\T66\Audit\Pending\TechnicalAudit_Thinned_2026-06-02\TECHNICAL_AUDIT_THINNED.md
- Independent answer path: C:\UE\T66\Audit\Pending\TechnicalAudit_Thinned_2026-06-02\ClaudeReview\20260602T113505-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
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


</original_prompt>

<validator_independent_answer>
Key structural claims confirmed. Producing the Validator answer.

Result: OK

## Independent Answer

This is a descriptive, read-only, flag-don't-fix audit re-run. It is fully within what the Operator/Validator models can execute by inspecting the live tree — no user decision, approval, or unavailable tool is required. Result is OK; the items below are scope/quality guardrails for Codex to satisfy, not user blockers.

Spot-checks against the live repo already corroborate the prompt's premises:

- **Module collapse confirmed.** `T66.uproject:6-17` declares only `T66` (Runtime) and `T66Editor` (Editor). No `T66Mini`/`T66Versus`/`T66Buried`/`DailyDescent`/`T66Idle`/`T66Deck` modules remain, and no `*.Build.cs` references them. The audit should still trace each former `/Script/T66Mini` soft-load route to its shell class and explicitly flag any route string that resolves to nothing (orphan).
- **Shelved gate exists and is referenced widely.** `Source\T66\Core\T66ShelvedFeatureGate.cpp` + `Public\Core\T66ShelvedFeatureGate.h` exist, with ~15 call sites across GameMode_Tower, WidgetGameRegistry, SessionSubsystem, GameInstance, MainMenuScreen, DirectEntry, PlayerController_Frontend, UIManagerReleaseVariant, backend APIs, etc. Audit must verify each site *defers* to the gate rather than re-implementing visibility/entry logic, and flag any independent gating residue. Note `T66DeprecatedFeatureSettings.cpp` co-exists — confirm the `SHELVED` vs `DEPRECATED` distinction is real in code, not just naming.

For the audit to be acceptable, Codex must:
1. Honor the schema additions verbatim — add `SHELVED` as a distinct lifecycle tag and map the four demoted shells (minigame shells, T66Versus, T66Buried, Daily Descent) to it.
2. Provide real `file:line` citations with evidence tiers; default to `READ`/`STATIC_TRACE` and never claim `RUNTIME_VERIFIED` since the prompt forbids runtime verification.
3. Carry forward every listed open finding (damage authority split, save-snapshot omissions, silently-handled UI handlers, AppID 480 residue, co-op ticket, video catalog drift, pet boss-row fallback, idol stale comments, build provenance) and mark each as still-current / corrected / superseded with fresh evidence — not copy them forward unverified.
4. Run the area-staged sub-agents and keep strictly read-only / no-git.

## Evidence Checked

- `T66.uproject:1-65` — only T66 + T66Editor modules; minigame modules gone.
- Grep for `T66Mini|T66Versus|T66Buried|DailyDescent|T66Idle|T66Deck` across `*.Build.cs` — no matches (scaffolding removed).
- Grep `T66ShelvedFeatureGate` — 17 hits; gate source/header present plus ~15 runtime call sites.
- Presence of `T66DeprecatedFeatureSettings.cpp` alongside the shelved gate (confirms two distinct lifecycle concepts).

## Questions Or Blockers

None requiring the user. The task is self-contained and read-only; the Operator can produce the document directly.

## Caveats

- I did not exhaustively trace casino-registry 4-game wiring, the 4-slot weighted shop, vendor floor/guarantee, companion cage unlock chain, or deletion of per-difficulty rarity getters — Codex must confirm each new path and prove the old paths are gone, not just absent from a shallow grep.
- "No dangling references" for soft-load routes needs a route-by-route trace (route string → shell class resolution), which my spot-check did not perform.
- Carry-forward findings must be re-verified against the *current* tree; several (e.g., idol stale comments) may now be corrected, so they should not be restated without fresh line citations.

</validator_independent_answer>

<codex_draft>
# T66 Technical Audit - Thinned Tree

Date: 2026-06-02  
Operator: Codex  
Validator: Claude Code  
Mode: descriptive, read-only, no git, no runtime verification  
Scope: current `C:\UE\T66` tree after the demolition/correction pass, including the now-shelved minigame surfaces, T66Versus, T66Buried expectation, Daily Descent, casino/shop/vendor/companion-cage changes, known-dead removals, and carry-forward technical findings.

## Schema

Lifecycle tags used here are the shared audit tokens plus the requested addition: `ACTIVE`, `DEMO_GATED`, `HIDDEN_RUNTIME`, `PARTIAL`, `DEPRECATED`, `COMPAT_LEGACY`, `BROKEN`, `STUB`, `ORPHAN_SUSPECT`, `UNKNOWN`, `SHELVED`.

Evidence tiers: `READ`, `STATIC_TRACE`, `PRIOR_ARTIFACT`, `RUNTIME_VERIFIED`. This pass uses `READ` and `STATIC_TRACE`; no claim in this document is `RUNTIME_VERIFIED`.

Element IDs use `TECH-{AREA}-{NNN}` and findings use `TFIND-{NNN}`. Area suffixes are intended to line up with Content/Inventory subjects: `BUILD`, `MINI`, `UI`, `ECONOMY`, `SAVE`, `BACKEND`, `PIPELINE`, `PET`, `IDOL`, `COMBAT`, `VIDEO`.

## Scope And Exclusions

This audit describes how the current tree is built and wired. It flags dead ends, doc drift, compatibility residue, and suspicious static traces; it does not fix them and does not prove runtime behavior. No Unreal editor launch, packaged launch, Steam/KV/network query, build, cook, runtime save/load, or git operation was run.

Mini/minigame scope is included because the prompt explicitly includes it. In the current thinned tree, "full depth" for minigames means tracing the removed module/content roots, retained compatibility route names, shell screens, data residue, and gate behavior. There are no separate `T66Mini`, `T66TD`, `T66Idle`, or `T66Deck` runtime modules left to audit as independent runtime architectures.

## Question Set And Answers

| Question | Answer | Evidence |
|---|---|---|
| Which runtime modules are actually declared now? | Only `T66` Runtime and `T66Editor` Editor are declared. Former Mini/TD/Idle/Deck modules are gone from the project module list and from source scaffolding. | `C:\UE\T66\T66.uproject:6-17`; `C:\UE\T66\Source\T66.Target.cs:13`; `C:\UE\T66\Source\T66Editor.Target.cs:13-14`; `C:\UE\T66\Source\T66\T66.Build.cs:6`; `C:\UE\T66\Source\T66Editor\T66Editor.Build.cs:6` |
| Are former minigame soft routes dangling? | No `/Script/T66Mini`, `/Script/T66TD`, `/Script/T66Idle`, `/Script/T66Deck`, `/Script/T66Versus`, or `/Script/T66Buried` soft paths were found by static scan. Current compatibility screen enum routes resolve inside the main `T66` module. | Current route enum values: `C:\UE\T66\Source\T66\UI\T66UITypes.h:45-63`; route resolver maps former screens to the shelved screen at `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Frontend.cpp:704-721`; only `/Script/T66` redirects are present at `C:\UE\T66\Config\DefaultEngine.ini:2-14` |
| What is the single source of truth for shelved feature entry? | `FT66ShelvedFeatureGate` owns the feature booleans and screen allowance logic. Current booleans for MinigameBundle, VersusArcade, DailyDescent, and ArcadeInteractables are all false. | `C:\UE\T66\Source\T66\Public\Core\T66ShelvedFeatureGate.h:8-23`; `C:\UE\T66\Source\T66\Core\T66ShelvedFeatureGate.cpp:7-10`; `C:\UE\T66\Source\T66\Core\T66ShelvedFeatureGate.cpp:55-72` |
| Are old fragmented gates still acting independently? | Most visible entry points now defer to the central gate. Compatibility booleans still exist in deprecated settings, but wrapper code calls the gate first. A residue remains: arcade interactable data still preloads and helper code can load its DataTable even when arcade entry is shelved. | `C:\UE\T66\Source\T66\Core\T66DeprecatedFeatureSettings.cpp:9-23`; `C:\UE\T66\Config\DefaultGame.ini:54-56`; arcade preload at `C:\UE\T66\Source\T66\Core\T66GameInstance.cpp:181`, `:355`, `:400`, `:947`; direct helper load at `C:\UE\T66\Source\T66\UI\WidgetGames\T66WidgetGameArcadeHelpers.cpp:30` |
| What is left of the four minigames? | Their modules/content roots are gone; route enum values and UI classes remain as compatibility shells, routed to `UT66ShelvedFeatureScreen`. Residual docs/manifests/mockup UI assets remain. | `C:\UE\T66\Source\T66\Core\T66ShelvedFeatureGate.cpp:16-31`; `C:\UE\T66\Source\T66\UI\Screens\T66ShelvedFeatureScreen.h:10`; `C:\UE\T66\Source\T66\UI\Screens\T66ShelvedFeatureScreen.cpp:31-42`; docs still name modes at `C:\UE\T66\Gameplay\README.md:10` and `C:\UE\T66\Gameplay\Minigames\README.md:11-14` |
| What is left of T66Versus? | `UT66VersusArcadeScreen` still exists as a shell/source screen, but the current screen resolver maps `VersusMainMenu` to the shelved screen and `VersusArcade` is false in the gate. | `C:\UE\T66\Source\T66\UI\Screens\T66VersusArcadeScreen.h:11`; `C:\UE\T66\Source\T66\UI\Screens\T66VersusArcadeScreen.cpp:61-65`; `C:\UE\T66\Source\T66\Core\T66ShelvedFeatureGate.cpp:8`; `C:\UE\T66\Source\T66\Core\T66ShelvedFeatureGate.cpp:62-64`; `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Frontend.cpp:704-721` |
| What is left of T66Buried? | The prompt names T66Buried as shelved, but static scan found no live `T66Buried`, `Buried`, screen enum, source path, content root, config route, or `/Script/T66Buried` symbol. This is flagged as a missing expected shelved surface, not confirmed runtime code. | Negative static trace across `Source`, `Config`, `Content`, `SourceAssets`, `Gameplay`, `UI`; no file-line exists for an absent symbol. Related route list lacks Buried at `C:\UE\T66\Source\T66\UI\T66UITypes.h:45-63` |
| What is left of Daily Descent? | Daily Descent UI/run/backend code remains, but entry/start paths check the shelved gate and return early. It is parked, not deleted. | `C:\UE\T66\Source\T66\UI\Screens\T66DailyClimbScreen.h:14`; `C:\UE\T66\Source\T66\UI\Screens\T66DailyClimbScreen.cpp:59-63`; `C:\UE\T66\Source\T66\Core\T66GameInstance.cpp:1497-1505`; `C:\UE\T66\Source\T66\Core\T66GameInstance.cpp:1540`; `C:\UE\T66\Source\T66\Core\Backend\T66BackendRunApi.cpp:299`, `:326`, `:364` |
| What casino games are active? | The casino registry and UI now expose four games: Coin Flip, Guess the Cup, Pick Longest/Shortest Stick, and Find Joker. Old source/widget/enums for BlackJack and RockPaperScissors were not found in current source/config/UI/gameplay scans, but localization strings remain. | registry at `C:\UE\T66\Source\T66\UI\WidgetGames\T66WidgetGameRegistry.cpp:51`; demo allow list at `C:\UE\T66\Config\DefaultDemoMode.ini:22-25`; UI enum/cards/handlers at `C:\UE\T66\Source\T66\UI\T66CasinoGamblerTabWidget.h:38-45`, `C:\UE\T66\Source\T66\UI\T66CasinoGamblerTabWidget.cpp:187-219`, `:441-481`; localization residue at `C:\UE\T66\Content\Localization\T66\T66.manifest:3446-3452`, `:4007-4012` |
| How does shop stock work now? | Shop stock is four weighted slots. Each slot rolls rarity from static weights and then chooses a unique template; vendor/buyback UI sizes itself from the four-slot constants. | `C:\UE\T66\Source\T66\Core\T66RunStateSubsystem.h:124-135`; rarity roll at `C:\UE\T66\Source\T66\Core\RunState\T66RunStateSubsystem_EconomyInventory.cpp:173-188`; stock generation loop at `C:\UE\T66\Source\T66\Core\RunState\T66RunStateSubsystem_EconomyInventory.cpp:303-381`; vendor UI arrays at `C:\UE\T66\Source\T66\UI\T66CasinoVendorTabWidget.cpp:423-442`, `:568-586` |
| Is vendor every floor wired? | World interactable spawn code attempts to spawn a vendor on each mob floor and logs/checks the expected `GuaranteedPerMobFloor` rule. | `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_WorldInteractables.cpp:1356-1372`; summary check at `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_WorldInteractables.cpp:1571-1587`; tower proof check at `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_Tower.cpp:552-594` |
| Is companion cage unlock wired? | Boss flow maps stages to companion IDs, spawns caged recruitables, frees them on boss clear, and the companion actor grants unlock on interaction after being freed. | stage map at `C:\UE\T66\Source\T66\Gameplay\T66GameMode.cpp:11`; cage spawn at `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_BossFlow.cpp:316`; cage state at `C:\UE\T66\Source\T66\Gameplay\T66RecruitableCompanion.cpp:171-205`; unlock interaction at `C:\UE\T66\Source\T66\Gameplay\T66RecruitableCompanion.cpp:235-258`; selection gate at `C:\UE\T66\Source\T66\UI\Screens\T66CompanionSelectionScreen.cpp:676` |
| Were per-difficulty rarity fields/getters deleted? | Scoped answer: item/shop per-difficulty rarity fields/getters are gone from the main economy path; generic loot/chest/crate/wheel per-difficulty rarity and idol/weapon rarity tuning still exist and should not be reported as deleted. | item/shop flat weights at `C:\UE\T66\Source\T66\Core\T66RunStateSubsystem.h:126-135`; item schema row at `C:\UE\T66\Source\T66\Data\T66DataTypes.h:1090`; `Items.csv` schema at `C:\UE\T66\Content\Data\Items.csv:1`; generic rarity getters at `C:\UE\T66\Source\T66\Core\T66PlayerExperienceSubSystem.h:72-109`, `:173-186`; implementations at `C:\UE\T66\Source\T66\Core\T66PlayerExperienceSubSystem.cpp:150-230`; config at `C:\UE\T66\Config\DefaultT66PlayerExperience.ini:8-12` |

## Architecture Map

| Element ID | Area | Status | Evidence | Technical description |
|---|---|---:|---|---|
| TECH-BUILD-001 | BUILD | ACTIVE | READ | Unreal project module architecture is now a main runtime module plus editor module: `T66` and `T66Editor` in `C:\UE\T66\T66.uproject:6-17`. Game target adds `T66` only at `C:\UE\T66\Source\T66.Target.cs:13`; editor target adds `T66` and `T66Editor` at `C:\UE\T66\Source\T66Editor.Target.cs:13-14`. |
| TECH-BUILD-002 | BUILD | ACTIVE | READ | Plugin/tooling remains Unreal-based with Steam online, Python/editor scripting, modeling tools, procedural mesh, movie render, Electra, and related plugins enabled in `C:\UE\T66\T66.uproject:20-64`. |
| TECH-MINI-001 | MINI | SHELVED | READ / STATIC_TRACE | Former Mini, TD, Idle, Deck screens remain as `ET66ScreenType` values, but `FT66ShelvedFeatureGate` classifies them as the MinigameBundle and the frontend resolver returns `UT66ShelvedFeatureScreen`. Evidence: `C:\UE\T66\Source\T66\UI\T66UITypes.h:45-63`; `C:\UE\T66\Source\T66\Core\T66ShelvedFeatureGate.cpp:16-31`; `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Frontend.cpp:704-721`. |
| TECH-MINI-002 | MINI | SHELVED | READ | The shared shell screen is implemented as `UT66ShelvedFeatureScreen`; the screen copy explicitly says `FEATURE SHELVED` and describes the feature as disabled/preserved. Evidence: `C:\UE\T66\Source\T66\UI\Screens\T66ShelvedFeatureScreen.h:10`; `C:\UE\T66\Source\T66\UI\Screens\T66ShelvedFeatureScreen.cpp:31-42`. |
| TECH-MINI-003 | MINI | SHELVED | READ / STATIC_TRACE | `T66Mini`, `T66TD`, `T66Idle`, and `T66Deck` are no longer independent runtime modules. Static source listing found no `Source\T66Mini`, `Source\T66TD`, `Source\T66Idle`, or `Source\T66Deck`; current Build.cs files are only `T66.Build.cs` and `T66Editor.Build.cs`. Evidence: project module list at `C:\UE\T66\T66.uproject:6-17`; Build.cs entries at `C:\UE\T66\Source\T66\T66.Build.cs:6` and `C:\UE\T66\Source\T66Editor\T66Editor.Build.cs:6`. |
| TECH-MINI-004 | MINI | SHELVED | STATIC_TRACE | Canonical minigame content/source roots are absent (`Content\Mini`, `Content\Minigames`, `Content\Data\Mini`, `SourceAssets\Mini`, `Source\T66Mini`, `Source\T66\Minigames`), while docs/manifests/mockup UI assets remain. Docs still describe Mini/TD/Deck/Idle ownership at `C:\UE\T66\Gameplay\README.md:10` and `C:\UE\T66\Gameplay\Minigames\README.md:11-14`. |
| TECH-MINI-005 | MINI | SHELVED | READ | T66Versus is parked. `UT66VersusArcadeScreen` still exists, but `VersusArcade` is false and `VersusMainMenu` is routed to the shelved screen. Evidence: `C:\UE\T66\Source\T66\UI\Screens\T66VersusArcadeScreen.h:11`; `C:\UE\T66\Source\T66\UI\Screens\T66VersusArcadeScreen.cpp:61-65`; `C:\UE\T66\Source\T66\Core\T66ShelvedFeatureGate.cpp:8`, `:62-64`; `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Frontend.cpp:704-721`. |
| TECH-MINI-006 | MINI | SHELVED / UNKNOWN | STATIC_TRACE | The user-specified T66Buried shelf surface is not found in the live tree. It should be treated as a shelved concept for cross-audit naming, but actual implementation is unknown/absent: no `T66Buried`, `Buried`, route enum, content root, source root, config path, or `/Script/T66Buried` symbol was found. Related route list lacks Buried at `C:\UE\T66\Source\T66\UI\T66UITypes.h:45-63`. Finding: `TFIND-006`. |
| TECH-MINI-007 | MINI | SHELVED | READ / STATIC_TRACE | Daily Descent is shelved, not deleted. UI and run/backend code remains, while gate checks prevent entry/start/submit. Evidence: `C:\UE\T66\Source\T66\UI\Screens\T66DailyClimbScreen.cpp:59-63`; `C:\UE\T66\Source\T66\Core\T66ShelvedFeatureGate.cpp:67-77`; `C:\UE\T66\Source\T66\Core\T66GameInstance.cpp:1497-1505`; `C:\UE\T66\Source\T66\Core\Backend\T66BackendRunApi.cpp:299`, `:326`, `:364`. |
| TECH-UI-001 | UI | ACTIVE | STATIC_TRACE | Frontend route resolution and direct-entry routes still accept former Mini/TD/Idle/Deck/Versus/Daily names, but they check `FT66ShelvedFeatureGate::IsScreenAllowed` and resolve shelved screens. Evidence: `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Frontend.cpp:379-386`, `:545-615`, `:704-721`; direct-entry resolver at `C:\UE\T66\Source\T66\Core\T66DirectEntry.cpp:30-32`, `:311-376`. |
| TECH-UI-002 | UI | SHELVED | STATIC_TRACE | Release-variant/UI manager gating defers screen allowance to the central gate. Evidence: `C:\UE\T66\Source\T66\UI\T66UIManagerReleaseVariant.cpp:46`; Daily/minigame screen cases in `C:\UE\T66\Source\T66\UI\T66UIManagerReleaseVariant.cpp:17-31`, `:67`. |
| TECH-UI-003 | UI | SHELVED | READ | Main menu/top bar visually suppress or block shelved entries through the gate. Evidence: minigames tab uses `AreMinigamesEnabled` at `C:\UE\T66\Source\T66\UI\T66FrontendTopBarWidget.cpp:939`; Daily button availability at `C:\UE\T66\Source\T66\UI\Screens\T66MainMenuScreen.cpp:573-575`; Daily click guard at `C:\UE\T66\Source\T66\UI\Screens\T66MainMenuScreen.cpp:2048-2062`; Minigames click guard at `C:\UE\T66\Source\T66\UI\Screens\T66MainMenuScreen.cpp:2072`. |
| TECH-UI-004 | UI | PARTIAL | STATIC_TRACE | Several handlers still return `FReply::Handled()` without user-visible failure/status outcomes. These are not new thinning regressions, but they remain current: power-up purchases on failed unlock, RetroFX cancel committing pending changes, Safe Mode and bug-report handlers. Findings: `TFIND-014`, `TFIND-015`, `TFIND-016`. |
| TECH-COMBAT-001 | COMBAT | ACTIVE | READ / STATIC_TRACE | Combat/traveler delivery uses `T66CombatComponent` callbacks and `T66OutgoingTravelerPoolSubsystem`. The old "idol traveler inert" comment appears corrected: current WorldSystemsAPI comment describes implemented outgoing traveler pool support, and delivery rows exist in `Idols.csv`. Evidence: `C:\UE\T66\Source\T66\Gameplay\T66WorldSystemsAPI.h:10-19`; `C:\UE\T66\Content\Data\Idols.csv:1-4`; traveler combat paths at `C:\UE\T66\Source\T66\Gameplay\T66CombatComponent.cpp:2631-2744`, `:4105-4364`. |
| TECH-COMBAT-002 | COMBAT | PARTIAL | STATIC_TRACE | Outgoing-traveler damage authority remains split by code path, but the old double-application risk appears narrowed: pool fallback damage is disabled when an arrival callback is bound. Evidence: `C:\UE\T66\Source\T66\Gameplay\T66OutgoingTravelerPoolSubsystem.cpp:215`, `:693-710`, `:718-780`; combat callback damage at `C:\UE\T66\Source\T66\Gameplay\T66CombatComponent.cpp:2695-2724`, `:4122-4136`, `:4191-4199`, `:4337-4364`. Finding: `TFIND-010`. |
| TECH-SAVE-001 | SAVE | PARTIAL | STATIC_TRACE | Run save snapshots serialize run-state fields, inventory/shop/buyback/etc., but do not serialize live projectile/traveler manager state. Evidence: snapshot fields at `C:\UE\T66\Source\T66\Core\T66RunSaveGame.h:331-580`; export/import at `C:\UE\T66\Source\T66\Core\RunState\T66RunStateSubsystem_Snapshot.cpp:11-138`, `:141-260`; live projectile/traveler state in managers at `C:\UE\T66\Source\T66\Gameplay\T66ProjectileManagerSubsystem.h:207`, `:213` and `C:\UE\T66\Source\T66\Gameplay\T66OutgoingTravelerPoolSubsystem.h:347-350`. Finding: `TFIND-011`. |
| TECH-PET-001 | PET | PARTIAL | STATIC_TRACE | Pets still prefer `PetsDataTable` but fall back to boss rows; no `DT_Pets.uasset` or `Pets.csv` was found in `Content\Data` by static listing. Evidence: pet soft ref at `C:\UE\T66\Source\T66\Core\T66GameInstance.h:72-74`; fallback comments at `C:\UE\T66\Source\T66\Core\T66GameInstance.h:513-515`, `:577-579`; runtime lookup/fallback at `C:\UE\T66\Source\T66\Core\T66GameInstance.cpp:730-768`; pet IDs mapped to boss row names at `C:\UE\T66\Source\T66\Core\T66GameInstance.cpp:1367-1375`. Finding: `TFIND-012`. |
| TECH-ECONOMY-001 | ECONOMY | ACTIVE | STATIC_TRACE | Casino registry is four games and matches demo allow-list: CoinFlip, GuessTheCup, PickLongestShortestStick, FindJoker. Evidence: `C:\UE\T66\Source\T66\UI\WidgetGames\T66WidgetGameRegistry.cpp:51`; `C:\UE\T66\Config\DefaultDemoMode.ini:22-25`; current casino UI enum and cards at `C:\UE\T66\Source\T66\UI\T66CasinoGamblerTabWidget.h:38-45`, `C:\UE\T66\Source\T66\UI\T66CasinoGamblerTabWidget.cpp:187-219`. |
| TECH-ECONOMY-002 | ECONOMY | ACTIVE | STATIC_TRACE | Casino result serialization keeps legacy `gambler_results` naming, but maps to the current four game values. This is compatibility naming, not evidence of old games still active. Evidence: `C:\UE\T66\Source\T66\Core\Backend\T66BackendRunSerializer.cpp:140`, `:669`; parser at `C:\UE\T66\Source\T66\Core\Backend\T66BackendRunSummaryParser.cpp:72`, `:462`. |
| TECH-ECONOMY-003 | ECONOMY | ACTIVE | STATIC_TRACE | Four-slot shop/buyback UI is driven from `UT66RunStateSubsystem::ShopDisplaySlotCount` and `BuybackDisplaySlotCount`. Evidence: `C:\UE\T66\Source\T66\Core\T66RunStateSubsystem.h:124-135`; `C:\UE\T66\Source\T66\UI\T66CasinoVendorTabWidget.cpp:423-442`, `:568-586`. |
| TECH-ECONOMY-004 | ECONOMY | ACTIVE | STATIC_TRACE | Vendor guaranteed per mob floor is implemented in world interactable spawn and checked in summary/proof logs. Evidence: `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_WorldInteractables.cpp:1356-1372`, `:1571-1587`; `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_Tower.cpp:552-594`. |
| TECH-ECONOMY-005 | ECONOMY | ACTIVE | STATIC_TRACE | Companion cage unlock chain is wired from stage mapping to caged actor spawn to free-on-clear to unlock-on-interact to selection gating. Evidence: `C:\UE\T66\Source\T66\Gameplay\T66GameMode.cpp:11`; `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_BossFlow.cpp:316`; `C:\UE\T66\Source\T66\Gameplay\T66RecruitableCompanion.cpp:171-205`, `:235-258`; `C:\UE\T66\Source\T66\Core\T66CompanionUnlockSubsystem.cpp:40`; `C:\UE\T66\Source\T66\UI\Screens\T66CompanionSelectionScreen.cpp:676`. |
| TECH-ECONOMY-006 | ECONOMY | COMPAT_LEGACY | STATIC_TRACE | Retired item IDs are blocked on add and stripped from loaded snapshots. This is compatibility protection, not active economy content. Evidence: `C:\UE\T66\Source\T66\Core\RunState\T66RunStateSubsystem_Private.h:256`; `C:\UE\T66\Source\T66\Core\RunState\T66RunStateSubsystem_EconomyInventory.cpp:797`; `C:\UE\T66\Source\T66\Core\RunState\T66RunStateSubsystem_Snapshot.cpp:184`. |
| TECH-ECONOMY-007 | ECONOMY | PARTIAL | READ | Active reward-only `Item_VendorToken` still uses Backrooms quick-revive icon fields, so the data is functional-looking but art/data provenance is placeholder. Evidence: `C:\UE\T66\Content\Data\Items.csv:31`; `C:\UE\T66\Source\T66\Core\RunState\T66RunStateSubsystem_Private.h:251`. Finding: `TFIND-021`. |
| TECH-BACKEND-001 | BACKEND | ACTIVE | STATIC_TRACE | Run-summary serialization writes anti-cheat context, integrity context, score budget context, and current casino result names. Evidence: integrity serialization at `C:\UE\T66\Source\T66\Core\Backend\T66BackendRunSerializer.cpp:514-555`; backend schema/evaluation accepts integrity context at `C:\UE\Backend\src\lib\schemas.ts:203-218` and `C:\UE\Backend\src\app\api\submit-run\route.ts:328-353`, `:603-635`. |
| TECH-BACKEND-002 | BACKEND | PARTIAL | STATIC_TRACE | Active Steam AppID config is now `4464300`, but `480` residue remains in diagnostics and an old staged hotfix root; backend docs still describe invite routes allowing `480` even though source now uses default auth. Evidence: `C:\UE\T66\Config\DefaultEngine.ini:306-312`; `C:\UE\T66\steam_appid.txt:1`; backend default auth at `C:\UE\Backend\src\lib\steam.ts:42-49`; diagnostics allow list at `C:\UE\Backend\src\app\api\client-diagnostics\route.ts:45-48`; stale doc at `C:\UE\T66\Backend\BACKEND_SYSTEM_REFERENCE.md:160-165`. Finding: `TFIND-023`. |
| TECH-BACKEND-003 | BACKEND | PARTIAL | STATIC_TRACE | Anti-cheat policy docs drift from current source: docs label `integrity_context` future/missing while client/backend now serialize, capture, schema, and evaluate it. Evidence: doc at `C:\UE\T66\Backend\Anti Cheat\ANTI_CHEAT_POLICY_REFERENCE.md:164`, `:325-329`; client baseline/final capture at `C:\UE\T66\Source\T66\Core\T66RunIntegritySubsystem.cpp:84-100`, `:121-160`; serializer at `C:\UE\T66\Source\T66\Core\Backend\T66BackendRunSerializer.cpp:514-555`; backend schema/eval at `C:\UE\Backend\src\lib\schemas.ts:203-218`; `C:\UE\Backend\src\app\api\submit-run\route.ts:328-353`, `:603-635`. Finding: `TFIND-024`. |
| TECH-BACKEND-004 | BACKEND | UNKNOWN | STATIC_TRACE | Co-op per-member summaries exist, but static trace found only one cached Steam ticket/header per submit request and no per-member ticket field in schema. Evidence: client ticket requirement at `C:\UE\T66\Source\T66\Core\Backend\T66BackendRunApi.cpp:48-50`, `:290-293`; single auth header at `C:\UE\T66\Source\T66\Core\Backend\T66BackendSubsystem.cpp:446-450`; member payload at `C:\UE\T66\Source\T66\Core\Backend\T66BackendRunApi.cpp:254-273`; backend schema at `C:\UE\Backend\src\lib\schemas.ts:239-264`; backend request auth at `C:\UE\Backend\src\app\api\submit-run\route.ts:126-139`, `:274-280`, `:444-454`. Finding: `TFIND-025`. |
| TECH-BACKEND-005 | BACKEND | UNKNOWN | STATIC_TRACE | Client-config KV route and parser exist, but live KV was not queried and frontend startup currently disables validation and proceeds. Evidence: route key/fallback at `C:\UE\Backend\src\app\api\client-config\route.ts:33-63`; GET response at `C:\UE\Backend\src\app\api\client-config\route.ts:78-107`; KV env at `C:\UE\Backend\src\lib\kv.ts:6-13`; client parser at `C:\UE\T66\Source\T66\Core\Backend\T66BackendAccountApi.cpp:237-259`, `:402-442`; disabled startup gate at `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Frontend.cpp:1212-1241`; block handler at `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Frontend.cpp:1266-1277`. Finding: `TFIND-026`. |
| TECH-VIDEO-001 | VIDEO | PARTIAL | STATIC_TRACE | Runtime video catalog, source manifest, job manifest, README, and CSV inventory disagree. Runtime parsed count is 34 hero, 16 companion, 1 main menu; source manifest is 48 hero, 32 companion, 1 main menu; jobs are 48 hero, 32 companion, 0 main menu; README says 48 hero, 48 companion, 1 main menu. Evidence: README claim at `C:\UE\T66\Video Generation\README.md:7-10`; ownership note at `C:\UE\T66\Video Generation\VIDEO_GENERATION_AGENTS.md:21-22`; runtime examples at `C:\UE\T66\RuntimeDependencies\T66\Video\frontend_videos.json:109-112`, `:139-160`, `:166-203`, `:370-373`; source examples at `C:\UE\T66\Video Generation\Manifests\frontend_videos.json:184-192`, `:298-318`, `:534-537`; active CSV counts from `C:\UE\T66\Content\Data\Heroes.csv:1`, `:13` and `C:\UE\T66\Content\Data\Companions.csv:1`, `:17`. Finding: `TFIND-027`. |
| TECH-PIPELINE-001 | PIPELINE | PARTIAL | READ / STATIC_TRACE | Build/stage provenance remains ambiguous because multiple staged roots exist and older staged roots carry AppID residue. Active staging script expects `Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`; Steam upload docs distinguish staged source root from live Steam branch. Evidence: `C:\UE\T66\Scripts\StageStandaloneBuild.ps1:342-347`, `:364-405`; upload docs at `C:\UE\T66\Release\Steam\STEAMWORKS_UPLOAD_AND_TESTING_INSTRUCTIONS.md:46-57`, `:13-18`, `:296-308`; upload wrapper at `C:\UE\T66\Tools\Release\Steam\UploadToSteam.ps1:1-8`, `:57-60`, `:89-100`. Finding: `TFIND-028`. |
| TECH-PIPELINE-002 | PIPELINE | ORPHAN_SUSPECT | READ / STATIC_TRACE | Arcade selector image literal points to `arcade_selector_front_machine.png`, while static path checks found the existing runtime file as `arcade_selector_front_cabinet.png`. Evidence: code literal at `C:\UE\T66\Source\T66\UI\T66ArcadeSelectionWidget.cpp:277`; remap rules at `C:\UE\T66\Source\T66\UI\Style\T66RuntimeUITextureAccess.cpp:282-295`, `:329-331`. Finding: `TFIND-029`. |

## Data And Asset Flow

`Content\Data` remains the main source of gameplay data consumed through Unreal DataTables or JSON-to-DataTable import assets. `UT66GameInstance` owns many soft table paths: weapons, combat VFX bindings, bosses, boss attacks/definitions/hazards/movement, stages, enemies, status effects, boss encounters, arcade interactables, NPCs, and unique enemies (`C:\UE\T66\Source\T66\Core\T66GameInstance.cpp:169-185`). This confirms the current architecture is still data-table driven even after minigame thinning.

For runtime combat/progression, the flow is:

1. Source CSV/JSON in `Content\Data` such as `Idols.csv`, `Items.csv`, `Heroes.csv`, `Companions.csv`, `ArcadeInteractables.json`.
2. Unreal DataTable assets referenced by `UT66GameInstance` and systems.
3. Runtime owners such as `UT66RunStateSubsystem`, `UT66PlayerExperienceSubSystem`, `AT66GameMode`, `UT66GameInstance`, `UT66CombatComponent`, and backend serializers.
4. UI surfaces such as casino/vendor tabs, main menu/top bar, companion selection, power-up/settings/report screens.
5. Save/run-summary/backend surfaces through `UT66RunSaveGame`, snapshot import/export, `T66BackendRunSerializer`, and backend submit-run routes.

The thinned-tree caveat is that arcade/minigame data still has retained descriptors/assets even where gameplay entry is parked. `ArcadeInteractables.json` still contains 14 rows, and enum names mirror those game types (`C:\UE\T66\Content\Data\ArcadeInteractables.json:3-24`, `:74-81`, `:123-130`, `:165-172`, `:210-217`, `:242-249`, `:274-281`, `:306-313`, `:338-345`, `:370-377`, `:402-409`, `:434-441`, `:466-473`, `:498-505`; `C:\UE\T66\Source\T66\Gameplay\T66ArcadeInteractableTypes.h:19-37`).

## Runtime Systems And Authority

### Combat, Travelers, Projectiles, Idols

The current traveler implementation is no longer inert. `Idols.csv` has `Delivery` rows including traveler delivery (`C:\UE\T66\Content\Data\Idols.csv:1-4`), the public API comment now describes outgoing traveler pool support (`C:\UE\T66\Source\T66\Gameplay\T66WorldSystemsAPI.h:10-19`), and combat component traveler paths exist for idol delivery and callbacks (`C:\UE\T66\Source\T66\Gameplay\T66CombatComponent.cpp:2631-2744`, `:4105-4364`). Old TFIND-001 should be closed as corrected for current code, while preserving doc-drift caution generally.

Damage authority remains split in two implementation owners: combat callback code and outgoing traveler pool fallback arrival damage. Current pool code sets `bApplyDamageOnArrival` false when an arrival delegate is bound (`C:\UE\T66\Source\T66\Gameplay\T66OutgoingTravelerPoolSubsystem.cpp:215`), calls the callback at `:693-700`, then uses fallback damage only after that path at `:701-710` and `:718-780`. This narrows the old damage double-application risk but leaves dual-authority logic to describe and later runtime-test (`TFIND-010`).

Save snapshots still omit live projectile/traveler manager state. The run snapshot struct covers run-state fields (`C:\UE\T66\Source\T66\Core\T66RunSaveGame.h:331-580`) and import/export mirrors those fields (`C:\UE\T66\Source\T66\Core\RunState\T66RunStateSubsystem_Snapshot.cpp:11-138`, `:141-260`), while active projectile/traveler state is held in gameplay managers (`C:\UE\T66\Source\T66\Gameplay\T66ProjectileManagerSubsystem.h:207`, `:213`; `C:\UE\T66\Source\T66\Gameplay\T66OutgoingTravelerPoolSubsystem.h:347-350`). This is unchanged as a descriptive risk (`TFIND-011`).

### Economy, Casino, Vendor, Companion Cage

The casino runtime is now four games. The widget registry has four descriptors, and the demo allow-list has the same four IDs (`C:\UE\T66\Source\T66\UI\WidgetGames\T66WidgetGameRegistry.cpp:51`; `C:\UE\T66\Config\DefaultDemoMode.ini:22-25`). The casino tab presents four cards and opens four handlers (`C:\UE\T66\Source\T66\UI\T66CasinoGamblerTabWidget.cpp:187-219`, `:441-481`). Backend run-summary compatibility still uses `gambler_results`, but maps values to current API names (`C:\UE\T66\Source\T66\Core\Backend\T66BackendRunSerializer.cpp:140`, `:669`; `C:\UE\T66\Source\T66\Core\Backend\T66BackendRunSummaryParser.cpp:72`, `:462`).

The shop runtime is four weighted stock slots. Constants live in `UT66RunStateSubsystem` (`C:\UE\T66\Source\T66\Core\T66RunStateSubsystem.h:124-135`); rarity is rolled from fixed weights (`C:\UE\T66\Source\T66\Core\RunState\T66RunStateSubsystem_EconomyInventory.cpp:173-188`); stock generation iterates four slots and chooses unique candidates (`C:\UE\T66\Source\T66\Core\RunState\T66RunStateSubsystem_EconomyInventory.cpp:303-381`). Vendor UI sizes shop/buyback cards from the constants (`C:\UE\T66\Source\T66\UI\T66CasinoVendorTabWidget.cpp:423-442`, `:568-586`).

Vendor per floor is wired in world interactables: the code spawns vendors for mob floors and logs/checks `GuaranteedPerMobFloor` (`C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_WorldInteractables.cpp:1356-1372`, `:1571-1587`). This is separate from `Item_VendorToken`, which is a reward/upgrading token path and still has placeholder icon data (`C:\UE\T66\Source\T66\Gameplay\T66VendorBoss.cpp:136`; `C:\UE\T66\Source\T66\Core\T66AchievementsSubsystem.cpp:1420`; `C:\UE\T66\Content\Data\Items.csv:31`).

Companion cage unlock is a live chain: stage mapping chooses companion IDs (`C:\UE\T66\Source\T66\Gameplay\T66GameMode.cpp:11`), boss flow spawns caged companions (`C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_BossFlow.cpp:316`), the actor locks/frees its cage state (`C:\UE\T66\Source\T66\Gameplay\T66RecruitableCompanion.cpp:171-205`), then grants unlock on interaction (`C:\UE\T66\Source\T66\Gameplay\T66RecruitableCompanion.cpp:235-258`). Companion selection consults the unlock subsystem (`C:\UE\T66\Source\T66\UI\Screens\T66CompanionSelectionScreen.cpp:676`).

### Pets And Boss Fallback

Pets remain partial because the code supports a `PetsDataTable`, but current static data does not provide `DT_Pets` or `Pets.csv`. `UT66GameInstance` has a pet soft ref (`C:\UE\T66\Source\T66\Core\T66GameInstance.h:72-74`), documents fallback from bosses (`C:\UE\T66\Source\T66\Core\T66GameInstance.h:513-515`, `:577-579`), tries pet rows first, then synthesizes from boss data (`C:\UE\T66\Source\T66\Core\T66GameInstance.cpp:730-768`). Pet IDs still map to boss row names (`C:\UE\T66\Source\T66\Core\T66GameInstance.cpp:1367-1375`). Carry forward as `TFIND-012`.

## UI-To-Logic Wiring

Shelved screens: frontend, direct entry, top bar, release-variant checks, backend daily routes, and game/session daily paths generally defer to `FT66ShelvedFeatureGate`. Evidence includes frontend alias resolution (`C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Frontend.cpp:545-615`), direct-entry alias resolution (`C:\UE\T66\Source\T66\Core\T66DirectEntry.cpp:311-376`), UI manager release variant checks (`C:\UE\T66\Source\T66\UI\T66UIManagerReleaseVariant.cpp:46`), daily run start guard (`C:\UE\T66\Source\T66\Core\T66GameInstance.cpp:1497-1505`), and daily backend early returns (`C:\UE\T66\Source\T66\Core\Backend\T66BackendRunApi.cpp:299`, `:326`, `:364`).

UI handlers with silent-handled behavior remain:

- Power-up and hero-selection purchase/unlock handlers return `FReply::Handled()` even when unlock/purchase returns false. Evidence: `C:\UE\T66\Source\T66\UI\Screens\T66PowerUpScreen.cpp:1264-1271`, `:1274-1284`, `:1287-1323`; `C:\UE\T66\Source\T66\UI\Screens\T66HeroSelectionScreen.cpp:128-147`; subsystem false paths at `C:\UE\T66\Source\T66\Core\T66BuffSubsystem.cpp:1174-1205`.
- RetroFX Cancel is wired to the normal close handler, and close/deactivate/destruct commit pending RetroFX changes. Evidence: `C:\UE\T66\Source\T66\UI\Screens\Settings\T66SettingsScreen_RetroFX.cpp:165-171`, `:384-424`; `C:\UE\T66\Source\T66\UI\Screens\T66SettingsScreen.cpp:132-141`, `:163-172`.
- Safe Mode and bug report handlers return handled without status/submit result surfacing. Evidence: `C:\UE\T66\Source\T66\UI\Screens\Settings\T66SettingsScreen_Crashing.cpp:77-95`; `C:\UE\T66\Source\T66\UI\Screens\T66ReportBugScreen.cpp:208-209`, `:305-329`.

## Backend And Online Services

The game-side backend code serializes run summaries and anti-cheat/integrity context (`C:\UE\T66\Source\T66\Core\Backend\T66BackendRunSerializer.cpp:514-555`). The external backend source under `C:\UE\Backend` validates Steam auth once per submit request, parses run members, and evaluates integrity context (`C:\UE\Backend\src\app\api\submit-run\route.ts:126-139`, `:274-280`, `:328-353`, `:444-454`, `:603-635`; schema at `C:\UE\Backend\src\lib\schemas.ts:203-218`, `:239-264`).

Current active Steam AppID configuration is `4464300` in game config and root `steam_appid.txt` (`C:\UE\T66\Config\DefaultEngine.ini:306-312`; `C:\UE\T66\steam_appid.txt:1`). Backend default auth appends `env.STEAM_APP_ID` (`C:\UE\Backend\src\lib\steam.ts:42-49`). Residual `480` appears in diagnostics and old staged roots, and docs mention invite routes allowing `480` even though current invite route source uses default auth (`C:\UE\Backend\src\app\api\client-diagnostics\route.ts:45-48`; `C:\UE\T66\Backend\BACKEND_SYSTEM_REFERENCE.md:160-165`; invite source at `C:\UE\Backend\src\app\api\party-invite\send\route.ts:15-16`, `pending\route.ts:5-6`, `respond\route.ts:11-12`).

Client-config KV is implemented but unverified. Backend route uses `t66:client-build-policy:${appId}:${branch}` and falls back to default (`C:\UE\Backend\src\app\api\client-config\route.ts:33-63`), while the client parser exists (`C:\UE\T66\Source\T66\Core\Backend\T66BackendAccountApi.cpp:237-259`, `:402-442`). Startup currently logs and proceeds without client-config validation (`C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Frontend.cpp:1212-1241`), so this remains `UNKNOWN/PARTIAL` until a live KV check and startup invocation path are proven.

## Build, Cook, Stage, Import, Generated Media

Staging remains anchored on `Scripts\StageStandaloneBuild.ps1`, which expects the staged executable under `Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe` (`C:\UE\T66\Scripts\StageStandaloneBuild.ps1:342-347`, `:364-405`). Steam upload docs point at `C:\UE\T66\Saved\StagedBuilds\Windows` as upload root and warn that uploaded builds may not be live until branch switching (`C:\UE\T66\Release\Steam\STEAMWORKS_UPLOAD_AND_TESTING_INSTRUCTIONS.md:46-57`, `:13-18`, `:296-308`). Upload wrapper defaults to `app_build_4464300_root.vdf` and removes `steam_appid.txt` (`C:\UE\T66\Tools\Release\Steam\UploadToSteam.ps1:1-8`, `:57-60`, `:89-100`). Static pass found multiple staged roots (`Windows`, `WindowsHotfix`, `WindowsTemp`); old `WindowsHotfix` carries `steam_appid.txt` value `480`, while current active root did not show a staged `steam_appid.txt` in this scan.

Video generation remains drifted. Runtime catalog and source manifests do not match each other or README claims. Runtime catalog count from static parse is 34 hero, 16 companion, 1 main menu; source manifest is 48 hero, 32 companion, 1 main menu; job manifest is 48 hero, 32 companion, 0 main menu; README claims 48 hero, 48 companion, 1 main menu (`C:\UE\T66\Video Generation\README.md:7-10`). Inventory data currently has 12 hero rows and 16 companion rows (`C:\UE\T66\Content\Data\Heroes.csv:1`, `:13`; `C:\UE\T66\Content\Data\Companions.csv:1`, `:17`). This is `TFIND-027`.

Data import/reload ownership remains script/document driven. `UT66GameInstance` still owns the table references, while generated media has explicit source/runtime ownership rules (`C:\UE\T66\Video Generation\VIDEO_GENERATION_AGENTS.md:21-22`). This audit did not run import/reload scripts.

## Known-Dead / Removal Confirmation

| Subject | Current result | Status | Evidence |
|---|---|---:|---|
| Minigame module scaffolding | Removed from project/module/build scaffolding. | SHELVED / confirmed removed scaffolding | `C:\UE\T66\T66.uproject:6-17`; `C:\UE\T66\Source\T66\T66.Build.cs:6`; `C:\UE\T66\Source\T66Editor\T66Editor.Build.cs:6`; no former module source dirs in static scan. |
| Former `/Script/T66Mini` etc. soft-load routes | No stale demoted-module script soft paths found. Compatibility routes are enum/screen paths inside `T66`. | SHELVED | `C:\UE\T66\Source\T66\UI\T66UITypes.h:45-63`; `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Frontend.cpp:704-721`; redirects only `/Script/T66` at `C:\UE\T66\Config\DefaultEngine.ini:2-14`. |
| Arcade runtime tower/popup/descriptors | Not deleted; shelved/gated. Arcade descriptors, interactable runtime code, tower spawn path, and popup path remain but gates disable entry. | SHELVED / ORPHAN_SUSPECT residue | descriptors at `C:\UE\T66\Content\Data\ArcadeInteractables.json:3-24`; enum at `C:\UE\T66\Source\T66\Gameplay\T66ArcadeInteractableTypes.h:19-37`; config disables at `C:\UE\T66\Config\DefaultGame.ini:54-56`; gate wrappers at `C:\UE\T66\Source\T66\Core\T66DeprecatedFeatureSettings.cpp:9-23`; tower spawn at `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_WorldInteractables.cpp:1339-1352`; popup at `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Overlays.cpp:7938-8045`. |
| Old casino BlackJack/RockPaperScissors source/widgets/enums | Removed from current source/config/UI/gameplay scans; localization residue remains. | COMPAT_LEGACY / ORPHAN_SUSPECT | current casino enum at `C:\UE\T66\Source\T66\Core\T66RunSaveGame.h:206-213`; current four widgets/cards at `C:\UE\T66\Source\T66\UI\T66CasinoGamblerTabWidget.cpp:136-150`, `:187-219`, `:441-481`; localization residue at `C:\UE\T66\Content\Localization\T66\T66.manifest:3446-3452`, `:4007-4012`; `C:\UE\T66\Content\Localization\T66\en\T66.archive:2865-2868`, `:3324-3327`. |
| Rarity relic | Exact `RarityRelic`/`RelicRarity` symbols not found. Relic-named arcade `RelicStack` and rarity-themed source-generation residue remain. | DEPRECATED / ORPHAN_SUSPECT residue | `RelicStack` descriptor at `C:\UE\T66\Content\Data\ArcadeInteractables.json:19`, `:338-345`; enum at `C:\UE\T66\Source\T66\Gameplay\T66ArcadeInteractableTypes.h:31`; `SourceAssets\ToonStyle\ImageGen\IdolRarity_20260523` exists by static count. |
| Per-difficulty item/shop rarity fields/getters | Main item/shop rarity path removed; generic loot/idol/weapon per-difficulty rarity remains live. | ACTIVE / scoped removal | item schema at `C:\UE\T66\Source\T66\Data\T66DataTypes.h:1090`; `C:\UE\T66\Content\Data\Items.csv:1`; shop flat weights at `C:\UE\T66\Source\T66\Core\T66RunStateSubsystem.h:126-135`; generic rarity still at `C:\UE\T66\Source\T66\Core\T66PlayerExperienceSubSystem.h:72-109`, `:173-186`. |

## Findings

| Finding ID | Element | Status | Evidence | Description |
|---|---|---:|---|---|
| TFIND-001 | TECH-IDOL-001 / TECH-COMBAT-001 | ACTIVE corrected | STATIC_TRACE | Old idol traveler stale-comment finding appears corrected in current code: API comments and runtime paths describe traveler pool support, and traveler delivery rows are present. Evidence: `C:\UE\T66\Source\T66\Gameplay\T66WorldSystemsAPI.h:10-19`; `C:\UE\T66\Content\Data\Idols.csv:1-4`; `C:\UE\T66\Source\T66\Gameplay\T66CombatComponent.cpp:2631-2744`, `:4105-4364`. |
| TFIND-006 | TECH-MINI-006 | UNKNOWN / ORPHAN_SUSPECT | STATIC_TRACE | T66Buried was expected to be a shelved shell, but no live symbol/path/route was found. Treat the product concept as shelved for cross-audit continuity, but implementation is absent in the static tree. |
| TFIND-010 | TECH-COMBAT-002 | PARTIAL | STATIC_TRACE | Outgoing traveler damage authority is still split between combat callback and pool fallback arrival damage. Current code suppresses fallback damage when callback is bound, narrowing the old risk but leaving dual ownership to runtime-verify later. |
| TFIND-011 | TECH-SAVE-001 | PARTIAL | STATIC_TRACE | Save snapshot omits live projectile/traveler manager state; save/load cannot be described as preserving active in-flight combat entities based on static code. |
| TFIND-012 | TECH-PET-001 | PARTIAL | STATIC_TRACE | Pets still have boss-row fallback/no current `DT_Pets` data found in `Content\Data`. |
| TFIND-013 | TECH-MINI-004 | ORPHAN_SUSPECT | READ / STATIC_TRACE | Minigame docs/manifests/mockup UI assets remain after module/content roots were deleted. This is expected shelf residue but should not be mistaken for active runtime. Evidence: `C:\UE\T66\Gameplay\README.md:10`; `C:\UE\T66\Gameplay\Minigames\README.md:11-14`, `:27`. |
| TFIND-014 | TECH-UI-004 | PARTIAL | STATIC_TRACE | Power-up purchase/unlock UI returns handled on failure. Evidence: `C:\UE\T66\Source\T66\UI\Screens\T66PowerUpScreen.cpp:1264-1271`, `:1274-1284`, `:1287-1323`; `C:\UE\T66\Source\T66\UI\Screens\T66HeroSelectionScreen.cpp:128-147`; `C:\UE\T66\Source\T66\Core\T66BuffSubsystem.cpp:1174-1205`. |
| TFIND-015 | TECH-UI-004 | PARTIAL | STATIC_TRACE | RetroFX cancel commits pending settings through normal close/deactivate/destruct path. Evidence: `C:\UE\T66\Source\T66\UI\Screens\Settings\T66SettingsScreen_RetroFX.cpp:165-171`, `:384-424`; `C:\UE\T66\Source\T66\UI\Screens\T66SettingsScreen.cpp:132-141`, `:163-172`. |
| TFIND-016 | TECH-UI-004 | PARTIAL | STATIC_TRACE | Safe Mode and bug-report handlers return handled without visible status/failure result. Evidence: `C:\UE\T66\Source\T66\UI\Screens\Settings\T66SettingsScreen_Crashing.cpp:77-95`; `C:\UE\T66\Source\T66\UI\Screens\T66ReportBugScreen.cpp:208-209`, `:305-329`. |
| TFIND-017 | TECH-MINI-001 / TECH-UI-001 | SHELVED | STATIC_TRACE | Former minigame routes are retained and routed to a shared shelved screen, not removed. This is intentional compatibility/shelving, not active runtime. |
| TFIND-018 | TECH-MINI-005 | SHELVED | STATIC_TRACE | T66Versus source screen remains, but entry is blocked by gate and route resolver maps to shelved screen. |
| TFIND-019 | TECH-MINI-007 | SHELVED | STATIC_TRACE | Daily Descent UI/run/backend code remains but all checked entry/start/submit paths are gated or short-circuited. |
| TFIND-020 | TECH-MINI-004 | ORPHAN_SUSPECT | STATIC_TRACE | Arcade data/table/assets remain and can preload/resolve while arcade entry is shelved. This is not a proven runtime bypass, but it is retained data/workload after feature shelving. Evidence: `C:\UE\T66\Source\T66\Core\T66GameInstance.cpp:181`, `:355`, `:400`, `:947`; `C:\UE\T66\Source\T66\UI\WidgetGames\T66WidgetGameArcadeHelpers.cpp:30`. |
| TFIND-021 | TECH-ECONOMY-007 | PARTIAL | READ | `Item_VendorToken` uses Backrooms quick-revive icon paths, likely placeholder art/data residue. Evidence: `C:\UE\T66\Content\Data\Items.csv:31`; `C:\UE\T66\Source\T66\Core\RunState\T66RunStateSubsystem_Private.h:251`. |
| TFIND-022 | TECH-ECONOMY-002 | COMPAT_LEGACY | STATIC_TRACE | Backend field name `gambler_results` persists even after old casino games were removed. Values map to current four casino games, so classify as compatibility naming rather than active old-game residue. |
| TFIND-023 | TECH-BACKEND-002 | PARTIAL | STATIC_TRACE | Steam AppID `480` residue remains in diagnostics and old staged root while active config is `4464300`; backend docs drift from current invite-route source. |
| TFIND-024 | TECH-BACKEND-003 | PARTIAL | STATIC_TRACE | Anti-cheat policy docs are stale on `integrity_context`; code/backend now implement it. |
| TFIND-025 | TECH-BACKEND-004 | UNKNOWN | STATIC_TRACE | Co-op per-member ticket proof not found. Static source shows one auth header/ticket for submit request and no member ticket field in schema. |
| TFIND-026 | TECH-BACKEND-005 | UNKNOWN / PARTIAL | STATIC_TRACE | Client-config KV route/client parser exist, but live KV is unverified and frontend startup currently disables validation. |
| TFIND-027 | TECH-VIDEO-001 | PARTIAL | STATIC_TRACE | Runtime/source/job/README video catalog counts drift: runtime `34/16/1`, source `48/32/1`, jobs `48/32/0`, README `48/48/1`, active CSV inventory `12/16`. |
| TFIND-028 | TECH-PIPELINE-001 | PARTIAL | READ / STATIC_TRACE | Build/stage provenance remains ambiguous due to multiple staged roots and AppID residue in older staged root. |
| TFIND-029 | TECH-PIPELINE-002 | ORPHAN_SUSPECT | STATIC_TRACE | Arcade selector code references `arcade_selector_front_machine.png`, while path/remap scan indicates existing file is `arcade_selector_front_cabinet.png`. |
| TFIND-030 | TECH-ECONOMY-001 | ORPHAN_SUSPECT | READ | Old BlackJack/RockPaperScissors source/widget names appear removed from current source/config/UI/gameplay, but localization manifests/archives still carry old text. |
| TFIND-031 | TECH-ECONOMY-006 | COMPAT_LEGACY | STATIC_TRACE | Deprecated secondary stat enum entries remain in enum-space and are excluded by live-stat filtering. Evidence: `C:\UE\T66\Source\T66\Data\T66DataTypes.h:1002`, `:1051`. |
| TFIND-032 | TECH-MINI-004 | SHELVED / ORPHAN_SUSPECT | READ | Removed item sprites are explicitly shelved pending Mini-inclusive ownership/reference audit. Evidence: `C:\UE\T66\Content\Data\pending_issues_Data.md:17`. |

## Cross-Audit References

- `TECH-MINI-*` should line up with Content/Inventory rows for former Mini, TD, Idle, Deck, Versus, Buried, Daily Descent, and arcade shell/residue.
- `TECH-ECONOMY-*` should line up with Content/Inventory rows for casino games, shop slots, vendor token, vendor per floor, companion cage unlock, retired item IDs, and localization residue.
- `TECH-COMBAT-*` and `TECH-IDOL-*` should line up with idol traveler delivery and outgoing traveler projectile/content rows.
- `TECH-PET-*` should line up with pet inventory/data rows and boss-row fallback content rows.
- `TECH-BACKEND-*` should line up with any inventory of backend source, docs, deployment/config files, and run-summary schemas.
- `TECH-VIDEO-*` should line up with generated media inventory and hero/companion video counts.
- `TECH-PIPELINE-*` should line up with build/stage/import/output inventories.

## Verification And Review

Verification performed: read-only source/file inspection, static route tracing, targeted file reads, static count/absence checks by subagents, and Claude independent validation of the audit plan. No runtime verification was performed by request. No git operations were run.

Subagents were staged by area:

- Architecture/shelved gate/module thinning.
- Economy/casino/shop/vendor/companion/rarity.
- Carry-forward combat/save/UI/pets/idol.
- Backend/online/build/video.
- Content/data deletion and known-dead residue.

Claude independent answer artifact: `C:\UE\T66\Audit\Pending\TechnicalAudit_Thinned_2026-06-02\ClaudeReview\20260602T113505-IndependentAnswer-pass1\claude_review_pass1.md`.


</codex_draft>
