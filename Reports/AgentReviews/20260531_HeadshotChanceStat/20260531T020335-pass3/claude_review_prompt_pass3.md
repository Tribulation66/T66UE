You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

Review depth: deepened.
Deepened validation is the risk-focused review mode. It is not a second implementation pass.
Keep the exact output headings below. Put risk/oversight content inside those headings; do not add headings.

For deepened review, actively look for:
- Hidden coupling, stale-doc/live-code mismatch, and assumptions that could make the plan incomplete.
- Unsafe cleanup, deprecation, deletion, migration, data reload, asset cook, or runtime/source mismatch consequences.
- Scope bleed, especially into explicitly excluded systems.
- Verification gaps, weak pass markers, rollback gaps, and evidence that would fail to prove the user's stated goal.
- The weakest implementation that could appear to pass while missing the real intent.

In Clarifying Questions, ask only user-owned decisions that block safe progress.
In Required Verification, name exact verification gaps and expected pass markers.
In Rationale, summarize the main assumption or oversight risk you challenged.

The first non-empty line of your review must be exactly one of these four lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: NEEDS_HUMAN_DECISION
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Verdict meanings:
- APPROVE: the reviewed plan/output is safe for Codex to proceed to implementation under the reviewed scope. Codex should not ask for redundant manual user approval after APPROVE unless the user explicitly marked the work planning-only, asked Codex to stop before implementation, the packet has an unresolved user-only decision, or AGENTS/PPF requires explicit approval for a method substitution.
- REVISE: Codex can resolve the issue by improving the plan/output, inspecting more repo state, tightening verification, changing implementation approach, or otherwise doing more Codex-owned work. Codex should revise and rerun review.
- NEEDS_HUMAN_DECISION: the plan/output depends on product direction, vision, risk acceptance, scope choice, or another decision only the user can make. Codex should save a decision block, ask once, and stop until the user answers.
- BLOCK: the plan/output cannot safely proceed because of a hard blocker, missing prerequisite, external-state issue, unavailable credential/context, or contradiction that is not solved by normal Codex revision.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\20260531_HeadshotChanceStat\completion_packet.md
- Output scope: review of the packet below only.

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

## Implemented Changes

- Data:
  - `Content/Data/Heroes.csv`: renamed `BaseCritDamage` to `BaseHeadshotChance`; authored hero values as `0.0`.
  - `Content/Data/Items.csv`: removed `Item_CritDamage`; added `Item_Headshot` as Accuracy/HeadshotChance.
  - `Content/Data/PlayerExperience.json`: added data-driven `HeadshotChancePerBonusPoint` and `HeadshotStunDurationSeconds`.
  - `/Game/Data/DT_Heroes`, `/Game/Data/DT_Items`, and `/Game/Data/DT_PlayerExperience` were reloaded from source data.
- Stat/runtime:
  - `ET66SecondaryStatType::HeadshotChance` added after the existing enum values; `CritDamage` is deprecated/compatibility-only.
  - Accuracy-family order is now `CritChance`, `HeadshotChance`, `AttackRange`, `Execute`.
  - `GetCritDamageMultiplier()` always returns `2.0`.
  - `GetSecondaryStatValue(CritDamage)` returns `2.0` as compatibility behavior.
  - `GetHeadshotChance01()` resolves from hero base, level-up bonus points, item bonuses, drug multipliers, and Accuracy-family multiplier.
  - `GetHeadshotStunDurationSeconds()` resolves from data.
- Combat:
  - Auto-attacks now roll Headshot Chance after successful damage application and apply a data-driven stun to hit enemies/mobs/bosses.
  - Non-shipping automation hook `DebugApplyHeadshotStunForAutomation` verifies the stun path.
  - Headshot stun path refreshes its run-state subsystem cache if needed before resolving chance/duration.
- UI/localization:
  - Stats panel, hero selection stat snapshot, power-up/drug display, run summary, temporary buff slug, and item localization use Headshot Chance instead of Crit Damage.
- Backend/save compatibility:
  - Serializer has explicit `HeadshotChance` key support.
  - Parser maps both `HeadshotChance` and legacy `CritDamage` backend keys into live `HeadshotChance`.
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
  - Reload completed with `0 Problems` for `DT_Items`.
- Build:
  - `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReload`: succeeded.
  - `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development`: succeeded and produced `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Editor runtime smokes:
  - `Reports/AgentReviews/20260531_HeadshotChanceStat/stat_pipeline_smoke.json`: `ok=true`.
    - Proves Headshot item replaces Crit Damage item.
    - Proves Crit damage is fixed at `2x` and `CritDamage` is not live.
    - Proves Headshot item and Headshot drug raise Headshot Chance.
    - Proves Headshot Chance can stun a hit target: `Chance=0.500 StunRemaining=0.750`.
    - Proves legacy backend `CritDamage` key maps to live `HeadshotChance`.
  - `Reports/AgentReviews/20260531_HeadshotChanceStat/item_taxonomy_smoke.json`: `ok=true`.
    - Proves Execute/Assassinate/Crush reject bosses and allow miniboss enemies/mobs.
    - Proves Loot Bag and Loot Wheel reward improvements.
    - Proves retired item IDs skip inventory, including `Item_CritDamage`.
- Staged standalone:
  - Shortcut `C:\UE\T66\T66 Standalone.lnk` target verified as `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`; target exists.
  - `Reports/AgentReviews/20260531_HeadshotChanceStat/staged_stat_pipeline_smoke.json`: `ok=true`.
    - Packaged build proves Headshot item/stat, fixed `2x` crit, legacy backend mapping, and `0.750s` stun application.
  - `Reports/AgentReviews/20260531_HeadshotChanceStat/staged_item_taxonomy_smoke.json`: `ok=true`.
    - Packaged build proves OHKO and loot reward adjacent behavior remains valid.
- Diff hygiene:
  - `git diff --check` over touched text/code/data/docs files returned exit code `0`; only LF-to-CRLF working-copy warnings were reported.

## Known Caveats

- Existing unrelated warning/noise remains during editor/staged runs, including Steam unavailable warnings, missing local audio package warnings, existing ToonStyle material include warning, and a known `Item_Alchemy` community-content warning.
- `Item_Headshot` currently references legacy Crit Damage sprite assets until dedicated Headshot sprites are created/imported.

</review_packet>
