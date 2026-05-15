# Pending Issues - Data

## Empty Stage Slots In Stages 1-3

- Severity tag: [Minor]
- What's wrong: `Content/Data/Stages.csv` uses 10 enemy slots per stage, but LocalStage 1 intentionally leaves three slots empty, LocalStage 2 leaves two slots empty, and LocalStage 3 leaves one slot empty.
- Why it's out of scope now: The production gating rule requires progressive slot fill across the four local stages.
- What fixing it would entail: Update spawn/UI consumers to treat `None` enemy slots as normal authored gaps, then revisit the data only if design changes the gating rule.

## Status Effects Not Assigned To Production Mobs

- Severity tag: [Minor]
- What's wrong: `Content/Data/Enemies.csv` assigns `StatusEffectOnHit=None` for all 50 production mobs, so existing status effects are not used by the new roster.
- Why it's out of scope now: The task explicitly defers status effect assignment to a future design pass.
- What fixing it would entail: Design per-archetype or per-mob status mappings, update `Enemies.csv`, and validate combat/UI presentation for each effect.

## Community Challenge References Invalid Starting Item

- Severity tag: [Minor]
- What's wrong: The staged gameplay smoke logged `Item_Alchemy` as a missing row in `/Game/Data/DT_Items.DT_Items`, then cleared it from the `Super Duper Challange` community starting item list.
- Why it's out of scope now: The Easy mob VAT pass did not touch community challenge definitions, item authoring, or item-data migration.
- What fixing it would entail: Locate the community challenge data source, either add a valid `Item_Alchemy` row to the item table or replace that starting item with an existing row, then verify the community-content load path in staged standalone.
