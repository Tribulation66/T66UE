# Pending Issues - Data

## Status Effects Not Assigned To Production Mobs

- Severity tag: [Minor]
- What's wrong: `Content/Data/Enemies.csv` assigns `StatusEffectOnHit=None` for all 50 production mobs, so existing status effects are not used by the new roster.
- Why it's out of scope now: This is not a table-only fix. `Scripts/ValidateEnemyBossRosterData.py` currently requires `StatusEffectOnHit=None` for this roster pass, and normal production mobs do not yet consume `StatusEffectOnHit` in their hit path.
- What fixing it would entail: Implement normal-mob status application in combat, decide the per-archetype or per-mob mappings, update `Enemies.csv` and the roster validator, then validate combat and UI presentation for each effect.

## Staged Smoke Logs Missing Data References

- Severity tag: [Minor]
- What's wrong: The Phase 1C FinalFoundationPass stage/smoke logs reported missing `/Game/Data/DT_HouseNPCs`, missing world visual prop references from `/Game/Data/DT_WorldVisualProps` to `/Game/World/VisualProps/Easy/WallLamp_Pixal3D`, `WallTorch_Pixal3D`, `BrokenVase_Pixal3D`, and `SkullRemains_Pixal3D`, plus item rows such as `Item_GamblersToken` and `Item_Alchemy` missing from `/Game/Data/DT_Items`.
- Why it's out of scope now: The FinalFoundationPass only activated ToonStyle close-the-gap and inner-line infrastructure. It did not change house NPC, world visual prop, community content, or item data tables.
- What fixing it would entail: Audit the referenced data tables and cooked package references, either restore/create the missing assets and item rows or remove/redirect stale references, then cook/stage and confirm the warnings are gone.
