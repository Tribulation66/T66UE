# Pending Issues - Data

## Status Effects Not Assigned To Production Mobs

- Severity tag: [Minor]
- What's wrong: `Content/Data/Enemies.csv` assigns `StatusEffectOnHit=None` for all 50 production mobs, so existing status effects are not used by the new roster.
- Why it's out of scope now: This is not a table-only fix. `Scripts/ValidateEnemyBossRosterData.py` currently requires `StatusEffectOnHit=None` for this roster pass, and normal production mobs do not yet consume `StatusEffectOnHit` in their hit path.
- What fixing it would entail: Implement normal-mob status application in combat, decide the per-archetype or per-mob mappings, update `Enemies.csv` and the roster validator, then validate combat and UI presentation for each effect.

## Staged Smoke Logs Missing Data References

- Severity tag: [Minor]
- What's wrong: The Phase 1C FinalFoundationPass stage/smoke logs reported missing `/Game/Data/DT_HouseNPCs`, missing world visual prop references from `/Game/Data/DT_WorldVisualProps` to `/Game/World/VisualProps/Easy/WallLamp_Pixal3D`, `WallTorch_Pixal3D`, `BrokenVase_Pixal3D`, and `SkullRemains_Pixal3D`, plus item rows such as `Item_GamblersToken` and `Item_Alchemy` missing from `/Game/Data/DT_Items`.
- Latest evidence: The 2026-06-08 tooltip infrastructure staged readiness gate failed during cook with the same missing `/Game/Data/DT_HouseNPCs` package and a missing `/Game/World/VisualProps/Easy/SM_BrokenVase_Easy_Pixal3D` reference from `DT_WorldVisualProps`.
- Why it's out of scope now: The FinalFoundationPass only activated ToonStyle close-the-gap and inner-line infrastructure. It did not change house NPC, world visual prop, community content, or item data tables.
- What fixing it would entail: Audit the referenced data tables and cooked package references, either restore/create the missing assets and item rows or remove/redirect stale references, then cook/stage and confirm the warnings are gone.
- Update 2026-06-09: `BP_T66GameInstance`'s stale `NPCsDataTable` default (`DT_HouseNPCs`) repointed to `/Game/Data/DT_NPCs` via `FixGameInstanceNPCsTableRef.py` — binary no longer contains the old package name. Dead `BrokenVase_Easy` row removed from `WorldVisualProps.json` (asset purged in v1.2) and `DT_WorldVisualProps` reloaded. `Item_GamblersToken`/`Item_Alchemy` references not found anywhere in Source/data — expected stale-cook residue; confirm via the next staged cook log, then close.

## Resolved: Shared HpRegen And LifeSteal Item Sprites Still Present

- Severity tag: [Resolved - Minor]
- What's wrong: `Content/Items/Sprites/Item_HpRegen_*.uasset` and `Content/Items/Sprites/Item_LifeSteal_*.uasset` remain after the main-run `Content/Data/Items.csv` rows for `Item_HpRegen` and `Item_LifeSteal` were removed.
- Why it's out of scope now: The current cleanup is scoped to the main-run item system. A Mini-inclusive ownership/reference pass has not been done, and the shared sprite assets should not be deleted until Mini-owned references are audited.
- What fixing it would entail: Run a Mini-inclusive reference audit for those sprite assets, update or remove any Mini-owned references, delete the no-longer-owned sprite assets and source import records, refresh affected data assets, and stage/smoke the standalone build.
- Resolution: Resolved 2026-06-09: Mini-inclusive reference audit (Source, Scripts, RuntimeDependencies, Content CSVs/JSON, binary scan of Content/Blueprints) found zero sprite references — only retired-ID `FName` compat checks, which need no art. Deleted the 8 `Item_HpRegen_*`/`Item_LifeSteal_*` uassets and dropped both prefixes from `ImportItemSprites.py`'s preserve list (comment documents the audit).

## Resolved: Headshot Item Still Reuses Legacy Crit Damage Sprites

- Severity tag: [Resolved - Minor]
- What's wrong: `Content/Data/Items.csv` now uses live row `Item_Headshot`, but its icon paths still point at `Content/Items/Sprites/Item_CritDamage_*.uasset` because no `Item_Headshot_*` sprite assets currently exist.
- Why it's out of scope now: This pass is a stat/data/combat rename and behavior change. Raw-copying or renaming Unreal `.uasset` files without an editor asset-rename pass would risk broken internal object names and stale redirectors.
- What fixing it would entail: Create or properly rename/import `Item_Headshot_*` sprite assets through the Unreal asset pipeline, update `Items.csv` paths, refresh `/Game/Data/DT_Items`, then run the item/UI smoke path to confirm the icons resolve.
- Resolution: Resolved by v1.2 (verified 2026-06-09): `Items.csv` row `Item_Headshot` points at `Item_Headshot_<rarity>` sprites and all four uassets exist under `Content/Items/Sprites`; `DT_Items` (6/8 09:54) is newer than `Items.csv` (6/8 09:52).

## Resolved: StatusEffects CSV Import Emits Missing Name And Cell Count Warnings

- Severity tag: [Resolved - Minor]
- What's wrong: The FriendSlop raw Pixal3D reload logged `DT_StatusEffects` import warnings: row `1` is missing a name, and several rows such as `Webbed`, `Poisoned`, `Rooted`, `Thorned`, `Bleeding`, `ArmorCracked`, `Cursed`, `Shocked`, `Chilled`, `Burning`, and `Dazed` have too few cells.
- Why it's out of scope now: The current task only corrected raw FriendSlop hero orientation. The warning is outside the `CharacterVisuals.csv` yaw fix and did not block the raw FriendSlop validator.
- What fixing it would entail: Audit `Content/Data/StatusEffects.csv` against its DataTable struct, repair row names and cell counts, rerun the owning DataTable reload path, and verify the import warnings disappear.
- Resolution: Resolved 2026-06-09: rows 2-12 were missing the `DisplayName` cell (all values shifted one column left — `Status_*` icon IDs sat in `DisplayName`). Inserted proper display names; reimported via `SetupCombatRosterDataTables.py` (`DT_StatusEffects` re-saved 21:17). Cook-warning absence check rides the next staged build.
