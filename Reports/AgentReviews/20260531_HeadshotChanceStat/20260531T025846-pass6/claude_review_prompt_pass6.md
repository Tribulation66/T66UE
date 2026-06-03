You are Claude reviewing a Codex draft for the T66 Unreal project.

Rules:
- Start your response immediately with the result line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the draft below.
- Treat Codex as the Operator/final router and you as the oversight reviewer.
- Look for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

The first non-empty line of your review must be exactly one of these four lines:
Result: OK
Result: NEEDS_FIX
Result: ASK_USER
Result: BLOCKED

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the draft/result is usable as-is or with only the included wording edits.
- NEEDS_FIX: Codex should fix concrete issues before answering or continuing.
- ASK_USER: only the user can decide the next path.
- BLOCKED: a hard prerequisite or external state prevents safe progress.

Review scope:
- Draft path: C:\UE\T66\Reports\AgentReviews\20260531_HeadshotChanceStat\completion_packet.md
- Output scope: oversight review of the draft below only.

<review_packet>
# Headshot Chance Stat Completion Packet

## Task Contract

Working task: Replace `CritDamage` as a live item/stat with `HeadshotChance`, keep critical hits fixed at `2x`, and make Headshot Chance stun hit enemies.

Operator: Codex.

Validator: Claude.

Scope: Core stat/data/item/runtime/UI/backend/docs/smoke paths for the main T66 gameplay systems. Mini/minigame implementation is excluded.

Stop condition: Data, runtime, UI, backend compatibility, DataTable reload, editor smokes, staged standalone refresh, staged smokes, and final Validator review are complete or a blocker is reported.

## Process Notes

- PPF ceremony skipped: this is a gameplay stat/data/runtime migration, not a solved-category visual/media/import/VFX/UI-fidelity production task where QA cares about a replicated authoring method.
- Previous planning review artifacts:
  - `Reports/AgentReviews/20260531_HeadshotChanceStat/review_packet_pass1.md`
  - `Reports/AgentReviews/20260531_HeadshotChanceStat/20260531T010246-pass1/claude_review_pass1.md` (`Verdict: REVISE`)
  - `Reports/AgentReviews/20260531_HeadshotChanceStat/review_packet_pass2.md`
  - `Reports/AgentReviews/20260531_HeadshotChanceStat/20260531T010432-pass2/claude_review_pass2.md` (`Verdict: APPROVE`)
  - `Reports/AgentReviews/20260531_HeadshotChanceStat/20260531T020335-pass3/claude_review_pass3.md` (`Verdict: REVISE`)
  - `Reports/AgentReviews/20260531_HeadshotChanceStat/20260531T021710-pass4/claude_review_pass4.md` (`Verdict: APPROVE`)
  - `Reports/AgentReviews/20260531_HeadshotChanceStat/20260531T023425-pass5/claude_review_pass5.md` (`Verdict: NEEDS_FIX`)
- This packet supersedes pass 5 after closing the remaining evidence gaps with clean DataTable reload evidence, explicit `CritDamage=1.0` legacy-boundary proof, stale buff-type skip proof, clamped `HeadshotChance=1.000` proof, live auto-attack `TryFire()` proof, and refreshed staged smokes.
- Pass-3 revision decisions:
  - Legacy `CritDamage` values in `[0.0, 1.0]` are treated as old percentage-shaped Headshot Chance compatibility values.
  - Legacy `CritDamage` values outside `[0.0, 1.0]` are treated as old crit-damage multiplier-shaped values and ignored so they cannot become out-of-range Headshot Chance.
  - The `CritDamage=1.0` boundary is intentionally inclusive for percentage-shaped compatibility. Current authored hero `BaseCritDamage` values before this migration were `1.5` for Heroes 1-7 and 9-12 and `1.8` for Hero 8; current authored hero multiplier snapshots therefore land above the boundary and are ignored as multiplier-shaped values.
  - Headshot stun is intended to affect boss targets; non-boss immunity remains specific to OHKO sources (`Execute`, `Assassinate`, `Crush`).
  - All hero `BaseHeadshotChance` values are intentionally `0.0` for now; Headshot Chance comes from items, level-up Accuracy-family scaling, and drugs until per-hero tuning is authored.

## Implemented Changes

