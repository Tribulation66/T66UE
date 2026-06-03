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

The first non-empty line of your review must be exactly one of these three lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Only use Verdict: APPROVE when the reviewed plan/output is safe for Codex to present as greenlit. If implementation still requires user go-ahead under AGENTS.md, APPROVE means safe to present at that go-ahead gate, not permission to skip the gate.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_ItemSpecialPrimaryCleanup\review_packet_pass2.md
- Output scope: review of the packet below only.

<review_packet>
# Claude Plan Review Packet - Pass 2

## Working Goal

Remove the deprecated HpRegen and LifeSteal main-run item templates, introduce a `Special` primary item category for Quick Revive and Gambler's Token, refresh item data assets, and verify the focused T66 item-system changes.

## User Request

The user wants infrastructure changes now:

- Delete HP regen and lifesteal items altogether, not just deprecate them.
- `Item_BackroomsQuickRevive` and the gambler token should have primary category `Special`.
- Special items should not fall into the other seven item/stat categories like Armor.

## Constraints And Instructions

- Root AGENTS instructions apply.
- Default scope excludes Mini/minigame systems. Do not edit Mini data/assets in this pass.
- Data asset import/reload process applies: CSV-backed Unreal DataTable must be refreshed via the owning import script after source edits.
- Staged standalone verification applies because runtime item data/code changes affect the playable build.
- PPF is not required: this is not a visual/media/process-governed production asset task; it is a focused code/data cleanup and enum/category change.
- Use current live repo state, not stale docs.

## Folder Instructions Read

- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/README.md`
- `Reports/AGENTS.md`
- `Tools/README.md`
- Existing pending issue files:
  - `Content/Data/pending_issues_Data.md`
  - `Source/T66/Data/pending_issues_Data.md`
  - `Source/T66/Core/pending_issues_Core.md`
  - `Source/T66/Core/RunState/pending_issues_RunState.md`

## Audit Since Pass 1

### Deprecated Item ID References

Command:

```powershell
rg -n "Item_HpRegen|Item_LifeSteal" Source Content Gameplay Scripts Tools -g "!*Mini*"
```

Findings:

- Production main-run item rows:
  - `Content/Data/Items.csv`: `Item_HpRegen`
  - `Content/Data/Items.csv`: `Item_LifeSteal`
- Historical doc note:
  - `Gameplay/Stats/Accuracy_Item_And_TempBuff_Audit.md`
- No main-run C++ hard-coded `Item_HpRegen` or `Item_LifeSteal` references were found outside docs and the CSV.
- A broader search did find Mini data references, but Mini is explicitly out of scope. Shared sprites will not be deleted in this pass because Mini still references them.

### HpRegen/LifeSteal Stat Runtime References

Command:

```powershell
rg -n "HpRegen|LifeSteal" Source Content Gameplay Scripts Tools -g "!*Mini*"
```

Findings:

- `ET66SecondaryStatType::HpRegen` and `ET66SecondaryStatType::LifeSteal` remain in broader runtime, hero/base-stat, backend serialization, localization, buff, combat, temp-buff UI, and deprecated-stat compatibility paths.
- The requested deletion is scoped to item templates, not removal of these runtime secondary stat enum values or mechanics.
- Removing the secondary enum values would be a wider save/backend/runtime compatibility task and is intentionally out of scope.

### `ET66HeroStatType` Consumers

Command:

```powershell
rg -n "case ET66HeroStatType|ET66HeroStatType::" Source\T66 -g "!*Mini*"
```

Findings:

- Stat-application code in `T66RunStateSubsystem_Stats.cpp` and `T66RunStateSubsystem_EconomyInventory.cpp` uses explicit switches with defaults/no-ops. `Special` should not add stats and can safely fall through/default.
- `T66RunStateSubsystem_Private.h` maps primary categories to secondaries. Default returns empty, which is desired for `Special`.
- Buff and power-up systems use explicit stat pools. `Special` should not be inserted into those pools.
- Loot wheel, arcade, boost, and account/power-up screens use explicit eight-stat boost arrays. `Special` should stay excluded.
- `T66ItemCardTextUtils.cpp` needs a `Special` label so item cards do not fall back to empty/fallback text when a special item is displayed.
- `T66CollectorOverlayWidget.cpp` uses the enum display name, so appending `Special UMETA(DisplayName = "Special")` is enough for that path.

### Random Item Pool

`T66GameInstance.cpp` already excludes:

- `Item_GamblersToken`
- `Item_BackroomsQuickRevive`

from `IsRandomItemPoolEligible()`. The deletion of `Item_HpRegen` and `Item_LifeSteal` from `Items.csv` reduces normal CSV rows from 31 to 29. The normal random pool remains the main live item set, with Quick Revive and Gambler's Token excluded by existing ID gates.

### Save Compatibility

Existing inventory recomputation looks up each stored item row and skips missing item data rows. After deleting `Item_HpRegen` and `Item_LifeSteal`, old saves that still contain those IDs should stop receiving stats from those missing templates rather than crashing. A save migration that purges legacy slot IDs is a separate compatibility cleanup and is not part of this focused infrastructure pass unless review identifies a concrete crash path.

## Proposed Edits

1. `Source/T66/Data/T66DataTypes.h`
   - Append `Special UMETA(DisplayName = "Special")` after `Accuracy` in `ET66HeroStatType` so existing serialized enum values are preserved.
   - Update the `FItemData::PrimaryStatType` comment to mention `Special` as the reward-only/non-stat category.

2. `Content/Data/Items.csv`
   - Delete `Item_HpRegen`.
   - Delete `Item_LifeSteal`.
   - Change `Item_BackroomsQuickRevive` `PrimaryStatType` from `Armor` to `Special`.

3. `Source/T66/Core/T66GameInstance.cpp`
   - Change synthetic `Item_GamblersToken` `PrimaryStatType` from `Luck` to `Special`.
   - Keep existing random-pool exclusions unchanged.

4. `Source/T66/UI/T66ItemCardTextUtils.cpp`
   - Add `Special` label handling in the localized-label switch and fallback switch.

5. `Tools/ArtPipeline/Items/T66ProcessReimaginedItemSheets.py`
   - Remove the `Armor/HpRegen` and `Evasion/LifeSteal` sheet series from the reimagined item pipeline so future batches do not regenerate deleted item templates.

6. `Gameplay/Stats/Accuracy_Item_And_TempBuff_Audit.md`
   - Update the historical deprecation note to state that main-run `Item_HpRegen` and `Item_LifeSteal` were removed from `Content/Data/Items.csv` by this cleanup, while `Item_Alchemy` remains a separate legacy row.

7. `Content/Data/pending_issues_Data.md`
   - Add a minor pending issue noting that shared HpRegen/LifeSteal sprite assets remain because Mini data still references them and Mini is out of scope. This prevents claiming a full shared asset deletion under a non-Mini request.

## Explicit Non-Edits

- Do not remove `ET66SecondaryStatType::HpRegen` or `ET66SecondaryStatType::LifeSteal`.
- Do not remove runtime HP regen/life steal mechanics.
- Do not edit Mini/minigame CSVs, code, or assets.
- Do not delete shared `Content/Items/Sprites/Item_HpRegen_*` or `Item_LifeSteal_*` assets in this pass because Mini still references them.
- Do not add `Special` to boost/power-up/stat-wheel pools.

## Risks

- Adding an enum value can break compile paths with exhaustive switches that lack a default. The audit found explicit default/no-op paths or deliberate eight-stat arrays; the build will verify this.
- Existing old saves may retain removed item IDs. Current data lookup behavior should skip missing rows; if verification exposes a crash or warning path, address the smallest focused guard.
- Special items should not accidentally render as blank category labels. `T66ItemCardTextUtils.cpp` label handling addresses the visible item-card path.

## Verification Plan

1. Static source/data checks:
   - CSV has no `Item_HpRegen` or `Item_LifeSteal`.
   - `Item_BackroomsQuickRevive.PrimaryStatType == Special`.
   - `T66GameInstance.cpp` synthetic `Item_GamblersToken.PrimaryStatType == Special`.
   - Main-run source search has no production `Item_HpRegen`/`Item_LifeSteal` references outside updated historical docs/pending notes.
   - Random-pool exclusion still excludes Quick Revive and Gambler's Token.

2. Build:
   - Run UE 5.7 focused editor build for `T66Editor Win64 Development`.

3. DataTable reload:
   - Run `Scripts/SetupItemsDataTable.py` through `UnrealEditor-Cmd.exe` to refresh `/Game/Data/DT_Items` from `Content/Data/Items.csv`.

4. DataTable/source proof:
   - Re-run CSV checks after the Unreal reload.
   - Confirm the reload command completes and saves `DT_Items` without import failure.

5. Standalone:
   - Run `Scripts/StageStandaloneBuild.ps1` after the data refresh.
   - Verify the taskbar shortcut target remains `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
   - Launch/smoke the staged executable if the existing staging helper provides that path; otherwise report the exact skipped smoke reason and rely on build + DataTable reload + static pool proofs.

## Requested Review

Please review this revised plan for:

- Any missed main-run references to `Item_HpRegen` or `Item_LifeSteal`.
- Any unsafe `ET66HeroStatType` consumer that requires an explicit `Special` case beyond item-card labeling.
- Whether save compatibility needs a code migration now or whether existing missing-row skip behavior is sufficient for this focused pass.
- Whether the verification plan is adequate for the requested infrastructure change.

Return the first non-empty line exactly as `Verdict: APPROVE`, `Verdict: REVISE`, or `Verdict: BLOCK`.

</review_packet>
