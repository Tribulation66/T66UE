Verdict: REVISE

## Blockers

None.

## Major Issues

- **Enum-iteration consumers not surveyed.** The runtime/UI grep covered `case ET66HeroStatType` and `ET66HeroStatType::`, but did not search for iteration patterns such as `StaticEnum<ET66HeroStatType>`, `GetMaxEnumValue`, `NumEnums`, `MAX`/sentinel-based loops, or stat-pool arrays sized by the enum. Adding `Special` to a list-driven UI surface (rolling pools, "all stats" panels, stat-roll generators) could silently inject a non-stat category. The plan asserts explicit eight-stat pools elsewhere but does not show the grep that proves it. Add this scan before edit.
- **ArtPipeline change and Mini exclusion interact.** `T66ProcessReimaginedItemSheets.py` is being trimmed of `Armor/HpRegen` and `Evasion/LifeSteal` series. The shared sprite uassets are kept because "Mini-owned data still references them," but the script that regenerates them is being modified in the same pass. Confirm explicitly that Mini does not depend on this script to regenerate those sheets — otherwise the Mini-owned references will rot the next time art is rebuilt. If the script is in fact main-run-only and Mini owns its own pipeline, state that with a path reference; if there is any shared regeneration path, the script edit needs to be deferred to the Mini-inclusive pass alongside the sprite cleanup.

## Minor Issues

- **Saved inventory slot fate.** The compatibility evidence shows `RecomputeItemDerivedStats` skips missing rows, but does not address what happens to a saved slot still holding `Item_HpRegen` or `Item_LifeSteal` as a template ID at the inventory UI layer (icon resolution, tooltip render, drop/sell flows). Confirm or note that the slot displays as empty/unknown rather than crashing on missing sprite/data lookup.
- **`T66CollectorOverlayWidget` will literally display "Special" via `GetDisplayNameTextByValue`.** That is described as "correct," but `Special` is not a stat. Confirm the overlay context (item card / item info) is one where the user reads it as a category label, not a stat readout. If it's the latter, a non-stat string in a stats grid is a UX bug worth catching now.
- **Pending issue placement.** The shared sprite cleanup goes only into `Content/Data/pending_issues_Data.md`. Given the entry references `Content/Items/Sprites/...`, consider whether `Source/T66/Data/pending_issues_Data.md` or a Content-side equivalent closer to `Content/Items/Sprites/` is the canonical location. Not blocking, but verify routing convention.
- **Historical doc update scope.** `Gameplay/Stats/Accuracy_Item_And_TempBuff_Audit.md` is the only doc named. A quick grep for other Gameplay/* docs that mention HpRegen/LifeSteal item rows (as opposed to the secondary-stat mechanic) would close the loop.

## Clarifying Questions

- Has anyone grepped for `StaticEnum<ET66HeroStatType>`, `GetEnumeratorIndex`, `MAX`, or array literals built from this enum? Please paste the result before edit.
- Is `T66ProcessReimaginedItemSheets.py` exclusively main-run, or do Mini build flows invoke it for shared sprites? A one-line confirmation with the call-site path is enough.
- For Quick Revive specifically, will the item card actually render `PrimaryStatType = Special` as a stat row, or is the card layout suppressed for non-stat primaries? If it renders, what is the intended visual treatment?

## Required Verification

The plan's verification sequence (CSV/reference grep → focused build → `SetupItemsDataTable.py` reload → `StageStandaloneBuild.ps1` → staged exe to menu → staged exe with `-T66Entry=Run:TestRoom` → `.sav` rescan) is acceptable as stated. Add:

- Post-edit grep for enum-iteration consumers (see Major Issues) with results pasted.
- In the staged smoke run, pick up Quick Revive (or force-grant it) and observe the item card to confirm the `Special` label renders sanely and no `T66CollectorOverlayWidget` regressions occur.
- Re-grep `Tools/ArtPipeline` and `Scripts` post-edit for `HpRegen|LifeSteal` to confirm nothing else regenerates the deleted series.

## Rationale

Edits, enum append position, CSV deltas, Mini exclusion handling, and staged smoke launches are well-scoped and address Pass 1/Pass 2 feedback. The two unresolved items are evidentiary, not structural: (1) the enum-iteration audit is missing and is the most likely place a `Special` insertion silently leaks into a stat pool; (2) the ArtPipeline script edit shares a boundary with the deferred shared-sprite cleanup and needs an explicit ownership statement so Mini doesn't break later. Both are tractable before edit, so REVISE rather than BLOCK; both are real enough that APPROVE without them is premature.