- Data:
  - `Content/Data/Heroes.csv`: renamed `BaseCritDamage` to `BaseHeadshotChance`; authored hero values as `0.0`.
  - `Content/Data/Items.csv`: removed `Item_CritDamage`; added `Item_Headshot` as Accuracy/HeadshotChance.
  - `Content/Data/PlayerExperience.json`: added data-driven `HeadshotChancePerBonusPoint` and `HeadshotStunDurationSeconds`.
  - `Content/Data/PlayerExperience.json`: authored previously missing LootWheel tuning fields so the source reloads without default-fill import problems.
  - `/Game/Data/DT_Heroes`, `/Game/Data/DT_Items`, and `/Game/Data/DT_PlayerExperience` were reloaded from source data.
- Stat/runtime:
  - `ET66SecondaryStatType::HeadshotChance` added after the existing enum values; `CritDamage` is deprecated/compatibility-only.
  - Accuracy-family order is now `CritChance`, `HeadshotChance`, `AttackRange`, `Execute`.
  - `GetCritDamageMultiplier()` always returns `2.0`.
  - `GetSecondaryStatValue(CritDamage)` returns `2.0` as compatibility behavior.
  - `GetHeadshotChance01()` resolves from hero base, level-up bonus points, item bonuses, drug multipliers, and Accuracy-family multiplier, then clamps to `[0.0, 1.0]`.
  - `GetHeadshotStunDurationSeconds()` resolves from data.
  - Added a non-shipping-only `DebugAddPersistentSecondaryStatBonusTenths(...)` helper so smoke verification can drive the same stat formula over `1.0` and prove clamping without changing production behavior.
- Combat:
  - Auto-attacks now roll Headshot Chance after successful damage application and apply a data-driven stun to hit enemies/mobs/bosses.
  - The live path is `UT66CombatComponent::TryFire()` -> `ApplyResolvedAutoAttackDamage(...)` -> `ApplyDamageToTargetHandle(...)` -> `TryApplyHeadshotStunToTargetHandle(...)`.
  - Non-shipping automation hook `DebugApplyHeadshotStunForAutomation` verifies the stun path.
  - Headshot stun path refreshes its run-state subsystem cache if needed before resolving chance/duration.
- UI/localization:
  - Stats panel, hero selection stat snapshot, power-up/drug display, run summary, temporary buff slug, and item localization use Headshot Chance instead of Crit Damage.
- Backend/save compatibility:
  - Serializer has explicit `HeadshotChance` key support.
  - Serializer skips deprecated `CritDamage` if a stale snapshot map contains it.
  - Parser maps `HeadshotChance` and in-range legacy `CritDamage` backend keys into live `HeadshotChance`.
  - Parser ignores legacy `CritDamage` values outside `[0.0, 1.0]` so old crit-damage multipliers do not import as invalid Headshot Chance.
- UI compatibility:
  - Deprecated `CritDamage` localization now resolves to Headshot Chance text if a stale caller reaches it, preventing a visible stale "Crit Damage 2.0" line.
- Docs:
  - `Gameplay/Stats/MASTER_STATS.md` and `Gameplay/Combat/MASTER_COMBAT.md` describe fixed `2x` crit, live Headshot Chance, stun behavior, deprecated CritDamage, and current data/runtime consumers.
- Pending issue:
  - `Content/Data/pending_issues_Data.md` documents that `Item_Headshot` still uses legacy `Item_CritDamage_*` sprite assets because new Headshot-specific sprites do not exist yet.

## Out Of Scope / Exclusions

- No Mini/minigame code or design work was intentionally edited. A pre-existing unrelated `Source/T66/UI/Screens/T66MinigamesScreen.cpp` modification is visible in the worktree and is not part of this packet.
- No new Headshot item sprite assets were created.
- No release commit/tag/push was requested.

## Verification Evidence

- Static/data checks:
  - `Content/Data/PlayerExperience.json` parsed successfully with `ConvertFrom-Json`.
  - `Content/Data/PlayerExperience.json` has all five difficulty rows populated with `LootWheelsPerStage`, `LootWheelRarityWeights`, `LootWheelRewardWeightsByRarity`, and `LootWheelGoldRangeByRarity`.
  - Pre-migration authored hero `BaseCritDamage` values from `git show HEAD:Content/Data/Heroes.csv` were `1.5` for Heroes 1-7 and 9-12 and `1.8` for Hero 8, so current authored hero multiplier snapshots are above the inclusive `1.0` compatibility boundary.
  - Pre-migration runtime formula was `HeroBaseCritDamage = FMath::Max(1.f, HD.BaseCritDamage)` and `GetSecondaryStatValue(CritDamage) = FMath::Max(1.f, (HeroBaseCritDamage + BonusPoints * 0.05f) * M * AccuracyMult)`, confirming authored crit-damage bases were multiplier-shaped.
  - CSV verification confirmed:
    - `Heroes=12`
    - `HasBaseHeadshot=True`
    - `HasBaseCrit=False`
    - `HeadshotItem=1`
    - `CritDamageItem=0`
    - `HeadshotSecondary=HeadshotChance`
    - `HeadshotPrimary=Accuracy`
  - DataTable export confirmed `Item_Headshot` row has `PrimaryStatType=Accuracy` and `SecondaryStatType=HeadshotChance`.
