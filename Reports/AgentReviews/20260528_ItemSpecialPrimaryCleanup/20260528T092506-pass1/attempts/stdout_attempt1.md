Verdict: REVISE

## Blockers
- None that prevent revision, but see Major Issues — several missing audits could turn into blockers once Codex starts editing.

## Major Issues
- **No audit for `Item_HpRegen`/`Item_LifeSteal` ID references in main-run C++/Blueprints/data.** The packet only cites CSV, the art-pipeline script, and Mini. It does not show a repo-wide search for these string IDs in main-run sources (achievements, tutorials, loot tables, test data, save-migration code, design docs). Deleting CSV rows without that audit risks dangling lookups that fail silently at runtime.
- **No audit of `ET66HeroStatType` switch consumers.** The risk section says "inspect core switch consumers and build," but the plan doesn't list which call sites were checked. Exhaustive `switch`es with no `default:` will fail to compile when `Special` is appended; non-exhaustive ones may silently mis-handle Quick Revive/Gambler's Token in icon/color/sort/tooltip paths. Required reading before edit, not after.
- **UI/HUD behavior for `Special` primary not addressed.** Inventory tiles, tooltips, and category filters typically key off primary stat type. The plan doesn't say what is displayed for `Special` (icon, label, color), nor whether it falls through to a sensible default. Quick Revive and Gambler's Token are visible to the player — this is the headline UX change of the cleanup.
- **Save/load compatibility not addressed.** If existing saves reference `Item_HpRegen` or `Item_LifeSteal`, the plan should state explicitly whether the loader tolerates missing item IDs or whether a migration is needed. Even in alpha this is worth one sentence.
- **Verification is build + CSV diff only.** No standalone smoke step confirms that (a) Quick Revive still grants/consumes correctly, (b) Gambler's Token still appears as a reward and applies its secondary, (c) random pools no longer surface HpRegen/LifeSteal. Staging a build without exercising the changed items is hollow verification for "playable standalone data/code."

## Minor Issues
- The "28 live modifier rows" count is asserted without showing the current row count or derivation. State the pre/post counts so the diff is checkable.
- Insertion point for `Special` is described as "after existing values" but the enum already has `Accuracy` appended. Specify literally: `…, Speed, Accuracy, Special`. Removes ambiguity for the implementer.
- New `pending_issues_Sprites.md` location is proposed without confirming the directory's existing convention. If `Content/Items/Sprites/` has no other `pending_issues_*.md`, prefer the established `Content/Data/pending_issues_Data.md` or whichever Sprites-owning doc already exists.
- `T66ProcessReimaginedItemSheets.py` edit: confirm there are no companion manifests, JSON, or test fixtures pinned to HpRegen/LifeSteal in `Tools/ArtPipeline/Items/` that would now be stale.
- The reload step `SetupItemsDataTable.py` is named — confirm the script exists at that path and is the current canonical importer; the repo also has `SetupWeaponsDataTable.py` styling, so the name is plausible but unverified in the packet.

## Clarifying Questions
- Should `Special` show any HUD/inventory affordance distinct from the seven main stats, or is the intent purely "exclude from progression buckets, keep cosmetic default"?
- Is there an existing save-migration path for deprecated item IDs, or do we accept that pre-cleanup saves may log warnings?
- Is removing the rows from `Items.csv` preferable to keeping them deprecated for one release as a soft-deprecation? The user said "delete" — confirming that overrides any in-place deprecation pattern the repo uses.

## Required Verification
- Repo grep for `Item_HpRegen` and `Item_LifeSteal` across `Source/`, `Content/`, `Gameplay/`, `Scripts/`, `Tools/` — produce the hit list before editing, and resolve each main-run hit.
- Repo grep for every `switch` on `ET66HeroStatType` and confirm each either has a `default:` or is updated to handle `Special` explicitly.
- Confirm DT_Items reload succeeds and that `/Game/Data/DT_Items` shows Quick Revive primary = `Special` post-reload.
- Standalone smoke: launch the staged build, acquire/use Quick Revive, acquire Gambler's Token, complete one run; verify no warnings/errors mentioning the deleted IDs in `Saved/Logs`.
- Confirm random pool spawns over a scripted sample (or a sufficiently long run) never surface HpRegen/LifeSteal.

## Rationale
The scope is correctly bounded to main-run, the deferral of shared sprite deletion due to Mini references is the right call, and the enum-append strategy preserves ordinals. However, the plan treats this as a CSV+enum edit when it is actually a cross-cutting change affecting UI display, save compatibility, switch exhaustiveness, and player-visible items. Before greenlighting, Codex should produce the reference audit, name the switch consumers, and commit to a behavioral smoke test rather than a build-and-stage check. None of this is fundamentally unsafe — it is incomplete homework — so REVISE rather than BLOCK.

