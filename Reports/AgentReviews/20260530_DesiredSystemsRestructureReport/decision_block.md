# Decision Block: Desired Systems Restructure

Generated: 2026-05-30  
Source: Claude validator review of `comprehensive_change_implications_report.md`

## Gate Status

Claude validated that the read-only report itself satisfies the requested report stop condition. The `NEEDS_HUMAN_DECISION` verdict applies to any follow-on implementation work, not to the existence of the report.

## Current Stop Point

Do not proceed into implementation until the user chooses the next phase direction and resolves or authorizes assumptions for the product decisions below.

## Decision Choices

1. Stop here for now and use the report as the handoff packet for the isolated Claude agent.
2. Start a Phase 0 data-contract pass next, still read-only/planning-first, to convert the report into exact implementation contracts.
3. Start implementation only after Phase 0 is approved and the open product decisions have answers or explicit assumptions.

## Cleanup Posture Choice

Choose one before implementation:

1. Conservative: keep old IDs/assets as compatibility aliases, hide old paths from UI, migrate saves, clean later.
2. Moderate: remove obsolete CSV rows after migration, keep assets for one release cycle, keep legacy enum/save compatibility.
3. Aggressive: delete old rows, old UI/game paths, and unused assets after reference audit, with an explicit old-save policy.

## Product Decisions Needed Before Implementation

- Final locked weapon branch for Heroes 6-12.
- Whether weapon altar still shows four cards or becomes one locked weapon pickup per stage/rarity.
- Exact rarity/stage mapping confirmation.
- Whether old weapon rows move from 192 rows to 48 rows immediately or remain deprecated/aliased first.
- Idol element page order and whether No Idol loops back to Fire.
- No Idol stat values, scaling, tier handling, and whether it occupies an equipped idol slot.
- Final mapping from existing idol attacks/animations to the new element/type grid.
- Elemental Power formula and caps for idol damage, attack speed, and attack scale.
- Whether elemental power appears only in stat summary or also in item cards/tooltips/run summary.
- Mob Loot representation: world pickup actor, stackable inventory item, or run-state currency displayed as a sell card.
- Mob Loot drop amount by difficulty and whether bosses/specials drop different amounts.
- Final four gambler games, wager limits, payout rounding, and result presentation.
- Pet capture rules: every stage boss or selected bosses, guaranteed or chance-based, first-time only or repeatable, one active pet or multiple active pets, and timing relative to descent/stage clear.
- Old-save policy for weapon, idol, item, and gambler changes.