- Data reload:
  - `Scripts/ReloadHeadshotStatDataTablesAndExit.py` reloaded `DT_Items`, `DT_Heroes`, and `DT_PlayerExperience`.
  - `Reports/AgentReviews/20260531_HeadshotChanceStat/reload_headshot_datatables_final_clean.log`:
    - `Imported DataTable 'DT_Items' - 0 Problems`
    - `Imported DataTable 'DT_Heroes' - 0 Problems`
    - `Imported DataTable 'DT_PlayerExperience' - 0 Problems`
    - `ReloadHeadshotStatDataTablesAndExit DONE`
- Build:
  - `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReload`: succeeded.
  - `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development`: succeeded after the final data/code state and produced `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Editor runtime smokes:
  - `Reports/AgentReviews/20260531_HeadshotChanceStat/stat_pipeline_smoke_final.json`: `ok=true`.
    - Proves Headshot item replaces Crit Damage item.
    - Proves Crit damage is fixed at `2x` and `CritDamage` is not live.
    - Proves Headshot item and Headshot drug raise Headshot Chance.
    - Proves deprecated `CritDamage` temporary buff type is skipped: `Listed=0 Owned=0 Selected=0`.
    - Proves Headshot Chance can stun a hit target: `Chance=0.500 Duration=0.750 Applied=1 StunRemaining=0.750`.
    - Proves Headshot Chance stuns boss targets: `Applied=1 BossAlive=1 Duration=0.750`.
    - Proves Headshot Chance clamps to `1.000` after capped inventory, persistent secondary bonus, and selected drug multiplier.
    - Proves the live auto-attack path selected real weapon `Hero_1_black_pierce`, invoked `PerformAutomationAutoAttackNow()` / `TryFire()`, damaged the target, and applied Headshot stun: `Chance=1.000 StunRemaining=0.750 HP=99905.0`.
    - Proves legacy backend `CritDamage` key maps to live `HeadshotChance`.
    - Proves legacy multiplier-shaped `CritDamage=1.5` does not become Headshot Chance.
    - Proves legacy boundary `CritDamage=1.0` maps intentionally to `HeadshotChance=1.000`.
    - Proves Headshot Chance serializes by string key, not ordinal, and skips stale `CritDamage`.
    - Proves deprecated CritDamage UI text resolves to Headshot text.
  - `Reports/AgentReviews/20260531_HeadshotChanceStat/item_taxonomy_smoke_final.json`: `ok=true`.
    - Proves Execute/Assassinate/Crush reject bosses and allow miniboss enemies/mobs.
    - Proves Loot Bag and Loot Wheel reward improvements.
    - Proves retired item IDs skip inventory, including `Item_CritDamage`.
- Staged standalone:
  - Shortcut `C:\UE\T66\T66 Standalone.lnk` target verified as `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`; target exists.
  - `Reports/AgentReviews/20260531_HeadshotChanceStat/staged_stat_pipeline_smoke_final.json`: `ok=true`.
    - Packaged build proves Headshot item/stat, fixed `2x` crit, stale `CritDamage` buff skip, `HeadshotChance=1.000` clamp, live auto-attack Headshot stun through `Hero_1_black_pierce`, legacy backend mapping, legacy `CritDamage=1.0` boundary mapping, legacy multiplier rejection, string-key serialization, retired UI text, boss stun, and `0.750s` target stun application.
  - `Reports/AgentReviews/20260531_HeadshotChanceStat/staged_item_taxonomy_smoke_final.json`: `ok=true`.
    - Packaged build proves OHKO and loot reward adjacent behavior remains valid.
- Diff hygiene:
  - `git diff --check` over touched text/code/data/docs files returned exit code `0`; only LF-to-CRLF working-copy warnings were reported.

## Known Caveats

- Existing unrelated warning/noise remains during editor/staged runs, including Steam unavailable warnings, missing local audio package warnings, existing ToonStyle material include warning, the project-level `LogAutomationTest` FText serialization startup error, and a known `Item_Alchemy` community-content warning.
- `Item_Headshot` currently references legacy Crit Damage sprite assets until dedicated Headshot sprites are created/imported.

</review_packet>
